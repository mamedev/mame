/***************************************************************************

  Konami GV System (aka "Baby Phoenix") - Arcade PSX Hardware
  ===========================================================
  Driver by R. Belmont & smf


Known Dumps
-----------

Game       Description                  Mother Board   Code       Version       Date   Time

pbball96   Powerful Pro Baseball '96    GV999          GV017   JAPAN 1.03   96.05.27  18:00
hyperath   Hyper Athlete                ZV610          GV021   JAPAN 1.00   96.06.09  19:00
susume     Susume! Taisen Puzzle-Dama   ZV610          GV027   JAPAN 1.20   96.03.04  12:00
btchamp    Beat the Champ               GV999          GV053   UAA01        ?
kdeadeye   Dead Eye                     GV999          GV054   UAA01        ?
weddingr   Wedding Rhapsody             ?              GX624   JAA          97.05.29   9:12
tokimosh   Tokimeki Memorial Oshiete    ?              GE755   JAA          97.08.06  11:52
           Your Heart
tokimosp   Tokimeki Memorial Oshiete    ?              GE756   JAB          97.09.27   9:10
           Your Heart Seal version PLUS
nagano98   Winter Olypmics in Nagano 98 GV999          GX720   EAA01 1.03   98.01.08  10:45
simpbowl   Simpsons Bowling             GV999          GQ829   UAA          ?

PCB Layouts
-----------

ZV610 PWB301331
|---------------------------------------|
|   000180       056602      LM324   CN8|
|CN2                                    |
|                                       |
|      999A01.7E                     CN6|
|                         CXD2922BQ     |
|      10E                KM416V256BLT-7|
|                                       |
|J     12E                              |
|A CXD2923AR     058239                 |
|M                                      |
|M                     CXD8530BQ        |
|A   D482445LGW-A70            93CF96-2 |
|               CXD8514Q               S|
|    D482445LGW-A70                    C|
|                      67.7376MHz      S|
|         53.693175MHz                 I|
|                                 32MHz |
|    93C46   KM48V514BJ-6  KM48V514BJ-6 |
|            KM48V514BJ-6  KM48V514BJ-6 |
|    CN5      CN3                001231 |
|---------------------------------------|

GV999 PWB301949A
|---------------------------------------|
|                056602      LM324   CN8|
|CN2                                    |
|TEST_SW                                |
|      999A01.7E                     CN6|
|MC44200                  CXD2925Q      |
|      9E                 TC51V4260BJ-80|
|                                       |
|J     12E                              |
|A               058239                 |
|M  53.693175MHz                        |
|M                     CXD8530CQ        |
|A                             93CF96-2 |
|      CXD8561Q                        S|
|              KM4132G271Q-12          C|
|                      67.7376MHz      S|
|         53.693175MHz                 I|
|                                 32MHz |
|    93C46   KM48V514BJ-6  KM48V514BJ-6 |
|            KM48V514BJ-6  KM48V514BJ-6 |
|    CN5      CN3                001231 |
|---------------------------------------|

Notes:
      - Simpsons Bowling and Dead Eye use a GV999 with a daughtercard containing flash ROMs and CPLDs:
        PWB402610
        Xilinx XC3020A
        Xilinx 1718DPC
        74F244N (2 of these)
        LVT245SS (2 of theses)

      - 000180 is used for driving the RGB output. It's a very thin piece of very brittle ceramic
        containing a circuit, a LM1203 chip, some smt transistors/caps/resistors etc (let's just say
        placing this thing on the edge of the PCB wasn't a good design choice!)
        On GV999, it has been replaced by three transistors and a MC44200.

      - 056602 seems to be some sort of A/D converter (another ceramic thing containing caps/resistors/transistors and a chip)

      - CXD2922 and CXD2925 are SPU's.

      - The BIOS on ZV610 and GV999 is identical. It's a 4M MASK ROM, compatible with 27C040.

      - The CD contains one MODE 1 data track and several Redbook audio tracks which are streamed to the speaker via CN8.

      - The ZV and GV PCB's are virtually identical aside from some minor component shuffling and the RGB output mechanism.
        However note that the GPU revision is different between the two boards and so are some of the other Sony IC's.

      - CN8 used to connect redbook audio output from CD drive to PCB.

      - CN6 used to connect power to CD drive.

      - CN2 used for extra speaker connection for stereo output.

      - CN3, CN5 used for connecting 3rd and 4th player controls.

      - 001231, 058239 are PALCE16V8H PALs.

      - 10E, 12E are unpopulated positions for 16M TSOP56 FLASHROMs (10E is 9E on GV999).

      - If the CD is swapped to another GV game, the game will boot but will stop with an error '25C MBAD' (the EEPROM is 25C)
        So the games can not be swapped by simply exchanging CDs because the EEPROM will not re-init itself if the CDROM is swapped.
        This appears to be some form of mild protection to stop operators swapping CD's.
        However it is possible to swap games to another PCB by exchanging the CD _AND_ the EEPROM from another PCB which belongs
        to that same game. It won't work with a blank EEPROM or a different games' EEPROM.
*/

#include "emu.h"
#include "cdrom.h"
#include "cpu/psx/psx.h"
#include "video/psx.h"
#include "includes/psx.h"
#include "machine/eeprom.h"
#include "machine/intelfsh.h"
#include "machine/am53cf96.h"
#include "machine/scsicd.h"
#include "sound/spu.h"
#include "sound/cdda.h"

class konamigv_state : public psx_state
{
public:
	konamigv_state(const machine_config &mconfig, device_type type, const char *tag)
		: psx_state(mconfig, type, tag) { }

	UINT32 m_flash_address;

	UINT16 m_trackball_prev[ 2 ];
	UINT32 m_trackball_data[ 2 ];
	UINT16 m_btc_trackball_prev[ 4 ];
	UINT32 m_btc_trackball_data[ 4 ];

	fujitsu_29f016a_device *m_flash8[4];
	sharp_lh28f400_device *m_flash16[4];

	UINT8 m_sector_buffer[ 4096 ];
	DECLARE_WRITE32_MEMBER(mb89371_w);
	DECLARE_READ32_MEMBER(mb89371_r);
	DECLARE_READ32_MEMBER(flash_r);
	DECLARE_WRITE32_MEMBER(flash_w);
	DECLARE_READ32_MEMBER(trackball_r);
	DECLARE_READ32_MEMBER(unknown_r);
	DECLARE_READ32_MEMBER(btcflash_r);
	DECLARE_WRITE32_MEMBER(btcflash_w);
	DECLARE_READ32_MEMBER(btc_trackball_r);
	DECLARE_WRITE32_MEMBER(btc_trackball_w);
	DECLARE_READ32_MEMBER(tokimeki_serial_r);
	DECLARE_WRITE32_MEMBER(tokimeki_serial_w);
	DECLARE_WRITE32_MEMBER(kdeadeye_0_w);
	DECLARE_WRITE32_MEMBER(eeprom_w);
};

/* EEPROM handlers */

WRITE32_MEMBER(konamigv_state::eeprom_w)
{
	device_t *device = machine().device("eeprom");
	eeprom_device *eeprom = downcast<eeprom_device *>(device);
	eeprom->write_bit((data&0x01) ? 1 : 0);
	eeprom->set_clock_line((data&0x04) ? ASSERT_LINE : CLEAR_LINE);
	eeprom->set_cs_line((data&0x02) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE32_MEMBER(konamigv_state::mb89371_w)
{
}

READ32_MEMBER(konamigv_state::mb89371_r)
{
	return 0xffffffff;
}

static ADDRESS_MAP_START( konamigv_map, AS_PROGRAM, 32, konamigv_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM	AM_SHARE("share1") /* ram */
	AM_RANGE(0x1f000000, 0x1f00001f) AM_READWRITE_LEGACY(am53cf96_r, am53cf96_w)
	AM_RANGE(0x1f100000, 0x1f100003) AM_READ_PORT("P1")
	AM_RANGE(0x1f100004, 0x1f100007) AM_READ_PORT("P2")
	AM_RANGE(0x1f100008, 0x1f10000b) AM_READ_PORT("P3_P4")
	AM_RANGE(0x1f180000, 0x1f180003) AM_WRITE(eeprom_w)
	AM_RANGE(0x1f680000, 0x1f68001f) AM_READWRITE(mb89371_r, mb89371_w)
	AM_RANGE(0x1f780000, 0x1f780003) AM_WRITENOP /* watchdog? */
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_SHARE("share2") AM_REGION("user1", 0) /* bios */
	AM_RANGE(0x80000000, 0x801fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa01fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END

/* SCSI */

static void scsi_dma_read( konamigv_state *state, UINT32 n_address, INT32 n_size )
{
	UINT32 *p_n_psxram = state->m_p_n_psxram;
	UINT8 *sector_buffer = state->m_sector_buffer;
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( state->m_sector_buffer ) / 4 )
		{
			n_this = sizeof( state->m_sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		if( n_this < 2048 / 4 )
		{
			/* non-READ commands */
			am53cf96_read_data( n_this * 4, sector_buffer );
		}
		else
		{
			/* assume normal 2048 byte data for now */
			am53cf96_read_data( 2048, sector_buffer );
			n_this = 2048 / 4;
		}
		n_size -= n_this;

		i = 0;
		while( n_this > 0 )
		{
			p_n_psxram[ n_address / 4 ] =
				( sector_buffer[ i + 0 ] << 0 ) |
				( sector_buffer[ i + 1 ] << 8 ) |
				( sector_buffer[ i + 2 ] << 16 ) |
				( sector_buffer[ i + 3 ] << 24 );
			n_address += 4;
			i += 4;
			n_this--;
		}
	}
}

static void scsi_dma_write( konamigv_state *state, UINT32 n_address, INT32 n_size )
{
	UINT32 *p_n_psxram = state->m_p_n_psxram;
	UINT8 *sector_buffer = state->m_sector_buffer;
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( state->m_sector_buffer ) / 4 )
		{
			n_this = sizeof( state->m_sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		n_size -= n_this;

		i = 0;
		while( n_this > 0 )
		{
			sector_buffer[ i + 0 ] = ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
			sector_buffer[ i + 1 ] = ( p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
			sector_buffer[ i + 2 ] = ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
			sector_buffer[ i + 3 ] = ( p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			n_address += 4;
			i += 4;
			n_this--;
		}

		am53cf96_write_data( n_this * 4, sector_buffer );
	}
}

static void scsi_irq(running_machine &machine)
{
	psx_irq_set(machine, 0x400);
}

static const SCSIConfigTable dev_table =
{
	1, /* 1 SCSI device */
	{
		{ SCSI_ID_4, ":cdrom", } /* SCSI ID 4, CD-ROM */
	}
};

static const struct AM53CF96interface scsi_intf =
{
	&dev_table,		/* SCSI device table */
	&scsi_irq,		/* command completion IRQ */
};

static DRIVER_INIT( konamigv )
{
	psx_driver_init(machine);

	/* init the scsi controller and hook up it's DMA */
	am53cf96_init(machine, &scsi_intf);
}

static MACHINE_START( konamigv )
{
	konamigv_state *state = machine.driver_data<konamigv_state>();

	state->save_item(NAME(state->m_sector_buffer));
	state->save_item(NAME(state->m_flash_address));
	state->save_item(NAME(state->m_trackball_prev));
	state->save_item(NAME(state->m_trackball_data));
	state->save_item(NAME(state->m_btc_trackball_prev));
	state->save_item(NAME(state->m_btc_trackball_data));
}

static MACHINE_RESET( konamigv )
{
	/* also hook up CDDA audio to the CD-ROM drive */
	cdda_set_cdrom(machine.device("cdda"), am53cf96_get_device(SCSI_ID_4));
}

static void spu_irq(device_t *device, UINT32 data)
{
	if (data)
	{
		psx_irq_set(device->machine(), 1<<9);
	}
}

static MACHINE_CONFIG_START( konamigv, konamigv_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8530BQ, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( konamigv_map )

	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 5, psx_dma_read_delegate( FUNC( scsi_dma_read ), (konamigv_state *) owner ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 5, psx_dma_write_delegate( FUNC( scsi_dma_write ), (konamigv_state *) owner ) )

	MCFG_MACHINE_START( konamigv )
	MCFG_MACHINE_RESET( konamigv )

	MCFG_EEPROM_93C46_ADD("eeprom")

	MCFG_DEVICE_ADD("cdrom", SCSICD, 0)

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8514Q, 0x100000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2, &spu_irq )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.75 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.75 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.0 )
MACHINE_CONFIG_END


static INPUT_PORTS_START( konamigv )
	PORT_START("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00001000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "eeprom", eeprom_device, read_bit )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_P4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Simpsons Bowling */

READ32_MEMBER(konamigv_state::flash_r)
{
	int reg = offset*2;
	//int shift = 0;

	if (mem_mask == 0xffff0000)
	{
		reg++;
		//shift = 16;
	}

	if (reg == 4)	// set odd address
	{
		m_flash_address |= 1;
	}

	if (reg == 0)
	{
		int chip = (m_flash_address >= 0x200000) ? 2 : 0;
		int ret;

		ret = m_flash8[chip]->read(m_flash_address & 0x1fffff) & 0xff;
		ret |= m_flash8[chip+1]->read(m_flash_address & 0x1fffff)<<8;
		m_flash_address++;

		return ret;
	}
	return 0;
}

WRITE32_MEMBER(konamigv_state::flash_w)
{
	int reg = offset*2;
	int chip;

	if (mem_mask == 0xffff0000)
	{
		reg++;
		data>>= 16;
	}

	switch (reg)
	{
		case 0:
			chip = (m_flash_address >= 0x200000) ? 2 : 0;
			m_flash8[chip]->write(m_flash_address & 0x1fffff, data&0xff);
			m_flash8[chip+1]->write(m_flash_address & 0x1fffff, (data>>8)&0xff);
			break;

		case 1:
			m_flash_address = 0;
			m_flash_address |= (data<<1);
			break;
		case 2:
			m_flash_address &= 0xff00ff;
			m_flash_address |= (data<<8);
			break;
		case 3:
			m_flash_address &= 0x00ffff;
			m_flash_address |= (data<<15);
			break;
	}
}

READ32_MEMBER(konamigv_state::trackball_r)
{

	if( offset == 0 && mem_mask == 0x0000ffff )
	{
		int axis;
		UINT16 diff;
		UINT16 value;
		static const char *const axisnames[] = { "TRACK0_X", "TRACK0_Y" };

		for( axis = 0; axis < 2; axis++ )
		{
			value = ioport(axisnames[axis])->read();
			diff = value - m_trackball_prev[ axis ];
			m_trackball_prev[ axis ] = value;
			m_trackball_data[ axis ] = ( ( diff & 0xf00 ) << 16 ) | ( ( diff & 0xff ) << 8 );
		}
	}
	return m_trackball_data[ offset ];
}

READ32_MEMBER(konamigv_state::unknown_r)
{
	return 0xffffffff;
}

static DRIVER_INIT( simpbowl )
{
	konamigv_state *state = machine.driver_data<konamigv_state>();

	state->m_flash8[0] = machine.device<fujitsu_29f016a_device>("flash0");
	state->m_flash8[1] = machine.device<fujitsu_29f016a_device>("flash1");
	state->m_flash8[2] = machine.device<fujitsu_29f016a_device>("flash2");
	state->m_flash8[3] = machine.device<fujitsu_29f016a_device>("flash3");

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f680080, 0x1f68008f, read32_delegate(FUNC(konamigv_state::flash_r),state), write32_delegate(FUNC(konamigv_state::flash_w),state) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler     ( 0x1f6800c0, 0x1f6800c7, read32_delegate(FUNC(konamigv_state::trackball_r),state));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler     ( 0x1f6800c8, 0x1f6800cb, read32_delegate(FUNC(konamigv_state::unknown_r),state)); /* ?? */

	DRIVER_INIT_CALL(konamigv);
}

static MACHINE_CONFIG_DERIVED( simpbowl, konamigv )
	MCFG_FUJITSU_29F016A_ADD("flash0")
	MCFG_FUJITSU_29F016A_ADD("flash1")
	MCFG_FUJITSU_29F016A_ADD("flash2")
	MCFG_FUJITSU_29F016A_ADD("flash3")
MACHINE_CONFIG_END

static INPUT_PORTS_START( simpbowl )
	PORT_INCLUDE( konamigv )

	PORT_START("TRACK0_X")
	PORT_BIT( 0xfff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xfff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

INPUT_PORTS_END

/* Beat the Champ */

READ32_MEMBER(konamigv_state::btcflash_r)
{

	if (mem_mask == 0x0000ffff)
	{
		return m_flash16[0]->read(offset*2);
	}
	else if (mem_mask == 0xffff0000)
	{
		return m_flash16[0]->read((offset*2)+1)<<16;
	}

	return 0;
}

WRITE32_MEMBER(konamigv_state::btcflash_w)
{

	if (mem_mask == 0x0000ffff)
	{
		m_flash16[0]->write(offset*2, data&0xffff);
	}
	else if (mem_mask == 0xffff0000)
	{
		m_flash16[0]->write((offset*2)+1, (data>>16)&0xffff);
	}
}

READ32_MEMBER(konamigv_state::btc_trackball_r)
{

//  mame_printf_debug( "r %08x %08x %08x\n", cpu_get_pc(&space.device()), offset, mem_mask );

	if( offset == 1 && mem_mask == 0xffff0000 )
	{
		int axis;
		UINT16 diff;
		UINT16 value;
		static const char *const axisnames[] = { "TRACK0_X", "TRACK0_Y", "TRACK1_X", "TRACK1_Y" };

		for( axis = 0; axis < 4; axis++ )
		{
			value = ioport(axisnames[axis])->read();
			diff = value - m_btc_trackball_prev[ axis ];
			m_btc_trackball_prev[ axis ] = value;
			m_btc_trackball_data[ axis ] = ( ( diff & 0xf00 ) << 16 ) | ( ( diff & 0xff ) << 8 );
		}
	}

	return m_btc_trackball_data[ offset ] | ( m_btc_trackball_data[ offset + 2 ] >> 8 );
}

WRITE32_MEMBER(konamigv_state::btc_trackball_w)
{
//  mame_printf_debug( "w %08x %08x %08x %08x\n", cpu_get_pc(&space.device()), offset, data, mem_mask );
}

static DRIVER_INIT( btchamp )
{
	konamigv_state *state = machine.driver_data<konamigv_state>();

	state->m_flash16[0] = machine.device<sharp_lh28f400_device>("flash");

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f680080, 0x1f68008f, read32_delegate(FUNC(konamigv_state::btc_trackball_r),state), write32_delegate(FUNC(konamigv_state::btc_trackball_w),state));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->nop_write                  ( 0x1f6800e0, 0x1f6800e3);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f380000, 0x1f3fffff, read32_delegate(FUNC(konamigv_state::btcflash_r),state), write32_delegate(FUNC(konamigv_state::btcflash_w),state) );

	DRIVER_INIT_CALL(konamigv);
}

static MACHINE_CONFIG_DERIVED( btchamp, konamigv )
	MCFG_SHARP_LH28F400_ADD("flash")
MACHINE_CONFIG_END

static INPUT_PORTS_START( btchamp )
	PORT_INCLUDE( konamigv )

	PORT_START("TRACK0_X")
	PORT_BIT( 0x7ff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACK0_Y")
	PORT_BIT( 0x7ff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("TRACK1_X")
	PORT_BIT( 0x7ff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("TRACK1_Y")
	PORT_BIT( 0x7ff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)
INPUT_PORTS_END

/* Tokimeki Memorial games - have a mouse and printer and who knows what else */

READ32_MEMBER(konamigv_state::tokimeki_serial_r)
{
	// bits checked: 0x80 and 0x20 for periodic status (800b6968 and 800b69e0 in tokimosh)
	// 0x08 for reading the serial device (8005e624)

	return 0xffffffff;
}

WRITE32_MEMBER(konamigv_state::tokimeki_serial_w)
{
	/*
        serial EEPROM-like device here: when mem_mask == 0x000000ff only,

        0x40 = chip enable
        0x20 = clock
        0x10 = data

        tokimosh sends 6 bits: 110100 then reads 8 bits.
        readback is bit 3 (0x08) of serial_r
        This happens starting around 8005e580.
    */

}

static DRIVER_INIT( tokimosh )
{
	konamigv_state *state = machine.driver_data<konamigv_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler ( 0x1f680080, 0x1f680083, read32_delegate(FUNC(konamigv_state::tokimeki_serial_r),state));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler( 0x1f680090, 0x1f680093, write32_delegate(FUNC(konamigv_state::tokimeki_serial_w),state));

	DRIVER_INIT_CALL(konamigv);
}

/*
Dead Eye

CD:
    P/N 002715
    054
    UA
    A01
*/

WRITE32_MEMBER(konamigv_state::kdeadeye_0_w)
{
}

static DRIVER_INIT( kdeadeye )
{
	konamigv_state *state = machine.driver_data<konamigv_state>();

	state->m_flash16[0] = machine.device<sharp_lh28f400_device>("flash");

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port  ( 0x1f680080, 0x1f680083, "GUNX1" );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port  ( 0x1f680090, 0x1f680093, "GUNY1" );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port  ( 0x1f6800a0, 0x1f6800a3, "GUNX2" );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port  ( 0x1f6800b0, 0x1f6800b3, "GUNY2" );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port  ( 0x1f6800c0, 0x1f6800c3, "BUTTONS" );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler    ( 0x1f6800e0, 0x1f6800e3, write32_delegate(FUNC(konamigv_state::kdeadeye_0_w),state) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f380000, 0x1f3fffff, read32_delegate(FUNC(konamigv_state::btcflash_r),state), write32_delegate(FUNC(konamigv_state::btcflash_w),state));

	DRIVER_INIT_CALL(konamigv);
}

static MACHINE_CONFIG_DERIVED( kdeadeye, konamigv )
	MCFG_SHARP_LH28F400_ADD("flash")
MACHINE_CONFIG_END

static INPUT_PORTS_START( kdeadeye )
	PORT_INCLUDE( konamigv )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0000007f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0000007f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P3_P4")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("GUNX1")
	PORT_BIT( 0xffff, 0x0100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x004c, 0x01bb ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 1 )

	PORT_START("GUNY1")
	PORT_BIT( 0xffff, 0x0077, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 1 )

	PORT_START("GUNX2")
	PORT_BIT( 0xffff, 0x0100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x004c, 0x01bb ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 2 )

	PORT_START("GUNY2")
	PORT_BIT( 0xffff, 0x0077, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 2 )

	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

#define GV_BIOS	\
	ROM_REGION32_LE( 0x080000, "user1", 0 )	\
	ROM_LOAD( "999a01.7e",   0x0000000, 0x080000, CRC(ad498d2d) SHA1(02a82a2fe1fba0404517c3602324bfa64e23e478) )

ROM_START( konamigv )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", ROMREGION_ERASE00 ) /* default eeprom */
ROM_END

ROM_START( susume )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "susume.25c",   0x000000, 0x000080, CRC(52f17df7) SHA1(b8ad7787b0692713439d7d9bebfa0c801c806006) )
	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "gv027j1", 0, BAD_DUMP SHA1(e7e6749ac65de7771eb8fed7d5eefaec3f902255) )
ROM_END

ROM_START( hyperath )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "hyperath.25c", 0x000000, 0x000080, CRC(20a8c435) SHA1(a0f203a999757fba68b391c525ac4b9684a57ba9) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "hyperath", 0, BAD_DUMP SHA1(694ef6200c61d3052316100cd9251b495eab88a1) )
ROM_END

ROM_START( pbball96 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "pbball96.25c", 0x000000, 0x000080, CRC(405a7fc9) SHA1(e2d978f49748ba3c4a425188abcd3d272ec23907) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "pbball96", 0, BAD_DUMP SHA1(ebd0ea18ff9ce300ea1e30d66a739a96acfb0621) )
ROM_END

ROM_START( weddingr )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "weddingr.25c", 0x000000, 0x000080, CRC(b90509a0) SHA1(41510a0ceded81dcb26a70eba97636d38d3742c3) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "weddingr", 0, BAD_DUMP SHA1(4e7122b191747ab7220fe4ce1b4483d62ab579af) )
ROM_END

ROM_START( simpbowl )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "simpbowl.25c", 0x000000, 0x000080, CRC(2c61050c) SHA1(16ae7f81cbe841c429c5c7326cf83e87db1782bf) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "simpbowl", 0, BAD_DUMP SHA1(72b32a863e6891ad3bfc1fdfe9cb90a2bd334d71) )
ROM_END

ROM_START( btchamp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "btchmp.25c", 0x000000, 0x000080, CRC(6d02ea54) SHA1(d3babf481fd89db3aec17f589d0d3d999a2aa6e1) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "btchamp", 0, BAD_DUMP SHA1(c9c858e9034826e1a12c3c003dd068a49a3577e1) )
ROM_END

ROM_START( kdeadeye )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "kdeadeye.25c", 0x000000, 0x000080, CRC(3935d2df) SHA1(cbb855c475269077803c380dbc3621e522efe51e) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "kdeadeye", 0, BAD_DUMP SHA1(3c737c51717925be724dcb93d30769649029b8ce) )
ROM_END

ROM_START( nagano98 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "nagano98.25c",  0x000000, 0x000080, CRC(b64b7451) SHA1(a77a37e0cc580934d1e7e05d523bae0acd2c1480) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "nagano98", 0, BAD_DUMP SHA1(1be7bd4531f249ff2233dd40a206c8d60054a8c6) )
ROM_END

ROM_START( tokimosh )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
        ROM_LOAD( "tokimosh.25c", 0x000000, 0x000080, CRC(e57b833f) SHA1(f18a0974a6be69dc179706643aab837ff61c2738) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "755jaa01", 0, BAD_DUMP SHA1(4af080f9650e34d1ddb91bb763469d5fb3c754bd) )
ROM_END

ROM_START( tokimosp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "tokimosp.25c", 0x000000, 0x000080, CRC(af4cdd87) SHA1(97041e287e4c80066043967450779b81b62b2b8e) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "756jab01", 0, BAD_DUMP SHA1(7bd974d908ae5a7bfa8d30db185ab01ac38dff28) )
ROM_END

/* BIOS placeholder */
GAME( 1995, konamigv, 0, konamigv, konamigv, konamigv, ROT0, "Konami", "Baby Phoenix/GV System", GAME_IS_BIOS_ROOT )

GAME( 1996, pbball96, konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Powerful Baseball '96 (GV017 Japan 1.03)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, hyperath, konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Hyper Athlete (GV021 Japan 1.00)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, susume,   konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Susume! Taisen Puzzle-Dama (GV027 Japan 1.20)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, btchamp,  konamigv, btchamp,  btchamp,  btchamp,  ROT0, "Konami", "Beat the Champ (GV053 UAA01)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, kdeadeye, konamigv, kdeadeye, kdeadeye, kdeadeye, ROT0, "Konami", "Dead Eye (GV054 UAA01)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, weddingr, konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Wedding Rhapsody (GX624 JAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, tokimosh, konamigv, konamigv, konamigv, tokimosh, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart (GE755 JAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1997, tokimosp, konamigv, konamigv, konamigv, tokimosh, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal version PLUS (GE756 JAB)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1998, nagano98, konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Nagano Winter Olympics '98 (GX720 EAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
GAME( 2000, simpbowl, konamigv, simpbowl, simpbowl, simpbowl, ROT0, "Konami", "Simpsons Bowling (GQ829 UAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
