#include <jive/machine.h>
#include <jive/internal/instruction_str.h>

#include <jive/i386/machine.h>

const jive_cpureg jive_i386_regs [] = {
	[jive_i386_cc] = {.name = "cc", .regcls = &jive_i386_regcls[jive_i386_flags], .code = 0, .index = jive_i386_cc, .class_mask = 1<<jive_i386_flags},
	[jive_i386_eax] = {.name = "eax", .regcls = &jive_i386_regcls[jive_i386_gpr_eax], .code = 0, .index = jive_i386_eax, .class_mask = (1<<jive_i386_gpr) | (1<<jive_i386_gpr_byte) | (1<<jive_i386_gpr_eax)},
	[jive_i386_ecx] = {.name = "ecx", .regcls = &jive_i386_regcls[jive_i386_gpr_byte], .code = 1, .index = jive_i386_ecx, .class_mask = (1<<jive_i386_gpr) | (1<<jive_i386_gpr_byte)},
	[jive_i386_ebx] = {.name = "ebx", .regcls = &jive_i386_regcls[jive_i386_gpr_byte], .code = 2, .index = jive_i386_ebx, .class_mask = (1<<jive_i386_gpr) | (1<<jive_i386_gpr_byte)},
	[jive_i386_edx] = {.name = "edx", .regcls = &jive_i386_regcls[jive_i386_gpr_edx], .code = 3, .index = jive_i386_edx, .class_mask = (1<<jive_i386_gpr) | (1<<jive_i386_gpr_byte) | (1<<jive_i386_gpr_edx)},
	[jive_i386_esi] = {.name = "esi", .regcls = &jive_i386_regcls[jive_i386_gpr], .code = 6, .index = jive_i386_esi, .class_mask = 1<<jive_i386_gpr},
	[jive_i386_edi] = {.name = "edi", .regcls = &jive_i386_regcls[jive_i386_gpr], .code = 7, .index = jive_i386_edi, .class_mask = 1<<jive_i386_gpr},
	[jive_i386_ebp] = {.name = "ebp", .regcls = &jive_i386_regcls[jive_i386_gpr], .code = 5, .index = jive_i386_ebp, .class_mask = 1<<jive_i386_gpr},
	[jive_i386_esp] = {.name = "esp", .regcls = &jive_i386_regcls[jive_i386_gpr_esp], .code = 4, .index = jive_i386_esp, .class_mask = (1<<jive_i386_gpr) | (1<<jive_i386_gpr_esp)},
};

const jive_cpureg_class jive_i386_regcls [] = {
	[jive_i386_flags] = {
		.name = "flags", .nbits = 16,
		.regs = &jive_i386_regs[jive_i386_cc], .nregs = 1,
		.index = jive_i386_flags, .parent = 0,
		.class_mask = 1<<jive_i386_flags
	},
	[jive_i386_gpr] = {
		.name = "gpr", .nbits = 32,
		.regs = &jive_i386_regs[jive_i386_eax], .nregs = 8,
		.index = jive_i386_gpr, .parent = 0,
		.class_mask = 1<<jive_i386_gpr
	},
	[jive_i386_gpr_byte] = {
		.name = "gpr_byte_addressible", .nbits = 32,
		.regs = &jive_i386_regs[jive_i386_eax], .nregs = 4,
		.index = jive_i386_gpr_byte, .parent = &jive_i386_regcls[jive_i386_gpr],
		.class_mask = (1<<jive_i386_gpr) | (1<<jive_i386_gpr_byte) 
	},
	[jive_i386_gpr_eax] = {
		.name = "gpr_eax", .nbits = 32,
		.regs = &jive_i386_regs[jive_i386_eax], .nregs = 1,
		.index = jive_i386_gpr_eax, .parent = &jive_i386_regcls[jive_i386_gpr_byte],
		.class_mask = (1<<jive_i386_gpr) | (1<<jive_i386_gpr_byte) | (1<<jive_i386_gpr_eax) 
	},
	[jive_i386_gpr_edx] = {
		.name = "gpr_edx", .nbits = 32,
		.regs = &jive_i386_regs[jive_i386_edx], .nregs = 1,
		.index = jive_i386_gpr_edx, .parent = &jive_i386_regcls[jive_i386_gpr_byte],
		.class_mask = (1<<jive_i386_gpr) | (1<<jive_i386_gpr_byte) | (1<<jive_i386_gpr_edx) 
	},
	[jive_i386_gpr_esp] = {
		.name = "gpr_esp", .nbits = 32,
		.regs = &jive_i386_regs[jive_i386_esp], .nregs = 1,
		.index = jive_i386_gpr_esp, .parent = &jive_i386_regcls[jive_i386_gpr],
		.class_mask = (1<<jive_i386_gpr) |  (1<<jive_i386_gpr_esp) 
	}
};

static jive_instruction *
jive_i386_transfer(const jive_machine * machine, jive_value * in, jive_value ** out)
{
	jive_node * transfer = jive_instruction_create(in->node->graph,
		&jive_i386_instructions[jive_i386_int_transfer],
		&in, 0);
	
	*out = jive_instruction_output(transfer, 0);
	return (jive_instruction *) transfer;
}
//jive_value * (*transfer)(const jive_machine * machine, jive_value * value, const jive_cpureg * to);


const jive_machine jive_i386_machine = {
	.name = "i386",
	.regcls = jive_i386_regcls, .nregcls = 5,
	.regs = jive_i386_regs, .nregs = 9,
	
	.regcls_budget = {
		1 /* flags*/,
		8 /* gpr */ , 4 /* gpr_byte */ , 1 /* gpr_eax */, 1 /* gpr_edx */, 1/* gpr_esp */,
		0
	},
	
	
	.transfer = &jive_i386_transfer
};
