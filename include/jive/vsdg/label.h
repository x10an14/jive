#ifndef JIVE_VSDG_LABEL_H
#define JIVE_VSDG_LABEL_H

#include <stdbool.h>
#include <stdint.h>

#include <jive/util/hash.h>
#include <jive/vsdg/node.h>

struct jive_context;
struct jive_graph;
struct jive_node;
struct jive_output;
struct jive_region;

struct jive_seq_graph;
struct jive_seq_node;

typedef uint64_t jive_addr;

typedef struct jive_label_class jive_label_class;
typedef struct jive_label jive_label;
typedef struct jive_label_internal_class jive_label_internal_class;
typedef struct jive_label_internal jive_label_internal;
typedef struct jive_label_node jive_label_node;
typedef struct jive_label_region jive_label_region;
typedef struct jive_label_external jive_label_external;

struct jive_label_class {
	void (*fini)(jive_label * self);
	jive_addr (*get_address)(const jive_label * self, const struct jive_seq_node * for_node);
	const char * (*get_asmname)(const jive_label * self);
};

typedef enum {
	jive_label_flags_none = 0,
	jive_label_flags_global = 1, /* whether label is supposed to be "globally visible" */
	jive_label_flags_external = 2, /* whether label must be resolved outside the current graph */
} jive_label_flags;

struct jive_label {
	const jive_label_class * class_;
	
	jive_label_flags flags;
};

JIVE_EXPORTED_INLINE void
jive_label_fini(jive_label * self)
{
	self->class_->fini(self);
}

JIVE_EXPORTED_INLINE jive_addr
jive_label_get_address(const jive_label * self, const struct jive_seq_node * for_node)
{
	return self->class_->get_address(self, for_node);
}

JIVE_EXPORTED_INLINE const char *
jive_label_get_asmname(const jive_label * self)
{
	return self->class_->get_asmname(self);
}

struct jive_label_internal_class {
	jive_label_class base;
	struct jive_seq_node * (*get_attach_node)(const jive_label_internal * self, const struct jive_seq_graph * seq_graph);
};

struct jive_label_internal {
	jive_label base;
	struct jive_graph * graph;
	struct {
		jive_label_internal * prev;
		jive_label_internal * next;
	} graph_label_list;
	char * asmname;
};

JIVE_EXPORTED_INLINE jive_addr
jive_label_internal_get_address(const jive_label_internal * self, const struct jive_seq_node * for_node)
{
	return jive_label_get_address(&self->base, for_node);
}

JIVE_EXPORTED_INLINE const char *
jive_label_internal_get_asmname(const jive_label_internal * self)
{
	return jive_label_get_asmname(&self->base);
}

JIVE_EXPORTED_INLINE struct jive_seq_node *
jive_label_internal_get_attach_node(const jive_label_internal * self, const struct jive_seq_graph * seq_graph)
{
	const jive_label_internal_class * cls = (const jive_label_internal_class *) self->base.class_;
	return cls->get_attach_node(self, seq_graph);
}

struct jive_label_node {
	jive_label_internal base;
	struct jive_node * node;
};

struct jive_label_region {
	jive_label_internal base;
	struct jive_region * region;
};

struct jive_label_external {
	jive_label base;
	struct jive_context * context;
	char * asmname;
	jive_addr address;
};

/**
	\brief Special label marking position of "current" instruction
*/
extern const jive_label jive_label_current;

extern const jive_label_class JIVE_LABEL_CURRENT_;

/**
	\brief Special label marking offset from frame pointer
*/
extern const jive_label jive_label_fpoffset;

extern const jive_label_class JIVE_LABEL_FPOFFSET_;

/**
	\brief Special label marking offset from stack pointer
*/
extern const jive_label jive_label_spoffset;

extern const jive_label_class JIVE_LABEL_SPOFFSET_;

/**
	\brief Label where node is sequenced
*/
jive_label *
jive_label_node_create(struct jive_node * node);

extern const jive_label_internal_class JIVE_LABEL_NODE_;
#define JIVE_LABEL_NODE (JIVE_LABEL_NODE_.base)

/**
	\brief Label where start of region is sequenced
*/
jive_label *
jive_label_region_start_create(struct jive_region * region);

/**
	\brief Exported label where start of region is sequenced
*/
jive_label *
jive_label_region_start_create_exported(struct jive_region * region, const char * name);

extern const jive_label_internal_class JIVE_LABEL_REGION_START;
#define JIVE_LABEL_REGION_START (JIVE_LABEL_REGION_START_.base)

/**
	\brief Label where end of region is sequenced
*/
jive_label *
jive_label_region_end_create(struct jive_region * region);

extern const jive_label_internal_class JIVE_LABEL_REGION_END;
#define JIVE_LABEL_REGION_END (JIVE_LABEL_REGION_END_.base)

/**
	\brief Exported label where end of region is sequenced
*/
jive_label *
jive_label_region_end_create_exported(struct jive_region * region, const char * name);

/**
	\brief Initialize label external to the graph from which it is referenced
*/
void
jive_label_external_init(jive_label_external * self, struct jive_context * context, const char * name, jive_addr address);

/**
	\brief Finalize label external to the graph from which it is referenced
*/
void
jive_label_external_fini(jive_label_external * self);

/* label_to_address node */

extern const jive_node_class JIVE_LABEL_TO_ADDRESS_NODE;

typedef struct jive_label_to_address_node jive_label_to_address_node;
typedef struct jive_label_to_address_node_attrs jive_label_to_address_node_attrs;

struct jive_label_to_address_node_attrs {
	jive_node_attrs base;
	const jive_label * label;
};

struct jive_label_to_address_node {
	jive_node base;
	jive_label_to_address_node_attrs attrs;
};

jive_node *
jive_label_to_address_node_create(struct jive_graph * graph, const jive_label * label);

jive_output *
jive_label_to_address_create(struct jive_graph * graph, const jive_label * label);

JIVE_EXPORTED_INLINE jive_label_to_address_node *
jive_label_to_address_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_LABEL_TO_ADDRESS_NODE)
		return (jive_label_to_address_node *) node;
	else
		return 0;
}

/* label_to_bitstring node */

extern const jive_node_class JIVE_LABEL_TO_BITSTRING_NODE;

typedef struct jive_label_to_bitstring_node jive_label_to_bitstring_node;
typedef struct jive_label_to_bitstring_node_attrs jive_label_to_bitstring_node_attrs;

struct jive_label_to_bitstring_node_attrs {
	jive_node_attrs base;
	const jive_label * label;
	size_t nbits;
};

struct jive_label_to_bitstring_node {
	jive_node base;
	jive_label_to_bitstring_node_attrs attrs;
};

jive_node *
jive_label_to_bitstring_node_create(struct jive_graph * graph, const jive_label * label, size_t nbits);

jive_output *
jive_label_to_bitstring_create(struct jive_graph * graph, const jive_label * label, size_t nbits);

JIVE_EXPORTED_INLINE jive_label_to_bitstring_node *
jive_label_to_bitstring_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_LABEL_TO_BITSTRING_NODE)
		return (jive_label_to_bitstring_node *) node;
	else
		return 0;
}

#endif
