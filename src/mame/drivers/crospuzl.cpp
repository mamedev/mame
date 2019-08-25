// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************

    Cross Puzzle

    driver by Angelo Salese, based off original crystal.cpp by ElSemi

    TODO:
    - Dies at POST with a SPU error and no Flash memory available;
    - RTC isn't DS1302


=============================================================================

 This PCB uses ADC 'Amazon-LF' SoC, EISC CPU core - However PCBs have been see
 with a standard VRenderZERO+ MagicEyes EISC chip

****************************************************************************/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "machine/ds1302.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/vrender0.h"
#include "sound/vrender0.h"
#include "video/vrender0.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

#define IDLE_LOOP_SPEEDUP

class crospuzl_state : public driver_device
{
public:
	crospuzl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_textureram(*this, "textureram"),
		m_frameram(*this, "frameram"),
		m_flash(*this, "flash"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
		m_vr0vid(*this, "vr0vid"),
		m_vr0snd(*this, "vr0snd"),
		m_ds1302(*this, "rtc"),
		m_screen(*this, "screen")
	{ }


	void crospuzl(machine_config &config);

private:

	/* memory pointers */
	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_textureram;
	required_shared_ptr<uint32_t> m_frameram;
	required_region_ptr<uint8_t> m_flash;

	/* devices */
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<vr0video_device> m_vr0vid;
	required_device<vr0sound_device> m_vr0snd;
	required_device<ds1302_device> m_ds1302;
	required_device<screen_device> m_screen;

#ifdef IDLE_LOOP_SPEEDUP
	uint8_t     m_FlipCntRead;
	DECLARE_WRITE_LINE_MEMBER(idle_skip_resume_w);
	DECLARE_WRITE_LINE_MEMBER(idle_skip_speedup_w);
#endif

	uint32_t    m_FlashCmd;
	uint32_t    m_crospuzl_addr;

//  DECLARE_WRITE32_MEMBER(Banksw_w);
	DECLARE_READ8_MEMBER(FlashCmd_r);
	DECLARE_WRITE32_MEMBER(FlashCmd_w);
	DECLARE_WRITE32_MEMBER(flash_addr_w);

	IRQ_CALLBACK_MEMBER(icallback);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void crospuzl_mem(address_map &map);

	// PIO
	DECLARE_READ32_MEMBER(PIOldat_r);
	uint32_t m_PIO;
	DECLARE_WRITE32_MEMBER(PIOldat_w);
	DECLARE_READ32_MEMBER(PIOedat_r);
};


uint32_t crospuzl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_vr0soc->crt_is_blanked()) // Blank Screen
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	m_vr0vid->screen_update(screen, bitmap, cliprect);
	return 0;
}

WRITE_LINE_MEMBER(crospuzl_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		if (m_vr0soc->crt_active_vblank_irq() == true)
			m_vr0soc->IntReq(24);      //VRender0 VBlank

		m_vr0vid->execute_drawing();
	}
}

IRQ_CALLBACK_MEMBER(crospuzl_state::icallback)
{
	return m_vr0soc->irq_callback();
}

WRITE32_MEMBER(crospuzl_state::FlashCmd_w)
{
	m_FlashCmd = data;
}

READ32_MEMBER(crospuzl_state::PIOedat_r)
{
	return machine().rand() & 0x04000000; // serial ready line
}

WRITE32_MEMBER(crospuzl_state::flash_addr_w)
{
	if (m_FlashCmd == 0x90)
		m_crospuzl_addr = 0;
	if (data)
		m_crospuzl_addr = (data << 16);
}

READ8_MEMBER(crospuzl_state::FlashCmd_r)
{
	if ((m_FlashCmd & 0xff) == 0xff)
	{
		return 0xff;
	}
	if ((m_FlashCmd & 0xff) == 0x90)
	{
		const uint8_t id[5] = { 0xee, 0x81, 0x00, 0x00, 0x00 };
		uint8_t res = id[m_crospuzl_addr];
		m_crospuzl_addr ++;
		m_crospuzl_addr %= 4;
		return res;
	}
	if ((m_FlashCmd & 0xff) == 0x30)
	{
		uint8_t res = m_flash[m_crospuzl_addr];
		m_crospuzl_addr++;
		return res;
	}
	return 0;
}

READ32_MEMBER(crospuzl_state::PIOldat_r)
{
	return m_PIO;
}

// PIO Latched output DATa Register
// TODO: change me
WRITE32_MEMBER(crospuzl_state::PIOldat_w)
{
	uint32_t RST = data & 0x01000000;
	uint32_t CLK = data & 0x02000000;
	uint32_t DAT = data & 0x10000000;

	m_ds1302->ce_w(RST ? 1 : 0);
	m_ds1302->io_w(DAT ? 1 : 0);
	m_ds1302->sclk_w(CLK ? 1 : 0);

	COMBINE_DATA(&m_PIO);
}

void crospuzl_state::crospuzl_mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().nopw();

	map(0x01500000, 0x01500000).r(FUNC(crospuzl_state::FlashCmd_r));
	map(0x01500100, 0x01500103).w(FUNC(crospuzl_state::FlashCmd_w));
	map(0x01500200, 0x01500203).w(FUNC(crospuzl_state::flash_addr_w));
	map(0x01510000, 0x01510003).portr("IN0");
	map(0x01511000, 0x01511003).portr("IN1");
	map(0x01512000, 0x01512003).portr("IN2");
	map(0x01513000, 0x01513003).portr("IN3");

	map(0x01600000, 0x01607fff).ram().share("nvram");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(crospuzl_state::PIOldat_r), FUNC(crospuzl_state::PIOldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(crospuzl_state::PIOedat_r));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x0300ffff).m(m_vr0vid, FUNC(vr0video_device::regs_map));
	map(0x03800000, 0x03ffffff).ram().share("textureram");
	map(0x04000000, 0x047fffff).ram().share("frameram");
	map(0x04800000, 0x04800fff).rw(m_vr0snd, FUNC(vr0sound_device::vr0_snd_read), FUNC(vr0sound_device::vr0_snd_write));

//  map(0x05000000, 0x05ffffff).bankr("mainbank");
//  map(0x05000000, 0x05000003).rw(FUNC(crospuzl_state::FlashCmd_r), FUNC(crospuzl_state::FlashCmd_w));
}

#ifdef IDLE_LOOP_SPEEDUP

WRITE_LINE_MEMBER(crospuzl_state::idle_skip_resume_w)
{
	m_FlipCntRead = 0;
	m_maincpu->resume(SUSPEND_REASON_SPIN);
}

WRITE_LINE_MEMBER(crospuzl_state::idle_skip_speedup_w)
{
	m_FlipCntRead++;
	if (m_FlipCntRead >= 16 && m_vr0soc->irq_pending() == false && state == ASSERT_LINE)
		m_maincpu->suspend(SUSPEND_REASON_SPIN, 1);
}
#endif

void crospuzl_state::machine_start()
{
	m_vr0vid->set_areas(reinterpret_cast<uint8_t*>(m_textureram.target()), reinterpret_cast<uint16_t*>(m_frameram.target()));
	m_vr0snd->set_areas(m_textureram, m_frameram);


#ifdef IDLE_LOOP_SPEEDUP
	save_item(NAME(m_FlipCntRead));
#endif

//  save_item(NAME(m_Bank));
	save_item(NAME(m_FlashCmd));
	save_item(NAME(m_PIO));
}

void crospuzl_state::machine_reset()
{
	m_FlashCmd = 0xff;
	m_crospuzl_addr = 0;
#ifdef IDLE_LOOP_SPEEDUP
	m_FlipCntRead = 0;
#endif
}

static INPUT_PORTS_START(crospuzl)
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSW2" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("PCB-SW1")
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void crospuzl_state::crospuzl(machine_config &config)
{
	SE3208(config, m_maincpu, 14318180 * 3); // TODO : different between each PCBs
	m_maincpu->set_addrmap(AS_PROGRAM, &crospuzl_state::crospuzl_mem);
	m_maincpu->set_irq_acknowledge_callback(FUNC(crospuzl_state::icallback));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// evolution soccer defaults
	m_screen->set_raw((XTAL(14'318'180)*2)/4, 455, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(crospuzl_state::screen_update));
	m_screen->screen_vblank().set(FUNC(crospuzl_state::screen_vblank));
	m_screen->set_palette("palette");

	VRENDER0_SOC(config, m_vr0soc, 0);
	m_vr0soc->set_host_cpu_tag(m_maincpu);
	m_vr0soc->set_host_screen_tag(m_screen);

	VIDEO_VRENDER0(config, m_vr0vid, 14318180, m_maincpu);
	#ifdef IDLE_LOOP_SPEEDUP
	m_vr0soc->idleskip_cb().set(FUNC(crospuzl_state::idle_skip_resume_w));
	m_vr0vid->idleskip_cb().set(FUNC(crospuzl_state::idle_skip_speedup_w));
	#endif

	PALETTE(config, "palette", palette_device::RGB_565);

	DS1302(config, m_ds1302, 32.768_kHz_XTAL);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SOUND_VRENDER0(config, m_vr0snd, 0);
	m_vr0snd->add_route(0, "lspeaker", 1.0);
	m_vr0snd->add_route(1, "rspeaker", 1.0);
}

ROM_START( crospuzl )
	ROM_REGION( 0x80010, "maincpu", 0 )
	ROM_LOAD("en29lv040a.u5",  0x000000, 0x80010, CRC(d50e8500) SHA1(d681cd18cd0e48854c24291d417d2d6d28fe35c1) )

	ROM_REGION( 0x8400010, "flash", ROMREGION_ERASEFF ) // NAND Flash
	// mostly empty, but still looks good
	ROM_LOAD("k9f1g08u0a.riser",  0x000000, 0x0020000, CRC(7f3c88c3) SHA1(db3169a7b4caab754e9d911998a2ece13c65ce5b) )
	ROM_CONTINUE(                 0x000000, 0x8400010-0x0020000 )
ROM_END

GAME( 200?, crospuzl, 0,        crospuzl, crospuzl, crospuzl_state, empty_init,    ROT0, "<unknown>",           "Cross Puzzle", MACHINE_NOT_WORKING )
