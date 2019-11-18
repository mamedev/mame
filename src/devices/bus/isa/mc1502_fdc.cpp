// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Electronika MC 1502 FDC device

**********************************************************************/

#include "emu.h"
#include "mc1502_fdc.h"

#include "cpu/i86/i86.h"
#include "formats/pc_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MC1502_FDC, mc1502_fdc_device, "mc1502_fdc", "MC-1502 floppy")

FLOPPY_FORMATS_MEMBER( mc1502_fdc_device::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static void mc1502_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  ROM( mc1502_fdc )
//-------------------------------------------------

ROM_START( mc1502_fdc )
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mc1502_fdc_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(FUNC(mc1502_fdc_device::mc1502_fdc_irq_drq));
	m_fdc->drq_wr_callback().set(FUNC(mc1502_fdc_device::mc1502_fdc_irq_drq));
	FLOPPY_CONNECTOR(config, "fdc:0", mc1502_floppies, "525qd", mc1502_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", mc1502_floppies, "525qd", mc1502_fdc_device::floppy_formats);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mc1502_fdc_device::device_rom_region() const
{
	return ROM_NAME(mc1502_fdc);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

TIMER_CALLBACK_MEMBER(mc1502_fdc_device::motor_callback)
{
	m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(ASSERT_LINE);
	m_fdc->subdevice<floppy_connector>("1")->get_device()->mon_w(ASSERT_LINE);
	motor_on = 0;
}

uint8_t mc1502_fdc_device::mc1502_wd17xx_aux_r()
{
	uint8_t data;

	data = 0;

	return data;
}

void mc1502_fdc_device::mc1502_wd17xx_aux_w(uint8_t data)
{
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();
	floppy_image_device *floppy = ((data & 0x10) ? floppy1 : floppy0);

	// master reset
	if ((data & 1) == 0) m_fdc->reset();

	m_fdc->set_floppy(floppy);

	// SIDE ONE
	floppy->ss_w((data & 2) ? 1 : 0);

	// bits 2, 3 -- motor on (drive 0, 1)
	floppy0->mon_w(!(data & 4));
	floppy1->mon_w(!(data & 8));

	if (data & 12)
	{
		motor_timer->adjust(attotime::from_msec(3000));
		motor_on = 1;
	}
}

/*
 * Accessing this port halts the CPU via READY line until DRQ, INTRQ or MOTOR ON
 */
uint8_t mc1502_fdc_device::mc1502_wd17xx_drq_r()
{
	cpu_device *maincpu = machine().device<cpu_device>("maincpu");

	if (!m_fdc->drq_r() && !m_fdc->intrq_r())
	{
		// fake cpu wait by resetting PC one insn back
		maincpu->set_state_int(I8086_IP, maincpu->state_int(I8086_IP) - 1);
		maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}

	return m_fdc->drq_r();
}

uint8_t mc1502_fdc_device::mc1502_wd17xx_motor_r()
{
	return motor_on;
}

WRITE_LINE_MEMBER(mc1502_fdc_device::mc1502_fdc_irq_drq)
{
	cpu_device *maincpu = machine().device<cpu_device>("maincpu");

	if (state) maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}

READ8_MEMBER(mc1502_fdc_device::mc1502_fdc_r)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0:
		data = mc1502_wd17xx_aux_r();
		break;
	case 8:
		data = mc1502_wd17xx_drq_r();
		break;
	case 10:
		data = mc1502_wd17xx_motor_r();
		break;
	}

	return data;
}

READ8_MEMBER(mc1502_fdc_device::mc1502_fdcv2_r)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0:
		data = mc1502_wd17xx_aux_r();
		break;
	case 1:
		data = mc1502_wd17xx_motor_r();
		break;
	case 2:
		data = mc1502_wd17xx_drq_r();
		break;
	}

	return data;
}

WRITE8_MEMBER(mc1502_fdc_device::mc1502_fdc_w)
{
	switch (offset)
	{
	case 0:
		mc1502_wd17xx_aux_w(data);
		break;
	}
}

//-------------------------------------------------
//  mc1502_fdc_device - constructor
//-------------------------------------------------

mc1502_fdc_device::mc1502_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MC1502_FDC, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, motor_on(0)
	, motor_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc1502_fdc_device::device_start()
{
	set_isa_device();

	// BIOS 5.0-5.2x
	m_isa->install_device(0x010c, 0x010f, read8sm_delegate(*m_fdc, FUNC(fd1793_device::read)), write8sm_delegate(*m_fdc, FUNC(fd1793_device::write)));
	m_isa->install_device(0x0100, 0x010b, read8_delegate(*this, FUNC(mc1502_fdc_device::mc1502_fdc_r)), write8_delegate(*this, FUNC(mc1502_fdc_device::mc1502_fdc_w)));

	// BIOS 5.3x
	m_isa->install_device(0x0048, 0x004b, read8sm_delegate(*m_fdc, FUNC(fd1793_device::read)), write8sm_delegate(*m_fdc, FUNC(fd1793_device::write)));
	m_isa->install_device(0x004c, 0x004f, read8_delegate(*this, FUNC(mc1502_fdc_device::mc1502_fdcv2_r)), write8_delegate(*this, FUNC(mc1502_fdc_device::mc1502_fdc_w)));

	motor_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc1502_fdc_device::motor_callback),this));
	motor_on = 0;
}
