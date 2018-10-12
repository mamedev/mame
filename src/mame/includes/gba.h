// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz

#ifndef MAME_INCLUDES_GBA_H
#define MAME_INCLUDES_GBA_H

#include "sound/gb.h"
#include "machine/intelfsh.h"
#include "bus/gba/gba_slot.h"
#include "sound/dac.h"
#include "video/gba_lcd.h"


class gba_state : public driver_device, protected gba_registers<(0x400 - 0x060) / 4, 0x060>
{
public:
	gba_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ldaca(*this, "ldaca"),
		m_rdaca(*this, "rdaca"),
		m_ldacb(*this, "ldacb"),
		m_rdacb(*this, "rdacb"),
		m_gbsound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_region_maincpu(*this, "maincpu"),
		m_io_inputs(*this, "INPUTS"),
		m_bios_hack(*this, "SKIP_CHECK")
	{ }

	void gbadv(machine_config &config);

	void init_gbadv();

private:
	required_device<cpu_device> m_maincpu;
	required_device<dac_byte_interface> m_ldaca;
	required_device<dac_byte_interface> m_rdaca;
	required_device<dac_byte_interface> m_ldacb;
	required_device<dac_byte_interface> m_rdacb;
	required_device<gameboy_sound_device> m_gbsound;
	required_device<gba_cart_slot_device> m_cart;

	void request_irq(uint32_t int_type);

	void dma_exec(int ch);
	void audio_tick(int ref);

	// DMA
	emu_timer *m_dma_timer[4];
	uint32_t m_dma_src[4];
	uint32_t m_dma_dst[4];
	uint16_t m_dma_cnt[4];

	// Timers
	uint32_t m_timer_regs[4];
	uint16_t m_timer_reload[4];
	int m_timer_recalc[4];

	emu_timer *m_tmr_timer[4], *m_irq_timer;

	double m_timer_hz[4];

	int m_fifo_a_ptr;
	int m_fifo_b_ptr;
	int m_fifo_a_in;
	int m_fifo_b_in;
	uint8_t m_fifo_a[20];
	uint8_t m_fifo_b[20];

	DECLARE_READ32_MEMBER(gba_io_r);
	DECLARE_WRITE32_MEMBER(gba_io_w);
	DECLARE_READ32_MEMBER(gba_bios_r);
	DECLARE_READ32_MEMBER(gba_10000000_r);
	DECLARE_WRITE_LINE_MEMBER(int_hblank_callback);
	DECLARE_WRITE_LINE_MEMBER(int_vblank_callback);
	DECLARE_WRITE_LINE_MEMBER(int_vcount_callback);
	DECLARE_WRITE_LINE_MEMBER(dma_hblank_callback);
	DECLARE_WRITE_LINE_MEMBER(dma_vblank_callback);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_CALLBACK_MEMBER(dma_complete);
	TIMER_CALLBACK_MEMBER(timer_expire);
	TIMER_CALLBACK_MEMBER(handle_irq);

	void gba_map(address_map &map);

	required_region_ptr<uint32_t> m_region_maincpu;
	required_ioport m_io_inputs;
	required_ioport m_bios_hack;
};

#endif
