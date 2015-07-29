// license:BSD-3-Clause
// copyright-holders:R. Belmont, smf
/***************************************************************************

  Konami GV System (aka "Baby Phoenix") - Arcade PSX Hardware
  ===========================================================
  Driver by R. Belmont & smf


Known Dumps
-----------

Game       Description                                              Mother Board   Code       Version       Date   Time

powyak96   Jikkyou Powerful Pro Yakyuu '96                          GV999          GV017   JAPAN 1.03   96.05.27  18:00
hyperath   Hyper Athlete                                            ZV610          GV021   JAPAN 1.00   96.06.09  19:00
lacrazyc   Let's Attack Crazy Cross                                 ZV610          GV027   ASIA  1.10   96.01.18  12:00
susume     Susume! Taisen Puzzle-Dama                               ZV610          GV027   JAPAN 1.20   96.03.04  12:00
btchamp    Beat the Champ                                           GV999          GV053   UAA01        ?
kdeadeye   Dead Eye                                                 GV999          GV054   UAA01        ?
weddingr   Wedding Rhapsody                                         ?              GX624   JAA          97.05.29   9:12
tmosh      Tokimeki Memorial Oshiete Your Heart                     ?              GQ673   JAA          97.03.14  ?
tmoshs     Tokimeki Memorial Oshiete Your Heart Seal Version        ?              GE755   JAA          97.08.06  11:52
tmoshspa   Tokimeki Memorial Oshiete Your Heart Seal Version Plus   ?              GE756   JAA          97.08.24  12:20
tmoshsp    Tokimeki Memorial Oshiete Your Heart Seal Version Plus   ?              GE756   JAB          97.09.27   9:10
nagano98   Winter Olypmics in Nagano 98                             GV999          GX720   EAA01 1.03   98.01.08  10:45
naganoj    Hyper Olympic in Nagano                                  GV999          GX720   JAA01 1.02   98.01.07  01:10
simpbowl   Simpsons Bowling                                         GV999          GQ829   UAA          ?

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
#include "machine/am53cf96.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/mb89371.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "sound/spu.h"
#include "sound/cdda.h"

class konamigv_state : public driver_device
{
public:
	konamigv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_am53cf96(*this, "am53cf96"),
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_READ16_MEMBER(flash_r);
	DECLARE_WRITE16_MEMBER(flash_w);
	DECLARE_READ16_MEMBER(trackball_r);
	DECLARE_READ16_MEMBER(unknown_r);
	DECLARE_READ16_MEMBER(btc_trackball_r);
	DECLARE_WRITE16_MEMBER(btc_trackball_w);
	DECLARE_READ16_MEMBER(tokimeki_serial_r);
	DECLARE_WRITE16_MEMBER(tokimeki_serial_w);
	DECLARE_DRIVER_INIT(simpbowl);
	void scsi_dma_read( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size );
	void scsi_dma_write( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size );

protected:
	virtual void driver_start();

private:
	required_device<am53cf96_device> m_am53cf96;

	UINT32 m_flash_address;

	UINT16 m_trackball_prev[ 2 ];
	UINT16 m_trackball_data[ 2 ];
	UINT16 m_btc_trackball_prev[ 4 ];
	UINT16 m_btc_trackball_data[ 4 ];

	fujitsu_29f016a_device *m_flash8[4];

	UINT8 m_sector_buffer[ 4096 ];
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( konamigv_map, AS_PROGRAM, 32, konamigv_state )
	AM_RANGE(0x1f000000, 0x1f00001f) AM_DEVREADWRITE8("am53cf96", am53cf96_device, read, write, 0x00ff00ff)
	AM_RANGE(0x1f100000, 0x1f100003) AM_READ_PORT("P1")
	AM_RANGE(0x1f100004, 0x1f100007) AM_READ_PORT("P2")
	AM_RANGE(0x1f100008, 0x1f10000b) AM_READ_PORT("P3_P4")
	AM_RANGE(0x1f180000, 0x1f180003) AM_WRITE_PORT("EEPROMOUT")
	AM_RANGE(0x1f680000, 0x1f68001f) AM_DEVREADWRITE8("mb89371", mb89371_device, read, write, 0x00ff00ff)
	AM_RANGE(0x1f780000, 0x1f780003) AM_WRITENOP /* watchdog? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( simpbowl_map, AS_PROGRAM, 32, konamigv_state )
	AM_IMPORT_FROM( konamigv_map )

	AM_RANGE(0x1f680080, 0x1f68008f) AM_READWRITE16(flash_r, flash_w, 0xffffffff)
	AM_RANGE(0x1f6800c0, 0x1f6800c7) AM_READ16(trackball_r, 0xffffffff)
	AM_RANGE(0x1f6800c8, 0x1f6800cb) AM_READ16(unknown_r, 0x0000ffff) /* ?? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( btchamp_map, AS_PROGRAM, 32, konamigv_state )
	AM_IMPORT_FROM( konamigv_map )

	AM_RANGE(0x1f380000, 0x1f3fffff) AM_DEVREADWRITE16("flash", intelfsh16_device, read, write, 0xffffffff)
	AM_RANGE(0x1f680080, 0x1f680087) AM_READ16(btc_trackball_r, 0xffffffff)
	AM_RANGE(0x1f680088, 0x1f68008b) AM_WRITE16(btc_trackball_w, 0xffffffff)
	AM_RANGE(0x1f6800e0, 0x1f6800e3) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( kdeadeye_map, AS_PROGRAM, 32, konamigv_state )
	AM_IMPORT_FROM( konamigv_map )

	AM_RANGE(0x1f380000, 0x1f3fffff) AM_DEVREADWRITE16("flash", intelfsh16_device, read, write, 0xffffffff)
	AM_RANGE(0x1f680080, 0x1f680083) AM_READ_PORT("GUNX1")
	AM_RANGE(0x1f680090, 0x1f680093) AM_READ_PORT("GUNY1")
	AM_RANGE(0x1f6800a0, 0x1f6800a3) AM_READ_PORT("GUNX2")
	AM_RANGE(0x1f6800b0, 0x1f6800b3) AM_READ_PORT("GUNY2")
	AM_RANGE(0x1f6800c0, 0x1f6800c3) AM_READ_PORT("BUTTONS")
	AM_RANGE(0x1f6800e0, 0x1f6800e3) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( tmosh_map, AS_PROGRAM, 32, konamigv_state )
	AM_IMPORT_FROM( konamigv_map )

	AM_RANGE(0x1f680080, 0x1f680083) AM_READ16(tokimeki_serial_r, 0x0000ffff)
	AM_RANGE(0x1f680090, 0x1f680093) AM_WRITE16(tokimeki_serial_w, 0x0000ffff)
ADDRESS_MAP_END

/* SCSI */

void konamigv_state::scsi_dma_read( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
	UINT8 *sector_buffer = m_sector_buffer;
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( m_sector_buffer ) / 4 )
		{
			n_this = sizeof( m_sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		if( n_this < 2048 / 4 )
		{
			/* non-READ commands */
			m_am53cf96->dma_read_data( n_this * 4, sector_buffer );
		}
		else
		{
			/* assume normal 2048 byte data for now */
			m_am53cf96->dma_read_data( 2048, sector_buffer );
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

void konamigv_state::scsi_dma_write( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
	UINT8 *sector_buffer = m_sector_buffer;
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( m_sector_buffer ) / 4 )
		{
			n_this = sizeof( m_sector_buffer ) / 4;
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

		m_am53cf96->dma_write_data( i, sector_buffer );
	}
}

void konamigv_state::driver_start()
{
	save_item(NAME(m_sector_buffer));
	save_item(NAME(m_flash_address));
	save_item(NAME(m_trackball_prev));
	save_item(NAME(m_trackball_data));
	save_item(NAME(m_btc_trackball_prev));
	save_item(NAME(m_btc_trackball_data));
}

static MACHINE_CONFIG_FRAGMENT( cdrom_config )
	MCFG_DEVICE_MODIFY( "cdda" )
	MCFG_SOUND_ROUTE( 0, "^^^^lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( 1, "^^^^rspeaker", 1.0 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( konamigv, konamigv_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8530BQ, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( konamigv_map )

	MCFG_RAM_MODIFY("maincpu:ram")
	MCFG_RAM_DEFAULT_SIZE("2M")

	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 5, psx_dma_read_delegate( FUNC( konamigv_state::scsi_dma_read ), (konamigv_state *) owner ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 5, psx_dma_write_delegate( FUNC( konamigv_state::scsi_dma_write ), (konamigv_state *) owner ) )

	MCFG_DEVICE_ADD("mb89371", MB89371, 0)
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "cdrom", SCSICD, SCSI_ID_4)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("cdrom", cdrom_config)

	MCFG_DEVICE_ADD("am53cf96", AM53CF96, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_AM53CF96_IRQ_HANDLER(DEVWRITELINE("maincpu:irq", psxirq_device, intin10))

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8514Q, 0x100000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.75 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.75 )
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
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "eeprom", eeprom_serial_93cxx_device, do_read )
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

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
INPUT_PORTS_END

/* Simpsons Bowling */

READ16_MEMBER(konamigv_state::flash_r)
{
	if (offset == 4)   // set odd address
	{
		m_flash_address |= 1;
	}

	if (offset == 0)
	{
		int chip = (m_flash_address >= 0x200000) ? 2 : 0;

		int ret = ( m_flash8[chip]->read(m_flash_address & 0x1fffff) & 0xff ) |
			( m_flash8[chip+1]->read(m_flash_address & 0x1fffff) << 8 );

		m_flash_address++;

		return ret;
	}

	return 0;
}

WRITE16_MEMBER(konamigv_state::flash_w)
{
	int chip;

	switch (offset)
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

READ16_MEMBER(konamigv_state::trackball_r)
{
	if( offset == 0 )
	{
		static const char *const axisnames[] = { "TRACK0_X", "TRACK0_Y" };

		for( int axis = 0; axis < 2; axis++ )
		{
			UINT16 value = ioport(axisnames[axis])->read();
			m_trackball_data[ axis ] = value - m_trackball_prev[ axis ];
			m_trackball_prev[ axis ] = value;
		}
	}

	if( ( offset & 1 ) == 0 )
	{
		return m_trackball_data[ offset >> 1 ] << 8;
	}

	return m_trackball_data[ offset >> 1 ] & 0xf00;
}

READ16_MEMBER(konamigv_state::unknown_r)
{
	return 0xffff;
}

DRIVER_INIT_MEMBER(konamigv_state,simpbowl)
{
	m_flash8[0] = machine().device<fujitsu_29f016a_device>("flash0");
	m_flash8[1] = machine().device<fujitsu_29f016a_device>("flash1");
	m_flash8[2] = machine().device<fujitsu_29f016a_device>("flash2");
	m_flash8[3] = machine().device<fujitsu_29f016a_device>("flash3");
}

static MACHINE_CONFIG_DERIVED( simpbowl, konamigv )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( simpbowl_map )

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

READ16_MEMBER(konamigv_state::btc_trackball_r)
{
//  osd_printf_debug( "r %08x %08x %08x\n", space.device().safe_pc(), offset, mem_mask );

	if( offset == 3 )
	{
		static const char *const axisnames[] = { "TRACK0_X", "TRACK0_Y", "TRACK1_X", "TRACK1_Y" };

		for( int axis = 0; axis < 4; axis++ )
		{
			UINT16 value = ioport(axisnames[axis])->read();
			m_btc_trackball_data[ axis ] = value - m_btc_trackball_prev[ axis ];
			m_btc_trackball_prev[ axis ] = value;
		}
	}

	if( ( offset & 1 ) == 0 )
	{
		return ( m_btc_trackball_data[ offset >> 1 ] << 8 ) | ( m_btc_trackball_data[ ( offset >> 1 ) + 2 ] & 0xff );
	}

	return ( m_btc_trackball_data[ offset >> 1 ] & 0xf00 ) | ( m_btc_trackball_data[ ( offset >> 1 ) + 2 ] >> 8 );
}

WRITE16_MEMBER(konamigv_state::btc_trackball_w)
{
//  osd_printf_debug( "w %08x %08x %08x %08x\n", space.device().safe_pc(), offset, data, mem_mask );
}

static MACHINE_CONFIG_DERIVED( btchamp, konamigv )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( btchamp_map )

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

READ16_MEMBER(konamigv_state::tokimeki_serial_r)
{
	// bits checked: 0x80 and 0x20 for periodic status (800b6968 and 800b69e0 in tmoshs)
	// 0x08 for reading the serial device (8005e624)

	return 0xffff;
}

WRITE16_MEMBER(konamigv_state::tokimeki_serial_w)
{
	/*
	    serial EEPROM-like device here: when mem_mask == 0x000000ff only,

	    0x40 = chip enable
	    0x20 = clock
	    0x10 = data

	    tmoshs sends 6 bits: 110100 then reads 8 bits.
	    readback is bit 3 (0x08) of serial_r
	    This happens starting around 8005e580.
	*/

}

static MACHINE_CONFIG_DERIVED( tmosh, konamigv )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( tmosh_map )
MACHINE_CONFIG_END

/*
Dead Eye

CD:
    P/N 002715
    054
    UA
    A01
*/

static MACHINE_CONFIG_DERIVED( kdeadeye, konamigv )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( kdeadeye_map )

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

#define GV_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "999a01.7e",   0x0000000, 0x080000, CRC(ad498d2d) SHA1(02a82a2fe1fba0404517c3602324bfa64e23e478) )

ROM_START( konamigv )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", ROMREGION_ERASE00 ) /* default eeprom */
ROM_END

ROM_START( lacrazyc )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "lacrazyc.25c",   0x000000, 0x000080, CRC(e20e5730) SHA1(066b49236c658a4ef2930f7bacc4b2354dd7f240) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gv027-a1", 0, BAD_DUMP SHA1(840d0d4876cf1b814c9d8db975aa6c92e1fe4039) )
ROM_END

ROM_START( susume )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "susume.25c",   0x000000, 0x000080, CRC(52f17df7) SHA1(b8ad7787b0692713439d7d9bebfa0c801c806006) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gv027j1", 0, BAD_DUMP SHA1(e7e6749ac65de7771eb8fed7d5eefaec3f902255) )
ROM_END

ROM_START( hyperath )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "hyperath.25c", 0x000000, 0x000080, CRC(20a8c435) SHA1(a0f203a999757fba68b391c525ac4b9684a57ba9) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gv021-j1", 0, SHA1(579442444025b18da658cd6455c51459fbc3de0e) )
ROM_END

ROM_START( powyak96 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "powyak96.25c", 0x000000, 0x000080, CRC(405a7fc9) SHA1(e2d978f49748ba3c4a425188abcd3d272ec23907) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "powyak96", 0, BAD_DUMP SHA1(ebd0ea18ff9ce300ea1e30d66a739a96acfb0621) )
ROM_END

ROM_START( weddingr )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "weddingr.25c", 0x000000, 0x000080, CRC(b90509a0) SHA1(41510a0ceded81dcb26a70eba97636d38d3742c3) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "weddingr", 0, BAD_DUMP SHA1(4e7122b191747ab7220fe4ce1b4483d62ab579af) )
ROM_END

ROM_START( simpbowl )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "simpbowl.25c", 0x000000, 0x000080, CRC(2c61050c) SHA1(16ae7f81cbe841c429c5c7326cf83e87db1782bf) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "simpbowl", 0, BAD_DUMP SHA1(72b32a863e6891ad3bfc1fdfe9cb90a2bd334d71) )
ROM_END

ROM_START( btchamp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "btchmp.25c", 0x000000, 0x000080, CRC(6d02ea54) SHA1(d3babf481fd89db3aec17f589d0d3d999a2aa6e1) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "btchamp", 0, BAD_DUMP SHA1(c9c858e9034826e1a12c3c003dd068a49a3577e1) )
ROM_END

ROM_START( kdeadeye )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "kdeadeye.25c", 0x000000, 0x000080, CRC(3935d2df) SHA1(cbb855c475269077803c380dbc3621e522efe51e) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "kdeadeye", 0, BAD_DUMP SHA1(3c737c51717925be724dcb93d30769649029b8ce) )
ROM_END

ROM_START( nagano98 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "nagano98.25c",  0x000000, 0x000080, CRC(b64b7451) SHA1(a77a37e0cc580934d1e7e05d523bae0acd2c1480) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "nagano98", 0, BAD_DUMP SHA1(1be7bd4531f249ff2233dd40a206c8d60054a8c6) )
ROM_END

ROM_START( naganoj )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "720ja.25c",  0x000000, 0x000080, CRC(34c473ba) SHA1(768225b04a293bdbc114a092d14dee28d52044e9) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "720jaa01", 0, SHA1(437160996551ef4dfca43899d1d14beca62eb4c9) )
ROM_END

ROM_START( tmosh )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "tmosh.25c", 0x000000, 0x000080, NO_DUMP )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "673jaa01", 0, SHA1(eaa76073749f9db48c1bee3dff9bea955683c8a8) )
ROM_END

ROM_START( tmoshs )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "tmoshs.25c", 0x000000, 0x000080, CRC(e57b833f) SHA1(f18a0974a6be69dc179706643aab837ff61c2738) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "755jaa01", 0, SHA1(fc742a0b763ba38350ba7eb5d775948632aafd9d) )
ROM_END

ROM_START( tmoshsp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "tmoshsp.25c", 0x000000, 0x000080, CRC(af4cdd87) SHA1(97041e287e4c80066043967450779b81b62b2b8e) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "756jab01", 0, SHA1(b2c59b9801debccbbd986728152f314535c67e53) )
ROM_END

ROM_START( tmoshspa )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "tmoshsp.25c", 0x000000, 0x000080, CRC(af4cdd87) SHA1(97041e287e4c80066043967450779b81b62b2b8e) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "756jaa01", 0, BAD_DUMP SHA1(5e6d349ad1a22c0dbb1ec26aa05febc830254339) ) // The CD was damaged
ROM_END

/* BIOS placeholder */
GAME( 1995, konamigv, 0,        konamigv, konamigv, driver_device,  0,        ROT0, "Konami", "Baby Phoenix/GV System", MACHINE_IS_BIOS_ROOT )

GAME( 1996, powyak96, konamigv, konamigv, konamigv, driver_device,  0,        ROT0, "Konami", "Jikkyou Powerful Pro Yakyuu '96 (GV017 Japan 1.03)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, hyperath, konamigv, konamigv, konamigv, driver_device,  0,        ROT0, "Konami", "Hyper Athlete (GV021 Japan 1.00)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, lacrazyc, konamigv, konamigv, konamigv, driver_device,  0,        ROT0, "Konami", "Let's Attack Crazy Cross (GV027 Asia 1.10)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, susume,   lacrazyc, konamigv, konamigv, driver_device,  0,        ROT0, "Konami", "Susume! Taisen Puzzle-Dama (GV027 Japan 1.20)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, btchamp,  konamigv, btchamp,  btchamp,  driver_device,  0,        ROT0, "Konami", "Beat the Champ (GV053 UAA01)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, kdeadeye, konamigv, kdeadeye, kdeadeye, driver_device,  0,        ROT0, "Konami", "Dead Eye (GV054 UAA01)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, weddingr, konamigv, konamigv, konamigv, driver_device,  0,        ROT0, "Konami", "Wedding Rhapsody (GX624 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, tmosh,    konamigv, tmosh,    konamigv, driver_device,  0,        ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart (GQ673 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1997, tmoshs,   konamigv, tmosh,    konamigv, driver_device,  0,        ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal Version (GE755 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1997, tmoshsp,  konamigv, tmosh,    konamigv, driver_device,  0,        ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal Version Plus (GE756 JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1997, tmoshspa, tmoshsp,  tmosh,    konamigv, driver_device,  0,        ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal Version Plus (GE756 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1998, nagano98, konamigv, konamigv, konamigv, driver_device,  0,        ROT0, "Konami", "Nagano Winter Olympics '98 (GX720 EAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 1998, naganoj,  nagano98, konamigv, konamigv, driver_device,  0,        ROT0, "Konami", "Hyper Olympic in Nagano (GX720 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, simpbowl, konamigv, simpbowl, simpbowl, konamigv_state, simpbowl, ROT0, "Konami", "Simpsons Bowling (GQ829 UAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
