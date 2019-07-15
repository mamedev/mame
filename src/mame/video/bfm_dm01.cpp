// license:BSD-3-Clause
// copyright-holders:James Wallace
/***************************************************************************

    Bellfruit dotmatrix driver, (under heavy construction !!!)

    19-08-2005: Re-Animator
    25-08-2005: Fixed feedback through sc2 input line

    A.G.E Code Copyright J. Wallace and the AGEMAME Development Team.
    Visit http://www.mameworld.net/agemame/ for more information.


Standard dm01 memorymap

    hex    |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-----------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k)
-----------+---+-----------------+-----------------------------------------
2000       |R/W| D D D D D D D D | control register
-----------+---+-----------------+-----------------------------------------
2800       |R/W| D D D D D D D D | mux
-----------+---+-----------------+-----------------------------------------
3000       |R/W| D D D D D D D D | communication with mainboard
-----------+---+-----------------+-----------------------------------------
3800       |R/W| D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
4000-FFFF  | W | D D D D D D D D | ROM (48k)
-----------+---+-----------------+-----------------------------------------

  TODO: - find out clockspeed of CPU
        - sometimes screen isn't cleared, is the a clear bit missing?

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "bfm_dm01.h"

// local vars /////////////////////////////////////////////////////////////

#define DM_MAXLINES    21


#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

DEFINE_DEVICE_TYPE(BFM_DM01, bfm_dm01_device, "bfm_dm01", "BFM Dotmatrix 01")


bfm_dm01_device::bfm_dm01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BFM_DM01, tag, owner, clock),
	m_matrixcpu(*this, "matrix"),
	m_screen(*this, "dmd"),
	m_palette(*this, "palette_lcd"),
	m_data_avail(0),
	m_control(0),
	m_xcounter(0),
	m_busy(0),
	m_comdata(0),
	m_busy_cb(*this)
{
	for (auto & elem : m_segbuffer)
	elem = 0;

	for (auto & elem : m_scanline)
	elem = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bfm_dm01_device::device_start()
{
	if(!m_screen->started())
		throw device_missing_dependencies();

	if(!m_palette->started())
		throw device_missing_dependencies();

	m_busy_cb.resolve_safe();

	save_item(NAME(m_data_avail));
	save_item(NAME(m_control));
	save_item(NAME(m_xcounter));
	save_item(NAME(m_busy));
	save_item(NAME(m_comdata));

	for (int i = 0; i < 65; i++)
	save_item(NAME(m_segbuffer), i);

	for (int i = 0; i < BYTES_PER_ROW; i++)
	save_item(NAME(m_scanline), i);

	m_screen->register_screen_bitmap(m_tmpbitmap);
	m_palette->set_pen_color(0, rgb_t(0, 0, 0));        // background
	m_palette->set_pen_color(1, rgb_t(15, 1, 1));       // off dot
	m_palette->set_pen_color(2, rgb_t(255, 31, 31));    // on dot
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bfm_dm01_device::device_reset()
{
	m_busy     = 0;
	m_control  = 0;
	m_xcounter = 0;
	m_data_avail = 0;

	m_busy_cb(m_busy);
}

///////////////////////////////////////////////////////////////////////////

int bfm_dm01_device::read_data(void)
{
	int data = m_comdata;

	m_data_avail = 0;

	return data;
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER( bfm_dm01_device::control_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER( bfm_dm01_device::control_w )
{
	int changed = m_control ^ data;

	m_control = data;

	if ( changed & 2 )
	{   // reset horizontal counter
		if ( !(data & 2) )
		{
			//int offset = 0;

			m_xcounter = 0;
		}
	}

	if ( changed & 8 )
	{ // bit 3 changed = BUSY line
		if ( data & 8 )   m_busy = 0;
		else              m_busy = 1;

		m_busy_cb(m_busy);
	}
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER( bfm_dm01_device::mux_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER( bfm_dm01_device::mux_w )
{
	g_profiler.start(PROFILER_USER2);

	if ( m_xcounter < BYTES_PER_ROW )
	{
		m_scanline[m_xcounter] = data;
		m_xcounter++;
	}
	if ( m_xcounter == 9 )
	{
		int row = ((0xFF^data) & 0x7C) >> 2;    // 7C = 000001111100
		m_scanline[8] &= 0x80;//filter all other bits
		if ( (row >= 0)  && (row < DM_MAXLINES) )
		{
			int p = 0;

			while ( p < (BYTES_PER_ROW) )
			{
				uint8_t d = m_scanline[p];

				for (int bitpos=0; bitpos <8; bitpos++)
				{
					if (((p*8)+bitpos) <65)
					{
						if (d & 1<<(7-bitpos)) m_segbuffer[(p*8)+bitpos]=1;
						else m_segbuffer[(p*8)+bitpos]=0;
					}
				}
				p++;
			}

			uint16_t* pix = &m_tmpbitmap.pix16(row*2);
			uint16_t* pix2 = &m_tmpbitmap.pix16((row*2)+1);
			for (int pos=0;pos<65;pos++)
			{
				pix[0 + (pos * 2)] = m_segbuffer[(pos)]+1;
				pix[1 + (pos * 2)] = 0;
				pix2[0 + (pos * 2)] = 0;
				pix2[1 + (pos * 2)] = 0;
			}
		}
	}

	g_profiler.stop();
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER( bfm_dm01_device::comm_r )
{
	int result = 0;

	if ( m_data_avail )
	{
		result = read_data();

		#ifdef UNUSED_FUNCTION
		if ( m_data_avail() )
		{
			cpu_set_irq_line(1, M6809_IRQ_LINE, ASSERT_LINE );  // trigger IRQ
		}
		#endif
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER( bfm_dm01_device::comm_w )
{
}
///////////////////////////////////////////////////////////////////////////

READ8_MEMBER( bfm_dm01_device::unknown_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER( bfm_dm01_device::unknown_w )
{
	m_matrixcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE ); //?
}

///////////////////////////////////////////////////////////////////////////

void bfm_dm01_device::bfm_dm01_memmap(address_map &map)
{
	map(0x0000, 0x1fff).ram();                             // 8k RAM
	map(0x2000, 0x2000).rw(FUNC(bfm_dm01_device::control_r), FUNC(bfm_dm01_device::control_w));  // control reg
	map(0x2800, 0x2800).rw(FUNC(bfm_dm01_device::mux_r), FUNC(bfm_dm01_device::mux_w));           // mux
	map(0x3000, 0x3000).rw(FUNC(bfm_dm01_device::comm_r), FUNC(bfm_dm01_device::comm_w));     //
	map(0x3800, 0x3800).rw(FUNC(bfm_dm01_device::unknown_r), FUNC(bfm_dm01_device::unknown_w));   // ???
	map(0x4000, 0xFfff).rom();                             // 48k  ROM
}


uint32_t bfm_dm01_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_tmpbitmap, 0, 0, 0, 0,       cliprect);
	return 0;
}



INTERRUPT_GEN_MEMBER( bfm_dm01_device::nmi_line_assert )
{
	m_matrixcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void bfm_dm01_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_matrixcpu, 8000000); // MC68B09CP (clock unknown)
	m_matrixcpu->set_addrmap(AS_PROGRAM, &bfm_dm01_device::bfm_dm01_memmap);
	m_matrixcpu->set_periodic_int(FUNC(bfm_dm01_device::nmi_line_assert), attotime::from_hz(1500));          /* generate 1500 NMI's per second ?? what is the exact freq?? */

	PALETTE(config, m_palette).set_entries(3);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(65*2, 21*2);
	m_screen->set_visarea(0, 65*2-1, 0, 21*2-1);
	m_screen->set_screen_update(FUNC(bfm_dm01_device::screen_update));
	m_screen->set_palette(m_palette);
}

///////////////////////////////////////////////////////////////////////////

void bfm_dm01_device::writedata(uint8_t data)
{
	m_comdata = data;
	m_data_avail = 1;

	//pulse IRQ line
	m_matrixcpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE ); // trigger IRQ
}

///////////////////////////////////////////////////////////////////////////
int bfm_dm01_device::busy(void)
{
	return m_data_avail;
}
