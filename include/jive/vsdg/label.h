/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_LABEL_H
#define JIVE_VSDG_LABEL_H

#include <stdbool.h>
#include <stdint.h>

#include <jive/vsdg/region.h>

namespace jive {
	class output;
}

struct jive_linker_symbol;

typedef uint64_t jive_offset;

typedef struct jive_address jive_address;
typedef struct jive_label jive_label;
typedef struct jive_label_class jive_label_class;
typedef struct jive_label_external jive_label_external;

struct jive_address {
	jive_offset offset;
	jive_stdsectionid section;
};

static inline void
jive_address_init(jive_address * self, jive_stdsectionid section, jive_offset offset)
{
	self->offset = offset;
	self->section = section;
}

struct jive_label_class {
	const jive_label_class * parent;
	void (*fini)(jive_label * self);
};

extern const jive_label_class JIVE_LABEL;

typedef enum {
	jive_label_flags_none = 0,
	jive_label_flags_global = 1, /* whether label is supposed to be "globally visible" */
	jive_label_flags_external = 2, /* whether label must be resolved outside the current graph */
} jive_label_flags;

struct jive_label {
	const jive_label_class * class_;
	
	jive_label_flags flags;
};

static inline void
jive_label_fini(jive_label * self)
{
	self->class_->fini(self);
}

static inline bool
jive_label_isinstance(const jive_label * self, const jive_label_class * class_)
{
	const jive_label_class * cls = self->class_;
	while (cls) {
		if (cls == class_)
			return true;
		cls = cls->parent;
	}
	return false;
}

struct jive_label_external {
	jive_label base;
	std::string asmname;
	const struct jive_linker_symbol * symbol;
};

/**
	\brief Special label marking position of "current" instruction
*/
extern const jive_label jive_label_current;

extern const jive_label_class JIVE_LABEL_CURRENT;

/**
	\brief Special label marking offset from frame pointer
*/
extern const jive_label jive_label_fpoffset;

extern const jive_label_class JIVE_LABEL_FPOFFSET;

/**
	\brief Special label marking offset from stack pointer
*/
extern const jive_label jive_label_spoffset;

extern const jive_label_class JIVE_LABEL_SPOFFSET;

extern const jive_label_class JIVE_LABEL_EXTERNAL;

/**
	\brief Initialize label external
*/
void
jive_label_external_init(
	jive_label_external * self,
	const char * name,
	const struct jive_linker_symbol * symbol);

#endif
