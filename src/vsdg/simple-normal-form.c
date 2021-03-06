/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/simple-normal-form.h>

static jive::node *
node_cse(
	jive::region * region,
	const jive::operation & op,
	const std::vector<jive::output*> & arguments)
{
	auto cse_test = [&](const jive::node * node)
	{
		return node->operation() == op && arguments == jive::operands(node);
	};

	if (!arguments.empty()) {
		for (const auto & user : *arguments[0]) {
			auto node = user->node();
			if (node && cse_test(node))
				return node;
		}
	} else {
		jive::node * node;
		JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
		if (cse_test(node))
			return node;
	}

	return nullptr;
}

namespace jive {

simple_normal_form::~simple_normal_form() noexcept
{}

simple_normal_form::simple_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph) noexcept
	: node_normal_form(operator_class, parent, graph)
	, enable_cse_(true)
{
	if (auto p = dynamic_cast<simple_normal_form*>(parent))
		enable_cse_ = p->get_cse();
}

bool
simple_normal_form::normalize_node(jive::node * node) const
{
	if (!get_mutable())
		return true;

	if (get_cse()) {
		auto new_node = node_cse(node->region(), node->operation(), operands(node));
		if (new_node && new_node != node) {
			JIVE_DEBUG_ASSERT(new_node->noutputs() == node->noutputs());
			for (size_t n = 0; n < node->noutputs(); n++)
				node->output(n)->replace(new_node->output(n));
			node->region()->remove_node(node);
			return false;
		}
	}

	return true;
}

std::vector<jive::output*>
simple_normal_form::normalized_create(
	jive::region * region,
	const jive::simple_op & op,
	const std::vector<jive::output*> & arguments) const
{
	jive::node * node = nullptr;
	if (get_mutable() && get_cse())
		node = node_cse(region, op, arguments);
	if (!node)
		node = region->add_simple_node(op, arguments);

	std::vector<jive::output*> outputs;
	for (size_t n = 0; n < node->noutputs(); n++)
		outputs.push_back(node->output(n));

	return outputs;
}

void
simple_normal_form::set_cse(bool enable)
{
	if (enable == enable_cse_)
		return;

	enable_cse_ = enable;
	children_set<simple_normal_form, &simple_normal_form::set_cse>(enable);

	if (get_mutable() && enable)
		graph()->mark_denormalized();
}

}

static jive::node_normal_form *
get_default_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	return new jive::simple_normal_form(operator_class, parent, graph);
}

static void __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(typeid(jive::simple_op), get_default_normal_form);
}
