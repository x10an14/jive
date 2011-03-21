#ifndef JIVE_BITSTRING_MULTIOP_H
#define JIVE_BITSTRING_MULTIOP_H

#include <jive/vsdg/node.h>
#include <jive/bitstring/type.h>

extern const jive_node_class JIVE_BITSTRING_MULTIOP_NODE;
extern const jive_node_class JIVE_BITSTRING_KEEPWIDTH_MULTIOP_NODE;
extern const jive_node_class JIVE_BITSTRING_EXPANDWIDTH_MULTIOP_NODE;

typedef struct jive_node jive_bitand_node;
extern const jive_node_class JIVE_BITAND_NODE;
jive_bitand_node *
jive_bitand_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);
jive_bitstring *
jive_bitand(size_t noperands, jive_bitstring * operands[const]);
static inline jive_bitand_node *
jive_bitand_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITAND_NODE) return (jive_bitand_node *) node;
	else return 0;
}

typedef struct jive_node jive_bitor_node;
extern const jive_node_class JIVE_BITOR_NODE;
jive_bitor_node *
jive_bitor_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);
jive_bitstring *
jive_bitor(size_t noperands, jive_bitstring * operands[const]);
static inline jive_bitor_node *
jive_bitor_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITOR_NODE) return (jive_bitor_node *) node;
	else return 0;
}

typedef struct jive_node jive_bitxor_node;
extern const jive_node_class JIVE_BITXOR_NODE;
jive_bitxor_node *
jive_bitxor_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);
jive_bitstring *
jive_bitxor(size_t noperands, jive_bitstring * operands[const]);
static inline jive_bitxor_node *
jive_bitxor_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITXOR_NODE) return (jive_bitxor_node *) node;
	else return 0;
}

typedef struct jive_node jive_bitsum_node;
extern const jive_node_class JIVE_BITSUM_NODE;
jive_bitsum_node *
jive_bitsum_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);
jive_bitstring *
jive_bitadd(size_t noperands, jive_bitstring * operands[const]);
static inline jive_bitsum_node *
jive_bitsum_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSUM_NODE) return (jive_bitsum_node *) node;
	else return 0;
}

typedef struct jive_node jive_bitproduct_node;
extern const jive_node_class JIVE_BITPRODUCT_NODE;
jive_bitproduct_node *
jive_bitproduct_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);
jive_bitstring *
jive_bitmultiply(size_t noperands, jive_bitstring * operands[const]);
static inline jive_bitproduct_node *
jive_bitproduct_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITPRODUCT_NODE) return (jive_bitproduct_node *) node;
	else return 0;
}

typedef struct jive_node jive_bitconcat_node;
extern const jive_node_class JIVE_BITCONCAT_NODE;
jive_bitconcat_node *
jive_bitconcat_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);
jive_bitstring *
jive_bitconcat(size_t noperands, jive_bitstring * operands[const]);
static inline jive_bitconcat_node *
jive_bitconcat_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITCONCAT_NODE) return (jive_bitconcat_node *) node;
	else return 0;
}

#endif