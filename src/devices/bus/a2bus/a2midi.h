// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2midi.h

    Apple II 6850 MIDI card, as made by Passport, Yamaha, and others.

*********************************************************************/

#ifndef __A2BUS_MIDI__
#define __A2BUS_MIDI__

#include "a2bus.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_midi_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2bus_midi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );
	DECLARE_WRITE_LINE_MEMBER( ptm_irq_w );
	DECLARE_WRITE_LINE_MEMBER( write_acia_clock );

protected:
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);

	required_device<ptm6840_device> m_ptm;
	required_device<acia6850_device> m_acia;

private:
	bool m_acia_irq, m_ptm_irq;
};

// device type definition
extern const device_type A2BUS_MIDI;

#endif  /* __A2BUS_MIDI__ */
