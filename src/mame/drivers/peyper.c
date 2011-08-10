#include "emu.h"
#include "cpu/z80/z80.h"
#include "peyper.lh"

class peyper_state : public driver_device
{
public:
	peyper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
   
    UINT8 irq_state;

    UINT8 display_block;
    UINT8 display[16];
};


static READ8_HANDLER(sw_r)
{
    return 0xff;
}

static WRITE8_HANDLER(col_w)
{
    peyper_state *state = space->machine().driver_data<peyper_state>();
    if (data==0x90) state->display_block = 0;
}

static const UINT8 hex_to_7seg[16] =
    {0x3F, 0x06, 0x5B, 0x4F,
     0x66, 0x6D, 0x7D, 0x07,
     0x7F, 0x6F, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00 };

static WRITE8_HANDLER(disp_w)
{
    peyper_state *state = space->machine().driver_data<peyper_state>();
    state->display[state->display_block] = data;
   
    UINT8 a = data & 0x0f;
    UINT8 b = data >> 4;
    UINT8 hex_a = hex_to_7seg[a];
    UINT8 hex_b = hex_to_7seg[b];
/*   
0 -> XA0 DPL25,DPL27
1 -> XA1 DPL26,DPL28
2 -> DPL23,DPL5
3 -> DPL22,DPL4
4 -> DPL21,DPL3
5 -> DPL20,DPL2
6 -> DPL19,DPL1
7 -> DPL30,DPL33
*/
    switch(state->display_block) {
        case 0 :                
                output_set_indexed_value("dpl_",25,hex_a);
                output_set_indexed_value("dpl_",27,hex_b);
                break;
        case 1 :                
                output_set_indexed_value("dpl_",26,hex_a);
                output_set_indexed_value("dpl_",28,hex_b);
                break;
        case 2 :                
                output_set_indexed_value("dpl_",23,hex_a);
                output_set_indexed_value("dpl_",5,hex_b);
                break;
        case 3 :                
                output_set_indexed_value("dpl_",22,hex_a);
                output_set_indexed_value("dpl_",4,hex_b);
                break;
        case 4 :                
                output_set_indexed_value("dpl_",21,hex_a);
                output_set_indexed_value("dpl_",3,hex_b);
                break;
        case 5 :                
                output_set_indexed_value("dpl_",20,hex_a);
                output_set_indexed_value("dpl_",2,hex_b);
                break;
        case 6 :                
                output_set_indexed_value("dpl_",19,hex_a);
                output_set_indexed_value("dpl_",1,hex_b);
                break;
        case 7 :                
                output_set_indexed_value("dpl_",30,hex_a);
                output_set_indexed_value("dpl_",33,hex_b);
                break;   
/*
8 ->  XB0
9 ->  XB1
10 -> DPL11,DPL17
11 -> DPL10,DPL16
12 -> DPL09,DPL15
13 -> DPL08,DPL14
14 -> DPL07,DPL13
15 -> DPL31,DPL32
*/
        case 8 :                
                /*
                if (BIT(a,3)) logerror("TILT\n");
                if (BIT(a,2)) logerror("ONC\n");
                if (BIT(a,1)) logerror("GAME OVER\n");
                if (BIT(a,0)) logerror("BALL IN PLAY\n");
                */
                output_set_indexed_value("led_",1,BIT(b,0)); // PLAYER 1
                output_set_indexed_value("led_",2,BIT(b,1)); // PLAYER 2
                output_set_indexed_value("led_",3,BIT(b,2)); // PLAYER 3
                output_set_indexed_value("led_",4,BIT(b,3)); // PLAYER 4               
                break;   
        case 9 :
                if (!BIT(b,0)) output_set_indexed_value("dpl_",6,hex_to_7seg[0]);
                if (!BIT(b,1)) output_set_indexed_value("dpl_",12,hex_to_7seg[0]);
                if (!BIT(b,2)) output_set_indexed_value("dpl_",24,hex_to_7seg[0]);
                if (!BIT(b,3)) output_set_indexed_value("dpl_",18,hex_to_7seg[0]);
                output_set_indexed_value("dpl_",29,hex_a);       
                break;   
        case 10 :                
                output_set_indexed_value("dpl_",11,hex_a);
                output_set_indexed_value("dpl_",17,hex_b);
                break;   
        case 11 :                
                output_set_indexed_value("dpl_",10,hex_a);
                output_set_indexed_value("dpl_",16,hex_b);
                break;   
        case 12 :                
                output_set_indexed_value("dpl_",9,hex_a);
                output_set_indexed_value("dpl_",15,hex_b);
                break;   
        case 13 :                
                output_set_indexed_value("dpl_",8,hex_a);
                output_set_indexed_value("dpl_",14,hex_b);
                break;   
        case 14 :                
                output_set_indexed_value("dpl_",7,hex_a);
                output_set_indexed_value("dpl_",13,hex_b);
                break;   
        case 15 :                
                output_set_indexed_value("dpl_",31,hex_a);
                output_set_indexed_value("dpl_",32,hex_b);
                break;   
    }

    state->display_block++;
    state->display_block&=0x0f;
}

static WRITE8_HANDLER(lamp_w)
{
    //logerror("lamp_w %02x\n",data);
    //logerror("[%d]= %02x\n",4+offset/4,data);
}

static WRITE8_HANDLER(lamp7_w)
{
    //logerror("[7]= %02x\n",data);
}

static WRITE8_HANDLER(sol_w)
{
    //logerror("sol_w %02x\n",data);
}


static ADDRESS_MAP_START( peyper_map, AS_PROGRAM, 8 )
//	AM_RANGE(0x0000, 0xffff) AM_NOP
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x5FFF) AM_ROM
	AM_RANGE(0x6000, 0x67FF) AM_RAM //AM_BASE_GENERIC(nvram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( peyper_io, AS_IO, 8 )
//	AM_RANGE(0x0000, 0xffff) AM_NOP
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(sw_r,disp_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(col_w)
//	AM_RANGE(0x04, 0x04) AM_DEVWRITE("ay8910_0", ay8910_address_w)
//	AM_RANGE(0x06, 0x06) AM_DEVWRITE("ay8910_0", ay8910_data_w)
//	AM_RANGE(0x08, 0x08) AM_DEVWRITE("ay8910_1", ay8910_address_w)
//	AM_RANGE(0x0a, 0x0a) AM_DEVWRITE("ay8910_1", ay8910_data_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(sol_w)
	AM_RANGE(0x10, 0x18) AM_WRITE(lamp_w)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("DSW0")
	AM_RANGE(0x24, 0x24) AM_READ_PORT("DSW1")   
	AM_RANGE(0x28, 0x28) AM_READ_PORT("SW0")
	AM_RANGE(0x2c, 0x2c) AM_WRITE(lamp7_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( peyper )
	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // N.C.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // N.C.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT ) // Tilt
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )  // Start game
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) // Small coin
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) // Medium coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 ) // Big coin
	
	PORT_START("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "Extra Ball"  ) // Bola Extra
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Premio Loteria" ) // Premio Loteria
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Reclamo" ) // Reclamo
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x00, "Partidas/Moneda" ) // Partidas/Moneda
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x18, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bolas Partida" ) // Bolas Partida
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPNAME( 0x03, 0x00, "Premios / Puntos" ) // Premios / Puntos
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "NC" ) // N.C.
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "NC" ) //  N.C.
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "NC" ) //  N.C.
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "NC" ) //  N.C.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "NC" ) //  N.C.
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Visualizac. H. Funcion" ) // Visualizac. H. Funcion
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Visualizac. Monederos" ) // Visualizac. Monederos
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x01, 0x00, "Visualizac. Partidas" ) // Visualizac. Partidas
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


static MACHINE_RESET( peyper )
{
    peyper_state *state = machine.driver_data<peyper_state>();
    state->irq_state = 0;
}

static MACHINE_CONFIG_START( peyper, peyper_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2500000)
	MCFG_CPU_PROGRAM_MAP(peyper_map)
	MCFG_CPU_IO_MAP(peyper_io)
    MCFG_CPU_PERIODIC_INT(irq0_line_hold, 1250 * 2)

	MCFG_MACHINE_RESET( peyper )

    /* video hardware */
    MCFG_DEFAULT_LAYOUT(layout_peyper)
MACHINE_CONFIG_END


static DRIVER_INIT( peyper )
{
}


/*-------------------------------------------------------------------
/ Odisea Paris-Dakar (1987)
/-------------------------------------------------------------------*/
ROM_START(odisea)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("odiseaa.bin", 0x0000, 0x2000, CRC(29a40242) SHA1(321e8665df424b75112589fc630a438dc6f2f459))
	ROM_LOAD("odiseab.bin", 0x2000, 0x2000, CRC(8bdf7c17) SHA1(7202b4770646fce5b2ba9e3b8ca097a993123b14))
	ROM_LOAD("odiseac.bin", 0x4000, 0x2000, CRC(832dee5e) SHA1(9b87ffd768ab2610f2352adcf22c4a7880de47ab))
ROM_END

/*-------------------------------------------------------------------
/ Wolf Man (1987)
/-------------------------------------------------------------------*/
ROM_START(wolfman)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("memoriaa.bin", 0x0000, 0x2000, CRC(1fec83fe) SHA1(5dc887d0fa00129ae31451c03bfe442f87dd2f54))
	ROM_LOAD("memoriab.bin", 0x2000, 0x2000, CRC(62a1e3ec) SHA1(dc472c7c9d223820f8f1031c92e36890c1fcba7d))
	ROM_LOAD("memoriac.bin", 0x4000, 0x2000, CRC(468f16f0) SHA1(66ce0464d82331cfc0ac1f6fbd871066e4e57262))
ROM_END


/*-------------------------------------------------------------------
/ Night Fever (1979)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Odin De Luxe (1985)
/-------------------------------------------------------------------*/
ROM_START(odin_dlx)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1a.bin", 0x0000, 0x2000, CRC(4fca9bfc) SHA1(05dce75919375d01a306aef385bcaac042243695))
	ROM_LOAD("2a.bin", 0x2000, 0x2000, CRC(46744695) SHA1(fdbd8a93b3e4a9697e77e7d381759829b86fe28b))
ROM_END

/*-------------------------------------------------------------------
/ Solar Wars (1986)
/-------------------------------------------------------------------*/
ROM_START(solarwap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("solarw1c.bin", 0x0000, 0x2000, CRC(aa6bf0cd) SHA1(7332a4b1679841283d846f3e4f1792cb8e9529bf))
	ROM_LOAD("solarw2.bin",  0x2000, 0x2000, CRC(95e2cbb1) SHA1(f9ab3222ca0b9e0796030a7a618847a4e8f77957))
ROM_END

/*-------------------------------------------------------------------
/ Gamatron (1986)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Pole Position (1987)
/-------------------------------------------------------------------*/
ROM_START(poleposn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x2000, CRC(fdd37f6d) SHA1(863fef32ab9b5f3aca51788b6be9373a01fa0698))
	ROM_LOAD("2.bin", 0x2000, 0x2000, CRC(967cb72b) SHA1(adef17018e2caf65b64bbfef72fe159b9704c409))
	ROM_LOAD("3.bin", 0x4000, 0x2000, CRC(461fe9ca) SHA1(01bf35550e2c55995f167293746f355cfd484af1))
ROM_END

/*-------------------------------------------------------------------
/ Star Wars (1987)
/-------------------------------------------------------------------*/
ROM_START(sonstwar)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("sw1.bin", 0x0000, 0x2000, CRC(a2555d92) SHA1(5c82be85bf097e94953d11c0d902763420d64de4))
	ROM_LOAD("sw2.bin", 0x2000, 0x2000, CRC(c2ae34a7) SHA1(0f59242e3aec5da7111e670c4d7cf830d0030597))
	ROM_LOAD("sw3.bin", 0x4000, 0x2000, CRC(aee516d9) SHA1(b50e54d4d5db59e3fb71fb000f9bc5e34ff7de9c))
ROM_END

/*-------------------------------------------------------------------
/ Hang-On (1988)
/-------------------------------------------------------------------*/


GAME( 1987, odisea,   0, peyper, peyper, peyper, ROT0, "Peyper", "Odisea Paris-Dakar",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1987, wolfman,  0, peyper, peyper, peyper, ROT0, "Peyper", "Wolf Man",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1985, odin_dlx, 0, peyper, peyper, peyper, ROT0, "Sonic", "Odin De Luxe",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1986, solarwap, 0, peyper, peyper, peyper, ROT0, "Sonic", "Solar Wars (Sonic)",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1987, poleposn, 0, peyper, peyper, peyper, ROT0, "Sonic", "Pole Position (Sonic)",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1987, sonstwar, 0, peyper, peyper, peyper, ROT0, "Sonic", "Star Wars (Sonic)",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
