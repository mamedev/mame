// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    osdmodule.h

    OSD module manangement

*******************************************************************c********/

//#pragma once

#ifndef MAME_OSD_MODULES_OSDMODULE_H
#define MAME_OSD_MODULES_OSDMODULE_H

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
template<class ModuleClass>
osd_module *module_creator()
{
	void *p = osd_malloc(sizeof(ModuleClass));
	return new(p) ModuleClass;
}

class osd_module_manager
{
public:

	static const int MAX_MODULES = 32;

	osd_module_manager();
	~osd_module_manager();

	void register_module(const module_type &mod_type);
	bool type_has_name(const char *type, const char *name) const;

	osd_module *get_module_generic(const char *type, const char *name);

	template<class C>
	C select_module(const char *type, const char *name = "")
	{
		return dynamic_cast<C>(select_module(type, name));
	}

	osd_module *select_module(const char *type, const char *name = "");

	void get_module_names(const char *type, const int max, int *num, const char *names[]) const;

	void init(const osd_options &options);

	void exit();

private:
	int get_module_index(const char *type, const char *name) const;

	osd_module *m_modules[MAX_MODULES];
	osd_module *m_selected[MAX_MODULES];
};

#define MODULE_DEFINITION(mod_id, mod_class) \
	extern const module_type mod_id ;  \
	const module_type mod_id = &module_creator< mod_class >;


#define MODULE_NOT_SUPPORTED(mod_class, mod_type, mod_name) \
	class mod_class : public osd_module { \
	public: \
		mod_class () : osd_module(mod_type, mod_name) { } \
		virtual int init(const osd_options &options) override { return -1; } \
		virtual bool probe() override { return false; } \
	};

#endif  /* MAME_OSD_MODULES_OSDMODULE_H */
