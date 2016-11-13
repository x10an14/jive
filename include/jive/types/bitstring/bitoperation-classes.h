/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H
#define JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H

#include <jive/types/bitstring/type.h>
#include <jive/types/bitstring/value-representation.h>
#include <jive/vsdg/operators.h>

namespace jive {
namespace bits {

/* Represents a unary operation on a bitstring of a specific width,
 * produces another bitstring of the same width. */
class unary_op : public base::unary_op {
public:
	virtual ~unary_op() noexcept;

	inline unary_op(const jive::bits::type & type) noexcept : type_(type) {}

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	inline const jive::bits::type & type() const noexcept { return type_; }

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::oport * arg) const noexcept override;

	virtual jive::oport *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::oport * arg) const override;

	virtual value_repr
	reduce_constant(
		const value_repr & arg) const = 0;

private:
	jive::bits::type type_;
};

/* Represents a binary operation (possibly normalized n-ary if associative)
 * on a bitstring of a specific width, produces another bitstring of the
 * same width. */
class binary_op : public base::binary_op {
public:
	virtual ~binary_op() noexcept;

	inline binary_op(const jive::bits::type & type, size_t arity = 2) noexcept
		: type_(type)
		, arity_(arity)
	{}

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::oport * arg1,
		const jive::oport * arg2) const noexcept override;

	virtual jive::oport *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::oport * arg1,
		jive::oport * arg2) const override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const = 0;


	inline const jive::bits::type & type() const noexcept { return type_; }

	inline size_t
	arity() const noexcept { return arity_; }

private:
	jive::bits::type type_;
	size_t arity_;
};

enum class compare_result {
	undecidable,
	static_true,
	static_false
};

class compare_op : public base::binary_op {
public:
	inline
	compare_op(
		const jive::bits::type & type) noexcept
		: type_(type)
	{
	}

	virtual ~compare_op() noexcept;

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::oport * arg1,
		const jive::oport * arg2) const noexcept override;

	virtual jive::oport *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::oport * arg1,
		jive::oport * arg2) const override;

	virtual compare_result
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const = 0;

	inline const jive::bits::type &
	type() const noexcept { return type_; }

private:
	jive::bits::type type_;
	size_t arity_;
};

}
}

#endif
