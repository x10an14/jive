/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/registerset.h>

#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/float/flttype.h>

const jive::register_name jive_i386_reg_cc("cc", &jive_i386_regcls_flags, 0);
const jive::register_name jive_i386_reg_eax("eax", &jive_i386_regcls_gpr_eax, 0);
const jive::register_name jive_i386_reg_ecx("ecx", &jive_i386_regcls_gpr_ecx, 1);
const jive::register_name jive_i386_reg_edx("edx", &jive_i386_regcls_gpr_edx, 2);
const jive::register_name jive_i386_reg_ebx("ebx", &jive_i386_regcls_gpr_ebx, 3);
const jive::register_name jive_i386_reg_esi("esi", &jive_i386_regcls_gpr_esi, 6);
const jive::register_name jive_i386_reg_edi("edi", &jive_i386_regcls_gpr_edi, 7);
const jive::register_name jive_i386_reg_ebp("ebp", &jive_i386_regcls_gpr_ebp, 5);
const jive::register_name jive_i386_reg_esp("esp", &jive_i386_regcls_gpr_esp, 4);

const jive::register_name jive_i386_reg_st0("st0", &jive_i386_regcls_fp_st0, 0);

const jive::register_name jive_i386_reg_xmm0("xmm0", &jive_i386_regcls_sse_xmm0, 0);
const jive::register_name jive_i386_reg_xmm1("xmm1", &jive_i386_regcls_sse_xmm1, 1);
const jive::register_name jive_i386_reg_xmm2("xmm2", &jive_i386_regcls_sse_xmm2, 2);
const jive::register_name jive_i386_reg_xmm3("xmm3", &jive_i386_regcls_sse_xmm3, 3);
const jive::register_name jive_i386_reg_xmm4("xmm4", &jive_i386_regcls_sse_xmm4, 4);
const jive::register_name jive_i386_reg_xmm5("xmm5", &jive_i386_regcls_sse_xmm5, 5);
const jive::register_name jive_i386_reg_xmm6("xmm6", &jive_i386_regcls_sse_xmm6, 6);
const jive::register_name jive_i386_reg_xmm7("xmm7", &jive_i386_regcls_sse_xmm7, 7);

#define CLS(x) &jive_i386_regcls_##x
#define STACK4 &jive_stackslot_class_4_4

static const jive::bits::type bits16(16);
static const jive::bits::type bits32(32);
static const jive::flt::type flt;

const jive::register_class jive_i386_regcls_flags(
	&JIVE_REGISTER_RESOURCE, "flags", {&jive_i386_reg_cc},
	&jive_root_register_class, jive_resource_class_priority_reg_high,
	{{CLS(gpr_eax), {CLS(flags), CLS(gpr_eax)}}, {STACK4, {CLS(flags), CLS(gpr_eax), STACK4}}},
	&bits16, 16, 0, 0);

const jive::register_class jive_i386_regcls_gpr(
	&JIVE_REGISTER_RESOURCE, "gpr",
	{&jive_i386_reg_eax, &jive_i386_reg_ecx, &jive_i386_reg_ebx, &jive_i386_reg_edx,
	 &jive_i386_reg_esi, &jive_i386_reg_edi, &jive_i386_reg_ebp, &jive_i386_reg_esp},
	&jive_root_register_class, jive_resource_class_priority_reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_gpr_byte(
	&JIVE_REGISTER_RESOURCE, "gpr_byte_addressible",
	{&jive_i386_reg_eax, &jive_i386_reg_ecx, &jive_i386_reg_ebx, &jive_i386_reg_edx},
	&jive_i386_regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_gpr_eax(
	&JIVE_REGISTER_RESOURCE, "gpr_eax", {&jive_i386_reg_eax},
	&jive_i386_regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_gpr_ecx(
	&JIVE_REGISTER_RESOURCE, "gpr_ecx", {&jive_i386_reg_ecx},
	&jive_i386_regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_gpr_ebx(
	&JIVE_REGISTER_RESOURCE, "gpr_ebx", {&jive_i386_reg_ebx},
	&jive_i386_regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_gpr_edx(
	&JIVE_REGISTER_RESOURCE, "gpr_edx", {&jive_i386_reg_edx},
	&jive_i386_regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_gpr_esi(
	&JIVE_REGISTER_RESOURCE, "gpr_esi", {&jive_i386_reg_esi},
	&jive_i386_regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_gpr_edi(
	&JIVE_REGISTER_RESOURCE, "gpr_edi", {&jive_i386_reg_edi},
	&jive_i386_regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_gpr_esp(
	&JIVE_REGISTER_RESOURCE, "gpr_esp", {&jive_i386_reg_esp},
	&jive_i386_regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_gpr_ebp(
	&JIVE_REGISTER_RESOURCE, "gpr_ebp", {&jive_i386_reg_ebp},
	&jive_i386_regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class jive_i386_regcls_fp(
	&JIVE_REGISTER_RESOURCE, "fp", {&jive_i386_reg_st0},
	&jive_root_register_class, jive_resource_class_priority_reg_low,
	{{STACK4, {CLS(fp), STACK4}}}, &flt, 80, 80, 80);

const jive::register_class jive_i386_regcls_fp_st0(
	&JIVE_REGISTER_RESOURCE, "fp_st0", {&jive_i386_reg_st0},
	&jive_i386_regcls_fp, jive_resource_class_priority_reg_low,
	{{CLS(fp), {CLS(fp), CLS(fp)}}, {STACK4, {CLS(gpr), STACK4}}},
	&flt, 80, 80, 80);

const jive::register_class jive_i386_regcls_sse(
	&JIVE_REGISTER_RESOURCE, "sse",
	{&jive_i386_reg_xmm0, &jive_i386_reg_xmm1, &jive_i386_reg_xmm2, &jive_i386_reg_xmm3,
	 &jive_i386_reg_xmm4, &jive_i386_reg_xmm5, &jive_i386_reg_xmm6, &jive_i386_reg_xmm7},
	&jive_root_register_class, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class jive_i386_regcls_sse_xmm0(
	&JIVE_REGISTER_RESOURCE, "sse_xmm0", {&jive_i386_reg_xmm0},
	&jive_i386_regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class jive_i386_regcls_sse_xmm1(
	&JIVE_REGISTER_RESOURCE, "sse_xmm1", {&jive_i386_reg_xmm1},
	&jive_i386_regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class jive_i386_regcls_sse_xmm2(
	&JIVE_REGISTER_RESOURCE, "sse_xmm2", {&jive_i386_reg_xmm2},
	&jive_i386_regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class jive_i386_regcls_sse_xmm3(
	&JIVE_REGISTER_RESOURCE, "sse_xmm3", {&jive_i386_reg_xmm3},
	&jive_i386_regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class jive_i386_regcls_sse_xmm4(
	&JIVE_REGISTER_RESOURCE, "sse_xmm4", {&jive_i386_reg_xmm4},
	&jive_i386_regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class jive_i386_regcls_sse_xmm5(
	&JIVE_REGISTER_RESOURCE, "sse_xmm5", {&jive_i386_reg_xmm5},
	&jive_i386_regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class jive_i386_regcls_sse_xmm6(
	&JIVE_REGISTER_RESOURCE, "sse_xmm6", {&jive_i386_reg_xmm6},
	&jive_i386_regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class jive_i386_regcls_sse_xmm7(
	&JIVE_REGISTER_RESOURCE, "sse_xmm7", {&jive_i386_reg_xmm7},
	&jive_i386_regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);
