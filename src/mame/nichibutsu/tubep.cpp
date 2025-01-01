// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/***************************************************************************

Tube Panic
(c)1984 Nichibutsu

Driver by Jarek Burczynski.

It wouldn't be possible without help from following people:
Al Kossow helped with finding TTL chips' numbers and made PCB scans.
Tim made some nice screenshots.
Dox lent the Tube Panic PCB to me - I have drawn the schematics using the PCB,
this allowed me to emulate the background-drawing circuit.

----

CPU
84P0100B

tp-b 6->1          19.968MHz

                  tp-2 tp-1  2147 2147 2147 2147 2147 2147 2147 2147

               +------ daughter board ------+
               tp-p 5->8 6116  6116 tp-p 4->1
               +----------------------------+

   z80a              z80a                     z80a

                8910 8910 8910     6116  - tp-s 2->1

       16MHz

 VID
 84P101B

   6MHz                                        +------+ daughter board
                                  6116          tp-c 1
   MS2010-A                                     tp-c 2
                              tp-g 3            tp-c 3
   tp-g 6                                       tp-c 4
                              tp-g 4
   tp-g 5                                       tp-c 8
                                                tp-c 7
   6116                                         tp-c 6
                                       tp-g 1   tp-c 5
                                               +------+
     tp-g 7
                                       tp-g 2
     tp-g 8
                                             4164 4164 4164 4164
                                        4164 4164 4164 4164
  2114

----

Roller Jammer
Nichibutsu 1985

84P0501A

               SW1      SW2                      16A

Z80   6116                        TP-B.5         16B     6116
TP-S.1 TP-S.2 TP-S.3 TP-B.1  8212 TP-B.2 TP-B.3          TP-B.4


 TP-P.1 TP-P.2 TP-P.3 TP-P.4 6116 6116 TP-P.5 TP-P.6 TP-P.7 TP-P.8    6116


       8910 8910 8910         Z80A      Z80A

                               16MHz                       19.968MHz



                      --------------------------------

  6MHz
                                     6116
                                                     TP-C.8
  MS2010-A                     TP-G.4                TP-C.7
                                                     TP-C.6
  TP-G.8                        TP-G.3               TP-C.5

  TP-G.7                                 TP-G.2
                                                     TP-C.4
  6116                                   TP-G.1      TP-C.3
                                                     TP-C.2
                                                     TP-C.1
   TP-G.6

   TP-G.5                                         4164 4164 4164 4164
                                             4164 4164 4164 4164
 2114

----

*/


#include "emu.h"
#include "tubep.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "machine/74259.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "speaker.h"

#define LOG_IRQ (1U << 1)
//#define VERBOSE (LOG_IRQ)

#include "logmacro.h"

#define LOGIRQ(...) LOGMASKED(LOG_IRQ, __VA_ARGS__)


/*************************************
 *
 *  Main CPU on main PCB
 *
 *************************************/


void tubep_state::coin1_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void tubep_state::coin2_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


void tubep_state::tubep_main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xa7ff).ram();
	map(0xc000, 0xc7ff).w(FUNC(tubep_state::tubep_textram_w)).share("textram");  /* RAM on GFX PCB @B13 */
	map(0xe000, 0xe7ff).writeonly().share("share1");
	map(0xe800, 0xebff).writeonly().share("bgram");         /* row of 8 x 2147 RAMs on main PCB */
}


void tubep_state::main_cpu_irq_line_clear_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	LOGIRQ("CPU#0 VBLANK int clear at scanline=%3i\n", m_curr_scanline);
}


void tubep_state::tubep_main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).portr("DSW1");
	map(0x90, 0x90).portr("DSW2");
	map(0xa0, 0xa0).portr("DSW3");

	map(0xb0, 0xb0).portr("SYSTEM");
	map(0xc0, 0xc0).portr("P2");
	map(0xd0, 0xd0).portr("P1");

	map(0x80, 0x80).w(FUNC(tubep_state::main_cpu_irq_line_clear_w));
	map(0xb0, 0xb7).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xd0, 0xd0).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}



/*************************************
 *
 *  Slave CPU on main PCB
 *
 *************************************/

void tubep_state::second_cpu_irq_line_clear_w(uint8_t data)
{
	m_slave->set_input_line(0, CLEAR_LINE);
	LOGIRQ("CPU#1 VBLANK int clear at scanline=%3i\n", m_curr_scanline);
}


void tubep_state::tubep_second_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xa000).w(FUNC(tubep_state::background_a000_w));
	map(0xc000, 0xc000).w(FUNC(tubep_state::background_c000_w));
	map(0xe000, 0xe7ff).ram().share("share1");              /* 6116 #1 */
	map(0xe800, 0xebff).writeonly().share("bgram");         /* row of 8 x 2147 RAMs on main PCB */
	map(0xf000, 0xf3ff).writeonly().share("sprite_cram");   /* sprites color lookup table */
	map(0xf800, 0xffff).ram().share("share2");              /* program copies here part of shared ram ?? */
}


void tubep_state::tubep_second_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x7f, 0x7f).w(FUNC(tubep_state::second_cpu_irq_line_clear_w));
	map(0xb6, 0xb6).nopw(); // ?
}


uint8_t tubep_state::tubep_soundlatch_r()
{
	return (m_soundlatch->pending_r() << 7) | (m_soundlatch->read() & 0x7f);
}

uint8_t tubep_state::tubep_sound_irq_ack()
{
	m_soundcpu->set_input_line(0, CLEAR_LINE);
	return 0;
}


void tubep_state::tubep_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xd000, 0xd000).r(FUNC(tubep_state::tubep_sound_irq_ack));
	map(0xe000, 0xe7ff).ram();     /* 6116 #3 */
}


void tubep_state::tubep_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x02, 0x03).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x04, 0x05).w("ay3", FUNC(ay8910_device::address_data_w));
	map(0x06, 0x06).r(FUNC(tubep_state::tubep_soundlatch_r));
	map(0x07, 0x07).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
}


TIMER_CALLBACK_MEMBER(tubep_state::assert_sprite_int)
{
	m_mcu->set_input_line(0, ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(tubep_state::scanline_callback)
{
	int scanline = param;

	m_curr_scanline = scanline;//for debugging

	/* CPU #0 interrupt */
	/* activates at the start of VBLANK signal which happens at the beginning of scaline number 240 */
	if (scanline == 240)
	{
		LOGIRQ("VBLANK CPU#0\n");
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}


	/* CPU #1 interrupt */
	/* activates at the _end_ of VBLANK signal which happens at the beginning of scanline number 16 */
	if (scanline == 16)
	{
		LOGIRQ("/VBLANK CPU#1\n");
		m_slave->set_input_line(0, ASSERT_LINE);
	}


	/* CPU #3 MS2010-A NMI */
	/* activates at the _end_ of VBLANK signal which happens at the beginning of scanline number 16 */
	if (scanline == 16)
	{
		LOGIRQ("/nmi CPU#3\n");
		vblank_end(); /* switch buffered sprite RAM page */
		m_mcu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	/* CPU #3 MS2010-A NMI */
	/* deactivates at the start of VBLANK signal which happens at the beginning of scanline number 240*/
	if (scanline == 240)
	{
		LOGIRQ("CPU#3 nmi clear\n");
		m_mcu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}


	/* sound CPU interrupt */
	/* activates whenever line V6 from video part goes lo->hi that is when the scanline becomes 64 and 192 */
	if ((scanline == 64) || (scanline == 192))
	{
		m_soundcpu->set_input_line(0, ASSERT_LINE); /* sound cpu interrupt (music tempo) */
	}


	m_screen->update_partial(m_screen->vpos());

	//debug
	LOGIRQ("scanline=%3i scrgetvpos(0)=%3i\n",scanline,m_screen->vpos());

	scanline++;
	if (scanline >= 264)
		scanline = 0;

	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}



void tubep_state::machine_start()
{
	// Create interrupt timer
	m_interrupt_timer = timer_alloc(FUNC(tubep_state::scanline_callback), this);
}


void tubep_state::machine_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(0));
}



/*************************************
 *
 *  NSC8105 memory handlers
 *
 *************************************/

/* MS2010-A CPU (equivalent to NSC8105 with one new opcode: 0xec) on graphics PCB */
void tubep_state::nsc_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share("sprite_cram");
	map(0x0800, 0x0fff).ram().share("share2");
	map(0x2000, 0x2009).w(FUNC(tubep_state::sprite_control_w));
	map(0x200a, 0x200b).nopw(); /* not used by the games - perhaps designed for debugging */
	map(0xc000, 0xffff).rom();
}



/*************************************
 *
 *  Roller Jammer memory handlers
 *
 *************************************/

void rjammer_state::rjammer_main_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xa7ff).ram();                                 /* MB8416 SRAM on daughterboard on main PCB (there are two SRAMs, this is the one on the left) */
	map(0xc000, 0xc7ff).w(FUNC(rjammer_state::tubep_textram_w)).share("textram");/* RAM on GFX PCB @B13 */
	map(0xe000, 0xe7ff).ram().share("share1");                      /* MB8416 SRAM on daughterboard (the one on the right) */
}


void rjammer_state::rjammer_main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW2");   /* a bug in game code (during attract mode) */
	map(0x80, 0x80).portr("DSW2");
	map(0x90, 0x90).portr("DSW1");
	map(0xa0, 0xa0).portr("SYSTEM");
	map(0xb0, 0xb0).portr("P1");
	map(0xc0, 0xc0).portr("P2");

	map(0xd0, 0xd7).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xe0, 0xe0).w(FUNC(rjammer_state::main_cpu_irq_line_clear_w));    /* clear IRQ interrupt */
	map(0xf0, 0xf0).w(FUNC(rjammer_state::soundlatch_nmi_w));
}

void rjammer_state::soundlatch_nmi_w(uint8_t data)
{
	m_soundlatch->write(data);

	// LS138 decode output is connected directly to NMI
	m_soundcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void rjammer_state::rjammer_second_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xa7ff).ram();                         /* M5M5117P @21G */
	map(0xe000, 0xe7ff).ram().share("share1");              /* MB8416 on daughterboard (the one on the right) */
	map(0xe800, 0xefff).ram().share("bgram");/* M5M5117P @19B (background) */
	map(0xf800, 0xffff).ram().share("share2");
}


void rjammer_state::rjammer_second_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0xb0, 0xb0).w(FUNC(rjammer_state::background_page_w));
	map(0xd0, 0xd0).w(FUNC(rjammer_state::background_ls377_w));
}


TIMER_CALLBACK_MEMBER(rjammer_state::scanline_callback)
{
	int scanline = param;

	m_curr_scanline = scanline;//for debugging

	/* CPU #0 interrupt */
	/* activates at the start of VBLANK signal which happens at the beginning of scaline number 240 */
	if (scanline == 240)
	{
		LOGIRQ("VBLANK CPU#0\n");
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}


	/* CPU #1 interrupt */
	/* activates at the _end_ of VBLANK signal which happens at the beginning of scanline number 16 */
	if (scanline == 16)
	{
		LOGIRQ("/VBLANK CPU#1\n");
		m_slave->set_input_line(0, HOLD_LINE);
	}


	/* CPU #3 MS2010-A NMI */
	/* activates at the _end_ of VBLANK signal which happens at the beginning of scanline number 16 */
	if (scanline == 16)
	{
		LOGIRQ("/nmi CPU#3\n");
		vblank_end(); /* switch buffered sprite RAM page */
		m_mcu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	/* CPU #3 MS2010-A NMI */
	/* deactivates at the start of VBLANK signal which happens at the beginning of scanline number 240*/
	if (scanline == 240)
	{
		LOGIRQ("CPU#3 nmi clear\n");
		m_mcu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}


	/* sound CPU interrupt */
	/* activates whenever line V6 from video part goes lo->hi that is when the scanline becomes 64 and 192 */
	if ((scanline == 64) || (scanline == 192))
	{
		m_soundcpu->set_input_line(0, ASSERT_LINE); /* sound cpu interrupt (music tempo) */
	}


	m_screen->update_partial(m_screen->vpos());

	LOGIRQ("scanline=%3i scrgetvpos(0)=%3i\n", scanline, m_screen->vpos());

	scanline++;
	if (scanline >= 264)
		scanline = 0;

	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


void rjammer_state::machine_start()
{
	// Create interrupt timer
	m_interrupt_timer = timer_alloc(FUNC(rjammer_state::scanline_callback), this);

	// Set up save state
	save_item(NAME(m_msm5205_toggle));
}

void rjammer_state::machine_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(0));

	m_msm5205_toggle = false;
	m_adpcm_mux->select_w(0);

	voice_startstop_w(0);
	voice_frequency_select_w(0);
}



/*************************************
 *
 *  Sound CPU
 *
 *************************************/

void rjammer_state::voice_startstop_w(uint8_t data)
{
	/* bit 0 of data selects voice start/stop (reset pin on MSM5205)*/
	// 0 -stop; 1-start
	m_msm->reset_w((data & 1)^1);
}


void rjammer_state::voice_frequency_select_w(uint8_t data)
{
	// bit 0 of data selects voice frequency on MSM5205 (pin 1)
	// 0 -4 KHz; 1- 8KHz
	if (data & 1)
		m_msm->playmode_w(msm5205_device::S48_4B); // 8 KHz
	else
		m_msm->playmode_w(msm5205_device::S96_4B); // 4 KHz
}


void rjammer_state::adpcm_vck_w(int state)
{
	if (state)
	{
		m_msm5205_toggle = !m_msm5205_toggle;
		m_adpcm_mux->select_w(m_msm5205_toggle);

		if (m_msm5205_toggle)
			m_soundcpu->set_input_line(0, ASSERT_LINE);
	}
}


void rjammer_state::voice_input_w(uint8_t data)
{
	/* 8 bits of adpcm data for MSM5205 */
	/* need to buffer the data, and switch two nibbles on two following interrupts*/

	m_adpcm_mux->ba_w(data);

	/* NOTE: game resets interrupt line on ANY access to ANY I/O port.
	        I do it here because this port (0x80) is first one accessed
	        in the interrupt routine.
	*/
	m_soundcpu->set_input_line(0, CLEAR_LINE);
}


void rjammer_state::voice_intensity_control_w(uint8_t data)
{
	/* 4 LSB bits select the intensity (analog circuit that alters the output from MSM5205) */
	/* need to buffer the data */
}


void rjammer_state::rjammer_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xe7ff).ram();     /* M5M5117P (M58125P @2C on schematics) */
}


void rjammer_state::rjammer_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x10, 0x10).w(FUNC(rjammer_state::voice_startstop_w));
	map(0x18, 0x18).w(FUNC(rjammer_state::voice_frequency_select_w));
	map(0x80, 0x80).w(FUNC(rjammer_state::voice_input_w));
	map(0x90, 0x91).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x92, 0x93).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x94, 0x95).w("ay3", FUNC(ay8910_device::address_data_w));
	map(0x96, 0x96).w(FUNC(rjammer_state::voice_intensity_control_w));
}


void tubep_state::ay8910_portA_0_w(uint8_t data)
{
	//analog sound control
}

void tubep_state::ay8910_portB_0_w(uint8_t data)
{
	//analog sound control
}

void tubep_state::ay8910_portA_1_w(uint8_t data)
{
	//analog sound control
}

void tubep_state::ay8910_portB_1_w(uint8_t data)
{
	//analog sound control
}

void tubep_state::ay8910_portA_2_w(uint8_t data)
{
	//analog sound control
}

void tubep_state::ay8910_portB_2_w(uint8_t data)
{
	//analog sound control
}



/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( tubep )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x0c, "40000" )
	PORT_DIPSETTING(    0x08, "50000" )
	PORT_DIPSETTING(    0x04, "60000" )
	PORT_DIPSETTING(    0x00, "80000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, "Service mode" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", FUNC(generic_latch_8_device::pending_r))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tubepb )
	PORT_INCLUDE(tubep )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x0c, "10000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rjammer )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Bonus Time" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x02, "100" )
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPNAME( 0x0c, 0x0c, "Clear Men" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x0c, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Time" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void tubep_state::tubep(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16000000 / 4);    /* 4 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &tubep_state::tubep_main_map);
	m_maincpu->set_addrmap(AS_IO, &tubep_state::tubep_main_portmap);

	Z80(config, m_slave, 16000000 / 4);     /* 4 MHz */
	m_slave->set_addrmap(AS_PROGRAM, &tubep_state::tubep_second_map);
	m_slave->set_addrmap(AS_IO, &tubep_state::tubep_second_portmap);

	Z80(config, m_soundcpu, 19968000 / 8);  /* X2 19968000 Hz divided by LS669 (on Qc output) (signal RH0) */
	m_soundcpu->set_addrmap(AS_PROGRAM, &tubep_state::tubep_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &tubep_state::tubep_sound_portmap);

	NSC8105(config, m_mcu, 6000000);        /* 6 MHz Xtal - divided internally ??? */
	m_mcu->set_ram_enable(false);
	m_mcu->set_addrmap(AS_PROGRAM, &tubep_state::nsc_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(tubep_state::coin1_counter_w));
	mainlatch.q_out_cb<1>().set(FUNC(tubep_state::coin2_counter_w));
	mainlatch.q_out_cb<5>().set_nop(); //something...
	mainlatch.q_out_cb<5>().set(FUNC(tubep_state::screen_flip_w));
	mainlatch.q_out_cb<6>().set(FUNC(tubep_state::background_romselect_w));
	mainlatch.q_out_cb<7>().set(FUNC(tubep_state::colorproms_A4_line_w));

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->set_separate_acknowledge(true);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 264);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(tubep_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(tubep_state::palette_init), 32 + 256*64);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 19968000 / 8 / 2));
	ay1.port_a_write_callback().set(FUNC(tubep_state::ay8910_portA_0_w));
	ay1.port_b_write_callback().set(FUNC(tubep_state::ay8910_portB_0_w));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.10);

	ay8910_device &ay2(AY8910(config, "ay2", 19968000 / 8 / 2));
	ay2.port_a_write_callback().set(FUNC(tubep_state::ay8910_portA_1_w));
	ay2.port_b_write_callback().set(FUNC(tubep_state::ay8910_portB_1_w));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.10);

	ay8910_device &ay3(AY8910(config, "ay3", 19968000 / 8 / 2));
	ay3.port_a_write_callback().set(FUNC(tubep_state::ay8910_portA_2_w));
	ay3.port_b_write_callback().set(FUNC(tubep_state::ay8910_portB_2_w));
	ay3.add_route(ALL_OUTPUTS, "mono", 0.10);
}

void tubep_state::tubepb(machine_config &config)
{
	tubep(config);

	M6802(config.replace(), m_mcu, 6000000); /* ? MHz Xtal */
	m_mcu->set_ram_enable(false);
	m_mcu->set_addrmap(AS_PROGRAM, &tubep_state::nsc_map);

	//m_screen->screen_vblank().set_inputline(m_mcu, INPUT_LINE_NMI);
}

void rjammer_state::rjammer(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);   /* 4 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &rjammer_state::rjammer_main_map);
	m_maincpu->set_addrmap(AS_IO, &rjammer_state::rjammer_main_portmap);

	Z80(config, m_slave, 16_MHz_XTAL / 4);     /* 4 MHz */
	m_slave->set_addrmap(AS_PROGRAM, &rjammer_state::rjammer_second_map);
	m_slave->set_addrmap(AS_IO, &rjammer_state::rjammer_second_portmap);

	Z80(config, m_soundcpu, 19968000 / 8);  // X2 19968000 Hz (schematic says 20 MHz?) divided by LS669 (on Qc output) (signal RH0)
	m_soundcpu->set_addrmap(AS_PROGRAM, &rjammer_state::rjammer_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &rjammer_state::rjammer_sound_portmap);

	GENERIC_LATCH_8(config, m_soundlatch);

	NSC8105(config, m_mcu, 6_MHz_XTAL);    /* 6 MHz Xtal - divided internally ??? */
	m_mcu->set_ram_enable(false);
	m_mcu->set_addrmap(AS_PROGRAM, &rjammer_state::nsc_map);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 3A
	mainlatch.q_out_cb<0>().set(FUNC(rjammer_state::coin1_counter_w));
	mainlatch.q_out_cb<1>().set(FUNC(rjammer_state::coin2_counter_w));
	mainlatch.q_out_cb<5>().set(FUNC(rjammer_state::screen_flip_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(19968000 / 4, 320, 0, 256, 264, 16, 240);
	m_screen->set_screen_update(FUNC(rjammer_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(rjammer_state::palette_init), 64);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 19968000 / 8 / 2));
	ay1.port_a_write_callback().set(FUNC(rjammer_state::ay8910_portA_0_w));
	ay1.port_b_write_callback().set(FUNC(rjammer_state::ay8910_portB_0_w));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.075);

	ay8910_device &ay2(AY8910(config, "ay2", 19968000 / 8 / 2));
	ay2.port_a_write_callback().set(FUNC(rjammer_state::ay8910_portA_1_w));
	ay2.port_b_write_callback().set(FUNC(rjammer_state::ay8910_portB_1_w));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.075);

	ay8910_device &ay3(AY8910(config, "ay3", 19968000 / 8 / 2));
	ay3.port_a_write_callback().set(FUNC(rjammer_state::ay8910_portA_2_w));
	ay3.port_b_write_callback().set(FUNC(rjammer_state::ay8910_portB_2_w));
	ay3.add_route(ALL_OUTPUTS, "mono", 0.075);

	MSM5205(config, m_msm, 384_kHz_XTAL);
	m_msm->vck_callback().set(FUNC(rjammer_state::adpcm_vck_w));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  // 8 KHz (changes at run time)
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.325);

	LS157(config, m_adpcm_mux).out_callback().set(m_msm, FUNC(msm5205_device::data_w)); // 9G (fed by LS377 @ 10G)
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( tubep )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 (master) cpu code */
	ROM_LOAD( "tp-p.5", 0x0000, 0x2000, CRC(d5e0cc2f) SHA1(db9b062b14af52bb5458fe71996da295a69148ac) )
	ROM_LOAD( "tp-p.6", 0x2000, 0x2000, CRC(97b791a0) SHA1(20ef87b3d3bdfc8b983bcb8231252f81d98ad452) )
	ROM_LOAD( "tp-p.7", 0x4000, 0x2000, CRC(add9983e) SHA1(70a517451553a8c0e74a1995d9afddb779efc92c) )
	ROM_LOAD( "tp-p.8", 0x6000, 0x2000, CRC(b3793cb5) SHA1(0ed622b6bb97b9877acb6dc174edcd9977fa784e) )

	ROM_REGION( 0x10000, "slave", 0 ) /* Z80 (slave) cpu code */
	ROM_LOAD( "tp-p.1", 0x0000, 0x2000, CRC(b4020fcc) SHA1(437a037adedd596d295a0b6e400d64dee6c4488e) )
	ROM_LOAD( "tp-p.2", 0x2000, 0x2000, CRC(a69862d6) SHA1(7180cc26cd11d2daf453fcda8e6cc90851068bc4) )
	ROM_LOAD( "tp-p.3", 0x4000, 0x2000, CRC(f1d86e00) SHA1(5c26f20f49e09a736cede4f276f5bdf76f932400) )
	ROM_LOAD( "tp-p.4", 0x6000, 0x2000, CRC(0a1027bc) SHA1(2ebb53a1da53a9c3f0b99da084030c4d2b62a7b3) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 (sound) cpu code */
	ROM_LOAD( "tp-s.1", 0x0000, 0x2000, CRC(78964fcc) SHA1(a2c6119275d6291d82ac11dcffdaf2e8726e935a) )
	ROM_LOAD( "tp-s.2", 0x2000, 0x2000, CRC(61232e29) SHA1(a9ef0fefb7250392ef51173b69a69c903ff91ee8) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* 64k for the custom CPU */
	ROM_LOAD( "tp-g5.e1", 0xc000, 0x2000, CRC(9f375b27) SHA1(9666d1b20169d899176fbdf5954df41df06b4b82) )
	ROM_LOAD( "tp-g6.d1", 0xe000, 0x2000, CRC(3ea127b8) SHA1(a5f83ee0eb871da81eeaf839499baf14b986c69e) )

	ROM_REGION( 0xc000, "background", 0 ) /* background data */
	ROM_LOAD( "tp-b.1", 0x0000, 0x2000, CRC(fda355e0) SHA1(3270c65a4ee5d01388727f38691f7fe38f541031) )
	ROM_LOAD( "tp-b.2", 0x2000, 0x2000, CRC(cbe30149) SHA1(e66057286bea3026743f6de27a7e8dc8a709f8f7) )
	ROM_LOAD( "tp-b.3", 0x4000, 0x2000, CRC(f5d118e7) SHA1(a899bef3accef8995c457e8142a0001eed033fae) )
	ROM_LOAD( "tp-b.4", 0x6000, 0x2000, CRC(01952144) SHA1(d1074c79b51d3e2c152c9f3df6892027fe3a0e00) )
	ROM_LOAD( "tp-b.5", 0x8000, 0x2000, CRC(4dabea43) SHA1(72b9df9a3665baf34fb1f7301c5b9dd2619ed206) )
	ROM_LOAD( "tp-b.6", 0xa000, 0x2000, CRC(01952144) SHA1(d1074c79b51d3e2c152c9f3df6892027fe3a0e00) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "tp-c.1", 0x0000, 0x2000, CRC(ec002af2) SHA1(f9643c2ff01412d3da42b050bce4cf7f7d2e6f6a) )
	ROM_LOAD( "tp-c.2", 0x2000, 0x2000, CRC(c44f7128) SHA1(e05c00a7094b3fbf7ac6ed6ed38e1b227d462b27) )
	ROM_LOAD( "tp-c.3", 0x4000, 0x2000, CRC(4146b0c9) SHA1(cd3d620531660834530c64cdf1ef0659f9f6f437) )
	ROM_LOAD( "tp-c.4", 0x6000, 0x2000, CRC(552b58cf) SHA1(4ffd50bd55a9f88275c96a180dafe5e04b7ffb40) )
	ROM_LOAD( "tp-c.5", 0x8000, 0x2000, CRC(2bb481d7) SHA1(c07a11b938952be36c27fbfaefd0707a704acdf6) )
	ROM_LOAD( "tp-c.6", 0xa000, 0x2000, CRC(c07a4338) SHA1(3a40bacc2a98dc54612352a80f9b9ebf769de339) )
	ROM_LOAD( "tp-c.7", 0xc000, 0x2000, CRC(87b8700a) SHA1(4ddb032de9d6e124fb2661da77e6ba078360ec75) )
	ROM_LOAD( "tp-c.8", 0xe000, 0x2000, CRC(a6497a03) SHA1(68b42a5fd55b7c08f140dc1e3bb2eaa563545ef6) )
	ROM_LOAD( "tp-g4.d10", 0x10000, 0x1000, CRC(40a1fe00) SHA1(2e1e12efe8083bf96233016a7712e6e486d968e4) ) /* 2732 eprom is used, but the PCB is prepared for 2764 eproms */
	ROM_RELOAD(            0x11000, 0x1000 )
	ROM_LOAD( "tp-g1.e13", 0x12000, 0x1000, CRC(4a7407a2) SHA1(7ca4e03c637a6f1c338ca438a7ab9e4ba537fee0) )
	ROM_LOAD( "tp-g2.f13", 0x13000, 0x1000, CRC(f0b26c2e) SHA1(54057c619675bb384035547becd2019974bf23fa) )

	ROM_LOAD( "tp-g7.h2",  0x14000, 0x2000, CRC(105cb9e4) SHA1(b9d8ffe35c1f66aa401e5d8e415bf7c016ff53bb) )
	ROM_LOAD( "tp-g8.i2",  0x16000, 0x2000, CRC(27e5e6c1) SHA1(f3896d0006351d165e36bafa4340175077b3d6ba) )

	ROM_REGION( 0x1000, "text", 0 )
	ROM_LOAD( "tp-g3.c10", 0x0000, 0x1000, CRC(657a465d) SHA1(848217c3b736550586e8e9ba7a6e99e884094066) )   /* text characters */

	ROM_REGION( 0x40, "proms", 0 ) /* color proms */
	ROM_LOAD( "tp-2.c12", 0x0000, 0x0020, CRC(ac7e582f) SHA1(9d8f9eda7130b49b91d9c63bafa119b2a91eeda0) ) /* text and sprites palette */
	ROM_LOAD( "tp-1.c13", 0x0020, 0x0020, CRC(cd0910d6) SHA1(1e6dae16115d5a03bbaf76c695327a06eb6da602) ) /* color control prom */
ROM_END


ROM_START( tubepb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 (master) cpu code */
	ROM_LOAD( "a15.bin", 0x0000, 0x1000, CRC(806370a8) SHA1(c1915fae15bd766ffbd3c47d65ade51d36117eb8) )
	ROM_LOAD( "a16.bin", 0x1000, 0x1000, CRC(0917fb76) SHA1(1ce2680700d6ce28dfd202f238f1fc6e9c4a2758) )
	ROM_LOAD( "a13.bin", 0x2000, 0x1000, CRC(6e4bb47e) SHA1(092eba1a90f43eb298ee9e4dc0f13d5411a14b4a) )
	ROM_LOAD( "a14.bin", 0x3000, 0x1000, CRC(3df78441) SHA1(8e078d4f674da12034e3bb82878e781253f227f2) )
	ROM_LOAD( "a11.bin", 0x4000, 0x1000, CRC(2b557e49) SHA1(b7b7fdca23b62ebf320d50548d0a09aa904dfe5d) )
	ROM_LOAD( "a12.bin", 0x5000, 0x1000, CRC(d04a548e) SHA1(bdb4aacb636c9fc9c94af67384b455459c26272e) )
	ROM_LOAD( "a9.bin",  0x6000, 0x1000, CRC(a20de3d1) SHA1(84f2417597dbaecace0ce72a1684345fb212fc3a) )
	ROM_LOAD( "a10.bin", 0x7000, 0x1000, CRC(033ba70c) SHA1(de561c5db7cd493aee05e3513e48d52ed95bd510) )

	ROM_REGION( 0x10000, "slave", 0 ) /* Z80 (slave) cpu code */
	ROM_LOAD( "a1.bin",  0x0000, 0x1000, CRC(8a68523d) SHA1(268c659a2312e4d1a29e2064f55dfa07e57f6bca) )
	ROM_LOAD( "a2.bin",  0x1000, 0x1000, CRC(d15a8645) SHA1(9970377ff49ac525f2ef21c36ded8e7447ed700c) )
	ROM_LOAD( "a3.bin",  0x2000, 0x1000, CRC(7acf777c) SHA1(042b051de8fcc1295aca459f762619771438625e) )
	ROM_LOAD( "a4.bin",  0x3000, 0x1000, CRC(8f2bed23) SHA1(fa682172ed7a7a99161a2eaaf2c2e822bbab3f80) )
	ROM_LOAD( "a5.bin",  0x4000, 0x1000, CRC(8ba045f0) SHA1(91196b96592d98cd49474672ad4cc6b8039a9bcf) )
	ROM_LOAD( "a6.bin",  0x5000, 0x1000, CRC(8672ab0f) SHA1(784d74b71ab9613fb8177b39295bff083933a002) )
	ROM_LOAD( "a7.bin",  0x6000, 0x1000, CRC(417dd321) SHA1(aa0faa19eed1397e46a67e8793c5a27991ea9c1b) )
	ROM_LOAD( "a8.bin",  0x7000, 0x1000, CRC(d26ab4c0) SHA1(8d92386e75114494d65df4cfebdbacd09fddb48e) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 (sound) cpu code */
	ROM_LOAD( "15.bin",  0x0000, 0x2000, CRC(78964fcc) SHA1(a2c6119275d6291d82ac11dcffdaf2e8726e935a) )
	ROM_LOAD( "16.bin",  0x2000, 0x2000, CRC(61232e29) SHA1(a9ef0fefb7250392ef51173b69a69c903ff91ee8) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* 64k for the custom CPU */
	ROM_LOAD( "5.bin",   0xc000, 0x2000, CRC(9f375b27) SHA1(9666d1b20169d899176fbdf5954df41df06b4b82) )
	ROM_LOAD( "6.bin",   0xe000, 0x2000, CRC(46a273b5) SHA1(ff862c9337b3eeadee5a3d3f0837931a7a71393e) )

	ROM_REGION( 0xc000, "background", 0 ) /* background data */
	ROM_LOAD( "9.bin",   0x0000, 0x2000, CRC(fda355e0) SHA1(3270c65a4ee5d01388727f38691f7fe38f541031) )
	ROM_LOAD( "10.bin",  0x2000, 0x2000, CRC(0ccb23b0) SHA1(71660a3476ed299684e5662058b8c40153d4a168) )
	ROM_LOAD( "11.bin",  0x4000, 0x2000, CRC(f5d118e7) SHA1(a899bef3accef8995c457e8142a0001eed033fae) )
	ROM_LOAD( "12.bin",  0x6000, 0x2000, CRC(01952144) SHA1(d1074c79b51d3e2c152c9f3df6892027fe3a0e00) )
	ROM_LOAD( "13.bin",  0x8000, 0x2000, CRC(4dabea43) SHA1(72b9df9a3665baf34fb1f7301c5b9dd2619ed206) )
	ROM_LOAD( "14.bin",  0xa000, 0x2000, CRC(01952144) SHA1(d1074c79b51d3e2c152c9f3df6892027fe3a0e00) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "d1.bin",  0x0000, 0x1000, CRC(702348d7) SHA1(717e48d8c3529acb9a216b4e99df1599fb2e6b3b) )
	ROM_LOAD( "d2.bin",  0x1000, 0x1000, CRC(47601e8b) SHA1(4e56fe72644a000648199e92b306365c100cca30) )
	ROM_LOAD( "d3.bin",  0x2000, 0x1000, CRC(caad3ee2) SHA1(a99b8da36bf26a193d92fa807f168e60ed4bdce5) )
	ROM_LOAD( "d4.bin",  0x3000, 0x1000, CRC(ba5d8666) SHA1(83bf9a4a6c4cabed7312f1c09dcf317a69fdf26a) )
	ROM_LOAD( "d5.bin",  0x4000, 0x1000, CRC(cc709b7f) SHA1(6218c4aaac8a1d025d00923e27e026f54eda8987) )
	ROM_LOAD( "d6.bin",  0x5000, 0x1000, CRC(b9be626a) SHA1(94806056d9f16d455d8993795f220827d5b03183) )
	ROM_LOAD( "d7.bin",  0x6000, 0x1000, CRC(934e09d4) SHA1(395a0ca3bdf76ef8f715bfddc514b583be9ed499) )
	ROM_LOAD( "d8.bin",  0x7000, 0x1000, CRC(7e1970a0) SHA1(9e5fb5aa63149348f3da885844eb08d3471ec409) )
	ROM_LOAD( "d15.bin", 0x8000, 0x1000, CRC(f1f15364) SHA1(e5b783acb3fa3e1b67770db8a8a623ad38dc6a73) )
	ROM_LOAD( "d16.bin", 0x9000, 0x1000, CRC(05c52829) SHA1(21cc9334cf393bcc808e9b0999e84a15b94cedcb) )
	ROM_LOAD( "d13.bin", 0xa000, 0x1000, CRC(7c0b9e16) SHA1(2ec6337d06702a542c0dae9eb963d2b8e1736232) )
	ROM_LOAD( "d14.bin", 0xb000, 0x1000, CRC(81b31170) SHA1(cfaf768dd98116df730fb5ee0468ebc78dc42b2e) )
	ROM_LOAD( "d11.bin", 0xc000, 0x1000, CRC(9e07ef70) SHA1(e814c7ed72d94011718e45fba640f8dafc49aa31) )
	ROM_LOAD( "d12.bin", 0xd000, 0x1000, CRC(77e72279) SHA1(594e8018d64fd9862f54de0152723e610163e1b8) )
	ROM_LOAD( "d9.bin",  0xe000, 0x1000, CRC(7a0edea8) SHA1(ca98c35ef2007363ead3d24a8556e24df031308b) )
	ROM_LOAD( "d10.bin", 0xf000, 0x1000, CRC(0c1c2cb1) SHA1(addc9cea5247b25c114b88e3e9e1804305aa53c8) )
	ROM_LOAD( "4.bin",   0x10000, 0x1000, CRC(40a1fe00) SHA1(2e1e12efe8083bf96233016a7712e6e486d968e4) ) /* 2732 eprom is used, but the PCB is prepared for 2764 eproms */
	ROM_RELOAD(          0x11000, 0x1000 )
	ROM_LOAD( "1.bin",   0x12000, 0x1000, CRC(4a7407a2) SHA1(7ca4e03c637a6f1c338ca438a7ab9e4ba537fee0) )
	ROM_LOAD( "2.bin",   0x13000, 0x1000, CRC(f0b26c2e) SHA1(54057c619675bb384035547becd2019974bf23fa) )

	ROM_LOAD( "7.bin",   0x14000, 0x2000, CRC(105cb9e4) SHA1(b9d8ffe35c1f66aa401e5d8e415bf7c016ff53bb) )
	ROM_LOAD( "8.bin",   0x16000, 0x2000, CRC(27e5e6c1) SHA1(f3896d0006351d165e36bafa4340175077b3d6ba) )

	ROM_REGION( 0x1000, "text", 0 )
	ROM_LOAD( "3.bin",   0x0000, 0x1000, CRC(657a465d) SHA1(848217c3b736550586e8e9ba7a6e99e884094066) ) /* text characters */

	ROM_REGION( 0x40, "proms", 0 ) /* color proms */
	ROM_LOAD( "prom6331.b", 0x0000, 0x0020, CRC(ac7e582f) SHA1(9d8f9eda7130b49b91d9c63bafa119b2a91eeda0) ) /* text and sprites palette */
	ROM_LOAD( "prom6331.a", 0x0020, 0x0020, CRC(cd0910d6) SHA1(1e6dae16115d5a03bbaf76c695327a06eb6da602) ) /* color control prom */
ROM_END


ROM_START( rjammer )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 (master) cpu code */
	ROM_LOAD( "tp-p.1", 0x0000, 0x2000, CRC(93eeed67) SHA1(9ccfc49f42c6b451ff1c541d6487276f4bf9338e) )
	ROM_LOAD( "tp-p.2", 0x2000, 0x2000, CRC(ed2830c4) SHA1(078046e88604617342d29f0f4a0473fe6d484b19) )
	ROM_LOAD( "tp-p.3", 0x4000, 0x2000, CRC(e29f25e3) SHA1(21abf0e7c315fac15dd39355c16f9401c2cf4593) )
	ROM_LOAD( "tp-p.4", 0x8000, 0x2000, CRC(6ed71fbc) SHA1(821506943b980077a9b4f309db095be7e952b13d) )
	ROM_CONTINUE(       0x6000, 0x2000 )

	ROM_REGION( 0x10000, "slave", 0 ) /* Z80 (slave) cpu code */
	ROM_LOAD( "tp-p.8", 0x0000, 0x2000, CRC(388b9c66) SHA1(6d3e614736a7f06c26191699e8a8a13b239b259f) )
	ROM_LOAD( "tp-p.7", 0x2000, 0x2000, CRC(595030bb) SHA1(00dd0b3af965a2768c71297ba2a358050bdb8ef7) )
	ROM_LOAD( "tp-p.6", 0x4000, 0x2000, CRC(b5aa0f89) SHA1(d7e8b7e76fe6e5ef1d9bcad8469d56b81c9509ac) )
	ROM_LOAD( "tp-p.5", 0x6000, 0x2000, CRC(56eae9ac) SHA1(e5cd75df0c38021b81de2abf049b12c10db4f3cb) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 (sound) cpu code */
	ROM_LOAD( "tp-b1.6d", 0x0000, 0x2000, CRC(b1c2525c) SHA1(7a184142e83982e33bc41cabae6fe804cec78748) )
	ROM_LOAD( "tp-s3.4d", 0x2000, 0x2000, CRC(90c9d0b9) SHA1(8657ee93d7b67ba89848bf94e03b5c3bcace92c4) )
	ROM_LOAD( "tp-s2.2d", 0x4000, 0x2000, CRC(444b6a1d) SHA1(1252b14d473d764a5326401aac782a1fa3419784) )
	ROM_LOAD( "tp-s1.1d", 0x6000, 0x2000, CRC(391097cd) SHA1(d4b48a3f26044b131e65f74479bf1671ad677eb4) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* 64k for the custom CPU */
	ROM_LOAD( "tp-g7.e1",  0xc000, 0x2000, CRC(9f375b27) SHA1(9666d1b20169d899176fbdf5954df41df06b4b82) )
	ROM_LOAD( "tp-g8.d1",  0xe000, 0x2000, CRC(2e619fec) SHA1(d3d5fa708ca0097abf12d59ae41cb852278fe45d) )

	ROM_REGION( 0x7000, "background", 0 ) /* background data */
	ROM_LOAD( "tp-b3.13d", 0x0000, 0x1000, CRC(b80ef399) SHA1(75fa17e1bb39363e194737a32db2d92e0cae5e79) )
	ROM_LOAD( "tp-b5.11b", 0x1000, 0x2000, CRC(0f260bfe) SHA1(975b7837f6c3c9d743903910fbdc3111c18a5955) )
	ROM_LOAD( "tp-b2.11d", 0x3000, 0x2000, CRC(8cd2c917) SHA1(472aceaf4a1050b2513d56b2703e556ac1e2a61a) )
	ROM_LOAD( "tp-b4.19c", 0x5000, 0x2000, CRC(6600f306) SHA1(2e25790839a465f5f8729964cfe27a587eb663f5) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "tp-c.8", 0x0000, 0x2000, CRC(9f31ecb5) SHA1(c4b979c7da096648d0c58b2c8a205e1622ee28e9) )
	ROM_LOAD( "tp-c.7", 0x2000, 0x2000, CRC(cbf093f1) SHA1(128e01249165a87304eaf8003a9adf6f38d35d5e) )
	ROM_LOAD( "tp-c.6", 0x4000, 0x2000, CRC(11f9752b) SHA1(11dcbbfe4e673e379afd67874b64b48cdafa00f5) )
	ROM_LOAD( "tp-c.5", 0x6000, 0x2000, CRC(513f8777) SHA1(ebdbf164c20bbb8a52e32beb148917023e30c72b) )
	ROM_LOAD( "tp-c.1", 0x8000, 0x2000, CRC(ef573117) SHA1(e2cf1e7b7c4f64bf3f9723eca2061a6cf8d2eddb) )
	ROM_LOAD( "tp-c.2", 0xa000, 0x2000, CRC(1d29f1e6) SHA1(278556f89c8aed9b16bdbef7ba2847736473e63d) )
	ROM_LOAD( "tp-c.3", 0xc000, 0x2000, CRC(086511a7) SHA1(92691aec024312e7c8593a35303df15cb6e9c9f2) )
	ROM_LOAD( "tp-c.4", 0xe000, 0x2000, CRC(49f372ea) SHA1(16b500157b95437ea27a097010e798f3e82b2b6a) )
	ROM_LOAD( "tp-g3.d10", 0x10000, 0x1000, CRC(1f2abec5) SHA1(3e7d2849d517cc4941ac86df507743782ed9c694) )  /* 2732 eprom is used, but the PCB is prepared for 2764 eproms */
	ROM_RELOAD(            0x11000, 0x1000 )
	ROM_LOAD( "tp-g2.e13", 0x12000, 0x1000, CRC(4a7407a2) SHA1(7ca4e03c637a6f1c338ca438a7ab9e4ba537fee0) )
	ROM_LOAD( "tp-g1.f13", 0x13000, 0x1000, CRC(f0b26c2e) SHA1(54057c619675bb384035547becd2019974bf23fa) )
	ROM_LOAD( "tp-g6.h2",  0x14000, 0x2000, CRC(105cb9e4) SHA1(b9d8ffe35c1f66aa401e5d8e415bf7c016ff53bb) )
	ROM_LOAD( "tp-g5.i2",  0x16000, 0x2000, CRC(27e5e6c1) SHA1(f3896d0006351d165e36bafa4340175077b3d6ba) )

	ROM_REGION( 0x1000, "text", 0 )
	ROM_LOAD( "tp-g4.c10", 0x0000, 0x1000, CRC(99e72549) SHA1(2509265c2d84ac6144aecd77f1b3f0d16bdcb572) )   /* text characters */

	ROM_REGION( 0x40, "proms", 0 ) /* color proms */
	ROM_LOAD( "16b", 0x0000, 0x0020, CRC(9a12873a) SHA1(70f088b6eb5431e2ac6afcf15531eeb02a169442) ) /* text palette, sprites palette */
	ROM_LOAD( "16a", 0x0020, 0x0020, CRC(90222a71) SHA1(c3fd49c8075b0af451f6d2a142a4c4a2e397ac08) ) /* background palette */
ROM_END



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

//    year  rom      parent  machine  inp      state          init
GAME( 1984, tubep,   0,      tubep,   tubep,   tubep_state,   empty_init, ROT0, "Nichibutsu / Fujitek", "Tube Panic",           MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, tubepb,  tubep,  tubepb,  tubepb,  tubep_state,   empty_init, ROT0, "bootleg",              "Tube Panic (bootleg)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

GAME( 1984, rjammer, 0,      rjammer, rjammer, rjammer_state, empty_init, ROT0, "Nichibutsu / Alice",   "Roller Jammer",        MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
