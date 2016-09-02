/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gamma.h>

#include <algorithm>
#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/util/strfmt.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma-normal-form.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/seqtype.h>
#include <jive/vsdg/traverser.h>

namespace jive {

gamma_head_op::~gamma_head_op() noexcept
{
}

size_t
gamma_head_op::nresults() const noexcept
{
	return 1;
}

const base::type &
gamma_head_op::result_type(size_t index) const noexcept
{
	return seq::seqtype;
}

std::string
gamma_head_op::debug_string() const
{
	return "GAMMA_HEAD";
}

std::unique_ptr<jive::operation>
gamma_head_op::copy() const
{
	return std::unique_ptr<jive::operation>(new gamma_head_op(*this));
}

gamma_tail_op::~gamma_tail_op() noexcept
{
}

size_t
gamma_tail_op::narguments() const noexcept
{
	return 1;
}

const base::type &
gamma_tail_op::argument_type(size_t index) const noexcept
{
	return seq::seqtype;
}

std::string
gamma_tail_op::debug_string() const
{
	return "GAMMA_TAIL";
}

std::unique_ptr<jive::operation>
gamma_tail_op::copy() const
{
	return std::unique_ptr<jive::operation>(new gamma_tail_op(*this));
}

gamma_op::~gamma_op() noexcept
{
}

size_t
gamma_op::narguments() const noexcept
{
	return 1 + nalternatives();
}

const base::type &
gamma_op::argument_type(size_t index) const noexcept
{
	if (index < nalternatives()) {
		static const achr::type anchor_type;
		return anchor_type;
	} else {
		return predicate_type_;
	}
}
std::string
gamma_op::debug_string() const
{
	return "GAMMA";
}

std::unique_ptr<jive::operation>
gamma_op::copy() const
{
	return std::unique_ptr<jive::operation>(new gamma_op(*this));
}

bool
gamma_op::operator==(const operation & other) const noexcept
{
	const gamma_op * op = dynamic_cast<const gamma_op*>(&other);
	return op && op->predicate_type_ == predicate_type_;
}

}

jive::node_normal_form *
jive_gamma_node_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
{
	jive::gamma_normal_form * normal_form = new jive::gamma_normal_form(
		operator_class, parent, graph);
	return normal_form;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::gamma_op), jive_gamma_node_get_default_normal_form_);
}


static jive_node *
jive_gamma_create(
	jive::output * predicate,
	const std::vector<const jive::base::type*> & types,
	const std::vector<std::vector<jive::output*>> & alternatives)
{
	size_t nvalues = types.size();
	size_t nalternatives = alternatives.size();
	jive_region * region = predicate->node()->region();

	jive::output * arguments[nalternatives+1];
	for (size_t n = 0; n < nalternatives; n++) {
		jive_region * subregion = new jive_region(region, region->graph);
		jive_node * head = jive::gamma_head_op().create_node(subregion, 0, nullptr);
		jive_node * tail = jive::gamma_tail_op().create_node(subregion, 1, &head->outputs[0]);
		arguments[n] = tail->outputs[0];
	}
	arguments[nalternatives] = predicate;

	jive_node * gamma = jive::gamma_op(nalternatives).create_node(region, nalternatives+1, arguments);
	
	for (size_t n = 0; n < nvalues; n++) {
		jive::gate * gate_head = jive_graph_create_gate(
			region->graph, jive::detail::strfmt("head_", gamma, "_", n), *types[n]);
		jive::gate * gate_tail = jive_graph_create_gate(
			region->graph, jive::detail::strfmt("gamma_", gamma, "_", n), *types[n]);

		for (size_t i = 0; i < nalternatives; i++) {
			jive_node * head = arguments[i]->node()->producer(0);
			head->add_input(gate_head, alternatives[i][n]);
			jive::output * value = head->add_output(gate_head);
			arguments[i]->node()->add_input(gate_tail, value);
		}
		gamma->add_output(gate_tail);
	}

	return gamma;
}

std::vector<jive::output *>
jive_gamma(jive::output * predicate,
	const std::vector<const jive::base::type*> & types,
	const std::vector<std::vector<jive::output*>> & alternatives)
{
	/*
		FIXME: This code is supposed to be in gamma-normal-form.c and nowhere else. However,
		we would need to extend the gamma operator with the types vector in order to make
		this possible.
	*/

	if (auto ctl = dynamic_cast<const jive::ctl::type*>(&predicate->type())) {
		if (ctl->nalternatives() != alternatives.size())
			throw jive::compiler_error("Incorrect number of alternatives.");
	}

	for (size_t n = 0; n < alternatives.size(); n++) {
		if (alternatives[n].size() != types.size())
			throw jive::compiler_error("Incorrect number of values for an alternative.");
	}

	/*
		FIXME: This is currently not guarded by any normal form attribute. We could make
		it a special case where two regions are equal, but we have to introduce this
		optimization first.
	*/
	if (alternatives.size() == 1)
		return alternatives[0];

	jive_graph * graph = predicate->node()->graph();
	jive::gamma_normal_form * nf = static_cast<jive::gamma_normal_form *>(
		jive_graph_get_nodeclass_form(graph, typeid(jive::gamma_op)));
	
	if (nf->get_mutable() && nf->get_predicate_reduction()) {
		if (auto op = dynamic_cast<const jive::ctl::constant_op*>(&predicate->node()->operation()))
				return alternatives[op->value().alternative()];
	}

	std::vector<jive::output*> results;
	jive_node * node = jive_gamma_create(predicate, types, alternatives);
	for (size_t n = 0; n < node->noutputs; n++)
		results.push_back(node->outputs[n]);

	/*
		FIXME: this algorithm is O(n^2), since we have to iterate through all inputs/outputs
		that have a greater index than the deleted one
	*/
	if (nf->get_mutable() && nf->get_invariant_reduction()) {
		results.clear();
		jive_node * tail0 = node->producer(0);
		jive_node * head0 = tail0->producer(0);
		size_t nalternatives = node->ninputs-1;
		for (size_t v = node->noutputs; v > 0; --v) {
			jive::output * value = tail0->inputs[v]->origin();
			if (value->node() != head0)
				continue;

			size_t n;
			value = head0->inputs[v-1]->origin();
			for (n = 1; n < nalternatives; n++) {
				jive_node * tail = node->producer(n);
				jive_node * head = tail->producer(0);
				if (tail->producer(v) != head || value != head->inputs[v-1]->origin())
					break;
			}
			if (n == nalternatives) {
				results.push_back(head0->inputs[v]->origin());

				/* FIXME: ugh, this should be done by DNE */
				delete node->outputs[v-1];
				for (size_t n = 0; n < nalternatives; n++) {
					jive_node * tail = node->producer(n);
					jive_node * head = tail->producer(0);
					delete tail->inputs[v];
					delete head->outputs[v];
					delete head->inputs[v-1];
				}
			} else
				results.push_back(node->outputs[v-1]);
		}
		std::reverse(results.begin(), results.end());
	}

	return results;
}
