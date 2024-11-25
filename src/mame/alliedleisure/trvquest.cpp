// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

Trivia Quest
Sunn/Techstar 1984

 driver by Pierpaolo Prazzoli

This is a subdriver of killcom.cpp, the hardware is similar to it,
video blitter hardware is identical.

TODO:
- button response is too rapid? PORT_IMPULSE on coin is also a bad sign


PCB notes:

CPU: 6809
Three SY6522 - for flashing lighted buttons?

Sound: Two AY-3-8910

Forty eight! MK4027 4K Rams

Six 6116 Rams with battery backup

Two Crystals:
11.6688 Mhz
6 Mhz

Two 8 position DIPS

rom3 through rom7 - Main pcb PRG.

roma through romi - Sub pcb Questions.

The main pcb had empty sockets for rom0, rom1 and rom2.
This pcb has been tested and works as is.

*/

#include "emu.h"
#include "killcom.h"

#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"

#include "speaker.h"


namespace {

class trvquest_state : public killcom_state
{
public:
	trvquest_state(const machine_config &mconfig, device_type type, const char *tag) :
		killcom_state(mconfig, type, tag),
		m_bank(*this, "bank")
	{ }

	void trvquest(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_bank m_bank;

	void bankswitch_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Bankswitch
 *
 *************************************/

void trvquest_state::machine_start()
{
	// no need to call killcom_state::machine_start()
	m_bank->configure_entries(0, 16, memregion("questions")->base(), 0x2000);
}

void trvquest_state::bankswitch_w(uint8_t data)
{
	m_bank->set_entry(data & 0xf);
}



/*************************************
 *
 *  Address map
 *
 *************************************/

void trvquest_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram"); // cmos ram
	map(0x2000, 0x27ff).ram(); // main ram
	map(0x2800, 0x2800).nopr(); // ?
	map(0x3800, 0x380f).m(m_via[1], FUNC(via6522_device::map));
	map(0x3810, 0x381f).m(m_via[2], FUNC(via6522_device::map));
	map(0x3820, 0x382f).m(m_via[0], FUNC(via6522_device::map));
	map(0x3830, 0x3831).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x3840, 0x3841).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x3850, 0x3850).nopr(); // watchdog_reset_r ?
	map(0x8000, 0x9fff).bankr(m_bank);
	map(0xa000, 0xa000).w(FUNC(trvquest_state::bankswitch_w)).nopr();
	map(0xb000, 0xffff).rom();
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( trvquest )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("UNK")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Machine config
 *
 *************************************/

void trvquest_state::trvquest(machine_config &config)
{
	M6809(config, m_maincpu, 6_MHz_XTAL/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &trvquest_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// via
	MOS6522(config, m_via[0], 6_MHz_XTAL/4);
	m_via[0]->writepa_handler().set(FUNC(trvquest_state::video_data_w));
	m_via[0]->writepb_handler().set(FUNC(trvquest_state::video_command_w));
	m_via[0]->readpb_handler().set(FUNC(trvquest_state::video_status_r));
	m_via[0]->ca2_handler().set(FUNC(trvquest_state::video_command_trigger_w));

	MOS6522(config, m_via[1], 6_MHz_XTAL/4);
	m_via[1]->readpa_handler().set_ioport("IN0");
	m_via[1]->readpb_handler().set_ioport("IN1");
	m_via[1]->ca2_handler().set(FUNC(trvquest_state::coin_w));

	MOS6522(config, m_via[2], 6_MHz_XTAL/4);
	m_via[2]->readpa_handler().set_ioport("UNK");
	m_via[2]->readpb_handler().set_ioport("DSW0");
	m_via[2]->irq_handler().set_inputline(m_maincpu, 0);

	// video hardware
	killcom_video(config);
	m_screen->screen_vblank().set(m_via[2], FUNC(via6522_device::write_ca1));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", 6_MHz_XTAL/2).add_route(ALL_OUTPUTS, "mono", 0.25);
	AY8910(config, "ay2", 6_MHz_XTAL/2).add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*************************************
 *
 *  ROM defintions
 *
 *************************************/

ROM_START( trvquest )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom3", 0xb000, 0x1000, CRC(2ff7f370) SHA1(66f40426ed02ee44235e17a49d9054ede42b83b9) )
	ROM_LOAD( "rom4", 0xc000, 0x1000, CRC(b1adebcb) SHA1(661cabc92b1defce5c2edb8e873a80d5032084d0) )
	ROM_LOAD( "rom5", 0xd000, 0x1000, CRC(2fc10a15) SHA1(8ecce32a5a167056c8fb48554a8907ae6299921e) )
	ROM_LOAD( "rom6", 0xe000, 0x1000, CRC(fabf4846) SHA1(862cac32de78f2ff4afef398b864d5533d302a4f) )
	ROM_LOAD( "rom7", 0xf000, 0x1000, CRC(a9f56551) SHA1(fb6fc3b17a6e66571a5ba837befbfac1ac26cc39) )

	ROM_REGION( 0x20000, "questions", ROMREGION_ERASEFF ) // Question roms
	// 0x00000 - 0x05fff empty
	ROM_LOAD( "romi", 0x06000, 0x2000, CRC(c8368f69) SHA1(c1dfb701482c5ae922df0a93665a519995a2f4f1) )
	ROM_LOAD( "romh", 0x08000, 0x2000, CRC(f3aa8a08) SHA1(2bf8f878cc1df84806a6fb8e7be2656c422d61b9) )
	ROM_LOAD( "romg", 0x0a000, 0x2000, CRC(f85f8e48) SHA1(38c9142181a8ee5c0bc80cf2a06d4137fcb2a8b9) )
	ROM_LOAD( "romf", 0x0c000, 0x2000, CRC(2bffdcab) SHA1(96bd9aede5a76f9ddcf29e8df2c632075d21b8f6) )
	ROM_LOAD( "rome", 0x0e000, 0x2000, CRC(3ff66402) SHA1(da13fe6b99d7517ad2ecd0e42d0c306d4e49563a) )
	ROM_LOAD( "romd", 0x10000, 0x2000, CRC(4e21653f) SHA1(719a8dda9b81963a6b6d7d3e4966ecde676b9ecf) )
	ROM_LOAD( "romc", 0x12000, 0x2000, CRC(081a5322) SHA1(09e7ea5f1ee1dc35ec00bcea1550c6fe0bbdf60d) )
	ROM_LOAD( "romb", 0x14000, 0x2000, CRC(819ab451) SHA1(78c181eae63d55d1d0643bb7be07ca3cdbe14285) )
	ROM_LOAD( "roma", 0x16000, 0x2000, CRC(b4bcaf33) SHA1(c6b08fb8d55b2834d0c6c5baff9f544c795e4c15) )
ROM_END

} // anonymous namespace



//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        SCREEN  COMPANY            FULLNAME        FLAGS
GAME( 1984, trvquest, 0,      trvquest, trvquest, trvquest_state, empty_init, ROT90,  "Techstar / Sunn", "Trivia Quest", MACHINE_SUPPORTS_SAVE )
