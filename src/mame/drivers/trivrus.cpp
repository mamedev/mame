// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************

    Trivia R Us (c) 2009 AGT

    driver by Angelo Salese, based off original crystal.cpp by ElSemi

    TODO:
    - touch panel;
    - RTC;

=============================================================================

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

class trivrus_state : public driver_device
{
public:
	trivrus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_textureram(*this, "textureram"),
		m_frameram(*this, "frameram"),
		m_flash(*this, "flash"),
		m_maincpu(*this, "maincpu"),
		m_mainbank(*this, "mainbank"),
		m_vr0soc(*this, "vr0soc"),
		m_vr0vid(*this, "vr0vid"),
		m_vr0snd(*this, "vr0snd"),
		m_ds1302(*this, "rtc"),
		m_screen(*this, "screen")
	{ }


	void trivrus(machine_config &config);

private:

	/* memory pointers */
	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_textureram;
	required_shared_ptr<uint32_t> m_frameram;
	required_region_ptr<uint32_t> m_flash;

	/* devices */
	required_device<se3208_device> m_maincpu;
	optional_memory_bank m_mainbank;
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
	uint32_t    m_Bank;
	uint32_t    m_maxbank;

	DECLARE_READ32_MEMBER(FlashCmd_r);
	DECLARE_WRITE32_MEMBER(FlashCmd_w);
	DECLARE_WRITE32_MEMBER(Banksw_w);

	IRQ_CALLBACK_MEMBER(icallback);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void trivrus_mem(address_map &map);

	// PIO
	DECLARE_READ32_MEMBER(PIOldat_r);
	uint32_t m_PIO;
	DECLARE_WRITE32_MEMBER(PIOldat_w);
	DECLARE_READ32_MEMBER(PIOedat_r);

	DECLARE_READ8_MEMBER(trivrus_input_r);
	DECLARE_WRITE8_MEMBER(trivrus_input_w);
	uint8_t m_trivrus_input;
};


uint32_t trivrus_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_vr0soc->crt_is_blanked()) // Blank Screen
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	m_vr0vid->screen_update(screen, bitmap, cliprect);
	return 0;
}

WRITE_LINE_MEMBER(trivrus_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		if (m_vr0soc->crt_active_vblank_irq() == true)
			m_vr0soc->IntReq(24);      //VRender0 VBlank

		m_vr0vid->execute_drawing();
	}
}

IRQ_CALLBACK_MEMBER(trivrus_state::icallback)
{
	return m_vr0soc->irq_callback();
}

WRITE32_MEMBER(trivrus_state::FlashCmd_w)
{
	m_FlashCmd = data;
}

READ32_MEMBER(trivrus_state::PIOedat_r)
{
	return m_ds1302->io_r() << 28;
}

READ32_MEMBER(trivrus_state::PIOldat_r)
{
	return m_PIO;
}

// PIO Latched output DATa Register
// TODO: change me
WRITE32_MEMBER(trivrus_state::PIOldat_w)
{
	uint32_t RST = data & 0x01000000;
	uint32_t CLK = data & 0x02000000;
	uint32_t DAT = data & 0x10000000;

	m_ds1302->ce_w(RST ? 1 : 0);
	m_ds1302->io_w(DAT ? 1 : 0);
	m_ds1302->sclk_w(CLK ? 1 : 0);

	COMBINE_DATA(&m_PIO);
}

READ8_MEMBER(trivrus_state::trivrus_input_r)
{
	switch (m_trivrus_input)
	{
		case 1: return ioport("IN1")->read();
		case 2: return ioport("IN2")->read();
		case 3: return ioport("IN3")->read();
		case 4: return ioport("IN4")->read();
		case 5: return ioport("IN5")->read();
		case 6: return ioport("DSW")->read();
	}
	logerror("%s: unknown input %02x read\n", machine().describe_context(), m_trivrus_input);
	return 0xff;
}

WRITE8_MEMBER(trivrus_state::trivrus_input_w)
{
	m_trivrus_input = data & 0xff;
}

READ32_MEMBER(trivrus_state::FlashCmd_r)
{
	if ((m_FlashCmd & 0xff) == 0xff)
	{
		if (m_Bank < m_maxbank)
		{
			uint32_t *ptr = (uint32_t*)(m_mainbank->base());
			return ptr[0];
		}
		else
			return 0xffffffff;
	}
	if ((m_FlashCmd & 0xff) == 0x90)
	{
		if (m_Bank < m_maxbank)
			return 0x00180089;  //Intel 128MBit
		else
			return 0xffffffff;
	}
	return 0;
}


WRITE32_MEMBER(trivrus_state::Banksw_w)
{
	m_Bank = (data >> 1) & 7;
	m_mainbank->set_entry(m_Bank);
}

void trivrus_state::trivrus_mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().nopw();

//  map(0x01280000, 0x01280003).w(FUNC(trivrus_state::Banksw_w));

//  0x01280000 & 0x0000ffff (written at boot)
	map(0x01500000, 0x01500000).rw(FUNC(trivrus_state::trivrus_input_r), FUNC(trivrus_state::trivrus_input_w));
//  0x01500010 & 0x000000ff = sec
//  0x01500010 & 0x00ff0000 = min
//  0x01500014 & 0x000000ff = hour
//  0x01500014 & 0x00ff0000 = day
//  0x01500018 & 0x000000ff = month
//  0x0150001c & 0x000000ff = year - 2000
	map(0x01600000, 0x01607fff).ram().share("nvram");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(trivrus_state::PIOldat_r), FUNC(trivrus_state::PIOldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(trivrus_state::PIOedat_r));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x0300ffff).m(m_vr0vid, FUNC(vr0video_device::regs_map));
	map(0x03800000, 0x03ffffff).ram().share("textureram");
	map(0x04000000, 0x047fffff).ram().share("frameram");
	map(0x04800000, 0x04800fff).rw(m_vr0snd, FUNC(vr0sound_device::vr0_snd_read), FUNC(vr0sound_device::vr0_snd_write));

	map(0x05000000, 0x05ffffff).bankr("mainbank");
	map(0x05000000, 0x05000003).rw(FUNC(trivrus_state::FlashCmd_r), FUNC(trivrus_state::FlashCmd_w));
}

#ifdef IDLE_LOOP_SPEEDUP

WRITE_LINE_MEMBER(trivrus_state::idle_skip_resume_w)
{
	m_FlipCntRead = 0;
	m_maincpu->resume(SUSPEND_REASON_SPIN);
}

WRITE_LINE_MEMBER(trivrus_state::idle_skip_speedup_w)
{
	m_FlipCntRead++;
	if (m_FlipCntRead >= 16 && m_vr0soc->irq_pending() == false && state == ASSERT_LINE)
		m_maincpu->suspend(SUSPEND_REASON_SPIN, 1);
}
#endif

void trivrus_state::machine_start()
{
	m_vr0vid->set_areas(reinterpret_cast<uint8_t*>(m_textureram.target()), reinterpret_cast<uint16_t*>(m_frameram.target()));
	m_vr0snd->set_areas(m_textureram, m_frameram);

	if (m_mainbank)
	{
		m_maxbank = (m_flash) ? m_flash.bytes() / 0x1000000 : 0;
		uint8_t *dummy_region = auto_alloc_array(machine(), uint8_t, 0x1000000);
		std::fill_n(&dummy_region[0], 0x1000000, 0xff); // 0xff Filled at Unmapped area
		uint8_t *ROM = (m_flash) ? (uint8_t *)&m_flash[0] : dummy_region;
		for (int i = 0; i < 8; i++)
		{
			if ((i < m_maxbank))
				m_mainbank->configure_entry(i, ROM + i * 0x1000000);
			else
				m_mainbank->configure_entry(i, dummy_region);
		}
	}

#ifdef IDLE_LOOP_SPEEDUP
	save_item(NAME(m_FlipCntRead));
#endif

	save_item(NAME(m_Bank));
	save_item(NAME(m_FlashCmd));
	save_item(NAME(m_PIO));
	save_item(NAME(m_trivrus_input));
}

void trivrus_state::machine_reset()
{
	m_Bank = 0;
	m_mainbank->set_entry(m_Bank);
	m_FlashCmd = 0xff;

#ifdef IDLE_LOOP_SPEEDUP
	m_FlipCntRead = 0;
#endif
}

static INPUT_PORTS_START(trivrus)
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Up")        PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Left/True") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Down")      PORT_CODE(KEYCODE_DOWN)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Enter/Exit")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Next")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Right/False") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Sound")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_OTHER )PORT_CODE(KEYCODE_9)

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Free Game
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   // Setup
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Interlace?" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Serial?" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )       // hangs at boot
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
	PORT_DIPNAME( 0x80, 0x80, "Touch Screen" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void trivrus_state::trivrus(machine_config &config)
{
	SE3208(config, m_maincpu, 14318180 * 3); // TODO : different between each PCBs
	m_maincpu->set_addrmap(AS_PROGRAM, &trivrus_state::trivrus_mem);
	m_maincpu->set_irq_acknowledge_callback(FUNC(trivrus_state::icallback));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// evolution soccer defaults
	m_screen->set_raw((XTAL(14'318'180)*2)/4, 455, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(trivrus_state::screen_update));
	m_screen->screen_vblank().set(FUNC(trivrus_state::screen_vblank));
	m_screen->set_palette("palette");

	VRENDER0_SOC(config, m_vr0soc, 0);
	m_vr0soc->set_host_cpu_tag(m_maincpu);
	m_vr0soc->set_host_screen_tag(m_screen);

	VIDEO_VRENDER0(config, m_vr0vid, 14318180, m_maincpu);
	#ifdef IDLE_LOOP_SPEEDUP
	m_vr0soc->idleskip_cb().set(FUNC(trivrus_state::idle_skip_resume_w));
	m_vr0vid->idleskip_cb().set(FUNC(trivrus_state::idle_skip_speedup_w));
	#endif

	PALETTE(config, "palette", palette_device::RGB_565);

	DS1302(config, m_ds1302, 32.768_kHz_XTAL);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SOUND_VRENDER0(config, m_vr0snd, 0);
	m_vr0snd->add_route(0, "lspeaker", 1.0);
	m_vr0snd->add_route(1, "rspeaker", 1.0);
}

ROM_START( trivrus )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u4", 0x00000, 0x80000, CRC(2d2e9a11) SHA1(73e7b19a032eae21312ca80f8c42cc16725496a7) )

	ROM_REGION32_LE( 0x2000000, "flash", ROMREGION_ERASEFF ) // Flash
	ROM_LOAD( "u3", 0x000000, 0x1000010, CRC(ba901707) SHA1(e281ba07024cd19ef1ab72d2197014f7b1f4d30f) )
ROM_END

GAME( 2009, trivrus,  0,        trivrus,  trivrus,  trivrus_state, empty_init,    ROT0, "AGT",                 "Trivia R Us (v1.07)", 0 )
