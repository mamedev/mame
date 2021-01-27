// license:BSD-3-Clause
// copyright-holders:
/*

    Sonic Blast Man's Janken Battle (c) 1998 Taito

    Redemption game based on Janken, a Japanese game similar to Rock Paper Scissors
    Video: https://www.youtube.com/watch?v=AFWLMHbpQz8

    A (possibly) one-off PCB design with the following main components:
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

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/te7750.h"
#include "machine/tc009xlvc.h"
#include "machine/timer.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"

class sbmjb_state : public driver_device
{
public:
	sbmjb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdpcpu(*this, "vdpcpu"),
		m_vdp(*this, "tc0091lvc")
	{ }

	void sbmjb(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_vdpcpu;
	required_device<tc0091lvc_device> m_vdp;

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
		m_vdp->screen_eof();
	}
}


void sbmjb_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

void sbmjb_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
}

void sbmjb_state::tc0091lvc_map(address_map &map) // TODO: copy-pasted from other drivers using same chip, to be verified
{
	map(0x0000, 0xfdff).m(m_vdp, FUNC(tc0091lvc_device::cpu_map));

	map(0xfe00, 0xfeff).rw(m_vdp, FUNC(tc0091lvc_device::vregs_r), FUNC(tc0091lvc_device::vregs_w));
	map(0xff00, 0xff02).rw(m_vdp, FUNC(tc0091lvc_device::irq_vector_r), FUNC(tc0091lvc_device::irq_vector_w));
	map(0xff03, 0xff03).rw(m_vdp, FUNC(tc0091lvc_device::irq_enable_r), FUNC(tc0091lvc_device::irq_enable_w));
	map(0xff04, 0xff07).rw(m_vdp, FUNC(tc0091lvc_device::ram_bank_r), FUNC(tc0091lvc_device::ram_bank_w));
	map(0xff08, 0xff08).rw(m_vdp, FUNC(tc0091lvc_device::rom_bank_r), FUNC(tc0091lvc_device::rom_bank_w));
}

static INPUT_PORTS_START( sbmjb )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(sbmjb_state::scanline_callback) // TODO: copy-pasted from other drivers using same chip, to be verified
{
	int scanline = param;

	if (scanline == 240 && (m_vdp->irq_enable() & 4))
	{
		m_vdpcpu->set_input_line_and_vector(0, HOLD_LINE, m_vdp->irq_vector(2)); // TC0091LVC
	}

	if (scanline == 0 && (m_vdp->irq_enable() & 2))
	{
		m_vdpcpu->set_input_line_and_vector(0, HOLD_LINE, m_vdp->irq_vector(1)); // TC0091LVC
	}

	if (scanline == 196 && (m_vdp->irq_enable() & 1))
	{
		//m_vdpcpu->set_input_line_and_vector(0, HOLD_LINE, m_vdp->irq_vector(0)); // TC0091LVC
	}
}

void sbmjb_state::sbmjb(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &sbmjb_state::main_map);

	z80_device &audiocpu(Z80(config, "audiocpu", 8_MHz_XTAL / 2)); // divider not verified, but marked as 4MHz on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &sbmjb_state::sound_map);

	Z80(config, m_vdpcpu, 13.33056_MHz_XTAL / 2); // TC0091LVC, divider not verified
	m_vdpcpu->set_addrmap(AS_PROGRAM, &sbmjb_state::tc0091lvc_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(sbmjb_state::scanline_callback), "screen", 0, 1);

	TE7750(config, "io"); // TE7751

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update("tc0091lvc", FUNC(tc0091lvc_device::screen_update));
	screen.screen_vblank().set(FUNC(sbmjb_state::screen_vblank));
	screen.set_palette("tc0091lvc:palette");

	TC0091LVC(config, m_vdp, 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ym2203_device &opn(YM2203(config, "opn", 8_MHz_XTAL / 2)); // divider not verified
	//opn.port_a_read_callback().set_ioport("IN0");
	//opn.port_b_read_callback().set_ioport("IN1");
	opn.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin not verified
}


ROM_START( sbmjb ) // all labels were peeled off / unreadable
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mprog.ic12", 0x00000, 0x10000, CRC(d11f14eb) SHA1(29c4b8e3ebb9ff3c5630c7bb3c8224a2f57e8fe8) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sndprog.ic5", 0x00000, 0x10000, CRC(e41575f1) SHA1(66ab1b81a618fe5e676b4f8768a5bee8d189de58) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "tc0091lvc", 0 )
	ROM_LOAD( "videoprg.ic52", 0x00000, 0x80000, CRC(21ebc096) SHA1(12f0ad4530560782a5b7517557d68526a51091e1) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "tc0091lvc:gfx", 0 )
	ROM_LOAD16_BYTE( "chr-l.ic49", 0x00000, 0x80000, CRC(54237760) SHA1(d183d4c3df3222e94219356a39ee5566ebdd54cf) )
	ROM_LOAD16_BYTE( "chr-h.ic48", 0x00001, 0x80000, CRC(0a815d7c) SHA1(5c2f65ff1f84979993f7c0df8d91ed2cd6f0acd1) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "adpcm.ic3", 0x00000, 0x40000, CRC(69158cf3) SHA1(f638bd7b5ef9e400eaa55eb3b4546e881e955087) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "e41-02.ic51", 0x000, 0x117, CRC(67fd54e0) SHA1(f64fb33b9a4a935af5662b5103709131727c8411) )
	ROM_LOAD( "e41-03.ic24", 0x200, 0x117, CRC(d906c8ea) SHA1(eae9c9c25b4affe4baf7ba034c61670d24f5c4d1) )
ROM_END

GAME( 1998, sbmjb, 0, sbmjb, sbmjb, sbmjb_state, empty_init, ROT0, "Taito", "Sonic Blast Man's Janken Battle", MACHINE_IS_SKELETON )
