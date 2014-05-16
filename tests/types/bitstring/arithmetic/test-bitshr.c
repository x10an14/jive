/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/types/bitstring.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive_output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");

	jive_output * c0 = jive_bitconstant_unsigned(graph, 32, 16);
	jive_output * c1 = jive_bitconstant_unsigned(graph, 32, 2);
	jive_output * c2 = jive_bitconstant_unsigned(graph, 32, 32);

	jive_output * shr0 = jive_bitshr(s0, s1);
	jive_output * shr1 = jive_bitshr(c0, c1);
	jive_output * shr2 = jive_bitshr(c0, c2);

	jive_graph_export(graph, shr0);
	jive_graph_export(graph, shr1);
	jive_graph_export(graph, shr2);

	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	assert(jive_node_isinstance(shr0->node, &JIVE_BITSHR_NODE));
	assert(jive_node_isinstance(shr1->node, &JIVE_BITCONSTANT_NODE));
	assert(jive_node_isinstance(shr2->node, &JIVE_BITCONSTANT_NODE));

	jive_bitconstant_node * bc1 = jive_bitconstant_node_cast(shr1->node);
	jive_bitconstant_node * bc2 = jive_bitconstant_node_cast(shr2->node);
	assert(jive_bitconstant_equals_unsigned(bc1, 4));
	assert(jive_bitconstant_equals_unsigned(bc2, 0));
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshr", test_main);
