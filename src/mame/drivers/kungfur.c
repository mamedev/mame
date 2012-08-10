/***************************************************************************

KUNG-FU ROUSHI
(c)1987 NAMCO

6809         (4MHz?)
RAM 2016x1
MSM5205 x 2  (2 SPEAKERS)
8255    x 2
custom chips : none

KR1.BIN : PROGRAM
KR2-KR6 : VOICE

50pin cable to 7seg&lamp control board.

Information:
http://www.wshin.com/games/review/ka/kung-fu-roushi.htm
http://www.youtube.com/watch?v=ssEfw-RbSjs
http://www.youtube.com/watch?v=1YacVjpUG8g

---------------------------------------------------------------------------

Game Panel:

tokuten                                       honjitsu yuuryoukiroku
(score)                                       (today's best scores)
XX                                            #1 XX points
                                              #2 XX points
dankai                                        #3 XX points
(level)
1 2 3 4 5

hannoujikan        heikinhannoujikan          shuuryou
(reaction time)    (average reaction time)    (finished, complete)
X.XX seconds       X.XX seconds


Control Panel:

washi no kakegoe no toori ni botan wo osunojazoi!
(hit the buttons as I call them out)

over 8, or 10 points --> level 1
"   16, or 20 "      --> level 2
"   28, or 35 "      --> level 3
"   42, or 52 "      --> level 4
"   52, or 73(?) "   --> level 5
menkyokaiden(full certification)

The 4 buttons are labeled:
mae(forward), migi(right), ushiro(back), hidari(left)


***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/msm5205.h"
#include "machine/i8255.h"

#include "kungfur.lh"


class kungfur_state : public driver_device
{
public:
	kungfur_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_latch[3];
	UINT8 m_control;

	UINT32 m_adpcm_pos[2];
	UINT8 m_adpcm_data[2];
	UINT8 m_adpcm_sel[2];
	DECLARE_WRITE8_MEMBER(kungfur_output_w);
	DECLARE_WRITE8_MEMBER(kungfur_latch1_w);
	DECLARE_WRITE8_MEMBER(kungfur_latch2_w);
	DECLARE_WRITE8_MEMBER(kungfur_latch3_w);
	DECLARE_WRITE8_MEMBER(kungfur_control_w);
	DECLARE_WRITE8_MEMBER(kungfur_adpcm1_w);
	DECLARE_WRITE8_MEMBER(kungfur_adpcm2_w);
};


static INTERRUPT_GEN( kungfur_irq )
{
	kungfur_state *state = device->machine().driver_data<kungfur_state>();
	if (state->m_control & 0x10)
		device_set_input_line(device, M6809_IRQ_LINE, ASSERT_LINE);
}


/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(kungfur_state::kungfur_output_w)
{

	// d0-d2: output led7seg
	static const int lut_digits[24] =
	{
		0, 2, 4, 6, 9, 12,14,0,
		0, 1, 3, 5, 8, 11,13,0,
		0, 0, 0, 0, 7, 10,0, 0
	};
	for (int i = 0; i < 3; i++)
	{
		int offs = i << 3 | (data & 7);
		if (lut_digits[offs])
			output_set_digit_value(lut_digits[offs] - 1, m_latch[i]);
	}

	// 2.6 goes to level lamps
	if ((data & 7) == 6)
	{
		for (int i = 0; i < 5; i++)
			output_set_lamp_value(i, m_latch[2] >> i & 1);
	}

	// d7: game-over lamp, d3-d4: marquee lamps
	output_set_lamp_value(5, data >> 7 & 1);
	output_set_lamp_value(6, data >> 3 & 1);
	output_set_lamp_value(7, data >> 4 & 1);

	// d5: N/C?
	// d6: coincounter
	coin_counter_w(machine(), 0, data & 0x40);
}


// lamp output latches
WRITE8_MEMBER(kungfur_state::kungfur_latch1_w)
{
	m_latch[0] = data;
}

WRITE8_MEMBER(kungfur_state::kungfur_latch2_w)
{
	m_latch[1] = data;
}

WRITE8_MEMBER(kungfur_state::kungfur_latch3_w)
{
	m_latch[2] = data;
}


WRITE8_MEMBER(kungfur_state::kungfur_control_w)
{

	// d0-d3: N/C
	// d4: irq ack
	if (~data & 0x10)
		cputag_set_input_line(machine(), "maincpu", M6809_IRQ_LINE, CLEAR_LINE);

	// d5: ?
	// d6-d7: sound trigger (edge)
	if ((data ^ m_control) & 0x40)
	{
		msm5205_reset_w(machine().device("adpcm1"), data >> 6 & 1);
		m_adpcm_pos[0] = m_adpcm_data[0] * 0x400;
		m_adpcm_sel[0] = 0;
	}
	if ((data ^ m_control) & 0x80)
	{
		msm5205_reset_w(machine().device("adpcm2"), data >> 7 & 1);
		m_adpcm_pos[1] = m_adpcm_data[1] * 0x400;
		m_adpcm_sel[1] = 0;
	}

	m_control = data;
}

// adpcm latches
WRITE8_MEMBER(kungfur_state::kungfur_adpcm1_w)
{
	m_adpcm_data[0] = data;
}

WRITE8_MEMBER(kungfur_state::kungfur_adpcm2_w)
{
	m_adpcm_data[1] = data;
}

// adpcm callbacks
static void kfr_adpcm1_int(device_t *device)
{
	kungfur_state *state = device->machine().driver_data<kungfur_state>();
	UINT8 *ROM = state->memregion("adpcm1")->base();
	UINT8 data = ROM[state->m_adpcm_pos[0] & 0x1ffff];

	msm5205_data_w(device, state->m_adpcm_sel[0] ? data & 0xf : data >> 4 & 0xf);
	state->m_adpcm_pos[0] += state->m_adpcm_sel[0];
	state->m_adpcm_sel[0] ^= 1;
}

static void kfr_adpcm2_int(device_t *device)
{
	kungfur_state *state = device->machine().driver_data<kungfur_state>();
	UINT8 *ROM = state->memregion("adpcm2")->base();
	UINT8 data = ROM[state->m_adpcm_pos[1] & 0x3ffff];

	msm5205_data_w(device, state->m_adpcm_sel[1] ? data & 0xf : data >> 4 & 0xf);
	state->m_adpcm_pos[1] += state->m_adpcm_sel[1];
	state->m_adpcm_sel[1] ^= 1;
}


static ADDRESS_MAP_START( kungfur_map, AS_PROGRAM, 8, kungfur_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(kungfur_adpcm1_w)
	AM_RANGE(0x4004, 0x4004) AM_WRITE(kungfur_adpcm2_w)
	AM_RANGE(0x4008, 0x400b) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x400c, 0x400f) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( kungfur )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Migi (Right)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_16WAY PORT_NAME("Hidari (Left)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_16WAY PORT_NAME("Ushiro (Back)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_16WAY PORT_NAME("Mae (Front)")

	PORT_START("IN1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING( 0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_2C ) )
//  PORT_DIPSETTING( 0x01, DEF_STR( 1C_2C ) ) // dupe
	PORT_DIPSETTING( 0x02, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING( 0x00, DEF_STR( 1C_3C ) ) // dupe
//  PORT_DIPSETTING( 0x04, DEF_STR( 0C_0C ) ) // invalid
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static I8255A_INTERFACE( ppi8255_0_intf )
{
	// $4008 - always $83 (PPI mode 0, ports B & lower C as input)
	DEVCB_NULL,							/* Port A read */
	DEVCB_DRIVER_MEMBER(kungfur_state,kungfur_output_w),	/* Port A write */
	DEVCB_INPUT_PORT("IN0"),			/* Port B read */
	DEVCB_NULL,							/* Port B write */
	DEVCB_INPUT_PORT("IN1"),			/* Port C read */
	DEVCB_DRIVER_MEMBER(kungfur_state,kungfur_control_w)	/* Port C write */
};

static I8255A_INTERFACE( ppi8255_1_intf )
{
	// $400c - always $80 (PPI mode 0, all ports as output)
	DEVCB_NULL,							/* Port A read */
	DEVCB_DRIVER_MEMBER(kungfur_state,kungfur_latch1_w),	/* Port A write */
	DEVCB_NULL,							/* Port B read */
	DEVCB_DRIVER_MEMBER(kungfur_state,kungfur_latch2_w),	/* Port B write */
	DEVCB_NULL,							/* Port C read */
	DEVCB_DRIVER_MEMBER(kungfur_state,kungfur_latch3_w)		/* Port C write */
};


static const msm5205_interface msm5205_config_1 =
{
	kfr_adpcm1_int,
	MSM5205_S48_4B
};

static const msm5205_interface msm5205_config_2 =
{
	kfr_adpcm2_int,
	MSM5205_S48_4B
};

static MACHINE_START( kungfur )
{
	kungfur_state *state = machine.driver_data<kungfur_state>();

	state->save_item(NAME(state->m_control));
	state->save_item(NAME(state->m_latch));

	state->save_item(NAME(state->m_adpcm_pos));
	state->save_item(NAME(state->m_adpcm_data));
	state->save_item(NAME(state->m_adpcm_sel));
}

static MACHINE_RESET( kungfur )
{
	kungfur_state *state = machine.driver_data<kungfur_state>();
	state->m_control = 0;
}

static MACHINE_CONFIG_START( kungfur, kungfur_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 8000000/2)	// 4MHz?
	MCFG_CPU_PROGRAM_MAP(kungfur_map)
	MCFG_CPU_PERIODIC_INT(kungfur_irq, 975)		// close approximation

	MCFG_I8255A_ADD( "ppi8255_0", ppi8255_0_intf )
	MCFG_I8255A_ADD( "ppi8255_1", ppi8255_1_intf )

	MCFG_MACHINE_START(kungfur)
	MCFG_MACHINE_RESET(kungfur)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("adpcm1", MSM5205, XTAL_384kHz)	// clock verified with recording
	MCFG_SOUND_CONFIG(msm5205_config_1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_SOUND_ADD("adpcm2", MSM5205, XTAL_384kHz)	// "
	MCFG_SOUND_CONFIG(msm5205_config_2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kungfur )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kr1.bin",   0x0c000, 0x04000, CRC(f5b93cc7) SHA1(ed962915aeafea823a6562e6f284a88422f09a08) )

	ROM_REGION( 0x20000, "adpcm1", 0 )
	ROM_LOAD( "kr2.bin",   0x00000, 0x10000, CRC(13f5eba8) SHA1(a3ae2d54ec60d48bfff6192e61033ec583e3603f) )
	ROM_LOAD( "kr3.bin",   0x10000, 0x10000, CRC(05fd1301) SHA1(6871d872315ffb025fea7d2ccd9a203863dc142d) )

	ROM_REGION( 0x40000, "adpcm2", 0 )
	ROM_LOAD( "kr4.bin",   0x00000, 0x10000, CRC(58929279) SHA1(d90f68dd8cf2ddc5e73ed40eb31ebbb0be7e35a4) )
	ROM_LOAD( "kr5.bin",   0x10000, 0x10000, CRC(31ed39c8) SHA1(8da50b2183a287fe3a41ec13078aff7fb40c43a3) )
	ROM_LOAD( "kr6.bin",   0x20000, 0x10000, CRC(9ea75d4a) SHA1(57445ccb961acb11a25cdac81f2e543d92bcb7f9) )
ROM_END

GAMEL(1987, kungfur,  0,       kungfur,  kungfur, driver_device,  0, ROT0, "Namco", "Kung-Fu Roushi", GAME_SUPPORTS_SAVE, layout_kungfur )
