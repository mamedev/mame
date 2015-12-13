// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Scandia Metric ABC FD2 floppy controller emulation

*********************************************************************/

/*

PCB Layout
----------

  |-------------------------------------------|
|-|                                           |
|-|    ROM0                          4MHz     |
|-|                                           |
|-|                Z80PIO                     |
|-|                                        CN1|
|-|                FD1771    2114             |
|-|                          2114             |
|-|                Z80       ROM1             |
|-|                                           |
  |-------------------------------------------|

Notes:
    Relevant IC's shown.

    ROM0    - AMI 8005SAJ 1Kx8 EPROM
    ROM1    - Motorola MCM2708C 1Kx8 EPROM
    Z80     - Zilog Z-80 CPU
    Z80PIO  - Zilog Z-80A PIO
    FD1771  - FD1771-B01
    2114    - National Semiconductor MM2114N 1Kx4 Static RAM
    CN1     - 2x17 pin PCB header

*/

#include "fd2.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG     "2e"
#define Z80PIO_TAG  "2c"
#define FD1771_TAG  "2d"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC_FD2 = &device_creator<abc_fd2_t>;


//-------------------------------------------------
//  ROM( abc_fd2 )
//-------------------------------------------------

ROM_START( abc_fd2 )
	ROM_REGION( 0x1000, "dos", 0 )
	ROM_LOAD( "ami 8005saj.1a", 0x000, 0x800, CRC(d865213f) SHA1(ae7399ede74520ccb2dd5be2e6bb13c33ee81bd0) ) // what's this?
	ROM_LOAD( "abcdos.3d",   0x0000, 0x1000, CRC(2cb2192f) SHA1(a6b3a9587714f8db807c05bee6c71c0684363744) )

	ROM_REGION( 0x400, Z80_TAG, 0 )
	ROM_LOAD( "1.02.3f", 0x000, 0x400, CRC(a19fbdc2) SHA1(d500377c34ac6c679c155f4a5208e1c3e00cd920) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc_fd2_t::device_rom_region() const
{
	return ROM_NAME( abc_fd2 );
}


//-------------------------------------------------
//  status_w -
//-------------------------------------------------

WRITE8_MEMBER( abc_fd2_t::status_w )
{
	/*

	    bit     description

	    0       _INT to main Z80
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	m_status = data & 0xfe;

	// interrupt
	m_slot->irq_w(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  ADDRESS_MAP( abc_fd2_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc_fd2_mem, AS_PROGRAM, 8, abc_fd2_t )
	AM_RANGE(0x0000, 0x03ff) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0x0800, 0x0bff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc_fd2_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc_fd2_io, AS_IO, 8, abc_fd2_t )
	ADDRESS_MAP_GLOBAL_MASK(0x73)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0x50, 0x53) AM_DEVREADWRITE(FD1771_TAG, fd1771_t, read, write)
	AM_RANGE(0x60, 0x60) AM_WRITE(status_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

READ8_MEMBER( abc_fd2_t::pio_pa_r )
{
	return m_data;
}

WRITE8_MEMBER( abc_fd2_t::pio_pa_w )
{
	m_data = data;
}

READ8_MEMBER( abc_fd2_t::pio_pb_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       DRQ
	    6       HLD
	    7       INTRQ

	*/

	UINT8 data = 0;

	data |= m_fdc->drq_r() << 5;
	data |= m_fdc->hld_r() << 6;
	data |= m_fdc->intrq_r() << 7;

	return data;
}

WRITE8_MEMBER( abc_fd2_t::pio_pb_w )
{
	/*

	    bit     description

	    0       SEL1
	    1       SEL2
	    2       TG43
	    3       MON
	    4       HLT
	    5
	    6
	    7

	*/

	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		// motor enable
		floppy->mon_w(BIT(data, 3));
	}

	m_fdc->hlt_w(BIT(data, 4));
}


//-------------------------------------------------
//  z80_daisy_config daisy_chain
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	{ Z80PIO_TAG },
	{ nullptr }
};


//-------------------------------------------------
//  SLOT_INTERFACE( abc_fd2_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( abc_fd2_floppies )
	SLOT_INTERFACE( "525sssd", FLOPPY_525_SSSD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( abc_fd2_t::floppy_formats )
	FLOPPY_ABC_FD2_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  MACHINE_DRIVER( abc_fd2 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc_fd2 )
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_4MHz/2)
	MCFG_CPU_PROGRAM_MAP(abc_fd2_mem)
	MCFG_CPU_IO_MAP(abc_fd2_io)
	MCFG_CPU_CONFIG(daisy_chain)

	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_4MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(abc_fd2_t, pio_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(abc_fd2_t, pio_pa_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(abc_fd2_t, pio_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(abc_fd2_t, pio_pb_w))

	MCFG_FD1771_ADD(FD1771_TAG, XTAL_4MHz/4)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE(Z80PIO_TAG, z80pio_device, pb7_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE(Z80PIO_TAG, z80pio_device, pb5_w))
	MCFG_WD_FDC_HLD_CALLBACK(DEVWRITELINE(Z80PIO_TAG, z80pio_device, pb6_w))

	MCFG_FLOPPY_DRIVE_ADD(FD1771_TAG ":0", abc_fd2_floppies, "525sssd", abc_fd2_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1771_TAG ":1", abc_fd2_floppies, "525sssd", abc_fd2_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc_fd2_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc_fd2 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_fd2_t - constructor
//-------------------------------------------------

abc_fd2_t::abc_fd2_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABC_FD2, "ABC FD2", tag, owner, clock, "abc_fd2", __FILE__),
	device_abcbus_card_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG),
	m_pio(*this, Z80PIO_TAG),
	m_fdc(*this, FD1771_TAG),
	m_floppy0(*this, FD1771_TAG ":0"),
	m_floppy1(*this, FD1771_TAG ":1"),
	m_dos_rom(*this, "dos"),
	m_cs(false), m_status(0), m_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_fd2_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_fd2_t::device_reset()
{
	m_cs = false;

	m_status = 0;
	m_slot->irq_w(CLEAR_LINE);

	m_maincpu->reset();
	m_fdc->soft_reset();
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void abc_fd2_t::abcbus_cs(UINT8 data)
{
	m_cs = (data == 0x2d);
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

UINT8 abc_fd2_t::abcbus_stat()
{
	UINT8 data = 0xff;

	if (m_cs)
	{
		data = (m_status & 0xfe) | m_pio->rdy_a();
	}

	return data;
}


//-------------------------------------------------
//  abcbus_inp -
//-------------------------------------------------

UINT8 abc_fd2_t::abcbus_inp()
{
	UINT8 data = 0xff;

	if (m_cs)
	{
		if (!BIT(m_status, 6))
		{
			data = m_data;
		}

		m_pio->strobe_a(0);
		m_pio->strobe_a(1);
	}

	return data;
}


//-------------------------------------------------
//  abcbus_out -
//-------------------------------------------------

void abc_fd2_t::abcbus_out(UINT8 data)
{
	if (!m_cs) return;

	if (BIT(m_status, 6))
	{
		m_data = data;
	}

	m_pio->strobe_a(0);
	m_pio->strobe_a(1);
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void abc_fd2_t::abcbus_c1(UINT8 data)
{
	if (m_cs)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}


//-------------------------------------------------
//  abcbus_c3 -
//-------------------------------------------------

void abc_fd2_t::abcbus_c3(UINT8 data)
{
	if (m_cs)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  abcbus_xmemfl -
//-------------------------------------------------

UINT8 abc_fd2_t::abcbus_xmemfl(offs_t offset)
{
	UINT8 data = 0xff;

	if ((offset & 0xf000) == 0x6000)
	{
		data = m_dos_rom->base()[offset & 0xfff];
	}

	return data;
}
