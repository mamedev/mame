// license:BSD-3-Clause
// copyright-holders:R. Belmont, smf
/***************************************************************************

  Konami GQ System - Arcade PSX Hardware
  ======================================
  Driver by R. Belmont & smf

  Crypt Killer
  Konami, 1995

  PCB Layout
  ----------

  GQ420  PWB354905B
  |----------------------------------------------------------------|
  |CN14          420A01.2G          420A02.3M         CN6 CN7 CN8  |
  |           056602  6264    |------| 424800                      |
  |           LA4705  6264    |058141|                             |
  |                   68000   |------| 424800                      |
  |     18.432MHz   PAL(000607)                                    |
  |     32MHz 48kHz PAL       |------| 424800                      |
  |                (000608)   |058800|                             |
  |    RESET_SW               |------| M514256                     |
  |                         |--------| M514256                     |
  |J                        |TMS57002|                             |
  |A                        |--------| MACH110                     |
  |M  000180          53.693175MHz    (000619A)                    |
  |M                        |--------|                             |
  |A  KM4216V256 KM4216V256 |CXD8538Q|           PAL (000613)      |
  |   KM4216V256 KM4216V256 |        |           PAL (000612)      |
  | TEST_SW                 |--------|           PAL (000618)      |
  |   CXD2923AR        67.7376MHz           93C46   NCR53CF96-2    |
  | DIPSW(8)           |---------|          420B03.27P    HDD_LED  |
  |  KM48V514 KM48V514 |CXD8530BQ|             |-----------------| |
  |  KM48V514 KM48V514 |         |             |543MB SCSI HDRIVE| |
  |CN3  UPA1556        |---------|             |TOSHIBA MK1924FBV| |
  |  KM48V514 KM48V514                         |(420UAA04)       | |
  |  KM48V514 KM48V514                         |-----------------| |
  |----------------------------------------------------------------|

  Notes:
        CN6/7/8 - 5-pin connector for guns. All 3 connectors are wired the same.
                  Pinout is...
                  1- gun opto
                  2- ground
                  3- trigger
                  4- +5v
                  5- reload (see below)
                 
                  The Crypt Killer shotguns have 6 wires coming out of the gun.
                  The wire colors and connections are....
                  Black  - Ground; joined to CN6/7/8 pin 2
                  Red    - +5v; joined to CN6/7/8 pin 4
                  Yellow - Gun opto; joined to CN6/7/8 pin 1
                  White  - Trigger; joined to CN6/7/8 pin 3
                  Purple - Reload; joined to CN6/7/8 pin 5
                  Grey   - Reload ground; this wire must be joined to any ground for the reload to work
                 
        CN3     - For connection of extra controls/buttons (e.g. player 3 start etc)
        CN14    - For connection of additional speaker for stereo output
        68000   - Clock input 8.000MHz [32/4]
        53CF96  - Clock input 16.000MHz [32/2]
        TMS57002- Clock input 24.000MHz [48/2]
        056602  - Konami custom ceramic module. Clock inputs 18.432MHz, 1.536MHz[18.432/12], 48kHz
        000180  - Konami custom ceramic module containing LM1203 and resistors/caps etc
        Vsync   - 59.825Hz
        HSync   - 15.613kHz
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/psx/psx.h"
#include "cpu/tms57002/tms57002.h"
#include "video/psx.h"
#include "machine/am53cf96.h"
#include "machine/eepromser.h"
#include "machine/mb89371.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "sound/k056800.h"
#include "sound/k054539.h"

class konamigq_state : public driver_device
{
public:
	konamigq_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_dasp(*this, "dasp"),
		m_am53cf96(*this, "am53cf96"),
		m_k056800(*this, "k056800")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<tms57002_device> m_dasp;
	required_device<am53cf96_device> m_am53cf96;
	required_device<k056800_device> m_k056800;

	UINT8 *m_p_n_pcmram;
	UINT8 m_sector_buffer[ 512 ];
	UINT8 m_sound_ctrl;
	UINT8 m_sound_intck;

	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE8_MEMBER(pcmram_w);
	DECLARE_READ8_MEMBER(pcmram_r);
	DECLARE_READ16_MEMBER(tms57002_data_word_r);
	DECLARE_WRITE16_MEMBER(tms57002_data_word_w);
	DECLARE_READ16_MEMBER(tms57002_status_word_r);
	DECLARE_WRITE16_MEMBER(tms57002_control_word_w);
	DECLARE_DRIVER_INIT(konamigq);
	DECLARE_MACHINE_START(konamigq);
	DECLARE_MACHINE_RESET(konamigq);
	INTERRUPT_GEN_MEMBER(tms_sync);
	DECLARE_WRITE_LINE_MEMBER(k054539_irq_gen);

	void scsi_dma_read( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size );
	void scsi_dma_write( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size );
};

/* EEPROM */

static const UINT16 konamigq_def_eeprom[64] =
{
	0x292b, 0x5256, 0x2094, 0x4155, 0x0041, 0x1414, 0x0003, 0x0101,
	0x0103, 0x0000, 0x0707, 0x0001, 0xaa00, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
};

WRITE16_MEMBER(konamigq_state::eeprom_w)
{
	ioport("EEPROMOUT")->write(data & 0x07, 0xff);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ( data & 0x40 ) ? CLEAR_LINE : ASSERT_LINE );
}


/* PCM RAM */

WRITE8_MEMBER(konamigq_state::pcmram_w)
{
	m_p_n_pcmram[ offset ] = data;
}

READ8_MEMBER(konamigq_state::pcmram_r)
{
	return m_p_n_pcmram[ offset ];
}

/* Video */

static ADDRESS_MAP_START( konamigq_map, AS_PROGRAM, 32, konamigq_state )
	AM_RANGE(0x1f000000, 0x1f00001f) AM_DEVREADWRITE8("am53cf96", am53cf96_device, read, write, 0x00ff00ff)
	AM_RANGE(0x1f100000, 0x1f10001f) AM_DEVREADWRITE8("k056800", k056800_device, host_r, host_w, 0x00ff00ff)
	AM_RANGE(0x1f180000, 0x1f180003) AM_WRITE16(eeprom_w, 0x0000ffff)
	AM_RANGE(0x1f198000, 0x1f198003) AM_WRITENOP            /* cabinet lamps? */
	AM_RANGE(0x1f1a0000, 0x1f1a0003) AM_WRITENOP            /* indicates gun trigger */
	AM_RANGE(0x1f200000, 0x1f200003) AM_READ_PORT("GUNX1")
	AM_RANGE(0x1f208000, 0x1f208003) AM_READ_PORT("GUNY1")
	AM_RANGE(0x1f210000, 0x1f210003) AM_READ_PORT("GUNX2")
	AM_RANGE(0x1f218000, 0x1f218003) AM_READ_PORT("GUNY2")
	AM_RANGE(0x1f220000, 0x1f220003) AM_READ_PORT("GUNX3")
	AM_RANGE(0x1f228000, 0x1f228003) AM_READ_PORT("GUNY3")
	AM_RANGE(0x1f230000, 0x1f230003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x1f230004, 0x1f230007) AM_READ_PORT("P3_SERVICE")
	AM_RANGE(0x1f238000, 0x1f238003) AM_READ_PORT("DSW")
	AM_RANGE(0x1f300000, 0x1f5fffff) AM_READWRITE8(pcmram_r, pcmram_w, 0x00ff00ff)
	AM_RANGE(0x1f680000, 0x1f68001f) AM_DEVREADWRITE8("mb89371", mb89371_device, read, write, 0x00ff00ff)
	AM_RANGE(0x1f780000, 0x1f780003) AM_WRITENOP /* watchdog? */
ADDRESS_MAP_END

/* SOUND CPU */

INTERRUPT_GEN_MEMBER(konamigq_state::tms_sync)
{
	// DASP is synced to the LRCLK of the 058141
	if (m_sound_ctrl & 0x20)
		m_dasp->sync_w(1);
}

READ16_MEMBER(konamigq_state::tms57002_data_word_r)
{
	return m_dasp->data_r(space, 0);
}

WRITE16_MEMBER(konamigq_state::tms57002_data_word_w)
{
	if (ACCESSING_BITS_0_7)
		m_dasp->data_w(space, 0, data);
}

READ16_MEMBER(konamigq_state::tms57002_status_word_r)
{
	return (m_dasp->dready_r() ? 4 : 0) |
		(m_dasp->pc0_r() ? 2 : 0) |
		(m_dasp->empty_r() ? 1 : 0);
}

WRITE16_MEMBER(konamigq_state::tms57002_control_word_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (!(data & 1))
			m_soundcpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);

		m_dasp->pload_w(data & 4);
		m_dasp->cload_w(data & 8);
		m_dasp->set_input_line(INPUT_LINE_RESET, data & 0x10 ? CLEAR_LINE : ASSERT_LINE);

		m_sound_ctrl = data;
	}
}

/* 68000 memory handling - near identical to Konami GX */
static ADDRESS_MAP_START( konamigq_sound_map, AS_PROGRAM, 16, konamigq_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x2004ff) AM_DEVREADWRITE8("k054539_1", k054539_device, read, write, 0xff00)
	AM_RANGE(0x200000, 0x2004ff) AM_DEVREADWRITE8("k054539_2", k054539_device, read, write, 0x00ff)
	AM_RANGE(0x300000, 0x300001) AM_READWRITE(tms57002_data_word_r,tms57002_data_word_w)
	AM_RANGE(0x400000, 0x40001f) AM_DEVREADWRITE8("k056800", k056800_device, sound_r, sound_w, 0x00ff)
	AM_RANGE(0x500000, 0x500001) AM_READWRITE(tms57002_status_word_r,tms57002_control_word_w)
	AM_RANGE(0x580000, 0x580001) AM_WRITENOP // 'NRES' - D2: K056602 /RESET
ADDRESS_MAP_END


/* TMS57002 memory handling */
static ADDRESS_MAP_START( konamigq_dasp_map, AS_DATA, 8, konamigq_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM
ADDRESS_MAP_END


/* 058141 */
WRITE_LINE_MEMBER(konamigq_state::k054539_irq_gen)
{
	if (m_sound_ctrl & 1)
	{
		// Trigger an interrupt on the rising edge
		if (!m_sound_intck && state)
			m_soundcpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
	}

	m_sound_intck = state;
}

/* SCSI */

void konamigq_state::scsi_dma_read( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
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
		m_am53cf96->dma_read_data( n_this * 4, sector_buffer );
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

void konamigq_state::scsi_dma_write( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
}

DRIVER_INIT_MEMBER(konamigq_state,konamigq)
{
	m_p_n_pcmram = memregion( "shared" )->base() + 0x80000;
}

MACHINE_START_MEMBER(konamigq_state,konamigq)
{
	save_pointer(NAME(m_p_n_pcmram), 0x380000);
	save_item(NAME(m_sector_buffer));
	save_item(NAME(m_sound_ctrl));
	save_item(NAME(m_sound_intck));
}

MACHINE_RESET_MEMBER(konamigq_state,konamigq)
{
}

static MACHINE_CONFIG_START( konamigq, konamigq_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", CXD8530BQ, XTAL_67_7376MHz)
	MCFG_CPU_PROGRAM_MAP(konamigq_map)

	MCFG_RAM_MODIFY("maincpu:ram")
	MCFG_RAM_DEFAULT_SIZE("4M")

	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 5, psx_dma_read_delegate( FUNC( konamigq_state::scsi_dma_read ), (konamigq_state *) owner ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 5, psx_dma_write_delegate( FUNC( konamigq_state::scsi_dma_write ), (konamigq_state *) owner ) )

	MCFG_CPU_ADD("soundcpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(konamigq_sound_map)

	MCFG_CPU_ADD("dasp", TMS57002, 24000000/2)
	MCFG_CPU_DATA_MAP(konamigq_dasp_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(konamigq_state, tms_sync, 48000)

	MCFG_MACHINE_START_OVERRIDE(konamigq_state, konamigq)
	MCFG_MACHINE_RESET_OVERRIDE(konamigq_state, konamigq)

	MCFG_DEVICE_ADD("mb89371", MB89371, 0)
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DATA(konamigq_def_eeprom, 128)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_0)

	MCFG_DEVICE_ADD("am53cf96", AM53CF96, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_AM53CF96_IRQ_HANDLER(DEVWRITELINE("maincpu:irq", psxirq_device, intin10))

	/* video hardware */
	MCFG_PSXGPU_ADD("maincpu", "gpu", CXD8538Q, 0x200000, XTAL_53_693175MHz)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_K056800_ADD("k056800", XTAL_18_432MHz)
	MCFG_K056800_INT_HANDLER(INPUTLINE("soundcpu", M68K_IRQ_1))

	MCFG_DEVICE_ADD("k054539_1", K054539, XTAL_18_432MHz)
	MCFG_K054539_REGION_OVERRRIDE("shared")
	MCFG_K054539_TIMER_HANDLER(WRITELINE(konamigq_state, k054539_irq_gen))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("k054539_2", K054539, XTAL_18_432MHz)
	MCFG_K054539_REGION_OVERRRIDE("shared")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static INPUT_PORTS_START( konamigq )
	PORT_START("GUNX1")
	PORT_BIT( 0x01ff, 0x011d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x007d, 0x01bc ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNY1")
	PORT_BIT( 0x00ff, 0x0078, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0x01ff, 0x011d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x007d, 0x01bc ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("GUNY2")
	PORT_BIT( 0x00ff, 0x0078, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("GUNX3")
	PORT_BIT( 0x01ff, 0x011d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x007d, 0x01bc ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START("GUNY3")
	PORT_BIT( 0x00ff, 0x0078, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START("P1_P2")
	PORT_BIT( 0x000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* trigger */
	PORT_BIT( 0x000010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* reload */
	PORT_BIT( 0x000020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x010000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x020000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* trigger */
	PORT_BIT( 0x100000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* reload */
	PORT_BIT( 0x200000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x800000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_SERVICE")
	PORT_BIT( 0x000001, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x000002, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) /* trigger */
	PORT_BIT( 0x000010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) /* reload */
	PORT_BIT( 0x000020, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x010000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x800000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00000001, 0x01, DEF_STR( Stereo ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Stereo ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Mono ) )
	PORT_DIPNAME( 0x00000002, 0x00, "Stage Set" )
	PORT_DIPSETTING(    0x02, "Endless" )
	PORT_DIPSETTING(    0x00, "6st End" )
	PORT_DIPNAME( 0x00000004, 0x04, "Mirror" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00000008, 0x08, "Woofer" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00000010, 0x10, "Number of Players" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPNAME( 0x00000020, 0x20, "Coin Mechanism (2p only)" )
	PORT_DIPSETTING(    0x20, "Common" )
	PORT_DIPSETTING(    0x00, "Independent" )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
INPUT_PORTS_END

ROM_START( cryptklr )
	ROM_REGION( 0x80000, "soundcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "420a01.2g", 0x000000, 0x080000, CRC(84fc2613) SHA1(e06f4284614d33c76529eb43b168d095200a9eac) )

	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "420a02.3m",    0x000000, 0x080000, CRC(2169c3c4) SHA1(6d525f10385791e19eb1897d18f0bab319640162) )

	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) /* bios */
	ROM_LOAD( "420b03.27p",   0x0000000, 0x080000, CRC(aab391b1) SHA1(bf9dc7c0c8168c22a4be266fe6a66d3738df916b) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":harddisk:image" )
	DISK_IMAGE( "420uaa04", 0, SHA1(67cb1418fc0de2a89fc61847dc9efb9f1bebb347) )
ROM_END

GAME( 1995, cryptklr, 0, konamigq, konamigq, konamigq_state, konamigq, ROT0, "Konami", "Crypt Killer (GQ420 UAA)", MACHINE_IMPERFECT_GRAPHICS )
