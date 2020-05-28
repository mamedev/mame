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


/// \brief Base class for configurable slot devices
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

	virtual ~device_slot_interface();

	/// \brief Set whether slot is fixed
	///
	/// Allows a slot to be configured as fixed.  Fixed slots can only
	/// be configured programmatically.  Options for fixed slots are not
	/// user-selectable.  By default, slots are not fixed.
	/// \param [in] fixed True to mark the slot as fixed, or false to
	///   mark it user-configurable.
	/// \sa fixed
	void set_fixed(bool fixed) { m_fixed = fixed; }

	/// \brief Set the default option
	///
	/// Set the default option the slot.  The corresponding card device
	/// will be loaded if no other option is selected by the user or
	/// programmatically.
	/// \param [in] option The name of the default option.  This must be
	///   correspond to an option added using #option_add or
	///   #option_add_internal, or be nullptr to not load any card
	///   device by default.  The string is not copied and must remain
	///   valid for the lifetime of the device (or until another default
	///   option is configured).
	/// \sa default_option
	void set_default_option(const char *option) { m_default_option = option; }

	/// \brief Clear options
	///
	/// This removes all previously added options.
	void option_reset() { m_options.clear(); }

	/// \brief Add a user-selectable option
	///
	/// Adds an option that may be selected by the user via the command
	/// line or other configureation methods.
	/// \param [in] option The name used to select the option.  This
	///   will also be used as the tag when instantiating the card.  It
	///   must be a valid device tag, and should be terse but
	///   descriptive.  The string is copied when the option is added.
	/// \param [in] devtype Device type of the option.  The device type
	///   description is used in the user interface.
	/// \return A reference to the added option for additional
	///   configuration.
	slot_option &option_add(char const *option, const device_type &devtype);

	/// \brief Add an internal option
	///
	/// Adds an option that may only be selected programmatically.  This
	/// is most often used for options used for loading software.
	/// \param [in] option The name used to select the option.  This
	///   will also be used as the tag when instantiating the card.  It
	///   must be a valid device tag.  The string is copied when the
	///   option is added.
	/// \param [in] devtype Device type of the option.  The device type
	///   description is used in the user interface.
	/// \return A reference to the added option for additional
	///   configuration.
	slot_option &option_add_internal(const char *option, const device_type &devtype);

	void set_option_default_bios(const char *option, const char *default_bios) { config_option(option)->default_bios(default_bios); }
	template <typename T> void set_option_machine_config(const char *option, T &&machine_config) { config_option(option)->machine_config(std::forward<T>(machine_config)); }
	void set_option_device_input_defaults(const char *option, const input_device_default *default_input) { config_option(option)->input_device_defaults(default_input); }

	/// \brief Returns whether the slot is fixed
	///
	/// Fixed slots can only be configured programmatically.  Slots are
	/// not fixed by default.
	/// \return True if the slot is fixed, or false if it is
	///   user-configurable.
	/// \sa set_fixed
	bool fixed() const { return m_fixed; }

	/// \brief Returns true if the slot has user-selectable options
	///
	/// Returns true if the slot is not marked as fixed and has at least
	/// one user-selectable option (added using #option_add rather than
	/// #option_add_internal).  Returns false if the slot is marked as
	/// fixed, all options are internal, or there are no options.
	/// \return True if the slot has user-selectable options, or false
	///   otherwise.
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
	device_slot_interface(machine_config const &mconfig, device_t &device);
	virtual void interface_validity_check(validity_checker &valid) const override ATTR_COLD;
	void set_default_clock(u32 clock) { m_default_clock = clock; }

private:
	// internal state
	std::unordered_map<std::string, std::unique_ptr<slot_option>> m_options;
	u32 m_default_clock;
	char const *m_default_option;
	bool m_fixed;
	device_t *m_card_device;

	slot_option *config_option(char const *option);
};


/// \brief Base class for slots that accept a single card interface
///
/// Performs basic validity checks to ensure the configured card (if
/// any) implements the required interface.
template <typename Card>
class device_single_card_slot_interface : public device_slot_interface
{
public:
	Card *get_card_device() const { return dynamic_cast<Card *>(device_slot_interface::get_card_device()); }

protected:
	device_single_card_slot_interface(machine_config const &mconfig, device_t &device) :
		device_slot_interface(mconfig, device)
	{
	}

	virtual void interface_validity_check(validity_checker &valid) const override ATTR_COLD
	{
		device_slot_interface::interface_validity_check(valid);
		device_t *const card(device_slot_interface::get_card_device());
		if (card && !dynamic_cast<Card *>(card))
		{
			osd_printf_error(
					"Card device %s (%s) does not implement %s\n",
					card->tag(),
					card->name(),
					typeid(Card).name());
		}
	}

	virtual void interface_pre_start() override ATTR_COLD
	{
		device_slot_interface::interface_pre_start();
		device_t *const card(device_slot_interface::get_card_device());
		if (card && !dynamic_cast<Card *>(card))
		{
			throw emu_fatalerror(
					"slot '%s' card device %s (%s) does not implement %s\n",
					device().tag(),
					card->tag(),
					card->name(),
					typeid(Card).name());
		}
	}
};

typedef device_interface_iterator<device_slot_interface> slot_interface_iterator;

#endif // MAME_EMU_DISLOT_H
