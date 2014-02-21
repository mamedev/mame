// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    IQ151 MS151A XY plotter module emulation

***************************************************************************/

#include "emu.h"
#include "ms151a.h"

#include "emuopts.h"
#include "png.h"

// paper is A4 (210x297mm)
#define PAPER_WIDTH         (210*10)
#define PAPER_HEIGHT        (297*10)

// usable area is 175x250mm step is 0.1mm
#define PAPER_MAX_X         (175*10)
#define PAPER_MAX_Y         (250*10)

// dump the m_paper bitmap into a png
#define DUMP_PAPER_INTO_PNG     0

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

ROM_START( iq151_ms151a )
	ROM_REGION(0x0800, "ms151a", 0)
	ROM_LOAD( "ms151a.rom", 0x0000, 0x0800, CRC(995c58d6) SHA1(ebdc4278cfe6d3cc7dafbaa05bc6c239e4e6c09b))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type IQ151_MS151A = &device_creator<iq151_ms151a_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iq151_ms151a_device - constructor
//-------------------------------------------------

iq151_ms151a_device::iq151_ms151a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, IQ151_MS151A, "IQ151 MS151A", tag, owner, clock, "iq151_ms151a", __FILE__),
		device_iq151cart_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iq151_ms151a_device::device_start()
{
	m_rom = (UINT8*)memregion("ms151a")->base();

	// allocate a bitmap for represent the paper
	m_paper = auto_bitmap_ind16_alloc(machine(), PAPER_WIDTH, PAPER_HEIGHT);
	m_paper->fill(0);

	m_pen = 0;
	m_posx = m_posy = 0;
}

//-------------------------------------------------
//  device_stop - clean up anything that needs to
//  happen before the running_machine goes away
//-------------------------------------------------

void iq151_ms151a_device::device_stop()
{
#if DUMP_PAPER_INTO_PNG
	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open("iq151_ms151a.png");

	if (filerr == FILERR_NONE)
	{
		static const rgb_t png_palette[] = { rgb_t::white, rgb_t::black };

		// save the paper into a png
		png_write_bitmap(file, NULL, *m_paper, 2, png_palette);
	}
#endif
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *iq151_ms151a_device::device_rom_region() const
{
	return ROM_NAME( iq151_ms151a );
}

//-------------------------------------------------
//  read
//-------------------------------------------------

void iq151_ms151a_device::read(offs_t offset, UINT8 &data)
{
	// interal ROM is mapped at 0xc000-0xc7ff
	if (offset >= 0xc000 && offset < 0xc800)
		data = m_rom[offset & 0x7ff];
}

//-------------------------------------------------
//  IO read
//-------------------------------------------------

void iq151_ms151a_device::io_read(offs_t offset, UINT8 &data)
{
	if (offset == 0xc4)
		data = plotter_status();
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

void iq151_ms151a_device::io_write(offs_t offset, UINT8 data)
{
	if (offset >= 0xc0 && offset <= 0xc4)
		plotter_update(offset - 0xc0, data);
}


//**************************************************************************
//  XY 4130/4131
//**************************************************************************

UINT8 iq151_ms151a_device::plotter_status()
{
	/*
	    bit 7 - plotter READY line
	*/

	return 0x80;
}

void iq151_ms151a_device::plotter_update(UINT8 offset, UINT8 data)
{
	// update pen and paper positions
	switch (offset)
	{
		case 0:     m_posy++;               break;
		case 1:     m_posy--;               break;
		case 2:     m_posx++;               break;
		case 3:     m_posx--;               break;
		case 4:     m_pen = data & 0x01;    break;
	}

	// clamp within range
	m_posx = MAX(m_posx, 0);
	m_posx = MIN(m_posx, PAPER_MAX_X);
	m_posy = MAX(m_posy, 0);
	m_posy = MIN(m_posy, PAPER_MAX_Y);

	// if pen is down draws a point
	if (m_pen)
		m_paper->pix16(((PAPER_HEIGHT-PAPER_MAX_Y)/2) + m_posy, ((PAPER_WIDTH-PAPER_MAX_X)/2) + m_posx) = 1;
}
