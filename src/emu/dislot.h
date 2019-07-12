// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DISLOT_H
#define MAME_EMU_DISLOT_H


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

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
	class slot_option
	{
	public:
		slot_option(char const *name, device_type const &devtype, bool selectable);

		char const *name() const { return m_name; }
		device_type const &devtype() const { return m_devtype; }
		bool selectable() const { return m_selectable; }
		char const *default_bios() const { return m_default_bios; }
		std::function<void (device_t *)> const &machine_config() const { return m_machine_config; }
		input_device_default const *input_device_defaults() const { return m_input_device_defaults; }
		u32 clock() const { return m_clock; }

		slot_option &default_bios(char const *default_bios) { m_default_bios = default_bios; return *this; }
		template <typename Object> slot_option &machine_config(Object &&cb) { m_machine_config = std::forward<Object>(cb); return *this; }
		slot_option &input_device_defaults(input_device_default const *dev_inp_def) { m_input_device_defaults = dev_inp_def; return *this; }
		slot_option &clock(u32 clock) { m_clock = clock; return *this; }
		slot_option &clock(XTAL clock) { clock.validate(std::string("Configuring slot option ") + m_name); m_clock = clock.value(); return *this; }

	private:
		// internal state
		char const *const m_name;
		device_type const m_devtype;
		bool const m_selectable;
		char const *m_default_bios;
		std::function<void (device_t *)> m_machine_config;
		input_device_default const *m_input_device_defaults;
		u32 m_clock;
	};

	// construction/destruction
	device_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_slot_interface();

	void set_fixed(bool fixed) { m_fixed = fixed; }
	void set_default_option(const char *option) { m_default_option = option; }
	void option_reset() { m_options.clear(); }
	slot_option &option_add(const char *option, const device_type &devtype);
	slot_option &option_add_internal(const char *option, const device_type &devtype);
	void set_option_default_bios(const char *option, const char *default_bios) { config_option(option)->default_bios(default_bios); }
	template <typename T> void set_option_machine_config(const char *option, T &&machine_config) { config_option(option)->machine_config(std::forward<T>(machine_config)); }
	void set_option_device_input_defaults(const char *option, const input_device_default *default_input) { config_option(option)->input_device_defaults(default_input); }
	bool fixed() const { return m_fixed; }
	bool has_selectable_options() const;
	const char *default_option() const { return m_default_option; }
	const std::unordered_map<std::string, std::unique_ptr<slot_option>> &option_list() const { return m_options; }
	const slot_option *option(const char *name) const;
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const { return std::string(); }
	device_t *get_card_device() const { return m_card_device; }
	void set_card_device(device_t *dev) { m_card_device = dev; }
	const char *slot_name() const { return device().tag() + 1; }
	slot_option &option_set(const char *tag, const device_type &devtype) { m_default_option = tag; m_fixed = true; return option_add_internal(tag, devtype); }

protected:
	void set_default_clock(u32 clock) { m_default_clock = clock; }

private:
	// internal state
	std::unordered_map<std::string,std::unique_ptr<slot_option>> m_options;
	u32 m_default_clock;
	const char *m_default_option;
	bool m_fixed;
	device_t *m_card_device;

	slot_option *config_option(const char *option);
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
