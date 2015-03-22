/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 */

#include "test-registry.h"
#include "testnodes.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>
#include <jive/types/bitstring.h>
#include <jive/types/record.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>


static int _test_rcdgroup(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();
	
	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive::value::type * decl_elems[] = {&bits8, &bits16, &bits32};
	static const jive::rcd::declaration decl = {3, decl_elems};
	static jive::rcd::type rcdtype(&decl);

	static const jive::rcd::declaration decl_empty = {0, NULL};
	static jive::rcd::type rcdtype_empty(&decl_empty);
	const jive::base::type * tmparray0[] = {&bits8, &bits16, &bits32};
	
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		3, tmparray0);
	jive::output * tmparray1[] = {top->outputs[0],
		top->outputs[1], top->outputs[2]};

	jive::output * g0 = jive_group_create(&decl, 3, tmparray1);
	jive::output * g1 = jive_empty_group_create(graph, &decl_empty);

	const jive::base::type * tmparray2[] = {&rcdtype, &rcdtype_empty};
	jive::output * tmparray3[] = {g0, g1};
	jive_node * bottom = jive_test_node_create(graph->root_region,
		2, tmparray2,
			tmparray3,
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(g0->node()->operation() != g1->node()->operation());

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdgroup", _test_rcdgroup);

static int _test_rcdselect()
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive::value::type * decl_elems[] = {&bits8, &bits16, &bits32};
	static const jive::rcd::declaration decl = {3, decl_elems};
	static jive::rcd::type rcdtype(&decl);

	jive::addr::type addrtype;
	const jive::base::type * tmparray0[] = {&bits8, &bits16, &bits32, &rcdtype, &rcdtype, &addrtype};
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		6, tmparray0);
	jive::output * tmparray1[] = {top->outputs[0],
		top->outputs[1], top->outputs[2]};

	jive::output * g0 = jive_group_create(&decl, 3, tmparray1);
	jive::output * load = jive_load_by_address_create(top->outputs[5], &rcdtype, 0, NULL);

	jive::output * s0 = jive_select_create(1, top->outputs[3]);
	jive::output * s1 = jive_select_create(1, g0);
	jive::output * s2 = jive_select_create(2, top->outputs[4]);
	jive::output * s3 = jive_select_create(0, load);
	const jive::base::type * tmparray2[] = {&bits16, &bits16, &bits32, &bits8};
	jive::output * tmparray3[] = {s0, s1, s2, s3};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		4, tmparray2, tmparray3,
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(bottom->producer(1) == top);
	assert(s0->node()->operation() != s2->node()->operation());
	assert(dynamic_cast<const jive::load_op *>(&bottom->producer(3)->operation()));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdselect", _test_rcdselect);