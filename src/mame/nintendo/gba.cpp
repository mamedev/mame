// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

  gba.cpp

  Driver file to handle emulation of the Nintendo Game Boy Advance.

  By R. Belmont & Ryan Holtz

***************************************************************************/

#include "emu.h"
#include "gba.h"

#include "bus/gba/rom.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "sound/gb.h"
#include "softlist_dev.h"
#include "speaker.h"

#include <cstdarg>


/* Sound Registers */
#define SOUNDCNT_L  HWLO(0x080)  /* 0x4000080  2  R/W   Control Stereo/Volume/Enable */
#define SOUNDCNT_H  HWHI(0x080)  /* 0x4000082  2  R/W   Control Mixing/DMA Control */
#define SOUNDCNT_X  HWLO(0x084)  /* 0x4000084  2  R/W   Control Sound on/off */
								 /* 0x4000086  2  -     Unused */
#define SOUNDBIAS   HWLO(0x088)  /* 0x4000088  2  BIOS  Sound PWM Control */
								 /* 0x400008A  2  -     Unused */

/* DMA Registers (4 Transfer Channels) */
#define DMASAD(c)   WORD(0x0B0 + c * 0xC) /* 0x40000B0  4  W     DMA 0 Source Address */
#define DMADAD(c)   WORD(0x0B4 + c * 0xC) /* 0x40000B4  4  W     DMA 0 Destination Address */
#define DMACNT_L(c) HWLO(0x0B8 + c * 0xC) /* 0x40000B8  2  W     DMA 0 Word Count */
#define DMACNT_H(c) HWHI(0x0B8 + c * 0xC) /* 0x40000BA  2  R/W   DMA 0 Control */

/* Serial Communication (1) Registers */
#define SIODATA32   WORD(0x120)  /* 0x4000120  4  R/W   SIO Data (Normal-32bit Mode; shared with below) */
#define SIOMULTI0   HWLO(0x120)  /* 0x4000120  2  R/W   SIO Data 0 (Parent)    (Multi-Player Mode) */
#define SIOMULTI1   HWHI(0x120)  /* 0x4000122  2  R/W   SIO Data 1 (1st Child) (Multi-Player Mode) */
#define SIOMULTI2   HWLO(0x124)  /* 0x4000124  2  R/W   SIO Data 2 (2nd Child) (Multi-Player Mode) */
#define SIOMULTI3   HWHI(0x124)  /* 0x4000126  2  R/W   SIO Data 3 (3rd Child) (Multi-Player Mode) */
#define SIOCNT      HWLO(0x128)  /* 0x4000128  2  R/W   SIO Control Register */
#define SIOMLT_SEND HWHI(0x128)  /* 0x400012A  2  R/W   SIO Data (Local of MultiPlayer; shared below) */
#define SIODATA8    HWHI(0x128)  /* 0x400012A  2  R/W   SIO Data (Normal-8bit and UART Mode) */
								 /* 0x400012C  2  -     Unused */

/* Keypad Input Registers */
#define KEYINPUT    HWLO(0x130)  /* 0x4000130  2  R     Key Status */
#define KEYCNT      HWHI(0x130)  /* 0x4000132  2  R/W   Key Interrupt Control */

/* Serial Communication (2) Registers */
#define RCNT        HWLO(0x134)  /* 0x4000134  2  R/W   SIO Mode Select/General Purpose Data */
#define IR          HWHI(0x134)  /* 0x4000136  2  R/W   Ancient - Infrared Register (Prototypes only) */
								 /* 0x4000138  8  -     Unused */
#define JOYCNT      HWLO(0x140)  /* 0x4000140  2  R/W   SIO JOY Bus Control */
								 /* 0x4000142  2  -     Unused */
#define JOY_RECV    WORD(0x150)  /* 0x4000150  4  R/W   SIO JOY Bus Receive Data */
#define JOY_TRANS   WORD(0x154)  /* 0x4000154  4  R/W   SIO JOY Bus Transmit Data */
#define JOYSTAT     HWLO(0x158)  /* 0x4000158  2  R/?   SIO JOY Bus Receive Status */
								 /* 0x400015A  2  -     Unused */

/* Interrupt, Waitstate, and Power-Down Control Registers */
#define IE          HWLO(0x200)  /* 0x4000200  2  R/W   Interrupt Enable Register */
#define IF          HWHI(0x200)  /* 0x4000202  2  R/W   Interrupt Request Flags / IRQ Acknowledge */
#define WAITCNT     HWLO(0x204)  /* 0x4000204  2  R/W   Game Pak Waitstate Control */
								 /* 0x4000206     -     Unused */
#define IME         HWLO(0x208)  /* 0x4000208  2  R/W   Interrupt Master Enable Register */
								 /* 0x400020A     -     Unused */
								 /* 0x4000300  1  R/W   Undocumented - Post Boot Flag */
								 /* 0x4000301  1  W     Undocumented - Power Down Control */
								 /* 0x4000302     -     Unused */
								 /* 0x4000410  ?  ?     Undocumented - Purpose Unknown / Bug ??? 0FFh */
								 /* 0x4000411     -     Unused */
								 /* 0x4000800  4  R/W   Undocumented - Internal Memory Control (R/W) */
								 /* 0x4000804     -     Unused */
								 /* 0x4xx0800  4  R/W   Mirrors of 4000800h (repeated each 64K) */

#define SOUNDBIAS_SET(val)      HWLO_SET(0x088, val)

#define DMASAD_SET(c, val)      WORD_SET(0x0B0 + (c * 0xC), val)
#define DMADAD_SET(c, val)      WORD_SET(0x0B4 + (c * 0xC), val)
#define DMACNT_L_SET(c, val)    HWLO_SET(0x0B8 + (c * 0xC), val)
#define DMACNT_H_SET(c, val)    HWHI_SET(0x0B8 + (c * 0xC), val)
#define DMACNT_H_RESET(c, val)  HWHI_RESET(0x0B8 + (c * 0xC), val)

#define SIOMULTI0_SET(val)      HWLO_SET(0x120, val)
#define SIOMULTI1_SET(val)      HWHI_SET(0x120, val)
#define SIOMULTI2_SET(val)      HWLO_SET(0x124, val)
#define SIOMULTI3_SET(val)      HWHI_SET(0x124, val)

#define SIOCNT_RESET(val)       HWLO_RESET(0x128, val)

#define KEYCNT_SET(val)         HWHI_SET(0x130, val)

#define RCNT_SET(val)           HWLO_SET(0x134, val)

#define JOYSTAT_SET(val)        HWLO_SET(0x158, val)

#define IF_SET(val)             HWHI_SET(0x200, val)
#define IF_RESET(val)           HWHI_RESET(0x200, val)

#define INT_VBL                 0x0001
#define INT_HBL                 0x0002
#define INT_VCNT                0x0004
#define INT_TM0_OVERFLOW        0x0008
#define INT_TM1_OVERFLOW        0x0010
#define INT_TM2_OVERFLOW        0x0020
#define INT_TM3_OVERFLOW        0x0040
#define INT_SIO                 0x0080
#define INT_DMA0                0x0100
#define INT_DMA1                0x0200
#define INT_DMA2                0x0400
#define INT_DMA3                0x0800
#define INT_KEYPAD              0x1000
#define INT_GAMEPAK             0x2000

#define VERBOSE_LEVEL   (0)

static inline void ATTR_PRINTF(3,4) verboselog(device_t &device, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror( "%08x: %s", device.machine().describe_context(), buf );
	}
}

static const XTAL timer_clks[4] = { XTAL(16'777'216), XTAL(16'777'216) / 64, XTAL(16'777'216) / 256, XTAL(16'777'216) / 1024 };


void gba_state::request_irq(uint32_t int_type)
{
	// set flag for later recovery
	IF_SET(int_type);

	// is this specific interrupt enabled?
	int_type &= IE;
	if (int_type != 0)
	{
		// master enable?
		if (IME & 1)
		{
			m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
			m_maincpu->set_input_line(ARM7_IRQ_LINE, CLEAR_LINE);
		}
	}
}

TIMER_CALLBACK_MEMBER(gba_state::dma_complete)
{
	static const uint32_t ch_int[4] = { INT_DMA0, INT_DMA1, INT_DMA2, INT_DMA3 };

	uintptr_t ch = param;

//  printf("dma complete: ch %d\n", ch);

	m_dma_timer[ch]->adjust(attotime::never);

	int ctrl = DMACNT_H(ch);

	// IRQ
	if (ctrl & 0x4000)
	{
		request_irq(ch_int[ch]);
	}

	// if we're supposed to repeat, don't clear "active" and then the next vbl/hbl will retrigger us
	// always clear active for immediate DMAs though
	if (!((ctrl>>9) & 1) || ((ctrl & 0x3000) == 0))
	{
		DMACNT_H_RESET(ch, 0x8000); // clear "active" bit
	}
	else
	{
		// if repeat, reload the count
		if ((ctrl>>9) & 1)
		{
			m_dma_cnt[ch] = DMACNT_L(ch);

			// if increment & reload mode, reload the destination
			if (((ctrl>>5)&3) == 3)
			{
				m_dma_dst[ch] = DMADAD(ch);
			}
		}
	}
}

void gba_state::dma_exec(int ch)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint32_t src = m_dma_src[ch];
	uint32_t dst = m_dma_dst[ch];
	uint16_t ctrl = DMACNT_H(ch);
	int srcadd = (ctrl >> 7) & 3;
	int dstadd = (ctrl >> 5) & 3;

	int cnt = m_dma_cnt[ch];
	if (cnt == 0)
	{
		if (ch == 3)
			cnt = 0x10000;
		else
			cnt = 0x4000;
	}

	// override special parameters
	if ((ctrl & 0x3000) == 0x3000)      // special xfer mode
	{
		switch (ch)
		{
			case 1:         // Ch 1&2 are for audio DMA
			case 2:
				dstadd = 2; // don't increment destination
				cnt = 4;    // always transfer 4 32-bit words
				ctrl |= 0x400;  // always 32-bit
				break;

			case 3:
				printf("Unsupported DMA 3 special mode\n");
				break;
		}
	}
	else
	{
//      if (dst >= 0x6000000 && dst <= 0x6017fff)
//      printf("DMA exec: ch %d from %08x to %08x, mode %04x, count %04x (%s)\n", (int)ch, src, dst, ctrl, cnt, ((ctrl>>10) & 1) ? "32" : "16");
	}

	for (int i = 0; i < cnt; i++)
	{
		if ((ctrl>>10) & 1)
		{
			src &= 0xfffffffe;
			dst &= 0xfffffffe;

			// 32-bit
			space.write_dword(dst, space.read_dword(src));
			switch (dstadd)
			{
				case 0: // increment
					dst += 4;
					break;
				case 1: // decrement
					dst -= 4;
					break;
				case 2: // don't move
					break;
				case 3: // increment and reload
					dst += 4;
					break;
			}
			switch (srcadd)
			{
				case 0: // increment
					src += 4;
					break;
				case 1: // decrement
					src -= 4;
					break;
				case 2: // don't move
					break;
				case 3: // not used ("Metal Max 2 Kai" expects no increment/decrement)
					break;
			}
		}
		else
		{
			src &= 0xfffffffe;
			dst &= 0xfffffffe;

			// 16-bit
			space.write_word(dst, space.read_word(src));
			switch (dstadd)
			{
				case 0: // increment
					dst += 2;
					break;
				case 1: // decrement
					dst -= 2;
					break;
				case 2: // don't move
					break;
				case 3: // increment and reload
					dst += 2;
					break;
			}
			switch (srcadd)
			{
				case 0: // increment
					src += 2;
					break;
				case 1: // decrement
					src -= 2;
					break;
				case 2: // don't move
					break;
				case 3: // not used (see note in 32-bit version above)
					break;
			}
		}
	}

	m_dma_src[ch] = src;
	m_dma_dst[ch] = dst;

//  printf("settng DMA timer %d for %d cycs (tmr %x)\n", ch, cnt, (uint32_t)m_dma_timer[ch]);
//  m_dma_timer[ch]->adjust(ATTOTIME_IN_CYCLES(0, cnt), ch);
	dma_complete(ch);
}

void gba_state::audio_tick(int ref)
{
	if (!(SOUNDCNT_X & 0x80))
		return;

	if (!ref)
	{
		if (m_fifo_a_ptr != m_fifo_a_in)
		{
			if (m_fifo_a_ptr == 17)
			{
				m_fifo_a_ptr = 0;
			}

			if (SOUNDCNT_H & 0x200)
			{
				m_ldaca->write(m_fifo_a[m_fifo_a_ptr]);
			}
			if (SOUNDCNT_H & 0x100)
			{
				m_rdaca->write(m_fifo_a[m_fifo_a_ptr]);
			}
			m_fifo_a_ptr++;
		}

		// fifo empty?
		if (m_fifo_a_ptr == m_fifo_a_in)
		{
			// is a DMA set up to feed us?
			if ((DMADAD(1) == 0x40000a0) && ((DMACNT_H(1) & 0x3000) == 0x3000))
			{
				// channel 1 it is
				dma_exec(1);
			}
			if ((DMADAD(2) == 0x40000a0) && ((DMACNT_H(2) & 0x3000) == 0x3000))
			{
				// channel 2 it is
				dma_exec(2);
			}
		}
	}
	else
	{
		if (m_fifo_b_ptr != m_fifo_b_in)
		{
			if (m_fifo_b_ptr == 17)
			{
				m_fifo_b_ptr = 0;
			}

			if (SOUNDCNT_H & 0x2000)
			{
				m_ldacb->write(m_fifo_b[m_fifo_b_ptr]);
			}
			if (SOUNDCNT_H & 0x1000)
			{
				m_rdacb->write(m_fifo_b[m_fifo_b_ptr]);
			}
			m_fifo_b_ptr++;
		}

		if (m_fifo_b_ptr == m_fifo_b_in)
		{
			// is a DMA set up to feed us?
			if ((DMADAD(1) == 0x40000a4) && ((DMACNT_H(1) & 0x3000) == 0x3000))
			{
				// channel 1 it is
				dma_exec(1);
			}
			if ((DMADAD(2) == 0x40000a4) && ((DMACNT_H(2) & 0x3000) == 0x3000))
			{
				// channel 2 it is
				dma_exec(2);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(gba_state::timer_expire)
{
	static const uint32_t tmr_ints[4] = { INT_TM0_OVERFLOW, INT_TM1_OVERFLOW, INT_TM2_OVERFLOW, INT_TM3_OVERFLOW };
	uintptr_t tmr = (uintptr_t) param;

	// "The reload value is copied into the counter only upon following two situations: Automatically upon timer overflows,"
	// "or when the timer start bit becomes changed from 0 to 1."
	if (m_timer_recalc[tmr] != 0)
	{
		double rate, clocksel, final;
		attotime time;
		m_timer_recalc[tmr] = 0;
		m_timer_regs[tmr] = (m_timer_regs[tmr] & 0xFFFF0000) | (m_timer_reload[tmr] & 0x0000FFFF);
		rate = 0x10000 - (m_timer_regs[tmr] & 0xffff);
		clocksel = timer_clks[(m_timer_regs[tmr] >> 16) & 3].dvalue();
		final = clocksel / rate;
		m_timer_hz[tmr] = final;
		time = attotime::from_hz(final);
		m_tmr_timer[tmr]->adjust(time, tmr, time);
	}

	// check if timers 0 or 1 are feeding directsound
	if (tmr == 0)
	{
		if (!(SOUNDCNT_H & 0x400))
			audio_tick(0);

		if (!(SOUNDCNT_H & 0x4000))
			audio_tick(1);
	}

	if (tmr == 1)
	{
		if (SOUNDCNT_H & 0x400)
			audio_tick(0);

		if (SOUNDCNT_H & 0x4000)
			audio_tick(1);
	}

	// Handle count-up timing
	switch (tmr)
	{
	case 0:
		if (m_timer_regs[1] & 0x40000)
		{
			m_timer_regs[1] = (( ( m_timer_regs[1] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (m_timer_regs[1] & 0xffff0000);
			if( ( m_timer_regs[1] & 0x0000ffff ) == 0 )
			{
				m_timer_regs[1] |= m_timer_reload[1];
				if( ( m_timer_regs[1] & 0x400000 ) && ( IME != 0 ) )
				{
					request_irq(tmr_ints[1]);
				}
				if( ( m_timer_regs[2] & 0x40000 ) )
				{
					m_timer_regs[2] = (( ( m_timer_regs[2] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (m_timer_regs[2] & 0xffff0000);
					if( ( m_timer_regs[2] & 0x0000ffff ) == 0 )
					{
						m_timer_regs[2] |= m_timer_reload[2];
						if( ( m_timer_regs[2] & 0x400000 ) && ( IME != 0 ) )
						{
							request_irq(tmr_ints[2]);
						}
						if( ( m_timer_regs[3] & 0x40000 ) )
						{
							m_timer_regs[3] = (( ( m_timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (m_timer_regs[3] & 0xffff0000);
							if( ( m_timer_regs[3] & 0x0000ffff ) == 0 )
							{
								m_timer_regs[3] |= m_timer_reload[3];
								if( ( m_timer_regs[3] & 0x400000 ) && ( IME != 0 ) )
								{
									request_irq(tmr_ints[3]);
								}
							}
						}
					}
				}
			}
		}
		break;
	case 1:
		if (m_timer_regs[2] & 0x40000)
		{
			m_timer_regs[2] = (( ( m_timer_regs[2] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (m_timer_regs[2] & 0xffff0000);
			if( ( m_timer_regs[2] & 0x0000ffff ) == 0 )
			{
				m_timer_regs[2] |= m_timer_reload[2];
				if( ( m_timer_regs[2] & 0x400000 ) && ( IME != 0 ) )
				{
					request_irq(tmr_ints[2]);
				}
				if( ( m_timer_regs[3] & 0x40000 ) )
				{
					m_timer_regs[3] = (( ( m_timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (m_timer_regs[3] & 0xffff0000);
					if( ( m_timer_regs[3] & 0x0000ffff ) == 0 )
					{
						m_timer_regs[3] |= m_timer_reload[3];
						if( ( m_timer_regs[3] & 0x400000 ) && ( IME != 0 ) )
						{
							request_irq(tmr_ints[3]);
						}
					}
				}
			}
		}
		break;
	case 2:
		if (m_timer_regs[3] & 0x40000)
		{
			m_timer_regs[3] = (( ( m_timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (m_timer_regs[3] & 0xffff0000);
			if( ( m_timer_regs[3] & 0x0000ffff ) == 0 )
			{
				m_timer_regs[3] |= m_timer_reload[3];
				if( ( m_timer_regs[3] & 0x400000 ) && ( IME != 0 ) )
				{
					request_irq(tmr_ints[3]);
				}
			}
		}
		break;
	}

	// are we supposed to IRQ?
	if ((m_timer_regs[tmr] & 0x400000) && (IME != 0))
	{
		request_irq(tmr_ints[tmr]);
	}
}

TIMER_CALLBACK_MEMBER(gba_state::handle_irq)
{
	request_irq(IF);

	m_irq_timer->adjust(attotime::never);
}

static const char *const reg_names[] = {
	/* Sound Registers */
	"SOUND1CNT_L", "SOUND1CNT_H", "SOUND1CNT_X", "Unused",
	"SOUND2CNT_L", "Unused",      "SOUND2CNT_H", "Unused",
	"SOUND3CNT_L", "SOUND3CNT_H", "SOUND3CNT_X", "Unused",
	"SOUND4CNT_L", "Unused",      "SOUND4CNT_H", "Unused",
	"SOUNDCNT_L",  "SOUNDCNT_H",  "SOUNDCNT_X",  "Unused",
	"SOUNDBIAS",   "Unused",      "WAVE_RAM",    "WAVE_RAM",
	"WAVE_RAM",    "WAVE_RAM",    "WAVE_RAM",    "WAVE_RAM",
	"WAVE_RAM",    "WAVE_RAM",    "WAVE_RAM",    "WAVE_RAM",
	"WAVE_RAM",    "WAVE_RAM",    "WAVE_RAM",    "WAVE_RAM",
	"WAVE_RAM",    "WAVE_RAM",    "FIFO_A_L",    "FIFO_A_H",
	"FIFO_B_L",    "FIFO_B_H"
};

uint32_t gba_state::gba_io_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t retval = 0;

	switch( offset + 0x60/4 )
	{
		case 0x0060/4:
			retval = m_gbsound->sound_r(0) | m_gbsound->sound_r(1)<<16 | m_gbsound->sound_r(2)<<24;
			break;
		case 0x0064/4:
			retval = m_gbsound->sound_r(3) | m_gbsound->sound_r(4)<<8;
			break;
		case 0x0068/4:
			retval = m_gbsound->sound_r(6) | m_gbsound->sound_r(7)<<8;
			break;
		case 0x006c/4:
			retval = m_gbsound->sound_r(8) | m_gbsound->sound_r(9)<<8;
			break;
		case 0x0070/4:
			retval = m_gbsound->sound_r(0xa) | m_gbsound->sound_r(0xb)<<16 | m_gbsound->sound_r(0xc)<<24;
			break;
		case 0x0074/4:
			retval = m_gbsound->sound_r(0xd) | m_gbsound->sound_r(0xe)<<8;
			break;
		case 0x0078/4:
			retval = m_gbsound->sound_r(0x10) | m_gbsound->sound_r(0x11)<<8;
			break;
		case 0x007c/4:
			retval = m_gbsound->sound_r(0x12) | m_gbsound->sound_r(0x13)<<8;
			break;
		case 0x0080/4:
			retval = m_gbsound->sound_r(0x14) | m_gbsound->sound_r(0x15)<<8;
			if( ACCESSING_BITS_16_31 )
			{
				retval |= SOUNDCNT_H << 16;
			}
			break;
		case 0x0084/4:
			retval = m_gbsound->sound_r(0x16);
			break;
		case 0x0090/4:
			retval = m_gbsound->wave_r(0) | m_gbsound->wave_r(1)<<8 | m_gbsound->wave_r(2)<<16 | m_gbsound->wave_r(3)<<24;
			break;
		case 0x0094/4:
			retval = m_gbsound->wave_r(4) | m_gbsound->wave_r(5)<<8 | m_gbsound->wave_r(6)<<16 | m_gbsound->wave_r(7)<<24;
			break;
		case 0x0098/4:
			retval = m_gbsound->wave_r(8) | m_gbsound->wave_r(9)<<8 | m_gbsound->wave_r(10)<<16 | m_gbsound->wave_r(11)<<24;
			break;
		case 0x009c/4:
			retval = m_gbsound->wave_r(12) | m_gbsound->wave_r(13)<<8 | m_gbsound->wave_r(14)<<16 | m_gbsound->wave_r(15)<<24;
			break;
		case 0x00a0/4:
		case 0x00a4/4:
			retval = 0; // (does this actually do anything on real h/w?)
			break;
		case 0x00b0/4:
		case 0x00b4/4:
			// read only
			break;
		case 0x00b8/4:
			if (ACCESSING_BITS_0_15)
			{
				// read only
			}
			if (ACCESSING_BITS_16_31)
			{
				retval |= DMACNT_H(0) << 16;
			}
			break;
		case 0x00bc/4:
		case 0x00c0/4:
			// read only
			break;
		case 0x00c4/4:
			if (ACCESSING_BITS_0_15)
			{
				// read only
			}
			if (ACCESSING_BITS_16_31)
			{
				retval |= DMACNT_H(1) << 16;
			}
			break;
		case 0x00c8/4:
		case 0x00cc/4:
			// read only
			break;
		case 0x00d0/4:
			if (ACCESSING_BITS_0_15)
			{
				// read only
			}
			if (ACCESSING_BITS_16_31)
			{
				retval |= DMACNT_H(2) << 16;
			}
			break;
		case 0x00d4/4:
		case 0x00d8/4:
			// read only
			break;
		case 0x00dc/4:
				// no idea why here, but it matches VBA better
				// note: this suspicious piece of code crashes "Buffy The Vampire Slayer" (08008DB4) and "The Ant Bully", so disable it for now
			if (ACCESSING_BITS_0_15)
			{
				// read only
			}
			if (ACCESSING_BITS_16_31)
			{
				retval |= DMACNT_H(3) << 16;
			}
			break;
		case 0x0100/4:
		case 0x0104/4:
		case 0x0108/4:
		case 0x010c/4:
			{
				uint32_t elapsed;
				double time, ticks;
				int timer = offset + 0x60/4 - 0x100/4;

//              printf("Read timer reg %x (PC=%x)\n", timer, m_maincpu->pc());

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

						elapsed = (uint32_t)time;
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
		case 0x0130/4:
			if( ACCESSING_BITS_0_15 )   // KEYINPUT
			{
				retval = m_io_inputs->read();
			}
			else if( ACCESSING_BITS_16_31 )
			{
				retval |= KEYCNT << 16;
			}
			break;
		case 0x0200/4:
			retval = IE | (IF << 16);
			break;
		default:
			if( ACCESSING_BITS_0_15 )
			{
				retval |= m_regs[offset] & 0x0000ffff;
			}
			if( ACCESSING_BITS_16_31 )
			{
				retval |= m_regs[offset] & 0xffff0000;
			}
			break;
	}

//  assert_always(offset < std::size(reg_names) / 2, "Not enough register names in gba_state");

	if (ACCESSING_BITS_0_15)
	{
		verboselog(*this, 2, "GBA I/O Read: %s = %04x\n", reg_names[offset * 2], retval & 0x0000ffff);
	}
	if (ACCESSING_BITS_16_31)
	{
		verboselog(*this, 2, "GBA I/O Read: %s = %04x\n", reg_names[offset * 2 + 1], (retval & 0xffff0000) >> 16);
	}

	return retval;
}

void gba_state::gba_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint8_t soundcnt_x = SOUNDCNT_X;
	uint16_t siocnt = SIOCNT;
	uint16_t dmachcnt[4] = { DMACNT_H(0), DMACNT_H(1), DMACNT_H(2), DMACNT_H(3) };

	COMBINE_DATA(&m_regs[offset]);

//  assert_always(offset < std::size(reg_names) / 2, "Not enough register names in gba_state");

	if (ACCESSING_BITS_0_15)
	{
		verboselog(*this, 2, "GBA I/O Write: %s = %04x\n", reg_names[offset * 2], data & 0x0000ffff);
	}
	if (ACCESSING_BITS_16_31)
	{
		verboselog(*this, 2, "GBA I/O Write: %s = %04x\n", reg_names[offset * 2 + 1], (data & 0xffff0000) >> 16);
	}

	switch( offset + 0x60/4 )
	{
		case 0x0060/4:
			if( ACCESSING_BITS_0_7 )   // SOUNDCNTL
			{
				m_gbsound->sound_w(0, data);
			}
			if( ACCESSING_BITS_16_23 )
			{
				m_gbsound->sound_w(1, data>>16);  // SOUND1CNT_H
			}
			if( ACCESSING_BITS_24_31 )
			{
				m_gbsound->sound_w(2, data>>24);
			}
			break;
		case 0x0064/4:
			if( ACCESSING_BITS_0_7 )   // SOUNDCNTL
			{
				m_gbsound->sound_w(3, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->sound_w(4, data>>8);   // SOUND1CNT_H
			}
			break;
		case 0x0068/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->sound_w(6, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->sound_w(7, data>>8);
			}
			break;
		case 0x006c/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->sound_w(8, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->sound_w(9, data>>8);
			}
			break;
		case 0x0070/4:  //SND3CNTL and H
			if( ACCESSING_BITS_0_7 )   // SOUNDCNTL
			{
				m_gbsound->sound_w(0xa, data);
			}
			if( ACCESSING_BITS_16_23 )
			{
				m_gbsound->sound_w(0xb, data>>16);    // SOUND1CNT_H
			}
			if( ACCESSING_BITS_24_31 )
			{
				m_gbsound->sound_w(0xc, data>>24);
			}
			break;
		case 0x0074/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->sound_w(0xd, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->sound_w(0xe, data>>8);
			}
			break;
		case 0x0078/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->sound_w(0x10, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->sound_w(0x11, data>>8);
			}
			break;
		case 0x007c/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->sound_w(0x12, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->sound_w(0x13, data>>8);
			}
			break;
		case 0x0080/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->sound_w(0x14, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->sound_w(0x15, data>>8);
			}

			if (ACCESSING_BITS_16_31)
			{
				// DAC A reset?
				if (data & 0x08000000)
				{
					m_fifo_a_ptr = 17;
					m_fifo_a_in = 17;
					m_ldaca->write(0);
					m_rdaca->write(0);
				}

				// DAC B reset?
				if (data & 0x80000000)
				{
					m_fifo_b_ptr = 17;
					m_fifo_b_in = 17;
					m_ldacb->write(0);
					m_rdacb->write(0);
				}
			}
			break;
		case 0x0084/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->sound_w(0x16, data);
				if ((data & 0x80) && !(soundcnt_x & 0x80))
				{
					m_fifo_a_ptr = m_fifo_a_in = 17;
					m_fifo_b_ptr = m_fifo_b_in = 17;
					m_ldaca->write(0);
					m_rdaca->write(0);
					m_ldacb->write(0);
					m_rdacb->write(0);
				}
			}
			break;
		case 0x0090/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->wave_w(0, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->wave_w(1, data>>8);
			}
			if( ACCESSING_BITS_16_23 )
			{
				m_gbsound->wave_w(2, data>>16);
			}
			if( ACCESSING_BITS_24_31 )
			{
				m_gbsound->wave_w(3, data>>24);
			}
			break;
		case 0x0094/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->wave_w(4, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->wave_w(5, data>>8);
			}
			if( ACCESSING_BITS_16_23 )
			{
				m_gbsound->wave_w(6, data>>16);
			}
			if( ACCESSING_BITS_24_31 )
			{
				m_gbsound->wave_w(7, data>>24);
			}
			break;
		case 0x0098/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->wave_w(8, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->wave_w(9, data>>8);
			}
			if( ACCESSING_BITS_16_23 )
			{
				m_gbsound->wave_w(0xa, data>>16);
			}
			if( ACCESSING_BITS_24_31 )
			{
				m_gbsound->wave_w(0xb, data>>24);
			}
			break;
		case 0x009c/4:
			if( ACCESSING_BITS_0_7 )
			{
				m_gbsound->wave_w(0xc, data);
			}
			if( ACCESSING_BITS_8_15 )
			{
				m_gbsound->wave_w(0xd, data>>8);
			}
			if( ACCESSING_BITS_16_23 )
			{
				m_gbsound->wave_w(0xe, data>>16);
			}
			if( ACCESSING_BITS_24_31 )
			{
				m_gbsound->wave_w(0xf, data>>24);
			}
			break;
		case 0x00a0/4:
			if (ACCESSING_BITS_0_7)
			{
				m_fifo_a_in %= 17;
				m_fifo_a[m_fifo_a_in++] = (data)&0xff;
			}
			if (ACCESSING_BITS_8_15)
			{
				m_fifo_a_in %= 17;
				m_fifo_a[m_fifo_a_in++] = (data>>8)&0xff;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_fifo_a_in %= 17;
				m_fifo_a[m_fifo_a_in++] = (data>>16)&0xff;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_fifo_a_in %= 17;
				m_fifo_a[m_fifo_a_in++] = (data>>24)&0xff;
			}
			break;
		case 0x00a4/4:
			if (ACCESSING_BITS_0_7)
			{
				m_fifo_b_in %= 17;
				m_fifo_b[m_fifo_b_in++] = (data)&0xff;
			}
			if (ACCESSING_BITS_8_15)
			{
				m_fifo_b_in %= 17;
				m_fifo_b[m_fifo_b_in++] = (data>>8)&0xff;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_fifo_b_in %= 17;
				m_fifo_b[m_fifo_b_in++] = (data>>16)&0xff;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_fifo_b_in %= 17;
				m_fifo_b[m_fifo_b_in++] = (data>>24)&0xff;
			}
			break;
		case 0x00b8/4:
		case 0x00c4/4:
		case 0x00d0/4:
		case 0x00dc/4:
			if( ACCESSING_BITS_16_31 )
			{
				int ch = (offset + 0x60/4 - 0xb0/4) / 3;

				int ctrl = data>>16;

				// Note: Metroid Fusion fails if we enforce the "rising edge" requirement... (who wrote this note?)

				// Note: Caesar's Palace Advance fails if we DO NOT enforce the "rising edge" requirement
				// (value @ 0x3003F9C is accidentally incremented because DMA completion interrupt is accidentally triggered @ 08002F2A)

				// retrigger/restart on a rising edge.
				// also reload internal regs
				if ((ctrl & 0x8000) && !(dmachcnt[ch] & 0x8000))
				{
					m_dma_src[ch] = DMASAD(ch);
					m_dma_dst[ch] = DMADAD(ch);
					m_dma_cnt[ch] = DMACNT_L(ch);

					// immediate start
					if ((ctrl & 0x3000) == 0)
					{
						dma_exec(ch);
						return;
					}
				}
			}
			break;
		case 0x0100/4:
		case 0x0104/4:
		case 0x0108/4:
		case 0x010c/4:
			{
				double rate, clocksel;
				uint32_t old_timer_regs;

				int timer = offset + 0x60/4 - 0x100/4;

				old_timer_regs = m_timer_regs[timer];

				m_timer_regs[timer] = (m_timer_regs[timer] & ~(mem_mask & 0xFFFF0000)) | (data & (mem_mask & 0xFFFF0000));

//              printf("%x to timer %d (mask %x PC %x)\n", data, timer, ~mem_mask, m_maincpu->pc());

				if (ACCESSING_BITS_0_15)
				{
					m_timer_reload[timer] = ((m_timer_reload[timer] & ~mem_mask) | (data & mem_mask)) & 0x0000FFFF;
					m_timer_recalc[timer] = 1;
				}

				// enabling this timer?
				if ((ACCESSING_BITS_16_31) && (data & 0x800000))
				{
					double final;

					if ((old_timer_regs & 0x00800000) == 0) // start bit 0 -> 1
					{
						m_timer_regs[timer] = (m_timer_regs[timer] & 0xFFFF0000) | (m_timer_reload[timer] & 0x0000FFFF);
					}

					rate = 0x10000 - (m_timer_regs[timer] & 0xffff);

					clocksel = timer_clks[(m_timer_regs[timer] >> 16) & 3].dvalue();

					final = clocksel / rate;

					m_timer_hz[timer] = final;

					m_timer_recalc[timer] = 0;

//                  printf("Enabling timer %d @ %f Hz\n", timer, final);

					// enable the timer
					if( !(data & 0x40000) ) // if we're not in Count-Up mode
					{
						attotime time = attotime::from_hz(final);
						m_tmr_timer[timer]->adjust(time, timer, time);
					}
				}
			}
			break;
		case 0x0128/4:
			if( ACCESSING_BITS_0_15 )
			{
				// normal mode ?
				if (!(RCNT & 0x8000) && !(data & 0x2000))
				{
					// start ?
					if (!(siocnt & 0x0080) && (data & 0x0080))
					{
						SIOCNT_RESET(0x0080);
						// request interrupt ?
						if (data & 0x4000)
						{
							request_irq(INT_SIO);
						}
					}
				}
			}
			break;
		case 0x0200/4:
			if( ACCESSING_BITS_0_15 )
			{
#if 0
				if (IE & IF)
				{
					request_irq(IF);
				}
#endif
			}
			if( ACCESSING_BITS_16_31 )
			{
				IF_RESET(( data & mem_mask ) >> 16);

				// if we still have interrupts, yank the IRQ line again
				if (IF)
				{
					m_irq_timer->adjust(m_maincpu->clocks_to_attotime(120));
				}
			}
			break;
		case 0x0208/4:
			if( ACCESSING_BITS_0_15 )
			{
				if (IF)
				{
					m_irq_timer->adjust(attotime::zero);
				}
			}
			break;
		case 0x0300/4:
			if( ACCESSING_BITS_8_15 )
			{
				// power down commanded, halt the CPU
				m_maincpu->spin_until_interrupt();
			}
			break;
	}
}

uint32_t gba_cons_state::gba_bios_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t *rom = m_region_maincpu;
	if (m_bios_hack->read())
	{
		// partially patch out logo and checksum checks
		// (useful to run some protos + to test homebrew)
		if (ACCESSING_BITS_0_15 && (offset == 0x6fc/4))
			return 0;
	}

	if (m_maincpu->pc() >= 0x4000)
	{
		//printf("GBA protection: blocking PC=%x\n", m_maincpu->pc());
		return 0;
	}

	return rom[offset & 0x3fff];
}

uint32_t gba_state::gba_10000000_r(offs_t offset, uint32_t mem_mask)
{
	auto &mspace = m_maincpu->space(AS_PROGRAM);
	uint32_t data;
	uint32_t pc = m_maincpu->state_int(ARM7_PC);
	if (pc >= 0x10000000)
	{
		return 0;
	}
	uint32_t cpsr = m_maincpu->state_int(ARM7_CPSR);
	if (T_IS_SET( cpsr))
	{
		data = mspace.read_dword(pc + 8);
	}
	else
	{
		uint16_t insn = mspace.read_word(pc + 4);
		data = (insn << 16) | (insn << 0);
	}
	logerror("%s: unmapped program memory read from %08X = %08X & %08X\n", machine().describe_context( ), 0x10000000 + (offset << 2), data, mem_mask);
	return data;
}

void gba_state::int_hblank_callback(int state)
{
	request_irq(INT_HBL);
}

void gba_state::int_vblank_callback(int state)
{
	request_irq(INT_VBL);
}

void gba_state::int_vcount_callback(int state)
{
	request_irq(INT_VCNT);
}

void gba_state::dma_hblank_callback(int state)
{
	for (int ch = 0; ch < 4; ch++)
	{
		int ctrl = DMACNT_H(ch);

		if ((ctrl & 0x8000) && ((ctrl & 0x3000) == 0x2000))
			dma_exec(ch);
	}
}

void gba_state::dma_vblank_callback(int state)
{
	for (int ch = 0; ch < 4; ch++)
	{
		int ctrl = DMACNT_H(ch);

		if ((ctrl & 0x8000) && ((ctrl & 0x3000) == 0x1000))
			dma_exec(ch);
	}
}

void gba_state::gba_map(address_map &map)
{
	map.unmap_value_high(); // for "Fruit Mura no Doubutsu Tachi" and "Classic NES Series"
	map(0x02000000, 0x0203ffff).ram().mirror(0xfc0000); // External RAM (16 bit)
	map(0x03000000, 0x03007fff).ram().mirror(0xff8000); // Internal RAM (32 bit)
	map(0x04000000, 0x0400005f).rw("lcd", FUNC(gba_lcd_device::video_r), FUNC(gba_lcd_device::video_w));
	map(0x04000060, 0x040003ff).rw(FUNC(gba_state::gba_io_r), FUNC(gba_state::gba_io_w));
	map(0x04000400, 0x04ffffff).noprw();                                         // Not used
	map(0x05000000, 0x050003ff).mirror(0x00fffc00).rw("lcd", FUNC(gba_lcd_device::gba_pram_r), FUNC(gba_lcd_device::gba_pram_w));  // Palette RAM
	map(0x06000000, 0x06017fff).mirror(0x00fe0000).rw("lcd", FUNC(gba_lcd_device::gba_vram_r), FUNC(gba_lcd_device::gba_vram_w));  // VRAM
	map(0x06018000, 0x0601ffff).mirror(0x00fe0000).rw("lcd", FUNC(gba_lcd_device::gba_vram_r), FUNC(gba_lcd_device::gba_vram_w));  // VRAM
	map(0x07000000, 0x070003ff).mirror(0x00fffc00).rw("lcd", FUNC(gba_lcd_device::gba_oam_r), FUNC(gba_lcd_device::gba_oam_w));    // OAM

	map(0x10000000, 0xffffffff).r(FUNC(gba_state::gba_10000000_r)); // for "Justice League Chronicles" (game bug)
}

void gba_cons_state::gba_cons_map(address_map &map)
{
	gba_map(map);

	map(0x00000000, 0x00003fff).rom().mirror(0x01ffc000).r(FUNC(gba_cons_state::gba_bios_r));
	//map(0x08000000, 0x0cffffff)  // cart ROM + mirrors, mapped here at machine_start if a cart is present
}

void gba_robotech_state::gba_robotech_map(address_map &map)
{
	gba_map(map);

	map(0x00000000, 0x007fffff).rom().region("maincpu", 0x00000000); // first part of the ROM is a BIOS replacement?
	map(0x08000000, 0x087fffff).rom().region("maincpu", 0x00800000); // second part is the game?
}

static INPUT_PORTS_START( gbadv )
	PORT_START("INPUTS")
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 L") PORT_PLAYER(1) // L
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 R") PORT_PLAYER(1) // R
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) // START
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT ) PORT_PLAYER(1)    // SELECT
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("B") PORT_PLAYER(1)    // B
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("A") PORT_PLAYER(1)    // A
INPUT_PORTS_END

static INPUT_PORTS_START( gbadv_cons )
	PORT_INCLUDE( gbadv )

	PORT_START("SKIP_CHECK")
	PORT_CONFNAME( 0x01, 0x00, "[HACK] Skip BIOS Logo check" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END


void gba_state::machine_reset()
{
	memset(m_regs, 0, sizeof(m_regs));

	SOUNDBIAS_SET(0x0200);
	SIOMULTI0_SET(0xffff);
	SIOMULTI1_SET(0xffff);
	SIOMULTI2_SET(0xffff);
	SIOMULTI3_SET(0xffff);
	KEYCNT_SET(0x03ff);
	RCNT_SET(0x8000);
	JOYSTAT_SET(0x0002);

	m_dma_timer[0]->adjust(attotime::never);
	m_dma_timer[1]->adjust(attotime::never, 1);
	m_dma_timer[2]->adjust(attotime::never, 2);
	m_dma_timer[3]->adjust(attotime::never, 3);

	m_fifo_a_ptr = m_fifo_b_ptr = 17;   // indicate empty
	m_fifo_a_in = m_fifo_b_in = 17;

	// and clear the DACs
	m_ldaca->write(0);
	m_rdaca->write(0);
	m_ldacb->write(0);
	m_rdacb->write(0);
}

void gba_state::machine_start()
{
	/* and one for each DMA channel */
	m_dma_timer[0] = timer_alloc(FUNC(gba_state::dma_complete), this);
	m_dma_timer[1] = timer_alloc(FUNC(gba_state::dma_complete), this);
	m_dma_timer[2] = timer_alloc(FUNC(gba_state::dma_complete), this);
	m_dma_timer[3] = timer_alloc(FUNC(gba_state::dma_complete), this);
	m_dma_timer[0]->adjust(attotime::never);
	m_dma_timer[1]->adjust(attotime::never, 1);
	m_dma_timer[2]->adjust(attotime::never, 2);
	m_dma_timer[3]->adjust(attotime::never, 3);

	/* also one for each timer (heh) */
	m_tmr_timer[0] = timer_alloc(FUNC(gba_state::timer_expire), this);
	m_tmr_timer[1] = timer_alloc(FUNC(gba_state::timer_expire), this);
	m_tmr_timer[2] = timer_alloc(FUNC(gba_state::timer_expire), this);
	m_tmr_timer[3] = timer_alloc(FUNC(gba_state::timer_expire), this);
	m_tmr_timer[0]->adjust(attotime::never);
	m_tmr_timer[1]->adjust(attotime::never, 1);
	m_tmr_timer[2]->adjust(attotime::never, 2);
	m_tmr_timer[3]->adjust(attotime::never, 3);

	/* and an IRQ handling timer */
	m_irq_timer = timer_alloc(FUNC(gba_state::handle_irq), this);
	m_irq_timer->adjust(attotime::never);

	save_item(NAME(m_regs));
	save_item(NAME(m_dma_src));
	save_item(NAME(m_dma_dst));
	save_item(NAME(m_dma_cnt));
	save_item(NAME(m_timer_regs));
	save_item(NAME(m_timer_reload));
	save_item(NAME(m_timer_recalc));
	save_item(NAME(m_timer_hz));
	save_item(NAME(m_fifo_a_ptr));
	save_item(NAME(m_fifo_b_ptr));
	save_item(NAME(m_fifo_a_in));
	save_item(NAME(m_fifo_b_in));
	save_item(NAME(m_fifo_a));
	save_item(NAME(m_fifo_b));
}

void gba_cons_state::machine_start()
{
	gba_state::machine_start();

	// install the cart ROM & SRAM into the address map, if present
	if (m_cart->exists())
	{

		std::string region_tag;
		memory_region *cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GBASLOT_ROM_REGION_TAG));

		// install ROM accesses
		m_maincpu->space(AS_PROGRAM).install_rom(0x08000000, 0x09ffffff, cart_rom->base());
		m_maincpu->space(AS_PROGRAM).install_rom(0x0a000000, 0x0bffffff, cart_rom->base());
		m_maincpu->space(AS_PROGRAM).install_rom(0x0c000000, 0x0cffffff, cart_rom->base());

		m_maincpu->space(AS_PROGRAM).install_read_handler(0x80000c4, 0x80000cb, read32s_delegate(*m_cart, FUNC(gba_cart_slot_device::read_gpio)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x80000c4, 0x80000cb, write32s_delegate(*m_cart, FUNC(gba_cart_slot_device::write_gpio)));

		// add nvram to save state
		m_cart->save_nvram();

		// install the cart NVRAM handlers if necessary
		if (m_cart->get_type() == GBA_SRAM || m_cart->get_type() == GBA_DRILLDOZ || m_cart->get_type() == GBA_WARIOTWS)
		{
			m_maincpu->space(AS_PROGRAM).install_read_handler(0xe000000, 0xe00ffff, read32s_delegate(*m_cart, FUNC(gba_cart_slot_device::read_ram)));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0xe000000, 0xe00ffff, write32s_delegate(*m_cart, FUNC(gba_cart_slot_device::write_ram)));
			m_maincpu->space(AS_PROGRAM).install_read_handler(0xe010000, 0xe01ffff, read32s_delegate(*m_cart, FUNC(gba_cart_slot_device::read_ram)));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0xe010000, 0xe01ffff, write32s_delegate(*m_cart, FUNC(gba_cart_slot_device::write_ram)));
		}
		if (m_cart->get_type() == GBA_EEPROM || m_cart->get_type() == GBA_EEPROM4 || m_cart->get_type() == GBA_EEPROM64 || m_cart->get_type() == GBA_BOKTAI)
		{
			// for games larger than 16MB the actual range is smaller but read_ram/write_ram handles that!
			m_maincpu->space(AS_PROGRAM).install_read_handler(0xd000000, 0xdffffff, read32s_delegate(*m_cart, FUNC(gba_cart_slot_device::read_ram)));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0xd000000, 0xdffffff, write32s_delegate(*m_cart, FUNC(gba_cart_slot_device::write_ram)));
		}
		if (m_cart->get_type() == GBA_YOSHIUG)
		{
			// EEPROM
			m_maincpu->space(AS_PROGRAM).install_read_handler(0xd000000, 0xdffffff, read32s_delegate(*m_cart, FUNC(gba_cart_slot_device::read_ram)));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0xd000000, 0xdffffff, write32s_delegate(*m_cart, FUNC(gba_cart_slot_device::write_ram)));
			// Tilt Sensor
			m_maincpu->space(AS_PROGRAM).install_read_handler(0xe008000, 0xe0085ff, read32s_delegate(*m_cart, FUNC(gba_cart_slot_device::read_tilt)));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0xe008000, 0xe0085ff, write32sm_delegate(*m_cart, FUNC(gba_cart_slot_device::write_tilt)));
		}
		// merge the two flash and mask accesses in read_ram?!?
		if (m_cart->get_type() == GBA_FLASH || m_cart->get_type() == GBA_FLASH512 || m_cart->get_type() == GBA_FLASH_RTC)
		{
			m_maincpu->space(AS_PROGRAM).install_read_handler(0xe000000, 0xe00ffff, read32s_delegate(*m_cart, FUNC(gba_cart_slot_device::read_ram)));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0xe000000, 0xe00ffff, write32s_delegate(*m_cart, FUNC(gba_cart_slot_device::write_ram)));
		}
		if (m_cart->get_type() == GBA_FLASH1M || m_cart->get_type() == GBA_FLASH1M_RTC)
		{
			m_maincpu->space(AS_PROGRAM).install_read_handler(0xe000000, 0xe01ffff, read32s_delegate(*m_cart, FUNC(gba_cart_slot_device::read_ram)));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0xe000000, 0xe01ffff, write32s_delegate(*m_cart, FUNC(gba_cart_slot_device::write_ram)));
		}
		if (m_cart->get_type() == GBA_3DMATRIX)
		{
			m_maincpu->space(AS_PROGRAM).install_write_handler(0x08800000, 0x088001ff, write32sm_delegate(*m_cart, FUNC(gba_cart_slot_device::write_mapper)));
			memory_region *cart_romhlp = memregion(region_tag.assign(m_cart->tag()).append(GBAHELP_ROM_REGION_TAG));
			m_maincpu->space(AS_PROGRAM).install_rom(0x08000000, 0x09ffffff, cart_romhlp->base());
		}

	}
}

static void gba_cart(device_slot_interface &device)
{
	device.option_add_internal("gba_rom",          GBA_ROM_STD);
	device.option_add_internal("gba_sram",         GBA_ROM_SRAM);
	device.option_add_internal("gba_drilldoz",     GBA_ROM_DRILLDOZ);       // Rumble output unemulated
	device.option_add_internal("gba_wariotws",     GBA_ROM_WARIOTWS);       // Rumble output unemulated
	device.option_add_internal("gba_eeprom",       GBA_ROM_EEPROM);
	device.option_add_internal("gba_eeprom_4k",    GBA_ROM_EEPROM);
	device.option_add_internal("gba_yoshiug",      GBA_ROM_YOSHIUG);
	device.option_add_internal("gba_eeprom_64k",   GBA_ROM_EEPROM64);
	device.option_add_internal("gba_boktai",       GBA_ROM_BOKTAI);
	device.option_add_internal("gba_flash",        GBA_ROM_FLASH);          // Panasonic
	device.option_add_internal("gba_flash_rtc",    GBA_ROM_FLASH_RTC);      // Panasonic
	device.option_add_internal("gba_flash_512",    GBA_ROM_FLASH);          // Panasonic
	device.option_add_internal("gba_flash_1m",     GBA_ROM_FLASH1M);        // Sanyo
	device.option_add_internal("gba_flash_1m_rtc", GBA_ROM_FLASH1M_RTC);    // Sanyo
	device.option_add_internal("gba_3dmatrix",     GBA_ROM_3DMATRIX);
}


void gba_state::gbadv(machine_config &config)
{
	ARM7(config, m_maincpu, 4.194304_MHz_XTAL * 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &gba_state::gba_map);

	gba_lcd_device &lcd(GBA_LCD(config, "lcd", 0));
	lcd.int_hblank_callback().set(FUNC(gba_state::int_hblank_callback));
	lcd.int_vblank_callback().set(FUNC(gba_state::int_vblank_callback));
	lcd.int_vcount_callback().set(FUNC(gba_state::int_vcount_callback));
	lcd.dma_hblank_callback().set(FUNC(gba_state::dma_hblank_callback));
	lcd.dma_vblank_callback().set(FUNC(gba_state::dma_vblank_callback));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	AGB_APU(config, m_gbsound, 4.194304_MHz_XTAL);
	m_gbsound->add_route(0, "lspeaker", 0.5);
	m_gbsound->add_route(1, "rspeaker", 0.5);

	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_ldaca, 0).add_route(ALL_OUTPUTS, "lspeaker", 0.5); // unknown DAC
	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_rdaca, 0).add_route(ALL_OUTPUTS, "rspeaker", 0.5); // unknown DAC
	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_ldacb, 0).add_route(ALL_OUTPUTS, "lspeaker", 0.5); // unknown DAC
	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_rdacb, 0).add_route(ALL_OUTPUTS, "rspeaker", 0.5); // unknown DAC

}

void gba_cons_state::gbadv_cons(machine_config &config)
{
	gbadv(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gba_cons_state::gba_cons_map);

	GBA_CART_SLOT(config, m_cart, gba_cart, nullptr);
	SOFTWARE_LIST(config, "cart_list").set_original("gba");
}

void gba_robotech_state::gbadv_robotech(machine_config &config)
{
	gbadv(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gba_robotech_state::gba_robotech_map);
}

ROM_START( gba )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gba.bin", 0x000000, 0x004000, CRC(81977335) SHA1(300c20df6731a33952ded8c436f7f186d25d3492) )
ROM_END

ROM_START( robotech )
	ROM_REGION( 0x1000100, "maincpu", 0 )
	ROM_LOAD( "coleco_robotech_mx29gl128elt21_00c22273.bin", 0x0000000, 0x1000100, CRC(04beee9c) SHA1(acf07d51c525b055679186cc07c6ac2cd8f45eac) )
ROM_END

//   YEAR  NAME       PARENT  COMPAT  MACHINE           INPUT       CLASS               INIT        COMPANY     FULLNAME            FLAGS
CONS(2001, gba,       0,      0,      gbadv_cons,       gbadv_cons, gba_cons_state,     empty_init, "Nintendo", "Game Boy Advance", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)

// this is a single game reissue of "Robotech - The Macross Saga (Euro, USA)" on the GBA but with double
// sized ROM (BIOS replacement in first half?) and other mods.  It is unclear how compatible this is with
// standard hardware.
CONS(2018, robotech,  0,      0,      gbadv_robotech,   gbadv,      gba_robotech_state, empty_init, "Coleco",   "Robotech",         MACHINE_NOT_WORKING)
