#include <string.h>

#include <jive/debug-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/variable.h>
#include <jive/util/list.h>

/* static type instance, to be returned by type queries */
const jive_type jive_type_singleton = {
	.class_ = &JIVE_TYPE
};


char *
_jive_type_get_label(const jive_type * self)
{
	return strdup("X");
}

jive_input *
_jive_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	/* FIXME: consider raising a "logic error" instead */
	DEBUG_ASSERT(jive_type_equals(self, jive_output_get_type(initial_operand)));
	jive_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->class_ = &JIVE_INPUT;
	_jive_input_init(input, node, index, initial_operand);
	return input;
}

jive_output *
_jive_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->class_ = &JIVE_OUTPUT;
	_jive_output_init(output, node, index);
	return output;
}

jive_gate *
_jive_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->class_ = &JIVE_GATE;
	_jive_gate_init(gate, graph, name);
	return gate;
}

bool
_jive_type_equals(const jive_type * self, const jive_type * other)
{
	return self->class_ == other->class_;
}

bool
_jive_type_accepts(const jive_type * self, const jive_type * other)
{
	return self->class_ == other->class_;
}

const jive_type_class JIVE_TYPE = {
	.parent = 0,
	.get_label = _jive_type_get_label,
	.create_input = _jive_type_create_input,
	.create_output = _jive_type_create_output,
	.create_gate = _jive_type_create_gate,
	.equals = _jive_type_equals,
};

/* inputs */

const struct jive_input_class JIVE_INPUT = {
	.parent = 0,
	.fini = _jive_input_fini,
	.get_label = _jive_input_get_label,
	.get_type = _jive_input_get_type,
};

static inline void
jive_input_add_as_user(jive_input * self, jive_output * output)
{
	JIVE_LIST_PUSH_BACK(output->users, self, output_users_list);
	jive_node_add_successor(output->node);
}

static inline void
jive_input_remove_as_user(jive_input * self, jive_output * output)
{
	JIVE_LIST_REMOVE(output->users, self, output_users_list);
	jive_node_remove_successor(self->origin->node);
}

void
_jive_input_init(jive_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	self->node = node;
	self->index = index;
	self->origin = origin;
	self->gate = 0;
	self->required_rescls = &jive_root_resource_class;
	self->ssavar = 0;
	
	self->output_users_list.prev = self->output_users_list.next = 0;
	self->gate_inputs_list.prev = self->gate_inputs_list.next = 0;
	self->ssavar_input_list.prev = self->ssavar_input_list.next = 0;
	
	jive_input_add_as_user(self, origin);
}

void
_jive_input_fini(jive_input * self)
{
	if (self->ssavar) jive_input_unassign_ssavar(self);
	
	if (self->gate) {
		jive_gate * gate = self->gate;
		JIVE_LIST_REMOVE(gate->inputs, self, gate_inputs_list);
		
		size_t n;
		for(n=0; n<self->node->ninputs; n++) {
			jive_input * other = self->node->inputs[n];
			if (other == self || !other->gate) continue;
			jive_gate_interference_remove(self->gate, other->gate);
		}
	}
	
	jive_input_remove_as_user(self, self->origin);
	
	size_t n;
	self->node->ninputs --;
	for(n = self->index; n<self->node->ninputs; n++) {
		self->node->inputs[n] = self->node->inputs[n+1];
		self->node->inputs[n]->index = n;
	}
	if (!self->node->ninputs)
		JIVE_LIST_PUSH_BACK(self->node->graph->top, self->node, graph_top_list);
}

char *
_jive_input_get_label(const jive_input * self)
{
	if (self->gate) return jive_gate_get_label(self->gate);
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "#%zd", self->index);
	return strdup(tmp);
}

const jive_type *
_jive_input_get_type(const jive_input * self)
{
	return &jive_type_singleton;
}

jive_variable *
jive_input_get_constraint(const jive_input * self)
{
	jive_variable * variable;
	if (self->gate) {
		variable = self->gate->variable;
		if (!variable) {
			variable = jive_gate_get_constraint(self->gate);
			jive_variable_assign_gate(variable, self->gate);
		}
		return variable;
	}
	variable = jive_variable_create(self->node->graph);
	jive_variable_set_resource_class(variable, self->required_rescls);
	return variable;
}

void
jive_input_unassign_ssavar(jive_input * self)
{
	if (self->ssavar) jive_ssavar_unassign_input(self->ssavar, self);
}

void
jive_input_divert_origin(jive_input * self, jive_output * new_origin)
{
	DEBUG_ASSERT(!self->ssavar);
	jive_input_internal_divert_origin(self, new_origin);
}

void
jive_input_internal_divert_origin(jive_input * self, jive_output * new_origin)
{
	DEBUG_ASSERT(jive_type_equals(jive_input_get_type(self), jive_output_get_type(new_origin)));
	DEBUG_ASSERT(self->node->graph == new_origin->node->graph);
	
	jive_output * old_origin = self->origin;
	
	jive_input_remove_as_user(self, old_origin);
	self->origin = new_origin;
	jive_input_add_as_user(self, new_origin);
	
	jive_node_invalidate_depth_from_root(self->node);
	
	jive_graph_notify_input_change(self->node->graph, self, old_origin, new_origin);
}

void
jive_input_swap(jive_input * self, jive_input * other)
{
	DEBUG_ASSERT(jive_type_equals(jive_input_get_type(self), jive_input_get_type(other)));
	DEBUG_ASSERT(self->node == other->node);
	
	jive_ssavar * v1 = self->ssavar;
	jive_ssavar * v2 = other->ssavar;
	
	if (v1) jive_ssavar_unassign_input(v1, self);
	if (v2) jive_ssavar_unassign_input(v2, other);
	
	jive_output * o1 = self->origin;
	jive_output * o2 = other->origin;
	
	jive_input_remove_as_user(self, o1);
	jive_input_remove_as_user(other, o2);
	
	jive_input_add_as_user(self, o2);
	jive_input_add_as_user(other, o1);
	
	self->origin = o2;
	other->origin = o1;
	
	if (v2) jive_ssavar_unassign_input(v2, self);
	if (v1) jive_ssavar_unassign_input(v1, other);
	
	jive_node_invalidate_depth_from_root(self->node);
	
	jive_graph_notify_input_change(self->node->graph, self, o1, o2);
	jive_graph_notify_input_change(self->node->graph, other, o2, o1);
}

jive_ssavar *
jive_input_auto_assign_variable(jive_input * self)
{
	if (self->ssavar)
		return self->ssavar;
	
	jive_ssavar * ssavar;
	if (self->origin->ssavar) {
		ssavar = self->origin->ssavar;
		jive_variable_merge(ssavar->variable, jive_input_get_constraint(self));
	} else {
		ssavar = jive_ssavar_create(self->origin, jive_input_get_constraint(self));
	}
	
	jive_ssavar_assign_input(ssavar, self);
	return ssavar;
}

jive_ssavar *
jive_input_auto_merge_variable(jive_input * self)
{
	return jive_input_auto_assign_variable(self);
}

void
jive_input_destroy(jive_input * self)
{
	if (self->ssavar) jive_ssavar_unassign_input(self->ssavar, self);
	if (self->node->region) jive_graph_notify_input_destroy(self->node->graph, self);
	
	self->class_->fini(self);
	jive_context_free(self->node->graph->context, self);
}

/* outputs */

const struct jive_output_class JIVE_OUTPUT = {
	.parent = 0,
	.fini = &_jive_output_fini,
	.get_label = &_jive_output_get_label,
	.get_type = &_jive_output_get_type,
};

void _jive_output_init(
	jive_output * self,
	struct jive_node * node,
	size_t index)
{
	self->node = node;
	self->index = index;
	self->users.first = self->users.last = 0;
	self->gate = 0;
	self->required_rescls = &jive_root_resource_class;
	self->ssavar = 0;
	
	self->gate_outputs_list.prev = self->gate_outputs_list.next = 0;
}

void _jive_output_fini(jive_output * self)
{
	DEBUG_ASSERT(self->users.first == 0 && self->users.last == 0);
	
	if (self->ssavar) jive_ssavar_unassign_output(self->ssavar, self);
		
	if (self->gate) {
		jive_gate * gate = self->gate;
		JIVE_LIST_REMOVE(gate->outputs, self, gate_outputs_list);
		
		size_t n;
		for(n=0; n<self->node->noutputs; n++) {
			jive_output * other = self->node->outputs[n];
			if (other == self || !other->gate) continue;
			jive_gate_interference_remove(self->gate, other->gate);
		}
	}
	
	self->node->noutputs --;
	size_t n;
	for(n=self->index; n<self->node->noutputs; n++) {
		self->node->outputs[n] = self->node->outputs[n+1];
		self->node->outputs[n]->index = n;
	}
}

char *
_jive_output_get_label(const jive_output * self)
{
	if (self->gate) return jive_gate_get_label(self->gate);
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "#%zd", self->index);
	return strdup(tmp);
}

const jive_type *
_jive_output_get_type(const jive_output * self)
{
	return &jive_type_singleton;
}
	
jive_variable *
jive_output_get_constraint(const jive_output * self)
{
	jive_variable * variable;
	if (self->gate) {
		variable = self->gate->variable;
		if (!variable) {
			variable = jive_gate_get_constraint(self->gate);
			jive_variable_assign_gate(variable, self->gate);
		}
		return variable;
	}
	variable = jive_variable_create(self->node->graph);
	jive_variable_set_resource_class(variable, self->required_rescls);
	return variable;
}

void
jive_output_unassign_ssavar(jive_output * self)
{
	if (self->ssavar) jive_ssavar_unassign_output(self->ssavar, self);
}

jive_ssavar *
jive_output_auto_assign_variable(jive_output * self)
{
	if (self->ssavar == 0) {
		jive_ssavar * ssavar = 0;
		jive_input * user;
		JIVE_LIST_ITERATE(self->users, user, output_users_list) {
			if (!user->ssavar) continue;
			if (ssavar) {
				jive_variable_merge(ssavar->variable, user->ssavar->variable);
				jive_ssavar_merge(ssavar, user->ssavar);
			} else
				ssavar = user->ssavar;
		}
		
		if (ssavar) {
			jive_variable_merge(ssavar->variable, jive_output_get_constraint(self));
		} else {
			ssavar = jive_ssavar_create(self, jive_output_get_constraint(self));
		}
		
		jive_ssavar_assign_output(ssavar, self);
	}
	
	return self->ssavar;
}

jive_ssavar *
jive_output_auto_merge_variable(jive_output * self)
{
	if (self->ssavar == 0) {
		jive_variable * variable = jive_output_get_constraint(self);
		jive_input * user;
		JIVE_LIST_ITERATE(self->users, user, output_users_list) {
			if (user->ssavar) {
				jive_variable_merge(user->ssavar->variable, variable);
				variable = user->ssavar->variable;
			}
		}
		jive_ssavar * ssavar = jive_ssavar_create(self, variable);
		jive_ssavar_assign_output(ssavar, self);
	}
	
	jive_input * user;
	JIVE_LIST_ITERATE(self->users, user, output_users_list) {
		if (!user->ssavar) {
			jive_variable_merge(self->ssavar->variable, jive_input_get_constraint(user));
			jive_ssavar_assign_input(self->ssavar, user);
		} else {
			/* FIXME: maybe better to merge ssavar? */
			jive_variable_merge(self->ssavar->variable, user->ssavar->variable);
			jive_input_unassign_ssavar(user);
			jive_ssavar_assign_input(self->ssavar, user);
		}
	}
	return self->ssavar;
}

void
jive_output_replace(jive_output * self, jive_output * other)
{
	while(self->users.first) {
		jive_input * input = self->users.first;
		jive_input_divert_origin(input, other);
	}
}

void
jive_output_destroy(jive_output * self)
{
	if (self->node->region) jive_graph_notify_output_destroy(self->node->graph, self);
	
	self->class_->fini(self);
	jive_context_free(self->node->graph->context, self);
}

/* gates */

const jive_gate_class JIVE_GATE = {
	.parent = 0,
	.fini = _jive_gate_fini,
	.get_label = _jive_gate_get_label,
	.get_type = _jive_gate_get_type,
	.create_input = _jive_gate_create_input,
	.create_output = _jive_gate_create_output
};

void
_jive_gate_init(jive_gate * self, struct jive_graph * graph, const char name[])
{
	self->graph = graph;
	self->name = jive_context_strdup(graph->context, name);
	self->inputs.first = self->inputs.last = 0;
	self->outputs.first = self->outputs.last = 0;
	self->may_spill = true;
	self->variable = 0;
	jive_gate_interference_hash_init(&self->interference, graph->context);
	self->variable_gate_list.prev = self->variable_gate_list.next = 0;
	self->graph_gate_list.prev = self->graph_gate_list.next = 0;
	
	JIVE_LIST_PUSH_BACK(graph->gates, self, graph_gate_list);
}

void
_jive_gate_fini(jive_gate * self)
{
	DEBUG_ASSERT(self->inputs.first == 0 && self->inputs.last == 0);
	DEBUG_ASSERT(self->outputs.first == 0 && self->outputs.last == 0);
	
	if (self->variable) jive_variable_unassign_gate(self->variable, self);
	
	jive_gate_interference_hash_fini(&self->interference);
	jive_context_free(self->graph->context, self->name);
	
	JIVE_LIST_REMOVE(self->graph->gates, self, graph_gate_list);
}

char *
_jive_gate_get_label(const jive_gate * self)
{
	return strdup(self->name);
}

const jive_type *
_jive_gate_get_type(const jive_gate * self)
{
	return &jive_type_singleton;
}

jive_variable *
jive_gate_get_constraint(jive_gate * self)
{
	if (self->variable) return self->variable;
	
	jive_variable * variable = jive_variable_create(self->graph);
	jive_variable_set_resource_class(variable, self->required_rescls);
	
	return variable;
}

jive_input *
_jive_gate_create_input(const jive_gate * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	return jive_type_create_input(jive_gate_get_type(self), node, index, initial_operand);
}

jive_output *
_jive_gate_create_output(const jive_gate * self, struct jive_node * node, size_t index)
{
	return jive_type_create_output(jive_gate_get_type(self), node, index);
}

size_t
jive_gate_interferes_with(const jive_gate * self, const jive_gate * other)
{
	jive_gate_interference_part * part;
	part = jive_gate_interference_hash_lookup(&self->interference, other);
	if (part) return part->whole->count;
	else return 0;
}

void
jive_gate_destroy(jive_gate * self)
{
	self->class_->fini(self);
	jive_context_free(self->graph->context, self);
}
