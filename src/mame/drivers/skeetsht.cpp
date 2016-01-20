// license:BSD-3-Clause
// copyright-holders:Philip Bennett, Angelo Salese
/***************************************************************************

    Dynamo Skeet Shot

    Notes:
        Pop Shot is a prototype sequel (or upgrade) to Skeet Shot

        Supposedly Skeet Shot used a laserdisc to supply video for eight
        different background "scenes". Pop Shot probably did too, in that case.

***************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "cpu/tms34010/tms34010.h"
#include "sound/ay8910.h"
#include "video/tlc34076.h"


/*************************************
 *
 *  Structs
 *
 *************************************/

class skeetsht_state : public driver_device
{
public:
	skeetsht_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_tlc34076(*this, "tlc34076"),
		m_tms_vram(*this, "tms_vram"),
		m_68hc11(*this, "68hc11"),
		m_ay(*this, "aysnd"),
		m_tms(*this, "tms")
	{
	}

	required_device<tlc34076_device> m_tlc34076;
	required_shared_ptr<UINT16> m_tms_vram;
	UINT8 m_porta_latch;
	UINT8 m_ay_sel;
	UINT8 m_lastdataw;
	UINT16 m_lastdatar;
	DECLARE_READ16_MEMBER(ramdac_r);
	DECLARE_WRITE16_MEMBER(ramdac_w);
	DECLARE_WRITE8_MEMBER(tms_w);
	DECLARE_READ8_MEMBER(tms_r);
	DECLARE_READ8_MEMBER(hc11_porta_r);
	DECLARE_WRITE8_MEMBER(hc11_porta_w);
	DECLARE_WRITE8_MEMBER(ay8910_w);
	DECLARE_WRITE_LINE_MEMBER(tms_irq);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_device<cpu_device> m_68hc11;
	required_device<ay8910_device> m_ay;
	required_device<tms34010_device> m_tms;
};


/*************************************
 *
 *  Initialisation
 *
 *************************************/

void skeetsht_state::machine_reset()
{
}


/*************************************
 *
 *  Video Functions
 *
 *************************************/

void skeetsht_state::video_start()
{
}

TMS340X0_SCANLINE_RGB32_CB_MEMBER(skeetsht_state::scanline_update)
{
	const rgb_t *const pens = m_tlc34076->get_pens();
	UINT16 *vram = &m_tms_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = &bitmap.pix32(scanline);
	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = pens[pixels & 0xff];
		dest[x + 1] = pens[pixels >> 8];
	}
}

READ16_MEMBER(skeetsht_state::ramdac_r)
{
	offset = (offset >> 12) & ~4;

	if (offset & 8)
		offset = (offset & ~8) | 4;

	return m_tlc34076->read(space, offset);
}

WRITE16_MEMBER(skeetsht_state::ramdac_w)
{
	offset = (offset >> 12) & ~4;

	if (offset & 8)
		offset = (offset & ~8) | 4;

	m_tlc34076->write(space, offset, data);
}


/*************************************
 *
 *  CPU Communications
 *
 *************************************/

WRITE_LINE_MEMBER(skeetsht_state::tms_irq)
{
	m_68hc11->set_input_line(MC68HC11_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


WRITE8_MEMBER(skeetsht_state::tms_w)
{
	if ((offset & 1) == 0)
		m_lastdataw = data;
	else
		m_tms->host_w(space, offset >> 1, (m_lastdataw << 8) | data, 0xffff);
}

READ8_MEMBER(skeetsht_state::tms_r)
{
	if ((offset & 1) == 0)
		m_lastdatar = m_tms->host_r(space, offset >> 1, 0xffff);

	return m_lastdatar >> ((offset & 1) ? 0 : 8);
}


/*************************************
 *
 *  I/O
 *
 *************************************/

READ8_MEMBER(skeetsht_state::hc11_porta_r)
{
	return m_porta_latch;
}

WRITE8_MEMBER(skeetsht_state::hc11_porta_w)
{
	if (!(data & 0x8) && (m_porta_latch & 8))
		m_ay_sel = m_porta_latch & 0x10;

	m_porta_latch = data;
}

WRITE8_MEMBER(skeetsht_state::ay8910_w)
{
	if (m_ay_sel)
		m_ay->data_w(space, 0, data);
	else
		m_ay->address_w(space, 0, data);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( hc11_pgm_map, AS_PROGRAM, 8, skeetsht_state )
	AM_RANGE(0x2800, 0x2807) AM_READWRITE(tms_r, tms_w)
	AM_RANGE(0x1800, 0x1800) AM_WRITE(ay8910_w)
	AM_RANGE(0xb600, 0xbdff) AM_RAM //internal EEPROM
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_REGION("68hc11", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hc11_io_map, AS_IO, 8, skeetsht_state )
	AM_RANGE(MC68HC11_IO_PORTA, MC68HC11_IO_PORTA) AM_READWRITE(hc11_porta_r, hc11_porta_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Video CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( tms_program_map, AS_PROGRAM, 16, skeetsht_state )
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("tms", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("tms_vram")
	AM_RANGE(0x00440000, 0x004fffff) AM_READWRITE(ramdac_r, ramdac_w)
	AM_RANGE(0xff800000, 0xffbfffff) AM_ROM AM_MIRROR(0x00400000) AM_REGION("tms", 0)
ADDRESS_MAP_END


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( skeetsht )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( skeetsht, skeetsht_state )

	MCFG_CPU_ADD("68hc11", MC68HC11, 4000000) // ?
	MCFG_CPU_PROGRAM_MAP(hc11_pgm_map)
	MCFG_CPU_IO_MAP(hc11_io_map)
	MCFG_MC68HC11_CONFIG( 0, 0x100, 0x01 )  // And 512 bytes EEPROM? (68HC11A1)

	MCFG_CPU_ADD("tms", TMS34010, 48000000)
	MCFG_CPU_PROGRAM_MAP(tms_program_map)
	MCFG_TMS340X0_HALT_ON_RESET(TRUE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(48000000 / 8) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(1) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_RGB32_CB(skeetsht_state, scanline_update)   /* scanline updater (rgb32) */
	MCFG_TMS340X0_OUTPUT_INT_CB(WRITELINE(skeetsht_state, tms_irq))

	MCFG_TLC34076_ADD("tlc34076", TLC34076_6_BIT)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(48000000 / 8, 156*4, 0, 100*4, 328, 0, 300) // FIXME
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms34010_device, tms340x0_rgb32)


	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000) // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( skeetsht )
	ROM_REGION( 0x40000, "68hc11", 0 )
	ROM_LOAD( "hc_11_v1.2.u34",  0x00000, 0x20000, CRC(b9801dea) SHA1(bc5bcd29b5880081c87b4014eb2c9c5077024db1) )
	ROM_LOAD( "sound.u35",       0x20000, 0x20000, CRC(0d9be853) SHA1(51eda4e0a99d50e09476704eb75310b5ee2690f4) )

	ROM_REGION16_LE( 0x200000, "tms", 0 )
	ROM_LOAD16_BYTE( "even_v1.2.u14", 0x000000, 0x40000, CRC(c7c9515e) SHA1(ce3e813c15085790d5335d9fc751b3cc5b617b20) )
	ROM_LOAD16_BYTE( "odd_v1.2.u13",  0x000001, 0x40000, CRC(ea4402fb) SHA1(b0b6b191a8b48bead660a385c638363943a6ffe2) )

	DISK_REGION( "laserdisc" )
		DISK_IMAGE_READONLY( "skeetsht", 0, NO_DUMP ) // unknown disc label?
ROM_END

ROM_START( popshot )
	ROM_REGION( 0x40000, "68hc11", 0 )
	ROM_LOAD( "popshot_hc68.u34",  0x00000, 0x20000, CRC(90de8bd3) SHA1(1809f209ead8e304464c697f42e9f6ac2d0ca594) )
	ROM_LOAD( "sound.u35",         0x20000, 0x20000, CRC(0d9be853) SHA1(51eda4e0a99d50e09476704eb75310b5ee2690f4) )

	ROM_REGION16_LE( 0x200000, "tms", 0 )
	ROM_LOAD16_BYTE( "popshot_tms34_even.u14", 0x000000, 0x80000, CRC(bf2f7309) SHA1(6ca252f857e5dc2e5267c176403c44e7a15f539e) )
	ROM_LOAD16_BYTE( "popshot_tms34_odd.u13",  0x000001, 0x80000, CRC(82d616d8) SHA1(83ab33727ebab882b79c9ebd3557e2c319b3387a) )

	DISK_REGION( "laserdisc" )
		DISK_IMAGE_READONLY( "popshot", 0, NO_DUMP ) // unknown disc label?
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, skeetsht, 0, skeetsht, skeetsht, driver_device, 0, ROT0, "Dynamo", "Skeet Shot", MACHINE_NOT_WORKING )
GAME( 1991, popshot,  0, skeetsht, skeetsht, driver_device, 0, ROT0, "Dynamo", "Pop Shot (prototype)", MACHINE_NOT_WORKING )
