/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/immediate-type.h>

#include <string.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/valuetype-private.h>

static void
jive_immediate_type_init_(jive_immediate_type * self);

/* immediate_input */

static void
jive_immediate_input_init_(jive_immediate_input * self, struct jive_node * node, size_t index,
	jive_output * origin)
{
	jive_value_input_init_(&self->base, node, index, origin);
	jive_immediate_type_init_(&self->type);
}

static const jive_type *
jive_immediate_input_get_type_(const jive_input * self_)
{
	const jive_immediate_input * self = (const jive_immediate_input *) self_;
	return &self->type.base.base;
}

const jive_input_class JIVE_IMMEDIATE_INPUT = {
	.parent = &JIVE_VALUE_INPUT,
	.fini = jive_input_fini_, /* inherit */
	.get_label = jive_input_get_label_, /* inherit */
	.get_type = jive_immediate_input_get_type_, /* override */
};

/* immediate_output inheritable members */

static void
jive_immediate_output_init_(jive_immediate_output * self, struct jive_node * node, size_t index)
{
	jive_value_output_init_(&self->base, node, index);
	jive_immediate_type_init_(&self->type);
}

static const jive_type *
jive_immediate_output_get_type_(const jive_output * self_)
{
	const jive_immediate_output * self = (const jive_immediate_output *) self_;
	return &self->type.base.base;
}

const jive_output_class JIVE_IMMEDIATE_OUTPUT = {
	.parent = &JIVE_VALUE_OUTPUT,
	.fini = jive_output_fini_, /* inherit */
	.get_label = jive_output_get_label_, /* inherit */
	.get_type = jive_immediate_output_get_type_, /* override */
};

/* immediate_gate inheritable members */

static void
jive_immediate_gate_init_(jive_immediate_gate * self, struct jive_graph * graph, const char name[])
{
	jive_value_gate_init_(&self->base, graph, name);
	jive_immediate_type_init_(&self->type);
}

static const jive_type *
jive_immediate_gate_get_type_(const jive_gate * self_)
{
	const jive_immediate_gate * self = (const jive_immediate_gate *) self_;
	return &self->type.base.base;
}

const jive_gate_class JIVE_IMMEDIATE_GATE = {
	.parent = &JIVE_VALUE_GATE,
	.fini = jive_gate_fini_, /* inherit */
	.get_label = jive_gate_get_label_, /* inherit */
	.get_type = jive_immediate_gate_get_type_, /* override */
};

/* immediate type */

static void
jive_immediate_type_fini_( jive_type* self_ )
{
	jive_immediate_type* self = (jive_immediate_type*) self_ ;

	jive_value_type_fini_( (jive_type*)&self->base ) ;
}

static char *
jive_immediate_type_get_label_(const jive_type * self_)
{
	return strdup("immediate");
}

static jive_input *
jive_immediate_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_immediate_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_IMMEDIATE_INPUT;
	jive_immediate_input_init_(input, node, index, initial_operand);
	return &input->base.base;
}

static jive_output *
jive_immediate_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	jive_immediate_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_IMMEDIATE_OUTPUT;
	jive_immediate_output_init_(output, node, index);
	return &output->base.base;
}

static jive_gate *
jive_immediate_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	jive_immediate_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_IMMEDIATE_GATE;
	jive_immediate_gate_init_(gate, graph, name);
	return &gate->base.base;
}

static jive_type *
jive_immediate_type_copy_(const jive_type * self_, jive_context * context)
{
	const jive_immediate_type * self = (const jive_immediate_type *) self_;
	
	jive_immediate_type * type = jive_context_malloc(context, sizeof(*type));
	
	*type = *self;
	
	return &type->base.base;
}

static void
jive_immediate_type_init_(jive_immediate_type * self)
{
	self->base.base.class_ = &JIVE_IMMEDIATE_TYPE;
}

const jive_type_class JIVE_IMMEDIATE_TYPE = {
	.parent = &JIVE_VALUE_TYPE,
	.fini = jive_immediate_type_fini_, /* override */
	.get_label = jive_immediate_type_get_label_, /* override */
	.create_input = jive_immediate_type_create_input_, /* override */
	.create_output = jive_immediate_type_create_output_, /* override */
	.create_gate = jive_immediate_type_create_gate_, /* override */
	.equals = jive_type_equals_, /* inherit */
	.copy = jive_immediate_type_copy_, /* override */
};