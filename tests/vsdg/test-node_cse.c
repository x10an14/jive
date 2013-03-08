/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int
test_main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_VALUE_TYPE(vtype);
	jive_region * inner_region = jive_region_create_subregion(graph->root_region);
	jive_node * inner_node = jive_node_create(inner_region, 0, NULL, NULL, 1, &vtype);
	jive_node_normal_form * normal_form = jive_graph_get_nodeclass_form(graph, &JIVE_NODE);
	jive_node * outer_node = jive_node_cse_create(normal_form, graph->root_region, NULL, 0, NULL);

	assert(inner_node != outer_node);

	jive_view(graph, stderr);

	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-node_cse", test_main);