// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    IQ151 Aritma Minigraf 0507 module emulation

***************************************************************************/

#include "emu.h"
#include "minigraf.h"

#include "emuopts.h"
#include "png.h"

// paper is A4 (297x210mm)
#define PAPER_WIDTH         (210*8)
#define PAPER_HEIGHT        (297*8)

// usable area is 187.5x262.5mm step is 0.125mm
#define PAPER_MAX_X         1500
#define PAPER_MAX_Y         2100

// dump the m_paper bitmap into a png
#define DUMP_PAPER_INTO_PNG     0

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

ROM_START( iq151_minigraf )
	ROM_REGION(0x1000, "minigraf", 0)
	ROM_LOAD( "minigraf_010787.rom", 0x0000, 0x0800, CRC(d854d203) SHA1(ae19c2859f8d78fda227a74ab50c6eb095d14014))
	ROM_LOAD( "minigraf_050986.rom", 0x0800, 0x0800, CRC(e0559e9e) SHA1(475d294e4976f88ad13e77a39b1c607827c791dc))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type IQ151_MINIGRAF = &device_creator<iq151_minigraf_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iq151_minigraf_device - constructor
//-------------------------------------------------

iq151_minigraf_device::iq151_minigraf_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, IQ151_MINIGRAF, "IQ151 Minigraf", tag, owner, clock, "iq151_minigraf", __FILE__),
		device_iq151cart_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iq151_minigraf_device::device_start()
{
	m_rom = (UINT8*)memregion("minigraf")->base();

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

void iq151_minigraf_device::device_stop()
{
#if DUMP_PAPER_INTO_PNG
	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open("iq151_minigraf.png");

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

const rom_entry *iq151_minigraf_device::device_rom_region() const
{
	return ROM_NAME( iq151_minigraf );
}

//-------------------------------------------------
//  read
//-------------------------------------------------

void iq151_minigraf_device::read(offs_t offset, UINT8 &data)
{
	// interal ROM is mapped at 0xc000-0xc7ff
	if (offset >= 0xc000 && offset < 0xc800)
		data = m_rom[offset & 0x7ff];
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

void iq151_minigraf_device::io_write(offs_t offset, UINT8 data)
{
	if (offset >= 0xf0 && offset < 0xf4)
	{
		/*
		    Plotter control lines

		    ---- -xxx   horizontal step
		    --xx x---   vertical step
		    -x-- ----   ???
		    x--- ----   pen up/down
		*/

		plotter_update(data);
	}
}


//**************************************************************************
//  Aritma MINIGRAF 0507
//**************************************************************************

inline int iq151_minigraf_device::get_direction(UINT8 old_val, UINT8 new_val)
{
	if (new_val == 0 && old_val == 7)   return +1;
	if (new_val == 7 && old_val == 0)   return -1;

	return (new_val - old_val);
}

void iq151_minigraf_device::plotter_update(UINT8 control)
{
	// update pen and paper positions
	m_posy += get_direction(m_control & 7, control & 7);
	m_posx += get_direction((m_control>>3) & 7, (control>>3) & 7);

	// bit 7 is pen up/down
	m_pen = BIT(control, 7);

	// clamp within range
	m_posx = MAX(m_posx, 0);
	m_posx = MIN(m_posx, PAPER_MAX_X);
	m_posy = MAX(m_posy, 0);
	m_posy = MIN(m_posy, PAPER_MAX_Y);

	// if pen is down draws a point
	if (m_pen)
		m_paper->pix16(((PAPER_HEIGHT-PAPER_MAX_Y)/2) + m_posy, ((PAPER_WIDTH-PAPER_MAX_X)/2) + m_posx) = 1;

	m_control = control;
}
