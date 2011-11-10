#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/basetype-private.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

#include <jive/util/list.h>

static const jive_value_type jive_value_type_singleton = {
	.base = {.class_ = &JIVE_VALUE_TYPE}
};

const jive_type_class JIVE_VALUE_TYPE = {
	.parent = &JIVE_TYPE,
	.fini = _jive_value_type_fini, /* override */
	.copy = _jive_value_type_copy, /* override */
	.get_label = jive_type_get_label_, /* inherit */
	.create_input = _jive_value_type_create_input, /* override */
	.create_output = _jive_value_type_create_output, /* override */
	.create_gate = _jive_value_type_create_gate, /* override */
	.equals = jive_type_equals_, /* inherit */
};

const jive_input_class JIVE_VALUE_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = jive_input_fini_, /* inherit */
	.get_label = jive_input_get_label_, /* inherit */
	.get_type = _jive_value_input_get_type, /* override */
};

const jive_output_class JIVE_VALUE_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = jive_output_fini_, /* inherit */
	.get_label = jive_output_get_label_, /* inherit */
	.get_type = _jive_value_output_get_type, /* override */
};

const jive_gate_class JIVE_VALUE_GATE = {
	.parent = &JIVE_GATE,
	.fini = jive_gate_fini_, /* inherit */
	.get_label = jive_gate_get_label_, /* inherit */
	.get_type = _jive_value_gate_get_type, /* override */
};

void
_jive_value_type_fini(jive_type * self_)
{
	jive_value_type * self = (jive_value_type *) self_;
	jive_type_fini_( &self->base ) ;
}

jive_type *
_jive_value_type_copy(const jive_type * self_, jive_context * context)
{
	const jive_value_type * self = (const jive_value_type *) self_;
	
	jive_value_type * type = jive_context_malloc(context, sizeof(*type));
	
	type->base = self->base;
	
	return &type->base;
} 

jive_input *
_jive_value_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_value_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.class_ = &JIVE_VALUE_INPUT;
	_jive_value_input_init(input, node, index, initial_operand);
	return &input->base; 
}

jive_output *
_jive_value_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_value_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.class_ = &JIVE_VALUE_OUTPUT;
	_jive_value_output_init(output, node, index);
	return &output->base; 
}

jive_gate *
_jive_value_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_value_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.class_ = &JIVE_VALUE_GATE;
	_jive_value_gate_init(gate, graph, name);
	return &gate->base; 
}

/* value inputs */

void
_jive_value_input_init(jive_value_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	jive_input_init_(&self->base, node, index, origin);
}

const jive_type *
_jive_value_input_get_type(const jive_input * self)
{
	return &jive_value_type_singleton.base;
}

/* value outputs */

void
_jive_value_output_init(jive_value_output * self, struct jive_node * node, size_t index)
{
	jive_output_init_(&self->base, node, index);
}

const jive_type *
_jive_value_output_get_type(const jive_output * self)
{
	return &jive_value_type_singleton.base;
}

/* value gates */

void
_jive_value_gate_init(jive_value_gate * self, struct jive_graph * graph, const char * name)
{
	jive_gate_init_(&self->base, graph, name);
}

const jive_type *
_jive_value_gate_get_type(const jive_gate * self)
{
	return &jive_value_type_singleton.base;
}
