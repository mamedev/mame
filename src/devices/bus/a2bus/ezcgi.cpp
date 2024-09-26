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

#include "video/tms9928a.h"
#include "video/v9938.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TMS_TAG "ezcgi_tms"
#define SCREEN_TAG "screen"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ezcgi_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ezcgi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ezcgi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<tms9918a_device> m_tms;

private:
	void tms_irq_w(int state);
};

class a2bus_ezcgi_9938_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ezcgi_9938_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ezcgi_9938_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<v9938_device> m_tms;

private:
	void tms_irq_w(int state);
};

class a2bus_ezcgi_9958_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ezcgi_9958_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ezcgi_9958_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<v9958_device> m_tms;

private:
	void tms_irq_w(int state);
};

#define MSX2_XBORDER_PIXELS     16
#define MSX2_YBORDER_PIXELS     28
#define MSX2_TOTAL_XRES_PIXELS      256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS      212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS 8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS 14 * 2

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_ezcgi_device::device_add_mconfig(machine_config &config)
{
	TMS9918A(config, m_tms, XTAL(10'738'635)).set_screen(SCREEN_TAG);
	m_tms->set_vram_size(0x4000); // 16k of VRAM
	m_tms->int_callback().set(FUNC(a2bus_ezcgi_device::tms_irq_w));
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);
}

void a2bus_ezcgi_9938_device::device_add_mconfig(machine_config &config)
{
	V9938(config, m_tms, XTAL(21'477'272));    // typical 9938 clock, not verified
	m_tms->set_vram_size(0x30000);    // 192K of VRAM
	m_tms->set_screen(SCREEN_TAG);
	m_tms->int_cb().set(FUNC(a2bus_ezcgi_9938_device::tms_irq_w));

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(MSX2_TOTAL_XRES_PIXELS, 262*2);
	screen.set_visarea(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1);
}

void a2bus_ezcgi_9958_device::device_add_mconfig(machine_config &config)
{
	V9958(config, m_tms, XTAL(21'477'272));    // typical 9938/9958 clock, not verified
	m_tms->set_vram_size(0x30000);    // 192K of VRAM
	m_tms->set_screen(SCREEN_TAG);
	m_tms->int_cb().set(FUNC(a2bus_ezcgi_9958_device::tms_irq_w));

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(MSX2_TOTAL_XRES_PIXELS, 262*2);
	screen.set_visarea(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1);
}

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

void a2bus_ezcgi_device::tms_irq_w(int state)
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

void a2bus_ezcgi_9938_device::tms_irq_w(int state)
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

void a2bus_ezcgi_9958_device::tms_irq_w(int state)
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

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_EZCGI,      device_a2bus_card_interface, a2bus_ezcgi_device,      "a2ezcgi",  "E-Z Color Graphics Interface")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_EZCGI_9938, device_a2bus_card_interface, a2bus_ezcgi_9938_device, "a2ezcgi3", "E-Z Color Graphics Interface (TMS9938)")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_EZCGI_9958, device_a2bus_card_interface, a2bus_ezcgi_9958_device, "a2ezcgi5", "E-Z Color Graphics Interface (TMS9958)")
