// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    akaiax80.cpp - Akai AX80 8-voice polyphonic synthesizer

    Skeleton driver by R. Belmont
    ROMs provided by Arashikage

    Hardware:
        CPU: uPD7811 with internal ROM
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

    There's a 2nd UPD7811 (IC1) to scan the keyboard. Its internal rom is undumped.
    Ports A/B/C go to the keyboard, Port D (d0-6) connects 1-to-1 to Port A of IC2
    (Main CPU). Ports F and AN are unused. PD7 goes to /INT2 of IC2, /NMI goes to PA7
    of IC2.

    Both CPUs are hardwired as MODE 1, and both share a CSA120MT crystal.

    RAM is backed up by a CR2430-T battery.

    A crystal, type HC-16, (6,554,800Hz) generates various frequencies to drive the PITs
    and the 8279. Later production used a 6,553,600Hz instead.


****************************************************************************/

#include "emu.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8279.h"

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
	{ }

	void ax80(machine_config &config);
	void ax80_map(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<upd7810_device> m_maincpu;
};

void ax80_state::machine_reset()
{
}

void ax80_state::ax80_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0); // internal ROM
	map(0x1000, 0x1003).mirror(0x000c).rw(PIT0_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC20
	map(0x1010, 0x1013).mirror(0x000c).rw(PIT1_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC21
	map(0x1020, 0x1023).mirror(0x000c).rw(PIT2_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC22
	map(0x1030, 0x1033).mirror(0x000c).rw(PIT3_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC23
	map(0x1040, 0x1043).mirror(0x000c).rw(PIT4_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC24
	map(0x1050, 0x1053).mirror(0x000c).rw(PIT5_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // IC25
	map(0x1060, 0x1060).mirror(0x000e).rw("kdc", FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));   // IC11
	map(0x1061, 0x1061).mirror(0x000e).rw("kdc", FUNC(i8279_device::status_r), FUNC(i8279_device::cmd_w));  // IC11
	map(0x1070, 0x1073).mirror(0x000c).rw(PPI1_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));   // IC10
	//AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x0dfe) AM_DEVREADWRITE(PPI0_TAG, i8255_device, read, write)   // IC9 - A9 connects to A1-pin
	//AM_RANGE(0x2200, 0x2201) AM_MIRROR(0x0dfe) AM_DEVREADWRITE(PPI0_TAG, i8255_device, read, write)   // IC9 - A9 connects to A1-pin
	//AM_RANGE(0x3000, 0x3fff) // steers audio to the various voice channels
	map(0x4000, 0x5fff).mirror(0x2000).rom().region("maincpu", 0x1000);    // external program EPROM
	map(0x8000, 0x87ff).mirror(0x3800).ram();
	map(0xc000, 0xc7ff).mirror(0x3800).ram();
}

MACHINE_CONFIG_START(ax80_state::ax80)
	MCFG_DEVICE_ADD("maincpu", UPD7810, XTAL(12'000'000))
	MCFG_DEVICE_PROGRAM_MAP(ax80_map)
	//MCFG_DEVICE_IO_MAP(ax80_io)

	MCFG_DEVICE_ADD(PIT0_TAG, PIT8253, 0)
	MCFG_DEVICE_ADD(PIT1_TAG, PIT8253, 0)
	MCFG_DEVICE_ADD(PIT2_TAG, PIT8253, 0)
	MCFG_DEVICE_ADD(PIT3_TAG, PIT8253, 0)
	MCFG_DEVICE_ADD(PIT4_TAG, PIT8253, 0)
	MCFG_DEVICE_ADD(PIT5_TAG, PIT8253, 0)

	MCFG_DEVICE_ADD(PPI0_TAG, I8255A, 0)
	MCFG_DEVICE_ADD(PPI1_TAG, I8255A, 0)

	MCFG_DEVICE_ADD("kdc", I8279, 6554800 / 8) // Keyboard/Display Controller
	//MCFG_I8279_OUT_IRQ_CB(INPUTLINE("maincpu", UPD7810_INTF1))   // irq
	//MCFG_I8279_OUT_SL_CB(WRITE8(*this, ax80_state, scanlines_w))          // scan SL lines
	//MCFG_I8279_OUT_DISP_CB(WRITE8(*this, ax80_state, digit_w))            // display A&B
	//MCFG_I8279_IN_RL_CB(READ8(*this, ax80_state, kbd_r))                  // kbd RL lines
	//MCFG_I8279_IN_SHIFT_CB(VCC) // not connected
	//MCFG_I8279_IN_CTRL_CB(VCC)  // not connected
MACHINE_CONFIG_END

static INPUT_PORTS_START( ax80 )
INPUT_PORTS_END

ROM_START( ax80 )
	ROM_REGION(0x3000, "maincpu", 0)
	// CPU internal mask
	ROM_LOAD( "akai ax80 main cpu mask rom.ic2", 0x000000, 0x001000, CRC(241c078f) SHA1(7f5d0d718f2d03ec446568ae440beaff0aac6bfd) )
	// external program EPROM
	ROM_SYSTEM_BIOS( 0, "k", "REV.K" )
	ROMX_LOAD( "ax-80k.ic4", 0x001000, 0x002000, CRC(a2f95ccf) SHA1(4e5f2c4c9a08ec1d38146cae786b400261a3dbb7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "l", "REV.L" )
	ROMX_LOAD( "ax-80l.ic4", 0x001000, 0x002000, CRC(bc3d21bd) SHA1(d6730ec33b28e705a0ff88946b7860fadcc37793), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "i", "REV.I" )
	ROMX_LOAD( "ax-80i.ic4", 0x001000, 0x002000, CRC(d616e435) SHA1(84820522e6a96fc29966f82e76254e54df15d7e6), ROM_BIOS(3) )
ROM_END

CONS( 1984, ax80, 0, 0, ax80, ax80, ax80_state, empty_init, "Akai", "AX80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
