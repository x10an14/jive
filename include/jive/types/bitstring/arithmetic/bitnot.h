/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITNOT_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITNOT_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitunary_operation_class JIVE_BITNOT_NODE_;
#define JIVE_BITNOT_NODE (JIVE_BITNOT_NODE_.base.base)

namespace jive {
namespace bitstring {

class not_operation final : public jive::bits_unary_operation {
public:
	virtual
	~not_operation() noexcept;

	inline
	not_operation(const jive_bitstring_type & type) noexcept
		: bits_unary_operation(type)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive_output * const arguments[]) const override;

	virtual value_repr
	reduce_constant(
		const value_repr & arg) const override;

	virtual std::string
	debug_string() const override;
};

}
}

/**
	\brief Create bitnot
	\param operand Input value
	\returns Bitstring value representing not
	
	Convenience function to create negation of value.
*/
jive_output *
jive_bitnot(jive_output * operand);

#endif
