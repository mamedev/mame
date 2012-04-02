/*
  Super Free Kick / Spinkick by HEC (Haesung Enterprise Co.)

    driver by Tomasz Slanina

  Hacked MSX2 home computer hardware. Romset contains
  modifed ( (c) strings removed and patched boot sequence)
  MSX2 bios. Yamaha VDP v9938 is hidden in huge epoxy block.
  There's also an additional Z80 to drive sound.


     1      2       3        4        5         6         7
+----------------------------------------------------------------+
|                                                                |
|  C1182  Y3014B       YM2203C           Z80A             a7     | A
|                                                                |
| 1UP     GL324                                                  | B
|                                                         c7     |
| 2UP                                     c5                     | C
|                                                                |
+-+       DSW1              74139        6116             d7     | D
  |                              21.47727 MHz                    |
+-+       74241         CN1              74244                   | E
|                                                                |
|         DSW2                           74373                   | G
|                                                                |
|         74241    4464     4464     7404     74139       h7     | H
| J                           CN2                                |
| A       74157    4464     4464     7400     74670              | J
| M                                                       j7     |
| M       40106    74169    7404     7432     74670              | K
| A                                                              |
|         74241    74169    74138    7432     7402        l7     | L
|                                                                |
+-+       74241    74153    74139    74138    74138      6264    | M
  |                                                              |
+-+       74241     82C55                 Z80A           6264    | N
|                                                                |
+----------------------------------------------------------------+

Board # CBK1029

CN1: 40 PIN Connector (Epoxy Block )
CN2: 8  PIN Connector (Epoxy Block)
1UP: 4 PIN Connector (Analog Controls ?)
2UP: 4 PIN Connector (Analog Controls ?)

Z8400A (x2)
UM82C55A-PC
YM2203C

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/v9938.h"
#include "machine/8255ppi.h"
#include "sound/2203intf.h"


class sfkick_state : public driver_device
{
public:
	sfkick_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_v9938(*this, "v9938") { }

	UINT8 *m_main_mem;
	int m_bank_cfg;
	int m_bank[8];
	int m_input_mux;
	required_device<v9938_device> m_v9938;
};


#define MSX2_XBORDER_PIXELS	16
#define MSX2_YBORDER_PIXELS	28
#define MSX2_TOTAL_XRES_PIXELS 256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS 212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS	8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS	14 * 2
#define MASTER_CLOCK	XTAL_21_4772MHz


static READ8_DEVICE_HANDLER( ppi_port_b_r )
{
	sfkick_state *state = device->machine().driver_data<sfkick_state>();
	switch(state->m_input_mux&0x0f)
	{
		case 0: return input_port_read(device->machine(), "IN0");
		case 1: return input_port_read(device->machine(), "IN1");
		case 2: return BITSWAP8(input_port_read(device->machine(), "DIAL"),4,5,6,7,3,2,1,0);
		case 3: return input_port_read(device->machine(), "DSW2");
		case 4: return input_port_read(device->machine(), "DSW1");
	}
	return 0xff;
}

static void sfkick_remap_banks(running_machine &machine)
{
	sfkick_state *state = machine.driver_data<sfkick_state>();
	/* 0000-3ffff */
	switch(state->m_bank_cfg&3)
	{
		case 0: /* bios */
		{
			UINT8 *mem = machine.region("bios")->base();
			memory_set_bankptr(machine,"bank1", mem);
			memory_set_bankptr(machine,"bank2", mem+0x2000);
		}
		break;

		case 1: /* ext rom */
		{
			UINT8 *mem = machine.region("extrom")->base();
			memory_set_bankptr(machine,"bank1", mem+0x4000);
			memory_set_bankptr(machine,"bank2", mem+0x6000);
		}
		break;

		case 2: /* banked */
		{
			UINT8 *mem = machine.region("banked")->base();
			memory_set_bankptr(machine,"bank1", mem+0x2000*state->m_bank[0]);
			memory_set_bankptr(machine,"bank2", mem+0x2000*state->m_bank[1]);
		}
		break;

		case 3: /* unknown */
		{
			UINT8 *mem = machine.region("banked")->base();
			memory_set_bankptr(machine,"bank1", mem+0x18000);
			memory_set_bankptr(machine,"bank2", mem+0x18000);
		}
		break;
	}

	/* 4000-7ffff */
	switch((state->m_bank_cfg>>2)&3)
	{
		case 0: /* bios - upper part */
		{
			UINT8 *mem = machine.region("bios")->base();
			memory_set_bankptr(machine,"bank3", mem+0x4000);
			memory_set_bankptr(machine,"bank4", mem+0x6000);
		}
		break;

		case 1:  /* unknown */
		case 3:
		{
			UINT8 *mem = machine.region("banked")->base();
			memory_set_bankptr(machine,"bank3", mem+0x18000);
			memory_set_bankptr(machine,"bank4", mem+0x18000);
		}
		break;

		case 2: /* banked */
		{
			UINT8 *mem = machine.region("banked")->base();
			memory_set_bankptr(machine,"bank3", mem+0x2000*state->m_bank[2]);
			memory_set_bankptr(machine,"bank4", mem+0x2000*state->m_bank[3]);
		}
		break;
	}

	/* 8000-bffff */
	switch((state->m_bank_cfg>>4)&3)
	{
		case 0: /* cartridge */
		{
			UINT8 *mem = machine.region("cartridge")->base();
			memory_set_bankptr(machine,"bank5", mem+0x4000);
			memory_set_bankptr(machine,"bank6", mem+0x6000);
		}
		break;

		case 1: /* unknown */
		case 3:
		{
			UINT8 *mem = machine.region("banked")->base();
			memory_set_bankptr(machine,"bank5", mem+0x18000);
			memory_set_bankptr(machine,"bank6", mem+0x18000);
		}
		break;

		case 2: /* banked */
		{
			UINT8 *mem = machine.region("banked")->base();
			memory_set_bankptr(machine,"bank5", mem+0x2000*state->m_bank[4]);
			memory_set_bankptr(machine,"bank6", mem+0x2000*state->m_bank[5]);
		}
		break;
	}

	/* c000-fffff */
	switch((state->m_bank_cfg>>6)&3)
	{
		case 0: /* unknown */
		case 1:
		{
			UINT8 *mem = machine.region("banked")->base();
			memory_set_bankptr(machine,"bank7", mem+0x18000);
			memory_set_bankptr(machine,"bank8", mem+0x18000);
		}
		break;

		case 2: /* banked */
		{
			UINT8 *mem = machine.region("banked")->base();
			memory_set_bankptr(machine,"bank7", mem+0x2000*state->m_bank[6]);
			memory_set_bankptr(machine,"bank8", mem+0x2000*state->m_bank[7]);
		}
		break;

		case 3: /* RAM */
		{
			memory_set_bankptr(machine,"bank7", state->m_main_mem);
			memory_set_bankptr(machine,"bank8", state->m_main_mem+0x2000);
		}
		break;
	}
}

static WRITE8_DEVICE_HANDLER ( ppi_port_a_w )
{
	sfkick_state *state = device->machine().driver_data<sfkick_state>();
	state->m_bank_cfg=data;
	sfkick_remap_banks(device->machine());
}

static void sfkick_bank_set(running_machine &machine,int num, int data)
{
	sfkick_state *state = machine.driver_data<sfkick_state>();
	/* ignore bit 1 */
	data&=0xf;
	num&=5;
	state->m_bank[num]=data;
	num|=2;
	state->m_bank[num]=data;
	sfkick_remap_banks(machine);
}

static WRITE8_HANDLER(page0_w)
{
	sfkick_state *state = space->machine().driver_data<sfkick_state>();
	if((state->m_bank_cfg&3)==2)
	{
		if(offset<0x2000)
		{
			sfkick_bank_set(space->machine(),0,data);
		}
		else
		{
			sfkick_bank_set(space->machine(),1,data);
		}
	}
}

static WRITE8_HANDLER(page1_w)
{
	sfkick_state *state = space->machine().driver_data<sfkick_state>();
	if(((state->m_bank_cfg>>2)&3)==2)
	{
		if(offset<0x2000)
		{
			sfkick_bank_set(space->machine(),2,data);
		}
		else
		{
			sfkick_bank_set(space->machine(),3,data);
		}
	}
}

static WRITE8_HANDLER(page2_w)
{
	sfkick_state *state = space->machine().driver_data<sfkick_state>();
	if(((state->m_bank_cfg>>4)&3)==2)
	{
		if(offset<0x2000)
		{
			sfkick_bank_set(space->machine(),4,data);
		}
		else
		{
			sfkick_bank_set(space->machine(),5,data);
		}
	}
}

static WRITE8_HANDLER(page3_w)
{
	sfkick_state *state = space->machine().driver_data<sfkick_state>();
	if(((state->m_bank_cfg>>6)&3)==2)
	{
		if(offset<0x2000)
		{
			sfkick_bank_set(space->machine(),6,data);
		}
		else
		{
			sfkick_bank_set(space->machine(),7,data);
		}
	}
	else
	{
		if(((state->m_bank_cfg>>6)&3)==3)
		{
			state->m_main_mem[offset]=data;
		}
	}
}



static ADDRESS_MAP_START( sfkick_map, AS_PROGRAM, 8, sfkick_state )
	AM_RANGE( 0x0000, 0x1fff) AM_ROMBANK("bank1")
	AM_RANGE( 0x2000, 0x3fff) AM_ROMBANK("bank2")
	AM_RANGE( 0x4000, 0x5fff) AM_ROMBANK("bank3")
	AM_RANGE( 0x6000, 0x7fff) AM_ROMBANK("bank4")
	AM_RANGE( 0x8000, 0x9fff) AM_ROMBANK("bank5")
	AM_RANGE( 0xa000, 0xbfff) AM_ROMBANK("bank6")
	AM_RANGE( 0xc000, 0xdfff) AM_ROMBANK("bank7")
	AM_RANGE( 0xe000, 0xffff) AM_ROMBANK("bank8")
	AM_RANGE( 0x0000, 0x3fff) AM_WRITE_LEGACY(page0_w )
	AM_RANGE( 0x4000, 0x7fff) AM_WRITE_LEGACY(page1_w )
	AM_RANGE( 0x8000, 0xbfff) AM_WRITE_LEGACY(page2_w )
	AM_RANGE( 0xc000, 0xffff) AM_WRITE_LEGACY(page3_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfkick_io_map, AS_IO, 8, sfkick_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0xa0, 0xa7) AM_WRITE_LEGACY(soundlatch_w )
	AM_RANGE( 0x98, 0x9b) AM_DEVREADWRITE( "v9938", v9938_device, read, write)
	AM_RANGE( 0xa8, 0xab) AM_DEVREADWRITE_LEGACY("ppi8255", ppi8255_r, ppi8255_w)
	AM_RANGE( 0xb4, 0xb5) AM_RAM /* loopback ? req by sfkicka (MSX Bios leftover)*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfkick_sound_map, AS_PROGRAM, 8, sfkick_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfkick_sound_io_map, AS_IO, 8, sfkick_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_LEGACY(soundlatch_r)
	AM_RANGE(0x04, 0x05) AM_DEVREADWRITE_LEGACY("ym1", ym2203_r, ym2203_w)
ADDRESS_MAP_END

static WRITE8_DEVICE_HANDLER ( ppi_port_c_w )
{
	sfkick_state *state = device->machine().driver_data<sfkick_state>();
	state->m_input_mux=data;
}

static const ppi8255_interface ppi8255_intf =
{
	DEVCB_NULL,
	DEVCB_HANDLER(ppi_port_b_r),
	DEVCB_NULL,
	DEVCB_HANDLER(ppi_port_a_w),
	DEVCB_NULL,
	DEVCB_HANDLER(ppi_port_c_w)

};

static INPUT_PORTS_START( sfkick )
    PORT_START("IN0")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED ) /* unused ? */
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

    PORT_START("IN1")
    PORT_BIT( 0x71, IP_ACTIVE_LOW, IPT_UNUSED ) /* unused ? */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

    PORT_START("DIAL")
    PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(-20)

    PORT_START("DSW1") /* bitswapped at read! 76543210 -> 45673210 */
    PORT_DIPNAME(   0x82, 0x02, DEF_STR( Unknown ) )  /* unknown, code @ $98a8 */
	PORT_DIPSETTING(      0x00, "3" )
	PORT_DIPSETTING(      0x02, "2" )
	PORT_DIPSETTING(      0x80, "1" )
	PORT_DIPSETTING(      0x82, "0" )
    PORT_DIPNAME(   0x0c, 0x08, DEF_STR( Difficulty ) ) /* not sure, code @ $9877 */
	PORT_DIPSETTING(      0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Hard ) )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* unused ? */
    PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x20, DEF_STR( On ) )
    PORT_DIPNAME( 0x41, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x00, "5" )
	PORT_DIPSETTING(      0x01, "3" )
	PORT_DIPSETTING(      0x40, "2" )
	PORT_DIPSETTING(      0x41, "1" )

    PORT_START("DSW2") /* bitswapped at read! 76543210 -> 45673210 */
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) /* unused ? */
    PORT_DIPNAME(   0x02, 0x02,  "Test Mode" )
    PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
    PORT_DIPNAME(   0x20, 0x20,  "Freeze" )
    PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
    PORT_DIPNAME(   0x8c, 0x8c, DEF_STR( Coinage ) )
    PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(    0x84, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 3C_2C ) )
    PORT_DIPSETTING(    0x8c, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x80, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0x88, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
INPUT_PORTS_END

static void sfkick_vdp_interrupt(device_t *, v99x8_device &device, int i)
{
	cputag_set_input_line (device.machine(), "maincpu", 0, (i ? HOLD_LINE : CLEAR_LINE));
}

static MACHINE_RESET(sfkick)
{
	sfkick_state *state = machine.driver_data<sfkick_state>();
	state->m_bank_cfg=0;
	state->m_bank[0]=0;
	state->m_bank[1]=0;
	state->m_bank[2]=0;
	state->m_bank[3]=0;
	state->m_bank[4]=0;
	state->m_bank[5]=0;
	state->m_bank[6]=0;
	state->m_bank[7]=0;
	sfkick_remap_banks(machine);
}

static TIMER_DEVICE_CALLBACK( sfkick_interrupt )
{
	sfkick_state *state = timer.machine().driver_data<sfkick_state>();
	state->m_v9938->interrupt();
}

static void irqhandler(device_t *device, int irq)
{
	cputag_set_input_line_and_vector(device->machine(), "soundcpu", 0, irq ? ASSERT_LINE : CLEAR_LINE, 0xff);
}

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL,DEVCB_NULL,DEVCB_NULL,DEVCB_NULL,
	},
	irqhandler
};

static MACHINE_CONFIG_START( sfkick, sfkick_state )

	MCFG_CPU_ADD("maincpu",Z80,MASTER_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(sfkick_map)
	MCFG_CPU_IO_MAP(sfkick_io_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", sfkick_interrupt, "screen", 0, 1)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000))

	MCFG_CPU_ADD("soundcpu",Z80,MASTER_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(sfkick_sound_map)
	MCFG_CPU_IO_MAP(sfkick_sound_io_map)

	MCFG_V9938_ADD("v9938", "screen", 0x80000)
	MCFG_V99X8_INTERRUPT_CALLBACK_STATIC(sfkick_vdp_interrupt)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DEVICE("v9938", v9938_device, screen_update)
	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, MSX2_TOTAL_YRES_PIXELS)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)

	MCFG_PALETTE_LENGTH(512)

	MCFG_PPI8255_ADD( "ppi8255", ppi8255_intf )

	MCFG_MACHINE_RESET(sfkick)

	MCFG_PALETTE_INIT( v9938 )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym1", YM2203, MASTER_CLOCK/6)
	MCFG_SOUND_CONFIG(ym2203_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.25)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

MACHINE_CONFIG_END

static DRIVER_INIT(sfkick)
{
	sfkick_state *state = machine.driver_data<sfkick_state>();
	state->m_main_mem=auto_alloc_array(machine, UINT8, 0x4000);
}




ROM_START( sfkick )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION(0x20000,  "banked", ROMREGION_ERASEFF)
	ROM_LOAD( "sfkick2.a7", 0x00000, 0x8000, CRC(1dcaec5e) SHA1(7e063d46fb6606df2d772866cc55f207035b98c4) )
	ROM_LOAD( "sfkick3.c7", 0x08000, 0x8000, CRC(639d3cf2) SHA1(950fd28058d32e4532eb6e99454dcaef092a955e) )
	ROM_LOAD( "sfkick4.d7", 0x10000, 0x8000, CRC(ee1b344e) SHA1(d33fbad017cc4838192e9c540621537edb7e8dc4) )
	/* 0x18000-0x1ffff = empty */

	ROM_REGION(0x8000,  "extrom", 0)
	ROM_LOAD( "sfkick5.h7", 0x00000, 0x8000, CRC(8e8bd9cf) SHA1(f493de40147fdd67a48d4c90b01170fbd6ea074e) )

	ROM_REGION(0x8000,  "cartridge", 0)
	ROM_LOAD( "sfkick6.j7", 0x0000, 0x8000, CRC(7a91ac4b) SHA1(afc5e2c2fe0cd108235ac6ae2775cc9a0b1c9f76) )

	ROM_REGION(0x8000,  "bios", 0)
	ROM_LOAD( "sfkick7.l7", 0x00000, 0x8000, CRC(8cd94c63) SHA1(e6dba66c8716593b8ab88f79f7205211938d1598) )

	ROM_REGION(0x10000,  "soundcpu", 0)
	ROM_LOAD( "sfkick1.c5", 0x00000, 0x8000, CRC(2f5e3b7a) SHA1(d2ff566b415ab10c0681fa1eb221a56e3c137ecf) )
ROM_END


ROM_START( sfkicka )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION(0x20000,  "banked", ROMREGION_ERASEFF)
	ROM_LOAD( "c145.bin", 0x00000, 0x8000, CRC(1dcaec5e) SHA1(7e063d46fb6606df2d772866cc55f207035b98c4) )
	ROM_LOAD( "c146.bin", 0x08000, 0x8000, CRC(57afc4c6) SHA1(ee28b3f74e3175c22f542855b09f1673d048b1fa) )
	ROM_LOAD( "c147.bin", 0x10000, 0x8000, CRC(ee1b344e) SHA1(d33fbad017cc4838192e9c540621537edb7e8dc4) )
	/* 0x18000-0x1ffff = empty */

	ROM_REGION(0x8000,  "extrom", 0)
	 ROM_LOAD( "c149.bin", 0x00000, 0x8000, CRC(2edbf61f) SHA1(23dcff43faf222a4b69001312ce4b1c920e2f4c2) )

	ROM_REGION(0x8000,  "cartridge", 0)
	ROM_LOAD( "c150.bin", 0x0000, 0x8000, CRC(20412918) SHA1(b0fefa957b20373ffb84d9ff97a2e84a9a3af56c) )

	ROM_REGION(0x8000,  "bios", 0)
	ROM_LOAD( "c151.bin", 0x00000, 0x8000, CRC(8cd94c63) SHA1(e6dba66c8716593b8ab88f79f7205211938d1598) )

	ROM_REGION(0x10000,  "soundcpu", 0)
	ROM_LOAD( "c130.bin", 0x00000, 0x8000, CRC(2f5e3b7a) SHA1(d2ff566b415ab10c0681fa1eb221a56e3c137ecf) )
ROM_END


ROM_START( spinkick )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION(0x20000,  "banked", ROMREGION_ERASEFF)
	ROM_LOAD( "spinkick.r2", 0x00000, 0x8000, CRC(1dcaec5e) SHA1(7e063d46fb6606df2d772866cc55f207035b98c4) )
	ROM_LOAD( "spinkick.r3", 0x08000, 0x8000, CRC(e86a194a) SHA1(19a02375ec463e795770403c3e948d754919458b) )
	ROM_LOAD( "spinkick.r4", 0x10000, 0x8000, CRC(ee1b344e) SHA1(d33fbad017cc4838192e9c540621537edb7e8dc4) )
	/* 0x18000-0x1ffff = empty */

	ROM_REGION(0x8000,  "extrom", 0)
	ROM_LOAD( "spinkick.r5", 0x00000, 0x8000, CRC(8e8bd9cf) SHA1(f493de40147fdd67a48d4c90b01170fbd6ea074e) )

	ROM_REGION(0x8000,  "cartridge", 0)
	ROM_LOAD( "spinkick.r6", 0x0000, 0x8000, CRC(7a91ac4b) SHA1(afc5e2c2fe0cd108235ac6ae2775cc9a0b1c9f76) )

	ROM_REGION(0x8000,  "bios", 0)
	ROM_LOAD( "spinkick.r7", 0x00000, 0x8000, CRC(8cd94c63) SHA1(e6dba66c8716593b8ab88f79f7205211938d1598) )

	ROM_REGION(0x10000,  "soundcpu", 0)
	ROM_LOAD( "spinkick.r1", 0x00000, 0x8000, CRC(2f5e3b7a) SHA1(d2ff566b415ab10c0681fa1eb221a56e3c137ecf) )
ROM_END


GAME( 1988, sfkick,   0,      sfkick, sfkick, sfkick, ROT90, "Haesung/HJ Corp", "Super Free Kick (set 1)", 0 )
GAME( 198?, sfkicka,  sfkick, sfkick, sfkick, sfkick, ROT90, "Haesung", "Super Free Kick (set 2)", 0 )
GAME( 1988, spinkick, sfkick, sfkick, sfkick, sfkick, ROT90, "Haesung/Seojin", "Hec's Spinkick", 0 )

