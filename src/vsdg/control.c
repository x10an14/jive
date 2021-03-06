/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/control.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>

namespace jive {
namespace base {
// explicit instantiation
template class domain_const_op<
	ctl::type, ctl::value_repr, ctl::format_value, ctl::type_of_value
>;
}

namespace ctl {

match_op::~match_op() noexcept
{}

match_op::match_op(
	size_t nbits,
	const std::unordered_map<uint64_t, uint64_t> & mapping,
	uint64_t default_alternative,
	size_t nalternatives)
: jive::unary_op()
, result_(type(nalternatives))
, argument_(jive::bits::type(nbits))
, default_alternative_(default_alternative)
, mapping_(mapping)
{}

bool
match_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const match_op*>(&other);
	return op
	    && op->result_ == result_
	    && op->argument_ == argument_
	    && op->default_alternative_ == default_alternative_
	    && op->mapping_ == mapping_;
}

const jive::port &
match_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return argument_;
}

const jive::port &
match_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
}

jive_unop_reduction_path_t
match_op::can_reduce_operand(const jive::output * arg) const noexcept
{
	if (arg->node() && dynamic_cast<const jive::bits::constant_op*>(&arg->node()->operation()))
		return jive_unop_reduction_constant;

	return jive_unop_reduction_none;
}

jive::output *
match_op::reduce_operand(jive_unop_reduction_path_t path, jive::output * arg) const
{
	if (path == jive_unop_reduction_constant) {
		auto op = static_cast<const jive::bits::constant_op*>(&arg->node()->operation());
		return jive_control_constant(arg->region(), nalternatives(),
			alternative(op->value().to_uint()));
	}

	return nullptr;
}

std::string
match_op::debug_string() const
{
	std::string str("[");
	for (const auto & pair : mapping_)
		str += detail::strfmt(pair.first, " -> ", pair.second, ", ");
	str += detail::strfmt(default_alternative_, "]");

	return "MATCH" + str;
}

std::unique_ptr<jive::operation>
match_op::copy() const
{
	return std::unique_ptr<jive::operation>(new match_op(*this));
}

jive::output *
match(
	size_t nbits,
	const std::unordered_map<uint64_t, uint64_t> & mapping,
	uint64_t default_alternative,
	size_t nalternatives,
	jive::output * operand)
{
	match_op op(nbits, mapping, default_alternative, nalternatives);
	return jive::create_normalized(operand->region(), op, {operand})[0];
}

}
}

jive::output *
jive_control_constant(jive::region * region, size_t nalternatives, size_t alternative)
{
	jive::ctl::constant_op op(jive::ctl::value_repr(alternative, nalternatives));
	return jive::create_normalized(region, op, {})[0];
}
