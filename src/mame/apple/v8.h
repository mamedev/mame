// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_V8_H
#define MAME_APPLE_V8_H

#pragma once

#include "machine/6522via.h"
#include "machine/applefdintf.h"
#include "machine/swim2.h"
#include "sound/asc.h"
#include "emupal.h"
#include "screen.h"

// ======================> v8_device

class v8_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	v8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// interface routines
	auto pb4_callback() { return write_pb4.bind(); }
	auto pb5_callback() { return write_pb5.bind(); }
	auto cb2_callback() { return write_cb2.bind(); }
	auto hdsel_callback() { return write_hdsel.bind(); }
	auto hmmu_enable_callback() { return write_hmmu_enable.bind(); }
	auto pb3_callback() { return read_pb3.bind(); }

	virtual void map(address_map &map);

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_rom_tag(T &&... args) { m_rom.set_tag(std::forward<T>(args)...); }
	void set_ram_info(u32 *ram, u32 size);
	void set_baseram_is_4M(bool ramSize) { m_baseIs4M = ramSize; }

	void cb1_w(int state);
	void cb2_w(int state);
	template <u8 mask> void slot_irq_w(int state);
	void scc_irq_w(int state);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<asc_device> m_asc;

	std::unique_ptr<u32 []> m_vram;

	u8 m_pseudovia_regs[256];
	u32 *m_ram_ptr;
	u32 m_ram_size;
	bool m_overlay;

	v8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	virtual u8 pseudovia_r(offs_t offset);

	void asc_irq(int state);

	virtual void ram_size(u8 config);

private:
	devcb_write_line write_pb4, write_pb5, write_cb2, write_hdsel, write_hmmu_enable;
	devcb_read_line read_pb3;

	optional_ioport m_montype;
	required_device<via6522_device> m_via1;
	required_region_ptr<u32> m_rom;

	sound_stream *m_stream;
	emu_timer *m_6015_timer;
	int m_via_interrupt, m_via2_interrupt, m_scc_interrupt, m_last_taken_interrupt;
	u8 m_pseudovia_ier, m_pseudovia_ifr;
	u8 m_pal_address, m_pal_idx, m_pal_control, m_pal_colkey;

	bool m_baseIs4M;

	u32 rom_switch_r(offs_t offset);

	void pseudovia_w(offs_t offset, u8 data);
	void pseudovia_recalc_irqs();

	u16 mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, u16 data, u16 mem_mask);

	virtual u8 via_in_a();
	u8 via_in_b();
	virtual void via_out_a(u8 data);
	void via_out_b(u8 data);
	void via_sync();
	void field_interrupts();
	void via_out_cb2(int state);
	void via1_irq(int state);
	void via2_irq(int state);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);

	u32 vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask);
	u8 dac_r(offs_t offset);
	void dac_w(offs_t offset, u8 data);

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

// ======================> eagle_device

class eagle_device : public v8_device
{
public:
	eagle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	u8 via_in_a() override;
	u8 pseudovia_r(offs_t offset) override;
	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
};

// ======================> spice_device

class spice_device : public v8_device
{
public:
	spice_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void map(address_map &map) override;

	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

protected:
	spice_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	void phases_w(u8 phases);
	void devsel_w(u8 devsel);

private:
	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel;

	virtual u8 via_in_a() override;
	virtual void via_out_a(u8 data) override;
	virtual u8 pseudovia_r(offs_t offset) override;
	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	u16 swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);

	void bright_contrast_w(offs_t offset, u8 data);
};

// ======================> tinkerbell_device

class tinkerbell_device : public spice_device
{
public:
	tinkerbell_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void ram_size(u8 config) override;

private:
	virtual u8 via_in_a() override;
	virtual u8 pseudovia_r(offs_t offset) override;
	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
};

// device type definition
DECLARE_DEVICE_TYPE(V8, v8_device)
DECLARE_DEVICE_TYPE(EAGLE, eagle_device)
DECLARE_DEVICE_TYPE(SPICE, spice_device)
DECLARE_DEVICE_TYPE(TINKERBELL, tinkerbell_device)

#endif // MAME_APPLE_V8_H
