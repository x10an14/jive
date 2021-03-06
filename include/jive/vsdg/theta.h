/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_THETA_H
#define JIVE_VSDG_THETA_H

#include <jive/vsdg/control.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/structural_node.h>

namespace jive {

class theta_op final : public structural_op {
public:
	virtual
	~theta_op() noexcept;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

static inline bool
is_theta_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::theta_op*>(&op) != nullptr;
}

static inline bool
is_theta_node(const jive::node * node) noexcept
{
	return is_opnode<theta_op>(node);
}

class theta;
class theta_builder;

class loopvar final {
	friend theta;

public:
	inline constexpr
	loopvar()
	: input_(nullptr)
	, output_(nullptr)
	{}

private:
	inline constexpr
	loopvar(
		jive::structural_input * input,
		jive::structural_output * output)
	: input_(input)
	, output_(output)
	{}

public:
	inline jive::structural_input *
	input() const noexcept
	{
		return input_;
	}

	inline jive::structural_output *
	output() const noexcept
	{
		return output_;
	}

	inline jive::argument *
	argument() const noexcept
	{
		if (input_ == nullptr)
			return nullptr;

		JIVE_DEBUG_ASSERT(input_->arguments.first != nullptr
		&& input_->arguments.first == input_->arguments.last);
		return input_->arguments.first;
	}

	inline jive::result *
	result() const noexcept
	{
		if (output_ == nullptr)
			return nullptr;

		JIVE_DEBUG_ASSERT(output_->results.first != nullptr
		&& output_->results.first == output_->results.last);
		return output_->results.first;
	}

	inline bool
	operator==(const loopvar & other) const noexcept
	{
		return input_ == other.input_ && output_ == other.output_;
	}

	inline bool
	operator!=(const loopvar & other) const noexcept
	{
		return !(*this == other);
	}

private:
	jive::structural_input * input_;
	jive::structural_output * output_;
};

class theta final {
public:
	class loopvar_iterator {
	public:
		inline constexpr
		loopvar_iterator(const jive::loopvar & lv) noexcept
		: lv_(lv)
		{}

		inline const loopvar_iterator &
		operator++() noexcept
		{
			auto input = lv_.input();
			if (input == nullptr)
				return *this;

			auto node = input->node();
			auto index = input->index();
			if (index == node->ninputs()-1) {
				lv_ = loopvar(nullptr, nullptr);
				return *this;
			}

			index++;
			lv_ = loopvar(node->input(index), node->output(index));
			return *this;
		}

		inline const loopvar_iterator
		operator++(int) noexcept
		{
			loopvar_iterator it(*this);
			++(*this);
			return it;
		}

		inline bool
		operator==(const loopvar_iterator & other) const noexcept
		{
			return lv_ == other.lv_;
		}

		inline bool
		operator!=(const loopvar_iterator & other) const noexcept
		{
			return !(*this == other);
		}

		inline loopvar &
		operator*() noexcept
		{
			return lv_;
		}

		inline loopvar *
		operator->() noexcept
		{
			return &lv_;
		}

	private:
		loopvar lv_;
	};

	inline
	theta(jive::structural_node * node)
	: node_(node)
	{
		if (!dynamic_cast<const jive::theta_op*>(&node->operation()))
			throw jive::compiler_error("Expected theta node.");
	}

	inline jive::structural_node *
	node() const noexcept
	{
		return node_;
	}

	inline jive::region *
	subregion() const noexcept
	{
		return node_->subregion(0);
	}

	inline jive::result *
	predicate() const noexcept
	{
		auto result = node_->subregion(0)->result(0);
		JIVE_DEBUG_ASSERT(dynamic_cast<const jive::ctl::type*>(&result->type()));
		return result;
	}

	inline size_t
	nloopvars() const noexcept
	{
		JIVE_DEBUG_ASSERT(node_->ninputs() == node_->noutputs());
		return node_->ninputs();
	}

	inline theta::loopvar_iterator
	begin() const
	{
		if (node_->ninputs() == 0)
			return loopvar_iterator({});

		return loopvar_iterator({node_->input(0), node_->output(0)});
	}

	inline theta::loopvar_iterator
	end() const
	{
		return loopvar_iterator({});
	}

	inline std::shared_ptr<jive::loopvar>
	add_loopvar(jive::output * origin)
	{
		auto input = node_->add_input(origin->type(), origin);
		auto output = node_->add_output(origin->type());
		auto argument = node_->subregion(0)->add_argument(input, origin->type());
		node_->subregion(0)->add_result(argument, output, origin->type());
		return std::make_shared<loopvar>(loopvar(input, output));
	}

private:
	jive::structural_node * node_;
};


class theta_builder final {
public:
	inline jive::region *
	subregion() const noexcept
	{
		return theta_ ? theta_->node()->subregion(0) : nullptr;
	}

	inline theta::loopvar_iterator
	begin() const
	{
		return theta_ ? theta_->begin() : theta::loopvar_iterator({});
	}

	inline theta::loopvar_iterator
	end() const
	{
		return theta_ ? theta_->end() : theta::loopvar_iterator({});
	}

	inline jive::region *
	begin_theta(jive::region * parent)
	{
		if (theta_)
			return theta_->node()->subregion(0);

		auto node = parent->add_structural_node(jive::theta_op(), 1);
		auto predicate = jive_control_false(node->subregion(0));
		node->subregion(0)->add_result(predicate, nullptr, jive::ctl::type(2));
		theta_ = std::make_unique<jive::theta>(node);
		return node->subregion(0);
	}

	inline std::shared_ptr<jive::loopvar>
	add_loopvar(jive::output * origin)
	{
		if (!theta_)
			return nullptr;

		return theta_ ? theta_->add_loopvar(origin) : nullptr;
	}

	inline std::unique_ptr<jive::theta>
	end_theta(jive::output * predicate)
	{
		if (!theta_)
			return nullptr;

		auto p = theta_->node()->subregion(0)->result(0);
		auto ctlconstant = p->origin()->node();
		p->divert_origin(predicate);
		if (!ctlconstant->has_users())
			remove(ctlconstant);
		return std::move(theta_);
	}

private:
	std::unique_ptr<jive::theta> theta_;
};

}

#endif
