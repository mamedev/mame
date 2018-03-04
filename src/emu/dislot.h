// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DISLOT_H
#define MAME_EMU_DISLOT_H

//**************************************************************************
//  LEGACY MACROS
//**************************************************************************

#define MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_option, _fixed) MCFG_SLOT_OPTION_RESET slot_options_##_slot_intf(device); MCFG_SLOT_DEFAULT_OPTION(_def_option) MCFG_SLOT_FIXED(_fixed)
#define SLOT_INTERFACE_NAME(name) slot_options_##name
#define SLOT_INTERFACE_START(name) void slot_options_##name(device_t *device) {
#define SLOT_INTERFACE(name,device) MCFG_SLOT_OPTION_ADD(name, device)
#define SLOT_INTERFACE_INTERNAL(name,device) MCFG_SLOT_OPTION_ADD(name, device) MCFG_SLOT_OPTION_SELECTABLE(name, false)
#define SLOT_INTERFACE_END }
#define SLOT_INTERFACE_EXTERN(name) void slot_options_##name(device_t *device)
#define MCFG_DEVICE_CARD_DEFAULT_BIOS(_option, _default_bios) MCFG_SLOT_OPTION_DEFAULT_BIOS(_option, _default_bios)
#define MCFG_DEVICE_CARD_MACHINE_CONFIG(_option, _machine_config_name) MCFG_SLOT_OPTION_MACHINE_CONFIG(_option, _machine_config_name)
#define MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS(_option, _dev_inp_def) MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS(_option, _dev_inp_def)
#define MCFG_DEVICE_CARD_CLOCK(_option, _clock) MCFG_SLOT_OPTION_CLOCK(_option, _clock)


//**************************************************************************
//  MACROS
//**************************************************************************

#define MCFG_SLOT_FIXED(_fixed) \
	dynamic_cast<device_slot_interface &>(*device).set_fixed(_fixed);

#define MCFG_SLOT_DEFAULT_OPTION(_option) \
	dynamic_cast<device_slot_interface &>(*device).set_default_option(_option);

#define MCFG_SLOT_OPTION_RESET \
	dynamic_cast<device_slot_interface &>(*device).option_reset();

#define MCFG_SLOT_OPTION_ADD(_option, _devtype) \
	dynamic_cast<device_slot_interface &>(*device).option_add(_option, _devtype);

#define MCFG_SLOT_OPTION_SELECTABLE(_option, _selectable) \
	dynamic_cast<device_slot_interface &>(*device).set_option_selectable(_option, _selectable);

#define MCFG_SLOT_OPTION_DEFAULT_BIOS(_option, _default_bios) \
	dynamic_cast<device_slot_interface &>(*device).set_option_default_bios(_option, _default_bios);

#define MCFG_SLOT_OPTION_MACHINE_CONFIG(_option, _machine_config_name) \
	dynamic_cast<device_slot_interface &>(*device).set_option_machine_config(_option, _machine_config_name);

#define MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS(_option, _dev_inp_def) \
	dynamic_cast<device_slot_interface &>(*device).set_option_device_input_defaults(_option, DEVICE_INPUT_DEFAULTS_NAME(_dev_inp_def));

#define MCFG_SLOT_OPTION_CLOCK(_option, _clock) \
	dynamic_cast<device_slot_interface &>(*device).set_option_clock(_option, _clock);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_slot_option

class device_slot_option
{
	friend class device_slot_interface;

public:
	device_slot_option(const char *name, const device_type &devtype);

	const char *name() const { return m_name; }
	const device_type &devtype() const { return m_devtype; }
	bool selectable() const { return m_selectable; }
	const char *default_bios() const { return m_default_bios; }
	std::function<void (device_t *)> machine_config() const { return m_machine_config; }
	const input_device_default *input_device_defaults() const { return m_input_device_defaults; }
	u32 clock() const { return m_clock; }

private:
	// internal state
	const char *m_name;
	const device_type &m_devtype;
	bool m_selectable;
	const char *m_default_bios;
	std::function<void (device_t *)> m_machine_config;
	const input_device_default *m_input_device_defaults;
	u32 m_clock;
};


// ======================> get_default_card_software_hook

class get_default_card_software_hook
{
	// goofy "hook" to pass to device_slot_interface::get_default_card_software
public:
	get_default_card_software_hook(const std::string &path, std::function<bool(util::core_file &, std::string&)> &&get_hashfile_extrainfo);

	// accesses the image file to be scrutinized by get_default_card_software(); is
	// nullptr in the case of images loaded by software list
	util::core_file::ptr &image_file() { return m_image_file;  }

	// checks to see if image is of the specified "file type" (in practice, file extension)
	bool is_filetype(const char *candidate_filetype) const { return !core_stricmp(m_file_type.c_str(), candidate_filetype); }

	// extra info from hashfile
	bool hashfile_extrainfo(std::string &extrainfo);

private:
	util::core_file::ptr                                    m_image_file;
	std::string                                             m_file_type;
	std::function<bool(util::core_file &, std::string&)>    m_get_hashfile_extrainfo;
	bool                                                    m_called_get_hashfile_extrainfo;
	bool                                                    m_has_hash_extrainfo;
	std::string                                             m_hash_extrainfo;
};


// ======================> device_slot_interface

class device_slot_interface : public device_interface
{
public:
	// construction/destruction
	device_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_slot_interface();

	void set_fixed(bool fixed) { m_fixed = fixed; }
	void set_default_option(const char *option) { m_default_option = option; }
	void option_reset() { m_options.clear(); }
	void option_add(const char *option, const device_type &devtype);
	void set_option_selectable(const char *option, bool selectable){ config_option(option)->m_selectable = selectable; }
	void set_option_default_bios(const char *option, const char *default_bios) { config_option(option)->m_default_bios = default_bios; }
	void set_option_machine_config(const char *option, std::function<void (device_t *)> machine_config) { config_option(option)->m_machine_config = machine_config; }
	void set_option_device_input_defaults(const char *option, const input_device_default *default_input) { config_option(option)->m_input_device_defaults = default_input; }
	void set_option_clock(const char *option, u32 default_clock) { config_option(option)->m_clock = default_clock; }
	bool fixed() const { return m_fixed; }
	bool has_selectable_options() const;
	const char *default_option() const { return m_default_option; }
	const std::unordered_map<std::string, std::unique_ptr<device_slot_option>> &option_list() const { return m_options; }
	device_slot_option *option(const char *name) const;
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const { return std::string(); }
	device_t *get_card_device() const { return m_card_device; }
	void set_card_device(device_t *dev) { m_card_device = dev; }
	const char *slot_name() const { return device().tag() + 1; }

private:
	// internal state
	std::unordered_map<std::string,std::unique_ptr<device_slot_option>> m_options;
	const char *m_default_option;
	bool m_fixed;
	device_t *m_card_device;

	device_slot_option *config_option(const char *option);
};

// iterator
typedef device_interface_iterator<device_slot_interface> slot_interface_iterator;

// ======================> device_slot_card_interface

class device_slot_card_interface : public device_interface
{
public:
	// construction/destruction
	device_slot_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_slot_card_interface();
};

#endif  /* MAME_EMU_DISLOT_H */
