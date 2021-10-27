// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * includes/msx.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_MSX_H
#define MAME_INCLUDES_MSX_H

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/rp5c01.h"
#include "machine/buffer.h"
#include "machine/input_merger.h"
#include "bus/centronics/ctronics.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "video/v9938.h"
#include "video/tms9928a.h"
#include "imagedev/cassette.h"
#include "formats/fmsx_cas.h"
#include "formats/msx_dsk.h"
#include "hashfile.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"
#include "bus/msx_slot/ram.h"
#include "bus/msx_slot/cartridge.h"
#include "bus/msx_slot/ram_mm.h"
#include "bus/msx_slot/disk.h"
#include "bus/msx_slot/music.h"
#include "bus/msx_slot/bunsetsu.h"
#include "bus/msx_slot/fs4600.h"
#include "bus/msx_slot/panasonic08.h"
#include "bus/msx_slot/sony08.h"
#include "machine/msx_switched.h"


#define TC8521_TAG  "rtc"


class msx_state : public driver_device
{
public:
	msx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_ay8910(*this, "ay8910")
		, m_dac(*this, "dac")
		, m_region_maincpu(*this, "maincpu")
		, m_region_kanji(*this, "kanji")
		, m_io_joy(*this, "JOY%u", 0U)
		, m_io_dsw(*this, "DSW")
		, m_io_mouse(*this, "MOUSE%u", 0U)
		, m_io_key(*this, "KEY%u", 0U)
		, m_leds(*this, "led%u", 1U)
		, m_psg_b(0)
		, m_kanji_latch(0)
		, m_empty_slot(mconfig, *this)
		, m_primary_slot(0)
		, m_port_c_old(0)
		, m_keylatch(0)
	{
		for (int prim = 0; prim < 4; prim++ )
		{
			m_slot_expanded[prim] = false;
			m_secondary_slot[prim] = 0;
			for (int sec = 0; sec < 4; sec++ )
			{
				for (int page = 0; page < 4; page++ )
				{
					m_all_slots[prim][sec][page] = nullptr;
				}
			}
		}
		m_mouse[0] = m_mouse[1] = 0;
		m_mouse_stat[0] = m_mouse_stat[1] = 0;
		m_empty_slot.set_memory_space(m_maincpu, AS_PROGRAM);
		m_empty_slot.set_io_space(m_maincpu, AS_IO);
	}

	void hc6(machine_config &config);
	void hb75d(machine_config &config);
	void dpc100(machine_config &config);
	void hb55p(machine_config &config);
	void hotbi13p(machine_config &config);
	void fspc800(machine_config &config);
	void mpc200(machine_config &config);
	void hb201p(machine_config &config);
	void hx22i(machine_config &config);
	void svi738dk(machine_config &config);
	void gfc1080a(machine_config &config);
	void svi738pl(machine_config &config);
	void hx10dp(machine_config &config);
	void perfect1(machine_config &config);
	void mlf110(machine_config &config);
	void cf1200(machine_config &config);
	void hx20(machine_config &config);
	void cf3000(machine_config &config);
	void mpc100(machine_config &config);
	void vg8020f(machine_config &config);
	void hx10f(machine_config &config);
	void cf2000(machine_config &config);
	void expertpl(machine_config &config);
	void fs4000a(machine_config &config);
	void mpc10(machine_config &config);
	void pv16(machine_config &config);
	void jvchc7gb(machine_config &config);
	void phc28(machine_config &config);
	void hx10sa(machine_config &config);
	void cx5m128(machine_config &config);
	void expert10(machine_config &config);
	void mbh50(machine_config &config);
	void hc7(machine_config &config);
	void gfc1080(machine_config &config);
	void cpc51(machine_config &config);
	void ax150(machine_config &config);
	void phc28s(machine_config &config);
	void mlfx1(machine_config &config);
	void tadpc200(machine_config &config);
	void gsfc200(machine_config &config);
	void hx10d(machine_config &config);
	void expertdp(machine_config &config);
	void yis303(machine_config &config);
	void canonv25(machine_config &config);
	void svi738sp(machine_config &config);
	void fmx(machine_config &config);
	void phc2(machine_config &config);
	void pv7(machine_config &config);
	void hx10(machine_config &config);
	void mlf48(machine_config &config);
	void cpc50b(machine_config &config);
	void hb10p(machine_config &config);
	void hx20i(machine_config &config);
	void mx10(machine_config &config);
	void mx15(machine_config &config);
	void expert13(machine_config &config);
	void bruc100(machine_config &config);
	void hx21(machine_config &config);
	void cf3300(machine_config &config);
	void cx5f1(machine_config &config);
	void hx10e(machine_config &config);
	void dpc200(machine_config &config);
	void svi738(machine_config &config);
	void dpc200e(machine_config &config);
	void canonv10(machine_config &config);
	void yis503(machine_config &config);
	void mpc200sp(machine_config &config);
	void svi738sw(machine_config &config);
	void vg8010f(machine_config &config);
	void dpc180(machine_config &config);
	void mlf120(machine_config &config);
	void hb201(machine_config &config);
	void piopxv60(machine_config &config);
	void hb10(machine_config &config);
	void hb501p(machine_config &config);
	void cx5m(machine_config &config);
	void mx101(machine_config &config);
	void mx64(machine_config &config);
	void hb55d(machine_config &config);
	void nms801(machine_config &config);
	void svi728(machine_config &config);
	void hotbi13b(machine_config &config);
	void hotbit12(machine_config &config);
	void hotbit11(machine_config &config);
	void vg8010(machine_config &config);
	void cf2700(machine_config &config);
	void hx21i(machine_config &config);
	void mbh2(machine_config &config);
	void cx5f(machine_config &config);
	void mpc64(machine_config &config);
	void yc64(machine_config &config);
	void yis503m(machine_config &config);
	void gsfc80u(machine_config &config);
	void cf2700g(machine_config &config);
	void ax170(machine_config &config);
	void y503iir(machine_config &config);
	void svi738ar(machine_config &config);
	void yis503ii(machine_config &config);
	void yis503f(machine_config &config);
	void cx5m2(machine_config &config);
	void spc800(machine_config &config);
	void canonv20(machine_config &config);
	void hb20p(machine_config &config);
	void mbh25(machine_config &config);
	void fs4000(machine_config &config);
	void hx10s(machine_config &config);
	void piopx7uk(machine_config &config);
	void hc5(machine_config &config);
	void dgnmsx(machine_config &config);
	void fdpc200(machine_config &config);
	void hx22(machine_config &config);
	void fs1300(machine_config &config);
	void phc28l(machine_config &config);
	void hb101p(machine_config &config);
	void expert11(machine_config &config);
	void vg802020(machine_config &config);
	void tadpc20a(machine_config &config);
	void hb75p(machine_config &config);
	void piopx7(machine_config &config);
	void canonv8(machine_config &config);
	void cpc88(machine_config &config);
	void vg802000(machine_config &config);
	void mlf80(machine_config &config);
	void cpc50a(machine_config &config);
	void hb701fd(machine_config &config);
	void vg8000(machine_config &config);
	void hb55(machine_config &config);
	void y503iir2(machine_config &config);
	void fpc500(machine_config &config);

protected:
	void msx(machine_config &config);
	template<typename VDPType> void msx1(VDPType &vdp_type, machine_config &config);

	void msx1_cartlist(machine_config &config);
	void msx1_floplist(machine_config &config);
	void msx_fd1793(machine_config &config);
	void msx_wd2793_force_ready(machine_config &config);
	void msx_wd2793(machine_config &config);
	void msx_mb8877a(machine_config &config);
	void msx_tc8566af(machine_config &config);
	void msx_microsol(machine_config &config);
	void msx_1_35_ssdd_drive(machine_config &config);
	void msx_1_35_dd_drive(machine_config &config);
	void msx_2_35_dd_drive(machine_config &config);

	// static configuration helpers
	void install_slot_pages(uint8_t prim, uint8_t sec, uint8_t page, uint8_t numpages, msx_internal_slot_interface &device);
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, uint8_t prim, uint8_t sec, uint8_t page, uint8_t numpages)
	{
		auto &device(std::forward<T>(type)(config, std::forward<U>(tag), 0U));
		device.set_memory_space(m_maincpu, AS_PROGRAM);
		device.set_io_space(m_maincpu, AS_IO);
		device.set_start_address(page * 0x4000);
		device.set_size(numpages * 0x4000);
		install_slot_pages(prim, sec, page, numpages, device);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, uint8_t prim, uint8_t sec, uint8_t page, uint8_t numpages, const char *region, uint32_t offset)
	{
		auto &device(std::forward<T>(type)(config, std::forward<U>(tag), 0U));
		device.set_memory_space(m_maincpu, AS_PROGRAM);
		device.set_io_space(m_maincpu, AS_IO);
		device.set_start_address(page * 0x4000);
		device.set_size(numpages * 0x4000);
		device.set_rom_start(region, offset);
		install_slot_pages(prim, sec, page, numpages, device);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_slot_mirrored(machine_config &config, T &&type, U &&tag, uint8_t prim, uint8_t sec, uint8_t page, uint8_t numpages, const char *region, uint32_t offset)
	{
		// Memory mapped FDC registers are also accessible through page 2
		auto &device(type(config, std::forward<U>(tag), 0U));
		device.set_memory_space(m_maincpu, AS_PROGRAM);
		device.set_io_space(m_maincpu, AS_IO);
		device.set_start_address(page * 0x4000);
		device.set_size(0x4000);
		device.set_rom_start(region, offset);
		install_slot_pages(prim, sec, page, numpages, device);
		return device;
	}
	template <int N, typename T, typename U, typename V>
	auto &add_cartridge_slot(machine_config &config, T &&type, U &&tag, uint8_t prim, uint8_t sec, V &&intf, const char *deft)
	{
		auto &device(type(config, std::forward<U>(tag), 0U));
		device.set_memory_space(m_maincpu, AS_PROGRAM);
		device.set_io_space(m_maincpu, AS_IO);
		device.option_reset();
		intf(device);
		device.set_default_option(deft);
		device.set_fixed(false);
		device.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<N>));
		install_slot_pages(prim, sec, 0, 4, device);
		return device;
	}

	virtual void driver_start() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_post_load() override;

	void msx_sec_slot_w(uint8_t data);
	uint8_t msx_sec_slot_r();
	uint8_t msx_kanji_r(offs_t offset);
	void msx_kanji_w(offs_t offset, uint8_t data);
	void msx_ppi_port_a_w(uint8_t data);
	void msx_ppi_port_c_w(uint8_t data);
	uint8_t msx_ppi_port_b_r();
	uint8_t msx_mem_read(offs_t offset);
	void msx_mem_write(offs_t offset, uint8_t data);
	uint8_t msx_psg_port_a_r();
	uint8_t msx_psg_port_b_r();
	void msx_psg_port_a_w(uint8_t data);
	void msx_psg_port_b_w(uint8_t data);

private:
	void msx_memory_map_all();
	void msx_memory_map_page(uint8_t page);
	void msx_memory_reset();
	void msx_memory_init();

	static void floppy_formats(format_registration &fr);

	INTERRUPT_GEN_MEMBER(msx_interrupt);

protected:
	void msx_io_map(address_map &map);
	void msx_memory_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<ay8910_device> m_ay8910;
	required_device<dac_bit_interface> m_dac;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_kanji;
	required_ioport_array<2> m_io_joy;
	required_ioport m_io_dsw;
	required_ioport_array<2> m_io_mouse;
	required_ioport_array<6> m_io_key;
	output_finder<2> m_leds;

private:
	/* PSG */
	int m_psg_b;
	/* mouse */
	uint16_t m_mouse[2];
	int m_mouse_stat[2];
	/* kanji */
	int m_kanji_latch;
	/* memory */
	msx_internal_slot_interface m_empty_slot;
	msx_internal_slot_interface *m_all_slots[4][4][4];
	msx_internal_slot_interface *m_current_page[4];
	bool m_slot_expanded[4];
	uint8_t m_primary_slot;
	uint8_t m_secondary_slot[4];
	int m_port_c_old;
	int m_keylatch;
};


class msx2_state : public msx_state
{
public:
	msx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx_state(mconfig, type, tag)
		, m_v9938(*this, "v9938")
		, m_v9958(*this, "v9958")
		, m_rtc(*this, TC8521_TAG)
		, m_rtc_latch(0)
	{
	}

	void ax350(machine_config &config);
	void ax370(machine_config &config);
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
	void fpc900(machine_config &config);
	void expert20(machine_config &config);
	void mbh70(machine_config &config);
	void kmc5000(machine_config &config);
	void mlg1(machine_config &config);
	void mlg3(machine_config &config);
	void mlg10(machine_config &config);
	void mlg30(machine_config &config);
	void fs4500(machine_config &config);
	void fs4600(machine_config &config);
	void fs4700(machine_config &config);
	void fs5000(machine_config &config);
	void fs5500f1(machine_config &config);
	void fs5500f2(machine_config &config);
	void fsa1(machine_config &config);
	void fsa1a(machine_config &config);
	void fsa1f(machine_config &config);
	void fsa1fm(machine_config &config);
	void fsa1mk2(machine_config &config);
	void nms8220(machine_config &config);
	void nms8220a(machine_config &config);
	void nms8245(machine_config &config);
	void nms8245f(machine_config &config);
	void nms8250(machine_config &config);
	void nms8250f(machine_config &config);
	void nms8250j(machine_config &config);
	void nms8255(machine_config &config);
	void nms8255f(machine_config &config);
	void nms8260(machine_config &config);
	void nms8280(machine_config &config);
	void nms8280f(machine_config &config);
	void nms8280g(machine_config &config);
	void vg8230(machine_config &config);
	void vg8230j(machine_config &config);
	void vg8235(machine_config &config);
	void vg8235f(machine_config &config);
	void vg8240(machine_config &config);
	void mpc2300(machine_config &config);
	void mpc2500f(machine_config &config);
	void mpc25fd(machine_config &config);
	void mpc27(machine_config &config);
	void phc23(machine_config &config);
	void phc55fd2(machine_config &config);
	void phc77(machine_config &config);
	void hotbit20(machine_config &config);
	void hbf1(machine_config &config);
	void hbf12(machine_config &config);
	void hbf1xd(machine_config &config);
	void hbf1xdm2(machine_config &config);
	void hbf5(machine_config &config);
	void hbf500(machine_config &config);
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
	void tpc310(machine_config &config);
	void tpp311(machine_config &config);
	void tps312(machine_config &config);
	void hx23(machine_config &config);
	void hx23f(machine_config &config);
	void hx23i(machine_config &config);
	void hx33(machine_config &config);
	void hx34(machine_config &config);
	void hx34i(machine_config &config);
	void fstm1(machine_config &config);
	void victhc90(machine_config &config);
	void victhc95(machine_config &config);
	void victhc95a(machine_config &config);
	void cx7m(machine_config &config);
	void cx7m128(machine_config &config);
	void y503iiir(machine_config &config);
	void y503iiire(machine_config &config);
	void yis60464(machine_config &config);
	void yis604(machine_config &config);
	void y805128(machine_config &config);
	void y805128r2(machine_config &config);
	void y805128r2e(machine_config &config);
	void y805256(machine_config &config);
	void expert3i(machine_config &config);
	void expert3t(machine_config &config);
	void expertac(machine_config &config);
	void expertdx(machine_config &config);
	void fsa1fx(machine_config &config);
	void fsa1wsx(machine_config &config);
	void fsa1wx(machine_config &config);
	void fsa1wxa(machine_config &config);
	void phc35j(machine_config &config);
	void phc70fd(machine_config &config);
	void phc70fd2(machine_config &config);
	void hbf1xdj(machine_config &config);
	void hbf1xv(machine_config &config);
	void hbf9sp(machine_config &config);
	void fsa1gt(machine_config &config);
	void fsa1st(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void msx2(machine_config &config);
	void msx2p(machine_config &config);
	void msx2_pal(machine_config &config);
	void turbor(machine_config &config);

	void msx2_cartlist(machine_config &config);
	void msx2_floplist(machine_config &config);
	void msx2p_floplist(machine_config &config);
	void msxr_floplist(machine_config &config);
	void msx_ym2413(machine_config &config);
	void msx2_64kb_vram(machine_config &config);

	uint8_t msx_rtc_reg_r();
	void msx_rtc_reg_w(uint8_t data);
	void msx_rtc_latch_w(uint8_t data);
	uint8_t msx_switched_r(offs_t offset);
	void msx_switched_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(turbo_w);

	void msx2_io_map(address_map &map);
	void msx2p_io_map(address_map &map);

	std::vector<msx_switched_interface *> m_switched;

	optional_device<v9938_device> m_v9938;
	optional_device<v9958_device> m_v9958;
	required_device<rp5c01_device> m_rtc;

	/* rtc */
	int m_rtc_latch;
};


#endif // MAME_INCLUDES_MSX_H
