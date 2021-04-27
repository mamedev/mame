// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Yamaha PSR-60/PSR-70 PortaSound keyboards
    Preliminary driver by R. Belmont, major thanks to reverse-engineering work by JKN0

    Documentation: https://github.com/JKN0/PSR70-reverse
    More documentation: https://retroandreverse.blogspot.com/2021/01/reversing-psr-70-hardware.html
                        https://retroandreverse.blogspot.com/2021/01/reversing-psr-70-firmware.html
                        https://retroandreverse.blogspot.com/2021/01/digging-into-ym3806.html

    Service manual: https://elektrotanya.com/yamaha_psr-70_sm.pdf/download.html

    CPU: Z80 @ 6 MHz
    Sound: YM3806 "OPQ" FM @ 3.58 MHz + YM2154 "RYP" sample playback chip for drums
    Panel and keyboard I/O: 82C55A PPI and Yamaha IG14330 "DRVIF"
    MIDI I/O: HD6350 ACIA, baud rate clock is 500 kHz

    Z80 IRQ is a wire-OR of IRQs from the OPQ, RYP4, ACIA, and DRVIF

    PPI ports:
    PA0-PA5: keyboard matrix readback (PA5-PA0 for most matrix selects, where PA5 is the lowest key and PA0 is the highest)
    PA6: single-use matrix readback for C1 (when PC1 is active)
    PA7: cassette input
    Keyboard matrix select writes:
        PC1: C1 - F#1
        PC0: G1 - C2
        PB7: C#2 - F#2
        PB6: G2 - C3
        PB5: C#3 - F#3
        PB4: G3 - C4
        PB3: C#4 - F#4
        PB2: G4 - C5
        PB1: C#5 - F#5
        PB0: G5 - C6
    PC3: unused
    PC4: ROM2 bank (goes to A14)
    PC5: cassette output
    PC6: MUTE
    PC7: to "RS2" signal, IC (reset, "initial clear" in Yamaha-speak) pin on T9500
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "bus/midi/midi.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class psr60_state : public driver_device
{
public:
	psr60_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi"),
		m_acia(*this, "acia"),
		m_rom2bank(*this, "rom2bank")
	{ }

	void psr60(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<acia6850_device> m_acia;
	required_memory_bank m_rom2bank;

	void psr60_map(address_map &map);
	void psr60_io_map(address_map &map);

	void ppi_pc_w(u8 data);
	void recalc_irqs();

	int m_acia_irq;

	WRITE_LINE_MEMBER(write_acia_clock) { m_acia->write_txc(state); m_acia->write_rxc(state); }
	WRITE_LINE_MEMBER(acia_irq_w) { m_acia_irq = state; recalc_irqs(); }
};

void psr60_state::psr60_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("rom1", 0);
	map(0x8000, 0xbfff).bankr("rom2bank");
	// c000-c0ff: YM3806 "OPQ" FM chip
	map(0xe000, 0xffff).ram();  // work RAM
}

void psr60_state::psr60_io_map(address_map &map)
{
	map.global_mask(0xff);  // top 8 bits of the address are ignored by this hardware for I/O access
	map(0x10, 0x11).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x20, 0x23).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	// 30-40: IG14330 "DRVIF" (front panel LED driver/multiplexer?)
	// 80-FF: YM2154 "RYP4" (drum sample playback)
}

void psr60_state::ppi_pc_w(u8 data)
{
	m_rom2bank->set_entry(BIT(data, 4));
}

void psr60_state::recalc_irqs()
{
	int irq_state = m_acia_irq; // (| OPQ, RYP4, and DRVIF interrupts eventually)
	m_maincpu->set_input_line(0, irq_state);
}

void psr60_state::machine_start()
{
	m_rom2bank->configure_entries(0, 2, memregion("rom2")->base(), 0x4000);
	m_rom2bank->set_entry(0);
	m_acia_irq = CLEAR_LINE;
}

void psr60_state::machine_reset()
{
}

static INPUT_PORTS_START(psr60)
INPUT_PORTS_END

void psr60_state::psr60(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &psr60_state::psr60_map);
	m_maincpu->set_addrmap(AS_IO, &psr60_state::psr60_io_map);

	I8255A(config, m_ppi, 6_MHz_XTAL);
	m_ppi->out_pc_callback().set(FUNC(psr60_state::ppi_pc_w));

	ACIA6850(config, m_acia, 500_kHz_XTAL); // actually an HD6350, differences unknown (if any)
	m_acia->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(FUNC(psr60_state::acia_irq_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	clock_device &acia_clock(CLOCK(config, "acia_clock", 500_kHz_XTAL));    // 31250 * 16 = 500,000
	acia_clock.signal_handler().set(FUNC(psr60_state::write_acia_clock));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

ROM_START( psr60 )
	ROM_REGION(0x8000, "rom1", 0)
	ROM_LOAD("yamaha_psr60_pgm_ic109.bin", 0x000000, 0x008000, CRC(95604916) SHA1(811fb88fc968c58234600eb9fbf1f64e411754cd))

	ROM_REGION(0x8000, "rom2", 0)
	ROM_LOAD("yamaha_psr60_pgm_ic110.bin", 0x000000, 0x008000, CRC(39db8c74) SHA1(7750104d1e5df3357aa553ac58768dbc34051cd8))
ROM_END

ROM_START(psr70)
	ROM_REGION(0x8000, "rom1", 0)
	ROM_LOAD("psr70-rom1.bin", 0x000000, 0x008000, CRC(bf134412) SHA1(318f33f8ef5e2d865e8ae657993763c9e032af8e))

	ROM_REGION(0x8000, "rom2", 0)
	ROM_LOAD("yamaha_psr60_pgm_ic110.bin", 0x000000, 0x008000, CRC(39db8c74) SHA1(7750104d1e5df3357aa553ac58768dbc34051cd8))
ROM_END

CONS(1985, psr60, 0, 0, psr60, psr60, psr60_state, empty_init, "Yamaha", "PSR-60 PortaSound", MACHINE_NOT_WORKING)
CONS(1985, psr70, psr60, 0, psr60, psr60, psr60_state, empty_init, "Yamaha", "PSR-70 PortaSound", MACHINE_NOT_WORKING)
