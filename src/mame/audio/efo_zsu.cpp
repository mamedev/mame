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

#include "machine/input_merger.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(EFO_ZSU,            efo_zsu_device,            "efo_zsu",      "ZSU Sound Control Unit")
DEFINE_DEVICE_TYPE(EFO_ZSU1,           efo_zsu1_device,           "efo_zsu1",     "ZSU1 Sound Control Unit")
DEFINE_DEVICE_TYPE(CEDAR_MAGNET_SOUND, cedar_magnet_sound_device, "gedmag_sound", "Cedar Sound")


efo_zsu_device::efo_zsu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_ctc(*this, "ctc%u", 0U)
	, m_ctc0_ch0(*this, "ctc0:ch0")
	, m_ctc0_ch2(*this, "ctc0:ch2")
	, m_soundlatch(*this, "soundlatch")
	, m_fifo(*this, "fifo")
	, m_adpcm(*this, "adpcm")
	, m_fifo_shift_timer(nullptr)
	, m_adpcm_clock_timer(nullptr)
	, m_ctc0_ck0_restart_timer(nullptr)
	, m_ay1_porta(0)
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


void efo_zsu_device::sound_command_w(u8 data)
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

	map(0x00, 0x03).rw(m_ctc[0], FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x04, 0x07).rw(m_ctc[1], FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

	map(0x08, 0x08).w(FUNC(efo_zsu_device::adpcm_fifo_w));

	map(0x0c, 0x0c).w("aysnd0", FUNC(ay8910_device::address_w));
	map(0x0d, 0x0d).w("aysnd0", FUNC(ay8910_device::data_w));

	map(0x10, 0x10).w("aysnd1", FUNC(ay8910_device::address_w));
	map(0x11, 0x11).w("aysnd1", FUNC(ay8910_device::data_w));

	map(0x14, 0x14).r(m_soundlatch, FUNC(generic_latch_8_device::read));

}

void efo_zsu_device::adpcm_fifo_w(u8 data)
{
	// Z80 code first unpacks 8 bytes of ADPCM sample data into nibbles
	// and, upon receiving interrupt vector E6, fills FIFO at once using OTIR
	// 4-bit data is shifted out of the FIFO to the MSM5205 by another timer
	m_fifo->write(data & 0x0f); // only low nibble is used here
	m_fifo->si_w(1);
	m_fifo->si_w(0);
}

void cedar_magnet_sound_device::ay0_porta_w(u8 data)
{
	// unknown (not in ZSU schematic); 0x80 written on reset
}

void efo_zsu_device::ay1_porta_w(u8 data)
{
	m_adpcm->reset_w(data & 1);
	if (data & 1)
		m_fifo->reset();

	if ((data & 0x60) != (m_ay1_porta & 0x60))
	{
		// CK2 selection (74HC03 open collectors)
		if (BIT(data, 6))
			m_ctc0_ch2->set_unscaled_clock(8_MHz_XTAL / 8); // 1 MHz
		else if (BIT(data, 5))
			m_ctc0_ch2->set_unscaled_clock(8_MHz_XTAL / 16); // 500 KHz
		else if (BIT(data, 4))
			m_ctc0_ch2->set_unscaled_clock(8_MHz_XTAL / 32); // 250 KHz
		else
			m_ctc0_ch2->set_unscaled_clock(0);
	}

	m_ay1_porta = data;
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc0_z0_w)
{
	if (state)
	{
		// TODO: also temporarily clears CA on HC4066 filtering switch
		m_ctc0_ch0->set_unscaled_clock(0);
		m_ctc0_ck0_restart_timer->adjust(attotime::from_hz(8_MHz_XTAL / 256));
	}
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc0_z1_w)
{
//  printf("USED  ctc0_z1_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc1_z0_w)
{
//  printf("ctc1_z0_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc1_z1_w)
{
//  printf("ctc1_z1_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc1_z2_w)
{
//  printf("ctc1_z2_w %d\n", state);
}

WRITE_LINE_MEMBER(efo_zsu_device::ctc0_z2_w)
{
	if (state)
	{
		// Retrigger timers (actually HC393 ripple counters using gated outputs of HC393 prescaler)
		// N.B. This must be done first to avoid spurious rising edge on TRG3
		m_fifo_shift_timer->adjust(attotime::from_ticks(8, 8_MHz_XTAL / 4));
		m_adpcm_clock_timer->adjust(attotime::from_ticks(4, 8_MHz_XTAL / 64));

		m_fifo->so_w(0);
		m_ctc[0]->trg3(0);
		m_adpcm->vclk_w(1);
	}
}

WRITE_LINE_MEMBER(efo_zsu_device::fifo_dor_w)
{
	if (!m_fifo_shift_timer->enabled())
		m_ctc[0]->trg3(!state);
}

TIMER_CALLBACK_MEMBER(efo_zsu_device::fifo_shift)
{
	m_fifo->so_w(1);
	if (!m_fifo->dor_r())
		m_ctc[0]->trg3(1);
}

TIMER_CALLBACK_MEMBER(efo_zsu_device::adpcm_clock)
{
	m_adpcm->vclk_w(0);
}

TIMER_CALLBACK_MEMBER(efo_zsu_device::ctc0_ck0_restart)
{
	m_ctc0_ch0->set_unscaled_clock(8_MHz_XTAL / 4);
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc1" },
	{ "ctc0" },
	{ nullptr }
};

TIMER_CALLBACK_MEMBER(cedar_magnet_sound_device::reset_assert_callback)
{
	cedar_magnet_board_interface::reset_assert_callback(param);
	// reset lines go to the ctc as well?
	m_ctc[0]->reset();
	m_ctc[1]->reset();
}


void efo_zsu_device::device_add_mconfig(machine_config &config)
{
	z80_device& soundcpu(Z80(config, "soundcpu", 8_MHz_XTAL / 2));
	soundcpu.set_addrmap(AS_PROGRAM, &efo_zsu_device::zsu_map);
	soundcpu.set_addrmap(AS_IO, &efo_zsu_device::zsu_io);
	soundcpu.set_daisy_config(daisy_chain);

	Z80CTC(config, m_ctc[0], 8_MHz_XTAL / 2);
	m_ctc[0]->set_clk<0>(8_MHz_XTAL / 4); // actually gated
	m_ctc[0]->set_clk<1>(8_MHz_XTAL / 4);
	m_ctc[0]->intr_callback().set("soundirq", FUNC(input_merger_device::in_w<0>));
	m_ctc[0]->zc_callback<0>().set(FUNC(efo_zsu_device::ctc0_z0_w));
	m_ctc[0]->zc_callback<1>().set(FUNC(efo_zsu_device::ctc0_z1_w));
	m_ctc[0]->zc_callback<2>().set(FUNC(efo_zsu_device::ctc0_z2_w));

	Z80CTC(config, m_ctc[1], 8_MHz_XTAL / 2);
	m_ctc[1]->set_clk<0>(8_MHz_XTAL / 8);
	m_ctc[1]->set_clk<1>(8_MHz_XTAL / 8);
	m_ctc[1]->set_clk<2>(8_MHz_XTAL / 8);
	m_ctc[1]->intr_callback().set("soundirq", FUNC(input_merger_device::in_w<1>));
	m_ctc[1]->zc_callback<0>().set(FUNC(efo_zsu_device::ctc1_z0_w));
	m_ctc[1]->zc_callback<1>().set(FUNC(efo_zsu_device::ctc1_z1_w));
	m_ctc[1]->zc_callback<2>().set(FUNC(efo_zsu_device::ctc1_z2_w));

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set("soundirq", FUNC(input_merger_device::in_w<2>));

	INPUT_MERGER_ANY_HIGH(config, "soundirq").output_handler().set_inputline("soundcpu", INPUT_LINE_IRQ0); // 74HC03 NAND gate

	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd0(AY8910(config, "aysnd0", 8_MHz_XTAL / 4));
	aysnd0.port_a_write_callback().set_membank("rombank").mask(0x03);
	aysnd0.add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &aysnd1(AY8910(config, "aysnd1", 8_MHz_XTAL / 4));
	aysnd1.port_a_write_callback().set(FUNC(efo_zsu_device::ay1_porta_w));
	aysnd1.add_route(ALL_OUTPUTS, "mono", 0.5);

	CD40105(config, m_fifo, 0);
	m_fifo->out_ready_cb().set(FUNC(efo_zsu_device::fifo_dor_w));
	m_fifo->out_cb().set(m_adpcm, FUNC(msm5205_device::data_w));

	MSM5205(config, m_adpcm, 8_MHz_XTAL / 16);
	m_adpcm->set_prescaler_selector(msm5205_device::SEX_4B); // Pins 1, 2 & 3 all tied to Vdd
	m_adpcm->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void cedar_magnet_sound_device::device_add_mconfig(machine_config &config)
{
	efo_zsu_device::device_add_mconfig(config);

	subdevice<z80_device>("soundcpu")->set_addrmap(AS_PROGRAM, &cedar_magnet_sound_device::cedar_magnet_sound_map);

	subdevice<ay8910_device>("aysnd0")->port_a_write_callback().set(FUNC(cedar_magnet_sound_device::ay0_porta_w));

	m_adpcm->add_route(ALL_OUTPUTS, "mono", 1.00);
}

void efo_zsu_device::device_start()
{
	memory_bank *rombank = membank("rombank");
	if (rombank != nullptr)
	{
		rombank->configure_entries(0, 4, &static_cast<u8 *>(memregion("soundcpu")->base())[0x8000], 0x8000);
		membank("rombank")->set_entry(3); // 10K/+5 pullups on banking lines
	}

	m_fifo_shift_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(efo_zsu_device::fifo_shift), this));
	m_adpcm_clock_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(efo_zsu_device::adpcm_clock), this));
	m_ctc0_ck0_restart_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(efo_zsu_device::ctc0_ck0_restart), this));

	save_item(NAME(m_ay1_porta));
}
