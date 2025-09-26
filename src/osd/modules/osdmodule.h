// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    osdmodule.h

    OSD module management

***************************************************************************/
#ifndef MAME_OSD_MODULES_OSDMODULE_H
#define MAME_OSD_MODULES_OSDMODULE_H

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


//============================================================
//  TYPE DEFINITIONS
//============================================================

class osd_interface;
class osd_options;


//============================================================
//  TYPE DEFINITIONS
//============================================================

class osd_module
{
public:
	virtual ~osd_module() { }

	std::string const &name() const { return m_name; }
	std::string const &type() const { return m_type; }

	virtual bool probe() { return true; }

	virtual int init(osd_interface &osd, const osd_options &options) = 0;
	virtual void exit() { }

protected:
	osd_module(const char *type, const char *name) : m_name(name), m_type(type) { }
	osd_module(osd_module const &) = delete;

private:
	std::string const m_name;
	std::string const m_type;
};


// a module_type is simply a pointer to its alloc function
typedef std::unique_ptr<osd_module> (*module_type)();


// this template function creates a stub which constructs a module
template <class ModuleClass>
std::unique_ptr<osd_module> module_creator()
{
	return std::unique_ptr<osd_module>(new ModuleClass);
}


class osd_module_manager
{
public:
	osd_module_manager();
	~osd_module_manager();

	void register_module(const module_type &mod_type);
	bool type_has_name(const char *type, const char *name) const;

	osd_module *get_module_generic(const char *type, const char *name);

	template<class C>
	C &select_module(osd_interface &osd, const osd_options &options, const char *type, const char *name = "")
	{
		return dynamic_cast<C &>(select_module(osd, options, type, name));
	}

	osd_module &select_module(osd_interface &osd, const osd_options &options, const char *type, const char *name = "");

	std::vector<std::string_view> get_module_names(const char *type) const;

	void exit();

private:
	int get_module_index(const char *type, const char *name) const;

	std::vector<std::unique_ptr<osd_module> > m_modules;
	std::vector<std::reference_wrapper<osd_module> > m_selected;
};


#define MODULE_DEFINITION(mod_id, mod_class) \
		extern const module_type mod_id ;  \
		const module_type mod_id = &module_creator<mod_class>;


#define MODULE_NOT_SUPPORTED(mod_class, mod_type, mod_name) \
		class mod_class : public osd_module \
		{ \
		public: \
			mod_class () : osd_module(mod_type, mod_name) { } \
			virtual ~mod_class() { } \
			virtual int init(osd_interface &osd, const osd_options &options) override { return -1; } \
			virtual bool probe() override { return false; } \
		};

#endif  /* MAME_OSD_MODULES_OSDMODULE_H */
