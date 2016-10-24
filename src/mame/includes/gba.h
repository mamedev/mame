// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
#ifndef _GBA_H_
#define _GBA_H_

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

	uint32_t m_bios_last_address;
	int m_bios_protected;

	offs_t gba_direct(direct_read_data &direct, offs_t address);
	uint32_t gba_io_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void gba_io_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t gba_bios_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t gba_10000000_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void init_gbadv();
	void int_hblank_callback(int state);
	void int_vblank_callback(int state);
	void int_vcount_callback(int state);
	void dma_hblank_callback(int state);
	void dma_vblank_callback(int state);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void dma_complete(void *ptr, int32_t param);
	void timer_expire(void *ptr, int32_t param);
	void handle_irq(void *ptr, int32_t param);

protected:
	required_region_ptr<uint32_t> m_region_maincpu;
	required_ioport m_io_inputs;
	required_ioport m_bios_hack;
};

#endif
