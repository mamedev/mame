// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    mpc3000.cpp - Akai / Roger Linn MPC-3000 music workstation
    Skeleton by R. Belmont

    Hardware:
        CPU: NEC V53 (33 MHz?)
             8086-compatible CPU
             8237-compatible DMA controller
             8254-compatible timer
             8259-compatible IRQ controller
        Floppy: uPD72069
        SCSI: MB89352
        LCD: LC7981
        Quad-UART: TE7774
        Panel controller CPU: NEC uPD7810 @ 12 MHz
        Sound DSP: L7A1045-L6048
            DSP's wavedata bus is 16 bits wide and has 24 address bits

        DMA channel 0 is SCSI, 1 is floppy, 2 is IC31 (some sort of direct-audio stream?), and 3 is the L7A1045 DSP
        IRQ 3 is wire-OR of the 72069 FDC and 89352 SCSI
        IRQ 4 is wire-OR of all 4 TXRDYs on the TE7774
        IRQ 5 is wire-OR of RXRDY1, 2, and 3 on the TE7774
        IRQ 6 is the SMPTE sync in
        IRQ 7 is RXRDY4 on the TE7774

        TE7774 hookups: RXD1 is MIDI IN 1, RXD2 is MIDI IN 2, RXD3 and 4 are wire-ORed to the uPD7810's TX line.
                        TXD1-4 are MIDI OUTs 1, 2, 3, and 4.

    MPC2000XL & MPC2000 Classic: same as 3000, except:
        Dual UART: MB89371A (V53's i8251 is used for panel comms with the 78C10)

    MPC2000 Classic:
        CPU: NEC V53
        Floppy: uPD72068
        SCSI: MB89352
        LCD:
        Dual UART: MB89371A
        (V53's 8251 is used for panel comms here)
        Panel controller CPU: NEC uPD7810 @ 12 MHz
        Sound DSP: L6048

    MPC1000:
        CPU, LCD, UART, panel controller, DSP: SH-3 7712 (HD6417712)

    MPC500:
        CPU, LCD, UART, panel controller, DSP: SH-3 7727 (HD6417727) @ 100 MHz

    MPC2500:
        CPU, LCD, UART, panel controller, DSP: SH-3 7727 (HD6417727) @ 160 MHz

***************************************************************************/

#include "emu.h"
#include "cpu/nec/v53.h"
#include "sound/l7a1045_l6028_dsp_a.h"
#include "bus/midi/midi.h"
#include "speaker.h"
#include "screen.h"

class mpc3000_state : public driver_device
{
public:
	mpc3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
		, m_mdout(*this, "mdout")
	{ }

	void mpc3000(machine_config &config);

	void init_mpc3000();

private:
	required_device<v53_base_device> m_maincpu;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<midi_port_device> m_mdout;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void mpc3000_map(address_map &map);
	void mpc3000_io_map(address_map &map);
};

void mpc3000_state::machine_start()
{
}

void mpc3000_state::machine_reset()
{
}

void mpc3000_state::mpc3000_map(address_map &map)
{
	map(0x000000, 0x07ffff).mirror(0x80000).rom().region("maincpu", 0);
	map(0x300000, 0x3fffff).ram();
	map(0x500000, 0x500fff).ram(); // actually 8-bit battery-backed RAM
}

void mpc3000_state::mpc3000_io_map(address_map &map)
{
}

void mpc3000_state::mpc3000(machine_config &config)
{
	V53A(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpc3000_state::mpc3000_map);
	m_maincpu->set_addrmap(AS_IO, &mpc3000_state::mpc3000_io_map);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	//mdin.rxd_handler().set(m_maincpu, FUNC());

	midiout_slot(MIDI_PORT(config, "mdout"));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	L7A1045(config, m_dsp, 16_MHz_XTAL);
	m_dsp->add_route(0, "lspeaker", 1.0);
	m_dsp->add_route(1, "rspeaker", 1.0);
}

static INPUT_PORTS_START( mpc3000 )
INPUT_PORTS_END

ROM_START( mpc3000 )
	ROM_REGION(0x80000, "maincpu", 0)   // V53 code
	ROM_LOAD16_BYTE( "mpc312ls.bin", 0x000000, 0x040000, CRC(d4fb6439) SHA1(555d388ed25f8b85638c325e7d9012eaa271ffa0) )
	ROM_LOAD16_BYTE( "mpc312ms.bin", 0x000001, 0x040000, CRC(80ee0ab9) SHA1(b8855118d59b8f73a3af5ff2e824cdaa0a9f564a) )

	ROM_REGION(0x80000, "subcpu", 0)    // uPD78C10 panel controller code
	ROM_LOAD( "mp3000__op_v1.0.am27c256__id0110.ic602.bin", 0x000000, 0x008000, CRC(b0b783d3) SHA1(a60016184fc07ba00dcc19ba4da60e78aceff63c) )

	ROM_REGION( 0x2000000, "dsp", ROMREGION_ERASE00 )   // sample RAM
ROM_END

void mpc3000_state::init_mpc3000()
{
}

CONS( 1994, mpc3000, 0, 0, mpc3000, mpc3000, mpc3000_state, init_mpc3000, "Akai / Roger Linn", "MPC-3000", MACHINE_IMPERFECT_SOUND )
