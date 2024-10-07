// license:BSD-3-Clause
// copyright-holders:

/*
Multi-games (?) from VLC / VLT
PCB description is for 'vlcunk', 'beezerk' is said to run on same / very similar hw.
The hardware appears to be an evolution of the one in vlc.cpp.

Main PCB marked 'Enhanced Video Main Logic  VLC, Inc.  Made in U.S.A.'
and '778-102-001 A/W Rev. D  ©1995,98 Powerhouse Technologies, Inc.'
Main components:
1x MC68EC000FN20 main CPU with a 40.000M XTAL nearby
4x Main CPU ROM sockets (all 4 populated)
1x TMS34010FNL-50 video CPU with a 50.000M XTAL nearby
4x Video CPU ROM sockets (only 2 populated)
1x Altera EPM7032LC44 (stickered 101310), etched VIDPLD
1x Altera EPM7064LC84-15 (stickered 101320), etched EVLMPLD
1x ICS5342-3 GENDAC (16-Bit Integrated Clock-LUT-DAC)
3x MC68681 with a 3.6864 MHz XTAL nearby
1x SDT7201
1x Oki M6242B
1x Lithium battery

Plug-in PCB marked 'EVML SERIAL INTERFACE' and 'MODEM BOARD 775-404-011 A/W REV. A'
with barely readable components and a 5-dip bank
Etched 'XECOM MODEM' 'XE1414V' 'XE2401/XE9601'
Plugs near the MC68681s.

Small plug-in PCB with markings partially covered by a sticker.
Readable: 'VLC, Inc. ©1998 Powerhouse Technol... EVML Vid... Memory 771-30... A/W Rev...'
1x TMS55165DGH
2x MT4C1M16C3DJ
Plugs near the Video CPU ROMs

Small plug-in PCB marked 'NVRAM Board 751-201-201 REV. A' (numbers may be wrong, difficult to read)
3x RAMs (probably, components unreadable)
3x batteries on the back
Plugs near the 68EC000
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "machine/mc68681.h"
#include "machine/microtch.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "video/bt47x.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class vlc34010_state : public driver_device
{
public:
	vlc34010_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms(*this, "tms"),
		m_rtc(*this, "rtc"),
		m_duart(*this, "duart%u", 0U),
		m_boot_view(*this, "boot")
	{ }

	void base(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	void switch_w(u8 data);
	u8 rtc_r(offs_t offset);
	void rtc_w(offs_t offset, u8 data);
	template <int N> u8 duart_r(offs_t offset);
	template <int N> void duart_w(offs_t offset, u8 data);

	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);

	required_device<cpu_device> m_maincpu;
	required_device<tms34010_device> m_tms;
	required_device<msm6242_device> m_rtc;
	required_device_array<mc68681_device, 3> m_duart;
	memory_view m_boot_view;

	void main_map(address_map &map) ATTR_COLD;
	void tms_map(address_map &map) ATTR_COLD;
};


void vlc34010_state::machine_reset()
{
	m_boot_view.select(0);
}

TMS340X0_SCANLINE_RGB32_CB_MEMBER(vlc34010_state::scanline_update)
{
}

void vlc34010_state::switch_w(u8 data)
{
	m_boot_view.select(1);
}

u8 vlc34010_state::rtc_r(offs_t offset)
{
	return m_rtc->read(offset >> 4);
}

void vlc34010_state::rtc_w(offs_t offset, u8 data)
{
	m_rtc->write(offset >> 4, data);
}

template <int N> u8 vlc34010_state::duart_r(offs_t offset)
{
	return m_duart[N]->read(offset >> 4);
}

template <int N> void vlc34010_state::duart_w(offs_t offset, u8 data)
{
	m_duart[N]->write(offset >> 4, data);
}


void vlc34010_state::main_map(address_map &map)
{
	map(0x000000, 0x7fffff).view(m_boot_view);
	m_boot_view[0](0x000000, 0x1fffff).rom().region("maincpu", 0);
	m_boot_view[1](0x000000, 0x03ffff).mirror(0x7c0000).ram(); // mirroring is probably not exactly correct
	m_boot_view[1](0x510000, 0x510000).select(0xf0).rw(FUNC(vlc34010_state::rtc_r), FUNC(vlc34010_state::rtc_w));
	m_boot_view[1](0x520000, 0x520007).rw(m_tms, FUNC(tms34010_device::host_r), FUNC(tms34010_device::host_w));
	map(0xa10001, 0xa10001).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xa70001, 0xa70001).w(FUNC(vlc34010_state::switch_w));
	map(0xb10001, 0xb10001).select(0xf0).rw(FUNC(vlc34010_state::duart_r<0>), FUNC(vlc34010_state::duart_w<0>));
	map(0xb20001, 0xb20001).select(0xf0).rw(FUNC(vlc34010_state::duart_r<1>), FUNC(vlc34010_state::duart_w<1>));
	map(0xb30001, 0xb30001).select(0xf0).rw(FUNC(vlc34010_state::duart_r<2>), FUNC(vlc34010_state::duart_w<2>));
	map(0xf00000, 0xffffff).rom().region("maincpu", 0);
}

void vlc34010_state::tms_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram();
	map(0x02000000, 0x027fffff).ram();
	map(0x05000000, 0x0500007f).m("ramdac", FUNC(bt471_device::map)).umask16(0xff00);
	map(0xff800000, 0xffffffff).rom().region("tms", 0);
}

static INPUT_PORTS_START( beezerk )
// no dips but the 5-dip bank on the modem PCB
INPUT_PORTS_END


void vlc34010_state::base(machine_config &config)
{
	M68000(config, m_maincpu, 40_MHz_XTAL / 10); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &vlc34010_state::main_map);

	TMS34010(config, m_tms, 50_MHz_XTAL);
	m_tms->set_addrmap(AS_PROGRAM, &vlc34010_state::tms_map);
	m_tms->set_scanline_rgb32_callback(FUNC(vlc34010_state::scanline_update));
	m_tms->set_pixel_clock(50_MHz_XTAL); // ???

	WATCHDOG_TIMER(config, "watchdog");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size((42+1)*8, (32+1)*8);
	screen.set_visarea(0*8, 31*8-1, 0*8, 31*8-1);
	screen.set_screen_update(m_tms, FUNC(tms34010_device::tms340x0_rgb32));

	BT471(config, "ramdac", 0); // type not correct

	MC68681(config, "duart0", 3.6864_MHz_XTAL);

	MC68681(config, "duart1", 3.6864_MHz_XTAL);

	MC68681(config, "duart2", 3.6864_MHz_XTAL);

	MSM6242(config, "rtc", 32.768_kHz_XTAL);

	SPEAKER(config, "mono").front_center();
}


ROM_START( beezerk ) // possibly multigame? ROM contains Bee-zerk, Ring'Em Up, Blackjack, Polly's Gold and Red Hot 7s strings
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "main_system_rom_even.u31",   0x000000, 0x80000, CRC(aa6f737c) SHA1(87f2f5d989016b4df5fc39e4f0de685e1d7f25a1) )
	ROM_LOAD16_BYTE( "main_system_rom_odd.u32",    0x000001, 0x80000, CRC(d7b2f6f7) SHA1(f3e915229fa9db9d9fe69c62fb94895daf3c0726) )
	ROM_LOAD16_BYTE( "second_system_rom_even.u33", 0x100000, 0x80000, CRC(d2d8e483) SHA1(87b29673e4554ae66ed4e77c4e0a861142c34e5f) )
	ROM_LOAD16_BYTE( "second_system_rom_odd.u34",  0x100001, 0x80000, CRC(ff8d36a4) SHA1(6b554e12fb59353b82b526a42d045f9228ea59ba) )

	ROM_REGION16_LE( 0x200000, "tms", 0 )
	ROM_LOAD16_BYTE( "main_video_rom_low.u37",  0x000000, 0x80000, CRC(f09e2648) SHA1(c63f2fcb5b7035bbae635936dd7b95c4558aafe4) )
	ROM_LOAD16_BYTE( "main_video_rom_high.u38", 0x000001, 0x80000, CRC(1d5e46ee) SHA1(c18b01f70fc9c96e5e2eef47d844925a76197fde) )
	// second_video_rom_low.u39 and second_video_rom_high.u40 not populated

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "epm7032", 0x000, 0x200, NO_DUMP ) // EPM7032LC44, size made up
	ROM_LOAD( "epm7064", 0x200, 0x400, NO_DUMP ) // EPM7064LC84-15, size made up
	ROM_LOAD( "secpal",  0x600, 0x200, NO_DUMP ) // etched 'SECURITY PAL', type unknown
	ROM_LOAD( "revpal",  0x800, 0x200, NO_DUMP ) // etched 'REVISION PAL', type unknown
ROM_END

ROM_START( vlcunk ) // Multigame. ROM contains strings for a lot of games, Bee-Zerk included
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "main_system_rom_even.u31",   0x000000, 0x80000, CRC(6bdd995c) SHA1(9a751e0211c83b89b157bc323eaff9b587dfb1c3) )
	ROM_LOAD16_BYTE( "main_system_rom_odd.u32",    0x000001, 0x80000, CRC(37ab6b79) SHA1(6273f17c74803a5248af51402d5f2e1d1edc52ae) )
	ROM_LOAD16_BYTE( "second_system_rom_even.u33", 0x100000, 0x80000, CRC(9b257042) SHA1(bb4d04eb12ecc9610f6ff9d955707c10da100188) )
	ROM_LOAD16_BYTE( "second_system_rom_odd.u34",  0x100001, 0x80000, CRC(cccdf854) SHA1(de60aac29af79195bb60e4c715a66429642d9f9a) )

	ROM_REGION16_LE( 0x200000, "tms", 0 )
	ROM_LOAD16_BYTE( "main_video_rom_low.u37",  0x000000, 0x80000, CRC(b876616f) SHA1(0ef05817f37ffdc1d46ce0294ebd1add777674bf) )
	ROM_LOAD16_BYTE( "main_video_rom_high.u38", 0x000001, 0x80000, CRC(cb2c0270) SHA1(3eccdcac7c69fe5b22f2447547f03528055710fb) )
	// second_video_rom_low.u39 and second_video_rom_high.u40 not populated

	ROM_REGION( 0x6800, "plds", 0 )
	ROM_LOAD( "u19_as_epm7032ae.jed", 0x0000, 0x5525, BAD_DUMP CRC(20836212) SHA1(86e72ed046b004be4f407dc04d0fc158c4209ff5) ) // EPM7032LC44, to be checked
	ROM_LOAD( "101320",               0x6000, 0x0400, NO_DUMP ) // EPM7064LC84-15, size made up
	ROM_LOAD( "011-03a2",             0x6400, 0x0200, NO_DUMP ) // etched 'SECURITY PAL', type unknown
	ROM_LOAD( "011-xxxx",             0x6600, 0x0200, NO_DUMP ) // etched 'REVISION PAL', type unknown
ROM_END

} // Anonymous namespace


GAME( 1997, beezerk, 0, base, beezerk, vlc34010_state, empty_init, ROT0, "VLT Inc.", "Bee-Zerk",              MACHINE_IS_SKELETON ) // copyright in ROM is VLT instead of VLC, dump came as BeeZerk, but probably a multigame
GAME( 2000, vlcunk,  0, base, beezerk, vlc34010_state, empty_init, ROT0, "VLC Inc.", "unknown VLC multigame", MACHINE_IS_SKELETON )
