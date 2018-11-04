// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************************

    Maygay M1 A/B driver, (under heavy construction !!!)

    This only loads the basic stuff - there needs to be more done to make this run.

    The sound ROM + OKIM6376 is on the game plug-in board, so not all games have it
    (although in some cases it is just missing)


    Gladiators
    ----------

    Produttore
        MayGay

    N.revisione
        M1A

    CPU
        on main board:
            1x TMP82C79P-2
            1x S22EA-EF68B21P
            1x EP840034.A-P-80C51AVW
            1x MC68681P
            1x S22EB-EF68B09P
            1x YM2149F
            2x oscillator 8.000MHz
            1x oscillator 12.000MHz

        on piggyback (SA5-029D):
            1x OKIM6376

    ROMs
        on main board:
            1x GAL16V8

        on piggyback (SA5-029D):
            1x AM27C512
            2x M27C4001
            1x GAL16V8

    Note
        on main board:
            1x 26 pins dual line connector (serial pot?)
            1x 2 legs connector (speaker)
            2x 15 legs connector (coin mech, switch matrix)
            3x 10 legs connector (meters, reel index, triacs)
            1x 11 legs connector (spare stepper motors)
            1x 20 legs connector (stepper motors)
            1x 19 legs connector (aux display)
            1x 9 legs connector (lamps)
            1x 17 legs connector (P3)
            1x 24 legs connector (lamps)
            1x 14 legs connector (power supply)
            1x 8 legs connector (control port)
            1x trimmer (volume)
            1x battery (2.4V 100mAh)
            9x red leds
            1x pushbutton
            2x 8 switches dip

        on piggyback (SA5-029D):
            1x 5 legs connector
            3x trimmer


        TODO: I/O is generally a nightmare, probably needs a rebuild at the address level.
              Inputs need a sort out.
              Some games require dongles for security, need to figure this out.
******************************************************************************************/
#include "emu.h"
#include "includes/maygay1b.h"
#include "machine/74259.h"
#include "speaker.h"

#include "maygay1b.lh"

#include "m1albsqp.lh"
#include "m1apollo2.lh"
#include "m1bargnc.lh"
#include "m1bghou.lh"
#include "m1bigdel.lh"
#include "m1calypsa.lh"
#include "m1casclb.lh"
#include "m1casroy1.lh"
#include "m1chain.lh"
#include "m1cik51o.lh"
#include "m1clbfvr.lh"
#include "m1cluecb1.lh"
#include "m1cluedo4.lh"
#include "m1cluessf.lh"
#include "m1coro21n.lh"
#include "m1cororrk.lh"
#include "m1dkong91n.lh"
#include "m1dxmono51o.lh"
#include "m1eastndl.lh"
#include "m1eastqv3.lh"
#include "m1fantfbb.lh"
#include "m1fightb.lh"
#include "m1frexplc.lh"
#include "m1gladg.lh"
#include "m1grescb.lh"
#include "m1guvnor.lh"
#include "m1hotpoth.lh"
#include "m1htclb.lh"
#include "m1imclb.lh"
#include "m1infern.lh"
#include "m1inwinc.lh"
#include "m1itjobc.lh"
#include "m1itskob.lh"
#include "m1jpmult.lh"
#include "m1lucknon.lh"
#include "m1luxorb.lh"
#include "m1manhat.lh"
#include "m1monclb.lh"
#include "m1mongam.lh"
#include "m1monmon.lh"
#include "m1monou.lh"
#include "m1nhp.lh"
#include "m1nudbnke.lh"
#include "m1omega.lh"
#include "m1onbusa.lh"
#include "m1pinkpc.lh"
#include "m1przeeb.lh"
#include "m1retpp.lh"
#include "m1search.lh"
#include "m1sptlgtc.lh"
#include "m1startr.lh"
#include "m1sudnima.lh"
#include "m1taknot.lh"
#include "m1thatlfc.lh"
#include "m1topstr.lh"
#include "m1triviax.lh"
#include "m1trtr.lh"
#include "m1ttcash.lh"
#include "m1wldzner.lh"
#include "m1wotwa.lh"


// not yet working
//#define USE_MCU

///////////////////////////////////////////////////////////////////////////
// called if board is reset ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void maygay1b_state::machine_reset()
{
	m_vfd->reset(); // reset display1
	m_Vmm=false;
}

///////////////////////////////////////////////////////////////////////////

/* 6809 IRQ handler */
void maygay1b_state::cpu0_firq(int data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, data ? ASSERT_LINE : CLEAR_LINE);
}


// IRQ from Duart (hopper?)
WRITE_LINE_MEMBER(maygay1b_state::duart_irq_handler)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE,  state?ASSERT_LINE:CLEAR_LINE);
}

// FIRQ, related to the sample playback?
READ8_MEMBER( maygay1b_state::m1_firq_trg_r )
{
	if (m_msm6376)
	{
		int nar = m_msm6376->nar_r();
		if (nar)
		{
			cpu0_firq(1);
		}
	}
	return 0xff;
}

READ8_MEMBER( maygay1b_state::m1_firq_clr_r )
{
	cpu0_firq(0);
	return 0xff;
}

// NMI is periodic? or triggered by a write?
TIMER_DEVICE_CALLBACK_MEMBER( maygay1b_state::maygay1b_nmitimer_callback )
{
	m_Vmm = !m_Vmm;
	cpu0_nmi();
}

void maygay1b_state::cpu0_nmi()
{
	if (m_Vmm && m_NMIENABLE)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

/***************************************************************************
    6821 PIA
***************************************************************************/

// some games might differ..
WRITE8_MEMBER(maygay1b_state::m1_pia_porta_w)
{
	m_vfd->por(data & 0x40);
	m_vfd->data(data & 0x10);
	m_vfd->sclk(data & 0x20);
}

WRITE8_MEMBER(maygay1b_state::m1_pia_portb_w)
{
	int i;
	for (i=0; i<8; i++)
	{
		if ( data & (1 << i) )
		{
			output().set_indexed_value("triac", i, data & (1 << i));
		}
	}
}

// input ports for M1 board ////////////////////////////////////////

INPUT_PORTS_START( maygay_m1 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "SW101" ) PORT_DIPLOCATION("SW1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "SW102" ) PORT_DIPLOCATION("SW1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "SW103" ) PORT_DIPLOCATION("SW1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "SW104" ) PORT_DIPLOCATION("SW1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "SW105" ) PORT_DIPLOCATION("SW1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "SW106" ) PORT_DIPLOCATION("SW1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "SW107" ) PORT_DIPLOCATION("SW1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "AntiFraud Protection" ) PORT_DIPLOCATION("SW1:08")
	PORT_DIPSETTING(    0x80, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x00, "SW201" ) PORT_DIPLOCATION("SW2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "SW202" ) PORT_DIPLOCATION("SW2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "SW203" ) PORT_DIPLOCATION("SW2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "SW204" ) PORT_DIPLOCATION("SW2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "SW205" ) PORT_DIPLOCATION("SW2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "SW206" ) PORT_DIPLOCATION("SW2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "SW207" ) PORT_DIPLOCATION("SW2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "SW208" ) PORT_DIPLOCATION("SW2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("24")

	PORT_START("STROBE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("25")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hi")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lo")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("28")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("29")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("30")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Rear Door") PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("STROBE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hi2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)//50p Tube
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)//100p Tube rear
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SPECIAL)//100p Tube front
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("49")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("50")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Cancel")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hold 2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Hold 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Hold 4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_START("STROBE6")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("58")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("59")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("60")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("61")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("62")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("63")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("64")

	PORT_START("STROBE7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("65")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("66")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("67")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("68")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("69")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("70")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("RESET")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("73")

INPUT_PORTS_END

void maygay1b_state::machine_start()
{
}
WRITE8_MEMBER(maygay1b_state::reel12_w)
{
	m_reel0->update( data     & 0x0F);
	m_reel1->update((data>>4) & 0x0F);

	awp_draw_reel(machine(),"reel1", *m_reel0);
	awp_draw_reel(machine(),"reel2", *m_reel1);
}

WRITE8_MEMBER(maygay1b_state::reel34_w)
{
	m_reel2->update( data     & 0x0F);
	m_reel3->update((data>>4) & 0x0F);

	awp_draw_reel(machine(),"reel3", *m_reel2);
	awp_draw_reel(machine(),"reel4", *m_reel3);
}

WRITE8_MEMBER(maygay1b_state::reel56_w)
{
	m_reel4->update( data     & 0x0F);
	m_reel5->update((data>>4) & 0x0F);

	awp_draw_reel(machine(),"reel5", *m_reel4);
	awp_draw_reel(machine(),"reel6", *m_reel5);
}

READ8_MEMBER(maygay1b_state::m1_duart_r)
{
	return ~(m_optic_pattern);
}

WRITE8_MEMBER(maygay1b_state::m1_meter_w)
{
	int i;
	for (i=0; i<8; i++)
	{
		if ( data & (1 << i) )
		{
			m_meters->update(i, data & (1 << i) );
			m_meter = data;
		}
	}
}

WRITE_LINE_MEMBER(maygay1b_state::ramen_w)
{
	m_RAMEN = state;
}

WRITE_LINE_MEMBER(maygay1b_state::alarmen_w)
{
	m_ALARMEN = state;
}

WRITE_LINE_MEMBER(maygay1b_state::nmien_w)
{
	if (m_NMIENABLE == 0 && state)
	{
		m_NMIENABLE = state;
		cpu0_nmi();
	}
	m_NMIENABLE = state;
}

WRITE_LINE_MEMBER(maygay1b_state::rts_w)
{
}

WRITE_LINE_MEMBER(maygay1b_state::psurelay_w)
{
	m_PSUrelay = state;
}

WRITE_LINE_MEMBER(maygay1b_state::wdog_w)
{
	m_WDOG = state;
}

WRITE_LINE_MEMBER(maygay1b_state::srsel_w)
{
	// this is the ROM banking?
	logerror("rom bank %02x\n", state);
	m_bank1->set_entry(state);
}

WRITE8_MEMBER(maygay1b_state::latch_ch2_w)
{
	m_msm6376->write(space, 0, data&0x7f);
	m_msm6376->ch2_w(data&0x80);
}

//A strange setup this, the address lines are used to move st to the right level
READ8_MEMBER(maygay1b_state::latch_st_hi)
{
	if (m_msm6376)
	{
		m_msm6376->st_w(1);
	}
	return 0xff;
}

READ8_MEMBER(maygay1b_state::latch_st_lo)
{
	if (m_msm6376)
	{
		m_msm6376->st_w(0);
	}
	return 0xff;
}

READ8_MEMBER(maygay1b_state::m1_meter_r)
{
	//TODO: Can we just return the AY port A data?
	return m_meter;
}
WRITE8_MEMBER(maygay1b_state::m1_lockout_w)
{
	int i;
	for (i=0; i<6; i++)
	{
		machine().bookkeeping().coin_lockout_w(i, data & (1 << i) );
	}
}

ADDRESS_MAP_START(maygay1b_state::m1_memmap)
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x2000, 0x2000) AM_WRITE(reel12_w)
	AM_RANGE(0x2010, 0x2010) AM_WRITE(reel34_w)
	AM_RANGE(0x2020, 0x2020) AM_WRITE(reel56_w)

	// there is actually an 8279 and an 8051 (which I guess is the MCU?).
	AM_RANGE(0x2030, 0x2031) AM_DEVREADWRITE("i8279", i8279_device, read, write)

#ifdef USE_MCU
	//8051
	AM_RANGE(0x2040, 0x2040) AM_WRITE( main_to_mcu_0_w )
	AM_RANGE(0x2041, 0x2041) AM_WRITE( main_to_mcu_1_w )
#else
	//8051
	AM_RANGE(0x2040, 0x2041) AM_DEVREADWRITE("i8279_2", i8279_device, read, write)
//  AM_RANGE(0x2050, 0x2050)// SCAN on M1B
#endif

	AM_RANGE(0x2070, 0x207f) AM_DEVREADWRITE("duart68681", mc68681_device, read, write )

	AM_RANGE(0x2090, 0x2091) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x20B0, 0x20B0) AM_READ(m1_meter_r)

	AM_RANGE(0x20A0, 0x20A3) AM_DEVWRITE("pia", pia6821_device, write)
	AM_RANGE(0x20A0, 0x20A3) AM_DEVREAD("pia", pia6821_device, read)

	AM_RANGE(0x20C0, 0x20C7) AM_DEVWRITE("mainlatch", hc259_device, write_d0)

	AM_RANGE(0x2400, 0x2401) AM_DEVWRITE("ymsnd", ym2413_device, write)
	AM_RANGE(0x2404, 0x2405) AM_READ(latch_st_lo)
	AM_RANGE(0x2406, 0x2407) AM_READ(latch_st_hi)

	AM_RANGE(0x2410, 0x2410) AM_READ(m1_firq_clr_r)

	AM_RANGE(0x2412, 0x2412) AM_READ(m1_firq_trg_r) // firq, sample playback?

	AM_RANGE(0x2420, 0x2421) AM_WRITE(latch_ch2_w ) // oki

	AM_RANGE(0x2800, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROMBANK("bank1")    /* 64k  paged ROM (4 pages)  */

ADDRESS_MAP_END



/*************************************************
 *
 *  NEC uPD7759 handling (used as OKI replacement)
 *
 *************************************************/
READ8_MEMBER(maygay1b_state::m1_firq_nec_r)
{
	int busy = m_upd7759->busy_r();
	if (!busy)
	{
		cpu0_firq(1);
	}
	return 0xff;
}

READ8_MEMBER(maygay1b_state::nec_reset_r)
{
	m_upd7759->reset_w(0);
	m_upd7759->reset_w(1);
	return 0xff;
}

WRITE8_MEMBER(maygay1b_state::nec_bank0_w)
{
	m_upd7759->set_bank_base(0x00000);
	m_upd7759->port_w(space, 0, data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}

WRITE8_MEMBER(maygay1b_state::nec_bank1_w)
{
	m_upd7759->set_bank_base(0x20000);
	m_upd7759->port_w(space, 0, data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}

ADDRESS_MAP_START(maygay1b_state::m1_nec_memmap)
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x2000, 0x2000) AM_WRITE(reel12_w)
	AM_RANGE(0x2010, 0x2010) AM_WRITE(reel34_w)
	AM_RANGE(0x2020, 0x2020) AM_WRITE(reel56_w)

	// there is actually an 8279 and an 8051 (which I guess is the MCU?).
	AM_RANGE(0x2030, 0x2031) AM_DEVREADWRITE("i8279", i8279_device, read, write)

#ifdef USE_MCU
	//8051
	AM_RANGE(0x2040, 0x2040) AM_WRITE( main_to_mcu_0_w )
	AM_RANGE(0x2041, 0x2041) AM_WRITE( main_to_mcu_1_w )
#else
	//8051
	AM_RANGE(0x2040, 0x2041) AM_DEVREADWRITE("i8279_2", i8279_device, read, write)
//  AM_RANGE(0x2050, 0x2050)// SCAN on M1B
#endif

	AM_RANGE(0x2070, 0x207f) AM_DEVREADWRITE("duart68681", mc68681_device, read, write )

	AM_RANGE(0x2090, 0x2091) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x20B0, 0x20B0) AM_READ(m1_meter_r)

	AM_RANGE(0x20A0, 0x20A3) AM_DEVWRITE("pia", pia6821_device, write)
	AM_RANGE(0x20A0, 0x20A3) AM_DEVREAD("pia", pia6821_device, read)

	AM_RANGE(0x20C0, 0x20C7) AM_DEVWRITE("mainlatch", hc259_device, write_d0)

	AM_RANGE(0x2400, 0x2401) AM_DEVWRITE("ymsnd", ym2413_device, write)
	AM_RANGE(0x2404, 0x2405) AM_WRITE(nec_bank0_w)
	AM_RANGE(0x2406, 0x2407) AM_WRITE(nec_bank1_w)

	AM_RANGE(0x2408, 0x2409) AM_READ(nec_reset_r)

	AM_RANGE(0x240c, 0x240d) AM_READ(m1_firq_clr_r)

	AM_RANGE(0x240e, 0x240f) AM_READ(m1_firq_nec_r)

	AM_RANGE(0x2800, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROMBANK("bank1")    /* 64k  paged ROM (4 pages)  */

ADDRESS_MAP_END


/*************************************
 *
 *  8279 display/keyboard driver
 *
 *************************************/

WRITE8_MEMBER( maygay1b_state::scanlines_w )
{
	m_lamp_strobe = data;
}

WRITE8_MEMBER( maygay1b_state::lamp_data_w )
{
	//The two A/B ports are merged back into one, to make one row of 8 lamps.

	if (m_old_lamp_strobe != m_lamp_strobe)
	{
		// Because of the nature of the lamping circuit, there is an element of persistance
		// As a consequence, the lamp column data can change before the input strobe without
		// causing the relevant lamps to black out.
		for (int i = 0; i < 8; i++)
		{
			output().set_lamp_value((8*m_lamp_strobe)+i, ((data  & (1 << (i^4))) !=0));
		}

		m_old_lamp_strobe = m_lamp_strobe;
	}

}

READ8_MEMBER( maygay1b_state::kbd_r )
{
	return (m_kbd_ports[(m_lamp_strobe&0x07)^4])->read();
}

WRITE8_MEMBER( maygay1b_state::scanlines_2_w )
{
	m_lamp_strobe2 = data;
}

WRITE8_MEMBER( maygay1b_state::lamp_data_2_w )
{
	//The two A/B ports are merged back into one, to make one row of 8 lamps.

	if (m_old_lamp_strobe2 != m_lamp_strobe2)
	{
		// Because of the nature of the lamping circuit, there is an element of persistance
		// As a consequence, the lamp column data can change before the input strobe without
		// causing the relevant lamps to black out.
		for (int i = 0; i < 8; i++)
		{
			output().set_lamp_value((8*m_lamp_strobe2)+i+128, ((data  & (1 << (i^4))) !=0));
		}

		m_old_lamp_strobe2 = m_lamp_strobe2;
	}

}

// MCU hookup not yet working

WRITE8_MEMBER(maygay1b_state::main_to_mcu_0_w)
{
	// we trigger the 2nd, more complex interrupt on writes here

	m_main_to_mcu = data;
	m_mcu->set_input_line(1, HOLD_LINE);
}


WRITE8_MEMBER(maygay1b_state::main_to_mcu_1_w)
{
	// we trigger the 1st interrupt on writes here
	// the 1st interrupt (03h) is a very simple one
	// it stores the value written as long at bit 0x40
	// isn't set.
	//
	// this is used as an index, so is probably the
	// row data written with
	// [:maincpu] ':maincpu' (F2CF): unmapped program memory write to 2041 = 8x & FF   ( m1glad )
	m_main_to_mcu = data;
	m_mcu->set_input_line(0, HOLD_LINE);
}


WRITE8_MEMBER(maygay1b_state::mcu_port0_w)
{
#ifdef USE_MCU
// only during startup
//  logerror("%s: mcu_port0_w %02x\n",machine().describe_context(),data);
#endif
}

WRITE8_MEMBER(maygay1b_state::mcu_port1_w)
{
#ifdef USE_MCU
	int bit_offset;
	for (int i = 0; i < 8; i++)
	{
		if (i < 4)
		{
			bit_offset = i + 4;
		}
		else
		{
			bit_offset = i - 4;
		}
		output().set_lamp_value((8 * m_lamp_strobe) + i + 128, ((data  & (1 << bit_offset)) != 0));
	}
#endif
}

WRITE8_MEMBER(maygay1b_state::mcu_port2_w)
{
#ifdef USE_MCU
// only during startup
	logerror("%s: mcu_port2_w %02x\n",machine().describe_context(),data);
#endif
}

WRITE8_MEMBER(maygay1b_state::mcu_port3_w)
{
#ifdef USE_MCU
// only during startup
	logerror("%s: mcu_port3_w %02x\n",machine().describe_context(),data);
#endif
}

READ8_MEMBER(maygay1b_state::mcu_port0_r)
{
	uint8_t ret = m_lamp_strobe;
#ifdef USE_MCU
	// the MCU code checks to see if the input from this port is stable in
	// the main loop
	// it looks like it needs to read the strobe
//  logerror("%s: mcu_port0_r returning %02x\n", machine().describe_context(), ret);
#endif
	return ret;

}


READ8_MEMBER(maygay1b_state::mcu_port2_r)
{
	// this is read in BOTH the external interrupts
	// it seems that both the writes from the main cpu go here
	// and the MCU knows which is is based on the interrupt level
	uint8_t ret = m_main_to_mcu;
#ifdef USE_MCU
	logerror("%s: mcu_port2_r returning %02x\n", machine().describe_context(), ret);
#endif
	return ret;
}

ADDRESS_MAP_START(maygay1b_state::maygay_mcu_io)
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P0) AM_READWRITE( mcu_port0_r, mcu_port0_w )
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_WRITE( mcu_port1_w )
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_READWRITE( mcu_port2_r, mcu_port2_w )
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_WRITE( mcu_port3_w )
ADDRESS_MAP_END


// machine driver for maygay m1 board /////////////////////////////////

MACHINE_CONFIG_START(maygay1b_state::maygay_m1)

	MCFG_CPU_ADD("maincpu", MC6809, M1_MASTER_CLOCK/2) // claimed to be 4 MHz
	MCFG_CPU_PROGRAM_MAP(m1_memmap)

	MCFG_CPU_ADD("mcu", I80C51, 2000000) //  EP840034.A-P-80C51AVW
	MCFG_CPU_IO_MAP(maygay_mcu_io)


	MCFG_DEVICE_ADD("duart68681", MC68681, M1_DUART_CLOCK)
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(maygay1b_state, duart_irq_handler))
	MCFG_MC68681_INPORT_CALLBACK(READ8(maygay1b_state, m1_duart_r))

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(maygay1b_state, m1_pia_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(maygay1b_state, m1_pia_portb_w))

	MCFG_DEVICE_ADD("mainlatch", HC259, 0) // U29
	MCFG_ADDRESSABLE_LATCH_Q0_OUT_CB(WRITELINE(maygay1b_state, ramen_w))  // m_RAMEN
	MCFG_ADDRESSABLE_LATCH_Q1_OUT_CB(WRITELINE(maygay1b_state, alarmen_w)) // AlarmEn
	MCFG_ADDRESSABLE_LATCH_Q2_OUT_CB(WRITELINE(maygay1b_state, nmien_w)) // Enable
	MCFG_ADDRESSABLE_LATCH_Q3_OUT_CB(WRITELINE(maygay1b_state, rts_w)) // RTS
	MCFG_ADDRESSABLE_LATCH_Q4_OUT_CB(WRITELINE(maygay1b_state, psurelay_w)) // PSURelay
	MCFG_ADDRESSABLE_LATCH_Q5_OUT_CB(WRITELINE(maygay1b_state, wdog_w)) // WDog
	MCFG_ADDRESSABLE_LATCH_Q6_OUT_CB(WRITELINE(maygay1b_state, srsel_w)) // Srsel

	MCFG_S16LF01_ADD("vfd",0)
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("aysnd", YM2149, M1_MASTER_CLOCK)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(maygay1b_state, m1_meter_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(maygay1b_state, m1_lockout_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SOUND_ADD("ymsnd", YM2413, M1_MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SOUND_ADD("msm6376", OKIM6376, 102400) //? Seems to work well with samples, but unconfirmed
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmitimer", maygay1b_state, maygay1b_nmitimer_callback, attotime::from_hz(75)) // freq?

	MCFG_DEVICE_ADD("i8279", I8279, M1_MASTER_CLOCK/4)    // unknown clock
	MCFG_I8279_OUT_SL_CB(WRITE8(maygay1b_state, scanlines_w))   // scan SL lines
	MCFG_I8279_OUT_DISP_CB(WRITE8(maygay1b_state, lamp_data_w))     // display A&B
	MCFG_I8279_IN_RL_CB(READ8(maygay1b_state, kbd_r))           // kbd RL lines

#ifndef USE_MCU
	// on M1B there is a 2nd i8279, on M1 / M1A a 8051 handles this task!
	MCFG_DEVICE_ADD("i8279_2", I8279, M1_MASTER_CLOCK/4)        // unknown clock
	MCFG_I8279_OUT_SL_CB(WRITE8(maygay1b_state, scanlines_2_w))   // scan SL lines
	MCFG_I8279_OUT_DISP_CB(WRITE8(maygay1b_state, lamp_data_2_w))       // display A&B
#endif

	MCFG_STARPOINT_48STEP_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(maygay1b_state, reel0_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(maygay1b_state, reel1_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(maygay1b_state, reel2_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(maygay1b_state, reel3_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(maygay1b_state, reel4_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(maygay1b_state, reel5_optic_cb))

	MCFG_DEVICE_ADD("meters", METERS, 0)
	MCFG_METERS_NUMBER(8)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEFAULT_LAYOUT(layout_maygay1b)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(maygay1b_state::maygay_m1_no_oki)
	maygay_m1(config);
	MCFG_DEVICE_REMOVE("msm6376")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(maygay1b_state::maygay_m1_nec)
	maygay_m1(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(m1_nec_memmap)

	MCFG_DEVICE_REMOVE("msm6376")

	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

WRITE8_MEMBER(maygay1b_state::m1ab_no_oki_w)
{
	popmessage("write to OKI, but no OKI rom");
}

DRIVER_INIT_MEMBER(maygay1b_state,m1common)
{
	//Initialise paging for non-extended ROM space
	uint8_t *rom = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &rom[0x0e000], 0x10000);
	membank("bank1")->set_entry(0);

	// print out the rom id / header info to give us some hints
	// note this isn't always correct, alley cat has 'Calpsyo' still in the ident string?
	{
		uint8_t *cpu = memregion( "maincpu" )->base();
		int base = 0xff20;
		for (int i=0;i<14;i++)
		{
			for (int j=0;j<16;j++)
			{
				uint8_t rom = cpu[base];

				if ((rom>=0x20) && (rom<0x7f))
				{
					printf("%c", rom);
				}
				else
				{
					printf("*");
				}

				base++;
			}
			printf("\n");
		}
	}
}


DRIVER_INIT_MEMBER(maygay1b_state,m1nec)
{
	DRIVER_INIT_CALL(m1common);
}

DRIVER_INIT_MEMBER(maygay1b_state,m1)
{
	DRIVER_INIT_CALL(m1common);

	//AM_RANGE(0x2420, 0x2421) AM_WRITE(latch_ch2_w ) // oki
	// if there is no OKI region disable writes here, the rom might be missing, so alert user

	if (m_oki_region == nullptr) {
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x2420, 0x2421, write8_delegate(FUNC(maygay1b_state::m1ab_no_oki_w), this));
	}
}

#include "maygay1b.hxx"
