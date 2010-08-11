#ifndef JIVE_GATE_INTERFERENCE_H
#define JIVE_GATE_INTERFERENCE_H

#include <jive/util/hash.h>

typedef struct jive_gate_interference jive_gate_interference;
typedef struct jive_gate_interference_part jive_gate_interference_part;
typedef struct jive_gate_interference_hash jive_gate_interference_hash;

JIVE_DECLARE_HASH_TYPE(jive_gate_interference_hash, jive_gate_interference_part, struct jive_gate *, gate, chain);

#endif
