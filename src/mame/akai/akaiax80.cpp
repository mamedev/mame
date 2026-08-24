// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    akaiax80.cpp - Akai AX80 8-voice polyphonic synthesizer

    Skeleton driver by R. Belmont
    ROMs provided by Arashikage

    Hardware:
        CPU: 2x uPD7811 with internal ROM
        Other H/W: 8253 PIT (x6), 8255 PPI (x2), 8279 keyboard/display controller
        Voices (x8):
            NJM4558D sawtooth generator (x2)
            TC4011BP wave shaper
            TC4013 sub oscillator
            CEM 3372 VCA/VCF
        Final out:
            TL082CP low-pass filter

    Service manual incl. schematics at:
    https://archive.org/download/AkaiAX80ServiceManual/Akai%20AX80%20Service%20Manual.pdf

    Both CPUs are hardwired as MODE 1, and both share a CSA120MT crystal.
    They both use the same internal ROM, but the PC5 pin selects internal or external ROM.
    (According to the service manual, the main CPU in earlier units may also use a different
    internal ROM, but the main CPU's internal ROM is almost entirely unused to begin with.)

    RAM is backed up by a CR2430-T battery.

    A crystal, type HC-16, (6,554,800Hz) generates various frequencies to drive the PITs
    and the 8279. Later production used a 6,553,600Hz instead.


****************************************************************************/

#include "emu.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/gen_latch.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8279.h"


namespace {

#define PIT0_TAG "pit0"
#define PIT1_TAG "pit1"
#define PIT2_TAG "pit2"
#define PIT3_TAG "pit3"
#define PIT4_TAG "pit4"
#define PIT5_TAG "pit5"
#define PPI0_TAG "ppi0"
#define PPI1_TAG "ppi1"

class ax80_state : public driver_device
{
public:
	ax80_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_keys(*this, "KEY%u", 0)
	{ }

	void ax80(machine_config &config);

private:
	void ax80_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void keys_w(u8 data) { m_key_sel = data; }
	u8 keys_r();

	required_device<upd7810_device> m_maincpu;
	required_device<upd7810_device> m_subcpu;

	required_ioport_array<8> m_keys;

	u8 m_key_sel;
};

void ax80_state::machine_start()
{
	m_key_sel = 0;

	save_item(NAME(m_key_sel));
}

void ax80_state::machine_reset()
{
}

void ax80_state::ax80_map(address_map &map)
{
	//map(0x0000, 0x0fff).rom().region("maincpu", 0); // internal ROM
	map(0x1000, 0x1003).mirror(0x000c).rw(PIT0_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC20
	map(0x1010, 0x1013).mirror(0x000c).rw(PIT1_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC21
	map(0x1020, 0x1023).mirror(0x000c).rw(PIT2_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC22
	map(0x1030, 0x1033).mirror(0x000c).rw(PIT3_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC23
	map(0x1040, 0x1043).mirror(0x000c).rw(PIT4_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC24
	map(0x1050, 0x1053).mirror(0x000c).rw(PIT5_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC25
	map(0x1060, 0x1061).mirror(0x000e).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));   // IC11
	map(0x1070, 0x1073).mirror(0x000c).rw(PPI1_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));   // IC10
	//map(0x2000, 0x2001).mirror(0x0dfe).rw(PPI0_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));   // IC9 - A9 connects to A1-pin
	//map(0x2200, 0x2201).mirror(0x0dfe).rw(PPI0_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));   // IC9 - A9 connects to A1-pin
	//map(0x3000, 0x3fff) // steers audio to the various voice channels
	map(0x4000, 0x5fff).mirror(0x2000).rom().region("program", 0);    // external program EPROM
	map(0x8000, 0x87ff).mirror(0x3800).ram();
	map(0xc000, 0xc7ff).mirror(0x3800).ram();
}

void ax80_state::ax80(machine_config &config)
{
	UPD7811(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ax80_state::ax80_map);
	m_maincpu->pc_in_cb().set_constant(0x00); // PC5 low = use external program

	UPD7811(config, m_subcpu, 12_MHz_XTAL);
	m_subcpu->pa_in_cb().set(FUNC(ax80_state::keys_r));
	m_subcpu->pb_in_cb().set(FUNC(ax80_state::keys_r)); // secondary contacts, used for velocity sensing
	m_subcpu->pc_out_cb().set(FUNC(ax80_state::keys_w));
	m_subcpu->pc_in_cb().set_constant(0x20); // PC5 high = use internal program

	auto &latch(GENERIC_LATCH_8(config, "latch"));
	m_maincpu->pa_in_cb().set(latch, FUNC(generic_latch_8_device::read));
	m_maincpu->pa_out_cb().set_inputline(m_subcpu, INPUT_LINE_NMI).bit(7).invert();
	m_subcpu->pd_out_cb().set(latch, FUNC(generic_latch_8_device::write));
	m_subcpu->pd_out_cb().append_inputline(m_maincpu, UPD7810_INTF2).bit(7).invert();

	PIT8253(config, PIT0_TAG);
	PIT8253(config, PIT1_TAG);
	PIT8253(config, PIT2_TAG);
	PIT8253(config, PIT3_TAG);
	PIT8253(config, PIT4_TAG);
	PIT8253(config, PIT5_TAG);

	I8255A(config, PPI0_TAG);
	I8255A(config, PPI1_TAG);

	I8279(config, "kdc", 6554800 / 8); // Keyboard/Display Controller
	//kdc.out_irq_calback().set_inputline("maincpu", UPD7810_INTF1);    // irq
	//kdc.out_sl_callback().set(FUNC(ax80_state::scanlines_w));         // scan SL lines
	//kdc.out_disp_callback().set(FUNC(ax80_state::digit_w));           // display A&B
	//kdc.in_rl_callback().set(FUNC(ax80_state::kbd_r))                 // kbd RL lines
	//kdc.in_shift_callback().set_constant(1);                          // not connected
	//kdc.in_ctrl_callback().set_constant(1);                           // not connected
}

u8 ax80_state::keys_r()
{
	u8 data = 0xff;
	for (int i = 0; i < 8; i++)
		if (!BIT(m_key_sel, i))
			data &= m_keys[i]->read();

	return data;
}

static INPUT_PORTS_START( ax80 )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G2")

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#3")

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B3")

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G4")

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#5")

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B5")

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G6")

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C7")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ROM_START( ax80 )
	ROM_REGION(0x1000, "maincpu", 0) // CPU internal mask
	ROM_LOAD( "akai ax80 main cpu mask rom.ic2", 0x0000, 0x1000, CRC(241c078f) SHA1(7f5d0d718f2d03ec446568ae440beaff0aac6bfd) )

	ROM_REGION(0x1000, "subcpu", 0)
	ROM_COPY( "maincpu", 0x0000, 0x0000, 0x1000 )

	ROM_REGION(0x2000, "program", 0) // external program EPROM
	ROM_SYSTEM_BIOS( 0, "k", "REV.K" )
	ROMX_LOAD( "ax-80k.ic4", 0x0000, 0x2000, CRC(a2f95ccf) SHA1(4e5f2c4c9a08ec1d38146cae786b400261a3dbb7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "l", "REV.L" )
	ROMX_LOAD( "ax-80l.ic4", 0x0000, 0x2000, CRC(bc3d21bd) SHA1(d6730ec33b28e705a0ff88946b7860fadcc37793), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "i", "REV.I" )
	ROMX_LOAD( "ax-80i.ic4", 0x0000, 0x2000, CRC(d616e435) SHA1(84820522e6a96fc29966f82e76254e54df15d7e6), ROM_BIOS(2) )
ROM_END

} // anonymous namespace


CONS( 1984, ax80, 0, 0, ax80, ax80, ax80_state, empty_init, "Akai", "AX80 Programmable Polyphonic Synthesizer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
