#include <jive/regalloc/notifiers.h>

#include <jive/util/list.h>

#define MAKE_NOTIFIER(name, NAME, parameters, arguments) \
struct jive_##name##_notifier { \
	jive_notifier base; \
	jive_##name##_notifier_function function; \
	void * closure; \
	jive_##name##_notifier_slot * slot; \
	jive_context * context; \
	struct { \
		jive_##name##_notifier * prev; \
		jive_##name##_notifier * next; \
	} notifier_list; \
}; \
 \
static inline void \
jive_##name##_notifier_unlink(jive_##name##_notifier * self) \
{ \
	if (!self->slot) return; \
	 \
	JIVE_LIST_REMOVE(self->slot->notifiers, self, notifier_list); \
	self->slot = 0; \
} \
 \
static void \
_jive_##name##_notifier_disconnect(jive_notifier * _self) \
{ \
	jive_##name##_notifier * self = (jive_##name##_notifier *) _self; \
	jive_##name##_notifier_unlink(self); \
	jive_context_free(self->context, self); \
} \
 \
static const jive_notifier_class JIVE_##NAME##_NOTIFIER = { \
	.disconnect = _jive_##name##_notifier_disconnect \
}; \
 \
void \
jive_##name##_notifier_slot_fini(jive_##name##_notifier_slot * self) \
{ \
	while(self->notifiers.first) jive_##name##_notifier_unlink(self->notifiers.first); \
} \
 \
jive_notifier * \
jive_##name##_notifier_slot_connect(jive_##name##_notifier_slot * self, jive_##name##_notifier_function function, void * closure) \
{ \
	jive_##name##_notifier * notifier = jive_context_malloc(self->context, sizeof(*notifier)); \
	notifier->base.class_ = &JIVE_##NAME##_NOTIFIER; \
	notifier->slot = self; \
	notifier->context = self->context; \
	notifier->function = function; \
	notifier->closure = closure; \
	JIVE_LIST_PUSH_BACK(self->notifiers, notifier, notifier_list); \
	 \
	return &notifier->base; \
} \
 \
void \
jive_##name##_notifier_slot_call(const jive_##name##_notifier_slot * self, parameters) \
{ \
	jive_##name##_notifier * notifier; \
	JIVE_LIST_ITERATE(self->notifiers, notifier, notifier_list) \
		notifier->function(notifier->closure, arguments); \
} \
 \

#define WRAP(args...) args

MAKE_NOTIFIER(shaped_region_ssavar, SHAPED_REGION_SSAVAR, WRAP(struct jive_shaped_region * shaped_region, struct jive_shaped_ssavar * shaped_ssavar), WRAP(shaped_region, shaped_ssavar))