// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  This file contains functions to emulate the sound hardware found on
  Scramble type boards.

  There are two types, one has 2 AY8910's and the other one has one of the
  AY8910's removed.  Interestingly, it appears that the one AY8910 version
  came after the 2 AY8910 one.  This is supported by the fact that in the
  one 8190 version, the filter system uses bits 6-11, while bits 0-5 are
  left unused.

***************************************************************************/

#include "emu.h"
#include "includes/scramble.h"

#include "cpu/z80/z80.h"
#include "machine/7474.h"
#include "sound/ay8910.h"
#include "speaker.h"

#define AD2083_TMS5110_CLOCK        XTAL(640'000)

/* The timer clock in Scramble which feeds the upper 4 bits of          */
/* AY-3-8910 port A is based on the same clock                          */
/* feeding the sound CPU Z80.  It is a divide by                        */
/* 5120, formed by a standard divide by 512,                            */
/* followed by a divide by 10 using a 4 bit                             */
/* bi-quinary count sequence. (See LS90 data sheet                      */
/* for an example).                                                     */
/*                                                                      */
/* Bit 4 comes from the output of the divide by 1024                    */
/*       0, 1, 0, 1, 0, 1, 0, 1, 0, 1                                   */
/* Bit 5 comes from the QC output of the LS90 producing a sequence of   */
/*       0, 0, 1, 1, 0, 0, 1, 1, 1, 0                                   */
/* Bit 6 comes from the QD output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 1, 0, 0, 0, 0, 1                                   */
/* Bit 7 comes from the QA output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 0, 1, 1, 1, 1, 1                                   */

static const int scramble_timer[10] =
{
	0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
};

READ8_MEMBER( scramble_state::scramble_portB_r )
{
	return scramble_timer[(m_audiocpu->total_cycles()/512) % 10];
}



/* The timer clock in Frogger which feeds the upper 4 bits of           */
/* AY-3-8910 port A is based on the same clock                          */
/* feeding the sound CPU Z80.  It is a divide by                        */
/* 5120, formed by a standard divide by 512,                            */
/* followed by a divide by 10 using a 4 bit                             */
/* bi-quinary count sequence. (See LS90 data sheet                      */
/* for an example).                                                     */
/*                                                                      */
/* Bit 4 comes from the output of the divide by 1024                    */
/*       0, 1, 0, 1, 0, 1, 0, 1, 0, 1                                   */
/* Bit 3 comes from the QC output of the LS90 producing a sequence of   */
/*       0, 0, 1, 1, 0, 0, 1, 1, 1, 0                                   */
/* Bit 6 comes from the QD output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 1, 0, 0, 0, 0, 1                                   */
/* Bit 7 comes from the QA output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 0, 1, 1, 1, 1, 1                                   */

static const int frogger_timer[10] =
{
	0x00, 0x10, 0x08, 0x18, 0x40, 0x90, 0x88, 0x98, 0x88, 0xd0
};

READ8_MEMBER( scramble_state::hustler_portB_r )
{
	return frogger_timer[(m_audiocpu->total_cycles()/512) % 10];
}

WRITE8_MEMBER( scramble_state::scramble_sh_irqtrigger_w )
{
	/* the complement of bit 3 is connected to the flip-flop's clock */
	m_konami_7474->clock_w((~data & 0x08) >> 3);

	/* bit 4 is sound disable */
	machine().sound().system_mute((data & 0x10) >> 4);
}

WRITE8_MEMBER( scramble_state::mrkougar_sh_irqtrigger_w )
{
	/* the complement of bit 3 is connected to the flip-flop's clock */
	m_konami_7474->clock_w((~data & 0x08) >> 3);
}

IRQ_CALLBACK_MEMBER(scramble_state::scramble_sh_irq_callback)
{
	/* interrupt acknowledge clears the flip-flop --
	   we need to pulse the CLR line because MAME's core never clears this
	   line, only asserts it */
	m_konami_7474->clear_w(0);

	m_konami_7474->clear_w(1);

	return 0xff;
}

WRITE_LINE_MEMBER(scramble_state::scramble_sh_7474_q_callback)
{
	/* the Q bar is connected to the Z80's INT line.  But since INT is complemented, */
	/* we need to complement Q bar */
	if (m_audiocpu)
		m_audiocpu->set_input_line(0, !state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(scramble_state::hotshock_sh_irqtrigger_w)
{
	m_audiocpu->set_input_line(0, ASSERT_LINE);
}

READ8_MEMBER( scramble_state::hotshock_soundlatch_r )
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return m_soundlatch->read();
}

void scramble_state::sh_init()
{
	/* PR is always 0, D is always 1 */
	m_konami_7474->d_w(1);
}


// Harem (same as scorpion)

READ8_MEMBER(scramble_state::harem_digitalker_intr_r)
{
	return m_digitalker->digitalker_0_intr_r();
}

WRITE8_MEMBER(scramble_state::harem_digitalker_control_w)
{
	m_digitalker->digitalker_0_cs_w (data & 1 ? ASSERT_LINE : CLEAR_LINE);
	m_digitalker->digitalker_0_cms_w(data & 2 ? ASSERT_LINE : CLEAR_LINE);
	m_digitalker->digitalker_0_wr_w (data & 4 ? ASSERT_LINE : CLEAR_LINE);
}


/***************************************************************************
    AD2083 TMS5110 implementation (still wrong!)

    ROMA: BIT1: Hypersoni ?
    ROMA: BIT2: Two thousand eighty three
    ROMA: BIT4: Atomic City Attack
    ROMA: BIT5: Thank you, please try again
    ROMA: BIT6: Two, one, fire
    ROMA: BIT7: Hyperjump
    ROMB: BIT1: Noise (Explosion ?)
    ROMB: BIT2: Welcome to the top 20
    ROMB: BIT4: Atomic City Attack
    ROMB: BIT5: Noise (Explosion ?)
    ROMB: BIT6: Keep going pressing
    ROMB: BIT7: You are the new champion

    The circuit consist of

    2x 2532 32Kbit roms
    2x 74LS393 quad 4bit counters to address roms
    1x 74LS174 hex-d-flipflops to latch control byte
    2x 74LS139 decoder to decode rom selects
    1x 74LS161 8bit input multiplexor to select sound bit for rom data read
    1x 74LS00  quad nand to decode signals (unknown) to latch control byte
               ==> 74LS139
    1x 74LS16  quad open collector inverters
    1x 74S288  32x8 Prom

    The prom obviously is used to provide the right timing when
    fetching data bits. This circuit should be comparable to bagman.

    Prom
    Q0      ==> NC
    Q1      ==> PDC
    Q2      ==> CTL2/CTL8 (Speak/Reset)
    Q6      ==> Reset Counters (LS393)
    Q7      ==> Trigger Logic

     only 16 bytes needed ... The original dump is bad. This
     is what is needed to get speech to work. The prom data has
     been updated and marked as BAD_DUMP. The information below
     is given for reference once another dump should surface.

     static const int prom[16] = {0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00,
                  0x02, 0x00, 0x40, 0x00, 0x04, 0x06, 0x04, 0x84 };


  ***************************************************************************/

/*
 *      Alt1 Alt2
 * 1 ==> C     B   Bit select
 * 2 ==> B     C   Bit select
 * 3 ==> A     A   Bit select
 * 4 ==> B1        Rom select
 * 5 ==> A1        Rom select
 *
 *      ALT1      ALT2
 * 321  CBA       CBA
 * 000  000       000
 * 001  100       010
 * 010  010       100
 * 011  110       110
 * 100  001       001
 * 101  101       011
 * 110  011       101
 * 111  111       111
 *
 * Alt1 provides more sensible sound. Both Alt1 and Alt2 are
 * possible on PCB so Alt2 is left in for documentation.
 *
 */

WRITE8_MEMBER( scramble_state::ad2083_tms5110_ctrl_w )
{
	static const int tbl[8] = {0,4,2,6,1,5,3,7};

	m_tmsprom->bit_w(space, 0, tbl[data & 0x07]);
	switch (data>>3)
	{
		case 0x01:
			m_tmsprom->rom_csq_w(space, 1, 0);
			break;
		case 0x03:
			m_tmsprom->rom_csq_w(space, 0, 0);
			break;
		case 0x00:
			/* Rom 2 select */
			logerror("Rom 2 select\n");
			break;
		case 0x02:
			logerror("Rom 3 select .. \n");
			/* Rom 3 select */
			break;
	}
	/* most likely triggered by write access */
	m_tmsprom->enable_w(0);
	m_tmsprom->enable_w(1);
}

void scramble_state::ad2083_sound_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x8000, 0x83ff).ram();
}

void scramble_state::ad2083_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).w(FUNC(scramble_state::ad2083_tms5110_ctrl_w));
	map(0x10, 0x10).w("ay1", FUNC(ay8910_device::address_w));
	map(0x20, 0x20).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x40, 0x40).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x80, 0x80).w("ay2", FUNC(ay8910_device::address_w));
}

void scramble_state::ad2083_audio(machine_config &config)
{
	Z80(config, m_audiocpu, 14318000/8);   /* 1.78975 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &scramble_state::ad2083_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &scramble_state::ad2083_sound_io_map);

	TMSPROM(config, m_tmsprom, AD2083_TMS5110_CLOCK / 2);  /* rom clock */
	m_tmsprom->set_region("5110ctrl"); /* prom memory region - sound region is automatically assigned */
	m_tmsprom->set_rom_size(0x1000);   /* individual rom_size */
	m_tmsprom->set_pdc_bit(1);         /* bit # of pdc line */
	/* virtual bit 8: constant 0, virtual bit 9:constant 1 */
	m_tmsprom->set_ctl1_bit(8);        /* bit # of ctl1 line */
	m_tmsprom->set_ctl2_bit(2);        /* bit # of ctl2 line */
	m_tmsprom->set_ctl4_bit(8);        /* bit # of ctl4 line */
	m_tmsprom->set_ctl8_bit(2);        /* bit # of ctl8 line */
	m_tmsprom->set_reset_bit(6);       /* bit # of rom reset */
	m_tmsprom->set_stop_bit(7);        /* bit # of stop */
	m_tmsprom->pdc().set("tms", FUNC(tms5110_device::pdc_w)); /* tms pdc func */
	m_tmsprom->ctl().set("tms", FUNC(tms5110_device::ctl_w)); /* tms ctl func */

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ay8910_device &ay1(AY8910(config, "ay1", 14318000/8));
	ay1.port_a_read_callback().set(FUNC(scramble_state::scramble_portB_r));
	ay1.add_route(ALL_OUTPUTS, "mono", 1.0);

	ay8910_device &ay2(AY8910(config, "ay2", 14318000/8));
	ay2.port_a_read_callback().set(FUNC(scramble_state::hotshock_soundlatch_r));
	ay2.add_route(ALL_OUTPUTS, "mono", 1.0);

	tms5110a_device &tms(TMS5110A(config, "tms", AD2083_TMS5110_CLOCK));
	tms.m0().set("tmsprom", FUNC(tmsprom_device::m0_w));
	tms.data().set("tmsprom", FUNC(tmsprom_device::data_r));
	tms.add_route(ALL_OUTPUTS, "mono", 1.0);
}
