// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, R. Belmont
/***************************************************************************

  nds.cpp

  Preliminary driver for first-generation Nintendo DS.

  Tech info: http://problemkaputt.de/gbatek.htm

  Notes:
    Timers and DMAs 0-3 are ARM9's, 4-7 are ARM7's.
    Interrupt registers [0] is ARM9, [1] is ARM7.

***************************************************************************/

#include "emu.h"
#include "nds.h"

#include <cstdarg>

#define VERBOSE_LEVEL   (0)

// Measured value from GBATEK.  Actual crystal unknown.
#define MASTER_CLOCK (33513982)

#define INT_VBL                 0x00000001
#define INT_HBL                 0x00000002
#define INT_VCNT                0x00000004
#define INT_TM0_OVERFLOW        0x00000008
#define INT_TM1_OVERFLOW        0x00000010
#define INT_TM2_OVERFLOW        0x00000020
#define INT_TM3_OVERFLOW        0x00000040
#define INT_SIO                 0x00000080  // also RCNT/RTC (arm7 only)
#define INT_DMA0                0x00000100
#define INT_DMA1                0x00000200
#define INT_DMA2                0x00000400
#define INT_DMA3                0x00000800
#define INT_KEYPAD              0x00001000
#define INT_GAMEPAK             0x00002000  // GBA slot IRQ line (never used?)
#define INT_NA1                 0x00004000  // unused
#define INT_NA2                 0x00008000  // unused
#define INT_IPCSYNC             0x00010000
#define INT_IPCSENDEMPTY        0x00020000
#define INT_IPCRECVNOTEMPTY     0x00040000
#define INT_CARDXFERCOMPLETE    0x00080000
#define INT_CARDIREQ            0x00100000
#define INT_GEOCMDFIFO          0x00200000  // arm9 only
#define INT_SCREENUNFOLD        0x00400000  // arm7 only
#define INT_SPIBUS              0x00800000  // arm7 only
#define INT_WIFI                0x01000000  // arm7 only - also DSP on DSi
#define INT_CAMERA              0x02000000  // DSi only
#define INT_NA3                 0x04000000
#define INT_NA4                 0x08000000
#define INT_NEWDMA0             0x10000000  // DSi only
#define INT_NEWDMA1             0x20000000  // DSi only
#define INT_NEWDMA2             0x40000000  // DSi only
#define INT_NEWDMA3             0x80000000  // DSi only

static const uint32_t timer_clks[4] = { MASTER_CLOCK, MASTER_CLOCK / 64, MASTER_CLOCK / 256, MASTER_CLOCK / 1024 };

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

uint32_t nds_state::arm7_io_r(offs_t offset, uint32_t mem_mask)
{
	uint8_t temp1, temp2;
	switch(offset)
	{
		case TIMER_OFFSET:
		case TIMER_OFFSET+1:
		case TIMER_OFFSET+2:
		case TIMER_OFFSET+3:
			{
				uint32_t elapsed;
				double time, ticks;
				int timer = (offset - TIMER_OFFSET) + 4;

				printf("Read timer reg %x (PC=%x)\n", timer, m_arm7->pc());

				// update times for
				if (m_timer_regs[timer] & 0x800000)
				{
					if (m_timer_regs[timer] & 0x00040000)
					{
						elapsed = m_timer_regs[timer] & 0xffff;
					}
					else
					{
						time = 0.1; //m_tmr_timer[timer]->elapsed().as_double();

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

				return (m_timer_regs[timer] & 0xffff0000) | (elapsed & 0xffff);
			}
			break;

		case IME_OFFSET:
			return m_ime[1];

		case IE_OFFSET:
			return m_ie[1];

		case IF_OFFSET:
			return m_if[1];

		case IPCSYNC_OFFSET:
			return m_arm7_ipcsync;

		case AUX_SPI_CNT_OFFSET:
			printf("arm7: read AUX_SPI_CNT mask %08x\n", mem_mask);
			return 0;
			break;

		case GAMECARD_BUS_CTRL_OFFSET:
			//printf("arm7: read GAMECARD_BUS_CTRL (%08x) mask %08x\n", m_gamecard_ctrl, mem_mask);
			return m_gamecard_ctrl;
			break;

		case GAMECARD_DATA_OFFSET:
			printf("arm7: read to GAMECARD_DATA mask %08x\n", mem_mask);
			return 0xffffffff;
			break;

		case GAMECARD_DATA_2_OFFSET:
			printf("arm7: read to GAMECARD_DATA2 mask %08x\n", mem_mask);
			return 0xffffffff;
			break;

		case GAMECARD_DATA_IN_OFFSET:
			//printf("arm7: read to GAMECARD_DATA_IN mask %08x (len = %x)\n", mem_mask, m_cartdata_len);
			if (m_cartdata_len >= 4)
			{
				m_cartdata_len -= 4;
			}
			else
			{
				m_cartdata_len = 0;
			}

			if (m_cartdata_len == 0)
			{
				printf("NDS: xfer over\n");
				m_gamecard_ctrl &= ~GAMECARD_DATA_READY;
				m_gamecard_ctrl &= ~GAMECARD_BLOCK_BUSY;
			}
			return 0xffffffff;
			break;

		case SPI_CTRL_OFFSET:
			//printf("arm7: read SPI_CTRL mask %08x\n", mem_mask);
			return 0;
			break;

		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*/
			return m_arm7_postflg;

		case WRAMSTAT_OFFSET:
			temp1 = (((m_vramcntc & 3) == 2) && (m_vramcntc & 0x80)) ? 1 : 0;
			temp2 = (((m_vramcntd & 3) == 2) && (m_vramcntd & 0x80)) ? 2 : 0;
			return (m_wramcnt << 8) | temp1 | temp2;

		default:
			verboselog(*this, 0, "[ARM7] [IO] Unknown read: %08x (%08x)\n", offset*4, mem_mask);
			break;
	}

	return 0;
}

void nds_state::arm7_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch(offset)
	{
		case TIMER_OFFSET:
		case TIMER_OFFSET+1:
		case TIMER_OFFSET+2:
		case TIMER_OFFSET+3:
			{
				double rate, clocksel;
				uint32_t old_timer_regs;

				int timer = (offset - TIMER_OFFSET)+4;

				old_timer_regs = m_timer_regs[timer];

				m_timer_regs[timer] = (m_timer_regs[timer] & ~(mem_mask & 0xFFFF0000)) | (data & (mem_mask & 0xFFFF0000));

				printf("%08x to timer %d (mask %08x PC %x)\n", data, timer, ~mem_mask, m_arm7->pc());

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

					clocksel = timer_clks[(m_timer_regs[timer] >> 16) & 3];

					final = clocksel / rate;

					m_timer_hz[timer] = final;

					m_timer_recalc[timer] = 0;

					printf("Enabling timer %d @ %f Hz regs %08x\n", timer, final, m_timer_regs[timer]);

					// enable the timer
					if( !(data & 0x40000) ) // if we're not in Count-Up mode
					{
						attotime time = attotime::from_hz(final);
						m_tmr_timer[timer]->adjust(time, timer, time);
					}
				}
			}
			break;

		case IME_OFFSET:
			printf("ARM7: %08x to IME\n", data);
			COMBINE_DATA(&m_ime[1]);
			break;

		case IE_OFFSET:
			printf("ARM7: %08x to IE\n", data);
			COMBINE_DATA(&m_ie[1]);
			break;

		case IF_OFFSET:
			COMBINE_DATA(&m_if[1]);
			break;

		case IPCSYNC_OFFSET:
			//printf("ARM7: %x to IPCSYNC\n", data);
			m_arm9_ipcsync &= ~0xf;
			m_arm9_ipcsync |= ((data >> 8) & 0xf);
			m_arm7_ipcsync &= 0xf;
			m_arm7_ipcsync |= (data & ~0xf);
			break;

		case AUX_SPI_CNT_OFFSET:
			//printf("arm7: %08x to AUX_SPI_CNT mask %08x\n", data, mem_mask);
			m_spicnt &= 0x0080;
			m_spicnt |= (data & 0xe043);

			break;

		case GAMECARD_BUS_CTRL_OFFSET:
			//printf("arm7: %08x to GAMECARD_BUS_CTRL mask %08x\n", data, mem_mask);
			m_gamecard_ctrl &= GAMECARD_DATA_READY;
			m_gamecard_ctrl |= (data & ~GAMECARD_DATA_READY);

			if (!(m_spicnt & (1<<15)))
			{
				return;
			}

			if (!(m_gamecard_ctrl & GAMECARD_BLOCK_BUSY))
			{
				return;
			}

			m_cartdata_len = (m_gamecard_ctrl >> 24) & 7;
			if (m_cartdata_len == 7)
			{
				m_cartdata_len = 4;
			}
			else if (m_cartdata_len != 0)
			{
				m_cartdata_len = 256 << m_cartdata_len;
			}
			printf("nds: cartdata for transfer = %x\n", m_cartdata_len);

			if (m_cartdata_len > 0)
			{
				m_gamecard_ctrl |= GAMECARD_DATA_READY;
			}
			else
			{
				printf("NDS: xfer over\n");
				m_gamecard_ctrl &= ~GAMECARD_DATA_READY;
				m_gamecard_ctrl &= ~GAMECARD_BLOCK_BUSY;
			}
			break;

		case GAMECARD_DATA_OFFSET:
			//printf("arm7: %08x to GAMECARD_DATA mask %08x\n", data, mem_mask);
			break;

		case GAMECARD_DATA_2_OFFSET:
			//printf("arm7: %08x to GAMECARD_DATA2 mask %08x\n", data, mem_mask);
			break;

		case SPI_CTRL_OFFSET:
			//printf("arm7: %08x to SPI_CTRL mask %08x\n", data, mem_mask);
			break;

		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*/
			if (!(m_arm7_postflg & POSTFLG_PBF_MASK) && m_arm7->pc() < 0x4000)
			{
				m_arm7_postflg &= ~POSTFLG_PBF_MASK;
				m_arm7_postflg |= data & POSTFLG_PBF_MASK;
			}

			if (ACCESSING_BITS_8_15)
			{
				if ((data>>8) & 0x80)
				{
					printf("arm7: HALT\n"); // halts the arm7 until an interrupt occurs
					m_arm7->suspend(SUSPEND_REASON_HALT, 1);
					m_arm7halted = true;
				}
			}
			break;
		default:
			verboselog(*this, 0, "[ARM7] [IO] Unknown write: %08x = %08x (%08x)\n", offset*4, data, mem_mask);
			break;
	}
}

uint32_t nds_state::arm9_io_r(offs_t offset, uint32_t mem_mask)
{
	switch(offset)
	{
		case TIMER_OFFSET:
		case TIMER_OFFSET+1:
		case TIMER_OFFSET+2:
		case TIMER_OFFSET+3:
			{
				uint32_t elapsed;
				double time, ticks;
				int timer = (offset - TIMER_OFFSET);

				//printf("Read timer reg %x (PC=%x)\n", timer, m_arm9->pc());

				// update times for
				if (m_timer_regs[timer] & 0x800000)
				{
					if (m_timer_regs[timer] & 0x00040000)
					{
						elapsed = m_timer_regs[timer] & 0xffff;
					}
					else
					{
						time = 0.1; //m_tmr_timer[timer]->elapsed().as_double();

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

				return (m_timer_regs[timer] & 0xffff0000) | (elapsed & 0xffff);
			}
			break;

		case IME_OFFSET:
			return m_ime[0];

		case IE_OFFSET:
			return m_ie[0];

		case IF_OFFSET:
			return m_if[0];

		case IPCSYNC_OFFSET:
			return m_arm9_ipcsync;

		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*  1     RAM
			*/
			return m_arm9_postflg;
		default:
			verboselog(*this, 0, "[ARM9] [IO] Unknown read: %08x (%08x)\n", offset*4, mem_mask);
			break;
	}

	return 0;
}

void nds_state::arm9_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch(offset)
	{
		case TIMER_OFFSET:
		case TIMER_OFFSET+1:
		case TIMER_OFFSET+2:
		case TIMER_OFFSET+3:
			{
				double rate, clocksel;
				uint32_t old_timer_regs;

				int timer = (offset - TIMER_OFFSET)+4;

				old_timer_regs = m_timer_regs[timer];

				m_timer_regs[timer] = (m_timer_regs[timer] & ~(mem_mask & 0xFFFF0000)) | (data & (mem_mask & 0xFFFF0000));

				printf("%x to timer %d (mask %x PC %x)\n", data, timer, ~mem_mask, m_arm9->pc());

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

					clocksel = timer_clks[(m_timer_regs[timer] >> 16) & 3];

					final = clocksel / rate;

					m_timer_hz[timer] = final;

					m_timer_recalc[timer] = 0;

					printf("Enabling timer %d @ %f Hz\n", timer, final);

					// enable the timer
					if( !(data & 0x40000) ) // if we're not in Count-Up mode
					{
						attotime time = attotime::from_hz(final);
						m_tmr_timer[timer]->adjust(time, timer, time);
					}
				}
			}
			break;

		case IME_OFFSET:
			printf("ARM9: %08x to IME\n", data);
			COMBINE_DATA(&m_ime[0]);
			break;

		case IE_OFFSET:
			printf("ARM9: %08x to IE\n", data);
			COMBINE_DATA(&m_ie[0]);
			break;

		case IF_OFFSET:
			COMBINE_DATA(&m_if[0]);
			break;

		case IPCSYNC_OFFSET:
			printf("ARM9: %x to IPCSYNC\n", data);
			m_arm7_ipcsync &= ~0xf;
			m_arm7_ipcsync |= ((data >> 8) & 0xf);
			m_arm9_ipcsync &= 0xf;
			m_arm9_ipcsync |= (data & ~0xf);
			break;

		case VRAMCNT_A_OFFSET:
			if (ACCESSING_BITS_0_7) // VRAMCNT_A
			{
				m_vramcnta = data & 0xff;
			}
			if (ACCESSING_BITS_8_15) // VRAMCNT_B
			{
				m_vramcntb = (data >> 8) & 0xff;
			}
			if (ACCESSING_BITS_16_23) // VRAMCNT_C
			{
				m_vramcntc = (data >> 16) & 0xff;
			}
			if (ACCESSING_BITS_24_31) // VRAMCNT_D
			{
				m_vramcntd = (data >> 24) & 0xff;
			}
			break;

		case WRAMCNT_OFFSET:
			if (ACCESSING_BITS_0_7) // VRAMCNT_E
			{
				m_vramcnte = data & 0xff;
			}
			if (ACCESSING_BITS_8_15) // VRAMCNT_F
			{
				m_vramcntf = (data >> 8) & 0xff;
			}
			if (ACCESSING_BITS_16_23) // VRAMCNT_G
			{
				m_vramcntg = (data >> 16) & 0xff;
			}
			if (ACCESSING_BITS_24_31) // WRAMCNT
			{
				m_wramcnt = (data>>24) & 0x3;
				m_arm7wrambnk->set_bank(m_wramcnt);
				m_arm9wrambnk->set_bank(m_wramcnt);
			}
			break;

		case VRAMCNT_H_OFFSET:
			if (ACCESSING_BITS_0_7) // VRAMCNT_H
			{
				m_vramcnth = data & 0xff;
			}
			if (ACCESSING_BITS_8_15) // VRAMCNT_I
			{
				m_vramcnti = (data >> 8) & 0xff;
			}
			break;

		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*  1     RAM
			*/
			if (!(m_arm9_postflg & POSTFLG_PBF_MASK))
			{
				m_arm9_postflg &= ~POSTFLG_PBF_MASK;
				m_arm9_postflg |= data & POSTFLG_PBF_MASK;
			}
			m_arm9_postflg &= ~POSTFLG_RAM_MASK;
			m_arm9_postflg |= data & POSTFLG_RAM_MASK;
			break;
		default:
			verboselog(*this, 0, "[ARM7] [IO] Unknown write: %08x = %08x (%08x)\n", offset*4, data, mem_mask);
			break;
	}
}

void nds_state::nds_arm7_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom().region("arm7", 0);
	map(0x02000000, 0x023fffff).ram().mirror(0x00400000).share("mainram");
	map(0x03000000, 0x03007fff).mirror(0x007f8000).m(m_arm7wrambnk, FUNC(address_map_bank_device::amap32));
	map(0x03800000, 0x0380ffff).ram().mirror(0x007f0000).share("arm7ram");
	map(0x04000000, 0x0410ffff).rw(FUNC(nds_state::arm7_io_r), FUNC(nds_state::arm7_io_w));
}

void nds_state::nds_arm9_map(address_map &map)
{
	map(0x02000000, 0x023fffff).ram().mirror(0x00400000).share("mainram");
	map(0x03000000, 0x03007fff).mirror(0x00ff8000).m("nds9wram", FUNC(address_map_bank_device::amap32));
	map(0x04000000, 0x0410ffff).rw(FUNC(nds_state::arm9_io_r), FUNC(nds_state::arm9_io_w));
	map(0xffff0000, 0xffff0fff).rom().mirror(0x1000).region("arm9", 0);
}

// ARM7 views of WRAM
void nds_state::nds7_wram_map(address_map &map)
{
	map(0x00000, 0x07fff).rw(FUNC(nds_state::wram_arm7mirror_r), FUNC(nds_state::wram_arm7mirror_w));
	map(0x08000, 0x0bfff).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
	map(0x0c000, 0x0ffff).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
	map(0x10000, 0x13fff).rw(FUNC(nds_state::wram_second_half_r), FUNC(nds_state::wram_second_half_w));
	map(0x14000, 0x17fff).rw(FUNC(nds_state::wram_second_half_r), FUNC(nds_state::wram_second_half_w));
	map(0x18000, 0x1ffff).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
}

// ARM9 views of WRAM
void nds_state::nds9_wram_map(address_map &map)
{
	map(0x00000, 0x07fff).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
	map(0x08000, 0x0bfff).rw(FUNC(nds_state::wram_second_half_r), FUNC(nds_state::wram_second_half_w));
	map(0x0c000, 0x0ffff).rw(FUNC(nds_state::wram_second_half_r), FUNC(nds_state::wram_second_half_w));
	map(0x10000, 0x13fff).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
	map(0x14000, 0x17fff).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
	map(0x18000, 0x1ffff).noprw().nopw();       // probably actually open bus?  GBATEK describes as "random"
}

uint32_t nds_state::wram_first_half_r(offs_t offset) { return m_WRAM[offset]; }
uint32_t nds_state::wram_second_half_r(offs_t offset) { return m_WRAM[offset+0x4000]; }
void nds_state::wram_first_half_w(offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_WRAM[offset]); }
void nds_state::wram_second_half_w(offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_WRAM[offset+0x4000]); }
uint32_t nds_state::wram_arm7mirror_r(offs_t offset) { return m_arm7ram[offset]; }
void nds_state::wram_arm7mirror_w(offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_arm7ram[offset]); }

static INPUT_PORTS_START( nds )
INPUT_PORTS_END

void nds_state::machine_reset()
{
	m_arm7_postflg = 0;
	m_arm9_postflg = 0;
	m_wramcnt = 0;
	m_arm7wrambnk->set_bank(0);
	m_arm9wrambnk->set_bank(0);
	m_arm7halted = false;
}

void nds_state::machine_start()
{
	int i;

	for (i = 0; i < 8; i++)
	{
		m_dma_timer[i] = timer_alloc(FUNC(nds_state::dma_complete), this);
		m_dma_timer[i]->adjust(attotime::never, i);

		m_tmr_timer[i] = timer_alloc(FUNC(nds_state::timer_expire), this);
		m_tmr_timer[i]->adjust(attotime::never, i);
	}

	m_irq_timer = timer_alloc(FUNC(nds_state::handle_irq), this);
	m_irq_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(nds_state::dma_complete)
{
	#if 0
	static const uint32_t ch_int[8] = { INT_DMA0, INT_DMA1, INT_DMA2, INT_DMA3, INT_DMA0, INT_DMA1, INT_DMA2, INT_DMA3 };

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
	#endif
}

void nds_state::dma_exec(int ch)
{
	#if 0
	address_space &space;
	uint32_t src = m_dma_src[ch];
	uint32_t dst = m_dma_dst[ch];
	uint16_t ctrl = DMACNT_H(ch);
	int srcadd = (ctrl >> 7) & 3;
	int dstadd = (ctrl >> 5) & 3;

	if (ch > 4)
	{
		space = m_arm7->space(AS_PROGRAM);
	}
	else
	{
		space = m_arm9->space(AS_PROGRAM);
	}

	int cnt = m_dma_cnt[ch];
	if (cnt == 0)
	{
		if (ch == 3)
			cnt = 0x10000;
		else
			cnt = 0x4000;
	}

//  if (dst >= 0x6000000 && dst <= 0x6017fff)
//  printf("DMA exec: ch %d from %08x to %08x, mode %04x, count %04x (%s)\n", (int)ch, src, dst, ctrl, cnt, ((ctrl>>10) & 1) ? "32" : "16");

	for (int i = 0; i < cnt; i++)
	{
		if ((ctrl>>10) & 1)
		{
			src &= 0xfffffffc;
			dst &= 0xfffffffc;

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
#endif
//  printf("settng DMA timer %d for %d cycs (tmr %x)\n", ch, cnt, (uint32_t)m_dma_timer[ch]);
//  m_dma_timer[ch]->adjust(ATTOTIME_IN_CYCLES(0, cnt), ch);
	dma_complete(ch);
}

TIMER_CALLBACK_MEMBER(nds_state::handle_irq)
{
	request_irq(0, m_if[0]);
	request_irq(1, m_if[1]);

	m_irq_timer->adjust(attotime::never);
}

void nds_state::request_irq(int cpu, uint32_t int_type)
{
	// set flag for later recovery
	m_if[cpu] |= int_type;

	printf("request IRQ %08x on CPU %d\n", int_type, cpu);

	// is this specific interrupt enabled?
	int_type &= m_ie[cpu];
	if (int_type != 0)
	{
		// master enable?
		if (m_ime[cpu] & 1)
		{
			if (cpu == 0)
			{
				m_arm9->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
				m_arm9->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, CLEAR_LINE);
			}
			else
			{
				if (m_arm7halted)
				{
					printf("ARM7 unhalting\n");
					m_arm7->resume(SUSPEND_REASON_HALT);
					m_arm7halted = false;
				}

				m_arm7->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
				m_arm7->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, CLEAR_LINE);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(nds_state::timer_expire)
{
	static const uint32_t tmr_ints[8] = { INT_TM0_OVERFLOW, INT_TM1_OVERFLOW, INT_TM2_OVERFLOW, INT_TM3_OVERFLOW };
	uintptr_t tmr = (uintptr_t) param;
	int cpu = (tmr > 4) ? 1 : 0;

	verboselog(*this, 1, "Timer %d expired\n", (int)tmr);

	// "The reload value is copied into the counter only upon following two situations: Automatically upon timer overflows,"
	// "or when the timer start bit becomes changed from 0 to 1."
	if (m_timer_recalc[tmr] != 0)
	{
		double rate, clocksel, final;
		attotime time;
		m_timer_recalc[tmr] = 0;
		m_timer_regs[tmr] = (m_timer_regs[tmr] & 0xFFFF0000) | (m_timer_reload[tmr] & 0x0000FFFF);
		rate = 0x10000 - (m_timer_regs[tmr] & 0xffff);
		clocksel = timer_clks[(m_timer_regs[tmr] >> 16) & 3];
		final = clocksel / rate;
		m_timer_hz[tmr] = final;
		time = attotime::from_hz(final);
		m_tmr_timer[tmr]->adjust(time, tmr, time);
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
				if( ( m_timer_regs[1] & 0x400000 ) && ( m_ime[cpu] != 0 ) )
				{
					request_irq(cpu, tmr_ints[1]);
				}
				if( ( m_timer_regs[2] & 0x40000 ) )
				{
					m_timer_regs[2] = (( ( m_timer_regs[2] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (m_timer_regs[2] & 0xffff0000);
					if( ( m_timer_regs[2] & 0x0000ffff ) == 0 )
					{
						m_timer_regs[2] |= m_timer_reload[2];
						if( ( m_timer_regs[2] & 0x400000 ) && ( m_ime[cpu] != 0 ) )
						{
							request_irq(cpu, tmr_ints[2]);
						}
						if( ( m_timer_regs[3] & 0x40000 ) )
						{
							m_timer_regs[3] = (( ( m_timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (m_timer_regs[3] & 0xffff0000);
							if( ( m_timer_regs[3] & 0x0000ffff ) == 0 )
							{
								m_timer_regs[3] |= m_timer_reload[3];
								if( ( m_timer_regs[3] & 0x400000 ) && ( m_ime[cpu] != 0 ) )
								{
									request_irq(cpu, tmr_ints[3]);
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
				if( ( m_timer_regs[2] & 0x400000 ) && ( m_ime[cpu] != 0 ) )
				{
					request_irq(cpu, tmr_ints[2]);
				}
				if( ( m_timer_regs[3] & 0x40000 ) )
				{
					m_timer_regs[3] = (( ( m_timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff) | (m_timer_regs[3] & 0xffff0000);
					if( ( m_timer_regs[3] & 0x0000ffff ) == 0 )
					{
						m_timer_regs[3] |= m_timer_reload[3];
						if( ( m_timer_regs[3] & 0x400000 ) && ( m_ime[cpu] != 0 ) )
						{
							request_irq(cpu, tmr_ints[3]);
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
				if( ( m_timer_regs[3] & 0x400000 ) && ( m_ime[cpu] != 0 ) )
				{
					request_irq(cpu, tmr_ints[3]);
				}
			}
		}
		break;
	}

	// are we supposed to IRQ?
	if ((m_timer_regs[tmr] & 0x400000) && (m_ime[cpu] != 0))
	{
		request_irq(cpu, tmr_ints[tmr & 3]);
	}
}

void nds_state::nds(machine_config &config)
{
	ARM7(config, m_arm7, MASTER_CLOCK);
	m_arm7->set_addrmap(AS_PROGRAM, &nds_state::nds_arm7_map);

	ARM946ES(config, m_arm9, MASTER_CLOCK*2);
	m_arm9->set_high_vectors();
	m_arm9->set_addrmap(AS_PROGRAM, &nds_state::nds_arm9_map);

	// WRAM
	ADDRESS_MAP_BANK(config, "nds7wram").set_map(&nds_state::nds7_wram_map).set_options(ENDIANNESS_LITTLE, 32, 32, 0x8000);
	ADDRESS_MAP_BANK(config, "nds9wram").set_map(&nds_state::nds9_wram_map).set_options(ENDIANNESS_LITTLE, 32, 32, 0x8000);
}

// Help identifying the region and revisions of the main set would be greatly appreciated!
ROM_START( nds )
	ROM_REGION( 0x1000, "arm9", 0 )
	ROM_LOAD( "biosnds9.rom", 0x0000, 0x1000, CRC(2ab23573) SHA1(bfaac75f101c135e32e2aaf541de6b1be4c8c62d) )

	ROM_REGION( 0x4000, "arm7", 0 )
	ROM_LOAD( "biosnds7.rom", 0x0000, 0x4000, CRC(1280f0d5) SHA1(24f67bdea115a2c847c8813a262502ee1607b7df) )

	ROM_REGION32_LE( 0x40000, "firmware", 0 )
	ROM_SYSTEM_BIOS( 0, "nds", "Nintendo DS" )
	ROMX_LOAD( "firmware.bin", 0x0000, 0x40000, CRC(945f9dc9) SHA1(cfe072921ee3fb93f688743f8beef89043c3e9ad), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "40820d", "Nintendo DS ver.40820D prototype" ) // from X4 prototype unit
	ROMX_LOAD( "fw0802d6.bin", 0x0000, 0x40000, CRC(18e137df) SHA1(d51be561a6538941f8f43d6db9cdb964a383080a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "040615", "Nintendo DS ver.040615 prototype" )
	ROMX_LOAD( "fw64b19d.bin", 0x0000, 0x40000, CRC(93487f12) SHA1(3af896a05736cc0385d0f858b431ff719164caf6), ROM_BIOS(2) )
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY     FULLNAME  FLAGS
CONS( 2004, nds,  0,      0,      nds,     nds,   nds_state, empty_init, "Nintendo", "DS",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
