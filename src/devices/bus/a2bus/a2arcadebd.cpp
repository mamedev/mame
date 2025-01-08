// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2arcadeboard.c

    Implementation of the Third Millenium Engineering Arcade Board

    TODO:
        - VDPTEST program seems to want more than 16K of RAM, but docs/ads/press releases say 16k, period
        - MLDEMO program needs vsync IRQ from the TMS but doesn't program the registers the way our emulation
          wants to enable IRQs

*********************************************************************/

#include "emu.h"
#include "a2arcadebd.h"

#include "video/tms9928a.h"
#include "sound/ay8910.h"

#include "speaker.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TMS_TAG "arcbd_tms"
#define AY_TAG "arcbd_ay"
#define SCREEN_TAG "screen"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_arcboard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_arcboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_arcboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

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
};

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_arcboard_device::device_add_mconfig(machine_config &config)
{
	TMS9918A(config, m_tms, XTAL(10'738'635)).set_screen(SCREEN_TAG);
	m_tms->set_vram_size(0x4000); // 16k of VRAM
	m_tms->int_callback().set(FUNC(a2bus_arcboard_device::tms_irq_w));
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, 1022727).add_route(ALL_OUTPUTS, "mono", 1.0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_arcboard_device::a2bus_arcboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_arcboard_device(mconfig, A2BUS_ARCADEBOARD, tag, owner, clock)
{
}

a2bus_arcboard_device::a2bus_arcboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG),
	m_ay(*this, AY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_arcboard_device::device_start()
{
}

void a2bus_arcboard_device::device_reset()
{
}

/*
    C0nx map:
    0 - TMS read vram
    1 - TMS read status
    2 - TMS write vram
    3 - TMS write register
    5 - AY register select
    6 - AY data
*/

uint8_t a2bus_arcboard_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return m_tms->vram_read();

		case 1:
			return m_tms->register_read();

		case 6:
			return m_ay->data_r();
	}

	return 0xff;
}

void a2bus_arcboard_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
		case 2:
			m_tms->vram_write(data);
			break;

		case 1:
		case 3:
			m_tms->register_write(data);
			break;

		case 5:
			m_ay->address_w(data);
			break;

		case 6:
			m_ay->data_w(data);
			break;
	}
}

void a2bus_arcboard_device::tms_irq_w(int state)
{
	if (state)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ARCADEBOARD, device_a2bus_card_interface, a2bus_arcboard_device, "a2arcbd", "Third Millenium Engineering Arcade Board")
