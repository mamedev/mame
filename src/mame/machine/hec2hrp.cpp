// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/********************************************************************************************
Systems supported:
- Hector 2HR+
- Victor
- Hector 2HR
- Hector HRX
- Hector MX40c
- Hector MX80c
- Hector 1
- Interact

2009-05-12 Skeleton driver - Micko
2009-06-30 Preliminary video - Robbbert

2009-10-29 Update skeleton to functional machine by yo_fr (jj.stac@aliceadsl.fr)
                => add Keyboard,
                => add color,
                => add cassette,
                => add sn76477 sound and 1bit sound,
                => add joysticks (stick, pot, fire)
                => add BR/HR switching
                => add bank switch for HRX
                => add device MX80c and bank switching for the ROM
2010-01-03 Update and clean prog by yo_fr       (jj.stac @ aliceadsl.fr)
                => add the port mapping for keyboard
2010-09-28 add the DISK II support by yo_fr      (jj.stac @ aliceadsl.fr)
                => Note that the DISK II boots and loads CP/M, but CP/M doesn't yet work.
2010-11-20 : synchronization between uPD765 and Z80 is now OK, CP/M works. JJStacino
2011-11-11 : add the minidisk support (3.5" drive)  JJStacino
2012-02-19 : few adjustments for the hrp and hr machine - JJStacino

More information:
- http://dchector.free.fr/
- http://hectorvictor.free.fr/

**********************************************************************************************/

#include "emu.h"
#include "includes/hec2hrp.h"

#include "cpu/z80/z80.h"

#include "speaker.h"

#include "formats/hect_tap.h"
#include "formats/hect_dsk.h"


#ifndef DEBUG_TRACE_COM_HECTOR
//#define DEBUG_TRACE_COM_HECTOR  1
#endif


/* machine List
hec2hrp
victor
hec2hr
hec2hrx
hec2mdhrx
hec2mx80
hec2mx40
*/


void hec2hrp_state::init_mx40()
{
	m_has_disc2 = true;
	m_has_minidisc = false;
	m_is_hr = true;
	m_is_extended = true;
}

void hec2hrp_state::init_mdhrx()
{
	m_has_disc2 = false;
	m_has_minidisc = true;
	m_is_hr = true;
	m_is_extended = true;
}

void hec2hrp_state::init_victor()
{
	m_has_disc2 = false;
	m_has_minidisc = false;
	m_is_hr = true;
	m_is_extended = false;
}

void hec2hrp_state::init_hrx()
{
	m_has_disc2 = true;
	m_has_minidisc = false;
	m_is_hr = false;
	m_is_extended = true;
}

void hec2hrp_state::init_interact()
{
	m_has_disc2 = false;
	m_has_minidisc = false;
	m_is_hr = false;
	m_is_extended = false;
}


/* Cassette timer*/
TIMER_CALLBACK_MEMBER(hec2hrp_state::cassette_clock)
{
	m_ck_signal++;
}

void hec2hrp_state::minidisc_control_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	if (BIT(data, 6)) floppy = m_floppy0->get_device();
	// bit 7 = drive 2?

	m_minidisc_fdc->set_floppy(floppy);

	if (floppy)
	{
		// don't know where the motor on signal is
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 4));
	}

	m_bank[2]->set_entry(BIT(data, 5) ? HECTOR_BANK_BASE : HECTOR_BANK_DISC);
}

void hec2hrp_state::switch_bank_w(offs_t offset, uint8_t data)
{
	if (offset==0x00)
	{
		if (m_is_extended)
			m_bank[1]->set_entry(HECTOR_BANK_VIDEO);

		if (m_flag_clk)
		{
			m_flag_clk = false;
			m_maincpu->set_unscaled_clock(XTAL(5'000'000));
		}
	}

	if (offset==0x04)
	{
		m_hector_flag_hr = false;
		if (m_is_extended)
			m_bank[1]->set_entry(HECTOR_BANK_VIDEO);

		if (!m_flag_clk)
		{
			m_flag_clk = true;
			m_maincpu->set_unscaled_clock(XTAL(1'750'000));
		}
	}

	if (offset==0x08)
	{
		if (m_is_extended)
			m_bank[1]->set_entry(HECTOR_BANK_PROG);

		if (m_flag_clk)
		{
			m_flag_clk = false;
			m_maincpu->set_unscaled_clock(XTAL(5'000'000));
		}
	}

	if (offset == 0x0c)
	{
		m_hector_flag_hr = false;
		if (m_is_extended)
			m_bank[1]->set_entry(HECTOR_BANK_PROG);

		if (!m_flag_clk)
		{
			m_flag_clk = true;
			m_maincpu->set_unscaled_clock(XTAL(1'750'000));
		}
	}
}

void hec2hrp_state::keyboard_w(uint8_t data)
{
	/* nothing to do (read function manages the value) */
}

uint8_t hec2hrp_state::keyboard_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset == 7) /* Only when reading joystick */
	{
		/* Read special key for analog joystick emulation only (button and pot are analog signals), and reset */
		data=m_keyboard[8]->read();

		if (data & 0x01) /* Reset machine */
		{
			m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
			if (m_is_hr)
			{
				m_hector_flag_hr = true;
				if (m_is_extended)
				{
					m_bank[1]->set_entry(HECTOR_BANK_PROG);
					m_bank[2]->set_entry(HECTORMX_BANK_PAGE0);
				}
				//RESET DISC II unit
				if (m_has_disc2)
					hector_disc2_reset();

				/* floppy md master reset */
				if (m_has_minidisc)
					m_minidisc_fdc->reset();
			}
			else
				m_hector_flag_hr = false;

			/*Common flag*/
			m_hector_flag_80c = false;
			m_flag_clk = false;
		}

		m_actions = 0;
		if (data & 0x02) /* Fire(0)*/
			m_actions += 1;

		if (data & 0x04) /* Fire(1)*/
			m_actions += 2;

		if (data & 0x08) /* Pot(0)+*/
			m_pot0 += 1;

		if (m_pot0>128)
			m_pot0 = 128;

		if (data & 0x10) /* Pot(0)-*/
			m_pot0 -= 1;

		if (m_pot0>250)
			m_pot0 = 0;

		if (data & 0x20) /* Pot(1)+*/
			m_pot1 += 1;

		if (m_pot1>128)
			m_pot1 = 128;

		if (data & 0x40) /* Pot(1)-*/
			m_pot1 -= 1;

		if (m_pot1>250)
			m_pot1 = 0;
	}

	/* in all case return the requested value */
	return m_keyboard[offset]->read();
}

void hec2hrp_state::sn_2000_w(offs_t offset, uint8_t data)
{
	update_state(0x2000+ offset, data);
	update_sound(data);
}

void hec2hrp_state::sn_2800_w(offs_t offset, uint8_t data)
{
	update_state(0x2800+ offset, data);
	update_sound(data);
}

uint8_t hec2hrp_state::cassette_r()
{
	double level;
	uint8_t value = 0;

	if ((m_state3000 & 0x38) != 0x38 )
	{
		m_data_k7 =  0x00;  /* No cassette => clear bit*/
		switch (m_state3000 & 0x38 )
		{
			case 0x08: value = (m_actions & 1) ? 0x80 : 0; break;
			case 0x10: value = m_pot0; break;
			case 0x20: value = (m_actions & 2) ? 0x80 : 0; break;
			case 0x28: value = m_pot1; break;
			default: value = 0; break;
		}
	}
	else
	{
		if (!m_write_cassette)
		{
			level = m_cassette->input();

			if  (level < -0.08)
				m_cassette_bit = false;
			if (level > +0.08)
				m_cassette_bit = true;
		}

		if ((m_cassette_bit != m_cassette_bit_mem) && m_cassette_bit)
			m_data_k7 = m_data_k7 ? 0 : 0x80;

		value = ( m_ck_signal & 0x7F ) + m_data_k7;
		m_cassette_bit_mem = m_cassette_bit;
	}
	return value;
}
void hec2hrp_state::sn_3000_w(uint8_t data)
{
	m_state3000 = data & 0xf8; /* except bit 0 to 2*/
	data &= 7;
	if (data != m_oldstate3000 )
	{
		/* Update sn76477 only when necessary!*/
		update_state(0x3000, data);
		update_sound(data);
	}
	m_oldstate3000 = data;
}

/* Color Interface */
void hec2hrp_state::color_a_w(uint8_t data)
{
	if (data & 0x40)
	{
		/* Bit 6 => motor ON/OFF => for cassette state!*/
		if (!m_write_cassette)
		{
			m_cassette->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
			// m_cassette->set_state(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
		}
	}
	else
	{   /* stop motor*/
		m_cassette->set_state(CASSETTE_STOPPED);
		m_write_cassette = false;
		m_counter_write =0;
	}
	if (((data & 0x80) != (m_oldstate1000 & 0x80)) && ((m_oldstate1000 & 7)==(data & 7)) ) /* Bit7 had change but not the color statement*/
	{
		/* Bit 7 => Write bit for cassette!*/
		m_counter_write +=1;

		if (m_counter_write > 5)
		{
			/* Wait several cycles before launch recording to prevent somes bugs*/
			m_counter_write = 6;
			if (!m_write_cassette)
			{
				// record
				m_cassette->change_state( CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
				m_cassette->set_state(CASSETTE_RECORD);
				m_write_cassette = true;
			}
		}
		/* cassette data */
		m_cassette->output(((data & 0x80) == 0x80) ? -1.0 : +1.0);
	}

	/* Other bit : color definition*/
	m_hector_color[0] = data & 0x07 ;
	m_hector_color[2] = BIT(data, 3, 3) | (m_hector_color[2] & 0x40);

	m_oldstate1000=data; /* For next step*/
}

void hec2hrp_state::color_b_w(uint8_t data)
{
	m_hector_color[1] = data & 0x07;
	m_hector_color[3] = BIT(data, 3, 3);

	/* Half light on color 2 only on HR machines:*/
	if (data & 0x40)
		m_hector_color[2] |= 8;
	else
		m_hector_color[2] &= 7;

	/* Play bit*/
	m_discrete->write(NODE_01, (data & 0x80) ? 0:1 );
}


/********************************************************************************
 Port Handling
********************************************************************************/

/*******************  READ PIO 8255 *******************/
uint8_t hec2hrp_state::io_8255_r(offs_t offset)
{
	/* 8255 in mode 0 */
	uint8_t data =0;
	uint8_t data_l=0;
	uint8_t data_h=0;
	offset &= 3;

	if (offset == 0) /* Port A */
		data = m_hector_port_a;
	else
	if (offset == 1) /* Port B */
	{
		data = m_hector_port_b;
		#ifdef DEBUG_TRACE_COM_HECTOR
			printf("\nLecture data par Hector %x (portB)",data);
		#endif
	}
	else
	if (offset == 2) /* Port C */
	{
		data_l = (m_hector_port_c_l & 0x0f);
		data_h = (m_hector_port_c_h & 0xf0);

		if (BIT(m_hector_port_cmd, 0))                  /* Quartet inf en entree ?*/
			data_l = (m_hector_port_c_l & 0x0f);  /*no change*/

		if (BIT(m_hector_port_cmd, 3))                  /* Quartet sup en entree ?*/
		{
			m_hector_port_c_h = (m_hector_port_c_h & 0xc0);    /* Clear bits 4 & 5*/

			if (m_hector_disc2_data_w_ready != 0x00)
				m_hector_port_c_h = m_hector_port_c_h + 0x10;  // PC4 (data write ready from Disc II to Hector)

			if (m_hector_disc2_data_r_ready != 0x00)
				m_hector_port_c_h = m_hector_port_c_h + 0x20;  // PC5 (data read ready from Hector to Disc2)

			m_hector_port_c_h = m_hector_port_c_h & 0x7F;      // PC7 (printer busy=0)
			data_h =  m_hector_port_c_h;
		}
		data = data_l + data_h;
	}
	return data;  // Return the value!
}

/*******************  WRITE PIO 8255 *******************/

void hec2hrp_state::io_8255_w(offs_t offset, uint8_t data)
{
	offset &= 3;
	/* 8255 in mode 0 */
	if (offset == 0) /* Port A => to printer or Disc II*/
	{
		m_hector_port_a = data;
		/* Port A => to printer*/
		/*  Caution : The strobe connection to the printer seems not be used
		So, everything sent to the Disc2 unit will be printed too! */

		if (BIT(m_hector_port_c_l, 0)) // PC0 (bit 0) = strobe printer
			m_printer->output(m_hector_port_a);
	}
	else
	if (offset == 1) /* Port B */
		m_hector_port_b = data;
	else
	if (offset == 2) /* Port C => depending cmd word */
	{
		if (!BIT(m_hector_port_cmd, 0))
		{
			m_hector_port_c_l = data & 0x0f;
			// Utilizing bits port C : PC0 for the printer : strobe
			if (BIT(m_hector_port_c_l, 0))        // PC0 (bit 0) = true
			{
				/* Port A goes to the printer */
			}
			// Utilizing bits port C : PC1 // PC2  for the communication with disc2
			if (!BIT(m_hector_port_c_l, 1))       // PC1 (bit 1) = true
			{
				m_hector_port_b = m_hector_disc2_data_write;
				m_hector_disc2_data_w_ready = 0x00;
			}
			if (!BIT(m_hector_port_c_l, 2))     // PC2 (bit 2) = true
			{
				m_hector_disc2_data_read = m_hector_port_a;
				m_hector_disc2_data_r_ready = 0x08;
			}
		}
		if (!BIT(m_hector_port_cmd, 3))
			m_hector_port_c_h = (data & 0xf0);
	}
	else  // offset=3
		m_hector_port_cmd = data;
}
/* End of 8255 managing */


/*******************  PIO write handler for MX40 *******************/
void hec2hrp_state::mx40_io_port_w(offs_t offset, uint8_t data)
{
	offset &= 0xff;

	/* Bank switching on several address */
	if (offset == 0x40) /* Port page 0*/
		m_bank[2]->set_entry(HECTORMX_BANK_PAGE0);
	else
	if (offset == 0x41) /* Port page 1*/
	{
		m_bank[2]->set_entry(HECTORMX_BANK_PAGE1);
		m_hector_flag_80c = false;
	}
	else
	if (offset == 0x44) /* Port page 2  => 42 on MX80*/
		m_bank[2]->set_entry(HECTORMX_BANK_PAGE2);
	else
	if (offset == 0x49) /* Port screen resolution*/
		m_hector_flag_80c = false;/* No 80c in 40c !*/
}

/*******************  PIO write handlerfor MX80 *******************/
void hec2hrp_state::mx80_io_port_w(offs_t offset, uint8_t data)
{
	offset &= 0xff;

	if (offset == 0x40) /* Port page 0*/
		m_bank[2]->set_entry(HECTORMX_BANK_PAGE0);
	else
	if (offset == 0x41) /* Port page 1*/
	{
		m_bank[2]->set_entry(HECTORMX_BANK_PAGE1);
		m_hector_flag_80c = false;
	}
	else
	if (offset == 0x42) /* Port page 2  => different port on MX40 */
		m_bank[2]->set_entry(HECTORMX_BANK_PAGE2);
	else
	if (offset == 0x49) /* Port screen resolution*/
		m_hector_flag_80c = true;
}

void hec2hrp_state::video_start()
{
	save_item(NAME(m_has_disc2));
	save_item(NAME(m_has_minidisc));
	save_item(NAME(m_is_hr));
	save_item(NAME(m_is_extended));
	save_item(NAME(m_hector_flag_hr));
	save_item(NAME(m_hector_flag_80c));
	save_item(NAME(m_hector_color));
	save_item(NAME(m_hector_disc2_data_r_ready));
	save_item(NAME(m_hector_disc2_data_w_ready));
	save_item(NAME(m_hector_disc2_data_read));
	save_item(NAME(m_hector_disc2_data_write));
	save_item(NAME(m_hector_disc2_rnmi));
	save_item(NAME(m_state3000));
	save_item(NAME(m_write_cassette));
	save_item(NAME(m_ck_signal));
	save_item(NAME(m_flag_clk));
	save_item(NAME(m_pin_value));
	save_item(NAME(m_au));
	save_item(NAME(m_val_mixer));
	save_item(NAME(m_oldstate3000));
	save_item(NAME(m_oldstate1000));
	save_item(NAME(m_pot0));
	save_item(NAME(m_pot1));
	save_item(NAME(m_actions));
	save_item(NAME(m_hector_port_a));
	save_item(NAME(m_hector_port_b));
	save_item(NAME(m_hector_port_c_h));
	save_item(NAME(m_hector_port_c_l));
	save_item(NAME(m_hector_port_cmd));
	save_item(NAME(m_cassette_bit));
	save_item(NAME(m_cassette_bit_mem));
	save_item(NAME(m_data_k7));
	save_item(NAME(m_counter_write));
	save_item(NAME(m_irq_current_state));
	save_item(NAME(m_nmi_current_state));

	m_hector_flag_80c = false;
}


/********************************************************************************
 sound management
********************************************************************************/

void hec2hrp_state::update_state(int Adresse, int Value )
{
/* Adjust value depending on I/O main CPU request*/
switch(Adresse )
{
	case 0x2000:
	{
		m_au[ 0] = BIT(Value, 7);
		m_au[ 8] = BIT(Value, 6);
		break;
	}

	case 0x2001:
	{
		m_au[ 1] = BIT(Value, 7);
		m_au[ 9] = BIT(Value, 6);
		break;
	}

	case 0x2002:
	{
		m_au[ 2] = BIT(Value, 7);
		m_au[10] = BIT(Value, 6);
		break;
	}

	case 0x2003:
	{
		m_au[ 3] = BIT(Value, 7);
		m_au[11] = BIT(Value, 6);
		break;
	}

	case 0x2800:
	{
		m_au[ 4] = BIT(Value, 7);
		m_au[12] = BIT(Value, 6);
		break;
	}

	case 0x2801:
	{
		m_au[ 5] = BIT(Value, 7);
		m_au[13] = BIT(Value, 6);
		break;
	}

	case 0x2802:
	{
		m_au[ 6] = BIT(Value, 7);
		m_au[14] = BIT(Value, 6);
		break;
	}

	case 0x2803:
	{
		m_au[ 7] = BIT(Value, 7);
		m_au[15] = BIT(Value, 6);
		break;
	}

	case 0x3000:
	{
		m_val_mixer = Value & 7;
		break;
	}
	default: break;
}
}


void hec2hrp_state::init_sn76477()
{
	/* R/C value setup */

	/* decay resistors */
	m_pin_value[7][1] = RES_K(680.0); /*680K  */
	m_pin_value[7][0] = RES_K(252.325); /* 142.325 (680 // 180KOhm)*/

	/* attack/decay capacitors */
	m_pin_value[8][0] = CAP_U(0.47); /* 0.47uf*/
	m_pin_value[8][1] = CAP_U(1.47);  /* 1.47*/

	/* attack resistors */
	m_pin_value[10][1]= RES_K(180.0);   /* 180*/
	m_pin_value[10][0]= RES_K(32.054); /* 32.054 (180 // 39 KOhm)*/

	/* Version 3 : Frequency measurement adjustment:
	            // 4  0 SOUND 255 Hz => ajuste a l'oreille
	            // 4  4 SOUND  65 Hz => ajuste a l'oreille
	            // 4  8 SOUND  17 Hz =>  ajuste a l'oreille
	            // 4 12 SOUND 4,3 Hz =>  ajuste a l'oreille*/
	/*   SLF C       Version 3*/
	m_pin_value[21][0]= CAP_U(0.1);  /*CAPU(0.1) */
	m_pin_value[21][1]= CAP_U(1.1);  /*1.1*/

	/* SLF R        Version 3*/
	m_pin_value[20][1]= RES_K(180);    //180 (based on visual inspection of the resistor)
	m_pin_value[20][0]= RES_K(37.268); // 37.268 (47//180 KOhms)

	/* Capa VCO*/
	/* Version 3 : Frequency measurement adjustment:
	        // 0 0  SOUND 5,5KHz => 5,1KHz
	        // 0 16 SOUND 1,3KHz => 1,2KHz
	        // 0 32 SOUND 580Hz  => 570Hz
	        // 0 48 SOUND 132Hz  => 120Hz*/
	m_pin_value[17][0] = CAP_N(47.0) ;  /* measured */
	m_pin_value[17][1] = CAP_N(580.0) ; /* measured */
	/* R VCO   Version 3*/
	m_pin_value[18][1] = RES_K(1400.0); /* Measured 1300, instead of 1Mohm*/
	m_pin_value[18][0] = RES_K(203.548);/* Measured 223, instead of 193.548 (1000 // 240KOhm)*/

	/* VCO Controle*/
	m_pin_value[16][0] = 0.0;  /* Volts  */
	m_pin_value[16][1] = 1.41; /* 2 =  10/15th of 5V*/

	/* Pitch*/
	m_pin_value[19][0] = 0.0;   /*Volts */
	m_pin_value[19][1] = 1.41;

	m_pin_value[22][0] = 0; /* TOR */
	m_pin_value[22][1] = 1;

	/* One-shot resistor */
	m_pin_value[24][1] = RES_K(100);
	m_pin_value[24][0] = RES_K(1000);  /* infinite on Hector due to lack of connection */

	/* One-shot capacitor */
	m_pin_value[23][0] = 1.0;
	m_pin_value[23][1] = 0.0;  /* bogus value on Hector, as +5V lacks a capacitor */

	/* Enabled*/
	m_pin_value[9][0] = 0;
	m_pin_value[9][1] = 1;

	/* Volume*/
	m_pin_value[11][0] = 128;
	m_pin_value[11][1] = 255;

	/* Noise filter*/
	m_pin_value[6][0] = CAP_U(0.390);    /* 0.390*/
	m_pin_value[6][1] = CAP_U(08.60);    /* 0.48*/

	/* Values from schematic */
	m_pin_value[5][1] = RES_K(3.30 );    /* 330Kohm*/
	m_pin_value[5][0] = RES_K(1.76 );    /* 76 Kohm*/

	/* Noise is not controlled by the audio bus! */
	/* Only value[0] is documented! */
	m_pin_value[4][0] = RES_K(47);       /* 47 K ohm*/
	m_pin_value[12][0] = RES_K(100);     /* 100K ohm*/
	m_pin_value[3][0] = 0;               /* NC*/

	/* Envelope-related */
	m_pin_value[ 1][0] = 0;
	m_pin_value[ 1][1] = 1;

	m_pin_value[28][0] = 0;
	m_pin_value[28][1] = 1;

	/* SN pins initialized to 0 */
	for (u8 i = 0; i < 16; i++)
		m_au[i]=0;

	m_val_mixer = 0;
}

void hec2hrp_state::update_sound(uint8_t data)
{
	/* MIXER */
	m_sn->mixer_a_w(BIT(m_val_mixer, 2));
	m_sn->mixer_b_w(BIT(m_val_mixer, 0));
	m_sn->mixer_c_w(BIT(m_val_mixer, 1)); /* Measured on HRX*/

	/* VCO oscillator */
	if (m_au[12]==1)
		m_sn->vco_res_w(m_pin_value[18][m_au[10]]/12.0); /* no AU11 */
	else
		m_sn->vco_res_w(m_pin_value[18][m_au[10]]); /* no AU11 */

	m_sn->vco_cap_w(m_pin_value[17][m_au[2]]);
	m_sn->pitch_voltage_w(m_pin_value[19][m_au[15]]);
	m_sn->vco_voltage_w(m_pin_value[16][m_au[15]]);
	m_sn->vco_w(m_pin_value[22][m_au[12]]); /* VCO Select Ext/SLF */

	/* SLF */
	m_sn->slf_res_w(m_pin_value[20][m_au[9]]); /* AU10 */
	m_sn->slf_cap_w(m_pin_value[21][m_au[1]]);

	/* One Shot */
	m_sn->one_shot_res_w(m_pin_value[24][0]); /* NC */
	m_sn->one_shot_cap_w(m_pin_value[23][m_au[13]]);

	/* amplitude value*/
	m_sn->amplitude_res_w(m_pin_value[11][m_au[5]]);

	/* attack/decay */
	m_sn->attack_res_w(m_pin_value[10][m_au[ 8]]);
	m_sn->decay_res_w(m_pin_value[7][m_au[11]]);
	m_sn->attack_decay_cap_w(m_pin_value[8][m_au[0]]);

	/* filter */
	m_sn->noise_filter_res_w(m_pin_value[5][m_au[4]]);
	m_sn->noise_filter_cap_w(m_pin_value[6][m_au[3]]);

	/* external noise clock */
	m_sn->noise_clock_res_w(m_pin_value[4][0]);
	m_sn->feedback_res_w(m_pin_value[12][0]);

	/* envelope */
	m_sn->envelope_1_w(m_pin_value[1] [m_au[6]]);
	m_sn->envelope_2_w(m_pin_value[28][m_au[7]]);

	/* finally, enable */
	m_sn->enable_w(m_pin_value[9][m_au[14]]);
}

void hec2hrp_state::hector_reset(bool hr, bool with_d2)
{
	// Hector init
	m_hector_flag_hr = hr;
	m_flag_clk = false;
	m_write_cassette = false;
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	// Disc II init
	if (with_d2)
	{
		m_disc2cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_upd_fdc->reset();
	}
}

void hec2hrp_state::hector_init()
{
	m_pot0 = m_pot1 = 0x40;

	/* for cassette sync */
	m_cassette_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hec2hrp_state::cassette_clock),this));
	m_cassette_timer->adjust(attotime::from_msec(100), 0, attotime::from_usec(64));/* => real sync scan speed for 15,624Khz*/

	init_sn76477();  /* init R/C values */
}


/* sound hardware */

static DISCRETE_SOUND_START( hec2hrp_discrete )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_OUTPUT(NODE_01, 5000)
DISCRETE_SOUND_END

void hec2hrp_state::hector_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SN76477(config, m_sn);
	m_sn->set_noise_params(RES_K(47), RES_K(330), CAP_P(390));
	m_sn->set_decay_res(RES_K(680));
	m_sn->set_attack_params(CAP_U(47), RES_K(180));
	m_sn->set_amp_res(RES_K(33));
	m_sn->set_feedback_res(RES_K(100));
	m_sn->set_vco_params(2, CAP_N(47), RES_K(1000));
	m_sn->set_pitch_voltage(2);
	m_sn->set_slf_params(CAP_U(0.1), RES_K(180));
	m_sn->set_oneshot_params(CAP_U(1.00001), RES_K(10000));
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.1);

	DISCRETE(config, m_discrete, hec2hrp_discrete).add_route(ALL_OUTPUTS, "mono", 1.0); /* 1-bit sound */
}

/*  DISK II drive for:
        Hector HRX
        Hector MX40c
        Hector MX80c

    JJStacino  jj.stacino@aliceadsl.fr

    15/02/2010 : Start of the disc2 project! JJStacino
    26/09/2010 : first sending with bug2 (the first "dir" command terminates in a crash of the Z80 disc II processor) -JJStacino
    01/11/2010 : first time boot sequence finishes, problem with CP/M launch -JJStacino
    20/11/2010 : synchronization between uPD765 and Z80 is now OK, CP/M works! -JJStacino
    28/11/2010 : Found at Bratislava that the disk writing with TRANS X: is NOT WORKING (the exchange Hector=>Disc2 ok)
*/

/* Callback uPD request */

/* How uPD765 works:
    * First we send at uPD the string of command (p.e. 9 bytes for read starting by 0x46) on port 60h
            between each byte, check the authorization of the uPD by reading the status register
    * When the command is finish, the data arrive with DMA interrupt, then:
            If read: in port 70 to retrieve the data,
            If write: in port 70 send the data
    * When all data had been send the uPD launch an INT
    * The Z80 Disc2 writes in FF12 a flag
    * if the flag is set, end of DMA function,
    * At this point the Z80 can read the RESULT in port 61h
*/

// Interrupt management

/* upd765 INT is connected to Z80 interrupt, with RNMI hardware authorization */
WRITE_LINE_MEMBER( hec2hrp_state::disc2_fdc_interrupt )
{
	m_irq_current_state = state;
	m_disc2cpu->set_input_line(INPUT_LINE_IRQ0, state && m_hector_disc2_rnmi ? ASSERT_LINE : CLEAR_LINE);
}

/* upd765 DRQ is connected to Z80 NMI, with RNMI hardware authorization */
WRITE_LINE_MEMBER( hec2hrp_state::disc2_fdc_dma_irq )
{
	m_nmi_current_state = state;
	m_disc2cpu->set_input_line(INPUT_LINE_NMI,  state && m_hector_disc2_rnmi ? ASSERT_LINE : CLEAR_LINE);
}

void hec2hrp_state::hector_disc2_reset()
{
	m_disc2cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	m_upd_fdc->reset();
	// Select ROM to cold restart
	m_bank[3]->set_entry(DISCII_BANK_ROM);

	// Clear the Hardware's buffers
	m_hector_disc2_data_r_ready = 0x0; /* =ff when PC2 = true and data is in read buffer (state->m_hector_disc2_data_read) */
	m_hector_disc2_data_w_ready = 0x0; /* =ff when Disc 2 Port 40 has data in write buffer (state->m_hector_disc2_data_write) */
	m_hector_disc2_data_read = 0;      /* Data sent by Hector to Disc 2 when PC2=true */
	m_hector_disc2_data_write = 0;     /* Data sent by Disc 2 to Hector when Write Port I/O 40 */
	m_hector_disc2_rnmi = 0;           /* I/O 50 D5 state = authorization for INT / NMI */
	m_irq_current_state = 0;           /* Clear the active IRQ request */
	m_nmi_current_state = 0;           /* Clear the active DMA request */
}

// Port handling for Z80 Disc II unit

uint8_t hec2hrp_state::disc2_io00_port_r()
{
	/* Switch Disc 2 to RAM */
	m_bank[3]->set_entry(DISCII_BANK_RAM);
	return 0;
}
void hec2hrp_state::disc2_io00_port_w(uint8_t data)
{
	/* Switch Disc 2 to RAM */
	m_bank[3]->set_entry(DISCII_BANK_RAM);
}
uint8_t hec2hrp_state::disc2_io20_port_r()
{
	// TODO: Implement 8251 chip communication
	return 0;
}
void hec2hrp_state::disc2_io20_port_w(uint8_t data)
{
	// TODO: Implement 8251 chip communication
}

uint8_t hec2hrp_state::disc2_io30_port_r()
{
	return m_hector_disc2_data_r_ready;
}

void hec2hrp_state::disc2_io30_port_w(uint8_t data)
{
}

uint8_t hec2hrp_state::disc2_io40_port_r() /* Read data sent to Hector by Disc2 */
{
	m_hector_disc2_data_r_ready = 0x00;
	return m_hector_disc2_data_read;
}

void hec2hrp_state::disc2_io40_port_w(uint8_t data)    /* Write data sent by Disc2 to Hector */
{
	m_hector_disc2_data_write = data;
	m_hector_disc2_data_w_ready = 0x80;
}

uint8_t hec2hrp_state::disc2_io50_port_r()
{
	return m_hector_disc2_data_w_ready;
}

void hec2hrp_state::disc2_io50_port_w(uint8_t data)
{
	/* FDC Motor Control - Bit 0/1 defines the state of the FDD 0/1 motor */
	m_upd_connector[0]->get_device()->mon_w(BIT(data, 0));    // FLoppy motor A
	m_upd_connector[1]->get_device()->mon_w(BIT(data, 1));    // Floppy motor B

	/* Write bit TC uPD765 on D4 of port I/O 50 */
	m_upd_fdc->tc_w(BIT(data, 4));


	/* allow interrupts by ANDing with RNMI signal */
	m_hector_disc2_rnmi = BIT(data, 5);
	m_disc2cpu->set_input_line(INPUT_LINE_IRQ0, m_irq_current_state && m_hector_disc2_rnmi ? ASSERT_LINE : CLEAR_LINE);
	m_disc2cpu->set_input_line(INPUT_LINE_NMI,  m_nmi_current_state && m_hector_disc2_rnmi ? ASSERT_LINE : CLEAR_LINE);
}
