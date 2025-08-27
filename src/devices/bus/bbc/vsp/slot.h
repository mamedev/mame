// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Micro Voice Synthesis Processor slot emulation

**********************************************************************/

#ifndef MAME_BUS_BBC_VSP_SLOT_H
#define MAME_BUS_BBC_VSP_SLOT_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_vsp_slot_device

class device_bbc_vsp_interface;

class bbc_vsp_slot_device : public device_t, public device_single_card_slot_interface<device_bbc_vsp_interface>, public device_mixer_interface
{
public:
	// construction/destruction
	template <typename T>
	bbc_vsp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: bbc_vsp_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_vsp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	template <class Object> void set_vsm_region(Object &&tag) { m_vsm.set_tag(std::forward<Object>(tag)); }

	virtual void combined_rsq_wsq_w(uint8_t data);
	virtual uint8_t read();
	virtual void write(uint8_t data);

	int readyq_r();
	int intq_r();

	memory_region &vsm() const { return *m_vsm; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	required_memory_region m_vsm;

	device_bbc_vsp_interface *m_card;
};


// ======================> device_bbc_vsp_interface

class device_bbc_vsp_interface : public device_interface
{
public:
	virtual void combined_rsq_wsq_w(uint8_t data) { };
	virtual uint8_t read() { return 0x00; }
	virtual void write(uint8_t data) { }

	virtual int readyq_r() { return 1; }
	virtual int intq_r() { return 1; }

protected:
	device_bbc_vsp_interface(const machine_config &mconfig, device_t &device);

	bbc_vsp_slot_device *const m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_VSP_SLOT, bbc_vsp_slot_device)

void bbc_vsp_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_VSP_SLOT_H
