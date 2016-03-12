// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Chris Hardy, David Haywood, Tomasz Slanina
/******************************************************************************************

PC-XT (c) 1987 IBM

(Actually Arcade games running on more or less modified PC-XT HW)

driver by Angelo Salese & Chris Hardy
original tetriunk.c by David Haywood & Tomasz Slanina

Notes:
- The Korean Tetris is a blantant rip-off of the Mirrorsoft/Andromeda Software Tetris PC
  version;

TODO:
- 02851: tetriskr: Corrupt game graphics after some time of gameplay, caused by a wrong
  reading of the i/o $3c8 bit 1.
- Add a proper FDC device.
- Filetto: Add UM5100 sound chip, might be connected to the prototyping card;
- buzzer sound has issues in both games

********************************************************************************************
Filetto HW notes:
The PCB is a un-modified IBM-PC with a CGA adapter & a prototyping card that controls the
interface between the pc and the Jamma connectors.Additionally there's also a UM5100 sound
chip for the sound.
PCB Part Number: S/N 90289764 NOVARXT
PCB Contents:
1x UMC 8923S-UM5100 voice processor (upper board)
1x MMI PAL16L8ACN-940CRK9 (upper board)
1x AMD AMPAL16R8APC-8804DM (upper board)
1x AMD P8088-1 main processor 8.000MHz (lower board)
1x Proton PT8010AF PLCC 28.636MHz (lower board)
1x UMC 8928LP-UM8272A floppy disk controller (lower board)
1x UMC 8935CS-UM82C11 Printer Adapter Interface (lower board)
1x UMC 8936CS-UM8250B Programmable asynchronous communications element (lower board)
There isn't any keyboard found connected to the pcb.
********************************************************************************************
Filetto SW notes:
The software of this game can be extracted with a normal Windows program extractor.
The files names are:
-command.com  (1)
-ibmbio.com   (1)
-ibmdos.com   (1)
-ansi.sys     (1)
-config.sys   (2)
-autoexec.bat (3)
-x.exe        (4)
(1)This is an old Italian version of MS-DOS (v3.30 18th March 1987).
(2)Contains "device=ansi.sys",it's an hook-up for the graphics used by the BIOS.
(3)It has an Echo off (as you can notice from the game itself) and then the loading of the
main program (x.exe).
(4)The main program,done in plain Basic with several Italian comments in it.The date of
the main program is 9th October 1990.

******************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "sound/hc55516.h"
#include "machine/bankdev.h"
#include "machine/genpc.h"

class pcxt_state : public driver_device
{
public:
	pcxt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_bank(*this, "bank"){ }

	int m_lastvalue;
	UINT8 m_disk_data[2];
	UINT8 m_port_b_data;
	UINT8 m_status;
	UINT8 m_clr_status;

	DECLARE_READ8_MEMBER(disk_iobank_r);
	DECLARE_WRITE8_MEMBER(disk_iobank_w);
	DECLARE_READ8_MEMBER(fdc765_status_r);
	DECLARE_READ8_MEMBER(fdc765_data_r);
	DECLARE_WRITE8_MEMBER(fdc765_data_w);
	DECLARE_WRITE8_MEMBER(fdc_dor_w);
	DECLARE_READ8_MEMBER(port_a_r);
	DECLARE_READ8_MEMBER(port_b_r);
	DECLARE_READ8_MEMBER(port_c_r);
	DECLARE_WRITE8_MEMBER(port_b_w);

	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<pc_noppi_mb_device> m_mb;
	optional_device<address_map_bank_device> m_bank;
};


class isa8_cga_filetto_device : public isa8_cga_device
{
public:
	// construction/destruction
	isa8_cga_filetto_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual const rom_entry *device_rom_region() const override;
};

const device_type ISA8_CGA_FILETTO = &device_creator<isa8_cga_filetto_device>;

//-------------------------------------------------
//  isa8_cga_filetto_device - constructor
//-------------------------------------------------

isa8_cga_filetto_device::isa8_cga_filetto_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_FILETTO, "ISA8_CGA_FILETTO", tag, owner, clock, "filetto_cga", __FILE__)
{
}

ROM_START( filetto_cga )
	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD("u67.bin", 0x0000, 0x2000, CRC(09710122) SHA1(de84bdd9245df287bbd3bb808f0c3531d13a3545) )
ROM_END

const rom_entry *isa8_cga_filetto_device::device_rom_region() const
{
	return ROM_NAME( filetto_cga );
}



class isa8_cga_tetriskr_device : public isa8_cga_superimpose_device
{
public:
	// construction/destruction
	isa8_cga_tetriskr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual void device_start() override;
	virtual const rom_entry *device_rom_region() const override;

	DECLARE_READ8_MEMBER(bg_bank_r);
	DECLARE_WRITE8_MEMBER(bg_bank_w);
private:
	UINT8 m_bg_bank;
};


/* for superimposing CGA over a different source video (i.e. tetriskr) */
const device_type ISA8_CGA_TETRISKR = &device_creator<isa8_cga_tetriskr_device>;

//-------------------------------------------------
//  isa8_cga_tetriskr_device - constructor
//-------------------------------------------------

isa8_cga_tetriskr_device::isa8_cga_tetriskr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_superimpose_device( mconfig, ISA8_CGA_TETRISKR, "ISA8_CGA_TETRISKR", tag, owner, clock, "tetriskr_cga", __FILE__)
{
}


void isa8_cga_tetriskr_device::device_start()
{
	m_bg_bank = 0;
	isa8_cga_superimpose_device::device_start();
	m_isa->install_device(0x3c0, 0x3c0, 0, 0,  read8_delegate( FUNC(isa8_cga_tetriskr_device::bg_bank_r), this ), write8_delegate( FUNC(isa8_cga_tetriskr_device::bg_bank_w), this ) );
}

WRITE8_MEMBER(isa8_cga_tetriskr_device::bg_bank_w)
{
	m_bg_bank = (data & 0x0f) ^ 8;
}

READ8_MEMBER(isa8_cga_tetriskr_device::bg_bank_r)
{
	return 0xff;
}


UINT32 isa8_cga_tetriskr_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int yi;
	const UINT8 *bg_rom = memregion("gfx2")->base();

	//popmessage("%04x",m_start_offs);

	bitmap.fill(rgb_t::black, cliprect);

	for(y=0;y<200/8;y++)
	{
		for(yi=0;yi<8;yi++)
		{
			for(x=0;x<320/8;x++)
			{
				UINT8 color;
				int xi,pen_i;

				for(xi=0;xi<8;xi++)
				{
					color = 0;
					/* TODO: first byte seems bogus? */
					for(pen_i = 0;pen_i<4;pen_i++)
						color |= ((bg_rom[y*320/8+x+(pen_i*0x20000)+yi*0x400+m_bg_bank*0x2000+1] >> (7-xi)) & 1) << pen_i;

					if(cliprect.contains(x*8+xi, y*8+yi))
						bitmap.pix32(y*8+yi, x*8+xi) = m_palette->pen(color);
				}
			}
		}
	}

	isa8_cga_device::screen_update(screen, bitmap, cliprect);
	return 0;
}

ROM_START( tetriskr_cga )
	ROM_REGION( 0x2000, "gfx1",ROMREGION_ERASE00 ) /* gfx - 1bpp font*/
	ROM_LOAD( "b-3.u36", 0x1800, 0x0800, CRC(1a636f9a) SHA1(a356cc57914d0c9b9127670b55d1f340e64b1ac9) )
	ROM_IGNORE( 0x1800 )

	ROM_REGION( 0x80000+1, "gfx2",ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "b-1.u59", 0x00000, 0x10000, CRC(4719d986) SHA1(6e0499944b968d96fbbfa3ead6237d69c769d634))
	ROM_LOAD( "b-2.u58", 0x10000, 0x10000, CRC(599e1154) SHA1(14d99f90b4fedeab0ac24ffa9b1fd9ad0f0ba699))
	ROM_LOAD( "b-4.u54", 0x20000, 0x10000, CRC(e112c450) SHA1(dfdecfc6bd617ec520b7563b7caf44b79d498bd3))
	ROM_LOAD( "b-5.u53", 0x30000, 0x10000, CRC(050b7650) SHA1(5981dda4ed43b6e81fbe48bfba90a8775d5ecddf))
	ROM_LOAD( "b-6.u49", 0x40000, 0x10000, CRC(d596ceb0) SHA1(8c82fb638688971ef11159a6b240253e63f0949d))
	ROM_LOAD( "b-7.u48", 0x50000, 0x10000, CRC(79336b6c) SHA1(7a95875f3071bdc3ee25c0e6a5a3c00ef02dc977))
	ROM_LOAD( "b-8.u44", 0x60000, 0x10000, CRC(1f82121a) SHA1(106da0f39f1260d0761217ed0a24c1611bfd7f05))
	ROM_LOAD( "b-9.u43", 0x70000, 0x10000, CRC(4ea22349) SHA1(14dfd3dbd51f8bd6f3290293b8ea1c165e8cf7fd))
ROM_END

const rom_entry *isa8_cga_tetriskr_device::device_rom_region() const
{
	return ROM_NAME( tetriskr_cga );
}

READ8_MEMBER(pcxt_state::disk_iobank_r)
{
	//printf("Read Prototyping card [%02x] @ PC=%05x\n",offset,space.device().safe_pc());
	//if(offset == 0) return ioport("DSW")->read();
	if(offset == 1) return ioport("IN1")->read();

	return m_disk_data[offset];
}

WRITE8_MEMBER(pcxt_state::disk_iobank_w)
{
/*
    BIOS does a single out $0310,$F0 on reset

    Then does 2 outs to set the bank..

        X1  X2

        $F0 $F2 = m0
        $F1 $F2 = m1
        $F0 $F3 = m2
        $F1 $F3 = m3

    The sequence of

    out $0310,X1
    out $0310,X2

    sets the selected rom that appears in $C0000-$CFFFF

*/
	int bank = 0;

//  printf("bank %d set to %02X\n", offset,data);

	if (data == 0xF0)
	{
		bank = 0;
	}
	else
	{
		if((m_lastvalue == 0xF0) && (data == 0xF2))
			bank = 0;
		else if ((m_lastvalue == 0xF1) && (data == 0xF2))
			bank = 1;
		else if ((m_lastvalue == 0xF0) && (data == 0xF3))
			bank = 2;
		else if ((m_lastvalue == 0xF1) && (data == 0xF3))
			bank = 3;
	}

	m_bank->set_bank(bank);

	m_lastvalue = data;

	m_disk_data[offset] = data;
}

READ8_MEMBER(pcxt_state::port_a_r)
{
	return 0xaa;//harmless keyboard error occurs without this
}

READ8_MEMBER(pcxt_state::port_b_r)
{
	return m_port_b_data;
}

READ8_MEMBER(pcxt_state::port_c_r)
{
	return 0x00;// DIPS?
}

/*'buzzer' sound routes here*/
/* Filetto uses this for either beep and um5100 sound routing,probably there's a mux somewhere.*/
/* The Korean Tetris uses it as a regular buzzer,probably the sound is all in there...*/
WRITE8_MEMBER(pcxt_state::port_b_w)
{
	m_mb->m_pit8253->write_gate2(BIT(data, 0));
	m_mb->pc_speaker_set_spkrdata(BIT(data, 1));
	m_port_b_data = data;
// device_t *cvsd = machine().device("cvsd");
//  hc55516_digit_w(cvsd, data);
}

/*Floppy Disk Controller 765 device*/
/*Currently we only emulate it at a point that the BIOS will pass the checks*/

#define FDC_BUSY 0x10
#define FDC_WRITE 0x40
#define FDC_READ 0x00 /*~0x40*/

READ8_MEMBER(pcxt_state::fdc765_status_r)
{
	UINT8 tmp;
	tmp = m_status | 0x80;
	m_clr_status++;
	if(m_clr_status == 0x10)
	{
		m_status = 0;
		m_clr_status = 0;
	}
	return tmp;
}

READ8_MEMBER(pcxt_state::fdc765_data_r)
{
	m_status = (FDC_READ);
	m_mb->m_pic8259->ir6_w(0);
	return 0xc0;
}

WRITE8_MEMBER(pcxt_state::fdc765_data_w)
{
	m_status = (FDC_WRITE);
}


WRITE8_MEMBER(pcxt_state::fdc_dor_w)
{
	m_mb->m_pic8259->ir6_w(1);
}

static ADDRESS_MAP_START( filetto_map, AS_PROGRAM, 8, pcxt_state )
	AM_RANGE(0xc0000, 0xcffff) AM_DEVICE("bank", address_map_bank_device, amap8)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( filetto_io, AS_IO, 8, pcxt_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x0060, 0x0060) AM_READ(port_a_r)  //not a real 8255
	AM_RANGE(0x0061, 0x0061) AM_READWRITE(port_b_r, port_b_w)
	AM_RANGE(0x0062, 0x0062) AM_READ(port_c_r)
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE("mb", pc_noppi_mb_device, map)
	AM_RANGE(0x0201, 0x0201) AM_READ_PORT("COIN") //game port
	AM_RANGE(0x0310, 0x0311) AM_READWRITE(disk_iobank_r,disk_iobank_w) //Prototyping card
	AM_RANGE(0x0312, 0x0312) AM_READ_PORT("IN0") //Prototyping card,read only
	AM_RANGE(0x03f2, 0x03f2) AM_WRITE(fdc_dor_w)
	AM_RANGE(0x03f4, 0x03f4) AM_READ(fdc765_status_r) //765 Floppy Disk Controller (FDC) Status
	AM_RANGE(0x03f5, 0x03f5) AM_READWRITE(fdc765_data_r,fdc765_data_w)//FDC Data
ADDRESS_MAP_END

static ADDRESS_MAP_START( tetriskr_map, AS_PROGRAM, 8, pcxt_state )
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tetriskr_io, AS_IO, 8, pcxt_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE("mb", pc_noppi_mb_device, map)
	AM_RANGE(0x03c8, 0x03c8) AM_READ_PORT("IN0")
	AM_RANGE(0x03c9, 0x03c9) AM_READ_PORT("IN1")
//  AM_RANGE(0x03ce, 0x03ce) AM_READ_PORT("IN1") //read then discarded?
ADDRESS_MAP_END

static ADDRESS_MAP_START( bank_map, AS_0, 8, pcxt_state )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM AM_REGION("game_prg", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( filetto )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Extra Play" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, "Play at 6th match reached" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tetriskr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) //probably unused
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_START("IN1") //dip-switches
	PORT_DIPNAME( 0x03, 0x03, "Starting Level" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Starting Bomb" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) ) duplicate
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void pcxt_state::machine_reset()
{
	m_lastvalue = -1;
}

static SLOT_INTERFACE_START( filetto_isa8_cards )
	SLOT_INTERFACE_INTERNAL("filetto",  ISA8_CGA_FILETTO)
	SLOT_INTERFACE_INTERNAL("tetriskr", ISA8_CGA_TETRISKR)
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( filetto, pcxt_state )
	MCFG_CPU_ADD("maincpu", I8088, XTAL_14_31818MHz/3)
	MCFG_CPU_PROGRAM_MAP(filetto_map)
	MCFG_CPU_IO_MAP(filetto_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)
	MCFG_PCNOPPI_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", filetto_isa8_cards, "filetto", true)

	MCFG_SOUND_ADD("voice", HC55516, 8000000/4)//8923S-UM5100 is a HC55536 with ROM hook-up
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mb:mono", 0.60)
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	MCFG_DEVICE_ADD("bank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(18)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( tetriskr, pcxt_state )
	MCFG_CPU_ADD("maincpu", I8088, XTAL_14_31818MHz/3)
	MCFG_CPU_PROGRAM_MAP(tetriskr_map)
	MCFG_CPU_IO_MAP(tetriskr_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)
	MCFG_PCNOPPI_MOTHERBOARD_ADD("mb","maincpu")

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", filetto_isa8_cards, "tetriskr", true)
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

ROM_START( filetto )
	ROM_REGION( 0x10000, "bios", 0 )
	ROM_LOAD("u49.bin", 0xc000, 0x2000, CRC(1be6948a) SHA1(9c433f63d347c211ee4663f133e8417221bc4bf0))
	ROM_RELOAD(         0x8000, 0x2000 )
	ROM_RELOAD(         0x4000, 0x2000 )
	ROM_RELOAD(         0x0000, 0x2000 )
	ROM_LOAD("u55.bin", 0xe000, 0x2000, CRC(1e455ed7) SHA1(786d18ce0ab1af45fc538a2300853e497488f0d4) )
	ROM_RELOAD(         0xa000, 0x2000 )
	ROM_RELOAD(         0x6000, 0x2000 )
	ROM_RELOAD(         0x2000, 0x2000 )

	ROM_REGION( 0x40000, "game_prg", 0 ) // program data
	ROM_LOAD( "m0.u1", 0x00000, 0x10000, CRC(2408289d) SHA1(eafc144a557a79b58bcb48545cb9c9778e61fcd3) )
	ROM_LOAD( "m1.u2", 0x10000, 0x10000, CRC(5b623114) SHA1(0d9a14e6b7f57ce4fa09762343b610a973910f58) )
	ROM_LOAD( "m2.u3", 0x20000, 0x10000, CRC(abc64869) SHA1(564fc9d90d241a7b7776160b3fd036fb08037355) )
	ROM_LOAD( "m3.u4", 0x30000, 0x10000, CRC(0c1e8a67) SHA1(f1b9280c65fcfcb5ec481cae48eb6f52d6cdbc9d) )

	ROM_REGION( 0x40000, "samples", 0 ) // UM5100 sample roms?
	ROM_LOAD16_BYTE("v1.u15",  0x00000, 0x20000, CRC(613ddd07) SHA1(ebda3d559315879819cb7034b5696f8e7861fe42) )
	ROM_LOAD16_BYTE("v2.u14",  0x00001, 0x20000, CRC(427e012e) SHA1(50514a6307e63078fe7444a96e39d834684db7df) )
ROM_END

ROM_START( tetriskr )
	ROM_REGION( 0x10000, "bios", 0 ) /* code */
	ROM_LOAD( "b-10.u10", 0x0000, 0x10000, CRC(efc2a0f6) SHA1(5f0f1e90237bee9b78184035a32055b059a91eb3) )
ROM_END

GAME( 1990, filetto,  0, filetto,  filetto, driver_device,  0,  ROT0,  "Novarmatic", "Filetto (v1.05 901009)",MACHINE_IMPERFECT_SOUND )
GAME( 1988?,tetriskr, 0, tetriskr, tetriskr, driver_device,  0, ROT0,  "bootleg",    "Tetris (bootleg of Mirrorsoft PC-XT Tetris version)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
