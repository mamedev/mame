// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ezcgi.c

    "E-Z Color Graphics Interface" by Steve Ciarcia
    from BYTE Magazine, August, 1982
    https://archive.org/details/byte-magazine-1982-08-rescan

*********************************************************************/

#include "emu.h"
#include "ezcgi.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TMS_TAG "ezcgi_tms"
#define SCREEN_TAG "screen"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_EZCGI,      a2bus_ezcgi_device,      "a2ezcgi",  "E-Z Color Graphics Interface")
DEFINE_DEVICE_TYPE(A2BUS_EZCGI_9938, a2bus_ezcgi_9938_device, "a2ezcgi3", "E-Z Color Graphics Interface (TMS9938)")
DEFINE_DEVICE_TYPE(A2BUS_EZCGI_9958, a2bus_ezcgi_9958_device, "a2ezcgi5", "E-Z Color Graphics Interface (TMS9958)")

#define MSX2_XBORDER_PIXELS     16
#define MSX2_YBORDER_PIXELS     28
#define MSX2_TOTAL_XRES_PIXELS      256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS      212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS 8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS 14 * 2

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(a2bus_ezcgi_device::device_add_mconfig)
	MCFG_DEVICE_ADD( TMS_TAG, TMS9918A, XTAL(10'738'635) / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000) // 16k of VRAM
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(*this, a2bus_ezcgi_device, tms_irq_w))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS_TAG, tms9918a_device, screen_update )
MACHINE_CONFIG_END

MACHINE_CONFIG_START(a2bus_ezcgi_9938_device::device_add_mconfig)
	MCFG_V9938_ADD(TMS_TAG, SCREEN_TAG, 0x30000, XTAL(21'477'272))    // 192K of VRAM / typical 9938 clock, not verified
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(*this, a2bus_ezcgi_9938_device, tms_irq_w))

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE(TMS_TAG, v9938_device, screen_update)
	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, 262*2)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(a2bus_ezcgi_9958_device::device_add_mconfig)
	MCFG_V9958_ADD(TMS_TAG, SCREEN_TAG, 0x30000, XTAL(21'477'272))    // 192K of VRAM / typcial 9938/9958 clock, not verified
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(*this, a2bus_ezcgi_9958_device, tms_irq_w))

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE(TMS_TAG, v9938_device, screen_update)
	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, 262*2)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ezcgi_device::a2bus_ezcgi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ezcgi_device(mconfig, A2BUS_EZCGI, tag, owner, clock)
{
}

a2bus_ezcgi_device::a2bus_ezcgi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_ezcgi_9938_device::a2bus_ezcgi_9938_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ezcgi_9938_device(mconfig, A2BUS_EZCGI_9938, tag, owner, clock)
{
}

a2bus_ezcgi_9938_device::a2bus_ezcgi_9938_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_ezcgi_9958_device::a2bus_ezcgi_9958_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ezcgi_9958_device(mconfig, A2BUS_EZCGI_9958, tag, owner, clock)
{
}

a2bus_ezcgi_9958_device::a2bus_ezcgi_9958_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ezcgi_device::device_start()
{
}

void a2bus_ezcgi_device::device_reset()
{
}

void a2bus_ezcgi_9938_device::device_start()
{
}

void a2bus_ezcgi_9938_device::device_reset()
{
}

void a2bus_ezcgi_9958_device::device_start()
{
}

void a2bus_ezcgi_9958_device::device_reset()
{
}

/*
    C0nx map:
    0 - TMS read
    1 - TMS write
*/

uint8_t a2bus_ezcgi_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return m_tms->vram_read();

		case 1:
			return m_tms->register_read();
	}

	return 0xff;
}

void a2bus_ezcgi_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_tms->vram_write(data);
			break;

		case 1:
			m_tms->register_write(data);
			break;
	}
}

uint8_t a2bus_ezcgi_9938_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return m_tms->vram_r();

		case 1:
			return m_tms->status_r();
	}

	return 0xff;
}

void a2bus_ezcgi_9938_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_tms->vram_w(data);
			break;

		case 1:
			m_tms->command_w(data);
			break;

		case 2:
			m_tms->palette_w(data);
			break;

		case 3:
			m_tms->register_w(data);
			break;
	}
}

uint8_t a2bus_ezcgi_9958_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return m_tms->vram_r();

		case 1:
			return m_tms->status_r();
	}

	return 0xff;
}

void a2bus_ezcgi_9958_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_tms->vram_w(data);
			break;

		case 1:
			m_tms->command_w(data);
			break;

		case 2:
			m_tms->palette_w(data);
			break;

		case 3:
			m_tms->register_w(data);
			break;
	}
}

WRITE_LINE_MEMBER( a2bus_ezcgi_device::tms_irq_w )
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

WRITE_LINE_MEMBER( a2bus_ezcgi_9938_device::tms_irq_w )
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

WRITE_LINE_MEMBER( a2bus_ezcgi_9958_device::tms_irq_w )
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
