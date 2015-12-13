// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/***************************************************************************

  IBM Professional Graphics Controller (PGC), skeleton driver.

  Designed for IBM by Vermont Microsystems.  References:

  IBM Options and Adapters manual
    http://www.minuszerodegrees.net/oa/OA%20-%20IBM%20Professional%20Graphics%20Controller.pdf
    http://bitsavers.org/pdf/ibm/pc/cards/Technical_Reference_Options_and_Adapters_Volume_3.pdf
  IBM Systems Journal white paper
    http://wayback.archive.org/web/20061015235146/http://www.research.ibm.com/journal/sj/241/ibmsj2401D.pdf
  John Elliott's page
    http://www.seasip.info/VintagePC/pgc.html

  To do:
  - decode memory map
  - various VRAM write modes
  - what's up with irq 3 (= vblank irq)? (causes soft reset)
  - "test pin of the microprocessor samples the hsync pulse"
  - CGA emulator
  - bus state handling?
  - VRAM address translator ROM?

***************************************************************************/

#include "emu.h"

#include "pgc.h"

#define VERBOSE_PGC     1

#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_PGC>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-24s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)

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
	ROMX_LOAD("ibm_6137323_pgc_card_27256.bin", 0x00000, 0x8000, CRC(f564f342) SHA1(c5ef17fd1569043cb59f61faf828ea8b0ee95526), ROM_BIOS(1))
	ROMX_LOAD("ibm_6137322_pgc_card_27256.bin", 0x08000, 0x8000, CRC(5e6cc82f) SHA1(45b3ffb5a9c51986862f8d47b3e03dcaaf4073d5), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "1985", "1985 firmware, P/N 59X7354/5")
	ROMX_LOAD("pgc_u44.bin", 0x00000, 0x8000, CRC(71280241) SHA1(7042ccd4ebd03f576a256a433b8aa38d1b4fefa8), ROM_BIOS(2))
	ROMX_LOAD("pgc_u43.bin", 0x08000, 0x8000, CRC(923f5ea3) SHA1(2b2a55d64b20d3a613b00c51443105aa03eca5d6), ROM_BIOS(2))

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

static ADDRESS_MAP_START( pgc_map, AS_PROGRAM, 8, isa8_pgc_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x07fff) AM_ROM
	AM_RANGE(0x08000, 0x0ffff) AM_ROM AM_REGION("maincpu", 0x8000)
	AM_RANGE(0x10000, 0x1001f) AM_READWRITE(stateparam_r, stateparam_w)
//  AM_RANGE(0x18000, 0x18fff) AM_RAM   // ??
	AM_RANGE(0x28000, 0x287ff) AM_RAM AM_REGION("commarea", 0) AM_MIRROR(0x800)
	AM_RANGE(0x3c000, 0x3c001) AM_READ(init_r)
//  AM_RANGE(0x3e000, 0x3efff) AM_RAM   // ??
	AM_RANGE(0xf8000, 0xfffff) AM_ROM AM_REGION("maincpu", 0x8000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pgc_io, AS_IO, 8, isa8_pgc_device )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

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

static GFXDECODE_START( pgc )
	GFXDECODE_REVERSEBITS("chargen", 0, pgc_charlayout, 0, 1)
GFXDECODE_END

MACHINE_CONFIG_FRAGMENT( pcvideo_pgc )
	MCFG_CPU_ADD("maincpu", I8088, XTAL_24MHz/3)
	MCFG_CPU_PROGRAM_MAP(pgc_map)
	MCFG_CPU_IO_MAP(pgc_io)
#if 0
	MCFG_CPU_VBLANK_INT_DRIVER(PGC_SCREEN_NAME, isa8_pgc_device, vblank_irq)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(isa8_pgc_device, irq_callback)
#endif

	MCFG_TIMER_DRIVER_ADD_PERIODIC("scantimer", isa8_pgc_device, scanline_callback,
		attotime::from_hz(60*PGC_TOTAL_VERT))
	MCFG_TIMER_START_DELAY(attotime::from_hz(XTAL_50MHz/(2*PGC_HORZ_START)))

	MCFG_SCREEN_ADD(PGC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_50MHz/2,
		PGC_TOTAL_HORZ, PGC_HORZ_START, PGC_HORZ_START+PGC_DISP_HORZ,
		PGC_TOTAL_VERT, PGC_VERT_START, PGC_VERT_START+PGC_DISP_VERT)
	MCFG_SCREEN_UPDATE_DRIVER(isa8_pgc_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pgc)
	MCFG_PALETTE_ADD( "palette", 256 )
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_PGC = &device_creator<isa8_pgc_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_pgc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcvideo_pgc );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_pgc_device::device_rom_region() const
{
	return ROM_NAME( pgc );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor isa8_pgc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pgc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_pgc_device - constructor
//-------------------------------------------------

isa8_pgc_device::isa8_pgc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ISA8_PGC, "IBM Professional Graphics Controller", tag, owner, clock, "isa_ibm_pgc", __FILE__),
	device_isa8_card_interface(mconfig, *this),
	m_cpu(*this, "maincpu"),
	m_screen(*this, PGC_SCREEN_NAME),
	m_palette(*this, "palette"), m_commarea(nullptr), m_vram(nullptr), m_eram(nullptr), m_bitmap(nullptr)
{
}

isa8_pgc_device::isa8_pgc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_isa8_card_interface(mconfig, *this),
	m_cpu(*this, "maincpu"),
	m_screen(*this, PGC_SCREEN_NAME),
	m_palette(*this, "palette"), m_commarea(nullptr), m_vram(nullptr), m_eram(nullptr), m_bitmap(nullptr)
{
}

void isa8_pgc_device::device_start()
{
	address_space &space = m_cpu->space( AS_PROGRAM );
	int width = PGC_DISP_HORZ;
	int height = PGC_DISP_VERT;

	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	set_isa_device();

	for (int i = 0; i < 256; i++ )
	{
		m_palette->set_pen_color( i, 0, 0, 0 );
	}

	m_bitmap = auto_bitmap_ind16_alloc(machine(), width, height);
	m_bitmap->fill(0);

	m_vram = auto_alloc_array(machine(), UINT8, 0x78000);
	space.install_readwrite_bank(0x80000, 0xf7fff, "vram");
	membank("vram")->set_base(m_vram);

	m_eram = auto_alloc_array(machine(), UINT8, 0x8000);

	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(isa8_pgc_device::reset_common), this));
}

void isa8_pgc_device::reset_common()
{
	address_space &space = m_cpu->space( AS_PROGRAM );

	space.unmap_readwrite(0xf8000, 0xfffff);
	space.install_rom(0xf8000, 0xfffff, memregion("maincpu")->base() + 0x8000);
}

void isa8_pgc_device::device_reset()
{
	memset(m_stateparam, 0, sizeof(m_stateparam));
	memset(m_lut, 0, sizeof(m_lut));

	m_commarea = memregion("commarea")->base();
	if (BIT(ioport("DSW")->read(), 1))
		m_isa->install_bank(0xc6400, 0xc67ff, 0, 0, "commarea", m_commarea);
	else
		m_isa->install_bank(0xc6000, 0xc63ff, 0, 0, "commarea", m_commarea);
}

//

INTERRUPT_GEN_MEMBER(isa8_pgc_device::vblank_irq)
{
	DBG_LOG(2,"irq",("vblank_irq\n"));
	m_cpu->set_input_line(0, ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER(isa8_pgc_device::irq_callback)
{
	DBG_LOG(2,"irq",("irq_callback\n"));
	m_cpu->set_input_line(0, CLEAR_LINE);
	return 3;
}

// memory handlers

READ8_MEMBER( isa8_pgc_device::stateparam_r ) {
	UINT8 ret;

	ret = m_stateparam[offset >> 1];
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		DBG_LOG(1,"stateparam",("R @ %02x == %02x\n", offset, ret));
	}
	return ret;
}

WRITE8_MEMBER( isa8_pgc_device::stateparam_w ) {
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		DBG_LOG(1,"stateparam",("W @ %02x <- %02x\n", offset, data));
	}
	m_stateparam[offset >> 1] = data;
}

WRITE8_MEMBER( isa8_pgc_device::lut_w ) {
	UINT8 o = (offset >> 1) * 3;

	if (offset & 1) {
		m_lut[o + 2] = (data & 15) << 4;
		m_palette->set_pen_color( offset >> 1, m_lut[o], m_lut[o + 1], m_lut[o + 2] );
		DBG_LOG(1,"lut",("W @ %02X <- %d %d %d\n",
			offset >> 1, m_lut[o], m_lut[o + 1], m_lut[o + 2] ));
	} else {
		m_lut[o    ] = data & 0xf0;
		m_lut[o + 1] = (data & 15) << 4;
	}
}

READ8_MEMBER( isa8_pgc_device::init_r ) {
	DBG_LOG(1,"INIT",("unmapping ROM\n"));
	space.unmap_read(0xf8000, 0xfffff);

	DBG_LOG(1,"INIT",("mapping emulator RAM\n"));
	space.install_readwrite_bank(0xf8000, 0xfffff, "eram");
	membank("eram")->set_base(m_eram);

	DBG_LOG(1,"INIT",("mapping LUT\n"));
	space.install_write_handler(0xf8400, 0xf85ff,
		write8_delegate(FUNC(isa8_pgc_device::lut_w), this));

	return 0; // XXX ignored
}

TIMER_DEVICE_CALLBACK_MEMBER(isa8_pgc_device::scanline_callback)
{
	UINT16 x, y = m_screen->vpos();
	UINT16 *p;
	UINT8 *v;

	// XXX hpos shifts every frame -- fix
	if (y == 0) DBG_LOG(2,"scanline_cb",
		("frame %d x %.4d y %.3d\n",
		(int) m_screen->frame_number(), m_screen->hpos(), y));

	if (y < PGC_VERT_START) return;
	y -= PGC_VERT_START;
	if (y >= PGC_DISP_VERT) return;

	// XXX address translation happens in hardware
	v = &m_vram[y * 1024];
	p = &m_bitmap->pix16(y, 0);

	for (x = 0; x < PGC_DISP_HORZ; x++) {
		*p++ = *v++;
	}
}

UINT32 isa8_pgc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_bitmap, 0, 0, PGC_HORZ_START, PGC_VERT_START, cliprect);
	return 0;
}
