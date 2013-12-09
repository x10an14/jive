/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/real/arithmetic/rldifference.h>
#include <jive/types/real/rloperation-classes-private.h>
#include <jive/types/real/rltype.h>
#include <jive/vsdg/node-private.h>

static jive_node *
jive_rldifference_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_rlbinary_operation_class JIVE_RLDIFFERENCE_NODE_ = {
	.base = {	/* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_RLBINARY_NODE,
			.name = "RLDIFFERENCE",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.check_operands = jive_rlbinary_operation_check_operands_, /* override */
			.create = jive_rldifference_node_create_, /* overrride */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},

	.flags = jive_binary_operation_none,
	.single_apply_under = NULL,
	.multi_apply_under = NULL,
	.distributive_over = NULL,
	.distributive_under = NULL,

	.can_reduce_operand_pair = jive_binary_operation_can_reduce_operand_pair_, /* inherit */
	.reduce_operand_pair = jive_binary_operation_reduce_operand_pair_ /* inherit */
	},

	.type = jive_rlop_code_difference
};

static void
jive_rldifference_node_init_(jive_rldifference_node * self, jive_region * region,
	struct jive_output * op1, struct jive_output * op2)
{
	JIVE_DECLARE_REAL_TYPE(rltype);
	jive_node_init_(&self->base, region,
		2, (const jive_type *[]){rltype, rltype}, (jive_output *[]){op1, op2},
		1, &rltype);
}

static jive_node *
jive_rldifference_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_rldifference_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_RLDIFFERENCE_NODE;
	jive_rldifference_node_init_(node, region, operands[0], operands[1]);

	return &node->base;
}

struct jive_output *
jive_rldifference(struct jive_output * op1, struct jive_output * op2)
{
	jive_graph * graph = op1->node->graph;
	return jive_binary_operation_create_normalized(&JIVE_RLDIFFERENCE_NODE_.base, graph, NULL,
		2, (jive_output *[]){op1, op2});
}