// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DISLOT_H__
#define __DISLOT_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
// ======================> device_slot_interface

struct slot_interface
{
	const char *    name;
	const device_type & devtype;
	bool            internal;
};

#define MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_card, _fixed) \
	device_slot_interface::static_set_slot_info(*device, SLOT_INTERFACE_NAME(_slot_intf), _def_card, _fixed);

#define MCFG_DEVICE_CARD_DEFAULT_BIOS(card, _default_bios) \
	device_slot_interface::static_set_card_default_bios(*device, card, _default_bios);

#define MCFG_DEVICE_CARD_MACHINE_CONFIG(card, _machine_config_name) \
	device_slot_interface::static_set_card_machine_config(*device, card, MACHINE_CONFIG_NAME(_machine_config_name));

#define MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS(card, _dev_inp_def) \
	device_slot_interface::static_set_card_device_input_defaults(*device, card, DEVICE_INPUT_DEFAULTS_NAME(_dev_inp_def));

#define MCFG_DEVICE_CARD_CONFIG(card, _config) \
	device_slot_interface::static_set_card_config(*device, card, _config);

#define MCFG_DEVICE_CARD_CLOCK(card, _clock) \
	device_slot_interface::static_set_card_clock(*device, card, _clock);

#define SLOT_INTERFACE_NAME(name)   slot_interface_##name

#define SLOT_INTERFACE_START(name)                              \
	const slot_interface slot_interface_##name[] =              \
	{
#define SLOT_INTERFACE(tag,device) \
	{ tag, device, false },
#define SLOT_INTERFACE_INTERNAL(tag,device) \
	{ tag, device, true },
#define SLOT_INTERFACE_END \
		{ NULL, NULL, false }                           \
	};

#define SLOT_INTERFACE_EXTERN(name) extern const slot_interface slot_interface_##name[]

class device_card_options
{
	friend class device_slot_interface;
	friend class simple_list<device_card_options>;

private:
	const char *m_default_bios;
	machine_config_constructor m_machine_config;
	const input_device_default *m_input_device_defaults;
	const void *m_config;
	UINT32 m_clock;

	// internal state
	device_card_options *        m_next;             // link to the next reference
};

class device_slot_interface : public device_interface
{
public:
	// construction/destruction
	device_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_slot_interface();

	static void static_set_slot_info(device_t &device, const slot_interface *slots_info, const char *default_card,bool fixed);
	static device_card_options *static_alloc_card_options(device_t &device, const char *card);
	static void static_set_card_default_bios(device_t &device, const char *card, const char *default_bios);
	static void static_set_card_machine_config(device_t &device, const char *card, const machine_config_constructor machine_config);
	static void static_set_card_device_input_defaults(device_t &device, const char *card, const input_device_default *default_input);
	static void static_set_card_config(device_t &device, const char *card, const void *config);
	static void static_set_card_clock(device_t &device, const char *card, UINT32 default_clock);
	const slot_interface* get_slot_interfaces() const { return m_slot_interfaces; };
	const char * get_default_card() const { return m_default_card; };
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options) { return NULL; };
	const char *card_default_bios(const char *card) const;
	const machine_config_constructor card_machine_config(const char *card) const;
	const input_device_default *card_input_device_defaults(const char *card) const;
	const void *card_config(const char *card) const;
	const UINT32 card_clock(const char *card) const;
	const bool fixed() const { return m_fixed; }
	const bool all_internal() const;
	bool is_internal_option(const char *option) const;
	device_t* get_card_device();
protected:
	const slot_interface *m_slot_interfaces;
	const char *m_default_card;
	tagged_list<device_card_options> m_card_options;
	bool m_fixed;
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

#endif  /* __DISLOT_H__ */
