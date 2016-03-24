// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

    Acorn Electron driver

    MESS Driver By:

    Wilbert Pol

******************************************************************************/

#include "emu.h"
#include "includes/electron.h"

/*
  From the ElectrEm site:

Timing is somewhat of a thorny issue on the Electron. It is almost certain the
Electron could have been a much faster machine if BBC Micro OS level
compatibility had not been not a design requirement.

When accessing the ROM regions, the CPU always runs at 2MHz. When accessing
the FC (1 MHz bus) or FD (JIM) pages, the CPU always runs at 1MHz.

The timing for RAM accesses varies depending on the graphics mode, and how
many bytes are required to be read by the video circuits per scanline. When
accessing RAM in modes 4-6, the CPU is simply moved to a 1MHz clock. This
occurs for any RAM access at any point during the frame.

In modes 0-3, if the CPU tries to access RAM at any time during which the
video circuits are fetching bytes, it is halted by means of receiving a
stopped clock until the video circuits next stop fetching bytes.

Each scanline is drawn in exactly 64us, and of that the video circuits fetch
bytes for 40us. In modes 0, 1 and 2, 256 scanlines have pixels on, whereas in
mode 3 only 250 scanlines are affected as mode 3 is a 'spaced' mode.

As opposed to one clock generator which changes pace, the 1MHz and 2MHz clocks
are always available, so the ULA acts to simply change which clock is piped to
the CPU. This means in half of all cases, a further 2MHz cycle is lost waiting
for the 2MHz and 1MHz clocks to synchronise during a 2MHz to 1MHz step.

The video circuits run from a constant 2MHz clock, and generate 312 scanlines
a frame, one scanline every 128 cycles. This actually gives means the Electron
is running at 50.08 frames a second.

Creating a scanline numbering scheme where the first scanline with pixels is
scanline 0, in all modes the end of display interrupt is generated at the end
of scanline 255, and the RTC interrupt is generated upon the end of scanline 99.

From investigating some code for vertical split modes printed in Electron User
volume 7, issue 7 it seems that the exact timing of the end of display interrupt
is somewhere between 24 and 40 cycles after the end of pixels. This may coincide
with HSYNC. I have no similarly accurate timing for the real time clock
interrupt at this time.

Mode changes are 'immediate', so any change in RAM access timing occurs exactly
after the write cycle of the changing instruction. Similarly palette changes
take effect immediately. VSYNC is not signalled in any way.

*/

void electron_state::video_start()
{
	int i;
	for( i = 0; i < 256; i++ ) {
		m_map4[i] = ( ( i & 0x10 ) >> 3 ) | ( i & 0x01 );
		m_map16[i] = ( ( i & 0x40 ) >> 3 ) | ( ( i & 0x10 ) >> 2 ) | ( ( i & 0x04 ) >> 1 ) | ( i & 0x01 );
	}
	m_scanline_timer = timer_alloc(TIMER_SCANLINE_INTERRUPT);
	m_scanline_timer->adjust( machine().first_screen()->time_until_pos(0), 0, machine().first_screen()->scan_period() );
}

inline UINT8 electron_state::read_vram(  UINT16 addr )
{
	return m_ula.vram[ addr % m_ula.screen_size ];
}

inline void electron_state::electron_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = (UINT16)color;
}

UINT32 electron_state::screen_update_electron(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	int x = 0;
	int pal[16];
	int scanline = screen.vpos();
	rectangle r = cliprect;
	r.min_y = r.max_y = scanline;

	/* set up palette */
	switch( m_ula.screen_mode )
	{
	case 0: case 3: case 4: case 6: case 7: /* 2 colour mode */
		pal[0] = m_ula.current_pal[0];
		pal[1] = m_ula.current_pal[8];
		break;
	case 1: case 5: /* 4 colour mode */
		pal[0] = m_ula.current_pal[0];
		pal[1] = m_ula.current_pal[2];
		pal[2] = m_ula.current_pal[8];
		pal[3] = m_ula.current_pal[10];
		break;
	case 2: /* 16 colour mode */
		for( i = 0; i < 16; i++ )
			pal[i] = m_ula.current_pal[i];
	}

	/* draw line */
	switch( m_ula.screen_mode )
	{
	case 0:
		for( i = 0; i < 80; i++ )
		{
			UINT8 pattern = read_vram( m_ula.screen_addr + (i << 3) );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)& 1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)& 1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)& 1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)& 1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)& 1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)& 1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)& 1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)& 1] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x278;
		break;

	case 1:
		for( i = 0; i < 80; i++ )
		{
			UINT8 pattern = read_vram( m_ula.screen_addr + i * 8 );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x278;
		break;

	case 2:
		for( i = 0; i < 80; i++ )
		{
			UINT8 pattern = read_vram( m_ula.screen_addr + i * 8 );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>0]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>0]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>0]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>0]] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x278;
		break;

	case 3:
		if ( ( scanline > 249 ) || ( scanline % 10 >= 8 ) )
			bitmap.fill(7, r );
		else
		{
			for( i = 0; i < 80; i++ )
			{
				UINT8 pattern = read_vram( m_ula.screen_addr + i * 8 );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
			}
			m_ula.screen_addr++;
		}
		if ( scanline % 10 == 9 )
			m_ula.screen_addr += 0x278;
		break;

	case 4:
	case 7:
		for( i = 0; i < 40; i++ )
		{
			UINT8 pattern = read_vram( m_ula.screen_addr + i * 8 );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
			electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x138;
		break;

	case 5:
		for( i = 0; i < 40; i++ )
		{
			UINT8 pattern = read_vram( m_ula.screen_addr + i * 8 );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
			electron_plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x138;
		break;

	case 6:
		if ( ( scanline > 249 ) || ( scanline % 10 >= 8 ) )
			bitmap.fill(7, r );
		else
		{
			for( i = 0; i < 40; i++ )
			{
				UINT8 pattern = read_vram( m_ula.screen_addr + i * 8 );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
				electron_plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
			}
			m_ula.screen_addr++;
			if ( ( scanline % 10 ) == 7 )
				m_ula.screen_addr += 0x138;
		}
		break;
	}

	return 0;
}

TIMER_CALLBACK_MEMBER(electron_state::electron_scanline_interrupt)
{
	switch (machine().first_screen()->vpos())
	{
	case 43:
		electron_interrupt_handler( INT_SET, INT_RTC );
		break;
	case 199:
		electron_interrupt_handler( INT_SET, INT_DISPLAY_END );
		break;
	case 0:
		m_ula.screen_addr = m_ula.screen_start - m_ula.screen_base;
		break;
	}
}
