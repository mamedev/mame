// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
#ifndef _GBA_H_
#define _GBA_H_

#include "sound/gb.h"
#include "machine/intelfsh.h"
#include "bus/gba/gba_slot.h"
#include "sound/dac.h"
#include "video/gba_lcd.h"

#define WORD(x)              (m_regs[(x - REG_BASE) / 4])              /* 32-bit Register */   
#define HWHI(x)              (m_regs[(x - REG_BASE) / 4] >> 16)        /* 16-bit Register, Upper Half-Word */
#define HWLO(x)              (m_regs[(x - REG_BASE) / 4] & 0x0000ffff) /* 16-bit Register, Lower Half-Word */

#define WORD_SET(x, y)       (m_regs[(x - REG_BASE) / 4] |= (y))
#define HWHI_SET(x, y)      ((m_regs[(x - REG_BASE) / 4] |= (y << 16)))
#define HWLO_SET(x, y)       (m_regs[(x - REG_BASE) / 4] |= (y))

#define WORD_RESET(x, y)     (m_regs[(x - REG_BASE) / 4] &= ~(y))
#define HWHI_RESET(x, y)    ((m_regs[(x - REG_BASE) / 4] &= ~(y << 16)))
#define HWLO_RESET(x, y)     (m_regs[(x - REG_BASE) / 4] &= ~(y))

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

/* driver state */
class gba_state : public driver_device
{
public:
	gba_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ladac(*this, "direct_a_left"),
		m_radac(*this, "direct_a_right"),
		m_lbdac(*this, "direct_b_left"),
		m_rbdac(*this, "direct_b_right"),
		m_gbsound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_region_maincpu(*this, "maincpu"),
		m_io_inputs(*this, "INPUTS"),
		m_bios_hack(*this, "SKIP_CHECK")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_ladac;
	required_device<dac_device> m_radac;
	required_device<dac_device> m_lbdac;
	required_device<dac_device> m_rbdac;
	required_device<gameboy_sound_device> m_gbsound;
	required_device<gba_cart_slot_device> m_cart;

	void request_irq(UINT32 int_type);

	enum dma_start_timing {
		immediately = 0,
		vblank,
		hblank,
		special
	};

	void request_dma(dma_start_timing start);
	void dma_exec(int ch);
	void audio_tick(int ref);

	UINT32 m_regs[(0x400 - 0x60) / 4];

	// DMA
	emu_timer *m_dma_timer[4];
	UINT32 m_dma_src[4];
	UINT32 m_dma_dst[4];
	UINT16 m_dma_cnt[4];

	// Timers
	UINT32 m_timer_regs[4];
	UINT16 m_timer_reload[4];
	int m_timer_recalc[4];

	emu_timer *m_tmr_timer[4], *m_irq_timer;

	double m_timer_hz[4];

	int m_fifo_a_ptr;
	int m_fifo_b_ptr;
	int m_fifo_a_in;
	int m_fifo_b_in;
	UINT8 m_fifo_a[20];
	UINT8 m_fifo_b[20];

	UINT32 m_bios_last_address;
	int m_bios_protected;

	DIRECT_UPDATE_MEMBER(gba_direct);
	DECLARE_READ32_MEMBER(gba_io_r);
	DECLARE_WRITE32_MEMBER(gba_io_w);
	DECLARE_READ32_MEMBER(gba_bios_r);
	DECLARE_READ32_MEMBER(gba_10000000_r);
	DECLARE_DRIVER_INIT(gbadv);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_CALLBACK_MEMBER(dma_complete);
	TIMER_CALLBACK_MEMBER(timer_expire);
	TIMER_CALLBACK_MEMBER(handle_irq);

protected:
	required_region_ptr<UINT32> m_region_maincpu;
	required_ioport m_io_inputs;
	required_ioport m_bios_hack;
};

#endif
