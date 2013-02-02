/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTION_CLASS_H
#define JIVE_ARCH_INSTRUCTION_CLASS_H

#include <string.h>

#include <jive/context.h>
#include <jive/arch/immediate-value.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/registers.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>

struct jive_buffer;
struct jive_compilate;
struct jive_label;
struct jive_label_symbol_mapper;
struct jive_section;

typedef struct jive_asmgen_imm jive_asmgen_imm;
typedef struct jive_codegen_imm jive_codegen_imm;
typedef struct jive_instruction_class jive_instruction_class;

/* immediates, as represented during code generation */

/** \brief Information about compile-time knowledge of value */
typedef enum jive_codegen_imm_info jive_codegen_imm_info;
typedef struct jive_codegen_imm jive_codegen_imm;

typedef enum jive_codegen_imm_info jive_codegen_imm_info;
enum jive_codegen_imm_info {
	/* the value is known at present time, and will be the same on
	each translation attempt; instruction encoding should use given value
	as is */
	jive_codegen_imm_info_static_known = 0,
	/* the value is known to be unknowable at translation time; instruction
	encoding must ensure that an arbitrary value can be substituted at link
	time */
	jive_codegen_imm_info_static_unknown = 1,
	/* the value is known at present time, but might change for future
	translation attempts; instruction encoding should use given value as
	is, but if the current value allows a "smaller" code representation
	as the one used in a previous translation attempt, it should stick
	with the larger representation (to ensure eventual convergence) */
	jive_codegen_imm_info_dynamic_known = 2,
	/* the value is not known at the present time, but a value will be
	supplied in subsequent translation attempts; instruction encoding
	should assume an arbitrary value at its own discretion and perform
	a dummy encoding */
	jive_codegen_imm_info_dynamic_unknown = 3
};

struct jive_codegen_imm {
	/** \brief Knowledge about immediate value */
	jive_codegen_imm_info info;
	/** \brief Numeric portion of immediate value */
	jive_immediate_int value;
	/** \brief Referenced symbol */
	jive_symref symref;
	/** \brief Whether the symbol is to be interpreted pc-relative */
	bool pc_relative;
};

/* immediates, as represented during asm generation */

struct jive_asmgen_imm {
	/** \brief Numeric portion of immediate value */
	jive_immediate_int value;
	/** \brief Symbol to be added to base value
	 
	String representation of the symbol to be added, or NULL if there
	is no symbol to be added. */
	const char * add_symbol;
	/** \brief Symbol to be subtracted from base value
	
	String representation of the symbol to be subtracted, or NULL if
	there is no symbol to be subtracted. */
	const char * sub_symbol;
};

/* instruction representation */

typedef enum {
	jive_instruction_flags_none = 0,
	/* instruction reuses first input register as output */
	jive_instruction_write_input = 1,
	/* first two input operands are commutative */
	jive_instruction_commutative = (1<<8),
	/* instruction is an (unconditional) jump */
	jive_instruction_jump = (1<<12),
	/* instruction is a relative jump */
	jive_instruction_jump_relative = (1<<13),
	/* instruction is a conditional jump and there is a matching "inverse" instruction */
	jive_instruction_jump_conditional_invertible = (1<<14)
} jive_instruction_flags;

typedef enum {
	jive_instruction_encoding_flags_none = 0,
	/* instruction is a conditional branch, and its decision logic
	is to be inverted during codegen */
	jive_instruction_encoding_flags_jump_conditional_invert = 1,
	
	/* the following flags may be updated by instruction encoding itself
	
	instructions may (depending on label distance etc.) have to choose
	between different displacement sizes, depending on labels; the idea is
	to start out conservative, but allow the instruction to "expand" its
	encoded size in case it does not fit (but never shrink) */
	
	jive_instruction_encoding_flags_option0 = (1<<16),
	jive_instruction_encoding_flags_option1 = (1<<17),
	jive_instruction_encoding_flags_option2 = (1<<18),
	jive_instruction_encoding_flags_option3 = (1<<19),
	jive_instruction_encoding_flags_option4 = (1<<20),
	jive_instruction_encoding_flags_option5 = (1<<21),
	jive_instruction_encoding_flags_option6 = (1<<22),
	jive_instruction_encoding_flags_option7 = (1<<23),
} jive_instruction_encoding_flags;

struct jive_instruction_class {
	/** \brief Descriptive name of instruction */
	const char * name;
	
	/** \brief Mnemonic name of instruction */
	const char * mnemonic;
	
	/**
		\brief Generate code
		\param target Target buffer to put encoded instructions into
		\param instruction Instruction to encode
	*/
	void (*encode)(
		const jive_instruction_class * icls,
		struct jive_section * target,
		const jive_register_name * inputs[],
		const jive_register_name * outputs[],
		const jive_codegen_imm immediates[],
		jive_instruction_encoding_flags * flags);
	
	/**
		\brief Generate mnemonic
		\param target Target buffer to put mnemonic instruction into
		\param instruction Instruction to encode
	*/
	void (*write_asm)(
		const jive_instruction_class * icls,
		struct jive_buffer * target,
		const jive_register_name * inputs[],
		const jive_register_name * outputs[],
		const jive_asmgen_imm immediates[],
		jive_instruction_encoding_flags * flags);
	
	const jive_register_class * const * inregs;
	const jive_register_class * const * outregs;
	
	jive_instruction_flags flags;
	
	unsigned short ninputs;
	unsigned short noutputs;
	unsigned short nimmediates;
	
	/** \brief Internal number, used for code generation */
	int code;
	
	/** \brief Inverse jump class (only meaningful if flag set accordingly) */
	const jive_instruction_class * inverse_jump;
};

extern const jive_instruction_class JIVE_PSEUDO_NOP;

#endif