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
	a1bus_cassette_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	a1bus_cassette_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	optional_device<cassette_image_device> m_cassette;

	DECLARE_READ8_MEMBER(cassette_r);
	DECLARE_WRITE8_MEMBER(cassette_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	void cassette_toggle_output();

private:
	UINT8 *m_rom;
	int m_cassette_output_flipflop;
};

// device type definition
extern const device_type A1BUS_CASSETTE;

#endif  /* __A1BUS_CASSETTE__ */
