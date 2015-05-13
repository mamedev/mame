// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
    Konami Ultra Sports hardware

    Driver by Ville Linde
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "sound/k054539.h"
#include "machine/eepromser.h"
#include "sound/k056800.h"

class ultrsprt_state : public driver_device
{
public:
	ultrsprt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k056800(*this, "k056800"),
		m_workram(*this, "workram"),
		m_palette(*this, "palette") { }

	static const UINT32 VRAM_PAGES      = 2;
	static const UINT32 VRAM_PAGE_BYTES = 512 * 1024;

	required_device<ppc_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k056800_device> m_k056800;
	required_shared_ptr<UINT32> m_workram;
	required_device<palette_device> m_palette;

	DECLARE_READ32_MEMBER(vram_r);
	DECLARE_WRITE32_MEMBER(vram_w);
	DECLARE_READ32_MEMBER(eeprom_r);
	DECLARE_WRITE32_MEMBER(eeprom_w);
	DECLARE_WRITE32_MEMBER(int_ack_w);
	DECLARE_CUSTOM_INPUT_MEMBER(flip_status_r);

	UINT32 screen_update_ultrsprt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start();
	virtual void machine_reset();

private:
	UINT8 *m_vram;
	UINT32 m_cpu_vram_page;
};


/*****************************************************************************/

UINT32 ultrsprt_state::screen_update_ultrsprt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *vram = m_vram + (m_cpu_vram_page ^ 1) * VRAM_PAGE_BYTES;

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		int fb_index = y * 1024;
		UINT16 *dest = &bitmap.pix16(y, cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
		{
			UINT8 p1 = vram[BYTE4_XOR_BE(fb_index + x + 512)];

			if (p1 == 0)
				*dest++ = vram[BYTE4_XOR_BE(fb_index + x)];
			else
				*dest++ = 0x100 + p1;
		}
	}

	return 0;
}


/*****************************************************************************/

WRITE32_MEMBER(ultrsprt_state::int_ack_w)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}


READ32_MEMBER(ultrsprt_state::eeprom_r)
{
	UINT32 r = 0;

	if (ACCESSING_BITS_24_31)
		r |= ioport("SERVICE")->read();

	return r;
}

WRITE32_MEMBER(ultrsprt_state::eeprom_w)
{
	if (ACCESSING_BITS_24_31)
	{
		/*
		    .... ...x - EEPROM DI
		    .... ..x. - EEPROM CLK
		    .... .x.. - EEPROM /CS
		    .... x... - VRAM page (CPU access)
		    ...x .... - Coin counter
		    ..x. .... - Watchdog /Reset
		    .x.. .... - Trackball /Reset
		    x... .... - Sound CPU /Reset
		*/
		ioport("EEPROMOUT")->write(data, 0xffffffff);

		UINT32 vram_page = (data & 0x08000000) >> 27;

		if (vram_page != m_cpu_vram_page)
		{
			membank("vram")->set_entry(vram_page);
			m_cpu_vram_page = vram_page;
		}

		coin_counter_w(machine(), 0, data & 0x10000000);
		m_audiocpu->set_input_line(INPUT_LINE_RESET, data & 0x80000000 ? CLEAR_LINE : ASSERT_LINE);
	}
}

/*****************************************************************************/

static ADDRESS_MAP_START( ultrsprt_map, AS_PROGRAM, 32, ultrsprt_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAMBANK("vram")
	AM_RANGE(0x70000000, 0x70000003) AM_READWRITE(eeprom_r, eeprom_w)
	AM_RANGE(0x70000020, 0x70000023) AM_READ_PORT("P1")
	AM_RANGE(0x70000040, 0x70000043) AM_READ_PORT("P2")
	AM_RANGE(0x70000080, 0x7000008f) AM_DEVREADWRITE8("k056800", k056800_device, host_r, host_w, 0xffffffff)
	AM_RANGE(0x700000c0, 0x700000cf) AM_WRITENOP // Written following DMA interrupt - unused int ack?
	AM_RANGE(0x700000e0, 0x700000e3) AM_WRITE(int_ack_w)
	AM_RANGE(0x7f000000, 0x7f01ffff) AM_RAM AM_SHARE("workram")
	AM_RANGE(0x7f700000, 0x7f703fff) AM_RAM_DEVWRITE("palette",  palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x7f800000, 0x7f9fffff) AM_MIRROR(0x00600000) AM_ROM AM_REGION("program", 0)
ADDRESS_MAP_END


/*****************************************************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 16, ultrsprt_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
	AM_RANGE(0x00100000, 0x00101fff) AM_RAM
	AM_RANGE(0x00200000, 0x0020000f) AM_DEVREADWRITE8("k056800", k056800_device, sound_r, sound_w, 0xffff)
	AM_RANGE(0x00400000, 0x004002ff) AM_DEVREADWRITE8("k054539", k054539_device, read, write, 0xffff)
ADDRESS_MAP_END


/*****************************************************************************/

static INPUT_PORTS_START( ultrsprt )
	PORT_START("P1")
	PORT_BIT( 0x00000fff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(80) PORT_PLAYER(1)
	PORT_BIT( 0x0fff0000, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(80) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x00000fff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(80) PORT_PLAYER(2)
	PORT_BIT( 0x0fff0000, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(80) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) // VRAM page flip status?
	PORT_SERVICE_NO_TOGGLE( 0x08000000, IP_ACTIVE_LOW )

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END


/*****************************************************************************/

void ultrsprt_state::machine_start()
{
	/* set conservative DRC options */
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	m_maincpu->ppcdrc_add_fastram(0xff000000, 0xff01ffff, FALSE, m_workram);

	m_vram = auto_alloc_array(machine(), UINT8, VRAM_PAGE_BYTES * VRAM_PAGES);

	membank("vram")->configure_entries(0, VRAM_PAGES, m_vram, VRAM_PAGE_BYTES);

	save_pointer(NAME(m_vram), VRAM_PAGE_BYTES * VRAM_PAGES);
	save_item(NAME(m_cpu_vram_page));
}

void ultrsprt_state::machine_reset()
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_cpu_vram_page = 0;
	membank("vram")->set_entry(m_cpu_vram_page);
}


/*****************************************************************************/

static MACHINE_CONFIG_START( ultrsprt, ultrsprt_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GA, 25000000)
	MCFG_CPU_PROGRAM_MAP(ultrsprt_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ultrsprt_state, irq1_line_assert)

	MCFG_CPU_ADD("audiocpu", M68000, 8000000) // Unconfirmed
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60) // TODO: Determine correct timings
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 399)
	MCFG_SCREEN_UPDATE_DRIVER(ultrsprt_state, screen_update_ultrsprt)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 8192)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	/* sound hardware */
	MCFG_K056800_ADD("k056800", XTAL_18_432MHz)
	MCFG_K056800_INT_HANDLER(INPUTLINE("audiocpu", M68K_IRQ_6))

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("k054539", K054539, XTAL_18_432MHz)
	MCFG_K054539_TIMER_HANDLER(INPUTLINE("audiocpu", M68K_IRQ_5))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*****************************************************************************/

ROM_START( fiveside )
	ROM_REGION(0x200000, "program", 0) /* PowerPC program ROMs */
	ROM_LOAD32_BYTE("479uaa01.bin", 0x000003, 0x80000, CRC(1bc4893d) SHA1(2c9df38ecb7efa7b686221ee98fa3aad9a63e152))
	ROM_LOAD32_BYTE("479uaa02.bin", 0x000002, 0x80000, CRC(ae74a6d0) SHA1(6113c2eea1628b22737c7b87af0e673d94984e88))
	ROM_LOAD32_BYTE("479uaa03.bin", 0x000001, 0x80000, CRC(5c0b176f) SHA1(9560259bc081d4cfd72eb485c3fdcecf484ba7a8))
	ROM_LOAD32_BYTE("479uaa04.bin", 0x000000, 0x80000, CRC(01a3e4cb) SHA1(819df79909d57fa12481698ffdb32b00586131d8))

	ROM_REGION(0x20000, "audiocpu", 0) /* M68K program */
	ROM_LOAD("479_a05.bin", 0x000000, 0x20000, CRC(251ae299) SHA1(5ffd74357e3c6ddb3a208c39a3b32b53fea90282))

	ROM_REGION(0x100000, "k054539", 0) /* Sound ROMs */
	ROM_LOAD("479_a06.bin", 0x000000, 0x80000, CRC(8d6ac8a2) SHA1(7c4b8bd47cddc766cbdb6a486acc9221be55b579))
	ROM_LOAD("479_a07.bin", 0x080000, 0x80000, CRC(75835df8) SHA1(105b95c16f2ce6902c2e4c9c2fd9f2f7a848c546))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fiveside.nv", 0x0000, 0x0080, CRC(aad11072) SHA1(8f777ee47801faa7ce8420c3052034720225aae7) )
ROM_END

// Undumped: Ultra Hockey
GAME(1995, fiveside, 0, ultrsprt, ultrsprt, driver_device, 0, ROT90, "Konami", "Five a Side Soccer (ver UAA)", 0)
