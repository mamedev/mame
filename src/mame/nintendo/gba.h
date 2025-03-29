// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz

#ifndef MAME_NINTENDO_GBA_H
#define MAME_NINTENDO_GBA_H

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
		m_ldac(*this, "ldac%c", 'a'),
		m_rdac(*this, "rdac%c", 'a'),
		m_gbsound(*this, "gbsound"),
		m_io_inputs(*this, "INPUTS")
	{ }

	void gbadv(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;

	void gba_map(address_map &map) ATTR_COLD;

private:
	required_device_array<dac_8bit_r2r_twos_complement_device, 2> m_ldac;
	required_device_array<dac_8bit_r2r_twos_complement_device, 2> m_rdac;
	required_device<gameboy_sound_device> m_gbsound;

	void request_irq(uint32_t int_type);

	void dma_exec(int ch);
	void audio_tick(int ref);

	// DMA
	emu_timer *m_dma_timer[4]{};
	uint32_t m_dma_src[4]{};
	uint32_t m_dma_dst[4]{};
	uint16_t m_dma_cnt[4]{};

	// Timers
	uint32_t m_timer_regs[4]{};
	uint16_t m_timer_reload[4]{};
	uint8_t m_timer_recalc[4]{};

	emu_timer *m_tmr_timer[4]{}, *m_irq_timer = nullptr;

	double m_timer_hz[4]{};

	struct fifo_t
	{
		int32_t ptr = 0;
		int32_t in = 0;
		int32_t size = 0;
		int32_t remains = 0;
		uint32_t sample = 0;
		uint32_t word[8]{};
	};

	fifo_t m_fifo[2];

	uint32_t gba_io_r(offs_t offset, uint32_t mem_mask = ~0);
	void gba_io_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gba_10000000_r(offs_t offset, uint32_t mem_mask = ~0);
	void int_hblank_callback(int state);
	void int_vblank_callback(int state);
	void int_vcount_callback(int state);
	void dma_hblank_callback(int state);
	void dma_vblank_callback(int state);
	TIMER_CALLBACK_MEMBER(dma_complete);
	TIMER_CALLBACK_MEMBER(timer_expire);
	TIMER_CALLBACK_MEMBER(handle_irq);

	required_ioport m_io_inputs;
};

class gba_cons_state : public gba_state
{
public:
	gba_cons_state(const machine_config &mconfig, device_type type, const char *tag)
		: gba_state(mconfig, type, tag),
		m_region_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_bios_hack(*this, "SKIP_CHECK")
	{ }

	void gbadv_cons(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void gba_cons_map(address_map &map) ATTR_COLD;

	uint32_t gba_bios_r(offs_t offset, uint32_t mem_mask = ~0);

	required_region_ptr<uint32_t> m_region_maincpu;
	required_device<gba_cart_slot_device> m_cart;
	required_ioport m_bios_hack;
};


class gba_robotech_state : public gba_state
{
public:
	gba_robotech_state(const machine_config &mconfig, device_type type, const char *tag)
		: gba_state(mconfig, type, tag)
	{ }

	void gbadv_robotech(machine_config &config);

protected:

	void gba_robotech_map(address_map &map) ATTR_COLD;
};


#endif // MAME_NINTENDO_GBA_H
