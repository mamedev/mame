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
	m_soundlatch->write(space, 0, data);
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

	map(0x00, 0x03).rw("ctc0", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x04, 0x07).rw("ctc1", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

	map(0x08, 0x08).w(this, FUNC(efo_zsu_device::adpcm_fifo_w));

	map(0x0c, 0x0c).w("aysnd0", FUNC(ay8910_device::address_w));
	map(0x0d, 0x0d).w("aysnd0", FUNC(ay8910_device::data_w));

	map(0x10, 0x10).w("aysnd1", FUNC(ay8910_device::address_w));
	map(0x11, 0x11).w("aysnd1", FUNC(ay8910_device::data_w));

	map(0x14, 0x14).r("soundlatch", FUNC(generic_latch_8_device::read));

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


MACHINE_CONFIG_START(efo_zsu_device::device_add_mconfig)
	MCFG_DEVICE_ADD("soundcpu", Z80, 4000000)
	MCFG_DEVICE_PROGRAM_MAP(zsu_map)
	MCFG_DEVICE_IO_MAP(zsu_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	MCFG_DEVICE_ADD("ctc0", Z80CTC, 4000000)
	MCFG_Z80CTC_INTR_CB(WRITELINE("soundirq", input_merger_device, in_w<0>))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(*this, efo_zsu_device, ctc0_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(*this, efo_zsu_device, ctc0_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(*this, efo_zsu_device, ctc0_z2_w))

	MCFG_DEVICE_ADD("ctc1", Z80CTC, 4000000)
	MCFG_Z80CTC_INTR_CB(WRITELINE("soundirq", input_merger_device, in_w<1>))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(*this, efo_zsu_device, ctc1_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(*this, efo_zsu_device, ctc1_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(*this, efo_zsu_device, ctc1_z2_w))

#if 0 // does nothing useful now
	MCFG_DEVICE_ADD("ck1mhz", CLOCK, 4000000/4)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("ctc1", z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("ctc1", z80ctc_device, trg1))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("ctc1", z80ctc_device, trg2))
#endif

	MCFG_GENERIC_LATCH_8_ADD("soundlatch")
	MCFG_GENERIC_LATCH_DATA_PENDING_CB(WRITELINE("soundirq", input_merger_device, in_w<2>))

	MCFG_INPUT_MERGER_ANY_HIGH("soundirq") // 74HC03 NAND gate
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("soundcpu", INPUT_LINE_IRQ0))

	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("aysnd0", AY8910, 4000000/2)
	MCFG_AY8910_PORT_A_WRITE_CB(MEMBANK("rombank")) MCFG_DEVCB_MASK(0x03)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_DEVICE_ADD("aysnd1", AY8910, 4000000/2)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(*this, efo_zsu_device, ay1_porta_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_DEVICE_ADD("fifo", CD40105, 0)
	MCFG_40105_DATA_OUT_READY_CB(WRITELINE(*this, efo_zsu_device, fifo_dor_w))
	MCFG_40105_DATA_OUT_CB(WRITELINE("adpcm", msm5205_device, data_w))

	MCFG_DEVICE_ADD("adpcm", MSM5205, 4000000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(cedar_magnet_sound_device::device_add_mconfig)
	efo_zsu_device::device_add_mconfig(config);

	MCFG_DEVICE_MODIFY("soundcpu")
	MCFG_DEVICE_PROGRAM_MAP(cedar_magnet_sound_map)

	MCFG_DEVICE_MODIFY("aysnd0")
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(*this, cedar_magnet_sound_device, ay0_porta_w))
MACHINE_CONFIG_END

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
