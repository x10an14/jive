/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_THETA_H
#define JIVE_VSDG_THETA_H

#include <jive/vsdg/anchor.h>
#include <jive/vsdg/node.h>

namespace jive {
class gate;
}

struct jive_theta_build_state;

namespace jive {

class theta_op final : public region_anchor_op {
public:
	virtual
	~theta_op() noexcept;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

}

typedef struct jive_theta jive_theta;
typedef struct jive_theta_loopvar jive_theta_loopvar;

/**
	\brief Represent a theta construct under construction
*/
struct jive_theta {
	struct jive::region * region;
	struct jive_theta_build_state * internal_state;
};

/**
	\brief Represent information about a loop-variant value
*/
struct jive_theta_loopvar {
	jive::oport * value;
	jive::gate * gate;
};

/**
	\brief Begin constructing a loop region
*/
jive_theta
jive_theta_begin(struct jive::region * parent);

/**
	\brief Add a loop-variant variable with a pre-loop value
*/
jive_theta_loopvar
jive_theta_loopvar_enter(jive_theta self, jive::oport * pre_value);

/**
	\brief Set post-iteration value of a loop-variant variable
*/
void
jive_theta_loopvar_leave(jive_theta self, jive::gate * var, jive::oport * post_value);

/**
	\brief End constructing a loop region, specify repetition predicate
*/
jive::node *
jive_theta_end(jive_theta self, jive::oport * predicate,
	size_t npost_values, jive_theta_loopvar * post_values);

#endif
