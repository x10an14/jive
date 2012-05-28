#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITUMOD_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITUMOD_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITUMOD_NODE_;
#define JIVE_BITUMOD_NODE (JIVE_BITUMOD_NODE_.base.base)

jive_node *
jive_bitumod_create(struct jive_region * region,
	jive_output * operand1, jive_output * operand2);

jive_output *
jive_bitumod(jive_output * operand1, jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_bitumod_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITUMOD_NODE))
		return node;
	else
		return 0;
}

#endif