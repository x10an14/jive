/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/float/fltconstant.h>
#include <jive/types/float/fltoperation-classes.h>
#include <jive/types/float/flttype.h>

#include <jive/vsdg/control.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace flt {

static const type type_instance;

unary_op::~unary_op() noexcept
{
}

const jive::base::type &
unary_op::argument_type(size_t index) const noexcept
{
	return type_instance;
}

const jive::base::type &
unary_op::result_type(size_t index) const noexcept
{
	return type_instance;
}

jive_unop_reduction_path_t
unary_op::can_reduce_operand(
	const jive::oport * arg) const noexcept
{
	auto op = dynamic_cast<const jive::output*>(arg);
	if (op && dynamic_cast<const constant_op*>(&op->node()->operation()))
		return jive_unop_reduction_constant;

	return jive_unop_reduction_none;
}

jive::oport *
unary_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::oport * arg) const
{
	if (path == jive_unop_reduction_constant) {
		auto op = static_cast<jive::output*>(arg);
		const constant_op & c = static_cast<const constant_op&>(op->node()->operation());
		value_repr result = reduce_constant(c.value());
		return jive_fltconstant(arg->region(), result);
	}

	return nullptr;
}

binary_op::~binary_op() noexcept
{
}

size_t
binary_op::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
binary_op::argument_type(size_t index) const noexcept
{
	return type_instance;
}

size_t
binary_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
binary_op::result_type(size_t index) const noexcept
{
	return type_instance;
}

/* reduction methods */
jive_binop_reduction_path_t
binary_op::can_reduce_operand_pair(
	const jive::oport * arg1,
	const jive::oport * arg2) const noexcept
{
	auto op1 = dynamic_cast<const jive::output*>(arg1);
	auto op2 = dynamic_cast<const jive::output*>(arg2);
	if (!op1 || !op2)
		return jive_binop_reduction_none;

	bool arg1_is_constant = dynamic_cast<const constant_op *>(&op1->node()->operation());
	bool arg2_is_constant = dynamic_cast<const constant_op *>(&op2->node()->operation());
	if (arg1_is_constant && arg2_is_constant)
		return jive_binop_reduction_constants;

	return jive_binop_reduction_none;
}

jive::oport *
binary_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::oport * arg1,
	jive::oport * arg2) const
{
	if (path == jive_binop_reduction_constants) {
		auto op1 = static_cast<jive::output*>(arg1);
		auto op2 = static_cast<jive::output*>(arg2);
		const constant_op & c1 = static_cast<const constant_op&>(op1->node()->operation());
		const constant_op & c2 = static_cast<const constant_op&>(op2->node()->operation());
		value_repr result = reduce_constants(c1.value(), c2.value());
		return jive_fltconstant(arg1->region(), result);
	}

	return nullptr;
}

compare_op::~compare_op() noexcept
{
}

size_t
compare_op::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
compare_op::argument_type(size_t index) const noexcept
{
	return type_instance;
}

size_t
compare_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
compare_op::result_type(size_t index) const noexcept
{
	static const jive::bits::type bit(1);
	return bit;
}

jive_binop_reduction_path_t
compare_op::can_reduce_operand_pair(
	const jive::oport * arg1,
	const jive::oport * arg2) const noexcept
{
	auto op1 = dynamic_cast<const jive::output*>(arg1);
	auto op2 = dynamic_cast<const jive::output*>(arg2);
	if (!op1 || !op2)
		return jive_binop_reduction_none;

	bool arg1_is_constant = dynamic_cast<const constant_op *>(&op1->node()->operation());
	bool arg2_is_constant = dynamic_cast<const constant_op *>(&op2->node()->operation());
	if (arg1_is_constant && arg2_is_constant)
		return jive_binop_reduction_constants;

	return jive_binop_reduction_none;
}

jive::oport *
compare_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::oport * arg1,
	jive::oport * arg2) const
{
	if (path == jive_binop_reduction_constants) {
		auto op1 = static_cast<jive::output*>(arg1);
		auto op2 = static_cast<jive::output*>(arg2);
		auto c1 = static_cast<const constant_op&>(op1->node()->operation());
		auto c2 = static_cast<const constant_op&>(op2->node()->operation());
		bool result = reduce_constants(c1.value(), c2.value());
		if (result) {
			return jive_bitconstant(arg1->region(), 1, "1");
		} else {
			return jive_bitconstant(arg1->region(), 1, "0");
		}
	}

	return nullptr;
}

}
}
