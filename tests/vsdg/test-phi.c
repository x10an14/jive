/*
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/phi.h>

#include "testnodes.h"

static int test_main()
{
	jive::graph graph;

	jive::test::valuetype vtype;
	jive::fct::type f0type((const std::vector<const jive::base::type*>){}, {});
	jive::fct::type f1type({&vtype}, {&vtype});

	jive::phi_builder pb;
	pb.begin(graph.root());
	auto rv1 = pb.add_recvar(f0type);
	auto rv2 = pb.add_recvar(f0type);
	auto rv3 = pb.add_recvar(f1type);

	jive::lambda_builder lb;
	lb.begin(pb.region(), f0type);
	auto lambda0 = lb.end({})->output(0);

	lb.begin(pb.region(), f0type);
	auto lambda1 = lb.end({})->output(0);

	lb.begin(pb.region(), f1type);
	auto dep = lb.add_dependency(rv3->value());
	jive::oport * arg[1] = {lb.region()->argument(0)};
	auto ret = jive_apply_create(dep, 1, arg)[0];
	auto lambda2 = lb.end({ret})->output(0);

	rv1->set_value(lambda0);
	rv2->set_value(lambda1);
	rv3->set_value(lambda2);

	auto phi = pb.end();
	graph.export_port(phi->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	jive::node * lambda_node2;
	lambda_node2 = phi->subregion(0)->result(2)->origin()->node();
	assert(jive_lambda_is_self_recursive(lambda_node2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-phi", test_main);
