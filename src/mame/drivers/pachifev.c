/**********************************************************************************************************

PACHI FEVER / SANKI DENSHI KOGYO


GEN6480830 (TEXAS INSTRUMENTS)
XTAL:12.000MHZ
RY050012   (TEXAS INSTRUMENTS)
XTAL:10.738MHZ

SOUND   :MSM5205 & ?

DIP SWITCH:8BIT x 3

===========================================================================================================

Many thanks to Olivier Galibert and Wilbert Pol for identify the CPU


Tomasz Slanina 10.02.2010:
---------------------------

There's very little info about the game or hardware, so all the above (except for clocks and MSM ) is
just a guess:

- CPU (GEN6480830 ?) is TMS9995 or derivative ( decrementer + lvl3 interrupt, internal ram)
- RY050012 could be a VDP ( probably TMS9928A )
- SN76469A (or similar) used for music
- MSM5205 - sample player (see below)

- TODO:
  - what's the correct game title - Pachifever ? Fever 777 ?
  - ic48.50 ROM redump (probably more adpcm samples + lookuptable .. mapped at $c000)
  - remaing DSW
  - unknown writes ($ffxx range)
  - controls : unused bits (is the BUTTON1 used _only_ for entering initials?)
  - controls : make PLUNGER (or whatever it is in reality) implementation more clear
               here's some code used to read plunger pos(angle?):

0284: 04CC             clr  R12            ; CRU address
0286: 0208 FF00        li   R8,>ff00     ; R8=ff00 - initial data ($ff)
028A: D688             movb R8,*R10     ; ff-> ff40  - write to output
028C: 3449             stcr R9,1        ; CRU read (one bit)
028E: 1603             jne  >0296        ; not zer0 - end
0290: 0228 FC00        ai   R8,>fc00    ; R8=R8-4
0294: 18FA             joc  >028a        ; loop

0296: D6A0 020F        movb @>020f,*R10  ; 00 ->ff40 - end of controls read
029A: 045B             b    *R11        ; b $1ca - process data in R8

***********************************************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9900.h"
#include "video/tms9928a.h"
#include "sound/msm5205.h"
#include "sound/sn76496.h"

#define USE_MSM 0
#define NUM_PLUNGER_REPEATS    50

typedef struct _pachifev_state pachifev_state;

struct _pachifev_state
{
 /* controls related */

 int power;
 int max_power;
 int input_power;
 int previous_power;
 int cnt;

};

static WRITE8_HANDLER(controls_w)
{
    if(!data)
    {
        pachifev_state *state = (pachifev_state *)space->machine->driver_data;

        /*end of input read*/
        state->power=0;
        state->max_power=state->input_power;
        if(--state->cnt <= 0) /* why to do it N times? no ide.. someone should fix it */
        {
            state->cnt=0;
            state->input_power=0;
        }
    }
}

static READ8_HANDLER(controls_r)
{
    pachifev_state *state = (pachifev_state *)space->machine->driver_data;
    int output_bit=(state->power < state->max_power)?0:1;
    ++state->power;
    return output_bit;
}

static ADDRESS_MAP_START( pachifev_map, ADDRESS_SPACE_PROGRAM, 8 )
    AM_RANGE(0x0000, 0x9fff) AM_ROM

    AM_RANGE(0xc000, 0xc0ff) AM_NOP /* game is expecting here some kind of lookup table (adpcm samples start?) */

    AM_RANGE(0xe000, 0xe7ff) AM_RAM
    AM_RANGE(0xf000, 0xf0fb) AM_NOP  /* internal ram */
    AM_RANGE(0xff00, 0xff00) AM_READ_PORT("IN0")
    AM_RANGE(0xff02, 0xff02) AM_READ_PORT("IN1")
    AM_RANGE(0xff04, 0xff04) AM_READ_PORT("DSW1")
    AM_RANGE(0xff06, 0xff06) AM_READ_PORT("DSW2")
    AM_RANGE(0xff08, 0xff08) AM_READ_PORT("DSW3")
    AM_RANGE(0xff10, 0xff10) AM_READWRITE(TMS9928A_vram_r, TMS9928A_vram_w)
    AM_RANGE(0xff12, 0xff12) AM_READWRITE(TMS9928A_register_r, TMS9928A_register_w)
    AM_RANGE(0xff20, 0xff20) AM_WRITENOP /* unknown */
    AM_RANGE(0xff30, 0xff30) AM_DEVWRITE("sn76", sn76496_w)
    AM_RANGE(0xff40, 0xff40) AM_WRITE(controls_w)
    AM_RANGE(0xff50, 0xff50) AM_WRITENOP /* unknown */
    AM_RANGE(0xfffa, 0xfffb) AM_NOP /* decrementer */
    AM_RANGE(0xfffc, 0xffff) AM_NOP /* nmi */
ADDRESS_MAP_END

static ADDRESS_MAP_START( pachifev_cru, ADDRESS_SPACE_IO, 8 )
    AM_RANGE(0x000, 0x000) AM_READ(controls_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( pachifev )
    PORT_START("IN0")
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )  /* used on enter player initials in top 5 */
    PORT_BIT( 0x4d, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

    PORT_START("IN1")
    PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

    PORT_START("DSW1")
    PORT_DIPUNKNOWN( 0x07, 0x07 )

    PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

    PORT_DIPNAME( 0x30, 0x10, "Balls" )
    PORT_DIPSETTING(    0x00, "200")
    PORT_DIPSETTING(    0x10, "100")
    PORT_DIPSETTING(    0x20, "50")
    PORT_DIPSETTING(    0x30, "25")

    PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) )
    PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ))
    PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )

    PORT_START("DSW2")

    PORT_DIPNAME( 0x03, 0x01, "Time" )
    PORT_DIPSETTING(    0x00, "180")
    PORT_DIPSETTING(    0x01, "120")
    PORT_DIPSETTING(    0x02, "150")
    PORT_DIPSETTING(    0x03, "90")

    PORT_DIPUNKNOWN( 0x0c, 0x0c )

    PORT_DIPNAME( 0x30, 0x20, "Limit (attract)" ) /* attract mode only??? */
    PORT_DIPSETTING(    0x00, "2000")
    PORT_DIPSETTING(    0x10, "1500")
    PORT_DIPSETTING(    0x20, "1000")
    PORT_DIPSETTING(    0x30, "500")

    PORT_DIPNAME( 0xc0, 0xc0, "Limit (game)" )  /* ball limit in game */
    PORT_DIPSETTING(    0x00, "1500")
    PORT_DIPSETTING(    0x40, "1000")
    PORT_DIPSETTING(    0x80, "500")
    PORT_DIPSETTING(    0xc0, "300")

    PORT_START("DSW3")
    PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
    PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )

    PORT_DIPUNKNOWN( 0xfe, 0xfe )

    PORT_START("PLUNGER")
    PORT_BIT( 0x3f, 0x00, IPT_POSITIONAL ) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(30) PORT_KEYDELTA(4) PORT_CENTERDELTA(0xff)

INPUT_PORTS_END


#if USE_MSM

static UINT32 adpcm_pos;
static UINT8 adpcm_idle=0;

static void pf_adpcm_int(running_device *device)
{
    static UINT8 trigger,adpcm_data;

    if (adpcm_pos >= 0x4000 || adpcm_idle)
    {
        adpcm_idle = 1;
        msm5205_reset_w(device,1);
        trigger = 0;
    }
    else
    {
        UINT8 *ROM = memory_region(device->machine, "adpcm");

        adpcm_data = ((trigger ? (ROM[adpcm_pos] & 0x0f) : (ROM[adpcm_pos] & 0xf0)>>4) );
        msm5205_data_w(device,adpcm_data & 0xf);
        trigger^=1;
        if(trigger == 0)
        {
            adpcm_pos++;
            if((ROM[adpcm_pos] & 0xff) == 0xff)
              adpcm_idle = 1;
        }
    }
}

static const msm5205_interface msm5205_config =
{
    pf_adpcm_int,    /* interrupt function */
    MSM5205_S48_4B    /* 8kHz */
};

#endif

static MACHINE_RESET( pachifev )
{
    pachifev_state *state = (pachifev_state *)machine->driver_data;

    state->power=0;
    state->max_power=0;
    state->input_power=0;
    state->previous_power=0;
    state->cnt=0;

#if USE_MSM
    adpcm_pos = 0;
#endif
}


static INTERRUPT_GEN( pachifev_vblank_irq )
{
    TMS9928A_interrupt(device->machine);

    {
        pachifev_state *state = (pachifev_state *)device->machine->driver_data;
        int current_power=input_port_read(device->machine, "PLUNGER") & 0x3f;
        if(current_power != state->previous_power)
        {
            popmessage    ("%d%%", (current_power * 100) / 0x3f);
        }

        if( (!current_power) && (state->previous_power) )
        {
            state->input_power=state->previous_power;
            state->cnt=NUM_PLUNGER_REPEATS;
        }

        state->previous_power=current_power;
    }

}

static const TMS9928a_interface tms9928a_interface =
{
    TMS99x8A,
    0x4000,
    0, 0,
    0 /* no interrupt is generated */
};

static MACHINE_START( pachifev)
{
    /* configure VDP */
    TMS9928A_configure(&tms9928a_interface);
    {
        pachifev_state *state = (pachifev_state *)machine->driver_data;

        state_save_register_global(machine, state->power);
        state_save_register_global(machine, state->max_power);
        state_save_register_global(machine, state->input_power);
        state_save_register_global(machine, state->previous_power);
        state_save_register_global(machine, state->cnt);
    }
}

static const struct tms9995reset_param pachifev_processor_config =
{
    1,0,0
};

static MACHINE_DRIVER_START( pachifev )

    MDRV_DRIVER_DATA(pachifev_state)

    /* basic machine hardware */
    MDRV_CPU_ADD("maincpu", TMS9995, XTAL_12MHz)
    MDRV_CPU_CONFIG(pachifev_processor_config)
    MDRV_CPU_PROGRAM_MAP(pachifev_map)
    MDRV_CPU_IO_MAP(pachifev_cru)
    MDRV_CPU_VBLANK_INT("screen",pachifev_vblank_irq)

    MDRV_MACHINE_START(pachifev)
    MDRV_MACHINE_RESET(pachifev)

    /* video hardware */

    MDRV_IMPORT_FROM(tms9928a)
    MDRV_SCREEN_MODIFY("screen")
    MDRV_SCREEN_REFRESH_RATE((float)XTAL_10_738635MHz/2/342/262)
    MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */

    /* sound hardware */
    MDRV_SPEAKER_STANDARD_MONO("mono")
#if USE_MSM
    MDRV_SOUND_ADD("adpcm", MSM5205, 288000)  /* guess */
    MDRV_SOUND_CONFIG(msm5205_config)
    MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
#endif
    MDRV_SOUND_ADD("sn76", SN76489, XTAL_10_738635MHz/3) /* guess */
    MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END

ROM_START( pachifev )
    ROM_REGION( 0x10000, "maincpu", 0 )
    ROM_LOAD( "ic42.00",   0x00000, 0x2000, CRC(9653546e) SHA1(0836d01118241d38bbf61732275afe3ae47d0622) )
    ROM_LOAD( "ic43.01",   0x02000, 0x2000, CRC(5572dce5) SHA1(fad45b33e095ac6e3ed3d7cdc3d8678c153a1b38) )
    ROM_LOAD( "ic44.02",   0x04000, 0x2000, CRC(98b3841f) SHA1(0563139877bf01e1673767ee1798bbcf68adadea) )
    ROM_LOAD( "ic45.03",   0x06000, 0x2000, CRC(6b76e6fa) SHA1(5be10ab0b76e2061fc7e9c77649572955bee7661) )
    ROM_LOAD( "ic46.04",   0x08000, 0x2000, CRC(1c8c66d7) SHA1(3b9b05f35b20d798651c7d5fdb35e6af956615a1) )

    ROM_REGION( 0x4000, "adpcm", 0 )
    ROM_LOAD( "ic66.10",   0x0000, 0x2000, CRC(217c573e) SHA1(6fb90865d1d81f5ea00fa7916d0ccb6756ef5ce5) )

    ROM_REGION( 0x2000, "user1", 0 )
    ROM_LOAD( "ic48.50",   0x00000, 0x2000, BAD_DUMP CRC(1c8c66d7) SHA1(3b9b05f35b20d798651c7d5fdb35e6af956615a1) )
ROM_END

GAME( 1983, pachifev,  0,       pachifev,  pachifev,  0, ROT270, "Sanki Denshi Kogyo", "Pachifever", GAME_IMPERFECT_SOUND )

