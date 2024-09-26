// license:BSD-3-Clause
// copyright-holders:Hau
/***************************************************************************

Quiz Do Re Mi Fa Grand Prix (Japan)     (GQ460) (c)1994 Konami
Quiz Do Re Mi Fa Grand Prix 2 (Japan)   (GE557) (c)1995 Konami


CPU  :MC68HC000FN16
OSC  :18.43200MHz/32.00000MHz
Other(GQ460):Konami 053252,054156,056832,054539
Other(GE557):Konami 056832,058141,058143

--
driver by Hau

Note:
GP1 HDD data contents:
    0x000-0x52D intro quiz musics
    0x52E-0x535 not used quiz (system music or invalid data)
***************************************************************************/

#include "emu.h"

#include "k054156_k054157_k056832.h"
#include "konami_helper.h"

#include "bus/ata/ataintf.h"
#include "cpu/m68000/m68000.h"
#include "machine/k053252.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/k054539.h"
#include "emupal.h"
#include "speaker.h"

namespace {

class qdrmfgp_state : public driver_device
{
public:
	qdrmfgp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_workram(*this, "workram"),
		m_k056832(*this, "k056832"),
		m_k054539(*this, "k054539"),
		m_k053252(*this, "k053252"),
		m_ata(*this, "ata"),
		m_inputs_port(*this, "INPUTS"),
		m_dsw_port(*this, "DSW"),
		m_palette(*this, "palette"),
		m_sndram(*this, "sndram")
	{
	}

	void qdrmfgp(machine_config &config);
	void qdrmfgp2(machine_config &config);

	ioport_value battery_sensor_r();

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_nvram;
	required_shared_ptr<uint16_t> m_workram;
	required_device<k056832_device> m_k056832;
	required_device<k054539_device> m_k054539;
	required_device<k053252_device> m_k053252;
	required_device<ata_interface_device> m_ata;
	required_ioport m_inputs_port;
	required_ioport m_dsw_port;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_sndram;

	uint16_t m_control = 0;
	int32_t m_gp2_irq_control = 0;
	int32_t m_pal = 0;
	emu_timer *m_gp2_timer = nullptr;

	void gp_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gp2_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t v_rom_r(offs_t offset);
	uint16_t gp2_vram_r(offs_t offset);
	uint16_t gp2_vram_mirror_r(offs_t offset);
	void gp2_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gp2_vram_mirror_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sndram_r(offs_t offset, uint16_t mem_mask = ~0);
	void sndram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t inputs_r();

	DECLARE_MACHINE_START(qdrmfgp);
	DECLARE_VIDEO_START(qdrmfgp);
	DECLARE_MACHINE_START(qdrmfgp2);
	DECLARE_VIDEO_START(qdrmfgp2);

	uint32_t screen_update_qdrmfgp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(qdrmfgp2_interrupt);
	TIMER_CALLBACK_MEMBER(gp2_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(qdrmfgp_interrupt);
	void ide_interrupt(int state);
	void gp2_ide_interrupt(int state);
	void k054539_irq1_gen(int state);
	K056832_CB_MEMBER(qdrmfgp_tile_callback);
	K056832_CB_MEMBER(qdrmfgp2_tile_callback);

	void qdrmfgp2_map(address_map &map) ATTR_COLD;
	void qdrmfgp_k054539_map(address_map &map) ATTR_COLD;
	void qdrmfgp_map(address_map &map) ATTR_COLD;
};


K056832_CB_MEMBER(qdrmfgp_state::qdrmfgp_tile_callback)
{
	*color = ((*color>>2) & 0x0f) | m_pal;
}

K056832_CB_MEMBER(qdrmfgp_state::qdrmfgp2_tile_callback)
{
	*color = (*color>>1) & 0x7f;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(qdrmfgp_state,qdrmfgp)
{
	m_k056832->set_layer_association(0);

	m_k056832->set_layer_offs(0, 2, 0);
	m_k056832->set_layer_offs(1, 4, 0);
	m_k056832->set_layer_offs(2, 6, 0);
	m_k056832->set_layer_offs(3, 8, 0);
}

VIDEO_START_MEMBER(qdrmfgp_state,qdrmfgp2)
{
	m_k056832->set_layer_association(0);

	m_k056832->set_layer_offs(0, 3, 1);
	m_k056832->set_layer_offs(1, 5, 1);
	m_k056832->set_layer_offs(2, 7, 1);
	m_k056832->set_layer_offs(3, 9, 1);
}

/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t qdrmfgp_state::screen_update_qdrmfgp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, 0, 4);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, 0, 8);
	return 0;
}


/*************************************
 *
 *  68k CPU memory handlers
 *
 *************************************/

uint16_t qdrmfgp_state::inputs_r()
{
	return m_control & 0x0080 ? m_inputs_port->read() : m_dsw_port->read();
}

ioport_value qdrmfgp_state::battery_sensor_r()
{
	/* bit 0-1  battery power sensor: 3=good, 2=low, other=bad */
	return 0x0003;
}


void qdrmfgp_state::gp_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* bit 0        enable irq 1 (sound) */
	/* bit 1        enable irq 2 (not used) */
	/* bit 2        enable irq 3 (vblank) */
	/* bit 3        enable irq 4 (hdd) */
	/* bit 4-6      palette (tilemap) */
	/* bit 7        inputports bankswitch */
	/* bit 8        enable volume control */
	/* bit 9        volume: 1=up, 0=down (low5,mid90,high180) */
	/* bit 10       enable headphone volume control */
	/* bit 11       headphone volume: 1=up, 0=down */
	/* bit 15       gfxrom bankswitch */

	COMBINE_DATA(&m_control);
	m_pal = m_control & 0x70;

	if (!(m_control & 1))
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);

	if (!(m_control & 2))
		m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);

	if (!(m_control & 4))
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);

	if (!(m_control & 8))
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);

	if (m_control & 0x0100)
	{
		int vol = m_nvram[0x10] & 0xff;
		if (vol)
		{
			int i;
			double gain = vol / 90.0;

			for (i=0; i<8; i++)
				m_k054539->set_gain(i, gain);
		}
	}
}

void qdrmfgp_state::gp2_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* bit 2        enable irq 3 (sound) */
	/* bit 3        enable irq 4 (vblank) */
	/* bit 4        enable irq 5 (hdd) */
	/* bit 7        inputports bankswitch */
	/* bit 8        enable volume control */
	/* bit 9        volume: 1=up, 0=down (low0,mid90,high255) */
	/* bit 10       enable headphone volume control */
	/* bit 11       headphone volume: 1=up, 0=down */
	/* bit 15       gfxrom bankswitch */

	COMBINE_DATA(&m_control);
	m_pal = 0;

	if (!(m_control & 4))
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);

	if (!(m_control & 8))
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);

	if (!(m_control & 0x10))
		m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);

	if (m_control & 0x0100)
	{
		int vol = m_nvram[0x8] & 0xff;
		if (vol)
		{
			int i;
			double gain = vol / 90.0;

			for (i=0; i<8; i++)
				m_k054539->set_gain(i, gain);
		}
	}
}


uint16_t qdrmfgp_state::v_rom_r(offs_t offset)
{
	uint8_t *mem8 = memregion("k056832")->base();
	int bank = m_k056832->word_r(0x34/2);

	offset += bank * 0x800 * 4;

	if (m_control & 0x8000)
		offset += 0x800 * 2;

	return (mem8[offset + 1] << 8) + mem8[offset];
}


uint16_t qdrmfgp_state::gp2_vram_r(offs_t offset)
{
	if (offset < 0x1000 / 2)
		return m_k056832->ram_word_r(offset * 2 + 1);
	else
		return m_k056832->ram_word_r((offset - 0x1000 / 2) * 2);
}

uint16_t qdrmfgp_state::gp2_vram_mirror_r(offs_t offset)
{
	if (offset < 0x1000 / 2)
		return m_k056832->ram_word_r(offset * 2);
	else
		return m_k056832->ram_word_r((offset - 0x1000 / 2) * 2 + 1);
}

void qdrmfgp_state::gp2_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset < 0x1000 / 2)
		m_k056832->ram_word_w(offset * 2 + 1, data, mem_mask);
	else
		m_k056832->ram_word_w((offset - 0x1000 / 2) * 2, data, mem_mask);
}

void qdrmfgp_state::gp2_vram_mirror_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset < 0x1000 / 2)
		m_k056832->ram_word_w(offset * 2, data, mem_mask);
	else
		m_k056832->ram_word_w((offset - 0x1000 / 2) * 2 + 1, data, mem_mask);
}


/*************/

uint16_t qdrmfgp_state::sndram_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		return m_sndram[offset];

	return 0;
}

void qdrmfgp_state::sndram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_sndram[offset] = data & 0xff;
	}
}


/*************************************
 *
 *  Interrupt handlers
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(qdrmfgp_state::qdrmfgp_interrupt)
{
	int scanline = param;

	/* trigger V-blank interrupt */
	if(scanline == 240)
		if (m_control & 0x0004)
			m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
}

void qdrmfgp_state::ide_interrupt(int state)
{
	if (m_control & 0x0008)
		if (state != CLEAR_LINE)
			m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}

/*************/

TIMER_CALLBACK_MEMBER(qdrmfgp_state::gp2_timer_callback)
{
	if (m_control & 0x0004)
		m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(qdrmfgp_state::qdrmfgp2_interrupt)
{
	/* trigger V-blank interrupt */
	if (m_control & 0x0008)
		device.execute().set_input_line(M68K_IRQ_4, ASSERT_LINE);
}

void qdrmfgp_state::gp2_ide_interrupt(int state)
{
	if (m_control & 0x0010)
		if (state != CLEAR_LINE)
			m_maincpu->set_input_line(M68K_IRQ_5, ASSERT_LINE);
}


/*************************************
 *
 *  Memory definitions
 *
 *************************************/

void qdrmfgp_state::qdrmfgp_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram().share("workram");                                                                     // work ram
	map(0x180000, 0x183fff).ram().share("nvram");                                                                       // backup ram
	map(0x280000, 0x280fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30003f).w(m_k056832, FUNC(k056832_device::word_w));                                                 // video reg
	map(0x320000, 0x32001f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write)).umask16(0x00ff);     // ccu
	map(0x330000, 0x330001).portr("SENSOR");                                                                            // battery power & service sw
	map(0x340000, 0x340001).r(FUNC(qdrmfgp_state::inputs_r));                                                           // inputport
	map(0x350000, 0x350001).nopw();                                                                                     // unknown
	map(0x360000, 0x360001).nopw();                                                                                     // unknown
	map(0x370000, 0x370001).w(FUNC(qdrmfgp_state::gp_control_w));                                                       // control reg
	map(0x380000, 0x380001).nopw();                                                                                     // Watchdog
	map(0x800000, 0x80045f).rw(m_k054539, FUNC(k054539_device::read), FUNC(k054539_device::write)).umask16(0x00ff);     // sound regs
	map(0x880000, 0x881fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));          // vram
	map(0x882000, 0x883fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));          // vram (mirror)
	map(0x900000, 0x901fff).r(FUNC(qdrmfgp_state::v_rom_r));                                                            // gfxrom through
	map(0xa00000, 0xa0000f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));            // IDE control regs
	map(0xa40000, 0xa4000f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));            // IDE status control reg
	map(0xc00000, 0xcbffff).rw(FUNC(qdrmfgp_state::sndram_r), FUNC(qdrmfgp_state::sndram_w));                           // sound ram
}


void qdrmfgp_state::qdrmfgp2_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x110fff).ram().share("workram");                                                                     // work ram
	map(0x180000, 0x183fff).ram().share("nvram");                                                                       // backup ram
	map(0x280000, 0x280fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30003f).w(m_k056832, FUNC(k056832_device::word_w));                                                 // video reg
	map(0x320000, 0x32001f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write)).umask16(0xff00);     // ccu
	map(0x330000, 0x330001).portr("SENSOR");                                                                            // battery power & service
	map(0x340000, 0x340001).r(FUNC(qdrmfgp_state::inputs_r));                                                           // inputport
	map(0x350000, 0x350001).nopw();                                                                                     // unknown
	map(0x360000, 0x360001).nopw();                                                                                     // unknown
	map(0x370000, 0x370001).w(FUNC(qdrmfgp_state::gp2_control_w));                                                      // control reg
	map(0x380000, 0x380001).nopw();                                                                                     // Watchdog
	map(0x800000, 0x80045f).rw(m_k054539, FUNC(k054539_device::read), FUNC(k054539_device::write)).umask16(0x00ff);     // sound regs
	map(0x880000, 0x881fff).rw(FUNC(qdrmfgp_state::gp2_vram_r), FUNC(qdrmfgp_state::gp2_vram_w));                       // vram
	map(0x89f000, 0x8a0fff).rw(FUNC(qdrmfgp_state::gp2_vram_mirror_r), FUNC(qdrmfgp_state::gp2_vram_mirror_w));         // vram (mirror)
	map(0x900000, 0x901fff).r(FUNC(qdrmfgp_state::v_rom_r));                                                            // gfxrom through
	map(0xa00000, 0xa0000f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));            // IDE control regs
	map(0xa40000, 0xa4000f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));            // IDE status control reg
	map(0xc00000, 0xcbffff).rw(FUNC(qdrmfgp_state::sndram_r), FUNC(qdrmfgp_state::sndram_w));                           // sound ram
}


void qdrmfgp_state::qdrmfgp_k054539_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("k054539", 0);
	map(0x100000, 0x45ffff).ram().share("sndram");
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( qdrmfgp )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)   /* 1P STOP */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)   /* 2P STOP */

	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* SERVICE */
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0004, 0x0004, "Extended Service Menu" )     /* and skipped initial checks. */
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Skip HDD Check" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Initialize Backup RAM" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x3000, "All The Time" )
	PORT_DIPSETTING(      0x1000, "Once Every 2Cycles" )
	PORT_DIPSETTING(      0x2000, "Once Every 4Cycles" )
	PORT_DIPSETTING(      0x0000, "Completely Off" )
	PORT_DIPNAME( 0xc000, 0x4000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )

	PORT_START("SENSOR")
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(qdrmfgp_state, battery_sensor_r)   /* battery power sensor */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( qdrmfgp2 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)   /* 1P STOP */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)   /* 2P STOP */

	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* SERVICE */
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0004, 0x0004, "Extended Service Menu & None Sounds Mode" )      /* and skipped initial checks. */
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Initialize Backup RAM" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x3000, "All The Time" )
	PORT_DIPSETTING(      0x1000, "Once Every 2Cycles" )
	PORT_DIPSETTING(      0x2000, "Once Every 4Cycles" )
	PORT_DIPSETTING(      0x0000, "Completely Off" )
	PORT_DIPNAME( 0xc000, 0x4000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )

	PORT_START("SENSOR")
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(qdrmfgp_state, battery_sensor_r)   /* battery power sensor */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

int m_sound_intck;

void qdrmfgp_state::k054539_irq1_gen(int state)
{
	if (m_control & 1)
	{
		// Trigger an interrupt on the rising edge
		if (!m_sound_intck && state)
			m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
	}

	m_sound_intck = state;
}

/*************************************
 *
 *  Machine-specific init
 *
 *************************************/

MACHINE_START_MEMBER(qdrmfgp_state,qdrmfgp)
{
	save_item(NAME(m_control));
	save_item(NAME(m_pal));
	save_item(NAME(m_gp2_irq_control));
}

MACHINE_START_MEMBER(qdrmfgp_state,qdrmfgp2)
{
	/* sound irq (CCU? 240Hz) */
	m_gp2_timer = timer_alloc(FUNC(qdrmfgp_state::gp2_timer_callback), this);
	m_gp2_timer->adjust(attotime::from_hz(XTAL(18'432'000)/76800), 0, attotime::from_hz(XTAL(18'432'000)/76800));

	MACHINE_START_CALL_MEMBER( qdrmfgp );
}

void qdrmfgp_state::machine_reset()
{
	/* reset the IDE controller */
	m_gp2_irq_control = 0;
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void qdrmfgp_state::qdrmfgp(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &qdrmfgp_state::qdrmfgp_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(qdrmfgp_state::qdrmfgp_interrupt), "screen", 0, 1);

	MCFG_MACHINE_START_OVERRIDE(qdrmfgp_state,qdrmfgp)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);
	m_ata->irq_handler().set(FUNC(qdrmfgp_state::ide_interrupt));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(40, 40+384-1, 16, 16+224-1);
	screen.set_screen_update(FUNC(qdrmfgp_state::screen_update_qdrmfgp));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	MCFG_VIDEO_START_OVERRIDE(qdrmfgp_state,qdrmfgp)

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(qdrmfgp_state::qdrmfgp_tile_callback));
	m_k056832->set_config(K056832_BPP_4dj, 1, 0);
	m_k056832->set_palette(m_palette);

	K053252(config, m_k053252, XTAL(32'000'000)/4);
	m_k053252->set_offsets(40, 16);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	k054539_device &k054539(K054539(config, m_k054539, XTAL(18'432'000)));
	k054539.set_addrmap(0, &qdrmfgp_state::qdrmfgp_k054539_map);
	k054539.timer_handler().set(FUNC(qdrmfgp_state::k054539_irq1_gen));
	k054539.add_route(0, "lspeaker", 1.0);
	k054539.add_route(1, "rspeaker", 1.0);
}

void qdrmfgp_state::qdrmfgp2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &qdrmfgp_state::qdrmfgp2_map);
	m_maincpu->set_vblank_int("screen", FUNC(qdrmfgp_state::qdrmfgp2_interrupt));

	MCFG_MACHINE_START_OVERRIDE(qdrmfgp_state,qdrmfgp2)
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);
	m_ata->irq_handler().set(FUNC(qdrmfgp_state::gp2_ide_interrupt));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(40, 40+384-1, 16, 16+224-1);
	screen.set_screen_update(FUNC(qdrmfgp_state::screen_update_qdrmfgp));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	MCFG_VIDEO_START_OVERRIDE(qdrmfgp_state,qdrmfgp2)

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(qdrmfgp_state::qdrmfgp2_tile_callback));
	m_k056832->set_config(K056832_BPP_4dj, 1, 0);
	m_k056832->set_palette(m_palette);

	K053252(config, m_k053252, XTAL(32'000'000)/4);
	m_k053252->set_offsets(40, 16);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	k054539_device &k054539(K054539(config, "k054539", XTAL(18'432'000)));
	k054539.set_addrmap(0, &qdrmfgp_state::qdrmfgp_k054539_map);
	k054539.add_route(0, "lspeaker", 1.0);
	k054539.add_route(1, "rspeaker", 1.0);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( qdrmfgp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "gq_460_b04.20e", 0x000000, 0x80000, CRC(293d8174) SHA1(cf507d0b29dab161190f0160c05c640f16306bae) )
	ROM_LOAD16_WORD_SWAP( "gq_460_a05.22e", 0x080000, 0x80000, CRC(4128cb3c) SHA1(4a16d85a66934a20afd074546de362c40a1ea785) )

	ROM_REGION( 0x100000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD( "gq_460_a01.15e", 0x000000, 0x80000, CRC(6536b700) SHA1(47ffe0cfbf80810179560150b23d825fe1a5c5ca) )
	ROM_LOAD( "gq_460_a02.17e", 0x080000, 0x80000, CRC(ac01d675) SHA1(bf66433ace95f4ef14699d03add7cbc2e5d90eea) )

	ROM_REGION( 0x100000, "k054539", 0)      /* SE SAMPLES + space for additional RAM */
	ROM_LOAD( "gq_460_a07.14h", 0x000000, 0x80000, CRC(67d8ea6b) SHA1(11af1b5a33de2a6e24823964d210bef193ecefe4) )
	ROM_LOAD( "gq_460_a06.12h", 0x080000, 0x80000, CRC(97ed5a77) SHA1(68600fd8d914451284cf181fb4bd5872860fb9ad) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "gq460a08", 0, SHA1(2f142f986fa3c79d5c4102e800980d1706c35f75) )
ROM_END

ROM_START( qdrmfgp2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ge_557_c05.20e", 0x000000, 0x80000, CRC(336df99f) SHA1(46fb36d40371761be0cfa17b34f28cc893a44a22) )
	ROM_LOAD16_WORD_SWAP( "ge_557_a06.22e", 0x080000, 0x80000, CRC(ad77e10f) SHA1(4a762a59fe3096d48e3cbf0da3bb0d75c5087e78) )

	ROM_REGION( 0x100000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD( "ge_557_a01.13e", 0x000000, 0x80000, CRC(c301d406) SHA1(5fad8cc611edd83380972abf37ec80561b9317a6) )
	ROM_LOAD( "ge_557_a02.15e", 0x080000, 0x80000, CRC(3bfe1e56) SHA1(9e4df512a804a96fcb545d4e0eb58b5421d65ea4) )

	ROM_REGION( 0x100000, "k054539", 0)      /* SE SAMPLES + space for additional RAM */
	ROM_LOAD( "ge_557_a07.19h", 0x000000, 0x80000, CRC(7491e0c8) SHA1(6459ab5e7af052ef7a1c4ce01cd844c0f4319f2e) )
	ROM_LOAD( "ge_557_a08.19k", 0x080000, 0x80000, CRC(3da2b20c) SHA1(fdc2cdc27f3299f541944a78ce36ed33a7926056) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "ge557a09", 0, SHA1(1ef8093b542fe0bf8240a5fd64e5af3839b6a04c) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

/*     year  rom       clone     machine   inputs    state          init */
GAME(  1994, qdrmfgp,  0,        qdrmfgp,  qdrmfgp,  qdrmfgp_state, empty_init, ROT0, "Konami", "Quiz Do Re Mi Fa Grand Prix (Japan)", 0 )
GAME(  1995, qdrmfgp2, 0,        qdrmfgp2, qdrmfgp2, qdrmfgp_state, empty_init, ROT0, "Konami", "Quiz Do Re Mi Fa Grand Prix 2 - Shin-Kyoku Nyuukadayo (Japan)", 0 )
