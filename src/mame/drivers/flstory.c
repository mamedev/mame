// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    The FairyLand Story

    added Victorious Nine by BUT

    TODO:
    - TA7630 emulation needs filter support (bass sounds from MSM5232 should be about 2 times louder)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "includes/flstory.h"

READ8_MEMBER(flstory_state::from_snd_r)
{
	m_snd_flag = 0;
	return m_snd_data;
}

READ8_MEMBER(flstory_state::snd_flag_r)
{
	return m_snd_flag | 0xfd;
}

WRITE8_MEMBER(flstory_state::to_main_w)
{
	m_snd_data = data;
	m_snd_flag = 2;
}

TIMER_CALLBACK_MEMBER(flstory_state::nmi_callback)
{
	if (m_sound_nmi_enable)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	else
		m_pending_nmi = 1;
}

WRITE8_MEMBER(flstory_state::sound_command_w)
{
	soundlatch_byte_w(space, 0, data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(flstory_state::nmi_callback),this), data);
}


WRITE8_MEMBER(flstory_state::nmi_disable_w)
{
	m_sound_nmi_enable = 0;
}

WRITE8_MEMBER(flstory_state::nmi_enable_w)
{
	m_sound_nmi_enable = 1;
	if (m_pending_nmi)
	{
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_pending_nmi = 0;
	}
}

static ADDRESS_MAP_START( flstory_map, AS_PROGRAM, 8, flstory_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(flstory_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM /* unknown */
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(flstory_mcu_r, flstory_mcu_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITENOP    /* watchdog? */
	AM_RANGE(0xd002, 0xd002) AM_WRITENOP    /* coin lock out? */
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(snd_flag_r)
	AM_RANGE(0xd403, 0xd403) AM_NOP /* unknown */
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSW0")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSW1")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSW2")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("P1")
	AM_RANGE(0xd805, 0xd805) AM_READ(flstory_mcu_status_r)
	AM_RANGE(0xd806, 0xd806) AM_READ_PORT("P2")
//  AM_RANGE(0xda00, 0xda00) AM_WRITEONLY
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM_WRITE(flstory_scrlram_w) AM_SHARE("scrlram")
	AM_RANGE(0xdcc0, 0xdcff) AM_RAM /* unknown */
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(flstory_palette_r, flstory_palette_w)
	AM_RANGE(0xdf03, 0xdf03) AM_WRITE(flstory_gfxctrl_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM /* work RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( onna34ro_map, AS_PROGRAM, 8, flstory_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(flstory_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM /* unknown */
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(onna34ro_mcu_r, onna34ro_mcu_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITENOP    /* watchdog? */
	AM_RANGE(0xd002, 0xd002) AM_WRITENOP    /* coin lock out? */
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(snd_flag_r)
	AM_RANGE(0xd403, 0xd403) AM_NOP /* unknown */
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSW0")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSW1")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSW2")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("P1")
	AM_RANGE(0xd805, 0xd805) AM_READ(onna34ro_mcu_status_r)
	AM_RANGE(0xd806, 0xd806) AM_READ_PORT("P2")
//  AM_RANGE(0xda00, 0xda00) AM_WRITEONLY
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM_WRITE(flstory_scrlram_w) AM_SHARE("scrlram")
	AM_RANGE(0xdcc0, 0xdcff) AM_RAM /* unknown */
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(flstory_palette_r, flstory_palette_w)
	AM_RANGE(0xdf03, 0xdf03) AM_WRITE(flstory_gfxctrl_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("workram") /* work RAM */
ADDRESS_MAP_END

CUSTOM_INPUT_MEMBER(flstory_state::victnine_mcu_status_bit01_r)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	return (victnine_mcu_status_r(space, 0) & 3);
}

static ADDRESS_MAP_START( victnine_map, AS_PROGRAM, 8, flstory_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(flstory_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM /* unknown */
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(victnine_mcu_r, victnine_mcu_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITENOP    /* watchdog? */
	AM_RANGE(0xd002, 0xd002) AM_NOP /* unknown read & coin lock out? */
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(snd_flag_r)
	AM_RANGE(0xd403, 0xd403) AM_READNOP /* unknown */
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSW0")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSW1")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSW2")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("P1")
	AM_RANGE(0xd805, 0xd805) AM_READ_PORT("EXTRA_P1")   /* also mcu */
	AM_RANGE(0xd806, 0xd806) AM_READ_PORT("P2")
	AM_RANGE(0xd807, 0xd807) AM_READ_PORT("EXTRA_P2")
//  AM_RANGE(0xda00, 0xda00) AM_WRITEONLY
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM_WRITE(flstory_scrlram_w) AM_SHARE("scrlram")
	AM_RANGE(0xdce0, 0xdce0) AM_READWRITE(victnine_gfxctrl_r, victnine_gfxctrl_w)
	AM_RANGE(0xdce1, 0xdce1) AM_WRITENOP    /* unknown */
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(flstory_palette_r, flstory_palette_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("workram") /* work RAM */
ADDRESS_MAP_END


READ8_MEMBER(flstory_state::rumba_mcu_r)
{
	//printf("PC=%04x R %02x\n",space.device().safe_pc(),m_mcu_cmd);

	if((m_mcu_cmd & 0xf0) == 0x00) // end packet cmd, value returned is meaningless (probably used for main <-> mcu comms syncronization)
		return 0;

	switch(m_mcu_cmd)
	{
		case 0x73: return 0xa4; //initial MCU check
		case 0x33: return m_mcu_b2_res; //0xb2 result
		case 0x31: return m_mcu_b1_res; //0xb1 result

		case 0x35: m_mcu_b5_res = 1; m_mcu_b6_res = 1; return 0;
		case 0x36: return m_mcu_b4_cmd; //0xb4 command, extra protection for lives (first play only), otherwise game gives one extra life at start-up (!)
		case 0x37: return m_mcu_b5_res; //0xb4 / 0xb5 / 0xb6 result y value
		case 0x38: return m_mcu_b6_res; //x value

		case 0x3b: return m_mcu_bb_res; //0xbb result
		case 0x40: return 0;
		case 0x41: return 0;
		case 0x42:
		{
			/* TODO: subtle behaviour for transitioning from level 16 to level 17 (loop clear?). Command is:
			0xc0 -> param -> 0xc1 -> param -> ... 0xc7 -> param -> 0x0e (end of packet) then reads at 0x40 -> 0x41 and 0x42

			Params written doesn't make any sense, they are copies from RAM addresses at 0xe450-7 and they looks like ... garbage.
			It's possible that all of this it just increments by one an internal RAM address in the MCU and then it sends a six when this counter
			has bits 0-3 == 0 (BCD operation?), but then the question is ... how it determines game over?

			According to a PCB test, game should roll back to level 1 layout but level counter should say "17" instead of current "11". Some of these ports also appears to control
			game-play speed and who is playing between player 1 and 2.
			*/
			//static UINT8 level_val;

			//level_val = read_byte(0xe247);

			//popmessage("%02x",level_val);

			//if((level_val & 0x0f) == 0x00)
			//  return 0; //6

			return 0;
		}
		//case 0x42: return 0x06;
		//default:  printf("PC=%04x R %02x\n",space.device().safe_pc(),m_mcu_cmd); break;
	}

	return 0;
}

WRITE8_MEMBER(flstory_state::rumba_mcu_w)
{
	//if((m_mcu_cmd & 0xf0) == 0xc0)
	//  printf("%02x ",data);

	//if(m_mcu_cmd == 0x42)
	//  printf("\n");

	if(m_mcu_param)
	{
		m_mcu_param = 0; // clear param

		//printf("%02x %02x\n",m_mcu_cmd,data);

		switch(m_mcu_cmd)
		{
			case 0xb0: // counter, used by command 0xb1 (and something else?
			{
				/*
				sends 0xb0 -> param then 0xb1 -> param -> 0x01 (end of cmd packet?) finally 0x31 for reply
				*/

				m_mcu_counter = data;

				break;
			}
			case 0xb1: // player death sequence, controls X position
			{
				m_mcu_b1_res = data;

				/* TODO: this is pretty hard to simulate ... */
				if(m_mcu_counter >= 0x10)
					m_mcu_b1_res++; // left
				else if(m_mcu_counter >= 0x08)
					m_mcu_b1_res--; // right
				else
					m_mcu_b1_res++; // left again

				break;
			}
			case 0xb2: // player sprite hook-up param when he throws the wheel
			{
				/*
				sends 0xb2 -> param -> 0x02 (end of cmd packet?) then 0x33 for reply
				*/

				switch(data)
				{
					case 1: m_mcu_b2_res = 0xaa; break; //left
					case 2: m_mcu_b2_res = 0xaa; break; //right
					case 4: m_mcu_b2_res = 0xab; break; //down
					case 8: m_mcu_b2_res = 0xa9; break; //up
				}
				break;
			}
			case 0xbb: // when you start a level, lives
			{
				/*
				sends 0xbb -> param -> 0x04 (end of cmd packet?) then 0x3b for reply
				*/

				m_mcu_bb_res = data;
				//printf("PC=%04x W %02x -> %02x\n",space.device().safe_pc(),m_mcu_cmd,data);
				break;
			}
			case 0xb4: // when the bird touches the top / bottom / left / right of the screen, for correct repositioning
			{
				m_mcu_b4_cmd = data;

				//popmessage("%02x",m_mcu_b4_cmd);

				/*
				sends 0xb4 -> param -> 0xb5 -> param (bird X coord) -> 0xb6 -> param (bird Y coord) ->
				*/

				#if 0
				switch(data)
				{
					case 1: break; // from up to down
					case 2: break; // from left to right
					case 3: break; // from right to left
					case 4: break; // from down to up
				}
				#endif
				break;
			}
			case 0xb5: // bird X coord
			{
				/* TODO: values might be off by one */
				m_mcu_b5_res = data;

				if(m_mcu_b4_cmd == 3) // from right to left
					m_mcu_b5_res = 0x0d;

				if(m_mcu_b4_cmd == 2) // from left to right
					m_mcu_b5_res = 0xe4;

				break;
			}
			case 0xb6: // bird Y coord
			{
				m_mcu_b6_res = data;

				if(m_mcu_b4_cmd == 1) // from up to down
					m_mcu_b6_res = 0x04;

				if(m_mcu_b4_cmd == 4) // from down to up
					m_mcu_b6_res = 0xdc;

				break;
			}
		}

		//if((m_mcu_cmd & 0xf0) == 0xc0)
		//  printf("%02x ",data);

		//if(m_mcu_cmd == 0xc7)
		//  printf("\n");

		return;
	}

	m_mcu_cmd = data;

	if(((data & 0xf0) == 0xb0 || (data & 0xf0) == 0xc0) && m_mcu_param == 0)
		m_mcu_param = 1;
}

static ADDRESS_MAP_START( rumba_map, AS_PROGRAM, 8, flstory_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(flstory_videoram_w) AM_SHARE("videoram")
//  AM_RANGE(0xc800, 0xcfff) AM_RAM /* unknown */
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(rumba_mcu_r, rumba_mcu_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITENOP    /* watchdog? */
//  AM_RANGE(0xd002, 0xd002) AM_NOP /* unknown read & coin lock out? */
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(snd_flag_r)
//  AM_RANGE(0xd403, 0xd403) AM_READNOP /* unknown */
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSW0")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSW1")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSW2")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("P1")
	AM_RANGE(0xd805, 0xd805) AM_READ_PORT("EXTRA_P1")   /* also mcu */
	AM_RANGE(0xd806, 0xd806) AM_READ_PORT("P2")
	AM_RANGE(0xd807, 0xd807) AM_READ_PORT("EXTRA_P2")
//  AM_RANGE(0xda00, 0xda00) AM_WRITEONLY
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM_WRITE(flstory_scrlram_w) AM_SHARE("scrlram")
	AM_RANGE(0xdce0, 0xdce0) AM_READWRITE(victnine_gfxctrl_r, victnine_gfxctrl_w)
//  AM_RANGE(0xdce1, 0xdce1) AM_WRITENOP    /* unknown */
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(flstory_palette_r, flstory_palette_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("workram") /* work RAM */
ADDRESS_MAP_END


MACHINE_RESET_MEMBER(flstory_state,ta7630)
{
	int i;

	double db           = 0.0;
	double db_step      = 1.50; /* 1.50 dB step (at least, maybe more) */
	double db_step_inc  = 0.125;
	for (i = 0; i < 16; i++)
	{
		double max = 100.0 / pow(10.0, db/20.0 );
		m_vol_ctrl[15 - i] = max;
		/*logerror("vol_ctrl[%x] = %i (%f dB)\n", 15 - i, m_vol_ctrl[15 - i], db);*/
		db += db_step;
		db_step += db_step_inc;
	}

	/* for (i = 0; i < 8; i++)
	    logerror("SOUND Chan#%i name=%s\n", i, mixer_get_name(i)); */
/*
  channels 0-2 AY#0
  channels 3,4 MSM5232 group1,group2
*/
}

WRITE8_MEMBER(flstory_state::sound_control_0_w)
{
	m_snd_ctrl0 = data & 0xff;
	//  popmessage("SND0 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);

	/* this definitely controls main melody voice on 2'-1 and 4'-1 outputs */
	m_msm->set_output_gain(0, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */
	m_msm->set_output_gain(1, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */
	m_msm->set_output_gain(2, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */
	m_msm->set_output_gain(3, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */

}
WRITE8_MEMBER(flstory_state::sound_control_1_w)
{
	m_snd_ctrl1 = data & 0xff;
	//  popmessage("SND1 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);
	m_msm->set_output_gain(4, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
	m_msm->set_output_gain(5, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
	m_msm->set_output_gain(6, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
	m_msm->set_output_gain(7, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
}

WRITE8_MEMBER(flstory_state::sound_control_2_w)
{
	device_t *device = machine().device("aysnd");
	int i;

	m_snd_ctrl2 = data & 0xff;
	//  popmessage("SND2 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);

	device_sound_interface *sound;
	device->interface(sound);
	for (i = 0; i < 3; i++)
		sound->set_output_gain(i, m_vol_ctrl[(m_snd_ctrl2 >> 4) & 15] / 100.0); /* ym2149f all */
}

WRITE8_MEMBER(flstory_state::sound_control_3_w)/* unknown */
{
	m_snd_ctrl3 = data & 0xff;
	//  popmessage("SND3 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);
}


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, flstory_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc801) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xca00, 0xca0d) AM_DEVWRITE("msm", msm5232_device, write)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(sound_control_0_w)
	AM_RANGE(0xce00, 0xce00) AM_WRITE(sound_control_1_w)
	AM_RANGE(0xd800, 0xd800) AM_READ(soundlatch_byte_r) AM_WRITE(to_main_w)
	AM_RANGE(0xda00, 0xda00) AM_READNOP AM_WRITE(nmi_enable_w)          /* unknown read*/
	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(nmi_disable_w)
	AM_RANGE(0xde00, 0xde00) AM_READNOP AM_DEVWRITE("dac", dac_device, write_unsigned8) /* signed 8-bit DAC &  unknown read */
	AM_RANGE(0xe000, 0xefff) AM_ROM                                         /* space for diagnostics ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( flstory_m68705_map, AS_PROGRAM, 8, flstory_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(flstory_68705_port_a_r, flstory_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(flstory_68705_port_b_r, flstory_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(flstory_68705_port_c_r, flstory_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(flstory_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(flstory_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(flstory_68705_ddr_c_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END



/* When "Debug Mode" Dip Switch is ON, keep IPT_SERVICE1 ('9') pressed to freeze the game.
   Once the game is frozen, you can press IPT_START1 ('5') to advance 1 frame, or IPT_START2
   ('6') to advance 6 frames.

   When "Continue" Dip Switch is ON, you can only continue in a 1 player game AND when level
   (0xe781) is between 8 and 98 (included).
*/

static INPUT_PORTS_START( flstory )
	PORT_START("DSW0")      /*D800*/
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000 100000" )
	PORT_DIPSETTING(    0x01, "30000 150000" )
	PORT_DIPSETTING(    0x02, "50000 150000" )
	PORT_DIPSETTING(    0x03, "70000 150000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x20, 0x20, "Debug Mode" )            // Check code at 0x0679
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")      /*D801*/
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW2")      /* D802 */
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Attract Animation" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Leave Off")              // Check code at 0x7859
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )          // (must be OFF or the game will
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )           // hang after the game is over !)
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("SYSTEM")      /* D803 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* "BAD IO" if low */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* "BAD IO" if low */

	PORT_START("P1")      /* D804 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")      /* D806 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( onna34ro )
	PORT_START("DSW0")      /* D800*/
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(   0x00, "200000 200000" )
	PORT_DIPSETTING(   0x01, "200000 300000" )
	PORT_DIPSETTING(   0x02, "100000 200000" )
	PORT_DIPSETTING(   0x03, "200000 100000" )
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x04, DEF_STR( On ) )
	PORT_DIPNAME(0x18, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(   0x10, "1" )
	PORT_DIPSETTING(   0x08, "2" )
	PORT_DIPSETTING(   0x00, "3" )
	PORT_DIPSETTING(   0x18, "Endless (Cheat)")
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(   0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(   0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")      /* D801 */
	PORT_DIPNAME(0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(   0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(   0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(   0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(   0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(   0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(   0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(   0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(   0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(   0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(   0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(   0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME(0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(   0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(   0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(   0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(   0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(   0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(   0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(   0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(   0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(   0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(   0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(   0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(   0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(   0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW2")      /* D802 */
	PORT_DIPNAME(0x01, 0x00, "Invulnerability (Cheat)")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x01, DEF_STR( On ) )
	PORT_DIPNAME(0x02, 0x00, "Rack Test" )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x02, DEF_STR( On ) )
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Unknown ) ) /* demo sounds */
	PORT_DIPSETTING(   0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x08, 0x00, "Freeze" )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x08, DEF_STR( On ) )
	PORT_DIPNAME(0x10, 0x00, "Coinage Display" )
	PORT_DIPSETTING(   0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x60, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(   0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(   0x60, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(   0x80, "A and B" )
	PORT_DIPSETTING(   0x00, "A only" )

	PORT_START("SYSTEM")      /* D803 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* "BAD IO" if low */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* "BAD IO" if low */

	PORT_START("P1")      /* D804 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")      /* D806 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( victnine )
	PORT_START("DSW0")      /* D800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xa0, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x00, "MA / MB" )

	PORT_START("DSW1")      /* D801 */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW2")      /* D802 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Show Year" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "No hit" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )

	PORT_START("SYSTEM")      /* D803 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")      /* D804 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )    // A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )    // C
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EXTRA_P1")      /* D805 */
	/* bits 0,1 are MCU related:
	    - bit 0: mcu is ready to receive data from main cpu
	    - bit 1: mcu has sent data to the main cpu       */
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, flstory_state,victnine_mcu_status_bit01_r, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")      /* D806 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL  // A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL  // C
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EXTRA_P2")      /* D807 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( rumba )
	PORT_START("DSW0")      /* D800 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000 50000" )
	PORT_DIPSETTING(    0x01, "10000 60000" )
	PORT_DIPSETTING(    0x02, "10000 40000" )
	PORT_DIPSETTING(    0x03, "10000 20000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")      /* D801 */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW2")      /* D802 */
	PORT_DIPNAME( 0x01, 0x01, "Training Stage" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract Sound" ) /* At title sequence only - NOT Demo Sounds */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coinage Display" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Copyright String" )
	PORT_DIPSETTING(    0x20, "Taito Corp. MCMLXXXIV" )
	PORT_DIPSETTING(    0x00, "Taito Corporation" )
	PORT_DIPNAME( 0x40, 0x40, "Infinite Lives" ) //???
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")      /* D803 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")      /* D804 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )    // A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )    // C
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EXTRA_P1")      /* D805 */
	/* bits 0,1 are MCU related:
	    - bit 0: mcu is ready to receive data from main cpu
	    - bit 1: mcu has sent data to the main cpu       */
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, flstory_state,victnine_mcu_status_bit01_r, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")      /* D806 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL  // A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL  // C
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EXTRA_P2")      /* D807 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16*8+3, 16*8+2, 16*8+1, 16*8+0, 16*8+8+3, 16*8+8+2, 16*8+8+1, 16*8+8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8
};

static GFXDECODE_START( flstory )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 256, 16 )
GFXDECODE_END


void flstory_state::machine_start()
{
	/* video */
	save_item(NAME(m_gfxctrl));
	save_item(NAME(m_char_bank));
	save_item(NAME(m_palette_bank));
	/* sound */
	save_item(NAME(m_snd_data));
	save_item(NAME(m_snd_flag));
	save_item(NAME(m_sound_nmi_enable));
	save_item(NAME(m_pending_nmi));
	save_item(NAME(m_vol_ctrl));
	save_item(NAME(m_snd_ctrl0));
	save_item(NAME(m_snd_ctrl1));
	save_item(NAME(m_snd_ctrl2));
	save_item(NAME(m_snd_ctrl3));
	/* mcu */
	save_item(NAME(m_from_main));
	save_item(NAME(m_from_mcu));
	save_item(NAME(m_mcu_sent));
	save_item(NAME(m_main_sent));
	save_item(NAME(m_port_a_in));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_port_b_in));
	save_item(NAME(m_port_b_out));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_port_c_in));
	save_item(NAME(m_port_c_out));
	save_item(NAME(m_ddr_c));
	save_item(NAME(m_mcu_select));
}

MACHINE_RESET_MEMBER(flstory_state,flstory)
{
	MACHINE_RESET_CALL_MEMBER(ta7630);

	/* video */
	m_gfxctrl = 0;
	m_char_bank = 0;
	m_palette_bank = 0;
	/* sound */
	m_snd_data = 0;
	m_snd_flag = 0;
	m_sound_nmi_enable = 0;
	m_pending_nmi = 0;
	m_snd_ctrl0 = 0;
	m_snd_ctrl1 = 0;
	m_snd_ctrl2 = 0;
	m_snd_ctrl3 = 0;
	/* mcu */
	m_from_main = 0;
	m_from_mcu = 0;
	m_mcu_sent = 0;
	m_main_sent = 0;
	m_port_a_in = 0;
	m_port_a_out = 0;
	m_ddr_a = 0;
	m_port_b_in = 0;
	m_port_b_out = 0;
	m_ddr_b = 0;
	m_port_c_in = 0;
	m_port_c_out = 0;
	m_ddr_c = 0;
	m_mcu_select = 0;
}

static MACHINE_CONFIG_START( flstory, flstory_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_10_733MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(flstory_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", flstory_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(flstory_state, irq0_line_hold, 2*60)   /* IRQ generated by ??? */
						/* NMI generated by the main CPU */

	MCFG_CPU_ADD("mcu", M68705, XTAL_18_432MHz/6)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(flstory_m68705_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MCFG_MACHINE_RESET_OVERRIDE(flstory_state,flstory)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(flstory_state, screen_update_flstory)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT180)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", flstory)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(flstory_state,flstory)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_8MHz/4) /* verified on pcb */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(flstory_state, sound_control_2_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(flstory_state, sound_control_3_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_SOUND_ADD("msm", MSM5232, XTAL_8MHz/4) /* verified on pcb */
	MCFG_MSM5232_SET_CAPACITORS(1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6) /* 1.0 uF capacitors (verified on real PCB) */
	MCFG_SOUND_ROUTE(0, "mono", 1.0)    // pin 28  2'-1
	MCFG_SOUND_ROUTE(1, "mono", 1.0)    // pin 29  4'-1
	MCFG_SOUND_ROUTE(2, "mono", 1.0)    // pin 30  8'-1
	MCFG_SOUND_ROUTE(3, "mono", 1.0)    // pin 31 16'-1
	MCFG_SOUND_ROUTE(4, "mono", 1.0)    // pin 36  2'-2
	MCFG_SOUND_ROUTE(5, "mono", 1.0)    // pin 35  4'-2
	MCFG_SOUND_ROUTE(6, "mono", 1.0)    // pin 34  8'-2
	MCFG_SOUND_ROUTE(7, "mono", 1.0)    // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( onna34ro, flstory_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 10733000/2)     /* ??? */
	MCFG_CPU_PROGRAM_MAP(onna34ro_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", flstory_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 8000000/2)     /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(flstory_state, irq0_line_hold, 2*60)   /* IRQ generated by ??? */
						/* NMI generated by the main CPU */

//  MCFG_CPU_ADD("mcu", M68705, XTAL_18_432MHz/6)  /* ??? */
//  MCFG_CPU_PROGRAM_MAP(m68705_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MCFG_MACHINE_RESET_OVERRIDE(flstory_state,flstory)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(flstory_state, screen_update_flstory)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", flstory)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(flstory_state,flstory)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 8000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(flstory_state, sound_control_2_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(flstory_state, sound_control_3_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_SOUND_ADD("msm", MSM5232, 8000000/4)
	MCFG_MSM5232_SET_CAPACITORS(1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6) /* 1.0 uF capacitors (verified on real PCB) */
	MCFG_SOUND_ROUTE(0, "mono", 1.0)    // pin 28  2'-1
	MCFG_SOUND_ROUTE(1, "mono", 1.0)    // pin 29  4'-1
	MCFG_SOUND_ROUTE(2, "mono", 1.0)    // pin 30  8'-1
	MCFG_SOUND_ROUTE(3, "mono", 1.0)    // pin 31 16'-1
	MCFG_SOUND_ROUTE(4, "mono", 1.0)    // pin 36  2'-2
	MCFG_SOUND_ROUTE(5, "mono", 1.0)    // pin 35  4'-2
	MCFG_SOUND_ROUTE(6, "mono", 1.0)    // pin 34  8'-2
	MCFG_SOUND_ROUTE(7, "mono", 1.0)    // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( victnine, flstory_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 8000000/2)      /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(victnine_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", flstory_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 8000000/2)     /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(flstory_state, irq0_line_hold, 2*60)   /* IRQ generated by ??? */
						/* NMI generated by the main CPU */

//  MCFG_CPU_ADD("mcu", M68705, XTAL_18_432MHz/6)  /* ??? */
//  MCFG_CPU_PROGRAM_MAP(m68705_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MCFG_MACHINE_RESET_OVERRIDE(flstory_state,flstory)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(flstory_state, screen_update_victnine)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", flstory)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(flstory_state,victnine)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 8000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(flstory_state, sound_control_2_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(flstory_state, sound_control_3_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5232, 8000000/4)
	MCFG_MSM5232_SET_CAPACITORS(1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6) /* 1.0 uF capacitors (verified on real PCB) */
	MCFG_SOUND_ROUTE(0, "mono", 1.0)    // pin 28  2'-1
	MCFG_SOUND_ROUTE(1, "mono", 1.0)    // pin 29  4'-1
	MCFG_SOUND_ROUTE(2, "mono", 1.0)    // pin 30  8'-1
	MCFG_SOUND_ROUTE(3, "mono", 1.0)    // pin 31 16'-1
	MCFG_SOUND_ROUTE(4, "mono", 1.0)    // pin 36  2'-2
	MCFG_SOUND_ROUTE(5, "mono", 1.0)    // pin 35  4'-2
	MCFG_SOUND_ROUTE(6, "mono", 1.0)    // pin 34  8'-2
	MCFG_SOUND_ROUTE(7, "mono", 1.0)    // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END

MACHINE_RESET_MEMBER(flstory_state,rumba)
{
	MACHINE_RESET_CALL_MEMBER(flstory);
	m_mcu_cmd = 0;
}

static MACHINE_CONFIG_START( rumba, flstory_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(rumba_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", flstory_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(flstory_state, irq0_line_hold, 2*60)   /* IRQ generated by ??? */
						/* NMI generated by the main CPU */

//  MCFG_CPU_ADD("mcu", M68705, XTAL_18_432MHz/6) /* verified on pcb */
//  MCFG_CPU_PROGRAM_MAP(m68705_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MCFG_MACHINE_RESET_OVERRIDE(flstory_state,rumba)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(flstory_state, screen_update_rumba)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", flstory)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(flstory_state,rumba)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_8MHz/4) /* verified on pcb */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(flstory_state, sound_control_2_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(flstory_state, sound_control_3_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5232, XTAL_8MHz/4) /* verified on pcb */
	MCFG_MSM5232_SET_CAPACITORS(1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6) /* 1.0 uF capacitors (verified on real PCB) */
	MCFG_SOUND_ROUTE(0, "mono", 1.0)    // pin 28  2'-1
	MCFG_SOUND_ROUTE(1, "mono", 1.0)    // pin 29  4'-1
	MCFG_SOUND_ROUTE(2, "mono", 1.0)    // pin 30  8'-1
	MCFG_SOUND_ROUTE(3, "mono", 1.0)    // pin 31 16'-1
	MCFG_SOUND_ROUTE(4, "mono", 1.0)    // pin 36  2'-2
	MCFG_SOUND_ROUTE(5, "mono", 1.0)    // pin 35  4'-2
	MCFG_SOUND_ROUTE(6, "mono", 1.0)    // pin 34  8'-2
	MCFG_SOUND_ROUTE(7, "mono", 1.0)    // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( flstory )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "cpu-a45.15",   0x0000, 0x4000, CRC(f03fc969) SHA1(c8dd25ca25fd413b1a29bd4e58ce5820e5f852b2) )
	ROM_LOAD( "cpu-a45.16",   0x4000, 0x4000, CRC(311aa82e) SHA1(c2dd806f70ea917818ec844a275fb2fecc2e6c19) )
	ROM_LOAD( "cpu-a45.17",   0x8000, 0x4000, CRC(a2b5d17d) SHA1(0198d048aedcbd2498d490a5c0c506f8fc66ed03) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "snd.22",       0x0000, 0x2000, CRC(d58b201d) SHA1(1c9c2936ec95a8fa920d58668bea420c5e15008f) )
	ROM_LOAD( "snd.23",       0x2000, 0x2000, CRC(25e7fd9d) SHA1(b9237459e3d8acf8502a693914e50714a37d515e) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a45.mcu",      0x0000, 0x0800, CRC(5378253c) SHA1(e1ae1ab01e470b896c1d74ad4088928602a21a1b) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "vid-a45.18",   0x00000, 0x4000, CRC(6f08f69e) SHA1(8f1b7e63a38f855cf26d57aed678da7cf1378fdf) )
	ROM_LOAD( "vid-a45.06",   0x04000, 0x4000, CRC(dc856a75) SHA1(6eedbf6b027c884502b6e7329f13829787138165) )
	ROM_LOAD( "vid-a45.08",   0x08000, 0x4000, CRC(d0b028ca) SHA1(c8bd9136ad3180002961ecfe600fc91a3c891539) )
	ROM_LOAD( "vid-a45.20",   0x0c000, 0x4000, CRC(1b0edf34) SHA1(e749c78053ed09bdb42c03cf4589b0fe122d9095) )
	ROM_LOAD( "vid-a45.19",   0x10000, 0x4000, CRC(2b572dc9) SHA1(9e14428663819e18829c625b4ae91a8a5530eb33) )
	ROM_LOAD( "vid-a45.07",   0x14000, 0x4000, CRC(aa4b0762) SHA1(6d4246753e80fe3ca05d47bd279f7ccc603f4700) )
	ROM_LOAD( "vid-a45.09",   0x18000, 0x4000, CRC(8336be58) SHA1(b92d37856870c4128a860d8ae02fa647743b99e3) )
	ROM_LOAD( "vid-a45.21",   0x1c000, 0x4000, CRC(fc382bd1) SHA1(a773c87454a3d7b80374a6d38ecb8633af2cd990) )
ROM_END

ROM_START( flstoryj )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "cpu-a45.15",   0x0000, 0x4000, CRC(f03fc969) SHA1(c8dd25ca25fd413b1a29bd4e58ce5820e5f852b2) )
	ROM_LOAD( "cpu-a45.16",   0x4000, 0x4000, CRC(311aa82e) SHA1(c2dd806f70ea917818ec844a275fb2fecc2e6c19) )
	ROM_LOAD( "cpu-a45.17",   0x8000, 0x4000, CRC(a2b5d17d) SHA1(0198d048aedcbd2498d490a5c0c506f8fc66ed03) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a45_12.8",     0x0000, 0x2000, CRC(d6f593fb) SHA1(8551ef22c2cdd9df8d7949a178883f56ea56a4a2) )
	ROM_LOAD( "a45_13.9",     0x2000, 0x2000, CRC(451f92f9) SHA1(f4196e6d3420983b74001303936d086a48b10827) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a45.mcu",      0x0000, 0x0800, CRC(5378253c) SHA1(e1ae1ab01e470b896c1d74ad4088928602a21a1b) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "vid-a45.18",   0x00000, 0x4000, CRC(6f08f69e) SHA1(8f1b7e63a38f855cf26d57aed678da7cf1378fdf) )
	ROM_LOAD( "vid-a45.06",   0x04000, 0x4000, CRC(dc856a75) SHA1(6eedbf6b027c884502b6e7329f13829787138165) )
	ROM_LOAD( "vid-a45.08",   0x08000, 0x4000, CRC(d0b028ca) SHA1(c8bd9136ad3180002961ecfe600fc91a3c891539) )
	ROM_LOAD( "vid-a45.20",   0x0c000, 0x4000, CRC(1b0edf34) SHA1(e749c78053ed09bdb42c03cf4589b0fe122d9095) )
	ROM_LOAD( "vid-a45.19",   0x10000, 0x4000, CRC(2b572dc9) SHA1(9e14428663819e18829c625b4ae91a8a5530eb33) )
	ROM_LOAD( "vid-a45.07",   0x14000, 0x4000, CRC(aa4b0762) SHA1(6d4246753e80fe3ca05d47bd279f7ccc603f4700) )
	ROM_LOAD( "vid-a45.09",   0x18000, 0x4000, CRC(8336be58) SHA1(b92d37856870c4128a860d8ae02fa647743b99e3) )
	ROM_LOAD( "vid-a45.21",   0x1c000, 0x4000, CRC(fc382bd1) SHA1(a773c87454a3d7b80374a6d38ecb8633af2cd990) )
ROM_END

ROM_START( onna34ro )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "a52-01-1.40c", 0x0000, 0x4000, CRC(ffddcb02) SHA1(d7002e8a577a5f9c2f63ec8d93076cd720443e05) )
	ROM_LOAD( "a52-02-1.41c", 0x4000, 0x4000, CRC(da97150d) SHA1(9b18f4d0bff811e332f6d2e151c7583400d60f23) )
	ROM_LOAD( "a52-03-1.42c", 0x8000, 0x4000, CRC(b9749a53) SHA1(15fd9624a500512f7b2c6766ed96f3734f61f160) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a52-12.08s",   0x0000, 0x2000, CRC(28f48096) SHA1(20aa5041cd71003e0981c32e34005bcbad53f707) )
	ROM_LOAD( "a52-13.09s",   0x2000, 0x2000, CRC(4d3b16f3) SHA1(8687b76398da875f69e9565277f00478c2b82a99) )
	ROM_LOAD( "a52-14.10s",   0x4000, 0x2000, CRC(90a6f4e8) SHA1(101767a90e963f3031e0830fd25a537ca8296de9) )
	ROM_LOAD( "a52-15.37s",   0x6000, 0x2000, CRC(5afc21d0) SHA1(317d5fb3a48ce5e13e02c5c6431fa08ada115d27) )
	ROM_LOAD( "a52-16.38s",   0x8000, 0x2000, CRC(ccf42aee) SHA1(a6eb01c5384724999631b55700dade430b71ca95) )

	ROM_REGION( 0x0800, "cpu2", 0 ) /* 2k for the microcontroller */
	ROM_LOAD( "a52-17.54c",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a52-04.11v",   0x00000, 0x4000, CRC(5b126294) SHA1(fc31e062e665f7313f923e84d6497716f0658ac0) )
	ROM_LOAD( "a52-06.10v",   0x04000, 0x4000, CRC(78114721) SHA1(d0e52544e05ab4fd1b131ed49beb252048bcbe31) )
	ROM_LOAD( "a52-08.09v",   0x08000, 0x4000, CRC(4a293745) SHA1(a54c1cfced63306db0ba7ee635dce41134c91dc8) )
	ROM_LOAD( "a52-10.08v",   0x0c000, 0x4000, CRC(8be7b4db) SHA1(e7ab373942b8ce75b36d0c9f547902fe65a3964d) )
	ROM_LOAD( "a52-05.35v",   0x10000, 0x4000, CRC(a1a99588) SHA1(eae63ae89058da1a92065e1d352cf81a15b556bc) )
	ROM_LOAD( "a52-07.34v",   0x14000, 0x4000, CRC(0bf420f2) SHA1(367e76efbed772fc8a6d7ac854407b62f8897d78) )
	ROM_LOAD( "a52-09.33v",   0x18000, 0x4000, CRC(39c543b5) SHA1(978c42f5eb23c15a96dae3578e742ef41bac689b) )
	ROM_LOAD( "a52-11.32v",   0x1c000, 0x4000, CRC(d1dda6b3) SHA1(fadf1404e8a03ec7e3fafb6281d33bc73bb5c473) )
ROM_END

ROM_START( onna34roa )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "ry-08.rom",    0x0000, 0x4000, CRC(e4587b85) SHA1(2fc4439953dd086eac11ba6d7937d8075fc39639) )
	ROM_LOAD( "ry-07.rom",    0x4000, 0x4000, CRC(6ffda515) SHA1(429e7bb22c66eb3c6d31981c2021af61c44ed51b) )
	ROM_LOAD( "ry-06.rom",    0x8000, 0x4000, CRC(6fefcda8) SHA1(f532e254a8bd7372bd9f8f21c907e44e0f5f4f32) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a52-12.08s",   0x0000, 0x2000, CRC(28f48096) SHA1(20aa5041cd71003e0981c32e34005bcbad53f707) )
	ROM_LOAD( "a52-13.09s",   0x2000, 0x2000, CRC(4d3b16f3) SHA1(8687b76398da875f69e9565277f00478c2b82a99) )
	ROM_LOAD( "a52-14.10s",   0x4000, 0x2000, CRC(90a6f4e8) SHA1(101767a90e963f3031e0830fd25a537ca8296de9) )
	ROM_LOAD( "a52-15.37s",   0x6000, 0x2000, CRC(5afc21d0) SHA1(317d5fb3a48ce5e13e02c5c6431fa08ada115d27) )
	ROM_LOAD( "a52-16.38s",   0x8000, 0x2000, CRC(ccf42aee) SHA1(a6eb01c5384724999631b55700dade430b71ca95) )

	ROM_REGION( 0x0800, "cpu2", 0 ) /* 2k for the microcontroller */
	ROM_LOAD( "a52-17.54c",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a52-04.11v",   0x00000, 0x4000, CRC(5b126294) SHA1(fc31e062e665f7313f923e84d6497716f0658ac0) )
	ROM_LOAD( "a52-06.10v",   0x04000, 0x4000, CRC(78114721) SHA1(d0e52544e05ab4fd1b131ed49beb252048bcbe31) )
	ROM_LOAD( "a52-08.09v",   0x08000, 0x4000, CRC(4a293745) SHA1(a54c1cfced63306db0ba7ee635dce41134c91dc8) )
	ROM_LOAD( "a52-10.08v",   0x0c000, 0x4000, CRC(8be7b4db) SHA1(e7ab373942b8ce75b36d0c9f547902fe65a3964d) )
	ROM_LOAD( "a52-05.35v",   0x10000, 0x4000, CRC(a1a99588) SHA1(eae63ae89058da1a92065e1d352cf81a15b556bc) )
	ROM_LOAD( "a52-07.34v",   0x14000, 0x4000, CRC(0bf420f2) SHA1(367e76efbed772fc8a6d7ac854407b62f8897d78) )
	ROM_LOAD( "a52-09.33v",   0x18000, 0x4000, CRC(39c543b5) SHA1(978c42f5eb23c15a96dae3578e742ef41bac689b) )
	ROM_LOAD( "a52-11.32v",   0x1c000, 0x4000, CRC(d1dda6b3) SHA1(fadf1404e8a03ec7e3fafb6281d33bc73bb5c473) )
ROM_END


/*
Victorious Nine
Taito, 1984

Hardware is similar to Elevator Action (uses same pinouts for wiring harness also)

Top Board (Sound)
---------
PCB No: J1100005A K1100011A (plus a sticker... K1100014A)
CPU   : NEC D780 (Z80)
SOUND : OKI M5232 (x1), YM2149 (x1), LM3900 (x3), TA7630 (x1)
XTAL  : 8.000MHz
RAM   : M5M5517 (=6116, x1)
OTHER : Volume Pot (x1)
PALs  : None
PROMs : None
DIPSW : None

            Byte
ROMs : (All type 2764)  C'sum
------------------------------
A16_12.8        059Bh
A16_13.9        3F12h
A16_14.10       CC99h
A16_15.37       9D55h
A16_16.38       B04Dh
A16_17.39       90B1h

*************


2nd Board (Small PCB contains ROMs ONLY, plugs into three empty sockets on 3rd PCB)
---------
PCB No: J9100006A K9100009A

            Byte
ROMs : (All type 2764)  C'sum
------------------------------
A16_19.1        22E3h
A16_20.2        D3AEh
A16_21.3        DB99h
A16_22.4        B4CDh
A16_23.5        92C8h
A16_24.6        1641h

*************


3rd PCB (Main Board with connectors G and H)
-------
PCB No: J1100007A K1100013A (plus 2 stickers... K1100027A  M4300007B)
CPU   : NEC D780 (Z80, plus one unpopulated socket for another Z80 CPU)
XTAL  : 8.000MHz
RAM   : M5M5517 (=6116, x1)
OTHER : MC68705P5S (labelled "A16 18", read-protected and not dumped)
DIPSW : 8 position (x3, see archive for DSW info)
PALs  : None
PROMs : None
ROMs  : None

*************


4th PCB (Video with connector T)
-------
PCB No: J1100006A K1100012A
XTAL  : 18.432MHz
RAM   : 2148 (x9), M5M5517 (x1)

            Byte
ROMs : (All type 2764)  C'sum
------------------------------
A16_04.5        64A6h
A16_05-1.6      5DB1h
A16_06-1.7      25E2h
A16_07-2.8      E61Eh
A16_08.88       2718h
A16_09-1.89     57AAh
A16_10.90       7A95h
A16_11-1.91     4DD6h

*/

ROM_START( victnine )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "a16-19.1",     0x0000, 0x2000, CRC(deb7c439) SHA1(e87c8f95bc31d8450a3deed7a14b5fe139778d47) )
	ROM_LOAD( "a16-20.2",     0x2000, 0x2000, CRC(60cdb6ae) SHA1(65f09ef624d758b138a87c4cc80bc3539cc89507) )
	ROM_LOAD( "a16-21.3",     0x4000, 0x2000, CRC(121bea03) SHA1(4925b56a3f5725f1e00bd6aa87949aca5caf476b) )
	ROM_LOAD( "a16-22.4",     0x6000, 0x2000, CRC(b20e3027) SHA1(fab83afd1010fe6cebbeee06099eb2be9b96ec8a) )
	ROM_LOAD( "a16-23.5",     0x8000, 0x2000, CRC(95fe9cb7) SHA1(cfd7c0123940f680365500a516c8435330ed5f60) )
	ROM_LOAD( "a16-24.6",     0xa000, 0x2000, CRC(32b5c155) SHA1(34d25f3d4fae580757b69431b8b58f6f86d2282e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a16-12.8",     0x0000, 0x2000, CRC(4b9bff43) SHA1(4bcd52d6d72213f8fa7b544dbdd344312a9e2115) )
	ROM_LOAD( "a16-13.9",     0x2000, 0x2000, CRC(355121b9) SHA1(69cbe31eed53456f49a81c37b6661f7ba4a72fa6) )
	ROM_LOAD( "a16-14.10",    0x4000, 0x2000, CRC(0f33ef4d) SHA1(6916016d7cf43870d2e19fc1e6f1b20e48e07d76) )
	ROM_LOAD( "a16-15.37",    0x6000, 0x2000, CRC(f91d63dc) SHA1(4585d0c7ed05249c17385f20b6557e2e4375a6bb) )
	ROM_LOAD( "a16-16.38",    0x8000, 0x2000, CRC(9395351b) SHA1(8f97bdf03dec47bcaaa62fb66c545566776116be) )
	ROM_LOAD( "a16-17.39",    0xa000, 0x2000, CRC(872270b3) SHA1(2298cb8ced6c3e9afb430faab1b38ba8f2fa93b5) )

	ROM_REGION( 0x0800, "cpu2", 0 ) /* 2k for the microcontroller */
	ROM_LOAD( "a16-18.mcu",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x10000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a16-06-1.7",   0x00000, 0x2000, CRC(b708134d) SHA1(9732be463cfbbe81ea0ad06da5a48b660ca429d0) )
	ROM_LOAD( "a16-07-2.8",   0x02000, 0x2000, CRC(cdaf7f83) SHA1(cf83af1655cb3ffce26c1b015b1e2249f7b12e3f) )
	ROM_LOAD( "a16-10.90",    0x04000, 0x2000, CRC(e8e42454) SHA1(c4923d4adfc0a48cf5a7d0145de5c9389495cac2) )
	ROM_LOAD( "a16-11-1.91",  0x06000, 0x2000, CRC(1f766661) SHA1(dfeecb587af7706e0e14539efc3386558f5d6da4) )
	ROM_LOAD( "a16-04.5",     0x08000, 0x2000, CRC(b2fae99f) SHA1(c8e56815159cd43a94c7e31b764d5bb996551a49) )
	ROM_LOAD( "a16-05-1.6",   0x0a000, 0x2000, CRC(85dfbb6e) SHA1(3643aab950d54eadded8d952033672aabb1e87c4) )
	ROM_LOAD( "a16-08.88",    0x0c000, 0x2000, CRC(1ddb6466) SHA1(0ea75c2fb584215f3cd4a7b7dfb3345a303e7e66) )
	ROM_LOAD( "a16-09-1.89",  0x0e000, 0x2000, CRC(23d4c43c) SHA1(ed0e059d3f97705331fdcc423a7c37aac9f07bb0) )
ROM_END


/*

RUMBA LUMBER by TAITO (1984)

Hardware similar to Fairyland Story except for the video board.
Wiring is the classic Taito one.
All clocks has been verified using a frequency counter.
Hardware is capable of playing samples (TTL circuit)


SOUND BOARD J1100022A / K1100066A

Xtal: 8mhz
Z80 NEC D780C-1 running at 8/2 = 4mhz
YM2149 running at 8/4 = 2mhz  pin 26 high
OKI M5232 running at 8/4 = 2mhz
2764 EPROM A23-08-1
2764 EPROM A23-09
2764 EPROM A23-10

CPU BOARD J1100024A / K1100065A

Xtal 8mhz
Z80 NEC D780C-1 running at 8/2 = 4mhz
MCU A23-11 MC68705P5S running at 18.432/6 = 3.072mhz
27128 EPROM A23-01-1
27128 EPROM A23-02-1
27128 EPROM A23-03-1

VIDEO BOARD J1100023A / K1100064A

xtal: 18.432mhz
2764 EPROM A23-04
2764 EPROM A23-05
2764 EPROM A23-06 (I cannot get a constant read,a couple of bytes differ everytime)
2764 EPROM A23-07

VSYNC = 60.55hz

Dumped by Corrado Tomaselli on 9/12/2010

*/

ROM_START( rumba )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "a23_01-1.bin",   0x0000, 0x4000, CRC(4bea6e18) SHA1(b9a85e65105773b5f93dcc5fc1e7c588b2d25056) )
	ROM_LOAD( "a23_02-1.bin",   0x4000, 0x4000, CRC(08f98c6f) SHA1(f2a850b1138cfefab6ff1d1adcda9e084f52e9c2) )
	ROM_LOAD( "a23_03-1.bin",   0x8000, 0x4000, CRC(ab595427) SHA1(1ff51740e1c7915e1f79a55801d11c8fdce764c8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a23_08-1.bin",     0x0000, 0x2000, CRC(a18eae00) SHA1(6ac1ad07bb5a97c6edaaf0e1fb842e1741f4cf1e) )
	ROM_LOAD( "a23_09.bin",       0x2000, 0x2000, CRC(d0a101d3) SHA1(c92bb1ce67bec394fd8ce303d9e61eac12493b5d) )
	ROM_LOAD( "a23_10.bin",       0x4000, 0x2000, CRC(f9447bd4) SHA1(68c02249ca0e5b923cddb4bff8d090963b9c78e4) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a23-11.mc68705p5s", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a23_07.bin",   0x02000, 0x2000, CRC(c98fbea6) SHA1(edd1e0b2551f726018ca6e0b2cf629046a482711) )
	ROM_LOAD( "a23_06.bin",   0x00000, 0x2000, CRC(bf1e3a7f) SHA1(1258be10739cee6e6a8b2ce4d39f89bff1ea7f16) ) // should be a good read
	ROM_LOAD( "a23_05.bin",   0x06000, 0x2000, CRC(b40db231) SHA1(85204efc05e95334576807e4dab866f4f40081e6) )
	ROM_LOAD( "a23_04.bin",   0x04000, 0x2000, CRC(1d4f001f) SHA1(c3245650e57138ed89e7de8289fe37c5d933ddca) )
ROM_END


GAME( 1985, flstory,   0,        flstory,  flstory, driver_device,  0, ROT180, "Taito", "The FairyLand Story", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, flstoryj,  flstory,  flstory,  flstory, driver_device,  0, ROT180, "Taito", "The FairyLand Story (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, onna34ro,  0,        onna34ro, onna34ro, driver_device, 0, ROT0,   "Taito", "Onna Sansirou - Typhoon Gal (set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, onna34roa, onna34ro, onna34ro, onna34ro, driver_device, 0, ROT0,   "Taito", "Onna Sansirou - Typhoon Gal (set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, victnine,  0,        victnine, victnine, driver_device, 0, ROT0,   "Taito", "Victorious Nine", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, rumba,     0,        rumba,    rumba, driver_device,    0, ROT270, "Taito", "Rumba Lumber", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
