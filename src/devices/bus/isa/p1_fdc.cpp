// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 FDC device (model B504)

**********************************************************************/

#include "emu.h"
#include "p1_fdc.h"

#include "cpu/i86/i86.h"
#include "formats/pc_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P1_FDC, p1_fdc_device, "p1_fdc", "Poisk-1 floppy B504")

FLOPPY_FORMATS_MEMBER( p1_fdc_device::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static void poisk1_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  ROM( p1_fdc )
//-------------------------------------------------

ROM_START( p1_fdc )
	ROM_REGION( 0x0800, "p1_fdc", 0 )
	ROM_DEFAULT_BIOS("a302")
	ROM_SYSTEM_BIOS(0, "normal", "B504 standard ROM")
	ROMX_LOAD( "b_ngmd_n.rf2", 0x00000, 0x0800, CRC(967e172a) SHA1(95117c40fd9f624fee08ccf37f615b16ff249688), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "a302", "v3.02") // Additional ROM BIOS v3.02 for DISKETTE service (c) Moscow 1991
	ROMX_LOAD( "b_ngmd_t.rf2", 0x00000, 0x0800, CRC(630010b1) SHA1(50876fe4f5f4f32a242faa70f9154574cd315ec4), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ae304", "v3.04") // Additional enhanced ROM BIOS v3.04 for DISKETTE service (c) V.Rusakow Moscow 1992
	ROMX_LOAD( "p_fdd_nm.bin", 0x00000, 0x0800, CRC(0b7f867d) SHA1(9fe7e0ab2242e50394d1162cf1a619b6f2994bfb), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "ae308", "v3.08") // Additional enhanced ROM BIOS v3.08 for DISKETTE service (c) V.Rusakov Tarasovka 1992
	ROMX_LOAD( "p_fdd_my.bin", 0x00000, 0x0800, CRC(da5d0eaf) SHA1(b188ba856bd28e4964a88feb0b0b2ba7eb320efc), ROM_BIOS(3))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void p1_fdc_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(FUNC(p1_fdc_device::p1_fdc_irq_drq));
	m_fdc->drq_wr_callback().set(FUNC(p1_fdc_device::p1_fdc_irq_drq));
	FLOPPY_CONNECTOR(config, "fdc:0", poisk1_floppies, "525qd", p1_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", poisk1_floppies, "525qd", p1_fdc_device::floppy_formats);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *p1_fdc_device::device_rom_region() const
{
	return ROM_NAME(p1_fdc);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

uint8_t p1_fdc_device::p1_wd17xx_motor_r()
{
	// XXX always on for now
	return 0;
}

uint8_t p1_fdc_device::p1_wd17xx_aux_r()
{
	cpu_device *maincpu = machine().device<cpu_device>("maincpu");

	if (!m_fdc->drq_r() && !m_fdc->intrq_r())
	{
		// fake cpu wait by resetting PC one insn back
		maincpu->set_state_int(I8086_IP, maincpu->state_int(I8086_IP) - 2);
		maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}

	return m_fdc->drq_r();
}

/*
;       D0 - DRIVE SELECT 0
;       D1 - DRIVE SELECT 1
;       D2 - MOTOR ON 0
;       D3 - MOTOR ON 1
;       D4 - SIDE (HEAD) SELECT
;       D5 - DOUBLE DENSITY
;       D6 - FDC RESET
;       D7 - NO USE
*/
void p1_fdc_device::p1_wd17xx_aux_w(int data)
{
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();
	floppy_image_device *floppy = ((data & 2) ? floppy1 : floppy0);

	if (!BIT(data, 6)) m_fdc->reset();

	m_fdc->set_floppy(floppy);

	floppy->ss_w(BIT(data, 4));
	m_fdc->dden_w(BIT(data, 5));

	floppy0->mon_w(!(data & 4));
	floppy1->mon_w(!(data & 8));
}

WRITE_LINE_MEMBER(p1_fdc_device::p1_fdc_irq_drq)
{
	cpu_device *maincpu = machine().device<cpu_device>("maincpu");

	if (state) maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}

READ8_MEMBER(p1_fdc_device::p1_fdc_r)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0:
		data = p1_wd17xx_aux_r();
		break;
	case 2:
		data = p1_wd17xx_motor_r();
		break;
	}

	return data;
}

WRITE8_MEMBER(p1_fdc_device::p1_fdc_w)
{
	switch (offset)
	{
	case 0:
		p1_wd17xx_aux_w(data);
		break;
	}
}

//-------------------------------------------------
//  p1_fdc_device - constructor
//-------------------------------------------------

p1_fdc_device::p1_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P1_FDC, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void p1_fdc_device::device_start()
{
	set_isa_device();
	m_isa->install_rom(this, 0xe0000, 0xe07ff, "XXX", "p1_fdc");
	m_isa->install_device(0x00c0, 0x00c3,
						  read8sm_delegate(FUNC(fd1793_device::read), m_fdc.target()),
						  write8sm_delegate(FUNC(fd1793_device::write), m_fdc.target()));
	m_isa->install_device(0x00c4, 0x00c7, read8_delegate( FUNC(p1_fdc_device::p1_fdc_r), this ), write8_delegate( FUNC(p1_fdc_device::p1_fdc_w), this ) );
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void p1_fdc_device::device_reset()
{
}
