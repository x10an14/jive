#ifndef JIVE_VSDG_VALUETYPE_H
#define JIVE_VSDG_VALUETYPE_H

#include <jive/vsdg/basetype.h>
#include <jive/arch/registers.h>

typedef struct jive_value_type jive_value_type;
typedef struct jive_value_input jive_value_input;
typedef struct jive_value_output jive_value_output;
typedef struct jive_value_gate jive_value_gate;

extern const jive_type_class JIVE_VALUE_TYPE;
#define JIVE_DECLARE_VALUE_TYPE(name) const jive_value_type name##_struct = {{&JIVE_VALUE_TYPE}}; const jive_type * name = &name##_struct.base

struct jive_value_type {
	jive_type base;
};

extern const jive_input_class JIVE_VALUE_INPUT;
struct jive_value_input {
	jive_input base;
};

extern const jive_output_class JIVE_VALUE_OUTPUT;
struct jive_value_output {
	jive_output base;
};

extern const jive_gate_class JIVE_VALUE_GATE;
struct jive_value_gate {
	jive_gate base;
};

#endif
