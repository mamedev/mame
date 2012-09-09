/***************************************************************************

  gba.c

  Driver file to handle emulation of the Nintendo Game Boy Advance.

  By R. Belmont & Harmony

  TODO:
  - Raster timing issues.  Castlevania (HOD and AOS)'s raster effects
    work if the VBlank is fired on scanline 0, else they're offset by
    the height of the vblank region.  Is scanline 0 the start of the
    visible area?

***************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "imagedev/cartslot.h"
#include "sound/dac.h"
#include "machine/intelfsh.h"
#include "audio/gb.h"
#include "includes/gba.h"
#include "rendlay.h"

#define VERBOSE_LEVEL	(0)

INLINE void verboselog(running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", cpu_get_pc(machine.device("maincpu")), buf );
	}
}

#define GBA_ATTOTIME_NORMALIZE(a)	do { while ((a).attoseconds >= ATTOSECONDS_PER_SECOND) { (a).seconds++; (a).attoseconds -= ATTOSECONDS_PER_SECOND; } } while (0)

static const UINT32 timer_clks[4] = { 16777216, 16777216/64, 16777216/256, 16777216/1024 };

static void gba_machine_stop(running_machine &machine)
{
	gba_state *state = machine.driver_data<gba_state>();

	// only do this if the cart loader detected some form of backup
	if (state->m_nvsize > 0)
	{
		device_image_interface *image = dynamic_cast<device_image_interface *>(state->m_nvimage);
		image->battery_save(state->m_nvptr, state->m_nvsize);
	}

	if (state->m_flash_size > 0)
	{
		device_image_interface *image = dynamic_cast<device_image_interface *>(state->m_nvimage);
		UINT8 *nvram = auto_alloc_array( machine, UINT8, state->m_flash_size);
		for (int i = 0; i < state->m_flash_size; i++)
		{
			nvram[i] = state->m_mFlashDev->read_raw( i);
		}
		image->battery_save( nvram, state->m_flash_size);
		auto_free( machine, nvram);
	}
}

static PALETTE_INIT( gba )
{
	UINT8 r, g, b;
	for( b = 0; b < 32; b++ )
	{
		for( g = 0; g < 32; g++ )
		{
			for( r = 0; r < 32; r++ )
			{
				palette_set_color_rgb( machine, ( b << 10 ) | ( g << 5 ) | r, pal5bit(r), pal5bit(g), pal5bit(b) );
			}
		}
	}
}

static void dma_exec(running_machine &machine, FPTR ch);

static void gba_request_irq(running_machine &machine, UINT32 int_type)
{
	gba_state *state = machine.driver_data<gba_state>();

	// set flag for later recovery
	state->m_IF |= int_type;

	// is this specific interrupt enabled?
	int_type &= state->m_IE;
	if (int_type != 0)
	{
		// master enable?
		if (state->m_IME & 1)
		{
			device_set_input_line(machine.device("maincpu"), ARM7_IRQ_LINE, ASSERT_LINE);
			device_set_input_line(machine.device("maincpu"), ARM7_IRQ_LINE, CLEAR_LINE);
		}
	}
}

static TIMER_CALLBACK( dma_complete )
{
	int ctrl;
	FPTR ch;
	static const UINT32 ch_int[4] = { INT_DMA0, INT_DMA1, INT_DMA2, INT_DMA3 };
	gba_state *state = machine.driver_data<gba_state>();

	ch = param;

//  printf("dma complete: ch %d\n", ch);

	state->m_dma_timer[ch]->adjust(attotime::never);

	ctrl = state->m_dma_regs[(ch*3)+2] >> 16;

	// IRQ
	if (ctrl & 0x4000)
	{
		gba_request_irq(machine, ch_int[ch]);
	}

	// if we're supposed to repeat, don't clear "active" and then the next vbl/hbl will retrigger us
	// always clear active for immediate DMAs though
	if (!((ctrl>>9) & 1) || ((ctrl & 0x3000) == 0))
	{
//      printf("clear active for ch %d\n", ch);
		state->m_dma_regs[(ch*3)+2] &= ~0x80000000;	// clear "active" bit
	}
	else
	{
		// if repeat, reload the count
		if ((ctrl>>9) & 1)
		{
			state->m_dma_cnt[ch] = state->m_dma_regs[(ch*3)+2]&0xffff;

			// if increment & reload mode, reload the destination
			if (((ctrl>>5)&3) == 3)
			{
				state->m_dma_dst[ch] = state->m_dma_regs[(ch*3)+1];
			}
		}
	}
}

static void dma_exec(running_machine &machine, FPTR ch)
{
	int i, cnt;
	int ctrl;
	int srcadd, dstadd;
	UINT32 src, dst;
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	gba_state *state = machine.driver_data<gba_state>();

	src = state->m_dma_src[ch];
	dst = state->m_dma_dst[ch];
	ctrl = state->m_dma_regs[(ch*3)+2] >> 16;
	srcadd = state->m_dma_srcadd[ch];
	dstadd = state->m_dma_dstadd[ch];

	cnt = state->m_dma_cnt[ch];
	if (!cnt)
	{
		if (ch == 3)
		{
			cnt = 0x10000;
		}
		else
		{
			cnt = 0x4000;
		}
	}

	// override special parameters
	if ((ctrl & 0x3000) == 0x3000)		// special xfer mode
	{
		switch (ch)
		{
			case 1:			// Ch 1&2 are for audio DMA
			case 2:
				dstadd = 2;	// don't increment destination
				cnt = 4;	// always transfer 4 32-bit words
				ctrl |= 0x400;	// always 32-bit
				break;

			case 3:
				printf("Unsupported DMA 3 special mode\n");
				break;
		}
	}
	else
	{
//      if (dst >= 0x6000000 && dst <= 0x6017fff)
//          printf("DMA exec: ch %d from %08x to %08x, mode %04x, count %04x (PC %x) (%s)\n", (int)ch, src, dst, ctrl, cnt, activecpu_get_pc(), ((ctrl>>10) & 1) ? "32" : "16");
	}

	for (i = 0; i < cnt; i++)
	{
		if ((ctrl>>10) & 1)
		{
			src &= 0xfffffffc;
			dst &= 0xfffffffc;

			// 32-bit
			space->write_dword(dst, space->read_dword(src));
			switch (dstadd)
			{
				case 0:	// increment
					dst += 4;
					break;
				case 1:	// decrement
					dst -= 4;
					break;
				case 2:	// don't move
					break;
				case 3:	// increment and reload
					dst += 4;
					break;
			}
			switch (srcadd)
			{
				case 0:	// increment
					src += 4;
					break;
				case 1:	// decrement
					src -= 4;
					break;
				case 2:	// don't move
					break;
				case 3:	// not used ("Metal Max 2 Kai" expects no increment/decrement)
					break;
			}
		}
		else
		{
			src &= 0xfffffffe;
			dst &= 0xfffffffe;

			// 16-bit
			space->write_word(dst, space->read_word(src));
			switch (dstadd)
			{
				case 0:	// increment
					dst += 2;
					break;
				case 1:	// decrement
					dst -= 2;
					break;
				case 2:	// don't move
					break;
				case 3:	// increment and reload
					dst += 2;
					break;
			}
			switch (srcadd)
			{
				case 0:	// increment
					src += 2;
					break;
				case 1:	// decrement
					src -= 2;
					break;
				case 2:	// don't move
					break;
				case 3:	// not used (see note in 32-bit version above)
					break;
			}
		}
	}

	state->m_dma_src[ch] = src;
	state->m_dma_dst[ch] = dst;

//  printf("settng DMA timer %d for %d cycs (tmr %x)\n", ch, cnt, (UINT32)state->m_dma_timer[ch]);
//  state->m_dma_timer[ch]->adjust(ATTOTIME_IN_CYCLES(0, cnt), ch);
	dma_complete(machine, NULL, ch);
}

static void audio_tick(running_machine &machine, int ref)
{
	gba_state *state = machine.driver_data<gba_state>();

	if (!(state->m_SOUNDCNT_X & 0x80))
	{
		return;
	}

	if (!ref)
	{
		if (state->m_fifo_a_ptr != state->m_fifo_a_in)
		{
			if (state->m_fifo_a_ptr == 17)
			{
				state->m_fifo_a_ptr = 0;
			}

			if (state->m_SOUNDCNT_H & 0x200)
			{
				dac_device *dac = machine.device<dac_device>("direct_a_left");

				dac->write_signed8(state->m_fifo_a[state->m_fifo_a_ptr]^0x80);
			}
			if (state->m_SOUNDCNT_H & 0x100)
			{
				dac_device *dac = machine.device<dac_device>("direct_a_right");

				dac->write_signed8(state->m_fifo_a[state->m_fifo_a_ptr]^0x80);
			}
			state->m_fifo_a_ptr++;
		}

		// fifo empty?
		if (state->m_fifo_a_ptr == state->m_fifo_a_in)
		{
			// is a DMA set up to feed us?
			if ((state->m_dma_regs[(1*3)+1] == 0x40000a0) && ((state->m_dma_regs[(1*3)+2] & 0x30000000) == 0x30000000))
			{
				// channel 1 it is
				dma_exec(machine, 1);
			}
			if ((state->m_dma_regs[(2*3)+1] == 0x40000a0) && ((state->m_dma_regs[(2*3)+2] & 0x30000000) == 0x30000000))
			{
				// channel 2 it is
				dma_exec(machine, 2);
			}
		}
	}
	else
	{
		if (state->m_fifo_b_ptr != state->m_fifo_b_in)
		{
			if (state->m_fifo_b_ptr == 17)
			{
				state->m_fifo_b_ptr = 0;
			}

			if (state->m_SOUNDCNT_H & 0x2000)
			{
				dac_device *dac = machine.device<dac_device>("direct_b_left");

				dac->write_signed8(state->m_fifo_b[state->m_fifo_b_ptr]^0x80);
			}
			if (state->m_SOUNDCNT_H & 0x1000)
			{
				dac_device *dac = machine.device<dac_device>("direct_b_right");

				dac->write_signed8(state->m_fifo_b[state->m_fifo_b_ptr]^0x80);
			}
			state->m_fifo_b_ptr++;
		}

		if (state->m_fifo_b_ptr == state->m_fifo_b_in)
		{
			// is a DMA set up to feed us?
			if ((state->m_dma_regs[(1*3)+1] == 0x40000a4) && ((state->m_dma_regs[(1*3)+2] & 0x30000000) == 0x30000000))
			{
				// channel 1 it is
				dma_exec(machine, 1);
			}
			if ((state->m_dma_regs[(2*3)+1] == 0x40000a4) && ((state->m_dma_regs[(2*3)+2] & 0x30000000) == 0x30000000))
			{
				// channel 2 it is
				dma_exec(machine, 2);
			}
		}
	}
}

static TIMER_CALLBACK(timer_expire)
{
	static const UINT32 tmr_ints[4] = { INT_TM0_OVERFLOW, INT_TM1_OVERFLOW, INT_TM2_OVERFLOW, INT_TM3_OVERFLOW };
	FPTR tmr = (FPTR) param;
	gba_state *state = machine.driver_data<gba_state>();

//  printf("Timer %d expired, SOUNDCNT_H %04x\n", tmr, state->m_SOUNDCNT_H);

	// "The reload value is copied into the counter only upon following two situations: Automatically upon timer overflows,"
	// "or when the timer start bit becomes changed from 0 to 1."
	if (state->m_timer_recalc[tmr] != 0)
	{
		double rate, clocksel, final;
		attotime time;
		state->m_timer_recalc[tmr] = 0;
		state->m_timer_regs[tmr] = (state->m_timer_regs[tmr] & 0xFFFF0000) | (state->m_timer_reload[tmr] & 0x0000FFFF);
		rate = 0x10000 - (state->m_timer_regs[tmr] & 0xffff);
		clocksel = timer_clks[(state->m_timer_regs[tmr] >> 16) & 3];
		final = clocksel / rate;
		state->m_timer_hz[tmr] = final;
		time = attotime::from_hz(final);
		GBA_ATTOTIME_NORMALIZE(time);
		state->m_tmr_timer[tmr]->adjust(time, tmr, time);
	}

	// check if timers 0 or 1 are feeding directsound
	if (tmr == 0)
	{
		if ((state->m_SOUNDCNT_H & 0x400) == 0)
		{
			audio_tick(machine, 0);
		}

		if ((state->m_SOUNDCNT_H & 0x4000) == 0)
		{
			audio_tick(machine, 1);
		}
	}

	if (tmr == 1)
	{
		if ((state->m_SOUNDCNT_H & 0x400) == 0x400)
		{
			audio_tick(machine, 0);
		}

		if ((state->m_SOUNDCNT_H & 0x4000) == 0x4000)
		{
			audio_tick(machine, 1);
		}
	}

	// Handle count-up timing
	switch (tmr)
	{
	case 0:
		if (state->m_timer_regs[1] & 0x40000)
		{
			state->m_timer_regs[1] = (( ( state->m_timer_regs[1] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (state->m_timer_regs[1] & 0xffff0000);
			if( ( state->m_timer_regs[1] & 0x0000ffff ) == 0 )
			{
				state->m_timer_regs[1] |= state->m_timer_reload[1];
				if( ( state->m_timer_regs[1] & 0x400000 ) && ( state->m_IME != 0 ) )
				{
					gba_request_irq( machine, tmr_ints[1] );
				}
				if( ( state->m_timer_regs[2] & 0x40000 ) )
				{
					state->m_timer_regs[2] = (( ( state->m_timer_regs[2] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (state->m_timer_regs[2] & 0xffff0000);
					if( ( state->m_timer_regs[2] & 0x0000ffff ) == 0 )
					{
						state->m_timer_regs[2] |= state->m_timer_reload[2];
						if( ( state->m_timer_regs[2] & 0x400000 ) && ( state->m_IME != 0 ) )
						{
							gba_request_irq( machine, tmr_ints[2] );
						}
						if( ( state->m_timer_regs[3] & 0x40000 ) )
						{
							state->m_timer_regs[3] = (( ( state->m_timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (state->m_timer_regs[3] & 0xffff0000);
							if( ( state->m_timer_regs[3] & 0x0000ffff ) == 0 )
							{
								state->m_timer_regs[3] |= state->m_timer_reload[3];
								if( ( state->m_timer_regs[3] & 0x400000 ) && ( state->m_IME != 0 ) )
								{
									gba_request_irq( machine, tmr_ints[3] );
								}
							}
						}
					}
				}
			}
		}
		break;
	case 1:
		if (state->m_timer_regs[2] & 0x40000)
		{
			state->m_timer_regs[2] = (( ( state->m_timer_regs[2] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (state->m_timer_regs[2] & 0xffff0000);
			if( ( state->m_timer_regs[2] & 0x0000ffff ) == 0 )
			{
				state->m_timer_regs[2] |= state->m_timer_reload[2];
				if( ( state->m_timer_regs[2] & 0x400000 ) && ( state->m_IME != 0 ) )
				{
					gba_request_irq( machine, tmr_ints[2] );
				}
				if( ( state->m_timer_regs[3] & 0x40000 ) )
				{
					state->m_timer_regs[3] = (( ( state->m_timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (state->m_timer_regs[3] & 0xffff0000);
					if( ( state->m_timer_regs[3] & 0x0000ffff ) == 0 )
					{
						state->m_timer_regs[3] |= state->m_timer_reload[3];
						if( ( state->m_timer_regs[3] & 0x400000 ) && ( state->m_IME != 0 ) )
						{
							gba_request_irq( machine, tmr_ints[3] );
						}
					}
				}
			}
		}
		break;
	case 2:
		if (state->m_timer_regs[3] & 0x40000)
		{
			state->m_timer_regs[3] = (( ( state->m_timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (state->m_timer_regs[3] & 0xffff0000);
			if( ( state->m_timer_regs[3] & 0x0000ffff ) == 0 )
			{
				state->m_timer_regs[3] |= state->m_timer_reload[3];
				if( ( state->m_timer_regs[3] & 0x400000 ) && ( state->m_IME != 0 ) )
				{
					gba_request_irq( machine, tmr_ints[3] );
				}
			}
		}
		break;
	}

	// are we supposed to IRQ?
	if ((state->m_timer_regs[tmr] & 0x400000) && (state->m_IME != 0))
	{
		gba_request_irq(machine, tmr_ints[tmr]);
	}
}

static TIMER_CALLBACK(handle_irq)
{
	gba_state *state = machine.driver_data<gba_state>();

	gba_request_irq(machine, state->m_IF);

	state->m_irq_timer->adjust(attotime::never);
}

READ32_MEMBER(gba_state::gba_io_r)
{
	UINT32 retval = 0;
	device_t *gb_device = machine().device("custom");

	switch( offset )
	{
		case 0x0000/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: DISPCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_DISPCNT );
				retval |= m_DISPCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: Green Swap (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_GRNSWAP );
				retval |= m_GRNSWAP << 16;
			}
			break;
		case 0x0004/4:
			retval = (m_DISPSTAT & 0xffff) | (machine().primary_screen->vpos()<<16);
			break;
		case 0x0008/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG0CNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_BG0CNT );
				retval |= m_BG0CNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG1CNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_BG1CNT );
				retval |= m_BG1CNT << 16;
			}
			break;
		case 0x000c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2CNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_BG2CNT );
				retval |= m_BG2CNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3CNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_BG3CNT );
				retval |= m_BG3CNT << 16;
			}
			break;
		case 0x0010/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG0HOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG0VOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0014/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG1HOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG1VOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0018/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2HOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2VOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x001c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3HOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3VOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0020/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2PA (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2PB (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0024/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2PC (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2PD (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0028/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2X_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2X_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x002c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2Y_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG2Y_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0030/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3PA (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3PB (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0034/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3PC (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3PD (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0038/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3X_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3X_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x003c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3Y_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BG3Y_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0040/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: WIN0H (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: WIN1H (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0044/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: WIN0V (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: WIN1V (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0048/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: WININ (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_WININ );
				retval |= m_WININ;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: WINOUT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_WINOUT );
				retval |= m_WINOUT << 16;
			}
			break;
		case 0x004c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: MOSAIC (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0050/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BLDCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_BLDCNT );
				retval |= m_BLDCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BLDALPHA (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_BLDALPHA );
				retval |= m_BLDALPHA << 16;
			}
			break;
		case 0x0054/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: BLDY (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0058/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x005c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0060/4:
			retval = gb_sound_r(gb_device, 0) | gb_sound_r(gb_device, 1)<<16 | gb_sound_r(gb_device, 2)<<24;
			break;
		case 0x0064/4:
			retval = gb_sound_r(gb_device, 3) | gb_sound_r(gb_device, 4)<<8;
			break;
		case 0x0068/4:
			retval = gb_sound_r(gb_device, 6) | gb_sound_r(gb_device, 7)<<8;
			break;
		case 0x006c/4:
			retval = gb_sound_r(gb_device, 8) | gb_sound_r(gb_device, 9)<<8;
			break;
		case 0x0070/4:
			retval = gb_sound_r(gb_device, 0xa) | gb_sound_r(gb_device, 0xb)<<16 | gb_sound_r(gb_device, 0xc)<<24;
			break;
		case 0x0074/4:
			retval = gb_sound_r(gb_device, 0xd) | gb_sound_r(gb_device, 0xe)<<8;
			break;
		case 0x0078/4:
			retval = gb_sound_r(gb_device, 0x10) | gb_sound_r(gb_device, 0x11)<<8;
			break;
		case 0x007c/4:
			retval = gb_sound_r(gb_device, 0x12) | gb_sound_r(gb_device, 0x13)<<8;
			break;
		case 0x0080/4:
			retval = gb_sound_r(gb_device, 0x14) | gb_sound_r(gb_device, 0x15)<<8;
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: SOUNDCNT_H (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_SOUNDCNT_H );
				retval |= m_SOUNDCNT_H << 16;
			}
			break;
		case 0x0084/4:
			retval = gb_sound_r(gb_device, 0x16);
			break;
		case 0x0088/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: SOUNDBIAS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_SOUNDBIAS );
				retval |= m_SOUNDBIAS;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0090/4:
			retval = gb_wave_r(gb_device, 0) | gb_wave_r(gb_device, 1)<<8 | gb_wave_r(gb_device, 2)<<16 | gb_wave_r(gb_device, 3)<<24;
			break;
		case 0x0094/4:
			retval = gb_wave_r(gb_device, 4) | gb_wave_r(gb_device, 5)<<8 | gb_wave_r(gb_device, 6)<<16 | gb_wave_r(gb_device, 7)<<24;
			break;
		case 0x0098/4:
			retval = gb_wave_r(gb_device, 8) | gb_wave_r(gb_device, 9)<<8 | gb_wave_r(gb_device, 10)<<16 | gb_wave_r(gb_device, 11)<<24;
			break;
		case 0x009c/4:
			retval = gb_wave_r(gb_device, 12) | gb_wave_r(gb_device, 13)<<8 | gb_wave_r(gb_device, 14)<<16 | gb_wave_r(gb_device, 15)<<24;
			break;
		case 0x00a0/4:
		case 0x00a4/4:
			retval = 0;	// (does this actually do anything on real h/w?)
			break;
		case 0x00b0/4:
		case 0x00b4/4:
		case 0x00b8/4:
		case 0x00bc/4:
		case 0x00c0/4:
		case 0x00c4/4:
		case 0x00c8/4:
		case 0x00cc/4:
		case 0x00d0/4:
		case 0x00d4/4:
		case 0x00d8/4:
		case 0x00dc/4:
			{
				// no idea why here, but it matches VBA better
				// note: this suspicious piece of code crashes "Buffy The Vampire Slayer" (08008DB4) and "The Ant Bully", so disable it for now
				#if 0
				if (((offset-0xb0/4) % 3) == 2)
				{
					retval = m_dma_regs[offset-(0xb0/4)] & 0xff000000;
				}
				else
				#endif

				retval = m_dma_regs[offset-(0xb0/4)];
			}
			break;
		case 0x0100/4:
		case 0x0104/4:
		case 0x0108/4:
		case 0x010c/4:
			{
				UINT32 elapsed;
				double time, ticks;
				int timer = offset-(0x100/4);

//              printf("Read timer reg %x (PC=%x)\n", timer, cpu_get_pc(&space.device()));

				// update times for
				if (m_timer_regs[timer] & 0x800000)
				{
					if (m_timer_regs[timer] & 0x00040000)
					{
						elapsed = m_timer_regs[timer] & 0xffff;
					}
					else
					{

					time = m_tmr_timer[timer]->elapsed().as_double();

					ticks = (double)(0x10000 - (m_timer_regs[timer] & 0xffff));

//                  printf("time %f ticks %f 1/hz %f\n", time, ticks, 1.0 / m_timer_hz[timer]);

					time *= ticks;
					time /= (1.0 / m_timer_hz[timer]);

					elapsed = (UINT32)time;

					}

//                  printf("elapsed = %x\n", elapsed);
				}
				else
				{
//                  printf("Reading inactive timer!\n");
					elapsed = 0;
				}

				retval = (m_timer_regs[timer] & 0xffff0000) | (elapsed & 0xffff);
			}
			break;
		case 0x0120/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: SIOMULTI0 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_SIOMULTI0 );
				retval |= m_SIOMULTI0;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: SIOMULTI1 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_SIOMULTI1 );
				retval |= m_SIOMULTI1 << 16;
			}
			break;
		case 0x0124/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: SIOMULTI2 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_SIOMULTI2 );
				retval |= m_SIOMULTI2;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: SIOMULTI3 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_SIOMULTI3 );
				retval |= m_SIOMULTI3 << 16;
			}
			break;
		case 0x0128/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: SIOCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_SIOCNT );
				retval |= m_SIOCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: SIODATA8 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_SIODATA8 );
				retval |= m_SIODATA8 << 16;
			}
			break;
		case 0x0130/4:
			if( (mem_mask) & 0x0000ffff )	// KEYINPUT
			{
				retval = ioport("IN0")->read();
			}
			else if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: KEYCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_KEYCNT );
				retval |= m_KEYCNT << 16;
			}
			break;
		case 0x0134/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: RCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_RCNT );
				retval |= m_RCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: IR (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0140/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: JOYCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_JOYCNT );
				retval |= m_JOYCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0150/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: JOY_RECV_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_JOY_RECV & 0x0000ffff );
				retval |= m_JOY_RECV & 0x0000ffff;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: JOY_RECV_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, ( m_JOY_RECV & 0xffff0000 ) >> 16 );
				retval |= m_JOY_RECV & 0xffff0000;
			}
			break;
		case 0x0154/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: JOY_TRANS_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_JOY_TRANS & 0x0000ffff );
				retval |= m_JOY_TRANS & 0x0000ffff;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: JOY_TRANS_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, ( m_JOY_TRANS & 0xffff0000 ) >> 16 );
				retval |= m_JOY_TRANS & 0xffff0000;
			}
			break;
		case 0x0158/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: JOYSTAT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_JOYSTAT );
				retval |= m_JOYSTAT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0200/4:
			if( (mem_mask) & 0x0000ffff )
			{
//              printf("Read: IE (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_IE );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: IF (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, m_IF );
			}

			retval = m_IE | (m_IF<<16);
			break;
		case 0x0204/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: WAITCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_WAITCNT );
				retval |= m_WAITCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0208/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Read: IME (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), m_IME );
				retval |= m_IME;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0300/4:
			retval = m_HALTCNT << 8;
			break;
		default:
//          verboselog(machine(), 0, "Unknown GBA I/O register Read: %08x (%08x)\n", 0x04000000 + ( offset << 2 ), ~mem_mask );
			break;
	}
	return retval;
}

WRITE32_MEMBER(gba_state::gba_io_w)
{
	device_t *gb_device = machine().device("custom");
	switch( offset )
	{
		case 0x0000/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: DISPCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_DISPCNT = ( m_DISPCNT & ~mem_mask ) | ( data & mem_mask );
				if(m_DISPCNT & (DISPCNT_WIN0_EN | DISPCNT_WIN1_EN))
				{
					m_windowOn = 1;
				}
				else
				{
					m_windowOn = 0;
				}
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: Green Swap (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_GRNSWAP = ( m_GRNSWAP & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0004/4:
			COMBINE_DATA(&m_DISPSTAT);
			break;
		case 0x0008/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG0CNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG0CNT = ( m_BG0CNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG1CNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG1CNT = ( m_BG1CNT & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x000c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2CNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG2CNT = ( m_BG2CNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3CNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG3CNT = ( m_BG3CNT & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0010/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG0HOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG0HOFS = ( m_BG0HOFS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG0VOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG0VOFS = ( m_BG0VOFS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0014/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG1HOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG1HOFS = ( m_BG1HOFS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG1VOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG1VOFS = ( m_BG1VOFS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0018/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2HOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG2HOFS = ( m_BG2HOFS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2VOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG2VOFS = ( m_BG2VOFS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x001c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3HOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG3HOFS = ( m_BG3HOFS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3VOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG3VOFS = ( m_BG3VOFS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0020/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2PA (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG2PA = ( m_BG2PA & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2PB (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG2PB = ( m_BG2PB & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0024/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2PC (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG2PC = ( m_BG2PC & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2PD (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG2PD = ( m_BG2PD & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0028/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2X_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2X_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			COMBINE_DATA(&m_BG2X);
			m_gfxBG2Changed |= 1;
			break;
		case 0x002c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2Y_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG2Y_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			COMBINE_DATA(&m_BG2Y);
			m_gfxBG2Changed |= 2;
			break;
		case 0x0030/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3PA (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG3PA = ( m_BG3PA & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3PB (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG3PB = ( m_BG3PB & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0034/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3PC (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BG3PC = ( m_BG3PC & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3PD (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BG3PD = ( m_BG3PD & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0038/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3X_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3X_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			COMBINE_DATA(&m_BG3X);
			m_gfxBG3Changed |= 1;
			break;
		case 0x003c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3Y_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BG3Y_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			COMBINE_DATA(&m_BG3Y);
			m_gfxBG3Changed |= 2;
			break;
		case 0x0040/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: WIN0H (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_WIN0H = ( m_WIN0H & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: WIN1H (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_WIN1H = ( m_WIN1H & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0044/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: WIN0V (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_WIN0V = ( m_WIN0V & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: WIN1V (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_WIN1V = ( m_WIN1V & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0048/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: WININ (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_WININ = ( m_WININ & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: WINOUT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_WINOUT = ( m_WINOUT & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x004c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: MOSAIC (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_MOSAIC = ( m_MOSAIC & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x0050/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BLDCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BLDCNT = ( m_BLDCNT & ~mem_mask ) | ( data & mem_mask );
				if(m_BLDCNT & BLDCNT_SFX)
				{
					m_fxOn = 1;
				}
				else
				{
					m_fxOn = 0;
				}
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BLDALPHA (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				m_BLDALPHA = ( m_BLDALPHA & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0054/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: BLDY (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_BLDY = ( m_BLDY & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x0058/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x005c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x0060/4:
			if( (mem_mask) & 0x000000ff )	// SOUNDCNTL
			{
				gb_sound_w(gb_device, 0, data);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_sound_w(gb_device, 1, data>>16);	// SOUND1CNT_H
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_sound_w(gb_device, 2, data>>24);
			}
			break;
		case 0x0064/4:
			if( (mem_mask) & 0x000000ff )	// SOUNDCNTL
			{
				gb_sound_w(gb_device, 3, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(gb_device, 4, data>>8);	// SOUND1CNT_H
			}
			break;
		case 0x0068/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(gb_device, 6, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(gb_device, 7, data>>8);
			}
			break;
		case 0x006c/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(gb_device, 8, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(gb_device, 9, data>>8);
			}
			break;
		case 0x0070/4:	//SND3CNTL and H
			if( (mem_mask) & 0x000000ff )	// SOUNDCNTL
			{
				gb_sound_w(gb_device, 0xa, data);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_sound_w(gb_device, 0xb, data>>16);	// SOUND1CNT_H
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_sound_w(gb_device, 0xc, data>>24);
			}
			break;
		case 0x0074/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(gb_device, 0xd, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(gb_device, 0xe, data>>8);
			}
			break;
		case 0x0078/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(gb_device, 0x10, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(gb_device, 0x11, data>>8);
			}
			break;
		case 0x007c/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(gb_device, 0x12, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(gb_device, 0x13, data>>8);
			}
			break;
		case 0x0080/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(gb_device, 0x14, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(gb_device, 0x15, data>>8);
			}

			if ((mem_mask) & 0xffff0000)
			{
				data >>= 16;
				m_SOUNDCNT_H = data;

				// DAC A reset?
				if (data & 0x0800)
				{
					dac_device *gb_a_l = machine().device<dac_device>("direct_a_left");
					dac_device *gb_a_r = machine().device<dac_device>("direct_a_right");

					m_fifo_a_ptr = 17;
					m_fifo_a_in = 17;
					gb_a_l->write_signed8(0x80);
					gb_a_r->write_signed8(0x80);
				}

				// DAC B reset?
				if (data & 0x8000)
				{
					dac_device *gb_b_l = machine().device<dac_device>("direct_b_left");
					dac_device *gb_b_r = machine().device<dac_device>("direct_b_right");

					m_fifo_b_ptr = 17;
					m_fifo_b_in = 17;
					gb_b_l->write_signed8(0x80);
					gb_b_r->write_signed8(0x80);
				}
			}
			break;
		case 0x0084/4:
			if( (mem_mask) & 0x000000ff )
			{
				dac_device *gb_a_l = machine().device<dac_device>("direct_a_left");
				dac_device *gb_a_r = machine().device<dac_device>("direct_a_right");
				dac_device *gb_b_l = machine().device<dac_device>("direct_b_left");
				dac_device *gb_b_r = machine().device<dac_device>("direct_b_right");

				gb_sound_w(gb_device, 0x16, data);
				if ((data & 0x80) && !(m_SOUNDCNT_X & 0x80))
				{
					m_fifo_a_ptr = m_fifo_a_in = 17;
					m_fifo_b_ptr = m_fifo_b_in = 17;
					gb_a_l->write_signed8(0x80);
					gb_a_r->write_signed8(0x80);
					gb_b_l->write_signed8(0x80);
					gb_b_r->write_signed8(0x80);
				}
				m_SOUNDCNT_X = data;
			}
			break;
		case 0x0088/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: SOUNDBIAS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				m_SOUNDBIAS = ( m_SOUNDBIAS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x0090/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_wave_w(gb_device, 0, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_wave_w(gb_device, 1, data>>8);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_wave_w(gb_device, 2, data>>16);
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_wave_w(gb_device, 3, data>>24);
			}
			break;
		case 0x0094/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_wave_w(gb_device, 4, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_wave_w(gb_device, 5, data>>8);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_wave_w(gb_device, 6, data>>16);
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_wave_w(gb_device, 7, data>>24);
			}
			break;
		case 0x0098/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_wave_w(gb_device, 8, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_wave_w(gb_device, 9, data>>8);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_wave_w(gb_device, 0xa, data>>16);
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_wave_w(gb_device, 0xb, data>>24);
			}
			break;
		case 0x009c/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_wave_w(gb_device, 0xc, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_wave_w(gb_device, 0xd, data>>8);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_wave_w(gb_device, 0xe, data>>16);
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_wave_w(gb_device, 0xf, data>>24);
			}
			break;
		case 0x00a0/4:
			m_fifo_a_in %= 17;
			m_fifo_a[m_fifo_a_in++] = (data)&0xff;
			m_fifo_a_in %= 17;
			m_fifo_a[m_fifo_a_in++] = (data>>8)&0xff;
			m_fifo_a_in %= 17;
			m_fifo_a[m_fifo_a_in++] = (data>>16)&0xff;
			m_fifo_a_in %= 17;
			m_fifo_a[m_fifo_a_in++] = (data>>24)&0xff;
			break;
		case 0x00a4/4:
			m_fifo_b_in %= 17;
			m_fifo_b[m_fifo_b_in++] = (data)&0xff;
			m_fifo_b_in %= 17;
			m_fifo_b[m_fifo_b_in++] = (data>>8)&0xff;
			m_fifo_b_in %= 17;
			m_fifo_b[m_fifo_b_in++] = (data>>16)&0xff;
			m_fifo_b_in %= 17;
			m_fifo_b[m_fifo_b_in++] = (data>>24)&0xff;
			break;
		case 0x00b0/4:
		case 0x00b4/4:
		case 0x00b8/4:

		case 0x00bc/4:
		case 0x00c0/4:
		case 0x00c4/4:

		case 0x00c8/4:
		case 0x00cc/4:
		case 0x00d0/4:

		case 0x00d4/4:
		case 0x00d8/4:
		case 0x00dc/4:
			{
				int ch;

				offset -= (0xb0/4);

				ch = offset / 3;

//              printf("%08x: DMA(%d): %x to reg %d (mask %08x)\n", activecpu_get_pc(), ch, data, offset%3, ~mem_mask);

				if (((offset % 3) == 2) && ((~mem_mask & 0xffff0000) == 0))
				{
					int ctrl = data>>16;

					// Note: Metroid Fusion fails if we enforce the "rising edge" requirement... (who wrote this note?)

					// Note: Caesar's Palace Advance fails if we DO NOT enforce the "rising edge" requirement
					// (value @ 0x3003F9C is accidentally incremented because DMA completion interrupt is accidentally triggered @ 08002F2A)

					// retrigger/restart on a rising edge.
					// also reload internal regs
					if ((ctrl & 0x8000) && !(m_dma_regs[offset] & 0x80000000))
					{
						m_dma_src[ch] = m_dma_regs[(ch*3)+0];
						m_dma_dst[ch] = m_dma_regs[(ch*3)+1];
						m_dma_srcadd[ch] = (ctrl>>7)&3;
						m_dma_dstadd[ch] = (ctrl>>5)&3;

						COMBINE_DATA(&m_dma_regs[offset]);
						m_dma_cnt[ch] = m_dma_regs[(ch*3)+2]&0xffff;

						// immediate start
						if ((ctrl & 0x3000) == 0)
						{
							dma_exec(machine(), ch);
							return;
						}
					}
				}

				COMBINE_DATA(&m_dma_regs[offset]);
			}
			break;
		case 0x0100/4:
		case 0x0104/4:
		case 0x0108/4:
		case 0x010c/4:
			{
				double rate, clocksel;
				UINT32 old_timer_regs;

				offset -= (0x100/4);

				old_timer_regs = m_timer_regs[offset];

				m_timer_regs[offset] = (m_timer_regs[offset] & ~(mem_mask & 0xFFFF0000)) | (data & (mem_mask & 0xFFFF0000));

//              printf("%x to timer %d (mask %x PC %x)\n", data, offset, ~mem_mask, cpu_get_pc(&space.device()));

				if (ACCESSING_BITS_0_15)
				{
					m_timer_reload[offset] = ((m_timer_reload[offset] & ~mem_mask) | (data & mem_mask)) & 0x0000FFFF;
					m_timer_recalc[offset] = 1;
				}

				// enabling this timer?
				if ((ACCESSING_BITS_16_31) && (data & 0x800000))
				{
					double final;

					if ((old_timer_regs & 0x00800000) == 0) // start bit 0 -> 1
					{
						m_timer_regs[offset] = (m_timer_regs[offset] & 0xFFFF0000) | (m_timer_reload[offset] & 0x0000FFFF);
					}

					rate = 0x10000 - (m_timer_regs[offset] & 0xffff);

					clocksel = timer_clks[(m_timer_regs[offset] >> 16) & 3];

					final = clocksel / rate;

					m_timer_hz[offset] = final;

					m_timer_recalc[offset] = 0;

//                  printf("Enabling timer %d @ %f Hz\n", offset, final);

					// enable the timer
					if( !(data & 0x40000) ) // if we're not in Count-Up mode
					{
						attotime time = attotime::from_hz(final);
						GBA_ATTOTIME_NORMALIZE(time);
						m_tmr_timer[offset]->adjust(time, offset, time);
					}
				}
			}
			break;
		case 0x0120/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: SIOMULTI0 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				m_SIOMULTI0 = ( m_SIOMULTI0 & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: SIOMULTI1 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
				m_SIOMULTI1 = ( m_SIOMULTI1 & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0124/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: SIOMULTI2 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				m_SIOMULTI2 = ( m_SIOMULTI2 & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: SIOMULTI3 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
				m_SIOMULTI3 = ( m_SIOMULTI3 & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0128/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: SIOCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				// normal mode ?
				if (((m_RCNT & 0x8000) == 0) && ((data & 0x2000) == 0))
				{
					// start ?
					if (((m_SIOCNT & 0x0080) == 0) && ((data & 0x0080) != 0))
					{
						data &= ~0x0080;
						// request interrupt ?
						if (data & 0x4000)
						{
							gba_request_irq( machine(), INT_SIO);
						}
					}
				}
				m_SIOCNT = ( m_SIOCNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: SIODATA8 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
				m_SIODATA8 = ( m_SIODATA8 & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0130/4:
			if( (mem_mask) & 0xffff0000 )
			{
//              printf("KEYCNT = %04x\n", data>>16);
				verboselog(machine(), 2, "GBA IO Register Write: KEYCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
				m_KEYCNT = ( m_KEYCNT & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0134/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: RCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				m_RCNT = ( m_RCNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: IR (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
				m_IR = ( m_IR & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0140/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: JOYCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				m_JOYCNT = ( m_JOYCNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
			}
			break;
		case 0x0150/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: JOY_RECV_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				m_JOY_RECV = ( m_JOY_RECV & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: JOY_RECV_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
				m_JOY_RECV = ( m_JOY_RECV & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0154/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: JOY_TRANS_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				m_JOY_TRANS = ( m_JOY_TRANS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: JOY_TRANS_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
				m_JOY_TRANS = ( m_JOY_TRANS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0158/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: JOYSTAT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				m_JOYSTAT = ( m_JOYSTAT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
			}
			break;
		case 0x0200/4:
			if( (mem_mask) & 0x0000ffff )
			{
//              printf("IE (%08x) = %04x raw %x (%08x) (scan %d PC %x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, data, ~mem_mask, machine.primary_screen->vpos(), cpu_get_pc(&space.device()));
				m_IE = ( m_IE & ~mem_mask ) | ( data & mem_mask );
#if 0
				if (m_IE & m_IF)
				{
					gba_request_irq(machine(), m_IF);
				}
#endif
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: IF (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ) + 2, ( data & mem_mask ) >> 16, ~mem_mask );
				m_IF &= ~( ( data & mem_mask ) >> 16 );

				// if we still have interrupts, yank the IRQ line again
				if (m_IF)
				{
					m_irq_timer->adjust(machine().device<cpu_device>("maincpu")->clocks_to_attotime(120));
				}
			}
			break;
		case 0x0204/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 2, "GBA IO Register Write: WAITCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				m_WAITCNT = ( m_WAITCNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
			}
			break;
		case 0x0208/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog(machine(), 3, "GBA IO Register Write: IME (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				m_IME = ( m_IME & ~mem_mask ) | ( data & mem_mask );
				if (m_IF)
				{
					m_irq_timer->adjust(attotime::zero);
				}
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 3, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
			}
			break;
		case 0x0300/4:
			if( (mem_mask) & 0x0000ffff )
			{
				if( (mem_mask) & 0x000000ff )
				{
					verboselog(machine(), 2, "GBA IO Register Write: POSTFLG (%08x) = %02x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x000000ff, ~mem_mask );
					m_POSTFLG = data & 0x000000ff;
				}
				else
				{
					m_HALTCNT = data & 0x000000ff;

					// either way, wait for an IRQ
					device_spin_until_interrupt(machine().device("maincpu"));
				}
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog(machine(), 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		default:
//          verboselog(machine, 0, "Unknown GBA I/O register write: %08x = %08x (%08x)\n", 0x04000000 + ( offset << 2 ), data, ~mem_mask );
			break;
	}
}

INLINE UINT32 COMBINE_DATA32_16(UINT32 prev, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&prev);
	switch(mem_mask)
	{
		case 0x000000ff:
			prev &= 0xffff00ff;
			prev |= data << 8;
			break;
		case 0x0000ff00:
			prev &= 0xffffff00;
			prev |= data >> 8;
			break;
		case 0x00ff0000:
			prev &= 0x00ffffff;
			prev |= data << 8;
			break;
		case 0xff000000:
			prev &= 0xff00ffff;
			prev |= data >> 8;
			break;
		default:
			break;
	}
	return prev;
}

WRITE32_MEMBER(gba_state::gba_pram_w)
{
	m_gba_pram[offset] = COMBINE_DATA32_16(m_gba_pram[offset], data, mem_mask);
}

WRITE32_MEMBER(gba_state::gba_vram_w)
{
	m_gba_vram[offset] = COMBINE_DATA32_16(m_gba_vram[offset], data, mem_mask);
}

WRITE32_MEMBER(gba_state::gba_oam_w)
{
	m_gba_oam[offset] = COMBINE_DATA32_16(m_gba_oam[offset], data, mem_mask);
}

READ32_MEMBER(gba_state::gba_bios_r)
{
	UINT32 *rom = (UINT32 *)(*machine().root_device().memregion("maincpu"));
	if (m_bios_protected != 0)
	{
		offset = (m_bios_last_address + 8) / 4;
	}
	return rom[offset&0x3fff];
}

READ32_MEMBER(gba_state::gba_10000000_r)
{
	UINT32 data, cpsr, pc;
	cpu_device *cpu = downcast<cpu_device *>(machine().device( "maincpu"));
	pc = cpu_get_reg( cpu, ARM7_PC);
	cpsr = cpu_get_reg( cpu, ARM7_CPSR);
	if (T_IS_SET( cpsr))
	{
		data = space.read_dword( pc + 8);
	}
	else
	{
		UINT16 insn = space.read_word( pc + 4);
		data = (insn << 16) | (insn << 0);
	}
	logerror( "%s: unmapped program memory read from %08X = %08X & %08X\n", machine().describe_context( ), 0x10000000 + (offset << 2), data, mem_mask);
	return data;
}

static ADDRESS_MAP_START( gbadvance_map, AS_PROGRAM, 32, gba_state )
	ADDRESS_MAP_UNMAP_HIGH // for "Fruit Mura no Doubutsu Tachi" and "Classic NES Series"
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM AM_READ(gba_bios_r)
	AM_RANGE(0x02000000, 0x0203ffff) AM_RAM AM_MIRROR(0xfc0000)
	AM_RANGE(0x03000000, 0x03007fff) AM_RAM AM_MIRROR(0xff8000)
	AM_RANGE(0x04000000, 0x040003ff) AM_READWRITE(gba_io_r, gba_io_w )
	AM_RANGE(0x05000000, 0x050003ff) AM_RAM_WRITE(gba_pram_w) AM_SHARE("gba_pram")	// Palette RAM
	AM_RANGE(0x06000000, 0x06017fff) AM_RAM_WRITE(gba_vram_w) AM_SHARE("gba_vram")	// VRAM
	AM_RANGE(0x07000000, 0x070003ff) AM_RAM_WRITE(gba_oam_w) AM_SHARE("gba_oam")	// OAM
	AM_RANGE(0x08000000, 0x09ffffff) AM_ROM AM_REGION("cartridge", 0)	// cartridge ROM (mirror 0)
	AM_RANGE(0x0a000000, 0x0bffffff) AM_ROM AM_REGION("cartridge", 0)	// cartridge ROM (mirror 1)
	AM_RANGE(0x0c000000, 0x0cffffff) AM_ROM AM_REGION("cartridge", 0)	// final mirror
	AM_RANGE(0x10000000, 0xffffffff) AM_READ(gba_10000000_r) // for "Justice League Chronicles" (game bug)
ADDRESS_MAP_END

static INPUT_PORTS_START( gbadv )
	PORT_START("IN0")
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_UNUSED
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 L") PORT_PLAYER(1)	// L
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 R") PORT_PLAYER(1)	// R
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1)	// START
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT ) PORT_PLAYER(1)	// SELECT
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B") PORT_PLAYER(1)	// B
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A") PORT_PLAYER(1)	// A
INPUT_PORTS_END

static TIMER_CALLBACK( perform_hbl )
{
	int ch, ctrl;
	gba_state *state = machine.driver_data<gba_state>();
	int scanline = machine.primary_screen->vpos();

	// draw only visible scanlines
	if (scanline < 160)
	{
		gba_draw_scanline(machine, scanline);
	}
	state->m_DISPSTAT |= DISPSTAT_HBL;
	if ((state->m_DISPSTAT & DISPSTAT_HBL_IRQ_EN ) != 0)
	{
		gba_request_irq(machine, INT_HBL);
	}

	for (ch = 0; ch < 4; ch++)
	{
		ctrl = state->m_dma_regs[(ch*3)+2]>>16;

		// HBL-triggered DMA?
		if ((ctrl & 0x8000) && ((ctrl & 0x3000) == 0x2000))
		{
			dma_exec(machine, ch);
		}
	}

	state->m_hbl_timer->adjust(attotime::never);
}

static TIMER_CALLBACK( perform_scan )
{
	int scanline;
	gba_state *state = machine.driver_data<gba_state>();

	// clear hblank and raster IRQ flags
	state->m_DISPSTAT &= ~(DISPSTAT_HBL|DISPSTAT_VCNT);

	scanline = machine.primary_screen->vpos();

	// VBL is set for scanlines 160 through 226 (but not 227, which is the last line)
	if (scanline >= 160 && scanline < 227)
	{
		state->m_DISPSTAT |= DISPSTAT_VBL;
	}
	else
	{
		state->m_DISPSTAT &= ~DISPSTAT_VBL;
	}

	// handle VCNT match interrupt/flag
	if (scanline == ((state->m_DISPSTAT >> 8) & 0xff))
	{
		state->m_DISPSTAT |= DISPSTAT_VCNT;
		if (state->m_DISPSTAT & DISPSTAT_VCNT_IRQ_EN)
		{
			gba_request_irq(machine, INT_VCNT);
		}
	}

	// exiting VBL, handle interrupts and DMA triggers
	if (scanline == 224)
	{
		// FIXME: some games are very picky with this trigger!
		// * Mario & Luigi SuperStar Saga loses pieces of gfx for 225-227.
		// * Driver 2 does not work with values > 217.
		// * Prince of Persia Sands of Time, Rayman Hoodlum's Revenge, Rayman 3 breaks for large
		//   values (say > 200, but exact threshold varies).
		// * Scooby-Doo Unmasked and Mystery Mayhem have problems with large values (missing dialogue
		//   text).
		// * Nicktoons Racign does not start with 227; and it resets before going to the race with
		//   values > 206.
		// * Phil of Future does not start for values > 221.
		// * Sabrina Teenage Witch does not even reach the Ubi Soft logo if we use the VBL exit value
		//   227; it does not display title screen graphics when using 225-226; the intro is broken
		//   with anything between 207-224.
		// * Anstoss Action and Ueki no Housoku have broken graphics for values > 223.
		// However, taking smaller values breaks raster effects in a LOT of games (e.g. Castlevania
		// series, Final Fantasy series, Tales of Phantasia, Banjo Pilot, NES 'collections' by Hudson,
		// Jaleco and Technos, plus tons of racing games, which show garbage in the lower half of the
		// screen with smaller values).
		// Already choosing 224 instead of 227 makes some glitches to appear in the bottom scanlines.
		// Other test cases are EA Sport games (like FIFA or Madden) which have various degrees of
		// glitchness depending on the value used here.
		// More work on IRQs is definitely necessary!
		int ch, ctrl;

		if (state->m_DISPSTAT & DISPSTAT_VBL_IRQ_EN)
		{
			gba_request_irq(machine, INT_VBL);
		}

		for (ch = 0; ch < 4; ch++)
		{
			ctrl = state->m_dma_regs[(ch*3)+2]>>16;

			// VBL-triggered DMA?
			if ((ctrl & 0x8000) && ((ctrl & 0x3000) == 0x1000))
			{
				dma_exec(machine, ch);
			}
		}
	}

	state->m_hbl_timer->adjust(machine.primary_screen->time_until_pos(scanline, 240));
	state->m_scan_timer->adjust(machine.primary_screen->time_until_pos(( scanline + 1 ) % 228, 0));
}

static MACHINE_RESET( gba )
{
	dac_device *gb_a_l = machine.device<dac_device>("direct_a_left");
	dac_device *gb_a_r = machine.device<dac_device>("direct_a_right");
	dac_device *gb_b_l = machine.device<dac_device>("direct_b_left");
	dac_device *gb_b_r = machine.device<dac_device>("direct_b_right");
	gba_state *state = machine.driver_data<gba_state>();

	//memset(state, 0, sizeof(state));
	state->m_SOUNDBIAS = 0x0200;
	state->m_eeprom_state = EEP_IDLE;
	state->m_SIOMULTI0 = 0xffff;
	state->m_SIOMULTI1 = 0xffff;
	state->m_SIOMULTI2 = 0xffff;
	state->m_SIOMULTI3 = 0xffff;
	state->m_KEYCNT = 0x03ff;
	state->m_RCNT = 0x8000;
	state->m_JOYSTAT = 0x0002;
	state->m_gfxBG2Changed = 0;
	state->m_gfxBG3Changed = 0;
	state->m_gfxBG2X = 0;
	state->m_gfxBG2Y = 0;
	state->m_gfxBG3X = 0;
	state->m_gfxBG3Y = 0;

	state->m_windowOn = 0;
	state->m_fxOn = 0;

	state->m_bios_protected = 0;

	state->m_scan_timer->adjust(machine.primary_screen->time_until_pos(0, 0));
	state->m_hbl_timer->adjust(attotime::never);
	state->m_dma_timer[0]->adjust(attotime::never);
	state->m_dma_timer[1]->adjust(attotime::never, 1);
	state->m_dma_timer[2]->adjust(attotime::never, 2);
	state->m_dma_timer[3]->adjust(attotime::never, 3);

	state->m_fifo_a_ptr = state->m_fifo_b_ptr = 17;	// indicate empty
	state->m_fifo_a_in = state->m_fifo_b_in = 17;

	// and clear the DACs
	gb_a_l->write_signed8(0x80);
	gb_a_r->write_signed8(0x80);
	gb_b_l->write_signed8(0x80);
	gb_b_r->write_signed8(0x80);

	if (state->m_flash_battery_load != 0)
	{
		device_image_interface *image = dynamic_cast<device_image_interface *>(state->m_nvimage);
		UINT8 *nvram = auto_alloc_array( machine, UINT8, state->m_flash_size);
		image->battery_load( nvram, state->m_flash_size, 0xff);
		for (int i = 0; i < state->m_flash_size; i++)
		{
			state->m_mFlashDev->write_raw( i, nvram[i]);
		}
		auto_free( machine, nvram);
		state->m_flash_battery_load = 0;
	}
}

static MACHINE_START( gba )
{
	gba_state *state = machine.driver_data<gba_state>();

	/* add a hook for battery save */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(gba_machine_stop),&machine));

	/* create a timer to fire scanline functions */
	state->m_scan_timer = machine.scheduler().timer_alloc(FUNC(perform_scan));
	state->m_hbl_timer = machine.scheduler().timer_alloc(FUNC(perform_hbl));
	state->m_scan_timer->adjust(machine.primary_screen->time_until_pos(0, 0));

	/* and one for each DMA channel */
	state->m_dma_timer[0] = machine.scheduler().timer_alloc(FUNC(dma_complete));
	state->m_dma_timer[1] = machine.scheduler().timer_alloc(FUNC(dma_complete));
	state->m_dma_timer[2] = machine.scheduler().timer_alloc(FUNC(dma_complete));
	state->m_dma_timer[3] = machine.scheduler().timer_alloc(FUNC(dma_complete));
	state->m_dma_timer[0]->adjust(attotime::never);
	state->m_dma_timer[1]->adjust(attotime::never, 1);
	state->m_dma_timer[2]->adjust(attotime::never, 2);
	state->m_dma_timer[3]->adjust(attotime::never, 3);

	/* also one for each timer (heh) */
	state->m_tmr_timer[0] = machine.scheduler().timer_alloc(FUNC(timer_expire));
	state->m_tmr_timer[1] = machine.scheduler().timer_alloc(FUNC(timer_expire));
	state->m_tmr_timer[2] = machine.scheduler().timer_alloc(FUNC(timer_expire));
	state->m_tmr_timer[3] = machine.scheduler().timer_alloc(FUNC(timer_expire));
	state->m_tmr_timer[0]->adjust(attotime::never);
	state->m_tmr_timer[1]->adjust(attotime::never, 1);
	state->m_tmr_timer[2]->adjust(attotime::never, 2);
	state->m_tmr_timer[3]->adjust(attotime::never, 3);

	/* and an IRQ handling timer */
	state->m_irq_timer = machine.scheduler().timer_alloc(FUNC(handle_irq));
	state->m_irq_timer->adjust(attotime::never);
}

ROM_START( gba )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gba.bin", 0x000000, 0x004000, CRC(81977335) SHA1(300c20df6731a33952ded8c436f7f186d25d3492) )

	/* cartridge region - 32 MBytes (128 Mbit) */
	ROM_REGION( 0x2000000, "cartridge", ROMREGION_ERASEFF )
ROM_END

READ32_MEMBER(gba_state::sram_r)
{

	return m_gba_sram[offset];
}

WRITE32_MEMBER(gba_state::sram_w)
{

	COMBINE_DATA(&m_gba_sram[offset]);
}

READ32_MEMBER(gba_state::flash_r)
{
	UINT32 rv;

	rv = 0;
	offset &= m_flash_mask;
	if (mem_mask & 0xff) rv |= m_mFlashDev->read(offset*4);
	if (mem_mask & 0xff00) rv |= m_mFlashDev->read((offset*4)+1)<<8;
	if (mem_mask & 0xff0000) rv |= m_mFlashDev->read((offset*4)+2)<<16;
	if (mem_mask & 0xff000000) rv |= m_mFlashDev->read((offset*4)+3)<<24;

	return rv;
}

WRITE32_MEMBER(gba_state::flash_w)
{

	offset &= m_flash_mask;
	switch (mem_mask)
	{
		case 0xff:
			m_mFlashDev->write(offset*4, data&0xff);
			break;
		case 0xff00:
			m_mFlashDev->write((offset*4)+1, (data>>8)&0xff);
			break;
		case 0xff0000:
			m_mFlashDev->write((offset*4)+2, (data>>16)&0xff);
			break;
		case 0xff000000:
			m_mFlashDev->write((offset*4)+3, (data>>24)&0xff);
			break;
		default:
			fatalerror("Unknown mem_mask for GBA flash_w %x\n", mem_mask);
			break;
	}
}

READ32_MEMBER(gba_state::eeprom_r)
{
	UINT32 out;

	switch (m_eeprom_state)
	{
		case EEP_IDLE:
//          printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %d\n", offset, ~mem_mask, m_eeprom_state, activecpu_get_pc(), 1);
			return 0x00010001;	// "ready"

		case EEP_READFIRST:
			m_eeprom_count--;

			if (!m_eeprom_count)
			{
				m_eeprom_count = 64;
				m_eeprom_bits = 0;
				m_eep_data = 0;
				m_eeprom_state = EEP_READ;
			}
			break;
		case EEP_READ:
			if ((m_eeprom_bits == 0) && (m_eeprom_count))
			{
				if (m_eeprom_addr >= sizeof( m_gba_eeprom))
				{
					fatalerror( "eeprom: invalid address (%x)\n", m_eeprom_addr);
				}
				m_eep_data = m_gba_eeprom[m_eeprom_addr];
				//printf("EEPROM read @ %x = %x (%x)\n", m_eeprom_addr, m_eep_data, (m_eep_data & 0x80) ? 1 : 0);
				m_eeprom_addr++;
				m_eeprom_bits = 8;
			}

			out = (m_eep_data & 0x80) ? 1 : 0;
			out |= (out<<16);
			m_eep_data <<= 1;

			m_eeprom_bits--;
			m_eeprom_count--;

			if (!m_eeprom_count)
			{
				m_eeprom_state = EEP_IDLE;
			}

//          printf("out = %08x\n", out);
//          printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %08x\n", offset, ~mem_mask, m_eeprom_state, activecpu_get_pc(), out);
			return out;
	}
//  printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %d\n", offset, ~mem_mask, m_eeprom_state, cpu_get_pc(&space.device()), 0);
	return 0;
}

WRITE32_MEMBER(gba_state::eeprom_w)
{

	if (~mem_mask == 0x0000ffff)
	{
		data >>= 16;
	}

//  printf("eeprom_w: %x @ %x (state %d) (PC=%x)\n", data, offset, m_eeprom_state, cpu_get_pc(&space.device()));

	switch (m_eeprom_state)
	{
		case EEP_IDLE:
			if (data == 1)
			{
				m_eeprom_state++;
			}
			break;

		case EEP_COMMAND:
			if (data == 1)
			{
				m_eeprom_command = EEP_READFIRST;
			}
			else
			{
				m_eeprom_command = EEP_WRITE;
			}
			m_eeprom_state = EEP_ADDR;
			m_eeprom_count = m_eeprom_addr_bits;
			m_eeprom_addr = 0;
			break;

		case EEP_ADDR:
			m_eeprom_addr <<= 1;
			m_eeprom_addr |= (data & 1);
			m_eeprom_count--;
			if (!m_eeprom_count)
			{
				m_eeprom_addr *= 8;	// each address points to 8 bytes
				if (m_eeprom_command == EEP_READFIRST)
				{
					m_eeprom_state = EEP_AFTERADDR;
				}
				else
				{
					m_eeprom_count = 64;
					m_eeprom_bits = 8;
					m_eeprom_state = EEP_WRITE;
					m_eep_data = 0;
				}
			}
			break;

		case EEP_AFTERADDR:
			m_eeprom_state = m_eeprom_command;
			m_eeprom_count = 64;
			m_eeprom_bits = 0;
			m_eep_data = 0;
			if( m_eeprom_state == EEP_READFIRST )
			{
				m_eeprom_count = 4;
			}
			break;

		case EEP_WRITE:
			m_eep_data<<= 1;
			m_eep_data |= (data & 1);
			m_eeprom_bits--;
			m_eeprom_count--;

			if (m_eeprom_bits == 0)
			{
				mame_printf_verbose("%08x: EEPROM: %02x to %x\n", cpu_get_pc(machine().device("maincpu")), m_eep_data, m_eeprom_addr );
				if (m_eeprom_addr >= sizeof( m_gba_eeprom))
				{
					fatalerror( "eeprom: invalid address (%x)\n", m_eeprom_addr);
				}
				m_gba_eeprom[m_eeprom_addr] = m_eep_data;
				m_eeprom_addr++;
				m_eep_data = 0;
				m_eeprom_bits = 8;
			}

			if (!m_eeprom_count)
			{
				m_eeprom_state = EEP_AFTERWRITE;
			}
			break;

		case EEP_AFTERWRITE:
			m_eeprom_state = EEP_IDLE;
			break;
	}
}

#define	GBA_CHIP_EEPROM     (1 << 0)
#define	GBA_CHIP_SRAM       (1 << 1)
#define	GBA_CHIP_FLASH      (1 << 2)
#define	GBA_CHIP_FLASH_1M   (1 << 3)
#define	GBA_CHIP_RTC        (1 << 4)
#define	GBA_CHIP_FLASH_512  (1 << 5)
#define	GBA_CHIP_EEPROM_64K (1 << 6)
#define	GBA_CHIP_EEPROM_4K  (1 << 7)

static UINT32 gba_detect_chip( UINT8 *data, int size)
{
	int i;
	UINT32 chip = 0;
	for (i = 0; i < size; i++)
	{
		if (!memcmp(&data[i], "EEPROM_V", 8))
		{
			chip |= GBA_CHIP_EEPROM; // should be either GBA_CHIP_EEPROM_4K or GBA_CHIP_EEPROM_64K, but it is not yet possible to automatically detect which one
		}
		else if ((!memcmp(&data[i], "SRAM_V", 6)) || (!memcmp(&data[i], "SRAM_F_V", 8))) // || (!memcmp(&data[i], "ADVANCEWARS", 11))) //advance wars 1 & 2 has SRAM, but no "SRAM_" string can be found inside the ROM space
		{
			chip |= GBA_CHIP_SRAM;
		}
		else if (!memcmp(&data[i], "FLASH1M_V", 9))
		{
			chip |= GBA_CHIP_FLASH_1M;
		}
		else if (!memcmp(&data[i], "FLASH512_V", 10))
		{
			chip |= GBA_CHIP_FLASH_512;
		}
		else if (!memcmp(&data[i], "FLASH_V", 7))
		{
			chip |= GBA_CHIP_FLASH;
		}
		else if (!memcmp(&data[i], "SIIRTC_V", 8))
		{
			chip |= GBA_CHIP_RTC;
		}
	}
	return chip;
}

static astring gba_chip_string( UINT32 chip)
{
	astring str;
	if (chip == 0) str += "NONE ";
	if (chip & GBA_CHIP_EEPROM) str += "EEPROM ";
	if (chip & GBA_CHIP_EEPROM_64K) str += "EEPROM_64K ";
	if (chip & GBA_CHIP_EEPROM_4K) str += "EEPROM_4K ";
	if (chip & GBA_CHIP_FLASH) str += "FLASH ";
	if (chip & GBA_CHIP_FLASH_1M) str += "FLASH_1M ";
	if (chip & GBA_CHIP_FLASH_512) str += "FLASH_512 ";
	if (chip & GBA_CHIP_SRAM) str += "SRAM ";
	if (chip & GBA_CHIP_RTC) str += "RTC ";
	return str.trimspace();
}

typedef struct
{
	char game_code[5];
	UINT32 chip;
} gba_chip_fix_conflict_item;

static const gba_chip_fix_conflict_item gba_chip_fix_conflict_list[] =
{
	{ "ABFJ", GBA_CHIP_SRAM       }, // 0059 - Breath of Fire - Ryuu no Senshi (JPN)
	{ "AHMJ", GBA_CHIP_EEPROM_4K  }, // 0364 - Dai-Mahjong (JPN)
	{ "A2GJ", GBA_CHIP_EEPROM_64K }, // 0399 - Advance GT2 (JPN)
	{ "AK9E", GBA_CHIP_EEPROM_4K  }, // 0479 - Medabots AX - Rokusho Version (USA)
	{ "AK8E", GBA_CHIP_EEPROM_4K  }, // 0480 - Medabots AX - Metabee Version (USA)
	{ "AK9P", GBA_CHIP_EEPROM_4K  }, // 0515 - Medabots AX - Rokusho Version (EUR)
	{ "AGIJ", GBA_CHIP_EEPROM_4K  }, // 0548 - Medarot G - Kuwagata Version (JPN)
	{ "A3DJ", GBA_CHIP_EEPROM_4K  }, // 0567 - Disney Sports - American Football (JPN)
	{ "AF7J", GBA_CHIP_EEPROM_64K }, // 0605 - Tokimeki Yume Series 1 - Ohanaya-san ni Narou! (JPN)
	{ "AH7J", GBA_CHIP_EEPROM_64K }, // 0617 - Nakayoshi Pet Advance Series 1 - Kawaii Hamster (JPN)
	{ "AGHJ", GBA_CHIP_EEPROM_4K  }, // 0620 - Medarot G - Kabuto Version (JPN)
	{ "AR8E", GBA_CHIP_SRAM       }, // 0727 - Rocky (USA)
	{ "ALUE", GBA_CHIP_EEPROM_4K  }, // 0751 - Super Monkey Ball Jr. (USA)
	{ "A3DE", GBA_CHIP_EEPROM_4K  }, // 0800 - Disney Sports - Football (USA)
	{ "A87J", GBA_CHIP_EEPROM_64K }, // 0817 - Ohanaya-San Monogatari GBA (JPN)
	{ "A56J", GBA_CHIP_EEPROM_64K }, // 0827 - DokiDoki Cooking Series 1 - Komugi-chan no Happy Cake (JPN)
	{ "AUSJ", GBA_CHIP_FLASH      }, // 0906 - One Piece - Mezase! King of Berries (JPN)
	{ "ANTJ", GBA_CHIP_SRAM       }, // 0950 - Nippon Pro Mahjong Renmei Kounin - Tetsuman Advance (JPN)
	{ "A8OJ", GBA_CHIP_EEPROM_64K }, // 0988 - DokiDoki Cooking Series 2 - Gourmet Kitchen - Suteki na Obentou (JPN)
	{ "AK8P", GBA_CHIP_EEPROM_4K  }, // 1022 - Medabots AX - Metabee Version (EUR)
	{ "A6OJ", GBA_CHIP_EEPROM_64K }, // 1092 - Onimusha Tactics (JPN)
	{ "A6OE", GBA_CHIP_EEPROM_64K }, // 1241 - Onimusha Tactics (USA)
	{ "A6OP", GBA_CHIP_EEPROM_64K }, // 1288 - Onimusha Tactics (EUR)
	{ "BKME", GBA_CHIP_EEPROM_4K  }, // 1545 - Kim Possible 2 - Drakken's Demise (USA)
	{ "BDKJ", GBA_CHIP_EEPROM_64K }, // 1555 - Digi Communication 2 in 1 Datou! Black Gemagema Dan (JPN)
	{ "BR4J", GBA_CHIP_FLASH      }, // 1586 - Rockman EXE 4.5 - Real Operation (JPN)
	{ "BG8J", GBA_CHIP_EEPROM_64K }, // 1853 - Ganbare! Dodge Fighters (JPN)
	{ "AROP", GBA_CHIP_EEPROM_4K  }, // 1862 - Rocky (EUR)
	{ "A2YE", GBA_CHIP_SRAM       }, // 1906 - Top Gun - Combat Zones (USA)
	{ "BKMJ", GBA_CHIP_EEPROM_4K  }, // 2039 - Kim Possible (JPN)
	{ "BKEJ", GBA_CHIP_EEPROM_64K }, // 2047 - Konjiki no Gashbell - The Card Battle for GBA (JPN)
	{ "BKMP", GBA_CHIP_EEPROM_4K  }, // 2297 - Kim Possible 2 - Drakken's Demise (EUR)
	{ "BUHJ", GBA_CHIP_EEPROM_4K  }, // 2311 - Ueki no Housoku Shinki Sakuretsu! Nouryokumono Battle (JPN)
	{ "BYUJ", GBA_CHIP_EEPROM_64K }, // 2322 - Yggdra Union (JPN)
};

typedef struct
{
	char game_code[5];
} gba_chip_fix_eeprom_item;

static const gba_chip_fix_eeprom_item gba_chip_fix_eeprom_list[] =
{
	// gba scan no. 7
	{ "AKTJ" }, // 0145 - Hello Kitty Collection - Miracle Fashion Maker (JPN)
	{ "AISP" }, // 0185 - International Superstar Soccer (EUR)
	{ "AKGP" }, // 0204 - Mech Platoon (EUR)
	{ "AX2E" }, // 0207 - Dave Mirra Freestyle BMX 2 (USA)
	{ "AASJ" }, // 0234 - World Advance Soccer - Shouri heno Michi (JPN)
	{ "AA2J" }, // 0237 - Super Mario World - Super Mario Advance 2 (JPN)
	{ "AJWJ" }, // 0242 - Jikkyou World Soccer Pocket (JPN)
	{ "AABE" }, // 0244 - American Bass Challenge (USA)
	{ "AWXJ" }, // 0254 - ESPN Winter X-Games Snowboarding 2002 (JPN)
	{ "AALJ" }, // 0259 - Kidou Tenshi Angelic Layer - Misaki to Yume no Tenshi-tachi (JPN)
	{ "AKGE" }, // 0263 - Mech Platoon (USA)
	{ "AGLJ" }, // 0273 - Tomato Adventure (JPN)
	{ "AWIJ" }, // 0274 - Hyper Sports 2002 Winter (JPN)
	{ "APNJ" }, // 0286 - Pinky Monkey Town (JPN)
	{ "AA2E" }, // 0288 - Super Mario World - Super Mario Advance 2 (USA)
	{ "AX2P" }, // 0293 - Dave Mirra Freestyle BMX 2 (EUR)
	{ "AMGP" }, // 0296 - ESPN Great Outdoor Games - Bass Tournament (EUR)
	{ "AMHJ" }, // 0308 - Bomberman Max 2 - Bomberman Version (JPN)
	{ "AGNJ" }, // 0311 - Goemon - New Age Shutsudou! (JPN)
	{ "AMYJ" }, // 0324 - Bomberman Max 2 - Max Version (JPN)
	{ "AT3E" }, // 0326 - Tony Hawk's Pro Skater 3 (USA)
	{ "AHHE" }, // 0327 - High Heat - Major League Baseball 2003 (USA)
	{ "ANLE" }, // 0328 - NHL 2002 (USA)
	{ "AAGJ" }, // 0345 - Angelique (JPN)
	{ "ABJP" }, // 0351 - Broken Sword - The Shadow of the Templars (EUR)
	{ "AKVJ" }, // 0357 - K-1 Pocket Grand Prix (JPN)
	{ "AKGJ" }, // 0361 - Kikaika Guntai - Mech Platoon (JPN)
	{ "ADDJ" }, // 0362 - Diadroids World - Evil Teikoku no Yabou (JPN)
	{ "ABJE" }, // 0365 - Broken Sword - The Shadow of the Templars (USA)
	{ "AABP" }, // 0380 - Super Black Bass Advance (EUR)
	{ "AA2P" }, // 0390 - Super Mario World - Super Mario Advance 2 (EUR)
	{ "A2GJ" }, // 0399 - Advance GT2 (JPN)
	{ "AEWJ" }, // 0400 - Ui-Ire - World Soccer Winning Eleven (JPN)
	{ "ADPJ" }, // 0417 - Doraemon - Dokodemo Walker (JPN)
	{ "AN5J" }, // 0420 - Kawa no Nushi Tsuri 5 - Fushigi no Mori Kara (JPN)
	{ "ACBJ" }, // 0421 - Gekitou! Car Battler Go!! (JPN)
	{ "AHIJ" }, // 0426 - Hitsuji no Kimochi (JPN)
	{ "ATFP" }, // 0429 - Alex Ferguson's Player Manager 2002 (EUR)
	{ "AFUJ" }, // 0431 - Youkaidou (JPN)
	{ "AEPP" }, // 0435 - Sheep (EUR)
	{ "AMHE" }, // 0442 - Bomberman Max 2 - Blue Advance (USA)
	{ "AMYE" }, // 0443 - Bomberman Max 2 - Red Advance (USA)
	{ "AT3F" }, // 0457 - Tony Hawk's Pro Skater 3 (FRA)
	{ "ARJJ" }, // 0497 - Custom Robo GX (JPN)
	{ "AFCJ" }, // 0521 - RockMan & Forte (JPN)
	{ "ANJE" }, // 0528 - Madden NFL 2003 (USA)
	{ "AN7J" }, // 0533 - Famista Advance (JPN)
	{ "ATYJ" }, // 0540 - Gambler Densetsu Tetsuya - Yomigaeru Densetsu (JPN)
	{ "AXBJ" }, // 0551 - Black Matrix Zero (JPN)
	{ "A3AE" }, // 0580 - Yoshi's Island - Super Mario Advance 3 (USA)
	{ "A3AJ" }, // 0582 - Super Mario Advance 3 (JPN)
	{ "AZUJ" }, // 0595 - Street Fighter Zero 3 - Upper (JPN)
	{ "ALOE" }, // 0600 - The Lord of the Rings - The Fellowship of the Ring (USA)
	{ "A2SE" }, // 0602 - Spyro 2 - Season of Flame (USA)
	{ "AF7J" }, // 0605 - Tokimeki Yume Series 1 - Ohanaya-san ni Narou! (JPN)
	{ "A3AP" }, // 0610 - Yoshi's Island - Super Mario Advance 3 (EUR)
	{ "AH7J" }, // 0617 - Nakayoshi Pet Advance Series 1 - Kawaii Hamster (JPN)
	{ "AI7J" }, // 0618 - Nakayoshi Pet Advance Series 2 - Kawaii Koinu (JPN)
	{ "AN3J" }, // 0619 - Nakayoshi Pet Advance Series 3 - Kawaii Koneko (JPN)
	{ "AAPJ" }, // 0632 - Metalgun Slinger (JPN)
	{ "A2JJ" }, // 0643 - J.League - Winning Eleven Advance 2002 (JPN)
	{ "AHXJ" }, // 0649 - High Heat - Major League Baseball 2003 (JPN)
	{ "AHAJ" }, // 0651 - Hamster Paradise Advance (JPN)
	{ "APUJ" }, // 0653 - PukuPuku Tennen Kairanban (JPN)
	{ "A2SP" }, // 0673 - Spyro 2 - Season of Flame (EUR)
	{ "AN9J" }, // 0675 - Tales of the World - Narikiri Dungeon 2 (JPN)
	{ "ACBE" }, // 0683 - Car Battler Joe (USA)
	{ "AT6E" }, // 0693 - Tony Hawk's Pro Skater 4 (USA)
	{ "ALOP" }, // 0702 - The Lord of the Rings - The Fellowship of the Ring (EUR)
	{ "A63J" }, // 0710 - Kawaii Pet Shop Monogatari 3 (JPN)
	{ "AAXJ" }, // 0748 - Fantastic Maerchen - Cake-yasan Monogatari (JPN)
	{ "AZLE" }, // 0763 - The Legend of Zelda - A Link to the Past & Four Swords (USA)
	{ "AZUP" }, // 0765 - Street Fighter Alpha 3 - Upper (EUR)
	{ "AJKJ" }, // 0769 - Jikkyou World Soccer Pocket 2 (JPN)
	{ "AB3E" }, // 0781 - Dave Mirra Freestyle BMX 3 (USA)
	{ "A2IJ" }, // 0791 - Magi Nation (JPN)
	{ "AK7J" }, // 0792 - Klonoa Heroes - Densetsu no Star Medal (JPN)
	{ "A2HJ" }, // 0794 - Hajime no Ippo - The Fighting! (JPN)
	{ "ALNE" }, // 0795 - Lunar Legend (USA)
	{ "AUCJ" }, // 0808 - Uchuu Daisakusen Choco Vader - Uchuu Kara no Shinryakusha (JPN)
	{ "A59J" }, // 0809 - Toukon Heat (JPN)
	{ "ALJE" }, // 0815 - Sea Trader - Rise of Taipan (USA)
	{ "A87J" }, // 0817 - Ohanaya-San Monogatari GBA (JPN)
	{ "A56J" }, // 0827 - DokiDoki Cooking Series 1 - Komugi-chan no Happy Cake (JPN)
	{ "AHZJ" }, // 0830 - Higanbana (JPN)
	{ "A8BP" }, // 0832 - Medabots - Metabee Version (EUR)
	{ "A2OJ" }, // 0833 - K-1 Pocket Grand Prix 2 (JPN)
	{ "AY2P" }, // 0843 - International Superstar Soccer Advance (EUR)
	{ "ANSJ" }, // 0845 - Marie, Elie & Anis no Atelier - Soyokaze Kara no Dengon (JPN)
	{ "ACOJ" }, // 0865 - Manga-ka Debut Monogatari (JPN)
	{ "AZLP" }, // 0870 - The Legend of Zelda - A Link to the Past & Four Swords (EUR)
	{ "AWKJ" }, // 0879 - Wagamama Fairy Mirumo de Pon! - Ougon Maracas no Densetsu (JPN)
	{ "AZUE" }, // 0886 - Street Fighter Alpha 3 - Upper (USA)
	{ "AZLJ" }, // 0887 - Zelda no Densetsu - Kamigami no Triforce & 4tsu no Tsurugi (JPN)
	{ "A6ME" }, // 0889 - MegaMan & Bass (USA)
	{ "A64J" }, // 0915 - Shimura Ken no Baka Tonosama (JPN)
	{ "A9HJ" }, // 0917 - Dragon Quest Monsters - Caravan Heart (JPN)
	{ "AMHP" }, // 0929 - Bomberman Max 2 - Blue Advance (EUR)
	{ "AMYP" }, // 0930 - Bomberman Max 2 - Red Advance (EUR)
	{ "AMGJ" }, // 0943 - Exciting Bass (JPN)
	{ "A5KJ" }, // 0946 - Medarot 2 Core - Kabuto Version (JPN)
	{ "A4LJ" }, // 0949 - Sylvania Family 4 - Meguru Kisetsu no Tapestry (JPN)
	{ "A2VJ" }, // 0955 - Kisekko Gurumi - Chesty to Nuigurumi-tachi no Mahou no Bouken (JPN)
	{ "A5QJ" }, // 0956 - Medarot 2 Core - Kuwagata Version (JPN)
	{ "AZBJ" }, // 0958 - Bass Tsuri Shiyouze! (JPN)
	{ "AO2J" }, // 0961 - Oshare Princess 2 (JPN)
	{ "AB4J" }, // 0965 - Summon Night - Craft Sword Monogatari (JPN)
	{ "AZAJ" }, // 0971 - Azumanga Daiou Advance (JPN)
	{ "AF3J" }, // 0974 - Zero One (JPN)
	{ "A8OJ" }, // 0988 - DokiDoki Cooking Series 2 - Gourmet Kitchen - Suteki na Obentou (JPN)
	{ "AT3D" }, // 1016 - Tony Hawk's Pro Skater 3 (GER)
	{ "A6MP" }, // 1031 - MegaMan & Bass (EUR)
	{ "ANNJ" }, // 1032 - Gekitou Densetsu Noah - Dream Management (JPN)
	{ "AFNJ" }, // 1036 - Angel Collection - Mezase! Gakuen no Fashion Leader (JPN)
	{ "ALFP" }, // 1041 - Dragon Ball Z - The Legacy of Goku II (EUR)
	{ "A9TJ" }, // 1055 - Metal Max 2 Kai (JPN)
	{ "ALFE" }, // 1070 - Dragon Ball Z - The Legacy of Goku II (USA)
	{ "BHCJ" }, // 1074 - Hamster Monogatari Collection (JPN)
	{ "BKKJ" }, // 1075 - Minna no Shiiku Series - Boku no Kabuto-Kuwagata (JPN)
	{ "BKIJ" }, // 1083 - Nakayoshi Pet Advance Series 4 - Kawaii Koinu Kogata Inu (JPN)
	{ "BGBJ" }, // 1084 - Get! - Boku no Mushi Tsukamaete (JPN)
	{ "A82J" }, // 1085 - Hamster Paradise - Pure Heart (JPN)
	{ "U3IJ" }, // 1087 - Bokura no Taiyou - Taiyou Action RPG (JPN)
	{ "A6OJ" }, // 1092 - Onimusha Tactics (JPN)
	{ "AN8J" }, // 1102 - Tales of Phantasia (JPN)
	{ "AC4J" }, // 1104 - Meitantei Conan - Nerawareta Tantei (JPN)
	{ "A8ZJ" }, // 1108 - Shin Megami Tensei Devil Children - Puzzle de Call! (JPN)
	{ "BGMJ" }, // 1113 - Gensou Maden Saiyuuki - Hangyaku no Toushin-taishi (JPN)
	{ "BMDE" }, // 1115 - Madden NFL 2004 (USA)
	{ "BO3J" }, // 1141 - Oshare Princess 3 (JPN)
	{ "U3IE" }, // 1145 - Boktai - The Sun is in Your Hand (USA)
	{ "BMRJ" }, // 1194 - Matantei Loki Ragnarok - Gensou no Labyrinth (JPN)
	{ "BLME" }, // 1204 - Lizzie McGuire (USA)
	{ "AOWE" }, // 1208 - Spyro - Attack of the Rhynocs (USA)
	{ "BTOE" }, // 1209 - Tony Hawk's Underground (USA)
	{ "BFJE" }, // 1212 - Frogger's Journey - The Forgotten Relic (USA)
	{ "A88P" }, // 1229 - Mario & Luigi - Superstar Saga (EUR)
	{ "BEYP" }, // 1236 - Beyblade VForce - Ultimate Blader Jam (EUR)
	{ "A85J" }, // 1239 - Sanrio Puroland All Characters (JPN)
	{ "BMZJ" }, // 1240 - Zooo (JPN)
	{ "A6OE" }, // 1241 - Onimusha Tactics (USA)
	{ "AOWP" }, // 1253 - Spyro Adventure (EUR)
	{ "A88E" }, // 1260 - Mario & Luigi - Superstar Saga (USA)
	{ "BEYE" }, // 1262 - Beyblade VForce - Ultimate Blader Jam (USA)
	{ "BCME" }, // 1264 - CIMA - The Enemy (USA)
	{ "A88J" }, // 1266 - Mario & Luigi RPG (JPN)
	{ "A5CP" }, // 1269 - Sim City 2000 (EUR)
	{ "BGAJ" }, // 1277 - SD Gundam G Generation (JPN)
	{ "A6OP" }, // 1288 - Onimusha Tactics (EUR)
	{ "BLMP" }, // 1289 - Lizzie McGuire (EUR)
	{ "ASIE" }, // 1295 - The Sims - Bustin' Out (USA)
	{ "BISJ" }, // 1299 - Koinu-Chan no Hajimete no Osanpo - Koinu no Kokoro Ikusei Game (JPN)
	{ "BK3J" }, // 1305 - Card Captor Sakura - Sakura Card de Mini Game (JPN)
	{ "A4GJ" }, // 1306 - Konjiki no Gashbell!! - Unare! Yuujou no Zakeru (JPN)
	{ "BTAJ" }, // 1315 - Astro Boy - Tetsuwan Atom (JPN)
	{ "BS5J" }, // 1322 - Sylvanian Family - Yousei no Stick to Fushigi no Ki (JPN)
	{ "A5CE" }, // 1326 - Sim City 2000 (USA)
	{ "B4PJ" }, // 1342 - The Sims (JPN)
	{ "BDTJ" }, // 1383 - Downtown - Nekketsu Monogatari EX (JPN)
	{ "B08J" }, // 1391 - One Piece - Going Baseball (JPN)
	{ "AWUP" }, // 1394 - Sabre Wulf (EUR)
	{ "BRPJ" }, // 1421 - Liliput Oukoku (JPN)
	{ "BPNJ" }, // 1435 - Pika Pika Nurse Monogatari - Nurse Ikusei Game (JPN)
	{ "BP3J" }, // 1446 - Pia Carrot he Youkoso!! 3.3 (JPN)
	{ "BKCJ" }, // 1461 - Crayon Shin-Chan - Arashi no Yobu Cinema-Land no Daibouken! (JPN)
	{ "BGNJ" }, // 1464 - Battle Suit Gundam Seed - Battle Assault (JPN)
	{ "U3IP" }, // 1465 - Boktai - The Sun is in Your Hand (EUR)
	{ "BDTE" }, // 1484 - River City Ransom EX (USA)
	{ "BHTE" }, // 1485 - Harry Potter and the Prisoner of Azkaban (USA)
	{ "FZLE" }, // 1494 - Classic NES Series - The Legend of Zelda (USA)
	{ "FEBE" }, // 1499 - Classic NES Series - ExciteBike (USA)
	{ "BUCE" }, // 1505 - Ultimate Card Games (USA)
	{ "AWUE" }, // 1511 - Sabre Wulf (USA)
	{ "B2DP" }, // 1522 - Donkey Kong Country 2 (EUR)
	{ "BHTJ" }, // 1528 - Harry Potter to Azkaban no Shuujin (JPN)
	{ "A5SJ" }, // 1534 - Oshare Wanko (JPN)
	{ "B2DJ" }, // 1541 - Super Donkey Kong 2 (JPN)
	{ "BTAE" }, // 1551 - Astro Boy - Omega Factor (USA)
	{ "BKOJ" }, // 1553 - Kaiketsu Zorori to Mahou no Yuuenchi (JPN)
	{ "BDKJ" }, // 1555 - Digi Communication 2 in 1 Datou! Black Gemagema Dan (JPN)
	{ "U32J" }, // 1567 - Zoku Bokura no Taiyou - Taiyou Shounen Django (JPN)
	{ "ALFJ" }, // 1573 - Dragon Ball Z - The Legacy of Goku II - International (JPN)
	{ "BGHJ" }, // 1575 - Gakkou no Kaidan - Hyakuyobako no Fuuin (JPN)
	{ "BZOJ" }, // 1576 - Zero One SP (JPN)
	{ "BDXJ" }, // 1587 - B-Densetsu! Battle B-Daman Moero! B-Kon (JPN)
	{ "BNBJ" }, // 1589 - Himawari Doubutsu Byouin Pet no Oishasan (JPN)
	{ "BMFE" }, // 1595 - Madden NFL 2005 (USA)
	{ "FMRJ" }, // 1599 - Famicom Mini Series 23 - Metroid (JPN)
	{ "FPTJ" }, // 1600 - Famicom Mini Series 24 - Hikari Shinwa - Palutena no Kagame (JPN)
	{ "FLBJ" }, // 1601 - Famicom Mini Series 25 - The Legend of Zelda 2 - Link no Bouken (JPN)
	{ "FSDJ" }, // 1606 - Famicom Mini Series 30 - SD Gundam World - Gachapon Senshi Scramble Wars (JPN)
	{ "BSKJ" }, // 1611 - Summon Night - Craft Sword Monogatari 2 (JPN)
	{ "BG3E" }, // 1628 - Dragon Ball Z - Buu's Fury (USA)
	{ "BECJ" }, // 1644 - Angel Collection 2 - Pichimo ni Narou (JPN)
	{ "B2TE" }, // 1672 - Tony Hawk's Underground 2 (USA)
	{ "BTYE" }, // 1689 - Ty the Tasmanian Tiger 2 - Bush Rescue (USA)
	{ "BT2E" }, // 1695 - Teenage Mutant Ninja Turtles 2 - Battlenexus (USA)
	{ "U32E" }, // 1697 - Boktai 2 - Solar Boy Django (USA)
	{ "BFDJ" }, // 1708 - Fruit Mura no Doubutsu Tachi (JPN)
	{ "BPQJ" }, // 1717 - PukuPuku Tennen Kairanban - Koi no Cupid Daisakusen (JPN)
	{ "BZMJ" }, // 1721 - The Legend of Zelda - Fushigi no Boushi (JPN)
	{ "FLBE" }, // 1723 - Classic NES Series - Zelda II - The Adventure of Link (USA)
	{ "BZMP" }, // 1736 - The Legend of Zelda - The Minish Cap (EUR)
	{ "B2DE" }, // 1754 - Donkey Kong Country 2 (USA)
	{ "BT2P" }, // 1758 - Teenage Mutant Ninja Turtles 2 - Battle Nexus (EUR)
	{ "BB2E" }, // 1759 - Beyblade G-Revolution (USA)
	{ "BRGE" }, // 1761 - Yu-Yu-Hakusho - Tournament Tactics (USA)
	{ "BFJJ" }, // 1766 - Frogger - Kodaibunmei no Nazo (JPN)
	{ "BB2P" }, // 1776 - Beyblade G-Revolution (EUR)
	{ "BSFJ" }, // 1791 - Sylvania Family - Fashion Designer ni Naritai (JPN)
	{ "BPIE" }, // 1798 - It's Mr Pants (USA)
	{ "B3PJ" }, // 1809 - Pukupuku Tennen Kairanban Youkoso Illusion Land (JPN)
	{ "BHDJ" }, // 1812 - Hello Idol Debut (JPN)
	{ "BKUJ" }, // 1823 - Shingata Medarot - Kuwagata Version (JPN)
	{ "BKVJ" }, // 1824 - Shingata Medarot - Kabuto Version (JPN)
	{ "BLIJ" }, // 1825 - Little Patissier Cake no Oshiro (JPN)
	{ "B3TJ" }, // 1833 - Tales of the World - Narikiri Dungeon 3 (JPN)
	{ "B2KJ" }, // 1836 - Kiss x Kiss - Seirei Gakuen (JPN)
	{ "A9BE" }, // 1837 - Medabots - Rokusho Version (USA)
	{ "BZME" }, // 1842 - The Legend of Zelda - The Minish Cap (USA)
	{ "B8MJ" }, // 1845 - Mario Party Advance (JPN)
	{ "A8BE" }, // 1871 - Medabots - Metabee Version (USA)
	{ "BTAP" }, // 1879 - Astro Boy - Omega Factor (EUR)
	{ "FSRJ" }, // 1916 - Famicom Mini Series - Dai 2 Ji Super Robot Taisen (JPN)
	{ "B8ME" }, // 1931 - Mario Party Advance (USA)
	{ "BO8K" }, // 1938 - One Piece - Going Baseball Haejeok Yaku (KOR)
	{ "B4ZJ" }, // 1941 - Rockman Zero 4 (JPN)
	{ "BQAJ" }, // 1953 - Meitantei Conan Akatsuki no Monument (JPN)
	{ "BIPJ" }, // 1956 - One Piece - Dragon Dream (JPN)
	{ "BQBJ" }, // 1970 - Konchu Monster Battle Master (JPN)
	{ "BQSJ" }, // 1971 - Konchu Monster Battle Stadium (JPN)
	{ "BWXJ" }, // 1982 - Wanko Mix Chiwanko World (JPN)
	{ "A9TJ" }, // 1984 - Metal Max 2 - Kai Version (JPN)
	{ "U32P" }, // 1992 - Boktai 2 - Solar Boy Django (EUR)
	{ "B4RJ" }, // 2005 - Shikakui Atama wo Marukusuru Advance - Kokugo Sansu Rika Shakai (JPN)
	{ "B4KJ" }, // 2007 - Shikakui Atama wo Marukusuru Advance - Kanji Keisan (JPN)
	{ "BFCJ" }, // 2019 - Fantasic Children (JPN)
	{ "BCSP" }, // 2020 - 2 in 1 - V-Rally 3 - Stuntman (EUR)
	{ "BM2J" }, // 2024 - Momotarou Densetsu G Gold Deck wo Tsukure! (JPN)
	{ "BEJJ" }, // 2026 - Erementar Gerad (JPN)
	{ "B5AP" }, // 2034 - Crash & Spyro - Super Pack Volume 1 (EUR)
	{ "B52P" }, // 2035 - Crash & Spyro - Super Pack Volume 2 (EUR)
	{ "BFMJ" }, // 2046 - Futari wa Precure Max Heart Maji! Maji! Fight de IN Janai (JPN)
	{ "BKEJ" }, // 2047 - Konjiki no Gashbell - The Card Battle for GBA (JPN)
	{ "U33J" }, // 2048 - Shin Bokura no Taiyou - Gyakushuu no Sabata (JPN)
	{ "BHFJ" }, // 2050 - Twin Series 4 - Ham Ham Monster EX + Fantasy Puzzle Hamster Monogatari (JPN)
	{ "BMWJ" }, // 2051 - Twin Series 5 - Wan Wan Meitantei EX + Mahou no Kuni no Keaki-Okusan Monogatari (JPN)
	{ "BMZP" }, // 2055 - Zooo (EUR)
	{ "B6ME" }, // 2057 - Madden NFL 06 (USA)
	{ "BT4E" }, // 2058 - Dragon Ball GT - Transformation (USA)
	{ "B2OJ" }, // 2071 - Pro Mahjong - Tsuwamono GBA (JPN)
	{ "BX4E" }, // 2079 - 2 in 1 - Tony Hawk's Underground + Kelly Slater's Pro Surfer (USA)
	{ "BRLE" }, // 2097 - Rebelstar - Tactical Command (USA)
	{ "BX5P" }, // 2100 - Rayman - 10th Anniversary (EUR)
	{ "B4ZP" }, // 2108 - MegaMan Zero 4 (EUR)
	{ "BX5E" }, // 2123 - Rayman - 10th Anniversary (USA)
	{ "BGXJ" }, // 2143 - Gunstar Super Heroes (JPN)
	{ "B4ZE" }, // 2144 - Megaman Zero 4 (USA)
	{ "B53P" }, // 2164 - Crash & Spyro - Super Pack Volume 3 (EUR)
	{ "B26E" }, // 2169 - World Poker Tour (USA)
	{ "BH9E" }, // 2172 - Tony Hawk's American Sk8land (USA)
	{ "BHGE" }, // 2177 - Gunstar Super Heroes (USA)
	{ "BCMJ" }, // 2178 - Frontier Stories (JPN)
	{ "BUZE" }, // 2182 - Ultimate Arcade Games (USA)
	{ "BTVE" }, // 2198 - Ty the Tasmanian Tiger 3 - Night of the Quinkan (USA)
	{ "BHGP" }, // 2199 - Gunstar Future Heroes (EUR)
	{ "BH9X" }, // 2214 - Tony Hawk's American Sk8land (EUR)
	{ "BWIP" }, // 2231 - Win X Club (EUR)
	{ "BQTP" }, // 2232 - My Pet Hotel (EUR)
	{ "B4LJ" }, // 2245 - Sugar Sugar Une - Heart Gaippai! Moegi Gakuen (JPN)
	{ "B3CJ" }, // 2249 - Summon Night Craft Sword Monogatari - Hajimari no Ishi (JPN)
	{ "A4GE" }, // 2260 - ZatchBell! - Electric Arena (USA)
	{ "BLFE" }, // 2264 - 2 in 1 - Dragon Ball Z 1 and 2 (USA)
	{ "BO2J" }, // 2272 - Ochainu no Bouken Jima (JPN)
	{ "BWIE" }, // 2276 - WinX Club (USA)
	{ "BGQE" }, // 2279 - Greg Hastings' Tournament Paintball Max'd (USA)
	{ "BURE" }, // 2298 - Paws & Claws - Pet Resort (USA)
	{ "A3AC" }, // 2303 - Yaoxi Dao (CHN)
	{ "AN8E" }, // 2305 - Tales of Phantasia (USA)
	{ "BZWJ" }, // 2309 - Akagi (JPN)
	{ "BT8P" }, // 2316 - Teenage Mutant Ninja Turtles Double Pack (EUR)
	{ "BYUJ" }, // 2322 - Yggdra Union (JPN)
	{ "AN3E" }, // 2324 - Catz (USA)
	{ "AN8P" }, // 2325 - Tales of Phantasia (EUR)
	{ "AA2C" }, // 2332 - Chaoji Maliou Shijie (CHN)
	{ "BWOP" }, // 2333 - World Poker Tour (EUR)
	{ "BKCS" }, // 2334 - Shinchan - Aventuras en Cineland (ESP)
	{ "BC2J" }, // 2341 - Crayon Shin chan - Densetsu wo Yobu Omake no Miyako Shockgaan (JPN)
	{ "BUOJ" }, // 2345 - Minna no Soft Series - Numpla Advance (JPN)
	{ "AN3J" }, // 2347 - Minna no Soft Series - Kawaii Koneko (JPN)
	{ "BUOE" }, // 2366 - Dr. Sudoku (USA)
	{ "B8SE" }, // 2368 - Spyro Superpack - Season of Ice + Season of Flame (USA)
	{ "U32J" }, // 2369 - Zoku Bokura no Taiyou - Taiyou Shounen Django (v01) (JPN)
	{ "BBMJ" }, // 2388 - B-Legend! Battle B-Daman - Moero! B-Damashi!! (JPN)
	{ "B53E" }, // 2395 - Crash & Spyro Superpack - Ripto's Rampage + The Cortex Conspiracy (USA)
	{ "BH9P" }, // 2399 - Tony Hawk's American Sk8land (EUR)
	{ "BAQP" }, // 2406 - Premier Action Soccer (EUR)
	{ "AB4E" }, // 2432 - Summon Night - Swordcraft Story (USA)
	{ "BBYE" }, // 2436 - Barnyard (USA)
	{ "BDXE" }, // 2438 - Battle B-Daman (USA)
	{ "B7ME" }, // 2446 - Madden NFL 07 (USA)
	{ "BUFE" }, // 2447 - 2 Games in 1 - Dragon Ball Z - Buu's Fury + Dragon Ball GT - Transformation (USA)
	{ "BUOP" }, // 2449 - Dr. Sudoku (EUR)
	{ "BBYX" }, // 2461 - Barnyard (EUR)
	{ "BFEE" }, // 2466 - Dogz - Fashion (USA)
	{ "AN3X" }, // 2468 - Catz (EUR)
	{ "BBME" }, // 2481 - Battle B-Daman - Fire Spirits (USA)
	{ "BQZE" }, // 2487 - Avatar - The Last Airbender (USA)
	{ "BXFE" }, // 2498 - Bratz - Forever Diamondz (USA)
	{ "BT8E" }, // 2500 - Teenage Mutant Ninja Turtles Double Pack (USA)
	{ "B3YE" }, // 2504 - The Legend of Spyro - A New Beginning (USA)
	{ "BSKE" }, // 2505 - Summon Night - Swordcraft Story 2 (USA)
	{ "BHBP" }, // 2513 - Best Friends - Hunde & Katzen (EUR)
	{ "B3YP" }, // 2519 - The Legend Of Spyro - A New Beginning (EUR)
	{ "BXFD" }, // 2520 - Bratz - Forever Diamondz (GER)
	{ "BRLP" }, // 2532 - Rebelstar - Tactical Command (EUR)
	{ "BENP" }, // 2560 - Eragon (EUR)
	{ "BENE" }, // 2561 - Eragon (USA)
	{ "BYUE" }, // 2573 - Yggdra Union - We'll Never Fight Alone (USA)
	{ "BFRP" }, // 2588 - My Animal Centre in Africa (EUR)
	{ "BFQE" }, // 2606 - Mazes of Fate (USA)
	{ "BQZP" }, // 2607 - Avatar - The Legend of Aang (EUR)
	{ "BFEP" }, // 2613 - Dogz Fashion (EUR)
	{ "BC2S" }, // 2631 - Shinchan contra los Munecos de Shock Gahn (ESP)
	{ "BXFP" }, // 2640 - Bratz - Forever Diamondz (EUR)
	{ "BEFP" }, // 2652 - Best Friends - My Horse (EUR)
	{ "BNBE" }, // 2695 - Petz Vet (USA)
	{ "BIME" }, // 2696 - Dogz 2 (USA)
	{ "BQTX" }, // 2710 - Mijn Dierenpension (EUR)
	{ "BIMP" }, // 2720 - Dogz 2 (EUR)
	{ "BIMX" }, // 2727 - Dogz 2 (EUR)
	{ "BHUE" }, // 2730 - Horsez (USA)
	{ "BQTF" }, // 2732 - Lea - Passion Veterinaire (FRA)
	{ "BJPP" }, // 2770 - Harry Potter Collection (EUR)
	{ "BEFE" }, // 2772 - Let's Ride - Friends Forever (USA)
	{ "BHBE" }, // 2774 - Best Friends - Dogs & Cats (USA)
	{ "BYUP" }, // 2781 - Yggdra Union - We'll Never Fight Alone (EUR)
	{ "ACOJ" }, // 2787 - Manga-ka Debut Monogatari (v01) (JPN)
	{ "BFDJ" }, // 2789 - Fruit Mura no Doubutsu Tachi (v02) (JPN)
	// gba scan no. 8
	{ "AYSJ" }, // 0229 - Gakkou wo Tsukurou!! Advance (JPN)
	{ "ASNJ" }, // 0260 - Sansara Naga 1x2 (JPN)
	{ "ACTX" }, // 0265 - Creatures (EUR)
	{ "ASFJ" }, // 0359 - Slot! Pro Advance - Takarabune & Ooedo Sakurafubuki 2 (JPN)
	{ "ABGJ" }, // 0404 - Sweet Cookie Pie (JPN)
	{ "ARNJ" }, // 0615 - Harukanaru Toki no Naka de (JPN)
	{ "AOPJ" }, // 0646 - Oshare Princess (JPN)
	{ "AHVJ" }, // 0664 - Nakayoshi Youchien - Sukoyaka Enji Ikusei Game (JPN)
	{ "AYCE" }, // 0758 - Phantasy Star Collection (USA)
	{ "AYCP" }, // 0877 - Phantasy Star Collection (EUR)
	{ "ATBJ" }, // 0948 - Slot! Pro 2 Advance - GoGo Juggler & New Tairyou (JPN)
	{ "A83J" }, // 1012 - Hamster Monogatari 3 GBA (JPN)
	{ "AEHJ" }, // 1014 - Puzzle & Tantei Collection (JPN)
	{ "BWDJ" }, // 1114 - Wan Nyan Doubutsu Byouin (JPN)
	{ "BKZE" }, // 1138 - Banjo-Kazooie - Grunty's Revenge (USA)
	{ "FZLJ" }, // 1369 - Famicom Mini Series 5 - Zelda no Denzetsu 1 (JPN)
	{ "BPVP" }, // 1738 - Pferd & Pony - Mein Pferdehof (EUR)
	{ "ACTY" }, // 1763 - Creatures (EUR)
	{ "BITJ" }, // 1811 - Onmyou Taisenki Zeroshik (JPN)
	{ "BOVJ" }, // 1848 - Bouken-Ou Beet - Busters Road (JPN)
	{ "BT3J" }, // 1852 - Tantei Jinguuji Saburou Shiroi Kage no Syoujyo (JPN)
	{ "BG8J" }, // 1853 - Ganbare! Dodge Fighters (JPN)
	{ "BLDS" }, // 1919 - 2 Games in 1 - Lizzie McGuire - Disney Princesas (ESP)
	{ "BLDP" }, // 1934 - 2 Games in 1 - Lizzie McGuire - Disney Princess (EUR)
	{ "B8MP" }, // 1993 - Mario Party Advance (EUR)
	{ "BYPP" }, // 2155 - Horse & Pony - Let`s Ride 2 (EUR)
	{ "B8AE" }, // 2174 - Crash Superpack - N-Tranced + Nitro Kart (USA)
	{ "BL9E" }, // 2329 - Let's Ride! Dreamer (USA)
	{ "B34E" }, // 2331 - Let's Ride! - Sunshine Stables (USA)
	{ "BQVP" }, // 2414 - Meine Tierarztpraxis (EUR)
	{ "BHUP" }, // 2480 - Horse and Pony - My Stud Farm (EUR)
	{ "BPVX" }, // 2645 - Pippa Funnell - Stable Adventures (EUR)
	{ "BYPX" }, // 2653 - Pippa Funell 2 (EUR)
	{ "BQVX" }, // 2711 - Mijn Dierenpraktijk (EUR)
	{ "BPVY" }, // 2712 - Paard & Pony - Mijn Manege (EUR)
	{ "BYPY" }, // 2713 - Paard & Pony - Paard in Galop (EUR)
	{ "B54E" }, // 2757 - Crash & Spyro Superpack - The Huge Adventure + Season of Ice (USA)
	// gba scan no. 9
	{ "BKZX" }, // 1199 - Banjo-Kazooie - Grunty's Revenge (EUR)
	{ "BKZI" }, // 1381 - Banjo Kazooie - La Vendetta di Grunty (ITA)
	{ "BAZJ" }, // 1710 - Akachan Doubutsu Sono (JPN)
	{ "BKZS" }, // 1883 - Banjo Kazooie - La Venganza de Grunty (ESP)
	// gba scan no. 11
	{ "A9BP" }, // 0925 - Medabots - Rokusho Version (EUR)
	{ "A3IJ" }, // bokura no taiyou - taiyou action rpg - kabunushi go-yuutai ban (japan) (demo)
};

static int gba_chip_has_conflict( UINT32 chip)
{
	int count1 = 0, count2 = 0;
	if (chip & GBA_CHIP_EEPROM) count1++;
	if (chip & GBA_CHIP_EEPROM_4K) count1++;
	if (chip & GBA_CHIP_EEPROM_64K) count1++;
	if (chip & GBA_CHIP_FLASH) count2++;
	if (chip & GBA_CHIP_FLASH_1M) count2++;
	if (chip & GBA_CHIP_FLASH_512) count2++;
	if (chip & GBA_CHIP_SRAM) count2++;
	return (count1 + count2) > 1; // if EEPROM + FLASH or EEPROM + SRAM carts exist, change to "(count1 > 1) || (count2 > 1)"
}

static UINT32 gba_fix_wrong_chip(running_machine &machine, UINT32 cart_size, UINT32 chip)
{
	char game_code[5] = { 0 };
	UINT8 *ROM = machine.root_device().memregion("cartridge")->base();

	if (cart_size >= 0xAC + 4)
	{
		memcpy(game_code, ROM + 0xAC, 4);
	}

	mame_printf_info( "GBA: Game Code \"%s\"\n", game_code);

	// fix for games which return more than one kind of chip: either it is one of the known titles, or we default to no battery
	if (gba_chip_has_conflict( chip))
	{
		int resolved = 0;
		chip &= ~(GBA_CHIP_EEPROM | GBA_CHIP_EEPROM_4K | GBA_CHIP_EEPROM_64K | GBA_CHIP_FLASH | GBA_CHIP_FLASH_1M | GBA_CHIP_FLASH_512 | GBA_CHIP_SRAM);
		for (int i = 0; i < sizeof( gba_chip_fix_conflict_list) / sizeof( gba_chip_fix_conflict_item); i++)
		{
			const gba_chip_fix_conflict_item *item = &gba_chip_fix_conflict_list[i];
			if (!strcmp( game_code, item->game_code))
			{
				chip |= item->chip;
				resolved = 1;
				break;
			}
		}
		if (!resolved)
		{
			mame_printf_warning( "GBA: NVRAM is disabled because multiple NVRAM chips were detected!\n");
		}
	}

	// fix for a games that require an eeprom with 14-bit addressing (64 kbit)
	if (chip & GBA_CHIP_EEPROM)
	{
		for (int i = 0; i < sizeof( gba_chip_fix_eeprom_list) / sizeof( gba_chip_fix_eeprom_item); i++)
		{
			const gba_chip_fix_eeprom_item *item = &gba_chip_fix_eeprom_list[i];
			if (!strcmp( game_code, item->game_code))
			{
				chip = (chip & ~GBA_CHIP_EEPROM) | GBA_CHIP_EEPROM_64K;
				break;
			}
		}
	}

	return chip;
}

typedef struct _gba_pcb  gba_pcb;
struct _gba_pcb
{
	const char              *pcb_name;
	int                     pcb_id;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const gba_pcb pcb_list[] =
{
	{"GBA-EEPROM",    GBA_CHIP_EEPROM},
	{"GBA-SRAM",      GBA_CHIP_SRAM},
	{"GBA-FLASH",     GBA_CHIP_FLASH},
	{"GBA-FLASH-1M",  GBA_CHIP_FLASH_1M},
	{"GBA-RTC",       GBA_CHIP_RTC},
	{"GBA-FLASH-512", GBA_CHIP_FLASH_512},
	{"GBA-EEPROM-4K", GBA_CHIP_EEPROM_4K},
	{"GBA-EEPROM-64K", GBA_CHIP_EEPROM_64K}
};

static int gba_get_pcb_id(const char *pcb)
{
	int	i;

	for (i = 0; i < ARRAY_LENGTH(pcb_list); i++)
	{
		if (!mame_stricmp(pcb_list[i].pcb_name, pcb))
			return pcb_list[i].pcb_id;
	}

	return 0;
}

static DEVICE_IMAGE_LOAD( gba_cart )
{
	UINT8 *ROM = image.device().machine().root_device().memregion("cartridge")->base();
	UINT32 cart_size;
	UINT32 chip = 0;
	gba_state *state = image.device().machine().driver_data<gba_state>();

	state->m_nvsize = 0;
	state->m_flash_size = 0;
	state->m_nvptr = (UINT8 *)NULL;
	state->m_flash_battery_load = 0;

	if (image.software_entry() == NULL)
	{
		cart_size = image.length();
		image.fread(ROM, cart_size);
	}
	else
	{
		const char *pcb_name = "";
		cart_size = image.get_software_region_length("rom");
		memcpy(ROM, image.get_software_region("rom"), cart_size);

		if ((pcb_name = image.get_feature("pcb_type")) == NULL)
			chip = 0;
		else
			chip = gba_get_pcb_id(pcb_name);

		mame_printf_info("Type from xml: %s\n", pcb_name);
		mame_printf_info( "GBA: Detected (XML) %s\n", gba_chip_string( chip).cstr());
	}

	if (!chip)
	{
		// detect nvram based on strings inside the file
		chip = gba_detect_chip(ROM, cart_size);
		mame_printf_info( "GBA: Detected (ROM) %s\n", gba_chip_string( chip).cstr());

		// fix the previous value when possible
		chip = gba_fix_wrong_chip(image.device().machine(), cart_size, chip);
	}

	mame_printf_info( "GBA: Emulate %s\n", gba_chip_string( chip).cstr());

	if ((chip & (GBA_CHIP_EEPROM | GBA_CHIP_EEPROM_4K | GBA_CHIP_EEPROM_64K)) != 0)
	{
		state->m_nvptr = (UINT8 *)&state->m_gba_eeprom;
		state->m_nvsize = (chip & GBA_CHIP_EEPROM_64K) ? 0x2000 : 0x200;

		state->m_eeprom_addr_bits = (chip & GBA_CHIP_EEPROM_64K) ? 14 : 6;

		if (cart_size <= (16 * 1024 * 1024))
		{
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0xd000000, 0xdffffff, read32_delegate(FUNC(gba_state::eeprom_r),state));
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0xd000000, 0xdffffff, write32_delegate(FUNC(gba_state::eeprom_w),state));
		}
		else
		{
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0xdffff00, 0xdffffff, read32_delegate(FUNC(gba_state::eeprom_r),state));
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0xdffff00, 0xdffffff, write32_delegate(FUNC(gba_state::eeprom_w),state));
		}
	}

	if (chip & GBA_CHIP_SRAM)
	{
		state->m_nvptr = (UINT8 *)&state->m_gba_sram;
		state->m_nvsize = 0x10000;

		image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0xe000000, 0xe00ffff, read32_delegate(FUNC(gba_state::sram_r),state));
		image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0xe000000, 0xe00ffff, write32_delegate(FUNC(gba_state::sram_w),state));
	}

	if (chip & GBA_CHIP_FLASH_1M)
	{
		state->m_nvptr = NULL;
		state->m_nvsize = 0;
		state->m_flash_size = 0x20000;
		state->m_flash_mask = 0x1ffff/4;

		image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0xe000000, 0xe01ffff, read32_delegate(FUNC(gba_state::flash_r),state));
		image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0xe000000, 0xe01ffff, write32_delegate(FUNC(gba_state::flash_w),state));
	}

	if ((chip & GBA_CHIP_FLASH) || (chip & GBA_CHIP_FLASH_512))
	{
		state->m_nvptr = NULL;
		state->m_nvsize = 0;
		state->m_flash_size = 0x10000;
		state->m_flash_mask = 0xffff/4;

		image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0xe000000, 0xe00ffff, read32_delegate(FUNC(gba_state::flash_r),state));
		image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0xe000000, 0xe00ffff, write32_delegate(FUNC(gba_state::flash_w),state));
	}

	if (chip & GBA_CHIP_RTC)
	{
		mame_printf_verbose("game has RTC\n");
	}

	// if save media was found, reload it
	if (state->m_nvsize > 0)
	{
		image.battery_load(state->m_nvptr, state->m_nvsize, 0x00);
		state->m_nvimage = image;
	}
	else
	{
		state->m_nvimage = NULL;
		state->m_nvsize = 0;
	}

	// init the flash here so it gets the contents from the battery_load above
	if (state->m_flash_size > 0)
	{
		if (state->m_flash_size == 0x10000)
			state->m_mFlashDev = image.device().machine().device<intelfsh8_device>("pflash");
		else
			state->m_mFlashDev = image.device().machine().device<intelfsh8_device>("sflash");
		state->m_flash_battery_load = 1;
		state->m_nvimage = image;
	}

	// mirror the ROM
	switch (cart_size)
	{
		case 2 * 1024 * 1024:
			memcpy(ROM + 0x200000, ROM, 0x200000);
		// intentional fall-through
		case 4 * 1024 * 1024:
			memcpy(ROM + 0x400000, ROM, 0x400000);
		// intentional fall-through
		case 8 * 1024 * 1024:
			memcpy(ROM + 0x800000, ROM, 0x800000);
		// intentional fall-through
		case 16 * 1024 * 1024:
			memcpy(ROM + 0x1000000, ROM, 0x1000000);
			break;
	}

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( gbadv, gba_state )

	MCFG_CPU_ADD("maincpu", ARM7, 16777216)
	MCFG_CPU_PROGRAM_MAP(gbadvance_map)

	MCFG_MACHINE_START(gba)
	MCFG_MACHINE_RESET(gba)

	MCFG_SCREEN_ADD("gbalcd", RASTER)	// htot hst vwid vtot vst vis
	MCFG_SCREEN_RAW_PARAMS(16777216/4, 308, 0,  240, 228, 0,  160)
	MCFG_SCREEN_UPDATE_DRIVER(gba_state, screen_update)

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_LENGTH(32768)
	MCFG_PALETTE_INIT( gba )

	MCFG_SPEAKER_STANDARD_STEREO("spkleft", "spkright")
	MCFG_SOUND_ADD("custom", GAMEBOY, 0)
	MCFG_SOUND_ROUTE(0, "spkleft", 0.50)
	MCFG_SOUND_ROUTE(1, "spkright", 0.50)
	MCFG_SOUND_ADD("direct_a_left", DAC, 0)			// GBA direct sound A left
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "spkleft", 0.50)
	MCFG_SOUND_ADD("direct_a_right", DAC, 0)		// GBA direct sound A right
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "spkright", 0.50)
	MCFG_SOUND_ADD("direct_b_left", DAC, 0)			// GBA direct sound B left
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "spkleft", 0.50)
	MCFG_SOUND_ADD("direct_b_right", DAC, 0)		// GBA direct sound B right
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "spkright", 0.50)

	MCFG_PANASONIC_MN63F805MNP_ADD("pflash")
	MCFG_SANYO_LE26FV10N1TS_ADD("sflash")

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("gba,bin")
	MCFG_CARTSLOT_INTERFACE("gba_cart")
	MCFG_CARTSLOT_LOAD(gba_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","gba")
MACHINE_CONFIG_END

/* this emulates the GBA's hardware protection: the BIOS returns only zeros when the PC is not in it,
   and some games verify that as a protection check (notably Metroid Fusion) */
DIRECT_UPDATE_MEMBER(gba_state::gba_direct)
{
	if (address > 0x4000)
	{
		m_bios_protected = 1;
	}
	else
	{
		m_bios_protected = 0;
		m_bios_last_address = address;
	}
	return address;
}

DRIVER_INIT_MEMBER(gba_state,gbadv)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM)->set_direct_update_handler(direct_update_delegate(FUNC(gba_state::gba_direct), this));
}

/*    YEAR  NAME PARENT COMPAT MACHINE INPUT   INIT   COMPANY     FULLNAME */
CONS( 2001, gba, 0,     0,     gbadv,  gbadv, gba_state,  gbadv, "Nintendo", "Game Boy Advance", GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND)
