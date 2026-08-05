// license:BSD-3-Clause
// copyright-holders:

/*
Hai Wei
WITSVS

Main components:
VRenderZERO+ MagicEyes EISC SoC
14.31818 MHz XTAL (Y1, near VR0+)
28.63636 MHz XTAL (OSC1, also near VR0+)
1.8432 MHz XTAL (X101, near MAX232)
3x Samsung K4S641632K-UC60 SDRAM
RTL8019AS Realtek Full-Duplex Ethernet Controller with Plug and Play Function
FB2022 Transmit / Receiver Filter
MAX232 Dual Driver / Receiver
battery
various jumpers
bank of 8 switches
reset push-button

System's promotional flyer claims:
- Supports touchscreen, mouse, keyboard and other input driver
- Supports IEEE802.2, RS232, RS485
- Supports coin in / out, bill acceptor, printer
- Supports CF card and other storage
- Supports VGA and LCD (high-resolution) and video-games (low-resolution)
  and TV (video)

TODO:
- currently stops at the manufacturer logo (tight loops between two JMPs);
- transmits to SIO if above is skipped;

*/

#include "emu.h"

#include "cpu/se3208/se3208.h"
#include "machine/vrender0.h"
#include "sound/vrender0.h"
#include "video/vrender0.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class haiwei_state : public driver_device
{
public:
	haiwei_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc")
	{}

	void hqdf(machine_config &config) ATTR_COLD;

	void init_hqdf() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;

	u32 m_pio = 0;

	void program_map(address_map &map) ATTR_COLD;

	u32 pioldat_r();
	void pioldat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pioedat_r();
};


void haiwei_state::machine_start()
{
	save_item(NAME(m_pio));
}

u32 haiwei_state::pioldat_r()
{
	return m_pio;
}

// PIO Latched output DATa Register
// TODO: bits 4 and 29 sets as output (latter flips between 1 and 0)
void haiwei_state::pioldat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pio);
}

// PIO External DATa Register
u32 haiwei_state::pioedat_r()
{
	return 0;
}


void haiwei_state::program_map(address_map &map)
{
	map(0x00000000, 0x005fffff).rom();

	map(0x01500000, 0x01500003).portr("IN0");
	map(0x01500008, 0x0150000b).portr("IN1");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(haiwei_state::pioldat_r), FUNC(haiwei_state::pioldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(haiwei_state::pioedat_r));
	// TODO: from SIO
	map(0x01802804, 0x01802807).nopw();
	map(0x01802810, 0x01802813).lr32(NAME([] () { return 0x4; }));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));
}


static INPUT_PORTS_START( hqdf )
	PORT_START("IN0")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN0" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN0-1" )
	PORT_DIPSETTING(          0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN1" )
	PORT_DIPSETTING(          0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


void haiwei_state::hqdf(machine_config &config)
{
	SE3208(config, m_maincpu, 14.318181_MHz_XTAL * 3); // multiplier not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &haiwei_state::program_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	VRENDER0_SOC(config, m_vr0soc, 14.318181_MHz_XTAL * 6); // multiplier not verified
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);
	m_vr0soc->set_external_vclk(28.636363_MHz_XTAL);

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}


// 环球大富翁 (Huánqiú Dàfùwēng)
ROM_START( hqdf )
	ROM_REGION32_LE( 0x600000, "maincpu", 0 ) // all ST27C160
	ROM_LOAD( "1", 0x000000, 0x200000, CRC(4ce1c5eb) SHA1(df69037a44ce9b8944ddb38d7c226d1acf57eaca) )
	ROM_LOAD( "2", 0x200000, 0x200000, CRC(9f5fbd56) SHA1(518937dafe754a575987913fa19c5c135930cecc) )
	ROM_LOAD( "3", 0x400000, 0x200000, CRC(4edea690) SHA1(ba919577418e9f245113160adbdca9d63c54822e) )
ROM_END


void haiwei_state::init_hqdf() // TODO: verify if it is complete (shouldn't get this far if it weren't complete, though)
{
	u8 *rom = memregion("maincpu")->base();
	std::vector<u8> buffer(0x600000);

	memcpy(&buffer[0], rom, 0x600000);

	for (int i = 0; i < 0x600000; i++)
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 7, 8, 6, 5, 4, 3, 2, 1, 0)];

	for (int i = 0; i < 0x600000; i++)
	{
		if (i & 0x01)
			rom[i] = bitswap<8>(rom[i], 6, 7, 5, 4, 0, 1, 2, 3);
		else
			rom[i] = bitswap<8>(rom[i], 6, 7, 5, 4, 3, 2, 1, 0);
	}
}

} // anonymous namespace


GAME( 2006, hqdf, 0, hqdf,  hqdf, haiwei_state, init_hqdf, ROT0, "Hai Wei Technology", "Huanqiu Dafuweng", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
