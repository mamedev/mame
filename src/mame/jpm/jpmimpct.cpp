// license:BSD-3-Clause
// copyright-holders:Philip Bennett, David Haywood
// thanks-to:Tony Friery
/***************************************************************************

    JPM IMPACT (aka System 6)
     and
    JPM IMPACT with Video hardware

    driver by Phil Bennett

    Games supported:
        * Coronation Street Quiz Game
        * Cluedo (3 sets)
        * Hangman
        * Scrabble
        * Trivial Pursuit

    ROMS wanted:
        * Snakes and Ladders

    Mechanical games note:

    Anything writing to 4800a0 within the first few instructions is guessed
    to be an IMPACT game, some things could be misplaced, some could be
    missing video roms, many are missing sound roms (or they're in the wrong
    sets)

****************************************************************************

    Memory map (preliminary)

****************************************************************************

    ========================================================================
    Main CPU (68000)
    ========================================================================
    000000-0FFFFF   R     xxxxxxxx xxxxxxxx   Program ROM bank 1
    100000-1FFFFF   R     xxxxxxxx xxxxxxxx   Program ROM bank 2
    400000-403FFF   R/W   xxxxxxxx xxxxxxxx   Program RAM (battery-backed)
    480000-48001F   R/W   -------- xxxxxxxx   MC68681 DUART 1
    480020-480033   R     -------- xxxxxxxx   Inputs
    480041          R     -xxxxxxx xxxxxxxx   Reel optos
    480060-480067   R/W   -------- xxxxxxxx   uPD71055C (NEC clone of 8255 PPI)
    480080-480081     W   -------- xxxxxxxx   uPD7559 communications
    480082-480083     W   -------- xxxxxxxx   Sound control
                          -------- -------x      (uPD7759 reset)
                          -------- -----xx-      (ROM A18-A17)
                          -------- ---x----      (X9C103 /INC)
                          -------- --x-----      (X9C103 U/#D)
                          -------- -x------      (X9C103 /CS)
    480084-480085   R     -------- xxxxxxxx   uPD7759 communications
    4800A0-4800AF     W   xxxxxxxx xxxxxxxx   Lamps?
    4800E0-4800E1     W   xxxxxxxx xxxxxxxx   Reset and status LEDs?
    4801DC-4801DD   R     -------- xxxxxxxx   Unknown
    4801DE-4801DF   R     -------- xxxxxxxx   Unknown
    4801E0-4801FF   R/W   -------- xxxxxxxx   MC68681 DUART 2 (on ROM PCB)
    800000-800007   R/W   xxxxxxxx xxxxxxxx   TMS34010 interface
    C00000-CFFFFF   R     xxxxxxxx xxxxxxxx   Question ROM bank 1
    D00000-DFFFFF   R     xxxxxxxx xxxxxxxx   Question ROM bank 2
    E00000-EFFFFF   R     xxxxxxxx xxxxxxxx   Question ROM bank 3
    F00000-FFFFFF   R     xxxxxxxx xxxxxxxx   Question ROM bank 4
    ========================================================================
    Interrupts:
        IRQ2 = TMS34010
        IRQ5 = MC68681 1
        IRQ6 = Watchdog?
        IRQ7 = Power failure detect
    ========================================================================

    ========================================================================
    Video CPU (TMS34010, all addresses are in bits)
    ========================================================================
    -----000 00xxxxxx xxxxxxxx xxxxxxxx   Video RAM
    -----000 1xxxxxxx xxxxxxxx xxxxxxxx   ROM
    -----010 0xxxxxxx xxxxxxxx xxxxxxxx   ROM
    -----001 0------- -------- --xxxxxx   Bt477 RAMDAC
    -----111 1-xxxxxx xxxxxxxx xxxxxxxx   RAM

****************************************************************************/

/**************************************************************************

IMPACT Games

IMPACT apparently stands for Interactive Moving Picture Amusement Control
Technology, and is intended as a replacement for the JPM System 5 board.
Large sections of the processing were moved to two identical custom ASICs
(U1 and U2), only half of each is used.

Thanks to Tony Friery and JPeMU for I/O routines and documentation.

***************************************************************************/


#include "emu.h"
#include "jpmimpct.h"

#include "awpvid.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"

#include "screen.h"
#include "speaker.h"

#include "jpmimpct.lh"
#include "cluedo.lh"


DEFINE_DEVICE_TYPE(JPM_TOUCHSCREEN, jpmtouch_device, "jpmtouch", "JPM Touchscreen")

jpmtouch_device::jpmtouch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jpmtouch_device(mconfig, JPM_TOUCHSCREEN, tag, owner, clock)
{
}

jpmtouch_device::jpmtouch_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, JPM_TOUCHSCREEN, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_rxd_handler(*this),
	m_sending(-1)
{
}

void jpmtouch_device::device_start()
{
	save_item(NAME(m_touch_data));
	save_item(NAME(m_sendpos));
	save_item(NAME(m_sending));
}

void jpmtouch_device::device_reset()
{
	int startbits = 1;
	int databits = 8;
	parity_t parity = device_serial_interface::PARITY_NONE;
	stop_bits_t stopbits =  device_serial_interface::STOP_BITS_1;

	set_data_frame(startbits, databits, parity, stopbits);

	set_tra_rate(9600);
	set_rcv_rate(9600);

	output_rxd(1);
}

void jpmtouch_device::tx_queue()
{
	if (is_transmit_register_empty())
	{
		if (m_sending != -1)
		{
			set_tra_rate(9600);
			uint8_t senddata = m_touch_data[m_sendpos];
			transmit_register_setup(senddata);
		}
	}
}

void jpmtouch_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void jpmtouch_device::tra_complete()
{
	if (m_sendpos == 2)
	{
		// Shut down transmitter until there's a character
		m_sending = -1;
		set_tra_rate(attotime::never);
		tx_queue();
	}
	else
	{
		m_sendpos++;
		tx_queue();
	}
}

void jpmtouch_device::touched(uint8_t x, uint8_t y)
{
	if (m_sending == -1)
	{
		m_sending = 1;

		m_touch_data[0] = 0x2a;
		m_touch_data[1] = 0x7 - (y >> 5) + 0x30;
		m_touch_data[2] = (x >> 5) + 0x30;
		m_sendpos = 0;

		tx_queue();
	}
}


/*************************************
 *
 *  MC68681 DUART (TODO)
 *
 *************************************/

#define MC68681_1_CLOCK     3686400
#define MC68681_2_CLOCK     3686400


/*************************************
 *
 *  68000 IRQ handling
 *
 *************************************/

void jpmimpct_state::update_irqs()
{
}

void jpmimpct_video_state::update_irqs()
{
	jpmimpct_state::update_irqs();
	m_maincpu->set_input_line(2, m_tms_irq ? ASSERT_LINE : CLEAR_LINE);
}


/*************************************
 *
 *  Initialisation
 *
 *************************************/

void jpmimpct_video_state::machine_start()
{
	jpmimpct_state::machine_start();

	save_item(NAME(m_tms_irq));
}

void jpmimpct_video_state::machine_reset()
{
	jpmimpct_state::machine_reset();

	/* Reset states */
	m_tms_irq = 0;
}

void jpmimpct_state::machine_start()
{
	m_digits.resolve();
	m_lamp_output.resolve();
	m_pwrled.resolve();
	m_statled.resolve();

	save_item(NAME(m_optic_pattern));
	save_item(NAME(m_payen));
	save_item(NAME(m_hopinhibit));
	save_item(NAME(m_slidesout));
	save_item(NAME(m_hopper));
	save_item(NAME(m_motor));
	save_item(NAME(m_volume_latch));
	save_item(NAME(m_global_volume));
	save_item(NAME(m_coinstate));

}

void jpmimpct_state::machine_reset()
{
	/* Reset states */
	if (m_vfd)
		m_vfd->reset();

	m_coinstate = 0xffff;
}


/*
 *  IP0: MC1489P U7 pin 8
 *  IP1: MC1489P U12 pin 6
 *  IP2: MC1489P U7 pin 11
 *  IP3: MC1489P U12 pin 3
 *  IP4: LM393N U2 pin 1
 *       - Coin meter sense (0 = meter active)
 *  IP5: TEST/DEMO PCB push switch
 *
 *  OP0: SN75188 U6 pins 9 & 10 -> SERIAL PORT pin 6
 *  OP1:
 *  OP2:
 *  OP3: DM7406N U4 pin 3 -> J7 pin 7 (COIN MECH)
 *  OP4: DM7406N U4 pin 5
 *  OP5: DM7406N U4 pin 9 -> J7 pin 5 (COIN MECH)
 *  OP6: DM7406N U4 pin 12
 *  OP7: DM7406N U4 pin 13 -> J7 pin ? (COIN MECH)
 *
 *  TxDA/RxDA: Auxillary serial port
 *  TxDB/TxDB: Data retrieval unit
 */



/*************************************
 *
 *  Sound control
 *
 *************************************/
void jpmimpct_state::volume_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{

	if (ACCESSING_BITS_0_7)
	{
		int changed = m_volume_latch^(data&0xf0);
		m_upd7759->set_rom_bank((data >> 1) & 3);
		m_upd7759->reset_w(BIT(data, 0));

		if ( changed & 0x10)
		{ // digital volume clock line changed
			if ( !(data & 0x10) )
			{ // changed from high to low,
				if ( !(data & 0x20) )//down
				{
					if ( m_global_volume > 0  ) m_global_volume--;
				}
				else
				{
					if ( m_global_volume < 99 ) m_global_volume++;
				}

				float percent = (m_global_volume)/99.0;
				m_upd7759->set_output_gain(0, percent);
			}
		}
		m_volume_latch = (data & 0xf0);
	}
}

void jpmimpct_state::upd7759_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_upd7759->port_w(data);
		m_upd7759->start_w(0);
		m_upd7759->start_w(1);
	}
}

uint16_t jpmimpct_state::upd7759_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_upd7759->busy_r();
	}

	return 0xffff;
}

/*************************************
 *
 *  Mysterious stuff
 *
 *************************************/

uint16_t jpmimpct_state::unk_r()
{
	return 0xffff;
}

void jpmimpct_state::unk_w(uint16_t data)
{
}


uint16_t jpmimpct_state::jpmio_r()
{
	return 0xffff;
}



void jpmimpct_state::pwrled_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_pwrled = !(data & 0x100);
	m_statled = !(data & 0x200);
}

void jpmimpct_state::reels_0123_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_reel[0])
	{
		m_reel[0]->update((data >> 0) & 0x0f);
		awp_draw_reel(machine(),"reel1", *m_reel[0]);
	}

	if (m_reel[1])
	{
		m_reel[1]->update((data >> 4)& 0x0f);
		awp_draw_reel(machine(),"reel2", *m_reel[1]);
	}

	if (m_reel[2])
	{
		m_reel[2]->update((data >> 8)& 0x0f);
		awp_draw_reel(machine(),"reel3", *m_reel[2]);
	}

	if (m_reel[3])
	{
		m_reel[3]->update((data >> 12)& 0x0f);
		awp_draw_reel(machine(),"reel4", *m_reel[3]);
	}
}

void jpmimpct_state::reels_45_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_reel[4])
	{
		m_reel[4]->update((data >> 0)& 0x0f);
		awp_draw_reel(machine(),"reel5", *m_reel[4]);
	}
	if (m_reel[5])
	{
		m_reel[5]->update((data >> 4)& 0x0f);
		awp_draw_reel(machine(),"reel6", *m_reel[5]);
	}
}

void jpmimpct_state::jpm_draw_lamps(uint16_t data, int lamp_strobe)
{
	int i;
	for (i=0; i<16; i++)
	{
		m_Lamps[(16*(m_lamp_strobe & 0xf))+i] = data & 1;
		m_lamp_output[(16*(lamp_strobe & 0xf))+i] = m_Lamps[(16*(lamp_strobe & 0xf))+i];
		data = data >> 1;
	}
}

void jpmimpct_state::lamps_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	jpm_draw_lamps(data, m_lamp_strobe);
}

void jpmimpct_state::digits_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_digits[m_lamp_strobe & 0xf] = data;
}

void jpmimpct_state::lampstrobe_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (data & 0x10)
	{
		m_lamp_strobe = (data + 1) & 0x0f;
	}
}

void jpmimpct_state::slides_non_video_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//Slides
	if ((data & 0xff) != 0x00)
	{
		m_slidesout = 2;
	}
	else if (((data & 0xff) == 0x00) && (m_slidesout == 2))
	{
		m_slidesout = 1;
	}

	// Meters
	int metno = (data >> 8) & 0xff;

	switch (metno)
	{
	case 0x00:
	{
		for (int i = 0; i < 5; i++)
		{
			m_meters->update(i, 0);
		}
		break;
	}
	default:
	{
		m_meters->update(((metno << 2) - 1), 1);
		break;
	}
	}

	int combined_meter = m_meters->get_activity(0) | m_meters->get_activity(1) |
						 m_meters->get_activity(2) | m_meters->get_activity(3) |
						 m_meters->get_activity(4);

	if (combined_meter)
	{
		m_duart->ip4_w(1);
	}
	else
	{
		m_duart->ip4_w(0);
	}
}

void jpmimpct_video_state::slides_video_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if ( data & 0x10 )
	{   // PAYEN ?
		if ( data & 0xf )
		{
			//slide = 1;
		}
		else
		{
			//slide = 0;
		}
	}
	else
	{
		//slide = 0;
	}

	m_meters->update(0, data >> 10);
	//set_duart_1_hack_ip(false);
}




/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/*
 *  0: DIP switches
 *  1: Percentage key
 *  2: Lamps + switches (J10)
 *  3: Lamps + switches (J10)
 *  4: Lamps + switches (J10)
 *      ---- ---x   Back door
 *      ---- --x-   Cash door
 *      ---- -x--   Refill key
 *  5: Lamps + switches (J9)
 *  6: Lamps + switches (J9)
 *  7: Lamps + switches (J9)
 *  8: Payslides
 *  9: Coin mechanism
 */



/*
   Some Input Switch numbers according to test modes

   DSW 0x01 = Switch 0
   DSW 0x02 = 1
   DSW 0x04 = 1
   DSW 0x08 = 1
   DSW 0x10 = 1
   DSW 0x20 = 1
   DSW 0x40 = 1
   DSW 0x80 = 1

   PIA_PORTB: 0x01 = 136
   PIA_PORTB: 0x02 = Switch 137
   PIA_PORTB: 0x04 = 138
   PIA_PORTB: 0x08 = 139
   PIA_PORTB: 0x10 = 140
   PIA_PORTB: 0x20 = 141
   PIA_PORTB: 0x40 = 142
   PIA_PORTB: 0x80 = 143

   PIA_PORTC: 0x01 = 144
   PIA_PORTC: 0x02 = 145
   PIA_PORTC: 0x04 = 146
   PIA_PORTC: 0x08 = 147
   PIA_PORTC: 0x10 = 148
   PIA_PORTC: 0x20 = Switch 149
   PIA_PORTC: 0x40 = 150
   PIA_PORTC: 0x80 = 151
*/


void jpmimpct_state::common_map(address_map& map)
{
	map(0x00000000, 0x001fffff).rom();
	map(0x00400000, 0x00403fff).ram().share("nvram");
	map(0x00480000, 0x0048001f).rw("main_duart", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x00480020, 0x00480021).portr("DSW");      // DSW 0x01 = Switch 0
	map(0x00480022, 0x00480023).portr("PERCENT");
	map(0x00480024, 0x00480025).portr("J10_0");
	map(0x00480026, 0x00480027).portr("J10_1");
	map(0x00480028, 0x00480029).portr("J10_2");   // J10_2: 0x01 = Switch 32
	map(0x0048002a, 0x0048002b).portr("J9_0");    // JP9_0: 0x01 = Switch 40
	map(0x0048002c, 0x0048002d).portr("J9_1");
	map(0x0048002e, 0x0048002f).portr("J9_2");
	map(0x00480030, 0x00480031).portr("PAYCOIN_LEVEL");
	map(0x00480032, 0x00480033).portr("COIN_SENSE");
	map(0x00480034, 0x00480035).r(FUNC(jpmimpct_state::ump_r));

	map(0x00480060, 0x00480067).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);

	map(0x00480080, 0x00480081).w(FUNC(jpmimpct_state::upd7759_w));
	map(0x00480082, 0x00480083).w(FUNC(jpmimpct_state::volume_w));
	map(0x00480084, 0x00480085).r(FUNC(jpmimpct_state::upd7759_r));

	map(0x004800a0, 0x004800af).r(FUNC(jpmimpct_video_state::jpmio_r));

	map(0x004800a0, 0x004800a1).w(FUNC(jpmimpct_state::pwrled_w));
	map(0x004800a2, 0x004800a3).w(FUNC(jpmimpct_state::reels_0123_w));
	map(0x004800a4, 0x004800a5).w(FUNC(jpmimpct_state::reels_45_w));
	map(0x004800a6, 0x004800a7).w(FUNC(jpmimpct_state::slides_non_video_w));
	map(0x004800a8, 0x004800a9).w(FUNC(jpmimpct_state::lamps_w));
	map(0x004800aa, 0x004800ab).w(FUNC(jpmimpct_state::digits_w));
	map(0x004800ae, 0x004800af).w(FUNC(jpmimpct_state::lampstrobe_w));

	// many later sets, eg. Roller Coaster Classic will no accept coins, and run too fast depending
	// on what gets read here, what is it?
	map(0x004801d8, 0x004801d9).r(FUNC(jpmimpct_video_state::unk_r));
	map(0x004801da, 0x004801db).r(FUNC(jpmimpct_video_state::unk_r));
}

void jpmimpct_video_state::impact_video_map(address_map &map)
{
	common_map(map);

	map(0x004800a6, 0x004800a7).w(FUNC(jpmimpct_video_state::slides_video_w));

	map(0x004800e0, 0x004800e1).w(FUNC(jpmimpct_video_state::unk_w));
	map(0x004801dc, 0x004801dd).r(FUNC(jpmimpct_video_state::unk_r));
	map(0x004801de, 0x004801df).r(FUNC(jpmimpct_video_state::unk_r));

	map(0x004801e0, 0x004801ff).rw(m_vidduart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);

	map(0x00800000, 0x00800007).rw(m_dsp, FUNC(tms34010_device::host_r), FUNC(tms34010_device::host_w));

	map(0x00c00000, 0x00ffffff).rom();
}

void jpmimpct_state::impact_non_video_map(address_map &map)
{
	common_map(map);

	map(0x00480040, 0x00480041).r(FUNC(jpmimpct_state::optos_r));

	// are these genuine reads, or just code going wrong prior to them happening?
	map(0x00480086, 0x0048009f).r(FUNC(jpmimpct_state::prot_1_r));
	map(0x004801dc, 0x004801dd).r(FUNC(jpmimpct_state::prot_1_r));
	map(0x004801de, 0x006575ff).r(FUNC(jpmimpct_state::prot_1_r));
	map(0x00657600, 0x00657601).r(FUNC(jpmimpct_state::prot_0_r));
	map(0x00657602, 0x00ffffff).r(FUNC(jpmimpct_state::prot_1_r));
}


/*************************************
 *
 *  Video CPU memory handlers
 *
 *************************************/

void jpmimpct_video_state::tms_program_map(address_map &map)
{
	map(0x00000000, 0x003fffff).mirror(0xf8000000).ram().share("vram");
	map(0x00800000, 0x00ffffff).mirror(0xf8000000).rom().region("user1", 0x100000);
	map(0x02000000, 0x027fffff).mirror(0xf8000000).rom().region("user1", 0);
	map(0x01000000, 0x0100007f).m(m_ramdac, FUNC(bt477_device::map)).umask16(0x00ff);
	map(0x07800000, 0x07bfffff).mirror(0xf8400000).ram();
}

/*************************************
 *
 *  Input definitions
 *
 *************************************/

INPUT_CHANGED_MEMBER(jpmimpct_video_state::touch_port_changed)
{
	if (newval)
	{
		if (m_touch && m_touchx && m_touchy)
			m_touch->touched(m_touchx->read(), m_touchy->read());
	}
}

static INPUT_PORTS_START( touchscreen )
	PORT_START("TOUCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(jpmimpct_video_state::touch_port_changed), 0)

	PORT_START("TOUCH_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("TOUCH_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

INPUT_PORTS_START( jpmimpct_coins )

	PORT_START("COIN_SENSE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::coinsense_r<0>))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::coinsense_r<1>))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::coinsense_r<2>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::coinsense_r<3>))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::coinsense_r<4>))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::coinsense_r<5>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME( "Coin: 1 pound" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(jpmimpct_state::coin_changed), 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME( "Coin: 50p" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(jpmimpct_state::coin_changed), 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME( "Coin: 20p" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(jpmimpct_state::coin_changed), 2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME( "Coin: 10p" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(jpmimpct_state::coin_changed), 3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME( "Token: 20" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(jpmimpct_state::coin_changed), 4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 ) PORT_NAME( "Coin: 5p" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(jpmimpct_state::coin_changed), 5)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(jpmimpct_state::coin_changed)
{
	if (newval)
	{
		logerror("coin inserted %d\n", param+1);
		m_coinstate &= ~(1 << param);
		m_cointimer[param]->adjust(attotime::from_msec(40));
	}
}

INPUT_PORTS_START( jpmimpct_inputs )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSW 0x01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW 0x02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 0x04")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW 0x08")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW 0x10")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 0x20")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW 0x40")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW 0x80")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PERCENT")
	PORT_CONFNAME( 0x0f, 0x00, "Percentage Key" )
	PORT_CONFSETTING(    0x0f, "Not fitted / 68% (Invalid for UK Games)"  )
	PORT_CONFSETTING(    0x0e, "70" )
	PORT_CONFSETTING(    0x0d, "72" )
	PORT_CONFSETTING(    0x0c, "74" )
	PORT_CONFSETTING(    0x0b, "76" )
	PORT_CONFSETTING(    0x0a, "78" )
	PORT_CONFSETTING(    0x09, "80" )
	PORT_CONFSETTING(    0x08, "82" )
	PORT_CONFSETTING(    0x07, "84" )
	PORT_CONFSETTING(    0x06, "86" )
	PORT_CONFSETTING(    0x05, "88" )
	PORT_CONFSETTING(    0x04, "90" )
	PORT_CONFSETTING(    0x03, "92" )
	PORT_CONFSETTING(    0x02, "94" )
	PORT_CONFSETTING(    0x01, "96" )
	PORT_CONFSETTING(    0x00, "98" )

	// some games will display 64p stake if the settings are invalid,
	// others show error "5.5 MODE OF PLAY INVALID", or "91 00 Illegal Mode"
	// games that don't require keys either expect them not to be fitted
	// or will simply ignore them
	PORT_START("J10_0")
	PORT_CONFNAME( 0x0f, 0x08, "Jackpot / Prize Key" )
	PORT_CONFSETTING(    0x0f, "Not Fitted" )
	PORT_CONFSETTING(    0x0e, "0x0e" )
	PORT_CONFSETTING(    0x0d, "0x0d" )
	PORT_CONFSETTING(    0x0c, "0x0c" )
	PORT_CONFSETTING(    0x0b, "0x0b" )
	PORT_CONFSETTING(    0x0a, "8 GBP Cash" )
	PORT_CONFSETTING(    0x09, "8 GBP Token" )
	PORT_CONFSETTING(    0x08, "10 GBP Cash" )
	PORT_CONFSETTING(    0x07, "5 GBP" )
	PORT_CONFSETTING(    0x06, "15 GBP" )
	PORT_CONFSETTING(    0x05, "25 GBP" )
	PORT_CONFSETTING(    0x04, "0x04" )
	PORT_CONFSETTING(    0x03, "0x03" )
	PORT_CONFSETTING(    0x02, "0x02" )
	PORT_CONFSETTING(    0x01, "0x01" )
	PORT_CONFSETTING(    0x00, "0x00" )
	PORT_CONFNAME( 0x70, 0x60, "Stake Key" )
	PORT_CONFSETTING(    0x00, "0x00" )
	PORT_CONFSETTING(    0x10, "0x10" )
	PORT_CONFSETTING(    0x20, "0x20" )
	PORT_CONFSETTING(    0x30, "30p" )
	PORT_CONFSETTING(    0x40, "25p" )
	PORT_CONFSETTING(    0x50, "20p" )
	PORT_CONFSETTING(    0x60, "10p" )
	PORT_CONFSETTING(    0x70, "5p" )
	PORT_CONFNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFSETTING(    0x80, DEF_STR( Off ) )

	PORT_START("J9_0")
	PORT_DIPNAME( 0x01, 0x01, "J9_0: 0x01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "J9_0: 0x02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "J9_0: 0x04")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "J9_0: 0x08")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "J9_0: 0x10")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "J9_0: 0x20")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "J9_0: 0x40")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "J9_0: 0x80")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("J9_1")
	PORT_DIPNAME( 0x01, 0x01, "J9_1: 0x01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "J9_1: 0x02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "J9_1: 0x04")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "J9_1: 0x08")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "J9_1: 0x10")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "J9_1: 0x20")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "J9_1: 0x40")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "J9_1: 0x80")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("J10_1")
	PORT_DIPNAME( 0x01, 0x01, "J10_1: 0x01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "J10_1: 0x02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "J10_1: 0x04")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "J10_1: 0x08")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "J10_1: 0x10")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "J10_1: 0x20")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "J10_1: 0x40")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "J10_1: 0x80")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("J10_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_DOOR ) PORT_NAME("Back Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE // always?
	PORT_DIPNAME( 0x02, 0x02, "J10_2: 0x02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "J10_2: 0x04")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "J10_2: 0x08")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "J10_2: 0x10")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "J10_2: 0x20")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "J10_2: 0x40")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "J10_2: 0x80")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("J9_2")
	PORT_DIPNAME( 0x01, 0x01, "J9_2: 0x01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "J9_2: 0x02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "J9_2: 0x04")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "J9_2: 0x08")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "J9_2: 0x10")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "J9_2: 0x20")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "J9_2: 0x40")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "J9_2: 0x80")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PAYCOIN_LEVEL") // maybe only for certain types of payout mechanism?
	PORT_DIPNAME( 0x01, 0x00, "PAYCOIN_LEVEL: 0x01 (20p cash low)")  // this must be ON or you get an IOU message in j6roller instead of payout, just high being set isn't enough
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "PAYCOIN_LEVEL: 0x02 (token f low)")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "PAYCOIN_LEVEL: 0x04 (token b low)")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "PAYCOIN_LEVEL: 0x08 (100p cash low)")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "PAYCOIN_LEVEL: 0x10 (token b full)")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "PAYCOIN_LEVEL: 0x20") // nothing on j6roller
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "PAYCOIN_LEVEL: 0x40 (20p cash full)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "PAYCOIN_LEVEL: 0x80 (100p cash full)")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TEST_DEMO")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( "Test/Demo" )

	PORT_START("PIA_PORTB")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::hopper_b_0_r))
	PORT_DIPNAME( 0x02, 0x00, "PIA_PORTB: 0x02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "PIA_PORTB: 0x04")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::hopper_b_3_r))
	PORT_DIPNAME( 0x10, 0x10, "PIA_PORTB: 0x10")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "PIA_PORTB: 0x20")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "PIA_PORTB: 0x40")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "PIA_PORTB: 0x80")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA_PORTC")
	PORT_DIPNAME( 0x01, 0x00, "PIA_PORTC: 0x01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "PIA_PORTC: 0x02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "PIA_PORTC: 0x04")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "PIA_PORTC: 0x08")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::hopper_c_4_r))
	PORT_DIPNAME( 0x20, 0x20, "Top Up Switch 0x20")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::hopper_c_6_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(jpmimpct_state::hopper_c_7_r))

	PORT_INCLUDE( jpmimpct_coins )
INPUT_PORTS_END

INPUT_PORTS_START( jpmimpct_video_inputs )
	PORT_INCLUDE( jpmimpct_inputs )

	PORT_MODIFY("J10_2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_DOOR) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE // not always, probably shouldn't be defined here
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( hngmnjpm )
	PORT_INCLUDE( jpmimpct_video_inputs )

	PORT_MODIFY("J10_0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "'3'" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "'2'" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "'1'" )
INPUT_PORTS_END

static INPUT_PORTS_START( coronatn )
	PORT_INCLUDE( jpmimpct_video_inputs )

	PORT_MODIFY("J10_0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Ask Ken" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "'1'" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "'2'" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "'3'" )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( cluedo )
	PORT_INCLUDE( jpmimpct_video_inputs )

	PORT_INCLUDE( touchscreen )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("J10_0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( trivialp )
	PORT_INCLUDE( jpmimpct_video_inputs )

	PORT_INCLUDE( touchscreen )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("J10_0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Pass" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( tqst )
	PORT_INCLUDE( jpmimpct_video_inputs )

	PORT_MODIFY("COINS") // TODO: check coinage
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("J10_0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // TODO: is 0x01 used for anything?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button 1 / Blue") // also used to start a game
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Button 4 / Red") // also used to start a game
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( scrabble )
	PORT_INCLUDE( jpmimpct_video_inputs )

	PORT_INCLUDE( touchscreen )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("J10_0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  TMS34010 configuration
 *
 *************************************/

void jpmimpct_video_state::tms_irq(int state)
{
	m_tms_irq = state;
	update_irqs();
}




/*************************************
 *
 *  Initialisation
 *
 *************************************/

// THIS COULD BE INCORRECT

// B0 = 100p Hopper Opto
// B1 = Hopper High
// B2 = Hopper Low
// B3 = 20p Hopper Opto

int jpmimpct_state::hopper_b_0_r()
{
	uint8_t retval = 0x01;

	if (!m_hopinhibit)//if inhibited, we don't change these flags
	{
		if (m_hopper[0] && m_motor[0]) //&& ((m_hopflag1 & 0x20)==0x20))
		{//100p
			retval &= ~0x01;
		}
	}
	else
	{
		// if payout is inhibited these must be 0, no coin detected? otherwise many sets will give 5.7 error
		// when they test the hoppers
		retval &= ~0x01;
	}

	return retval;
}

int jpmimpct_state::hopper_b_3_r()
{
	uint8_t retval = 0x01;

	if (!m_hopinhibit)//if inhibited, we don't change these flags
	{
		if (((m_hopper[1] && m_motor[1]) || (m_hopper[2] && m_slidesout))) //&& ((m_hopflag2 & 0x20)==0x20))
		{
			retval &= ~0x01;
		}
	}
	else
	{
		// if payout is inhibited these must be 0, no coin detected? otherwise many sets will give 5.7 error
		// when they test the hoppers
		retval &= ~0x01;
	}

	return retval;
}




// C0-C2 = Alpha (output)
// C3
// C4 = 20p Hopper Detect
// C5 = Hopper Top-Up
// C6 = 100p Hopper Detect
// C7 = Payout Verif (Slides)

//    if (StatBtns & 0x20) // Top Up switch
//    retval &= ~0x20;

int jpmimpct_state::hopper_c_4_r()
{
	uint8_t retval = 0x01;

	if (m_hopper[1])
	{
		retval &= ~0x01;
	}

	return retval;
}

int jpmimpct_state::hopper_c_6_r()
{
	uint8_t retval = 0x01;

	if (m_hopper[0])
	{
		retval &= ~0x01;
	}

	return retval;
}

int jpmimpct_state::hopper_c_7_r()
{
	uint8_t retval = 0x01;

	if (!m_hopinhibit)
	{
		if ((m_slidesout==1) && ((m_hopper[2]==0)))
		{
			m_slidesout=0;
			retval &= ~0x01;
		}
	}

	return retval;
}



void jpmimpct_state::payen_a_w(uint8_t data)
{
	m_motor[0] = (data & 0x01);
	m_payen = (data & 0x10);
	m_motor[1] = (data & 0x40);
	m_hopinhibit = (data & 0x80); // prevents coin out

	// same bit as m_payen?
	m_slidesout = (data & 0x10);
}

void jpmimpct_state::display_c_w(uint8_t data)
{
	if (m_vfd)
	{
		//Reset 0x04, data 0x02, clock 0x01
		m_vfd->por(data & 0x04);
		m_vfd->data(data & 0x02);
		m_vfd->sclk(data & 0x01);
	}
}

/*************************************
 *
 *  I/O handlers
 *
 *************************************/

uint16_t jpmimpct_state::optos_r()
{
	return m_optic_pattern;
}

uint16_t jpmimpct_state::prot_1_r()
{
	return 0x01;
}

uint16_t jpmimpct_state::prot_0_r()
{
	return 0x00;
}


uint16_t jpmimpct_state::ump_r()
{
	return 0xff;//0xffff;
}

TIMER_DEVICE_CALLBACK_MEMBER(jpmimpct_state::duart_set_ip5)
{
	auto state = m_testdemo->read() ? 1 : 0;
	m_duart->ip5_w(state);
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jpmimpct_state::duart_irq_handler(int state)
{
	// triggers IRQ 5
	m_maincpu->set_input_line(5, state);
}


// Note 68k is on a sub-card, as is the UPD, so these things can change
// TODO: work out exactly which components are on each card and full list of motherboard components

void jpmimpct_state::base(machine_config &config)
{
	M68000(config, m_maincpu, 8000000);
	// map set later

	MC68681(config, m_duart, MC68681_1_CLOCK);
	m_duart->irq_cb().set(FUNC(jpmimpct_state::duart_irq_handler));

	TIMER(config, "ip5wtimer").configure_periodic(FUNC(jpmimpct_state::duart_set_ip5), attotime::from_hz(1000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(jpmimpct_state::payen_a_w));
	m_ppi->in_pb_callback().set_ioport("PIA_PORTB");
	m_ppi->in_pc_callback().set_ioport("PIA_PORTC");
	m_ppi->out_pc_callback().set(FUNC(jpmimpct_state::display_c_w));

	TIMER(config, "cointimer0").configure_generic(FUNC(jpmimpct_state::coinoff<0>));
	TIMER(config, "cointimer1").configure_generic(FUNC(jpmimpct_state::coinoff<1>));
	TIMER(config, "cointimer2").configure_generic(FUNC(jpmimpct_state::coinoff<2>));
	TIMER(config, "cointimer3").configure_generic(FUNC(jpmimpct_state::coinoff<3>));
	TIMER(config, "cointimer4").configure_generic(FUNC(jpmimpct_state::coinoff<4>));
	TIMER(config, "cointimer5").configure_generic(FUNC(jpmimpct_state::coinoff<5>));

	SPEAKER(config, "mono").front_center();
	UPD7759(config, m_upd7759).add_route(ALL_OUTPUTS, "mono", 0.50);

	METERS(config, m_meters, 0).set_number(5);

	// TODO: only add this to the sets that need it connected
	m_duart->a_tx_cb().set(m_datalogger, FUNC(bacta_datalogger_device::write_txd));
	BACTA_DATALOGGER(config, m_datalogger, 0);
	m_datalogger->rxd_handler().set(m_duart, FUNC(mc68681_device::rx_a_w));
}

void jpmimpct_state::impact_nonvideo_base(machine_config & config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jpmimpct_state::impact_non_video_map);

	config.set_maximum_quantum(attotime::from_hz(30000));
	S16LF01(config, m_vfd);

	config.set_default_layout(layout_jpmimpct);
}

void jpmimpct_state::impact_nonvideo(machine_config &config)
{
	impact_nonvideo_base(config);

	REEL(config, m_reel[0], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[0]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<0>));
	REEL(config, m_reel[1], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[1]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<1>));
	REEL(config, m_reel[2], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[2]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<2>));
	REEL(config, m_reel[3], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[3]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<3>));
	REEL(config, m_reel[4], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[4]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<4>));
	REEL(config, m_reel[5], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[5]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<5>));
}

void jpmimpct_state::impact_nonvideo_altreels(machine_config &config)
{
	impact_nonvideo_base(config);

	// TODO: This is probably incorrect, but j6kungfu startup checks are looking
	// for different reel types than the other games
	REEL(config, m_reel[0], STARPOINT_48STEP_REEL, 4, 12, 0x00, 2);
	m_reel[0]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<0>));
	REEL(config, m_reel[1], STARPOINT_48STEP_REEL, 4, 12, 0x00, 2);
	m_reel[1]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<1>));
	REEL(config, m_reel[2], STARPOINT_48STEP_REEL, 4, 12, 0x00, 2);
	m_reel[2]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<2>));
	REEL(config, m_reel[3], STARPOINT_48STEP_REEL, 4, 12, 0x00, 2);
	m_reel[3]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<3>));
	REEL(config, m_reel[4], STARPOINT_48STEP_REEL, 4, 12, 0x00, 2);
	m_reel[4]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<4>));
	REEL(config, m_reel[5], STARPOINT_48STEP_REEL, 4, 12, 0x00, 2);
	m_reel[5]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<5>));
}

void jpmimpct_state::impact_nonvideo_disc(machine_config &config)
{
	impact_nonvideo_base(config);

	REEL(config, m_reel[0], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[0]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<0>));
	REEL(config, m_reel[1], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[1]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<1>));
	REEL(config, m_reel[2], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[2]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_cb<2>));
	// this is a wheel, not a standard reel, there are can be a number of open windows into it showing all symbols(e.g. 2 in big50, with other cards on it can be seen through grilles)
	// to render this properly in the layout would require a new type of element
	REEL(config, m_reel[3], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[3]->optic_handler().set(FUNC(jpmimpct_state::reel_optic_inv_cb<3>));
}

void jpmimpct_video_state::impact_video(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jpmimpct_video_state::impact_video_map);

	TMS34010(config, m_dsp, 40000000);
	m_dsp->set_addrmap(AS_PROGRAM, &jpmimpct_video_state::tms_program_map);
	m_dsp->set_halt_on_reset(true);
	m_dsp->set_pixel_clock(40000000/16);
	m_dsp->set_pixels_per_clock(4);
	m_dsp->set_scanline_rgb32_callback(FUNC(jpmimpct_video_state::scanline_update));
	m_dsp->output_int().set(FUNC(jpmimpct_video_state::tms_irq));
	m_dsp->set_shiftreg_in_callback(FUNC(jpmimpct_video_state::to_shiftreg));
	m_dsp->set_shiftreg_out_callback(FUNC(jpmimpct_video_state::from_shiftreg));

	// Is this on all video cards? or somwhere else? currently uses a hack
	MC68681(config, m_vidduart, MC68681_2_CLOCK);

	config.set_maximum_quantum(attotime::from_hz(30000));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(40000000/4, 156*4, 0, 100*4, 328, 0, 300);
	screen.set_screen_update("dsp", FUNC(tms34010_device::tms340x0_rgb32));

	BT477(config, m_ramdac, 40000000); // clock unknown
}

void jpmimpct_video_state::impact_video_touch(machine_config &config)
{
	impact_video(config);

	JPM_TOUCHSCREEN(config, m_touch, MC68681_2_CLOCK);
	m_touch->rxd_handler().set(m_vidduart, FUNC(mc68681_device::rx_b_w));
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cluedo )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7322.bin", 0x000000, 0x080000, CRC(049ad02d) SHA1(10297dd466d0019e8d6c162028a23dd235494fb4) )
	ROM_LOAD16_BYTE( "7323.bin", 0x000001, 0x080000, CRC(47ce9c40) SHA1(596a1628142d3c81f2c4ab11ed421f27d082d5f6) )
	ROM_LOAD16_BYTE( "7324.bin", 0x100000, 0x080000, CRC(5946bd75) SHA1(cc4ffa1e4c3628de6b60027d95df413b6d94e669) )
	ROM_LOAD16_BYTE( "7325.bin", 0x100001, 0x080000, CRC(416843ab) SHA1(0d758f7df96384a04596366b1864d5005ca540ee) )

	ROM_LOAD16_BYTE( "6977.bin", 0xc00000, 0x080000, CRC(6030dfc1) SHA1(8746909b0b7f7eb99cf5388ac85db6addb6deee3) )
	ROM_LOAD16_BYTE( "6978.bin", 0xc00001, 0x080000, CRC(21e30e06) SHA1(4e97baa9e39663b662dd202bbaf34be0e29930de) )
	ROM_LOAD16_BYTE( "6979.bin", 0xd00000, 0x080000, CRC(5575162a) SHA1(27f7b5f4ee7d95319b03e2414a25d5b1a6c54fc7) )
	ROM_LOAD16_BYTE( "6980.bin", 0xd00001, 0x080000, CRC(968224df) SHA1(726c278622681206a7f34bafe1b5bb4421232cc4) )
	ROM_LOAD16_BYTE( "6981.bin", 0xe00000, 0x080000, CRC(2ad3ee20) SHA1(9370dab84a255864f40254772199211884d8557b) )
	ROM_LOAD16_BYTE( "6982.bin", 0xe00001, 0x080000, CRC(7478e91b) SHA1(158b473b46aeccf011669cb58dc3a1596370d8f1) )
	ROM_FILL(                    0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "clugrb1", 0x000000, 0x80000, CRC(176ae2df) SHA1(135fd2640c255e5321b1a6ba35f72fa2ba8f04b8) )
	ROM_LOAD16_BYTE( "clugrb2", 0x000001, 0x80000, CRC(06ab2f78) SHA1(4325fd9096e73956310e97e244c7fe1ee8d27f5c) )
	ROM_COPY( "user1", 0x00000, 0x100000, 0x100000 )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "1214.bin", 0x000000, 0x80000, CRC(fe43aeae) SHA1(017a471af5766ef41fa46982c02941fb4fc35174) )
ROM_END

ROM_START( cluedod )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7326.bin", 0x000000, 0x080000, CRC(6c6b523e) SHA1(3a140aff92c00da45433698c3c946fc0134b4863) )
	ROM_LOAD16_BYTE( "7323.bin", 0x000001, 0x080000, CRC(47ce9c40) SHA1(596a1628142d3c81f2c4ab11ed421f27d082d5f6) )
	ROM_LOAD16_BYTE( "7324.bin", 0x100000, 0x080000, CRC(5946bd75) SHA1(cc4ffa1e4c3628de6b60027d95df413b6d94e669) )
	ROM_LOAD16_BYTE( "7325.bin", 0x100001, 0x080000, CRC(416843ab) SHA1(0d758f7df96384a04596366b1864d5005ca540ee) )

	ROM_LOAD16_BYTE( "6977.bin", 0xc00000, 0x080000, CRC(6030dfc1) SHA1(8746909b0b7f7eb99cf5388ac85db6addb6deee3) )
	ROM_LOAD16_BYTE( "6978.bin", 0xc00001, 0x080000, CRC(21e30e06) SHA1(4e97baa9e39663b662dd202bbaf34be0e29930de) )
	ROM_LOAD16_BYTE( "6979.bin", 0xd00000, 0x080000, CRC(5575162a) SHA1(27f7b5f4ee7d95319b03e2414a25d5b1a6c54fc7) )
	ROM_LOAD16_BYTE( "6980.bin", 0xd00001, 0x080000, CRC(968224df) SHA1(726c278622681206a7f34bafe1b5bb4421232cc4) )
	ROM_LOAD16_BYTE( "6981.bin", 0xe00000, 0x080000, CRC(2ad3ee20) SHA1(9370dab84a255864f40254772199211884d8557b) )
	ROM_LOAD16_BYTE( "6982.bin", 0xe00001, 0x080000, CRC(7478e91b) SHA1(158b473b46aeccf011669cb58dc3a1596370d8f1) )
	ROM_FILL(                    0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "clugrb1", 0x000000, 0x80000, CRC(176ae2df) SHA1(135fd2640c255e5321b1a6ba35f72fa2ba8f04b8) )
	ROM_LOAD16_BYTE( "clugrb2", 0x000001, 0x80000, CRC(06ab2f78) SHA1(4325fd9096e73956310e97e244c7fe1ee8d27f5c) )
	ROM_COPY( "user1", 0x00000, 0x100000, 0x100000 )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "1214.bin", 0x000000, 0x80000, CRC(fe43aeae) SHA1(017a471af5766ef41fa46982c02941fb4fc35174) )
ROM_END

ROM_START( cluedo2c )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clu2c1.bin", 0x000000, 0x080000, CRC(bf94a3c0) SHA1(e5a0d17136691642aba339f574aec7c27ed90848) )
	ROM_LOAD16_BYTE( "clu2c2.bin", 0x000001, 0x080000, CRC(960cda80) SHA1(6b5946ed1241bc673f42991f57e0c74753085b63) )
	ROM_LOAD16_BYTE( "clu2c3.bin", 0x100000, 0x080000, CRC(9d61b28d) SHA1(41c0e17b3933686a2e6f343cd39f90e5663c7787) )
	ROM_LOAD16_BYTE( "clu2c4.bin", 0x100001, 0x080000, CRC(a427d67b) SHA1(a8944e1d86548911a65b398245a0f8f236491644) )

	ROM_LOAD16_BYTE( "6977.bin", 0xc00000, 0x080000, CRC(6030dfc1) SHA1(8746909b0b7f7eb99cf5388ac85db6addb6deee3) )
	ROM_LOAD16_BYTE( "6978.bin", 0xc00001, 0x080000, CRC(21e30e06) SHA1(4e97baa9e39663b662dd202bbaf34be0e29930de) )
	ROM_LOAD16_BYTE( "6979.bin", 0xd00000, 0x080000, CRC(5575162a) SHA1(27f7b5f4ee7d95319b03e2414a25d5b1a6c54fc7) )
	ROM_LOAD16_BYTE( "6980.bin", 0xd00001, 0x080000, CRC(968224df) SHA1(726c278622681206a7f34bafe1b5bb4421232cc4) )
	ROM_LOAD16_BYTE( "6981.bin", 0xe00000, 0x080000, CRC(2ad3ee20) SHA1(9370dab84a255864f40254772199211884d8557b) )
	ROM_LOAD16_BYTE( "6982.bin", 0xe00001, 0x080000, CRC(7478e91b) SHA1(158b473b46aeccf011669cb58dc3a1596370d8f1) )
	ROM_FILL(                    0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "clugrb1", 0x000000, 0x80000, CRC(176ae2df) SHA1(135fd2640c255e5321b1a6ba35f72fa2ba8f04b8) )
	ROM_LOAD16_BYTE( "clugrb2", 0x000001, 0x80000, CRC(06ab2f78) SHA1(4325fd9096e73956310e97e244c7fe1ee8d27f5c) )
	ROM_COPY( "user1", 0x00000, 0x100000, 0x100000 )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "1214.bin", 0x000000, 0x80000, CRC(fe43aeae) SHA1(017a471af5766ef41fa46982c02941fb4fc35174) )
ROM_END

ROM_START( cluedo2 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clu21.bin", 0x000000, 0x080000, CRC(b1aa0103)SHA1(52d10a428710cd04313a2638fc3c23fb9d0ab6db))
	ROM_LOAD16_BYTE( "clu22.bin", 0x000001, 0x080000, CRC(90d8dd28)SHA1(3124a8313c6b362176283e145c4af27f5deac683))
	ROM_LOAD16_BYTE( "clu23.bin", 0x100000, 0x080000, CRC(196bd993)SHA1(50920441707fc6cae9d36961d92ce213e53c4238))
	ROM_LOAD16_BYTE( "clu24.bin", 0x100001, 0x080000, CRC(3f5c1259)SHA1(dfdbb66a81716a0ced7510e277f6f321516f57af))

	ROM_LOAD16_BYTE( "6977.bin", 0xc00000, 0x080000, CRC(6030dfc1) SHA1(8746909b0b7f7eb99cf5388ac85db6addb6deee3) )
	ROM_LOAD16_BYTE( "6978.bin", 0xc00001, 0x080000, CRC(21e30e06) SHA1(4e97baa9e39663b662dd202bbaf34be0e29930de) )
	ROM_LOAD16_BYTE( "6979.bin", 0xd00000, 0x080000, CRC(5575162a) SHA1(27f7b5f4ee7d95319b03e2414a25d5b1a6c54fc7) )
	ROM_LOAD16_BYTE( "6980.bin", 0xd00001, 0x080000, CRC(968224df) SHA1(726c278622681206a7f34bafe1b5bb4421232cc4) )
	ROM_LOAD16_BYTE( "6981.bin", 0xe00000, 0x080000, CRC(2ad3ee20) SHA1(9370dab84a255864f40254772199211884d8557b) )
	ROM_LOAD16_BYTE( "6982.bin", 0xe00001, 0x080000, CRC(7478e91b) SHA1(158b473b46aeccf011669cb58dc3a1596370d8f1) )
	ROM_FILL(                    0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "clugrb1", 0x000000, 0x80000, CRC(176ae2df) SHA1(135fd2640c255e5321b1a6ba35f72fa2ba8f04b8) )
	ROM_LOAD16_BYTE( "clugrb2", 0x000001, 0x80000, CRC(06ab2f78) SHA1(4325fd9096e73956310e97e244c7fe1ee8d27f5c) )
	ROM_COPY( "user1", 0x00000, 0x100000, 0x100000 )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "clue2as1.bin", 0x000000, 0x80000, CRC(16b2bc45) SHA1(56963f5d63b5a091b89b96f4ca9327010006c024) )
ROM_END

ROM_START( trivialp )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1422.bin", 0x000000, 0x080000, CRC(5e39c946) SHA1(bae7f572a32e90d716813271f03e7868be603086) )
	ROM_LOAD16_BYTE( "1423.bin", 0x000001, 0x080000, CRC(bb48c225) SHA1(b479f0bdb69ad11af17b5457c02a9d9618ede455) )
	ROM_LOAD16_BYTE( "1424.bin", 0x100000, 0x080000, CRC(c37d045b) SHA1(3c127b14e1dc1e453fb08c741847c712d1fea78b) )
	ROM_LOAD16_BYTE( "1425.bin", 0x100001, 0x080000, CRC(8d209f61) SHA1(3e16ee4c43a31da2e6773a938a20c616a5e6179b) )

	ROM_LOAD16_BYTE( "tp-q1.bin", 0xc00000, 0x080000, CRC(98d42cfd) SHA1(67a6745d55493034128f767b518d86dedc9c22a6) )
	ROM_LOAD16_BYTE( "tp-q2.bin", 0xc00001, 0x080000, CRC(8a670ee8) SHA1(33628b34f4a0413f2f39e26520169d0eff9942c5) )
	ROM_LOAD16_BYTE( "tp-q3.bin", 0xd00000, 0x080000, CRC(eb47f94e) SHA1(957812b63de4532b9175214db7947c96264a48f1) )
	ROM_LOAD16_BYTE( "tp-q4.bin", 0xd00001, 0x080000, CRC(23c01c99) SHA1(187c3448ae1cb44ca6a4a829e64b860ee7548ac5) )
	ROM_LOAD16_BYTE( "tp-q5.bin", 0xe00000, 0x080000, CRC(1c9f4f8a) SHA1(7541d518d24e59140d62a869b27bcc15b205054d) )
	ROM_LOAD16_BYTE( "tp-q6.bin", 0xe00001, 0x080000, CRC(df9da57d) SHA1(a3e29cb03bd780de2c5454c86d6dc48e1c6c63bc) )
	ROM_LOAD16_BYTE( "tp-q7.bin", 0xf00000, 0x080000, CRC(e075e5d7) SHA1(3490730c569678d48fb2d810484de063882f71a5) )
	ROM_LOAD16_BYTE( "tp-q8.bin", 0xf00001, 0x080000, CRC(12f90e74) SHA1(a39a1cee6107d1e83954e3cabf191fd5c89777f8) )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "tp-gr1.bin", 0x000000, 0x100000, CRC(7fa955f7) SHA1(9ecae4c8c26bfa1701c39148099bf0f8b5974ac8) )
	ROM_LOAD16_BYTE( "tp-gr2.bin", 0x000001, 0x100000, CRC(2495d785) SHA1(eb89eb299a7000364a0a0f59459d1ec27755fca1) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "tp-snd.bin", 0x000000, 0x80000, CRC(7e2cb00a) SHA1(670ee5dd5c60313676b9271901b4df9e6ebd5955) )
ROM_END

ROM_START( trivialpd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1426.bin", 0x000000, 0x080000, CRC(36c84b55) SHA1(c01dc797bd578dfe5979f39a6acfdb3c5744b298) ) //was labelled 1424, typo?
	ROM_LOAD16_BYTE( "1423.bin", 0x000001, 0x080000, CRC(bb48c225) SHA1(b479f0bdb69ad11af17b5457c02a9d9618ede455) )
	ROM_LOAD16_BYTE( "1424.bin", 0x100000, 0x080000, CRC(c37d045b) SHA1(3c127b14e1dc1e453fb08c741847c712d1fea78b) )
	ROM_LOAD16_BYTE( "1425.bin", 0x100001, 0x080000, CRC(8d209f61) SHA1(3e16ee4c43a31da2e6773a938a20c616a5e6179b) )

	ROM_LOAD16_BYTE( "tp-q1.bin", 0xc00000, 0x080000, CRC(98d42cfd) SHA1(67a6745d55493034128f767b518d86dedc9c22a6) )
	ROM_LOAD16_BYTE( "tp-q2.bin", 0xc00001, 0x080000, CRC(8a670ee8) SHA1(33628b34f4a0413f2f39e26520169d0eff9942c5) )
	ROM_LOAD16_BYTE( "tp-q3.bin", 0xd00000, 0x080000, CRC(eb47f94e) SHA1(957812b63de4532b9175214db7947c96264a48f1) )
	ROM_LOAD16_BYTE( "tp-q4.bin", 0xd00001, 0x080000, CRC(23c01c99) SHA1(187c3448ae1cb44ca6a4a829e64b860ee7548ac5) )
	ROM_LOAD16_BYTE( "tp-q5.bin", 0xe00000, 0x080000, CRC(1c9f4f8a) SHA1(7541d518d24e59140d62a869b27bcc15b205054d) )
	ROM_LOAD16_BYTE( "tp-q6.bin", 0xe00001, 0x080000, CRC(df9da57d) SHA1(a3e29cb03bd780de2c5454c86d6dc48e1c6c63bc) )
	ROM_LOAD16_BYTE( "tp-q7.bin", 0xf00000, 0x080000, CRC(e075e5d7) SHA1(3490730c569678d48fb2d810484de063882f71a5) )
	ROM_LOAD16_BYTE( "tp-q8.bin", 0xf00001, 0x080000, CRC(12f90e74) SHA1(a39a1cee6107d1e83954e3cabf191fd5c89777f8) )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "tp-gr1.bin", 0x000000, 0x100000, CRC(7fa955f7) SHA1(9ecae4c8c26bfa1701c39148099bf0f8b5974ac8) )
	ROM_LOAD16_BYTE( "tp-gr2.bin", 0x000001, 0x100000, CRC(2495d785) SHA1(eb89eb299a7000364a0a0f59459d1ec27755fca1) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "tp-snd.bin", 0x000000, 0x80000, CRC(7e2cb00a) SHA1(670ee5dd5c60313676b9271901b4df9e6ebd5955) )
ROM_END

ROM_START( trivialpo )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	// set only contained these 2 roms.. others are assumed to be the same for now
	ROM_LOAD16_BYTE( "tpswpp1", 0x000000, 0x080000, CRC(9d3cb9b7) SHA1(959cc0e2254aa3a3a4e9f5814ca6ee2b0e486fb3) )
	ROM_LOAD16_BYTE( "tpswpp2", 0x000001, 0x080000, CRC(4a2f1476) SHA1(c08a5c99b44ee3e5457cb26a29405b2f01fd5a27) )
	ROM_LOAD16_BYTE( "1424.bin", 0x100000, 0x080000, CRC(c37d045b) SHA1(3c127b14e1dc1e453fb08c741847c712d1fea78b) )
	ROM_LOAD16_BYTE( "1425.bin", 0x100001, 0x080000, CRC(8d209f61) SHA1(3e16ee4c43a31da2e6773a938a20c616a5e6179b) )

	ROM_LOAD16_BYTE( "tp-q1.bin", 0xc00000, 0x080000, CRC(98d42cfd) SHA1(67a6745d55493034128f767b518d86dedc9c22a6) )
	ROM_LOAD16_BYTE( "tp-q2.bin", 0xc00001, 0x080000, CRC(8a670ee8) SHA1(33628b34f4a0413f2f39e26520169d0eff9942c5) )
	ROM_LOAD16_BYTE( "tp-q3.bin", 0xd00000, 0x080000, CRC(eb47f94e) SHA1(957812b63de4532b9175214db7947c96264a48f1) )
	ROM_LOAD16_BYTE( "tp-q4.bin", 0xd00001, 0x080000, CRC(23c01c99) SHA1(187c3448ae1cb44ca6a4a829e64b860ee7548ac5) )
	ROM_LOAD16_BYTE( "tp-q5.bin", 0xe00000, 0x080000, CRC(1c9f4f8a) SHA1(7541d518d24e59140d62a869b27bcc15b205054d) )
	ROM_LOAD16_BYTE( "tp-q6.bin", 0xe00001, 0x080000, CRC(df9da57d) SHA1(a3e29cb03bd780de2c5454c86d6dc48e1c6c63bc) )
	ROM_LOAD16_BYTE( "tp-q7.bin", 0xf00000, 0x080000, CRC(e075e5d7) SHA1(3490730c569678d48fb2d810484de063882f71a5) )
	ROM_LOAD16_BYTE( "tp-q8.bin", 0xf00001, 0x080000, CRC(12f90e74) SHA1(a39a1cee6107d1e83954e3cabf191fd5c89777f8) )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "tp-gr1.bin", 0x000000, 0x100000, CRC(7fa955f7) SHA1(9ecae4c8c26bfa1701c39148099bf0f8b5974ac8) )
	ROM_LOAD16_BYTE( "tp-gr2.bin", 0x000001, 0x100000, CRC(2495d785) SHA1(eb89eb299a7000364a0a0f59459d1ec27755fca1) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "tp-snd.bin", 0x000000, 0x80000, CRC(7e2cb00a) SHA1(670ee5dd5c60313676b9271901b4df9e6ebd5955) )
ROM_END


ROM_START( scrabble )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1562.bin", 0x000000, 0x080000, CRC(d7303b98) SHA1(46e8ed04c8fdc092b7d8910d3e3f6cc62f691646) )
	ROM_LOAD16_BYTE( "1563.bin", 0x000001, 0x080000, CRC(77f61ba1) SHA1(276dc8b2c23880740309c456d4e4b2eae249cdde) )
	ROM_FILL(                    0x100000, 0x100000, 0xff )

	ROM_LOAD16_BYTE( "scra-q1.bin", 0xc00000, 0x080000, CRC(bcbc6328) SHA1(cbf8901e80e7bc1f82f6f7d4d5f6a658af98a6f9) )
	ROM_LOAD16_BYTE( "scra-q2.bin", 0xc00001, 0x080000, CRC(c2147999) SHA1(f21dc0f3f4ba0d6304801bc492a759534447d747) )
	ROM_LOAD16_BYTE( "scra-q3.bin", 0xd00000, 0x080000, CRC(622cebb9) SHA1(9b7c2204462d4912462bad6c4dcf096abe1381bb) )
	ROM_LOAD16_BYTE( "scra-q4.bin", 0xd00001, 0x080000, CRC(fd4b587b) SHA1(e29512a075fbc511271d6902c8900a9b0261355c) )
	ROM_LOAD16_BYTE( "scra-q5.bin", 0xe00000, 0x080000, CRC(fbc28978) SHA1(ce2549da858888d49677ec982ab3c21cf292939b) )
	ROM_LOAD16_BYTE( "scra-q6.bin", 0xe00001, 0x080000, CRC(8b792c9c) SHA1(9a5cc6c4d7e807cbabd174ab7454cdaa93dc3cec) )
	ROM_FILL(                       0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "scra-g1.bin", 0x000000, 0x100000, CRC(04a17df9) SHA1(c215c90d8add3ff608c24aac242369874f6bf9d7) )
	ROM_LOAD16_BYTE( "scra-g2.bin", 0x000001, 0x100000, CRC(724375e6) SHA1(709211a2d7b86f4e83c94a37010fe61ef9a734de) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "scra-snd.bin", 0x000000, 0x80000, CRC(287759ef) SHA1(bd37500689b7b2fb4fbc65056e92486c0c00ff61) )
ROM_END

ROM_START( scrabbled )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1564.bin", 0x000000, 0x080000, CRC(bfc1b98b) SHA1(09278f06efa18c1578f61e9b1bfed0f4f6657cb6) )
	ROM_LOAD16_BYTE( "1563.bin", 0x000001, 0x080000, CRC(77f61ba1) SHA1(276dc8b2c23880740309c456d4e4b2eae249cdde) )
	ROM_FILL(                    0x100000, 0x100000, 0xff )

	ROM_LOAD16_BYTE( "scra-q1.bin", 0xc00000, 0x080000, CRC(bcbc6328) SHA1(cbf8901e80e7bc1f82f6f7d4d5f6a658af98a6f9) )
	ROM_LOAD16_BYTE( "scra-q2.bin", 0xc00001, 0x080000, CRC(c2147999) SHA1(f21dc0f3f4ba0d6304801bc492a759534447d747) )
	ROM_LOAD16_BYTE( "scra-q3.bin", 0xd00000, 0x080000, CRC(622cebb9) SHA1(9b7c2204462d4912462bad6c4dcf096abe1381bb) )
	ROM_LOAD16_BYTE( "scra-q4.bin", 0xd00001, 0x080000, CRC(fd4b587b) SHA1(e29512a075fbc511271d6902c8900a9b0261355c) )
	ROM_LOAD16_BYTE( "scra-q5.bin", 0xe00000, 0x080000, CRC(fbc28978) SHA1(ce2549da858888d49677ec982ab3c21cf292939b) )
	ROM_LOAD16_BYTE( "scra-q6.bin", 0xe00001, 0x080000, CRC(8b792c9c) SHA1(9a5cc6c4d7e807cbabd174ab7454cdaa93dc3cec) )
	ROM_FILL(                       0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "scra-g1.bin", 0x000000, 0x100000, CRC(04a17df9) SHA1(c215c90d8add3ff608c24aac242369874f6bf9d7) )
	ROM_LOAD16_BYTE( "scra-g2.bin", 0x000001, 0x100000, CRC(724375e6) SHA1(709211a2d7b86f4e83c94a37010fe61ef9a734de) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "scra-snd.bin", 0x000000, 0x80000, CRC(287759ef) SHA1(bd37500689b7b2fb4fbc65056e92486c0c00ff61) )
ROM_END

ROM_START( hngmnjpm )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20264.bin", 0x000000, 0x080000, CRC(50074528) SHA1(8128b2270518af873df4b94d50c5c9849dda3e42) )
	ROM_LOAD16_BYTE( "20265.bin", 0x000001, 0x080000, CRC(a0a6985c) SHA1(ed960e6e88df111aebf208d7105dc241aa916684) )
	ROM_FILL(                     0x100000, 0x100000, 0xff )

	ROM_LOAD16_BYTE( "hang-q1.bin", 0xc00000, 0x080000, CRC(0be99a57) SHA1(49fe7faeccd3f9608927ff333fd5783e3cd7d266) )
	ROM_LOAD16_BYTE( "hang-q2.bin", 0xc00001, 0x080000, CRC(71328f71) SHA1(59481b27dbcad109070cc4fd5c9c93f948991f03) )
	ROM_LOAD16_BYTE( "hang-q3.bin", 0xd00000, 0x080000, CRC(3fabeb81) SHA1(67b4561ec4ac8c00728c86e2bce66f432c5f1e86) )
	ROM_LOAD16_BYTE( "hang-q4.bin", 0xd00001, 0x080000, CRC(64fbf56b) SHA1(c5077f9995b890925ef608742ba77ef995de5a3b) )
	ROM_LOAD16_BYTE( "hang-q5.bin", 0xe00000, 0x080000, CRC(283e0c7f) SHA1(64ed626e181d851d3ffd4a1c0e613cd769e0ae31) )
	ROM_LOAD16_BYTE( "hang-q6.bin", 0xe00001, 0x080000, CRC(9a6d3667) SHA1(b4706d77dcd43e6f75e3e5e8bd1fbeebe84b8f60) )
	ROM_FILL(                       0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "hang-gr1.bin", 0x000000, 0x100000, CRC(5919344c) SHA1(b5c1f98ebfc65743fa2f6c264179ed7115532a6b) )
	ROM_LOAD16_BYTE( "hang-gr2.bin", 0x000001, 0x100000, CRC(3194c6d4) SHA1(11d5e7bfe60912b0eab2a1d06d1a74853ec23567) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "hang-so1.bin", 0x000000, 0x80000, CRC(5efe1712) SHA1(e4e7a73a1b1897ed6e96306f99d234fb3b47c59b) )

	/* Likely to be the same for the other games */
	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "s60-3.bin", 0x000000, 0x0117, CRC(19e1d28b) SHA1(12dff4bea16b95807f1a9455b6785468ca5de858) )
	ROM_LOAD( "s61-6.bin", 0x000000, 0x0117, CRC(c72cec0e) SHA1(9d6e5510600987f9359af9ecc3e95f5bd8444bcd) )
	ROM_LOAD( "ig1.1.bin", 0x000000, 0x02DD, CRC(4e11fa4e) SHA1(ded2d2086c4360708462024054e5409962ea8589) )
	ROM_LOAD( "ig2.1.bin", 0x000000, 0x0157, CRC(2365878b) SHA1(d91d9906aadcfd8cff7ee6b92449c522f73a29e1) )
	ROM_LOAD( "ig3.2.bin", 0x000000, 0x0117, CRC(4970dad7) SHA1(c5931db3d66c7d1027a762be10f9e3d9e321b70f) )
	ROM_LOAD( "jpms6.bin", 0x000000, 0x0117, CRC(1fba3b6f) SHA1(0e33e49cbf24e836deb1ef16385ff20549ef188e) )
	ROM_LOAD( "mem-2.bin", 0x000000, 0x0157, CRC(92832445) SHA1(b6edcc6d4f721f0e91e9fcf322163db017afaee1) )
ROM_END

ROM_START( hngmnjpmd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20266.bin", 0x000000, 0x080000, CRC(38f6c73b) SHA1(71bdeee0656686bd420d9cf1928a8118372c57e4) )
	ROM_LOAD16_BYTE( "20265.bin", 0x000001, 0x080000, CRC(a0a6985c) SHA1(ed960e6e88df111aebf208d7105dc241aa916684) )
	ROM_FILL(                     0x100000, 0x100000, 0xff )

	ROM_LOAD16_BYTE( "hang-q1.bin", 0xc00000, 0x080000, CRC(0be99a57) SHA1(49fe7faeccd3f9608927ff333fd5783e3cd7d266) )
	ROM_LOAD16_BYTE( "hang-q2.bin", 0xc00001, 0x080000, CRC(71328f71) SHA1(59481b27dbcad109070cc4fd5c9c93f948991f03) )
	ROM_LOAD16_BYTE( "hang-q3.bin", 0xd00000, 0x080000, CRC(3fabeb81) SHA1(67b4561ec4ac8c00728c86e2bce66f432c5f1e86) )
	ROM_LOAD16_BYTE( "hang-q4.bin", 0xd00001, 0x080000, CRC(64fbf56b) SHA1(c5077f9995b890925ef608742ba77ef995de5a3b) )
	ROM_LOAD16_BYTE( "hang-q5.bin", 0xe00000, 0x080000, CRC(283e0c7f) SHA1(64ed626e181d851d3ffd4a1c0e613cd769e0ae31) )
	ROM_LOAD16_BYTE( "hang-q6.bin", 0xe00001, 0x080000, CRC(9a6d3667) SHA1(b4706d77dcd43e6f75e3e5e8bd1fbeebe84b8f60) )
	ROM_FILL(                       0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "hang-gr1.bin", 0x000000, 0x100000, CRC(5919344c) SHA1(b5c1f98ebfc65743fa2f6c264179ed7115532a6b) )
	ROM_LOAD16_BYTE( "hang-gr2.bin", 0x000001, 0x100000, CRC(3194c6d4) SHA1(11d5e7bfe60912b0eab2a1d06d1a74853ec23567) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "hang-so1.bin", 0x000000, 0x80000, CRC(5efe1712) SHA1(e4e7a73a1b1897ed6e96306f99d234fb3b47c59b) )

	/* Likely to be the same for the other games */
	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "s60-3.bin", 0x000000, 0x0117, CRC(19e1d28b) SHA1(12dff4bea16b95807f1a9455b6785468ca5de858) )
	ROM_LOAD( "s61-6.bin", 0x000000, 0x0117, CRC(c72cec0e) SHA1(9d6e5510600987f9359af9ecc3e95f5bd8444bcd) )
	ROM_LOAD( "ig1.1.bin", 0x000000, 0x02DD, CRC(4e11fa4e) SHA1(ded2d2086c4360708462024054e5409962ea8589) )
	ROM_LOAD( "ig2.1.bin", 0x000000, 0x0157, CRC(2365878b) SHA1(d91d9906aadcfd8cff7ee6b92449c522f73a29e1) )
	ROM_LOAD( "ig3.2.bin", 0x000000, 0x0117, CRC(4970dad7) SHA1(c5931db3d66c7d1027a762be10f9e3d9e321b70f) )
	ROM_LOAD( "jpms6.bin", 0x000000, 0x0117, CRC(1fba3b6f) SHA1(0e33e49cbf24e836deb1ef16385ff20549ef188e) )
	ROM_LOAD( "mem-2.bin", 0x000000, 0x0157, CRC(92832445) SHA1(b6edcc6d4f721f0e91e9fcf322163db017afaee1) )
ROM_END

ROM_START( coronatn )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20824.bin", 0x000000, 0x080000, CRC(f5cc07cb) SHA1(45b83829ba9bd5f22c2978bbde9c0e25c476e719) )
	ROM_LOAD16_BYTE( "20825.bin", 0x000001, 0x080000, CRC(2e749edf) SHA1(12b24836a71085aef8ca1bc61e6671f8d6e1908c) )
	ROM_FILL(                     0x100000, 0x100000, 0xff )

	ROM_LOAD16_BYTE( "cs-q1.bin", 0xc00000, 0x080000, CRC(beef496a) SHA1(6089ee8b0821d5b8cb8f724748888a0915083622) )
	ROM_LOAD16_BYTE( "cs-q2.bin", 0xc00001, 0x080000, CRC(16f88f36) SHA1(78c829d837cc09fdd1119ba73168d272843f7f50) )
	ROM_LOAD16_BYTE( "cs-q3.bin", 0xd00000, 0x080000, CRC(1d412b03) SHA1(2400fa776effeb2ab21234a6ecf183ed0cffa92e) )
	ROM_LOAD16_BYTE( "cs-q4.bin", 0xd00001, 0x080000, CRC(55c23ab9) SHA1(0eaa8c88315ef4544f1d1ef2fec2c6edc3589db3) )
	ROM_LOAD16_BYTE( "cs-q5.bin", 0xe00000, 0x080000, CRC(289f4db0) SHA1(8eca9df9e278bf77be4b2aad4c80ea6a1880fe96) )
	ROM_LOAD16_BYTE( "cs-q6.bin", 0xe00001, 0x080000, CRC(791d9d39) SHA1(44f3dcbfe8523118d52785844e103a480e8e13b5) )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "cs-ag1.bin", 0x000000, 0x100000, CRC(7ce449cc) SHA1(408e1405c80e623ee120cea65760ca9a8554cc29) )
	ROM_LOAD16_BYTE( "cs-ag2.bin", 0x000001, 0x100000, CRC(7026df0c) SHA1(a000d72c06ad37879673324880fb0e715f55788e) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "cs-sound.bin", 0x000000, 0x80000, CRC(96ea4e9f) SHA1(a5443d893f38f3e279f2eb9f4500547e7b8efa37) )
ROM_END

ROM_START( coronatnd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20826.bin", 0x000000, 0x080000, CRC(9d3d85d8) SHA1(a6ab622fac9ece04f9b255e10eac7812549afb8a) )
	ROM_LOAD16_BYTE( "20825.bin", 0x000001, 0x080000, CRC(2e749edf) SHA1(12b24836a71085aef8ca1bc61e6671f8d6e1908c) )
	ROM_FILL(                     0x100000, 0x100000, 0xff )

	ROM_LOAD16_BYTE( "cs-q1.bin", 0xc00000, 0x080000, CRC(beef496a) SHA1(6089ee8b0821d5b8cb8f724748888a0915083622) )
	ROM_LOAD16_BYTE( "cs-q2.bin", 0xc00001, 0x080000, CRC(16f88f36) SHA1(78c829d837cc09fdd1119ba73168d272843f7f50) )
	ROM_LOAD16_BYTE( "cs-q3.bin", 0xd00000, 0x080000, CRC(1d412b03) SHA1(2400fa776effeb2ab21234a6ecf183ed0cffa92e) )
	ROM_LOAD16_BYTE( "cs-q4.bin", 0xd00001, 0x080000, CRC(55c23ab9) SHA1(0eaa8c88315ef4544f1d1ef2fec2c6edc3589db3) )
	ROM_LOAD16_BYTE( "cs-q5.bin", 0xe00000, 0x080000, CRC(289f4db0) SHA1(8eca9df9e278bf77be4b2aad4c80ea6a1880fe96) )
	ROM_LOAD16_BYTE( "cs-q6.bin", 0xe00001, 0x080000, CRC(791d9d39) SHA1(44f3dcbfe8523118d52785844e103a480e8e13b5) )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "cs-ag1.bin", 0x000000, 0x100000, CRC(7ce449cc) SHA1(408e1405c80e623ee120cea65760ca9a8554cc29) )
	ROM_LOAD16_BYTE( "cs-ag2.bin", 0x000001, 0x100000, CRC(7026df0c) SHA1(a000d72c06ad37879673324880fb0e715f55788e) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "cs-sound.bin", 0x000000, 0x80000, CRC(96ea4e9f) SHA1(a5443d893f38f3e279f2eb9f4500547e7b8efa37) )
ROM_END

ROM_START( tqst )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prom1n.bin",0x000000, 0x080000, CRC(a9cacb88) SHA1(2cc565e8083926acab8c8b14ad90bd50f7597038) )
	ROM_LOAD16_BYTE( "prom2.bin", 0x000001, 0x080000, CRC(a665e72e) SHA1(76440ae69f61eac1c6fe59dae295826a145bc940) )

	ROM_REGION16_LE( 0x200000, "user1", ROMREGION_ERASEFF )
	/* 0x000000 - 0x0fffff empty? */
	ROM_LOAD16_BYTE( "u16.bin",   0x100000, 0x080000, CRC(ae9b6829) SHA1(2c8ed5060d751bca0af54305164512fae8ff88e9) )
	ROM_LOAD16_BYTE( "u17.bin",   0x100001, 0x080000, CRC(7786340d) SHA1(96ded0af403fa3f0e7604f9ae0952036b3652665) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "025rs1-0.bin", 0x0000, 0x080000, CRC(c4dbff24) SHA1(2e4d1d1905b9cd8254989d1653beb6756664839e) )
ROM_END

ROM_START( tqstp )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prom1p.bin", 0x000000, 0x080000, CRC(c13b499b) SHA1(e8389568e5bec6462e02b69949691b14e29d7d8e) )
	ROM_LOAD16_BYTE( "prom2.bin", 0x000001, 0x080000, CRC(a665e72e) SHA1(76440ae69f61eac1c6fe59dae295826a145bc940) )

	ROM_REGION16_LE( 0x200000, "user1", ROMREGION_ERASEFF )
	/* 0x000000 - 0x0fffff empty? */
	ROM_LOAD16_BYTE( "u16.bin",   0x100000, 0x080000, CRC(ae9b6829) SHA1(2c8ed5060d751bca0af54305164512fae8ff88e9) )
	ROM_LOAD16_BYTE( "u17.bin",   0x100001, 0x080000, CRC(7786340d) SHA1(96ded0af403fa3f0e7604f9ae0952036b3652665) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "025rs1-0.bin", 0x0000, 0x080000, CRC(c4dbff24) SHA1(2e4d1d1905b9cd8254989d1653beb6756664839e) )
ROM_END

ROM_START( snlad ) // probably incomplete
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8718.bin", 0x00000, 0x080000, CRC(599ca023) SHA1(fe6792ac97d18e2a04dbe8700d9f16b95be0f486) )
	ROM_LOAD16_BYTE( "8719.bin", 0x00001, 0x080000, CRC(155156dc) SHA1(7f43d52413c31c5f44907ebb9eb419ccb8047c68) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8720.bin", 0x0000, 0x080000, CRC(316d2230) SHA1(f2e330bcbc55dc0a47571f10d8c31e0e272ef8a9) )

	ROM_REGION16_LE( 0x200000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gr1.bin", 0x000000, 0x100000, NO_DUMP )
	ROM_LOAD16_BYTE( "gr2.bin", 0x000001, 0x100000, NO_DUMP )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "slswpsnd.bin", 0x0000, 0x080000, CRC(9a48b772) SHA1(d8fbaa60f09a1d31cf6c61c6dd02ad1bd7b7ffc9) )
ROM_END


ROM_START( buzzundr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prom1n.bin", 0x000000, 0x080000, CRC(2b47efd8) SHA1(bc96a5ea2511081f73a120e025249018c517c638) )
	ROM_LOAD16_BYTE( "prom2.bin",  0x000001, 0x080000, CRC(3a1c38a3) SHA1(cb85e1a9535ba646724db5e3dfbdb81384ada918) )

	ROM_REGION16_LE( 0x200000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gr1.bin", 0x000000, 0x100000, NO_DUMP )
	ROM_LOAD16_BYTE( "gr2.bin", 0x000001, 0x100000, NO_DUMP )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( monspdr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "msp10dsk.1", 0x000000, 0x080000, CRC(892aa085) SHA1(cfb8d4edbf22a88906b3b1fa52156be201d81b44) )
	ROM_LOAD16_BYTE( "msp10.2",    0x000001, 0x080000, CRC(3db5e13e) SHA1(79eb1f17a8e1b3220cd7c5f46212b8a2e1a112cb) )

	ROM_REGION16_LE( 0x200000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gr1.bin", 0x000000, 0x100000, NO_DUMP )
	ROM_LOAD16_BYTE( "gr2.bin", 0x000001, 0x100000, NO_DUMP )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

/************************************
 *
 *  Game driver(s)
 *
 *************************************/

// Touchscreen
GAMEL( 1995, cluedo,    0,        impact_video_touch, cluedo,   jpmimpct_video_state, empty_init, ROT0, "JPM", "Cluedo (prod. 2D)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_cluedo )
GAMEL( 1995, cluedod,   cluedo,   impact_video_touch, cluedo,   jpmimpct_video_state, empty_init, ROT0, "JPM", "Cluedo (prod. 2D) (Protocol)",MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_cluedo )
GAMEL( 1995, cluedo2c,  cluedo,   impact_video_touch, cluedo,   jpmimpct_video_state, empty_init, ROT0, "JPM", "Cluedo (prod. 2C)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_cluedo )
GAMEL( 1995, cluedo2,   cluedo,   impact_video_touch, cluedo,   jpmimpct_video_state, empty_init, ROT0, "JPM", "Cluedo (prod. 2)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_cluedo )
GAME( 1996, trivialp,  0,        impact_video_touch, trivialp, jpmimpct_video_state, empty_init, ROT0, "JPM", "Trivial Pursuit (New Edition) (prod. 1D)",  MACHINE_SUPPORTS_SAVE )
GAME( 1996, trivialpd, trivialp, impact_video_touch, trivialp, jpmimpct_video_state, empty_init, ROT0, "JPM", "Trivial Pursuit (New Edition) (prod. 1D) (Protocol)",MACHINE_SUPPORTS_SAVE )
GAME( 1996, trivialpo, trivialp, impact_video_touch, trivialp, jpmimpct_video_state, empty_init, ROT0, "JPM", "Trivial Pursuit",  MACHINE_SUPPORTS_SAVE )
GAME( 1997, scrabble,  0,        impact_video_touch, scrabble, jpmimpct_video_state, empty_init, ROT0, "JPM", "Scrabble (rev. F)",           MACHINE_SUPPORTS_SAVE )
GAME( 1997, scrabbled, scrabble, impact_video_touch, scrabble, jpmimpct_video_state, empty_init, ROT0, "JPM", "Scrabble (rev. F) (Protocol)",MACHINE_SUPPORTS_SAVE )

// Non Touchscreen
GAME( 1998, hngmnjpm,  0,        impact_video, hngmnjpm, jpmimpct_video_state, empty_init, ROT0, "JPM", "Hangman (JPM)",               MACHINE_SUPPORTS_SAVE )
GAME( 1998, hngmnjpmd, hngmnjpm, impact_video, hngmnjpm, jpmimpct_video_state, empty_init, ROT0, "JPM", "Hangman (JPM) (Protocol)",    MACHINE_SUPPORTS_SAVE )
GAME( 1999, coronatn,  0,        impact_video, coronatn, jpmimpct_video_state, empty_init, ROT0, "JPM", "Coronation Street Quiz Game", MACHINE_SUPPORTS_SAVE )
GAME( 1999, coronatnd, coronatn, impact_video, coronatn, jpmimpct_video_state, empty_init, ROT0, "JPM", "Coronation Street Quiz Game (Protocol)", MACHINE_SUPPORTS_SAVE )
// This acts a bit like a touchscreen game, is there an unmapped Dipswitch to enable touch support, or a different software revision?
GAME( 1996, tqst,      0,        impact_video, tqst,     jpmimpct_video_state, empty_init, ROT0, "Ace", "Treasure Quest"             , MACHINE_SUPPORTS_SAVE)
GAME( 1996, tqstp,     tqst,     impact_video, tqst,     jpmimpct_video_state, empty_init, ROT0, "Ace", "Treasure Quest (Protocol)"  , MACHINE_SUPPORTS_SAVE)

// sets below are incomplete, missing video ROMs etc.
GAME( 199?, snlad,     0,        impact_video, cluedo,   jpmimpct_video_state, empty_init, ROT0, "JPM", "Snake & Ladders"            , MACHINE_NOT_WORKING) // incomplete
GAME( 1997, buzzundr,  0,        impact_video, cluedo,   jpmimpct_video_state, empty_init, ROT0, "Ace", "Buzzundrum (Ace)", MACHINE_NOT_WORKING )
GAME( 1997, monspdr ,  0,        impact_video, cluedo,   jpmimpct_video_state, empty_init, ROT0, "Ace", "Money Spider (Ace)", MACHINE_NOT_WORKING )
