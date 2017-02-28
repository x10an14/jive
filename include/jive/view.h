/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_H
#define JIVE_VIEW_H

#include <string>

namespace jive {

class region;

void
view(const jive::region * region, FILE * out);

std::string
view(const jive::region * region);

std::string
region_tree(const jive::region * region);

void
region_tree(const jive::region * region, FILE * out);

}

#endif
