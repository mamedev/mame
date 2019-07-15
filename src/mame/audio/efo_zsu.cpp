// license:BSD-3-Clause
// copyright-holders:David Haywood,AJR

/***************************************************************************

   ZSU Sound Control Unit (Proyectado 21/4/86 J. Gamell)
   ZSU1 Sound Control Unit (Proyectado 12/6/86 J. Gamell)
   Cedar Magnet Sound Board

   The ZSU board is a component of the Z-Pinball hardware developed by
   E.F.O. (Electrónica Funcional Operativa) S.A. of Barcelona, Spain. Its
   sound generators are 2 AY-3-8910As and 1 OKI MSM5205, and 2 MF10s and
   1 HC4066 are used to mix their outputs. The timing circuits are rather
   intricate, using Z80-CTCs, HC74s and HC393s and various other gates to
   drive both the 5205 and the SGS HCF40105BE (equivalent to CD40105B)
   through which its samples are funneled.

   There are no available schematics for the Cedar Magnet video game
   system (also designed by E.F.O.), but its sound board is believed to be
   a close analogue of ZSU, since it includes all of the aforementioned
   devices. The main known difference is that the Cedar Magnet sound
   code and data are externally loaded into 64K of RAM (2xTMM41464P-15),
   whereas ZSU's memory map consists primarily of a bank of up to 5 27256
   EPROMs switched from two output lines of the first 8910, overlaid with
   a mere 2K of RAM.

   irq vectors

   0xe6 - from ctc0 channel 3 (vector = E0) used to drive MSM5205 through FIFO
   0xee - from ctc0 channel 3 (vector = E8) ^^
   0xf6 - drive AY (once per frame?) triggered by ctc1 channel 3 (vector = F0)
   0xff - read sound latch (triggered by write from master board; default vector set by 5K/+5 pullups on D0-D7)

***************************************************************************/

#include "emu.h"
#include "audio/efo_zsu.h"

#include "machine/clock.h"
#include "machine/input_merger.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(EFO_ZSU,            efo_zsu_device,            "efo_zsu",      "ZSU Sound Control Unit")
DEFINE_DEVICE_TYPE(EFO_ZSU1,           efo_zsu1_device,           "efo_zsu1",     "ZSU1 Sound Control Unit")
DEFINE_DEVICE_TYPE(CEDAR_MAGNET_SOUND, cedar_magnet_sound_device, "gedmag_sound", "Cedar Sound")


efo_zsu_device::efo_zsu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_ctc0(*this, "ctc0")
	, m_ctc1(*this, "ctc1")
	, m_soundlatch(*this, "soundlatch")
	, m_fifo(*this, "fifo")
	, m_adpcm(*this, "adpcm")
{
}


efo_zsu_device::efo_zsu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: efo_zsu_device(mconfig, EFO_ZSU, tag, owner, clock)
{
}


efo_zsu1_device::efo_zsu1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: efo_zsu_device(mconfig, EFO_ZSU1, tag, owner, clock)
{
}


cedar_magnet_sound_device::cedar_magnet_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: efo_zsu_device(mconfig, CEDAR_MAGNET_SOUND, tag, owner, clock)
	, cedar_magnet_board_interface(mconfig, *this, "soundcpu", "ram")
{
}


WRITE8_MEMBER(efo_zsu_device::sound_command_w)
{
	m_soundlatch->write(data);
}



void efo_zsu_device::zsu_map(address_map &map)
{
	map(0x0000, 0x6fff).rom();
	map(0x7000, 0x77ff).mirror(0x0800).ram();
	map(0x8000, 0xffff).bankr("rombank");
}

void cedar_magnet_sound_device::cedar_magnet_sound_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("ram");
}

void efo_zsu_device::zsu_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x00, 0x03).rw(m_ctc0, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x04, 0x07).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

	map(0x08, 0x08).w(FUNC(efo_zsu_device::adpcm_fifo_w));

	map(0x0c, 0x0c).w("aysnd0", FUNC(ay8910_device::address_w));
	map(0x0d, 0x0d).w("aysnd0", FUNC(ay8910_device::data_w));

	map(0x10, 0x10).w("aysnd1", FUNC(ay8910_device::address_w));
	map(0x11, 0x11).w("aysnd1", FUNC(ay8910_device::data_w));

	map(0x14, 0x14).r(m_soundlatch, FUNC(generic_latch_8_device::read));

}

WRITE8_MEMBER(efo_zsu_device::adpcm_fifo_w)
{
	// Z80 code first unpacks 8 bytes of ADPCM sample data into nibbles
	// and, upon receiving interrupt vector E6, fills FIFO at once using OTIR
	// 4-bit data is shifted out of the FIFO to the MSM5205 by another timer
	m_fifo->write(data & 0x0f); // only low nibble is used here
	m_fifo->si_w(1);
	m_fifo->si_w(0);
}

WRITE8_MEMBER(cedar_magnet_sound_device::ay0_porta_w)
{
	// unknown (not in ZSU schematic); 0x80 written on reset
}

WRITE8_MEMBER(efo_zsu_device::ay1_porta_w)
{
	m_adpcm->reset_w(data & 1);
	if (data & 1)
		m_fifo->reset();
	// D4-D6 likely used to select clock for ctc0 channel 2
	// other bits probably used to modulate analog sound output
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc0_z0_w)
{
//  printf("USED ctc0_z0_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc0_z1_w)
{
//  printf("USED  ctc0_z1_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc1_z0_w)
{
	printf("ctc1_z0_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc1_z1_w)
{
	printf("ctc1_z1_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc1_z2_w)
{
	printf("ctc1_z2_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc0_z2_w)
{
	printf("ctc0_z2_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::fifo_dor_w)
{
	// combined with a clock signal and used to drive ctc0 channel 3
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc1" },
	{ "ctc0" },
	{ nullptr }
};

TIMER_CALLBACK_MEMBER(cedar_magnet_sound_device::reset_assert_callback)
{
	cedar_magnet_board_interface::reset_assert_callback(ptr,param);
	// reset lines go to the ctc as well?
	m_ctc0->reset();
	m_ctc1->reset();
}


void efo_zsu_device::device_add_mconfig(machine_config &config)
{
	z80_device& soundcpu(Z80(config, "soundcpu", 4000000));
	soundcpu.set_addrmap(AS_PROGRAM, &efo_zsu_device::zsu_map);
	soundcpu.set_addrmap(AS_IO, &efo_zsu_device::zsu_io);
	soundcpu.set_daisy_config(daisy_chain);

	Z80CTC(config, m_ctc0, 4000000);
	m_ctc0->intr_callback().set("soundirq", FUNC(input_merger_device::in_w<0>));
	m_ctc0->zc_callback<0>().set(FUNC(efo_zsu_device::ctc0_z0_w));
	m_ctc0->zc_callback<1>().set(FUNC(efo_zsu_device::ctc0_z1_w));
	m_ctc0->zc_callback<2>().set(FUNC(efo_zsu_device::ctc0_z2_w));

	Z80CTC(config, m_ctc1, 4000000);
	m_ctc1->intr_callback().set("soundirq", FUNC(input_merger_device::in_w<1>));
	m_ctc1->zc_callback<0>().set(FUNC(efo_zsu_device::ctc1_z0_w));
	m_ctc1->zc_callback<1>().set(FUNC(efo_zsu_device::ctc1_z1_w));
	m_ctc1->zc_callback<2>().set(FUNC(efo_zsu_device::ctc1_z2_w));

#if 0 // does nothing useful now
	clock_device &ck1mhz(CLOCK(config, "ck1mhz", 4000000/4);
	ck1mhz.signal_handler().set(m_ctc1, FUNC(z80ctc_device::trg0));
	ck1mhz.signal_handler().append(m_ctc1, FUNC(z80ctc_device::trg1));
	ck1mhz.signal_handler().append(m_ctc1, FUNC(z80ctc_device::trg2));
#endif

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set("soundirq", FUNC(input_merger_device::in_w<2>));

	INPUT_MERGER_ANY_HIGH(config, "soundirq").output_handler().set_inputline("soundcpu", INPUT_LINE_IRQ0); // 74HC03 NAND gate

	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd0(AY8910(config, "aysnd0", 4000000/2));
	aysnd0.port_a_write_callback().set_membank("rombank").mask(0x03);
	aysnd0.add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &aysnd1(AY8910(config, "aysnd1", 4000000/2));
	aysnd1.port_a_write_callback().set(FUNC(efo_zsu_device::ay1_porta_w));
	aysnd1.add_route(ALL_OUTPUTS, "mono", 0.5);

	CD40105(config, m_fifo, 0);
	m_fifo->out_ready_cb().set(FUNC(efo_zsu_device::fifo_dor_w));
	m_fifo->out_cb().set(m_adpcm, FUNC(msm5205_device::data_w));

	MSM5205(config, m_adpcm, 4000000/8).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void cedar_magnet_sound_device::device_add_mconfig(machine_config &config)
{
	efo_zsu_device::device_add_mconfig(config);

	subdevice<z80_device>("soundcpu")->set_addrmap(AS_PROGRAM, &cedar_magnet_sound_device::cedar_magnet_sound_map);

	subdevice<ay8910_device>("aysnd0")->port_a_write_callback().set(FUNC(cedar_magnet_sound_device::ay0_porta_w));
}

void efo_zsu_device::device_start()
{
	memory_bank *rombank = membank("rombank");
	rombank->configure_entries(0, 4, &static_cast<u8 *>(memregion("soundcpu")->base())[0x8000], 0x8000);
	rombank->set_entry(0); // 10K/GND pulldowns on banking lines
}

void efo_zsu1_device::device_start()
{
	memory_bank *rombank = membank("rombank");
	rombank->configure_entries(0, 4, &static_cast<u8 *>(memregion("soundcpu")->base())[0x8000], 0x8000);
	rombank->set_entry(3); // 10K/+5 pullups on banking lines
}

void cedar_magnet_sound_device::device_start()
{
}
