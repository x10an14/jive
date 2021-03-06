/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_CALL_H
#define JIVE_ARCH_CALL_H

#include <memory>
#include <vector>

#include <jive/common.h>
#include <jive/vsdg/node.h>

namespace jive {
	class type;
}

/* FIXME: opaque type for now -- to be filled in later */
struct jive_calling_convention;

namespace jive {

class call_operation final : public simple_op {
public:
	virtual ~call_operation() noexcept;

	call_operation(
		const jive_calling_convention * calling_convention,
		const std::vector<std::unique_ptr<jive::base::type>> & argument_types,
		const std::vector<std::unique_ptr<jive::base::type>> & result_types);

	call_operation(const call_operation & other) = default;

	call_operation(call_operation && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive_calling_convention *
	calling_convention() const noexcept { return calling_convention_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::vector<jive::port> results_;
	std::vector<jive::port> arguments_;
	const jive_calling_convention * calling_convention_;
};

}

jive::node *
jive_call_by_address_node_create(struct jive::region * region,
	jive::output * target_address, const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const result_types[]);

std::vector<jive::output*>
jive_call_by_address_create(jive::output * target_address,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const result_types[]);

jive::node *
jive_call_by_bitstring_node_create(struct jive::region * region,
	jive::output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const result_types[]);

std::vector<jive::output*>
jive_call_by_bitstring_create(jive::output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const result_types[]);

#endif
