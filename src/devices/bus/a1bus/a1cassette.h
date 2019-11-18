// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a1cassette.h

    Apple I Cassette Interface

*********************************************************************/

#ifndef MAME_BUS_A1BUS_A1CASSETTE_H
#define MAME_BUS_A1BUS_A1CASSETTE_H

#pragma once

#include "a1bus.h"
#include "imagedev/cassette.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a1bus_cassette_device:
	public device_t,
	public device_a1bus_card_interface
{
public:
	// construction/destruction
	a1bus_cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(cassette_r);
	DECLARE_WRITE8_MEMBER(cassette_w);

protected:
	a1bus_cassette_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;


	void cassette_toggle_output();

	optional_device<cassette_image_device> m_cassette;

private:
	required_region_ptr<uint8_t> m_rom;
	int m_cassette_output_flipflop;
};

// device type definition
DECLARE_DEVICE_TYPE(A1BUS_CASSETTE, a1bus_cassette_device)

#endif  // MAME_BUS_A1BUS_A1CASSETTE_H
