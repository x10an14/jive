/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/regselector.h>

#include <stdint.h>

#include <jive/arch/load.h>
#include <jive/arch/regvalue.h>
#include <jive/arch/store.h>
#include <jive/common.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/nullary.h>
#include <jive/vsdg/splitnode.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/type.h>

jive_reg_classifier::~jive_reg_classifier() noexcept
{
}

class jive_regselector_option final : public jive_negotiator_option {
public:
	virtual
	~jive_regselector_option() noexcept {}

	inline constexpr
	jive_regselector_option() noexcept : mask(0) {}

	inline constexpr
	jive_regselector_option(uint32_t init_mask) noexcept : mask(init_mask) {}

	/* test two options for equality */
	virtual bool
	operator==(const jive_negotiator_option & generic_other) const noexcept override
	{
		const jive_regselector_option & other =
			static_cast<const jive_regselector_option &>(generic_other);
		return mask == other.mask;
	}

	/* specialize option, return true if changed */
	virtual bool
	specialize() noexcept override
	{
		/* if this is a power of two already, nothing to do */
		if ( (mask & (mask - 1)) == 0 )
			return false;

		uint32_t x = 0x80000000;
		while ( !(x & mask) )
			x >>= 1;
		mask = x;
		return true;
	}

	virtual bool
	intersect(const jive_negotiator_option & generic_other) noexcept override
	{
		const jive_regselector_option & other =
			static_cast<const jive_regselector_option &>(generic_other);

		uint32_t tmp = other.mask & mask;
		if (tmp == 0) {
			return false;
		} else {
			mask = tmp;
			return true;
		}
	}

	virtual bool
	assign(const jive_negotiator_option & generic_other) noexcept override
	{
		const jive_regselector_option & other =
			static_cast<const jive_regselector_option &>(generic_other);
		if (mask == other.mask) {
			return false;
		} else {
			mask = other.mask;
			return true;
		}
	}

	virtual jive_regselector_option *
	copy() const
	{
		return new jive_regselector_option(mask);
	}

	uint32_t mask;
};

static jive_negotiator_option *
jive_regselector_option_create_(const jive_negotiator * self)
{
	jive_regselector_option * option = new jive_regselector_option(0);
	option->mask = 0;
	return option;
}

static uint32_t
jive_regselector_classify_regcls(
	const jive_regselector * self,
	const jive::register_class * regcls)
{
	uint32_t option_mask = 0;
	size_t n = 0;
	for (n = 0; n < self->classifier->nclasses(); ++n) {
		if (regcls == self->classifier->classes()[n])
			option_mask |= (1 << n);
	}
	return option_mask;
}

static void
jive_regselector_annotate_node_proper_(jive_negotiator * self_, jive::node * node)
{
	jive_regselector * self = (jive_regselector *) self_;
	const jive::operation * gen_op = &node->operation();
	
	jive_regselector_option option;
	if (auto op = dynamic_cast<const jive::regvalue_op *>(gen_op)) {
		auto regcls = op->regcls();
		option.mask = jive_regselector_classify_regcls(self, regcls);
		jive_negotiator_annotate_simple_output(&self->base,
			dynamic_cast<jive::simple_output*>(node->output(0)), &option);
	} else if (auto op = dynamic_cast<const jive::flt::unary_op *>(gen_op)) {
		option.mask = self->classifier->classify_float_unary(*op);
		jive_negotiator_annotate_identity_node(&self->base, node, &option);
	} else if (auto op = dynamic_cast<const jive::flt::binary_op *>(gen_op)) {
		option.mask = self->classifier->classify_float_binary(*op);
		jive_negotiator_annotate_identity_node(&self->base, node, &option);
	} else if (auto op = dynamic_cast<const jive::flt::compare_op *>(gen_op)) {
		option.mask = self->classifier->classify_float_compare(*op);
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		std::vector<jive::simple_output*> outputs;
		for (size_t n = 0; n < node->noutputs(); n++)
			outputs.push_back(dynamic_cast<jive::simple_output*>(node->output(n)));
		jive_negotiator_annotate_identity(&self->base, 2, &input, 0, &outputs[0], &option);
	} else if (auto op = dynamic_cast<const jive::bits::unary_op *>(gen_op)) {
		option.mask = self->classifier->classify_fixed_unary(*op);
		jive_negotiator_annotate_identity_node(&self->base, node, &option);
	} else if (auto op = dynamic_cast<const jive::bits::binary_op *>(gen_op)) {
		option.mask = self->classifier->classify_fixed_binary(*op);
		jive_negotiator_annotate_identity_node(&self->base, node, &option);
	} else if (auto op = dynamic_cast<const jive::bits::compare_op *>(gen_op)) {
		option.mask = self->classifier->classify_fixed_compare(*op);
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		std::vector<jive::simple_output*> outputs;
		for (size_t n = 0; n < node->noutputs(); n++)
			outputs.push_back(dynamic_cast<jive::simple_output*>(node->output(n)));
		jive_negotiator_annotate_identity(&self->base, 2, &input, 0, &outputs[0], &option);
	} else if (dynamic_cast<const jive::load_op *>(gen_op)) {
		option.mask = self->classifier->classify_address();
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		jive_negotiator_annotate_identity(&self->base, 1, &input, 0, NULL, &option);
		
		const jive::base::type * type = &node->output(0)->type();
		auto rescls = node->output(0)->port().rescls();
		option.mask = self->classifier->classify_type(type, rescls);
		std::vector<jive::simple_output*> outputs;
		for (size_t n = 0; n < node->noutputs(); n++)
			outputs.push_back(dynamic_cast<jive::simple_output*>(node->output(n)));
		jive_negotiator_annotate_identity(&self->base, 0, NULL, 1, &outputs[0], &option);
	} else if (dynamic_cast<const jive::store_op *>(gen_op)) {
		option.mask = self->classifier->classify_address();
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		jive_negotiator_annotate_identity(&self->base, 1, &input, 0, NULL, &option);
		
		const jive::base::type * type = &node->input(1)->type();
		auto rescls = node->input(1)->port().rescls();
		option.mask = self->classifier->classify_type(type, rescls);
		input = dynamic_cast<jive::simple_input*>(node->input(1));
		jive_negotiator_annotate_identity(&self->base, 1, &input, 0, NULL, &option);
	} else if (dynamic_cast<const jive::split_operation *>(gen_op)) {
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		auto output = dynamic_cast<jive::simple_output*>(node->output(0));
		
		if (dynamic_cast<const jive::value::type*>(&input->type())) {
			jive_regselector_option option;
			option.mask = self->classifier->classify_type(&input->type(), input->port().rescls());
			if (option.mask) {
				auto input = dynamic_cast<jive::simple_input*>(node->input(0));
				jive_negotiator_annotate_identity(&self->base,1, &input, 0, NULL, &option);
			}
		}
		
		if (dynamic_cast<const jive::value::type*>(&output->type())) {
			jive_regselector_option option;
			option.mask = self->classifier->classify_type(&output->type(), output->port().rescls());
			if (option.mask) {
				std::vector<jive::simple_output*> outputs;
				for (size_t n = 0; n < node->noutputs(); n++)
					outputs.push_back(dynamic_cast<jive::simple_output*>(node->output(n)));
				jive_negotiator_annotate_identity(&self->base,0, NULL, 1, &outputs[0], &option);
			}
		}
	}
}

static bool
jive_regselector_option_gate_default_(const jive_negotiator * self_, jive_negotiator_option * dst,
	const jive::gate * gate)
{
	if (!dynamic_cast<const jive::value::type*>(&gate->type()))
		return false;
	jive_regselector * self = (jive_regselector *) self_;
	jive_regselector_option * option = (jive_regselector_option *) dst;
	option->mask = self->classifier->classify_type(&gate->type(), gate->rescls());
	return !!option->mask;
}

static const jive_negotiator_class JIVE_REGSELECTOR_CLASS = {
	option_create : jive_regselector_option_create_,
	option_gate_default : jive_regselector_option_gate_default_,
	annotate_node_proper : jive_regselector_annotate_node_proper_,
	annotate_node : jive_negotiator_annotate_node_,
	process_region : jive_negotiator_process_region_
};

static const jive::register_class *
jive_regselector_map_port(const jive_regselector * self, const jive_negotiator_port * port)
{
	if (!port) return NULL;
	const jive_regselector_option * option;
	option = (const jive_regselector_option *) port->option;
	if (!option->mask) return NULL;
	
	jive_regselect_index index = 0;
	while (! (option->mask & (1 << index) ) )
		index ++;
	return self->classifier->classes()[index];
}

static void
jive_regselector_pull_node(jive_regselector * self, jive::node * node)
{
	jive::region * root_region = self->base.graph->root();
	
	if (node->region() == root_region)
		return;
	
	/* FIXME: this is broken */
	return;

	/* determine function region */
	jive::region * region = node->region();
	//while (region->parent() != root_region)
	//	region = region->parent();

	if (auto op = dynamic_cast<const jive::regvalue_op *>(&node->operation())) {
		auto regcls = op->regcls();
		auto origin = node->input(1)->origin()->node();

		if (dynamic_cast<const jive::binary_op*>(&origin->operation())) {
			jive::output * operands[origin->ninputs()];
			for (size_t n = 0; n < origin->ninputs(); n++) {
				auto operand = origin->input(n)->origin();
				auto regvalue = dynamic_cast<jive::simple_output*>(jive_regvalue(operand, regcls));
				jive_negotiator_port * reg_port = jive_negotiator_map_output(&self->base, regvalue);
				if (!reg_port)
					self->base.class_->annotate_node(&self->base, regvalue->node());
				operands[n] = regvalue;
			}
			
			auto subst = dynamic_cast<jive::simple_output*>(jive::create_normalized(
				region,
				static_cast<const jive::simple_op&>(origin->operation()),
				std::vector<jive::output*>(operands, operands + origin->ninputs()))[0]);
			
			jive_negotiator_port_split(jive_negotiator_map_output(&self->base,
				dynamic_cast<jive::simple_output*>(node->output(0))));
			node->output(0)->replace(subst);
			
			if (!jive_negotiator_map_output(&self->base, subst))
				self->base.class_->annotate_node(&self->base, subst->node());
			jive_negotiator_fully_specialize(&self->base);
		} else if (dynamic_cast<const jive::unary_op*>(&origin->operation())) {
			jive::simple_output * operands[origin->ninputs()];
			for (size_t n = 0; n < origin->ninputs(); n++) {
				auto operand = origin->input(n)->origin();
				auto regvalue = dynamic_cast<jive::simple_output*>(jive_regvalue(operand, regcls));
				jive_negotiator_port * reg_port = jive_negotiator_map_output(&self->base, regvalue);
				if (!reg_port)
					self->base.class_->annotate_node(&self->base, regvalue->node());
				operands[n] = regvalue;
			}
			auto subst = dynamic_cast<jive::simple_output*>(jive::create_normalized(
				region, static_cast<const jive::simple_op&>(origin->operation()), {operands[0]})[0]);
			
			jive_negotiator_port_split(jive_negotiator_map_output(&self->base,
				dynamic_cast<jive::simple_output*>(node->output(0))));
			node->output(0)->replace(subst);
			
			if (!jive_negotiator_map_output(&self->base, subst))
				self->base.class_->annotate_node(&self->base, subst->node());
			jive_negotiator_fully_specialize(&self->base);
		}
		return;
	}

	size_t n;
	for (n = 0; n < node->ninputs(); n++) {
		auto input = dynamic_cast<jive::simple_input*>(node->input(n));
		if (input->origin()->region() != root_region
		&& !dynamic_cast<const jive::base::nullary_op*>(
			&input->origin()->node()->operation()))
			continue;
		jive_negotiator_port * port = jive_negotiator_map_input(&self->base, input);
		if (!port)
			continue;
		auto regcls = jive_regselector_map_port(self, port);
		if (!regcls)
			continue;
		
		auto regvalue = dynamic_cast<jive::simple_output*>(jive_regvalue(input->origin(), regcls));
		jive_negotiator_port * reg_port = jive_negotiator_map_output(&self->base, regvalue);
		if (!reg_port) {
			self->base.class_->annotate_node(&self->base, regvalue->node());
			reg_port = jive_negotiator_map_output(&self->base, regvalue);
		}
		
		jive_negotiator_port_divert(port, reg_port->connection);
		input->divert_origin(regvalue);
		jive_negotiator_fully_specialize(&self->base);
	}
}

void
jive_regselector_pull(jive_regselector * self)
{
	for (jive::node * node : jive::bottomup_traverser(self->base.graph->root(), true)) {
		jive_regselector_pull_node(self, node);
	}
}

void
jive_regselector_init(jive_regselector * self, jive::graph * graph,
	const jive_reg_classifier * classifier)
{
	jive_negotiator_init_(&self->base, &JIVE_REGSELECTOR_CLASS, graph);
	self->classifier = classifier;
}

void
jive_regselector_process(jive_regselector * self)
{
	jive_negotiator_process(&self->base);
	jive_negotiator_insert_split_nodes(&self->base);
	jive_regselector_pull(self);
	jive_negotiator_process(&self->base);
	jive_negotiator_remove_split_nodes(&self->base);
}

const jive::register_class *
jive_regselector_map_output(const jive_regselector * self, jive::simple_output * output)
{
	const jive_negotiator_port * port = jive_negotiator_map_output(&self->base, output);
	return jive_regselector_map_port(self, port);
}

const jive::register_class *
jive_regselector_map_input(const jive_regselector * self, jive::simple_input * input)
{
	const jive_negotiator_port * port = jive_negotiator_map_input(&self->base, input);
	return jive_regselector_map_port(self, port);
}

void
jive_regselector_fini(jive_regselector * self)
{
	jive_negotiator_fini_(&self->base);
}

