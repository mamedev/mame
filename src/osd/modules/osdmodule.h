// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    osdmodule.h

    OSD module manangement

*******************************************************************c********/

//#pragma once

#ifndef __OSDMODULE_H__
#define __OSDMODULE_H__

#include "osdcore.h"
#include "osdepend.h"

//============================================================
//  TYPE DEFINITIONS
//============================================================

class osd_options;
class osd_module;

// ======================> osd_module

class osd_module
{
public:

	osd_module(const char *type, const char *name)
	: m_name(name), m_type(type)
	{}
	virtual ~osd_module() { }

	const char * name() const { return m_name.c_str(); }
	const char * type() const { return m_type.c_str(); }

	virtual bool probe() { return true; }

	virtual int init(const osd_options &options) = 0;
	virtual void exit() { }

private:
	std::string     m_name;
	std::string     m_type;
};

// a module_type is simply a pointer to its alloc function
typedef osd_module *(*module_type)();

// this template function creates a stub which constructs a module
template<class _ModuleClass>
osd_module *module_creator()
{
	void *p = osd_malloc(sizeof(_ModuleClass));
	return new(p) _ModuleClass;
}

class osd_module_manager
{
public:

	static const int MAX_MODULES = 32;

	osd_module_manager();
	~osd_module_manager();

	void register_module(const module_type &mod_type);
	bool type_has_name(const char *type, const char *name);

	osd_module *get_module_generic(const char *type, const char *name);

	template<class C>
	C select_module(const char *type, const char *name = "")
	{
		return dynamic_cast<C>(select_module(type, name));
	}

	osd_module *select_module(const char *type, const char *name = "");

	void get_module_names(const char *type, const int max, int *num, const char *names[]);

	void init(const osd_options &options);

	void exit();

private:
	int get_module_index(const char *type, const char *name);

	osd_module *m_modules[MAX_MODULES];
	osd_module *m_selected[MAX_MODULES];
};

#define MODULE_DEFINITION(_id, _class) \
	extern const module_type _id ;  \
	const module_type _id = &module_creator< _class >;


#define MODULE_NOT_SUPPORTED(_mod, _type, _name) \
	class _mod : public osd_module { \
	public: \
		_mod () : osd_module(_type, _name) { } \
		virtual int init(const osd_options &options) override { return -1; } \
		virtual bool probe() override { return false; } \
	};

#endif  /* __OSDMODULE_H__ */
