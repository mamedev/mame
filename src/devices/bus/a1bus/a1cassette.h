// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a1cassette.h

    Apple I Cassette Interface

*********************************************************************/

#ifndef __A1BUS_CASSETTE__
#define __A1BUS_CASSETTE__

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
	a1bus_cassette_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	optional_device<cassette_image_device> m_cassette;

	uint8_t cassette_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cassette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	void cassette_toggle_output();

private:
	uint8_t *m_rom;
	int m_cassette_output_flipflop;
};

// device type definition
extern const device_type A1BUS_CASSETTE;

#endif  /* __A1BUS_CASSETTE__ */
