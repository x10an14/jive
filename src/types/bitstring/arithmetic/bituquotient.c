/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bituquotient.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

udiv_op::~udiv_op() noexcept {}

bool
udiv_op::operator==(const operation & other) const noexcept
{
	const udiv_op * o = dynamic_cast<const udiv_op *>(&other);
	return o && o->type() == type();
}
value_repr
udiv_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.udiv(arg2);
}

jive_binary_operation_flags
udiv_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
udiv_op::debug_string() const
{
	return "BITUQUOTIENT";
}

std::unique_ptr<jive::operation>
udiv_op::copy() const
{
	return std::unique_ptr<jive::operation>(new udiv_op(*this));
}

}
}

jive::output *
jive_bituquotient(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::udiv_op(type),
		{op1, op2})[0];
}
