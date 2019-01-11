// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Chips & Technologies CS8221 chipset

    a.k.a. NEW ENHANCED AT (NEAT)

    Consists of four individual chips:

    * 82C211 - CPU/Bus controller
    * 82C212 - Page/Interleave and EMS Memory controller
    * 82C215 - Data/Address buffer
    * 82C206 - Integrated Peripherals Controller(IPC)

***************************************************************************/

#ifndef MAME_MACHINE_CS8221_H
#define MAME_MACHINE_CS8221_H

#pragma once

class cs8221_device : public device_t
{
public:
	// construction/destruction
	cs8221_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const char *cputag, const char *isatag, const char *biostag)
		: cs8221_device(mconfig, tag, owner, clock)
	{
		set_cputag(cputag);
		set_isatag(isatag);
		set_biostag(biostag);
	}

	cs8221_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	void set_cputag(const char *tag) { m_cputag = tag; }
	void set_isatag(const char *tag) { m_isatag = tag; }
	void set_biostag(const char *tag) { m_biostag = tag; }

	void map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	// internal state
	//address_space *m_space;
	//uint8_t *m_isa;
	//uint8_t *m_bios;
	//uint8_t *m_ram;

	// address selection
	uint8_t m_address;
	bool m_address_valid;

	const char *m_cputag;
	const char *m_isatag;
	const char *m_biostag;

	uint8_t m_registers[0x10];

	DECLARE_WRITE8_MEMBER( address_w );
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );
};

DECLARE_DEVICE_TYPE(CS8221, cs8221_device)

#endif // MAME_MACHINE_CS8221_H
