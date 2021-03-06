/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/load.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/load-normal-form.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/simple_node.h>

static jive::output *
jive_load_node_normalized_create(
	const jive::node_normal_form * nf,
	jive::graph * graph,
	const jive::simple_op & op,
	jive::output * address,
	size_t nstates, jive::output * const states[])
{
	std::vector<jive::output*> args = {address};
	for (size_t n = 0; n < nstates; ++n) {
		args.push_back(states[n]);
	}

	return jive::create_normalized(address->region(), op, args)[0];
}

namespace jive {

load_op::~load_op() noexcept
{
}

bool
load_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const load_op *>(&other);
	return op
	    && op->value_ == value_
	    && op->address_ == address_
			&& op->states_ == states_;
}

size_t
load_op::narguments() const noexcept
{
	return 1 + states_.size();
}

const jive::port &
load_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());

	if (index == 0)
		return address_;

	return states_[index-1];
}

size_t
load_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
load_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return value_;
}

std::string
load_op::debug_string() const
{
	return "LOAD";
}

std::unique_ptr<jive::operation>
load_op::copy() const
{
	return std::unique_ptr<jive::operation>(new load_op(*this));
}

}

jive::node_normal_form *
jive_load_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	jive::node_normal_form * nf = new jive::load_normal_form(
		operator_class, parent, graph);

	return nf;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::load_op), jive_load_get_default_normal_form_);
}

jive::output *
jive_load_by_address_create(jive::output * address,
	const jive::value::type * datatype,
	size_t nstates, jive::output * const states[])
{
	auto graph = address->region()->graph();
	const auto nf = graph->node_normal_form(typeid(jive::load_op));
	
	std::vector<std::unique_ptr<jive::base::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(states[n]->type()).copy());
	}

	jive::load_op op(jive::addr::type(), state_types, *datatype);

	return jive_load_node_normalized_create(nf, graph, op, address, nstates, states);
}

jive::output *
jive_load_by_bitstring_create(jive::output * address, size_t nbits,
	const jive::value::type * datatype,
	size_t nstates, jive::output * const states[])
{
	auto graph = address->region()->graph();
	const auto nf = graph->node_normal_form(typeid(jive::load_op));
	
	std::vector<std::unique_ptr<jive::base::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(states[n]->type()).copy());
	}

	jive::load_op op(jive::bits::type(nbits), state_types, *datatype);

	return jive_load_node_normalized_create(nf, graph, op, address, nstates, states);
}
