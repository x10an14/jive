#ifndef JIVE_VSDG_NEGOTIATOR_H
#define JIVE_VSDG_NEGOTIATOR_H

#include <stdbool.h>

#include <jive/util/hash.h>

struct jive_context;
struct jive_graph;
struct jive_node;
struct jive_input;
struct jive_output;
struct jive_gate;
struct jive_region;

typedef struct jive_negotiator jive_negotiator;
typedef struct jive_negotiator_class jive_negotiator_class;
typedef struct jive_negotiator_option jive_negotiator_option;
typedef struct jive_negotiator_option_class jive_negotiator_option_class;
typedef struct jive_negotiator_port jive_negotiator_port;
typedef struct jive_negotiator_connection jive_negotiator_connection;
typedef struct jive_negotiator_constraint jive_negotiator_constraint;
typedef struct jive_negotiator_constraint_class jive_negotiator_constraint_class;
typedef struct jive_negotiator_identity_constraint jive_negotiator_identity_constraint;

struct jive_negotiator_option {
	/* empty */
};

struct jive_negotiator_port {
	jive_negotiator_constraint * constraint;
	struct {
		jive_negotiator_port * prev;
		jive_negotiator_port * next;
	} constraint_port_list;
	
	jive_negotiator_connection * connection;
	struct {
		jive_negotiator_port * prev;
		jive_negotiator_port * next;
	} connection_port_list;
	
	jive_negotiator_option * option;
	
	struct {
		jive_negotiator_port * prev;
		jive_negotiator_port * next;
	} hash_chain;
	union {
		struct jive_input * input;
		struct jive_output * output;
	} hash_key;
};

struct jive_negotiator_connection {
	jive_negotiator * negotiator;
	struct {
		jive_negotiator_port * first;
		jive_negotiator_port * last;
	} ports;
	
	struct {
		jive_negotiator_connection * prev;
		jive_negotiator_connection * next;
	} negotiator_connection_list;
	bool validated;
};

jive_negotiator_connection *
jive_negotiator_connection_create(jive_negotiator * negotiator);

void
jive_negotiator_connection_invalidate(jive_negotiator_connection * self);

struct jive_negotiator_constraint {
	const jive_negotiator_constraint_class * class_;
	jive_negotiator * negotiator;
	struct {
		jive_negotiator_port * first;
		jive_negotiator_port * last;
	} ports;
	
	struct {
		jive_negotiator_constraint * prev;
		jive_negotiator_constraint * next;
	} negotiator_constraint_list;
	
	struct {
		jive_negotiator_constraint * prev;
		jive_negotiator_constraint * next;
	} hash_chain;
	union {
		struct jive_node * node;
		struct jive_gate * gate;
	} hash_key;
};

struct jive_negotiator_constraint_class {
	void (*fini)(jive_negotiator_constraint * self);
	void (*revalidate)(jive_negotiator_constraint * self, jive_negotiator_port * port);
};

jive_negotiator_constraint *
jive_negotiator_identity_constraint_create(jive_negotiator * self);

JIVE_DECLARE_HASH_TYPE(jive_negotiator_node_hash, jive_negotiator_constraint, struct jive_node *, hash_key.node, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_negotiator_gate_hash, jive_negotiator_constraint, struct jive_gate *, hash_key.gate, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_negotiator_input_hash, jive_negotiator_port, struct jive_input *, hash_key.input, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_negotiator_output_hash, jive_negotiator_port, struct jive_output *, hash_key.output, hash_chain);

typedef struct jive_negotiator_node_hash jive_negotiator_node_hash;
typedef struct jive_negotiator_gate_hash jive_negotiator_gate_hash;
typedef struct jive_negotiator_input_hash jive_negotiator_input_hash;
typedef struct jive_negotiator_output_hash jive_negotiator_output_hash;

struct jive_negotiator_class {
	/* finalize option instance */
	void (*option_fini)(const jive_negotiator * self, jive_negotiator_option * option);
	
	/* create empty option (probably invalid) */
	jive_negotiator_option * (*option_create)(const jive_negotiator * self);
	
	/* test two options for equality */
	bool (*option_equals)(const jive_negotiator * self, const jive_negotiator_option * o1, const jive_negotiator_option * o2);
	
	/* specialize option, return true if changed */
	bool (*option_specialize)(const jive_negotiator * self, jive_negotiator_option * option);
	
	/* try to compute intersection; return true if changed, return false if it would be empty (and is therefore unchanged) */
	bool (*option_intersect)(const jive_negotiator * self, jive_negotiator_option * dst, const jive_negotiator_option * src);
	
	/* assign new value to option, return true if changed */
	bool (*option_assign)(const jive_negotiator * self, jive_negotiator_option * dst, const jive_negotiator_option * src);
	
	/* store suitable default options for this type/resource class pair */
	bool (*option_gate_default)(const jive_negotiator * self, jive_negotiator_option * dst, const struct jive_gate * gate);
	
	/* annotate non-gate ports of node */
	void (*annotate_node_proper)(jive_negotiator * self, struct jive_node * node);
	
	/* annotate ports of node */
	void (*annotate_node)(jive_negotiator * self, struct jive_node * node);
	
	/* process region */
	void (*process_region)(jive_negotiator * self, struct jive_region * region);
};

struct jive_negotiator {
	const jive_negotiator_class * class_;
	
	struct jive_graph * graph;
	struct jive_context * context;
	jive_negotiator_input_hash input_map;
	jive_negotiator_output_hash output_map;
	jive_negotiator_node_hash node_map;
	jive_negotiator_gate_hash gate_map;
	
	struct {
		jive_negotiator_connection * first;
		jive_negotiator_connection * last;
	} validated_connections;
	
	struct {
		jive_negotiator_connection * first;
		jive_negotiator_connection * last;
	} invalidated_connections;
	
	struct {
		jive_negotiator_constraint * first;
		jive_negotiator_constraint * last;
	} constraints;
	
	jive_negotiator_option * tmp_option;
};

jive_negotiator_port *
jive_negotiator_port_create(jive_negotiator_constraint * constraint, jive_negotiator_connection * connection, const jive_negotiator_option * option);

/* inheritable initializer for constraint */
void
jive_negotiator_constraint_init_(jive_negotiator_constraint * self, jive_negotiator * negotiator, const jive_negotiator_constraint_class * class_);

/* inheritable finalizer for constraint */
void
jive_negotiator_constraint_fini_(jive_negotiator_constraint * self);

/* inheritable default node annotator */
void
jive_negotiator_annotate_node_(jive_negotiator * self, struct jive_node * node);

/* inheritable default proper node annotator */
void
jive_negotiator_annotate_node_proper_(jive_negotiator * self, struct jive_node * node);

/* inheritable default gate annotator */
bool
jive_negotiator_option_gate_default_(const jive_negotiator * self, jive_negotiator_option * dst, const struct jive_gate * gate);

void
jive_negotiator_process_region_(jive_negotiator * self, struct jive_region * region);

void
jive_negotiator_init_(jive_negotiator * self, const jive_negotiator_class * class_, struct jive_graph * graph);

void
jive_negotiator_fini_(jive_negotiator * self);

void
jive_negotiator_process(jive_negotiator * self);

jive_negotiator_constraint *
jive_negotiator_annotate_identity_node(jive_negotiator * self, struct jive_node * node, const jive_negotiator_option * option);

jive_negotiator_port *
jive_negotiator_map_output(const jive_negotiator * self, struct jive_output * output);

jive_negotiator_port *
jive_negotiator_map_input(const jive_negotiator * self, struct jive_input * input);

#endif