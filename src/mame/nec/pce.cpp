// license:BSD-3-Clause
// copyright-holders:Charles MacDonald, Wilbert Pol, Angelo Salese
/****************************************************************************

 PC-Engine / Turbo Grafx 16 driver
 by Charles Mac Donald
 E-Mail: cgfm2@hooked.net

 Thanks to David Shadoff and Brian McPhail for help with the driver.

****************************************************************************/

/**********************************************************************
          To-Do List:
- convert h6280-based drivers to internal memory map for the I/O region
- test sprite collision and overflow interrupts
- sprite precaching
- rewrite the base renderer loop
- Add expansion port support
- Add 263 line mode
- Sprite DMA should use vdc VRAM functions
- properly implement the pixel clocks instead of the simple scaling we do now

Banking
=======

Normally address spacebanks 00-F6 are assigned to regular HuCard ROM space. There
are a couple of special situations:

Street Fighter II:
  - address space banks 40-7F switchable by writing to 1FF0-1FF3
    1FF0 - select rom banks 40-7F
    1FF1 - select rom banks 80-BF
    1FF2 - select rom banks C0-FF
    1FF3 - select rom banks 100-13F

Populous:
  - address space banks 40-43 contains 32KB RAM

CDRom units:
  - address space banks 80-87 contains 64KB RAM

Super System Card:
  - address space banks 68-7F contains 192KB RAM

**********************************************************************/

/**********************************************************************
                          Known Bugs
***********************************************************************
- Cadash: After choosing character and name, the game starts and the display 'jiggles' like tracking if off a VCR
- Fighting Run: has color and sprite issues during gameplay;
**********************************************************************/

#include "emu.h"
#include "pce.h"

#include "bus/pce/pce_acard.h"
#include "bus/pce/pce_rom.h"
#include "bus/pce/pce_scdsys.h"
#include "cpu/h6280/h6280.h"
#include "sound/cdda.h"
#include "sound/msm5205.h"
#include "video/huc6202.h"
#include "video/huc6270.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"


static constexpr XTAL MAIN_CLOCK = XTAL(21'477'272);

static constexpr uint8_t TG_16_JOY_SIG = 0x00;
static constexpr uint8_t PCE_JOY_SIG   = 0x40;
//static constexpr uint8_t NO_CD_SIG     = 0x80;
//static constexpr uint8_t CD_SIG        = 0x00;
/* these might be used to indicate something, but they always seem to return 1 */
static constexpr uint8_t CONST_SIG     = 0x30;

// hucard pachikun gives you option to select pachinko controller after pressing start, likely because it doesn't have a true header id
static INPUT_PORTS_START( pce )
INPUT_PORTS_END

void pce_state::controller_w(u8 data)
{
	m_port_ctrl->sel_w(BIT(data, 0));
	m_port_ctrl->clr_w(BIT(data, 1));
}

u8 pce_state::controller_r()
{
	return (m_port_ctrl->port_r() & 0x0f) | m_io_port_options;
}


void pce_state::cd_intf_w(offs_t offset, u8 data)
{
	m_cd->update();

	m_cd->intf_w(offset, data);

	m_cd->update();
}

u8 pce_state::cd_intf_r(offs_t offset)
{
	m_cd->update();

	return m_cd->intf_r(offset);
}

void pce_state::pce_mem(address_map &map)
{
	map(0x100000, 0x10ffff).ram().share("cd_ram");
	map(0x110000, 0x1edfff).noprw();
	map(0x1ee000, 0x1ee7ff).rw(m_cd, FUNC(pce_cd_device::bram_r), FUNC(pce_cd_device::bram_w));
	map(0x1ee800, 0x1effff).noprw();
	map(0x1f0000, 0x1f1fff).ram().mirror(0x6000);
	map(0x1fe000, 0x1fe3ff).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
	map(0x1fe400, 0x1fe7ff).rw(m_huc6260, FUNC(huc6260_device::read), FUNC(huc6260_device::write));
	map(0x1ff800, 0x1ffbff).rw(FUNC(pce_state::cd_intf_r), FUNC(pce_state::cd_intf_w));
}

void pce_state::pce_io(address_map &map)
{
	map(0x00, 0x03).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
}


void pce_state::sgx_mem(address_map &map)
{
	map(0x100000, 0x10ffff).ram().share("cd_ram");
	map(0x110000, 0x1edfff).noprw();
	map(0x1ee000, 0x1ee7ff).rw(m_cd, FUNC(pce_cd_device::bram_r), FUNC(pce_cd_device::bram_w));
	map(0x1ee800, 0x1effff).noprw();
	map(0x1f0000, 0x1f7fff).ram();
	map(0x1fe000, 0x1fe007).rw("huc6270_0", FUNC(huc6270_device::read), FUNC(huc6270_device::write)).mirror(0x03e0);
	map(0x1fe008, 0x1fe00f).rw("huc6202", FUNC(huc6202_device::read), FUNC(huc6202_device::write)).mirror(0x03e0);
	map(0x1fe010, 0x1fe017).rw("huc6270_1", FUNC(huc6270_device::read), FUNC(huc6270_device::write)).mirror(0x03e0);
	map(0x1fe400, 0x1fe7ff).rw(m_huc6260, FUNC(huc6260_device::read), FUNC(huc6260_device::write));
	map(0x1ff800, 0x1ffbff).rw(FUNC(pce_state::cd_intf_r), FUNC(pce_state::cd_intf_w));
}


void pce_state::sgx_io(address_map &map)
{
	map(0x00, 0x03).rw("huc6202", FUNC(huc6202_device::io_read), FUNC(huc6202_device::io_write));
}


uint32_t pce_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_huc6260->video_update(bitmap, cliprect);
	return 0;
}


void pce_state::machine_start()
{
	if (m_cd)
		m_cd->late_setup();

	// saving is only partially supported: it should be fine with cart games
	// OTOH CD states are saved but not correctly restored!
	save_item(NAME(m_io_port_options));
}

void pce_state::machine_reset()
{
}

static void pce_cart(device_slot_interface &device)
{
	device.option_add_internal("rom", PCE_ROM_STD);
	device.option_add_internal("cdsys3u", PCE_ROM_CDSYS3U);
	device.option_add_internal("cdsys3j", PCE_ROM_CDSYS3J);
	device.option_add_internal("populous", PCE_ROM_POPULOUS);
	device.option_add_internal("sf2", PCE_ROM_SF2);
	device.option_add_internal("tennokoe", PCE_ROM_TENNOKOE);
	device.option_add_internal("acard_duo", PCE_ROM_ACARD_DUO);
	device.option_add_internal("acard_pro", PCE_ROM_ACARD_PRO);
}

void pce_state::pce_common(machine_config &config)
{
	// basic machine hardware
	H6280(config, m_maincpu, MAIN_CLOCK/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &pce_state::pce_mem);
	m_maincpu->set_addrmap(AS_IO, &pce_state::pce_io);
	m_maincpu->port_in_cb().set(FUNC(pce_state::controller_r));
	m_maincpu->port_out_cb().set(FUNC(pce_state::controller_w));
	m_maincpu->add_route(0, "speaker", 1.00, 0);
	m_maincpu->add_route(1, "speaker", 1.00, 1);

	config.set_maximum_quantum(attotime::from_hz(60));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MAIN_CLOCK, huc6260_device::WPF, 64, 64 + 1024 + 64, huc6260_device::LPF, 18, 18 + 242);
	screen.set_screen_update(FUNC(pce_state::screen_update));
	screen.set_palette(m_huc6260);

	HUC6260(config, m_huc6260, MAIN_CLOCK);
	m_huc6260->next_pixel_data().set("huc6270", FUNC(huc6270_device::next_pixel));
	m_huc6260->time_til_next_event().set("huc6270", FUNC(huc6270_device::time_until_next_event));
	m_huc6260->vsync_changed().set("huc6270", FUNC(huc6270_device::vsync_changed));
	m_huc6260->hsync_changed().set("huc6270", FUNC(huc6270_device::hsync_changed));

	huc6270_device &huc6270(HUC6270(config, "huc6270", 0));
	huc6270.set_vram_size(0x10000);
	huc6270.irq().set_inputline(m_maincpu, 0);

	SPEAKER(config, "speaker", 2).front();

	PCE_CONTROL_PORT(config, m_port_ctrl, pce_control_port_devices, "joypad2");

	// TODO: expansion port not emulated
	PCE_CD(config, m_cd, 0);
	m_cd->irq().set_inputline(m_maincpu, 1);
	m_cd->set_maincpu(m_maincpu);
	m_cd->add_route(0, "speaker", 1.0, 0);
	m_cd->add_route(1, "speaker", 1.0, 1);

	SOFTWARE_LIST(config, "cd_list").set_original("pcecd");
}


void pce_state::pce(machine_config &config)
{
	pce_common(config);
	PCE_CART_SLOT(config, m_cartslot, pce_cart, nullptr, "pce_cart").set_must_be_loaded(true);
	m_cartslot->set_address_space(m_maincpu, AS_PROGRAM);
	SOFTWARE_LIST(config, "cart_list").set_original("pce");

	// bundled pad (in white PC engine) has not support autofire
}


void pce_state::tg16(machine_config &config)
{
	pce_common(config);
	PCE_CART_SLOT(config, m_cartslot, pce_cart, nullptr, "tg16_cart").set_must_be_loaded(true);
	m_cartslot->set_address_space(m_maincpu, AS_PROGRAM);
	SOFTWARE_LIST(config, "cart_list").set_original("tg16");

	// turbo pad bundled
	m_port_ctrl->set_default_option("joypad2_turbo");
}


void pce_state::sgx(machine_config &config)
{
	// basic machine hardware
	H6280(config, m_maincpu, MAIN_CLOCK/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &pce_state::sgx_mem);
	m_maincpu->set_addrmap(AS_IO, &pce_state::sgx_io);
	m_maincpu->port_in_cb().set(FUNC(pce_state::controller_r));
	m_maincpu->port_out_cb().set(FUNC(pce_state::controller_w));
	m_maincpu->add_route(0, "speaker", 1.00, 0);
	m_maincpu->add_route(1, "speaker", 1.00, 1);

	config.set_maximum_quantum(attotime::from_hz(60));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MAIN_CLOCK, huc6260_device::WPF, 64, 64 + 1024 + 64, huc6260_device::LPF, 18, 18 + 242);
	screen.set_screen_update(FUNC(pce_state::screen_update));
	screen.set_palette(m_huc6260);

	HUC6260(config, m_huc6260, MAIN_CLOCK);
	m_huc6260->next_pixel_data().set("huc6202", FUNC(huc6202_device::next_pixel));
	m_huc6260->time_til_next_event().set("huc6202", FUNC(huc6202_device::time_until_next_event));
	m_huc6260->vsync_changed().set("huc6202", FUNC(huc6202_device::vsync_changed));
	m_huc6260->hsync_changed().set("huc6202", FUNC(huc6202_device::hsync_changed));

	huc6270_device &huc6270_0(HUC6270(config, "huc6270_0", 0));
	huc6270_0.set_vram_size(0x10000);
	huc6270_0.irq().set_inputline(m_maincpu, 0); // needs input merger?

	huc6270_device &huc6270_1(HUC6270(config, "huc6270_1", 0));
	huc6270_1.set_vram_size(0x10000);
	huc6270_1.irq().set_inputline(m_maincpu, 0); // needs input merger?

	huc6202_device &huc6202(HUC6202(config, "huc6202", 0 ));
	huc6202.next_pixel_0_callback().set("huc6270_0", FUNC(huc6270_device::next_pixel));
	huc6202.time_til_next_event_0_callback().set("huc6270_0", FUNC(huc6270_device::time_until_next_event));
	huc6202.vsync_changed_0_callback().set("huc6270_0", FUNC(huc6270_device::vsync_changed));
	huc6202.hsync_changed_0_callback().set("huc6270_0", FUNC(huc6270_device::hsync_changed));
	huc6202.read_0_callback().set("huc6270_0", FUNC(huc6270_device::read));
	huc6202.write_0_callback().set("huc6270_0", FUNC(huc6270_device::write));
	huc6202.next_pixel_1_callback().set("huc6270_1", FUNC(huc6270_device::next_pixel));
	huc6202.time_til_next_event_1_callback().set("huc6270_1", FUNC(huc6270_device::time_until_next_event));
	huc6202.vsync_changed_1_callback().set("huc6270_1", FUNC(huc6270_device::vsync_changed));
	huc6202.hsync_changed_1_callback().set("huc6270_1", FUNC(huc6270_device::hsync_changed));
	huc6202.read_1_callback().set("huc6270_1", FUNC(huc6270_device::read));
	huc6202.write_1_callback().set("huc6270_1", FUNC(huc6270_device::write));

	SPEAKER(config, "speaker", 2).front();

	// turbo pad bundled
	PCE_CONTROL_PORT(config, m_port_ctrl, pce_control_port_devices, "joypad2_turbo");

	PCE_CART_SLOT(config, m_cartslot, pce_cart, nullptr, "pce_cart").set_must_be_loaded(true);
	m_cartslot->set_address_space(m_maincpu, AS_PROGRAM);
	SOFTWARE_LIST(config, "cart_list").set_original("sgx");
	SOFTWARE_LIST(config, "pce_list").set_compatible("pce");

	// TODO: expansion port not emulated
	PCE_CD(config, m_cd, 0);
	m_cd->irq().set_inputline(m_maincpu, 1);
	m_cd->set_maincpu(m_maincpu);
	m_cd->add_route(0, "speaker", 1.0, 0);
	m_cd->add_route(1, "speaker", 1.0, 1);

	SOFTWARE_LIST(config, "cd_list").set_original("pcecd");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pce )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
ROM_END

#define rom_tg16 rom_pce
#define rom_sgx rom_pce

void pce_state::init_pce()
{
	m_io_port_options = PCE_JOY_SIG | CONST_SIG;
}

void pce_state::init_tg16()
{
	m_io_port_options = TG_16_JOY_SIG | CONST_SIG;
}

CONS( 1987, pce,  0,   0, pce,  pce, pce_state, init_pce,  "NEC / Hudson Soft", "PC Engine",     MACHINE_IMPERFECT_SOUND )
CONS( 1989, tg16, pce, 0, tg16, pce, pce_state, init_tg16, "NEC / Hudson Soft", "TurboGrafx-16", MACHINE_IMPERFECT_SOUND )
CONS( 1989, sgx,  pce, 0, sgx,  pce, pce_state, init_pce,  "NEC / Hudson Soft", "SuperGrafx",    MACHINE_IMPERFECT_SOUND )
// TODO: TurboGrafx for PAL region?
