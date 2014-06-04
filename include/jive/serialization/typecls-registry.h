/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_SERIALIZATION_TYPECLS_REGISTRY_H
#define JIVE_SERIALIZATION_TYPECLS_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>

#include <typeinfo>

#include <jive/common.h>

struct jive_context;
struct jive_type;
struct jive_type_class;
struct jive_serialization_driver;
struct jive_token_ostream;
struct jive_token_istream;

typedef struct jive_serialization_typecls jive_serialization_typecls;

typedef void (*jive_typecls_serialize_function_t)(
	const jive_serialization_typecls * self,
	struct jive_serialization_driver * driver,
	const struct jive_type * type,
	struct jive_token_ostream * os);

typedef bool (*jive_typecls_deserialize_function_t)(
	const jive_serialization_typecls * self,
	struct jive_serialization_driver * driver,
	struct jive_token_istream * is,
	struct jive_type ** type);

struct jive_serialization_typecls {
	const char * tag;
	const std::type_info * cls;
	jive_typecls_serialize_function_t serialize;
	jive_typecls_deserialize_function_t deserialize;
	
	struct {
		jive_serialization_typecls * prev, * next;
	} tag_chain;
	struct {
		jive_serialization_typecls * prev, * next;
	} cls_chain;
};

typedef struct jive_serialization_typecls_registry jive_serialization_typecls_registry;

const jive_serialization_typecls_registry *
jive_serialization_typecls_registry_get(void);

JIVE_EXPORTED_INLINE void
jive_serialization_typecls_registry_put(
	const jive_serialization_typecls_registry * reg)
{
}

const jive_serialization_typecls *
jive_serialization_typecls_lookup_by_cls(
	const jive_serialization_typecls_registry * self,
	const std::type_info & cls);

const jive_serialization_typecls *
jive_serialization_typecls_lookup_by_tag(
	const jive_serialization_typecls_registry * self,
	const char * tag);

void
jive_serialization_typecls_register(
	const std::type_info & typecls,
	const char tag[],
	jive_typecls_serialize_function_t serialize,
	jive_typecls_deserialize_function_t deserialize);

#define JIVE_SERIALIZATION_TYPECLS_REGISTER(typecls, name, tag, serialize, deserialize) \
	static void __attribute__((constructor)) register_##name(void)\
	{ \
		jive_serialization_typecls_register(typeid(typecls), tag, serialize, deserialize); \
	} \

#endif
