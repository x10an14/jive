/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESSTYPE_H
#define JIVE_ARCH_ADDRESSTYPE_H

#include <jive/vsdg/valuetype.h>

namespace jive {
namespace addr {

/* address type */

class type final : public jive::value::type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive::value::type() {};

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::addr::type * copy() const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;

	static const type & singleton();
};

/* address output */

class output final : public jive::value::output {
public:
	virtual ~output() noexcept;

	output(jive_node * node, size_t index);

private:
	output(const output & rhs) = delete;
	output& operator=(const output & rhs) = delete;

	jive::addr::type type_;
};

/* address gate */

class gate final : public jive::value::gate {
public:
	virtual ~gate() noexcept;

	gate(jive_graph * graph, const char name[]);

	virtual const jive::addr::type & type() const noexcept { return type_; }

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::addr::type type_;
};

}
}

#endif
