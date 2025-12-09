// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************************

    Maygay M1 -/A/B driver, (under heavy construction !!!)

    This only loads the basic stuff - there needs to be more done to make this run.

    There are 3 Mainboards, original M1 uses 2x TMP82C79P's, M1A and M1B use 1x TMP82C79P and 1x 80c51 microcontroller with a 12MHz crystal

    There are 4 sound options,
    1: AY3-8930 on the mainboard.
    2: "Digital Sound Board" sound ROM + NEC D7759 on the romcard.
    3: "FM & Digital Sound Board" sound ROM + NEC D7759 & YM2413 (3.579Mhz) on the romcard.
    4: "E.S.P. Board" sound ROMS + OKI M6376, + NEC C1892
    ** later ESP boards also have a sub-board marked "RAM Protection Board"


    Gladiators (Info is wrong, according to Highwayman dump)
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
#include "maygay1b.h"

#include "machine/74259.h"
#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"

#include "maygay1b.lh"


#define M1_MASTER_CLOCK (XTAL(8'000'000))
#define M1_DUART_CLOCK  (XTAL(3'686'400))

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
void maygay1b_state::duart_irq_handler(int state)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE,  state?ASSERT_LINE:CLEAR_LINE);
}

// FIRQ, related to the sample playback?
uint8_t maygay1b_state::m1_firq_trg_r()
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

uint8_t maygay1b_state::m1_firq_clr_r()
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
void maygay1b_state::m1_pia_porta_w(uint8_t data)
{
	m_vfd->por(data & 0x40);
	m_vfd->data(data & 0x10);
	m_vfd->sclk(data & 0x20);
}

void maygay1b_state::m1_pia_portb_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
	{
		if (BIT(data, i))
			m_triacs[i] = 1;
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
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)//50p Tube
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM)//100p Tube rear
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM)//100p Tube front
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
	m_lamps.resolve();
	m_triacs.resolve();
}

void maygay1b_state::reel12_w(uint8_t data)
{
	m_reels[0]->update( data     & 0x0F);
	m_reels[1]->update((data>>4) & 0x0F);

	awp_draw_reel(machine(),"reel1", *m_reels[0]);
	awp_draw_reel(machine(),"reel2", *m_reels[1]);
}

void maygay1b_state::reel34_w(uint8_t data)
{
	m_reels[2]->update( data     & 0x0F);
	m_reels[3]->update((data>>4) & 0x0F);

	awp_draw_reel(machine(),"reel3", *m_reels[2]);
	awp_draw_reel(machine(),"reel4", *m_reels[3]);
}

void maygay1b_state::reel56_w(uint8_t data)
{
	m_reels[4]->update( data     & 0x0F);
	m_reels[5]->update((data>>4) & 0x0F);

	awp_draw_reel(machine(),"reel5", *m_reels[4]);
	awp_draw_reel(machine(),"reel6", *m_reels[5]);
}

uint8_t maygay1b_state::m1_duart_r()
{
	return ~(m_optic_pattern);
}

void maygay1b_state::m1_meter_w(uint8_t data)
{
	for (int i=0; i<8; i++)
	{
		if ( data & (1 << i) )
		{
			m_meters->update(i, data & (1 << i) );
			m_meter = data;
		}
	}
}

void maygay1b_state::ramen_w(int state)
{
	m_RAMEN = state;
}

void maygay1b_state::alarmen_w(int state)
{
	m_ALARMEN = state;
}

void maygay1b_state::nmien_w(int state)
{
	if (m_NMIENABLE == 0 && state)
	{
		m_NMIENABLE = state;
		cpu0_nmi();
	}
	m_NMIENABLE = state;
}

void maygay1b_state::rts_w(int state)
{
}

void maygay1b_state::psurelay_w(int state)
{
	m_PSUrelay = state;
}

void maygay1b_state::wdog_w(int state)
{
	m_WDOG = state;
}

void maygay1b_state::srsel_w(int state)
{
	// this is the ROM banking?
	logerror("rom bank %02x\n", state);
	m_bank1->set_entry(state);
}

void maygay1b_state::latch_ch2_w(uint8_t data)
{
	m_msm6376->write(data&0x7f);
	m_msm6376->ch2_w(data&0x80);
}

//A strange setup this, the address lines are used to move st to the right level
uint8_t maygay1b_state::latch_st_hi()
{
	if (m_msm6376)
	{
		m_msm6376->st_w(1);
	}
	return 0xff;
}

uint8_t maygay1b_state::latch_st_lo()
{
	if (m_msm6376)
	{
		m_msm6376->st_w(0);
	}
	return 0xff;
}

uint8_t maygay1b_state::m1_meter_r()
{
	//TODO: Can we just return the AY port A data?
	return m_meter;
}
void maygay1b_state::m1_lockout_w(uint8_t data)
{
	for (int i=0; i<6; i++)
	{
		machine().bookkeeping().coin_lockout_w(i, data & (1 << i) );
	}
}

void maygay1b_state::m1_memmap(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");

	map(0x2000, 0x2000).w(FUNC(maygay1b_state::reel12_w));
	map(0x2010, 0x2010).w(FUNC(maygay1b_state::reel34_w));
	map(0x2020, 0x2020).w(FUNC(maygay1b_state::reel56_w));

	// there is actually an 8279 and an 8051 (which I guess is the MCU?).
	map(0x2030, 0x2031).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));

#ifdef USE_MCU
	//8051
	map(0x2040, 0x2040).w(FUNC(maygay1b_state::main_to_mcu_0_w));
	map(0x2041, 0x2041).w(FUNC(maygay1b_state::main_to_mcu_1_w));
#else
	//8051
	map(0x2040, 0x2041).rw("i8279_2", FUNC(i8279_device::read), FUNC(i8279_device::write));
//  map(0x2050, 0x2050)// SCAN on M1B
#endif

	map(0x2070, 0x207f).rw(m_duart68681, FUNC(mc68681_device::read), FUNC(mc68681_device::write));

	map(0x2090, 0x2091).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x20B0, 0x20B0).r(FUNC(maygay1b_state::m1_meter_r));

	map(0x20A0, 0x20A3).w("pia", FUNC(pia6821_device::write));
	map(0x20A0, 0x20A3).r("pia", FUNC(pia6821_device::read));

	map(0x20C0, 0x20C7).w("mainlatch", FUNC(hc259_device::write_d0));

	map(0x2400, 0x2401).w("ymsnd", FUNC(ym2413_device::write));
	map(0x2404, 0x2405).r(FUNC(maygay1b_state::latch_st_lo));
	map(0x2406, 0x2407).r(FUNC(maygay1b_state::latch_st_hi));

	map(0x2410, 0x2410).r(FUNC(maygay1b_state::m1_firq_clr_r));

	map(0x2412, 0x2412).r(FUNC(maygay1b_state::m1_firq_trg_r)); // firq, sample playback?

	map(0x2420, 0x2421).w(FUNC(maygay1b_state::latch_ch2_w)); // oki

	map(0x2800, 0xdfff).rom();
	map(0xe000, 0xffff).bankr("bank1");    /* 64k  paged ROM (4 pages)  */

}



/*************************************************
 *
 *  NEC uPD7759 handling (used as OKI replacement)
 *
 *************************************************/
uint8_t maygay1b_state::m1_firq_nec_r()
{
	int busy = m_upd7759->busy_r();
	if (!busy)
	{
		cpu0_firq(1);
	}
	return 0xff;
}

uint8_t maygay1b_state::nec_reset_r()
{
	m_upd7759->reset_w(0);
	m_upd7759->reset_w(1);
	return 0xff;
}

void maygay1b_state::nec_bank0_w(uint8_t data)
{
	m_upd7759->set_rom_bank(0);
	m_upd7759->port_w(data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}

void maygay1b_state::nec_bank1_w(uint8_t data)
{
	m_upd7759->set_rom_bank(1);
	m_upd7759->port_w(data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}

void maygay1b_state::m1_nec_memmap(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");

	map(0x2000, 0x2000).w(FUNC(maygay1b_state::reel12_w));
	map(0x2010, 0x2010).w(FUNC(maygay1b_state::reel34_w));
	map(0x2020, 0x2020).w(FUNC(maygay1b_state::reel56_w));

	// there is actually an 8279 and an 8051 (which I guess is the MCU?).
	map(0x2030, 0x2031).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));

#ifdef USE_MCU
	//8051
	map(0x2040, 0x2040).w(FUNC(maygay1b_state::main_to_mcu_0_w));
	map(0x2041, 0x2041).w(FUNC(maygay1b_state::main_to_mcu_1_w));
#else
	//8051
	map(0x2040, 0x2041).rw("i8279_2", FUNC(i8279_device::read), FUNC(i8279_device::write));
//  map(0x2050, 0x2050)// SCAN on M1B
#endif

	map(0x2070, 0x207f).rw(m_duart68681, FUNC(mc68681_device::read), FUNC(mc68681_device::write));

	map(0x2090, 0x2091).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x20B0, 0x20B0).r(FUNC(maygay1b_state::m1_meter_r));

	map(0x20A0, 0x20A3).w("pia", FUNC(pia6821_device::write));
	map(0x20A0, 0x20A3).r("pia", FUNC(pia6821_device::read));

	map(0x20C0, 0x20C7).w("mainlatch", FUNC(hc259_device::write_d0));

	map(0x2400, 0x2401).w("ymsnd", FUNC(ym2413_device::write));
	map(0x2404, 0x2405).w(FUNC(maygay1b_state::nec_bank0_w));
	map(0x2406, 0x2407).w(FUNC(maygay1b_state::nec_bank1_w));

	map(0x2408, 0x2409).r(FUNC(maygay1b_state::nec_reset_r));

	map(0x240c, 0x240d).r(FUNC(maygay1b_state::m1_firq_clr_r));

	map(0x240e, 0x240f).r(FUNC(maygay1b_state::m1_firq_nec_r));

	map(0x2800, 0xdfff).rom();
	map(0xe000, 0xffff).bankr("bank1");    /* 64k  paged ROM (4 pages)  */

}


/*************************************
 *
 *  8279 display/keyboard driver
 *
 *************************************/

void maygay1b_state::scanlines_w(uint8_t data)
{
	m_lamp_strobe = data;
}

void maygay1b_state::lamp_data_w(uint8_t data)
{
	//The two A/B ports are merged back into one, to make one row of 8 lamps.

	if (m_old_lamp_strobe != m_lamp_strobe)
	{
		// Because of the nature of the lamping circuit, there is an element of persistance
		// As a consequence, the lamp column data can change before the input strobe without
		// causing the relevant lamps to black out.
		for (int i = 0; i < 8; i++)
			m_lamps[((m_lamp_strobe << 3) & 0x78) | i] = BIT(data, i ^ 4);

		m_old_lamp_strobe = m_lamp_strobe;
	}

}

uint8_t maygay1b_state::kbd_r()
{
	return (m_kbd_ports[(m_lamp_strobe&0x07)^4])->read();
}

void maygay1b_state::scanlines_2_w(uint8_t data)
{
	m_lamp_strobe2 = data;
}

void maygay1b_state::lamp_data_2_w(uint8_t data)
{
	//The two A/B ports are merged back into one, to make one row of 8 lamps.

	if (m_old_lamp_strobe2 != m_lamp_strobe2)
	{
		// Because of the nature of the lamping circuit, there is an element of persistance
		// As a consequence, the lamp column data can change before the input strobe without
		// causing the relevant lamps to black out.
		for (int i = 0; i < 8; i++)
			m_lamps[((m_lamp_strobe2 << 3) & 0x78) | i | 0x80] = BIT(data, i ^ 4);

		m_old_lamp_strobe2 = m_lamp_strobe2;
	}

}

// MCU hookup not yet working

void maygay1b_state::main_to_mcu_0_w(uint8_t data)
{
	// we trigger the 2nd, more complex interrupt on writes here

	m_main_to_mcu = data;
	m_mcu->set_input_line(1, HOLD_LINE);
}


void maygay1b_state::main_to_mcu_1_w(uint8_t data)
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


void maygay1b_state::mcu_port0_w(uint8_t data)
{
#ifdef USE_MCU
// only during startup
//  logerror("%s: mcu_port0_w %02x\n",machine().describe_context(),data);
#endif
}

void maygay1b_state::mcu_port1_w(uint8_t data)
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
		m_lamps[((m_lamp_strobe << 3) & 0x78) | i | 128] = BIT(data, bit_offset);
	}
#endif
}

void maygay1b_state::mcu_port2_w(uint8_t data)
{
#ifdef USE_MCU
// only during startup
	logerror("%s: mcu_port2_w %02x\n",machine().describe_context(),data);
#endif
}

void maygay1b_state::mcu_port3_w(uint8_t data)
{
#ifdef USE_MCU
// only during startup
	logerror("%s: mcu_port3_w %02x\n",machine().describe_context(),data);
#endif
}

uint8_t maygay1b_state::mcu_port0_r()
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


uint8_t maygay1b_state::mcu_port2_r()
{
	// this is read in BOTH the external interrupts
	// it seems that both the writes from the main cpu go here
	// and the MCU knows which it is based on the interrupt level
	uint8_t ret = m_main_to_mcu;
#ifdef USE_MCU
	logerror("%s: mcu_port2_r returning %02x\n", machine().describe_context(), ret);
#endif
	return ret;
}


// machine driver for maygay m1 board /////////////////////////////////

void maygay1b_state::maygay_m1(machine_config &config)
{
	MC6809(config, m_maincpu, M1_MASTER_CLOCK/2); // claimed to be 4 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &maygay1b_state::m1_memmap);

	I80C51(config, m_mcu, 2000000); //  EP840034.A-P-80C51AVW
	m_mcu->port_in_cb<0>().set(FUNC(maygay1b_state::mcu_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(maygay1b_state::mcu_port0_w));
	m_mcu->port_out_cb<1>().set(FUNC(maygay1b_state::mcu_port1_w));
	m_mcu->port_in_cb<2>().set(FUNC(maygay1b_state::mcu_port2_r));
	m_mcu->port_out_cb<2>().set(FUNC(maygay1b_state::mcu_port2_w));
	m_mcu->port_out_cb<3>().set(FUNC(maygay1b_state::mcu_port3_w));

	MC68681(config, m_duart68681, M1_DUART_CLOCK);
	m_duart68681->irq_cb().set(FUNC(maygay1b_state::duart_irq_handler));
	m_duart68681->inport_cb().set(FUNC(maygay1b_state::m1_duart_r));

	pia6821_device &pia(PIA6821(config, "pia"));
	pia.writepa_handler().set(FUNC(maygay1b_state::m1_pia_porta_w));
	pia.writepb_handler().set(FUNC(maygay1b_state::m1_pia_portb_w));

	hc259_device &mainlatch(HC259(config, "mainlatch")); // U29
	mainlatch.q_out_cb<0>().set(FUNC(maygay1b_state::ramen_w));     // m_RAMEN
	mainlatch.q_out_cb<1>().set(FUNC(maygay1b_state::alarmen_w));   // AlarmEn
	mainlatch.q_out_cb<2>().set(FUNC(maygay1b_state::nmien_w));     // Enable
	mainlatch.q_out_cb<3>().set(FUNC(maygay1b_state::rts_w));       // RTS
	mainlatch.q_out_cb<4>().set(FUNC(maygay1b_state::psurelay_w));  // PSURelay
	mainlatch.q_out_cb<5>().set(FUNC(maygay1b_state::wdog_w));      // WDog
	mainlatch.q_out_cb<6>().set(FUNC(maygay1b_state::srsel_w));     // Srsel

	S16LF01(config, m_vfd);
	SPEAKER(config, "speaker", 2).front();
	YM2149(config, m_ay, M1_MASTER_CLOCK);
	m_ay->port_a_write_callback().set(FUNC(maygay1b_state::m1_meter_w));
	m_ay->port_b_write_callback().set(FUNC(maygay1b_state::m1_lockout_w));
	m_ay->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_ay->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	ym2413_device &ymsnd(YM2413(config, "ymsnd", M1_MASTER_CLOCK/4));
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	OKIM6376(config, m_msm6376, 102400); //? Seems to work well with samples, but unconfirmed
	m_msm6376->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_msm6376->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	TIMER(config, "nmitimer").configure_periodic(FUNC(maygay1b_state::maygay1b_nmitimer_callback), attotime::from_hz(75)); // freq?

	i8279_device &kbdc(I8279(config, "i8279", M1_MASTER_CLOCK/4));      // unknown clock
	kbdc.out_sl_callback().set(FUNC(maygay1b_state::scanlines_w));      // scan SL lines
	kbdc.out_disp_callback().set(FUNC(maygay1b_state::lamp_data_w));    // display A&B
	kbdc.in_rl_callback().set(FUNC(maygay1b_state::kbd_r));             // kbd RL lines

#ifndef USE_MCU
	// on M1B there is a 2nd i8279, on M1 / M1A a 8051 handles this task!
	i8279_device &kbdc2(I8279(config, "i8279_2", M1_MASTER_CLOCK/4));   // unknown clock
	kbdc2.out_sl_callback().set(FUNC(maygay1b_state::scanlines_2_w));   // scan SL lines
	kbdc2.out_disp_callback().set(FUNC(maygay1b_state::lamp_data_2_w)); // display A&B
#endif

	REEL(config, m_reels[0], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reels[0]->optic_handler().set(FUNC(maygay1b_state::reel_optic_cb<0>));
	REEL(config, m_reels[1], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reels[1]->optic_handler().set(FUNC(maygay1b_state::reel_optic_cb<1>));
	REEL(config, m_reels[2], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reels[2]->optic_handler().set(FUNC(maygay1b_state::reel_optic_cb<2>));
	REEL(config, m_reels[3], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reels[3]->optic_handler().set(FUNC(maygay1b_state::reel_optic_cb<3>));
	REEL(config, m_reels[4], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reels[4]->optic_handler().set(FUNC(maygay1b_state::reel_optic_cb<4>));
	REEL(config, m_reels[5], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reels[5]->optic_handler().set(FUNC(maygay1b_state::reel_optic_cb<5>));

	METERS(config, m_meters, 0).set_number(8);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	config.set_default_layout(layout_maygay1b);
}

void maygay1b_state::maygay_m1_no_oki(machine_config &config)
{
	maygay_m1(config);
	config.device_remove("msm6376");
}

void maygay1b_state::maygay_m1_nec(machine_config &config)
{
	maygay_m1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &maygay1b_state::m1_nec_memmap);

	config.device_remove("msm6376");

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_upd7759->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

void maygay1b_state::m1ab_no_oki_w(uint8_t data)
{
	popmessage("write to OKI, but no OKI rom");
}

void maygay1b_state::init_m1common()
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


void maygay1b_state::init_m1nec()
{
	init_m1common();
}

void maygay1b_state::init_m1()
{
	init_m1common();

	//map(0x2420, 0x2421).w(FUNC(maygay1b_state::latch_ch2_w)); // oki
	// if there is no OKI region disable writes here, the rom might be missing, so alert user

	if (m_oki_region == nullptr) {
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x2420, 0x2421, write8smo_delegate(*this, FUNC(maygay1b_state::m1ab_no_oki_w)));
	}
}
