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
		m_ldaca(*this, "ldaca"),
		m_rdaca(*this, "rdaca"),
		m_ldacb(*this, "ldacb"),
		m_rdacb(*this, "rdacb"),
		m_gbsound(*this, "custom"),
		m_io_inputs(*this, "INPUTS")
	{ }

	void gbadv(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;

	void gba_map(address_map &map) ATTR_COLD;

private:
	required_device<dac_byte_interface> m_ldaca;
	required_device<dac_byte_interface> m_rdaca;
	required_device<dac_byte_interface> m_ldacb;
	required_device<dac_byte_interface> m_rdacb;
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
	int m_timer_recalc[4]{};

	emu_timer *m_tmr_timer[4]{}, *m_irq_timer = nullptr;

	double m_timer_hz[4]{};

	int m_fifo_a_ptr = 0;
	int m_fifo_b_ptr = 0;
	int m_fifo_a_in = 0;
	int m_fifo_b_in = 0;
	uint8_t m_fifo_a[20]{};
	uint8_t m_fifo_b[20]{};


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
