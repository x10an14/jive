/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitsgreater.h>

#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

sgt_op::~sgt_op() noexcept {}

bool
sgt_op::operator==(const operation & other) const noexcept
{
	const sgt_op * o = dynamic_cast<const sgt_op *>(&other);
	return o && o->type() == type();
}

jive_node *
sgt_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<sgt_op>(
		*this,
		&JIVE_BITSGREATER_NODE,
		region,
		arguments[0],
		arguments[1]);
}

compare_result
sgt_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char result = jive_bitstring_sgreater(
		&arg1[0], &arg2[0], nbits);

	switch (result) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
sgt_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sgt_op::debug_string() const
{
	return "BITSGREATER";
}

std::unique_ptr<jive::operation>
sgt_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sgt_op(*this));
}

}
}

const jive_node_class JIVE_BITSGREATER_NODE = {
	parent : &JIVE_BITCOMPARISON_NODE,
	name : "BITSGREATER",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitsgreater(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::sgt_op(type),
		{op1, op2})[0];
}
