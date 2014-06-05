/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_PRIVATE_H
#define JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_PRIVATE_H

#include <jive/vsdg/node-private.h>

namespace jive {
namespace bitstring {
namespace detail {

template<typename Op>
static inline jive_output *
unop_normalized_create(const jive_unary_operation_class * cls, jive_output * argument)
{
	const jive::bits::type * type = dynamic_cast<const jive::bits::type*>(&argument->type());

	Op op(*type);
	return jive_unary_operation_create_normalized(cls, argument->node()->graph, &op, argument);
}

template<typename Op>
static inline jive_output *
binop_normalized_create(const jive_binary_operation_class * cls, size_t narguments,
	jive_output * const * arguments)
{
	JIVE_DEBUG_ASSERT(narguments != 0);
	const jive::bits::type * type = dynamic_cast<const jive::bits::type*>(&arguments[0]->type());

	jive_graph * graph = arguments[0]->node()->graph;
	Op op(*type, narguments);
	return jive_binary_operation_create_normalized(cls, graph, &op,
		narguments, arguments);
}

template<typename Op>
static inline jive_output *
binop_normalized_create(
	const jive_binary_operation_class * cls,
	jive_output * arg1,
	jive_output * arg2)
{
	const jive::bits::type * type = dynamic_cast<const jive::bits::type*>(&arg1->type());

	jive_graph * graph = arg1->node()->graph;
	Op op(*type);
	jive_output * arguments[] = {arg1, arg2};
	return jive_binary_operation_create_normalized(cls, graph, &op,
		2, arguments);
}

template<typename Op>
static inline jive_node *
unop_create(
	const Op & operation,
	const jive_node_class * cls,
	jive_region * region,
	jive_output * argument)
{
	jive_node * node = jive::create_operation_node(operation);
	node->class_ = cls;

	const jive_type * types[1] = {&operation.type()};

	jive_node_init_(
		node, region,
		1, types, &argument,
		1, types);

	return node;
}

template<typename Op>
static inline jive_node *
binop_create(
	const Op & operation,
	const jive_node_class * cls,
	jive_region * region, 
	size_t narguments,
	jive_output * const * arguments)
{
	JIVE_DEBUG_ASSERT(narguments == operation.narguments());

	jive_node * node = jive::create_operation_node(operation);
	node->class_ = cls;

	const jive_type * argument_types[narguments];
	for(size_t n = 0; n < narguments; n++)
		argument_types[n] = &operation.argument_type(n);

	const jive_type * result_types[1] = {&operation.result_type(0)};
	jive_node_init_(
		node, region,
		narguments, argument_types, arguments,
		1, result_types);

	return node;
}

template<typename Op>
static inline jive_node *
binop_create(
	const Op & operation,
	const jive_node_class * cls,
	jive_region * region, 
	jive_output * arg1,
	jive_output * arg2)
{
	JIVE_DEBUG_ASSERT(2 == operation.narguments());

	jive_node * node = jive::create_operation_node(operation);
	node->class_ = cls;

	jive_output * arguments[2] = {arg1, arg2};
	const jive_type * argument_types[2];
	for(size_t n = 0; n < 2; n++)
		argument_types[n] = &operation.argument_type(n);

	const jive_type * result_types[1] = {&operation.result_type(0)};
	jive_node_init_(
		node, region,
		2, argument_types, arguments,
		1, result_types);

	return node;
}

}
}
}
/* bitbinary operation class inhertiable members */

void
jive_bitbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* bitunary operation class inheritable members */

void
jive_bitunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* bitcomparison operation class inhertiable members */

void
jive_bitcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context);

#endif
