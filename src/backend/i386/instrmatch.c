/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/instrmatch.h>

#include <jive/arch/address.h>
#include <jive/arch/instruction.h>
#include <jive/arch/load.h>
#include <jive/arch/regvalue.h>
#include <jive/arch/store.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/types/bitstring.h>
#include <jive/util/typeinfo-map.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/traverser.h>

static inline bool
is_gpr_immediate(jive::output * arg)
{
	return arg->node() && dynamic_cast<const jive::regvalue_op *>(&arg->node()->operation());
}

static void
swap(jive::output ** arg1, jive::output ** arg2)
{
	auto tmp = *arg1;
	*arg1 = *arg2;
	*arg2 = tmp;
}

static jive::immediate
regvalue_to_immediate(const jive::output * regvalue)
{
	jive::node * rvnode = regvalue->node();
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::regvalue_op *>(&rvnode->operation()));
	auto value = dynamic_cast<jive::simple_output*>(rvnode->input(1)->origin());

	if (auto bcop = dynamic_cast<const jive::bits::constant_op *>(&value->node()->operation())) {
		return bcop->value().to_uint();
	}

	auto lbop = dynamic_cast<const jive::address::label_to_bitstring_op *>(
		&value->node()->operation());
	if (lbop) {
		return jive::immediate(0, lbop->label());
	}
	
	JIVE_DEBUG_ASSERT(false);
}

static void
convert_bitbinary(jive::node * node,
	const jive::instruction_class * regreg_icls,
	const jive::instruction_class * regimm_icls)
{
	auto arg1 = node->input(0)->origin();
	auto arg2 = node->input(1)->origin();
	
	bool commutative = (regreg_icls->flags() & jive_instruction_commutative) != 0;
	bool second_is_immediate = false;
	if (commutative && is_gpr_immediate(arg1)) {
		swap(&arg1, &arg2);
		second_is_immediate = true;
	} else if (is_gpr_immediate(arg2)) {
		second_is_immediate = true;
	}
	
	jive::node * instr;
	
	if (second_is_immediate) {
		auto imm = regvalue_to_immediate(arg2);
		auto tmp = jive_immediate_create(node->region(), &imm);
		instr = jive::create_instruction(node->region(), regimm_icls, {arg1, tmp});
	} else {
		instr = jive::create_instruction(node->region(), regreg_icls, {arg1, arg2});
	}
	
	node->output(0)->replace(instr->output(0));
}

static void
convert_divmod(jive::node * node, bool sign, size_t index)
{
	auto arg1 = node->input(0)->origin();
	auto arg2 = node->input(1)->origin();
	
	jive::output * ext;
	const jive::instruction_class * icls;
	if (sign) {
		jive::immediate imm(31);
		auto i = jive_immediate_create(node->region(), &imm);
		auto tmp = jive::create_instruction(node->region(),
			&jive::i386::instr_int_ashr_immediate::instance(), {arg1, i});
		
		ext = tmp->output(0);
		icls = &jive::i386::instr_int_sdiv::instance();
	} else {
		jive::immediate imm(0);
		auto i = jive_immediate_create(node->region(), &imm);
		auto tmp = jive::create_instruction(node->region(),
			&jive::i386::instr_int_load_imm::instance(), {i});
		
		ext = tmp->output(0);
		icls = &jive::i386::instr_int_udiv::instance();
	}

	auto instr = jive::create_instruction(node->region(), icls, {ext, arg1, arg2});
	node->output(0)->replace(instr->output(index));
}

static void
convert_complex_bitbinary(jive::node * node,
	const jive::instruction_class * icls,
	size_t result_index)
{
	auto arg1 = node->input(0)->origin();
	auto arg2 = node->input(1)->origin();

	auto instr = jive::create_instruction(node->region(), icls, {arg1, arg2});
	node->output(0)->replace(instr->output(result_index));
}

static const jive::detail::typeinfo_map<
	std::function<void(jive::node*)>
> bitbinary_map = {
	{
		&typeid(jive::bits::and_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_and::instance(),
			&jive::i386::instr_int_and_immediate::instance())
	},
	{
		&typeid(jive::bits::or_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_or::instance(),
			&jive::i386::instr_int_or_immediate::instance())
	},
	{
		&typeid(jive::bits::xor_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_xor::instance(),
			&jive::i386::instr_int_xor_immediate::instance())
	},
	{
		&typeid(jive::bits::add_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_add::instance(),
			&jive::i386::instr_int_add_immediate::instance())
	},
	{
		&typeid(jive::bits::sub_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_sub::instance(),
			&jive::i386::instr_int_sub_immediate::instance())
	},
	{
		&typeid(jive::bits::mul_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_mul::instance(),
			&jive::i386::instr_int_mul_immediate::instance())
	},
	{
		&typeid(jive::bits::xor_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_xor::instance(),
			&jive::i386::instr_int_xor_immediate::instance())
	},
	{
		&typeid(jive::bits::shr_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_shr::instance(),
			&jive::i386::instr_int_shr_immediate::instance())
	},
	{
		&typeid(jive::bits::ashr_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_ashr::instance(),
			&jive::i386::instr_int_ashr_immediate::instance())
	},
	{
		&typeid(jive::bits::shl_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_shl::instance(),
			&jive::i386::instr_int_shl_immediate::instance())
	},
	{
		&typeid(jive::bits::umulh_op),
		std::bind(convert_complex_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_mul_expand_unsigned::instance(),
			0)
	},
	{
		&typeid(jive::bits::smulh_op),
		std::bind(convert_complex_bitbinary,
			std::placeholders::_1,
			&jive::i386::instr_int_mul_expand_signed::instance(),
			0)
	},
	{
		&typeid(jive::bits::udiv_op),
		std::bind(convert_divmod,
			std::placeholders::_1,
			false,
			1)
	},
	{
		&typeid(jive::bits::sdiv_op),
		std::bind(convert_divmod,
			std::placeholders::_1,
			true,
			1)
	},
	{
		&typeid(jive::bits::umod_op),
		std::bind(convert_divmod,
			std::placeholders::_1,
			false,
			0)
	},
	{
		&typeid(jive::bits::smod_op),
		std::bind(convert_divmod,
			std::placeholders::_1,
			true,
			0)
	}
};

static void
match_gpr_bitbinary(jive::node * node)
{
	auto i = bitbinary_map.find(&typeid(node->operation()));
	if (i != bitbinary_map.end()) {
		i->second(node);
	} else {
		throw std::logic_error("Unknown operator");
	}
}

static void
convert_bitcmp(
	jive::node * node_,
	const jive::instruction_class * jump_icls,
	const jive::instruction_class * inv_jump_icls)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::ctl::match_op*>(&node_->operation()));

	auto node = node_->input(0)->origin()->node();

	auto arg1 = node->input(0)->origin();
	auto arg2 = node->input(1)->origin();
	
	bool second_is_immediate = is_gpr_immediate(arg2);
	if (!second_is_immediate) {
		bool first_is_immediate = is_gpr_immediate(arg1);
		if (first_is_immediate) {
			auto tmp = arg1;
			arg1 = arg2;
			arg2 = tmp;
			jump_icls = inv_jump_icls;
			second_is_immediate = first_is_immediate;
		}
	}
	
	jive::node * cmp_instr;
	
	if (second_is_immediate) {
		auto imm = regvalue_to_immediate(arg2);
		auto tmp = jive_immediate_create(node->region(), &imm);
		cmp_instr = jive::create_instruction(node->region(),
			&jive::i386::instr_int_cmp_immediate::instance(), {arg1, tmp});
	} else {
		cmp_instr = jive::create_instruction(node->region(),
			&jive::i386::instr_int_cmp::instance(), {arg1, arg2});
	}

	jive::immediate imm(0);
	auto tmp = jive_immediate_create(node->region(), &imm);
	auto jump_instr = jive::create_instruction(node->region(), jump_icls,
		{cmp_instr->output(0), tmp}, {}, {jive::ctl::boolean});
	node_->output(0)->replace(jump_instr->output(0));
}

static const jive::detail::typeinfo_map<
	std::pair<
		const jive::instruction_class *,
		const jive::instruction_class *
	>
> bitcompare_map = {
	{
		&typeid(jive::bits::eq_op),
		{
			&jive::i386::instr_int_jump_equal::instance(),
			&jive::i386::instr_int_jump_equal::instance()
		}
	},
	{
		&typeid(jive::bits::ne_op),
		{
			&jive::i386::instr_int_jump_notequal::instance(),
			&jive::i386::instr_int_jump_notequal::instance()
		}
	},
	{
		&typeid(jive::bits::slt_op),
		{
			&jive::i386::instr_int_jump_sless::instance(),
			&jive::i386::instr_int_jump_sgreater::instance()
		}
	},
	{
		&typeid(jive::bits::sle_op),
		{
			&jive::i386::instr_int_jump_slesseq::instance(),
			&jive::i386::instr_int_jump_sgreatereq::instance()
		}
	},
	{
		&typeid(jive::bits::sgt_op),
		{
			&jive::i386::instr_int_jump_sgreater::instance(),
			&jive::i386::instr_int_jump_sless::instance()
		}
	},
	{
		&typeid(jive::bits::sge_op),
		{
			&jive::i386::instr_int_jump_sgreatereq::instance(),
			&jive::i386::instr_int_jump_slesseq::instance()
		}
	},
	{
		&typeid(jive::bits::ult_op),
		{
			&jive::i386::instr_int_jump_uless::instance(),
			&jive::i386::instr_int_jump_ugreater::instance()
		}
	},
	{
		&typeid(jive::bits::ule_op),
		{
			&jive::i386::instr_int_jump_ulesseq::instance(),
			&jive::i386::instr_int_jump_ugreatereq::instance()
		}
	},
	{
		&typeid(jive::bits::ugt_op),
		{
			&jive::i386::instr_int_jump_ugreater::instance(),
			&jive::i386::instr_int_jump_uless::instance()
		}
	},
	{
		&typeid(jive::bits::uge_op),
		{
			&jive::i386::instr_int_jump_ugreatereq::instance(),
			&jive::i386::instr_int_jump_ulesseq::instance()
		}
	}
};

static void
match_gpr_bitcmp(jive::node * node)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::ctl::match_op*>(&node->operation()));

	auto i = bitcompare_map.find(&typeid(
		node->input(0)->origin()->node()->operation()));
	if (i != bitcompare_map.end()) {
		convert_bitcmp(node, i->second.first, i->second.second);
	} else {
		throw std::logic_error("Unknown operator");
	}
}

static const jive::detail::typeinfo_map<
	const jive::instruction_class *
> bitunary_map = {
	{
		&typeid(jive::bits::neg_op),
		&jive::i386::instr_int_neg::instance()
	},
	{
		&typeid(jive::bits::not_op),
		&jive::i386::instr_int_not::instance()
	}
};

static void
match_gpr_bitunary(jive::node * node)
{
	auto i = bitunary_map.find(&typeid(node->operation()));
	if (i != bitunary_map.end()) {
		auto icls = i->second;
		auto instr = jive::create_instruction(node->region(), icls, {node->input(0)->origin()});
		node->output(0)->replace(instr->output(0));
	} else {
		throw std::logic_error("Unknown operator");
	}
}

static void
match_gpr_immediate(jive::node * node)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::regvalue_op *>(&node->operation()));

	auto imm = regvalue_to_immediate(node->output(0));
	auto tmp = jive_immediate_create(node->region(), &imm);
	auto instr = jive::create_instruction(node->region(),
		&jive::i386::instr_int_load_imm::instance(), {tmp});

	node->output(0)->replace(instr->output(0));
}

typedef enum {
	jive_i386_addr_mode_disp
} jive_i386_addr_mode;

typedef struct jive_i386_addr_info {
	jive_i386_addr_mode mode;
	union {
		struct {
			jive::output * base;
			int32_t offset;
		} disp;
	} info;
} jive_i386_addr_info;

static jive_i386_addr_info
classify_address(jive::output * output)
{
	jive_i386_addr_info info;
	info.mode = jive_i386_addr_mode_disp;
	info.info.disp.base = output;
	info.info.disp.offset = 0;
	return info;
}

static void
match_gpr_load(jive::node * node)
{
	std::vector<jive::port> iports;
	std::vector<jive::port> oports;
	std::vector<jive::output*> operands;

	jive_i386_addr_info info = classify_address(node->input(0)->origin());
	JIVE_DEBUG_ASSERT(info.mode == jive_i386_addr_mode_disp);

	jive::immediate imm(info.info.disp.offset);
	operands.push_back(info.info.disp.base);
	operands.push_back(jive_immediate_create(node->region(), &imm));

	for (size_t n = 1; n < node->ninputs(); n++) {
		iports.push_back(node->input(n)->port());
		operands.push_back(node->input(n)->origin());
	}

	for (size_t n = 1; n < node->noutputs(); n++)
		oports.push_back(node->output(n)->port());

	auto instr = jive::create_instruction(node->region(),
		&jive::i386::instr_int_load32_disp::instance(), operands, iports, oports);

	node->output(0)->replace(instr->output(0));
	for (size_t n = 1; n < node->noutputs(); n++)
		node->output(n)->replace(instr->output(n));
}

static void
match_gpr_store(jive::node * node)
{
	std::vector<jive::port> iports;
	std::vector<jive::port> oports;
	std::vector<jive::output*> operands;

	jive_i386_addr_info info = classify_address(node->input(0)->origin());
	JIVE_DEBUG_ASSERT(info.mode == jive_i386_addr_mode_disp);

	jive::immediate imm(info.info.disp.offset);
	operands.push_back(info.info.disp.base);
	operands.push_back(node->input(1)->origin());
	operands.push_back(jive_immediate_create(node->region(), &imm));

	for (size_t n = 2; n < node->ninputs(); n++) {
		iports.push_back(node->input(n)->port());
		operands.push_back(node->input(n)->origin());
	}

	for (size_t n = 0; n < node->noutputs(); n++)
		oports.push_back(node->output(n)->port());

	auto instr = jive::create_instruction(node->region(),
		&jive::i386::instr_int_store32_disp::instance(), operands, iports, oports);

	for (size_t n = 0; n < node->noutputs(); n++)
		node->output(n)->replace(instr->output(n));
}

static void
match_single(jive::node * node, const jive_regselector * regselector)
{
	if (dynamic_cast<const jive::bits::binary_op *>(&node->operation())) {
		auto regcls = jive_regselector_map_output(regselector,
			dynamic_cast<jive::simple_output*>(node->output(0)));
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_bitbinary(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::bits::unary_op *>(&node->operation())) {
		auto regcls = jive_regselector_map_output(regselector,
			dynamic_cast<jive::simple_output*>(node->output(0)));
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_bitunary(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::ctl::match_op *>(&node->operation())
		&& dynamic_cast<const jive::bits::compare_op *>(
		& node->input(0)->origin()->node()->operation())) {
		auto cmp_input = dynamic_cast<jive::simple_input*>(node->input(0)->origin()->node()->input(0));
		auto regcls = jive_regselector_map_input(regselector, cmp_input);
		if (true || (regcls == &jive_i386_regcls_gpr)) {
			match_gpr_bitcmp(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::regvalue_op *>(&node->operation())) {
		auto regcls = jive_regselector_map_output(regselector,
			dynamic_cast<jive::simple_output*>(node->output(0)));
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_immediate(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::load_op *>(&node->operation())) {
		auto regcls = jive_regselector_map_output(regselector,
			dynamic_cast<jive::simple_output*>(node->output(0)));
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_load(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::store_op *>(&node->operation())) {
		auto regcls = jive_regselector_map_input(regselector,
			dynamic_cast<jive::simple_input*>(node->input(1)));
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_store(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	}
}

void
jive_i386_match_instructions(jive::graph * graph, const jive_regselector * regselector)
{
	for (jive::node * node : jive::bottomup_traverser(graph->root())) {
		match_single(node, regselector);
	}
}

