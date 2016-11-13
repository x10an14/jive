/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/load.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/store.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::addr::type addr;
	jive::mem::type mem;
	jive::bits::type bits64(64);
	jive_node * top = jive_test_node_create(graph.root(), {}, {}, {&bits64, &bits64, &mem});

	jive::output * address0 = jive_bitstring_to_address_create(top->output(0), 64, &addr);
	jive::output * address1 = jive_bitstring_to_address_create(top->output(1), 64, &addr);

	std::shared_ptr<const jive::rcd::declaration> decl(new jive::rcd::declaration({&addr, &addr}));

	jive::output * memberof = jive_memberof(address0, decl, 0);
	jive::output * containerof = jive_containerof(address1, decl, 1);

	jive_linker_symbol write_symbol;
	jive_label_external write_label;
	jive_label_external_init(&write_label, "write", &write_symbol);
	jive::output * label = jive_label_to_address_create(graph.root(), &write_label.base);
	jive::output * tmparray2[] = {memberof, containerof};
	const jive::base::type * tmparray3[] = {&addr, &addr};
	jive_node * call = jive_call_by_address_node_create(graph.root(),
		label, NULL,
		2, tmparray2,
		2, tmparray3);

	jive::output * constant = jive_bitconstant_unsigned(graph.root(), 64, 1);
	jive::output * arraysub = jive_arraysubscript(call->output(0), &addr, constant);

	jive::output * arrayindex = jive_arrayindex(call->output(0), call->output(1), &addr, &bits64);
	
	jive::output * state = top->output(2);
	jive::output * load = jive_load_by_address_create(arraysub, &addr, 1, &state);
	jive_node * store = jive_store_by_address_create(arraysub,
		&bits64, arrayindex, 1, &state)[0]->node();

	jive::output * o_addr = jive_address_to_bitstring_create(load, 64, &load->type());

	jive_node * bottom = jive_test_node_create(graph.root(),
		{&bits64, &mem}, {o_addr, store->output(0)}, {&bits64});
	graph.export_port(bottom->output(0), "dummy");

	jive_view(&graph, stdout);

	jive::memlayout_mapper_simple mapper(8);
	jive_graph_address_transform(&graph, &mapper);

	graph.prune();
	jive_view(&graph, stdout);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform", test_main);
