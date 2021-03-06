/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"

#include <assert.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>
#include <jive/types/bitstring.h>
#include <jive/types/union.h>
#include <jive/view.h>
#include <jive/vsdg.h>


static int test_unnchoose(void)
{
	jive::graph graph;

	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive::value::type * decl_elems[] = {&bits8, &bits16, &bits32};
	static const jive::unn::declaration decl = {3, decl_elems};
	static jive::unn::type unntype(&decl);

	jive::addr::type addrtype;
	auto top = jive::test::simple_node_create(graph.root(), {}, {},
		{bits8, unntype, unntype, addrtype});

	auto u0 = jive_unify_create(&decl, 0, top->output(0));
	auto load = jive_load_by_address_create(top->output(3), &unntype, 0, NULL);

	auto c0 = jive_choose_create(1, top->output(1));
	auto c1 = jive_choose_create(0, u0);
	auto c2 = jive_choose_create(1, top->output(2));
	auto c3 = jive_choose_create(0, load);

	auto bottom = jive::test::simple_node_create(graph.root(),
		{bits16, bits8, bits16, bits8}, {c0, c1, c2, c3}, {bits8});
	graph.export_port(bottom->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(bottom->input(1)->origin()->node() == top);
	assert(c0->node()->operation() == c2->node()->operation());
	assert(dynamic_cast<const jive::load_op *>(&bottom->input(3)->origin()->node()->operation()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/union/test-unnchoose", test_unnchoose);

static int test_unnunify(void)
{
	jive::graph graph;
	
	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive::value::type * decl_elems[] = {&bits8, &bits16, &bits32};
	static const jive::unn::declaration decl = {3, decl_elems};
	static jive::unn::type unntype(&decl);

	static const jive::unn::declaration decl_empty = {0, NULL};
	static jive::unn::type unntype_empty(&decl_empty);
	
	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {bits8});

	auto u0 = jive_unify_create(&decl, 0, top->output(0));
	auto u1 = jive_empty_unify_create(graph.root(), &decl_empty);

	auto bottom = jive::test::simple_node_create(graph.root(), {unntype, unntype_empty}, {u0, u1},
		{bits8});
	graph.export_port(bottom->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(u0->node()->operation() != u1->node()->operation());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/union/test-unnunify", test_unnunify);
