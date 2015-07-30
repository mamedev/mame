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

const device_type A2BUS_EZCGI = &device_creator<a2bus_ezcgi_device>;
const device_type A2BUS_EZCGI_9938 = &device_creator<a2bus_ezcgi_9938_device>;
const device_type A2BUS_EZCGI_9958 = &device_creator<a2bus_ezcgi_9958_device>;

MACHINE_CONFIG_FRAGMENT( ezcgi )
	MCFG_DEVICE_ADD( TMS_TAG, TMS9918A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000) // 16k of VRAM
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(a2bus_ezcgi_device, tms_irq_w))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS_TAG, tms9918a_device, screen_update )
MACHINE_CONFIG_END

#define MSX2_XBORDER_PIXELS     16
#define MSX2_YBORDER_PIXELS     28
#define MSX2_TOTAL_XRES_PIXELS      256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS      212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS 8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS 14 * 2

MACHINE_CONFIG_FRAGMENT( ezcgi9938 )
	MCFG_V9938_ADD(TMS_TAG, SCREEN_TAG, 0x30000, XTAL_21_4772MHz)    // 192K of VRAM / typical 9938 clock, not verified
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(a2bus_ezcgi_9938_device, tms_irq_w))

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE(TMS_TAG, v9938_device, screen_update)
	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, 262*2)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
	MCFG_SCREEN_PALETTE("ezcgi_tms:palette")
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( ezcgi9958 )
	MCFG_V9958_ADD(TMS_TAG, SCREEN_TAG, 0x30000, XTAL_21_4772MHz)    // 192K of VRAM / typcial 9938/9958 clock, not verified
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(a2bus_ezcgi_9958_device, tms_irq_w))

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE(TMS_TAG, v9938_device, screen_update)
	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, 262*2)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
	MCFG_SCREEN_PALETTE("ezcgi_tms:palette")
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_ezcgi_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ezcgi );
}

machine_config_constructor a2bus_ezcgi_9938_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ezcgi9938 );
}

machine_config_constructor a2bus_ezcgi_9958_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ezcgi9958 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ezcgi_device::a2bus_ezcgi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_EZCGI, "E-Z Color Graphics Interface", tag, owner, clock, "a2ezcgi", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_ezcgi_device::a2bus_ezcgi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_ezcgi_9938_device::a2bus_ezcgi_9938_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_EZCGI_9938, "E-Z Color Graphics Interface (TMS9938)", tag, owner, clock, "a2ezcgi3", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_ezcgi_9938_device::a2bus_ezcgi_9938_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_ezcgi_9958_device::a2bus_ezcgi_9958_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_EZCGI_9958, "E-Z Color Graphics Interface (TMS9958)", tag, owner, clock, "a2ezcgi5", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_ezcgi_9958_device::a2bus_ezcgi_9958_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ezcgi_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

void a2bus_ezcgi_device::device_reset()
{
}

void a2bus_ezcgi_9938_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

void a2bus_ezcgi_9938_device::device_reset()
{
}

void a2bus_ezcgi_9958_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

void a2bus_ezcgi_9958_device::device_reset()
{
}

/*
    C0nx map:
    0 - TMS read
    1 - TMS write
*/

UINT8 a2bus_ezcgi_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0:
			return m_tms->vram_read(space, 0);

		case 1:
			return m_tms->register_read(space, 0);
	}

	return 0xff;
}

void a2bus_ezcgi_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 0:
			m_tms->vram_write(space, 0, data);
			break;

		case 1:
			m_tms->register_write(space, 0, data);
			break;
	}
}

UINT8 a2bus_ezcgi_9938_device::read_c0nx(address_space &space, UINT8 offset)
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

void a2bus_ezcgi_9938_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
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

UINT8 a2bus_ezcgi_9958_device::read_c0nx(address_space &space, UINT8 offset)
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

void a2bus_ezcgi_9958_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
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
