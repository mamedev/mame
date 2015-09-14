// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Tora Tora (c) 1980 Game Plan

driver by Nicola Salmoria

TODO:
- The game doesn't seem to work right. It also reads some unmapped memory
  addresses, are the two things related? Missing ROMs? There's an empty
  socket for U3 on the board, which should map at 5000-57ff, however the
  game reads mostly from 4800-4fff, which would be U6 according to the
  schematics.

- The manual mentions dip switch settings and the schematics show the switches,
  the game reads them but ignores them, forcing 1C/1C and 3 lives.
  Maybe the dump is from a proto?

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/sn76477.h"


class toratora_state : public driver_device
{
public:
	toratora_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"),
		m_pia_u1(*this, "pia_u1"),
		m_pia_u2(*this, "pia_u2"),
		m_pia_u3(*this, "pia_u3") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	int        m_timer;
	UINT8      m_last;
	UINT8      m_clear_tv;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<sn76477_device> m_sn1;
	required_device<sn76477_device> m_sn2;
	required_device<pia6821_device> m_pia_u1;
	required_device<pia6821_device> m_pia_u2;
	required_device<pia6821_device> m_pia_u3;
	DECLARE_WRITE8_MEMBER(clear_tv_w);
	DECLARE_READ8_MEMBER(timer_r);
	DECLARE_WRITE8_MEMBER(clear_timer_w);
	DECLARE_WRITE_LINE_MEMBER(cb2_u3_w);
	DECLARE_WRITE8_MEMBER(port_b_u1_w);
	DECLARE_WRITE_LINE_MEMBER(main_cpu_irq);
	DECLARE_WRITE8_MEMBER(sn1_port_a_u2_u3_w);
	DECLARE_WRITE8_MEMBER(sn1_port_b_u2_u3_w);
	DECLARE_WRITE_LINE_MEMBER(sn1_ca2_u2_u3_w);
	DECLARE_WRITE8_MEMBER(sn2_port_a_u2_u3_w);
	DECLARE_WRITE8_MEMBER(sn2_port_b_u2_u3_w);
	DECLARE_WRITE_LINE_MEMBER(sn2_ca2_u2_u3_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_toratora(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(toratora_timer);
};



/*************************************
 *
 *  Input handling
 *
 *************************************/

WRITE_LINE_MEMBER(toratora_state::cb2_u3_w)
{
	logerror("DIP tristate %sactive\n",(state & 1) ? "in" : "");
}


/*************************************
 *
 *  Video hardware
 *
 *************************************/

UINT32 toratora_state::screen_update_toratora(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;

	for (offs = 0; offs < m_videoram.bytes(); offs++)
	{
		int i;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;
		UINT8 data = m_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? rgb_t::white : rgb_t::black;
			bitmap.pix32(y, x) = pen;

			data = data << 1;
			x = x + 1;
		}

		/* the video system clears as it writes out the pixels */
		if (m_clear_tv)
			m_videoram[offs] = 0;
	}

	m_clear_tv = 0;

	return 0;
}


WRITE8_MEMBER(toratora_state::clear_tv_w)
{
	m_clear_tv = 1;
}


/*************************************
 *
 *  Coin counter
 *
 *************************************/

WRITE8_MEMBER(toratora_state::port_b_u1_w)
{
	if (m_pia_u1->port_b_z_mask() & 0x20)
		coin_counter_w(machine(), 0, 1);
	else
		coin_counter_w(machine(), 0, data & 0x20);
}


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

WRITE_LINE_MEMBER(toratora_state::main_cpu_irq)
{
	int combined_state = m_pia_u1->irq_a_state() | m_pia_u1->irq_b_state();

	logerror("GEN IRQ: %x\n", combined_state);
	m_maincpu->set_input_line(0, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


INTERRUPT_GEN_MEMBER(toratora_state::toratora_timer)
{
	m_timer++;  /* timer counting at 16 Hz */

	/* also, when the timer overflows (16 seconds) watchdog would kick in */
	if (m_timer & 0x100)
		popmessage("watchdog!");

	if (m_last != (ioport("INPUT")->read() & 0x0f))
	{
		m_last = ioport("INPUT")->read() & 0x0f;
		generic_pulse_irq_line(device.execute(), 0, 1);
	}
	m_pia_u1->porta_w(ioport("INPUT")->read() & 0x0f);
	m_pia_u1->ca1_w(ioport("INPUT")->read() & 0x10);
	m_pia_u1->ca2_w(ioport("INPUT")->read() & 0x20);
}

READ8_MEMBER(toratora_state::timer_r)
{
	return m_timer;
}

WRITE8_MEMBER(toratora_state::clear_timer_w)
{
	m_timer = 0;
}


/*************************************
 *
 *  Audio hardware
 *
 *************************************/


WRITE8_MEMBER(toratora_state::sn1_port_a_u2_u3_w)
{
	m_sn1->vco_voltage_w(2.35 * (data & 0x7f) / 128.0);
	m_sn1->enable_w((data >> 7) & 0x01);
}


WRITE8_MEMBER(toratora_state::sn1_port_b_u2_u3_w)
{
	static const double resistances[] =
	{
		0,  /* N/C */
		RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200) + RES_K(360) + RES_K(750) + RES_M(1.5),
		RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200) + RES_K(360) + RES_K(750),
		RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200) + RES_K(360),
		RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200),
		RES_K(47) + RES_K(47) + RES_K(91),
		RES_K(47) + RES_K(47) + RES_K(91),
		RES_K(47)
	};

	m_sn1->mixer_a_w      ((data >> 0) & 0x01);
	m_sn1->mixer_b_w      ((data >> 1) & 0x01);
	m_sn1->mixer_c_w      ((data >> 2) & 0x01);
	m_sn1->envelope_1_w   ((data >> 3) & 0x01);
	m_sn1->envelope_2_w   ((data >> 4) & 0x01);
	m_sn1->amplitude_res_w(resistances[(data >> 5)] * 2);  /* the *2 shouldn't be neccassary, but... */
}


WRITE_LINE_MEMBER(toratora_state::sn1_ca2_u2_u3_w)
{
	m_sn1->vco_w(state);
}

WRITE8_MEMBER(toratora_state::sn2_port_a_u2_u3_w)
{
	m_sn2->vco_voltage_w(2.35 * (data & 0x7f) / 128.0);
	m_sn2->enable_w((data >> 7) & 0x01);
}


WRITE8_MEMBER(toratora_state::sn2_port_b_u2_u3_w)
{
	static const double resistances[] =
	{
		0,  /* N/C */
		RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200) + RES_K(360) + RES_K(750) + RES_M(1.5),
		RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200) + RES_K(360) + RES_K(750),
		RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200) + RES_K(360),
		RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200),
		RES_K(47) + RES_K(47) + RES_K(91),
		RES_K(47) + RES_K(47) + RES_K(91),
		RES_K(47)
	};

	m_sn2->mixer_a_w      ((data >> 0) & 0x01);
	m_sn2->mixer_b_w      ((data >> 1) & 0x01);
	m_sn2->mixer_c_w      ((data >> 2) & 0x01);
	m_sn2->envelope_1_w   ((data >> 3) & 0x01);
	m_sn2->envelope_2_w   ((data >> 4) & 0x01);
	m_sn2->amplitude_res_w(resistances[(data >> 5)] * 2);  /* the *2 shouldn't be neccassary, but... */
}


WRITE_LINE_MEMBER(toratora_state::sn2_ca2_u2_u3_w)
{
	m_sn2->vco_w(state);
}


/*************************************
 *
 *  Memory handlers
 *
 *  No mirrors, all addresses are
 *  fully decoded by the hardware!
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, toratora_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x7fff) AM_ROM  /* not fully populated */
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xa000, 0xf047) AM_NOP
	AM_RANGE(0xf048, 0xf049) AM_NOP
	AM_RANGE(0xf04a, 0xf04a) AM_WRITE(clear_tv_w)   /* the read is mark *LEDEN, but not used */
	AM_RANGE(0xf04b, 0xf04b) AM_READWRITE(timer_r, clear_timer_w)
	AM_RANGE(0xa04c, 0xf09f) AM_NOP
	AM_RANGE(0xf0a0, 0xf0a3) AM_DEVREADWRITE("pia_u1", pia6821_device, read, write)
	AM_RANGE(0xf0a4, 0xf0a7) AM_DEVREADWRITE("pia_u3", pia6821_device, read, write)
	AM_RANGE(0xf0a8, 0xf0ab) AM_DEVREADWRITE("pia_u2", pia6821_device, read, write)
	AM_RANGE(0xf0ac, 0xf7ff) AM_NOP
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( toratora )
	PORT_START("INPUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "0" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x02, "0" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void toratora_state::machine_start()
{
	save_item(NAME(m_timer));
	save_item(NAME(m_last));
	save_item(NAME(m_clear_tv));
}

void toratora_state::machine_reset()
{
	m_timer = 0xff;
	m_last = 0;
	m_clear_tv = 0;
}

static MACHINE_CONFIG_START( toratora, toratora_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800,500000)   /* ?????? game speed is entirely controlled by this */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(toratora_state, toratora_timer, 16)    /* timer counting at 16 Hz */

	MCFG_DEVICE_ADD("pia_u1", PIA6821, 0)
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(toratora_state,port_b_u1_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(toratora_state,main_cpu_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(toratora_state,main_cpu_irq))

	MCFG_DEVICE_ADD("pia_u2", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(toratora_state, sn1_port_a_u2_u3_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(toratora_state, sn1_port_b_u2_u3_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(toratora_state, sn1_ca2_u2_u3_w))

	MCFG_DEVICE_ADD("pia_u3", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(IOPORT("DSW"))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(toratora_state,sn2_port_a_u2_u3_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(toratora_state,sn2_port_b_u2_u3_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(toratora_state,sn2_ca2_u2_u3_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(toratora_state,cb2_u3_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0,256-1,8,248-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(toratora_state, screen_update_toratora)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), RES_M(1.2) /* RES_K(120) */, CAP_P(470)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_K(680))                   // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(0.2),  RES_K(3.3))  // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(0)                              // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(50))                 // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(51))    // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                      // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(0.1), RES_K(470))      // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(0.1), RES_M(1))    // oneshot caps + res
	MCFG_SN76477_VCO_MODE(0)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(0, 0)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                               // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn2", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), RES_M(1.2) /* RES_K(120) */, CAP_P(470)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_K(680))                   // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(0.2),  RES_K(3.3))  // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(0)                              // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(50))                 // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(51))    // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                      // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(0.1), RES_K(470))      // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(0.1), RES_M(1))    // oneshot caps + res
	MCFG_SN76477_VCO_MODE(0)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(0, 0)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                               // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( toratora )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tora.u1",  0x1000, 0x0800, CRC(413c743a) SHA1(a887dfaaee557327a1699bb424488b934dab8612) )
	ROM_LOAD( "tora.u10", 0x1800, 0x0800, CRC(dc771b1c) SHA1(1bd81decb4d0a854878227c52d45ac0eea0602ec) )
	ROM_LOAD( "tora.u2",  0x2000, 0x0800, CRC(c574c664) SHA1(9f41a53ca51d04e5bec7525fe83c5f4bdfcf128d) )
	ROM_LOAD( "tora.u9",  0x2800, 0x0800, CRC(b67aa11f) SHA1(da9e77255640a4b32eed2be89b686b98a248bd72) )
	ROM_LOAD( "tora.u11", 0xf800, 0x0800, CRC(55135d6f) SHA1(c48f180a9d6e894aafe87b2daf74e9a082f4600e) )
ROM_END

/* Tora Tora? Game Plan?
Etched in copper on top of board		20-00047C
						20-10051A

Etched in copper on back of daughter board	20-00048C
						20-10052A

ROM text showed		TORA TOR*		* was A with bit 7 set
			1980 GAME PLAN,INC

and  war stuff (PLANE, BOMB, SQUAD, etc)

.u2	2716	handwritten sticker	U-2
.u9	2716	handwritten sticker	U-9
.u10	2716	handwritten sticker	U-10
.u11	2716	handwritten sticker	U-11

open 24 pin socket @ U1 and U3
open 40 pin socket @ U42

Main board
crystal with 5 185 on the top
5280	x8
socketed	ds8833	x2
socketed	ds8t28	x2

Daughter board
open 40 pin socket @ U3 @ U2
76477	X2 */

ROM_START( toratorab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1027.u1",  0x1000, 0x0800, BAD_DUMP CRC(413c743a) SHA1(a887dfaaee557327a1699bb424488b934dab8612) ) /* rom u1 is missing in this set, using the toratora one */
	ROM_LOAD( "1027.u10", 0x1800, 0x0800, CRC(6a906292) SHA1(4ceff91b7dcd398e57cd19a91d2199c09cb37c39) )
	ROM_LOAD( "1027.u2",  0x2000, 0x0800, CRC(c1331648) SHA1(379101c6c1b8dab3e043ece01579cc96f6bb18a9) )
	ROM_LOAD( "1027.u9",  0x2800, 0x0800, CRC(59b021b5) SHA1(ea5a0c1f58c0e08231969ad161b79af6e1ae4431) )
	ROM_LOAD( "1027.u11", 0xf800, 0x0800, CRC(336f6659) SHA1(ea1151db54b68316908874da6983d6de5c94c29e) )
ROM_END


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1980, toratora, 0,        toratora, toratora, driver_device, 0, ROT90, "Game Plan", "Tora Tora (prototype?)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, toratorab,toratora, toratora, toratora, driver_device, 0, ROT90, "Game Plan", "Tora Tora (prototype?, set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
