/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_RESOURCE_H
#define JIVE_VSDG_RESOURCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <jive/common.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace jive {
namespace base {
	class type;
}
	class gate;
	class resource_class;
	class resource_name;
}

typedef struct jive_resource_class_class jive_resource_class_class;

typedef enum {
	jive_resource_class_priority_invalid = 0,
	jive_resource_class_priority_control = 1,
	jive_resource_class_priority_reg_implicit = 2,
	jive_resource_class_priority_mem_unique = 3,
	jive_resource_class_priority_reg_high = 4,
	jive_resource_class_priority_reg_low = 5,
	jive_resource_class_priority_mem_generic = 6,
	jive_resource_class_priority_lowest = 7
} jive_resource_class_priority;

namespace jive {

class resource_class_demotion final {
public:
	inline
	resource_class_demotion(
		const resource_class * target,
		const std::vector<const resource_class*> & path)
	: target_(target)
	, path_(path)
	{}

	inline const resource_class *
	target() const noexcept
	{
		return target_;
	}

	inline const std::vector<const resource_class*>
	path() const noexcept
	{
		return path_;
	}

private:
	const resource_class * target_;
	std::vector<const resource_class*> path_;
};

class resource_class {
public:
	virtual
	~resource_class();

	inline
	resource_class(
		const jive_resource_class_class * cls,
		const std::string & name,
		const std::unordered_set<const jive::resource_name*> resources,
		const jive::resource_class * parent,
		jive_resource_class_priority pr,
		const std::vector<resource_class_demotion> & demotions,
		const jive::base::type * type)
	: class_(cls)
	, priority(pr)
	, depth_(parent ? parent->depth()+1 : 0)
	, name_(name)
	, type_(type)
	, parent_(parent)
	, resources_(resources)
	, demotions_(demotions)
	{}

	inline size_t
	depth() const noexcept
	{
		return depth_;
	}

	inline const std::string &
	name() const noexcept
	{
		return name_;
	}

	inline const jive::base::type &
	type() const noexcept
	{
		JIVE_DEBUG_ASSERT(type_ != nullptr);
		return *type_;
	}

	inline const jive::resource_class *
	parent() const noexcept
	{
		return parent_;
	}

	inline size_t
	nresources() const noexcept
	{
		return resources_.size();
	}

	inline const std::unordered_set<const jive::resource_name*> &
	resources() const noexcept
	{
		return resources_;
	}

	inline const std::vector<resource_class_demotion> &
	demotions() const noexcept
	{
		return demotions_;
	}

	const jive_resource_class_class * class_;
	
	/** \brief Priority for register allocator */
	jive_resource_class_priority priority;
	
private:
	/** \brief Number of steps from root resource class */
	size_t depth_;
	std::string name_;

	/** \brief Port and gate type corresponding to this resource */
	const jive::base::type * type_;

	/** \brief Parent resource class */
	const jive::resource_class * parent_;

	/** \brief Available resources */
	std::unordered_set<const jive::resource_name*> resources_;

	/** \brief Paths for "demoting" this resource to a different one */
	std::vector<resource_class_demotion> demotions_;
};

}

struct jive_resource_class_class {
	const jive_resource_class_class * parent;
	const char * name;
	bool is_abstract;
};

const jive::resource_class *
jive_resource_class_union(const jive::resource_class * self, const jive::resource_class * other);

const jive::resource_class *
jive_resource_class_intersection(const jive::resource_class * self,
	const jive::resource_class * other);

static inline bool
jive_resource_class_isinstance(const jive::resource_class * self,
	const jive_resource_class_class * cls)
{
	const jive_resource_class_class * tmp = self->class_;
	while (tmp) {
		if (tmp == cls)
			return true;
		tmp = tmp->parent;
	}
	return false;
}

static inline bool
jive_resource_class_is_abstract(const jive::resource_class * self)
{
	return self->class_->is_abstract;
}

/** \brief Find largest resource class of same general type containing this class */
const jive::resource_class *
jive_resource_class_relax(const jive::resource_class * self);

extern const jive_resource_class_class JIVE_ABSTRACT_RESOURCE;
extern const jive::resource_class jive_root_resource_class;

namespace jive {

class resource_name {
public:
	virtual
	~resource_name();

	inline
	resource_name(const std::string & name, const jive::resource_class * rescls)
	: resource_class(rescls)
	, name_(name)
	{}

	inline const std::string &
	name() const noexcept
	{
		return name_;
	}

	const jive::resource_class * resource_class;
private:
	std::string name_;
};

}

#endif
