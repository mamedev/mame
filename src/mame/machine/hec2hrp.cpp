// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/////////////////////////////////////////////////////////////////////////
// HEC2HRP.C  in machine
/*      Hector 2HR+
        Victor
        Hector 2HR
        Hector HRX
        Hector MX40c
        Hector MX80c
        Hector 1
        Interact

        12/05/2009 Skeleton driver - Micko : mmicko@gmail.com
        31/06/2009 Video - Robbbert

        29/10/2009 Update skeleton to functional machine
                        by yo_fr       (jj.stac @ aliceadsl.fr)

                => add Keyboard,
                => add color,
                => add cassette,
                => add sn76477 sound and 1bit sound,
                => add joysticks (stick, pot, fire)
                => add BR/HR switching
                => add bank switch for HRX
                => add device MX80c and bank switching for the ROM
        03/01/2010 Update and clean prog  by yo_fr       (jj.stac @ aliceadsl.fr)
                => add the port mapping for keyboard
        28/09/2010 add the DISK II support by yo_fr      (jj.stac @ aliceadsl.fr)
                => Note that actually the DISK II boot (loading CPM : OK) but do not run (don't run the CPM...).
        20/11/2010 : synchronization between uPD765 and Z80 are now OK, CP/M runnig! JJStacino
        11/11/2011 : add the minidisque support -3 pouces 1/2 driver-  JJStacino  (jj.stac @ aliceadsl.fr)
        19/02/2012 : few adjustment for the hrp and hr machine - JJStacino

    don't forget to keep some information about these machines, see DChector project : http://dchector.free.fr/ made by DanielCoulom
        (and thank's to Daniel!) and Yves site : http://hectorvictor.free.fr/ (thank's too Yves!)

    TODO :  Add the cartridge function,
            Adjust the one shot and A/D timing (sn76477)
*/

#include "emu.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "sound/wave.h"      /* for K7 sound*/
#include "sound/discrete.h"  /* for 1 Bit sound*/
#include "machine/upd765.h" /* for floppy disc controller */

#include "formats/hect_tap.h"
#include "includes/hec2hrp.h"

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

/* Helper function*/
int hec2hrp_state::isHectorWithDisc2()
{
return ((strncmp(machine().system().name , "hec2hrx"  , 7)==0) ||
		(strncmp(machine().system().name , "hec2mx40" , 8)==0) ||
		(strncmp(machine().system().name , "hec2mx80" , 8)==0));
}

int hec2hrp_state::isHectorWithMiniDisc()
{
return ((strncmp(machine().system().name , "hec2mdhrx", 9)==0));
}

int hec2hrp_state::isHectorHR()
{
return ((strncmp(machine().system().name , "hec2hr"   , 6)==0) ||  //Aviable for hr & hrp
		(strncmp(machine().system().name , "hec2mdhrx", 9)==0) ||
		(strncmp(machine().system().name , "victor"   , 6)==0) ||
		(strncmp(machine().system().name , "hec2mx40" , 8)==0) ||
		(strncmp(machine().system().name , "hec2mx80" , 8)==0));
}

int hec2hrp_state::isHectoreXtend()
{
return ((strncmp(machine().system().name , "hec2mdhrx", 9)==0) ||
		(strncmp(machine().system().name , "hec2hrx"  , 7)==0) ||
		(strncmp(machine().system().name , "hec2mx40" , 8)==0) ||
		(strncmp(machine().system().name , "hec2mx80" , 8)==0));
}

/* Cassette timer*/
TIMER_CALLBACK_MEMBER(hec2hrp_state::Callback_CK)
{
/* To generate the CK signal (K7)*/
	m_CK_signal++;
}

WRITE8_MEMBER( hec2hrp_state::minidisc_control_w )
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

	membank("bank2")->set_entry(BIT(data, 5) ? HECTOR_BANK_BASE : HECTOR_BANK_DISC);
}

WRITE8_MEMBER(hec2hrp_state::hector_switch_bank_w)
{
	if (offset==0x00)   {   /* 0x800 et 0x000=> video page, HR*/
							if (isHectoreXtend())
								membank("bank1")->set_entry(HECTOR_BANK_VIDEO);
							if (m_flag_clk ==1)
							{
								m_flag_clk=0;
								m_maincpu->set_unscaled_clock(XTAL_5MHz);  /* increase CPU*/
							}
						}
	if (offset==0x04)   {   /* 0x804 => video page, BR*/
							m_hector_flag_hr=0;
							if (isHectoreXtend())
								membank("bank1")->set_entry(HECTOR_BANK_VIDEO);
							if (m_flag_clk ==0)
							{
								m_flag_clk=1;
								m_maincpu->set_unscaled_clock(XTAL_1_75MHz);  /* slowdown CPU*/
							}
						}
	if (offset==0x08)   {   /* 0x808 => base page, HR*/
							if (isHectoreXtend())
								membank("bank1")->set_entry(HECTOR_BANK_PROG);
							if (m_flag_clk ==1)
							{
								m_flag_clk=0;
								m_maincpu->set_unscaled_clock(XTAL_5MHz);  /* increase CPU*/
							}

						}
	if (offset==0x0c)   {   /* 0x80c => base page, BR*/
							m_hector_flag_hr=0;
							if (isHectoreXtend())
								membank("bank1")->set_entry(HECTOR_BANK_PROG);
							if (m_flag_clk ==0)
							{
								m_flag_clk=1;
								m_maincpu->set_unscaled_clock(XTAL_1_75MHz);  /* slowdown CPU*/
							}
						}
}

WRITE8_MEMBER(hec2hrp_state::hector_keyboard_w)
{
	/*nothing to do => read function manage the value*/
}

READ8_MEMBER(hec2hrp_state::hector_keyboard_r)
{
	UINT8 data = 0xff;

	if (offset ==7) /* Only when joy reading*/
	{
		/* Read special key for analog joystick emulation only (button and pot are analog signal!) and the reset */
		data=m_keyboard[8]->read();

		if (data & 0x01) /* Reset machine ! (on ESC key)*/
		{
			m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
			if (isHectorHR()) /* aviable for HR and up */
			{
				m_hector_flag_hr=1;
				if (isHectoreXtend())
					{
						membank("bank1")->set_entry(HECTOR_BANK_PROG);
						membank("bank2")->set_entry(HECTORMX_BANK_PAGE0);
					}
				//RESET DISC II unit
				if (isHectorWithDisc2() )
					hector_disc2_reset();

				/* floppy md master reset */
				if (isHectorWithMiniDisc())
					m_minidisc_fdc->reset();
			}

			else /* aviable for BR machines */
			m_hector_flag_hr=0;


		/*Common flag*/
		m_hector_flag_80c = 0;
		m_flag_clk = 0;
		}

		m_actions = 0;
		if (data & 0x02) /* Fire(0)*/
			m_actions += 1;

		if (data & 0x04) /* Fire(1)*/
			m_actions += 2;

		if (data & 0x08) /* Pot(0)+*/
			m_pot0 += 1;
		if (m_pot0>128) m_pot0 = 128;

		if (data & 0x10) /* Pot(0)-*/
			m_pot0 -= 1;

		if (m_pot0>250) m_pot0 = 0;

		if (data & 0x20) /* Pot(1)+*/
			m_pot1 += 1;
		if (m_pot1>128) m_pot1 = 128;

		if (data & 0x40) /* Pot(1)-*/
			m_pot1 -= 1;

		if (m_pot1>250) m_pot1 = 0;
	}

	/* in all case return the request value*/
	return m_keyboard[offset]->read();
}

WRITE8_MEMBER(hec2hrp_state::hector_sn_2000_w)
{
	Mise_A_Jour_Etat(0x2000+ offset, data);
	Update_Sound(space, data);
}
WRITE8_MEMBER(hec2hrp_state::hector_sn_2800_w)
{
	Mise_A_Jour_Etat(0x2800+ offset, data);
	Update_Sound(space, data);
}
READ8_MEMBER(hec2hrp_state::hector_cassette_r)
{
	double level;
	UINT8 value=0;

	if ((m_state3000 & 0x38) != 0x38 )   /* Selon Sb choix cassette ou timer (74153)*/
	{
		m_Data_K7 =  0x00;  /* No cassette => clear bit*/
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
		if (m_write_cassette == 0)
		{
			/* Accee a la cassette*/
			level = m_cassette->input();

			/* Travail du 741 en trigger*/
			if  (level < -0.08)
				m_cassette_bit = 0x00;
			if (level > +0.08)
				m_cassette_bit = 0x01;
		}
		/* Programme du sn7474 (bascule) : Changement ??tat bit Data K7 ?? chaque front montant de m_cassette_bit*/
		if ((m_cassette_bit != m_cassette_bit_mem) && (m_cassette_bit !=0))
		{
			if (m_Data_K7 == 0x00)
				m_Data_K7 =  0x80;/* En poids fort*/
			else
				m_Data_K7 =  0x00;
		}
		value = ( m_CK_signal & 0x7F ) + m_Data_K7;
		m_cassette_bit_mem = m_cassette_bit;  /* Memorisation etat bit cassette*/
	}
	return value;
}
WRITE8_MEMBER(hec2hrp_state::hector_sn_3000_w)
{
	m_state3000 = data & 0xf8; /* except bit 0 to 2*/
	if ((data & 7) != m_oldstate3000 )
	{
		/* Update sn76477 only when necessary!*/
		Mise_A_Jour_Etat(0x3000, data & 7 );
		Update_Sound(space, data & 7);
	}
	m_oldstate3000 = data & 7;
}

/* Color Interface */
WRITE8_MEMBER(hec2hrp_state::hector_color_a_w)
{
	if (data & 0x40)
	{
		/* Bit 6 => motor ON/OFF => for cassette state!*/
		if (m_write_cassette==0)
		{
				m_cassette->change_state(
						CASSETTE_MOTOR_ENABLED ,
						CASSETTE_MASK_MOTOR);
			// m_cassette->set_state((cassette_state)(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED ));
		}
	}
	else
	{   /* stop motor*/
		m_cassette->set_state(CASSETTE_STOPPED);
		m_write_cassette=0;
		m_counter_write =0;
	}
	if (((data & 0x80) != (m_oldstate1000 & 0x80)) && ((m_oldstate1000 & 7)==(data & 7)) ) /* Bit7 had change but not the color statement*/
	{
		/* Bit 7 => Write bit for cassette!*/
		m_counter_write +=1;

		if (m_counter_write > 5)
		{
			/* Wait several cycle before lauch the record to prevent somes bugs*/
			m_counter_write = 6;
			if (m_write_cassette==0)
			{   /* C'est la 1er fois => record*/
							m_cassette->change_state(
						CASSETTE_MOTOR_ENABLED ,
						CASSETTE_MASK_MOTOR);
				m_cassette->set_state(CASSETTE_RECORD);
				m_write_cassette=1;
			}
		}
		/* cassette data */
		m_cassette->output(((data & 0x80) == 0x80) ? -1.0 : +1.0);
	}

	/* Other bit : color definition*/
	m_hector_color[0] =  data        & 0x07 ;
	m_hector_color[2] = ((data >> 3)  & 0x07) | (m_hector_color[2] & 0x40);

	m_oldstate1000=data; /* For next step*/
}

WRITE8_MEMBER(hec2hrp_state::hector_color_b_w)
{
	discrete_device *discrete = machine().device<discrete_device>("discrete");
	m_hector_color[1] =  data        & 0x07;
	m_hector_color[3] = (data >> 3)  & 0x07;

	/* Half light on color 2 only on HR machines:*/
	if (data & 0x40) m_hector_color[2] |= 8; else m_hector_color[2] &= 7;

	/* Play bit*/
	discrete->write(space, NODE_01,  (data & 0x80) ? 0:1 );
}


/********************************************************************************
 Port Handling
********************************************************************************/

/*******************  READ PIO 8255 *******************/
READ8_MEMBER(hec2hrp_state::hector_io_8255_r)
{
	/* 8255 in mode 0 */
	UINT8 data =0;
	UINT8 data_l=0;
	UINT8 data_h=0;


	if ((offset & 0x3) == 0x0) /* Port A */
		data = m_hector_port_a;

	if ((offset & 0x3) == 0x1) /* Port B */
	{
		data = m_hector_port_b;
		#ifdef DEBUG_TRACE_COM_HECTOR
			printf("\nLecture data par Hector %x (portB)",data);
		#endif
	}

	if ((offset & 0x3) == 0x2) /* Port C */
	{
		data_l = (m_hector_port_c_l & 0x0f);
		data_h = (m_hector_port_c_h & 0xf0);

		if (BIT(m_hector_port_cmd, 0))                  /* Quartet inf en entree ?*/
			data_l = (m_hector_port_c_l & 0x0f);  /*no change*/

		if (BIT(m_hector_port_cmd, 3))                  /* Quartet sup en entree ?*/
		{
			m_hector_port_c_h = (m_hector_port_c_h & 0x0c0);    /* Clear bits 4 & 5*/

			if (m_hector_disc2_data_w_ready != 0x00)
				m_hector_port_c_h = m_hector_port_c_h + 0x010;  // PC4 (data write ready from Disc II to Hector)

			if (m_hector_disc2_data_r_ready != 0x00)
				m_hector_port_c_h = m_hector_port_c_h + 0x020;  // PC5 (data read ready from Hector to Disc2)

			m_hector_port_c_h = m_hector_port_c_h & 0x07F;      // PC7 (printer busy=0)
			data_h =  m_hector_port_c_h;
		}
		data= data_l + data_h;
	}
	return data;  // Return the value!
}

/*******************  WRITE PIO 8255 *******************/

WRITE8_MEMBER(hec2hrp_state::hector_io_8255_w)
{
	/* 8255 in mode 0 */
	if ((offset & 0x3) == 0x0) /* Port A => to printer or Disc II*/
	{
		m_hector_port_a = data;
		/* Port A => to printer*/
		/*  Caution : The strobe connection to the printer seems not be used
		So, all what is send to the Disc2 unit will be printed too! */

		if (BIT(m_hector_port_c_l, 0)) {        // PC0 (bit X0)= strobe printer !
			printer_image_device *printer = machine().device<printer_image_device>("printer");
			printer->output(m_hector_port_a);
		}

#ifdef DEBUG_TRACE_COM_HECTOR
		printf("\nEcriture data par Hector %x (dans portA)",data);
#endif
	}

	if ((offset & 0x3) == 0x1) /* Port B */
		m_hector_port_b = data;


	if ((offset & 0x3) == 0x2) /* Port C => depending cmd word */
	{
		if (!BIT(m_hector_port_cmd, 0))  /* cmd -> Quartet inf en sortie ?*/
		{
			m_hector_port_c_l = data & 0x0f;
			// Utilizing bits port C : PC0 for the printer : strobe!
			if (BIT(m_hector_port_c_l  , 0))        // PC0 (bit X0)= true
			{
				/* Port A => to printer*/
				//printer_output(machine().device("printer"), m_hector_port_a);
			}
			// Utilizing bits port C : PC1 // PC2  for the communication with disc2
			if (!BIT(m_hector_port_c_l  , 1))       // PC1 (bit X1)= true
			{
				// Lecture effectuee => RAZ memoire donnee m_hector_disc2_data_write dispo
				m_hector_port_b = m_hector_disc2_data_write; // Mep sur port B si 2eme 74374 existant !
				m_hector_disc2_data_w_ready = 0x00;
				#ifdef DEBUG_TRACE_COM_HECTOR
					printf("\nEcriture port B vers m_hector_disc2_data_write suite a PC1");
				#endif
			}
			if (!BIT(m_hector_port_c_l, 2))     // PC2 (bit X2)= true
			{
				m_hector_disc2_data_read = m_hector_port_a; /* mise en place de l'info presente sur le port A */
				m_hector_disc2_data_r_ready = 0x08;      /* memorisation de l'info */
				#ifdef DEBUG_TRACE_COM_HECTOR
					printf("\nEcriture port A pour m_hector_disc2_data_read suite a PC2");
				#endif
			}
		}
		if (!BIT(m_hector_port_cmd, 3))  /* cmd -> Quartet sup en sortie ?*/
			m_hector_port_c_h = (data & 0xf0);
	}

	if ((offset & 0x3) == 0x3) /* Port commande */
	{
		m_hector_port_cmd = data;
	}
}
/* End of 8255 managing */


/*******************  Ecriture PIO specifique machine MX40 *******************/
WRITE8_MEMBER(hec2hrp_state::hector_mx40_io_port_w)
{
/* Bank switching on several address */
	if ((offset &0x0ff) == 0x40) /* Port page 0*/
		membank("bank2")->set_entry(HECTORMX_BANK_PAGE0);
	if ((offset &0x0ff) == 0x41) /* Port page 1*/
	{
		membank("bank2")->set_entry(HECTORMX_BANK_PAGE1);
		m_hector_flag_80c=0;
	}
	if ((offset &0x0ff) == 0x44) /* Port page 2  => 42 pour MX80*/
		membank("bank2")->set_entry(HECTORMX_BANK_PAGE2);
	if ((offset &0x0ff) == 0x49) /* Port screen resolution*/
		m_hector_flag_80c=0;/* No 80c in 40c !*/
}

/*******************  Ecriture PIO specifique machine MX80 *******************/
WRITE8_MEMBER(hec2hrp_state::hector_mx80_io_port_w)
{
	if ((offset &0x0ff) == 0x40) /* Port page 0*/
		membank("bank2")->set_entry(HECTORMX_BANK_PAGE0);
	if ((offset &0x0ff) == 0x41) /* Port page 1*/
	{
		membank("bank2")->set_entry(HECTORMX_BANK_PAGE1);
		m_hector_flag_80c=0;
	}
	if ((offset &0x0ff) == 0x42) /* Port page 2  => port different du MX40*/
		membank("bank2")->set_entry(HECTORMX_BANK_PAGE2);
	if ((offset &0x0ff) == 0x49) /* Port screen resolution*/
		m_hector_flag_80c=1;
}

/********************************************************************************
 sound managment
********************************************************************************/

void hec2hrp_state::Mise_A_Jour_Etat(int Adresse, int Value )
{
/* Adjust value depending on I/O main CPU request*/
switch(Adresse )
{
	case 0x2000:
		/* Modification AU0 / AU8*/
		{   /* AU0*/
			m_AU[ 0] =  ((Value & 0x080 )==0) ? 0 : 1 ;
			/* AU8 : 0*/
			m_AU[ 8] =  ((Value & 0x040 )==0) ? 0 : 1 ;
			break;
		}
	case 0x2001:
		/* Modification AU1 / AU9*/
		{   /* AU1*/
			m_AU[ 1] =  ((Value & 0x080 )==0) ? 0 : 1 ;
			/* AU9*/
			m_AU[ 9] =  ((Value & 0x040 )==0) ? 0 : 1 ;
			break;
		}
	case 0x2002:
		/* Modification AU2 / AU10*/
		{   /* AU2*/
			m_AU[ 2] =  ((Value & 0x080 )==0) ? 0 : 1 ;
			/* AU10*/
			m_AU[10] =  ((Value & 0x040 )==0) ? 0 : 1 ;
			break;
		}
	case 0x2003:
		/* Modification AU3 / AU11*/
		{   /* AU3*/
			m_AU[ 3] =  ((Value & 0x080 )==0) ? 0 : 1 ;
			/* AU11*/
			m_AU[11] =  ((Value & 0x040 )==0) ? 0 : 1 ;
			break;
		}
	case 0x2800:
		/* Modification AU4 / AU12*/
		{   /* AU4*/
			m_AU[ 4] =  ((Value & 0x080 )==0) ? 0 : 1 ;
			/* AU8*/
			m_AU[12] =  ((Value & 0x040 )==0) ? 0 : 1 ;
			break;
		}
	case 0x2801:
		/* Modification AU5 / AU13*/
		{   /* AU5*/
			m_AU[ 5] =  ((Value & 0x080 )==0) ? 0 : 1 ;
			/* AU13*/
			m_AU[13] =  ((Value & 0x040 )==0) ? 0 : 1 ;
			break;
		}
	case 0x2802:
		{   /* Modification AU6 / AU14*/
			/* AU6*/
			m_AU[ 6] = ((Value & 0x080 )==0) ? 0 : 1 ;
			/* AU14*/
			m_AU[14] = ((Value & 0x040 )==0) ? 0 : 1 ;
			break;
		}
	case 0x2803:
		/* Modification AU7 / AU15*/
		{   /* AU7*/
			m_AU[ 7] =  ((Value & 0x080 )==0) ? 0 : 1 ;
			/* AU15*/
			m_AU[15] =  ((Value & 0x040 )==0) ? 0 : 1 ;
			break;
		}
	case 0x3000:
		/* Mixer modification*/
		{
			m_ValMixer = (Value & 7) ;
			break;
		}
	default: break;
} /*switch*/
}


void hec2hrp_state::Init_Value_SN76477_Hector()
{
	/* Remplissage des valeurs de resistance et capacite d'Hector*/

	/* Decay R*/
	m_Pin_Value[7][1] = RES_K(680.0); /*680K  */
		m_Pin_Value[7][0] = RES_K(252.325); /* 142.325 (680 // 180KOhm)*/

	/* Capa A/D*/
	m_Pin_Value[8][0] = CAP_U(0.47); /* 0.47uf*/
	m_Pin_Value[8][1] = CAP_U(1.47);  /* 1.47*/

	/* ATTACK R*/
	m_Pin_Value[10][1]= RES_K(180.0);   /* 180*/
	m_Pin_Value[10][0]= RES_K(32.054); /* 32.054 (180 // 39 KOhm)*/

	/* Version 3 : Ajuste pour les frequences mesurees :
	            // 4  0 SOUND 255 Hz => ajuste a l'oreille
	            // 4  4 SOUND  65 Hz => ajuste a l'oreille
	            // 4  8 SOUND  17 Hz =>  ajuste a l'oreille
	            // 4 12 SOUND 4,3 Hz =>  ajuste a l'oreille*/
	/*   SLF C       Version 3*/
	m_Pin_Value[21][0]= CAP_U(0.1);  /*CAPU(0.1) */
	m_Pin_Value[21][1]= CAP_U(1.1);  /*1.1*/

	/*SLF R        Version 3*/
	m_Pin_Value[20][1]= RES_K(180);    //180 vu
	m_Pin_Value[20][0]= RES_K(37.268); //37.268 (47//180 KOhms)

	/* Capa VCO*/
	/* Version 3 : Ajust?? pour les frequences mesur??es :
	        // 0 0  SOUND 5,5KHz => 5,1KHz
	        // 0 16 SOUND 1,3KHz => 1,2KHz
	        // 0 32 SOUND 580Hz  => 570Hz
	        // 0 48 SOUND 132Hz  => 120Hz*/
	m_Pin_Value[17][0] = CAP_N(47.0) ;  /*47,0 mesure ok */
	m_Pin_Value[17][1] = CAP_N(580.0) ; /*580  mesure ok */
	/* R VCO   Version 3*/
	m_Pin_Value[18][1] = RES_K(1400.0   );/*1300 mesure ok    // au lieu de 1Mohm*/
	m_Pin_Value[18][0] = RES_K( 203.548 );/*223  mesure ok    // au lieu de 193.548 (1000 // 240KOhm)*/

	/* VCO Controle*/
	m_Pin_Value[16][0] = 0.0;  /* Volts  */
	m_Pin_Value[16][1] = 1.41; /* 2 =  10/15eme de 5V*/

	/* Pitch*/
	m_Pin_Value[19][0] = 0.0;   /*Volts */
	m_Pin_Value[19][1] = 1.41;

	m_Pin_Value[22][0] = 0; /* TOR */
	m_Pin_Value[22][1] = 1;

	/* R OneShot*/
	m_Pin_Value[24][1] = RES_K(100);
		m_Pin_Value[24][0] = RES_K(1000);  /*RES_M(1) infini sur Hector car non connectee*/

	/* Capa OneShot*/
	m_Pin_Value[23][0] = 1.0;
	m_Pin_Value[23][1] = 0.0;  /* Valeur Bidon sur Hector car mise au 5Volts sans capa*/

	/* Enabled*/
	m_Pin_Value[9][0] = 0;
	m_Pin_Value[9][1] = 1;

	/* Volume*/
	m_Pin_Value[11][0] = 128; /* Rapport 50% et 100%  128*/
	m_Pin_Value[11][1] = 255; /*                      255*/

	/* Noise filter*/
	m_Pin_Value[6][0] = CAP_U(0.390);    /* 0.390*/
	m_Pin_Value[6][1] = CAP_U(08.60);    /* 0.48*/

	/* Valeur corrige par rapport au schema :*/
	m_Pin_Value[5][1] = RES_K(3.30 ) ;   /* 330Kohm*/
	m_Pin_Value[5][0] = RES_K(1.76 ) ;   /* 76 Kohm*/

	/* Noise pas commande par le bus audio !*/
		/* Seule la valeur [0] est documentee !*/
	m_Pin_Value[4][0] = RES_K(47) ;      /* 47 K ohm*/
	m_Pin_Value[12][0] = RES_K(100);     /* 100K ohm*/
	m_Pin_Value[3][0] = 0 ;              /* NC*/

	/* Gestion du type d'enveloppe*/
	m_Pin_Value[ 1][0] = 0;
	m_Pin_Value[ 1][1] = 1;

	m_Pin_Value[28][0] = 0;
	m_Pin_Value[28][1] = 1;

	/* Initialisation a 0 des pin du SN*/
	m_AU[0]=0;
	m_AU[1]=0;
	m_AU[2]=0;
	m_AU[3]=0;
	m_AU[4]=0;
	m_AU[5]=0;
	m_AU[6]=0;
	m_AU[7]=0;
	m_AU[8]=0;
	m_AU[9]=0;
	m_AU[10]=0;
	m_AU[11]=0;
	m_AU[12]=0;
	m_AU[13]=0;
	m_AU[14]=0;
	m_AU[15]=0;
	m_ValMixer = 0;
}

void hec2hrp_state::Update_Sound(address_space &space, UINT8 data)
{
	/* keep device*/
	/* MIXER*/
	m_sn->mixer_a_w(((m_ValMixer & 0x04)==4) ? 1 : 0);
	m_sn->mixer_b_w(((m_ValMixer & 0x01)==1) ? 1 : 0);
	m_sn->mixer_c_w(((m_ValMixer & 0x02)==2) ? 1 : 0);/* Revu selon mesure electronique sur HRX*/

	/* VCO oscillateur*/
	if (m_AU[12]==1)
		m_sn->vco_res_w(m_Pin_Value[18][m_AU[10]]/12.0); /* en non AU11*/
	else
		m_sn->vco_res_w(m_Pin_Value[18][m_AU[10]]); /* en non AU11*/

	m_sn->vco_cap_w(m_Pin_Value[17][m_AU[2 ]]);
	m_sn->pitch_voltage_w(m_Pin_Value[19][m_AU[15]]);
	m_sn->vco_voltage_w(m_Pin_Value[16][m_AU[15]]);
	m_sn->vco_w(m_Pin_Value[22][m_AU[12]]); /* VCO Select Ext/SLF*/

	/* SLF*/
	m_sn->slf_res_w(m_Pin_Value[20][m_AU[ 9]]);/*AU10*/
	m_sn->slf_cap_w(m_Pin_Value[21][m_AU[1 ]]);

	/* One Shot*/
	m_sn->one_shot_res_w(m_Pin_Value[24][     0]); /* NC*/
	m_sn->one_shot_cap_w(m_Pin_Value[23][m_AU[13]]);

	/* Ampli value*/
	m_sn->amplitude_res_w(m_Pin_Value[11][m_AU[5]]);

	/* Attack / Decay*/
	m_sn->attack_res_w(m_Pin_Value[10][m_AU[ 8]]);
	m_sn->decay_res_w(m_Pin_Value[7 ][m_AU[11]]);/*AU9*/
	m_sn->attack_decay_cap_w(m_Pin_Value[8][m_AU[0]]);

	/* Filtre*/
	m_sn->noise_filter_res_w(m_Pin_Value[5][m_AU[4]]);
	m_sn->noise_filter_cap_w(m_Pin_Value[6][m_AU[3]]);

	/* Clock Extern Noise*/
	m_sn->noise_clock_res_w(m_Pin_Value[4][0]);   /* fix*/
	m_sn->feedback_res_w(m_Pin_Value[12][0]);     /*fix*/

	/*  Envelope*/
	m_sn->envelope_1_w(m_Pin_Value[1 ][m_AU[6]]);
	m_sn->envelope_2_w(m_Pin_Value[28][m_AU[7]]);

	/* En dernier on lance (ou pas !)*/
	m_sn->enable_w(m_Pin_Value[9][m_AU[14]]);
}

void hec2hrp_state::hector_reset(int hr, int with_D2 )
{
	// Initialization Hector
	m_hector_flag_hr = hr;
	m_flag_clk = 0;
	m_write_cassette = 0;
	m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);

	// Initialization Disc II
	if (with_D2==1)

	{
		upd765a_device *fdc = machine().device<upd765a_device>("upd765");
		m_disc2cpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
		fdc->reset();
	}
}

void hec2hrp_state::hector_init()
{
	m_pot0 = m_pot1 = 0x40;

	/* For Cassette synchro*/
	m_Cassette_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hec2hrp_state::Callback_CK),this));
	m_Cassette_timer->adjust(attotime::from_msec(100), 0, attotime::from_usec(64));/* => real synchro scan speed for 15,624Khz*/

	/* Sound sn76477*/
	Init_Value_SN76477_Hector();  /*init R/C value*/
}


/* sound hardware */

static DISCRETE_SOUND_START( hec2hrp )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_OUTPUT(NODE_01, 5000)
DISCRETE_SOUND_END

MACHINE_CONFIG_FRAGMENT( hector_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(0, "mono", 0.25)  /* Sound level for cassette, as it is in mono => output channel=0*/

	MCFG_SOUND_ADD("sn76477", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), RES_K(330), CAP_P(390)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_K(680))                  // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(47), RES_K(180))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(33))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(100))               // feedback_res
	MCFG_SN76477_VCO_PARAMS(2, CAP_N(47), RES_K(1000))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(2)                       // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(0.1), RES_K(180))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(1.00001), RES_K(10000))   // oneshot caps + res
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.1)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0) /* Son 1bit*/
	MCFG_DISCRETE_INTF(hec2hrp)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END
