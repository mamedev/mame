// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Orla HK1000 music keyboard.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/tms7000/tms7000.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/6850acia.h"
#include "machine/i8255.h"
#include "sound/ymopl.h"


namespace {

class hk1000_state : public driver_device
{
public:
	hk1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_slotcpu(*this, "slotcpu")
	{
	}

	void hk1000(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void slot_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<cpu_device> m_slotcpu;
};


void hk1000_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("mainpcb", 0);
	map(0x2000, 0x2000).nopw(); // ?
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa003).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa300, 0xa300).nopr(); // ?
	map(0xc000, 0xc000).nopr(); // ?
}

void hk1000_state::sound_map(address_map &map)
{
	map(0xc000, 0xffff).rom().region("soundpcb", 0);
}

void hk1000_state::slot_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1001).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x2000, 0x2001).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x8000, 0xffff).rom().region("slotpcb", 0);
}


static INPUT_PORTS_START(hk1000)
INPUT_PORTS_END

void hk1000_state::hk1000(machine_config &config)
{
	UPD7810(config, m_maincpu, 12'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hk1000_state::main_map);

	I8255(config, "ppi");

	TMS7002(config, m_soundcpu, 4'000'000);
	m_soundcpu->set_addrmap(AS_PROGRAM, &hk1000_state::sound_map);

	M6502(config, m_slotcpu, 2'000'000);
	m_slotcpu->set_addrmap(AS_PROGRAM, &hk1000_state::slot_map);

	acia6850_device &acia(ACIA6850(config, "acia"));
	acia.irq_handler().set_inputline(m_slotcpu, INPUT_LINE_NMI);

	YM3812(config, "ymsnd", 4'000'000);
}


/*
Dumper's notes:

This EPROM set stems from a working specimen of the Orla HK1000 music keyboard.

layout
------
The Orla HK1000 contains many small PCBs. Except PCB3 (analog) they all contain each one eprom. From left to right I number these PCB1 to PCB5. The slot PCB is in the case top to the right.


PCB1 (FM main voice)
--------------------
Suono
HK

PCB2 (main CPU)
---------------
HK 1000
V2

PCB4 (reverb)
-------------
STD.OEM 4/2/87
ALESIS 4D49 A

PCB5 (pcm percussion)
---------------------
TIMBRI
PCM

slot PCB (accompaniment)
------------------------
AKK
HK
V2
*/

ROM_START(hk1000)
	ROM_REGION(0x8000, "mainpcb", 0)
	ROM_LOAD("hk1000 v2_s 27c256-20.bin", 0x0000, 0x8000, CRC(b5d005e9) SHA1(5b1b6e45e84494254c5937ba726ea0f15162bdc9))

	ROM_REGION(0x4000, "soundpcb", 0)
	ROM_LOAD("suono hk_ti tms 27c128-2jl.bin", 0x0000, 0x4000, CRC(02f99a30) SHA1(fec3884150a68b6d3ed29cbcb1ce84ebf2c90dd9))

	ROM_REGION(0x2000, "reverb", 0)
	ROM_LOAD("std.oem 4-2-87 alesis 4d49 a_nmc27c64q 200.bin", 0x0000, 0x2000, CRC(44319276) SHA1(b916d8e88ec28f6b49eb814c5328a35dc8ed857f))

	ROM_REGION(0x10000, "pcm", 0)
	ROM_LOAD("timbri pcm_ti tms 27c512-20jl.bin", 0x00000, 0x10000, CRC(dd573584) SHA1(1d554e11dbcbab390d3fc995976daf122908bad0))

	ROM_REGION(0x8000, "slotpcb", 0)
	ROM_LOAD("akk hk v2_ti tms 27c512-20jl.bin", 0x0000, 0x8000, CRC(74f6461b) SHA1(b2c1b44842b8825123beaad4c0e6692a57e1a9ed))
	ROM_CONTINUE(0x0000, 0x8000) // 0xxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


SYST(198?, hk1000, 0, 0, hk1000, hk1000, hk1000_state, empty_init, "Orla", "HK1000", MACHINE_IS_SKELETON)
