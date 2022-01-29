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

#include "emu.h"
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

DEFINE_DEVICE_TYPE(ABC_FD2, abc_fd2_device, "abc_fd2", "ABC FD2")


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

const tiny_rom_entry *abc_fd2_device::device_rom_region() const
{
	return ROM_NAME( abc_fd2 );
}


//-------------------------------------------------
//  status_w -
//-------------------------------------------------

void abc_fd2_device::status_w(uint8_t data)
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

void abc_fd2_device::abc_fd2_mem(address_map &map)
{
	map(0x0000, 0x03ff).rom().region(Z80_TAG, 0);
	map(0x0800, 0x0bff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( abc_fd2_io )
//-------------------------------------------------

void abc_fd2_device::abc_fd2_io(address_map &map)
{
	map.global_mask(0x73);
	map(0x30, 0x33).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x50, 0x53).rw(FD1771_TAG, FUNC(fd1771_device::read), FUNC(fd1771_device::write));
	map(0x60, 0x60).w(FUNC(abc_fd2_device::status_w));
}


//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

uint8_t abc_fd2_device::pio_pa_r()
{
	return m_data;
}

void abc_fd2_device::pio_pa_w(uint8_t data)
{
	m_data = data;
}

uint8_t abc_fd2_device::pio_pb_r()
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

	uint8_t data = 0;

	data |= m_fdc->drq_r() << 5;
	data |= m_fdc->hld_r() << 6;
	data |= m_fdc->intrq_r() << 7;

	return data;
}

void abc_fd2_device::pio_pb_w(uint8_t data)
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

static void abc_fd2_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
}

void abc_fd2_device::floppy_formats(format_registration &fr)
{
	fr.add_fm_containers();
	fr.add(FLOPPY_ABC_FD2_FORMAT);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc_fd2_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &abc_fd2_device::abc_fd2_mem);
	m_maincpu->set_addrmap(AS_IO, &abc_fd2_device::abc_fd2_io);
	m_maincpu->set_daisy_config(daisy_chain);

	Z80PIO(config, m_pio, 4_MHz_XTAL / 2);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set(FUNC(abc_fd2_device::pio_pa_r));
	m_pio->out_pa_callback().set(FUNC(abc_fd2_device::pio_pa_w));
	m_pio->in_pb_callback().set(FUNC(abc_fd2_device::pio_pb_r));
	m_pio->out_pb_callback().set(FUNC(abc_fd2_device::pio_pb_w));

	FD1771(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(m_pio, FUNC(z80pio_device::pb7_w));
	m_fdc->drq_wr_callback().set(m_pio, FUNC(z80pio_device::pb5_w));
	m_fdc->hld_wr_callback().set(m_pio, FUNC(z80pio_device::pb6_w));

	FLOPPY_CONNECTOR(config, m_floppy0, abc_fd2_floppies, "525sssd", abc_fd2_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, abc_fd2_floppies, "525sssd", abc_fd2_device::floppy_formats).enable_sound(true);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_fd2_device - constructor
//-------------------------------------------------

abc_fd2_device::abc_fd2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC_FD2, tag, owner, clock),
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

void abc_fd2_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_fd2_device::device_reset()
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

void abc_fd2_device::abcbus_cs(uint8_t data)
{
	m_cs = (data == 0x2d);
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

uint8_t abc_fd2_device::abcbus_stat()
{
	uint8_t data = 0xff;

	if (m_cs)
	{
		data = (m_status & 0xfe) | m_pio->rdy_a();
	}

	return data;
}


//-------------------------------------------------
//  abcbus_inp -
//-------------------------------------------------

uint8_t abc_fd2_device::abcbus_inp()
{
	uint8_t data = 0xff;

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

void abc_fd2_device::abcbus_out(uint8_t data)
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

void abc_fd2_device::abcbus_c1(uint8_t data)
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

void abc_fd2_device::abcbus_c3(uint8_t data)
{
	if (m_cs)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  abcbus_xmemfl -
//-------------------------------------------------

uint8_t abc_fd2_device::abcbus_xmemfl(offs_t offset)
{
	uint8_t data = 0xff;

	if ((offset & 0xf000) == 0x6000)
	{
		data = m_dos_rom->base()[offset & 0xfff];
	}

	return data;
}
