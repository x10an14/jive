#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/arch/stackframe.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testarch.h"

static void
proc_frame(jive_context * context, jive_graph ** graph, jive_node ** enter, jive_node ** leave)
{
	*graph = jive_graph_create(context);
	*enter = jive_instruction_node_create(
		(*graph)->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	*leave = jive_instruction_node_create(
		(*graph)->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_gate * stackptr_var = jive_register_class_create_gate(&jive_testarch_regcls[cls_r0], *graph, "stackptr");
	stackptr_var->may_spill = false;
	jive_output * stackptr = jive_node_gate_output(*enter, stackptr_var);
	jive_node_gate_input(*leave, stackptr_var, stackptr);
}

static jive_graph *
create_testgraph_split(jive_context * context)
{
	/* register usage counts are satisfied locally, but there
	is a "critical value" where no global assignment of one
	register is possible */
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_gate * arg1_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "arg1");
	jive_gate * arg2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "arg2");
	
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "retval");
	
	jive_output * arg1 = jive_node_gate_output(enter, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(enter, arg2_gate);
	
	jive_output * critical_value = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[move_gpr_index],
		(jive_output *[]) {arg1}, NULL)->outputs[0];
	
	jive_output * tmp = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[setr1_index],
		(jive_output *[]) {critical_value}, NULL)->outputs[0];
	
	tmp = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[setr2_index],
		(jive_output *[]) {tmp}, NULL)->outputs[0];
	tmp = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[add_index],
		(jive_output *[]) {critical_value, tmp}, NULL)->outputs[0];
	
	jive_node_gate_input(leave, retval_gate, tmp);
	jive_node_gate_input(leave, arg2_gate, arg2);
	
	return graph;
}

static jive_graph *
create_testgraph_emerg_split(jive_context * context)
{
	/* creates variables with the following lifetimes:
	 a   c
	 |   |
	 |     d
	 |     |
	 |       e
	 |       |
	   b     |
	   |     |
	   |       f
	   |       |
	
	with the following register constraints:
	a - r1
	b - r2
	c,d,e,f - each either r1 or r2
	
	Due to number of intefering values, a will be assigned
	r1 first (denying r1 to e), and subsequently b will
	be considered. Even after life range split, assigning
	the only possible choice r2 to b, it leaves no register
	for e over its lifetime.
	
	The register allocator must allow r2 for b after splitting
	the lifetime such that no use of e falls within its
	lifetime. */
	
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_gate * arg3_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "arg2");
	jive_output * arg3 = jive_node_gate_output(enter, arg3_gate);
	jive_node_gate_input(leave, arg3_gate, arg3);
	
	jive_gate * cls2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "cls2");
	jive_output * arg1 = jive_node_gate_output(enter, cls2_gate);
	jive_node * op3 = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[setr1_index],
		(jive_output *[]) {arg1}, NULL);
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"), op3->outputs[0]);
	
	jive_gate * arg2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "1or2");
	jive_output * arg2 = jive_node_gate_output(enter, arg2_gate);
	jive_node * op1 = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[move_gpr_index],
		(jive_output *[]) {arg2}, NULL);
	jive_node * op2 = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[move_gpr_index],
		(jive_output *[]) {op1->outputs[0]}, NULL);
	jive_node * op4 = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[move_gpr_index],
		(jive_output *[]) {op2->outputs[0]}, NULL);
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "cls1"), op4->outputs[0]);
	
	JIVE_DECLARE_STATE_TYPE(state_type);
	
	jive_node_add_input(op3, state_type, jive_node_add_output(op2, state_type));
	jive_node_add_input(op4, state_type, jive_node_add_output(op3, state_type));
	
	return graph;
}

typedef jive_graph * (*creator_function_t)(jive_context *);

static const creator_function_t tests[] = {
	create_testgraph_split,
	create_testgraph_emerg_split
};

int main()
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	size_t n;
	for (n = 0; n < sizeof(tests)/sizeof(tests[0]); n++) {
		fprintf(stderr, "%zd\n", n);
		jive_graph * graph = tests[n](context);
		jive_view(graph, stdout);
		jive_shaped_graph * shaped_graph = jive_regalloc(graph, &jive_testarch_xfer_factory);
		jive_view(graph, stdout);
		jive_shaped_graph_destroy(shaped_graph);
		jive_graph_destroy(graph);
	}
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}