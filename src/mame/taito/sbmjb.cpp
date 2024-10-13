// license:BSD-3-Clause
// copyright-holders:

/*
    Taito redemption games

    Honoo no Invader (炎のインベーダー) (c) 1997 Taito
    Bubblen Roulette (バブルンるーれっと) (c) 1997 Taito - video: https://www.youtube.com/watch?v=AaugRz3cqv0
    Packy's Treasure Slot (パッキイのトレジャースロット) (c) 1997 Taito - video: https://www.youtube.com/watch?v=IPse14eGiqM
    Sonic Blast Man's Janken Battle (ソニックブラストマンのジャンケンバトル) (c) 1998 Taito - video: https://www.youtube.com/watch?v=AFWLMHbpQz8
    Renda Fighter (レンダファイター) (c) 2000 Taito - video: https://www.youtube.com/watch?v=GbeX28lbqbk

    Other undumped games believed to use the same hardware:
    Harikiri Junior Baseball (はりきりジュニアベースボール) (c) 1998 Taito - video: https://www.youtube.com/watch?v=eRZctnd8whE

    Main components:
    2 x Z0840004PSC
    1 x TC0091LVC (marked as TC0090LVC on PCB)
    1 x YM2203C
    1 x OKI M6295
    1 x TE7751
    1 x MACH211 (undumped)
    1 x 8.0000MHz Osc (near the Z80s)
    1 x 13.3306MHz Osc (near TC0091LVC, marked as 13.33056MHz on PCB)
    2 x M66220FP
    various unpopulated IC spaces (marked M66011FP, M66230FP, M66240FP, M66300FP, uPD4701A, 2 TLP521-4)
    no dips

    TODO:
    general:
    - everything is guesswork and should be taken with a grain of salt, especially the M66220FP hook up;
    - are the correct sounds played at the right times?
    - hopper / medal (main roadblock before sbmjb can be considered playable, while honooinv also needs the mechanical parts);
    sbmjb:
    - NVRAM hook-up
    - at boot the game zero-fills the 0x020000-0x020fff range in the tc0091lvc VRAM space, which is currently unmapped in tc009xlvc.cpp.
      Doesn't seem to use it afterwards, though.
    honooinv:
    - mechanical parts
*/

#include "emu.h"

#include "taitoio_opto.h"

#include "cpu/z80/z80.h"
#include "machine/tc009xlvc.h"
#include "machine/te7750.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "screen.h"
#include "speaker.h"


namespace {

class sbmjb_state : public driver_device
{
public:
	sbmjb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdpcpu(*this, "vdpcpu"),
		m_opto(*this, "opto")
	{ }

	void honooinv(machine_config &config);
	void sbmjb(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<tc0091lvc_device> m_vdpcpu;
	required_device<taitoio_opto_device> m_opto;

	void screen_vblank(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void tc0091lvc_map(address_map &map) ATTR_COLD;
};


void sbmjb_state::screen_vblank(int state) // TODO: copy-pasted from other drivers using same chip, to be verified
{
	if (state)
	{
		m_vdpcpu->screen_eof();
	}
}


void sbmjb_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe0ff).ram().share("m66220fp0"); // TODO: really?
	map(0xe100, 0xe7ff).ram();
	map(0xe800, 0xe8ff).ram().share("m66220fp1"); // TODO: really?
	map(0xe900, 0xefff).ram();
	map(0xf000, 0xf00f).rw("io", FUNC(te7751_device::read), FUNC(te7751_device::write));
}

void sbmjb_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe0ff).ram().share("m66220fp1");
	map(0xf000, 0xf001).rw("opn", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf800, 0xf800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void sbmjb_state::tc0091lvc_map(address_map &map) // TODO: copy-pasted from other drivers using same chip, to be verified
{
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa0ff).ram().share("m66220fp0");
	map(0xfe00, 0xfeff).rw(m_vdpcpu, FUNC(tc0091lvc_device::vregs_r), FUNC(tc0091lvc_device::vregs_w));
	map(0xff00, 0xff02).rw(m_vdpcpu, FUNC(tc0091lvc_device::irq_vector_r), FUNC(tc0091lvc_device::irq_vector_w));
	map(0xff03, 0xff03).rw(m_vdpcpu, FUNC(tc0091lvc_device::irq_enable_r), FUNC(tc0091lvc_device::irq_enable_w));
	map(0xff04, 0xff07).rw(m_vdpcpu, FUNC(tc0091lvc_device::ram_bank_r), FUNC(tc0091lvc_device::ram_bank_w));
	map(0xff08, 0xff08).rw(m_vdpcpu, FUNC(tc0091lvc_device::rom_bank_r), FUNC(tc0091lvc_device::rom_bank_w));
}

static INPUT_PORTS_START( honooinv ) // no dips on PCB, game options selectable in test mode
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Separator") // in I/O test; also seems to have effect on 'Hopper Rotation'. Effects shown also in the shot test
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Select Sw") // in I/O test
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Enter Sw") // in I/O test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Wait")  // in shot test; seems to have effect on 'Show Power Level', too
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Charge Solenoid") // in shot test
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("Shoot Solenoid") // in shot test
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // seems to have no effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // "

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) // in pinpanel test
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Start Close") // in pinpanel test
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Win-L") // in pinpanel test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Win-R") // in pinpanel test
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("Lucky-L") // in pinpanel test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("Lucky-C") // in pinpanel test
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("Lucky-R") // in pinpanel test
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("Lucky Close") // in pinpanel test

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("Pay Out Sen") // in I/O test
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("Reset-Key") // in I/O test; seems to have effect on 'Lock Out Coil', too
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("Crt-Key") // in I/O test
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW ) // 'Operate Sw' in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3) PORT_NAME("All Clear Sw") // in I/O test
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_h_r)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_l_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // in I/O test

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // seems to have no effect in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // seems to have no effect in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // seems to have no effect in test mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // seems to have no effect in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("Rail Min") // in shot test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("Rail Max") // in shot test
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("Open-L") // in pinpanel test
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_NAME("Open-R") // in pinpanel test

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, coin_sense_w)
INPUT_PORTS_END

static INPUT_PORTS_START( bubbroul )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet 5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Bet 4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Bet 8")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Bet 10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Start/Stop Switch") // Also used to confirm in test mode

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON7 ) PORT_NAME("100 Yen Switch")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_h_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_l_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, coin_sense_w)
INPUT_PORTS_END

static INPUT_PORTS_START( packysts )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Yes")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("No")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Start/Stop Switch") // Also used to confirm in test mode

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON5 ) PORT_NAME("100 Yen Switch")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_h_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_l_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, coin_sense_w)
INPUT_PORTS_END

static INPUT_PORTS_START( sbmjb ) // no dips on PCB, game options selectable in test mode
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet 5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Bet 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Choki")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Guu")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Paa")// Also used to confirm in test mode

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) // Also used to select in test mode
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) // Hopper Sensor, active high or it will stop booting due to hopper related problems
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON7 ) PORT_NAME("100 Yen Switch")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_h_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_l_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // No effect in test mode

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, coin_sense_w)
INPUT_PORTS_END

static INPUT_PORTS_START( rendfgtr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Enter")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Select")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // No effect in test mode

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON5 ) PORT_NAME("100 Yen Switch")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_h_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_l_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, coin_sense_w)
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(sbmjb_state::scanline_callback) // TODO: copy-pasted from other drivers using same chip, to be verified
{
	int const scanline = param;

	if (scanline == 240 && (m_vdpcpu->irq_enable() & 4))
	{
		m_vdpcpu->set_input_line_and_vector(0, HOLD_LINE, m_vdpcpu->irq_vector(2));
	}

	if (scanline == 0 && (m_vdpcpu->irq_enable() & 2))
	{
		m_vdpcpu->set_input_line_and_vector(0, HOLD_LINE, m_vdpcpu->irq_vector(1));
	}
}

void sbmjb_state::sbmjb(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &sbmjb_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(sbmjb_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 8_MHz_XTAL / 2)); // divider not verified, but marked as 4MHz on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &sbmjb_state::sound_map);

	TC0091LVC(config, m_vdpcpu, 13.33056_MHz_XTAL / 2); // TC0091LVC, divider not verified
	m_vdpcpu->set_addrmap(AS_PROGRAM, &sbmjb_state::tc0091lvc_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(sbmjb_state::scanline_callback), "screen", 0, 1);

	te7751_device &io(TE7751(config, "io"));
	io.in_port1_cb().set_ioport("IN0");
	io.in_port2_cb().set_ioport("IN1");
	// TODO: rest of ports. port9 medal / hopper?

	TAITOIO_OPTO(config, "opto", 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update("vdpcpu", FUNC(tc0091lvc_device::screen_update));
	screen.screen_vblank().set(FUNC(sbmjb_state::screen_vblank));
	screen.set_palette("vdpcpu:palette");

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ym2203_device &opn(YM2203(config, "opn", 8_MHz_XTAL / 2)); // divider not verified
	opn.irq_handler().set_inputline("audiocpu", 0);
	opn.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // pin not verified
}

void sbmjb_state::honooinv(machine_config &config)
{
	sbmjb(config);

	te7751_device &io(*subdevice<te7751_device>("io"));
	io.in_port1_cb().set_ioport("IN0");
	io.in_port2_cb().set_ioport("IN1");
	io.in_port3_cb().set_ioport("IN2");
	io.in_port4_cb().set_ioport("IN3");
}

ROM_START( honooinv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e41-08.ic12", 0x00000, 0x10000, CRC(71fc6a58) SHA1(8e95f42fa227e7bbf80dc7690a016f5e43a8125d) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "e41-07.ic5", 0x00000, 0x10000, CRC(0f1faa0b) SHA1(ff8a242507207be3ced2a77705b24a5b6dd1189c) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "vdpcpu", 0 )
	ROM_LOAD( "e41-04.ic52", 0x00000, 0x80000, CRC(2e236b65) SHA1(4c5dda35d083742ad708d575e1d8cd158fa89210) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "vdpcpu:gfx", 0 )
	ROM_LOAD16_BYTE( "e41-05.ic49", 0x00000, 0x80000, CRC(493ddfac) SHA1(b37ac65c08dda284e15446bdc545cca889e03ba7) )
	ROM_LOAD16_BYTE( "e41-06.ic48", 0x00001, 0x80000, CRC(edb38f82) SHA1(d94f05f96a7acdb9e3c98646cbdd797fdc3b92b7) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "e41-09.ic3", 0x00000, 0x40000, CRC(5d25fb77) SHA1(b474be05cdb2e57632595caee7f8c27572db8935) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "e41-02.ic51", 0x000, 0x117, CRC(67fd54e0) SHA1(f64fb33b9a4a935af5662b5103709131727c8411) )
	ROM_LOAD( "e41-03.ic24", 0x200, 0x117, CRC(d906c8ea) SHA1(eae9c9c25b4affe4baf7ba034c61670d24f5c4d1) )
ROM_END

ROM_START( bubbroul )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Main ver. 1.8 1998/11/02
	ROM_LOAD( "e48-05-1.ic12", 0x00000, 0x10000, CRC(25db07c9) SHA1(c14ce8ce4e9182952ffa83452e0ca52a3d3c99ef) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "e48-04.ic5", 0x00000, 0x10000, CRC(b25f98c6) SHA1(fc6f36037c3d6a0eef1a266511b83f0cb88fe003) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "vdpcpu", 0 ) // Video ver. 1.3 1998/01/05
	ROM_LOAD( "e48-01s.ic52", 0x00000, 0x80000, CRC(e6dd4cff) SHA1(11666036715c117fc5429b8a72a63c468b82c796) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "vdpcpu:gfx", 0 )
	ROM_LOAD16_BYTE( "e48-02.ic49", 0x00000, 0x80000, CRC(c8b98e3c) SHA1(e25d52e14223cc60f3047ae22ccea711fdf4066a) )
	ROM_LOAD16_BYTE( "e48-03.ic48", 0x00001, 0x80000, CRC(a46dcf53) SHA1(573ea201d9d61703002d5da6b278b7ee02efab35) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "e48-06.ic3", 0x00000, 0x40000, CRC(96c2f509) SHA1(d2a81dcbb1c9225b30d8da2ada09b5063dfecca0) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "e41-02.ic51", 0x000, 0x117, CRC(67fd54e0) SHA1(f64fb33b9a4a935af5662b5103709131727c8411) )
	ROM_LOAD( "e41-03.ic24", 0x200, 0x117, CRC(d906c8ea) SHA1(eae9c9c25b4affe4baf7ba034c61670d24f5c4d1) )
ROM_END

ROM_START( packysts )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Main ver. 1.3 1998/11/02
	ROM_LOAD( "e55-05-1.ic12", 0x00000, 0x10000, CRC(3bf64ee6) SHA1(af7dc848f3cee4e0391ff9f940534647856a78f3) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "e55-04.ic5", 0x00000, 0x10000, CRC(6b72541c) SHA1(e2731edd6275786e135b1cf0b533ab4fe6c695d5) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "vdpcpu", 0 ) // Video ver. 1.1 1998/03/09
	ROM_LOAD( "e55-01.ic52", 0x00000, 0x80000, CRC(5ab757af) SHA1(38851b8362075448f30ce66df1821e598de3a870) )

	ROM_REGION( 0x100000, "vdpcpu:gfx", 0 )
	ROM_LOAD16_BYTE( "e55-02.ic49", 0x00000, 0x80000, CRC(88220981) SHA1(d597881f85ed4ade0fab788aa311825db9ddf194) )
	ROM_LOAD16_BYTE( "e55-03.ic48", 0x00001, 0x80000, CRC(4bb0dcd8) SHA1(c563a21831dc1beadfde4f8b9ea0b8c208b57aa0) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "e55-06.ic3", 0x00000, 0x40000, CRC(031c573e) SHA1(7c05e0aea9d90fb821b7e65d17a906bf6bf8dc4d) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "e41-02.ic51", 0x000, 0x117, CRC(67fd54e0) SHA1(f64fb33b9a4a935af5662b5103709131727c8411) )
	ROM_LOAD( "e41-03.ic24", 0x200, 0x117, CRC(d906c8ea) SHA1(eae9c9c25b4affe4baf7ba034c61670d24f5c4d1) )
ROM_END

ROM_START( sbmjb ) // all labels were peeled off / unreadable
	ROM_REGION( 0x10000, "maincpu", 0 ) // Main ver. 1.1 1998/08/25
	ROM_LOAD( "mprog.ic12", 0x00000, 0x10000, CRC(d11f14eb) SHA1(29c4b8e3ebb9ff3c5630c7bb3c8224a2f57e8fe8) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sndprog.ic5", 0x00000, 0x10000, CRC(e41575f1) SHA1(66ab1b81a618fe5e676b4f8768a5bee8d189de58) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "vdpcpu", 0 ) // Video ver. 1.0 1998/06/01
	ROM_LOAD( "videoprg.ic52", 0x00000, 0x80000, CRC(21ebc096) SHA1(12f0ad4530560782a5b7517557d68526a51091e1) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "vdpcpu:gfx", 0 )
	ROM_LOAD16_BYTE( "chr-l.ic49", 0x00000, 0x80000, CRC(54237760) SHA1(d183d4c3df3222e94219356a39ee5566ebdd54cf) )
	ROM_LOAD16_BYTE( "chr-h.ic48", 0x00001, 0x80000, CRC(0a815d7c) SHA1(5c2f65ff1f84979993f7c0df8d91ed2cd6f0acd1) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "adpcm.ic3", 0x00000, 0x40000, CRC(69158cf3) SHA1(f638bd7b5ef9e400eaa55eb3b4546e881e955087) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "e41-02.ic51", 0x000, 0x117, CRC(67fd54e0) SHA1(f64fb33b9a4a935af5662b5103709131727c8411) )
	ROM_LOAD( "e41-03.ic24", 0x200, 0x117, CRC(d906c8ea) SHA1(eae9c9c25b4affe4baf7ba034c61670d24f5c4d1) )
ROM_END

ROM_START( rendfgtr )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Main ver. 2.02 2000/04/17
	ROM_LOAD( "e88-05.ic12", 0x00000, 0x10000, CRC(8d723f55) SHA1(d73ae0938180a32ef82c195b97ebfd46aa0e1346) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "e88-04.ic5", 0x00000, 0x10000, CRC(205f589b) SHA1(01fb8f5d40f1188a1f3ae0826f00271260fb3aee) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "vdpcpu", 0 ) // Video ver. 2.03 2000/04/18
	ROM_LOAD( "e88-01.ic52", 0x00000, 0x80000, CRC(6fe53a0c) SHA1(6b8748c8a2f0109581615594b9c95521eb878c7f) )

	ROM_REGION( 0x100000, "vdpcpu:gfx", 0 )
	ROM_LOAD16_BYTE( "e88-02.ic49", 0x00000, 0x80000, CRC(f5fdfd55) SHA1(e96ae62130a752f788b6c50fd422ff44591cd0a1) )
	ROM_LOAD16_BYTE( "e88-03.ic48", 0x00001, 0x80000, CRC(d1d37b94) SHA1(8cbe4852292c14bf6011d18727b3432b3f5f2874) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "e88-06.ic3", 0x00000, 0x40000, CRC(7d57afed) SHA1(48bf94b69af0d363e1561cf3ab23db534fa94d3e) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "e41-02.ic51", 0x000, 0x117, CRC(67fd54e0) SHA1(f64fb33b9a4a935af5662b5103709131727c8411) )
	ROM_LOAD( "e41-03.ic24", 0x200, 0x117, CRC(d906c8ea) SHA1(eae9c9c25b4affe4baf7ba034c61670d24f5c4d1) )
ROM_END

} // anonymous namespace

GAME( 1997, honooinv, 0, honooinv, honooinv, sbmjb_state, empty_init, ROT0, "Taito Corporation", "Honoo no Invader (Japan, main ver. 1.35, video ver. 1.35)",              MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1997, bubbroul, 0, sbmjb,    bubbroul, sbmjb_state, empty_init, ROT0, "Taito Corporation", "Bubblen Roulette (Japan, main ver. 1.8, video ver. 1.3)",                MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1997, packysts, 0, sbmjb,    packysts, sbmjb_state, empty_init, ROT0, "Taito Corporation", "Packy's Treasure Slot (Japan, main ver. 1.3, video ver. 1.1)",           MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1998, sbmjb,    0, sbmjb,    sbmjb,    sbmjb_state, empty_init, ROT0, "Taito Corporation", "Sonic Blast Man's Janken Battle (Japan, main ver. 1.1, video ver. 1.0)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 2000, rendfgtr, 0, sbmjb,    rendfgtr, sbmjb_state, empty_init, ROT0, "Taito Corporation", "Renda Fighter (Japan, main ver. 2.02, video ver. 2.03)",                 MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
