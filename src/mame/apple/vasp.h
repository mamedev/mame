// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_VASP_H
#define MAME_APPLE_VASP_H

#pragma once

#include "machine/6522via.h"
#include "sound/asc.h"
#include "emupal.h"
#include "speaker.h"
#include "screen.h"

// ======================> vasp_device

class vasp_device :  public device_t
{
public:
	// construction/destruction
	vasp_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: vasp_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	vasp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	auto pb4_callback() { return write_pb4.bind(); }
	auto pb5_callback() { return write_pb5.bind(); }
	auto cb2_callback() { return write_cb2.bind(); }
	auto hdsel_callback() { return write_hdsel.bind(); }
	auto pb3_callback() { return read_pb3.bind(); }

	void map(address_map &map);

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_rom_tag(T &&... args) { m_rom.set_tag(std::forward<T>(args)...); }
	void set_ram_info(u32 *ram, u32 size);

	DECLARE_WRITE_LINE_MEMBER(cb1_w);
	DECLARE_WRITE_LINE_MEMBER(cb2_w);
	DECLARE_WRITE_LINE_MEMBER(vbl_w);
	DECLARE_WRITE_LINE_MEMBER(scc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(slot0_irq_w);
	DECLARE_WRITE_LINE_MEMBER(slot1_irq_w);
	DECLARE_WRITE_LINE_MEMBER(slot2_irq_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	devcb_write_line write_pb4, write_pb5, write_cb2, write_hdsel;
	devcb_read_line read_pb3;

	required_device<cpu_device> m_maincpu;
	required_ioport m_montype;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<via6522_device> m_via1;
	required_device<asc_device> m_asc;
	required_region_ptr<u32> m_rom;

	std::unique_ptr<u32[]> m_vram;
	emu_timer *m_6015_timer;
	int m_via_interrupt, m_via2_interrupt, m_scc_interrupt, m_last_taken_interrupt;
	u8 m_pseudovia_regs[256], m_pseudovia_ier, m_pseudovia_ifr;
	u8 m_pal_address, m_pal_idx, m_pal_control, m_pal_colkey;
	bool m_overlay;
	u32 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_size, m_rom_size;

	u32 rom_switch_r(offs_t offset);

	uint8_t pseudovia_r(offs_t offset);
	void pseudovia_w(offs_t offset, uint8_t data);
	void pseudovia_recalc_irqs();

	uint16_t mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint8_t via_in_a();
	uint8_t via_in_b();
	void via_out_a(uint8_t data);
	void via_out_b(uint8_t data);
	void via_sync();
	void field_interrupts();
	DECLARE_WRITE_LINE_MEMBER(via_out_cb2);
	DECLARE_WRITE_LINE_MEMBER(via1_irq);
	DECLARE_WRITE_LINE_MEMBER(via2_irq);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);

	u32 vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask);
	u8 dac_r(offs_t offset);
	void dac_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

// device type definition
DECLARE_DEVICE_TYPE(VASP, vasp_device)

#endif // MAME_APPLE_VASP_H
