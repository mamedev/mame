// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Taito Bingo Wave

    This is a skeleton driver for what appears to be a satellite betting
    game similar to Sega's Caribbean Boule.

    "MAIN PCB"
    - Toshiba TMP68303F-16 (IC26)
    - Mitsubishi M66220SP (IC19)
    - Unpopulated space for 68000 (IC36)
    - TEL TE7750 (IC38, IC47)
    - Taito TC0140SYT (IC68)
    - Zilog Z80-A (IC74)
    - Yamaha YM2610 (IC77)
    - ??MHz XTAL (OSC1)

    "TERMINAL PCB"
    - Toshiba TMP68303F-16 (IC35)
    - Taito TC0650FDA (IC26)
    - Unpopulated space for 68000-16 (IC41)
    - TEL TE7750 (IC42)
    - Taito TCO470LIN (IC33)
    - Taito TC0600OBT (IC11)
    - Yamaha YM2149F (IC53)
    - 36MHz XTAL (OSC1)
    - 32MHz XTAL (OSC2)
    - DIP switches (1x8)
    - JAMMA edge connector

    "CONTROL PCB"
    - Taito PC060HA (IC2)
    - Zilog Z80 (IC13)
    - 16MHz XTAL (OSC1)
    - DIP switches (1x8)

***************************************************************************/

#include "emu.h"
#include "speaker.h"
#include "cpu/m68000/tmp68301.h"
#include "cpu/z80/z80.h"
#include "machine/te7750.h"
#include "sound/ymopn.h"
#include "taitosnd.h"


namespace {

class bingowav_state : public driver_device
{
public:
	bingowav_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void bingowav(machine_config &config);
	void bingowav_audio_map(address_map &map) ATTR_COLD;
	void bingowav_control_map(address_map &map) ATTR_COLD;
	void bingowav_drive_map(address_map &map) ATTR_COLD;
	void bingowav_main_map(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
};


void bingowav_state::machine_start()
{
}


void bingowav_state::bingowav_main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x120000, 0x12001f).rw("mainioh", FUNC(te7750_device::read), FUNC(te7750_device::write)).umask16(0xff00);
	map(0x120000, 0x12001f).rw("mainiol", FUNC(te7750_device::read), FUNC(te7750_device::write)).umask16(0x00ff);
	map(0x140000, 0x140001).nopr();
	map(0x150001, 0x150001).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x150003, 0x150003).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}

void bingowav_state::bingowav_audio_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).w("tc0140syt", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("tc0140syt", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xf200, 0xf200).nopw();
}

void bingowav_state::bingowav_drive_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
}

void bingowav_state::bingowav_control_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xf000, 0xf000).nopw();
}


static INPUT_PORTS_START( bingowav )
INPUT_PORTS_END


void bingowav_state::bingowav(machine_config &config)
{
	TMP68303(config, m_maincpu, 12000000); // TMP63803F-16
	m_maincpu->set_addrmap(AS_PROGRAM, &bingowav_state::bingowav_main_map);

	te7750_device &mainioh(TE7750(config, "mainioh"));
	mainioh.ios_cb().set_constant(5);

	te7750_device &mainiol(TE7750(config, "mainiol"));
	mainiol.ios_cb().set_constant(4);

	Z80(config, "audiocpu", 4000000).set_addrmap(AS_PROGRAM, &bingowav_state::bingowav_audio_map);

	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline("audiocpu", INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline("audiocpu", INPUT_LINE_RESET);

	m68000_device &termcpu(TMP68303(config, "termcpu", 12000000)); // actually TMP63803F-16
	termcpu.set_addrmap(AS_PROGRAM, &bingowav_state::bingowav_drive_map);
	termcpu.set_disable();

	Z80(config, "ctrlcpu", XTAL(16'000'000) / 4).set_addrmap(AS_PROGRAM, &bingowav_state::bingowav_control_map);
}


ROM_START( bingowav )
	// Program and graphics ROMs from the "Terminal PCB" are currently missing
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prg-h.ic35", 0x00000, 0x20000, CRC(0ae8c738) SHA1(c040c4f46089a07ef3c4d5067068ea3cd1546e4e) )
	ROM_LOAD16_BYTE( "prg-l.ic42", 0x00001, 0x20000, CRC(a7742e54) SHA1(2ab75c51ced0bb37b50f5077b611cde1770e2411) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic66", 0x00000, 0x10000, CRC(9fa09b9a) SHA1(e333b11129141649a18906120805842c2dcccd05) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "ic69", 0x00000, 0x80000, CRC(5d86d5bd) SHA1(81ebb63f984690e63e45c8b1fe4251fac8d5cf04) )

	ROM_REGION( 0x40000, "termcpu", 0 )
	ROM_LOAD16_BYTE( "prg-h.ic25", 0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "prg-l.ic20", 0x00001, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "ctrlcpu", 0 )
	ROM_LOAD( "ic1", 0x00000, 0x20000, CRC(d257bbaf) SHA1(9d1414594dc1c1fca256ab5b6039e733bff1414a) )
ROM_END

} // anonymous namespace


GAME( 1994, bingowav, 0, bingowav, bingowav, bingowav_state, empty_init, ROT0, "Taito", "Bingo Wave", MACHINE_IS_SKELETON )
