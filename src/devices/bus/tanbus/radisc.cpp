// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Ralph Allen Disc Controller Card

    http://www.microtan.ukpc.net/6809/DOS_RTC.pdf

**********************************************************************/


#include "emu.h"
#include "radisc.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_RADISC, tanbus_radisc_device, "tanbus_radisc", "Ralph Allen Disc Controller Card")

//-------------------------------------------------
//  MACHINE_DRIVER( radisc )
//-------------------------------------------------

static void tandos_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_radisc_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irq_line).output_handler().set(FUNC(tanbus_radisc_device::fdc_irq_w));

	FD1793(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(m_irq_line, FUNC(input_merger_device::in_w<IRQ_FDC>));
	m_fdc->drq_wr_callback().set(FUNC(tanbus_radisc_device::fdc_drq_w));
	m_fdc->hld_wr_callback().set(FUNC(tanbus_radisc_device::fdc_hld_w));
	m_fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, m_floppies[0], tandos_floppies, "525qd", floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], tandos_floppies, "525qd", floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[2], tandos_floppies, nullptr, floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[3], tandos_floppies, nullptr, floppy_image_device::default_floppy_formats).enable_sound(true);

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_irq_line, FUNC(input_merger_device::in_w<IRQ_RTC>));

	VIA6522(config, m_via, 4_MHz_XTAL / 4);
	m_via->irq_handler().set(m_irq_line, FUNC(input_merger_device::in_w<IRQ_VIA>));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 1000); // TODO: unknown frequency
	m_beeper->add_route(ALL_OUTPUTS, "mono", 1.0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_radisc_device - constructor
//-------------------------------------------------

tanbus_radisc_device::tanbus_radisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_RADISC, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_floppies(*this, "fdc:%u", 0)
	, m_floppy(nullptr)
	, m_rtc(*this, "rtc")
	, m_via(*this, "via")
	, m_irq_line(*this, "irq_line")
	, m_beeper(*this, "beeper")
	, m_beeper_state(0)
	, m_drive_control(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_radisc_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tanbus_radisc_device::device_reset()
{
}

//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_radisc_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0xbf80: case 0xbf81: case 0xbf82: case 0xbf83: case 0xbf84: case 0xbf85: case 0xbf86: case 0xbf87: case 0xbf88: case 0xbf89: case 0xbf8a: case 0xbf8b: case 0xbf8c: case 0xbf8d: case 0xbf8e: case 0xbf8f:
		data = m_via->read(offset & 0x0f);
		break;
	case 0xbf90: case 0xbf91: case 0xbf92: case 0xbf93:
		data = m_fdc->read(offset & 0x03);
		break;
	case 0xbf94:
		data = control_r();
		break;
	case 0xbf98: case 0xbf99:
		data = m_rtc->read(offset & 0x01);
		break;
	}

	return data;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_radisc_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	switch (offset)
	{
	case 0xbf80: case 0xbf81: case 0xbf82: case 0xbf83: case 0xbf84: case 0xbf85: case 0xbf86: case 0xbf87: case 0xbf88: case 0xbf89: case 0xbf8a: case 0xbf8b: case 0xbf8c: case 0xbf8d: case 0xbf8e: case 0xbf8f:
		m_via->write(offset & 0x0f, data);
		break;
	case 0xbf90: case 0xbf91: case 0xbf92: case 0xbf93:
		m_fdc->write(offset & 0x03, data);
		break;
	case 0xbf94:
		control_w(data);
		break;
	case 0xbf95:
		m_beeper_state ^= 1;
		m_beeper->set_state(m_beeper_state);
		break;
	case 0xbf98: case 0xbf99:
		m_rtc->write(offset & 0x01, data);
		break;
	}
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void tanbus_radisc_device::control_w(uint8_t val)
{
	logerror("control_w %02x\n", val);
	m_drive_control = val;

	// bit 0: irq enable
	m_irq_enable = BIT(val, 0);

	// bit 1: data select (data stream controller)

	// bit 2, 3: drive select
	m_floppy = m_floppies[(val >> 2) & 0x03]->get_device();
	m_fdc->set_floppy(m_floppy);

	// bit 4: side select
	if (m_floppy)
		m_floppy->ss_w(BIT(val, 4));

	// bit 5: density
	m_fdc->dden_w(BIT(val, 5));

	// bit 6: head load
	if (m_floppy)
		m_floppy->mon_w(BIT(val, 6));

	// bit 7: drq enable
	m_drq_enable = BIT(val, 7);
}

uint8_t tanbus_radisc_device::control_r()
{
	logerror("control_r %02x\n", m_drive_control);
	return m_drive_control;
}

WRITE_LINE_MEMBER(tanbus_radisc_device::fdc_drq_w)
{
	m_tanbus->so_w((m_drq_enable && state) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(tanbus_radisc_device::fdc_irq_w)
{
	m_tanbus->irq_w((m_irq_enable && state) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(tanbus_radisc_device::fdc_hld_w)
{
	logerror("fdc_hld_w %d\n", state);
	if (m_floppy)
		m_floppy->mon_w(state);
}
