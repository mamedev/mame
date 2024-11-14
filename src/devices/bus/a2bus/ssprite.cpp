// license:BSD-3-Clause
// copyright-holders:R. Belmont, Golden Child
/*********************************************************************

    ssprite.cpp

    Implementation of the Synetix SuperSprite

*********************************************************************/

#include "emu.h"
#include "ssprite.h"

#include "sound/ay8910.h"
#include "sound/tms5220.h"
#include "video/tms9928a.h"

#include "speaker.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TMS_TAG "ssprite_tms"
#define TMS5220_TAG "ssprite_tms5220"
#define AY_TAG "ssprite_ay"
#define SCREEN_TAG "screen"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ssprite_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ssprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ssprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

private:
	void tms_irq_w(int state);

	required_device<tms9918a_device> m_tms;
	required_device<ay8910_device> m_ay;
	required_device<tms5220_device> m_tms5220;
};

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_ssprite_device::device_add_mconfig(machine_config &config)
{
	TMS9918A(config, m_tms, XTAL(10'738'635)).set_screen(SCREEN_TAG);
	m_tms->set_vram_size(0x4000); // 16k of VRAM
	m_tms->int_callback().set(FUNC(a2bus_ssprite_device::tms_irq_w));
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	SPEAKER(config, "mono").front_center();
	AY8912(config, m_ay, 1022727).add_route(ALL_OUTPUTS, "mono", 1.0);
	TMS5220(config, m_tms5220, 640000).add_route(ALL_OUTPUTS, "mono", 1.0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ssprite_device::a2bus_ssprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ssprite_device(mconfig, A2BUS_SSPRITE, tag, owner, clock)
{
}

a2bus_ssprite_device::a2bus_ssprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG),
	m_ay(*this, AY_TAG),
	m_tms5220(*this, TMS5220_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ssprite_device::device_start()
{
}

void a2bus_ssprite_device::device_reset()
{
}

/*
    C0nx map: (info from Synetix SuperSprite Owners manual.pdf page 33 of 266)
    0 - TMS9918 VDP vram read/write
    1 - TMS9918 VDP register write
    2 - TMS5220 Speech read/write
    3 - Video Switch APPLE VIDEO IN OFF
    4 - Video Switch APPLE VIDEO IN ON
    5 - Video Switch APPLE ONLY OUT
    6 - Video Switch MIX VDP/EXTERNAL VIDEO
    7 - TMS 9918 WRITE ONLY/FRAME RESET
    C - AY Sound data write
    D - AY Sound data write
    E - AY Sound register write or data read
    F - AY Sound register write or data read
*/


uint8_t a2bus_ssprite_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return m_tms->vram_read();
		case 1:
			return m_tms->register_read();
		case 2:
			return 0x1f | m_tms5220->status_r();
		case 14:
		case 15:
			return m_ay->data_r();
	}

	return 0xff;
}

void a2bus_ssprite_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_tms->vram_write(data);
			break;
		case 1:
			m_tms->register_write(data);
			break;
		case 2:
			m_tms5220->data_w(data);
			break;
		case 12:
		case 13:
			m_ay->data_w(data);
			break;
		case 14:
		case 15:
			m_ay->address_w(data);
			break;
	}
}

void a2bus_ssprite_device::tms_irq_w(int state)
{
	if (state)
		raise_slot_irq();
	else
		lower_slot_irq();
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SSPRITE, device_a2bus_card_interface, a2bus_ssprite_device, "a2ssprite", "Synetix SuperSprite")
