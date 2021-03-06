/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_REGION_H
#define JIVE_VSDG_REGION_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/common.h>
#include <jive/util/list.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/section.h>
#include <jive/vsdg/structural_node.h>

namespace jive {
	class substitution_map;
}

struct jive_cut;

namespace jive {

class node;
class simple_node;
class simple_op;
class structural_input;
class structural_op;
class structural_output;

class argument final : public output {
public:
	virtual
	~argument() noexcept;

	argument(
		jive::region * region,
		size_t index,
		jive::structural_input * input,
		const jive::port & port);

	argument(const argument &) = delete;

	argument(argument &&) = delete;

	argument &
	operator=(const argument &) = delete;

	argument &
	operator=(argument &&) = delete;

	virtual jive::node *
	node() const noexcept override;

	inline jive::structural_input *
	input() const noexcept
	{
		return input_;
	}

	struct {
		jive::argument * prev;
		jive::argument * next;
	} input_argument_list;

private:
	jive::structural_input * input_;
};

class result final : public input {
public:
	virtual
	~result() noexcept;

	result(
		jive::region * region,
		size_t index,
		jive::output * origin,
		jive::structural_output * output,
		const jive::port & port);

	result(const result &) = delete;

	result(result &&) = delete;

	result &
	operator=(const result &) = delete;

	result &
	operator=(result &&) = delete;

	virtual jive::node *
	node() const noexcept override;

	inline jive::structural_output *
	output() const noexcept
	{
		return output_;
	}

	struct {
		jive::result * prev;
		jive::result * next;
	} output_result_list;

private:
	jive::structural_output * output_;
};

class structural_node;

class region {
	typedef jive::detail::intrusive_list<
		jive::node,
		jive::node::region_node_list_accessor
	> region_nodes_list;

public:
	~region();

	region(jive::region * parent, jive::graph * graph);

	region(jive::structural_node * node);

	inline region_nodes_list::iterator
	begin()
	{
		return nodes.begin();
	}

	inline region_nodes_list::const_iterator
	begin() const
	{
		return nodes.begin();
	}

	inline region_nodes_list::iterator
	end()
	{
		return nodes.end();
	}

	inline region_nodes_list::const_iterator
	end() const
	{
		return nodes.end();
	}

	inline jive::graph *
	graph() const noexcept
	{
		return graph_;
	}

	inline jive::structural_node *
	node() const noexcept
	{
		return node_;
	}

	jive::argument *
	add_argument(jive::structural_input * input, const jive::port & port);

	void
	remove_argument(size_t index);

	inline size_t
	narguments() const noexcept
	{
		return arguments_.size();
	}

	inline jive::argument *
	argument(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < narguments());
		return arguments_[index];
	}

	jive::result *
	add_result(jive::output * origin, structural_output * output, const jive::port & port);

	void
	remove_result(size_t index);

	inline size_t
	nresults() const noexcept
	{
		return results_.size();
	}

	inline jive::result *
	result(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < nresults());
		return results_[index];
	}

	inline size_t
	nnodes() const noexcept
	{
		return nodes.size();
	}

	jive::simple_node *
	add_simple_node(const jive::simple_op & op, const std::vector<jive::output*> & operands);

	jive::structural_node *
	add_structural_node(const jive::structural_op & op, size_t nsubregions);

	void
	remove_node(jive::node * node);

	/**
		\brief Copy a region with substitutions
		\param target Target region to create nodes in
		\param substitution Operand and gate substitutions
		\param copy_arguments Copy region arguments
		\param copy_results Copy region results

		Copies all nodes of the specified region and its
		subregions into the target region. Substitutions
		will be performed as specified, and the substitution
		map will be updated as nodes are copied.
	*/
	void
	copy(
		region * target,
		substitution_map & smap,
		bool copy_arguments,
		bool copy_results) const;

	void
	prune(bool recursive);

	void
	normalize(bool recursive);

	region_nodes_list nodes;

	struct {
		jive::node * first;
		jive::node * last;
	} top_nodes;

	struct {
		jive::node * first;
		jive::node * last;
	} bottom_nodes;

private:
	jive::graph * graph_;
	jive::structural_node * node_;
	std::vector<jive::result*> results_;
	std::vector<jive::argument*> arguments_;
};

static inline void
remove(jive::node * node)
{
	return node->region()->remove_node(node);
}

template <class T> static inline bool
contains(const jive::region * region, bool recursive)
{
	for (const auto & node : region->nodes) {
		if (is_opnode<T>(node))
			return true;


		auto structnode = dynamic_cast<const jive::structural_node*>(node);
		if (recursive && structnode) {
			for (size_t n = 0; n < structnode->nsubregions(); n++)
				return contains<T>(structnode->subregion(n), recursive);
		}
	}

	return false;
}

static inline size_t
nnodes(const jive::region * region) noexcept
{
	size_t n = region->nnodes();
	for (const auto & node : region->nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t r = 0; r < snode->nsubregions(); r++)
				n += nnodes(snode->subregion(r));
		}
	}

	return n;
}

static inline size_t
nstructnodes(const jive::region * region) noexcept
{
	size_t n = 0;
	for (const auto & node : region->nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t r = 0; r < snode->nsubregions(); r++)
				n += nstructnodes(snode->subregion(r));
			n += 1;
		}
	}

	return n;
}

static inline size_t
nsimpnodes(const jive::region * region) noexcept
{
	size_t n = 0;
	for (const auto & node : region->nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t r = 0; r < snode->nsubregions(); r++)
				n += nsimpnodes(snode->subregion(r));
		} else {
			n += 1;
		}
	}

	return n;
}

static inline size_t
ninputs(const jive::region * region) noexcept
{
	size_t n = region->nresults();
	for (const auto & node : region->nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t r = 0; r < snode->nsubregions(); r++)
				n += ninputs(snode->subregion(r));
		}
		n += node.ninputs();
	}

	return n;
}

} //namespace

#endif
