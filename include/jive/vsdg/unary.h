/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_UNARY_H
#define JIVE_VSDG_UNARY_H

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/simple-normal-form.h>

typedef size_t jive_unop_reduction_path_t;

namespace jive {

class unary_normal_form final : public simple_normal_form {
public:
	virtual
	~unary_normal_form() noexcept;

	unary_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph);

	virtual bool
	normalize_node(jive::node * node) const override;

	virtual std::vector<jive::output*>
	normalized_create(
		jive::region * region,
		const jive::simple_op & op,
		const std::vector<jive::output*> & arguments) const override;

	virtual void
	set_reducible(bool enable);
	inline bool
	get_reducible() const noexcept { return enable_reducible_; }

private:
	bool enable_reducible_;
};

/**
	\brief Unary operator
	
	Operator taking a single argument.
*/
class unary_op : public simple_op {
public:
	virtual ~unary_op() noexcept;

	virtual size_t
	narguments() const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(const jive::output * arg) const noexcept = 0;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const = 0;

	static jive::unary_normal_form *
	normal_form(jive::graph * graph) noexcept
	{
		return static_cast<jive::unary_normal_form*>(graph->node_normal_form(typeid(unary_op)));
	}
};

}

typedef struct jive_unary_operation_normal_form jive_unary_operation_normal_form;
typedef struct jive_unary_operation_normal_form_class jive_unary_operation_normal_form_class;

static const jive_unop_reduction_path_t jive_unop_reduction_none = 0;
/* operation is applied to constant, compute immediately */
static const jive_unop_reduction_path_t jive_unop_reduction_constant = 1;
/* operation does not change input operand */
static const jive_unop_reduction_path_t jive_unop_reduction_idempotent = 2;
/* operation is applied on inverse operation, can eliminate */
static const jive_unop_reduction_path_t jive_unop_reduction_inverse = 4;
/* operation "supersedes" immediately preceding operation */
static const jive_unop_reduction_path_t jive_unop_reduction_narrow = 5;
/* operation can be distributed into operands of preceding operation */
static const jive_unop_reduction_path_t jive_unop_reduction_distribute = 6;

#endif
