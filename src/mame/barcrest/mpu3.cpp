// license:BSD-3-Clause
// copyright-holders:James Wallace
// thanks-to:Chris Wren, Tony Friery, MFME
/* Notes 17/07/11 DH
 added most other MPU3 sets

 most fail to boot, giving the CPU a 'WAIT' instruction then sitting there
 some complain about Characterizer (protection) and then do the same
 a few boot to show light displays with no LED text
 some display misaligned LED text
 many run VERY slowly, even when the CPU is inactive (inefficient MAME timer system overhead?)

 */

/***********************************************************************************************************
  Barcrest MPU3 highly preliminary driver
  based on MPU4 driver (see mpu4.cpp for credits)

  Thanks to Chris Wren and MFME for documentation, particularly of meters and characteriser.

--- Board Setup ---

This original board uses a ~1.Mhz 6808 CPU, and a number of PIA6821 chips for multiplexing inputs and the like.

A 6840PTM is used for internal timing
A GAME CARD (cartridge) plugs into the board containing the game, and a protection PAL (the 'characteriser').

--- Preliminary MPU3 Memorymap  ---

   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+--------------------------------------------------------------------------
 0000-07FF |R/W| D D D D D D D D | 2k RAM
-----------+---+-----------------+--------------------------------------------------------------------------

 8800-881F |R/W| ???????????????? | MC6840 PTM IC2 (unusual bus setup, address shifted two bytes)

Outputs hooked into IC 19

  Clock1 <--------------------------------------
     |                                          |
     V                                          |
  Output1 ---> Clock2                           |
                                                |
               Output2 --+-> Clock3             |
                         |                      |
                         |   Output3 ---> 'to audio amp' ??
                         |
                         +--------> CA1 IC3 (

IRQ line connected to CPU

-----------+---+-----------------+--------------------------------------------------------------------------
9000-      |R/W| D D D D D D D D | PIA6821 IC3
           |   |                 | PA2-PA7 INPUT  multiplexed inputs data
           |   |                 | PA1, CB1 connected to IC2 C1 pin
           |   |                 |        CA1     INPUT, not connected
           |   |                 |        CA2    OUTPUT, A on strobe multiplexer
           |   |                 |        IRQA           Not connected
           |   |                 |
           |   |                 |        CB2 IC19 Triac switch
           |   |                 | Port B triacs

9800       |R/W| D D D D D D D D | PIA6821 IC4
           |   |                 | PA0-PA6 INPUT  Yellow 15 way data (7SEG LED or Meters)
           |   |                 | PA7     802 (Q) IC11 G1
           |   |                 | PB0-PB7 INPUT  IC24
           |   |                 |        CA2    OUTPUT, B on strobe multiplexer
           |   |                 |        CB2    TRiac switch

A000-A003  |R/W| D D D D D D D D | PIA6821 IC5 port A Reel drives
           |   |                 |          port B Reel opto
           |   |                 |
           |   |                 |        CA2    OUTPUT, C on strobe multiplexer
           |   |                 |        CB2    TRiac switch
-----------+---+-----------------+--------------------------------------------------------------------------
A800-A803  |R/W| D D D D D D D D | PIA6821 IC6 PA0-PA7, AUX1 connector (ALPHA)
           |   |                 |             PB0-PB7, AUX2 connector
           |   |                 |
           |   |                 |

-----------+---+-----------------+--------------------------------------------------------------------------
??? B000-FFFF | R | D D D D D D D D | ROM
-----------+---+-----------------+--------------------------------------------------------------------------

NOTES:
A number of different modifications exist to the main board, essentially all are compatible with one another aside from RAM sizes.

MOD0- 3 - early boards with small RAM allocations
MOD4- Some modifications on the PCB that didnt work, so field engineers reverted them to MOD3.
MOD5- board revision with bigger RAM and reset sensitivity circuit added on the PCB.

MOD6- adaptation of the PCB to use small daughter card with 6116 RAM

Collectors have gone further with zero power RAM and the like, but these are the ones out in the wild.

TODO: - Distinguish door switches using manual
      - Add discrete sound functionality (same as MPU4 alarm)
      - Is pulse timer ic11 right?
      - 12V meter alarm on hyper viper - how are meters sensed?

***********************************************************************************************************/

#include "emu.h"
#include "awpvid.h"       //Fruit Machines Only

#include "mpu4_characteriser_pal.h"

#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/timer.h"

#include "cpu/m6800/m6800.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/meters.h"
#include "machine/rescap.h"

#include "mpu3.lh"

// MFME2MAME layouts:
#include "m3acech.lh"
#include "m3autort.lh"
#include "m3biggam.lh"
#include "m3bigsht.lh"
#include "m3blkhle.lh"
#include "m3cabreta.lh"
#include "m3chase.lh"
#include "m3circle.lh"
#include "m3cskill.lh"
#include "m3cunlim.lh"
#include "m3fortun.lh"
#include "m3gcrown.lh"
#include "m3gmine.lh"
#include "m3lineup.lh"
#include "m3loony.lh"
#include "m3nnice.lh"
#include "m3oddson.lh"
#include "m3online.lh"
#include "m3oxo.lh"
#include "m3ratrce.lh"
#include "m3razdaza.lh"
#include "m3replay.lh"
#include "m3rockpl.lh"
#include "m3rollem.lh"
#include "m3rxchng.lh"
#include "m3sexcu.lh"
#include "m3slight.lh"
#include "m3snaphp.lh"
#include "m3spoof.lh"
#include "m3supadr.lh"
#include "m3supnud.lh"
#include "m3supspo.lh"
#include "m3supwin.lh"
#include "m3sweep.lh"
#include "m3tfair.lh"
#include "m3toplin.lh"
#include "m3winagnb.lh"
#include "m3winstr.lh"
#include "m3xchngg.lh"

#ifdef MAME_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif

#include "logmacro.h"


namespace {

class mpu3_state : public driver_device
{
public:
	mpu3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_reels(*this, "reel%u", 0U)
		, m_meters(*this, "meters")
		, m_vfd(*this, "vfd")
		, m_triac(*this, "triac%u", 0U)
		, m_digit(*this, "digit%u", 0U)
		, m_lamp(*this, "lamp%u", 0U)
		, m_pia3(*this, "pia_ic3")
		, m_pia4(*this, "pia_ic4")
		, m_pia5(*this, "pia_ic5")
		, m_pia6(*this, "pia_ic6")
		, m_ptm2(*this, "ptm_ic2")
	{ }

	void mpu3base(machine_config &config);

	void init_mpu3();

protected:
	void mpu3_basemap(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;

private:
	template <unsigned N> void reel_optic_cb(int state) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }

	void mpu3ptm_w(offs_t offset, uint8_t data);
	uint8_t mpu3ptm_r(offs_t offset);
	void ic2_o1_callback(int state);
	void ic2_o2_callback(int state);
	void ic2_o3_callback(int state);
	uint8_t pia_ic3_porta_r();
	void pia_ic3_portb_w(uint8_t data);
	void pia_ic3_ca2_w(int state);
	uint8_t pia_ic4_porta_r();
	void pia_ic4_porta_w(uint8_t data);
	void pia_ic4_portb_w(uint8_t data);
	void pia_ic4_ca2_w(int state);
	void pia_ic4_cb2_w(int state);
	void pia_ic5_porta_w(uint8_t data);
	uint8_t pia_ic5_portb_r();
	void pia_ic5_portb_w(uint8_t data);
	void pia_ic5_ca2_w(int state);
	void pia_ic5_cb2_w(int state);
	uint8_t pia_ic6_porta_r();
	uint8_t pia_ic6_portb_r();
	void pia_ic6_porta_w(uint8_t data);
	void pia_ic6_portb_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(ic21_timeout);
	TIMER_DEVICE_CALLBACK_MEMBER(gen_50hz);
	TIMER_DEVICE_CALLBACK_MEMBER(ic10_callback);
	void update_triacs();
	void ic11_update();
	void ic21_output(int data);
	void ic21_setup();

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	int m_triac_ic3 = 0;
	int m_triac_ic4 = 0;
	int m_triac_ic5 = 0;
	int m_ic3_data = 0;
	int m_IC11G1 = 0;
	int m_IC11G2A = 0;
	int m_IC11G2B = 0;
	int m_IC11GC = 0;
	int m_IC11GB = 0;
	int m_IC11GA = 0;

	int m_ic10_output = 0;
	int m_ic11_active = 0;
	int m_disp_func = 0;

	int m_ic4_input_a = 0;
	int m_aux1_input = 0;
	int m_aux2_input = 0;
	int m_input_strobe = 0;   /* IC11 74LS138 A = CA2 IC3, B = CA2 IC4, C = CA2 IC5 */
	uint8_t m_lamp_strobe = 0;
	uint8_t m_led_strobe = 0;
	int m_signal_50hz = 0;

	int m_optic_pattern = 0;

	emu_timer *m_ic21_timer = nullptr;

	required_shared_ptr<uint8_t> m_nvram;
	required_device_array<stepper_device, 4> m_reels;
	required_device<meters_device> m_meters;
	optional_device<rocvfd_device> m_vfd;
	output_finder<8> m_triac;
	output_finder<8> m_digit;
	output_finder<8 * 8> m_lamp;
	required_device<pia6821_device> m_pia3;
	required_device<pia6821_device> m_pia4;
	required_device<pia6821_device> m_pia5;
	required_device<pia6821_device> m_pia6;
	required_device<ptm6840_device> m_ptm2;
};

#define DISPLAY_PORT 0
#define METER_PORT 1
#define BWB_FUNCTIONALITY 2


class mpu3_chr_state : public mpu3_state
{
public:
	mpu3_chr_state(const machine_config &mconfig, device_type type, const char *tag)
		: mpu3_state(mconfig, type, tag)
		, m_characteriser(*this, "characteriser")
	{ }

	void mpu3_chr_3000(machine_config &config);
	void mpu3_chr_c000(machine_config &config);

private:
	void mpu3_map_chr_3000(address_map &map) ATTR_COLD;
	void mpu3_map_chr_c000(address_map &map) ATTR_COLD;

	required_device<mpu4_characteriser_pal> m_characteriser;
};


void mpu3_state::update_triacs()
{
	int const triacdata = m_triac_ic3 + (m_triac_ic4 << 8) + (m_triac_ic5 << 9);

	// FIXME: m_triac_ic4 and m_triac_ic5 won't be used because they're shifted past what's checked here
	for (int i = 0; i < 8; i++)
		m_triac[i] = BIT(triacdata, i);
}

/* called if board is reset */
void mpu3_state::machine_reset()
{
	m_vfd->reset();

	m_lamp_strobe   = 0;
	m_led_strobe    = 0;

	m_IC11GC    = 0;
	m_IC11GB    = 0;
	m_IC11GA    = 0;
	m_IC11G1    = 1;
	m_IC11G2A   = 0;
	m_IC11G2B   = 0;
}


/* IC2 6840 PTM handler probably clocked from elsewhere*/
void mpu3_state::ic2_o1_callback(int state)
{
}

//FIXME FROM HERE
void mpu3_state::ic2_o2_callback(int state)
{
}


void mpu3_state::ic2_o3_callback(int state)
{
}

/*
IC23 emulation

IC23 is a 74LS138 1-of-8 Decoder

It is used as a multiplexer for the LEDs, lamp selects and inputs.*/

void mpu3_state::ic11_update()
{
	if (!m_IC11G2A)
	{
		if (!m_IC11G2B)
		{
			if (m_IC11G1)
			{
				if ( m_IC11GA )  m_input_strobe |= 0x01;
				else             m_input_strobe &= ~0x01;

				if ( m_IC11GB )  m_input_strobe |= 0x02;
				else             m_input_strobe &= ~0x02;

				if ( m_IC11GC )  m_input_strobe |= 0x04;
				else             m_input_strobe &= ~0x04;
			}
		}
	}
	else if ((m_IC11G2A)||(m_IC11G2B)||(!m_IC11G1))
	{
		m_input_strobe = 0x00;
	}
}


/*
IC21 emulation

IC21 is an 8602 or equivalent pulse generator

CLEAR and B2 are tied high, so a high to low transition on A2
(tied to IC11 GA) will trigger operation.

Cx = 2.2 uF

Rx = 47k

K = ~0.34

t/ns = .34 * 47 * 2.2e6 [ 1+ (1/47)]


This seems less stable than the revised version used in MPU4
*/
void mpu3_state::ic21_output(int data)
{
	m_IC11G1 = data;
	ic11_update();
}

void mpu3_state::ic21_setup()
{
	if (m_IC11GA)
	{
		{
			m_ic11_active=1;
			ic21_output(1);
			m_ic21_timer->adjust(attotime::from_nsec( (0.34 * 47 * 2200000) *(1+(1/47))));
		}
	}
}

TIMER_CALLBACK_MEMBER(mpu3_state::ic21_timeout)
{
	m_ic11_active=0;
	ic21_output(0);
}

uint8_t mpu3_state::pia_ic3_porta_r()
{
	static const char *const portnames[] = { "ORANGE1", "ORANGE2", "BLACK1", "BLACK2", "DIL1", "DIL1", "DIL2", "DIL2" };
	int data=0,swizzle;
	LOG("%s: IC3 PIA Read of Port A (MUX input data)\n", machine().describe_context());
	//popmessage("%x",m_input_strobe);
	switch (m_input_strobe)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			data = (ioport(portnames[m_input_strobe])->read()<<2);
			break;
		}
		case 4://DIL1
		case 6://DIL2
		{
			swizzle = (ioport(portnames[m_input_strobe])->read());
			data = (((swizzle & 0x01) << 7) + ((swizzle & 0x02) << 5) + ((swizzle & 0x04) << 3)
					+ ((swizzle & 0x08) << 1) +((swizzle & 0x10) >> 1) + ((swizzle & 0x20) >> 3));
			break;
		}
		case 5://DIL1
		case 7://DIL2
		{
			swizzle = (ioport(portnames[m_input_strobe])->read());
			data = (((swizzle & 0x80) >> 1) + ((swizzle & 0x40) << 1));
			break;
		}
	}
	if (m_signal_50hz)
	{
		data |= 0x02;
	}
	else
	{
		data &= ~0x02;
	}
	return data;
}

void mpu3_state::pia_ic3_portb_w(uint8_t data)
{
	LOG("%s: IC3 PIA Port B Set to %2x (Triac)\n", machine().describe_context(),data);
	m_triac_ic3 =data;
}


void mpu3_state::pia_ic3_ca2_w(int state)
{
	LOG("%s: IC3 PIA Port CA2 Set to %2x (input A)\n", machine().describe_context(),state);
	m_IC11GA = state;
	ic21_setup();
	ic11_update();
}

uint8_t mpu3_state::pia_ic4_porta_r()
{
	if (m_ic11_active)
	{
		m_ic4_input_a|=0x80;
	}
	else
	{
		m_ic4_input_a&=~0x80;
	}
	return m_ic4_input_a;
}

/*  IC4, 7 seg leds */
void mpu3_state::pia_ic4_porta_w(uint8_t data)
{
	LOG("%s: IC4 PIA Port A Set to %2x (DISPLAY PORT)\n", machine().describe_context(),data);
	m_ic4_input_a=data;
	switch (m_disp_func)
	{
	case DISPLAY_PORT:
		if(m_ic11_active)
		{
			if(m_led_strobe != m_input_strobe)
			{
				int const swizzle = ((m_ic4_input_a & 0x01) << 2)+(m_ic4_input_a & 0x02)+((m_ic4_input_a & 0x4) >> 2)+(m_ic4_input_a & 0x08)+((m_ic4_input_a & 0x10) << 2)+(m_ic4_input_a & 0x20)+((m_ic4_input_a & 0x40) >> 2);
				m_digit[7 - m_input_strobe] = swizzle;
			}
			m_led_strobe = m_input_strobe;
		}
		break;

	case METER_PORT:
		for (int meter = 0; meter < 6; meter ++)
		{
			int const swizzle = ((m_ic4_input_a ^ 0xff) & 0x3f);
			m_meters->update(meter, swizzle & (1 << meter));
		}
		break;

	case BWB_FUNCTIONALITY:
			//Need to find a game to work this out, MFME has a specific option for it, but I see no activity there.
		break;

	}
}

void mpu3_state::pia_ic4_portb_w(uint8_t data)
{
	LOG("%s: IC4 PIA Port B Set to %2x (Lamp)\n", machine().describe_context(),data);
	if (m_ic11_active)
	{
		if (m_lamp_strobe != m_input_strobe)
		{
			// Because of the nature of the lamping circuit, there is an element of persistance where the lamp retains residual charge
			// As a consequence, the lamp column data can change before the input strobe (effectively writing 0 to the previous strobe)
			// without causing the relevant lamps to black out.

			for (int i = 0; i < 8; i++)
				m_lamp[(m_input_strobe << 3)|i] = BIT(data, i);

			m_lamp_strobe = m_input_strobe;
		}
	}
}

void mpu3_state::pia_ic4_ca2_w(int state)
{
	LOG("%s: IC4 PIA Port CA2 Set to %2x (Input B)\n", machine().describe_context(),state);
	m_IC11GB = state;
	ic11_update();
}

void mpu3_state::pia_ic4_cb2_w(int state)
{
	LOG("%s: IC4 PIA Port CA2 Set to %2x (Triac)\n", machine().describe_context(),state);
	m_triac_ic4=state;
}

/* IC5, AUX ports, coin lockouts and AY sound chip select (MODs below 4 only) */
void mpu3_state::pia_ic5_porta_w(uint8_t data)
{
	LOG("%s: IC5 PIA Port A Set to %2x (Reel)\n", machine().describe_context(),data);
	m_reels[0]->update( data     & 0x03);
	m_reels[1]->update((data>>2) & 0x03);
	m_reels[2]->update((data>>4) & 0x03);
	m_reels[3]->update((data>>6) & 0x03);
	awp_draw_reel(machine(),"reel1", *m_reels[0]);
	awp_draw_reel(machine(),"reel2", *m_reels[1]);
	awp_draw_reel(machine(),"reel3", *m_reels[2]);
	awp_draw_reel(machine(),"reel4", *m_reels[3]);
}

uint8_t mpu3_state::pia_ic5_portb_r()
{
	if (m_ic3_data & 0x80 )
	{
		return m_optic_pattern;
	}
	else
	{
		return m_ic3_data;
	}
}



void mpu3_state::pia_ic5_portb_w(uint8_t data)
{
	m_ic3_data = data;
}

void mpu3_state::pia_ic5_ca2_w(int state)
{
	LOG("%s: IC5 PIA Port CA2 Set to %2x (C)\n", machine().describe_context(),state);
	m_IC11GC = state;
	ic11_update();
}

void mpu3_state::pia_ic5_cb2_w(int state)
{
	LOG("%s: IC5 PIA Port CB2 Set to %2x (Triac)\n", machine().describe_context(),state);
	m_triac_ic5 = state;
}


/* IC6, AUX ports*/
uint8_t mpu3_state::pia_ic6_porta_r()
{
	return (ioport("AUX1")->read())|m_aux1_input;
}


uint8_t mpu3_state::pia_ic6_portb_r()
{
	return (ioport("AUX2")->read())|m_aux2_input;
}

void mpu3_state::pia_ic6_porta_w(uint8_t data)
{
	LOG("%s: IC6 PIA Port A Set to %2x (Alpha)\n", machine().describe_context(),data);
	m_vfd->por((data & 0x08));
	m_vfd->data((data & 0x20) >> 5);
	m_vfd->sclk((data & 0x10) >>4);
}

void mpu3_state::pia_ic6_portb_w(uint8_t data)
{
	LOG("%s: IC6 PIA Port B Set to %2x (AUX2)\n", machine().describe_context(),data);
	m_aux2_input = data;
}

static INPUT_PORTS_START( mpu3 )
	PORT_START("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("01")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("02")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("03")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("04")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("05")


	PORT_START("ORANGE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("08")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("09")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Exchange")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("11")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("12")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("13")

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hi")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Lo")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("Auto Nudge")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("20p")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10p")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Token")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hold 2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Hold 3")

	PORT_START("DIL1")
	PORT_DIPNAME( 0x80, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x40, "DIL102" ) PORT_DIPLOCATION("DIL1:02") // Hyper Viper boots with this ON
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x01, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x80, 0x00, "DIL201" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL204" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL206" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x01, 0x00, "DIL208" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7")

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")PORT_IMPULSE(5)
INPUT_PORTS_END

/* Common configurations */
void mpu3_state::machine_start()
{
	m_ic21_timer = timer_alloc(FUNC(mpu3_state::ic21_timeout), this);

	m_triac.resolve();
	m_digit.resolve();
	m_lamp.resolve();
}

/* generate a 50 Hz signal (some components rely on this for external sync) */
TIMER_DEVICE_CALLBACK_MEMBER(mpu3_state::gen_50hz)
{
	/* Although reported as a '50Hz' signal, the fact that both rising and
	falling edges of the pulse are used means the timer actually gives a 100Hz
	oscillating signal.*/
	m_signal_50hz = m_signal_50hz?0:1;
	m_ptm2->set_c1(m_signal_50hz);
	m_pia3->cb1_w(~m_signal_50hz);
	update_triacs();
}

TIMER_DEVICE_CALLBACK_MEMBER(mpu3_state::ic10_callback)
{
	// TODO: Use discrete handler for 555, this is far too simplistic

	m_ic10_output = m_ic10_output?0:1;
	m_ptm2->set_c2(m_ic10_output);
	m_pia4->ca1_w(m_ic10_output);

}
void mpu3_state::mpu3ptm_w(offs_t offset, uint8_t data)
{
	m_ptm2->write(offset >>2,data);//((offset & 0x1f) >>2),data);
}

uint8_t mpu3_state::mpu3ptm_r(offs_t offset)
{
	return m_ptm2->read(offset >>2);
}

void mpu3_state::mpu3_basemap(address_map &map)
{
	map(0x0000, 0x07ff).ram().share(m_nvram);
	map(0x1000, 0xffff).rom();
	map(0x8800, 0x881f).rw(FUNC(mpu3_state::mpu3ptm_r), FUNC(mpu3_state::mpu3ptm_w));/* PTM6840 IC2 */
	map(0x9000, 0x9003).rw(m_pia3, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC3 */
	map(0x9800, 0x9803).rw(m_pia4, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC4 */
	map(0xa000, 0xa003).rw(m_pia5, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC5 */
	map(0xa800, 0xa803).rw(m_pia6, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC6 */
}

void mpu3_chr_state::mpu3_map_chr_3000(address_map &map)
{
	mpu3_basemap(map);
	map(0x3000, 0x3000).rw(m_characteriser, FUNC(mpu4_characteriser_pal::read), FUNC(mpu4_characteriser_pal::write));
}

void mpu3_chr_state::mpu3_map_chr_c000(address_map &map)
{
	mpu3_basemap(map);
	map(0xc000, 0xc000).rw(m_characteriser, FUNC(mpu4_characteriser_pal::read), FUNC(mpu4_characteriser_pal::write));
}

void mpu3_state::mpu3base(machine_config &config)
{
	M6808(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu3_state::mpu3_basemap);

	input_merger_device &mainirq(INPUT_MERGER_ANY_HIGH(config, "mainirq")); // open collector
	mainirq.output_handler().set_inputline("maincpu", M6808_IRQ_LINE);

	MSC1937(config, m_vfd);

	TIMER(config, "50hz").configure_periodic(FUNC(mpu3_state::gen_50hz), attotime::from_hz(100));
	TIMER(config, "555_ic10").configure_periodic(FUNC(mpu3_state::ic10_callback), PERIOD_OF_555_ASTABLE(10000,1000,0.0000001));

	/* 6840 PTM */
	PTM6840(config, m_ptm2, 4_MHz_XTAL / 4);
	m_ptm2->set_external_clocks(0, 0, 0);
	m_ptm2->o1_callback().set(FUNC(mpu3_state::ic2_o1_callback));
	m_ptm2->o2_callback().set(FUNC(mpu3_state::ic2_o2_callback));
	m_ptm2->o3_callback().set(FUNC(mpu3_state::ic2_o3_callback));
	m_ptm2->irq_callback().set("mainirq", FUNC(input_merger_device::in_w<0>));

	PIA6821(config, m_pia3);
	m_pia3->readpa_handler().set(FUNC(mpu3_state::pia_ic3_porta_r));
	m_pia3->writepb_handler().set(FUNC(mpu3_state::pia_ic3_portb_w));
	m_pia3->ca2_handler().set(FUNC(mpu3_state::pia_ic3_ca2_w));
	m_pia3->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia4);
	m_pia4->readpa_handler().set(FUNC(mpu3_state::pia_ic4_porta_r));
	m_pia4->writepa_handler().set(FUNC(mpu3_state::pia_ic4_porta_w));
	m_pia4->writepb_handler().set(FUNC(mpu3_state::pia_ic4_portb_w));
	m_pia4->ca2_handler().set(FUNC(mpu3_state::pia_ic4_ca2_w));
	m_pia4->cb2_handler().set(FUNC(mpu3_state::pia_ic4_cb2_w));
	m_pia4->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia5);
	m_pia5->readpb_handler().set(FUNC(mpu3_state::pia_ic5_portb_r));
	m_pia5->writepa_handler().set(FUNC(mpu3_state::pia_ic5_porta_w));
	m_pia5->writepb_handler().set(FUNC(mpu3_state::pia_ic5_portb_w));
	m_pia5->ca2_handler().set(FUNC(mpu3_state::pia_ic5_ca2_w));
	m_pia5->cb2_handler().set(FUNC(mpu3_state::pia_ic5_cb2_w));

	PIA6821(config, m_pia6);
	m_pia6->readpa_handler().set(FUNC(mpu3_state::pia_ic6_porta_r));
	m_pia6->readpb_handler().set(FUNC(mpu3_state::pia_ic6_portb_r));
	m_pia6->writepa_handler().set(FUNC(mpu3_state::pia_ic6_porta_w));
	m_pia6->writepb_handler().set(FUNC(mpu3_state::pia_ic6_portb_w));
	m_pia6->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<3>));
	m_pia6->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<4>));

	REEL(config, m_reels[0], MPU3_48STEP_REEL, 96, 2, 0x00, 2);
	m_reels[0]->optic_handler().set(FUNC(mpu3_state::reel_optic_cb<0>));
	REEL(config, m_reels[1], MPU3_48STEP_REEL, 96, 2, 0x00, 2);
	m_reels[1]->optic_handler().set(FUNC(mpu3_state::reel_optic_cb<1>));
	REEL(config, m_reels[2], MPU3_48STEP_REEL, 96, 2, 0x00, 2);
	m_reels[2]->optic_handler().set(FUNC(mpu3_state::reel_optic_cb<2>));
	REEL(config, m_reels[3], MPU3_48STEP_REEL, 96, 2, 0x00, 2);
	m_reels[3]->optic_handler().set(FUNC(mpu3_state::reel_optic_cb<3>));

	METERS(config, m_meters, 0).set_number(8);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2x HM4334 or HMI6514 or MB8414 + 2.4V battery

	config.set_default_layout(layout_mpu3);
}

void mpu3_chr_state::mpu3_chr_3000(machine_config &config)
{
	mpu3base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu3_chr_state::mpu3_map_chr_3000);

	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("maincpu");
	m_characteriser->set_allow_6800_cheat(true);
}

void mpu3_chr_state::mpu3_chr_c000(machine_config &config)
{
	mpu3base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu3_chr_state::mpu3_map_chr_c000);

	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("maincpu");
	m_characteriser->set_allow_6800_cheat(true);
}

void mpu3_state::init_mpu3()
{
	m_disp_func = METER_PORT;
}

ROM_START( m3tst )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut5.bin",  0x7800, 0x0800,  CRC(89994adb) SHA1(03e38a860b6ba9d7ae2dbfa7845e85bcf67a6f4d))
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END



ROM_START( m3acech )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ac10p.p1", 0x6000, 0x2000, CRC(bf72f9cd) SHA1(78885c39d08949cfac662c37564e71b01129b9bb) )
	ROM_LOAD( "ac10p.p2", 0x4000, 0x2000, CRC(2c4fb553) SHA1(65f7d9f07a86a447d77b57d889fd33658531b16d) )
	ROM_LOAD( "ac10p.p3", 0x2000, 0x2000, CRC(63955392) SHA1(ee52f42928f5040260e2ad021cf6a668aa02b0f8) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END




ROM_START( m3autort )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ar1.bin", 0x6000, 0x2000, CRC(00ff29b7) SHA1(62c73f313ed8b862fd1e8f261bceafdcb4dd9cdb) )
	ROM_LOAD( "ar2.bin", 0x4000, 0x2000, CRC(ca084dbf) SHA1(86eb343e6b252cfd6efcfa4f6166f53fd6627c01) )
	ROM_LOAD( "ar3.bin", 0x2000, 0x2000, CRC(6fbe9fa8) SHA1(0b66cb6274c5ffdfff9caf1171a13b3a719ae537) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3bankr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "bankerp1.bin", 0x6000, 0x2000, CRC(912e7c86) SHA1(67de6af2620a5caae34f76266529bd2e0b2dd6de) )
	ROM_LOAD( "bankerp2.bin", 0x5000, 0x1000, CRC(7fa01e70) SHA1(82b7a07548cab8d7828409f6b9a0676db0664be2) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3big20j )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "b20j11.p1", 0x6000, 0x2000, CRC(ce524bb5) SHA1(f7f21982376a8e5cd6f3f8b6d966a17044e6c1a8) )
	ROM_LOAD( "b20j11.p2", 0x5000, 0x1000, CRC(8ce0fffb) SHA1(eda619a1d21cd45eae0c9962dab6559d4b72fc7b) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3biggam )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "b2g2-4.p1", 0x6000, 0x002000, CRC(2bd82bcb) SHA1(58140b0ac6035652110dc0a2b2bd39980d5c518b) )
	ROM_LOAD( "b2g2-4.p2", 0x4000, 0x002000, CRC(4cbe3dd2) SHA1(3a5d2e693dfb615365eb83c1c3d7ce1144c94b4c) )
	ROM_LOAD( "b2g2-4.p3", 0x2000, 0x002000, CRC(2287a208) SHA1(21566c2b713e2e41fe400632510255455cd8ce45) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3bigsht )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "bs.p1", 0x6000, 0x2000, CRC(9a8cf66e) SHA1(de7c916722e20463c1838ad0848826b4a879e9ae) )
	ROM_LOAD( "bs.p2", 0x4000, 0x2000, CRC(113927b3) SHA1(c1dee8f967b1678b37fe4109e1ea5d4782882e2b) )
	ROM_LOAD( "bs.p3", 0x2000, 0x2000, CRC(ebe9fd8b) SHA1(d1d4184007f998f68db10dbc0f534fc52cc4f6ee) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3blkhle )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "p1-4460.bin", 0x7000, 0x1000, CRC(b3f82e48) SHA1(e3968c0e41c3b30fea6e8270aa67e078495fbc4c) )
	ROM_LOAD( "p2-4460.bin", 0x6000, 0x1000, CRC(d7dac5a8) SHA1(1e1f8e472e23b617854aab13b4cb6ba5ee21a691) )
	ROM_LOAD( "p3-4460.bin", 0x5000, 0x1000, CRC(4ba3ce96) SHA1(599b77026a6351bf1cad0598fd4578c3f65026e9) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )

	ROM_REGION( 0x10000, "altrevs", ROMREGION_ERASE00  )
	// missing rom 1, or is it the same?
	ROM_LOAD( "bh2p2.bin", 0x6000, 0x001000, CRC(30325800) SHA1(4d49e20e53e46e33d1f4f9008d7943e8595aa81c) )
	ROM_LOAD( "bh2p3.bin", 0x5000, 0x001000, CRC(6a1dcb32) SHA1(23614d366f2c04494d73f4ff36aba4e444a06b3a) )
ROM_END


ROM_START( m3cabret )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "cab50.p1", 0x7000, 0x1000, CRC(7c651961) SHA1(fdca9e7513d5ae0696c4011eedc05e3abdde074d) )
	ROM_LOAD( "cab50.p2", 0x6000, 0x1000, CRC(f187d599) SHA1(7c6482aa4cd426e8046ceb50a18b1a2794f514f6) )
	ROM_LOAD( "cab50.p3", 0x5000, 0x1000, CRC(95745414) SHA1(17da3491a77ae7c3320b9d8869f5f01d4a64ea8e) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3cabreta )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "cch12-0.p1", 0x7000, 0x1000, CRC(49376c4b) SHA1(fb8070d43ca103acdc3f120c5bc2b84603998404) )
	ROM_LOAD( "cch12-0.p2", 0x6000, 0x1000, CRC(e1d7d4d8) SHA1(107d8ebce24e946119bc7ae54085e847c1bef9b9) )
	ROM_LOAD( "cch12-0.p3", 0x5000, 0x1000, CRC(dd3a4d5e) SHA1(d9fa506a8f1d68cc4b7d2d91a4e524f39e269860) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END




ROM_START( m3cdash )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "cashdash.bin", 0x6000, 0x2000, CRC(1e8ff344) SHA1(021cca5079d0979767143babd23a6f5da93bc151) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3cunlim )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "cu1_2.p1", 0x6000, 0x2000, CRC(bbfe4e36) SHA1(8e3d9809d882a7608b9c75f8950e2d7ad30b2584) )
	ROM_LOAD( "cu1_2.p2", 0x4000, 0x2000, CRC(183a90a3) SHA1(faf00b248677538932da036135354ed38187cac3) )
	ROM_LOAD( "cu1_2.p3", 0x2000, 0x2000, CRC(a83497d4) SHA1(5092e5c7501e7293e33e5a93177047e3836bcd01) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

//VFS conversion based around Chances and Options unlimited
ROM_START( m3mremon )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "mmchancs.bin", 0x6000, 0x2000, CRC(3eaede51) SHA1(6914fcaed6e51736c6dc725ba82b691803571222) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3chase )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "chaseit1.bin", 0x6000, 0x2000, CRC(bc379354) SHA1(5375774ff7afa0f4623a174de69770630219b446) )
	ROM_LOAD( "chaseit2.bin", 0x4000, 0x2000, CRC(2bcabf7b) SHA1(f4d3e9691ddb68e9b678199a2474db1d9383e73e) )
	ROM_LOAD( "chaseit3.bin", 0x2000, 0x2000, CRC(6a761b8c) SHA1(3dfc30c13b423368b460b6225399508a00f14c24) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3cskill )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "cs_03_p1.bin", 0x6000, 0x2000, CRC(b230cc6c) SHA1(8cade054c6a3a57991db046baf250d6a5d47eedd) )
	ROM_LOAD( "cs_03_p2.bin", 0x4000, 0x2000, CRC(c3acb5be) SHA1(856401da7a84c7495cae7bea5b64c909b1a5cfe9) )
	ROM_LOAD( "cs_03_p3.bin", 0x2000, 0x2000, CRC(ad5de108) SHA1(92304e8da523623606c9ed23b82fcbd5711ec55f) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3cjoker )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "crjo54.p1", 0x6000, 0x2000, CRC(fee59a82) SHA1(816f68242a1382a26ff54a92e5d8df55ad6c8ece) )
	ROM_LOAD( "crjo54.p2", 0x4000, 0x2000, CRC(b0722d30) SHA1(4ab9785fbb9a04a85015cf8818567f0ec1b41808) )
	ROM_LOAD( "crjo54.p3", 0x2000, 0x2000, CRC(7af2985e) SHA1(b11fcfc83e93dc3737f9f71d3166c38a948a8aa4) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3xchngg )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "eg.p1", 0x7000, 0x1000, CRC(92edf95f) SHA1(473a8f1e845170953e55b55249a633bd9d9d03ad) )
	ROM_LOAD( "eg.p2", 0x6000, 0x1000, CRC(ba05c6ee) SHA1(6d5f1769ca81fc29f2c1f7257286b8cb287bc01c) )
	ROM_LOAD( "eg.p3", 0x5000, 0x1000, CRC(e28ca67d) SHA1(96a7c7dcf06bfad230b581d396c19419b98e2e03) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3xchngu )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "eu3.p1", 0x7000, 0x1000, CRC(db6d5c27) SHA1(c7c9b293bf7c37b499905f9cd00f2ba8d37e560e) )
	ROM_LOAD( "eu3.p2", 0x6000, 0x1000, CRC(717277b4) SHA1(6be70f72d4d57d785a85d1b8c9585bf65cca3243) )
	ROM_LOAD( "eu3.p3", 0x5000, 0x1000, CRC(0eafec89) SHA1(89b1766d35f69e28e662d87d3df1fee36956aa3b) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3xchngua ) // check if this actually differs
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "exchanges.hex", 0x5000, 0x3000, CRC(758323b9) SHA1(524fcb81148ec940ef98568d99fc7a0bda7d727a) ) // == m3sexcu
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END




ROM_START( m3fortun )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "fn1-5.p1", 0x6000, 0x2000, CRC(110c8c2b) SHA1(daaa4a2c71459b277571d751d44961aed38f1c84) )
	ROM_LOAD( "fn1-5.p2", 0x4000, 0x2000, CRC(2fb803de) SHA1(68f8dafc41931aa8882730acb95d0c95be7aa639) )
	ROM_LOAD( "fn1-5.p3", 0x2000, 0x2000, CRC(6fbfcb75) SHA1(5c28b2d03b7644715feed9fc31f5de5ad434c824) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3fortuna )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "fortune numbers v1-0 p1,2764", 0x6000, 0x2000, CRC(e864c266) SHA1(73c9ae327be0c8fd862a2533be1a60c6dd9d44f1) )
	ROM_LOAD( "fortune numbers v1-0 p2,2764", 0x4000, 0x2000, CRC(34f5ea73) SHA1(2009e87ce80da637c83ed4ca66661e1b95e47b50) )
	ROM_LOAD( "fortune numbers v1-0 p3,2764", 0x2000, 0x2000, CRC(4779cc92) SHA1(d191263fb11f2521cbbc0012f88294914ed9d17b) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3fortund )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "dfn-p1.bin", 0x6000, 0x2000, CRC(16ba6cca) SHA1(64ff2c8c60d3e44fa2692f63ec3593e7b1d4eae8) )
	ROM_LOAD( "dfn-p2.bin", 0x4000, 0x2000, CRC(c67461b9) SHA1(a63554b7af1bb3748acf43608e8958757a42c7b1) )
	ROM_LOAD( "dfn-p3.bin", 0x2000, 0x2000, CRC(f6e880c5) SHA1(13a3668335fb4a916b52fc132f91778edc4b9d01) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3gmine )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "gm1.bin", 0x6000, 0x2000, CRC(94a25adc) SHA1(6e4a3b50229202e1ab6142958d2e00784d6199a8) )
	ROM_LOAD( "gm2.bin", 0x4000, 0x2000, CRC(a71c5eb7) SHA1(e08e696a3f2cb0435388848cb2efe608f9d8677a) )
	ROM_LOAD( "gm3.bin", 0x2000, 0x2000, CRC(53b6a756) SHA1(e035142ebda9d32e1c430364e0ef7e6acabd6f3b) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3gaward )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ga04_p1.bin", 0x6000, 0x2000, CRC(730779da) SHA1(a35f80a8b46551b5fd4de2a34fdea4134a989b5d) )
	ROM_LOAD( "ga04_p2.bin", 0x4000, 0x2000, CRC(4503e0a7) SHA1(296a909ec5c86ee155b127b20ef9c13cb7824f83) )
	ROM_LOAD( "ga04_p3.bin", 0x2000, 0x2000, CRC(d9419ee7) SHA1(cdab52cd1aaa5f2791827ac6ea218bdbeed2648a) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3gcrown )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "crown.2l", 0x7000, 0x1000, CRC(184780c5) SHA1(e07f56e536823c28d524a687d607cb2877199210) )
	ROM_LOAD( "crown.2l2", 0x6000, 0x1000, CRC(04fadad3) SHA1(d82536f18122b9fb33fae6a238f29e4c615b3681) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3hprvpr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "hv.p1",  0x6000, 0x2000,  CRC(05c0cf97) SHA1(085f432b608cec6054f1e03cf24ca80d3949a0de))
	ROM_LOAD( "hv.p2",  0x4000, 0x2000,  CRC(b3b6f19e) SHA1(b2251dbe632b04ea7b5952b8a6217605b0df904c))
	ROM_LOAD( "hv.p3",  0x2000, 0x2000,  CRC(63f8e04f) SHA1(9bfe06a6ea8c308e4a7035752722babbfd792160))
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3replay )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ir6sl.p1", 0x7000, 0x1000, CRC(b6346bce) SHA1(56ec83411a21d161f22111abf6977daafcd05e98) )
	ROM_LOAD( "ir6sl.p2", 0x6000, 0x1000, CRC(16ce7113) SHA1(e919cfa46c1083fb6073cf34e8ba067035dc81a2) )
	ROM_LOAD( "ir6sl.p3", 0x5000, 0x1000, CRC(43951960) SHA1(58560119722c3583bd4094a47137e4d1d41b6c95) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3lineup )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "lineup.hex", 0x4000, 0x4000, CRC(d0c043af) SHA1(39d0fee039c7bd4d257a5537a04f8b35d7ec94df) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END



ROM_START( m3loony )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "loonybin.p1", 0x6000, 0x2000, CRC(33a2877e) SHA1(928ae3968e495b00b3384ba79f229aca25255996) )
	ROM_LOAD( "loonybin.p2", 0x4000, 0x2000, CRC(d17dfcec) SHA1(9ffcf392fbf44d751b188d9cefcfd7e98985a4dc) )
	ROM_LOAD( "loonybin.p3", 0x2000, 0x2000, CRC(0f3abac7) SHA1(1b6ae40a815ad8841f634816855ac2c0c45976f4) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3llotto )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "llp1.bin", 0x6000, 0x2000, CRC(3ab4045d) SHA1(4df1040c5699716ce3e1b9b3850451da8272d9ba) )
	ROM_LOAD( "llp2.bin", 0x4000, 0x2000, CRC(3aa789df) SHA1(a2a95339ae0a670e36eee356282adddb26e9a3e7) )
	ROM_LOAD( "llp3.bin", 0x2000, 0x2000, CRC(0c146c03) SHA1(a5e0c3a5b1d324ef702d5cdd583416dda5ea55eb) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3lstrik )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "lucky1.bin", 0x6000, 0x2000, CRC(d543f94c) SHA1(9186914cb3a014c573da1a08985ef4069f6550e4) )
	ROM_LOAD( "lucky2.bin", 0x4000, 0x2000, CRC(5708c22b) SHA1(721491de86517ab13eebd77fa0f3cf25410a244a) )
	ROM_LOAD( "lucky3.bin", 0x2000, 0x2000, CRC(d3b139ff) SHA1(119001d53f40650c090ebcad0d7263745e33f2ce) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3lstrika )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ls1_3_0.p1", 0x6000, 0x2000, CRC(3369ac39) SHA1(f02a7b14c9e2145813036c7cc2303ae003b618db) )
	ROM_LOAD( "lucky2.bin", 0x4000, 0x2000, CRC(5708c22b) SHA1(721491de86517ab13eebd77fa0f3cf25410a244a) )
	ROM_LOAD( "lucky3.bin", 0x2000, 0x2000, CRC(d3b139ff) SHA1(119001d53f40650c090ebcad0d7263745e33f2ce) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3magrp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "mr1.bin", 0x6000, 0x2000, CRC(b6109d11) SHA1(de11cfa34fc3ece4e9de03c1c81e9d082abd2a8c) )
	ROM_LOAD( "mr2.bin", 0x4000, 0x2000, CRC(6beedb00) SHA1(37e3addd20068778da8db9aa1c9723b519bfe89e) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3minmax )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "minimax.p1", 0x7000, 0x1000, CRC(941ab5a7) SHA1(c3031e3bd3a71b3a0cabf04059f1c540a8fa745f) )
	ROM_LOAD( "minimax.p2", 0x6000, 0x1000, CRC(140a4841) SHA1(822a54a6764226ec81f0c4625af0b2a4a3384db4) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3nnice )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "nn2-1.p1", 0x6000, 0x2000, CRC(584a2420) SHA1(3a2a759d1ab3ce216cde440a81571edf24b95822) )
	ROM_LOAD( "nn2-1.p2", 0x4000, 0x2000, CRC(78163deb) SHA1(72779113462a75ca3dacfe2a629f54a5eb5e6cc0) )
	ROM_LOAD( "nn2-1.p3", 0x2000, 0x2000, CRC(bed829ee) SHA1(d46ab52f4f250f4ec54bb5bf55dca6e275a20e13) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3oxo )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "oxo1.bin", 0x7000, 0x1000, CRC(ec0c0145) SHA1(817917897360036d26b52d68b9e5594f09b34747) )
	ROM_LOAD( "oxo2.bin", 0x6000, 0x1000, CRC(0007923a) SHA1(08207fff9e245033aacceb4f03da866d1703fa80) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END



ROM_START( m3oddson )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "oo3.1x.p1", 0x6000, 0x2000, CRC(bc5f4d43) SHA1(cacdc2d75f908fd85200b6cee014291e8417ce5c) )
	ROM_LOAD( "oo3.1x.p2", 0x4000, 0x2000, CRC(87835dc2) SHA1(b9a91a27f930eb3a14e63d566ebae0c5723a150a) )
	ROM_LOAD( "oo3.1x.p3", 0x2000, 0x2000, CRC(f621f387) SHA1(5261d501fd51addb510cf1beec2032def08f5873) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3online )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "online2p.bin", 0x6000, 0x2000, CRC(9d5dc1e3) SHA1(74b7af2c25a1a0eb4829328508a4a5d3599ed25f) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END



ROM_START( m3optunl )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ou6l.p1", 0x6000, 0x2000, CRC(5f65f88f) SHA1(e6353707cf936a80a191d8d068dae9117f2aae45) )
	ROM_LOAD( "ou6l.p2", 0x4000, 0x2000, CRC(d8cb1627) SHA1(3c533ee626108c9e962d77ab9b6d651a9a24bdc7) )
	ROM_LOAD( "ou6l.p3", 0x2000, 0x2000, CRC(9f097213) SHA1(a95763e35590e51d8e07f55aaab6a6f4afa30f9a) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3ratrce )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "rr1", 0x6000, 0x2000, CRC(015263ce) SHA1(5a2253e245acf81b2a6cd288b226e359d77586f1) )
	ROM_LOAD( "rr2", 0x4000, 0x2000, CRC(04e69380) SHA1(44561be88548dd46fa3117ca3ab9a1348745b534) )
	ROM_LOAD( "rr3", 0x2000, 0x2000, CRC(6f3210c8) SHA1(eb522bf06ac46181e23580bcbfeae5d42ff027ad) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END





ROM_START( m3razdaz )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "rd0-3x.p1", 0x6000, 0x2000, CRC(921a43a9) SHA1(b73446aa28842cc72d30664457743181e0e8a585) )
	ROM_LOAD( "rd0-3x.p2", 0x4000, 0x2000, CRC(adf4c26c) SHA1(a1204c2a960691f00dd10cb0073948453b230feb) )
	ROM_LOAD( "rd0-3x.p3", 0x2000, 0x2000, CRC(7442e22d) SHA1(b94ced271be0107c56a904e2beedb73af8f4b696) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3razdaza )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "rd07x1.bin", 0x6000, 0x2000, CRC(acdd1fc0) SHA1(0011c8af8e8ff5603f41879d897f6ac5fa533e8f) )
	ROM_LOAD( "rd07x2.bin", 0x4000, 0x2000, CRC(e7c1be48) SHA1(400f22515a25d208c9345570017cfbf557961def) )
	ROM_LOAD( "rd07x3.bin", 0x2000, 0x2000, CRC(f2ffe770) SHA1(ae6a4ce8e2fb1492cdd51d324cd697ed50e22947) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3razdazd )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "razzle-p1.bin", 0x6000, 0x2000, CRC(4f1aa030) SHA1(3a09af6a3525300df3ee55d0e129ebd42dfd17c0) )
	ROM_LOAD( "razzle-p2.bin", 0x4000, 0x2000, CRC(c1d3ac78) SHA1(dcac9db7be72f0fefd11105c645f2c7a122afe7b) )
	ROM_LOAD( "razzle-p3.bin", 0x2000, 0x2000, CRC(98867482) SHA1(8f4a2387ad06fb1e659f8b6a20cb8a1928a28c42) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3rockpl )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "rpl.p1", 0x6000, 0x2000, CRC(8507e316) SHA1(3d7aae76c0ab4d09c286ce0474ad8ae51c204e0b) )
	ROM_LOAD( "rpl.p2", 0x4000, 0x2000, CRC(c11a123b) SHA1(52ffa34cd3d196cc7a13fc7d15f144aa198dc1cd) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3rolla ) // R-5LS V.30
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ic1", 0x6000, 0x2000, CRC(16e320ae) SHA1(3489e81f52db025ca2239c4072fa01f77f4d8d31) )
	// ic2,ic3 and ic4 empty, ic5 is a 74ls138n
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3rollem )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "roll-em.p1", 0x6000, 0x2000, CRC(2ec2cfa9) SHA1(461240315725f29f53fdcf89e767d17a4d2301eb) )
	ROM_LOAD( "roll-em.p2", 0x4000, 0x2000, CRC(fff42143) SHA1(ca0efe2ad5e8d353c6352a4f8fbabe4d2d375fc0) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3rxchng )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "re41tp1.bin", 0x6000, 0x2000, CRC(ba6e84f8) SHA1(fcf6310c5ce285104a0fde00656e3ab1d599fac8) )
	ROM_LOAD( "re41tp2.bin", 0x4000, 0x2000, CRC(7c7f67b0) SHA1(5448d755bf6536312898abb7b42141559772fe04) )
	ROM_LOAD( "re41tp3.bin", 0x2000, 0x2000, CRC(78b51719) SHA1(78f972ce57cbb0df1d590d6f4302d5405ff1834c) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3snaphp ) // runs but LED display is wrong?
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sn.bin", 0x6000, 0x2000, CRC(68a9d720) SHA1(5294309082ff89ee33339bee4b350ae672dd34c0) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3snappy ) // clone of hyper viper
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  ) // this should be split..
	ROM_LOAD( "snappy.hex", 0x2000, 0x6000, CRC(6bf2052b) SHA1(3cb1dc19aa58cc60193745a04d8fdebcfc1dc200) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3circle )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "spc.p1", 0x7000, 0x1000, CRC(17bfae90) SHA1(e04ab893dfaaf2bf178a6d51daa99aa9b412dfff) )
	ROM_LOAD( "spc.p2", 0x6000, 0x1000, CRC(27fac879) SHA1(bb738cb4f541ed51488ab5e4e50288052cd911c7) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3circlea ) // most likely a bad dump of both roms
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "specialcircle.p1", 0x7000, 0x1000, BAD_DUMP CRC(21f7e745) SHA1(5b050cad957a055396f5aaa5558034cf55d62d18) ) // bad? boot vectors are missing..
	ROM_LOAD( "specialcircle.p2", 0x6000, 0x1000, BAD_DUMP CRC(8d5d26bd) SHA1(e88e368ad411529e982c76bff0696f6966fd7f11) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3circleb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "splc4.p1", 0x7000, 0x1000, CRC(252eb228) SHA1(e71b7d04bd0aaf5b047f25c755d0ce0593dd2ade) )
	ROM_LOAD( "splc4.p2", 0x6000, 0x1000, CRC(86fb0c95) SHA1(d2dc55241f3dddfd7ece8ed041d81abe0da5ecc8) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3scoop )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "scoopv-2.p1", 0x7000, 0x1000, CRC(e937e298) SHA1(edb4dfb5afd6c2640b6bbd83be591987225bd8fc) )
	ROM_LOAD( "scoopv-2.p2", 0x6000, 0x1000, CRC(2a97a254) SHA1(a249e013d86f7e65e43b07ff916c4d0fd5099f44) )
	ROM_LOAD( "scoopv-2.p3", 0x5000, 0x1000, CRC(34ab1805) SHA1(1e389e9b47c4b3305ec70c94f49a4e3ca0a6f439) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3spoof )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "spoof1.bin", 0x7000, 0x1000, CRC(8111eec5) SHA1(9fbfeafa20a79fcb8fb71fcd32b515d04bc6bc3a) )
	ROM_LOAD( "spoof2.bin", 0x6000, 0x1000, CRC(47309111) SHA1(af7ee94d9405bcadf9432da4725361c6600d1788) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3spoofa )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "spoof10pp1286.bin", 0x7000, 0x1000, BAD_DUMP CRC(2805d082) SHA1(b6b31ec6705b175a8c641089924b100c8427eac0) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "spoof10pp2286.bin", 0x6000, 0x1000, CRC(3c104fbe) SHA1(c7825288ae06cd6adabe02532c39944d5f4d3e44) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3slight )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sal2-9.p1", 0x6000, 0x2000, CRC(1acccff5) SHA1(5c9cb9fe88ca6e11fcb4f22d93fefe67b5b165ff) )
	ROM_LOAD( "sal2-9.p2", 0x4000, 0x2000, CRC(28a02a70) SHA1(d9f68031b02c6ab5aee717c04c1834a1c9b250e9) )
	ROM_LOAD( "sal2-9.p3", 0x2000, 0x2000, CRC(4dff2186) SHA1(bd15bbe248ed3270278c9e848d600ed49c06c72d) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END




ROM_START( m3supasw )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sswop_p1.bin", 0x6000, 0x2000, CRC(8bf34554) SHA1(b981dc67905f7ceb02271ca67dd224dc1b4d5a40) )
	ROM_LOAD( "sswop_p2.bin", 0x4000, 0x2000, CRC(8b68fd45) SHA1(8cfbf2b7a427a0360640af142dbce5eb834bc648) )
	ROM_LOAD( "sswop_p3.bin", 0x2000, 0x2000, CRC(3d821ed4) SHA1(77fa7c67593ea175ad29f0959524c0e225391bbc) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3supadr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa1_0.p1", 0x6000, 0x2000, CRC(b6088056) SHA1(d887e92a76db848045cfe4fec9b1f16ed4d733d9) )
	ROM_LOAD( "sa1_0.p2", 0x5000, 0x1000, CRC(7efb91e0) SHA1(faa9292ba3671e0b6458f010a1005037d01ff501) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3sdeal ) // dutch? (based on error message)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "superdeal-p1.bin", 0x6000, 0x2000, CRC(1ca6bc04) SHA1(477a2a9267cefa1cdd8b173886aedaacf8f83a26) )
	ROM_LOAD( "superdeal-p2.bin", 0x4000, 0x2000, CRC(49da28eb) SHA1(4192707eb5abb3d3a0679a2c243810c7e61f873d) )
	ROM_LOAD( "superdeal-p3.bin", 0x2000, 0x2000, CRC(94e8341a) SHA1(5908995ae4cbf5cd45704b3e4ed7129859c85c0b) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3sexcu )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  ) // needs to be split
	ROM_LOAD( "superexchanges.hex", 0x5000, 0x003000, CRC(758323b9) SHA1(524fcb81148ec940ef98568d99fc7a0bda7d727a) ) // == m3xchngua
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3suplin )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "slu1_1.p1", 0x6000, 0x2000, CRC(5b939310) SHA1(4a668cebef092e1f61172850368489818cd50009) )
	ROM_LOAD( "slu1_1.p2", 0x4000, 0x2000, CRC(7ae23b70) SHA1(f516f34884f5967ea9145b227561641391d201f1) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3suplina )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "superlineup2v1-0.p1", 0x6000, 0x2000, CRC(9aa1ea5a) SHA1(4649a28aabb71187e8378d123caf60b8400fc4ad) )
	ROM_LOAD( "superlineup2v1-0.p2", 0x4000, 0x2000, CRC(3d341b1b) SHA1(9ca413f8c5eaa10f59ba16e09c12934181d795b4) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END



ROM_START( m3supnud )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "snu.p1", 0x7000, 0x1000, CRC(8255e3a4) SHA1(bc7dc2d5e33f88cb96ca311b323d4442f0ec4e1b) )
	ROM_LOAD( "snu.p2", 0x6000, 0x1000, CRC(9a5cf1ef) SHA1(b18049cae70c514b68bad526ef7d27f90f4aa439) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3supser )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "spseries-9f.p1", 0x7000, 0x1000, CRC(baf8f44b) SHA1(3f8fcca4a4125f24fd44dfa995876a6743f2624f) )
	ROM_LOAD( "spseries-9f.p2", 0x6000, 0x1000, CRC(c1963f6b) SHA1(8d720a95dd041de22000db2504f97e907480cb80) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3supspo ) // boots
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "s-spoof10p_4.p1", 0x6000, 0x2000, CRC(9ec3896f) SHA1(e9b79a0d3bfd6fb1ae7a7ef21112b095f4386e95) )
	ROM_LOAD( "s-spoof10p_4.p2", 0x4000, 0x2000, CRC(5bc9036b) SHA1(6209cebeaf9a4841d8bffd7777df6df3f76457c1) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3supspoa )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "superspoof1.bin", 0x6000, 0x2000, CRC(1a9ce342) SHA1(6297528891e9629e489e0ea0c01465dc650d9b4a) )
	ROM_LOAD( "superspoof2.bin", 0x4000, 0x2000, CRC(d5108b58) SHA1(10e81c9a2802b820801d3bdee26a360b1d43ebd7) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END






ROM_START( m3sweep )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ss56p1.bin", 0x6000, 0x2000, CRC(c1fb3eb0) SHA1(95b9c6fe641ef24135a384c2deb58a249d6db632) )
	ROM_LOAD( "ss56p2.bin", 0x4000, 0x2000, CRC(29347490) SHA1(ac033708e396810043b73789daca6af993c095ea) )
	ROM_LOAD( "ss56p3.bin", 0x2000, 0x2000, CRC(87bea2a5) SHA1(b654eb19a9285746788f9ef4f5e305ec275abbe1) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3sweepa )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ss56tp1.bin", 0x6000, 0x2000, CRC(815b77f9) SHA1(ba827d9ee73a50b8f0c42e18af899450cb8538b8) )
	ROM_LOAD( "ss56tp2.bin", 0x4000, 0x2000, CRC(29347490) SHA1(ac033708e396810043b73789daca6af993c095ea) )
	ROM_LOAD( "ss56tp3.bin", 0x2000, 0x2000, CRC(87bea2a5) SHA1(b654eb19a9285746788f9ef4f5e305ec275abbe1) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3tlktwn )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ttown1.bin", 0x7000, 0x1000, CRC(996c435b) SHA1(1fcf5a637cddacd6660da752a1fe10e56a7653c7) )
	ROM_LOAD( "ttown2.bin", 0x6000, 0x1000, CRC(93bd6446) SHA1(ef271bb45d27844f0ef1437bbc7f4847f4c0c6ee) )
	ROM_LOAD( "ttown3.bin", 0x5000, 0x1000, CRC(0ca34690) SHA1(1734692d82af89272a2243e2c5f584965bfe197f) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3toplin )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "tlines.hex", 0x6000, 0x2000, CRC(b8d652b4) SHA1(fe356ba773f876a390f845daa6464a41e8cd2826) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3topsht )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "topshtp1.bin", 0x6000, 0x2000, CRC(8ee171c9) SHA1(34db6556c9af75c1cb734fe610814fcd3c4bba93) )
	ROM_LOAD( "topshtp2.bin", 0x4000, 0x2000, CRC(708cb3b5) SHA1(1e33ce713d0cc75bac675d739c7332109ce1da2d) )
	ROM_LOAD( "topshtp3.bin", 0x2000, 0x2000, CRC(ead364ae) SHA1(207e082fb18049863cbc35975bc6885b39ef76da) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END



ROM_START( m3tfair )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "tf1.bin", 0x6000, 0x2000, CRC(0f0e0ef6) SHA1(768e162463ecc3fb6ca4c8bf01504d25ce427cd9) )
	ROM_LOAD( "tf2.bin", 0x4000, 0x2000, CRC(8e25c8a5) SHA1(e924677b1820210157706f093f12bdf5acc5211e) )
	ROM_LOAD( "tf3.bin", 0x2000, 0x2000, CRC(2ab74abc) SHA1(cf10fa2d4dfceb6358005525fa71bc58a8875128) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3wacky ) // draws letters with the lights!
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "wacky1.bin", 0x6000, 0x2000, CRC(8fad0357) SHA1(0bcf21bb2edf7cce32fa6f028dcceb8ea8a1bec7) )
	ROM_LOAD( "wacky2.bin", 0x5000, 0x1000, CRC(534d31c3) SHA1(51d5e174f19bafde9d6d20c0bd0671b3f2857314) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3wigwam )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "wigwamp1.bin", 0x6000, 0x2000, CRC(2ec2cfa9) SHA1(461240315725f29f53fdcf89e767d17a4d2301eb) )
	ROM_LOAD( "wigwamp2.bin", 0x4000, 0x2000, CRC(f386dd1a) SHA1(0506c3b79e7394064b1ee97758cd1deb5933c290) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3supwin )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "superwin.p1", 0x6000, 0x2000, CRC(d6193f5b) SHA1(2813a982d1527d994a045840aaaa20032bd39c45) )
	ROM_LOAD( "superwin.p2", 0x4000, 0x2000, CRC(89f9af8a) SHA1(1705a0eb31a5ee77e77811798d3c99b65dc7ade4) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3supwina )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "wag.p1", 0x6000, 0x2000, CRC(d6193f5b) SHA1(2813a982d1527d994a045840aaaa20032bd39c45) )
	ROM_LOAD( "wag.p2", 0x4000, 0x2000, CRC(f4210f20) SHA1(e0c7e28950683e35102bfd0bef7b74c63a36798f) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3winagn )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "2pwag.p1", 0x6000, 0x2000, CRC(597df8e6) SHA1(c28bafa9f7552b1f23218844592f0b12b9215891) )
	ROM_LOAD( "2pwag.p2", 0x4000, 0x2000, CRC(a48877f9) SHA1(a52cebb9d2f80b2ee4a821628138c406e1b68fe1) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END
	// these appear to be something else / different hardware
	//ROM_LOAD( "bwbnwagn.bin", 0x0000, 0x8000, CRC(1163559a) SHA1(9e13b23eae478fd5b5468599b1163d06e189d446) )
	//ROM_LOAD( "winagain.bin", 0x0000, 0x4000, CRC(193e743d) SHA1(1c2d0dc8bea80c29c012a6a43d5bcd342b2b9f2a) )

ROM_START( m3winagnb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "wag1.bin", 0x6000, 0x2000, CRC(ded10570) SHA1(abb938e02cac34ec800c53d1c19a66ef8456d483) )
	ROM_LOAD( "wag2.bin", 0x4000, 0x2000, CRC(5c6643b5) SHA1(dbaeb22d6879e005c296456f5424eb6147564e9f) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END


ROM_START( m3winstr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "winst1.bin", 0x6000, 0x2000, CRC(d06f8312) SHA1(b32b6eff6928b26422d66f90d6643d03bc021181) )
	ROM_LOAD( "winst2.bin", 0x4000, 0x2000, CRC(8bc5df32) SHA1(6a16223f7f93c87dac29758206abe45cc3f45d78) )
	ROM_LOAD( "winst3.bin", 0x2000, 0x2000, CRC(d8ce68e8) SHA1(65518db36a195ac5303033086da8ca8e4a61ca13) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

ROM_START( m3winstra )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "2p wst p1.bin", 0x6000, 0x2000, CRC(34577170) SHA1(6dbe71074b069e44da5231b40a6a9f89a9315f34) )
	ROM_LOAD( "2p wst p2.bin", 0x4000, 0x2000, CRC(8bc5df32) SHA1(6a16223f7f93c87dac29758206abe45cc3f45d78) )
	ROM_LOAD( "p3.bin",        0x2000, 0x2000, CRC(80bba633) SHA1(2c8fd48c20e6fd9d6ea7cae7466d18dae82fbec3) )
	ROM_COPY( "maincpu", 0x0000, 0x8000, 0x8000 )
ROM_END

} // anonymous namespace


/* Barcrest */

// search for cmpa (x+$01) in trace logs to find protection accesses

#define GAME_FLAGS MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL




GAME(  198?, m3tst,      0,          mpu3base, mpu3, mpu3_state, empty_init,    ROT0, "Barcrest","MPU3 Unit Test (Program 5) (Barcrest) (MPU3)",GAME_FLAGS )


// no protection, boots without testing reels?
GAMEL( 198?, m3blkhle,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Black Hole (Barcrest) (MPU3)", GAME_FLAGS, layout_m3blkhle )

// no protection, boots without testing reels?
GAMEL( 198?, m3xchngg,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Exchanges Galore (Barcrest) (MPU3)", GAME_FLAGS, layout_m3xchngg )

// no protection, boots without testing reels?
GAME(  198?, m3xchngu,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Exchanges Unlimited (Barcrest) (MPU3, set 1)",GAME_FLAGS )
GAME(  198?, m3xchngua,  m3xchngu,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Exchanges Unlimited (Barcrest) (MPU3, set 2)",GAME_FLAGS )
GAMEL( 198?, m3sexcu,    m3xchngu,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Super Exchanges Unlimited (Barcrest) (MPU3)", GAME_FLAGS, layout_m3sexcu ) // == m3xchngua, what is the correct name?

// no protection, boots without testing reels?
GAMEL( 198?, m3replay,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Instant Replay (Barcrest) (MPU3)", GAME_FLAGS, layout_m3replay )


// no protection
GAMEL( 198?, m3cabret,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Cabaret (Barcrest) (MPU3, set 1)", GAME_FLAGS, layout_m3cabreta )
GAMEL( 198?, m3cabreta,  m3cabret,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Cabaret (Barcrest) (MPU3, set 2)", GAME_FLAGS, layout_m3cabreta )

// no protection
GAMEL( 198?, m3lineup,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Line Up (Barcrest) (MPU3)", GAME_FLAGS, layout_m3lineup )

// no protection
GAME(  198?, m3magrp,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Magic Replay (Barcrest) (MPU3)",GAME_FLAGS )

// no protection
GAMEL( 198?, m3supadr,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Super Adders & Ladders (Barcrest) (MPU3)", GAME_FLAGS, layout_m3supadr )

// no protection
GAME(  198?, m3suplin,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Super Line Up (Barcrest) (MPU3, set 1)",GAME_FLAGS )
GAME(  198?, m3suplina,  m3suplin,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Super Line Up (Barcrest) (MPU3, set 2)",GAME_FLAGS )


// protection write sequence and address is different, maybe hacked?
GAME(  198?, m3big20j,   0,          mpu3_chr_3000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Big 20 Joker (Barcrest) (MPU3)",GAME_FLAGS )


// 00 a0 ac 1c 94 2c 10 64 e4 ec (shared sequence m3autort, m3nnice)
GAMEL( 198?, m3autort,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Autoroute (Barcrest) (MPU3)", GAME_FLAGS, layout_m3autort )

// 00 a0 a8 3c 94 2c 10 60 e4 e8
GAMEL( 198?, m3biggam,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","The Big Game (Barcrest) (MPU3)", GAME_FLAGS, layout_m3biggam )

// 00 a0 88 18 b0 48 50 60 e4 c8 (shared sequence m3bigsht, m3topsht, also topgear_characteriser_prot on mpu4)
GAMEL( 198?, m3bigsht,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Big Shot (Barcrest) (MPU3)", GAME_FLAGS, layout_m3bigsht )

// 00 a0 88 58 d4 2c 30 20 e4 c8 (shared sequence m3cunlim, m3optunl, m3lstrik?)
GAMEL( 198?, m3cunlim,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Chances Unlimited (Barcrest) (MPU3)", GAME_FLAGS, layout_m3cunlim )

// 00 e0 88 18 b0 48 50 60 e4 c8 (also bdash_characteriser_prot on mpu4?)
GAMEL( 198?, m3cskill,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Circle Skill (Barcrest) (MPU3)", GAME_FLAGS, layout_m3cskill )

// 00 60 68 bc d0 0c 94 20 e4 e8
GAME(  198?, m3cjoker,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Crazy Joker (Barcrest) (MPU3)",GAME_FLAGS )

// 00 a0 a8 38 b4 28 30 64 e4 e8
GAMEL( 198?, m3fortun,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Fortune Numbers (Barcrest) (MPU3, set 1)", GAME_FLAGS, layout_m3fortun )
GAMEL( 198?, m3fortuna,  m3fortun,   mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Fortune Numbers (Barcrest) (MPU3, set 2)", GAME_FLAGS, layout_m3fortun )
// 00 60 68 bc d0 2c 94 20 e4 e8 (shared sequence as m3fortund, m3sdeal, also m400_characteriser_prot on mpu4)
GAMEL( 198?, m3fortund,  m3fortun,   mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Fortune Numbers (Barcrest) (Dutch) (MPU3)", GAME_FLAGS, layout_m3fortun )

// 00 e0 88 78 94 48 50 60 e4 c8
GAME(  198?, m3gaward,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Golden Award (Barcrest) (MPU3)",GAME_FLAGS )

// 00 80 ec 1c f0 28 54 04 a4 ac
GAME(  198?, m3hprvpr,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Hyper Viper (Barcrest) (MPU3)",GAME_FLAGS )
GAME(  198?, m3snappy,   m3hprvpr,   mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Snappy Viper (Barcrest) (MPU3)",GAME_FLAGS )

// 00 e0 a8 3c 90 68 34 40 e4 e8
GAME(  198?, m3llotto,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Lucky Lotto (Barcrest) (MPU3)",GAME_FLAGS )

// 00 a0 88 58 d4 2c 30 20 e4 c8 (shared sequence m3cunlim, m3optunl, m3lstrik?)
GAME(  198?, m3lstrik,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Lucky Strike Club (Barcrest) (MPU3, set 1)",GAME_FLAGS )
GAME(  198?, m3lstrika,  m3lstrik,   mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Lucky Strike Club (Barcrest) (MPU3, set 2)",GAME_FLAGS )

// 00 a0 ac 1c 94 2c 10 64 e4 ec (shared sequence m3autort, m3nnice)
GAMEL( 198?, m3nnice,    0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Naughty But Nice (Barcrest) (MPU3)", GAME_FLAGS, layout_m3nnice )

// 00 e0 a8 38 90 68 30 60 e4 e8 (also hittop_characteriser_prot on mpu4)
GAMEL( 198?, m3oddson,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Odds On (Barcrest) (MPU3)", GAME_FLAGS, layout_m3oddson )

// 00 a0 88 58 d4 2c 30 20 e4 c8 (shared sequence m3cunlim, m3optunl, m3lstrik?)
GAME(  198?, m3optunl,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Options Unlimited (Barcrest) (MPU3)",GAME_FLAGS )

// 00 e0 88 18 b0 48 10 60 e4 c8
GAMEL( 198?, m3razdaz,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Razzle Dazzle (Barcrest) (MPU3, set 1)", GAME_FLAGS, layout_m3razdaza )
GAMEL( 198?, m3razdaza,  m3razdaz,   mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Razzle Dazzle (Barcrest) (MPU3, set 2)", GAME_FLAGS, layout_m3razdaza )
// 00 60 68 98 d4 2c b0 24 e4 e8
GAMEL( 198?, m3razdazd,  m3razdaz,   mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Razzle Dazzle (Barcrest) (Dutch) (MPU3)", GAME_FLAGS, layout_m3razdaza )

// 00 a0 a8 38 f4 2c 30 60 e4 e8
GAMEL( 198?, m3rxchng,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Royal Exchange Club (Barcrest) (MPU3)", GAME_FLAGS, layout_m3rxchng )

// 00 e0 a8 38 94 48 50 60 e4 e8 (also m407_characteriser_prot on mpu4)
GAMEL( 198?, m3slight,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Strike A Light (Barcrest) (MPU3)", GAME_FLAGS, layout_m3slight )

// 00 60 68 bc d0 2c 94 20 e4 e8 (shared sequence as m3fortund, m3sdeal, also m400_characteriser_prot on mpu4)
GAME(  198?, m3sdeal,    0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Super Deal (Barcrest) (Dutch) (MPU3)",GAME_FLAGS )

// 00 a0 a8 18 f4 2c 70 60 e4 e8 (also viva_characteriser_prot on mpu4?)
GAMEL( 198?, m3sweep,    0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Sweep Stake Club (Barcrest) (MPU3, set 1)", GAME_FLAGS, layout_m3sweep )
GAMEL( 198?, m3sweepa,   m3sweep,    mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Sweep Stake Club (Barcrest) (MPU3, set 2)", GAME_FLAGS, layout_m3sweep )

// 00 a0 88 18 b0 48 50 60 e4 c8 (shared sequence m3bigsht, m3topsht, also topgear_characteriser_prot on mpu4)
GAME(  198?, m3topsht,   0,          mpu3_chr_c000, mpu3, mpu3_chr_state, init_mpu3, ROT0, "Barcrest","Top Shot (Barcrest) (MPU3)",GAME_FLAGS )


// doesn't boot, does nothing?
GAMEL( 198?, m3supnud,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Super Nudges Unlimited (Barcrest) (MPU3)", GAME_FLAGS, layout_m3supnud )

// doesn't boot, does nothing?
GAME(  198?, m3supser,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Super Series (Barcrest) (MPU3)",GAME_FLAGS )

// doesn't boot, does nothing?
GAMEL( 198?, m3circle,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Special Circle Club (Barcrest) (MPU3, set 1)", GAME_FLAGS, layout_m3circle )
GAMEL( 198?, m3circlea,  m3circle,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Special Circle Club (Barcrest) (MPU3, set 2, bad)", GAME_FLAGS, layout_m3circle )
GAMEL( 198?, m3circleb,  m3circle,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Barcrest","Special Circle Club (Barcrest) (MPU3, set 3)", GAME_FLAGS, layout_m3circle )


/* BWB */

GAMEL( 198?, m3acech,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Ace Chase (BWB) (MPU3)", GAME_FLAGS, layout_m3acech )

GAME(  198?, m3bankr,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Banker (BWB) (MPU3)",GAME_FLAGS )

GAMEL( 198?, m3chase,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Chase It (BWB) (MPU3)", GAME_FLAGS, layout_m3chase )

GAMEL( 198?, m3gmine,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Gold Mine (BWB) (MPU3)", GAME_FLAGS, layout_m3gmine )

GAMEL( 198?, m3ratrce,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Rat Race (BWB) (MPU3)", GAME_FLAGS, layout_m3ratrce )

GAME(  198?, m3supasw,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Supaswop (BWB) (MPU3)",GAME_FLAGS )

GAMEL( 198?, m3supwin,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Super Win (BWB) (MPU3, set 1) (W.AG2) ", GAME_FLAGS, layout_m3supwin ) // offset VFD
GAMEL( 198?, m3supwina,  m3supwin,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Super Win (BWB) (MPU3, set 2) (S.W.2 1.0)", GAME_FLAGS, layout_m3supwin )

GAMEL( 198?, m3winagn,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Win-A-Gain (BWB) (MPU3, set 1) (W.A.G. 2.2)", GAME_FLAGS, layout_m3winagnb )
GAMEL( 198?, m3winagnb,  m3winagn,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Win-A-Gain (BWB) (MPU3, set 2) (W.A.G. 5.1)", GAME_FLAGS, layout_m3winagnb )

GAMEL( 198?, m3winstr,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Winstrike (BWB) (MPU3, set 1)", GAME_FLAGS, layout_m3winstr )
GAMEL( 198?, m3winstra,  m3winstr,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "BWB","Winstrike (BWB) (MPU3, set 2)", GAME_FLAGS, layout_m3winstr )

/* Pcp */

// boot, does reel test, and runs
GAME(  198?, m3cdash,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Cash Dash (Pcp) (MPU3)",GAME_FLAGS )
GAMEL( 198?, m3snaphp,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Snap Happy (Pcp) (MPU3)", GAME_FLAGS, layout_m3snaphp )
GAMEL( 198?, m3spoof,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Spoof (Pcp) (MPU3, set 1)", GAME_FLAGS, layout_m3spoof )
GAMEL( 198?, m3spoofa,   m3spoof,    mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Spoof (Pcp) (MPU3, set 2)", GAME_FLAGS, layout_m3spoof ) // bad dump
GAMEL( 198?, m3supspo,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Super Spoof (Pcp) (MPU3, set 1)", GAME_FLAGS, layout_m3supspo )
GAMEL( 198?, m3supspoa,  m3supspo,   mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Super Spoof (Pcp) (MPU3, set 2)", GAME_FLAGS, layout_m3supspo )

// boots, reel error a
GAMEL( 198?, m3loony,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Loonybin (Pcp) (MPU3)", GAME_FLAGS, layout_m3loony )

// does nothing
GAMEL( 198?, m3online,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","On Line (Pcp) (MPU3)", GAME_FLAGS, layout_m3online )
GAMEL( 198?, m3toplin,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Top Line (Pcp) (MPU3)", GAME_FLAGS, layout_m3toplin )

// offset VFD, don't boot (act like m3supwin above)
GAMEL( 198?, m3rockpl,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Rock Pile (Pcp) (MPU3)", GAME_FLAGS, layout_m3rockpl )
GAMEL( 198?, m3rollem,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Roll 'Em (Pcp) (MPU3)", GAME_FLAGS, layout_m3rollem )
GAME(  198?, m3wigwam,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Pcp","Wig Wam (Pcp) (MPU3)",GAME_FLAGS )


/* Mdm */

// no protection, boots without testing reels?
GAMEL( 198?, m3gcrown,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Mdm","Golden Crowns (Mdm) (MPU3)", GAME_FLAGS, layout_m3gcrown )

// reel A alarm
GAMEL( 198?, m3tfair,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Mdm","Tuppenny Fair (Mdm) (MPU3)", GAME_FLAGS, layout_m3tfair )
GAME(  198?, m3wacky,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Mdm","Wacky Racer (Mdm) (MPU3)",GAME_FLAGS )

/* VFS */

// no protection, boots without testing reels?
GAMEL( 198?, m3oxo,      0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "VFS","Noughts 'n' Crosses (VFS) (MPU3)", GAME_FLAGS, layout_m3oxo )

// shows 'initializing' then '.20'
GAME(  198?, m3mremon,   0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "VFS","More Money (VFS) (MPU3)",GAME_FLAGS )

/* Others */

GAME( 198?, m3minmax,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Associated Leisure","Mini Max (Associated Leisure) (MPU3)",GAME_FLAGS )
GAME( 198?, m3scoop,     0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Peter Simper","Scoop (Peter Simper, prototype?) (MPU3)",GAME_FLAGS )
GAME( 198?, m3tlktwn,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "<unknown>","Talk of the Town (MPU3?)",GAME_FLAGS )

// no protection
GAME(  198?, m3rolla,    0,          mpu3base, mpu3, mpu3_state, init_mpu3, ROT0, "Nationwide Group","Rollamatic 5-Line Special (MPU3)", GAME_FLAGS ) // or 5-Line Special Rollamatic on the glass, using the title from the ROM
