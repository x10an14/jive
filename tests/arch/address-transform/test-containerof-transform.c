/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg/traverser.h>

#include "testnodes.h"

static int test_main(void)
{
	jive::graph graph;

	jive::addr::type addrtype;
	jive::bits::type bits8(8);
	jive::bits::type bits16(16);
	jive::bits::type bits32(32);
	std::shared_ptr<const jive::rcd::declaration> decl(
		new jive::rcd::declaration({&bits8, &bits16, &bits32, &bits32}));

	jive::node * top = jive_test_node_create(graph.root(),
		{}, {}, std::vector<const jive::base::type*>(4, &bits32));

	auto address0 = jive_bitstring_to_address_create(top->output(0), 32, &addrtype);
	auto address1 = jive_bitstring_to_address_create(top->output(1), 32, &addrtype);
	auto address2 = jive_bitstring_to_address_create(top->output(2), 32, &addrtype);
	auto address3 = jive_bitstring_to_address_create(top->output(3), 32, &addrtype);
	
	auto container0 = jive_containerof(address0, decl, 0);
	auto container1 = jive_containerof(address1, decl, 1);
	auto container2 = jive_containerof(address2, decl, 2);
	auto container3 = jive_containerof(address3, decl, 3);

	auto offset0 = jive_address_to_bitstring_create(container0, 32, &container0->type());
	auto offset1 = jive_address_to_bitstring_create(container1, 32, &container1->type());
	auto offset2 = jive_address_to_bitstring_create(container2, 32, &container2->type());
	auto offset3 = jive_address_to_bitstring_create(container3, 32, &container3->type());

	jive::node * bottom = jive_test_node_create(graph.root(),
		std::vector<const jive::base::type*>(4, &bits32), {offset0, offset1, offset2, offset3},
		{&bits32});
	graph.export_port(bottom->output(0), "dummy");
	
	jive::view(graph.root(), stdout);

	jive::memlayout_mapper_simple mapper(4);
	jive_graph_address_transform(&graph, &mapper);

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	for (jive::node * node : jive::topdown_traverser(graph.root())) {
		for (size_t i = 0; i < node->ninputs(); i++){
			assert(!dynamic_cast<const jive::addr::type*>(&node->input(i)->type()));
		}
		for (size_t i = 0; i < node->noutputs(); i++){
			assert(!dynamic_cast<const jive::addr::type*>(&node->output(i)->type()));
		}
	}
	
	jive::node * sum = dynamic_cast<jive::output*>(bottom->input(0)->origin())->node();
	assert(sum->operation() == jive::bits::sub_op(32));
	jive::node * constant = dynamic_cast<jive::output*>(sum->input(1)->origin())->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 0));
	
	sum = dynamic_cast<jive::output*>(bottom->input(1)->origin())->node();
	assert(sum->operation() == jive::bits::sub_op(32));
	constant = dynamic_cast<jive::output*>(sum->input(1)->origin())->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 2));

	sum = dynamic_cast<jive::output*>(bottom->input(2)->origin())->node();
	assert(sum->operation() == jive::bits::sub_op(32));
	constant = dynamic_cast<jive::output*>(sum->input(1)->origin())->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 4));

	sum = dynamic_cast<jive::output*>(bottom->input(3)->origin())->node();
	assert(sum->operation() == jive::bits::sub_op(32));
	constant = dynamic_cast<jive::output*>(sum->input(1)->origin())->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 8));
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-containerof-transform", test_main);
