// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtanic Video 80/82 Board

    http://www.microtan.ukpc.net/pageProducts.html#VIDEO

    TODO:
    - implement the communication between the 8212's

**********************************************************************/


#include "emu.h"
#include "tug8082.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_TUG8082, tanbus_tug8082_device, "tanbus_vid8082", "Microtanic Video 80/82 Board")

//-------------------------------------------------
//  ADDRESS_MAP( vid8082_map )
//-------------------------------------------------

void tanbus_tug8082_device::vid8082_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x4000, 0x7fff).ram().share(m_videoram);
	map(0x8000, 0x8000).w(m_iop[0], FUNC(i8212_device::write));
	map(0x8001, 0x8001).r(m_iop[1], FUNC(i8212_device::read));
	map(0xe000, 0xffff).rom().region("rom", 0);
}

//-------------------------------------------------
//  ROM( tug8082 )
//-------------------------------------------------

ROM_START(tug8082)
	ROM_REGION(0x2000, "rom", 0)
	ROM_LOAD("vdu80.ic28", 0x0000, 0x2000, CRC(e9df89c3) SHA1(1db8b055306067eb62940c0e9e63d97a2fab20f9))
ROM_END

//-------------------------------------------------
//  INPUT_PORTS( tug8082 )
//-------------------------------------------------

INPUT_PORTS_START(tug8082)
	PORT_START("DSW0")
	PORT_DIPNAME(0x01, 0x01, "VDU Interrupt Request") PORT_DIPLOCATION("DSWA:1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "VDU Non Maskable Interrupt") PORT_DIPLOCATION("DSWA:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "Not used") PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x00, "Not used") PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0xf0, 0x00, "I/O Address") PORT_DIPLOCATION("DSWA:5,6,7,8")
	PORT_DIPSETTING(0x60, "$BE60 - $BE61")
	PORT_DIPSETTING(0x40, "$BE40 - $BE41")
	PORT_DIPSETTING(0x20, "$BE20 - $BE21")
	PORT_DIPSETTING(0x00, "$BE00 - $BE01")

	PORT_START("DSW1")
	PORT_DIPNAME(0x01, 0x00, "Not used") PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "Not used") PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "80 Column Mode 16K Ram") PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x00, "Not used") PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, "Not used") PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "Write Protect") PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x00, "40 Column Mode 8K Ram") PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "Video Select Disable") PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x80, DEF_STR(On))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor tanbus_tug8082_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tug8082);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_tug8082_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, DERIVED_CLOCK(1, 4));
	m_maincpu->set_addrmap(AS_PROGRAM, &tanbus_tug8082_device::vid8082_map);

	I8212(config, m_iop[0], 0);
	//m_iop[0]->md_rd_callback().set(CONSTANT(0));
	m_iop[0]->int_wr_callback().set(FUNC(tanbus_tug8082_device::bus_irq_w));
	//m_iop->do_wr_callback().set(FUNC(tanbus_tug8082_device::write));

	I8212(config, m_iop[1], 0);
	//m_iop[1]->md_rd_callback().set(CONSTANT(0));
	m_iop[1]->int_wr_callback().set(FUNC(tanbus_tug8082_device::vdu_irq_w));
	//m_iop->di_rd_callback().set(FUNC(tanbus_tug8082_device::read));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512 - 1, 0, 256 - 1);
	m_screen->set_screen_update(FUNC(tanbus_tug8082_device::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *tanbus_tug8082_device::device_rom_region() const
{
	return ROM_NAME(tug8082);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_tug8082_device - constructor
//-------------------------------------------------

tanbus_tug8082_device::tanbus_tug8082_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_TUG8082, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_iop(*this, "iop%u", 0)
	, m_dips(*this, "DSW%u", 0)
	, m_rom(*this, "rom")
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
	, m_videoram(*this, "videoram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_tug8082_device::device_start()
{
}

//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_tug8082_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	offs_t addr = 0xbe00 | (m_dips[0]->read() & 0xf0);

	if (offset == addr)
	{
			data = m_iop[0]->read();
	}

	return data;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_tug8082_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	offs_t addr = 0xbe01 | (m_dips[0]->read() & 0xf0);

	if (offset == addr)
	{
		m_iop[1]->write(data);
	}
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(tanbus_tug8082_device::bus_irq_w)
{
	/* Manual says these DIPs are not used but schematic suggests they are */
	//switch (m_dips[0]->read() >> 2)
	//{
	//case 0x01:
		m_tanbus->irq_w(state);
	//  break;
	//case 0x02:
	//  m_tanbus->nmi_w(state);
	//  break;
	//}
}

WRITE_LINE_MEMBER(tanbus_tug8082_device::vdu_irq_w)
{
	switch (m_dips[0]->read() & 0x03)
	{
	case 0x01:
		m_maincpu->set_input_line(M6502_IRQ_LINE, state);
		break;
	case 0x02:
		m_maincpu->set_input_line(M6502_NMI_LINE, state);
		break;
	case 0x03:
		m_maincpu->set_input_line(M6502_IRQ_LINE, state);
		m_maincpu->set_input_line(M6502_NMI_LINE, state);
		break;
	}
}

uint32_t tanbus_tug8082_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels;

	const pen_t *pen = m_palette->pens();

	for (y = 0; y < 256; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 64; x++)
		{
			pixels = m_videoram[(y * 64) + x];

			for (int i = 0; i < 8; ++i)
				*scanline++ = pen[BIT(pixels, 7 - i)];
		}
	}

	return 0;
}
