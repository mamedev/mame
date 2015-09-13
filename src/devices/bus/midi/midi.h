// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef __BUS_MIDI_H__
#define __BUS_MIDI_H__

#include "emu.h"

#define MCFG_MIDI_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, MIDI_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_MIDI_RX_HANDLER(_devcb) \
	devcb = &midi_port_device::set_rx_handler(*device, DEVCB_##_devcb);

class device_midi_port_interface;

class midi_port_device : public device_t,
	public device_slot_interface
{
	friend class device_midi_port_interface;

public:
	midi_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~midi_port_device();

	// static configuration helpers
	template<class _Object> static devcb_base &set_rx_handler(device_t &device, _Object object) { return downcast<midi_port_device &>(device).m_rxd_handler.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( write_txd );

	DECLARE_READ_LINE_MEMBER( rx_r ) { return m_rxd; }

protected:
	virtual void device_start();
	virtual void device_config_complete();

	int m_rxd;

	devcb_write_line m_rxd_handler;

private:
	device_midi_port_interface *m_dev;
};

class device_midi_port_interface : public device_slot_card_interface
{
	friend class midi_port_device;

public:
	device_midi_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_midi_port_interface();

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) {}
	DECLARE_WRITE_LINE_MEMBER( output_rxd ) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }

protected:
	midi_port_device *m_port;
};

extern const device_type MIDI_PORT;

SLOT_INTERFACE_EXTERN(midiin_slot);
SLOT_INTERFACE_EXTERN(midiout_slot);

#endif
