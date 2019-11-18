// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/***************************************************************************

  IBM Professional Graphics Controller (PGC).

  Designed for IBM by Vermont Microsystems.  References:

  IBM Options and Adapters manual
    http://www.minuszerodegrees.net/oa/OA%20-%20IBM%20Professional%20Graphics%20Controller.pdf
    http://bitsavers.org/pdf/ibm/pc/cards/Technical_Reference_Options_and_Adapters_Volume_3.pdf
  IBM Systems Journal white paper
    http://wayback.archive.org/web/20061015235146/http://www.research.ibm.com/journal/sj/241/ibmsj2401D.pdf
  John Elliott's page
    http://www.seasip.info/VintagePC/pgc.html

  To do:
  - pass IBM diagnostics (currently fail with code 3905)
  - CGA emulator
  - what's up with irq 3 (= vblank irq)? (causes soft reset)
  - "test pin of the microprocessor samples the hsync pulse"
  - bus state handling?
  - VRAM address translator ROM?
  - clones/compatibles?  CompuShow apparently was tested with a clone and sends opcode E6, writes 2 to C630C

***************************************************************************/

#include "emu.h"
#include "pgc.h"

#include "screen.h"


//#define LOG_GENERAL (1U << 0) //defined in logmacro.h already
#define LOG_VRAM    (1U << 1)
#define LOG_CMD     (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_VRAM)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGV(...)   LOGMASKED(LOG_VRAM, __VA_ARGS__)
#define LOGCMD(...) LOGMASKED(LOG_CMD,  __VA_ARGS__)


#define PGC_SCREEN_NAME "pgc_screen"

#define PGC_TOTAL_HORZ 820
#define PGC_DISP_HORZ  640
#define PGC_HORZ_START 80

#define PGC_TOTAL_VERT 508
#define PGC_DISP_VERT  480
#define PGC_VERT_START 10

/*
    Prototypes
*/

ROM_START( pgc )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_DEFAULT_BIOS("1985")

	ROM_SYSTEM_BIOS(0, "1984", "1984 firmware, P/N 6137322/3")
	ROMX_LOAD("ibm_6137323_pgc_card_27256.bin", 0x00000, 0x8000, CRC(f564f342) SHA1(c5ef17fd1569043cb59f61faf828ea8b0ee95526), ROM_BIOS(0))
	ROMX_LOAD("ibm_6137322_pgc_card_27256.bin", 0x08000, 0x8000, CRC(5e6cc82f) SHA1(45b3ffb5a9c51986862f8d47b3e03dcaaf4073d5), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "1985", "1985 firmware, P/N 59X7354/5")
	ROMX_LOAD("pgc_u44.bin", 0x00000, 0x8000, CRC(71280241) SHA1(7042ccd4ebd03f576a256a433b8aa38d1b4fefa8), ROM_BIOS(1))
	ROMX_LOAD("pgc_u43.bin", 0x08000, 0x8000, CRC(923f5ea3) SHA1(2b2a55d64b20d3a613b00c51443105aa03eca5d6), ROM_BIOS(1))

	ROM_REGION(0x800, "commarea", ROMREGION_ERASE00)

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("pgc_u27.bin", 0x0000, 0x1000, CRC(6be256cc) SHA1(deb1195886268dcddce10459911e020f7a9f74f7))
ROM_END

static INPUT_PORTS_START( pgc )
	PORT_START("DSW")
/*
    PORT_DIPNAME( 0x01, 0x00, "CGA emulator")
    PORT_DIPSETTING(    0x00, DEF_STR(No) )
    PORT_DIPSETTING(    0x01, DEF_STR(Yes) )
*/
	PORT_DIPNAME( 0x02, 0x00, "Communication area")
	PORT_DIPSETTING(    0x00, "C6000" )
	PORT_DIPSETTING(    0x02, "C6400" )
INPUT_PORTS_END

/*
write only
    30000       LUT WR O L
    30001       LUT WR I L
    32000       MODE WT L
    32001       NIBBLE WT L
    3200A       ??
    34000       FUNCTION WT L
    34001       STARTADD WT L
    36001       CURSOR WT L

read only
    38000       LUT RD O L
    38001       LUT RD I L
    3C001       INIT L/INIT H
*/

void isa8_pgc_device::pgc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x07fff).rom();
	map(0x08000, 0x0ffff).rom().region("maincpu", 0x8000);
	map(0x10000, 0x1001f).rw(FUNC(isa8_pgc_device::stateparam_r), FUNC(isa8_pgc_device::stateparam_w));
//  map(0x18000, 0x18fff).ram();   // ??
	map(0x28000, 0x287ff).ram().region("commarea", 0).mirror(0x800);
	map(0x32001, 0x32001).nopw();
	map(0x32020, 0x3203f).w(FUNC(isa8_pgc_device::accel_w));
	map(0x3c000, 0x3c001).r(FUNC(isa8_pgc_device::init_r));
//  map(0x3e000, 0x3efff).ram();   // ??
	map(0x80000, 0xf7fff).rw(FUNC(isa8_pgc_device::vram_r), FUNC(isa8_pgc_device::vram_w));
	map(0xf8000, 0xfffff).rom().region("maincpu", 0x8000);
}

static const gfx_layout pgc_charlayout =
{
	8, 16,                  /* 8x16 pixels */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 10 bytes */
};

static GFXDECODE_START( gfx_pgc )
	GFXDECODE_REVERSEBITS("chargen", 0, pgc_charlayout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_PGC, isa8_pgc_device, "isa_ibm_pgc", "IBM Professional Graphics Controller")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_pgc_device::device_add_mconfig(machine_config &config)
{
	I8088(config, m_cpu, XTAL(24'000'000)/3);
	m_cpu->set_addrmap(AS_PROGRAM, &isa8_pgc_device::pgc_map);
#if 0
	m_cpu->set_irq_acknowledge_callback(FUNC(isa8_pgc_device::irq_callback));
#endif

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(50'000'000)/2,
		PGC_TOTAL_HORZ, PGC_HORZ_START, PGC_HORZ_START+PGC_DISP_HORZ,
		PGC_TOTAL_VERT, PGC_VERT_START, PGC_VERT_START+PGC_DISP_VERT);
	m_screen->set_screen_update(FUNC(isa8_pgc_device::screen_update));
	m_screen->set_palette(m_palette);
#if 0
	m_screen->screen_vblank().set(FUNC(isa8_pgc_device::vblank_irq));
#endif

	GFXDECODE(config, "gfxdecode", m_palette, gfx_pgc);
	PALETTE(config, m_palette).set_entries(256);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_pgc_device::device_rom_region() const
{
	return ROM_NAME(pgc);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor isa8_pgc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pgc);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_pgc_device - constructor
//-------------------------------------------------

isa8_pgc_device::isa8_pgc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_pgc_device(mconfig, ISA8_PGC, tag, owner, clock)
{
}

isa8_pgc_device::isa8_pgc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_cpu(*this, "maincpu"),
	m_screen(*this, PGC_SCREEN_NAME),
	m_palette(*this, "palette"),
	m_commarea(nullptr), m_vram(nullptr), m_eram(nullptr)
{
}

void isa8_pgc_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	set_isa_device();

	for (int i = 0; i < 256; i++)
	{
		m_palette->set_pen_color(i, 0, 0, 0);
	}

	m_vram = std::make_unique<uint8_t[]>(0x78000);
	m_eram = std::make_unique<uint8_t[]>(0x8000);

	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(&isa8_pgc_device::reset_common, this));
}

void isa8_pgc_device::reset_common()
{
	address_space &space = m_cpu->space(AS_PROGRAM);

	space.unmap_readwrite(0xf8000, 0xfffff);
	space.install_rom(0xf8000, 0xfffff, memregion("maincpu")->base() + 0x8000);
}

void isa8_pgc_device::device_reset()
{
	memset(m_stateparam, 0, sizeof(m_stateparam));
	memset(m_lut, 0, sizeof(m_lut));
	m_accel = 0;

	m_commarea = memregion("commarea")->base();
	if (BIT(ioport("DSW")->read(), 1))
		m_isa->install_bank(0xc6400, 0xc67ff, "commarea", m_commarea);
	else
		m_isa->install_bank(0xc6000, 0xc63ff, "commarea", m_commarea);
}

//

WRITE_LINE_MEMBER(isa8_pgc_device::vblank_irq)
{
	if (state)
	{
		LOGCMD("vblank_irq\n");
		m_cpu->set_input_line(0, ASSERT_LINE);
	}
}

IRQ_CALLBACK_MEMBER(isa8_pgc_device::irq_callback)
{
	LOGCMD("irq_callback\n");
	m_cpu->set_input_line(0, CLEAR_LINE);
	return 3;
}

// memory handlers

READ8_MEMBER(isa8_pgc_device::vram_r)
{
	uint8_t ret;

	ret = m_vram[offset];
	LOGV("vram R @ %02x == %02x\n", offset, ret);
	return ret;
}

/*
 * accel modes (decimal)
 *
 * 0 - none
 * 1 - write 4 pixels, starting at offset
 * 2 - write up to 4 pixels, ending at offset
 * 3 - write up to 4 pixels, starting at offset
 * 5 - write 20 pixels, starting at offset
 * 9 - write up to 5 pixel groups, ending at offset.  offset may be in the middle of pixel group.
 * 13 - write up to 5 pixel groups, starting at offset.
 */
WRITE8_MEMBER(isa8_pgc_device::vram_w)
{
	bool handled = true;

	switch (m_accel)
	{
	case 0:
		m_vram[offset] = data;
		break;

	case 1:
		std::fill(&m_vram[offset], &m_vram[offset + 4], data);
		break;

	case 2:
		std::fill(&m_vram[offset & ~3], &m_vram[offset + 1], data);
		break;

	case 3:
		std::fill(&m_vram[offset], &m_vram[(offset + 4) & ~3], data);
		break;

	case 5:
		std::fill(&m_vram[offset], &m_vram[offset + 20], data);
		break;

	case 9:
		std::fill(&m_vram[offset - ((offset % 1024) % 20)], &m_vram[(offset + 4) & ~3], data);
		break;

	case 13:
		std::fill(&m_vram[offset], &m_vram[(offset + 20) - ((offset % 1024) + 20) % 20], data);
		break;

	default:
		m_vram[offset] = data;
		handled = false;
		break;
	}
	LOGV("vram W @ %02x <- %02x (accel %d)%s\n", offset, data, m_accel,
		 handled ? "" : " (unsupported)");
}

WRITE8_MEMBER(isa8_pgc_device::accel_w)
{
	m_accel = offset >> 1;
	LOGV("accel  @ %05x <- %02x (%d)\n", 0x32020 + offset, data, m_accel);
}

READ8_MEMBER(isa8_pgc_device::stateparam_r)
{
	uint8_t ret;

	ret = m_stateparam[offset >> 1];
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		LOG("stateparam R @ %02x == %02x\n", offset, ret);
	}
	return ret;
}

WRITE8_MEMBER(isa8_pgc_device::stateparam_w)
{
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		LOG("stateparam W @ %02x <- %02x\n", offset, data);
	}
	m_stateparam[offset >> 1] = data;
}

WRITE8_MEMBER(isa8_pgc_device::lut_w)
{
	uint8_t o = (offset >> 1) * 3;

	if (offset & 1)
	{
		m_lut[o + 2] = (data & 15) * 17;
		m_palette->set_pen_color( offset >> 1, m_lut[o], m_lut[o + 1], m_lut[o + 2] );
		LOG("lut W @ %02X <- %d %d %d\n",
			offset >> 1, m_lut[o], m_lut[o + 1], m_lut[o + 2] );
	} else {
		m_lut[o    ] = (data >> 4) * 17;
		m_lut[o + 1] = (data & 15) * 17;
	}
}

READ8_MEMBER(isa8_pgc_device::init_r)
{
	LOG("INIT: unmapping ROM\n");
	space.unmap_read(0xf8000, 0xfffff);

	LOG("INIT: mapping emulator RAM\n");
	space.install_readwrite_bank(0xf8000, 0xfffff, "eram");
	membank("eram")->set_base(m_eram.get());

	LOG("INIT: mapping LUT\n");
	space.install_write_handler(0xf8400, 0xf85ff, write8_delegate(*this, FUNC(isa8_pgc_device::lut_w)));

	return 0; // XXX ignored
}

uint32_t isa8_pgc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t *p;
	uint8_t *v;

	for (int y = 0; y < PGC_DISP_VERT; y++)
	{
		// XXX address translation happens in hardware
		v = &m_vram[y * 1024];
		p = &bitmap.pix16(y + PGC_VERT_START, PGC_HORZ_START);

		for (int x = 0; x < PGC_DISP_HORZ; x++)
		{
			*p++ = *v++;
		}
	}
	return 0;
}
