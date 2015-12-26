// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*****************************************************************************************

    Bellfruit scorpion1 driver, (under heavy construction !!!)

******************************************************************************************

 sound roms don't seem to actually get used in many sets, playing through the AY only?
 ---

     04-2011: J Wallace: Fixed watchdog to match actual circuit, also fixed lamping code.
  20-01-2007: J Wallace: Tidy up of coding
  30-12-2006: J Wallace: Fixed init routines.
  07-03-2006: El Condor: Recoded to more accurately represent the hardware setup.
  19-08-2005: Re-Animator
  16-08-2005: Converted to MAME protocol for when Viper board is completed.
  25-08-2005: Added support for adder2 (Toppoker), added support for NEC upd7759 soundcard

Standard scorpion1 memorymap
Note the similarity to system85 - indeed, the working title for this system was System 88,
and was considered an update to existing technology. Later revisions made it a platform in
its own right, prompting marketers to change the name.

SOME SETS COULD BE IN THE WRONG DRIVER AS A RESULT OF THESE SIMILARITIES!

___________________________________________________________________________________
   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-------------------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k) battery backed up
-----------+---+-----------------+-------------------------------------------------
2000-21FF  | W | D D D D D D D D | Reel 3 + 4 stepper latch
-----------+---+-----------------+-------------------------------------------------
2200-23FF  | W | D D D D D D D D | Reel 1 + 2 stepper latch
-----------+---+-----------------+-------------------------------------------------
2400-25FF  | W | D D D D D D D D | vfd + coin inhibits
-----------+---+-----------------+-------------------------------------------------
2600-27FF  | W | D D D D D D D D | electro mechanical meters
-----------+---+-----------------+-------------------------------------------------
2800-28FF  | W | D D D D D D D D | triacs used for payslides/hoppers
-----------+---+-----------------+-------------------------------------------------
2A00       |R/W| D D D D D D D D | MUX1 control reg (IN: mux inputs, OUT:lamps)
-----------+---+-----------------+-------------------------------------------------
2A01       | W | D D D D D D D D | MUX1 low data bits
-----------+---+-----------------+-------------------------------------------------
2A02       | W | D D D D D D D D | MUX1 hi  data bits
-----------+---+-----------------+-------------------------------------------------
2E00       | R | ? ? ? ? ? ? D D | IRQ status
-----------+---+-----------------+-------------------------------------------------
3001       |   | D D D D D D D D | AY-8912 data
-----------+---+-----------------+-------------------------------------------------
3101       | W | D D D D D D D D | AY-8912 address
-----------+---+-----------------+-------------------------------------------------
3406       |R/W| D D D D D D D D | MC6850
-----------+---+-----------------+-------------------------------------------------
3407       |R/W| D D D D D D D D | MC6850
-----------+---+-----------------+-------------------------------------------------
3408       |R/W| D D D D D D D D | MUX2 control reg (IN: reel optos, OUT: lamps)
-----------+---+-----------------+-------------------------------------------------
3409       | W | D D D D D D D D | MUX2 low data bits
-----------+---+-----------------+-------------------------------------------------
340A       | W | D D D D D D D D | MUX2 hi  data bits
-----------+---+-----------------+-------------------------------------------------
3600       | W | ? ? ? ? ? ? D D | ROM page select (select page 3 after reset)
-----------+---+-----------------+-------------------------------------------------
4000-5FFF  | R | D D D D D D D D | ROM (8k)
-----------+---+-----------------+-------------------------------------------------
6000-7FFF  | R | D D D D D D D D | Paged ROM (8k)
           |   |                 |   page 0 : rom area 0x0000 - 0x1FFF
           |   |                 |   page 1 : rom area 0x2000 - 0x3FFF
           |   |                 |   page 2 : rom area 0x4000 - 0x5FFF
           |   |                 |   page 3 : rom area 0x6000 - 0x7FFF
-----------+---+-----------------+-------------------------------------------------
8000-FFFF  | R | D D D D D D D D | ROM (32k)
-----------+---+-----------------+-------------------------------------------------

Optional (on expansion card) (Viper)
-----------+---+-----------------+-------------------------------------------------
3404       | R | D D D D D D D D | Coin acceptor, direct input
-----------+---+-----------------+-------------------------------------------------
3800-38FF  |R/W| D D D D D D D D | NEC uPD7759 soundchip W:control R:status
           |   |                 | normally 0x3801 is used can conflict with reel5+6
-----------+---+-----------------+-------------------------------------------------
3800-38FF  | W | D D D D D D D D | Reel 5 + 6 stepper latch (normally 0x3800 used)
-----------+---+-----------------+-------------------------------------------------

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "video/awpvid.h"
#include "video/bfm_adr2.h"
#include "machine/steppers.h" // stepper motor
#include "machine/bfm_bd1.h" // vfd
#include "machine/meters.h"
#include "sound/ay8910.h"
#include "sound/upd7759.h"
#include "machine/nvram.h"
#include "machine/bfm_comn.h"

#include "sc1_vfd.lh"
#include "sc1_vid.lh"

class bfm_sc1_state : public driver_device
{
public:
	bfm_sc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vfd0(*this, "vfd0"),
		m_maincpu(*this, "maincpu"),
		m_reel0(*this, "reel0"),
		m_reel1(*this, "reel1"),
		m_reel2(*this, "reel2"),
		m_reel3(*this, "reel3"),
		m_reel4(*this, "reel4"),
		m_reel5(*this, "reel5"),
		m_upd7759(*this, "upd") { }

	optional_device<bfm_bd1_t> m_vfd0;

	int m_mmtr_latch;
	int m_triac_latch;
	int m_vfd_latch;  //initialized but not used
	int m_irq_status;
	int m_optic_pattern;
	DECLARE_WRITE_LINE_MEMBER(reel0_optic_cb) { if (state) m_optic_pattern |= 0x01; else m_optic_pattern &= ~0x01; }
	DECLARE_WRITE_LINE_MEMBER(reel1_optic_cb) { if (state) m_optic_pattern |= 0x02; else m_optic_pattern &= ~0x02; }
	DECLARE_WRITE_LINE_MEMBER(reel2_optic_cb) { if (state) m_optic_pattern |= 0x04; else m_optic_pattern &= ~0x04; }
	DECLARE_WRITE_LINE_MEMBER(reel3_optic_cb) { if (state) m_optic_pattern |= 0x08; else m_optic_pattern &= ~0x08; }
	DECLARE_WRITE_LINE_MEMBER(reel4_optic_cb) { if (state) m_optic_pattern |= 0x10; else m_optic_pattern &= ~0x10; }
	DECLARE_WRITE_LINE_MEMBER(reel5_optic_cb) { if (state) m_optic_pattern |= 0x20; else m_optic_pattern &= ~0x20; }
	int m_acia_status;
	int m_locked;
	int m_is_timer_enabled;
	int m_coin_inhibits; //initialized but not used
	int m_mux1_outputlatch;
	int m_mux1_datalo;
	int m_mux1_datahi;
	int m_mux1_input;
	int m_mux2_outputlatch;
	int m_mux2_datalo;
	int m_mux2_datahi;
	int m_mux2_input;
	UINT8 m_sc1_Inputs[64];
	UINT8 m_codec_data[256];

	int m_defaultbank;
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(irqlatch_r);
	DECLARE_WRITE8_MEMBER(reel12_w);
	DECLARE_WRITE8_MEMBER(reel34_w);
	DECLARE_WRITE8_MEMBER(reel56_w);
	DECLARE_WRITE8_MEMBER(mmtr_w);
	DECLARE_READ8_MEMBER(mmtr_r);
	DECLARE_READ8_MEMBER(dipcoin_r);
	DECLARE_WRITE8_MEMBER(vfd_w);
	DECLARE_READ8_MEMBER(mux1latch_r);
	DECLARE_READ8_MEMBER(mux1datlo_r);
	DECLARE_READ8_MEMBER(mux1dathi_r);
	DECLARE_WRITE8_MEMBER(mux1latch_w);
	DECLARE_WRITE8_MEMBER(mux1datlo_w);
	DECLARE_WRITE8_MEMBER(mux1dathi_w);
	DECLARE_READ8_MEMBER(mux2latch_r);
	DECLARE_READ8_MEMBER(mux2datlo_r);
	DECLARE_READ8_MEMBER(mux2dathi_r);
	DECLARE_WRITE8_MEMBER(mux2latch_w);
	DECLARE_WRITE8_MEMBER(mux2datlo_w);
	DECLARE_WRITE8_MEMBER(mux2dathi_w);
	DECLARE_WRITE8_MEMBER(aciactrl_w);
	DECLARE_WRITE8_MEMBER(aciadata_w);
	DECLARE_READ8_MEMBER(aciastat_r);
	DECLARE_READ8_MEMBER(aciadata_r);
	DECLARE_WRITE8_MEMBER(triac_w);
	DECLARE_READ8_MEMBER(triac_r);
	DECLARE_READ8_MEMBER(nec_r);
	DECLARE_WRITE8_MEMBER(nec_latch_w);

	void save_state();

	DECLARE_DRIVER_INIT(toppoker);
	DECLARE_DRIVER_INIT(lotse_bank0);
	DECLARE_DRIVER_INIT(nocrypt_bank0);
	DECLARE_DRIVER_INIT(lotse);
	DECLARE_DRIVER_INIT(clatt);
	DECLARE_DRIVER_INIT(rou029);
	DECLARE_DRIVER_INIT(nocrypt);
	virtual void machine_reset() override;
	INTERRUPT_GEN_MEMBER(timer_irq);
	void sc1_common_init(int reels, int decrypt, int defaultbank);
	void Scorpion1_SetSwitchState(int strobe, int data, int state);
	int sc1_find_project_string( );
	required_device<cpu_device> m_maincpu;
	required_device<stepper_device> m_reel0;
	required_device<stepper_device> m_reel1;
	required_device<stepper_device> m_reel2;
	required_device<stepper_device> m_reel3;
	required_device<stepper_device> m_reel4;
	required_device<stepper_device> m_reel5;
	optional_device<upd7759_device> m_upd7759;
};

#define VFD_RESET  0x20
#define VFD_CLOCK1 0x80
#define VFD_DATA   0x40

#define MASTER_CLOCK    (XTAL_4MHz)


void bfm_sc1_state::save_state()
{
	save_item(NAME(m_mmtr_latch));
	save_item(NAME(m_triac_latch));
	// save_item(NAME(m_vfd_latch));  //enable when used
	save_item(NAME(m_irq_status));
	save_item(NAME(m_optic_pattern));
	save_item(NAME(m_acia_status));
	save_item(NAME(m_locked));
	//save_item(NAME(m_is_timer_enabled)); //currently always set to 1
	//save_item(NAME(m_coin_inhibits)); //enable when used
	save_item(NAME(m_mux1_outputlatch));
	save_item(NAME(m_mux1_datalo));
	save_item(NAME(m_mux1_datahi));
	save_item(NAME(m_mux1_input));
	save_item(NAME(m_mux2_outputlatch));
	save_item(NAME(m_mux2_datalo));
	save_item(NAME(m_mux2_datahi));
	save_item(NAME(m_mux2_input));
	save_item(NAME(m_sc1_Inputs));
	save_item(NAME(m_codec_data));
	save_item(NAME(m_defaultbank));
}

///////////////////////////////////////////////////////////////////////////

void bfm_sc1_state::Scorpion1_SetSwitchState(int strobe, int data, int state)
{
	if ( state ) m_sc1_Inputs[strobe] |=  (1<<data);
	else         m_sc1_Inputs[strobe] &= ~(1<<data);
}

///////////////////////////////////////////////////////////////////////////
#ifdef UNUSED_FUNCTION
int bfm_sc1_state::Scorpion1_GetSwitchState(int strobe, int data)
{
	int state = 0;

	if ( strobe < 7 && data < 8 ) state = (sc1_Inputs[strobe] & (1<<data))?1:0;

	return state;
}
#endif
///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x03);
}

///////////////////////////////////////////////////////////////////////////

INTERRUPT_GEN_MEMBER(bfm_sc1_state::timer_irq)
{
	if ( m_is_timer_enabled )
	{
		m_irq_status = 0x01 |0x02; //0xff;

		m_sc1_Inputs[2] = ioport("STROBE0")->read();

		m_maincpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
	}
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::irqlatch_r)
{
	int result = m_irq_status | 0x02;

	m_irq_status = 0;

	return result;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::reel12_w)
{
	if ( m_locked & 0x01 )
	{   // hardware is still locked,
		if ( data == 0x46 ) m_locked &= ~0x01;
	}
	else
	{
		m_reel0->update((data>>4)&0x0f);
		m_reel1->update( data    &0x0f);
	}
	awp_draw_reel("reel1", m_reel0);
	awp_draw_reel("reel2", m_reel1);
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::reel34_w)
{
	if ( m_locked & 0x02 )
	{   // hardware is still locked,
		if ( data == 0x42 ) m_locked &= ~0x02;
	}
	else
	{
		m_reel2->update((data>>4)&0x0f);
		m_reel3->update( data    &0x0f);
	}
	awp_draw_reel("reel3", m_reel2);
	awp_draw_reel("reel4", m_reel3);
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::reel56_w)
{
	m_reel4->update((data>>4)&0x0f);
	m_reel5->update( data    &0x0f);

	awp_draw_reel("reel5", m_reel4);
	awp_draw_reel("reel6", m_reel5);
}

///////////////////////////////////////////////////////////////////////////
// mechanical meters //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::mmtr_w)
{
	int i;
	if ( m_locked & 0x04 )
	{   // hardware is still locked,
		m_locked &= ~0x04;
	}
	else
	{
		int  changed = m_mmtr_latch ^ data;

		m_mmtr_latch = data;

		for (i=0; i<8; i++)
		{
			if ( changed & (1 << i) )
			{
				MechMtr_update(i, data & (1 << i) );
				m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::mmtr_r)
{
	return m_mmtr_latch;
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::dipcoin_r)
{
	return ioport("STROBE0")->read() & 0x1F;
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::nec_r)
{
	return 1;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::vfd_w)
{
	m_vfd0->por(data & VFD_RESET);
	m_vfd0->data(data & VFD_DATA);
	m_vfd0->sclk(data & VFD_CLOCK1);
}

/////////////////////////////////////////////////////////////////////////////////////
// input / output multiplexers //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

// conversion table BFM strobe data to internal lamp numbers

static const UINT8 BFM_strcnv[] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, 0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17, 0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27, 0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, 0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47, 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57, 0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67, 0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, 0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,

	0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F, 0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F, 0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F, 0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F, 0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F, 0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F, 0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F, 0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
	0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F, 0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

//ACIA helper functions

/////////////////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::mux1latch_r)
{
	return m_mux1_input;
}

/////////////////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::mux1datlo_r)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::mux1dathi_r)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::mux1latch_w)
{
	int changed = m_mux1_outputlatch ^ data;
	static const char *const portnames[] = { "STROBE0", "STROBE1", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7" };
	m_mux1_outputlatch = data;

	if ( changed & 0x08 )
	{ // clock changed

		int input_strobe = data & 0x07;
		if ( !(data & 0x08) )
		{ // clock changed to low
			int strobe, offset, pattern, i;

			strobe  = data & 0x07;
			offset  = strobe<<4;
			pattern = 0x01;

			for ( i = 0; i < 8; i++ )
			{
				output_set_lamp_value(BFM_strcnv[offset  ], (m_mux1_datalo & pattern?1:0) );
				output_set_lamp_value(BFM_strcnv[offset+8], (m_mux1_datahi & pattern?1:0) );
				pattern<<=1;
				offset++;
			}

		}

		if ( !(data & 0x08) )
		{
			m_sc1_Inputs[ input_strobe ] = ioport(portnames[input_strobe])->read();

			m_mux1_input = m_sc1_Inputs[ input_strobe ];
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::mux1datlo_w)
{
	m_mux1_datalo = data;
}

/////////////////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::mux1dathi_w)
{
	m_mux1_datahi = data;
}

/////////////////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::mux2latch_r)
{
	return m_mux2_input;
}

/////////////////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::mux2datlo_r)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::mux2dathi_r)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::mux2latch_w)
{
	int changed = m_mux2_outputlatch ^ data;

	m_mux2_outputlatch = data;

	if ( changed & 0x08 )
	{ // clock changed

		if ( !(data & 0x08) )
		{ // clock changed to low
			int strobe, offset, pattern, i;

			strobe  = data & 0x07;
			offset  = 128+(strobe<<4);
			pattern = 0x01;

			for ( i = 0; i < 8; i++ )
			{
				output_set_lamp_value(BFM_strcnv[offset  ], (m_mux2_datalo & pattern?1:0) );
				output_set_lamp_value(BFM_strcnv[offset+8], (m_mux2_datahi & pattern?1:0) );
				pattern<<=1;
				offset++;
			}
		}

		if ( !(data & 0x08) )
		{
			m_mux2_input = 0x3F ^ m_optic_pattern;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::mux2datlo_w)
{
	m_mux2_datalo = data;
}

/////////////////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::mux2dathi_w)
{
	m_mux2_datahi = data;
}

/////////////////////////////////////////////////////////////////////////////////////
// serial port //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::aciactrl_w)
{
}

/////////////////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::aciadata_w)
{
}

/////////////////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::aciastat_r)
{
	return m_acia_status;
}

/////////////////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::aciadata_r)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// payslide triacs //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc1_state::triac_w)
{
	m_triac_latch = data;
}

/////////////////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc1_state::triac_r)
{
	return m_triac_latch;
}

/////////////////////////////////////////////////////////////////////////////////////
#ifdef UNUSED_FUNCTION
WRITE8_MEMBER(bfm_sc1_state::nec_reset_w)
{
	m_upd7759->start_w(0);
	m_upd7759->reset_w(data);
}
#endif
/////////////////////////////////////////////////////////////////////////////////////
WRITE8_MEMBER(bfm_sc1_state::nec_latch_w)
{
	m_upd7759->port_w (space, 0, data&0x3F);   // setup sample
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);         // start
}

// machine start (called only once) /////////////////////////////////////////////////

void bfm_sc1_state::machine_reset()
{
	m_vfd_latch         = 0;
	m_mmtr_latch        = 0;
	m_triac_latch       = 0;
	m_irq_status        = 0;
	m_is_timer_enabled  = 1;
	m_coin_inhibits     = 0;
	m_mux1_outputlatch  = 0x08; // clock HIGH
	m_mux1_datalo       = 0;
	m_mux1_datahi         = 0;
	m_mux1_input        = 0;
	m_mux2_outputlatch  = 0x08; // clock HIGH
	m_mux2_datalo       = 0;
	m_mux2_datahi         = 0;
	m_mux2_input        = 0;

	m_vfd0->reset();

	m_acia_status   = 0x02; // MC6850 transmit buffer empty !!!
	m_locked          = 0x07; // hardware is locked

// init rom bank ////////////////////////////////////////////////////////////////////
	{
		UINT8 *rom = memregion("maincpu")->base();

		membank("bank1")->configure_entries(0, 4, &rom[0x0000], 0x02000);
		membank("bank1")->set_entry(m_defaultbank);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board memory map ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( sc1_base, AS_PROGRAM, 8, bfm_sc1_state )

	AM_RANGE(0x0000, 0x1FFF) AM_RAM AM_SHARE("nvram") //8k RAM
	AM_RANGE(0x2000, 0x21FF) AM_WRITE(reel34_w)             // reel 2+3 latch
	AM_RANGE(0x2200, 0x23FF) AM_WRITE(reel12_w)             // reel 1+2 latch
	AM_RANGE(0x2400, 0x25FF) AM_WRITE(vfd_w)                // vfd latch

	AM_RANGE(0x2600, 0x27FF) AM_READWRITE(mmtr_r,mmtr_w)    // mechanical meters
	AM_RANGE(0x2800, 0x2800) AM_READWRITE(triac_r,triac_w)  // payslide triacs

	AM_RANGE(0x2A00, 0x2A00) AM_READWRITE(mux1latch_r,mux1latch_w) // mux1
	AM_RANGE(0x2A01, 0x2A01) AM_READWRITE(mux1datlo_r,mux1datlo_w)
	AM_RANGE(0x2A02, 0x2A02) AM_READWRITE(mux1dathi_r,mux1dathi_w)

	AM_RANGE(0x2E00, 0x2E00) AM_READ(irqlatch_r)            // irq latch

	AM_RANGE(0x3001, 0x3001) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x3001, 0x3001) AM_DEVWRITE("aysnd", ay8910_device, data_w)
	AM_RANGE(0x3101, 0x3201) AM_DEVWRITE("aysnd", ay8910_device, address_w)

	AM_RANGE(0x3406, 0x3406) AM_READWRITE(aciastat_r,aciactrl_w)  // MC6850 status register
	AM_RANGE(0x3407, 0x3407) AM_READWRITE(aciadata_r,aciadata_w)  // MC6850 data register

	AM_RANGE(0x3408, 0x3408) AM_READWRITE(mux2latch_r,mux2latch_w) // mux2
	AM_RANGE(0x3409, 0x3409) AM_READWRITE(mux2datlo_r,mux2datlo_w)
	AM_RANGE(0x340A, 0x340A) AM_READWRITE(mux2dathi_r,mux2dathi_w)

	AM_RANGE(0x3600, 0x3600) AM_WRITE(bankswitch_w)         // write bank
	AM_RANGE(0x3800, 0x39FF) AM_WRITE(reel56_w)             // reel 5+6 latch

	AM_RANGE(0x4000, 0x5FFF) AM_ROM                         // 8k  ROM
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")                    // 8k  paged ROM (4 pages)
	AM_RANGE(0x8000, 0xFFFF) AM_ROM AM_WRITE (watchdog_reset_w) // 32k ROM

ADDRESS_MAP_END

/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board + adder2 expansion memory map ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( sc1_adder2, AS_PROGRAM, 8, bfm_sc1_state )
	AM_IMPORT_FROM( sc1_base )

	AM_RANGE(0x3E00, 0x3E00) AM_DEVREADWRITE("adder2", bfm_adder2_device, vid_uart_ctrl_r,vid_uart_ctrl_w)  // video uart control reg read
	AM_RANGE(0x3E01, 0x3E01) AM_DEVREADWRITE("adder2", bfm_adder2_device, vid_uart_rx_r,vid_uart_tx_w)      // video uart receive  reg
ADDRESS_MAP_END


/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board + upd7759 soundcard memory map ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( sc1_viper, AS_PROGRAM, 8, bfm_sc1_state )
	AM_IMPORT_FROM( sc1_base )

	AM_RANGE(0x3404, 0x3404) AM_READ(dipcoin_r ) // coin input on gamecard
	AM_RANGE(0x3801, 0x3801) AM_READ(nec_r)
	AM_RANGE(0x3800, 0x39FF) AM_WRITE(nec_latch_w)
ADDRESS_MAP_END

// input ports for scorpion1 board //////////////////////////////////////////////////

static INPUT_PORTS_START( scorpion1 )

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Green Test")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Red Test")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) // collect?
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON7 )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 ) // service?
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON9 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON10 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON11 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON12 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON13 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON14 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON15 )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x01, 0x00, "DIL01" )PORT_DIPLOCATION("DIL:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL02" )PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" )PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Acceptor" )PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" )PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Lockout" )PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Cashpot Frequency?" )PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High ))
	PORT_DIPSETTING(    0x40, DEF_STR( Low ))
	PORT_DIPNAME( 0x80, 0x00, "DIL08" )PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("STROBE7")
	PORT_DIPNAME( 0x01, 0x00, "DIL09" )PORT_DIPLOCATION("DIL:09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL11" )PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL12" )PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL13" )PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0xe0, 0x00, "Payout Percentage?" )PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x60, "72%")
	PORT_DIPSETTING(    0x20, "75%")
	PORT_DIPSETTING(    0x00, "78%")
	PORT_DIPSETTING(    0x80, "81%")
	PORT_DIPSETTING(    0xc0, "85%")

INPUT_PORTS_END

// input ports for scorpion1 board ////////////////////////////////////////

static INPUT_PORTS_START( clatt )
	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("1 Pound")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Green Test")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Red Test")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hold 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hold 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hold 4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Exchange")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 )   PORT_NAME("Stop/Gamble")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1  )   PORT_NAME("Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_INTERLOCK ) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE )   PORT_NAME("Refill Key")   PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_INTERLOCK ) PORT_NAME("Front Door")   PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x01, 0x00, "DIL01" ) PORT_DIPLOCATION("DIL:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL02" )PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" )PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Acceptor" )PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" )PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Lockout" )PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Cashpot Frequency?" )PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High ))
	PORT_DIPSETTING(    0x40, DEF_STR( Low ))
	PORT_DIPNAME( 0x80, 0x00, "DIL08" )PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("STROBE7")
	PORT_DIPNAME( 0x01, 0x00, "DIL09" )PORT_DIPLOCATION("DIL:09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL11" )PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL12" )PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL13" )PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0xe0, 0x00, "Payout Percentage?" )PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x60, "72%")
	PORT_DIPSETTING(    0x20, "75%")
	PORT_DIPSETTING(    0x00, "78%")
	PORT_DIPSETTING(    0x80, "81%")
	PORT_DIPSETTING(    0xc0, "85%")

INPUT_PORTS_END

static INPUT_PORTS_START( toppoker )
	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Green Test")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Red Test")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Vast 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Vast 2/Kop")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Vast 3/Munt")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Herstellen/Neem Win")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Verander/Inzet")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Neem Club Win")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Narr Club/Deal")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Neem Feature")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Neem Club Meter")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_NAME("Neem Win Bank")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON11) PORT_NAME("Uitbetalen")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON12) PORT_NAME("Vast Monitor 1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_INTERLOCK)PORT_NAME("Back Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Slide Dump") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Meter Key") PORT_CODE(KEYCODE_E) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )//Tube status Low switch for 1 Pound
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON13) PORT_NAME("Vast Monitor 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON14) PORT_NAME("Vast Monitor 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON15) PORT_NAME("Vast Monitor 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_INTERLOCK)PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON16) PORT_NAME("Vast Monitor 5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x01, 0x00, "DIL01" ) PORT_DIPLOCATION("DIL:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL02" )PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" )PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Acceptor" )PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" )PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Lockout" )PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Cashpot Frequency?" )PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High ))
	PORT_DIPSETTING(    0x40, DEF_STR( Low ))
	PORT_DIPNAME( 0x80, 0x00, "DIL08" )PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("STROBE7")
	PORT_DIPNAME( 0x01, 0x00, "DIL09" )PORT_DIPLOCATION("DIL:09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL11" )PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL12" )PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL13" )PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0xe0, 0x00, "Payout Percentage?" )PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x60, "72%")
	PORT_DIPSETTING(    0x20, "75%")
	PORT_DIPSETTING(    0x00, "78%")
	PORT_DIPSETTING(    0x80, "81%")
	PORT_DIPSETTING(    0xc0, "85%")

INPUT_PORTS_END

/////////////////////////////////////////////////////////////////////////////////////
// machine driver for scorpion1 board ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static MACHINE_CONFIG_START( scorpion1, bfm_sc1_state )
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4)          // 6809 CPU at 1 Mhz
	MCFG_CPU_PROGRAM_MAP(sc1_base)                      // setup read and write memorymap
	MCFG_CPU_PERIODIC_INT_DRIVER(bfm_sc1_state, timer_irq,  1000)               // generate 1000 IRQ's per second
	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_MONOSTABLE(120000,100e-9))


	MCFG_BFMBD1_ADD("vfd0",0)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd",AY8912, MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_DEFAULT_LAYOUT(layout_sc1_vfd)

	MCFG_STARPOINT_48STEP_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc1_state, reel0_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc1_state, reel1_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc1_state, reel2_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc1_state, reel3_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc1_state, reel4_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc1_state, reel5_optic_cb))
MACHINE_CONFIG_END

/////////////////////////////////////////////////////////////////////////////////////
// machine driver for scorpion1 board + adder2 extension ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static MACHINE_CONFIG_DERIVED( scorpion1_adder2, scorpion1 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sc1_adder2)                // setup read and write memorymap

	MCFG_DEFAULT_LAYOUT(layout_sc1_vid)

	MCFG_BFM_ADDER2_ADD("adder2")
MACHINE_CONFIG_END

/////////////////////////////////////////////////////////////////////////////////////
// machine driver for scorpion1 board ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static MACHINE_CONFIG_DERIVED( scorpion1_viper, scorpion1 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sc1_viper)                 // setup read and write memorymap

	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


void bfm_sc1_state::sc1_common_init(int reels, int decrypt, int defaultbank)
{
	memset(m_sc1_Inputs, 0, sizeof(m_sc1_Inputs));

	// setup n default 96 half step reels ///////////////////////////////////////////
	/*switch (reels)
	{
	case 6: m_reel5->configure(&starpoint_interface_48step);
	case 5: m_reel4->configure(&starpoint_interface_48step);
	case 4: m_reel3->configure(&starpoint_interface_48step);
	case 3: m_reel2->configure(&starpoint_interface_48step);
	case 2: m_reel1->configure(&starpoint_interface_48step);
	case 1: m_reel0->configure(&starpoint_interface_48step);
	}*/
	if (decrypt) bfm_decode_mainrom(machine(),"maincpu", m_codec_data);    // decode main rom

	m_defaultbank = defaultbank;

}


int bfm_sc1_state::sc1_find_project_string( )
{
	// search for the project string to find the title (usually just at ff00)
	char title_string[7][32] = { "PROJECT NUMBER", "PROJECT PR", "PROJECT ", "CASH ON THE NILE 2", "PR6121", "CHINA TOWN\x0d\x0a", "PROJECTNUMBER" };
	UINT8 *src = memregion( "maincpu" )->base();
	int size = memregion( "maincpu" )->bytes();

	for (auto & elem : title_string)
	{
		int strlength = strlen(elem);

		for (int i=0;i<size-strlength;i++)
		{
			int j;
			int found = 1;
			for (j=0;j<strlength;j+=1)
			{
				UINT8 rom = src[(i+j)];
				UINT8 chr = elem[j];

				if (rom != chr)
				{
					found = 0;
					break;
				}
			}

			if (found!=0)
			{
				int end=0;
				int count = 0;
				int blankcount = 0;
				printf("ID String @ %08x\n", i);

				while (!end)
				{
					UINT8 rom;
					int addr;

					addr = (i+count);

					if (addr<size)
					{
						rom = src[addr];

						if ((rom>=0x20) && (rom<0x7f))
						{
							printf("%c", rom);
							blankcount = 0;
						}
						else
						{
							blankcount++;
							if (blankcount<10) printf(" ");
						}

						count++;
					}
					else
						end = 1;

					if (count>=0x100)
						end = 1;
				}
				printf("\n");

				return 1;
			}
		}
	}

	return 0;
}



DRIVER_INIT_MEMBER(bfm_sc1_state,toppoker)
{
	sc1_common_init(3,1, 3);
	MechMtr_config(machine(),8);
	sc1_find_project_string();
	save_state();
}

DRIVER_INIT_MEMBER(bfm_sc1_state,lotse)
{
	sc1_common_init(6,1, 3);
	MechMtr_config(machine(),8);
	sc1_find_project_string();
	save_state();
}

DRIVER_INIT_MEMBER(bfm_sc1_state,lotse_bank0)
{
	sc1_common_init(6,1, 0);
	MechMtr_config(machine(),8);
	sc1_find_project_string();
	save_state();
}


DRIVER_INIT_MEMBER(bfm_sc1_state,nocrypt)
{
	sc1_common_init(6,0, 3);
	MechMtr_config(machine(),8);
	sc1_find_project_string();
	save_state();
}

DRIVER_INIT_MEMBER(bfm_sc1_state,nocrypt_bank0)
{
	sc1_common_init(6,0, 0);
	MechMtr_config(machine(),8);
	sc1_find_project_string();
	save_state();
}


/////////////////////////////////////////////////////////////////////////////////////

DRIVER_INIT_MEMBER(bfm_sc1_state,rou029)
{
	sc1_common_init(6,0, 3);
	MechMtr_config(machine(),8);
	sc1_find_project_string();
	save_state();
}

/////////////////////////////////////////////////////////////////////////////////////

DRIVER_INIT_MEMBER(bfm_sc1_state,clatt)
{
	sc1_common_init(6,1, 3);
	MechMtr_config(machine(),8);

	Scorpion1_SetSwitchState(3,2,1);
	Scorpion1_SetSwitchState(3,3,1);
	Scorpion1_SetSwitchState(3,6,1);
	Scorpion1_SetSwitchState(4,1,1);
	sc1_find_project_string();
	save_state();
}


// ROM definition ///////////////////////////////////////////////////////////////////





#define sc1_pwrl_sound \
	ROM_REGION( 0x40000, "upd", 0 ) \
	ROM_LOAD( "powl_snd.bin", 0x00000, 0x40000, CRC(e87af436) SHA1(fc853eca052fe13babde5f4579e202321ecb8f7e) )\
	ROM_REGION( 0x40000, "altupd", 0 ) \
	ROM_LOAD( "95000013.bin", 0x00000, 0x8000, CRC(80573db9) SHA1(34e028d1d01328719f6260aafb58f40d664ab7ea) ) \
	ROM_LOAD( "95000014.bin", 0x08000, 0x8000, CRC(cad7c87b) SHA1(052324bbad28b67d23a018d61a03783dd4dfd9cf) ) \
	ROM_LOAD( "95000015.bin", 0x10000, 0x8000, CRC(c46911ca) SHA1(a270d0708574a549b88f13f9cde1d7dcdfc624a9) )

#define sc1_winst_sound \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "winningstreaksnd.bin", 0x0000, 0x080000, CRC(ba30cb97) SHA1(e7f5ca36ca993ad14b3a348868e73d7ba02be7c5) )
//not upd?
#define sc1_driv_sound \
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "ds_snd1.bin", 0x000000, 0x020000, CRC(a9d7e8ec) SHA1(5b1d459d378e23d3108a1190b5988eebedf95667) )\
	ROM_LOAD( "ds_snd2.bin", 0x020000, 0x020000, CRC(3b67c1b3) SHA1(8b9dbff45955f72a73fb739b5e74aa2f9c23dd08) )\
	ROM_LOAD( "ds_snd3.bin", 0x040000, 0x020000, CRC(00c252ec) SHA1(5de2e70f142a71f22eeb28a271ca9d7809322faa) )
ROM_START( sc1lotus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lotusse.bin",  0x00000, 0x10000,  CRC(636dadc4) SHA1(85bad5d76dac028fe9f3303dd09e8266aba7db4d))
ROM_END

ROM_START( sc1lotusa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lotusse_.bin", 0x00000, 0x010000, CRC(e5f51a36) SHA1(9cddf757c1636911fce370168e636ffcff7bfab6) )
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

ROM_START( sc1roul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rou029.bin",   0x8000, 0x8000,  CRC(31723f0a) SHA1(e220976116a0aaf24dc0c4af78a9311a360e8104))
ROM_END

/////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////

ROM_START( m_tppokr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750899.bin", 0x00000, 0x10000,  CRC(639d1d62) SHA1(80620c14bf9f953588555510fc2e6e930140923f))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD( "tpk010.vid", 0x00000, 0x20000,  CRC(ea4eddca) SHA1(5fb805d35376ec7ee8d58684e584621dbb2b2a9c))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "tpk011.chr", 0x00000, 0x20000,  CRC(4dc23ad8) SHA1(8e8cc699412dbb092e16e14518f407353f477ee1))
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

ROM_START( sc1actv8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ac82019.bin", 0x00000, 0x10000, CRC(91855497) SHA1(dee8b6df953a3761fb67395842f701672e93a71e) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "95000600.bin", 0x00000, 0x10000, CRC(f324959a) SHA1(5be8c81dcfcf5f6b8b64a85891cd17e221e9ca08) )
	ROM_LOAD( "95000601.bin", 0x10000, 0x10000, CRC(585323f3) SHA1(e2e83b16bbad24f748a7dc9313b722862a91e5a2) )
ROM_END

ROM_START( sc1armad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "armada.bin", 0x00000, 0x010000, CRC(9a1be4ca) SHA1(d18b7c8779a8eb50321fbff4d6d8cf6d512bea8b) )
ROM_END






#if 0
ROM_START( sc1barcdb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "barcode.p2", 0x0000, 0x8000, CRC(44b79b14) SHA1(ec0745be0dde818c673c62ca584e22871a73e66e) )
	ROM_LOAD( "barcode.p1", 0x8000, 0x8000, CRC(0be64bfb) SHA1(3b5cfee8825f2b7d2598f04411d50b8f1245ac65) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "barsnd1.bin", 0x00000, 0x10000, CRC(c9de8ff4) SHA1(c3e77e84d4ecc1c779929a96d1c445a1af24865b) )
	ROM_LOAD( "barsnd2.bin", 0x10000, 0x10000, CRC(56af984a) SHA1(aebd30f3ca767dc5fc77fb01765833ee627a5aee) )
ROM_END
#endif

ROM_START( sc1bigmt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigmatch.bin", 0x00000, 0x10000, CRC(3c81663c) SHA1(a9670a48059d35d6581ce3007c0a6223291e0a12) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "bigmsnd1.bin", 0x00000, 0x10000, CRC(51828aa0) SHA1(99b46c1c4b45f26a393bf3e658ad499c84bdf8f5) )
	ROM_LOAD( "bigmsnd2.bin", 0x10000, 0x10000, CRC(cf1f0f6b) SHA1(6521f0fe52a0587af049940bb81846d40d8847b8) )
ROM_END


ROM_START( sc1calyp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "calypso.bin", 0x00000, 0x10000, CRC(b8194d31) SHA1(de7d374d8a1c18ec324daf92112652461e2a113e) )
ROM_END

ROM_START( sc1carro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "carrousel.bin", 0x00000, 0x10000, CRC(d1f7ae57) SHA1(301727b95f30d8e934a9c790838daf65aadd6dc7) )
ROM_END


ROM_START( sc1cshcd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cascrd2.bin", 0x0000, 0x8000, CRC(862d5ea9) SHA1(f0c0334aed028ab995b4d092abe10ece90be40a5) )
	ROM_LOAD( "cascrd1.bin", 0x8000, 0x8000, CRC(23142134) SHA1(40a900d190480677c883912e60f447e83b4a5c92) )
ROM_END

ROM_START( sc1cshcda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717128 var% b.bin", 0x0000, 0x8000, CRC(10662200) SHA1(79a35b88eca408ae2f5daead498662303e0360e1) )
	ROM_LOAD( "95717127 var% a.bin", 0x8000, 0x8000, CRC(1f7ef1ec) SHA1(9f8f43037788787f4f11501689cb82eeebc6d7f8) )
ROM_END

ROM_START( sc1cshcdb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95719104b c.cards 78%.bin", 0x0000, 0x8000, CRC(e9055e1b) SHA1(8c6b7e164c9998c3b932e16c3c4e4a95beb29f50) )
	ROM_LOAD( "95719103a c.cards 78%.bin", 0x8000, 0x8000, CRC(af65962c) SHA1(d10dd9e1bbdd1e506d5f8732ffbb6521e34fbefe) )
ROM_END

ROM_START( sc1ccoin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashcoin005.bin", 0x0000, 0x10000, CRC(5ce29d18) SHA1(c9e8d0aa52ba532177d912901a39e4fc8024810f) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "cashcoinic7.bin", 0x00000, 0x10000, CRC(7c2f52ed) SHA1(d435402459efc9311707ac691992874b56cbbeec) )
	ROM_LOAD( "cashcoinic8.bin", 0x10000, 0x10000, CRC(23b99731) SHA1(7cc1c51d9b72480d8a1020fc3621a05ba83d7629) )
ROM_END

ROM_START( sc1cexpd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashexpl.bin", 0x8000, 0x008000, CRC(83c6196c) SHA1(931fb5223c3ebc52ca2bd232d71000b8af4397e1) )
ROM_END




ROM_START( sc1cexpl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashxpl2.bin", 0x0000, 0x8000, CRC(0199136c) SHA1(bed1df64ecc0d7ef951a59717e219e6fe7ebf99c) )
	ROM_LOAD( "cashxpl1.bin", 0x8000, 0x8000, CRC(0fe62ead) SHA1(bd56a216292e9bf2b7753616a6cd25b37e22095f) )
ROM_END

ROM_START( sc1cexpla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717084b mk1.bin", 0x0000, 0x8000, CRC(e2d973be) SHA1(56ed3e3d6caf12f82d6ccc1527ff8da215e09cb0) )
	ROM_LOAD( "95717083a mk1.bin", 0x8000, 0x8000, CRC(949c2d18) SHA1(26db983c7d8624c3fd0461dba2336d5c59be29f0) )
ROM_END

ROM_START( sc1cexplb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717084b mk2 var%.bin", 0x0000, 0x8000, CRC(e2d973be) SHA1(56ed3e3d6caf12f82d6ccc1527ff8da215e09cb0) ) // aka 95717084b.bin
	ROM_LOAD( "95717083a mk2 var%.bin", 0x8000, 0x8000, CRC(1fe1b3a1) SHA1(dd10d74c71a455900a2325ac9d7b3c8e45eb9c6c) ) // aka 95717083a.bin
ROM_END




ROM_START( sc1clbxpa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cbexpp2", 0x0000, 0x8000, CRC(82eb61ac) SHA1(9c06542b43b01be5ec7be081fead92bfe9f905c5) )
	ROM_LOAD( "cbexpp1", 0x8000, 0x8000, CRC(8819728c) SHA1(691d6317fd38e09fa333fc49c82e85f69a04e359) )
ROM_END



ROM_START( sc1cshin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashino-b.bin", 0x0000, 0x8000, CRC(c0d3fb09) SHA1(7e0a302547b18946851d31be4d25c17aca32b767) )
	ROM_LOAD( "cashino-a.bin", 0x8000, 0x8000, CRC(8a585683) SHA1(01859c82a6d6b082de11e9208f8d38c519dc2575) )
ROM_END

ROM_START( sc1class )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "classic606.bin", 0x0000, 0x010000, CRC(f1dc300e) SHA1(17b1d69ed2fd3ce91ce86e9b0160a150a74a624b) )
ROM_END



ROM_START( sc1clown )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clowarou.786", 0x0000, 0x010000, CRC(d63b991c) SHA1(2f016aec3d2d8ebadcdbe794230ebf18dd660876) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "clownsound1.bin", 0x0000, 0x010000, CRC(d72b344b) SHA1(dbf180830ce74dc8d0b832f3932b5c11259acab2) )
	ROM_LOAD( "clownsound2.bin", 0x0000, 0x010000, CRC(98c0440c) SHA1(ef6d7ecf21d49aa8e838429e3431ebcd30fec21e) )
ROM_END


ROM_START( sc1cl2k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club2000.rom", 0x00000, 0x10000, CRC(2806b89d) SHA1(d1641b33e61de42dc7a643875226a276cf480832) )
ROM_END

ROM_START( sc1cl2k1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club2001.bin", 0x00000, 0x10000, CRC(4bb26aca) SHA1(41a896be314f2fefdaba962b44e9562aaf0642b1) )
ROM_END

ROM_START( sc1clbdm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubdiamond.bin", 0x8000, 0x8000, CRC(7e6a569e) SHA1(ba15478ae0312d3e9c21546aa676b4ab95ae944c) )
ROM_END

ROM_START( sc1clbxp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubexplosion2.bin", 0x0000, 0x8000, CRC(da56fbdd) SHA1(0ea35f6672a4a4b9236d8341733496450b64238e) )
	ROM_LOAD( "clubexplosion1.bin", 0x8000, 0x8000, CRC(876161db) SHA1(a6262d70870a6edb71469ec8cea317b185aec49e) )

	ROM_REGION( 0x20000, "upd", 0 )//Did a version of this have a UPD sound board, if so, these seem to be ROMs for it
	ROM_LOAD( "95000004.bin", 0x000000, 0x008000, CRC(6ed10c9b) SHA1(cd209e8f9e0a3fd41e4ed8b6c9387ee91c19704c) )
	ROM_LOAD( "95000005.bin", 0x008000, 0x008000, CRC(9e16aee2) SHA1(25610fcd4c073ff7f20a3d24f96792913fa447f7) )
	ROM_LOAD( "95000006.bin", 0x010000, 0x008000, CRC(41636b3d) SHA1(8bc4dfcd5bd56422e303c73d50c2e7afa2edef5a) )
ROM_END

ROM_START( sc1clbrn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubrunner.bin", 0x00000, 0x10000, CRC(32b2d57b) SHA1(26523518bfb726d55d6808451f4041756f99b1d9) )
ROM_END

ROM_START( sc1clbsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rveclsp0108_1.bin", 0x0000, 0x010000, CRC(d60c9f4b) SHA1(dcbb6a10db2f658b734ed0fdecf907a4a32eedaa) )
ROM_END





ROM_START( sc1copdd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "game_835.bin", 0x00000, 0x10000, CRC(af134088) SHA1(c6467102903a2910c67f2b8051e1f788576ef62f) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "snd1ic7_519.bin", 0x00000, 0x10000, CRC(5cd39b04) SHA1(83cc51c208e8a9d3ccd0b4fcd2ab74a5f71e0c28) )
	ROM_LOAD( "snd2ic8_520.bin", 0x00000, 0x10000, CRC(a22621ec) SHA1(add91e6b1e14118c718614a7cfaa2d3aabbf01b3) )
ROM_END

ROM_START( sc1cops )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cop56cp2", 0x0000, 0x008000, CRC(c862ee34) SHA1(e807d1072953e67581ce0181bfd82a7efcee7bf0) )
	ROM_LOAD( "cop56cp1", 0x8000, 0x008000, CRC(214edd7d) SHA1(007c17cc522c8f0d30bc1fd08bb18850344f62ad) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "copssnd.bin", 0x0000, 0x040000, CRC(4bebbc37) SHA1(10eb8542a9de35efc0f75b532c94e1b3e0d21e47) )
ROM_END

ROM_START( sc1copsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops & robbers 5p v1-3 b (27256)", 0x0000, 0x008000, CRC(6f5425d6) SHA1(7673841ccfe16eaa0a5cfca1596383f7711f2dbe) )
	ROM_LOAD( "cops & robbers 5p v1-3 a (27256)", 0x8000, 0x008000, CRC(29513083) SHA1(f2ce0b573d6756e7d835488b8d8eed3266787255) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "copssnd.bin", 0x0000, 0x040000, CRC(4bebbc37) SHA1(10eb8542a9de35efc0f75b532c94e1b3e0d21e47) )
ROM_END

ROM_START( sc1copdx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops & robbers deluxe 5p-10p 6 p2 95840220 (27256)", 0x0000, 0x8000, CRC(32a22682) SHA1(c173688ace476a2ada398d5e7b5dfed5306e3c50) )
	ROM_LOAD( "cops & robbers deluxe 5p-10p 6 p1 95840219 (27256)", 0x8000, 0x8000, CRC(47867f55) SHA1(33f879a8e1e4e2f53b5da8b4ee597bd3870c75d1) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "cops & robbers deluxe sound 1 295 (27512)", 0x00000, 0x10000, CRC(81227c21) SHA1(6af8e15f8405fdfbaa3a8853ec7ec62fe5ec34ae) )
	ROM_LOAD( "cops & robbers deluxe sound 2 296 (27512)", 0x10000, 0x10000, CRC(8ecf1f5e) SHA1(4159b5c3800708cde94ce62a5e07b58ad8aaedf8) )
ROM_END







ROM_START( sc1dago )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dgnlg051_single_site_euro.bin", 0x0000, 0x010000, CRC(5ccb9773) SHA1(4940d7b17d36d409504a263acd54d017c6cbb1e1) )
ROM_END


ROM_START( sc1disc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "disc-88.b", 0x0000, 0x008000, CRC(f6e2d800) SHA1(a0c7ab0c913d9284cdbfa1d35b62afefb903c086) )
	ROM_LOAD( "disc-88.a", 0x8000, 0x008000, CRC(1ac052d0) SHA1(a37cc2896fb884af7e922289d7fda1e7d26fc387) )
ROM_END





ROM_START( sc1dream )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95752021.bin", 0x0000, 0x010000, CRC(ea7d1cec) SHA1(d277b639575498c458f98e0e1a629d914ca36cfe) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "95000604.bin", 0x00000, 0x10000, CRC(7c2f52ed) SHA1(d435402459efc9311707ac691992874b56cbbeec) )
	ROM_LOAD( "95000605.bin", 0x00000, 0x10000, CRC(23b99731) SHA1(7cc1c51d9b72480d8a1020fc3621a05ba83d7629) )
ROM_END


ROM_START( sc1final )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "finaltouch_prg.bin", 0x00000, 0x10000, CRC(27a74fbb) SHA1(8c3e76c67605866acf8e6e28b14788a5cbcd43b4) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "finaltouch_snd1.bin", 0x00000, 0x10000, CRC(18ebcdd2) SHA1(17efd903d205d7285f642017de8b5799ede2110b) )
	ROM_LOAD( "finaltouch_snd2.bin", 0x10000, 0x10000, CRC(75a76a6a) SHA1(a660285d56517876414dc951e98185ea14e8fb4e) )
ROM_END



ROM_START( sc1flash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flash.bin", 0x00000, 0x10000, CRC(42475bd9) SHA1(634759e64ecc001da5eca01b89e5b93749de541d) )
ROM_END



ROM_START( sc1fruit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruitlnsb.bin", 0x0000, 0x8000, CRC(9a44bfdc) SHA1(cd7890b781411b1fdf8abe17e3337a92b40596c7) )
	ROM_LOAD( "fruitlnsa.bin", 0x8000, 0x8000, CRC(c002dde4) SHA1(7f7601108975f09ed5846d8acf90a5db36319bbd) )
ROM_END



ROM_START( sc1frtln )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruitlines.bin", 0x0000, 0x010000, CRC(b26f8c8f) SHA1(f0384046e52fcf5fe5eabb7a155b119725f3cdd9) )
ROM_END






ROM_START( sc1gtime )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goodtime.bin", 0x00000, 0x10000, CRC(9958dc86) SHA1(43221d0eb50ebe3db8b1d1e784e19b5cbb86c24c) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "goodtimesound1.bin", 0x00000, 0x10000, CRC(554d1157) SHA1(ae802338b40a0b35dcdf788c19ef42c2ed7e9a37) )
	ROM_LOAD( "goodtimesound2.bin", 0x10000, 0x10000, CRC(e6c53e20) SHA1(30cb83d03fe873b4ec822d3aa1001b7fed9571ff) )
ROM_END

ROM_START( sc1tiara )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tiara_prg1u.bin", 0x00000, 0x10000, CRC(963fc838) SHA1(375bc2fb72c89095d1afae77762e94d7adb79133) )

	ROM_REGION( 0x40000, "upd", 0 ) // same sound roms as good times?
	ROM_LOAD( "tiara_snd1.bin", 0x00000, 0x10000, CRC(554d1157) SHA1(ae802338b40a0b35dcdf788c19ef42c2ed7e9a37) )
	ROM_LOAD( "tiara_snd2.bin", 0x00000, 0x10000, CRC(e6c53e20) SHA1(30cb83d03fe873b4ec822d3aa1001b7fed9571ff) )
ROM_END



ROM_START( sc1gprix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gp.bin", 0x00000, 0x10000, CRC(35cfe52c) SHA1(5debd45553e91d2aab102c5a712f912efdd6ada3) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "grandprix_snd1.bin", 0x00000, 0x10000, CRC(86139b6a) SHA1(13b9483f7379e3cc25f5474fa950878e0a2853d2) )
	ROM_LOAD( "grandprix_snd2.bin", 0x10000, 0x10000, CRC(f1a91ced) SHA1(97e3f03b7eac975ff9dd4e0f10eb18314c36f201) )
ROM_END




ROM_START( sc1gslam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gs.bin", 0x00000, 0x10000, CRC(7a239eef) SHA1(5af894dd2df7256c9347b46a5aabd93961c83324) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "gslsnd1.bin", 0x00000, 0x10000, CRC(52ca000a) SHA1(a8d4cedf02fae8bb24ea8cf1f62dace49c773858) )
	ROM_LOAD( "gslsnd2.bin", 0x10000, 0x10000, CRC(cb52721e) SHA1(395024a425f057f78a8d83cdbbbc9bf1521f3597) )
ROM_END




ROM_START( sc1happy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "happyhourromd.bin", 0x00000, 0x10000, CRC(7a03df4f) SHA1(78dbadd4acc3ac7d06e2bc8bf9be080e4cd888fb) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "happyhoursnd1.bin", 0x00000, 0x10000, CRC(a7b84f42) SHA1(bfe0b4f7b1c6c55d4fa45ac26f95a045cc21313e) )
	ROM_LOAD( "happyhoursnd2.bin", 0x10000, 0x10000, CRC(e90ffa86) SHA1(8b4f68e3f010854e13abd689db0961092d2dc491) )
ROM_END




ROM_START( sc1impc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "impact.bin", 0x00000, 0x10000, CRC(dd5d94d4) SHA1(1674ec497daa7dd61412a07ebca3447b69c5780e) )
ROM_END



ROM_START( sc1kings )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kingsclub.bin", 0x00000, 0x10000, CRC(6f547e05) SHA1(e52872ab94e6bdcb8aa131db6f21535b78cf53ef) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "kings279.bin", 0x00000, 0x10000, CRC(8388d112) SHA1(fa31f001011fb463d7cffe88b7cd994ceb3a6977) )
	ROM_LOAD( "kings280.bin", 0x10000, 0x10000, CRC(5566f2bd) SHA1(a49b7a25cf3a008c78dc59c08aaccb6e0e1e480f) )
ROM_END




ROM_START( sc1linx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95710138 b.bin", 0x0000, 0x8000, CRC(e731dc61) SHA1(c65b2c7006e58e924370261bdb5ac3f5e3e86471) )
	ROM_LOAD( "95710137 a.bin", 0x8000, 0x8000, CRC(29c6b8b1) SHA1(643d06a5064ba74902bfbe115b1bd7b1abe14381) )
ROM_END

ROM_START( sc1linxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "linx 20p std 95717316 b.bin", 0x0000, 0x8000, CRC(419b3d7e) SHA1(83b27914cb95afc1053578a279dc936181562217) )
	ROM_LOAD( "linx 20p n.p.a 95717315.bin", 0x8000, 0x8000, CRC(ea53e0e1) SHA1(d8d57d44188a33e2751bfc4f21249efc32815877) )
ROM_END

ROM_START( sc1linxp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "linx var% data 10p b.bin", 0x0000, 0x8000, CRC(c23fc39c) SHA1(4e6d2a16606544c00bd175ade4d9e6491ec317ff) )
	ROM_LOAD( "linx 10p a.bin", 0x8000, 0x8000, CRC(87d8907c) SHA1(3584441870b0a57284e831b0e68422fa3138b4bf) )
ROM_END



ROM_START( sc1magc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc.bin", 0x8000, 0x008000, CRC(75cd57c4) SHA1(94409fdf206ebe071fd58bc175622c2bfa439299) )
ROM_END




ROM_START( sc1manha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "manb.bin", 0x0000, 0x8000, CRC(a0e73800) SHA1(bb56f2aa211ff48e5d4d8bfdff4fc1c7464e01ca) )
	ROM_LOAD( "mana.bin", 0x8000, 0x8000, CRC(961dc746) SHA1(98ecfce91f8d111b38d9e658b50bfd921e567a68) )
ROM_END




ROM_START( sc1mast )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "masterclubgame.bin", 0x00000, 0x10000, CRC(1156383a) SHA1(eb93fae25b1083bfd343015bcbc33f029571b700) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "asnd1.bin", 0x00000, 0x10000, CRC(229d0666) SHA1(68650c920d60df1eff00cd77e0308f5c2fd88baf) )
	ROM_LOAD( "asnd2.bin", 0x10000, 0x10000, CRC(3b286391) SHA1(0e0cd818d23d73b681905db98c0b9890809b25f6) )
ROM_END




ROM_START( sc1mist )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mistral.bin", 0x00000, 0x10000, CRC(7da31d22) SHA1(a63cd098d66af869d3967b15694b6d6ba8cc8d1e) )
ROM_END




ROM_START( sc1olym )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "olympia.bin", 0x00000, 0x10000, CRC(15728d0a) SHA1(addf84f0efec140eecad48116a84c36662a85db2) )
ROM_END




ROM_START( sc1orac )
	ROM_REGION( 0x10000, "maincpu", 0 )//Is this the right way round? Goes against other labels...
	ROM_LOAD( "oracle_a.bin", 0x0000, 0x8000, CRC(2177f249) SHA1(5144819a8934734b5de9d56384ae89d015b8acee) )
	ROM_LOAD( "oracle_b.bin", 0x8000, 0x8000, CRC(93f38e60) SHA1(741402d0f25b59a9d651875bf5ccbc06389b1ea9) )
ROM_END




ROM_START( sc1pwrl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "powlineb.bin", 0x0000, 0x8000, CRC(9d13e39e) SHA1(2df1f402fb49aacc3fc1fecdf536ea1dcee5521f) )
	ROM_LOAD( "powlinea.bin", 0x8000, 0x8000, CRC(6d03d6ce) SHA1(4a932b87e44e37fed44ff80da542228f2d4b9876) )

	sc1_pwrl_sound
ROM_END






ROM_START( sc1quat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "quatro.bin", 0x00000, 0x10000, CRC(c264c520) SHA1(469e0b394061ae4dcf9b0a2c66c6b85404113f5f) )
ROM_END




ROM_START( sc1rain )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rainbow1.bin", 0x8000, 0x8000, CRC(cdc20fc9) SHA1(306f9877c59b4cfb0653e1f453ef188c93b7b4d3) )
	ROM_LOAD( "rainbow2.bin", 0x0000, 0x8000, CRC(1adf16b0) SHA1(90d0935ec3a0803e1f7fcf8be24cce36f3a53962) )
ROM_END




ROM_START( sc1re )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reelcashd.bin", 0x00000, 0x10000, CRC(2519c32f) SHA1(b371dbddad617a6d749e1b784cba11758e3b37b8) )
ROM_END




ROM_START( sc1rese )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reelcashse.bin", 0x00000, 0x10000, CRC(db24d5aa) SHA1(3e673b3652899d2e7e65554ffdfaca67cf3b02bf) )
ROM_END




ROM_START( sc1revo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "revolution.bin", 0x8000, 0x8000, CRC(d477e4ab) SHA1(01614f9009f1736d1f1c5f2ddea48cf92fd66b0e) )
ROM_END





ROM_START( sc1rose )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rosecrow.802", 0x00000, 0x10000, CRC(b4cb3517) SHA1(d19e0cc8da5da7d1bcde174cf68cf7d9230cd53d) )
ROM_END



ROM_START( sc1sant )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "santana.bin", 0x00000, 0x10000, CRC(debd19fc) SHA1(e4394c014c5db621647dd54aa7d434705431750c) )
ROM_END




ROM_START( sc1sat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sat.bin", 0x8000, 0x8000, CRC(5e1843db) SHA1(14cef347b5409ded4e52ae60fc4990dc79bfbae3) )
ROM_END




ROM_START( sc1shan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sha626b.dat", 0x0000, 0x8000, CRC(1df2ca25) SHA1(c960a5e536a3fe1c868ae7f0f9983e7f77f61a2a) )
	ROM_LOAD( "sha626a.dat", 0x8000, 0x8000, CRC(ea770c35) SHA1(247cec799c439d11d739a7a6f2d1c0cdc7b61e18) )
ROM_END





ROM_START( sc1spct )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spectre_2.bin", 0x0000, 0x008000, CRC(7edc2788) SHA1(8336166151a89f9df5735e969d376375059b0024) )
	ROM_LOAD( "spectre_1.bin", 0x8000, 0x008000, CRC(2dadb250) SHA1(d648678864e482bedd27008b50c3bfe50553f0c2) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "spectre_snd_1.bin", 0x000000, 0x010000, CRC(ecdf085b) SHA1(117c63f7672112308bfe64527148ee66f8c26c12) )
	ROM_LOAD( "spectre_snd_2.bin", 0x010000, 0x010000, CRC(55087557) SHA1(a3f2613a27defa547f8c2e46ee0cdf9ee18678be) )
ROM_END

ROM_START( sc1spcta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spec5pa", 0x0000, 0x010000, BAD_DUMP CRC(65fa549c) SHA1(68fd5a11eb89088f87a727e9c3bb621a4235adf4) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "spectre_snd_1.bin", 0x000000, 0x010000, CRC(ecdf085b) SHA1(117c63f7672112308bfe64527148ee66f8c26c12) )
	ROM_LOAD( "spectre_snd_2.bin", 0x010000, 0x010000, CRC(55087557) SHA1(a3f2613a27defa547f8c2e46ee0cdf9ee18678be) )
ROM_END

ROM_START( sc1spit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spitfire.bin", 0x0000, 0x010000, CRC(557cdd61) SHA1(1c4a6c969569267e61119b2cd9e506d948c35517) )
ROM_END



ROM_START( sc1ster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sterling.bin", 0x0000, 0x010000, CRC(a6e68f9a) SHA1(d49c4a0c6ab78f369217cc06f82a847db4f208b9) )
ROM_END



ROM_START( sc1str4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strike4.ts", 0x8000, 0x8000, CRC(c636f698) SHA1(7373ad663966e51dd1a0737a447bd61e07cd16e2) )
ROM_END

ROM_START( sc1str4a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strike4.bin", 0x8000, 0x8000, CRC(1abcfb49) SHA1(f13891a38e260a72ffe841862ed73532c94f6c44) )
ROM_END



ROM_START( sc1sups )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_prg.bin", 0x0000, 0x010000, CRC(e859e20a) SHA1(b72ace14ceb0e601c8284a1b654a3e49368644b9) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "superstar_snd1.bin", 0x000000, 0x010000, CRC(639467fb) SHA1(b40563735fa8053350d4a6eca9cc00cbfe2d2c11) )
	ROM_LOAD( "superstar_snd2.bin", 0x010000, 0x010000, CRC(29157c5c) SHA1(ccece847979358626819d6f265cd3eb932b5a400) )
ROM_END



ROM_START( sc1torn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tor930b", 0x0000, 0x8000, CRC(f6181dd5) SHA1(44b12e6f66bf45e2b2a91424941b10ea5e75428f) )
	ROM_LOAD( "tor930a", 0x8000, 0x8000, CRC(c645212d) SHA1(0cb0a6f15b22e3174a1600fe15a742d5f63d9ab2) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "torsnd1.bin", 0x0000, 0x010000, CRC(713ae672) SHA1(a6038004da7a4907eb413b5f39a00d7e131a2382) )
	ROM_LOAD( "torsnd2.bin", 0x010000, 0x010000, CRC(187f0c17) SHA1(acc8cffc91f8a92257bfd87ee8dc809139dc5301) )
ROM_END

ROM_START( sc1torna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tornado.792", 0x0000, 0x010000, CRC(7e8e8ad1) SHA1(0e093b81f4ab3d202f89215b26b360aac7f32218) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "torsnd1.bin", 0x0000, 0x010000, CRC(713ae672) SHA1(a6038004da7a4907eb413b5f39a00d7e131a2382) )
	ROM_LOAD( "torsnd2.bin", 0x010000, 0x010000, CRC(187f0c17) SHA1(acc8cffc91f8a92257bfd87ee8dc809139dc5301) )
ROM_END



ROM_START( sc1typ )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-typhoon_std_ac_rot_bss.bin", 0x0000, 0x8000, CRC(0f3e160d) SHA1(3be936fe288b23e2f35be7d5638894776d676c11) )
	ROM_LOAD( "club-typhoon_std_ac_rot_ass.bin", 0x8000, 0x8000, CRC(5d6819b2) SHA1(14cc0b3b5f42f4ff92ff96629737b9e75bb0ea10) )

	ROM_REGION( 0x40000, "xxxx", 0 )//Don't decode as Intel Hex, what are they?
	ROM_LOAD( "club-typhoon_snd_a_(inhex)ss.hex", 0x0000, 0x026efc, CRC(c913008a) SHA1(9b75a40670db0fbe8a0f6fc54784d3b415a975f5) )
	ROM_LOAD( "club-typhoon_snd_b_(inhex)ss.hex", 0x0000, 0x023972, CRC(2106a5f1) SHA1(17e0f24c4e9a8ba227c5a6ec63bcba3d8796f7f7) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "club-typhoon_snd_a.bin", 0x00000, 0x10000, CRC(ffec0dde) SHA1(a8c66a6ebb4d805e04d7eb7d1fe2ecd90e7eee54) )
	ROM_LOAD( "club-typhoon_snd_b.bin", 0x10000, 0x10000, CRC(52e36599) SHA1(4bc003a08e666f9e1abfe00e82bb43a33009b6f2) )
ROM_END

ROM_START( sc1typp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-typhoon_dat_ac_rot_bss.bin", 0x0000, 0x8000, CRC(0f3e160d) SHA1(3be936fe288b23e2f35be7d5638894776d676c11) )
	ROM_LOAD( "club-typhoon_dat_ac_rot_ass.bin", 0x8000, 0x8000, CRC(3a67d55e) SHA1(ce75e5c07795b3c67f234a869efb78fbf22b76c2) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "club-typhoon_snd_a.bin", 0x00000, 0x10000, CRC(ffec0dde) SHA1(a8c66a6ebb4d805e04d7eb7d1fe2ecd90e7eee54) )
	ROM_LOAD( "club-typhoon_snd_b.bin", 0x10000, 0x10000, CRC(52e36599) SHA1(4bc003a08e666f9e1abfe00e82bb43a33009b6f2) )

ROM_END

ROM_START( sc1ult )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ultimate.bin", 0x00000, 0x010000, CRC(66c34ef8) SHA1(2c7e2e826f6bd7a31cb3432dc74ebe382c131225) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "ult1.bin", 0x00000, 0x10000, CRC(7c2f52ed) SHA1(d435402459efc9311707ac691992874b56cbbeec) )
	ROM_LOAD( "ult2.bin", 0x00000, 0x10000, CRC(23b99731) SHA1(7cc1c51d9b72480d8a1020fc3621a05ba83d7629) )
ROM_END



ROM_START( sc1vent )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ventura2.bin", 0x0000, 0x8000, CRC(3c396285) SHA1(a9cb8b54ace1d228a0d365909836bc2b02db1931) )
	ROM_LOAD( "ventura1.bin", 0x8000, 0x8000, CRC(7c05f39e) SHA1(84793abbffbc345bf08873ddd3185bffd8fc95df) )
ROM_END



ROM_START( sc1vict )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vic2.bin", 0x0000, 0x8000, CRC(9eab3510) SHA1(29ba3445c75a0dcdf325312fbc64e8911ba958c3) )
	ROM_LOAD( "vic1.bin", 0x8000, 0x8000, CRC(2cfbdc26) SHA1(45e492f4cba1cb90e0670fbe8f4fcd0440414316) )

	ROM_REGION( 0x10000, "xxx", 0 )
	ROM_LOAD( "pal.bin", 0x0000, 0x000010, CRC(d33fb7d2) SHA1(6de1a205808bccb9bc86f630c0eda261041a3b00) )
ROM_END




ROM_START( sc1voy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "voyager_042_arcade.bin", 0x00000, 0x10000, CRC(7db87ef9) SHA1(e2160457a862d6eba3d8348866429043df0ed2bb) )
ROM_END

ROM_START( sc1voya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "voyager_052_single_site_euro.bin", 0x00000, 0x10000, CRC(a9042f9e) SHA1(0469ef2d2a2f9c7c4147ee8d528ec369bf943103) )
ROM_END



ROM_START( sc1winfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "winfalls_a.bin", 0x0000, 0x8000, CRC(4617ec80) SHA1(8ac5a47d2ae94c2869cc6645f01cfe9d880b1e5c) )
	ROM_LOAD( "winfalls_b.bin", 0x8000, 0x8000, CRC(7498ede7) SHA1(5fb66c39865ea963fb7eeb9d4813cfa5e68f709e) )
ROM_END



ROM_START( sc1winst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717110 var% b.bin", 0x0000, 0x8000, CRC(1c2ebd26) SHA1(462baa4df7c01d101798df1d90bb5719cdc9647e) )
	ROM_LOAD( "95717109 var% a.bin", 0x8000, 0x8000, CRC(f8b03a06) SHA1(b919366b432d23fd9f0c986e112650048621d7b2) )

	sc1_winst_sound
ROM_END


ROM_START( sc1winstp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717110 var% b.bin", 0x0000, 0x8000, CRC(1c2ebd26) SHA1(462baa4df7c01d101798df1d90bb5719cdc9647e) )
	ROM_LOAD( "95718109 proto var% a.bin", 0x8000, 0x8000, CRC(05d5ad4a) SHA1(5e165499601978e88159726f83310576216853c4) )

	sc1_winst_sound
ROM_END

ROM_START( sc1winsta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95719110b ws 78%.bin", 0x0000, 0x8000, BAD_DUMP CRC(5871aad0) SHA1(6677c94b74a2e2dcece3fdcd730fbc8034833a7d) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "95719109a ws 78%.bin", 0x8000, 0x8000, BAD_DUMP CRC(cea7ff32) SHA1(ce20742bcad1eea450affab81822cfdaaf927984) ) // 1ST AND 2ND HALF IDENTICAL

	sc1_winst_sound
ROM_END


ROM_START( sc1zep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "zepp1.bin", 0x0000, 0x8000, CRC(bfbbbc35) SHA1(5c28b6359d79c96d53319408fbc2d7cb2629185d) )
	ROM_LOAD( "zepp.bin", 0x8000, 0x8000, CRC(fbc38903) SHA1(9eefef9bbde263e35a98e51b7aeb8d2348d36c06) )
ROM_END

ROM_START( sc1wthn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wildthingprogamm1.bin", 0x00000, 0x010000, CRC(80157a9c) SHA1(ec8e217e17ac7f4c5bc05d9848bf5f37b2d82fac) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "wildthingsound1.bin", 0x00000, 0x10000, CRC(85389209) SHA1(029dda285b035525b730b4c72ff182554f5dbe47) )
	ROM_LOAD( "wildthingsound2.bin", 0x10000, 0x10000, CRC(664ab695) SHA1(d4148ebffbe41eb1d265548991ad3cb984205497) )
ROM_END


ROM_START( sc1days )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "all2-5n.p2", 0x0000, 0x8000, CRC(fa75d835) SHA1(78e6b48bea8f1297530f08dff6bada4d228e090d) )
	ROM_LOAD( "all2-5n.p1", 0x8000, 0x8000, CRC(7d58a415) SHA1(8bd0d23ac825ba0294f2fd26e9acb87eb1f3d10c) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END

ROM_START( sc1daysa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "allinv1-6b.bin", 0x0000, 0x8000, CRC(59589a00) SHA1(c73b45f383f908d1257f6d031f359f73e5b2f966) )
	ROM_LOAD( "allinv1-6a.bin", 0x8000, 0x8000, CRC(36a83181) SHA1(a2cb6493efb00e9bcf76388f65098af9346f855e) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END


ROM_START( sc1cscl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cldl1-8n.p2", 0x0000, 0x8000, CRC(8feee244) SHA1(50c7eab298078def5d82bc3bebbe3e08b612bc47) )
	ROM_LOAD( "cldl1-8n.p1", 0x8000, 0x8000, CRC(4cb5239d) SHA1(a0f22440a5453ea28093f32856ab5417a6c82037) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END

ROM_START( sc1cscla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clas2-0n.p2", 0x0000, 0x8000, CRC(45d40f1e) SHA1(03388a8ea809b088850865cb288af3181d3dd962) )
	ROM_LOAD( "clas2-0n.p1", 0x8000, 0x8000, CRC(ebd514b1) SHA1(5267b49de98f8a93ac206f68d56ee12e1d228a7d) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END




ROM_START( sc1driv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ddslb1-3.p2", 0x0000, 0x8000, CRC(32896702) SHA1(1ef36daca6bf3f45dfff5edc401bdbd313ad9121) )
	ROM_LOAD( "ddslb1-3.p1", 0x8000, 0x8000, CRC(81fc84a7) SHA1(f0d5a181d4ca027df2c5ca11573eb7687b3abf29) )

	sc1_driv_sound
ROM_END

ROM_START( sc1driva )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ddsnl1-3.p2", 0x0000, 0x8000, CRC(a5b663c8) SHA1(5b6675874ff4e3a5c74dbd66c4a47c34d36f1222) )
	ROM_LOAD( "ddsnl1-3.p1", 0x8000, 0x8000, CRC(96f8bc52) SHA1(de0d180d4640eef451984f466be8732d0a08cee8) )

	sc1_driv_sound
ROM_END

ROM_START( sc1drivb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dslb1-5.p2", 0x0000, 0x8000, CRC(193e6aaa) SHA1(f083747a9cad72690b01181cc46ae7bdc3de6ea6) )
	ROM_LOAD( "dslb1-5.p1", 0x8000, 0x8000, CRC(6adaf17b) SHA1(8930daac71fbe3f7eb91358d7101f2b8d05d224e) )

	sc1_driv_sound
ROM_END

ROM_START( sc1drivc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dsnl1-6.p2", 0x0000, 0x8000, CRC(0eb10c01) SHA1(16456ec1e32bfbd873bdebd6a760041bc9cd8648) )
	ROM_LOAD( "dsnl1-6.p1", 0x8000, 0x8000, CRC(174c4432) SHA1(82519ede8220d3d717ee0ebe57374357afe38949) )

	sc1_driv_sound
ROM_END

ROM_START( sc1vsd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supd1-4n.p1", 0x8000, 0x8000, CRC(ad581f7d) SHA1(99b9bf1016cd52467f5c9f6e427305e81033e82f) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END


ROM_START( sc1moonl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "moon lite 5p 86var v5.1.bin", 0x8000, 0x8000, CRC(31db928a) SHA1(0e07c11bf85a13df62bb704a03a42712d6e7ff62) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1ltdv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "little devil v4 arcade.bin", 0x8000, 0x8000, CRC(ff32cdcf) SHA1(84bb86e30ace57aa8f591a3778801d44fb3f8fe1) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1t1k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "top1000a.bin", 0x0000, 0x8000, CRC(c986ee8b) SHA1(e5a600942e725d0ad6be10fbac7fb05eb0d2b07f) )
	ROM_LOAD( "top1000b.bin", 0x8000, 0x8000, CRC(0124b7c0) SHA1(620196e7580f44423ede6644f76e37091fdf30b6) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

ROM_START( sc1dip )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dip0111.bin", 0x00000, 0x10000, CRC(19632509) SHA1(69c9947da11892b99e9936675d0b1bdabdc16ae8) )

	ROM_REGION( 0x200000, "ram", ROMREGION_ERASE00 ) // is this just some default settings?
	ROM_LOAD( "ram.bin", 0x0000, 0x2000, CRC(3962d8cf) SHA1(b893a92d467e8f5ffc2cffa8a7121d92fe2492eb) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

ROM_START( sc1lamb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lambada.bin", 0x00000, 0x10000, CRC(4321495c) SHA1(d3ef15d2a1b2c7aec33ac226c89a7a0c0a18884a) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

ROM_START( sc1reply )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "repl0110.bin", 0x00000, 0x10000, CRC(b2bfa2fb) SHA1(9c704321428c05f97593ea7541ba1a08ff448571) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "repl0110snd.bin", 0x00000, 0x10000, CRC(86547dc7) SHA1(4bf64f22e84c0ee82d961b0ba64932b8bf6a521f) )
ROM_END


ROM_START( sc1smoke )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sb6-3_0b.bin", 0x0000, 0x8000, CRC(31647e2f) SHA1(35dd1bef0dd72fd45c063f181cd190f8d21df207) )
	ROM_LOAD( "sb6-3_0a.bin", 0x8000, 0x8000, CRC(45ca0067) SHA1(be1947d055320c101ea75c669733b19d2f61a0f9) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "svb58pa", 0x0000, 0x008000, CRC(4496ce3d) SHA1(400dee4249fd930473cb003d85b25bb991041bc6) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1smokea )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sb8-5_0b.bin", 0x0000, 0x8000, CRC(0aac6b91) SHA1(85b4dfe15d456b7d808295c890264163bc6115f1) )
	ROM_LOAD( "sb8-5_0a.bin", 0x8000, 0x8000, CRC(eafe5fac) SHA1(4798a37ada523d078f2e10976c5f90cccab1c406) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1ccroc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc6-4_0b.bin", 0x0000, 0x8000, CRC(4093cbe6) SHA1(6ea678d0e288bb075d58ef72089ae387d6285477) )
	ROM_LOAD( "cc6-4_0a.bin", 0x8000, 0x8000, CRC(c3d963e8) SHA1(35688841e102c264124c23de526417db618ea898) )

	ROM_REGION( 0x80000, "altrevs", 0 ) //this can't be the right hardware
	ROM_LOAD( "cs1_1.rom", 0x0000, 0x040000, CRC(f4c6f9f1) SHA1(4277ff51dc91c35d4c6e9ab1c16e087ef7e8d140) )
	ROM_LOAD( "cs1_2.rom", 0x0000, 0x040000, CRC(ba4dad49) SHA1(795342d5fd3deaa058a20d491206c028c529fd55) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1ccroca )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc8-7_0b.bin", 0x0000, 0x8000, CRC(a638c58a) SHA1(0a5d53a9c0f772263c7a726f90943a1ccfe5db20) )
	ROM_LOAD( "cc8-7_0a.bin", 0x8000, 0x8000, CRC(4a6cd887) SHA1(27f394a63bdb68d35d6eecb6b0f6b3f3f61d36b5) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1ccrocb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cct-8_0b.bin", 0x0000, 0x8000, CRC(1e0e93f3) SHA1(e5ceef529bd406d2b395b6e24cff370422b0e1f2) )
	ROM_LOAD( "cct-8_0a.bin", 0x8000, 0x8000, CRC(a90a5f23) SHA1(befb389cc6ff045462f02c2aa9025d92c47da0fa) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1ccrocc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "crocs6b.bin", 0x0000, 0x8000, CRC(42ad6fb0) SHA1(d60961d9993a8458668177013d7561d0b7423cda) )
	ROM_LOAD( "crocs6a.bin", 0x8000, 0x8000, CRC(087330cb) SHA1(f143a8a44024f0f851a8b677f42b9d4011ab92d4) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1crocr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "croc58pb", 0x0000, 0x10000, CRC(95d2b0ac) SHA1(369a2f5efc981aa03780b80e0b14d5171c25e72b) ) // 2nd half empty
	ROM_LOAD( "croc58pa", 0x8000, 0x10000, CRC(39501e80) SHA1(f03bc602df839374adf7722af295cee562353782) ) // 2nd half empty

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1btclk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bc6-4_0b.bin", 0x0000, 0x8000, CRC(106265c8) SHA1(6465f7e868c5b04776fee69295a52197abb45ad0) )
	ROM_LOAD( "bc6-4_0a.bin", 0x8000, 0x8000, CRC(750645e7) SHA1(65eee2a00a1914bb8dc989b131eaa39d2881105d) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1btclka )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bc8-5_0b.bin", 0x0000, 0x8000, CRC(4be7220f) SHA1(5eb2b3fd05ff06b645f16bf95f6766b8bea82525) )
	ROM_LOAD( "bc8-5_0a.bin", 0x8000, 0x8000, CRC(f8fafc49) SHA1(7d8109fdabe37c7e958696512d3c2c35f9890bee) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1btclkb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "btc58pb", 0x0000, 0x10000, CRC(4cfde48a) SHA1(8567667f4af96fd00a807380a65fe809cd051c76) ) // 2nd half empty
	ROM_LOAD( "btc58pa", 0x8000, 0x10000, CRC(d21e5ed9) SHA1(99c189fde84f5abbdcd85d1f816c61f8fe72554e) ) // 2nd half empty

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1btbc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "beatb34", 0x8000, 0x8000, CRC(0791f889) SHA1(f090b9aacdbb33cc0934f53621e43520b970d789) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1boncl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cb_v91.bin", 0x8000, 0x8000, CRC(a1b902f4) SHA1(47bff5f0921800052ac99fd7b945ea05fc5951d6) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc1clins )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashlines5p2.40p2 9.3.90.bin", 0x0000, 0x8000, CRC(068959a2) SHA1(6c212ceb756024662ed880b66b4c6aac21b0c726) )
	ROM_LOAD( "cashlines5p2.40p1 9.3.90.bin", 0x8000, 0x8000, CRC(cb69c335) SHA1(8fe302274d01e98f8636fbc44eb4736180345b16) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1clinsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashlines2p2.40p2 9.3.90.bin", 0x0000, 0x8000, CRC(0a4d6692) SHA1(9437a0ed1fb9eb706dede7a6b1670e2bd873d7fe) )
	ROM_LOAD( "cashlines2p2.40p1 9.3.90.bin", 0x8000, 0x8000, CRC(4dcdfcd1) SHA1(38b67a2450ededd9cf27b9f5d5fffe45f4e4b80d) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clinsb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717067b.bin", 0x0000, 0x8000, CRC(fca396e1) SHA1(3304a58a30fd0c79e8d1decd4bd8792d3acbad3e) )
	ROM_LOAD( "95718066a.bin", 0x8000, 0x8000, CRC(0977e287) SHA1(e937a3787d4cd056c5f9944bca1532b84ed335f6) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clinsc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "39370028b.bin", 0x0000, 0x8000, CRC(fca396e1) SHA1(3304a58a30fd0c79e8d1decd4bd8792d3acbad3e) )
	ROM_LOAD( "39370028a.bin", 0x8000, 0x8000, CRC(e0250ea4) SHA1(01cc9013c37bc22f5ab69565d453ece99f739e6b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clinsd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "957172.01 all cash.bin", 0x0000, 0x8000, CRC(27d941cf) SHA1(797d47c15d6a52f5647a566eb8ad1985324d81cb) )
	ROM_LOAD( "957172.00 all cash.bin", 0x8000, 0x8000, CRC(cafc2409) SHA1(125f7c1826e58619a53b56ecd4f5b0b7f607aeef) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clinse ) // bad? (SUMCHECK ERROR)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "957171.12 var% b.bin", 0x0000, 0x8000, CRC(80243558) SHA1(6b7cc811998d11397e5fa03a50154d165997ae7b) )
	ROM_LOAD( "957171.11 var% a.bin", 0x8000, 0x8000, CRC(1e74ef1a) SHA1(6c70f9b7f3caf6a5e9734b2e4ee74985c2b169d6) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1clb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cl3000.bin", 0x00000, 0x10000, CRC(998b58fa) SHA1(73b2837d6287667f16c64edada1e3ec5ffa54c74) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "95000593.ic7", 0x00000, 0x10000, CRC(743d4ecd) SHA1(23c2a3673d6b09bc829297751c283de444d32fa3) )
	ROM_LOAD( "95000594.ic8", 0x10000, 0x10000, CRC(9e143e49) SHA1(28547cc2f271f76a29d332f670e47a8bb836593e) )
ROM_END




ROM_START( sc1czbrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crazybreakp1.bin", 0x0000, 0x8000, CRC(47cbb5fd) SHA1(b5a7a20f9874f1010f7fc973d0cc5fcb87beaaf5) )
	ROM_LOAD( "crazybreakp2.bin", 0x8000, 0x8000, CRC(71bfb2fe) SHA1(3371421268a1e0a4518eafd27b2c23a0c7475e11) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1energ )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "energy_v109_u4.bin", 0x0000, 0x8000, CRC(bde92e45) SHA1(ae1b73ecd59131a11202487ecb4d34fc68e4101d) )
	ROM_LOAD( "energy_v109_u2.bin", 0x8000, 0x8000, CRC(ce5da71b) SHA1(c0cb687523bf7a8f42740dd3f54999eaa1db3cd0) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1frpus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750025.p1", 0x00000, 0x10000, CRC(75d21cbf) SHA1(8161dec9b0533383acc6172da564f1353e4367c1) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1frpusa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95752025.p1", 0x00000, 0x10000, BAD_DUMP CRC(0d223a7d) SHA1(7b110989b988f5fc57eac2b21b9f0cdb326174a0) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1hipt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "high point 6 b 20p.bin", 0x0000, 0x8000, CRC(535c1df1) SHA1(99d3033ee708c27134d461591eb7d19a573768d4) )
	ROM_LOAD( "high point 6 a 20p.bin", 0x8000, 0x8000, CRC(228c3eef) SHA1(c60da857fa5630809b072c20cf1f24ee26c38d0b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1hipta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "high point 70 6.bin", 0x0000, 0x8000, CRC(752b4c6e) SHA1(30503128ccca5c88e66174bd3e54b115eded1db6) )
	ROM_LOAD( "high point 69 6.bin", 0x8000, 0x8000, CRC(82564d75) SHA1(436b75e6617a6c2bb89ea0994696928b8452317d) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc1satse )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "satellitese.bin", 0x00000, 0x10000, CRC(de88d59c) SHA1(0df9ff2aa4be2634bc66e8f5539a7aa8c71b340a) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1strk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strike.bin", 0x00000, 0x10000, CRC(8bfae942) SHA1(325b74e3df527ad56e68b58b206fb3a491a44305) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "strikesoundp1.bin", 0x0000, 0x010000, CRC(bca5518c) SHA1(1b66e72e110702754eb3991f351cce689d6ad41c) )
	ROM_LOAD( "strikesoundp2.bin", 0x0000, 0x010000, CRC(50d6c506) SHA1(cb9851ebad21c0b14cf3d57159034a8660a32f74) )
ROM_END

ROM_START( sc1supfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superflushse.bin", 0x00000, 0x10000, CRC(50f890d1) SHA1(6edad44aaba069b2a3cc2bd16ed4cf383d6f7029) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1ofs56 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ofs56cp2", 0x0000, 0x8000, CRC(c3af2861) SHA1(4fe47355ea9431360f17ff4004a7529111aa1d50) )
	ROM_LOAD( "ofs56cp1", 0x8000, 0x8000, CRC(928c0a32) SHA1(1c83e497d62112850ff1607f9b20a12fe07a88cc) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc1wof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "frds1-4n.p2", 0x0000, 0x8000, CRC(60e56657) SHA1(4f02be663cfb36beeaa47be37fca7447d6ff9ebc) )
	ROM_LOAD( "frds1-4n.p1", 0x8000, 0x8000, CRC(add2f2f8) SHA1(6c9852493b5e13cc694deacb96fe6d04f49e5c30) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1wofa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wfor3-3n.p2", 0x0000, 0x8000, CRC(610f5700) SHA1(1752604a2ea3ac658d86b5a5baea03d67b8a6e99) )
	ROM_LOAD( "wfor3-3n.p1", 0x8000, 0x8000, CRC(a6bb27bd) SHA1(abe240ecb5ceee1012d0ff547380e2d122380efc) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1wofb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wof.bin", 0x00000, 0x10000, CRC(6aa9ccce) SHA1(d8781e225c97ccf2fd847ead1ae8e200358f8a96) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1crzyc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "craz1-5n.p1", 0x8000, 0x8000, CRC(943166ce) SHA1(9fbc97a1ede5ef18d5d5c544484b4a63f9a9901b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1crzyca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ccds1-2n.p1", 0x8000, 0x8000, CRC(62d75b51) SHA1(82c9a211e9465c04cbf7597481ee4fb3cbac9a94) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clbdy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ddpv1-1b.gg", 0x0000, 0x8000, CRC(692d4347) SHA1(6cdf3dbbaffe47fc026debaa74303d4ad36a5b63) )
	ROM_LOAD( "ddpv1-1a.gg", 0x8000, 0x8000, CRC(1614ee7b) SHA1(e777a122062f24a18fbe827371e359bbdd4298e7) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clbdya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dyn1-6n.p2", 0x0000, 0x8000, CRC(425b8cf6) SHA1(8b3dd294ff965103b5621da462b39629445456b9) )
	ROM_LOAD( "dyn1-6n.p1", 0x8000, 0x8000, CRC(9cb42e58) SHA1(bb92e7618efb9a95e96d55d6ee46ba4f08cb825b) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1chqfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cflg1-5n.p1", 0x8000, 0x8000, CRC(2337b8ed) SHA1(c27b3b91ca52dd7edb05743753b4510c05f29055) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1s1000 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super1000.bin", 0x8000, 0x8000, CRC(879e56e6) SHA1(5c0a08375a30213142e1d3835ea46462d882982d) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1cdm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dd13b.bin", 0x0000, 0x8000, CRC(7a663587) SHA1(6d03a34047ba5f995b1877fc4c0ab9703aa4defc) )
	ROM_LOAD( "dd13a.bin", 0x8000, 0x8000, CRC(e674bca9) SHA1(31481d791f3aaf1d4ba790924f0f9e4100a82da5) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "dd_snd1.bin", 0x00000, 0x10000, CRC(c00a70ab) SHA1(c0014b3e4308281203921994f41f19e0243148e0) )
	ROM_LOAD( "dd_snd2.bin", 0x10000, 0x10000, CRC(c03827f6) SHA1(16e844fb83d79d1e4fbb0069debaf71af5ad6814) )
ROM_END

ROM_START( sc1cdmp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dd13b.bin", 0x0000, 0x8000, CRC(7a663587) SHA1(6d03a34047ba5f995b1877fc4c0ab9703aa4defc) )
	ROM_LOAD( "dd13ap.bin", 0x8000, 0x8000, CRC(84a51666) SHA1(89cf10c7e732b5f77b798bf58fe8ebfc701da57b) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "dd_snd1.bin", 0x00000, 0x10000, CRC(c00a70ab) SHA1(c0014b3e4308281203921994f41f19e0243148e0) )
	ROM_LOAD( "dd_snd2.bin", 0x10000, 0x10000, CRC(c03827f6) SHA1(16e844fb83d79d1e4fbb0069debaf71af5ad6814) )
ROM_END

ROM_START( sc1hfcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cz14b.bin", 0x0000, 0x8000, CRC(976233ca) SHA1(554da440a0fe1d66fa95bef51ac168cec35d1636) )
	ROM_LOAD( "cz14a.bin", 0x8000, 0x8000, CRC(34324f0b) SHA1(946ff8fa40788748a0caabd48d125f2a4f9c36c3) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( sc1hfccp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cz14b.bin", 0x0000, 0x8000, CRC(976233ca) SHA1(554da440a0fe1d66fa95bef51ac168cec35d1636) )
	ROM_LOAD( "cz14ap.bin", 0x8000, 0x8000, CRC(56e3e5c4) SHA1(3017007e03139204732f7945ded61d35499055ac) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( sc1twice )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twiceasnice-arcstd.bin", 0x8000, 0x008000, CRC(4ba39f58) SHA1(185513023e0c87d926e0e821ed94f121182880c1) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc1chain )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717174.bin", 0x0000, 0x8000, CRC(6cdc8d15) SHA1(582e5e7bcefe0085917d3499b7c83e27c19662d2) )
	ROM_LOAD( "95717173.bin", 0x8000, 0x8000, CRC(4989e6c6) SHA1(17184c6a3624dfaa61bc4ddb3ac1813949eaf834) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( sc1chainp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717174.bin", 0x0000, 0x8000, CRC(6cdc8d15) SHA1(582e5e7bcefe0085917d3499b7c83e27c19662d2) )
	ROM_LOAD( "95716173 proto.bin", 0x8000, 0x8000, CRC(4f806f1d) SHA1(cfa8bcc2afbb47e549836d968c3390bef04c6c30) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END



// these mostly look like the same thing, and clearly have the BFM address scramble, but might be
// bad dumps / missing the first half (in all cases it's either 0xff or a mirror of the 2nd half)
// alternatively there might be an additional scramble
ROM_START( sc1scunk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "su_cashx", 0x0000, 0x010000, CRC(1ed97ef6) SHA1(1aaf911369dc814ee2edf5d59baa2961bfc73168) )
	ROM_LOAD( "s_nudge.p1", 0x0000, 0x010000, CRC(ca5fdbca) SHA1(60079aeb4904e42a4a45feb7f31cf6c71b611845) )
	ROM_LOAD( "s_ghost.p1", 0x0000, 0x010000, CRC(e1e63cfd) SHA1(1e966758eb890eb8515bd943e7f8077e2948e22c) )
	ROM_LOAD( "s.che", 0x0000, 0x010000, CRC(e285d761) SHA1(1d5aebebd41d388bc69777610dc3ee449e4a504e) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1wud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "car2-9n.p1", 0x008000, 0x008000, CRC(be523840) SHA1(a6b5b36cf8ee495c2271a879b28e4f388b9deba1) )
	ROM_LOAD( "car2-9n.p2", 0x000000, 0x008000, CRC(865c23f3) SHA1(e4e874cc003cb62012cdc741e163becfb29caa12) )
ROM_END

ROM_START( sc1goldw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "958402.11 10p no enrich.bin", 0x0000, 0x8000, BAD_DUMP CRC(00ed0ab4) SHA1(60e6a4abcf74ed705007cda699cdf8f52160a683) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "958402.12 10p no enrich.bin", 0x8000, 0x8000, BAD_DUMP CRC(51af0108) SHA1(e6333e2879f7b2b3b558b6909e177f3101f503e6) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

ROM_START( sc1druby )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95730028.bin", 0x8000, 0x8000, BAD_DUMP CRC(015f3760) SHA1(74dfd188f4a7ad057fda45a349e684be37a3f6bc) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

ROM_START( sc1drubya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95730068.bin", 0x8000, 0x8000, BAD_DUMP CRC(2bcbcf0d) SHA1(30dbb5ec3be34520ad89aedead42e1eda7841b63) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

#define GAME_FLAGS MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL

//Adder 2
GAME( 1996, m_tppokr        , 0         ,  scorpion1_adder2 , toppoker  , bfm_sc1_state, toppoker       , 0,       "BFM/ELAM",    "Top Poker (Dutch, Game Card 95-750-899)", MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING )


/********************************************************************************************************************************************************************************************************************
 Cash Attraction
  project numbers 5489 / 5602
  all sets pass ROM check and boot, pairings should be good
********************************************************************************************************************************************************************************************************************/

ROM_START( sc1cshata ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957272.21 74-78b.bin",    0x0000, 0x8000, CRC(531e97fb) SHA1(c7ae94c503f9e13d68ae463dd19212f146b0e8bc) ) ROM_LOAD( "957272.20 74-78 standard.bin", 0x8000, 0x8000, CRC(06def19d) SHA1(721d8ffc7e6b0e76f097d82b3be7618d97d73041) ) ROM_END
ROM_START( sc1cshati ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957272.21 74-78b.bin",    0x0000, 0x8000, CRC(531e97fb) SHA1(c7ae94c503f9e13d68ae463dd19212f146b0e8bc) ) ROM_LOAD( "957282.20 74-78 proto a.bin",  0x8000, 0x8000, CRC(7e557f21) SHA1(49bbbbafff757acd078d156bae2c942991f055af) ) ROM_END
ROM_START( sc1cshatc ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957172.21 var% b.bin",    0x0000, 0x8000, CRC(ea705443) SHA1(fdd941b5e6785d97e990f4ca74578e539512422b) ) ROM_LOAD( "957172.20 std var% a.bin",     0x8000, 0x8000, CRC(e67fc9e1) SHA1(39ac2c30d605f2b3109a57c6633a597e77651e79) ) ROM_END
ROM_START( sc1cshatf ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957172.21 var% b.bin",    0x0000, 0x8000, CRC(ea705443) SHA1(fdd941b5e6785d97e990f4ca74578e539512422b) ) ROM_LOAD( "957182.20 var% proto a.bin",   0x8000, 0x8000, CRC(3a2dd72d) SHA1(29d962702095aa0f252210da68a89c557fa9db69) ) ROM_END
ROM_START( sc1cshath ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957272.40 74-78b.bin",    0x0000, 0x8000, CRC(e72d4241) SHA1(487a00f49fa5451f39c2400f6f23a5f067afaa66) ) ROM_LOAD( "957182.39 proto var%.bin",     0x8000, 0x8000, CRC(43f452a7) SHA1(13ef94b4a4ecf729dfe481da26804f2e6f0631b0) ) ROM_END
ROM_START( sc1cshatg ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957172.40 b std var%.bin",0x0000, 0x8000, CRC(5e4381f9) SHA1(ae6d64c42ae7ddc2ed0ab5c3b56222090004d88a) ) ROM_LOAD( "957182.39 74-78 proto a.bin",  0x8000, 0x8000, CRC(f890b2d3) SHA1(e714973c63486e6983912fb6aebee3a71e003be5) ) ROM_END
ROM_START( sc1cshatb ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957172.71 20p std b.bin", 0x0000, 0x8000, CRC(79870574) SHA1(89e5db89064a9e24bc37389d78f4defb7d2f479b) ) ROM_LOAD( "957172.70 20 n.p a.bin",       0x8000, 0x8000, CRC(4e90868a) SHA1(f88a1b578b2d9091f5e5212768547db19e6b5379) ) ROM_END
ROM_START( sc1cshat )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "cash_attraction_b",       0x0000, 0x8000, CRC(79870574) SHA1(89e5db89064a9e24bc37389d78f4defb7d2f479b) ) ROM_LOAD( "cash_attraction_a",            0x8000, 0x8000, CRC(fab3283c) SHA1(669b425687faad0ebf88c1aaaafa40c446fa2e24) ) ROM_END

// PROJECT NUMBER 5489  CASH ATTRACTION - 18-JUL-1989 12:48:39
GAME( 198?, sc1cshata       , sc1cshat  , scorpion1         , scorpion1 , bfm_sc1_state, lotse      , 0,       "BFM",      "Cash Attraction (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-380-109
GAME( 198?, sc1cshati       , sc1cshat  , scorpion1         , scorpion1 , bfm_sc1_state, lotse      , 0,       "BFM",      "Cash Attraction (Bellfruit) (set 1, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-381-109
// PROJECT NUMBER 5489  CASH ATTRACTION VARIABLE % - 18-JUL-1989 14:33:44
GAME( 198?, sc1cshatc       , sc1cshat  , scorpion1         , scorpion1 , bfm_sc1_state, lotse      , 0,       "BFM",      "Cash Attraction (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-109
GAME( 198?, sc1cshatf       , sc1cshat  , scorpion1         , scorpion1 , bfm_sc1_state, lotse      , 0,       "BFM",      "Cash Attraction (Bellfruit) (set 2, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-109
// PROJECT NUMBER 5489  CASH ATTRACTION - 26-OCT-1989 16:31:38
GAME( 198?, sc1cshath       , sc1cshat  , scorpion1         , scorpion1 , bfm_sc1_state, lotse      , 0,       "BFM",      "Cash Attraction (Bellfruit) (set 3, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-381-119
// PROJECT NUMBER 5489  CASH ATTRACTION VARIABLE % - 26-OCT-1989 16:35:40
GAME( 198?, sc1cshatg       , sc1cshat  , scorpion1         , scorpion1 , bfm_sc1_state, lotse      , 0,       "BFM",      "Cash Attraction (Bellfruit) (set 4, Protocol) (Scorpion 1)", GAME_FLAGS ) //  GAME No 39-371-119
// PROJECT NUMBER 5602  CASH ATTRACTION 20P VARIABLE % - 3-JAN-1990 16:57:23
GAME( 198?, sc1cshatb       , sc1cshat  , scorpion1         , scorpion1 , bfm_sc1_state, lotse      , 0,       "BFM",      "Cash Attraction (Bellfruit) (set 5) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-132
GAME( 198?, sc1cshat        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse      , 0,       "BFM",      "Cash Attraction (Bellfruit) (set 5, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-132


/********************************************************************************************************************************************************************************************************************
 Club Attraction
  sc1clatta fails the rom check, likely one rom is bad
********************************************************************************************************************************************************************************************************************/

ROM_START( sc1clatt )  ROM_REGION( 0x10000, "maincpu", 0 )  ROM_LOAD( "39370196.2",           0x0000, 0x8000,          CRC(c809c22d) SHA1(fca7515bc84d432150ffe5e32fccc6aed458b8b0) ) ROM_LOAD( "39370196.1",           0x8000, 0x8000,          CRC(4c2e465f) SHA1(101939d37d9c033f6d1dfb83b4beb54e4061aec2) ) ROM_END
ROM_START( sc1clatta ) ROM_REGION( 0x10000, "maincpu", 0 )  ROM_LOAD( "393717553 prom b.bin", 0x0000, 0x8000, BAD_DUMP CRC(06f41627) SHA1(0e54314147a5f0d833d83f6f0ee828bd1c875f3e) ) ROM_LOAD( "393717552 prom a.bin", 0x8000, 0x8000, BAD_DUMP CRC(795e93cf) SHA1(017fa5ea3d9ad1f7a7a619d88a5892a9ffe6f3bc) ) ROM_END

// PROJECT NUMBER 5527  CLUB ATTRACTION DUAL #1 - 3-APR-1990 17:17:23
GAME( 1990, sc1clatt        , 0         , scorpion1         , clatt     , bfm_sc1_state, clatt          , 0,       "BFM",      "Club Attraction (UK, Game Card 39-370-196)", GAME_FLAGS ) // GAME No 39-370-196
// PROJECT NUMBER 5527  CLUB ATTRACTION DUAL #1 - 22-NOV-1990 16:26:05
GAME( 1990, sc1clatta       , sc1clatt  , scorpion1         , clatt     , bfm_sc1_state, clatt          , 0,       "BFM",      "Club Attraction (set 2)", GAME_FLAGS ) // GAME No 39-370-266


/********************************************************************************************************************************************************************************************************************
 Cash Wise
 ********************************************************************************************************************************************************************************************************************/

ROM_START( sc1cshwza ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95727206b 74-78 adj.bin", 0x0000, 0x8000, CRC(2b0ea9dc) SHA1(a9099abe2cf4cdf119a00e5a218507798d410eff) ) ROM_LOAD( "95727205a 74-78 adj.bin",       0x8000, 0x8000, CRC(7c7ddabc) SHA1(b4c7a9ee929b5635091366948257f273a21d7818) ) ROM_END
ROM_START( sc1cshwze ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95727206b 74-78 adj.bin", 0x0000, 0x8000, CRC(2b0ea9dc) SHA1(a9099abe2cf4cdf119a00e5a218507798d410eff) ) ROM_LOAD( "95728205a 74-78 adj proto.bin", 0x8000, 0x8000, CRC(1cdadddb) SHA1(33c7ed10b1c9ddc0fc6065ad9b1cf80ee9f8e958) ) ROM_END
ROM_START( sc1cshwz )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95717206b std.bin",       0x0000, 0x8000, CRC(2478530f) SHA1(be82a4e36a3c076b9e94fa2364904ca463b6b4ed) ) ROM_LOAD( "95717205a std.bin",             0x8000, 0x8000, CRC(795bbeea) SHA1(22e0fc9bc3c70e05e51cb98837a9c706eb2ca080) ) ROM_END
ROM_START( sc1cshwzc ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95717206b std.bin",       0x0000, 0x8000, CRC(2478530f) SHA1(be82a4e36a3c076b9e94fa2364904ca463b6b4ed) ) ROM_LOAD( "95718205a std ptel.bin",        0x8000, 0x8000, CRC(c88f476c) SHA1(a5d8f12ade77bdb100ece5f2eecec35ae09f3b0e) ) ROM_END
ROM_START( sc1cshwzb ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95727211b 74-78 sw.bin",  0x0000, 0x8000, CRC(e20ee4d3) SHA1(3440ad647f8e009a13de6ff9797a47c636a50123) ) ROM_LOAD( "95727210a 74-78 sw.bin",        0x8000, 0x8000, CRC(6276ee67) SHA1(cc9b794f0add6d68677858719831e10afbdbc699) ) ROM_END
ROM_START( sc1cshwzf ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95727211b 74-78 sw.bin",  0x0000, 0x8000, CRC(e20ee4d3) SHA1(3440ad647f8e009a13de6ff9797a47c636a50123) ) ROM_LOAD( "95728210a 74-78 proto.bin",     0x8000, 0x8000, CRC(5c502423) SHA1(4fc93de9dd3aff7a8a8f828760d8b095b7a13630) ) ROM_END
ROM_START( sc1cshwzg ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95717211.bin",            0x0000, 0x8000, CRC(ed781e00) SHA1(67ebb58beda5123f061a22dacd008f1feb75b8d9) ) ROM_LOAD( "95717210.bin",                  0x8000, 0x8000, CRC(102d2bc8) SHA1(8ed5f44e6014e21f677762e40076d648901d1ff2) ) ROM_END
ROM_START( sc1cshwzd ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95717211.bin",            0x0000, 0x8000, CRC(ed781e00) SHA1(67ebb58beda5123f061a22dacd008f1feb75b8d9) ) ROM_LOAD( "95718210a proto var.bin",       0x8000, 0x8000, CRC(0997c4e9) SHA1(1013a12803796d3926cceeb671c7c07cc66d418e) ) ROM_END



// PROJECT NUMBER 5423  CASH WISE - 13-MAY-1989 14:46:29
GAME( 198?, sc1cshwza       , sc1cshwz  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Wise (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-380-100
GAME( 198?, sc1cshwze       , sc1cshwz  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Wise (Bellfruit) (set 1, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-381-100
// PROJECT NUMBER 5423  CASH WISE  VARIABLE % - 13-MAY-1989 14:49:12
GAME( 198?, sc1cshwz        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Wise (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-100
GAME( 198?, sc1cshwzc       , sc1cshwz  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Wise (Bellfruit) (set 2, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-100
// PROJECT NUMBER 5423  CASH WISE - 2-JUN-1989 13:08:30
GAME( 198?, sc1cshwzb       , sc1cshwz  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Wise (Bellfruit) (set 3) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-380-104
GAME( 198?, sc1cshwzf       , sc1cshwz  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Wise (Bellfruit) (set 3, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-381-104
// PROJECT NUMBER 5423  CASH WISE  VARIABLE % - 2-JUN-1989 13:24:44
GAME( 199?, sc1cshwzg       , sc1cshwz  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Wise (Bellfruit) (set 4) (Scorpion 1)", GAME_FLAGS) // GAME No 39-370-104
GAME( 198?, sc1cshwzd       , sc1cshwz  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Wise (Bellfruit) (set 4, Protocol) (Scorpion 1)", GAME_FLAGS ) //  GAME No 39-371-104


/********************************************************************************************************************************************************************************************************************
 Club Wise
  the 1-JUN-1990 set has an odd game code
 ********************************************************************************************************************************************************************************************************************/

	#define sc1_clbw_sound \
	ROM_REGION( 0x20000, "upd", 0 )\
	ROM_LOAD( "wisesnd1.bin", 0x0000, 0x010000, CRC(204605a6) SHA1(193a60878ed46f122e5d2d8f35fc6ea967b8734f) )\
	ROM_LOAD( "wisesnd2.bin", 0x010000, 0x010000, CRC(6aa66166) SHA1(2e7cc67afdce2febb541bb1d0e7c107876d4233d) )
ROM_START( sc1clbw )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "clbwzp2",                 0x0000, 0x8000, CRC(44bb7e16) SHA1(d3c258ea286be18dc667df6a7138280462db661b) ) ROM_LOAD( "clbwzp1",                 0x8000, 0x8000, CRC(c61dd4eb) SHA1(e1756f8841dabe1bc002aadba6b224a558096a96) ) sc1_clbw_sound ROM_END
ROM_START( sc1clbwa ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club wise 95717551b.bin", 0x0000, 0x8000, CRC(0528a718) SHA1(27f4225c948d93ce1c833679f97e045f3b7a6aac) ) ROM_LOAD( "club wise 95717550a.bin", 0x8000, 0x8000, CRC(5b305f11) SHA1(592ea71fcb72eaa90fd421e3bd3761cfd686b019) ) sc1_clbw_sound ROM_END

// PROJECT NUMBER 5731  V1 5/10/20p PLAY - 21-NOV-1990 12:00:09
GAME( 198?, sc1clbwa        , sc1clbw   , scorpion1         , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club Wise (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-265
// PROJECT NUMBER 5731  V1 5/10/20p PLAY - 1-JUN-1990 12:03:09
GAME( 198?, sc1clbw         , 0         , scorpion1         , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club Wise (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No PR5731S11.HEX


/********************************************************************************************************************************************************************************************************************
 Barcode
 ********************************************************************************************************************************************************************************************************************/

#define sc1barcd_sound \
	ROM_REGION( 0x20000, "upd", 0 ) \
	ROM_LOAD( "barsnd1.bin", 0x00000, 0x10000, CRC(c9de8ff4) SHA1(c3e77e84d4ecc1c779929a96d1c445a1af24865b) ) \
	ROM_LOAD( "barsnd2.bin", 0x10000, 0x10000, CRC(56af984a) SHA1(aebd30f3ca767dc5fc77fb01765833ee627a5aee) )
ROM_START( sc1barcd ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95740352 b.bin",      0x0000, 0x8000, CRC(6dc3cfd3) SHA1(d71d433ae560ac4db345630ee7f04a7cfb7e933e) ) ROM_LOAD( "95740351 a.bin",      0x8000, 0x8000, CRC(0891350b) SHA1(ea1295768738b9b89eac19d04411220a8c9d10c7) ) sc1barcd_sound ROM_END
ROM_START( sc1barcda )ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "barcode 5_10p b.bin", 0x0000, 0x8000, CRC(69d4d0b2) SHA1(bb73b917cf414623dcd239c5daeeccb4e0ccc2ed) ) ROM_LOAD( "barcode 5_10p a.bin", 0x8000, 0x8000, CRC(e864aba1) SHA1(b3f707b6d5f3d7236e4a5e9ed78c61a78c3e8196) ) sc1barcd_sound ROM_END

// PROJECT NUMBER 5907  BARCODE 20P PAYOUT - 8-JAN-1992 15:34:28
GAME( 198?, sc1barcd        , 0         , scorpion1_viper   , clatt     , bfm_sc1_state, lotse          , 0,       "BFM",      "Barcode (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-502
// PROJECT NUMBER 6380  BARCODE 5P 10P PLAY- 17-FEB-1994 09:23:56
GAME( 198?, sc1barcda       , sc1barcd  , scorpion1_viper   , clatt     , bfm_sc1_state, lotse          , 0,       "BFM",      "Barcode (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) //  GAME No 39-370-959

/********************************************************************************************************************************************************************************************************************
 Double Chance
  sc1dblcha has a severe fixed bits problem, it's almost not worth keeping
  sc1dblchb also seem like bad dumps (both halves identical) BUT they contain alpha strings for the 'Double Chance' game where the parent set doesn't
            is it a different game or roms for some kind of extra display hardware?

 Sound roms don't seem to get used?
********************************************************************************************************************************************************************************************************************/

#define sc1_dblch_sound \
	ROM_REGION( 0x40000, "upd", 0 )\
	ROM_LOAD( "doublechancesnd1.bin", 0x00000, 0x010000, CRC(bee6af3e) SHA1(334fe491a00f58a2142f65344674b26c766a7c5b) )\
	ROM_LOAD( "doublechancesnd2.bin", 0x10000, 0x010000, CRC(bbadc876) SHA1(902e387ea9bcd833cf75a6f049b5b2822ec6dc2a) )
ROM_START( sc1dblch )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "doublechancegame2.bin", 0x0000, 0x8000,          CRC(d4f49454) SHA1(53b97f941a4abfeb3e498b4295f98e80bd182b7e) ) ROM_LOAD( "doublechancegame1.bin", 0x8000, 0x8000,          CRC(9e24e0e3) SHA1(fff1fe9219c052750709d13c06148c7926a22910) ) sc1_dblch_sound ROM_END
ROM_START( sc1dblcha ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95717417_dc_std.b",     0x0000, 0x8000, BAD_DUMP CRC(51e5459b) SHA1(b6ffbcff63fd3543226778c61fbe2246f40635dd) ) ROM_LOAD( "95717416_dc_std.a",     0x8000, 0x8000, BAD_DUMP CRC(949726ed) SHA1(6ecebd20387aa73b0404ab4b7342e2b39d77b37f) ) sc1_dblch_sound ROM_END

// WHAT are these? they contain Double Chance game strings, unlike our actual Double Chance set, maybe a Club version with Alpha display?
ROM_START( sc1dblchb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717789 10p.bin", 0x8000, 0x8000, CRC(fc338d38) SHA1(65457f2611ffa22ac35f1e7ad10c290c01b9c3ac) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "95717790 5p.bin", 0x8000, 0x8000, CRC(c82e57f9) SHA1(456ce5290db322292170412a00f0252b86743ed0) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "95717787 10p20p.bin", 0x8000, 0x8000, CRC(69ba126c) SHA1(f59aa5a632d0bc5102c206f986f86b6c7c1352fb) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

// PROJECT NUMBER 5599  DOUBLE CHANCE 20P - 6-APR-1990 11:02:09
GAME( 198?, sc1dblch        , 0         , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Double Chance (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-203
// too bad to get PROJECT identification
GAME( 198?, sc1dblcha       , sc1dblch  , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Double Chance (Bellfruit) (set 2, bad) (Scorpion 1)", GAME_FLAGS )
GAME( 198?, sc1dblchb       , sc1dblch  , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Double Chance (Bellfruit) (set 3) (Scorpion 1)", GAME_FLAGS )


/********************************************************************************************************************************************************************************************************************
 Club 65 Special
  2 sets of sound roms, they start off roughly the same, but end up different midway through the first rom, maybe they relate to the 2 different project codes?

  Sound roms don't seem to get used?
********************************************************************************************************************************************************************************************************************/

#define sc1_cl65_sound \
	ROM_REGION( 0x20000, "upd", 0 )\
	ROM_LOAD( "65sndp1.bin", 0x000000, 0x010000, CRC(e532fcf5) SHA1(7de3bd4a3efae7d1cfeee23c008efbff39ce46f8) )\
	ROM_LOAD( "65sndp2.bin", 0x010000, 0x010000, CRC(2703ea2d) SHA1(a4876a10d8d4b1de01dfab76e4ee21cb120aa783) )
#define sc1_cl65_sound_alt  \
	ROM_REGION( 0x20000, "upd", 0 )\
	ROM_LOAD( "club-six-five-special_snd_a.bin", 0x0000, 0x010000, CRC(915802cd) SHA1(5bca3a80199a6534e084a5cf4337da4e9c48f45c) )\
	ROM_LOAD( "club-six-five-special_snd_b.bin", 0x0000, 0x010000, CRC(b3b230d8) SHA1(022e95f38b14922137222805c0bec7498c5956cc) )
ROM_START( sc1cl65 )   ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "65spp2",      0x0000, 0x8000, CRC(11332a28) SHA1(76f9eee54351e0d8dc4b620ec92661538929e75d) ) ROM_LOAD( "65spp1",                                               0x8000, 0x8000, CRC(2c4cb63b) SHA1(5d09b575cf80beecd83c07286b74af29de7ec553) ) sc1_cl65_sound     ROM_END
ROM_START( sc1cl65d )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "39370694.p2", 0x0000, 0x8000, CRC(3371dc55) SHA1(52d75a90933acc7a03821e5c2821df6126c72a6c) ) ROM_LOAD( "club-six-five-special_std_ac_rot_10po_ass.bin",        0x8000, 0x8000, CRC(cf48ba99) SHA1(5da4321ff349964e903f1bebd3e5ddd0799fc478) ) sc1_cl65_sound_alt ROM_END
ROM_START( sc1cl65dp ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "39370694.p2", 0x0000, 0x8000, CRC(3371dc55) SHA1(52d75a90933acc7a03821e5c2821df6126c72a6c) ) ROM_LOAD( "club-six-five-special_dat_ac_rot_10po_ass.bin",        0x8000, 0x8000, CRC(77ddf81d) SHA1(522d9f84ab6e31586f371548e2f146ac193f06f5) ) sc1_cl65_sound_alt ROM_END
ROM_START( sc1cl65c )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "39370714.p2", 0x0000, 0x8000, CRC(cb9f944f) SHA1(49955a968264f3d963317f5c772629d9bbdd33f7) ) ROM_LOAD( "club-six-five-special_std_ac_a.bin",                   0x8000, 0x8000, CRC(8bd817f8) SHA1(6ae91a29a6263c085f6254a049fb3c2ba9cac662) ) sc1_cl65_sound_alt ROM_END
ROM_START( sc1cl65cp ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "39370714.p2", 0x0000, 0x8000, CRC(cb9f944f) SHA1(49955a968264f3d963317f5c772629d9bbdd33f7) ) ROM_LOAD( "club-six-five-special_dat_ac_200pnd_rot_20po_ass.bin", 0x8000, 0x8000, CRC(83a0253f) SHA1(a9b463e2aa87a736f88c5e71f233ff9d6a8b25b4) ) sc1_cl65_sound_alt ROM_END
ROM_START( sc1cl65b )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "39370859.p2", 0x0000, 0x8000, CRC(f04065a0) SHA1(d63ba578931b2f4f156ca875da9cf69cf283a27c) ) ROM_LOAD( "club-six-five-special_std_ac_200pnd_rot_10po_ass.bin", 0x8000, 0x8000, CRC(eedd9fa1) SHA1(6233e8304ac94798cfb908b2ba31ec6c98808ce8) ) sc1_cl65_sound_alt ROM_END
ROM_START( sc1cl65bp ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "39370859.p2", 0x0000, 0x8000, CRC(f04065a0) SHA1(d63ba578931b2f4f156ca875da9cf69cf283a27c) ) ROM_LOAD( "club-six-five-special_dat_ac_200pnd_rot_10po_ass.bin", 0x8000, 0x8000, CRC(836e65d8) SHA1(931e5831b0b64e7ce29fb497435d486e40dce839) ) sc1_cl65_sound_alt ROM_END
ROM_START( sc1cl65a )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "39370858.p2", 0x0000, 0x8000, CRC(ff0e35c0) SHA1(0d3d46b541e188200cb4b9cc65eb60eac913dc2b) ) ROM_LOAD( "club-six-five-special_std_ac_200pnd_rot_20po_ass.bin", 0x8000, 0x8000, CRC(dd188272) SHA1(d6b7f7b060e632bd3eacc7f7721399a1c8349698) ) sc1_cl65_sound_alt ROM_END
ROM_START( sc1cl65ap ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "39370858.p2", 0x0000, 0x8000, CRC(ff0e35c0) SHA1(0d3d46b541e188200cb4b9cc65eb60eac913dc2b) ) ROM_LOAD( "club-six-five-special_dat_ac_rot_20po_ass.bin",        0x8000, 0x8000, CRC(028ff7b2) SHA1(500b6f8d85678e99ae804600099fe78b542ad6a3) ) sc1_cl65_sound_alt ROM_END

// PROJECT NUMBER 5732  SIX FIVE SPECIAL -  6-SEP-1990 14:55:09
GAME( 198?, sc1cl65         , 0         , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club 65 Special (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-240
// PROJECT NUMBER 5732  SIX FIVE SPECIAL -  5-OCT-1992 16:23:33
GAME( 198?, sc1cl65d        , sc1cl65   , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club 65 Special (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-694
GAME( 198?, sc1cl65dp       , sc1cl65   , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club 65 Special (Bellfruit) (set 2, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-694
// PROJECT NUMBER 5732  SIX FIVE SPECIAL 20P PAYOUT - 13-OCT-1992 12:18:09
GAME( 198?, sc1cl65c        , sc1cl65   , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club 65 Special (Bellfruit) (set 3) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-714
GAME( 198?, sc1cl65cp       , sc1cl65   , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club 65 Special (Bellfruit) (set 3, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-714
// PROJECT NUMBER 6124  SIX FIVE SPECIAL 200 POUND JP - 21-APR-1993 14:43:38
GAME( 198?, sc1cl65b        , sc1cl65   , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club 65 Special (Bellfruit) (set 4) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-859
GAME( 198?, sc1cl65bp       , sc1cl65   , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club 65 Special (Bellfruit) (set 4, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-859
// PROJECT NUMBER 6124 20P PAYOUT  SIX FIVE SPECIAL #200/20P PAYOUT - 21-APR-1993 14:46:20
GAME( 198?, sc1cl65a        , sc1cl65   , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club 65 Special (Bellfruit) (set 5) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-858
GAME( 198?, sc1cl65ap       , sc1cl65   , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club 65 Special (Bellfruit) (set 5, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-858

/********************************************************************************************************************************************************************************************************************
 China Town
********************************************************************************************************************************************************************************************************************/

#define sc1_china_sound \
	ROM_REGION( 0x20000, "upd", 0 )  \
	ROM_LOAD( "ctowsnd1.bin", 0x00000, 0x010000, CRC(faf28e18) SHA1(0586a905f944bcc990d4a1b400629412a69fc160) )\
	ROM_LOAD( "ctowsnd2.bin", 0x10000, 0x010000, CRC(f4f9c1a4) SHA1(af5aff58b3e362a14e26a5e8cae83affda905819) )
ROM_START( sc1china )   ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-china-town_std_ac_200pnd_rot_bss.bin", 0x0000, 0x8000, CRC(4895098f) SHA1(e08f9b85c634a423a93608a7b592436ae253ca42) ) ROM_LOAD( "club-china-town_std_ac_200pnd_rot_ass.bin", 0x8000, 0x8000, CRC(a9ed6493) SHA1(8049fe4b42110afab91dd2d9ccd132d4f2c1c0ff) ) sc1_china_sound ROM_END
ROM_START( sc1chinap )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-china-town_dat_ac_200pnd_bss.bin",     0x0000, 0x8000, CRC(4895098f) SHA1(e08f9b85c634a423a93608a7b592436ae253ca42) ) ROM_LOAD( "club-china-town_dat_ac_200pnd_ass.bin",     0x8000, 0x8000, CRC(5aa465b9) SHA1(3c2d805f0421d7d1db93f21358a2beb648c05f8e) ) sc1_china_sound ROM_END
ROM_START( sc1chinaa )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-china-town_std_ac_rot_bss.bin",        0x0000, 0x8000, CRC(6e09a878) SHA1(4084b1dc3425ceb980ef5c63a883720f3ad84d7f) ) ROM_LOAD( "club-china-town_std_ac_rot_ass.bin",        0x8000, 0x8000, CRC(de12ac34) SHA1(0caeb2a6b209ee34d67d4c619dd63562c839261e) ) sc1_china_sound ROM_END
ROM_START( sc1chinaap ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-china-town_dat_ac_rot_bss.bin",        0x0000, 0x8000, CRC(6e09a878) SHA1(4084b1dc3425ceb980ef5c63a883720f3ad84d7f) ) ROM_LOAD( "club-china-town_dat_ac_rot_ass.bin",        0x8000, 0x8000, CRC(109b722c) SHA1(19426f3f907f108dc16b4036d3986c6395f799d0) ) sc1_china_sound ROM_END
ROM_START( sc1chinab )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-china-town_std_ac_150pnd_lfj_bss.bin", 0x0000, 0x8000, CRC(d41c6999) SHA1(cc2eb2e74ca3bfa78d74dd08f83acb2fe650e13d) ) ROM_LOAD( "club-china-town_std_ac_150pnd_lfj_ass.bin", 0x8000, 0x8000, CRC(8c3e69f1) SHA1(cb0cbf7a6039549b969160a162a0cd5511b24cd3) ) sc1_china_sound ROM_END
ROM_START( sc1chinabp ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-china-town_dat_ac_150pnd_lfj_bss.bin", 0x0000, 0x8000, CRC(d41c6999) SHA1(cc2eb2e74ca3bfa78d74dd08f83acb2fe650e13d) ) ROM_LOAD( "club-china-town_dat_ac_150pnd_lfj_ass.bin", 0x8000, 0x8000, CRC(9547727a) SHA1(ac4a23ae78d9331261ee0ab59816f65c5c1547d7) ) sc1_china_sound ROM_END

// CHINA TOWN  CHINA TOWN - 18-MAR-1992 13:59:59
GAME( 198?, sc1chinaa       , sc1china  , scorpion1_viper   , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "China Town Club (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-551
GAME( 198?, sc1chinaap      , sc1china  , scorpion1_viper   , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "China Town Club (Bellfruit) (set 1, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-551
// PR6121 CHINA TOWN  CHINA TOWN 200 POUND JACKPOT - 14-APR-1993 12:15:57
GAME( 198?, sc1china        , 0         , scorpion1_viper   , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "China Town Club (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-857
GAME( 198?, sc1chinap       , sc1china  , scorpion1_viper   , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "China Town Club (Bellfruit) (set 2, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-857
// PROJECT NUMBER PR5989 CHINA TOWN  CHINA TOWN SMOOTHED JACKPOT - 12-MAY-1994 12:15:56
GAME( 198?, sc1chinab       , sc1china  , scorpion1_viper   , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "China Town Club (Bellfruit) (set 3) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-979
GAME( 198?, sc1chinabp      , sc1china  , scorpion1_viper   , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "China Town Club (Bellfruit) (set 3, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-979

/********************************************************************************************************************************************************************************************************************
 Club Temptation
********************************************************************************************************************************************************************************************************************/

/*
ROM_LOAD( "temp11a.bin", 0x8000, 0x8000, CRC(37c8b73e) SHA1(f718572d170be7b582c3818df7163309cea232b5) ) // FIXED BITS (xxxxxx1x)
appears to be a bad dump of
ROM_LOAD( "95717692a.bin", 0x8000, 0x8000, CRC(f9fe7b9a) SHA1(0e3fe5da9fc837726d08f02a2c6ed782f016c982) )
*/

#define sc1_clbtm_sound \
	ROM_REGION( 0x40000, "upd", 0 ) \
	ROM_LOAD( "tempsnd1.bin", 0x00000, 0x10000, CRC(168e2a18) SHA1(db97acf9131b1a54efe1cd375aecae1679bab19e) ) \
	ROM_LOAD( "tempsnd2.bin", 0x00000, 0x10000, CRC(b717f347) SHA1(189c82318d622f18580a23eed48b17c0c34dedd5) )
ROM_START( sc1clbtm )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "temp12b.bin", 0x0000, 0x8000, CRC(3c27c592) SHA1(081d61f974e2ae5c64729b32be4c0e5067a20550) ) ROM_LOAD( "95717692a.bin", 0x8000, 0x8000, CRC(f9fe7b9a) SHA1(0e3fe5da9fc837726d08f02a2c6ed782f016c982) ) sc1_clbtm_sound ROM_END
ROM_START( sc1clbtma ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "temptp2",     0x0000, 0x8000, CRC(d165fa87) SHA1(aef8a4af8b6e83ef09dffc8aca305eaf7dd3936b) ) ROM_LOAD( "temptp1",       0x8000, 0x8000, CRC(6f03648d) SHA1(a6402c94ebf4d570d1d3fb462eb621566c27f307) ) sc1_clbtm_sound ROM_END

// PROJECT NUMBER 5491  TEMPTATION - 1-MAY-1991 13:36:44
GAME( 198?, sc1clbtm        , 0         , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club Temptation (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-342
// PROJECT NUMBER 5491  TEMPTATION HI-FREQ CASHPOT - 31-OCT-1991 12:50:19
GAME( 198?, sc1clbtma       , sc1clbtm  , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club Temptation (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-449

/********************************************************************************************************************************************************************************************************************
 Count Cash Club
********************************************************************************************************************************************************************************************************************/

// might not be used
#define sc1_count_sound \
	ROM_REGION( 0x40000, "upd", ROMREGION_ERASE00 )
ROM_START( sc1count )   ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-count-cash_std_ac_200pnd_rot_bss.bin", 0x0000, 0x8000, CRC(8e385a9e) SHA1(67c45734501c16be3b8270f388dc1313bce289f8) ) ROM_LOAD( "club-count-cash_std_ac_200pnd_rot_ass.bin", 0x8000, 0x8000, CRC(a6a1a604) SHA1(86e59578fed7023b0e6a42495b9a60e7178ee566) ) sc1_count_sound ROM_END
ROM_START( sc1countp )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-count-cash_dat_ac_200pnd_rot_bss.bin", 0x0000, 0x8000, CRC(8e385a9e) SHA1(67c45734501c16be3b8270f388dc1313bce289f8) ) ROM_LOAD( "club-count-cash_dat_ac_200pnd_rot_ass.bin", 0x8000, 0x8000, CRC(da097abe) SHA1(85f01d8b5dce535a5559fadaf1cf7373c6967882) ) sc1_count_sound ROM_END
ROM_START( sc1counta )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-count-cash_std_ac_rot_bss.bin",        0x0000, 0x8000, CRC(69df417d) SHA1(a7788a9f3056919017616960ba5017bcd94b8a98) ) ROM_LOAD( "club-count-cash_std_ac_rot_ass.bin",        0x8000, 0x8000, CRC(b081333c) SHA1(75a46634458a790f91360be26cace0e42bbf3481) ) sc1_count_sound ROM_END
ROM_START( sc1countap ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-count-cash_dat_ac_rnr_bss.bin",        0x0000, 0x8000, CRC(69df417d) SHA1(a7788a9f3056919017616960ba5017bcd94b8a98) ) ROM_LOAD( "club-count-cash_dat_ac_rnr_ass.bin",        0x8000, 0x8000, CRC(87f68f57) SHA1(fe99c8577a80a7ec791bf87e78cf429eebbc7785) ) sc1_count_sound ROM_END

// PROJECT NUMBER 6031  COUNT CASH standard - 14-SEP-1992 11:07:14
GAME( 198?, sc1counta       , sc1count  , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Count Cash Club (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-698
GAME( 198?, sc1countap      , sc1count  , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Count Cash Club (Bellfruit) (set 1, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-698
// PROJECT NUMBER 6120  COUNT CASH 200 POUND JACKPOT - 14-APR-1993 12:12:42
GAME( 198?, sc1count        , 0         , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Count Cash Club (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-855
GAME( 198?, sc1countp       , sc1count  , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Count Cash Club (Bellfruit) (set 2, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-855

/********************************************************************************************************************************************************************************************************************
 Strike It Rich
********************************************************************************************************************************************************************************************************************/

ROM_START( sc1sir )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "s.i.rich 2p 2.40 p2 9.3.90 1a96.bin", 0x0000, 0x8000, CRC(618841ca) SHA1(2e690ca91da0a1ff36245a6f1e2ad681a6ed4f32) ) ROM_LOAD( "s.i.rich 2p 2.40 p1 9.3.90 0b49.bin", 0x8000, 0x8000, CRC(c54703f8) SHA1(9ac3af9021cf5012562b0ab057a30e11e01eef65) ) ROM_END
ROM_START( sc1sirb ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "s.i.rich 5p 2.40 p2 9.3.90.bin",      0x0000, 0x8000, CRC(cd3df765) SHA1(798d051afbba5a474b1b619621e4425f5ff7f8db) ) ROM_LOAD( "s.i.rich 5p 2.40 p1 9.3.90.bin",      0x8000, 0x8000, CRC(6a37f38d) SHA1(1e7640446ecb6e00d57a92ab3592c389a172f257) ) ROM_END

/* these are both bad dumps, similar to each other, but not to the sets we support */

ROM_START( sc1sira )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717078 proto.bin", 0x0000, 0x8000, BAD_DUMP CRC(7a83b794) SHA1(8befa43c7afa37a296b309730d5cdfd32dfc363d) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "95717077 proto.bin", 0x8000, 0x8000, BAD_DUMP CRC(38691d92) SHA1(17f33f74d221ac37249a04846670a1a1c0ee618e) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

ROM_START( sc1sirc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strike it rich a.bin", 0x0000, 0x8000, BAD_DUMP CRC(92ddbbca) SHA1(d888e663d0965d99dbfa68e3aed995e31411f2ba) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "strike it rich b.bin", 0x8000, 0x8000, BAD_DUMP CRC(bdafc4c9) SHA1(5ffc46088818f0e89eb840e039296945905ca4f3) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

// PROJECT NUMBER 5773  STRIKE IT RICH - 2P - 7-MAR-1990 15:24:32
GAME( 198?, sc1sir          , 0         , scorpion1         , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Strike It Rich (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-180
// PROJECT NUMBER 5773  STRIKE IT RICH - 5P - 9-MAR-1990 10:48:23
GAME( 198?, sc1sirb         , sc1sir    , scorpion1         , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Strike It Rich (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) //  GAME No 39-370-184
// 2nd half with the ident strings is missing
GAME( 198?, sc1sira         , sc1sir    , scorpion1         , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Strike It Rich (Bellfruit) (set 3, bad) (Scorpion 1)", GAME_FLAGS )
GAME( 198?, sc1sirc         , sc1sir    , scorpion1         , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Strike It Rich (Bellfruit) (set 4, bad) (Scorpion 1)", GAME_FLAGS )

/********************************************************************************************************************************************************************************************************************
 Fun House Club
********************************************************************************************************************************************************************************************************************/

#define sc1_funh_sound \
	ROM_REGION( 0x40000, "upd", 0 )\
	ROM_LOAD( "fhsesnd1.bin", 0x000000, 0x010000, CRC(bf371dbf) SHA1(0c9bc0d0964a858fba5324080a2cf5da119bf3db) )\
	ROM_LOAD( "fhsesnd2.bin", 0x010000, 0x010000, CRC(c51415e3) SHA1(f0e4eb5ce38faaef336a5b69e598985ea2486ceb) )
ROM_START( sc1funh )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-fun-house_std_ac_b.bin"      , 0x0000, 0x8000, CRC(1a838f0d) SHA1(747153e1bb9fc4fc28451e828fa2473f2e6d5e0e) ) ROM_LOAD( "club-fun-house_std_ac_a.bin",       0x8000, 0x8000,           CRC(f81dff1b) SHA1(4c205b3901f683d3679af9d311813ad912ecb436) ) sc1_funh_sound ROM_END
ROM_START( sc1funhp ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "club-fun-house_dat_ac_rot_bss.bin", 0x0000, 0x8000, CRC(1a838f0d) SHA1(747153e1bb9fc4fc28451e828fa2473f2e6d5e0e) ) ROM_LOAD( "club-fun-house_dat_ac_rot_ass.bin", 0x8000, 0x781f,  BAD_DUMP CRC(9a24dc71) SHA1(bb19ef26d6d46605107c8b53c6d9b4f08ed4c721) ) sc1_funh_sound ROM_END
ROM_START( sc1funha ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "funhop2",                           0x0000, 0x8000, CRC(2454e295) SHA1(9785d278afe05c632e1ab326d1b8fbabcc591fb6) ) ROM_LOAD( "funhop1",                           0x8000, 0x8000,           CRC(282d5651) SHA1(bd8c0985143d8fb5c8e0a2bfedea248569c8cf98) ) sc1_funh_sound ROM_END

// PROJECT NUMBER 5944  FUN HOUSE - 18-FEB-1992 16:16:01
GAME( 198?, sc1funh         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Fun House Club (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-530
GAME( 198?, sc1funhp        , sc1funh   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Fun House Club (Bellfruit) (set 1, Protocol, bad) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-530
// PROJECT NUMBER 5944  FUN HOUSE - 9-OCT-1991 14:08:13
GAME( 198?, sc1funha        , sc1funh   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Fun House Club (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-432

/********************************************************************************************************************************************************************************************************************
 Tri Star
********************************************************************************************************************************************************************************************************************/

ROM_START( sc1tri )   ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "20p b.bin",                0x0000, 0x8000, CRC(ef5bc525) SHA1(2881b9292f9dd7376997992941e07d288640703b) ) ROM_LOAD( "20p a.bin",                  0x8000, 0x8000, CRC(d162ebd5) SHA1(cfab100ab8cc34b61108fc7b8a3ec1f1b22f90ba) ) ROM_END
ROM_START( sc1tria )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957172.42 std var% b.bin", 0x0000, 0x8000, CRC(d8d70cac) SHA1(8137ab06912bc27f26bcbb800a09b095ba2175bb) ) ROM_LOAD( "957172.41 std var% a.bin",   0x8000, 0x8000, CRC(b314f739) SHA1(793c01f292c5144a1f5975b276b4985c565a2833) ) ROM_END
ROM_START( sc1triap ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957172.42 std var% b.bin", 0x0000, 0x8000, CRC(d8d70cac) SHA1(8137ab06912bc27f26bcbb800a09b095ba2175bb) ) ROM_LOAD( "957182.41 proto var% a.bin", 0x8000, 0x8000, CRC(1af55594) SHA1(9e65c7bbb37d75662e4243fc6ba13f249183e2a3) ) ROM_END
ROM_START( sc1trib )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957272.42 std b.bin",      0x0000, 0x8000, CRC(634b1927) SHA1(60f2bf02a12021da3c7995122dff85ce7831ed42) ) ROM_LOAD( "957272.41 std a.bin",        0x8000, 0x8000, CRC(635ded7e) SHA1(3e8bda8c2fa6fc8e46ba3e3a70dfb183fad3223b) ) ROM_END
ROM_START( sc1tribp ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "957272.42 std b.bin",      0x0000, 0x8000, CRC(634b1927) SHA1(60f2bf02a12021da3c7995122dff85ce7831ed42) ) ROM_LOAD( "957282.41 proto std a.bin",  0x8000, 0x8000, CRC(e5999ec8) SHA1(0a11544da03fc2197dc2cc6780cbaeee55372069) ) ROM_END

// PROJECT NUMBER 5600  TRISTAR 20P VARIABLE % - 21-DEC-1989 17:54:13
GAME( 198?, sc1tri          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Tri Star (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-131
// PROJECT NUMBER 5490  TRISTAR VARIABLE % - 26-OCT-1989 16:45:43
GAME( 198?, sc1tria         , sc1tri    , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Tri Star (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-120
GAME( 198?, sc1triap        , sc1tri    , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Tri Star (Bellfruit) (set 2, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-371-120
// PROJECT NUMBER 5490  TRISTAR - 26-OCT-1989 16:43:39
GAME( 198?, sc1trib         , sc1tri    , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Tri Star (Bellfruit) (set 3) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-380-120
GAME( 198?, sc1tribp        , sc1tri    , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Tri Star (Bellfruit) (set 3, Protocol) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-381-120

/********************************************************************************************************************************************************************************************************************
 Club Explosion
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 5523  VE 5/10/20p PLAY - 9-MAR-1990 12:25:00
GAME( 198?, sc1clbxp        , 0         , scorpion1_viper   , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club Explosion (Bellfruit) (Scorpion 1) (set 1)", GAME_FLAGS ) // GAME No 39-370-175
// PROJECT NUMBER 5523  VE 5/10/20p PLAY - 13-NOV-1989 14:13:58
GAME( 198?, sc1clbxpa       , sc1clbxp  , scorpion1_viper   , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Club Explosion (Bellfruit) (Scorpion 1) (set 2)", GAME_FLAGS ) // GAME No 39-370-121

/********************************************************************************************************************************************************************************************************************
 Clockwise
********************************************************************************************************************************************************************************************************************/

#define sc1_cwcl_sound \
	ROM_REGION( 0x20000, "upd", 0 ) \
	ROM_LOAD( "95000001snd.bin", 0x00000, 0x008000, CRC(38f85127) SHA1(c9c7c8892396180aa4c4a727422391b9ce93a10a) ) \
	ROM_LOAD( "95000002snd.bin", 0x08000, 0x008000, CRC(ca2f5547) SHA1(fe8378ee485ce396b665ea504650caf51843fd74) ) \
	ROM_LOAD( "95000003snd.bin", 0x10000, 0x008000, CRC(475695f9) SHA1(9f6ba3de7b4b38946106a3aeab9a2a2eb2a99193) )
ROM_START( sc1cwcl ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "95717154b.bin", 0x0000, 0x8000, CRC(e6422f75) SHA1(4ab33a5503209377f4739dbe11e4afa8d7e43699) )  ROM_LOAD( "95717153a.bin", 0x8000, 0x8000, CRC(233174a1) SHA1(94cf071a955e3716f463c4370daabfe94db2fd0e) ) sc1_cwcl_sound ROM_END

// PROJECT NUMBER 5216  VE 5/10/20p PLAY - 17-FEB-1989 12:23:30
GAME( 198?, sc1cwcl         , 0         , scorpion1_viper           , clatt , bfm_sc1_state, lotse          , 0,       "BFM",      "Clockwise (Bellfruit) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-076

/********************************************************************************************************************************************************************************************************************
 Bar Trek
********************************************************************************************************************************************************************************************************************/

#define sc1_bartk_sound \
	ROM_REGION( 0x20000, "upd", 0 ) \
	ROM_LOAD( "bartreksnd1.bin", 0x000000, 0x010000, CRC(690b18c3) SHA1(0a3ecadc8d47670bc0f36d76b4335f027ef68542) ) \
	ROM_LOAD( "bartreksnd2.bin", 0x010000, 0x010000, CRC(4ff8201c) SHA1(859378b4bb8fc5d3497a53c9218302410884e091) )
ROM_START( sc1bartk ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "bartrekgameb.bin", 0x0000, 0x8000, CRC(24c7c803) SHA1(ab5051c8727cab44ad59913edab3d5d145728cb5) ) ROM_LOAD( "bartrekgamea.bin", 0x8000, 0x8000, CRC(a7a84c16) SHA1(8c5ab34268e932be12e85eed5a56386681f13da4) ) sc1_bartk_sound ROM_END

// PROJECT NUMBER 6006  BAR TREK #3/#6 - 1-DEC-1992 08:20:06
GAME( 198?, sc1bartk        , 0         , scorpion1_viper   , clatt     , bfm_sc1_state, lotse          , 0,       "BFM",      "Bar Trek (Bellfruit) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-746



// PROJECT NUMBER 5146  CASH CARD  GAME No 39-370-064 -   17-NOV-1988 11:06:39
GAME( 198?, sc1cshcda       , sc1cshcd  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Card (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 5146  CASH CARD VERSION 2  GAME No 39-372-052 -   17-OCT-1988 15:24:53
GAME( 198?, sc1cshcdb       , sc1cshcd  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Card (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5415  CASH EXPLOSION VERSION 8  GAME No 39-370-063 -   17-NOV-1988 11:59:28
GAME( 198?, sc1cexpl        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Explosion (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 5415  CASH EXPLOSION FIXED 78%  GAME No 39-373-042 -   12-AUG-1988 13:36:16
GAME( 198?, sc1cexpla       , sc1cexpl  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Explosion (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 5415  CASH EXPLOSION VERSION 2  GAME No 39-370-042 -   12-AUG-1988 13:47:56
GAME( 198?, sc1cexplb       , sc1cexpl  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Explosion (Bellfruit) (set 3) (Scorpion 1)", GAME_FLAGS )


// these hang after showing some lamps, what is PHOENIX1?
// PROJECT NUMBER 6218  COPS & ROBBERS PHOENIX1 - 10-SEP-1993 11:39:11
GAME( 198?, sc1cops         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cops 'n' Robbers (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-918
// PROJECT NUMBER 6218  COPS & ROBBERS PHOENIX1 - 12-MAY-1994 09:35:23
GAME( 198?, sc1copsa        , sc1cops   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cops 'n' Robbers (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-981
// PROJECT NUMBER 6218  COPS & ROBBERS PHOENIX1 HIGH TOKEN - 31-AUG-1993 09:49:28
GAME( 198?, sc1copdx        , sc1cops   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cops 'n' Robbers (Bellfruit) (set 3) (Scorpion 1)", GAME_FLAGS ) // GAME No 39-370-916

// PROJECT NUMBER 6207  OFAH - PHOENIX 1 - 18-APR-1994 10:44:57
GAME( 199?, sc1ofs56        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Only Fools and Horses (Bellfruit) (Scorpion 1?)", GAME_FLAGS) // GAME No 39-370-974

// PROJECT NUMBER 5420  AWP10 VERSION 1 FIXED 78%  GAME No 39-373-069 -   04-JAN-1989 10:16:18
GAME( 198?, sc1linx         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Linx (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS ) // bad rom
// PROJECT NUMBER 5613  LINX 20P PLAY VARIABLE %  GAME No 39-370-154 -  1-FEB-1990 08:35:47
GAME( 198?, sc1linxa        , sc1linx   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Linx (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 5420  AWP10 VERSION 1  GAME No 39-370-078 -   28-FEB-1989 17:10:59
GAME( 198?, sc1linxp        , sc1linx   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Linx (Bellfruit) (set 3, Protocol) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5493  20P POWERLINES  VARIABLE %  GAME No 39-370-130 - 13-DEC-1989 16:21:27
GAME( 198?, sc1pwrl         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Power Lines (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6104  SPECTRE #6/#3  GAME No 39-370-765 - 11-JAN-1993 13:52:50
GAME( 198?, sc1spct         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Spectre (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 6104  SPECTRE #6/#3 5P/10P PLAY 10P/20P P/O  GAME No 39-370-966 - 10-MAR-1994 07:57:48
GAME( 198?, sc1spcta        , sc1spct   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Spectre (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // bad rom

// different inputs, hold ALT to run
// PROJECT NUMBER 6171  TYPHOON PHOENIX1 #200  GAME No 39-370-944 -  5-NOV-1993 12:02:03
GAME( 198?, sc1typ          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Typhoon Club (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 6171  TYPHOON PHOENIX1 #200  GAME No 39-371-944 -  5-NOV-1993 12:02:03
GAME( 198?, sc1typp         , sc1typ    , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Typhoon Club (Bellfruit) (set 1, Protocol) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5417  WINNING STREAK  GAME No 39-370-055 -   10-NOV-1988 09:31:58
GAME( 198?, sc1winst        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 5417  WINNING STREAK  GAME No 39-371-055 -   10-NOV-1988 09:31:58
GAME( 198?, sc1winstp       , sc1winst  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 1, Protocol) (Scorpion 1)", GAME_FLAGS )
// no header data due to bad rom
GAME( 198?, sc1winsta       , sc1winst  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS ) // bad rom

// the first 2 sets are (unusually) licensed
// PROJECT NUMBER 5774  5P PLAY CASH LINES  GAME No 39-370-183 -  9-MAR-1990 10:45:33
GAME( 199?, sc1clins        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/PCP",  "Cash Lines (Bellfruit) (Scorpion 1) (set 1)", GAME_FLAGS)
// PROJECT NUMBER 5774  2P PLAY CASH LINES  GAME No 39-370-181 -  7-MAR-1990 15:27:24
GAME( 199?, sc1clinsa       , sc1clins  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/PCP",  "Cash Lines (Bellfruit) (Scorpion 1) (set 2)", GAME_FLAGS)
// PROJECT NUMBER 5159  CASH LINES  GAME No 39-371-028 -   28-MAR-1988 07:34:28
GAME( 199?, sc1clinsb       , sc1clins  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Lines (Bellfruit) (Scorpion 1) (set 3)", GAME_FLAGS)
// PROJECT NUMBER 5159  CASH LINES  GAME No 39-370-028 -   28-MAR-1988 07:34:28
GAME( 199?, sc1clinsc       , sc1clins  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Lines (Bellfruit) (Scorpion 1) (set 4)", GAME_FLAGS)
// PROJECT NUMBER 5159  ALL CASH CASH LINES  GAME No 39-370-098 -  9-MAY-1989 14:40:54
GAME( 199?, sc1clinsd       , sc1clins  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Lines (Bellfruit) (Scorpion 1) (set 5)", GAME_FLAGS)
// PROJECT NUMBER 5159  CASH LINES  GAME No 39-370-056 -   17-NOV-1988 09:55:47
GAME( 199?, sc1clinse       , sc1clins  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Cash Lines (Bellfruit) (Scorpion 1) (set 6)", GAME_FLAGS) // bad rom

// PROJECT NUMBER 6108  HIGH POINT 20P PAYOUT S+P #3/#6  GAME No 39-370-787 - 22-FEB-1993 16:00:23
GAME( 199?, sc1hipt         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "High Point (Bellfruit) (Scorpion 1) (set 1)", GAME_FLAGS)
// PROJECT NUMBER 6108  HIGH POINT 10P PAYOUT S+P #3/#6  GAME No 39-370-793 - 24-FEB-1993 16:58:57
GAME( 199?, sc1hipta        , sc1hipt   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "High Point (Bellfruit) (Scorpion 1) (set 2)", GAME_FLAGS)

// just alarms (part of a video game maybe?)
// PROJECT NUMBER 6842  Fruit Pursuit  GAME No PR6842S12 -  3-DEC-1997 16:45:49
GAME( 199?, sc1frpus        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Fruit Pursuit (Bellfruit) (set 1) (Scorpion 1?)", GAME_FLAGS)
// no header data due to bad rom
GAME( 199?, sc1frpusa       , sc1frpus  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Fruit Pursuit (Bellfruit) (set 2) (Scorpion 1?)", GAME_FLAGS) // bad rom?

// PROJECT NUMBER 5422  CHAIN REACTION VARIABLE %  GAME No 39-370-084 -   30-MAR-1989 16:17:14
GAME( 199?, sc1chain        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Chain Reaction (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS)
// PROJECT NUMBER 5422  CHAIN REACTION VARIABLE %  GAME No 39-371-084 -   30-MAR-1989 16:17:14
GAME( 199?, sc1chainp       , sc1chain  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Chain Reaction (Bellfruit) (set 1, Protocol) (Scorpion 1)", GAME_FLAGS)



// no header data due to bad rom
GAME( 199?, sc1goldw        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Golden Winner (Bellfruit) (Scorpion ?)", GAME_FLAGS) /// bad rom

// no header data due to bad rom
GAME( 199?, sc1druby        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Diamonds & Rubys (Bellfruit) (Scorpion ?) (set 1)", GAME_FLAGS) // bad rom
// no header data due to bad rom
GAME( 199?, sc1drubya       , sc1druby  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM",      "Diamonds & Rubys (Bellfruit) (Scorpion ?) (set 2)", GAME_FLAGS) // bad rom



/********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
********************************************************************************************************************************************************************************************************************/

/* The BFM / ELAM Dutch releases are NOT the same games as the English ones with the same name */

/********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
********************************************************************************************************************************************************************************************************************/

/* PROM ERROR 3 */

// PROJECT NUMBER 6757  ACTIVE 8 SCORP I  GAME No 95-752-019 - 08-JAN-1998 14:46:17
GAME( 198?, sc1actv8        , 0         , scorpion1_viper   , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Active 8 (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6541  BIG MATCH SCORP I  GAME No 39-372-084 - 27-JUN-1996 15:56:53
GAME( 198?, sc1bigmt        , 0         , scorpion1_viper   , clatt     , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "The Big Match (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 1051  CASHCOIN SCORP I  GAME No 95100005 - 05-OCT-1998 13:44:52
GAME( 198?, sc1ccoin        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Cash Coin (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6883  DREAM MACHINE  GAME No 95-752-021 - 08-JAN-1998 14:25:32
GAME( 198?, sc1dream        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Dream Machine (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 1113  GOODTIMES SCORP I  GAME No 95-100-020 - 23-FEB-1999 11:02:32
GAME( 198?, sc1gtime        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Good Times (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 1112  TIARA SCORP I  GAME No 95-100-014 - 16-DEC-1998 10:37:21
GAME( 198?, sc1tiara        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Tiara (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6363  GRAND SLAM SCORP I  GAME No 39-372-041 - 14-SEP-1995 14:01:04
GAME( 198?, sc1gslam        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Grand Slam (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 1096  HAPPY HOUR  GAME No 95-100-025 - 15-JUL-1999 16:25:41
GAME( 198?, sc1happy        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Happy Hour (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6764  MASTER CLUB SCORP I  GAME No 95-750-971 - 26-MAY-1997 14:48:32
GAME( 198?, sc1mast         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Master Club (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6881  ULTIMATE SCORP I  GAME No 95-752-020 - 08-JAN-1998 12:12:12
GAME( 198?, sc1ult          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Ultimate (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6763  CLUB 3000 SCORP I  GAME No 95-750-920 - 05-FEB-1997 14:38:48
GAME( 199?, sc1clb3         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Club 3000 (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS)

/********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
********************************************************************************************************************************************************************************************************************/

/* All these boot */

// PROJECT NUMBER 6642  LOTUS                GAME No 95-750-911 - 07-JAN-1997 14:48:18
GAME( 1988, sc1lotus        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Lotus SE (Dutch) (set 1)", GAME_FLAGS )
// PROJECT NUMBER 6642  LOTUS                GAME No 95-750-911 - 07-JAN-1997 14:48:18
GAME( 1988, sc1lotusa       , sc1lotus  , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Lotus SE (Dutch) (set 2)", GAME_FLAGS )

// PROJECT NUMBER 5142  DUTCH ROULETTE  GAME No 39-360-029 - 18-APR-1989 16:03:00
GAME( 1988, sc1roul         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, rou029         , 0,       "BFM/ELAM", "Roulette (Dutch, Game Card 39-360-129?)", GAME_FLAGS )

// PROJECT NUMBER 6045       ARMADA          GAME No 39-370-729  - 28-AUG-1992 14:37:00
GAME( 198?, sc1armad        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Armada (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6043       CALYPSO         GAME No 39-370-754  - 08-DEC-1992 13:20:00
GAME( 198?, sc1calyp        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Calypso (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6041  CARROUSEL TOPBOX  GAME No 95-750-801 - 02-JUL-1996 10:43:35
GAME( 198?, sc1carro        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt_bank0  , 0,       "BFM/ELAM", "Carrousel (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5514  DUTCH 6 V1 1989  GAME No 39-370-650 - 29-JUL-1992 15:51:26
GAME( 198?, sc1cshcd        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Cash Card (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5512  DUTCH CASH EXPLOSION  GAME No 39-360-036 - 15-SEP-1989 15:04:00
GAME( 198?, sc1cexpd        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Cash Explosion (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5725  DUTCH 5 V1 1989  GAME No 39-370-608 - 18-JUN-1992 09:20:55
GAME( 198?, sc1cshin        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Cashino (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5729    CLASSIC     GAME No 39-370-606 - 18-JUN-1992 08:32:02
GAME( 198?, sc1class        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Classic (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6046  CLOWN AROUND  GAME No 95-750-786 - 19-JUN-1996 13:59:25
GAME( 198?, sc1clown        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Clown Around (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6641  DUTCH CLUB 2000 SPECIAL EDITION  GAME No 95-750-912 - 07-JAN-1997 13:10:21
GAME( 198?, sc1cl2k         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Club 2000 (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6050  CLUB 2001  GAME No 39-370-865 - 30-APR-1993 10:41:43
GAME( 198?, sc1cl2k1        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Club 2001 (Dutch (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5724  DUTCH CLUB DIAMOND  GAME No 39-360-097 - 13-JUL-1992 15:12:03
GAME( 198?, sc1clbdm        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Club Diamond (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6250  COPS N ROBBERS  GAME No 95-750-835 - 05-AUG-1996 16:33:34
GAME( 198?, sc1copdd        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Cops 'n' Robbers Deluxe (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5399  DUTCH 2 V1 1989  GAME No 39-370-651 - 29-JUL-1992 15:25:57
GAME( 198?, sc1disc         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Discovey (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5956  FLASH      GAME No 95-750-771 - 17-JUN-1996 09:30:53
GAME( 198?, sc1flash        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Flash (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5391  FRUIT LINES  GAME No 39-370-653 - 30-JUL-1992 09:31:10
GAME( 198?, sc1fruit        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Fruit Lines (Dutch) (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 5391  FRUIT LINES  GAME No 39-370-653 - 30-JUL-1992 09:31:10
GAME( 198?, sc1frtln        , sc1fruit  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Fruit Lines (Dutch) (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6048  GRAND PRIX  GAME No 39-370-805 - 26-MAR-1993 11:26:08
GAME( 198?, sc1gprix        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Grand Prix (Dutch) (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5957  Impact  GAME No 95-750-769 - 02-JUL-1996 12:10:32
GAME( 198?, sc1impc         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Impact (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6246  KINGS CLUB           GAME No 95-750-757  - 14-JUN-1996 14:06:15
GAME( 198?, sc1kings        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse_bank0    , 0,       "BFM/ELAM", "Kings Club (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5467  DUTCH MAGIC CIRCLE  GAME No 39-360-031 - 19-APR-1989 16:59:00
GAME( 198?, sc1magc         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Magic Circle (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECTNUMBER 5726  DUTCH MANHATTAN  GAME No 39-370-368 - 05-JUL-1991 15:01:00
GAME( 198?, sc1manha        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse_bank0    , 0,       "BFM/ELAM", "Manhattan (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5398  DUTCH QUATRO  GAME No 39-360-032 - 24-APR-1989 13:46:00
GAME( 198?, sc1quat         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Quatro (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5728  DUTCH MISTRAL  GAME No 95-750-796 - 20-JUN-1996 13:39:32
GAME( 198?, sc1mist         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse_bank0    , 0,       "BFM/ELAM", "Mistral (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5727  PHOENIX OLYMPIA  GAME No 39-372-031 - 28-APR-1995
GAME( 198?, sc1olym         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Olympia (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6247  ORACLE  GAME No 95-750-803 - 01-JUL-1996 11:31:21
GAME( 198?, sc1orac         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Oracle (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6645  RAINBOW  GAME No RBA8GMV6 - 21-OCT-1999 20:11:02
GAME( 198?, sc1rain         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Rainbow (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5511  DUTCH REEL CASH  GAME No 39-360-035 - 20-JUN-1989 09:01:00
GAME( 198?, sc1re           , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Reel Cash (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 5511  DUTCH REEL CASH 90 SPECIAL EDITION  GAME No 95-750-837 - 09-AUG-1996 16:14:43
GAME( 198?, sc1rese         , sc1re     , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Reel Cash SE (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS ) // doesn't say 'SE'

// PROJECT NUMBER 5390  DUTCH REVOLUTION  GAME No 39-360-020 - 23-FEB-1989 10:49:00
GAME( 198?, sc1revo         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Revolution (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6354  ROSE N CROWN 90 TOPBOX  GAME No 95-750-802 - 03-JUL-1996 14:29:11
GAME( 198?, sc1rose         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Rose 'n' Crown (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6245  SANTANA              GAME No 95-750-793 - 20-JUN-1996 13:01:14
GAME( 198?, sc1sant         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Santana (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5515  SATELLITE  GAME No 39-360-038 - 03-NOV-1989 14:27:00@
GAME( 198?, sc1sat          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Satellite (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 1052  SATELLITE  GAME No 95100000   - 16-MAR-1998  9:05:08
GAME( 199?, sc1satse        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Satellite SE (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS) // different game?

// PROJECT NUMBER 5959  SHANGHAI  GAME No 39-370-626 - 26-JUN-1992 14:04:06
GAME( 198?, sc1shan         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Shanghai (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5952  STERLING  GAME No 95-750-787 - 19-JUN-1996 13:16:41
GAME( 198?, sc1ster         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Sterling (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5367  DUTCH STRIKE 4  GAME No 01-ST8-0A1 - 30-AUG-1991 13:13:27
GAME( 198?, sc1str4         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Strike 4 (Dutch) (Bellfruit) (Scorpion 1) (set 1)", GAME_FLAGS )
// PROJECT NUMBER 5367  DUTCH STRIKE 4  GAME No 39-360-009 - 14-MRT-1988 16:40:00
GAME( 198?, sc1str4a        , sc1str4   , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "BFM/ELAM", "Strike 4 (Dutch) (Bellfruit) (Scorpion 1) (set 2)", GAME_FLAGS )

// PROJECT NUMBER 6244  TORNADO  GAME No 39-370-930 - 19-OCT-1993 12:16:25
GAME( 198?, sc1torn         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Tornado (Dutch) (Bellfruit) (set 1) (Scorpion 1)", GAME_FLAGS )
// PROJECT NUMBER 6244  TORNADO  GAME No 95-750-792 - 24-JUN-1996 12:16:25
GAME( 198?, sc1torna        , sc1torn   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Tornado (Dutch) (Bellfruit) (set 2) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5517  DUTCH 8 V1 1989  GAME No 39-370-939 -  3-NOV-1993 15:24:36
GAME( 198?, sc1vent         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Ventura (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 5721  DUTCH VICTORY  GAME No 39-360-043 - 05-APR-1990 16:30:00@
GAME( 198?, sc1vict         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Victory (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6241  WINFALLS             GAME No 39-370-809  - 17-MRT-1993 13:30:02
GAME( 198?, sc1winfl        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Winfalls (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS )

// PROJECT NUMBER 6882  STRIKE  SCORP I  GAME No 95-752-023 - 02-FEB-1998 11:23:13
GAME( 199?, sc1strk         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Strike (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS)

// PROJECT NUMBER 6766  SUPERFLUSH  GAME No 95-750-926 - 11-FEB-1997 16:28:47
GAME( 199?, sc1supfl        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "BFM/ELAM", "Super Flush (Dutch) (Bellfruit) (Scorpion 1)", GAME_FLAGS)





/********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
********************************************************************************************************************************************************************************************************************/

/* 3rd Party stuff */

/********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
********************************************************************************************************************************************************************************************************************/


/* ELAM, but not BFM, no BFM headers */

GAME( 198?, sc1dago         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "ELAM",     "Dagobert's Vault (Dutch) (Elam) (Scorpion 1)", GAME_FLAGS )

GAME( 198?, sc1spit         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "ELAM",     "Spitfire (Dutch) (Elam) (Scorpion 1)", GAME_FLAGS )

GAME( 198?, sc1voy          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "ELAM",     "Voyager (Dutch) (Elam) (set 1) (Scorpion 1)", GAME_FLAGS )
GAME( 198?, sc1voya         , sc1voy    , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "ELAM",     "Voyager (Dutch) (Elam) (set 2) (Scorpion 1)", GAME_FLAGS )

/* ELAM, but not BFM, BFM style header */

// PROJECT NUMBER 1005  ZEPPELIN  GAME No ZPA8GMV8 - 21-OCT-1999 20:30:55
GAME( 198?, sc1zep          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "ELAM",     "Zeppelin (Dutch) (Elam) (Scorpion 1)", GAME_FLAGS )

/* 3rd Party Games without BFM headers, many manufacturers are unknown / unconfirmed */

// has ELAM and Barcrest strings, but I think that's just relating to hardware it can hook up to
// has GOEDGEKEURD DOOR HET IJKWEZEN ONDER NR. TK-0000 near the end, like several other games we've seen
GAME( 198?, sc1final        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "<unknown>", "Final Touch (Dutch) (unknown) (Scorpion 1)", GAME_FLAGS ) // PAL ERROR
GAME( 198?, sc1sups         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "<unknown>", "Superstar (Dutch) (unknown) (Scorpion 1)", GAME_FLAGS ) // PAL ERROR
GAME( 198?, sc1wthn         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Eurocoin",  "Wild Thing (Eurocoin) (Scorpion 1)", GAME_FLAGS ) // PAL ERROR
GAME( 199?, sc1reply        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Eurocoin",  "Replay (Eurocoin) (Scorpion 1)", GAME_FLAGS) // PAL ERROR
// similar, but different error
GAME( 198?, sc1t1k          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Eurocoin",  "Top 1000 (Dutch) (Eurocoin) (Scorpion 1)", GAME_FLAGS ) // BATTERIJ DEFECT
GAME( 199?, sc1czbrk        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "<unknown>", "Crazy Break (Dutch) (unknown) (Scorpion 1)", GAME_FLAGS)  // BATTERIJ DEFECT
GAME( 199?, sc1energ        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "<unknown>", "Energy (Dutch) (unknown) (Scorpion 1)", GAME_FLAGS) // BATTERIJ DEFECT
GAME( 199?, sc1dip          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Eurocoin",  "Diplomat (Eurocoin) (Scorpion 1)", GAME_FLAGS) // BATTERIJ DEFECT (no sound)
GAME( 199?, sc1lamb         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Eurocoin",  "Lambada (Eurocoin) (Scorpion 1)", GAME_FLAGS) // BATTERIJ DEFECT


// Misc 3rd Party

GAME( 199?, sc1smoke        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Smokey Vs The Bandit (Mdm) (set 1) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )
GAME( 199?, sc1smokea       , sc1smoke  , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Smokey Vs The Bandit (Mdm) (set 2) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )

GAME( 199?, sc1ccroc        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Crazy Crocs (Mdm) (set 1) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )
GAME( 199?, sc1ccroca       , sc1ccroc  , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Crazy Crocs (Mdm) (set 2) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )
GAME( 199?, sc1ccrocb       , sc1ccroc  , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Crazy Crocs (Mdm) (set 3) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )
GAME( 199?, sc1ccrocc       , sc1ccroc  , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Crazy Crocs (Mdm) (set 4) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )

GAME( 199?, sc1crocr        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Croc And Roll (Mdm) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )

GAME( 199?, sc1btclk        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Beat The Clock (Mdm) (set 1) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )
GAME( 199?, sc1btclka       , sc1btclk  , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Beat The Clock (Mdm) (set 2) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )
GAME( 199?, sc1btclkb       , sc1btclk  , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Mdm",      "Beat The Clock (Mdm) (set 3) (Scorpion 2/3?)",   MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_NO_SOUND )

GAME( 199?, sc1days         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "All In A Days Work (Global) (set 1)", GAME_FLAGS)
GAME( 199?, sc1daysa        , sc1days   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "All In A Days Work (Global) (set 2)", GAME_FLAGS)

GAME( 199?, sc1cscl         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Cash Classic (Global) (set 1)", GAME_FLAGS)
GAME( 199?, sc1cscla        , sc1cscl   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Cash Classic (Global) (set 2)", GAME_FLAGS)

GAME( 199?, sc1driv         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Driving School (Global) (set 1)", GAME_FLAGS)
GAME( 199?, sc1driva        , sc1driv   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Driving School (Global) (set 2)", GAME_FLAGS)
GAME( 199?, sc1drivb        , sc1driv   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Driving School (Global) (set 3)", GAME_FLAGS)
GAME( 199?, sc1drivc        , sc1driv   , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Driving School (Global) (set 4)", GAME_FLAGS)

GAME( 199?, sc1vsd          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Vegas Super Deal (Global)", GAME_FLAGS)

GAME( 199?, sc1wof          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Wheel Of Fortune (Global) (set 1)", GAME_FLAGS)
GAME( 199?, sc1wofa         , sc1wof    , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Wheel Of Fortune (Global) (set 2)", GAME_FLAGS)
GAME( 199?, sc1wofb         , sc1wof    , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "Global",   "Wheel Of Fortune (Global) (set 3)", GAME_FLAGS)

GAME( 199?, sc1crzyc        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Crazy Cash (Global) (set 1)", GAME_FLAGS)
GAME( 199?, sc1crzyca       , sc1crzyc  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Crazy Cash (Global) (set 2)", GAME_FLAGS)

GAME( 199?, sc1clbdy        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Club Dynamite (Global) (set 1)", GAME_FLAGS)
GAME( 199?, sc1clbdya       , sc1clbdy  , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Club Dynamite (Global) (set 2)", GAME_FLAGS)

GAME( 199?, sc1chqfl        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Global",   "Chequered Flag (Global)", GAME_FLAGS)

GAME( 199?, sc1cdm          , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse_bank0    , 0,       "Crystal",  "Club Diamond (Crystal) (set 1) (Scorpion 1)", GAME_FLAGS)
GAME( 199?, sc1cdmp         , sc1cdm    , scorpion1         , scorpion1 , bfm_sc1_state, lotse_bank0    , 0,       "Crystal",  "Club Diamond (Crystal) (set 1, Protocol) (Scorpion 1)", GAME_FLAGS)

GAME( 199?, sc1hfcc         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse_bank0    , 0,       "Crystal",  "Hi Flyer Club (Crystal) (set 1) (Scorpion 1)", GAME_FLAGS)
GAME( 199?, sc1hfccp        , sc1hfcc   , scorpion1         , scorpion1 , bfm_sc1_state, lotse_bank0    , 0,       "Crystal",  "Hi Flyer Club (Crystal) (set 1, Protocol) (Scorpion 1)", GAME_FLAGS)

GAME( 199?, sc1moonl        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Bwb",      "Moon Lite (Bwb)", GAME_FLAGS)

GAME( 199?, sc1ltdv         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Pcp",      "Little Devil (Pcp)", GAME_FLAGS)

GAME( 199?, sc1twice        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse_bank0    , 0,       "Associated Leisure",   "Twice As Nice (Associated Leisure) (Scorpion 1)", GAME_FLAGS) // this has valid strings in it BEFORE the bfm decode, but decodes to valid code, does it use some funky mapping, or did they just fill unused space with valid looking data?

GAME( 1992, sc1s1000        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "Deltasoft","Super 1000 (Deltasoft)", GAME_FLAGS) // JT/Deltasoft Nov 1992

// these 2 are both the same manufacturer
GAME( 198?, sc1clbrn        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "<unknown>",      "Club Runner (Dutch) (unknown) (Scorpion 1)", GAME_FLAGS )
GAME( 198?, sc1clbsp        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, nocrypt        , 0,       "<unknown>",      "Club Spinner (Dutch) (unknown) (Scorpion 1)", GAME_FLAGS )

GAME( 199?, sc1scunk        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "<unknown>",      "unknown Scorpion 1 'Super ?' (Bellfruit) (Scorpion 1)", GAME_FLAGS) // ?

GAME( 199?, sc1wud          ,0          , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "<unknown>",      "What's Up Dr (Scorpion 1?)", GAME_FLAGS) // was in maygayep.c whats up doc set

GAME( 199?, sc1btbc         , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "<unknown>",      "Beat The Bank Club (unknown) (Scorpion 1?)", GAME_FLAGS) // behaves like sc1clbdya, but then locks up

GAME( 199?, sc1boncl        , 0         , scorpion1         , scorpion1 , bfm_sc1_state, lotse          , 0,       "<unknown>",      "Bonanza Club (unknown) (Scorpion 1)", GAME_FLAGS) // just alarms
