// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX_H
#define MAME_MSX_MSX_H

#include "bus/centronics/ctronics.h"
#include "bus/msx_slot/cartridge.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/buffer.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/rp5c01.h"
#include "msx_switched.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "speaker.h"
#include "video/v9938.h"
#include "video/tms9928a.h"


class msx_hw_def
{
public:
	msx_hw_def() {}
	bool has_cassette() const { return m_has_cassette; }
	bool has_printer_port() const { return m_has_printer_port; }
	bool has_cartslot() const { return m_has_cartslot; }
	bool has_fdc() const { return m_has_fdc; }
	msx_hw_def &has_cassette(bool has_cassette) { m_has_cassette = has_cassette; return *this;}
	msx_hw_def &has_printer_port(bool has_printer_port) { m_has_printer_port = has_printer_port; return *this; }
	msx_hw_def &has_cartslot(bool has_cartslot) { m_has_cartslot = has_cartslot; return *this; }
	msx_hw_def &has_fdc(bool has_fdc) { m_has_fdc = has_fdc; return *this; }

private:
	bool m_has_cassette = true;
	bool m_has_printer_port = true;
	bool m_has_cartslot = false;
	bool m_has_fdc = false;
};

class msx_state : public driver_device
{
public:
	msx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_ay8910(*this, "ay8910")
		, m_dac(*this, "dac")
		, m_ppi(*this, "ppi8255")
		, m_tms9928a(*this, "tms9928a")
		, m_cent_status_in(*this, "cent_status_in")
		, m_cent_ctrl_out(*this, "cent_ctrl_out")
		, m_cent_data_out(*this, "cent_data_out")
		, m_centronics(*this, "centronics")
		, m_speaker(*this, "speaker")
		, m_mainirq(*this, "mainirq")
		, m_screen(*this, "screen")
		, m_region_kanji(*this, "kanji")
		, m_io_joy(*this, "JOY%u", 0U)
		, m_io_dsw(*this, "DSW")
		, m_io_mouse(*this, "MOUSE%u", 0U)
		, m_io_key(*this, "KEY%u", 0U)
		, m_leds(*this, "led%u", 1U)
		, m_view_page0(*this, "view0")
		, m_view_page1(*this, "view1")
		, m_view_page2(*this, "view2")
		, m_view_page3(*this, "view3")
		, m_view_slot0_page0(*this, "view0_0")
		, m_view_slot0_page1(*this, "view0_1")
		, m_view_slot0_page2(*this, "view0_2")
		, m_view_slot0_page3(*this, "view0_3")
		, m_view_slot1_page0(*this, "view1_0")
		, m_view_slot1_page1(*this, "view1_1")
		, m_view_slot1_page2(*this, "view1_2")
		, m_view_slot1_page3(*this, "view1_3")
		, m_view_slot2_page0(*this, "view2_0")
		, m_view_slot2_page1(*this, "view2_1")
		, m_view_slot2_page2(*this, "view2_2")
		, m_view_slot2_page3(*this, "view2_3")
		, m_view_slot3_page0(*this, "view3_0")
		, m_view_slot3_page1(*this, "view3_1")
		, m_view_slot3_page2(*this, "view3_2")
		, m_view_slot3_page3(*this, "view3_3")
		, m_psg_b(0)
		, m_kanji_latch(0)
		, m_slot_expanded{false, false, false, false}
		, m_primary_slot(0)
		, m_secondary_slot{0, 0, 0, 0}
		, m_port_c_old(0)
		, m_keylatch(0)
	{
		m_mouse[0] = m_mouse[1] = 0;
		m_mouse_stat[0] = m_mouse_stat[1] = 0;
		m_cartslot[0] = nullptr;
		m_cartslot[1] = nullptr;
		m_generic_internal = nullptr;
		m_view[0] = &m_view_page0;
		m_view[1] = &m_view_page1;
		m_view[2] = &m_view_page2;
		m_view[3] = &m_view_page3;
		m_exp_view[0][0] = &m_view_slot0_page0;
		m_exp_view[0][1] = &m_view_slot0_page1;
		m_exp_view[0][2] = &m_view_slot0_page2;
		m_exp_view[0][3] = &m_view_slot0_page3;
		m_exp_view[1][0] = &m_view_slot1_page0;
		m_exp_view[1][1] = &m_view_slot1_page1;
		m_exp_view[1][2] = &m_view_slot1_page2;
		m_exp_view[1][3] = &m_view_slot1_page3;
		m_exp_view[2][0] = &m_view_slot2_page0;
		m_exp_view[2][1] = &m_view_slot2_page1;
		m_exp_view[2][2] = &m_view_slot2_page2;
		m_exp_view[2][3] = &m_view_slot2_page3;
		m_exp_view[3][0] = &m_view_slot3_page0;
		m_exp_view[3][1] = &m_view_slot3_page1;
		m_exp_view[3][2] = &m_view_slot3_page2;
		m_exp_view[3][3] = &m_view_slot3_page3;
	}

	void ax150(machine_config &config);
	void ax170(machine_config &config);
	void ax230(machine_config &config);
	void canonv8(machine_config &config);
	void canonv10(machine_config &config);
	void canonv20(machine_config &config);
	void canonv20e(machine_config &config);
	void canonv25(machine_config &config);
	void cf1200(machine_config &config);
	void cf2000(machine_config &config);
	void cf2700(machine_config &config);
	void cf2700g(machine_config &config);
	void cf2700uk(machine_config &config);
	void cf3000(machine_config &config);
	void cf3300(machine_config &config);
	void cpc50a(machine_config &config);
	void cpc50b(machine_config &config);
	void cpc51(machine_config &config);
	void cpc88(machine_config &config);
	void cx5f(machine_config &config);
	void cx5f1(machine_config &config);
	void cx5mu(machine_config &config);
	void dgnmsx(machine_config &config);
	void dpc100(machine_config &config);
	void dpc180(machine_config &config);
	void dpc200(machine_config &config);
	void dpc200e(machine_config &config);
	void expert10(machine_config &config);
	void expert11(machine_config &config);
	void expert13(machine_config &config);
	void expertdp(machine_config &config);
	void expertpl(machine_config &config);
	void fmx(machine_config &config);
	void fdpc200(machine_config &config);
	void fpc500(machine_config &config);
	void fs1300(machine_config &config);
	void fs4000(machine_config &config);
	void fs4000a(machine_config &config);
	void fspc800(machine_config &config);
	void gfc1080(machine_config &config);
	void gfc1080a(machine_config &config);
	void gsfc80u(machine_config &config);
	void gsfc200(machine_config &config);
	void hb10(machine_config &config);
	void hb10p(machine_config &config);
	void hb20p(machine_config &config);
	void hb55(machine_config &config);
	void hb55d(machine_config &config);
	void hb55p(machine_config &config);
	void hb75(machine_config &config);
	void hb75d(machine_config &config);
	void hb75p(machine_config &config);
	void hb101(machine_config &config);
	void hb101p(machine_config &config);
	void hb201(machine_config &config);
	void hb201p(machine_config &config);
	void hb501p(machine_config &config);
	void hb701fd(machine_config &config);
	void hb8000(machine_config &config);
	void hc5(machine_config &config);
	void hc6(machine_config &config);
	void hc7(machine_config &config);
	void hotbi13b(machine_config &config);
	void hotbi13p(machine_config &config);
	void hx10(machine_config &config);
	void hx10d(machine_config &config);
	void hx10dp(machine_config &config);
	void hx10e(machine_config &config);
	void hx10f(machine_config &config);
	void hx10s(machine_config &config);
	void hx10sa(machine_config &config);
	void hx20(machine_config &config);
	void hx20e(machine_config &config);
	void hx20i(machine_config &config);
	void hx21(machine_config &config);
	void hx21f(machine_config &config);
	void hx22(machine_config &config);
	void hx22i(machine_config &config);
	void hx32(machine_config &config);
	void hx51i(machine_config &config);
	void jvchc7gb(machine_config &config);
	void mbh1(machine_config &config);
	void mbh1e(machine_config &config);
	void mbh2(machine_config &config);
	void mbh25(machine_config &config);
	void mbh50(machine_config &config);
	void ml8000(machine_config &config);
	void mlf48(machine_config &config);
	void mlf80(machine_config &config);
	void mlf110(machine_config &config);
	void mlf120(machine_config &config);
	void mlfx1(machine_config &config);
	void mpc10(machine_config &config);
	void mpc64(machine_config &config);
	void mpc100(machine_config &config);
	void mpc200(machine_config &config);
	void mpc200sp(machine_config &config);
	void mx10(machine_config &config);
	void mx15(machine_config &config);
	void mx64(machine_config &config);
	void mx101(machine_config &config);
	void nms801(machine_config &config);
	void perfect1(machine_config &config);
	void phc2(machine_config &config);
	void phc28(machine_config &config);
	void phc28l(machine_config &config);
	void phc28s(machine_config &config);
	void piopx7(machine_config &config);
	void piopx7uk(machine_config &config);
	void piopxv60(machine_config &config);
	void pv7(machine_config &config);
	void pv16(machine_config &config);
	void spc800(machine_config &config);
	void svi728(machine_config &config);
	void sx100(machine_config &config);
	void tadpc200(machine_config &config);
	void vg8000(machine_config &config);
	void vg8010(machine_config &config);
	void vg8010f(machine_config &config);
	void vg802000(machine_config &config);
	void vg802020(machine_config &config);
	void vg8020f(machine_config &config);
	void yc64(machine_config &config);
	void yis303(machine_config &config);
	void yis503(machine_config &config);
	void yis503f(machine_config &config);

protected:
	template<typename AY8910Type> void msx_base(AY8910Type &ay8910_type, machine_config &config, XTAL xtal, int cpu_divider);
	template<typename AY8910Type, typename T, typename Ret, typename... Params> void msx_base(AY8910Type &ay8910_type, machine_config &config, XTAL xtal, int cpu_divider, Ret (T::*func)(Params...));
	template<typename VDPType, typename AY8910Type> void msx1(VDPType &vdp_type, AY8910Type &ay8910_type, machine_config &config);
	template<typename VDPType, typename AY8910Type, typename T, typename Ret, typename... Params> void msx1(VDPType &vdp_type, AY8910Type &ay8910_type, machine_config &config, Ret (T::*func)(Params...));
	void msx1_add_softlists(machine_config &config);

	template <u8 Game_port>
	u8 game_port_r();

	// configuration helpers
	template <typename T, typename U>
	auto &add_base_slot(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages)
	{
		auto &device(std::forward<T>(type)(config, std::forward<U>(tag), 0U));
		device.set_memory_space(m_maincpu, AS_PROGRAM);
		device.set_io_space(m_maincpu, AS_IO);
		device.set_maincpu(m_maincpu);
		m_internal_slots.push_back(std::make_tuple(prim, expanded, sec, page, numpages, &device));
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages)
	{
		auto &device = add_base_slot(config, std::forward<T>(type), std::forward<U>(tag), prim, expanded, sec, page, numpages);
		device.set_start_address(page * 0x4000);
		device.set_size(numpages * 0x4000);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages)
	{
		return add_internal_slot(config, std::forward<T>(type), std::forward<U>(tag), prim, true, sec, page, numpages);
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages)
	{
		return add_internal_slot(config, std::forward<T>(type), std::forward<U>(tag), prim, false, 0, page, numpages);
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		auto &device = add_internal_slot(config, std::forward<T>(type), std::forward<U>(tag), prim, expanded, sec, page, numpages);
		device.set_rom_start(region, offset);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_slot(config, std::forward<T>(type), std::forward<U>(tag), prim, true, sec, page, numpages, region, offset);
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_slot(config, std::forward<T>(type), std::forward<U>(tag), prim, false, 0, page, numpages, region, offset);
	}
	template <typename T, typename U>
	auto &add_internal_disk(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		auto &device = add_internal_slot(config, std::forward<T>(type), std::forward<U>(tag), prim, true, sec, page, numpages, region, offset);
		m_hw_def.has_fdc(true);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_disk(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_disk(config, std::forward<T>(type), std::forward<U>(tag), prim, true, sec, page, numpages, region, offset);
	}
	template <typename T, typename U>
	auto &add_internal_disk(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_disk(config, std::forward<T>(type), std::forward<U>(tag), prim, false, 0, page, numpages, region, offset);
	}
	template <int N, typename T, typename U>
	auto &add_internal_slot_irq(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		auto &device = add_internal_slot(config, std::forward<T>(type), std::forward<U>(tag), prim, expanded, sec, page, numpages, region, offset);
		device.irq_handler().set(m_mainirq, FUNC(input_merger_device::in_w<N>));
		return device;
	}
	template <int N, typename T, typename U>
	auto &add_internal_slot_irq(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_slot_irq<N>(config, std::forward<T>(type), std::forward<U>(tag), prim, true, sec, page, numpages, region, offset);
	}
	template <int N, typename T, typename U>
	auto &add_internal_slot_irq(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_slot_irq<N>(config, std::forward<T>(type), std::forward<U>(tag), prim, false, 0, page, numpages, region, offset);
	}
	template <int N, typename T, typename U>
	auto &add_internal_slot_irq_mirrored(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		auto &device = add_internal_slot_irq<N>(config, std::forward<T>(type), std::forward<U>(tag), prim, sec, page, numpages, region, offset);
		device.set_size(0x4000);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_disk_mirrored(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		// Memory mapped FDC registers are also accessible through page 2
		auto &device = add_internal_disk(config, std::forward<T>(type), std::forward<U>(tag), prim, expanded, sec, page, numpages, region, offset);
		device.set_size(0x4000);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_disk_mirrored(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_disk_mirrored(config, std::forward<T>(type), std::forward<U>(tag), prim, true, sec, page, numpages, region, offset);
	}
	template <typename T, typename U>
	auto &add_internal_disk_mirrored(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_disk_mirrored(config, std::forward<T>(type), std::forward<U>(tag), prim, false, 0, page, numpages, region, offset);
	}
	template <int N, typename T, typename U, typename V>
	auto &add_cartridge_slot(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, V &&intf, const char *deft)
	{
		auto &device = add_base_slot(config, std::forward<T>(type), std::forward<U>(tag), prim, expanded, sec, 0, 4);
		device.option_reset();
		intf(device);
		device.set_default_option(deft);
		device.set_fixed(false);
		device.irq_handler().set(m_mainirq, FUNC(input_merger_device::in_w<N>));
		m_hw_def.has_cartslot(true);
		return device;
	}
	template <int N, typename T, typename U, typename V>
	auto &add_cartridge_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, V &&intf, const char *deft)
	{
		return add_cartridge_slot<N>(config, std::forward<T>(type), std::forward<U>(tag), prim, true, sec, intf, deft);
	}
	template <int N, typename T, typename U, typename V>
	auto &add_cartridge_slot(machine_config &config, T &&type, U &&tag, u8 prim, V &&intf, const char *deft)
	{
		return add_cartridge_slot<N>(config, std::forward<T>(type), std::forward<U>(tag), prim, false, 0, intf, deft);
	}
	template <int N>
	auto &add_cartridge_slot(machine_config &config, u8 prim, bool expanded, u8 sec)
	{
		static const char *tags[4] = {
			"cartslot1", "cartslot2", "cartslot3", "cartslot4"
		};
		if (N < 1 || N > 4)
			fatalerror("Invalid cartridge slot number %d.\n", N);
		return add_cartridge_slot<N>(config, MSX_SLOT_CARTRIDGE, tags[N-1], prim, expanded, sec, msx_cart, nullptr);
	}
	template <int N>
	auto &add_cartridge_slot(machine_config &config, u8 prim, u8 sec)
	{
		return add_cartridge_slot<N>(config, prim, true, sec);
	}
	template <int N>
	auto &add_cartridge_slot(machine_config &config, u8 prim)
	{
		return add_cartridge_slot<N>(config, prim, false, 0);
	}
	virtual void driver_start() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void expanded_slot_w(offs_t offset, u8 data);
	u8 expanded_slot_r(offs_t offset);
	u8 kanji_r(offs_t offset);
	void kanji_w(offs_t offset, u8 data);
	void ppi_port_a_w(u8 data);
	void ppi_port_c_w(u8 data);
	u8 ppi_port_b_r();
	u8 psg_port_a_r();
	u8 psg_port_b_r();
	void psg_port_a_w(u8 data);
	void psg_port_b_w(u8 data);

	void msx_io_map(address_map &map);
	void memory_map(address_map &map);
	void memory_expand_slot(int slot);
	memory_view::memory_view_entry *get_view(int page, int prim, int sec);

	required_device<z80_device> m_maincpu;
	optional_device<cassette_image_device> m_cassette;
	required_device<ay8910_device> m_ay8910;
	required_device<dac_bit_interface> m_dac;
	required_device<i8255_device> m_ppi;
	optional_device<tms9928a_device> m_tms9928a;
	optional_device<input_buffer_device> m_cent_status_in;
	optional_device<output_latch_device> m_cent_ctrl_out;
	optional_device<output_latch_device> m_cent_data_out;
	optional_device<centronics_device> m_centronics;
	required_device<speaker_device> m_speaker;
	required_device<input_merger_any_high_device> m_mainirq;
	required_device<screen_device> m_screen;
	optional_memory_region m_region_kanji;
	required_ioport_array<2> m_io_joy;
	required_ioport m_io_dsw;
	required_ioport_array<2> m_io_mouse;
	required_ioport_array<11> m_io_key;
	output_finder<2> m_leds;
	msx_hw_def m_hw_def;
	// This is here until more direct rom dumps from kanji font roms become available.
	bool m_kanji_fsa1fx = false;
	memory_view m_view_page0;
	memory_view m_view_page1;
	memory_view m_view_page2;
	memory_view m_view_page3;
	memory_view *m_view[4];
	// There must be a better way to do this
	memory_view m_view_slot0_page0;
	memory_view m_view_slot0_page1;
	memory_view m_view_slot0_page2;
	memory_view m_view_slot0_page3;
	memory_view m_view_slot1_page0;
	memory_view m_view_slot1_page1;
	memory_view m_view_slot1_page2;
	memory_view m_view_slot1_page3;
	memory_view m_view_slot2_page0;
	memory_view m_view_slot2_page1;
	memory_view m_view_slot2_page2;
	memory_view m_view_slot2_page3;
	memory_view m_view_slot3_page0;
	memory_view m_view_slot3_page1;
	memory_view m_view_slot3_page2;
	memory_view m_view_slot3_page3;
	memory_view *m_exp_view[4][4];
	msx_slot_cartridge_device *m_cartslot[2];
	msx_internal_slot_interface *m_generic_internal;
	std::vector<std::tuple<int, bool, int, int, int, msx_internal_slot_interface *>> m_internal_slots;

	INTERRUPT_GEN_MEMBER(msx_interrupt);

	// PSG
	u8 m_psg_b = 0;
	// mouse
	u16 m_mouse[2]{};
	s8 m_mouse_stat[2]{};
	// kanji
	u32 m_kanji_latch = 0;
	// memory
	bool m_slot_expanded[4]{};
	u8 m_primary_slot = 0;
	u8 m_secondary_slot[4]{};
	u8 m_port_c_old = 0;
	u8 m_keylatch = 0;
};


class msx2_state : public msx_state
{
public:
	msx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx_state(mconfig, type, tag)
		, m_v9938(*this, "v9938")
		, m_v9958(*this, "v9958")
		, m_rtc(*this, "rtc")
		, m_rtc_latch(0)
	{
	}

	void ax350(machine_config &config);
	void ax350ii(machine_config &config);
	void ax350iif(machine_config &config);
	void ax370(machine_config &config);
	void ax500(machine_config &config);
	void canonv25(machine_config &config);
	void canonv30(machine_config &config);
	void canonv30f(machine_config &config);
	void cpc300(machine_config &config);
	void cpc300e(machine_config &config);
	void cpc330k(machine_config &config);
	void cpc400(machine_config &config);
	void cpc400s(machine_config &config);
	void cpc61(machine_config &config);
	void cpg120(machine_config &config);
	void cx7128(machine_config &config);
	void cx7m128(machine_config &config);
	void expert20(machine_config &config);
	void expert3i(machine_config &config);
	void expert3t(machine_config &config);
	void expertac(machine_config &config);
	void expertdx(machine_config &config);
	void fpc900(machine_config &config);
	void kmc5000(machine_config &config);
	void mbh70(machine_config &config);
	void mlg1(machine_config &config);
	void mlg3(machine_config &config);
	void mlg10(machine_config &config);
	void mlg30(machine_config &config);
	void mlg30_2(machine_config &config);
	void mpc2300(machine_config &config);
	void mpc2500f(machine_config &config);
	void mpc25fd(machine_config &config);
	void mpc25fs(machine_config &config);
	void mpc27(machine_config &config);
	void fs4500(machine_config &config);
	void fs4600f(machine_config &config);
	void fs4700f(machine_config &config);
	void fs5000f2(machine_config &config);
	void fs5500f1(machine_config &config);
	void fs5500f2(machine_config &config);
	void fsa1(machine_config &config);
	void fsa1a(machine_config &config);
	void fsa1f(machine_config &config);
	void fsa1fm(machine_config &config);
	void fsa1fx(machine_config &config);
	void fsa1gt(machine_config &config);
	void fsa1st(machine_config &config);
	void fsa1mk2(machine_config &config);
	void fsa1wsx(machine_config &config);
	void fsa1wx(machine_config &config);
	void fsa1wxa(machine_config &config);
	void fstm1(machine_config &config);
	void hbf1(machine_config &config);
	void hbf1ii(machine_config &config);
	void hbf1xd(machine_config &config);
	void hbf1xdj(machine_config &config);
	void hbf1xv(machine_config &config);
	void hbf5(machine_config &config);
	void hbf500(machine_config &config);
	void hbf500_2(machine_config &config);
	void hbf500f(machine_config &config);
	void hbf500p(machine_config &config);
	void hbf700d(machine_config &config);
	void hbf700f(machine_config &config);
	void hbf700p(machine_config &config);
	void hbf700s(machine_config &config);
	void hbf900(machine_config &config);
	void hbf900a(machine_config &config);
	void hbf9p(machine_config &config);
	void hbf9pr(machine_config &config);
	void hbf9s(machine_config &config);
	void hbg900ap(machine_config &config);
	void hbg900p(machine_config &config);
	void hotbit20(machine_config &config);
	void hx23(machine_config &config);
	void hx23f(machine_config &config);
	void hx33(machine_config &config);
	void hx34(machine_config &config);
	void mbh3(machine_config &config);
	void nms8220(machine_config &config);
	void nms8245(machine_config &config);
	void nms8245f(machine_config &config);
	void nms8250(machine_config &config);
	void nms8255(machine_config &config);
	void nms8255f(machine_config &config);
	void nms8260(machine_config &config);
	void nms8280(machine_config &config);
	void nms8280f(machine_config &config);
	void nms8280g(machine_config &config);
	void phc23(machine_config &config);
	void phc23jb(machine_config &config);
	void phc35j(machine_config &config);
	void phc55fd2(machine_config &config);
	void phc70fd(machine_config &config);
	void phc70fd2(machine_config &config);
	void phc77(machine_config &config);
	void tpc310(machine_config &config);
	void tpp311(machine_config &config);
	void tps312(machine_config &config);
	void ucv102(machine_config &config);
	void vg8230(machine_config &config);
	void vg8235(machine_config &config);
	void vg8235f(machine_config &config);
	void vg8240(machine_config &config);
	void victhc80(machine_config &config);
	void victhc90(machine_config &config);
	void victhc95(machine_config &config);
	void victhc95a(machine_config &config);
	void y503iiir(machine_config &config);
	void y503iiire(machine_config &config);
	void yis604(machine_config &config);
	void y805128(machine_config &config);
	void y805128r2(machine_config &config);
	void y805128r2e(machine_config &config);
	void y805256(machine_config &config);

protected:
	virtual void machine_start() override;

	template<typename AY8910Type> void msx2_base(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2_pal(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2plus_base(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2plus(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2plus_pal(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void turbor(AY8910Type &ay8910_type, machine_config &config);

	void msx2_add_softlists(machine_config &config);
	void msx2plus_add_softlists(machine_config &config);
	void turbor_add_softlists(machine_config &config);
	void msx_ym2413(machine_config &config);
	void msx2_64kb_vram(machine_config &config);

	u8 rtc_reg_r();
	void rtc_reg_w(u8 data);
	void rtc_latch_w(u8 data);
	u8 switched_r(offs_t offset);
	void switched_w(offs_t offset, u8 data);
	DECLARE_WRITE_LINE_MEMBER(turbo_w);

	void msx2_io_map(address_map &map);
	void msx2plus_io_map(address_map &map);

	std::vector<msx_switched_interface *> m_switched;

	optional_device<v9938_device> m_v9938;
	optional_device<v9958_device> m_v9958;
	required_device<rp5c01_device> m_rtc;

	// rtc
	u8 m_rtc_latch = 0;
};

#endif // MAME_MSX_MSX_H
