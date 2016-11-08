/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive_view(&graph, stdout);

	jive_graph_prune(&graph);

	assert(graph.root()->nodes.size() == 1);

	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-empty_graph_pruning", test_main);
