/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>

#include <stdio.h>
#include <string.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/phi.h>
#include <jive/vsdg/structural_node.h>

/* lambda node */

namespace jive {
namespace fct {

lambda_op::~lambda_op() noexcept
{
}

bool
lambda_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const lambda_op*>(&other);
	return op && op->function_type() == function_type();
}

size_t
lambda_op::narguments() const noexcept
{
	return 0;
}

size_t
lambda_op::nresults() const noexcept
{
	return 1;
}

std::string
lambda_op::debug_string() const
{
	return "LAMBDA";
}

std::unique_ptr<jive::operation>
lambda_op::copy() const
{
	return std::unique_ptr<jive::operation>(new lambda_op(*this));
}

}
}

bool
jive_lambda_is_self_recursive(const jive::node * self)
{
	JIVE_DEBUG_ASSERT(self->noutputs() == 1);

	auto lambda = dynamic_cast<const jive::structural_node*>(self);

	auto phi_region = self->region();
	auto phi = phi_region->node();
	if (phi && typeid(phi->operation()) != typeid(jive::phi_op))
		return false;

	/* find index of lambda output in the phi leave node */
	size_t index = phi_region->nresults();
	for (const auto & user : *self->output(0)) {
		if (dynamic_cast<const jive::result*>(user)) {
			index = user->index();
			break;
		}
	}
	if (index == phi_region->nresults())
		return false;

	/* the lambda is self-recursive if one of its external dependencies originates from the same
	*  index in the phi enter node
	*/
	for (size_t n = 0; n < lambda->ninputs(); n++) {
		if (lambda->input(n)->origin()->index() == index)
			return true;
	}

	return false;
}
