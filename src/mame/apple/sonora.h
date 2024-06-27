// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_SONORA_H
#define MAME_APPLE_SONORA_H

#pragma once

#include "machine/6522via.h"
#include "machine/applefdintf.h"
#include "machine/mv_sonora.h"
#include "machine/swim2.h"
#include "sound/asc.h"
#include "speaker.h"

// ======================> sonora_device

class sonora_device :  public device_t, public device_sound_interface
{
public:
	// construction/destruction
	sonora_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: sonora_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	sonora_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	auto pb4_callback() { return write_pb4.bind(); }
	auto pb5_callback() { return write_pb5.bind(); }
	auto cb2_callback() { return write_cb2.bind(); }
	auto pb3_callback() { return read_pb3.bind(); }

	void map(address_map &map);

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_rom_tag(T &&... args) { m_rom.set_tag(std::forward<T>(args)...); }
	void set_ram_info(u32 *ram, u32 size);

	void cb1_w(int state);
	void cb2_w(int state);
	template <u8 mask> void slot_irq_w(int state);
	void scc_irq_w(int state);

	void pixel_clock_w(u32 pclk) { m_video->set_pixel_clock(pclk); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	devcb_write_line write_pb4, write_pb5, write_cb2;
	devcb_read_line read_pb3;

	required_device<cpu_device> m_maincpu;
	required_device<mac_video_sonora_device> m_video;
	required_device<via6522_device> m_via1;
	required_device<asc_device> m_asc;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_region_ptr<u32> m_rom;

	std::unique_ptr<u32[]> m_vram;
	sound_stream *m_stream;
	emu_timer *m_6015_timer;
	int m_via_interrupt, m_via2_interrupt, m_scc_interrupt, m_last_taken_interrupt;
	uint8_t m_pseudovia_regs[256], m_pseudovia_ier, m_pseudovia_ifr;
	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel;
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
	void via_out_cb2(int state);
	void via1_irq(int state);
	void via2_irq(int state);
	void asc_irq(int state);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);

	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);
	uint16_t swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);

	u32 vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask);
};

// device type definition
DECLARE_DEVICE_TYPE(SONORA, sonora_device)

#endif // MAME_APPLE_SONORA_H
