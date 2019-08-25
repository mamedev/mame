// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************

    Sealy HW running on VRender0+ SoC

    driver by Angelo Salese, based off original crystal.cpp by ElSemi

    TODO:
    - HY04 protection (controls tile RNG at very least)
    - 8bpp colors are washed, data from flash ROMs is XORed with contents
      of NVRAM area 0x14000700-80f, might be shared with HY04 as well.
    - EEPROM hookup;
    - extract password code when entering test mode in-game;

=============================================================================

Crazy Dou Di Zhu II
Sealy, 2006

PCB Layout
----------

070405-fd-VER1.2
|--------------------------------------|
|       PAL        27C322.U36          |
|                               BATTERY|
|    M59PW1282     62256  14.31818MHz  |
|                             W9864G66 |
|                                      |
|J                     VRENDERZERO+    |
|A            W9864G66                 |
|M                            W9864G66 |
|M              8MHz                   |
|A    HY04    0260F8A                  |
|                     28.63636MHz      |
|                                      |
|                 VR1       TLDA1311   |
|                               TDA1519|
|  18WAY                  VOL  10WAY   |
|--------------------------------------|
Notes:
      0260F8A   - unknown TQFP44
      HY04      - rebadged DIP8 PIC - type unknown *
      W9864G66  - Winbond 64MBit DRAM
      M59PW1282 - ST Microelectronics 128MBit SOP44 FlashROM.
                  This is two 64MB SOP44 ROMs in one package

* The pins are:
  1 ground
  2 nothing
  3 data (only active for 1/4 second when the playing cards or "PASS" shows in game next to each player)
  4 nothing
  5 nothing
  6 clock
  7 +5V (could be VPP for programming voltage)
  8 +5V

=====

Meng Hong Lou (Dream of the Red Chamber)
Sealy, 2004?

Red PCB, very similar to crzyddz2

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

class menghong_state : public driver_device
{
public:
	menghong_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_textureram(*this, "textureram"),
		m_frameram(*this, "frameram"),
		m_flash(*this, "flash"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
		m_vr0vid(*this, "vr0vid"),
		m_vr0snd(*this, "vr0snd"),
		m_ds1302(*this, "rtc"),
		m_screen(*this, "screen"),
		m_eeprom(*this, "eeprom")
	{ }


	void crzyddz2(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_textureram;
	required_shared_ptr<uint32_t> m_frameram;
	optional_region_ptr<uint32_t> m_flash;
	optional_memory_bank m_mainbank;

	/* devices */
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<vr0video_device> m_vr0vid;
	required_device<vr0sound_device> m_vr0snd;
	required_device<ds1302_device> m_ds1302;
	required_device<screen_device> m_screen;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

#ifdef IDLE_LOOP_SPEEDUP
	uint8_t     m_FlipCntRead;
	DECLARE_WRITE_LINE_MEMBER(idle_skip_resume_w);
	DECLARE_WRITE_LINE_MEMBER(idle_skip_speedup_w);
#endif

	uint32_t    m_Bank;
	uint32_t    m_maxbank;
	uint32_t    m_FlashCmd;

	DECLARE_WRITE32_MEMBER(Banksw_w);
	DECLARE_READ32_MEMBER(FlashCmd_r);
	DECLARE_WRITE32_MEMBER(FlashCmd_w);

	DECLARE_READ32_MEMBER(crzyddz2_key_r);

	IRQ_CALLBACK_MEMBER(icallback);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void crzyddz2_mem(address_map &map);

	// PIO
	DECLARE_READ32_MEMBER(PIOldat_r);
	DECLARE_WRITE32_MEMBER(PIOldat_w);
	DECLARE_READ32_MEMBER(PIOedat_r);
	uint32_t m_PIO;
	DECLARE_WRITE32_MEMBER(crzyddz2_PIOldat_w);
	DECLARE_READ32_MEMBER(crzyddz2_PIOedat_r);
	uint8_t m_crzyddz2_prot;
};



uint32_t menghong_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_vr0soc->crt_is_blanked()) // Blank Screen
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	m_vr0vid->screen_update(screen, bitmap, cliprect);
	return 0;
}

WRITE_LINE_MEMBER(menghong_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		if (m_vr0soc->crt_active_vblank_irq() == true)
			m_vr0soc->IntReq(24);      //VRender0 VBlank

		m_vr0vid->execute_drawing();
	}
}

IRQ_CALLBACK_MEMBER(menghong_state::icallback)
{
	return m_vr0soc->irq_callback();
}

WRITE32_MEMBER(menghong_state::Banksw_w)
{
	m_Bank = (data >> 1) & 7;
	m_mainbank->set_entry(m_Bank);
}

READ32_MEMBER(menghong_state::FlashCmd_r)
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

WRITE32_MEMBER(menghong_state::FlashCmd_w)
{
	m_FlashCmd = data;
}



// Crazy Dou Di Zhu II
// To do: HY04 (pic?) protection, 93C46 hookup

READ32_MEMBER(menghong_state::PIOldat_r)
{
	return m_PIO;
}

WRITE32_MEMBER(menghong_state::crzyddz2_PIOldat_w)
{
	COMBINE_DATA(&m_PIO);
	//uint32_t RST = data & 0x01000000;
	//uint32_t CLK = data & 0x02000000;
	//uint32_t DAT = data & 0x10000000;

//  m_eeprom->cs_write(RST ? 1 : 0);
//  m_eeprom->di_write(DAT ? 1 : 0);
//  m_eeprom->clk_write(CLK ? 1 : 0);

	if (ACCESSING_BITS_8_15)
	{
		int mux = (m_PIO >> 8) & 0x1f;
		if (mux == 0x1f)
		{
			m_crzyddz2_prot = ((m_PIO >> 8) & 0xc0) ^ 0x40;
			logerror("%s: PIO = %08x, prot = %02x\n", machine().describe_context(), m_PIO, m_crzyddz2_prot);
		}
	}
}

READ32_MEMBER(menghong_state::crzyddz2_PIOedat_r)
{
	return 0;//m_eeprom->do_read();
}

READ32_MEMBER(menghong_state::crzyddz2_key_r)
{
	static const char *const key_names[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };

	int mux = (m_PIO >> 8) & 0x1f;

	uint8_t data = 0x3f;
	for (int i = 0; i < sizeof(key_names)/sizeof(key_names[0]); ++i)
		if (!BIT(mux,i))
			data =  ioport(key_names[i])->read();

/*
crzyddz2    in      out
            00      40
            40      00
            c0      80
*/
// menghong Sealy logo pal offset is at 0x3ea7400, relevant code is at 2086034
//  m_crzyddz2_prot = (m_PIO >> 8) & 0xc0) ^ 0x40;
	m_crzyddz2_prot = (machine().rand() & 0xc0);

	return 0xffffff00 | data | m_crzyddz2_prot;
}

void menghong_state::crzyddz2_mem(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().nopw();

	map(0x01280000, 0x01280003).w(FUNC(menghong_state::Banksw_w));
	map(0x01400000, 0x0140ffff).ram().share("nvram");
	map(0x01500000, 0x01500003).portr("P1_P2");
	map(0x01500004, 0x01500007).r(FUNC(menghong_state::crzyddz2_key_r));

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(menghong_state::PIOldat_r), FUNC(menghong_state::crzyddz2_PIOldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(menghong_state::crzyddz2_PIOedat_r));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x0300ffff).m(m_vr0vid, FUNC(vr0video_device::regs_map));
	map(0x03800000, 0x03ffffff).ram().share("textureram");
	map(0x04000000, 0x047fffff).ram().share("frameram");
	map(0x04800000, 0x04800fff).rw(m_vr0snd, FUNC(vr0sound_device::vr0_snd_read), FUNC(vr0sound_device::vr0_snd_write));

	map(0x05000000, 0x05ffffff).bankr("mainbank");
	map(0x05000000, 0x05000003).rw(FUNC(menghong_state::FlashCmd_r), FUNC(menghong_state::FlashCmd_w));
}

#ifdef IDLE_LOOP_SPEEDUP

WRITE_LINE_MEMBER(menghong_state::idle_skip_resume_w)
{
	m_FlipCntRead = 0;
	m_maincpu->resume(SUSPEND_REASON_SPIN);
}

WRITE_LINE_MEMBER(menghong_state::idle_skip_speedup_w)
{
	m_FlipCntRead++;
	if (m_FlipCntRead >= 16 && m_vr0soc->irq_pending() == false && state == ASSERT_LINE)
		m_maincpu->suspend(SUSPEND_REASON_SPIN, 1);
}
#endif

void menghong_state::machine_start()
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
}

void menghong_state::machine_reset()
{
	m_Bank = 0;
	m_mainbank->set_entry(m_Bank);
	m_FlashCmd = 0xff;

#ifdef IDLE_LOOP_SPEEDUP
	m_FlipCntRead = 0;
#endif

	m_crzyddz2_prot = 0x00;
}

static INPUT_PORTS_START(crzyddz2)
	PORT_START("P1_P2") // 1500002 & 1500000
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) // up
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) // down  (next secret code)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) // left  (inc secret code)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // right (dec secret code)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1        ) // A
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2        ) // B
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3        ) // C     (bet)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4        ) // D
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START1         ) // start (secret code screen)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_SERVICE2       ) // .. 2  (next secret code / stats)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE        ) // .. 1  (secret code screen / service mode)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_SERVICE1       ) // .. 3  (inc secret code / credit)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_SERVICE3       ) // .. 4  (exit secret screen / clear credits)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SERVICE4       ) //       (reset and clear ram?)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// 1500004 (multiplexed by 1802005)
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E         )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M         )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN       ) // kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) // start?

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F         )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N         )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH     ) // ?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET       ) // ? + C

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G         )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI       ) // chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE     ) // ?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H         )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON       ) // pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON       ) // ron
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG       ) // big
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL     ) // small + D
INPUT_PORTS_END

void menghong_state::crzyddz2(machine_config &config)
{
	SE3208(config, m_maincpu, 14318180 * 3); // TODO : different between each PCBs
	m_maincpu->set_addrmap(AS_PROGRAM, &menghong_state::crzyddz2_mem);
	m_maincpu->set_irq_acknowledge_callback(FUNC(menghong_state::icallback));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// evolution soccer defaults
	m_screen->set_raw((XTAL(14'318'180)*2)/4, 455, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(menghong_state::screen_update));
	m_screen->screen_vblank().set(FUNC(menghong_state::screen_vblank));
	m_screen->set_palette("palette");

	VRENDER0_SOC(config, m_vr0soc, 0);
	m_vr0soc->set_host_cpu_tag(m_maincpu);
	m_vr0soc->set_host_screen_tag(m_screen);

	VIDEO_VRENDER0(config, m_vr0vid, 14318180, m_maincpu);
	#ifdef IDLE_LOOP_SPEEDUP
	m_vr0soc->idleskip_cb().set(FUNC(menghong_state::idle_skip_resume_w));
	m_vr0vid->idleskip_cb().set(FUNC(menghong_state::idle_skip_speedup_w));
	#endif

	PALETTE(config, "palette", palette_device::RGB_565);

	DS1302(config, m_ds1302, 32.768_kHz_XTAL);
	EEPROM_93C46_16BIT(config, "eeprom");

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SOUND_VRENDER0(config, m_vr0snd, 0);
	m_vr0snd->add_route(0, "lspeaker", 1.0);
	m_vr0snd->add_route(1, "rspeaker", 1.0);
}

ROM_START( crzyddz2 )
	ROM_REGION32_LE( 0x1000000, "flash", 0 ) // Flash
	ROM_LOAD( "rom.u48", 0x000000, 0x1000000, CRC(0f3a1987) SHA1(6cad943846c79db31226676c7391f32216cfff79) )

	ROM_REGION( 0x0400000, "maincpu", ROMREGION_ERASEFF )
	//ROM_COPY( "flash",      0x000000, 0x000000, 0x1000000 ) // copy flash here
	ROM_LOAD( "27c322.u49", 0x000000, 0x0400000, CRC(b3177f39) SHA1(2a28bf8045bd2e053d88549b79fbc11f30ef9a32) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x4280, "pic", 0 ) // hy04
	ROM_LOAD("hy04", 0x000000, 0x4280, NO_DUMP )
ROM_END

ROM_START( menghong )
	ROM_REGION32_LE( 0x1000000, "flash", 0 ) // Flash
	ROM_LOAD( "rom.u48", 0x000000, 0x1000000, CRC(e24257c4) SHA1(569d79a61ff6d35100ba5727069363146df9e0b7) )

	ROM_REGION( 0x0400000, "maincpu", 0 )
	//ROM_COPY( "flash",      0x000000, 0x000000, 0x1000000 ) // copy flash here
	ROM_LOAD( "060511_08-01-18.u49",  0x0000000, 0x0200000, CRC(b0c12107) SHA1(b1753757bbdb7d996df563ac6abdc6b46676704b) ) // 27C160
	ROM_RELOAD(                       0x0200000, 0x0200000 )

	ROM_REGION( 0x4280, "pic", 0 ) // hy04
	ROM_LOAD("menghong_hy04", 0x000000, 0x4280, NO_DUMP )
ROM_END

GAME( 2004?,menghong, 0,        crzyddz2, crzyddz2, menghong_state, empty_init,    ROT0, "Sealy", "Meng Hong Lou", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 2006, crzyddz2, 0,        crzyddz2, crzyddz2, menghong_state, empty_init,    ROT0, "Sealy", "Crazy Dou Di Zhu II", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
