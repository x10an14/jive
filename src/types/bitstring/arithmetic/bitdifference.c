/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitdifference.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

sub_op::~sub_op() noexcept {}

bool
sub_op::operator==(const operation & other) const noexcept
{
	const sub_op * o = dynamic_cast<const sub_op *>(&other);
	return o && o->type() == type();
}
value_repr
sub_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	jive_bitstring_difference(&result[0], &arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
sub_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sub_op::debug_string() const
{
	return "BITDIFFERENCE";
}

std::unique_ptr<jive::operation>
sub_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sub_op(*this));
}

}
}

jive::output *
jive_bitdifference(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::sub_op(type),
		{op1, op2})[0];
}
