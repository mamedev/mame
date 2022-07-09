// license:BSD-3-Clause
// copyright-holders:

/*
    Sonic Blast Man's Janken Battle (ソニックブラストマンのジャンケンバトル) (c) 1998 Taito

    Redemption game based on Janken, a Japanese game similar to Rock Paper Scissors
    Video: https://www.youtube.com/watch?v=AFWLMHbpQz8

    Other undumped games believed to use the same hardware:
    Bubblen Roulette (バブルンるーれっと) (c) 1997 Taito - video: https://www.youtube.com/watch?v=AaugRz3cqv0
    Harikiri Junior Baseball (はりきりジュニアベースボール) (c) 1998 Taito - video: https://www.youtube.com/watch?v=eRZctnd8whE
    Packy's Treasure Slot (パッキイのトレジャースロット) (c) 1997 Taito - video: https://www.youtube.com/watch?v=IPse14eGiqM

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
    - everything is guesswork and should be taken with a grain of salt, especially the M66220FP hook up;
    - are the correct sounds played at the right times?
    - hopper / medal (main roadblock before it can be considered playable);
    - at boot the game zero-fills the 0x020000-0x020fff range in the tc0091lvc VRAM space, which is currently unmapped in tc009xlvc.cpp.
      Doesn't seem to use it afterwards, though.
*/

#include "emu.h"
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
		m_vdpcpu(*this, "vdpcpu")
	{ }

	void sbmjb(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<tc0091lvc_device> m_vdpcpu;

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void tc0091lvc_map(address_map &map);
};


WRITE_LINE_MEMBER(sbmjb_state::screen_vblank) // TODO: copy-pasted from other drivers using same chip, to be verified
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) // TODO: active high doesn't allow coining in but it also doesn't cause the medal in error
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) // TODO: active high doesn't allow coining in but it also doesn't cause the medal in error
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // No effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // No effect in test mode
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(sbmjb_state::scanline_callback) // TODO: copy-pasted from other drivers using same chip, to be verified
{
	int scanline = param;

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

	te7751_device & io(TE7751(config, "io"));
	io.in_port1_cb().set_ioport("IN0");
	io.in_port2_cb().set_ioport("IN1");
	// TODO: rest of ports. port9 medal / hopper?

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

} // Anonymous namespace


GAME( 1998, sbmjb, 0, sbmjb, sbmjb, sbmjb_state, empty_init, ROT0, "Taito Corporation", "Sonic Blast Man's Janken Battle (main ver. 1.1, video ver. 1.0)", MACHINE_NOT_WORKING )
