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
#include "screen.h"
#include "speaker.h"


namespace {

/*************************************
 *
 *  Structs
 *
 *************************************/

class skeetsht_state : public driver_device
{
public:
	skeetsht_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tlc34076(*this, "tlc34076"),
		m_tms_vram(*this, "tms_vram"),
		m_68hc11(*this, "68hc11"),
		m_ay(*this, "aysnd"),
		m_tms(*this, "tms")
	{
	}

	void skeetsht(machine_config &config);

private:
	required_device<tlc34076_device> m_tlc34076;
	required_shared_ptr<uint16_t> m_tms_vram;
	uint8_t m_porta_latch = 0;
	uint8_t m_ay_sel = 0;
	uint8_t m_lastdataw = 0;
	uint16_t m_lastdatar = 0;
	uint16_t ramdac_r(offs_t offset);
	void ramdac_w(offs_t offset, uint16_t data);
	void tms_w(offs_t offset, uint8_t data);
	uint8_t tms_r(offs_t offset);
	void hc11_porta_w(uint8_t data);
	void ay8910_w(uint8_t data);
	void tms_irq(int state);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	required_device<mc68hc11_cpu_device> m_68hc11;
	required_device<ay8910_device> m_ay;
	required_device<tms34010_device> m_tms;
	void hc11_pgm_map(address_map &map) ATTR_COLD;
	void tms_program_map(address_map &map) ATTR_COLD;
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
	pen_t const *const pens = m_tlc34076->pens();
	uint16_t *vram = &m_tms_vram[(params->rowaddr << 8) & 0x3ff00];
	uint32_t *const dest = &bitmap.pix(scanline);
	int coladdr = params->coladdr;

	for (int x = params->heblnk; x < params->hsblnk; x += 2)
	{
		uint16_t pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = pens[pixels & 0xff];
		dest[x + 1] = pens[pixels >> 8];
	}
}

uint16_t skeetsht_state::ramdac_r(offs_t offset)
{
	offset = (offset >> 12) & ~4;

	if (offset & 8)
		offset = (offset & ~8) | 4;

	return m_tlc34076->read(offset);
}

void skeetsht_state::ramdac_w(offs_t offset, uint16_t data)
{
	offset = (offset >> 12) & ~4;

	if (offset & 8)
		offset = (offset & ~8) | 4;

	m_tlc34076->write(offset, data);
}


/*************************************
 *
 *  CPU Communications
 *
 *************************************/

void skeetsht_state::tms_irq(int state)
{
	m_68hc11->set_input_line(MC68HC11_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


void skeetsht_state::tms_w(offs_t offset, uint8_t data)
{
	if ((offset & 1) == 0)
		m_lastdataw = data;
	else
		m_tms->host_w(offset >> 1, (m_lastdataw << 8) | data);
}

uint8_t skeetsht_state::tms_r(offs_t offset)
{
	if ((offset & 1) == 0)
		m_lastdatar = m_tms->host_r(offset >> 1);

	return m_lastdatar >> ((offset & 1) ? 0 : 8);
}


/*************************************
 *
 *  I/O
 *
 *************************************/

void skeetsht_state::hc11_porta_w(uint8_t data)
{
	if (!(data & 0x8) && (m_porta_latch & 8))
		m_ay_sel = m_porta_latch & 0x10;

	m_porta_latch = data;
}

void skeetsht_state::ay8910_w(uint8_t data)
{
	if (m_ay_sel)
		m_ay->data_w(data);
	else
		m_ay->address_w(data);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void skeetsht_state::hc11_pgm_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("68hc11", 0);
	map(0x1800, 0x1800).w(FUNC(skeetsht_state::ay8910_w));
	map(0x2800, 0x2807).rw(FUNC(skeetsht_state::tms_r), FUNC(skeetsht_state::tms_w));
}


/*************************************
 *
 *  Video CPU memory handlers
 *
 *************************************/

void skeetsht_state::tms_program_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("tms_vram");
	map(0x00440000, 0x004fffff).rw(FUNC(skeetsht_state::ramdac_r), FUNC(skeetsht_state::ramdac_w));
	map(0xff800000, 0xffbfffff).rom().mirror(0x00400000).region("tms", 0);
}


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

void skeetsht_state::skeetsht(machine_config &config)
{
	MC68HC11A1(config, m_68hc11, 8000000); // ?
	m_68hc11->set_addrmap(AS_PROGRAM, &skeetsht_state::hc11_pgm_map);
	m_68hc11->out_pa_callback().set(FUNC(skeetsht_state::hc11_porta_w));

	TMS34010(config, m_tms, 48000000);
	m_tms->set_addrmap(AS_PROGRAM, &skeetsht_state::tms_program_map);
	m_tms->set_halt_on_reset(true);
	m_tms->set_pixel_clock(48000000 / 8);
	m_tms->set_pixels_per_clock(1);
	m_tms->set_scanline_rgb32_callback(FUNC(skeetsht_state::scanline_update));
	m_tms->output_int().set(FUNC(skeetsht_state::tms_irq));

	TLC34076(config, m_tlc34076, 0);
	m_tlc34076->set_bits(tlc34076_device::TLC34076_6_BIT);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(48000000 / 8, 156*4, 0, 100*4, 328, 0, 300); // FIXME
	screen.set_screen_update("tms", FUNC(tms34010_device::tms340x0_rgb32));

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay, 2000000).add_route(ALL_OUTPUTS, "mono", 0.50); // ?
}


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

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, skeetsht, 0, skeetsht, skeetsht, skeetsht_state, empty_init, ROT0, "Dynamo", "Skeet Shot",           MACHINE_NOT_WORKING )
GAME( 1991, popshot,  0, skeetsht, skeetsht, skeetsht_state, empty_init, ROT0, "Dynamo", "Pop Shot (prototype)", MACHINE_NOT_WORKING )
