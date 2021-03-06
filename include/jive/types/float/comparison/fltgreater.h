/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTGREATER_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTGREATER_H

#include <jive/types/float/fltoperation-classes.h>

namespace jive {
namespace flt {

extern const char fltgreater_name[];

typedef detail::make_cmpop<
	std::greater<value_repr>,
	fltgreater_name,
	jive_binary_operation_none> gt_op;

}
}

jive::output *
jive_fltgreater(jive::output * operand1, jive::output * operand2);

#endif

