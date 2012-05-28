#ifndef JIVE_TYPES_BITSTRING_COMPARISON_BITUGREATER_H
#define JIVE_TYPES_BITSTRING_COMPARISON_BITUGREATER_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitcomparison_operation_class JIVE_BITUGREATER_NODE_;
#define JIVE_BITUGREATER_NODE (JIVE_BITUGREATER_NODE_.base.base)

jive_node *
jive_bitugreater_create(struct jive_region * region,
	struct jive_output * operand1, struct jive_output * operand2);

jive_output *
jive_bitugreater(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_bitugreater_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITUGREATER_NODE))
		return node;
	else
		return NULL;
}

#endif
