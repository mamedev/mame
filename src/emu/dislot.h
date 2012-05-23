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
	const char *	name;
	const device_type & devtype;
	bool			internal;
};

#define MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, _fixed) \
	device_slot_interface::static_set_slot_info(*device, SLOT_INTERFACE_NAME(_slot_intf), _def_slot, DEVICE_INPUT_DEFAULTS_NAME(_def_inp), _fixed);

#define SLOT_INTERFACE_NAME(name)	slot_interface_##name

#define SLOT_INTERFACE_START(name)								\
	const slot_interface slot_interface_##name[] =				\
	{															\

#define SLOT_INTERFACE(tag,device) \
	{ tag, device, false }, \

#define SLOT_INTERFACE_INTERNAL(tag,device) \
	{ tag, device, true }, \

#define SLOT_INTERFACE_END \
		{ NULL, NULL, false }							\
	};

#define SLOT_INTERFACE_EXTERN(name) extern const slot_interface slot_interface_##name[]

class device_slot_card_interface;

class device_slot_interface : public device_interface
{
public:
	// construction/destruction
	device_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_slot_interface();

	static void static_set_slot_info(device_t &device, const slot_interface *slots_info, const char *default_card,const input_device_default *default_input, bool fixed);
	const slot_interface* get_slot_interfaces() const { return m_slot_interfaces; };
	const char * get_default_card(const machine_config &config, emu_options &options) const { return m_default_card; };
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options) { return NULL; };
	const input_device_default *input_ports_defaults() const { return m_input_defaults; }
	const bool fixed() const { return m_fixed; }
	device_t* get_card_device();
protected:
	const char *m_default_card;
	const input_device_default *m_input_defaults;
	const slot_interface *m_slot_interfaces;
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
