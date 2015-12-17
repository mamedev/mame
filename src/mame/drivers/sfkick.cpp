// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
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
#include "machine/i8255.h"
#include "sound/2203intf.h"


class sfkick_state : public driver_device
{
public:
	sfkick_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_v9938(*this, "v9938"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_region_bios(*this, "bios"),
		m_region_extrom(*this, "extrom"),
		m_region_banked(*this, "banked"),
		m_region_cartridge(*this, "cartridge"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_bank5(*this, "bank5"),
		m_bank6(*this, "bank6"),
		m_bank7(*this, "bank7"),
		m_bank8(*this, "bank8"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_dial(*this, "DIAL"),
		m_dsw1(*this, "DSW1"),
		m_dsw2(*this, "DSW2")
	{ }

	std::unique_ptr<UINT8[]> m_main_mem;
	int m_bank_cfg;
	int m_bank[8];
	int m_input_mux;
	required_device<v9938_device> m_v9938;
	DECLARE_WRITE8_MEMBER(page0_w);
	DECLARE_WRITE8_MEMBER(page1_w);
	DECLARE_WRITE8_MEMBER(page2_w);
	DECLARE_WRITE8_MEMBER(page3_w);
	DECLARE_READ8_MEMBER(ppi_port_b_r);
	DECLARE_WRITE8_MEMBER(ppi_port_a_w);
	DECLARE_WRITE8_MEMBER(ppi_port_c_w);
	DECLARE_DRIVER_INIT(sfkick);
	virtual void machine_reset() override;
	void sfkick_remap_banks();
	void sfkick_bank_set(int num, int data);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	DECLARE_WRITE_LINE_MEMBER(sfkick_vdp_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_memory_region m_region_bios;
	required_memory_region m_region_extrom;
	required_memory_region m_region_banked;
	required_memory_region m_region_cartridge;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_memory_bank m_bank5;
	required_memory_bank m_bank6;
	required_memory_bank m_bank7;
	required_memory_bank m_bank8;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_dial;
	required_ioport m_dsw1;
	required_ioport m_dsw2;
};


#define MASTER_CLOCK    XTAL_21_4772MHz


READ8_MEMBER(sfkick_state::ppi_port_b_r)
{
	switch(m_input_mux&0x0f)
	{
		case 0: return m_in0->read();
		case 1: return m_in1->read();
		case 2: return BITSWAP8(m_dial->read(),4,5,6,7,3,2,1,0);
		case 3: return m_dsw2->read();
		case 4: return m_dsw1->read();
	}
	return 0xff;
}

void sfkick_state::sfkick_remap_banks()
{
	/* 0000-3fff */
	switch(m_bank_cfg&3)
	{
		case 0: /* bios */
		{
			UINT8 *mem = m_region_bios->base();
			m_bank1->set_base(mem);
			m_bank2->set_base(mem+0x2000);
		}
		break;

		case 1: /* ext rom */
		{
			UINT8 *mem = m_region_extrom->base();
			m_bank1->set_base(mem+0x4000);
			m_bank2->set_base(mem+0x6000);
		}
		break;

		case 2: /* banked */
		{
			UINT8 *mem = m_region_banked->base();
			m_bank1->set_base(mem+0x2000*m_bank[0]);
			m_bank2->set_base(mem+0x2000*m_bank[1]);
		}
		break;

		case 3: /* unknown */
		{
			UINT8 *mem = m_region_banked->base();
			m_bank1->set_base(mem+0x18000);
			m_bank2->set_base(mem+0x18000);
		}
		break;
	}

	/* 4000-7fff */
	switch((m_bank_cfg>>2)&3)
	{
		case 0: /* bios - upper part */
		{
			UINT8 *mem = m_region_bios->base();
			m_bank3->set_base(mem+0x4000);
			m_bank4->set_base(mem+0x6000);
		}
		break;

		case 1:  /* unknown */
		case 3:
		{
			UINT8 *mem = m_region_banked->base();
			m_bank3->set_base(mem+0x18000);
			m_bank4->set_base(mem+0x18000);
		}
		break;

		case 2: /* banked */
		{
			UINT8 *mem = m_region_banked->base();
			m_bank3->set_base(mem+0x2000*m_bank[2]);
			m_bank4->set_base(mem+0x2000*m_bank[3]);
		}
		break;
	}

	/* 8000-bfff */
	switch((m_bank_cfg>>4)&3)
	{
		case 0: /* cartridge */
		{
			UINT8 *mem = m_region_cartridge->base();
			m_bank5->set_base(mem+0x4000);
			m_bank6->set_base(mem+0x6000);
		}
		break;

		case 1: /* unknown */
		case 3:
		{
			UINT8 *mem = m_region_banked->base();
			m_bank5->set_base(mem+0x18000);
			m_bank6->set_base(mem+0x18000);
		}
		break;

		case 2: /* banked */
		{
			UINT8 *mem = m_region_banked->base();
			m_bank5->set_base(mem+0x2000*m_bank[4]);
			m_bank6->set_base(mem+0x2000*m_bank[5]);
		}
		break;
	}

	/* c000-ffff */
	switch((m_bank_cfg>>6)&3)
	{
		case 0: /* unknown */
		case 1:
		{
			UINT8 *mem = m_region_banked->base();
			m_bank7->set_base(mem+0x18000);
			m_bank8->set_base(mem+0x18000);
		}
		break;

		case 2: /* banked */
		{
			UINT8 *mem = m_region_banked->base();
			m_bank7->set_base(mem+0x2000*m_bank[6]);
			m_bank8->set_base(mem+0x2000*m_bank[7]);
		}
		break;

		case 3: /* RAM */
		{
			m_bank7->set_base(m_main_mem.get());
			m_bank8->set_base(m_main_mem.get()+0x2000);
		}
		break;
	}
}

WRITE8_MEMBER(sfkick_state::ppi_port_a_w)
{
	m_bank_cfg=data;
	sfkick_remap_banks();
}

void sfkick_state::sfkick_bank_set(int num, int data)
{
	/* ignore bit 1 */
	data&=0xf;
	num&=5;
	m_bank[num]=data;
	num|=2;
	m_bank[num]=data;
	sfkick_remap_banks();
}

WRITE8_MEMBER(sfkick_state::page0_w)
{
	if((m_bank_cfg&3)==2)
	{
		if(offset<0x2000)
		{
			sfkick_bank_set(0,data);
		}
		else
		{
			sfkick_bank_set(1,data);
		}
	}
}

WRITE8_MEMBER(sfkick_state::page1_w)
{
	if(((m_bank_cfg>>2)&3)==2)
	{
		if(offset<0x2000)
		{
			sfkick_bank_set(2,data);
		}
		else
		{
			sfkick_bank_set(3,data);
		}
	}
}

WRITE8_MEMBER(sfkick_state::page2_w)
{
	if(((m_bank_cfg>>4)&3)==2)
	{
		if(offset<0x2000)
		{
			sfkick_bank_set(4,data);
		}
		else
		{
			sfkick_bank_set(5,data);
		}
	}
}

WRITE8_MEMBER(sfkick_state::page3_w)
{
	if(((m_bank_cfg>>6)&3)==2)
	{
		if(offset<0x2000)
		{
			sfkick_bank_set(6,data);
		}
		else
		{
			sfkick_bank_set(7,data);
		}
	}
	else
	{
		if(((m_bank_cfg>>6)&3)==3)
		{
			m_main_mem[offset]=data;
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
	AM_RANGE( 0x0000, 0x3fff) AM_WRITE(page0_w )
	AM_RANGE( 0x4000, 0x7fff) AM_WRITE(page1_w )
	AM_RANGE( 0x8000, 0xbfff) AM_WRITE(page2_w )
	AM_RANGE( 0xc000, 0xffff) AM_WRITE(page3_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfkick_io_map, AS_IO, 8, sfkick_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0xa0, 0xa7) AM_WRITE(soundlatch_byte_w )
	AM_RANGE( 0x98, 0x9b) AM_DEVREADWRITE( "v9938", v9938_device, read, write)
	AM_RANGE( 0xa8, 0xab) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE( 0xb4, 0xb5) AM_RAM /* loopback ? req by sfkicka (MSX Bios leftover)*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfkick_sound_map, AS_PROGRAM, 8, sfkick_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfkick_sound_io_map, AS_IO, 8, sfkick_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x04, 0x05) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
ADDRESS_MAP_END

WRITE8_MEMBER(sfkick_state::ppi_port_c_w)
{
	m_input_mux=data;
}

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

WRITE_LINE_MEMBER(sfkick_state::sfkick_vdp_interrupt)
{
	m_maincpu->set_input_line(0, (state ? HOLD_LINE : CLEAR_LINE));
}

void sfkick_state::machine_reset()
{
	m_bank_cfg=0;
	m_bank[0]=0;
	m_bank[1]=0;
	m_bank[2]=0;
	m_bank[3]=0;
	m_bank[4]=0;
	m_bank[5]=0;
	m_bank[6]=0;
	m_bank[7]=0;
	sfkick_remap_banks();
}

WRITE_LINE_MEMBER(sfkick_state::irqhandler)
{
	m_soundcpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xff);
}

static MACHINE_CONFIG_START( sfkick, sfkick_state )

	MCFG_CPU_ADD("maincpu",Z80,MASTER_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(sfkick_map)
	MCFG_CPU_IO_MAP(sfkick_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000))

	MCFG_CPU_ADD("soundcpu",Z80,MASTER_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(sfkick_sound_map)
	MCFG_CPU_IO_MAP(sfkick_sound_io_map)

	MCFG_V9938_ADD("v9938", "screen", 0x80000, MASTER_CLOCK)
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(sfkick_state,sfkick_vdp_interrupt))
	MCFG_V99X8_SCREEN_ADD_NTSC("screen", "v9938", MASTER_CLOCK)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(sfkick_state, ppi_port_a_w))
	MCFG_I8255_IN_PORTB_CB(READ8(sfkick_state, ppi_port_b_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(sfkick_state, ppi_port_c_w))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym1", YM2203, MASTER_CLOCK/6)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(sfkick_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.25)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(sfkick_state,sfkick)
{
	m_main_mem=std::make_unique<UINT8[]>(0x4000);
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


GAME( 1988, sfkick,   0,      sfkick, sfkick, sfkick_state, sfkick, ROT90, "Haesung/HJ Corp", "Super Free Kick (set 1)", 0 )
GAME( 198?, sfkicka,  sfkick, sfkick, sfkick, sfkick_state, sfkick, ROT90, "Haesung", "Super Free Kick (set 2)", 0 )
GAME( 1988, spinkick, sfkick, sfkick, sfkick, sfkick_state, sfkick, ROT90, "Haesung/Seojin", "Hec's Spinkick", 0 )
