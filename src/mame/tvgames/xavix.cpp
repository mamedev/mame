// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese, MagikalUnicorn
/***************************************************************************

    XaviX TV plug-and-play consoles and Let's! Play TV Classic systems.
    Defines board configurations, ROM mappings, inputs, power control and
    external peripheral connections around the shared XaviX SoC devices.

    The CPU is a 6502 derivative with extra opcodes for far calls. Parts marked
    SSD 2000 and SSD 2002 add further opcodes beyond the 1997/1998 generation
    and have separate driver files.

    NOTES:
    Play TV Monster Truck runs off an entirely different codebase to
    everything else, presumably coded by the developer from scratch rather
    than using code supplied by SSD Company Ltd.
    Play TV Rescue Heroes fails to display any kind of XaviX logo or SSD
    copyright. It is the only XaviX-based game so far to not show these
    details anywhere in the game.

    Test-mode entry:
    - rad_bb2: Reset while holding Console X and Console O (works every other
      reset), then release. Use Console X to cycle through modes until reaching
      the input test.
    - rad_mtrk, rad_mtrkp: Hold Horn and Nitro on startup, then release.
    - rad_rh: Hold Joystick Right and Menu on startup, then release.
    - ltv_tam: Hold both Push Button / Confirm buttons from startup and soft
      reset during the Bandai logo (about five seconds after startup), then
      release. A cold start alone does not enter the test.
    - ban_krrj: Hold Right Arrow and X from startup and soft reset about
      3.1 seconds later, then release.
    - tak_wdg: Hold Left and Right on startup, then release.
    - tomplc: Hold Push-to-Talk and Headlights / Left from startup, then soft
      reset and release.
    - epo_tp2s: Hold Blue Mat Right and Red Mat Confirm / Orange from startup,
      then soft reset and release.
    - epo_tp2p: Hold Right Side Right and Left Side Confirm from startup and
      soft reset about 2.7 seconds later, then release.
    - pl1000: Hold P1 Trigger, P1 Start and P2 Start on startup, then release.

    Undumped Software (preliminary; some titles may still be missing):
    Compiled from various sources; thanks to Sean Riddle.

    year      name                                                                                            PCB ID      ROM width       TSOP pads   ROM size        SEEPROM             die markings            extra components / notes
    2006      Let's!TVプレイ ふしぎ星のふたご姫Gyu! ドレスチェンジでキュートにダンス / バンダイ / 日本   Let's!TV Play Gyu, the Twin Princess of the Mysterious Planet! Dance cutely with a dress change / Bandai / Japan
              Jala Jaland /atlus/Japan (arcade version)                                                       -           -               -           -               -                   -                       -
    2004      Printer for TV computer /EPOCH/Japan                                                            -           -               -           -               -                   -                       -
              Wildest computer robot "Daigander" (Korean version) /TAKARA/Korea                               -           -               -           -               -                   -                       -
    2001      Ping-pong(Chinese version) /Tenpon/China                                                        -           -               -           -               -                   -                       -
              Baseball Korean version /SONOKONG/Korea                                                         -           -               -           -               -                   -                       -
    1999      ABC Jungle Fun Hippo VTech/HK (USA and UK versions only, FR is dumped)                          -           -               -           -               -                   -                       -

    TODO:
    - hikara: Verify CT675A-F video processing, reset values, panel
      scaling and colour processing.
    - Shared battery monitoring (e.g. rad_bb/rad_bb2, epo_epp2/epo_epp3
      and hikara): Measure physical voltage scaling.
    - RC power-hold boards (e.g. ekara, popira, evio and epo_guru): Measure
      physical decay time.
    - Sensor peripherals (e.g. bistro, rad_bb/rad_bb2 and rad_mtrk/rad_mtrkp):
      Verify physical timing and remaining sensor encodings.
    - Optical inputs (e.g. gungunad/gungunrv, rad_hnt/rad_hnt2, tomshoot,
      epo_dmon, pl1000 and tak_daig): Verify physical calibration and event timing.
    - Shared IR controllers (e.g. epo_epp/epo_epp2/epo_epp3, tak_comt and
      tak_wdg): Measure physical report cadence.
    - jarajal: Measure the hopper pulse interval and establish payout-control
      wiring.
    - epo_guru: Measure the physical pedal-resistance motor force response.

***************************************************************************/

#include "emu.h"
#include "xavix.h"
#include "xavix_io_bb_ball.h"
#include "xavix_io_bb_bat.h"
#include "xavix_io_beyu_link.h"
#include "xavix_io_beyu_ripcord.h"
#include "xavix_io_madf_ball.h"
#include "xavix_io_mtrk_wheel.h"

#include "sound/beep.h"

#include "render.h"
#include "softlist_dev.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>

#include "ekara.lh"
#include "hikara.lh"
#include "multiplt.lh"

#define LOG_IO         (1U << 3)

// #define VERBOSE 1
#include "logmacro.h"

//**************************************************************************
//  Shared Hardware
//**************************************************************************

// Instruction fetches use a code-bank view separate from data accesses.  Keeping this in program
// space also preserves disassembly when executing from low-bus RAM.
void xavix_state::xavix_map(address_map &map)
{
	map(0x000000, 0x7fffff).r(FUNC(xavix_state::code_low_bank_r));
	map(0x800000, 0xffffff).r(FUNC(xavix_state::code_high_bank_r));

	map(0x000000, 0x003fff).w(FUNC(xavix_state::debug_mem_w));
}

// Mapped external accesses arrive here after the CPU has selected the appropriate view.
void xavix_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom().region("bios", 0x00000);
}

void xavix_state::xavix_1mb_extbus_map(address_map &map)
{
	map(0x000000, 0x0fffff).mirror(0x700000).rom().region("bios", 0x00000);
}

void xavix_state::xavix_2mb_extbus_map(address_map &map)
{
	map(0x000000, 0x1fffff).mirror(0x600000).rom().region("bios", 0x00000);
}

void xavix_state::xavix_4mb_extbus_map(address_map &map)
{
	map(0x000000, 0x3fffff).mirror(0x400000).rom().region("bios", 0x00000);
}

void xavix_state::configure_board(machine_config &config)
{
	m_soc->set_program_map(*this, &xavix_state::xavix_map);
	m_soc->set_external_map(*this, &xavix_state::xavix_extbus_map);
	m_soc->set_screen(m_screen);
	m_soc->set_palette(m_palette);
	m_soc->set_gfx_region(m_rom);
	m_soc->set_video_read_callback(*this, FUNC(xavix_state::read_banked_data_bypass));
	m_soc->set_vblank_callback(*this, FUNC(xavix_state::vblank_extra));
	m_soc->lightgun_read_callback().set(*this, FUNC(xavix_state::lightgun_r));
	m_soc->power_read_callback().set(*this, FUNC(xavix_state::power_status_r));
	m_soc->power_off_callback().set(*this, FUNC(xavix_state::power_off_w));
	m_soc->external_bus_ctrl_callback().set(*this, FUNC(xavix_state::external_bus_ctrl_changed));

	m_soc->io().input_callback<0>().set(*this, FUNC(xavix_state::io0_input_cb));
	m_soc->io().input_callback<1>().set(*this, FUNC(xavix_state::io1_input_cb));
	m_soc->io().output_callback<0>().set(*this, FUNC(xavix_state::io0_output_cb));
	m_soc->io().output_callback<1>().set(*this, FUNC(xavix_state::io1_output_cb));

	m_soc->adc().input_callback<0>().set_ioport("AN0");
	m_soc->adc().input_callback<1>().set_ioport("AN1");
	m_soc->adc().input_callback<2>().set_ioport("AN2");
	m_soc->adc().input_callback<3>().set_ioport("AN3");
	m_soc->adc().input_callback<4>().set_ioport("AN4");
	m_soc->adc().input_callback<5>().set_ioport("AN5");
	m_soc->adc().input_callback<6>().set_ioport("AN6");
	m_soc->adc().input_callback<7>().set_ioport("AN7");

	m_soc->dma_source_read_callback().set(*this, FUNC(xavix_state::read_banked_data_bypass));

	PALETTE(config, m_palette, palette_device::BLACK, 256);

	SCREEN(config, m_screen);
	// Round the raster to whole pixels/lines, retaining the 525-half-line field period.
	// TODO: Confirm the earlier chips' exact counter endpoints and output phase.
	m_screen->set_raw(m_soc->clock() / xavix_video_device::PICTURE_PIXEL_CLOCKS,
			NTSC_RASTER_WIDTH, NTSC_HBLANK_END, NTSC_RASTER_WIDTH, NTSC_RASTER_HEIGHT, NTSC_VBLANK_END, NTSC_RASTER_HEIGHT);
	m_screen->set_refresh(attotime::from_ticks(
			u64(NTSC_LINE_CLOCKS) * xavix_video_device::NTSC_FIELD_HALF_LINES, m_soc->clock() * 2));
	m_screen->set_video_attributes(VIDEO_SELF_RENDER);
	m_screen->set_screen_update(FUNC(xavix_state::screen_update));
	m_screen->screen_vblank().append(FUNC(xavix_state::screen_vblank));

	m_soc->sound_sample_read_callback().set(*this, FUNC(xavix_state::read_banked_data_bypass));
	m_soc->add_route(0, "speaker", 1.0, 0);
	m_soc->add_route(1, "speaker", 1.0, 1);

	SPEAKER(config, "speaker", 2).front();
}

void xavix_state::configure_power_hold(machine_config &config)
{
	TIMER(config, m_power_hold_timer).configure_generic(FUNC(xavix_state::power_hold_expired));
	m_soc->io().output_callback<1>().append(*this, FUNC(xavix_state::io1_power_hold_w));
}

void xavix_power_state::configure_board(machine_config &config)
{
	xavix_state::configure_board(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(xavix_power_state::io1_power_off_w));
}

void xavix_i2c_power_state::configure_board(machine_config &config)
{
	xavix_i2c_state::configure_board(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(xavix_i2c_power_state::io1_power_off_w));
}

void xavix_io0_matrix_power_state::configure_board(machine_config &config)
{
	xavix_io0_matrix_state::configure_board(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(xavix_io0_matrix_power_state::io1_power_off_w));
}

void xavix_ir_power_state::configure_board(machine_config &config)
{
	xavix_ir_state::configure_board(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(xavix_ir_power_state::io1_power_off_w));
}

void xavix_state::xavix(machine_config &config)
{
	XAVIX_SOC(config, m_soc, NTSC_MASTER_CLOCK);
	configure_board(config);
}

void xavix_state::xavix_1mb(machine_config &config)
{
	xavix(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_1mb_extbus_map);
}

void xavix_state::xavix_2mb(machine_config &config)
{
	xavix(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_2mb_extbus_map);
}

void xavix_state::xavix_4mb(machine_config &config)
{
	xavix(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_4mb_extbus_map);
}

void xavix_state::xavix_nv(machine_config &config)
{
	xavix(config);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void xavix_state::xavix_nv_1mb(machine_config &config)
{
	xavix_nv(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_1mb_extbus_map);
}

void xavix_state::xavix_nv_2mb(machine_config &config)
{
	xavix_nv(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_2mb_extbus_map);
}

void xavix_state::xavix_nv_4mb(machine_config &config)
{
	xavix_nv(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_4mb_extbus_map);
}

void xavix_state::xavixp(machine_config &config)
{
	xavix(config);
	m_soc->set_video_standard(video_standard::PAL);
	m_soc->set_clock(PAL_MASTER_CLOCK);
	m_screen->set_raw(m_soc->clock() / xavix_video_device::PICTURE_PIXEL_CLOCKS,
			PAL_RASTER_WIDTH, PAL_HBLANK_END, PAL_RASTER_WIDTH, PAL_RASTER_HEIGHT, PAL_VBLANK_END, PAL_RASTER_HEIGHT);
	m_screen->set_refresh(attotime::from_ticks(
			u64(PAL_LINE_CLOCKS) * xavix_video_device::PAL_FIELD_HALF_LINES, m_soc->clock() * 2));
}

void xavix_state::xavixp_1mb(machine_config &config)
{
	xavixp(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_1mb_extbus_map);
}

void xavix_state::xavixp_2mb(machine_config &config)
{
	xavixp(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_2mb_extbus_map);
}

void xavix_state::xavixp_4mb(machine_config &config)
{
	xavixp(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_4mb_extbus_map);
}

void xavix_state::xavixp_nv(machine_config &config)
{
	xavixp(config);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void xavix_state::xavixp_nv_1mb(machine_config &config)
{
	xavixp_nv(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_1mb_extbus_map);
}

//-------------------------------------------------
//  Machine Lifecycle
//-------------------------------------------------

void xavix_state::machine_start()
{
	// Synthetic microphone tones do not need the beeper's 32x oversampling.
	for (beep_device &beeper : device_type_enumerator<beep_device>(*this))
		beeper.output_to_stream_output(0).first->set_sample_rate(machine().sample_rate());

	m_soc->cpu().space(xavix_device::AS_LOW_BUS).specific(m_code_lowbus);
	m_screen_output.start(machine());
	if (m_nvram)
	{
		auto const ram = m_soc->backup_ram();
		m_nvram->set_base(ram.data(), ram.size());
	}
	save_item(NAME(m_power_hold_level));
}

void xavix_state::device_stop()
{
	m_screen_output.stop(machine());
	driver_device::device_stop();
}

void xavix_state::machine_reset()
{
	m_power_hold_level = 0xff;
	if (m_power_hold_timer)
		m_power_hold_timer->adjust(attotime::never);
}

//-------------------------------------------------
//  Screen Output
//-------------------------------------------------

void xavix_state::screen_output::start(running_machine &machine)
{
	m_texture = machine.render().texture_alloc();
}

void xavix_state::screen_output::stop(running_machine &machine)
{
	machine.render().texture_free(m_texture);
}

void xavix_state::screen_output::update(screen_device &screen, bitmap_rgb32 const &bitmap, rectangle const &cliprect, rectangle area)
{
	if ((cliprect.bottom() != screen.visible_area().bottom()) || (cliprect.right() != screen.visible_area().right()))
		return;

	// Publish the completed picture before post-processing, not a zoomed electrical raster.
	area &= screen.visible_area();
	area &= bitmap.cliprect();
	bitmap_rgb32 &display = m_bitmap[m_index];
	if (area.empty())
	{
		display.resize(1, 1);
		display.fill(rgb_t::black());
	}
	else
	{
		display.resize(area.width(), area.height());
		copybitmap(display, bitmap, 0, 0, -area.left(), -area.top(), display.cliprect());
	}
	m_texture->set_bitmap(display, display.cliprect(), TEXFORMAT_RGB32);
	m_index ^= 1;
	restore(screen);
}

void xavix_state::screen_output::restore(screen_device &screen)
{
	screen.container().empty();
	if (m_bitmap[m_index ^ 1].valid())
		screen.container().add_quad(0.0f, 0.0f, 1.0f, 1.0f, rgb_t::white(), m_texture,
				PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));
}

u32 xavix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u32 const result = m_soc->screen_update(screen, bitmap, cliprect);
	m_screen_output.update(screen, bitmap, cliprect, m_soc->picture_area());
	return result;
}

void xavix_state::screen_vblank(int state)
{
	// Discard transient overlays even when the next field is skipped.
	if (!state)
		m_screen_output.restore(*m_screen);
}

//-------------------------------------------------
//  Reset and Power Controls
//-------------------------------------------------

INPUT_CHANGED_MEMBER(xavix_state::reset_button)
{
	// Reset the SoC controls while retaining its RAM.
	m_soc->reset_w(newval);
}

u8 xavix_state::power_status_r()
{
	// Software debounces bit 7 and acts on its active-high to inactive transition.
	return m_power.read_safe(0) & 0x80;
}

INPUT_CHANGED_MEMBER(xavix_state::power_off)
{
	if (newval && !machine().side_effects_disabled())
		machine().schedule_exit();
}

TIMER_DEVICE_CALLBACK_MEMBER(xavix_state::power_hold_expired)
{
	machine().schedule_exit();
}

void xavix_state::power_off_w(int state)
{
	if (state)
		machine().schedule_exit();
}

//-------------------------------------------------
//  IO & Power Hold Levels
//-------------------------------------------------

u8 xavix_state::io0_input_r(u8 direction)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IO, "%s: io0_input_r\n", machine().describe_context());

	// no special handling
	return m_in0->read();
}

u8 xavix_state::io1_input_r(u8 direction)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IO, "%s: io1_input_r\n", machine().describe_context());

	// no special handling
	return m_in1->read();
}

void xavix_state::io0_output_w(u8 data, u8 direction)
{
	// no special handling
}

void xavix_state::io1_output_w(u8 data, u8 direction)
{
	// no special handling
}

u8 xavix_state::io0_input_cb(offs_t, u8 mem_mask)
{
	return io0_input_r(mem_mask);
}

u8 xavix_state::io1_input_cb(offs_t, u8 mem_mask)
{
	return io1_input_r(mem_mask);
}

void xavix_state::io0_output_cb(offs_t, u8 data, u8 mem_mask)
{
	io0_output_w(data, mem_mask);
}

void xavix_state::io1_output_cb(offs_t, u8 data, u8 mem_mask)
{
	io1_output_w(data, mem_mask);
}

void xavix_state::io1_power_hold_w(offs_t, u8 data, u8 mem_mask)
{
	if (BIT(mem_mask, 7))
	{
		u8 const level = BIT(data, 7);
		if (m_power_hold_level == 0xff)
			m_power_hold_level = level;
		else if (level != m_power_hold_level)
		{
			m_power_hold_level = level;
			// Patents US7093164 and WO2004093053A1 identify IO1.7 as the RC power-hold pulse; decay time is unmeasured.
			m_power_hold_timer->adjust(attotime::from_seconds(2));
		}
	}
}

void xavix_state::io1_power_off_w(offs_t, u8 data, u8 mem_mask)
{
	// Selected boards drive IO1.7 high as the final step of software shutdown.
	if (BIT(data & mem_mask, 7) && !machine().side_effects_disabled())
		machine().schedule_exit();
}

u8 xavix_io0_matrix_state::io0_input_r(u8 direction)
{
	// IO0 bits 2 and 3 select two active-high rows sharing the upper-nibble columns.
	u8 data = 0;
	u8 const active_rows = m_soc->io().data_latch<0>() & direction;
	if (BIT(active_rows, 2))
		data |= m_matrix[0]->read();
	if (BIT(active_rows, 3))
		data |= m_matrix[1]->read();

	return data;
}

//-------------------------------------------------
//  EEPROM Connections
//-------------------------------------------------

void xavix_i2c_state::xavix_i2c_24c02(machine_config &config)
{
	xavix(config);
	I2C_24C02(config, "i2cmem");
}

void xavix_i2c_state::xavix_i2c_24c02_2mb(machine_config &config)
{
	xavix_i2c_24c02(config);
	m_soc->set_external_map(*this, &xavix_i2c_state::xavix_2mb_extbus_map);
}

void xavix_i2c_state::xavix_i2c_24c02_4mb(machine_config &config)
{
	xavix_i2c_24c02(config);
	m_soc->set_external_map(*this, &xavix_i2c_state::xavix_4mb_extbus_map);
}

void xavix_i2c_state::xavix_i2c_24c04(machine_config &config)
{
	xavix(config);
	I2C_24C04(config, "i2cmem"); // 24LC04 on Nostalgia games, 24C04 on others
}

void xavix_i2c_state::xavix_i2c_24c04_1mb(machine_config &config)
{
	xavix_i2c_24c04(config);
	m_soc->set_external_map(*this, &xavix_i2c_state::xavix_1mb_extbus_map);
}

void xavix_i2c_state::xavix_i2c_24c04_2mb(machine_config &config)
{
	xavix_i2c_24c04(config);
	m_soc->set_external_map(*this, &xavix_i2c_state::xavix_2mb_extbus_map);
}

void xavix_i2c_state::xavix_i2c_24c04_4mb(machine_config &config)
{
	xavix_i2c_24c04(config);
	m_soc->set_external_map(*this, &xavix_i2c_state::xavix_4mb_extbus_map);
}

void xavix_i2c_state::xavix_i2c_24c08(machine_config &config)
{
	xavix(config);
	I2C_24C08(config, "i2cmem");
}

void xavix_i2c_state::xavix_i2c_24c08_4mb(machine_config &config)
{
	xavix_i2c_24c08(config);
	m_soc->set_external_map(*this, &xavix_i2c_state::xavix_4mb_extbus_map);
}

void xavix_i2c_state::xavix_i2c_24c16(machine_config &config)
{
	xavix(config);
	I2C_24C16(config, "i2cmem");
}

void xavix_i2c_state::io1_output_w(u8 data, u8 direction)
{
	m_i2cmem->write_sda(BIT(direction, 3) ? BIT(data, 3) : 1);
	m_i2cmem->write_scl(BIT(direction, 4) ? BIT(data, 4) : 0);
}

//-------------------------------------------------
//  Cartridge Connection
//-------------------------------------------------

void xavix_cart_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x5fffff).rw(FUNC(xavix_cart_state::cart_r), FUNC(xavix_cart_state::cart_w));
	map(0x600000, 0x7fffff).rom().region("bios", 0x600000);
}

void xavix_cart_state::xavix_cart(machine_config &config)
{
	xavix(config);

	XAVIX_CART_SLOT(config, m_cartslot, xavix_cart_slot_device::cart_devices, nullptr);
}

int xavix_cart_state::cart_sda_r()
{
	if (m_cartslot->has_cart())
		return m_cartslot->read_sda();
	else
		return 0x1;
}

void xavix_i2c_cart_state::io1_output_w(u8 data, u8 direction)
{
	m_i2cmem->write_sda(BIT(direction, 3) ? BIT(data, 3) : 1);
	m_i2cmem->write_scl(BIT(direction, 4) ? BIT(data, 4) : 0);
}

//-------------------------------------------------
//  IR Connection
//-------------------------------------------------

void xavix_state::configure_ir(machine_config &config, required_device<xavix_ir_motion_device> &controllers, xavix_ir_motion_device::controller_type type)
{
	XAVIX_IR_MOTION(config, controllers, peripheral_clock()).set_controller_type(type);
	controllers->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(controllers, FUNC(xavix_ir_motion_device::vblank_w));
}

void xavix_ir_state::xavix_ir(machine_config &config)
{
	xavix(config);
	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::SHAKERS);
}

void xavix_ir_state::xavix_ir_1mb(machine_config &config)
{
	xavix_1mb(config);
	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::PADDLES);
}

void xavix_ir_state::xavixp_ir_1mb(machine_config &config)
{
	xavixp_1mb(config);
	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::PADDLES);
}

//-------------------------------------------------
//  Microphone Connection
//-------------------------------------------------

void xavix_state::configure_microphone(machine_config &config)
{
	// Software samples ADC channel 0 and implements microphone monitoring and effects through PCM voices.
	m_soc->adc().set_audio_channel(0);
	MICROPHONE(config, "microphone", 1).front_center().add_route(0, m_soc->adc(), 1.0);
}


//**************************************************************************
//  Shared Controls
//**************************************************************************

INPUT_PORTS_START( xavix )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("AN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN4")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN7")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MOUSE0X")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("MOUSE0Y")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("MOUSE1X")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("MOUSE1Y")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

INPUT_PORTS_START( xavix_i2c )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_ir )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_ir_motion_device::data_r))
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_battery_status )
	PORT_MODIFY("AN7")
	// Representative differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x2c, "Low" )
	PORT_CONFSETTING(    0x40, "Very Low" )
INPUT_PORTS_END

INPUT_PORTS_START( xavix_soft_power_control )
	PORT_START("POWER")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
INPUT_PORTS_END

INPUT_PORTS_START( xavix_hard_power_control )
	PORT_START("POWER")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_state::power_off), 0)
INPUT_PORTS_END

INPUT_PORTS_START( xavix_reset_control )
	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Reset") PORT_CODE(input_seq::end_code)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_state::reset_button), 0)
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_lightgun_1p )
	PORT_START("GUN1_0")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUN1_1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_lightgun_2p )
	PORT_INCLUDE(xavix_lightgun_1p)

	PORT_START("GUN2_0")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("GUN2_1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CODE_DEC(KEYCODE_E)
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_excite_controls )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Confirm (O)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel (X)")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

INPUT_PORTS_END

static INPUT_PORTS_START( xavix_excite_panel )
	PORT_INCLUDE(xavix_excite_controls)

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_excite_ir_sports )
	PORT_INCLUDE(xavix_excite_panel)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_ir_motion_device::data_r))
INPUT_PORTS_END

//**************************************************************************
//  Standalone Input Layouts
//**************************************************************************

// Play TV Boxin'
static INPUT_PORTS_START( rad_box )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// The receiver exposes six decoded punch lines and one combined Block line; Left/Right are console buttons.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Jab")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Left Hook")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Left Uppercut")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Right Jab")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Right Hook")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Right Uppercut")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Left Console Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Right Console Button")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Block")
	// This boot strap selects the parallel decoded receiver instead of the alternate raw infrared path.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch")
	PORT_BIT( 0xa7, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's three threshold bands; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x14, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Connectv Boxin'
static INPUT_PORTS_START( rad_boxp )
	PORT_INCLUDE(rad_box)
INPUT_PORTS_END


// Play TV Snowboarder (Blue)
static INPUT_PORTS_START( rad_snow )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Tilt Forward") PORT_16WAY
	// Bit 1 is scanned and timed, but its result is not consumed and no physical control is documented.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Tilt Back / Enter") PORT_16WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Tilt Left") PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Tilt Right") PORT_16WAY
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Connectv Snowboarder (Blue)
static INPUT_PORTS_START( rad_snowp )
	PORT_INCLUDE(rad_snow)
INPUT_PORTS_END


// Play TV SSX Snowboarder
static INPUT_PORTS_START( rad_ssx )
	PORT_INCLUDE(rad_snow)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The base buttons duplicate the board tilt switches electrically.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Tilt Left / Left Button") PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Tilt Right / Right Button") PORT_16WAY

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x28, "Low" )
	PORT_CONFSETTING(    0x30, "Very Low" )
INPUT_PORTS_END


// Connectv SSX Snowboarder
static INPUT_PORTS_START( rad_ssxp )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Tilt Forward") PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Tilt Back / Enter") PORT_16WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Tilt Left") PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Tilt Right") PORT_16WAY
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_reset_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The base buttons duplicate the board tilt switches electrically.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Tilt Left / Left Button") PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Tilt Right / Right Button") PORT_16WAY

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x28, "Low" )
	PORT_CONFSETTING(    0x30, "Very Low" )
INPUT_PORTS_END


// Play TV Snowboarder (White)
static INPUT_PORTS_START( rad_sbw )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Tilt Forward") PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Tilt Back / Enter") PORT_16WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Tilt Left") PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Tilt Right") PORT_16WAY
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_reset_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The base buttons duplicate the board tilt switches electrically.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Tilt Left / Left Button") PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Tilt Right / Right Button") PORT_16WAY

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x28, "Low" )
	PORT_CONFSETTING(    0x30, "Very Low" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Tilt Back / Enter Button") PORT_16WAY
INPUT_PORTS_END


// Bakushin Sno-Bo: Gingin Boarders
static INPUT_PORTS_START( tak_gin )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// Four independent board contacts permit the compound directions used for tricks.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Tilt Forward / Cancel") PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Tilt Back / OK") PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Tilt Left / Previous") PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Tilt Right / Next") PORT_16WAY
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch")
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// The blink-gated status strongly implies battery monitoring; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x2f, "Low" )
	PORT_CONFSETTING(    0x40, "Very Low" )
INPUT_PORTS_END


// Geigeki Go Go Shooting
static INPUT_PORTS_START( tak_geig )
	PORT_INCLUDE(xavix)
	PORT_INCLUDE(xavix_soft_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Main Gun / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Bomb")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Pause")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Main Gun / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Bomb")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Pause")
	// This bit is directly polled by inactivity and factory paths, but its physical function is unknown.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("AN7")
	// Representative values select the ROM's three threshold bands; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x18, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// mini-moni. Stage! Dance Dapyon! / Plus
static INPUT_PORTS_START( epo_mms )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Pink / Select") PORT_CODE(KEYCODE_A) PORT_CODE(MOUSECODE_BUTTON3_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON2_INDEXED(0))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Green / Confirm") PORT_CODE(KEYCODE_S) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(0))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Purple / Select") PORT_CODE(KEYCODE_D) PORT_CODE(MOUSECODE_BUTTON3_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON2_INDEXED(1))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Orange / Confirm") PORT_CODE(KEYCODE_F) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(1))
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_UNUSED )
	// This unknown boot strap reaches display/factory paths and overlaps some internal effects of the auto-off-disable chord.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's three warning states; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x20, "Low" )
	PORT_CONFSETTING(    0x28, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Poko Poko Hammers
static INPUT_PORTS_START( tak_hamr )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_soft_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Player 1 - Green / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Player 1 - Yellow / Select")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Player 1 - Red / Cancel")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Player 2 - Green / Confirm")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Player 2 - Yellow / Select")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Player 2 - Red / Cancel")
	// The two console buttons select slower/normal/faster entries in the ROM's timing table.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Tempo Slower")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Tempo Faster")

	PORT_MODIFY("AN7")
	// Representative values select the ROM's three threshold bands; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x20, "Low" )
	PORT_CONFSETTING(    0x24, "Very Low" )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// TV de Asobitai Tsuukai! Goraku Katsugeki: Champiyon Pinball
static INPUT_PORTS_START( tomcpin )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_soft_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Right Flipper / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Left Flipper / Select")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Plunger")
	// Each Furikon's shake switch supplies a digital nudge edge; no direction or magnitude is reported.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Nudge / Shake Controller")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Right Flipper")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Left Flipper")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Plunger")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Nudge / Shake Controller")

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf5, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x02, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x02, "AC Adapter" )

	PORT_MODIFY("AN7")
	// Representative ADC differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1c, "Low" )
	PORT_CONFSETTING(    0x24, "Very Low" )
INPUT_PORTS_END


// Tokyo Friend Park II
// The six mat contacts are independent buttons; several activities require simultaneous contacts.
static INPUT_PORTS_START( epo_tfp2 )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	// Original-pad colour names follow period photography and may vary with lighting.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Left / Red") PORT_PLAYER(1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Right / Blue") PORT_PLAYER(1) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Confirm / Orange") PORT_PLAYER(1) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Left / Green") PORT_PLAYER(2) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Right / Pink") PORT_PLAYER(2) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Confirm / Purple") PORT_PLAYER(2) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative values select the ROM's three warning states; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x10, "Low" )
	PORT_CONFSETTING(    0x14, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END


// Tokyo Friend Park II Special!
static INPUT_PORTS_START( epo_tp2s )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Red Mat Left") PORT_PLAYER(1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Red Mat Right") PORT_PLAYER(1) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Red Mat Confirm / Orange") PORT_PLAYER(1) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Blue Mat Left") PORT_PLAYER(2) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Blue Mat Right") PORT_PLAYER(2) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Blue Mat Confirm / Green") PORT_PLAYER(2) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_CONFNAME( 0x01, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x01, "AC Adapter" )
	// This boot strap reaches an otherwise dormant internal path; its physical function is unknown.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xee, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative values select the ROM's three warning states; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x10, "Low" )
	PORT_CONFSETTING(    0x14, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END


// Tokyo Friend Park II Perfect!
static INPUT_PORTS_START( epo_tp2p )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Side Left") PORT_PLAYER(1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Left Side Right") PORT_PLAYER(1) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Left Side Confirm") PORT_PLAYER(1) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Right Side Left") PORT_PLAYER(2) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Side Right") PORT_PLAYER(2) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Right Side Confirm") PORT_PLAYER(2) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_CONFNAME( 0x01, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x01, "AC Adapter" )
	// This boot strap reaches an otherwise dormant internal path; its physical function is unknown.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xbe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative values select the ROM's three warning states; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1c, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END


// Play TV Jr. Construction
static INPUT_PORTS_START( rad_jcon )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Horn / Start") // selects game
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel / Back")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Lever Up") // for rock crushing game etc.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Lever Down")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Handle Up") // for crane fishing game etc.
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Handle Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Wheel Right") // for menu, crane construction etc.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Wheel Left")

	PORT_MODIFY("IN1")
	// The back-and-forth shifter reaches the ROM as one edge-triggered switch, not two maintained gear positions.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Gear Shift")
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	// The quarter-turn ignition key interrupts power outside the nine-contact gameplay scanner.
	PORT_INCLUDE(xavix_hard_power_control)
	PORT_MODIFY("POWER")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Ignition Key")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_state::power_off), 0)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Iroiro Unten: Kouji no Kuruma 5
static INPUT_PORTS_START( tak_town )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Confirm / Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel / Back")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Shovel Lever Up")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Shovel Lever Down")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Crane Lever Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Crane Lever Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Wheel Right")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Wheel Left")

	PORT_MODIFY("IN1")
	// As in the US version, one press represents a complete forward/back drive-lever stroke.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Drive Lever")
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END


// TV de Asobitai Tomica Carnavi Drive
static INPUT_PORTS_START( tcarnavi )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Accelerator Lever Up / Drive")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Accelerator Lever Down / Stop")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Back")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Ignition Start")
	// The maintained contact accompanies the momentary Start contact; the exact key-switch topology is inferred.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Ignition On / Run") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Steer Left") PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Steer Right") PORT_2WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Lights")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Horn")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Wipers")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Car Navi")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The adapter documentation and distinct inactivity period identify this line as the power-source sense.
	PORT_CONFNAME( 0x20, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x20, "AC Adapter" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_battery_status)
INPUT_PORTS_END


// Let's! TV Play Classic: Taito / Namco Nostalgia
static INPUT_PORTS_START( nostalgia )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0") // mappings based on Dragon Buster button list
	// Numbered action buttons are remapped per game; 4/3 also Confirm/Back in menus.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // Not on the documented panel; ROM prompt/test code and demo streams can use bit 2.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Credit / Pause") PORT_CODE(KEYCODE_5) // Credit button, not a coin slot.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END


// Barbie Dance Party
static INPUT_PORTS_START( rad_bdp )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// Independent flower contacts permit the two-pad combinations used for Double moves.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Purple / Up") PORT_CODE(KEYCODE_A) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(0))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Red / Down") PORT_CODE(KEYCODE_S) PORT_CODE(MOUSECODE_BUTTON3_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON2_INDEXED(0))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Blue / Back") PORT_CODE(KEYCODE_D) PORT_CODE(MOUSECODE_BUTTON2_INDEXED(0))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Pink / Enter") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch")
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's three threshold bands; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x14, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Play TV Card Night
static INPUT_PORTS_START( rad_crdn )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW0")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Enter")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("New Game")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Sound")

	PORT_START("ROW1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Dock Remote / Power Off")
	PORT_BIT( 0xef, IP_ACTIVE_HIGH, IPT_UNUSED )

	// The base Reset restarts the game; the remote's separate recessed Reset only restarts its transmitter.
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Connectv Card Night
static INPUT_PORTS_START( rad_crdnp )
	PORT_INCLUDE(rad_crdn)
INPUT_PORTS_END


// Play TV Opus / Plug & Play Opus
static INPUT_PORTS_START( rad_opus )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0") // comments show how inputs labeled on PCB
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Trigger Button L")  PORT_CODE(KEYCODE_A) // PI00
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Trigger Button LM") PORT_CODE(KEYCODE_S) // PI01
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Trigger Button RM") PORT_CODE(KEYCODE_D) // PI02
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Trigger Button R")  PORT_CODE(KEYCODE_F) // PI03
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Pause") PORT_CODE(KEYCODE_P) // PI05
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Menu Select") // PI06
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start / Smart Bomb") // PI07

	PORT_MODIFY("IN1")
	// The momentary button is debounced on release; firmware then drives IO1.7 high to cut power.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Connectv Opus
static INPUT_PORTS_START( rad_opusp )
	PORT_INCLUDE(rad_opus)
INPUT_PORTS_END


// Let's! TV Play Keroro Shoutai Dai Panic! Dotabata Daikessen de Arimasu
static INPUT_PORTS_START( ban_krrj )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// The four mat contacts are independent so multiplayer games can accept simultaneous steps.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("X") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Left Arrow") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Right Arrow") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("O") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	// The documented battery/AC alternatives select the ROM's distinct inactivity timeouts.
	PORT_CONFNAME( 0x40, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x40, "AC Adapter" )
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative values select the ROM's three supply states; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1c, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )

	// The recessed top-shell pinhole is inferred to be Reset; no legible label or manual diagram is available.
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

//**************************************************************************
//  System Implementations
//**************************************************************************

//-------------------------------------------------
//  Systems Sharing Lightgun Inputs
//-------------------------------------------------

void xavix_state::configure_lightgun_scanlines(machine_config &config, u8 event_mask)
{
	m_lightgun_event_mask = event_mask;
	TIMER(config, "lightgun_timer").configure_scanline(FUNC(xavix_state::lightgun_scanline_cb), "screen", 53, 10);
}

u8 xavix_state::lightgun_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IO, "%s: unhandled lightgun_r %d\n", machine().describe_context(), offset);
	return 0xff;
}

u8 xavix_state::lightgun_position_r(unsigned player, offs_t offset, u16 x_offset, u16 y_scale, u16 y_offset, u16 x_scale)
{
	if (player > 1)
		return 0xff;

	u16 const rawx = player ? m_lightgun2[0].read_safe(0x80) : m_lightgun1[0].read_safe(0x80);
	u16 const gunx = ((rawx * x_scale) >> 8) + x_offset;
	u16 const rawy = player ? m_lightgun2[1].read_safe(0x80) : m_lightgun1[1].read_safe(0x80);
	u16 const guny = ((rawy * y_scale) >> 8) + y_offset;
	switch (offset & 3)
	{
	case 0:
		return gunx;
	case 1:
		return gunx >> 8;
	case 2:
		return guny;
	default:
		return 0;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(xavix_state::lightgun_scanline_cb)
{
	// These NTSC boards place picture row zero at raster row 13.
	int const line = param - 13;
	// Spread eight optical samples across the field so each event can be acknowledged before the next arrives.
	if ((line >= 40) && (line <= 110) && ((line % 10) == 0))
	{
		if (BIT(m_lightgun_event_mask, 0))
			m_soc->io().event_w<0>(1);
		if (BIT(m_lightgun_event_mask, 1))
			m_soc->io().event_w<1>(1);
		if (BIT(m_lightgun_event_mask, 2))
			m_soc->io().event_w<2>(1);
		if (BIT(m_lightgun_event_mask, 3))
			m_soc->io().event_w<3>(1);
	}
}


// Gun Gun Adventure
static INPUT_PORTS_START( gungunad )
	PORT_INCLUDE(xavix)
	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_reset_control)
	PORT_INCLUDE(xavix_lightgun_2p)

	PORT_MODIFY("IN0")
	// IO0.0 reaches a dormant P1 action path, but no additional physical control is documented.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Trigger") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Trigger") PORT_PLAYER(2) PORT_CONDITION("IN0", 0x40, EQUALS, 0x40)
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	// This debounced line gates P2 activity; its identity as the detachable gun-present contact is inferred.
	PORT_CONFNAME( 0x40, 0x00, "P2 Gun" )
	PORT_CONFSETTING(    0x00, "Disconnected" )
	PORT_CONFSETTING(    0x40, "Connected" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	// This line changes the inactivity policy, but no AC adapter is documented for this model.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0xbb, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("GUN2_0")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x40, EQUALS, 0x40)

	PORT_MODIFY("GUN2_1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CODE_DEC(KEYCODE_E) PORT_CONDITION("IN0", 0x40, EQUALS, 0x40)
INPUT_PORTS_END

void xavix_gungunad_state::xavix_gungunad(machine_config &config)
{
	xavix_nv_2mb(config);
	configure_lightgun_scanlines(config, 0x03);
}

u8 xavix_gungunad_state::lightgun_r(offs_t offset)
{
	// The ROM subtracts 17 from X and indexes Y against visible picture rows.
	return lightgun_position_r(offset >> 2, offset, 0x11, 0xe0, 0x24);
}


// Gun Gun Revolution
static INPUT_PORTS_START( gungunrv )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
	PORT_INCLUDE(xavix_lightgun_2p)

	PORT_MODIFY("IN0")
	// The trigger is mirrored to a probable second pole that qualifies the vibration motor.
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Machine Gun Trigger / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Support Gun Trigger") PORT_PLAYER(2) PORT_CONDITION("IN1", 0x02, EQUALS, 0x02)
	// The disconnected trigger level is inferred from the ROM's inactivity check.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CONDITION("IN1", 0x02, EQUALS, 0x00)
	// Startup treats IO0.6 low as a factory-test strap when the Support Gun is absent.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xb4, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	// The ROM debounces this contact before accepting shots from the optional Support Gun.
	PORT_CONFNAME( 0x02, 0x00, "P2 Gun" )
	PORT_CONFSETTING(    0x00, "Disconnected" )
	PORT_CONFSETTING(    0x02, "Connected" )
	PORT_CONFNAME( 0x04, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x04, "AC Adapter" )
	// The physical three-position switch combines hard Off with these two powered settings.
	PORT_CONFNAME( 0x40, 0x00, "Vibration" )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFSETTING(    0x40, DEF_STR( Off ) )
	PORT_BIT( 0xb1, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's narrow three-state battery classifier.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x28, "Low" )
	PORT_CONFSETTING(    0x30, "Very Low" )

	PORT_MODIFY("GUN2_0")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("IN1", 0x02, EQUALS, 0x02)

	PORT_MODIFY("GUN2_1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CODE_DEC(KEYCODE_E) PORT_CONDITION("IN1", 0x02, EQUALS, 0x02)
INPUT_PORTS_END

void xavix_gungunrv_state::xavix_gungunrv(machine_config &config)
{
	xavix_i2c_24c04_4mb(config);
	configure_lightgun_scanlines(config, 0x03);
}

void xavix_gungunrv_state::machine_reset()
{
	xavix_i2c_state::machine_reset();
	m_gun_recoil = 0;
}

void xavix_gungunrv_state::io0_output_w(u8 data, u8 direction)
{
	// IO0.4 drives the main gun's vibration motor; the Support Gun has no separate software drive.
	m_gun_recoil = BIT(data & direction, 4);
}

void xavix_gungunrv_state::io1_output_w(u8 data, u8 direction)
{
	xavix_i2c_state::io1_output_w(data, direction);
	if (BIT(data & direction, 7) && !machine().side_effects_disabled())
	{
		// Startup tests each output with an $ff data latch; any other IO1.7 assertion requests shutdown.
		if (m_soc->io().data_latch<1>() != 0xff)
			machine().schedule_exit();
	}
}

u8 xavix_gungunrv_state::lightgun_r(offs_t offset)
{
	// The steady-state capture path doubles X; hit testing indexes Y against visible picture rows.
	return lightgun_position_r(offset >> 2, offset, 0x3b, 0xe0, 0x24, 0x80);
}


// Play TV Buckmasters Huntin' / Huntin' 2
static INPUT_PORTS_START( rad_hnt )
	PORT_INCLUDE(xavix)
	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_reset_control)
	PORT_INCLUDE(xavix_lightgun_1p)

	PORT_MODIFY("IN0")
	// Keep paging buttons separate from the direction pad and keyboard aiming.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu Next") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu Previous") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Menu Select")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Pause / Menu")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Trigger")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Safety") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0xba, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_rad_hnt_state::xavix_rad_hnt(machine_config &config)
{
	xavix_nv_1mb(config);
}

void xavix_rad_hnt_state::xavix_rad_hnt2(machine_config &config)
{
	xavix_nv_2mb(config);
}

void xavix_rad_hnt_state::vblank_extra()
{
	// The hunting rifle supplies one accepted optical capture on event 3 per field.
	m_soc->io().event_w<3>(1);
}

u8 xavix_rad_hnt_state::lightgun_r(offs_t offset)
{
	// The nominal mapping omits the first revision's still-unverified factory calibration trim.
	return lightgun_position_r(0, offset, 0x20, 0xe0, 0x1a);
}


// TV de Asobitai Kindai Uchimakuri Variety Shateking
static INPUT_PORTS_START( tomshoot )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_soft_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pull / Charge") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Push / Shoot") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Pull / Charge") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Push / Shoot") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Factory/demo strap with no identified public control.
	PORT_BIT( 0xd0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x37, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The selected chip version routes Power through $7ff8, leaving the generic IO1.6 input path dead.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	// A dormant factory monitor polls IO1.7, but its physical input-side function is unknown.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("AN7")
	// Representative values select the ROM's three threshold bands; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1f, "Low" )
	PORT_CONFSETTING(    0x24, "Very Low" )

	PORT_INCLUDE(xavix_lightgun_2p)
INPUT_PORTS_END

void xavix_tomshoot_state::vblank_extra()
{
	// Approximate each controller's beam-timed optical capture with one event token per field.
	m_soc->io().event_w<0>(1);
	m_soc->io().event_w<1>(1);
}

u8 xavix_tomshoot_state::lightgun_r(offs_t offset)
{
	unsigned const player = offset >> 2;
	// The capture bias and 224-line Y range centre both guns on the native mode-selection targets.
	return lightgun_position_r(player, offset, 0x1b, 0xe0, 0x10);
}


// Doraemon-goe de Dokan! Wakuwaku Kuukihou!!
static INPUT_PORTS_START( epo_dmon )
	PORT_INCLUDE(xavix_i2c)
	// The primary air cannon has an underside latching power switch; inactivity uses the SoC's separate shutdown output.
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Air Cannon Trigger") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Air Cannon Trigger") PORT_PLAYER(2)

	// Usability hack for users without host microphones.
	PORT_START("MIC_SIMULATE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Simulate Voice") PORT_PLAYER(1) PORT_CODE(KEYCODE_N) PORT_WRITE_LINE_DEVICE_MEMBER("microphone_tone1", FUNC(beep_device::set_state))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Simulate Voice") PORT_PLAYER(2) PORT_CODE(KEYCODE_M) PORT_WRITE_LINE_DEVICE_MEMBER("microphone_tone2", FUNC(beep_device::set_state))

	PORT_MODIFY("AN7")
	// Representative values select the ROM's three threshold bands; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1a, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )

	PORT_INCLUDE(xavix_lightgun_2p)
INPUT_PORTS_END

void xavix_epo_dmon_state::xavix_epo_dmon(machine_config &config)
{
	xavix_i2c_24c02_4mb(config);

	// Each tethered air cannon has its own microphone: P1 uses AN0 and P2 uses AN1.
	m_soc->adc().set_audio_input(0, 0);
	m_soc->adc().set_audio_input(1, 1);
	MICROPHONE(config, "microphone1", 1).front_center().add_route(0, m_soc->adc(), 1.0, 0);
	MICROPHONE(config, "microphone2", 1).front_center().add_route(0, m_soc->adc(), 1.0, 1);
	BEEP(config, "microphone_tone1", 1000).add_route(0, m_soc->adc(), 0.5, 0);
	BEEP(config, "microphone_tone2", 1000).add_route(0, m_soc->adc(), 0.5, 1);
}

void xavix_epo_dmon_state::vblank_extra()
{
	// Approximate each controller's beam-timed optical capture with one event token per field.
	m_soc->io().event_w<0>(1);
	m_soc->io().event_w<1>(1);
}

u8 xavix_epo_dmon_state::lightgun_r(offs_t offset)
{
	unsigned const player = offset >> 2;
	// Exact phototransistor-to-raster calibration is unknown; the ROM treats $80 as the centre of both axes.
	return lightgun_position_r(player, offset);
}


// Video Challenger (PL-1000)
static INPUT_PORTS_START( pl1000 )
	PORT_INCLUDE(xavix)
	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
	PORT_INCLUDE(xavix_lightgun_2p)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Trigger") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Trigger") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x99, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_pl1000_state::xavix_pl1000(machine_config &config)
{
	xavix_nv_2mb(config);
	configure_lightgun_scanlines(config, 0x08);
}

void xavix_pl1000_state::init_pl1000()
{
	// this could be the earliest XaviX SoC as it doesn't seem to have an A15 line, meaning we need
	// to mirror the ROM data so that everything appears in the correct place
	u8 *plainrom = memregion("plainrom")->base();
	for (offs_t i = 0; i < m_rom.bytes(); i++)
	{
		u32 addr = bitswap<20>(i, 20, 19, 18, 17, 16, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
		m_rom[i] = plainrom[addr];
	}
}

void xavix_pl1000_state::machine_start()
{
	xavix_state::machine_start();
	save_item(NAME(m_io1_out));
}

void xavix_pl1000_state::machine_reset()
{
	xavix_state::machine_reset();
	m_io1_out = 0x00;
}

u8 xavix_pl1000_state::lightgun_r(offs_t offset)
{
	// IO1.7 multiplexes the two tethered guns on alternate capture passes.
	return lightgun_position_r(BIT(m_io1_out, 7), offset, 0x20, 0xe0, 0x20);
}

//-------------------------------------------------
//  Systems Sharing IR Reports
//-------------------------------------------------

// Goal Kimeruze! Excite Striker / Let's Play Excite Striker / Play TV Soccer
static INPUT_PORTS_START( epo_strk )
	PORT_INCLUDE(xavix_excite_ir_sports)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_ir_motion_device::data_r))
	// Shared firmware tests this auto-off qualifier, but documented Japanese and US hardware is battery-only.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( epo_strkp )
	PORT_INCLUDE(xavix_excite_ir_sports)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_ir_motion_device::data_r))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	// The UK supply options are undocumented, so retain the auto-off qualifier without naming its source.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

void xavix_ir_state::xavix_epo_strk(machine_config &config)
{
	xavix(config);
	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::KICK_SENSORS);
}

void xavix_ir_state::xavixp_epo_strk(machine_config &config)
{
	xavixp(config);
	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::KICK_SENSORS);
}


// Ace Kimeruze! Excite Tennis
static INPUT_PORTS_START( epo_tenn )
	PORT_INCLUDE(xavix_excite_ir_sports)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_ir_motion_device::data_r))
	// The documented adapter and this line's verified auto-off behaviour identify the source selector.
	PORT_CONFNAME( 0x02, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x02, "AC Adapter" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_battery_status)

	PORT_MODIFY("AN7")
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x28, "Low" )
	PORT_CONFSETTING(    0x40, "Very Low" )
INPUT_PORTS_END

void xavix_ir_state::xavix_epo_tenn(machine_config &config)
{
	xavix(config);
	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::TENNIS_RACKETS);
}


// Shoubu Shiyouze! Excite Stadium DX / Hanshin Tigers Excite Stadium DX
static INPUT_PORTS_START( epo_esdx )
	PORT_INCLUDE(xavix_excite_panel)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_esdx_controllers_device::data_r))
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// This classifier needs smaller representative differences than the shared battery status control.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1c, "Low" )
	PORT_CONFSETTING(    0x24, "Very Low" )
INPUT_PORTS_END

static INPUT_PORTS_START( epo_esht )
	PORT_INCLUDE(xavix_excite_panel)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_esdx_controllers_device::data_r))
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// This classifier needs smaller representative differences than the shared battery status control.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1c, "Low" )
	PORT_CONFSETTING(    0x24, "Very Low" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_esdx_controllers_device::data_r))
	// The documented adapter and this line's verified auto-off behaviour identify the source selector.
	PORT_CONFNAME( 0x02, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x02, "AC Adapter" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_epo_baseball_state::configure_controllers(machine_config &config)
{
	XAVIX_ESDX_CONTROLLERS(config, m_controllers, peripheral_clock());
	m_controllers->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_controllers, FUNC(xavix_esdx_controllers_device::vblank_w));
}

void xavix_epo_baseball_state::xavix_epo_esdx(machine_config &config)
{
	xavix_4mb(config);
	configure_controllers(config);
}

void xavix_epo_baseball_state::xavix_epo_esht(machine_config &config)
{
	xavix_nv_4mb(config);
	configure_controllers(config);
}


// Home Run Datouze! Excite Stadium
static INPUT_PORTS_START( epo_stad )
	PORT_INCLUDE(xavix_excite_controls)

	// Supply-side power control; its physical switch/latch wiring is unverified.
	PORT_INCLUDE(xavix_hard_power_control)
	PORT_MODIFY("POWER")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_state::power_off), 0)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_stadium_bat_device::data_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // This changes timeout pacing, but its physical source is unknown.
	PORT_BIT( 0xbe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_epo_stad_state::xavix_epo_stad(machine_config &config)
{
	xavix_2mb(config);

	XAVIX_STADIUM_BAT(config, m_controllers, peripheral_clock());
	m_controllers->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_controllers, FUNC(xavix_stadium_bat_device::vblank_w));
}


// Comet-san Lovelin Baton
static INPUT_PORTS_START( tak_comt )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED ) // Bit 2 enables demonstration mode; its physical connection is unconfirmed.
	// Split the single slider's mode and Off contacts so a saved Off setting cannot prevent the next boot.
	PORT_CONFNAME( 0x20, 0x20, "Station Switch / Mode" )
	PORT_CONFSETTING(    0x20, "Game" )
	PORT_CONFSETTING(    0x00, "TV" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Station Switch / Off")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_reset_control) // The recessed Reset button's electrical connection is unconfirmed.
INPUT_PORTS_END

void xavix_tak_comt_state::xavix_tak_comt(machine_config &config)
{
	xavix_nv_2mb(config);

	// The station recognises TV musical cues through AN0; see Takara patent JP2003019364A.
	m_soc->adc().set_audio_channel(0);
	MICROPHONE(config, "station_microphone", 1).map_on_request_only().add_route(0, m_soc->adc(), 1.0);

	XAVIX_COMET_BATON(config, m_baton_controller, peripheral_clock());
	m_baton_controller->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_soc->io().output_callback<1>().append(m_baton_controller, FUNC(xavix_comet_baton_device::station_w));
	m_screen->screen_vblank().append(m_baton_controller, FUNC(xavix_comet_baton_device::vblank_w));
}

u8 xavix_tak_comt_state::io1_input_r(u8 direction)
{
	u8 const data = xavix_power_state::io1_input_r(direction);
	// The slider's Off position opens the Game contact.
	return BIT(data, 6) ? (data & ~0x20) : data;
}


// Webdiver DX W-05 Gladion
static INPUT_PORTS_START( tak_wdg )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Confirm / Shot / Whistle") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Move Right Arm / Charge") PORT_PLAYER(1)
	// The attachment contact cannot identify the Web Knight; its IR report supplies the identity.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Web Knight Attached") PORT_TOGGLE PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // output
	// Independent contacts allow the opposing Left+Right factory chord.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_16WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // factory-test strap
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Head (Vehicle Position)") PORT_TOGGLE PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Lower Body / Legs (Vehicle Position)") PORT_TOGGLE PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Left Arm (Vehicle Position)") PORT_TOGGLE PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Right Arm (Vehicle Position)") PORT_TOGGLE PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // shutdown output

	PORT_MODIFY("AN7")
	// Representative ADC values select the ROM's bands; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x12, "Low" )
	PORT_CONFSETTING(    0x1c, "Very Low" ) // Select live after Low to cross the ROM's falling threshold.
INPUT_PORTS_END

void xavix_tak_wdg_state::xavix_tak_wdg(machine_config &config)
{
	xavix_nv_2mb(config);

	XAVIX_GRANBLADE(config, m_granblade, peripheral_clock());
	m_granblade->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_granblade, FUNC(xavix_granblade_device::vblank_w));

	XAVIX_WEBKNIGHT(config, m_webknight, peripheral_clock());
	// These callbacks deliver receiver-edge events, not maintained signal levels.
	m_webknight->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_webknight, FUNC(xavix_webknight_device::vblank_w));
}


static INPUT_PORTS_START( epo_epp_ir )
	PORT_INCLUDE(xavix_ir)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("O")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("X")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


// Excite Ping Pong / Real Ping Pong / Shamoji de Ping Pong
static INPUT_PORTS_START( epo_epp1 )
	PORT_INCLUDE(epo_epp_ir)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Button")

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_ir_state::xavix_epo_epp(machine_config &config)
{
	xavix_1mb(config);
	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::PING_PONG_RACKETS);
}


// Takkyuu Yarouze! Excite Ping Pong 2
static INPUT_PORTS_START( epo_epp2 )
	PORT_INCLUDE(epo_epp_ir)

	PORT_MODIFY("IN1")
	// IN1.1 bypasses battery auto-off; its identification as AC-adapter sense is inferred.
	PORT_CONFNAME( 0x02, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x02, "AC Adapter" )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x28, "Low" )
	PORT_CONFSETTING(    0x30, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_ir_state::xavix_epo_epp2(machine_config &config)
{
	xavix_2mb(config);
	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::POWERED_PING_PONG_RACKETS);
}


// Ai-chan ni Chousen! Excite Ping Pong
static INPUT_PORTS_START( epo_epp3 )
	PORT_INCLUDE(epo_epp_ir)

	PORT_MODIFY("IN1")
	// Ping Pong 2 uses IN1.1 to bypass battery auto-off; its AC-adapter sense is inferred.
	PORT_CONFNAME( 0x02, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x02, "AC Adapter" )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x28, "Low" )
	PORT_CONFSETTING(    0x30, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Y")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	// This hardware sense selects the documented 15-minute battery or 16-hour AC auto-off period.
	PORT_CONFNAME( 0x40, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x40, "AC Adapter" )
INPUT_PORTS_END


// Play TV Ping Pong
static INPUT_PORTS_START( rad_ping )
	PORT_INCLUDE(xavix_ir)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Enter")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Pause")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Restart")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x3e, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The momentary button is debounced on release before the firmware drives IO1.7 high to cut power.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative values select the ROM's warning threshold; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x80, "Low" )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Connectv Table Tennis
static INPUT_PORTS_START( rad_pingp )
	PORT_INCLUDE(rad_ping)
INPUT_PORTS_END


// Ham-chans Dai Shuugou: Dance Surunoda! Hashirunoda!
static INPUT_PORTS_START( epo_hamd )
	PORT_INCLUDE(xavix_ir)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("O")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("X")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Croket! Itada Kinka! Banker Battle!!
static INPUT_PORTS_START( epo_crok )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	// The enclosure button is unlabelled; this name follows its confirmed title/menu role.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Main Unit Button")
	// IN0.4 and IN1.1 enter unresolved power/test paths but are not fitted user controls.
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_croket_controllers_device::data_r))
	PORT_BIT( 0xf6, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BANKER")
	PORT_CONFNAME( 0x01, 0x00, "Battle Code Banker Connection" )
	PORT_CONFSETTING(    0x00, "Disconnected" )
	PORT_CONFSETTING(    0x01, "Connected" )

	// The optional AC adapter has no identified firmware-visible source selector.
	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void xavix_epo_crok_state::xavix_epo_crok(machine_config &config)
{
	// The 24C04 is retained storage in the optional Battle Code Banker accessory.
	xavix_i2c_24c04_4mb(config);

	XAVIX_CROKET_CONTROLLERS(config, m_controllers, peripheral_clock());
	m_controllers->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_controllers, FUNC(xavix_croket_controllers_device::vblank_w));
}

u8 xavix_epo_crok_state::io1_input_r(u8 direction)
{
	u8 const data = xavix_i2c_power_state::io1_input_r(direction);
	// The separately sold Banker releases SDA when its communication cable is disconnected.
	return m_banker->read() ? data : data | 0x08;
}

void xavix_epo_crok_state::io1_output_w(u8 data, u8 direction)
{
	if (m_banker->read())
		xavix_i2c_power_state::io1_output_w(data, direction);
	else
		m_i2cmem->reset_bus();
}


// Zuba Zuba Blade
static INPUT_PORTS_START( tak_zuba )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_soft_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_ir_motion_device::data_r))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Physical status input; its effect on the inactivity path is not identified.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The receiver strap selects the report table shared with the other XaviX motion controllers.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's three classifier states; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x16, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )
INPUT_PORTS_END

void xavix_tak_zuba_state::xavix_tak_zuba(machine_config &config)
{
	xavix_i2c_24c02_4mb(config);

	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::SWORDS);
}


//-------------------------------------------------
//  Systems Sharing Baseball Devices
//-------------------------------------------------

xavix_baseball_ball_device &xavix_state::configure_baseball(machine_config &config)
{
	xavix_baseball_ball_device &ball(XAVIX_BASEBALL_BALL(config, "pitch_ball"));
	m_soc->adc().input_callback<0>().set(ball, FUNC(xavix_baseball_ball_device::vertical_buttons_r));
	m_soc->adc().input_callback<1>().set(ball, FUNC(xavix_baseball_ball_device::horizontal_buttons_r));
	m_soc->adc().input_callback<4>().set(ball, FUNC(xavix_baseball_ball_device::motion_r));
	m_screen->screen_vblank().append(ball, FUNC(xavix_baseball_ball_device::vblank_w));
	// AN5 and AN7 are equal at the normal setting; their difference is the low-battery monitor, not a motion axis.

	xavix_baseball_bat_device &bat(XAVIX_BASEBALL_BAT(config, "bat"));
	bat.set_screen(m_screen);
	bat.event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(bat, FUNC(xavix_baseball_bat_device::vblank_w));
	return ball;
}


// Play TV Baseball
static INPUT_PORTS_START( rad_bb )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Console Enter")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Restart")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pause")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Console Up") PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Console Down") PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Batter")
	// The momentary button is debounced on release before the firmware drives IO1.7 high to cut power.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x9f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_state::xavix_rad_bb(machine_config &config)
{
	xavix_1mb(config);
	configure_baseball(config);
}


// Play TV Baseball 2
static INPUT_PORTS_START( rad_bb2 )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Console Down") PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Console Up") PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Console Right") PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Console Left") PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Console X")
	// The ROM aliases bit 0x10 to X, but the photographed control panel has no corresponding switch.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Console O")
	// The momentary button is debounced on release before the firmware drives IO1.7 high to cut power.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x87, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x16, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_state::xavix_rad_bb2(machine_config &config)
{
	xavix_2mb(config);
	configure_baseball(config).set_model(xavix_baseball_ball_device::model::RAD_BB2);
}

//-------------------------------------------------
//  Systems Sharing the Reel Input
//-------------------------------------------------

int xavix_state::fishing_reel_r()
{
	return BIT(m_reel->read(), 0);
}


// Black Bass Tsurouze: Excite Fishing
static INPUT_PORTS_START( epo_fish )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel")
	// Fishing controllers expose one reel phase here; their ROMs count its transitions.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_state::fishing_reel_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN0")
	// Keep casting separate from the Confirm button's Control key.
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_NAME("Rod Swing Strength") PORT_MINMAX(0x00, 0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(8) PORT_CENTERDELTA(8) PORT_CODE_INC(KEYCODE_SPACE)

	PORT_START("REEL")
	// Keep reel movement separate from the direction pad.
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Reel Handle") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_T) PORT_CODE_INC(KEYCODE_Y)

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Dai Mono Tsurouze! Excite Fishing DX
static INPUT_PORTS_START( epo_efdx )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel")
	// Fishing controllers expose one reel phase here; their ROMs count its transitions.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_state::fishing_reel_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN0")
	// Keep casting separate from the Confirm button's Control key.
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_NAME("Rod Swing Strength") PORT_MINMAX(0x00, 0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(8) PORT_CENTERDELTA(8) PORT_CODE_INC(KEYCODE_SPACE)

	PORT_START("REEL")
	// Keep reel movement separate from the direction pad.
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Reel Handle") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_T) PORT_CODE_INC(KEYCODE_Y)

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf5, IP_ACTIVE_HIGH, IPT_UNUSED )
	// IN1.1 bypasses battery auto-off; its identification as AC-adapter sense is inferred.
	PORT_CONFNAME( 0x02, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x02, "AC Adapter" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))

	PORT_INCLUDE(xavix_battery_status)
INPUT_PORTS_END


// Play TV Bass Fishin'
static INPUT_PORTS_START( rad_bass )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_state::fishing_reel_r))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch") // pressing this will turn the game off.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Menu")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Select")
	// Back/Forward page through menu items independently of the direction pad.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Back") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Forward") PORT_CODE(KEYCODE_PGDN)

	PORT_START("ROW1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Line Release")
	// The physical drag dial is decoded into separate step inputs.
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Drag Increase")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Drag Decrease")
	// The rod sensor reaches the ROM as a debounced event used to cast or set the hook.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Rod Motion")

	PORT_START("REEL")
	// Keep reel movement separate from the direction pad.
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Reel Handle") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_T) PORT_CODE_INC(KEYCODE_Y)

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END


// Connectv Bass Fishin'
static INPUT_PORTS_START( rad_bassp )
	PORT_INCLUDE(rad_bass)
INPUT_PORTS_END

//-------------------------------------------------
//  Systems Sharing the Cartridge Interface
//-------------------------------------------------

// e-kara / i-Singer
static INPUT_PORTS_START( ekara )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// The BIOS reads this fixed panel-layout strap; ordinary e-kara units tie it low.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_ekara_state::matrix_column_r<0>))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_ekara_state::matrix_column_r<1>))

	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch") // pressing this will turn the game off.

	PORT_START("EXTRA0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )       PORT_NAME("Echo Mode")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_NAME("Effects Mode")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Select / Key Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )   PORT_NAME("Select / Key Up")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 )       PORT_NAME("Volume Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 )       PORT_NAME("Volume Up")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 )       PORT_NAME("Tempo Down")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 )       PORT_NAME("Tempo Up")

	PORT_START("EXTRA1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_NAME("Start / Apply Effects")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )       PORT_NAME("Cancel")
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNUSED )
	// no 40/80 due to multiplexer code

	PORT_INCLUDE(xavix_battery_status)
INPUT_PORTS_END

void xavix_ekara_state::xavix_ekara(machine_config &config)
{
	xavix_cart(config);
	SOFTWARE_LIST(config, "cart_list").set_original("ekara_cart").set_filter("EKARA");
	configure_microphone(config);
	m_has_segment_display = true;
	configure_power_hold(config);
	config.set_default_layout(layout_ekara);
}

void xavix_ekara_state::xavix_isinger(machine_config &config)
{
	xavix_cart(config);
	SOFTWARE_LIST(config, "cart_list").set_original("ekara_cart").set_filter("ISINGER");
	configure_microphone(config);
	m_has_segment_display = true;
	configure_power_hold(config);
	config.set_default_layout(layout_ekara);
}

void xavix_ekara_state::machine_start()
{
	xavix_cart_state::machine_start();
	save_item(NAME(m_matrix_select));
	save_item(NAME(m_segment_data_pending));
}

void xavix_ekara_state::machine_reset()
{
	xavix_cart_state::machine_reset();
	m_matrix_select = 0;
	m_segment_data_pending = false;
	for (auto &digit : m_digits)
		digit = 0;
}

int xavix_ekara_state::matrix_column_r(unsigned column)
{
	switch (m_matrix_select & 0x7f)
	{
	case 0x01: return BIT(m_extra0->read(), column);
	case 0x02: return BIT(m_extra0->read(), 2 + column);
	case 0x04: return BIT(m_extra0->read(), 4 + column);
	case 0x08: return BIT(m_extra0->read(), 6 + column);
	case 0x10: return BIT(m_extra1->read(), column);
	case 0x20: return BIT(m_extra1->read(), 2 + column);
	case 0x40: return BIT(m_extra1->read(), 4 + column);
	default:
		LOG("latching inputs with invalid matrix select value of %02x\n", m_matrix_select);
		return 0;
	}
}

void xavix_ekara_state::io0_output_w(u8 data, u8 direction)
{
	// Bit 7 is outside the button/display matrix and remains unidentified.
	u8 const select = data & direction;
	u8 const segment = select & 0x7f;
	m_segment_data_pending =
		m_has_segment_display &&
		std::has_single_bit(segment) &&
		(segment != (m_matrix_select & 0x7f));
	m_matrix_select = select;
}

void xavix_ekara_state::io1_output_w(u8 data, u8 direction)
{
	// The scanner blanks the previous digit data before advancing IO0.  Accumulate
	// only the data write following a new segment selection.
	if (m_segment_data_pending)
	{
		u8 const segment = m_matrix_select & 0x7f;
		u8 const digit_select = data & direction;

		// IO1 bits 3 and 2 select the left and right digits respectively.
		for (unsigned digit = 0; digit < m_digits.size(); digit++)
		{
			u8 segments = m_digits[digit];
			if (BIT(digit_select, 3 - digit))
				segments |= segment;
			else
				segments &= ~segment;
			m_digits[digit] = segments;
		}

		m_segment_data_pending = false;
	}
}


// e-pitch
static INPUT_PORTS_START( epitch )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// The BIOS reads this fixed panel-layout strap.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_ekara_state::matrix_column_r<0>))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_ekara_state::matrix_column_r<1>))

	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch") // pressing this will turn the game off.

	PORT_START("EXTRA0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )       PORT_NAME("Echo Mode")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_NAME("Effects Mode")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Select / Key Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )   PORT_NAME("Select / Key Up")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 )       PORT_NAME("Volume Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 )       PORT_NAME("Volume Up")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 )       PORT_NAME("Tempo Down")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 )       PORT_NAME("Tempo Up")

	PORT_START("EXTRA1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_NAME("Start / Apply Effects")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )       PORT_NAME("Cancel")
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNUSED )
	// no 40/80 due to multiplexer code

	PORT_INCLUDE(xavix_battery_status)

	PORT_MODIFY("IN0")
	// e-pitch ties the BIOS panel-layout strap high and has only six switches.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("EXTRA0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_NAME("Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )       PORT_NAME("Cancel")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )       PORT_NAME("BGM Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_NAME("BGM Up")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Select / Key Down")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )   PORT_NAME("Select / Key Up")

	PORT_MODIFY("EXTRA1")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


// Hi-kara
static INPUT_PORTS_START( hikara )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("AN7")
	// Representative differences select the version 0x10 warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x12, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_hikara_state::matrix_column0_r))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_hikara_state::matrix_column1_r))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_hikara_state::matrix_column2_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_hikara_state::matrix_column3_r))

	PORT_START("EXTRA0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_NAME("Confirm / Apply Effects")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Select / Key Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )   PORT_NAME("Select / Key Up")
	// Effects and Echo double as LCD brightness controls while selecting a song.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_NAME("Effects Mode / Brightness Up")

	PORT_START("EXTRA1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Echo Mode / Brightness Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Volume Up")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Volume Down")

	PORT_START("EXTRA2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Tempo Up")

	PORT_START("EXTRA3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Tempo Down")

	// Each headset has an inline microphone-level resistor; its taper is unmeasured.
	PORT_START("MICVOLUME1")
	PORT_ADJUSTER(100, "Microphone 1 Volume") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_hikara_state::microphone_volume_changed), 0)

	PORT_START("MICVOLUME2")
	PORT_ADJUSTER(100, "Microphone 2 Volume") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_hikara_state::microphone_volume_changed), 1)

	// Patent JP2010019869A locates the rear PSW outside the processor's ten-switch matrix.
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

// CT675A-F stub: acknowledge I2C transfers and retain registers for firmware readback.
// TODO: Emulate video processing and establish the register reset values.
DEFINE_DEVICE_TYPE(XAVIX_HIKARA_I2C, xavix_hikara_i2c_device, "xavix_hikara_i2c", "Hi-Kara CT675A-F Register Bank")

xavix_hikara_i2c_device::xavix_hikara_i2c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, XAVIX_HIKARA_I2C, tag, owner, clock),
	i2c_hle_interface(mconfig, *this, 0)
{
}

void xavix_hikara_state::xavix_hikara(machine_config &config)
{
	xavix_cart(config);
	// JP2010019869A shows analogue dry monitoring; omit that route until its slow host playback is resolved.
	m_soc->adc().set_audio_channel(0);
	MICROPHONE(config, m_microphone[0], 1).front_center()
		.add_route(0, m_soc->adc(), 1.0);
	// The second headset microphone only mixes after an explicit Audio Mixer channel route is selected.
	MICROPHONE(config, m_microphone[1], 1).map_on_request_only()
		.add_route(0, m_soc->adc(), 1.0);

	// The serial framing and boot registers match an OTA5182-compatible LCD controller.
	// TODO: Verify the fitted controller/panel and CT675 scaling and colour processing.
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE | VIDEO_SELF_RENDER);
	m_screen->set_screen_update(FUNC(xavix_hikara_state::screen_update));
	m_screen->screen_vblank().append(FUNC(xavix_hikara_state::lcd_vblank));

	// Retain the source geometry while presenting it at the panel's 5:4 aspect ratio.
	SCREEN(config, m_lcd_screen).set_lcd();
	rectangle const visible = m_screen->visible_area();
	m_lcd_screen->set_raw(m_soc->clock() / xavix_video_device::PICTURE_PIXEL_CLOCKS, m_screen->width(), visible.left(), visible.right() + 1,
			m_screen->height(), visible.top(), visible.bottom() + 1);
	m_lcd_screen->set_refresh(m_screen->frame_period());
	m_lcd_screen->set_physical_aspect(5, 4);
	m_lcd_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE | VIDEO_SELF_RENDER);
	m_lcd_screen->set_screen_update(FUNC(xavix_hikara_state::screen_update_lcd));
	m_lcd_screen->screen_vblank().append(FUNC(xavix_hikara_state::lcd_screen_vblank));
	config.set_default_layout(layout_hikara);

	// U7 is marked Cheertek CT675A-F; TODO: verify its A-revision address selection and register reset values on hardware.
	// FC/FD is the CT675A-F scaler register bank.
	XAVIX_HIKARA_I2C(config, m_ct675_i2c[0]).set_address(0x7e);
	m_ct675_i2c[0]->sda_callback().set(*this, FUNC(xavix_hikara_state::ct675_i2c_sda_w<0>));
	// 4A/4B is the CT675A-F video-ADC register bank.
	XAVIX_HIKARA_I2C(config, m_ct675_i2c[1]).set_address(0x25);
	m_ct675_i2c[1]->sda_callback().set(*this, FUNC(xavix_hikara_state::ct675_i2c_sda_w<1>));
	// 48/49 is the CT675A-F video-decoder register bank.
	XAVIX_HIKARA_I2C(config, m_ct675_i2c[2]).set_address(0x24);
	m_ct675_i2c[2]->sda_callback().set(*this, FUNC(xavix_hikara_state::ct675_i2c_sda_w<2>));

	// The fitted amplifier is unidentified, but its volume-control protocol is LM4811-compatible.
	m_soc->reset_routes();
	m_soc->add_route(0, m_volume_filter[0], 1.0);
	m_soc->add_route(1, m_volume_filter[1], 1.0);
	FILTER_VOLUME(config, m_volume_filter[0]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	FILTER_VOLUME(config, m_volume_filter[1]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("hikara_cart");
}

void xavix_hikara_state::machine_start()
{
	xavix_ekara_state::machine_start();

	m_lcd_output.start(machine());
	m_screen->register_screen_bitmap(m_lcd_source_bitmap);
	save_item(NAME(m_lcd_source_bitmap));
	save_item(NAME(m_ct675_i2c_sda));
	save_item(NAME(m_lcd.regs));
	save_item(NAME(m_lcd.shift));
	save_item(NAME(m_lcd.read_shift));
	save_item(NAME(m_lcd.bits));
	save_item(NAME(m_lcd.select));
	save_item(NAME(m_lcd.clock));
	save_item(NAME(m_lcd.reading));
	save_item(NAME(m_lcd.output));
	save_item(NAME(m_lcd.brightness));
	save_item(NAME(m_volume_step));
	save_item(NAME(m_volume_clock));
}

void xavix_hikara_state::device_stop()
{
	m_lcd_output.stop(machine());
	xavix_ekara_state::device_stop();
}

void xavix_hikara_state::machine_reset()
{
	xavix_ekara_state::machine_reset();

	std::ranges::fill(m_ct675_i2c_sda, true);
	m_lcd = {};
	reset_lcd_registers();
	m_lcd_source_bitmap.fill(rgb_t::black());
	m_volume_step = 11;
	m_volume_clock = false;
	update_volume();

	for (unsigned index = 0; index < m_microphone.size(); index++)
		m_microphone[index]->set_output_gain(ALL_OUTPUTS, float(m_microphone_volume[index]->read()) / 100.0f);
}

int xavix_hikara_state::matrix_column0_r()
{
	// The ten panel switches connect every pair of five bidirectional matrix lines.
	return (m_matrix_select & m_extra0->read() & 0x1e) ? 0x01 : 0x00;
}

int xavix_hikara_state::matrix_column1_r()
{
	return (m_matrix_select & m_extra1->read() & 0x1e) ? 0x01 : 0x00;
}

int xavix_hikara_state::matrix_column2_r()
{
	return (m_matrix_select & m_extra2->read() & 0x1e) ? 0x01 : 0x00;
}

int xavix_hikara_state::matrix_column3_r()
{
	return (m_matrix_select & m_extra3->read() & 0x1e) ? 0x01 : 0x00;
}

u8 xavix_hikara_state::io1_input_r(u8 direction)
{
	u8 ret = xavix_state::io1_input_r(direction);

	if (!BIT(direction, 5))
	{
		// Both serial buses share the bidirectional data pin and release it high
		// when idle.  Only the selected peripheral drives read data.
		ret &= ~0x20;
		ret |= (m_ct675_i2c_sda[0] & m_ct675_i2c_sda[1] & m_ct675_i2c_sda[2] & m_lcd.output) << 5;
	}

	return ret;
}

void xavix_hikara_state::io1_output_w(u8 data, u8 direction)
{
	xavix_ekara_state::io1_output_w(data, direction);

	// The CT675A-F register banks share SCL bit 4 and SDA bit 5.
	for (unsigned index = 0; index < m_ct675_i2c.size(); index++)
	{
		m_ct675_i2c[index]->sda_write(BIT(direction, 5) ? BIT(data, 5) : 1);
		m_ct675_i2c[index]->scl_write(BIT(direction, 4) ? BIT(data, 4) : 0);
	}

	// LCD select is active low on bit 6, clock on bit 1, and data shares bit 5.
	lcd_serial_w(!BIT(direction, 6) || BIT(data, 6),
			!BIT(direction, 1) || BIT(data, 1), !BIT(direction, 5) || BIT(data, 5));
}

void xavix_hikara_state::reset_lcd_registers()
{
	// OTA5182-compatible defaults; undocumented registers remain opaque storage.
	m_lcd.regs = { 0x0d, 0x04, 0x01, 0x08, 0x03, 0x20, 0x00, 0x40,
			0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00 };
	m_lcd.brightness = 0;
}

void xavix_hikara_state::lcd_serial_w(bool select, bool clock, bool data)
{
	if (m_lcd.select && !select)
	{
		m_lcd.shift = 0;
		m_lcd.bits = 0;
		m_lcd.reading = false;
		m_lcd.output = true;
	}

	if (!select && !m_lcd.clock && clock)
	{
		if (m_lcd.bits < 16)
		{
			if (!m_lcd.reading)
				m_lcd.shift = (m_lcd.shift << 1) | data;
			m_lcd.bits++;
			if ((m_lcd.bits == 5) && BIT(m_lcd.shift, 0))
			{
				m_lcd.reading = true;
				m_lcd.read_shift = m_lcd.regs[BIT(m_lcd.shift, 1, 4)];
			}
		}
		else
			m_lcd.bits = 17;
	}

	// Reads turn the data line around after the four address bits and R/W bit.
	if (!select && m_lcd.clock && !clock && m_lcd.reading && (m_lcd.bits < 16))
	{
		m_lcd.output = BIT(m_lcd.read_shift, 10);
		m_lcd.read_shift <<= 1;
	}

	if (!m_lcd.select && select)
	{
		if (!m_lcd.reading && (m_lcd.bits == 16))
			lcd_register_w(m_lcd.shift >> 12, m_lcd.shift & 0x07ff);
		m_lcd.reading = false;
		m_lcd.output = true;
	}

	m_lcd.select = select;
	m_lcd.clock = clock;
}

void xavix_hikara_state::lcd_register_w(unsigned address, u16 data)
{
	if (address == 0)
	{
		// Reset and standby act immediately; picture adjustments latch at VSYNC.
		m_lcd_screen->update_now();
		if (!BIT(data, 3))
		{
			reset_lcd_registers();
			m_lcd.regs[0] &= ~0x08;
			return;
		}
	}
	else if (!BIT(m_lcd.regs[0], 3))
		return;

	m_lcd.regs[address] = data;
}

u32 xavix_hikara_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	xavix_state::screen_update(screen, bitmap, cliprect);
	copybitmap(m_lcd_source_bitmap, bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void xavix_hikara_state::lcd_vblank(int state)
{
	if (state)
		m_lcd.brightness = (int(m_lcd.regs[7] & 0x7f) - 0x40) * 4;
}

void xavix_hikara_state::lcd_screen_vblank(int state)
{
	if (!state)
		m_lcd_output.restore(*m_lcd_screen);
}

u32 xavix_hikara_state::screen_update_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if ((m_lcd.regs[0] & 0x0c) != 0x0c)
	{
		bitmap.fill(rgb_t::black(), cliprect);
	}
	else
	{
		// Both displays share the decoded picture, including any partial updates.
		m_screen->update_partial(cliprect.max_y, cliprect.max_x + 1);
		copybitmap(bitmap, m_lcd_source_bitmap, 0, 0, 0, 0, cliprect);
		if (m_lcd.brightness)
		{
			// Approximate the documented brightness offset in RGB; panel response is unknown.
			auto const adjust = [this](u8 value) { return std::clamp(value + m_lcd.brightness, 0, 255); };
			for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
				{
					rgb_t const color = bitmap.pix(y, x);
					bitmap.pix(y, x) = rgb_t(adjust(color.r()), adjust(color.g()), adjust(color.b()));
				}
			}
		}
	}
	m_lcd_output.update(screen, bitmap, cliprect, m_soc->picture_area());
	return 0;
}

void xavix_hikara_state::io0_output_w(u8 data, u8 direction)
{
	xavix_ekara_state::io0_output_w(data, direction);

	bool const clock = BIT(data & direction, 6);
	if (clock && !m_volume_clock && BIT(m_soc->io().direction<1>(), 5))
	{
		if (BIT(m_soc->io().data_latch<1>(), 5))
			m_volume_step = std::min<u8>(m_volume_step + 1, 15);
		else
			m_volume_step = m_volume_step ? m_volume_step - 1 : 0;
		update_volume();
	}
	m_volume_clock = clock;
}

void xavix_hikara_state::update_volume()
{
	float const gain = powf(10.0f, (3.0f * (m_volume_step - 11)) / 20.0f);
	m_volume_filter[0]->set_gain(gain);
	m_volume_filter[1]->set_gain(gain);
}

INPUT_CHANGED_MEMBER(xavix_hikara_state::microphone_volume_changed)
{
	m_microphone[param]->set_output_gain(ALL_OUTPUTS, float(newval) / 100.0f);
}


// Popira
static INPUT_PORTS_START( popira )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pad 1") PORT_PLAYER(1) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Pad 2") PORT_PLAYER(1) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pad 3") PORT_PLAYER(1) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Pad 4") PORT_PLAYER(1) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Select Up") PORT_PLAYER(1) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Select Down") PORT_PLAYER(1) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start") PORT_PLAYER(1)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // halts execution and enables a memory viewer if used with cartridge gc0001 (debug leftover?)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_cart_state::xavix_popira(machine_config &config)
{
	xavix_cart(config);

	// is a battery / power source required to store NVRAM in the CPU?  Popira definitely needs NVRAM storing on power-of
	// XaviX Tennis won't boot if you do (but that could be an unrelated SEEPROM issue?) & DDR Family Mat gets stuck in 2 Player mode with no obvious way of changing back
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// A recurring IO1.7 pulse maintains the external power hold until firmware shutdown.
	configure_power_hold(config);

	SOFTWARE_LIST(config, "cart_list").set_original("ekara_cart").set_filter("POPIRA");
}

void xavix_cart_state::xavix_popirak(machine_config &config)
{
	xavix_cart(config);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// A recurring IO1.7 pulse maintains the external power hold until firmware shutdown.
	configure_power_hold(config);

	SOFTWARE_LIST(config, "cart_list").set_original("ekara_cart").set_filter("POPIRA_KOREA");
}


// Popira 2
static INPUT_PORTS_START( popira2 ) // player 2 buttons have reported heavy latency and are sampled by the timer-driven ADC path
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pad 1") PORT_PLAYER(1) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pad 2") PORT_PLAYER(1) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Pad 3") PORT_PLAYER(1) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Pad 4") PORT_PLAYER(1) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Select Up") PORT_PLAYER(1) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Select Down") PORT_PLAYER(1) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start") PORT_PLAYER(1)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_cart_state::cart_sda_r))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The adapter sense is inferred from its auto-off bypass and the documented dual supply options.
	PORT_CONFNAME( 0x20, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x20, "AC Adapter" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	// main input processing code is at 028059, which ends with setting a timer (028079) to read analog ports and get these buttons that way.  main timer handler is at 00eb77, which reads ports via the ADC.  $c3 where ports are stored is also checked at 00e6f4
	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Pad 1") PORT_PLAYER(2) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Pad 2") PORT_PLAYER(2) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Pad 3") PORT_PLAYER(2) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Pad 4") PORT_PLAYER(2) PORT_CODE(KEYCODE_L)

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_popira2_state::xavix_popira2(machine_config &config)
{
	xavix_popira(config);

	m_soc->adc().input_callback<0>().set(*this, FUNC(xavix_popira2_state::player2_adc0_r));
	m_soc->adc().input_callback<1>().set(*this, FUNC(xavix_popira2_state::player2_adc1_r));
}

// see code at 028060, using table from 00eb6d for conversion
u8 xavix_popira2_state::player2_adc0_r()
{
	u8 p2 = m_p2->read() & 0x03;
	switch (p2)
	{
	case 0x00: return 0xa0;
	case 0x01: return 0x60;
	case 0x02: return 0x10;
	case 0x03: return 0x00;
	}

	return 0x00;
}

u8 xavix_popira2_state::player2_adc1_r()
{
	u8 p2 = (m_p2->read() >> 2) & 0x03;
	switch (p2)
	{
	case 0x00: return 0xa0;
	case 0x01: return 0x60;
	case 0x02: return 0x10;
	case 0x03: return 0x00;
	}

	return 0x00;
}

// the cart pins Popira 2 uses for IO with cart gc0010 are not controllable by the CPU on other ekara systems
void xavix_popira2_state::io1_output_w(u8 data, u8 direction)
{
	if (m_cartslot->has_cart())
	{
		m_cartslot->write_sda(BIT(direction, 3) ? BIT(data, 3) : 1);
		m_cartslot->write_scl(BIT(direction, 4) ? BIT(data, 4) : 0);
	}
}


// Jumping Popira
static INPUT_PORTS_START( jpopira )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Cancel") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Select Left") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Select Right") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Confirm") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	// The documented AC input and this line's verified auto-off bypass identify the source selector.
	PORT_CONFNAME( 0x20, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x20, "AC Adapter" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x97, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_i2c_cart_state::xavix_jpopira(machine_config &config)
{
	xavix_cart(config);

	SOFTWARE_LIST(config, "cart_list").set_original("ekara_cart").set_filter("JPOPIRA");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I2C_24C02(config, "i2cmem"); // 24LC02

	configure_power_hold(config);
}


// Taiko de Popira
static INPUT_PORTS_START( taikodp )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_battery_status)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )

	// The ROM thresholds each ADC transient to one hit bit; a one-frame impulse avoids repeat hits.
	PORT_MODIFY("AN0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Drum") PORT_IMPULSE(1)
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Drum") PORT_IMPULSE(1)
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_i2c_cart_state::xavix_taikodp(machine_config &config)
{
	xavix_cart(config);

	SOFTWARE_LIST(config, "cart_list").set_original("ekara_cart").set_filter("TAIKO");

	I2C_24C02(config, "i2cmem"); // 24LC02
}


// Dance Dance Revolution Family Mat
static INPUT_PORTS_START( ddrfammt )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// Independent mat switches allow opposing directions to be pressed simultaneously.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Up Arrow") PORT_PLAYER(1) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Down Arrow") PORT_PLAYER(1) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Left Arrow") PORT_PLAYER(1) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Right Arrow") PORT_PLAYER(1) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Up Arrow") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Down Arrow") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Left Arrow") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Right Arrow") PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	// Either this line or a P2 contact makes the second-mat detection sticky for the session.
	PORT_CONFNAME( 0x02,  0x00, "2nd Mat Connected" )
	PORT_CONFSETTING(     0x00, DEF_STR( No ) )
	PORT_CONFSETTING(     0x02, DEF_STR( Yes ) )
	// This line bypasses automatic shutdown, but its physical identity is unknown.
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x9d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_cart_state::xavix_ddrfammt(machine_config &config)
{
	xavix_cart(config);

	SOFTWARE_LIST(config, "cart_list").set_original("ekara_cart").set_filter("FAMMAT");

	configure_power_hold(config);
}


// Evio
static INPUT_PORTS_START( evio )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_NAME("Select Down")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_NAME("Select Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Confirm")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The AC-adapter identity is inferred from hardware support and the proven auto-off bypass.
	PORT_CONFNAME( 0x20, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x20, "AC Adapter" )
	PORT_CONFNAME( 0x40, 0x00, "Output Mode" )
	PORT_CONFSETTING(    0x00, "TV" )
	PORT_CONFSETTING(    0x40, "Speaker" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_cart_state::cart_sda_r))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN0")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Vibrato") PORT_PLAYER(1)
	PORT_MODIFY("AN1")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Vibrato") PORT_PLAYER(2)

	PORT_MODIFY("AN7")
	// Representative values select eVio's unusually narrow three-state battery classifier.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x11, "Low" )
	PORT_CONFSETTING(    0x18, "Very Low" )
	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_evio_state::xavix_evio(machine_config &config)
{
	xavix_cart(config);

	SOFTWARE_LIST(config, "cart_list_evio").set_original("evio_cart");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	XAVIX_EVIO_BOW(config, m_bow[0]);
	XAVIX_EVIO_BOW(config, m_bow[1]).set_player(1);

	configure_power_hold(config);
}

u8 xavix_evio_state::io1_input_r(u8)
{
	return (m_in1->read() & 0xf0) | m_bow[0]->quadrature_r() | (m_bow[1]->quadrature_r() << 2);
}

void xavix_evio_state::io1_output_w(u8 data, u8 direction)
{
	if (m_cartslot->has_cart())
	{
		m_cartslot->write_sda(BIT(direction, 4) ? BIT(data, 4) : 1);
		m_cartslot->write_scl(BIT(direction, 5) ? BIT(data, 5) : 0);
	}
}


// Bakutou Sengen Daigunder DX
static INPUT_PORTS_START( daig )
	PORT_INCLUDE(xavix)
	PORT_INCLUDE(xavix_reset_control)
	PORT_INCLUDE(xavix_lightgun_1p)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Dragon Cannon sensor status is not understood.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Cannon Attack / Shot")
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_UNUSED )
	// A maintained body contact selects the alternate form when the unit boots.
	PORT_CONFNAME( 0x20, 0x00, "Body Form" )
	PORT_CONFSETTING(    0x00, "Robot" )
	PORT_CONFSETTING(    0x20, "Dragon Cannon" )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("robots", FUNC(xavix_daigunder_robots_device::data_r))
	PORT_BIT( 0x38, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_tak_daig_state::xavix_tak_daig(machine_config &config)
{
	xavix_cart(config);
	// Firmware alternates IO1.7 every field and releases this external RC hold on shutdown.
	configure_power_hold(config);
	// Eight event-0 samples satisfy the Dragon Cannon accumulator; the physical sensor cadence is unmeasured.
	configure_lightgun_scanlines(config, 0x01);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	XAVIX_DAIGUNDER_ROBOTS(config, m_robots, peripheral_clock());
	m_robots->set_screen(m_screen);
	m_robots->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<2>));
	m_screen->screen_vblank().append(m_robots, FUNC(xavix_daigunder_robots_device::vblank_w));

	SOFTWARE_LIST(config, "cart_list_daig").set_original("daig_cart");
}

u8 xavix_tak_daig_state::lightgun_r(offs_t offset)
{
	// Firmware centres the Dragon Cannon after adding $10 to the captured horizontal position.
	return lightgun_position_r(0, offset, 0x10);
}


// Gachinko Shoubu! PachisloTV
static INPUT_PORTS_START( gcslottv )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 ) PORT_NAME("Stop Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) PORT_NAME("Stop Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 ) PORT_NAME("Stop Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Menu / OK")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The normally closed start-lever switch opens when the lever is pulled.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start Lever")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_NAME("1 Bet")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_N)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Medal")
	PORT_BIT( 0x3e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_gcslottv_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(xavix_gcslottv_state::cart_r), FUNC(xavix_gcslottv_state::cart_w));
}

void xavix_gcslottv_state::xavix_gcslottv(machine_config &config)
{
	xavix_cart(config);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SOFTWARE_LIST(config, "cart_list_gcslottv").set_original("gcslottv_cart");

	// A recurring IO1.7 pulse maintains the external power hold until firmware shutdown.
	configure_power_hold(config);
}

//-------------------------------------------------
//  Shared TV-PC Implementation
//-------------------------------------------------

// TV-PC
static INPUT_PORTS_START( tvpc )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xd7, IP_ACTIVE_HIGH, IPT_UNUSED )
	// IO1.5 selects the firmware's battery or AC-adapter inactivity timeout.
	PORT_CONFNAME( 0x20, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x20, "AC Adapter" )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"6 \u304a") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&') // 6 お
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"5 \u3048") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') // 5 え
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"4 \u3046") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') // 4 う
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"3 \u3042") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') // 3 あ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"2 \u3075") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') // 2 ふ
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"1 \u306c") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') // 1 ぬ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mail") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"7 \u3084") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'') // 7 や

	PORT_START("KEY1")
	// The hardware photographs do not establish formal player numbers for the two cursor clusters.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left Cursor Left") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left Cursor Up") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left Cursor Down") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left Cursor Right") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right Cursor Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right Cursor Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right Cursor Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right Cursor Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Back Space") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"[ \u309c") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') // [ ゜
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"^ \u3078") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~') // ^ へ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"- \u307b") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=') // - ほ
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"0 \u308f") PORT_CODE(KEYCODE_0) PORT_CHAR('0') // 0 わ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"9 \u3088") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')') // 9 よ
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"8 \u3086") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(') // 8 ゆ

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"I \u306b") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') // I に
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"U \u306a") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') // U な
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Y \u3093") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') // Y ん
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"T \u304b") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') // T か
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"R \u3059") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') // R す
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"E \u3044") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') // E い
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"W \u3066") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') // W て
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Q \u305f") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') // Q た

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"@ \u309b") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`') // @ ゛
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"¥ \u30fc") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(U'¥') PORT_CHAR('|') // ¥ ー
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"P \u305b") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') // P せ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"O \u3089") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') // O ら
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"F \u306f") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') // F は
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"D \u3057") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') // D し
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"S \u3068") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') // S と
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"A \u3061") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') // A ち

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"] \u3080") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') // ] む
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8": \u3051") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*') // : け
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"; \u308c") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+') // ; れ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"L \u308a") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') // L り
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"K \u306e") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') // K の
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"J \u307e") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') // J ま
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"H \u304f") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') // H く
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"G \u304d") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') // G き

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"M \u3082") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') // M も
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"N \u307f") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') // N み
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"B \u3053") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') // B こ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"V \u3072") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') // V ひ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"C \u305d") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') // C そ
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"X \u3055") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') // X さ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Z \u3064") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') // Z つ
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\\ \u308d") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('_') // \ ろ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"/ \u3081") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') // / め
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8". \u308b") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') // . る
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8", \u306d") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') // , ね
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Input Mode (\u5165\u529b\u30e2\u30fc\u30c9)") PORT_CODE(KEYCODE_DEL_PAD) // Input Mode (入力モード)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void xavix_tvpc_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("bios", 0x000000);
	map(0x400000, 0x5fffff).rom().region("bios", 0x000000);
	map(0x600000, 0x6000ff).r(FUNC(xavix_tvpc_state::keyboard_r));
	map(0x600100, 0x7fffff).rom().region("bios", 0x200100);
}

void xavix_tvpc_state::xavix_tvpc(machine_config &config)
{
	xavix_i2c_24c16(config);
	TVPC_MOUSE_PORT(config, m_mouse).set_options([] (device_slot_interface &port)
	{
		port.option_add("mouse", TVPC_MOUSE);
		port.option_add("mic_mouse", TVPC_HK_MOUSE);
	}, "mouse", false);

	// The mouse uses writable counters 2 and 3; vertical polarity is opposite MAME's mouse Y.
	m_soc->anport().input_callback<2>().set(m_mouse, FUNC(tvpc_mouse_port_device::x_r));
	m_soc->anport().input_callback<3>().set(m_mouse, FUNC(tvpc_mouse_port_device::y_r));
	m_soc->anport().set_counter_mode(2);
	m_soc->anport().set_counter_mode(3, true);
	// These ROMs retain signed cursor positions in the counters, including the positive endpoint.
	// The counters saturate at -128 and +127, discarding motion beyond either endpoint.
	m_soc->anport().set_counter_saturation(2);
	m_soc->anport().set_counter_saturation(3);
}

void xavix_tvpc_state::xavix_tvpc_hk(machine_config &config)
{
	xavix_tvpc(config);
	m_mouse->set_default_option("mic_mouse");
	m_soc->adc().set_audio_channel(0);
	m_mouse->add_route(0, m_soc->adc(), 1.0);
}

u8 xavix_tvpc_state::io0_input_r(u8 direction)
{
	return (xavix_i2c_power_state::io0_input_r(direction) & ~0x80) | (m_mouse->button_r() << 7);
}

u8 xavix_tvpc_state::keyboard_r(offs_t offset)
{
	switch (offset & 0xff)
	{
	case 0x01: return m_keyboard[0]->read();
	case 0x02: return m_keyboard[1]->read();
	case 0x04: return m_keyboard[2]->read();
	case 0x08: return m_keyboard[3]->read();
	case 0x10: return m_keyboard[4]->read();
	case 0x20: return m_keyboard[5]->read();
	case 0x40: return m_keyboard[6]->read();
	case 0x80: return m_keyboard[7]->read();
	default:   return 0;
	}
}

//-------------------------------------------------
//  Other Systems
//-------------------------------------------------

// All-Star Kansha-sai Chou Gouka! Quiz Kettieban: Akasaka 5-choume Taikan Studio
static INPUT_PORTS_START( epo_quiz )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// Square-pad bindings preserve their 2x2 positions and simultaneous contacts.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A / 1 / Red / Back") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B / 2 / Blue / Confirm") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C / 3 / Yellow / Left") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 D / 4 / Green / Right") PORT_CODE(KEYCODE_3_PAD)

	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A / 1 / Red / Back") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B / 2 / Blue / Confirm") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C / 3 / Yellow / Left") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 D / 4 / Green / Right") PORT_CODE(KEYCODE_C)

	PORT_MODIFY("IN1")
	PORT_CONFNAME( 0x01, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x01, "AC Adapter" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 A / 1 / Red / Back") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 B / 2 / Blue / Confirm") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 C / 3 / Yellow / Left") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(3) PORT_NAME("P3 D / 4 / Green / Right") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x98, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("P4 A / 1 / Red / Back") PORT_CODE(KEYCODE_I)
	PORT_MODIFY("AN6")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("P4 B / 2 / Blue / Confirm") PORT_CODE(KEYCODE_P)
	PORT_MODIFY("AN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("P4 C / 3 / Yellow / Left") PORT_CODE(KEYCODE_COMMA)
	PORT_MODIFY("AN4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_NAME("P4 D / 4 / Green / Right") PORT_CODE(KEYCODE_SLASH)

	PORT_START("ROUND_PADS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Round Pad 1") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("Round Pad 2") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(3) PORT_NAME("Round Pad 3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(4) PORT_NAME("Round Pad 4") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_epo_quiz_state::xavix_epo_quiz(machine_config &config)
{
	xavix_4mb(config);
	m_soc->adc().input_callback<6>().set(FUNC(xavix_epo_quiz_state::an6_r));
}

u8 xavix_epo_quiz_state::io0_input_r(u8 direction)
{
	// Action modes collapse each square station to one pad; its representative Blue contact is unmeasured.
	u8 const pads = m_round_pads->read();
	return xavix_power_state::io0_input_r(direction) | (BIT(pads, 0) << 3) | (BIT(pads, 1) << 7);
}

u8 xavix_epo_quiz_state::io1_input_r(u8 direction)
{
	return xavix_power_state::io1_input_r(direction) | (BIT(m_round_pads->read(), 2) << 6);
}

u8 xavix_epo_quiz_state::an6_r()
{
	return m_an6->read() | (BIT(m_round_pads->read(), 3) << 7);
}


// Bistro Kids: Boku mo Watashi mo Kok-san!
static INPUT_PORTS_START( bistro )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Right Hand Button (Blue)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Left Hand Button (Yellow)")
	// The ROM distinguishes both cookware encodings, but the physical sensing method is unknown.
	PORT_CONFNAME( 0x0c, 0x00, "Cookware on Burner" )
	PORT_CONFSETTING(    0x00, DEF_STR(None) )
	PORT_CONFSETTING(    0x04, "Pot" )
	PORT_CONFSETTING(    0x0c, "Frying Pan" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Selection Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Selection Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Selection Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Selection Right")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("sensor", FUNC(xavix_bistro_sensor_device::sense_r))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Cutting Board")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Faucet Lever")
	PORT_CONFNAME( 0x30, 0x00, "Gas Control" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x10, "Low" )
	PORT_CONFSETTING(    0x20, "High" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_bistro_state::xavix_bistro(machine_config &config)
{
	xavix_2mb(config);
	XAVIX_BISTRO_SENSOR(config, m_sensor, peripheral_clock());
}

void xavix_bistro_state::io1_output_w(u8 data, u8 direction)
{
	xavix_power_state::io1_output_w(data, direction);
	m_sensor->excite_w(BIT(data & direction, 0));
}


// TV Mail PC mail*cot
static INPUT_PORTS_START( epo_mail )
	PORT_INCLUDE(xavix_i2c)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"1 \u306c") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') // 1 ぬ
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"2 \u3075") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') // 2 ふ
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"3 \u3042") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') // 3 あ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"4 \u3046") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') // 4 う
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_NAME("Selection Disc Up") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_NAME("Selection Disc Left") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_NAME("Selection Disc Down") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_NAME("Selection Disc Right") PORT_CODE(KEYCODE_6_PAD)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"5 \u3048") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') // 5 え
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"6 \u304a") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&') // 6 お
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"7 \u3084") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'') // 7 や
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"8 \u3086") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(') // 8 ゆ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"9 \u3088") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')') // 9 よ
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"0 \u308f / \u3092") PORT_CODE(KEYCODE_0) PORT_CHAR('0') // 0 わ / を
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"- \u307b") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=') // - ほ
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"^ \u3078") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~') // ^ へ

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Q \u305f") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') // Q た
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"W \u3066") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') // W て
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"E \u3044") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') // E い
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"R \u3059") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') // R す
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"T \u304b") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') // T か
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Y \u3093") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') // Y ん
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"U \u306a") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') // U な
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"I \u306b") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') // I に

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Confirm") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Back Space") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"O \u3089") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') // O ら
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"P \u305b") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') // P せ
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u2665 / \u00b0 / \u309b") PORT_CODE(KEYCODE_OPENBRACE) // ♥ / ° / ゛
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"A \u3061") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') // A ち
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"S \u3068") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') // S と
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"D \u3057") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') // D し
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"F \u306f") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') // F は
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"G \u304d") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') // G き
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"H \u304f") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') // H く
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"J \u307e") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') // J ま
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"K \u306e") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') // K の

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"L \u308a") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') // L り
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"! \u308c") PORT_CODE(KEYCODE_COLON) PORT_CHAR('!') // ! れ
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"? \u3080 / \u3051") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('?') // ? む / け
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8", \u306d") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') // , ね
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8". \u308b") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') // . る
	// The pictogram on this key is indistinct in the patent drawing; its two kana legends are clear.
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Pictogram / \u308d / \u3081") PORT_CODE(KEYCODE_BACKSLASH) // Pictogram / ろ / め
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Text Cursor Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Z \u3064") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') // Z つ
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"X \u3055") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') // X さ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"C \u305d") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') // C そ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"V \u3072") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') // V ひ
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"B \u3053") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') // B こ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"N \u307f") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') // N み
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"M \u3082") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') // M も

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Input Mode") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Font") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Color") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Character Type (\u3042 / \u30a2 / A)") PORT_CODE(KEYCODE_F6) // Character Type (あ / ア / A)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space / Convert") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Stamp") PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Sheet / Background") PORT_CODE(KEYCODE_F8)

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Sound Effect") PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Attach") PORT_CODE(KEYCODE_F10)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Text Cursor Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Text Cursor Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Text Cursor Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_battery_status)
	// Firmware asserts IO1.7 to shut down; patent US7555105 identifies the separate hard power switch.
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void xavix_epo_mail_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("bios", 0x000000);
	map(0x400000, 0x5fffff).rom().region("bios", 0x000000);
	map(0x600000, 0x600000).r(FUNC(xavix_epo_mail_state::keyboard_r));
	map(0x600001, 0x7fffff).rom().region("bios", 0x200001);
}

void xavix_epo_mail_state::xavix_epo_mail(machine_config &config)
{
	xavix_i2c_24c16(config);
	XAVIX_MAILCOT_MASCOT(config, m_mascot);
}

u8 xavix_epo_mail_state::io1_input_r(u8 direction)
{
	return (xavix_i2c_power_state::io1_input_r(direction) & ~0x26) | (m_mascot->id_r() << 1) | (m_mascot->sda_r() << 5);
}

void xavix_epo_mail_state::io1_output_w(u8 data, u8 direction)
{
	xavix_i2c_power_state::io1_output_w(data, direction);
	m_mascot->write(BIT(direction, 5) ? BIT(data, 5) : 1, BIT(direction, 6) ? BIT(data, 6) : 0);
}

u8 xavix_epo_mail_state::keyboard_r()
{
	// Patent US7555105 shows eight IO0-selected rows and a ninth selected by IO1.0 feeding one 74HC541.
	u8 data = 0;
	u8 const io0_rows = m_soc->io().data_latch<0>() & m_soc->io().direction<0>();
	for (unsigned row = 0; row != 8; row++)
		if (BIT(io0_rows, row))
			data |= m_keyboard[row]->read();

	if (BIT(m_soc->io().data_latch<1>() & m_soc->io().direction<1>(), 0))
		data |= m_keyboard[8]->read();

	return data;
}


// 2 en 1 Hippo: et la formidable aventure des lettres
// TODO: Identify the On button connection.
static INPUT_PORTS_START( hippofr )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED ) // controls form passive contacts between GPIO lines

	PORT_MODIFY("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x00, "Mode" )
	PORT_CONFSETTING(    0x00, "TV" )
	PORT_CONFSETTING(    0x04, "Toy" )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Repeat") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Off")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_state::power_off), 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Demo / Options") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIRECTIONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACTIVITY")
	// Positions follow the five pictures from left to right on the appliance.
	PORT_CONFNAME( 0x07, 0x00, "Activity Selector" )
	PORT_CONFSETTING(    0x00, "Learn the Letters" )
	PORT_CONFSETTING(    0x01, "Learn Letter Sounds" )
	PORT_CONFSETTING(    0x02, "Letter Order" )
	PORT_CONFSETTING(    0x03, "Find the Letters" )
	PORT_CONFSETTING(    0x04, "Jungle Jam" )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

u16 xavix_hippofr_state::panel_levels_r()
{
	std::array<u16, 10> contacts{};
	auto const close_contact = [&contacts](unsigned first, unsigned second)
	{
		contacts[first] |= u16(1) << second;
		contacts[second] |= u16(1) << first;
	};

	for (unsigned key = 0; key != KEY_CONTACTS.size(); ++key)
	{
		if (BIT(m_keyboard[key / 8]->read(), key % 8))
		{
			auto const [first, second] = KEY_CONTACTS[key];
			close_contact(first, second);
		}
	}

	u8 const directions = m_directions->read();
	if (BIT(directions, 0)) close_contact(3, 9); // Up
	if (BIT(directions, 1)) close_contact(2, 8); // Down
	if (BIT(directions, 2)) close_contact(3, 8); // Left
	if (BIT(directions, 3)) close_contact(2, 9); // Right

	u16 levels = m_soc->io().data_latch<0>() & m_soc->io().direction<0>();
	levels |= u16(m_soc->io().data_latch<1>() & m_soc->io().direction<1>() & 0x03) << 8;
	return propagate_passive_levels<u16>(levels, contacts);
}

u8 xavix_hippofr_state::io0_input_r(u8)
{
	return panel_levels_r();
}

u8 xavix_hippofr_state::io1_input_r(u8 direction)
{
	std::array<u8, 8> contacts{};
	auto const close_contact = [&contacts](unsigned first, unsigned second)
	{
		contacts[first] |= 1U << second;
		contacts[second] |= 1U << first;
	};

	// The five slider positions use every connection state among IO1.4-IO1.6.
	switch (m_activity->read() & 0x07)
	{
	case 0: close_contact(4, 6); break;
	case 1: break;
	case 2: close_contact(5, 6); break;
	case 3: close_contact(4, 5); close_contact(4, 6); break;
	case 4: close_contact(4, 5); break;
	}

	u8 const selector_levels = propagate_passive_levels<u8>(m_soc->io().data_latch<1>() & direction, contacts);
	return m_in1->read() | u8(panel_levels_r() >> 8) | selector_levels;
}


// 2 en 1 Multi'Pilote
static INPUT_PORTS_START( multiplt )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED ) // controls form passive contacts between PIO0 lines

	PORT_MODIFY("IN1")
	// The standalone activities deliberately disable video and use the illuminated controls.
	PORT_CONFNAME( 0x03, 0x02, u8"Sélecteur d'activité" )
	PORT_CONFSETTING(    0x02, "TV" )
	PORT_CONFSETTING(    0x00, "Créa Musique" )
	PORT_CONFSETTING(    0x01, "Mémo Fun" )
	PORT_CONFSETTING(    0x03, "Rap Conduite" )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME(u8"Arrêt / Marche")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME(u8"Démo / Volume")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Rejouer / Menu")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Bouton lumineux rouge")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Bouton lumineux jaune")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Klaxon")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Bouton lumineux vert")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Aide")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("WHEEL")
	PORT_BIT( 0x07, 0x02, IPT_POSITIONAL ) PORT_NAME("Volant") PORT_POSITIONS(5) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN)

	PORT_START("LEVER")
	PORT_BIT( 0x03, 0x01, IPT_POSITIONAL_V ) PORT_NAME("Levier de vitesse") PORT_POSITIONS(3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_RIGHT) PORT_CODE_INC(KEYCODE_LEFT)
INPUT_PORTS_END

void xavix_multiplt_state::xavix_multiplt(machine_config &config)
{
	xavix_2mb(config);
	config.set_default_layout(layout_multiplt);
}

void xavix_multiplt_state::machine_reset()
{
	xavix_state::machine_reset();
	for (auto &lamp : m_lamps)
		lamp = 0;
}

u8 xavix_multiplt_state::io0_input_r(u8 direction)
{
	std::array<u8, 8> contacts{};
	auto const close_contact = [&contacts](unsigned first, unsigned second)
	{
		contacts[first] |= 1U << second;
		contacts[second] |= 1U << first;
	};

	u8 const controls = m_controls->read();
	if (BIT(controls, 0)) close_contact(4, 0); // Demo / Volume
	if (BIT(controls, 1)) close_contact(4, 1); // Replay / Menu
	if (BIT(controls, 2)) close_contact(3, 0); // Red
	if (BIT(controls, 3)) close_contact(3, 1); // Yellow
	if (BIT(controls, 4)) close_contact(3, 2); // Horn
	if (BIT(controls, 5)) close_contact(2, 0); // Green
	if (BIT(controls, 6)) close_contact(2, 1); // Help

	switch (m_lever->read() & 3)
	{
	case 0: close_contact(7, 1); break; // Accelerate
	case 2: close_contact(7, 0); break; // Stop
	}

	switch (m_wheel->read() & 7)
	{
	case 0: close_contact(5, 4); break; // Far left
	case 1: close_contact(6, 4); close_contact(6, 5); break; // Left
	case 2: close_contact(6, 5); break; // Centre
	case 4: close_contact(6, 4); break; // Far right
	}

	// Passive contacts propagate a driven-high level through every closed path.
	u8 const levels = propagate_passive_levels<u8>(m_soc->io().data_latch<0>() & direction, contacts);
	return levels & ~direction;
}

void xavix_multiplt_state::io1_output_w(u8 data, u8 direction)
{
	u8 const outputs = data & direction;
	// IO1.2-IO1.4 drive red, yellow and green respectively.
	for (unsigned lamp = 0; lamp < m_lamps.size(); lamp++)
		m_lamps[lamp] = BIT(outputs, lamp + 2);
}


// Play TV Monster Truck
static INPUT_PORTS_START( rad_mtrk )
	// Play TV Monster Truck runs off an entirely different codebase to everything else, presumably coded by the developer from scratch rather than using code supplied by SSD Company LTD

	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Nitro")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Gear Shifter Forward (High)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear Shifter Forward (Low)")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gear Shifter Reverse / Back")
	PORT_BIT( 0x62, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )  PORT_READ_LINE_DEVICE_MEMBER("wheel", FUNC(xavix_mtrk_wheel_device::direction_r))

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Horn / Weapon / Enter")
	// The momentary button is handled on release before the firmware drives IO1.7 high to cut power.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power")
	PORT_BIT( 0xbd, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN0")
	// Representative values select the ROM's warning threshold; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x30, "Low" )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_state::xavix_rad_mtrk(machine_config &config)
{
	xavix_4mb(config);

	xavix_mtrk_wheel_device &wheel(XAVIX_MTRK_WHEEL(config, "wheel"));
	wheel.event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<3>));
}


// Connectv Monster Truck
static INPUT_PORTS_START( rad_mtrkp )
	PORT_INCLUDE(rad_mtrk)
INPUT_PORTS_END

void xavix_state::xavix_rad_mtrkp(machine_config &config)
{
	xavixp_4mb(config);

	xavix_mtrk_wheel_device &wheel(XAVIX_MTRK_WHEEL(config, "wheel"));
	wheel.event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<3>));
}


// Play TV Football / EA Sports Madden Football
static INPUT_PORTS_START( rad_fb )
	PORT_INCLUDE(xavix)
	PORT_INCLUDE(xavix_reset_control)

	PORT_MODIFY("IN0")
	// The floor contacts remain independent when a player stands on both sides.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Home L") PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Home X / Action") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Home R") PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Away L") PORT_PLAYER(2) PORT_16WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Away X / Action") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Away R") PORT_PLAYER(2) PORT_16WAY
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNUSED )
	// A short press returns to the main menu; holding the button requests shutdown.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power / Main Menu")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_state::xavix_rad_madf(machine_config &config)
{
	xavix_4mb(config);

	xavix_madfb_ball_device &ball(XAVIX_MADFB_BALL(config, "ball", peripheral_clock()));
	ball.event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
}


// TV Wild Adventure Mini Golf
static INPUT_PORTS_START( has_wamg )
	PORT_INCLUDE(xavix)
	// The maintained switch removes supply directly; the ROM also has a separate IO1.7 auto-off path.
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN0")
	// The dial's push switch also pauses play and confirms prompts.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Dial Push / Select") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0xfb, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	// Bit 0 selects the sensor threshold margin, but its board-level source is unknown.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	// Bit 3 must idle high for normal operation; its hardware source is unknown.
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xf6, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Aim Dial") PORT_SENSITIVITY(3) PORT_KEYDELTA(8) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT)
INPUT_PORTS_END

void xavix_has_wamg_state::xavix_has_wamg(machine_config &config)
{
	xavix_4mb(config);

	// The dial is a writable signed counter that the firmware clears after every sample.
	m_soc->anport().input_callback<2>().set_ioport("DIAL");
	m_soc->anport().set_counter_mode(2);

	XAVIX_WAMG_SENSOR(config, m_sensor, peripheral_clock());
	m_soc->adc().input_callback<0>().set(m_sensor, FUNC(xavix_wamg_sensor_device::adc_r));
	m_screen->screen_vblank().append(m_sensor, FUNC(xavix_wamg_sensor_device::vblank_w));
}

void xavix_has_wamg_state::io0_output_w(u8 data, u8 direction)
{
	m_sensor->emitter_w(BIT(data & direction, 3));
}


// TV Hockey
static INPUT_PORTS_START( tom_tvho )
	PORT_INCLUDE(xavix)
	PORT_INCLUDE(xavix_soft_power_control)
	PORT_INCLUDE(xavix_reset_control)

	PORT_MODIFY("MOUSE0X")
	// Relative scaling is provisional because the original counts per distance have not been measured.
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_NAME("P1 Controller X") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1) PORT_CODE(MOUSECODE_X_INDEXED(0)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)
	PORT_MODIFY("MOUSE0Y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_NAME("P1 Controller Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1) PORT_CODE(MOUSECODE_Y_INDEXED(0)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)
	PORT_MODIFY("MOUSE1X")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_NAME("P2 Controller X") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2) PORT_CODE(MOUSECODE_X_INDEXED(1)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)
	PORT_MODIFY("MOUSE1Y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_NAME("P2 Controller Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2) PORT_CODE(MOUSECODE_Y_INDEXED(1)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Controller Button / Confirm") PORT_PLAYER(2) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1))
	// This production-unused line inhibits shutdown in the external-power factory path.
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xcf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Controller Button / Confirm") PORT_PLAYER(1) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0))
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_state::xavix_tom_tvho(machine_config &config)
{
	xavix_2mb(config);

	// Each controller is a mechanical ball mouse connected to wrapping hardware counters.
	m_soc->anport().input_callback<0>().set_ioport("MOUSE0X");
	m_soc->anport().input_callback<1>().set_ioport("MOUSE0Y");
	m_soc->anport().input_callback<2>().set_ioport("MOUSE1X");
	m_soc->anport().input_callback<3>().set_ioport("MOUSE1Y");
	m_soc->anport().set_counter_mode(0);
	m_soc->anport().set_counter_mode(1, true);
	m_soc->anport().set_counter_mode(2);
	m_soc->anport().set_counter_mode(3, true);
}


// Beyblade Ultimate Shooter / Arcade Challenge 5-in-1
static INPUT_PORTS_START( tak_bey )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Enter / Trigger")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel / Signature Move")
	// IO0.6-7 carry the two-main-unit link over the three-contact battle cable.
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	// IO1.2 is a hardware-variant strap sampled at boot; its physical source is unidentified.
	PORT_CONFNAME( 0x04, 0x04, "Launcher Version" )
	PORT_CONFSETTING(    0x04, "A-55 Dragoon / Tyson" )
	PORT_CONFSETTING(    0x00, "A-62 Dranzer / Kai" )
	// IO1.3 disables the firmware timer; its physical strap/source is unidentified.
	PORT_CONFNAME( 0x08, 0x00, "Automatic Power Off" )
	PORT_CONFSETTING(    0x08, DEF_STR( No ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xf3, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select this ROM's states; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x20, "Low" )
	PORT_CONFSETTING(    0x28, "Very Low" )

	PORT_START("DIAL")
	// Four counts moves one menu item; the real detent scale and polarity are not measured.
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Select / Dial Controller") PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT)

	PORT_INCLUDE(xavix_reset_control)
	PORT_INCLUDE(xavix_soft_power_control)

	PORT_MODIFY("POWER")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Button")
INPUT_PORTS_END

void xavix_state::xavix_tak_beyu(machine_config &config)
{
	xavix_nv_2mb(config);

	xavix_beyblade_link_device &link(XAVIX_BEYBLADE_LINK(config, "link"));
	link.set_input_source(*this);
	link.controls_callback().set_ioport("IN0");
	m_soc->io().input_callback<0>().set(link, FUNC(xavix_beyblade_link_device::io_r));
	m_soc->io().output_callback<0>().set(link, FUNC(xavix_beyblade_link_device::io_w));
	m_screen->screen_vblank().append(link, FUNC(xavix_beyblade_link_device::vblank_w));

	m_soc->anport().input_callback<0>().set_ioport("DIAL");
	m_soc->anport().set_counter_mode(0);

	xavix_beyblade_ripcord_device &ripcord(XAVIX_BEYBLADE_RIPCORD(config, "ripcord"));
	ripcord.set_screen(m_screen);
	ripcord.event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(ripcord, FUNC(xavix_beyblade_ripcord_device::vblank_w));
}


// Jyarajyaland
static INPUT_PORTS_START( jarajal )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x10, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x10, "AC Adapter" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Main Button")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Back")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Coin Return")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	// Firmware needs four high polls before rearming the inserted-coin event.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(5)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_jarajal_state::coin_inserted), 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select this ROM's states; physical voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x13, "Low" )
	PORT_CONFSETTING(    0x18, "Very Low" )

	PORT_START("DIAL")
	// Route default controls through the digital sequences so relative axes cannot retain an off-centre position.
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Rotating Lever") PORT_SENSITIVITY(100) PORT_KEYDELTA(12) PORT_CENTERDELTA(128) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_INCLUDE(xavix_reset_control)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void xavix_jarajal_state::xavix_jarajal(machine_config &config)
{
	xavix_nv_2mb(config);

	// The lever is read through writable signed counter 0 and cleared by software.
	m_soc->anport().input_callback<0>().set_ioport("DIAL");
	m_soc->anport().set_counter_mode(0);

	HOPPER(config, m_hopper, attotime::from_msec(100)); // TODO: Measure the real hopper pulse interval.
	m_hopper->dispense_handler().set(*this, FUNC(xavix_jarajal_state::hopper_sensor_w));
}

void xavix_jarajal_state::machine_reset()
{
	xavix_state::machine_reset();
	m_hopper->motor_w(0);
}

INPUT_CHANGED_MEMBER(xavix_jarajal_state::coin_inserted)
{
	if (newval)
		m_soc->io().event_w<1>(1);
}

void xavix_jarajal_state::hopper_sensor_w(int state)
{
	if (state)
		m_soc->io().event_w<0>(1);
}

void xavix_jarajal_state::io0_output_w(u8 data, u8 direction)
{
	xavix_state::io0_output_w(data, direction);
	m_hopper->motor_w(BIT(data & direction, 0));
	// IO0.1 accompanies payouts of six or more coins; its electrical function is unknown.
}


// TV Mahjong Shoudan Taikyoku: 4-nin Uchi
static INPUT_PORTS_START( epo_mj )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("OK")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH ) PORT_NAME("Riichi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_RON ) PORT_NAME("Agari")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN ) PORT_NAME("Kan")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI ) PORT_NAME("Chi")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_MAHJONG_PON ) PORT_NAME("Pon")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Jog Dial") PORT_SENSITIVITY(10) PORT_KEYDELTA(4)

	// The optional AC adapter has no identified firmware-visible source selector.
	PORT_INCLUDE(xavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void xavix_epo_mj_state::xavix_epo_mj(machine_config &config)
{
	xavix_i2c_24c02(config);

	m_soc->set_external_map(*this, &xavix_epo_mj_state::xavix_2mb_extbus_map);

	// The jog dial is a writable signed counter; the game consumes one selection step per four counts.
	m_soc->anport().input_callback<0>().set_ioport("DIAL");
	m_soc->anport().set_counter_mode(0);
}

void xavix_epo_mj_state::io1_output_w(u8 data, u8 direction)
{
	m_i2cmem->write_sda(BIT(data | ~direction, 1));
	m_i2cmem->write_scl(BIT(data | ~direction, 0));
}


// Let's! TV Play Chou Ninki Spot! Korogashi-Houdai Tamagotchi Resort
static INPUT_PORTS_START( ltv_tam )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("MOUSE0X")
	// Relative scaling is provisional because the original counts per distance have not been measured.
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_NAME("P1 Trackball X") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1) PORT_CODE(MOUSECODE_X_INDEXED(0)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)
	PORT_MODIFY("MOUSE0Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_NAME("P1 Trackball Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1) PORT_CODE(MOUSECODE_Y_INDEXED(0)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)
	PORT_MODIFY("MOUSE1X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_NAME("P2 Trackball X") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2) PORT_CODE(MOUSECODE_X_INDEXED(1)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)
	PORT_MODIFY("MOUSE1Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_NAME("P2 Trackball Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2) PORT_CODE(MOUSECODE_Y_INDEXED(1)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Push Button / Confirm") PORT_PLAYER(2) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1))
	// The documented adapter and high-level idle policy support this identity, but a factory strap cannot be excluded.
	PORT_CONFNAME( 0x20, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x20, "AC Adapter" )
	PORT_BIT( 0xcf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Push Button / Confirm") PORT_PLAYER(1) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("tamagotchi", FUNC(xavix_tamagotchi_ir_device::data_r))
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's three threshold bands; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x10, "Low" )
	PORT_CONFSETTING(    0x18, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void xavix_ltv_tam_state::xavix_ltv_tam(machine_config &config)
{
	xavix_i2c_24c04_4mb(config);

	m_soc->anport().input_callback<0>().set_ioport("MOUSE0X");
	m_soc->anport().input_callback<1>().set_ioport("MOUSE0Y");
	m_soc->anport().input_callback<2>().set_ioport("MOUSE1X");
	m_soc->anport().input_callback<3>().set_ioport("MOUSE1Y");
	m_soc->anport().set_counter_mode(0);
	m_soc->anport().set_counter_mode(1, true);
	m_soc->anport().set_counter_mode(2);
	m_soc->anport().set_counter_mode(3, true);

	XAVIX_TAMAGOTCHI_IR(config, m_tamagotchi, peripheral_clock());
	m_tamagotchi->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<1>));
}

void xavix_ltv_tam_state::io0_output_w(u8 data, u8 direction)
{
	xavix_i2c_state::io0_output_w(data, direction);
	// The terminal auto-off signal is IO0.6; IO1.7 remains an input at shutdown.
	power_off_w(BIT(data & direction, 6));
	m_tamagotchi->tx_w(BIT(data & direction, 7));
}

void xavix_ltv_tam_state::io1_output_w(u8 data, u8 direction)
{
	m_i2cmem->write_sda(BIT(direction, 3) ? BIT(data, 3) : 1);
	m_i2cmem->write_scl(BIT(direction, 2) ? BIT(data, 2) : 0);

}


// TV de Asobitai Nihon Isshuu: Boku wa Plarail Untenshi
static INPUT_PORTS_START( tomplc )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	// Panel navigation keys must not also move the arrow-bound brake lever.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Doors / Right") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Wipers / Back")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Headlights / Left") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Horn / Select")

	// this is the up/down handle on the left (pull towards player to increase power)
	PORT_BIT( 0x30, 0x00, IPT_POSITIONAL_V ) PORT_POSITIONS(3) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_FULL_TURN_COUNT(3) PORT_NAME("Power")

	PORT_MODIFY("IN1")
	// this is the rotary lever (move anti-clockwise to the right to apply brakes)
	PORT_BIT( 0x03, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(3) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_FULL_TURN_COUNT(3) PORT_NAME("Brake")

	// The handheld switch pulls this line low while enabling ADC0 microphone sampling.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Push-to-Talk")

	// Usability hack for users without a host microphone; hold with Push-to-Talk.
	PORT_START("MIC_SIMULATE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Simulate Announcement") PORT_CODE(KEYCODE_N) PORT_WRITE_LINE_DEVICE_MEMBER("microphone_tone", FUNC(beep_device::set_state))

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's states; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x30, "Low" )

	// Product photographs show a latching ON/OFF slider.
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void xavix_i2c_power_state::xavix_tomplc(machine_config &config)
{
	xavix_i2c_24c02_4mb(config);
	configure_microphone(config);
	BEEP(config, "microphone_tone", 1000).add_route(0, m_soc->adc(), 0.5);
}


// TV de Asobitai Hyper Rescue: Boku wa Kyuujotai!
static INPUT_PORTS_START( tomthr )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// The forward/reverse lever and steering wheel reach the program as digital lines.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Move Forwards")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Move Backwards")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Steer Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Steer Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Horn / Select")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Ignition Key")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Map")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Wipers")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Headlights")
	// The radio-handset switch is electrically active low and gates ADC0 sampling.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Push-to-Talk")

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's states; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x10, "Low" )
	PORT_CONFSETTING(    0x14, "Very Low" )

	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix_power_state::xavix_tomthr(machine_config &config)
{
	xavix_4mb(config);
	configure_microphone(config);
}


// Gururin World
static INPUT_PORTS_START( epo_guru )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jump / Confirm")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_NAME("Direction Right")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_NAME("Direction Left")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_CONFNAME( 0x40, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x40, "AC Adapter" )
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's states; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x28, "Low" )
	PORT_CONFSETTING(    0x30, "Very Low" )

	PORT_MODIFY("MOUSE1X")
	// The free-spinning pedal supplies signed increments, and firmware clears its writable counter after sampling.
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Pedal Crank") PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_CODE_DEC(KEYCODE_A) PORT_CODE_INC(KEYCODE_D)
INPUT_PORTS_END

void xavix_epo_guru_state::xavix_epo_guru(machine_config &config)
{
	xavix_nv_4mb(config);

	m_soc->anport().input_callback<2>().set_ioport("MOUSE1X");
	m_soc->anport().set_counter_mode(2);

	// A field-rate IO1.7 heartbeat maintains the external RC power hold.
	configure_power_hold(config);
}

void xavix_epo_guru_state::machine_reset()
{
	xavix_state::machine_reset();
	m_motor_drive[0] = 0;
	m_motor_drive[1] = 0;
}

void xavix_epo_guru_state::io0_output_w(u8 data, u8 direction)
{
	// IO0.6/7 drive the pedal-resistance motor's opposite polarities; physical torque is unmeasured.
	m_motor_drive[0] = BIT(data & direction, 6);
	m_motor_drive[1] = BIT(data & direction, 7);
}


// Play TV Rescue Heroes
static INPUT_PORTS_START( rad_rh )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	// The maintained ignition switch is outside the all-output GPIO matrix.
	PORT_INCLUDE(xavix_hard_power_control)
	PORT_MODIFY("POWER")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Ignition Switch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_state::power_off), 0)

	PORT_INCLUDE(xavix_reset_control)

	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu") // Hold with Joystick Right while resetting to enter the internal diagnostics.
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_2WAY PORT_NAME("Steering Wheel Left")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_2WAY PORT_NAME("Steering Wheel Right")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY PORT_NAME("Cannon Down")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY PORT_NAME("Cannon Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_NAME("Cannon Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY PORT_NAME("Cannon Left")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Rescue Button") // Also sounds the firetruck siren.
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Throttle Forward")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Throttle Reverse")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

u8 xavix_rad_rh_state::io0_input_r(u8 direction)
{
	// IO0 bit 7 and IO1 bit 5 alternately select two active-high control rows.
	bool const row0 = BIT(m_soc->io().data_latch<0>() & direction, 7);
	bool const row1 = BIT(m_soc->io().data_latch<1>(), 5);

	if (row0 && !row1)
		return m_matrix[0]->read();
	if (!row0 && row1)
		return m_matrix[1]->read();

	return 0;
}


//**************************************************************************
//  ROM Definitions
//**************************************************************************

ROM_START( taitons1 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "taitonostalgia1.u3", 0x000000, 0x200000, CRC(25bd8c67) SHA1(a109cd2da6aa4596e3ca3abd1afce2d0001a473f) )
ROM_END

ROM_START( taitons2 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "taitonostalgia2.bin", 0x000000, 0x200000, CRC(d7dbd93d) SHA1(ad96f80d317e7fd64682a1fe406c5ee9dd5eabf9) )
ROM_END

ROM_START( namcons1 )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "namconostalgia1.bin", 0x000000, 0x100000, CRC(9bcccccd) SHA1(cf8fe6de76fbd23974f999299db6f558f79c8f22) )
ROM_END

ROM_START( namcons2 )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "nostalgia.bin", 0x000000, 0x100000, CRC(03f7f755) SHA1(bdf1b10ab0104ed580951b0c428c4e93e7373afe) )
ROM_END

ROM_START( rad_box )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("boxing.bin", 0x000000, 0x200000, CRC(5cd40714) SHA1(165260228c029a9502ca0598c84c24fd9bdeaebe) )
ROM_END

ROM_START( rad_boxp )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("boxing.bin", 0x000000, 0x200000, CRC(5cd40714) SHA1(165260228c029a9502ca0598c84c24fd9bdeaebe) )
ROM_END

ROM_START( rad_bass )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("bassfishin.bin", 0x000000, 0x100000, CRC(b54eb1c5) SHA1(084faa9349369f2b8846950765f9c8f758db3e9e) )
ROM_END

ROM_START( rad_bassp )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("bassfishin.bin", 0x000000, 0x100000, CRC(b54eb1c5) SHA1(084faa9349369f2b8846950765f9c8f758db3e9e) )
ROM_END

ROM_START( rad_opus )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("opus.bin", 0x000000, 0x100000, CRC(f84c11c5) SHA1(6e34129d0ca9c52b7c342fc94860629d81705523) )
ROM_END

ROM_START( rad_opusp )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("opus.bin", 0x000000, 0x100000, CRC(f84c11c5) SHA1(6e34129d0ca9c52b7c342fc94860629d81705523) )
ROM_END

ROM_START( rad_opusa )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("opus_alt.bin", 0x000000, 0x100000, CRC(509df402) SHA1(c5b863670eac8498ddda9dfde91387634cf7aa9f) )
ROM_END

ROM_START( rad_hnt )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("huntin1.bin", 0x000000, 0x100000, CRC(e51e250f) SHA1(d72199096d466cd344bb243ef1228e0df9501d00) )
ROM_END

ROM_START( rad_hnt2 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("huntin2.bin", 0x000000, 0x200000, CRC(fb6846df) SHA1(267632790ed42eba7ef1517b86b024799a78839d) )
ROM_END


ROM_START( rad_snow )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("snoblu.bin", 0x000000, 0x100000, CRC(593e40b3) SHA1(03483ac39eddd7746470fb60018e704382b0da59) )
ROM_END

ROM_START( rad_snowp )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("snoblu.bin", 0x000000, 0x100000, CRC(593e40b3) SHA1(03483ac39eddd7746470fb60018e704382b0da59) )
ROM_END


ROM_START( rad_ping )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "pingpong.bin", 0x000000, 0x100000, CRC(629f7f47) SHA1(2bb19fd202f1e6c319d2f7d18adbfed8a7669235) )
ROM_END

ROM_START( rad_pingp )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "pingpong.bin", 0x000000, 0x100000, CRC(629f7f47) SHA1(2bb19fd202f1e6c319d2f7d18adbfed8a7669235) )
ROM_END

ROM_START( rad_crdn )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "cardnight.bin", 0x000000, 0x100000, CRC(d19eba08) SHA1(cedb9fe785f2a559f518a1d8ecf80d500ddc63c7) )
ROM_END

ROM_START( rad_crdnp )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "cardnight.bin", 0x000000, 0x100000, CRC(d19eba08) SHA1(cedb9fe785f2a559f518a1d8ecf80d500ddc63c7) )
ROM_END

ROM_START( rad_bb )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "radicabaseball.bin", 0x000000, 0x100000, CRC(3fa6f490) SHA1(0772156a67a22d06f5ffd6d1a77f6dc867d0a6d2) )
ROM_END

ROM_START( rad_bb2 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "baseball2.bin", 0x000000, 0x200000, CRC(bdbf6202) SHA1(18d5cc2d77cbb734629a7a5b6e0f419d21beedbd) )
ROM_END

ROM_START( rad_mtrk )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "monstertruck.bin", 0x000000, 0x400000, CRC(dccda0a7) SHA1(7953cf29643672f8367639555b797c20bb533eab) )
ROM_END

ROM_START( rad_mtrkp ) // rom was dumped from NTSC unit, assuming to be the same
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "monstertruck.bin", 0x000000, 0x400000, CRC(dccda0a7) SHA1(7953cf29643672f8367639555b797c20bb533eab) )
ROM_END

ROM_START( rad_ssx )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("snowssx.bin", 0x000000, 0x400000, CRC(108e19a6) SHA1(3dfb18efb6331b96a53138a5ba29dae9cd966e90) )
ROM_END

ROM_START( rad_ssxp )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("snowssx.bin", 0x000000, 0x400000, CRC(108e19a6) SHA1(3dfb18efb6331b96a53138a5ba29dae9cd966e90) )
ROM_END

ROM_START( rad_sbw )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("snowbwhite.bin", 0x000000, 0x400000, CRC(640c1473) SHA1(d37d1484a5b14735b35afbca305dad7d178b08a2) )
ROM_END

ROM_START( rad_bdp )
	ROM_REGION(0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("barbiepad.bin", 0x000000, 0x200000, CRC(48731512) SHA1(377d4e1c98cafcd9d5e1ee27943289d250a6e7a9) )
ROM_END

ROM_START( rad_madf )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("madden.bin", 0x000000, 0x400000, CRC(e972fdcf) SHA1(52001316254880755da959c3441d232fd2c72c7a) )
ROM_END

ROM_START( rad_fb )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("rfootball.bin", 0x000000, 0x400000, CRC(025e0cb4) SHA1(60ce363de236d5119d078e346ad5d2ae50dbc7e1) )
ROM_END

ROM_START( epo_fish )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("fish.bin", 0x000000, 0x200000, CRC(72392caf) SHA1(16a65c79ff7e3b5f5f514a024bd652412ed38b74) )
ROM_END

ROM_START( epo_efdx )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("excitefishing.bin", 0x000000, 0x400000, CRC(9c85b261) SHA1(6a363faed2ec89c5176e46554a98ca1e20132579) )
ROM_END

ROM_START( epo_esdx )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("baseballdx.bin", 0x000000, 0x400000, CRC(fe2e832e) SHA1(e6343f5e5f52316538d918d0d67c15764aa40f65) )
ROM_END

ROM_START( epo_stad )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("excitestadium.bin", 0x000000, 0x200000, CRC(b58035b9) SHA1(1382a9e42ff932e7ec2186b210917bcf5c571b86) )
ROM_END

ROM_START( epo_esht ) // ESTJ-MAIN REV:0 PCB
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("tigers.u3", 0x000000, 0x400000, CRC(51a17ef3) SHA1(864190e91775716218be3ac0699570844d67d3e7) )
ROM_END

ROM_START( epo_epp )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("excitepingpong.bin", 0x000000, 0x100000, CRC(1fdb9cbd) SHA1(8ed0c1f6d2708ab6e79f0b9553e587c6446e8338) )
ROM_END

ROM_START( epo_epps )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("excitepingpong_special.bin", 0x000000, 0x100000, CRC(d59c4b44) SHA1(5a48b9046a1d1beb10972fc1d41d6d63fe829465) )
ROM_END

ROM_START( epo_eppk )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("sonokongpingpong.bin", 0x000000, 0x100000, CRC(ea81ced6) SHA1(ef8961d3670148501a478c17cd09f5088e32ad41) )
ROM_END

ROM_START( epo_epp2 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("excitepingpong2.u3", 0x000000, 0x200000, CRC(4b70012a) SHA1(4dd80472067027be5a416ec953f4ed6e7df5fc25) )
ROM_END

ROM_START( epo_epp3 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("excitepingpong3.bin", 0x000000, 0x200000, CRC(a2ee8bff) SHA1(6e16dbaac9680e1f311c08e3f573d0cf8708b446))
ROM_END

ROM_START( epo_mj ) // MARJ MAIN-11A
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("marj.u4", 0x000000, 0x200000, CRC(4e496424) SHA1(9661130ad8315082bd211b3817e5830b576907a8) )
ROM_END

ROM_START( rad_socr )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("soccer.u5", 0x000000, 0x100000, CRC(0564a766) SHA1(bded4f2e0d059731164c35de7df62ef411402901) )
	ROM_LOAD("soccer.u4", 0x400000, 0x200000, CRC(e0e2c488) SHA1(907511d945dec51782af268a94b372dfeb7517c5) )
ROM_END

ROM_START( epo_strk )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("excitestriker.u5", 0x000000, 0x100000, CRC(fa3f95fd) SHA1(d3bc7f2f321b81a7783887b29218bf711c0d5518) )
	ROM_LOAD("excitestriker.u4", 0x400000, 0x200000, CRC(0f37ca15) SHA1(b35aaf7f5adddba57c898538f157146bb4d1f9b4) )
ROM_END

ROM_START( epo_strkp )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("excitestriker_uk.u5", 0x000000, 0x100000, CRC(201e150e) SHA1(f11405a4b7349e4bdfeb04110b1b6f67e0b63f64) )
	ROM_LOAD("excitestriker_uk.u4", 0x400000, 0x200000, CRC(efd648aa) SHA1(59358b9a9df27dadc33ed4c3da9083f739b68340) )
ROM_END

// Startup object 04:aa62 appears to have six extra bytes before its graphics fields; firmware consequently attempts ROM writes.
ROM_START( rad_jcon )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("jrconstr_u5.bin", 0x000000, 0x200000, CRC(306699d2) SHA1(121b04bb598d4531db53e497be9270449bb8ffe4) )
	ROM_LOAD("jrconstr_u4.bin", 0x400000, 0x100000, CRC(4cfb6193) SHA1(0f56dfc91db3cc066836daf37ff46f9ff5ec5f94) )
ROM_END

ROM_START( tak_town )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("rom.u5", 0x000000, 0x200000, CRC(46587e78) SHA1(f163492139087f7aaeea6448b652f58afcb4dd5c) )
	ROM_LOAD("rom.u4", 0x400000, 0x100000, CRC(642e7e6a) SHA1(d8ff88ad20a60d2eb702dd353f307aa3fcfbbdd5) )
ROM_END

ROM_START( epo_guru )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("gururinworld.bin", 0x000000, 0x400000, CRC(e5ae4523) SHA1(0e39ef8f94203d34e49422081667805f50a339a1) )
ROM_END

ROM_START( epo_dmon ) // Doraemon has a phototransistor, microphone and 24LC02, PCB says XaviX 2002
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("doraemon.bin", 0x000000, 0x400000, CRC(d8f5412f) SHA1(11a4c017ed400f7aa585be744d9693efe734c192) )
ROM_END

ROM_START( rad_rh )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("rescueheroes.bin", 0x000000, 0x200000, CRC(38c391a7) SHA1(120334d4ce89d98438c2a35bf7e53af5096cc878) )
ROM_END

ROM_START( has_wamg )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "minigolf.bin", 0x000000, 0x400000, CRC(35cee2ad) SHA1(c7344e8ba336bc329638485ea571cd731ebf7649) )
ROM_END

ROM_START( ltv_tam )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("letstvtamagotchi.bin", 0x000000, 0x400000, CRC(e3723272) SHA1(e72e128a7a24afb96baafe5f13e13a0d74db4033) )
ROM_END

ROM_START( epo_crok )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("sgm3244.u2", 0x000000, 0x400000, CRC(a801779b) SHA1(e6e4235dc7c7db3073737b10ba4bc5b00deca2c3) )
ROM_END

ROM_START( epo_mms )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("mmnj-main-4.u4", 0x000000, 0x400000, CRC(22f14ca2) SHA1(122e735eb7c54a22de16f65cd43d2cae788e0102) )
ROM_END

ROM_START( tak_zuba ) // CHBJ MAIN REV:01
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("chbj.u2", 0x000000, 0x400000, CRC(6d60c8d2) SHA1(ba687fc95503223dd484ed9533dcb097ecfea00d) )
ROM_END

ROM_START( epo_mmsp )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("mm1j main-00.u1", 0x000000, 0x400000, CRC(65b40a27) SHA1(8f88973122277fe8f31bacb3a070609fde062946) )
ROM_END

ROM_START( ban_krrj ) // KRRJ MAIN PCB 01
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("krrj.u2", 0x000000, 0x400000, CRC(16f0fe78) SHA1(2b18cf6336e5adc64d4db0914788c159e60d91a2) )
ROM_END

ROM_START( tak_geig )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("geigeki.bin", 0x000000, 0x400000, CRC(bd0c3576) SHA1(06f614dbec0225ce4ed866b98450912986d72faf) )
ROM_END

ROM_START( tak_wdg )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("wdgrobot.bin", 0x000000, 0x200000, CRC(7ffc6386) SHA1(e33de5f8e6686e4160d1b90d59167418e87f5a47) )
ROM_END

ROM_START( tom_tvho ) // ET105 REV 0.0
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("tvhockey.u4", 0x000000, 0x200000, CRC(9cd72ae2) SHA1(0530851123b607ddb85f9513405ce97c493f5fd6) )
ROM_END

ROM_START( tak_comt )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("comet.u3", 0x000000, 0x200000, CRC(407c5566) SHA1(41d73c34af8cc3d07a34fcac0bc1856442c94200) )
ROM_END

ROM_START( jarajal )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("takaraslots.bin", 0x000000, 0x200000, CRC(afae0b72) SHA1(b1c5d80a8dc8466982efd79d54cd82a58f0ff997) )
ROM_END

ROM_START( tomshoot )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("airgun.u4", 0x000000, 0x200000, CRC(3e4f7b65) SHA1(4e1660d4952c498e250526c2c3f027253e1fcbe1) )
ROM_END

ROM_START( tcarnavi )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("navi.bin", 0x000000, 0x400000, CRC(f4e693fb) SHA1(be37b35f1e1e661e10187253c2c3aa9858a90812) )
ROM_END

ROM_START( tomcpin )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("championpinball.bin", 0x000000, 0x400000, CRC(24f6d753) SHA1(3d3b39692bef8156da9e350b456c4e2f0af74484) )
ROM_END

ROM_START( tomplc )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("imaplayrailconductor.bin", 0x000000, 0x400000, CRC(b775d0ed) SHA1(33142509b11bbe45b0b9222232033dd64ef01ff2) )
ROM_END

ROM_START( tomthr ) // THRJ MAIN PCB   REV 0.6
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("thrj.u4", 0x000000, 0x400000, CRC(a7e8dc74) SHA1(676b2a905b757356c6c1dfe3f10148484caa44c5) )
ROM_END

ROM_START( gungunad )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	// some lines were swapped going into the die, but it is unclear if they're swapped back inside it
	ROM_LOAD("gga.bin", 0x000000, 0x40000, CRC(5252b6bb) SHA1(8a9f920e4bccabbd337f37a838af574e2b16746f) )
	ROM_CONTINUE(0x080000,0x040000)
	ROM_CONTINUE(0x040000,0x040000)
	ROM_CONTINUE(0x0c0000,0x040000)

	ROM_CONTINUE(0x100000,0x040000)
	ROM_CONTINUE(0x180000,0x040000)
	ROM_CONTINUE(0x140000,0x040000)
	ROM_CONTINUE(0x1c0000,0x040000)
ROM_END


ROM_START( gungunrv )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("gungunrevolution.u1", 0x000000, 0x400000, CRC(4e34f624) SHA1(7acdd0991df78ecffd156381817ed4f85f6aef09) )
ROM_END

ROM_START( bistro )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("bistro.u2", 0x000000, 0x200000, CRC(40865e05) SHA1(597a615c61f29c6f6e7ce997a229175cb151242f) )
ROM_END

/*
    The e-kara cartridges require the BIOS rom to map into 2nd external bus space as they fetch palette data from
    it etc.
*/

ROM_START( ekara )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekara.bin", 0x600000, 0x100000, CRC(9b27c4a2) SHA1(d75dda7434933135d2f7e353840a9384e9a0d586) )
	ROM_RELOAD(0x000000, 0x100000)
ROM_END

ROM_START( ekaraa )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekara2.bin", 0x600000, 0x100000, CRC(3c92d48d) SHA1(450fbe53826cdb87ec797f84b9757987afcc1ec5) )
	ROM_RELOAD(0x000000, 0x100000)
ROM_END

ROM_START( ekaraj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekarajapan.bin", 0x600000, 0x100000, CRC(e459e43b) SHA1(58b7f36a81571a2df5e812c118fdf68812a05abc) )
	ROM_RELOAD(0x000000, 0x100000)
ROM_END

ROM_START( ekaraphs )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekaraheadset.bin", 0x600000, 0x200000, CRC(dd9b3cd7) SHA1(baaf35d56fa45b6f995b8466331bb30f0035f734) )
	ROM_RELOAD(0x000000, 0x200000)
ROM_END

ROM_START( ekarag )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekaragermany.bin", 0x600000, 0x200000, CRC(644f06b0) SHA1(433e65a6ea029b67ee62b4b3de52ff3504cfdf16) )
	ROM_RELOAD(0x000000, 0x200000)
ROM_END

ROM_START( ekaras )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekaraspanish.bin", 0x600000, 0x200000, CRC(77925ab0) SHA1(402053e22e371c5f5e8069a4bf3b33841e742375) )
	ROM_RELOAD(0x000000, 0x200000)
ROM_END

ROM_START( isinger )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "kr_isinger.bin", 0x600000, 0x100000, CRC(6ad588fe) SHA1(879da069ca8c126232a1b2dfcce25c8615540e48) )
	ROM_RELOAD(0x000000, 0x100000)
ROM_END

ROM_START( epitch )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekarajapan.bin", 0x600000, 0x100000, CRC(e459e43b) SHA1(58b7f36a81571a2df5e812c118fdf68812a05abc) )
	ROM_RELOAD(0x000000, 0x100000)
ROM_END

ROM_START( ekaramix )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	// also has a MX87L100C 28-pin chip in the unit, possibly the USB controller.  Part of the ROM contains a FAT filesystem that could possibly appear as a USB drive on the PC?
	ROM_LOAD( "ekaramix.bin", 0x600000, 0x200000, CRC(ee71576e) SHA1(26f8c9edcbbed77e86a1cb5a0b91c92a16fef433) )
	ROM_RELOAD(0x000000, 0x200000)
ROM_END

ROM_START( hikara )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "hikara.u3", 0x000000, 0x100000, CRC(6b91102a) SHA1(684dcfeaa8ac2888da2055617603494ce5fed93c) )
	ROM_RELOAD(0x600000, 0x100000)
ROM_END

ROM_START( ddrfammt )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekara_ddr_ha010_81947.bin", 0x600000, 0x200000, CRC(737d5d1a) SHA1(a1043047056dd27bca69767ee2044461ec549465) )
	ROM_RELOAD(0x000000, 0x200000)
ROM_END

ROM_START( popira )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "popira.bin", 0x600000, 0x100000, CRC(71d20afc) SHA1(fb2ae023fc8a188c79b2d6550f737e06c05e12da) )
	ROM_RELOAD(0x000000, 0x100000)
ROM_END

ROM_START( popirak )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "kr_main.u2", 0x600000, 0x100000, CRC(785d8e21) SHA1(7877ac33b57bdd8ceac6bda3b0006a3274ad4dce) )
	ROM_RELOAD(0x000000, 0x100000)
ROM_END

ROM_START( popira2 )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "popira2.bin", 0x600000, 0x200000, CRC(6acd8be0) SHA1(8b6c812d37e2dc2f2531b0e47d84ad8485729dc5) )
	ROM_RELOAD(0x000000, 0x200000)
ROM_END

ROM_START( taikodp )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "taikodepopira.bin", 0x600000, 0x200000, CRC(037a8472) SHA1(03cae465965935fc084fb906f8f5de7679f42dd1) )
	ROM_RELOAD(0x000000, 0x200000)
ROM_END

ROM_START( jpopira )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "jumpingpopira.bin", 0x600000, 0x200000, CRC(a7bedbd2) SHA1(d62d4ca660c8df14891217fb7b7a2b4a931ff35f) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 ) // maybe we can remove this eventually, but for now it won't init without a reset between
	ROM_LOAD( "i2cmem.bin", 0x000, 0x100, CRC(70a05af1) SHA1(e8f4ab51445777fe459f9ff09333f548c4e3507c) )
ROM_END

ROM_START( evio )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "evio.bin", 0x600000, 0x200000, CRC(ee22c764) SHA1(f2b7e213eb78065a63ef484a619bcfc61299e30e))
	ROM_RELOAD(0x000000, 0x200000)
ROM_END

// 爆闘宣言ダイガンダー
ROM_START( tak_daig )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "robottransformer_unit.bin", 0x600000, 0x200000, CRC(02deddaa) SHA1(7d7b54684f8b0b92daf02738560779c7df627b91) )
	ROM_RELOAD(0x000000, 0x200000)
ROM_END

// ASKJ MAIN-09 PCB
ROM_START( epo_quiz )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "quizmaster.u1", 0x000000, 0x400000, CRC(e91868b8) SHA1(0128603d755731dafe328b142292dc6e5fe00d78) )
ROM_END


// ガチンコ勝負！ パチスロTV
ROM_START( gcslottv )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "sammyslotunit.bin", 0x000000, 0x200000, CRC(2ba6f3ab) SHA1(1c7fc0c85d817db1550d40c0258f424770e0bd81) )
ROM_END

ROM_START( epo_tfp2 )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "funpark2.u1", 0x000000, 0x400000, CRC(97ad5183) SHA1(77310b42d0a015838a1cef4eb5e74cc8335284d1) )
ROM_END

ROM_START( epo_tp2s ) // TF2J MAIN_01 REV:05
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "tf2j.u3", 0x000000, 0x400000, CRC(db2f124c) SHA1(fd60d4560ed53c63f95cf70a7d1ef13d1ecd1f42) )
ROM_END

ROM_START( epo_tp2p ) // TF3J MAIN PCB 01
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "grandslam.u3", 0x000000, 0x800000, CRC(d458ee01) SHA1(dd1a85d822121c46f89ca013fa56c482ab411c6e) )
ROM_END

ROM_START( epo_tenn )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD( "excitetennis.u4", 0x000000, 0x100000, CRC(10b0e1dd) SHA1(ba438201434f2b51792b119a3e3d07cc3e53b89a) )
	ROM_LOAD( "excitetennis.u2", 0x400000, 0x200000, CRC(6c2cdc90) SHA1(3c5b391e5e7b4a9a73038ef619df564143724437) )
ROM_END

ROM_START( epo_hamd )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD( "hamster.u2", 0x000000, 0x100000, CRC(6c2d9d98) SHA1(89a8e6d236ea3dadb882e3ecf12e41bd50222710) )
	ROM_LOAD( "hamster.u3", 0x400000, 0x200000, CRC(e437c8d0) SHA1(f57c54a73ed38826f4b98610a0aa1f15cf95614d) )
ROM_END

ROM_START( tvpc_tom )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "tvpc_thomas.u1", 0x000000, 0x400000, CRC(507f334e) SHA1(d66efd13f166fcd2a66133dc981c8a67b2a26d5f) )
ROM_END

ROM_START( tvpc_dor )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "tvpc_doreamon.u3", 0x000000, 0x400000, CRC(6f2edbb2) SHA1(98fa86f85e00aa40e7a585ff0bc930cb5ca88362) )
ROM_END

ROM_START( tvpc_ham )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "hpcj.u3", 0x000000, 0x400000, CRC(76e8c854) SHA1(5998c03292a16107d0d7ae00f77677582680f323) )
ROM_END

ROM_START( tvpc_hk )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "kpcj.u2", 0x000000, 0x400000, CRC(87fc2f73) SHA1(29a284b907abec175d4289d290490af17a2a963f) )
ROM_END

ROM_START( epo_mail )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "mailcot.u3", 0x000000, 0x400000, CRC(87f13ce7) SHA1(97326257210df411ac8b47e8513bb0da7b02dbde) )

	// has a HT24LC16 in the main unit

	// Removable mascots contain an I2C EEPROM for data transfer between units, with no additional ROM.
ROM_END


ROM_START( tak_gin )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "snowboard.bin", 0x000000, 0x200000, CRC(79fdeae3) SHA1(ab08790e95cdccf3541ecbddb87ebf0dedb3718b) )
ROM_END

ROM_START( tak_hamr ) // HAMJ MAIN on PCB
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "hamj.u4", 0x000000, 0x400000, CRC(2f4f5270) SHA1(dfb75f0d20247cf1c886840149c7cf91780ae1b9) )
ROM_END

ROM_START( tak_beyb )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "beyblade.u2", 0x000000, 0x200000, CRC(bcf6b3a7) SHA1(1c80f1241138b9d7816f1e5285ff8f3c61739c95) )
ROM_END

ROM_START( tak_beyu )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "beybladeus.u2", 0x000000, 0x200000, CRC(f72eb949) SHA1(20ef0411610d490d07a0924ba111ddfadcbaf407) )
ROM_END

ROM_START( hippofr )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "54-6447-010.u3", 0x000000, 0x200000, CRC(1fb15364) SHA1(ff2bb54f7d6ccd3c83e722599c6f2b213bf35df8) )
ROM_END

ROM_START( multiplt )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "2en1multipilotefr.u2", 0x000000, 0x200000, CRC(77f2ebf6) SHA1(0181bc7d3b6ea5a4a247ffe02fee2cb4942c1623) )
ROM_END

ROM_START( pl1000 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	// code is expanded from 'plainrom' region below

	ROM_REGION( 0x100000, "plainrom", ROMREGION_ERASE00 )
	ROM_LOAD( "video_challenger_pl-1000.u2", 0x000000, 0x100000, CRC(039e6f07) SHA1(d554976913beaa03828bd920d24ede6e433a8b4d) )
ROM_END

//**************************************************************************
//  System Registrations
//**************************************************************************

/* XaviX hardware titles (1st Generation)

    These use
     SSD 98 PL7351-181
     SSD 98 PA7351-107
     SSD 97 PA7270-107
    type CPUs

    The SSD 97/98 CPU adds CALLF ($22) and RETF ($80) to the 6502 instruction set.

*/

CONS( 1999,  pl1000,    0,        0, xavix_pl1000,        pl1000,    xavix_pl1000_state,           init_pl1000, "Pelican Accessories / Mani Industries / Gameone Systems", "Video Challenger (PL-1000)", 0 )

/* Product code 80-32705. Some sites say 1997, but 1999 is what SSD had listed, and seems more fitting.
   also exists as 80-32703 Hippo's Alphabet Adventure (UK), and 80-32700 ABC Jungle Fun (US?) */
CONS( 1999,  hippofr,   0,        0, xavix_2mb,           hippofr,   xavix_hippofr_state,          empty_init,  "VTech", "2 en 1 Hippo: et la formidable aventure des lettres (France)", MACHINE_NOT_WORKING )

CONS( 1999?, multiplt,  0,        0, xavix_multiplt,      multiplt,  xavix_multiplt_state,         empty_init,  "VTech", "2 en 1 Multi'Pilote (France)", MACHINE_NOT_WORKING )

// Let's!TVプレイCLASSIC タイトーノスタルジア1
CONS( 2006,  taitons1,  0,        0, xavix_i2c_24c04_2mb, nostalgia, xavix_i2c_state,              empty_init,  "Bandai / SSD Company Ltd. / Taito", "Let's! TV Play Classic: Taito Nostalgia 1 (Japan)", 0 )

// Let's!TVプレイCLASSIC タイトーノスタルジア2
CONS( 2006,  taitons2,  0,        0, xavix_i2c_24c04_2mb, nostalgia, xavix_i2c_state,              empty_init,  "Bandai / SSD Company Ltd. / Taito", "Let's! TV Play Classic: Taito Nostalgia 2 (Japan)", 0 )

// Let's!TVプレイCLASSIC ナムコノスタルジア1
CONS( 2006,  namcons1,  0,        0, xavix_i2c_24c04_1mb, nostalgia, xavix_i2c_state,              empty_init,  "Bandai / SSD Company Ltd. / Namco", "Let's! TV Play Classic: Namco Nostalgia 1 (Japan)", 0 )

// Let's!TVプレイCLASSIC ナムコノスタルジア2
CONS( 2006,  namcons2,  0,        0, xavix_i2c_24c04_1mb, nostalgia, xavix_i2c_state,              empty_init,  "Bandai / SSD Company Ltd. / Namco", "Let's! TV Play Classic: Namco Nostalgia 2 (Japan)", 0 )

CONS( 2000,  rad_ping,  0,        0, xavix_ir_1mb,        rad_ping,  xavix_ir_power_state,         empty_init,  "Radica / SSD Company Ltd. / Simmer Technology", "Play TV Ping Pong (NTSC)", MACHINE_NOT_WORKING ) // "Simmer Technology" is also known as "Hummer Technology Co., Ltd"
CONS( 2000,  rad_pingp, rad_ping, 0, xavixp_ir_1mb,       rad_pingp, xavix_ir_power_state,         empty_init,  "Radica / SSD Company Ltd. / Simmer Technology", "Connectv Table Tennis (PAL)", MACHINE_NOT_WORKING )

// this set reads the region byte and will show either 'RADICA: Play TV Opus' or 'RADICA: Connectv Opus' as the title
CONS( 2000,  rad_opus,  0,        0, xavix_nv_1mb,        rad_opus,  xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Play TV Opus (NTSC)", 0 )
CONS( 2000,  rad_opusp, rad_opus, 0, xavixp_nv_1mb,       rad_opusp, xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Connectv Opus (PAL)", 0 )
// the set below does NOT read the PAL/NTSC flag, and will only display 'RADICA: Plug & Play Opus' as the title
// older release, or region where the Play TV / Connectv trademarks weren't used?
CONS( 2000,  rad_opusa, rad_opus, 0, xavix_nv_1mb,        rad_opus,  xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Plug & Play Opus (NTSC)", 0 )

CONS( 2000,  rad_hnt,   0,        0, xavix_rad_hnt,       rad_hnt,   xavix_rad_hnt_state,          empty_init,  "Radica / Human Code Presage Studios / SSD Company Ltd.", "Play TV Buckmasters Huntin' (NTSC)", MACHINE_NOT_WORKING ) // box shows "Play TV"; title screen shows "Plug & Play"

CONS( 2003,  rad_hnt2,  0,        0, xavix_rad_hnt2,      rad_hnt,   xavix_rad_hnt_state,          empty_init,  "Radica / SSD Company Ltd.", "Play TV Buckmasters Huntin' 2 (NTSC)", MACHINE_NOT_WORKING )

CONS( 2003,  rad_mtrk,  0,        0, xavix_rad_mtrk,      rad_mtrk,  xavix_power_state,            empty_init,  "Radica / Sennari Interactive / SSD Company Ltd.", "Play TV Monster Truck (NTSC)", MACHINE_NOT_WORKING )
CONS( 2003,  rad_mtrkp, rad_mtrk, 0, xavix_rad_mtrkp,     rad_mtrkp, xavix_power_state,            empty_init,  "Radica / Sennari Interactive / SSD Company Ltd.", "Connectv Monster Truck (PAL)", MACHINE_NOT_WORKING )

CONS( 200?,  rad_box,   0,        0, xavix_2mb,           rad_box,   xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Play TV Boxin' (NTSC)", MACHINE_NOT_WORKING )
CONS( 200?,  rad_boxp,  rad_box,  0, xavixp_2mb,          rad_boxp,  xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Connectv Boxin' (PAL)", MACHINE_NOT_WORKING )

CONS( 200?,  rad_crdn,  0,        0, xavix_1mb,           rad_crdn,  xavix_io0_matrix_power_state, empty_init,  "Radica / SSD Company Ltd.", "Play TV Card Night (NTSC)", 0 )
CONS( 200?,  rad_crdnp, rad_crdn, 0, xavixp_1mb,          rad_crdnp, xavix_io0_matrix_power_state, empty_init,  "Radica / SSD Company Ltd.", "Connectv Card Night (PAL)", 0 )

CONS( 2000,  rad_bb,    0,        0, xavix_rad_bb,        rad_bb,    xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Play TV Baseball (NTSC)", MACHINE_NOT_WORKING ) // Play TV branding used on box, not ingame

CONS( 2001,  rad_bass,  0,        0, xavix_1mb,           rad_bass,  xavix_io0_matrix_power_state, empty_init,  "Radica / Human Code Presage Studios / SSD Company Ltd.", "Play TV Bass Fishin' (NTSC)", MACHINE_NOT_WORKING )
CONS( 2001,  rad_bassp, rad_bass, 0, xavixp_1mb,          rad_bassp, xavix_io0_matrix_power_state, empty_init,  "Radica / Human Code Presage Studios / SSD Company Ltd.", "Connectv Bass Fishin' (PAL)", MACHINE_NOT_WORKING )

// there is another 'Snowboarder' with a white coloured board, it appears to be a newer game closer to 'SSX Snowboarder' but without the SSX license.
CONS( 2001,  rad_snow,  0,        0, xavix_nv_1mb,        rad_snow,  xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Play TV Snowboarder (Blue) (NTSC)", 0 )
CONS( 2001,  rad_snowp, rad_snow, 0, xavixp_nv_1mb,       rad_snowp, xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Connectv Snowboarder (Blue) (PAL)", 0 )

CONS( 2003,  rad_madf,  0,        0, xavix_rad_madf,      rad_fb,    xavix_power_state,            empty_init,  "Radica / Electronic Arts / SSD Company Ltd.", "EA Sports Madden Football (NTSC)", MACHINE_NOT_WORKING ) // no Play TV branding, USA only release?

CONS( 2005,  rad_fb,    0,        0, xavix_rad_madf,      rad_fb,    xavix_power_state,            empty_init,  "Radica / FarSight Studios / SSD Company Ltd.", "Play TV Football (NTSC)", MACHINE_NOT_WORKING ) // USA only release? doesn't change logo for PAL.

// fails to display any kind of XaviX logo or SSD Copyright, it is the only XaviX based game so far to not show these details anywhere in the game.
CONS( 2003,  rad_rh,    0,        0, xavix_2mb,           rad_rh,    xavix_rad_rh_state,           empty_init,  "Radica / Fisher-Price / Pronto Games / SSD Company Ltd.", "Play TV Rescue Heroes (NTSC)", MACHINE_NOT_WORKING )

CONS( 2004,  rad_ssx,   0,        0, xavix_4mb,           rad_ssx,   xavix_power_state,            empty_init,  "Radica / Electronic Arts / SSD Company Ltd.", "Play TV SSX Snowboarder (NTSC)", 0 )
CONS( 2004,  rad_ssxp,  rad_ssx,  0, xavixp_4mb,          rad_ssxp,  xavix_power_state,            empty_init,  "Radica / Electronic Arts / SSD Company Ltd.", "Connectv SSX Snowboarder (PAL)", 0 )

// basically a reissue of SSX but without the license - doesn't exist with Connectv branding?
CONS( 2006,  rad_sbw,   0,        0, xavix_4mb,           rad_sbw,   xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Play TV Snowboarder (White) (NTSC)", 0 )

CONS( 2002,  rad_bdp,   0,        0, xavix_2mb,           rad_bdp,   xavix_power_state,            empty_init,  "Radica / Mattel / SSD Company Ltd.", "Barbie Dance Party", 0 )

// ゴール決めるぜ！ エキサイトストライカー
CONS( 2001,  epo_strk,  0,        0, xavix_epo_strk,      epo_strk,  xavix_ir_power_state,         empty_init,  "Epoch / SSD Company Ltd.", "Goal Kimeruze! Excite Striker (Japan)", MACHINE_NOT_WORKING )
CONS( 2002,  epo_strkp, epo_strk, 0, xavixp_epo_strk,     epo_strkp, xavix_ir_power_state,         empty_init,  "Epoch / SSD Company Ltd.", "Let's Play Excite Striker (UK)", MACHINE_NOT_WORKING )
CONS( 2002,  rad_socr,  epo_strk, 0, xavix_epo_strk,      epo_strk,  xavix_ir_power_state,         empty_init,  "Radica / Epoch / SSD Company Ltd.", "Play TV Soccer", MACHINE_NOT_WORKING )

// いろいろうんてん 工事のくるま5
CONS( 2001,  tak_town,  0,        0, xavix,               tak_town,  xavix_state,                  empty_init,  "Takara / SSD Company Ltd.", "Iroiro Unten: Kouji no Kuruma 5 (Japan)", 0 )
// this is roughly based on tak_town above
CONS( 2002,  rad_jcon,  tak_town, 0, xavix,               rad_jcon,  xavix_state,                  empty_init,  "Radica / Takara / SSD Company Ltd.", "Play TV Jr. Construction", 0 )

// ホームラン打とうぜ! エキサイトスタジアム
CONS( 2000,  epo_stad,  0,        0, xavix_epo_stad,      epo_stad,  xavix_epo_stad_state,         empty_init,  "Epoch / SSD Company Ltd.", "Home Run Datouze! Excite Stadium (Japan)", MACHINE_NOT_WORKING )
// this seems to be based off the epo_stad code, but heavily modified
CONS( 2002,  rad_bb2,   0,        0, xavix_rad_bb2,       rad_bb2,   xavix_power_state,            empty_init,  "Radica / SSD Company Ltd.", "Play TV Baseball 2 (NTSC)", MACHINE_NOT_WORKING ) // contains string "Radica RBB2 V1.0"

// 勝負しようぜ！ エキサイトスタジアムDX
CONS( 2002,  epo_esdx,  0,        0, xavix_epo_esdx,      epo_esdx,  xavix_epo_baseball_state,     empty_init,  "Epoch / SSD Company Ltd.", "Shoubu Shiyouze! Excite Stadium DX (Japan)", MACHINE_NOT_WORKING )

// 阪神タイガース エキサイトスタジアムＤＸ
CONS( 2003,  epo_esht,  0,        0, xavix_epo_esht,      epo_esht,  xavix_epo_baseball_state,     empty_init,  "Epoch / SSD Company Ltd.", "Hanshin Tigers Excite Stadium DX (Japan)", MACHINE_NOT_WORKING )

// エースきめるぜ！エキサイトテニス
CONS( 2002,  epo_tenn,  0,        0, xavix_epo_tenn,      epo_tenn,  xavix_ir_power_state,         empty_init,  "Epoch / SSD Company Ltd.", "Ace Kimeruze! Excite Tennis (Japan)", MACHINE_NOT_WORKING )

// ハムちゃんず大集合 ダンスするのだ！走るのだ！
CONS( 2001,  epo_hamd,  0,        0, xavix_ir,            epo_hamd,  xavix_ir_state,               empty_init,  "Epoch / SSD Company Ltd.", "Ham-chans Dai Shuugou: Dance Surunoda! Hashirunoda! (Japan)", MACHINE_NOT_WORKING )

// 卓球やろうぜ！ エキサイトピンポン
CONS( 2000,  epo_epp,   0,        0, xavix_epo_epp,       epo_epp1,  xavix_ir_power_state,         empty_init,  "Epoch / SSD Company Ltd.", "Takkyuu Yarouze! Excite Ping Pong (Japan)", MACHINE_NOT_WORKING )
// 리얼핑퐁
CONS( 2000,  epo_eppk,  epo_epp,  0, xavix_epo_epp,       epo_epp1,  xavix_ir_power_state,         empty_init,  "Epoch / SSD Company Ltd. / D-Gate / Sonokong", "Real Ping Pong (Korea)", MACHINE_NOT_WORKING )

// しゃもじdeピンポン
// This special version of Excite Ping Pong was a competition prize, not a retail product. The competition was sponsored by ミツカン (Mizkan)
CONS( 2001,  epo_epps,  0,        0, xavix_epo_epp,       epo_epp1,  xavix_ir_power_state,         empty_init,  "Epoch / Mizkan / SSD Company Ltd.", "Shamoji de Ping Pong (Japan)", MACHINE_NOT_WORKING )

// 卓球やろうぜ！ エキサイトピンポン2
CONS( 2003,  epo_epp2,  0,        0, xavix_epo_epp2,      epo_epp2,  xavix_ir_power_state,         empty_init,  "Epoch / SSD Company Ltd.", "Takkyuu Yarouze! Excite Ping Pong 2 (Japan)", MACHINE_NOT_WORKING )

// 愛ちゃんに挑戦！エキサイトピンポン
CONS( 2006,  epo_epp3,  0,        0, xavix_epo_epp2,      epo_epp3,  xavix_ir_power_state,         empty_init,  "Epoch / SSD Company Ltd.", "Ai-chan ni Chousen! Excite Ping Pong (Japan)", MACHINE_NOT_WORKING )

// TV麻雀 昇段対局～4人打ち
CONS( 2003,  epo_mj,    0,        0, xavix_epo_mj,        epo_mj,    xavix_epo_mj_state,           empty_init,  "Epoch / SSD Company Ltd.", "TV Mahjong Shoudan Taikyoku: 4-nin Uchi (Japan)", MACHINE_NOT_WORKING )

// ブラックバス釣ろうぜ エキサイトフィッシング
CONS( 2001,  epo_fish,  0,        0, xavix_2mb,           epo_fish,  xavix_power_state,            empty_init,  "Epoch / SSD Company Ltd.", "Black Bass Tsurouze: Excite Fishing (Japan)", MACHINE_NOT_WORKING )

// 大モノ釣ろうぜ！ エキサイトフィッシングＤＸ
CONS( 2003,  epo_efdx,  0,        0, xavix_i2c_24c08_4mb, epo_efdx,  xavix_i2c_power_state,        empty_init,  "Epoch / SSD Company Ltd.", "Dai Mono Tsurouze! Excite Fishing DX (Japan)", MACHINE_NOT_WORKING )

// ぐるりんワールド
CONS( 2005,  epo_guru,  0,        0, xavix_epo_guru,      epo_guru,  xavix_epo_guru_state,         empty_init,  "Epoch / SSD Company Ltd.", "Gururin World (Japan)", 0 )

// ドラえもん こえでドカン！ わくわくくうきほう！！
CONS( 2002,  epo_dmon,  0,        0, xavix_epo_dmon,      epo_dmon,  xavix_epo_dmon_state,         empty_init,  "Epoch / SSD Company Ltd.", "Doraemon-goe de Dokan! Wakuwaku Kuukihou!! (Japan)", MACHINE_NOT_WORKING )

// コロッケ！いただ禁貨！バンカーバトル!!
CONS( 2003,  epo_crok,  0,        0, xavix_epo_crok,      epo_crok,  xavix_epo_crok_state,         empty_init,  "Epoch / SSD Company Ltd.", "Croket! Itada Kinka! Banker Battle!! (Japan)", MACHINE_NOT_WORKING )

// ミニモニ。ステージ！ダンスだぴょん！
CONS( 2002,  epo_mms,   0,        0, xavix_i2c_24c02_4mb, epo_mms,   xavix_i2c_state,              empty_init,  "Epoch / SSD Company Ltd.", "mini-moni. Stage! Dance Dapyon! (Japan)", 0 )

// ズバズバブレード
CONS( 2002,  tak_zuba,  0,        0, xavix_tak_zuba,      tak_zuba,  xavix_tak_zuba_state,         empty_init,  "Takara / SSD Company Ltd.", "Zuba Zuba Blade (Japan)", MACHINE_NOT_WORKING )

// ミニモニ。ステージ！ダンスだぴょん！ぷらすっ
CONS( 2003,  epo_mmsp,  0,        0, xavix_i2c_24c02_4mb, epo_mms,   xavix_i2c_state,              empty_init,  "Epoch / SSD Company Ltd.", "mini-moni. Stage! Dance Dapyon! Plus (Japan)", 0 )

// Let's!TVプレイ ケロロ小隊大パニック！ドタバタ大決戦であります
CONS( 2006,  ban_krrj,  0,        0, xavix_4mb,           ban_krrj,  xavix_power_state,            empty_init,  "Bandai / SSD Company Ltd.", "Let's! TV Play Keroro Shoutai Dai Panic! Dotabata Daikessen de Arimasu (Japan)", 0 )

// オールスター感謝祭　超豪華！クイズ決定版～赤坂５丁目体感スタジオ～
CONS( 2004,  epo_quiz,  0,        0, xavix_epo_quiz,      epo_quiz,  xavix_epo_quiz_state,         empty_init,  "Epoch / SSD Company Ltd.", "All-Star Kansha-sai Chou Gouka! Quiz Kettieban: Akasaka 5-choume Taikan Studio (Japan)", 0 )

CONS( 2005,  has_wamg,  0,        0, xavix_has_wamg,      has_wamg,  xavix_has_wamg_state,         empty_init,  "Hasbro / Milton Bradley / SSD Company Ltd.", "TV Wild Adventure Mini Golf (NTSC)", 0 )

// GEIGEKI ゴーゴーシューティング
CONS( 2002,  tak_geig,  0,        0, xavix_nv_4mb,        tak_geig,  xavix_state,                  empty_init,  "Takara / SSD Company Ltd.", "Geigeki Go Go Shooting (Japan)", 0 )

// 電脳冒険記ウェブダイバー DXウェブナイトシリーズ W-05 グラディオン
CONS( 2001,  tak_wdg,   0,        0, xavix_tak_wdg,       tak_wdg,   xavix_tak_wdg_state,          empty_init,  "Takara / SSD Company Ltd.", "Webdiver DX W-05 Gladion (Japan)", 0 )

// ティービーホッケー
CONS( 2001,  tom_tvho,  0,        0, xavix_tom_tvho,      tom_tvho,  xavix_state,                  empty_init,  "Tomy / SSD Company Ltd.", "TV Hockey (Japan)", MACHINE_NOT_WORKING )

// コメットさん☆ラブリンバトン
CONS( 2001,  tak_comt,  0,        0, xavix_tak_comt,      tak_comt,  xavix_tak_comt_state,         empty_init,  "Takara / SSD Company Ltd.", "Comet-san Lovelin Baton (Japan)", MACHINE_NOT_WORKING )

// 爆進スノボ ギンギンボーダーズ
CONS( 2001,  tak_gin,   0,        0, xavix_2mb,           tak_gin,   xavix_state,                  empty_init,  "Takara / SSD Company Ltd.", "Bakushin Sno-Bo: Gingin Boarders (Japan)", 0 )

// ぽこぽこハンマーズ
CONS( 2002,  tak_hamr,  0,        0, xavix_i2c_24c02_4mb, tak_hamr,  xavix_i2c_state,              empty_init,  "Takara / SSD Company Ltd.", "Poko Poko Hammers (Japan)", 0 )

// Beyblade V-Force: Arcade Challenge 5-in-1
CONS( 2003,  tak_beyu,  0,        0, xavix_tak_beyu,      tak_bey,   xavix_state,                  empty_init,  "Takara / Hasbro / SSD Company Ltd.", "Beyblade V-Force: Arcade Challenge 5-in-1 (US)", MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN )

// 爆転シュートベイブレード2002 ベイアルティメットシューター
CONS( 2002,  tak_beyb,  tak_beyu, 0, xavix_tak_beyu,      tak_bey,   xavix_state,                  empty_init,  "Takara / SSD Company Ltd.", "Bakuten Shoot Beyblade 2002: Bey Ultimate Shooter (Japan)", MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN )

// ジャラジャランド
// was also distributed by Atlus as an arcade cabinet in 2005, ROM almost certainly different (this one will auto-power off after inactivity, an arcade wouldn't do that)
CONS( 2003,  jarajal,   0,        0, xavix_jarajal,       jarajal,   xavix_jarajal_state,          empty_init,  "Takara / SSD Company Ltd.", "Jyarajyaland (Japan, PlugIt! version)", MACHINE_NOT_WORKING )

// テレビで遊び隊 近代撃ちまくりバラエティ 射的王(シャテキング)
CONS( 2002,  tomshoot,  0,        0, xavix_i2c_24c02_2mb, tomshoot,  xavix_tomshoot_state,         empty_init,  "Tomy / SSD Company Ltd.", "TV de Asobitai Kindai Uchimakuri Variety Shateking (Japan)", MACHINE_NOT_WORKING )

// テレビで遊び隊 トミカ カーナビドライブ / トミー
CONS( 2003,  tcarnavi,  0,        0, xavix_nv_4mb,        tcarnavi,  xavix_power_state,            empty_init,  "Tomy / SSD Company Ltd.", "TV de Asobitai Tomica Carnavi Drive (Japan)", 0 )

// テレビで遊び隊 痛快！娯楽活劇 ちゃんぴよんピンボール
CONS( 2003,  tomcpin,   0,        0, xavix_i2c_24c08_4mb, tomcpin,   xavix_i2c_state,              empty_init,  "Tomy / SSD Company Ltd.", "TV de Asobitai Tsuukai! Goraku Katsugeki: Champiyon Pinball (Japan)", 0 )

// テレビで遊び隊 日本一周 僕はプラレール運転士
CONS( 2004,  tomplc,    0,        0, xavix_tomplc,        tomplc,    xavix_i2c_power_state,        empty_init,  "Tomy / SSD Company Ltd.", "TV de Asobitai Nihon Isshuu: Boku wa Plarail Untenshi (Japan)", 0 )

// テレビで遊び隊 ハイパーレスキュー ぼくは救助隊！
CONS( 2006,  tomthr,    0,        0, xavix_tomthr,        tomthr,    xavix_power_state,            empty_init,  "Takara Tomy / SSD Company Ltd.", "TV de Asobitai Hyper Rescue: Boku wa Kyuujotai! (Japan)", 0 )

// ガンガンアドベンチャー
CONS( 2001,  gungunad,  0,        0, xavix_gungunad,      gungunad,  xavix_gungunad_state,         empty_init,  "Takara / SSD Company Ltd.", "Gun Gun Adventure (Japan)", MACHINE_NOT_WORKING )

// ガンガンレボリューション
CONS( 2004,  gungunrv,  0,        0, xavix_gungunrv,      gungunrv,  xavix_gungunrv_state,         empty_init,  "Takara / SSD Company Ltd.", "Gun Gun Revolution (Japan)", MACHINE_NOT_WORKING )

// ビストロキッズ ぼくもわたしもコックさん！
CONS( 2001,  bistro,    0,        0, xavix_bistro,        bistro,    xavix_bistro_state,           empty_init,  "Sega Toys / SSD Company Ltd.", "Bistro Kids: Boku mo Watashi mo Kok-san! (Japan)", MACHINE_NOT_WORKING )

// e-Kara Series
CONS( 2000,  ekara,     0,        0, xavix_ekara,         ekara,     xavix_ekara_state,            empty_init,  "Takara / SSD Company Ltd. / Hasbro", "e-kara (US?, NTSC, set 1)", MACHINE_NOT_WORKING ) // shows "Please insert a cartridge before turn it on" without cart
CONS( 2000,  ekaraa,    ekara,    0, xavix_ekara,         ekara,     xavix_ekara_state,            empty_init,  "Takara / SSD Company Ltd. / Hasbro", "e-kara (US?, NTSC, set 2)", MACHINE_NOT_WORKING ) // shows "Please insert a cartridge before turning on e-kara" without cart
CONS( 2000,  ekaraj,    ekara,    0, xavix_ekara,         ekara,     xavix_ekara_state,            empty_init,  "Takara / SSD Company Ltd.", "e-kara (Japan)", MACHINE_NOT_WORKING ) // shows Japanese message without cart
CONS( 2002,  ekarag,    ekara,    0, xavix_ekara,         ekara,     xavix_ekara_state,            empty_init,  "Takara / SSD Company Ltd.", "e-kara (Europe, includes 3 songs)", MACHINE_NOT_WORKING ) // found in Germany and UK, could just be a Europe-wide model, NOT a headset model, but still has 3 songs.
CONS( 2002,  ekaras,    ekara,    0, xavix_ekara,         ekara,     xavix_ekara_state,            empty_init,  "Takara / SSD Company Ltd. / newgent", "e-kara (Spain, includes 3 songs)", MACHINE_NOT_WORKING )
// the Korean i-singer release from Sonokong has a unique bios and different data resources, meaning carts are not fully compatible between it and e-kara
CONS( 2000,  isinger,   ekara,    0, xavix_isinger,       ekara,     xavix_ekara_state,            empty_init,  "Sonokong / SSD Company Ltd. / D-Gate", "i-Singer (Korea)", MACHINE_NOT_WORKING )
// the 'e-kara pro headset' has 3 songs built in for the US release.  The Japanese release of this appears to be called 'e-kara H.S.' and it is unclear if it also has built in songs.  The Canadian box says 'cartridge contains' instead of 'songs included' but is likely a printing error.
CONS( 2002,  ekaraphs,  ekara,    0, xavix_ekara,         ekara,     xavix_ekara_state,            empty_init,  "Takara / SSD Company Ltd.", "e-kara Pro Headset (US, includes 3 songs)", MACHINE_NOT_WORKING )
// e-pitch uses the Japanese e-kara BIOS but ties its panel-layout strap high for the reduced six-button matrix.
CONS( 2003,  epitch,    0,        0, xavix_ekara,         epitch,    xavix_ekara_state,            empty_init,  "Takara / SSD Company Ltd.", "e-pitch (Japan)", MACHINE_NOT_WORKING ) // shows Japanese message without cart
// e-kara mix was another unit that allowed you to connect to a PC, unlike e-kara web it also functions as a regular device
CONS( 200?,  ekaramix,  0,        0, xavix_ekara,         ekara,     xavix_ekara_state,            empty_init,  "Takara / SSD Company Ltd.", "e-kara Mix (Japan)", MACHINE_NOT_WORKING )

// ダンスダンスレボリューション　ふぁみマット
CONS( 2001,  ddrfammt,  0,        0, xavix_ddrfammt,      ddrfammt,  xavix_cart_state,             empty_init,  "Takara / Konami / SSD Company Ltd.", "Dance Dance Revolution Family Mat (Japan)", 0 )

// ポピラ
CONS( 2000,  popira,    0,        0, xavix_popira,        popira,    xavix_cart_state,             empty_init,  "Takara / SSD Company Ltd.", "Popira (Japan)", 0 ) // The original Popira is a single yellow unit
// the Korean release is not properly compatible with the usual Popira carts, although some of the Korean i-singer range are compatible
CONS( 2001,  popirak,   popira,   0, xavix_popirak,       popira,    xavix_cart_state,             empty_init,  "Sonokong / SSD Company Ltd. / D-Gate", "PoPiRa: Finger Sports (Korea)", 0 )

// ポピラ2
CONS( 2002,  popira2,   0,        0, xavix_popira2,       popira2,   xavix_popira2_state,          empty_init,  "Takara / SSD Company Ltd.", "Popira 2 (Japan)", 0 ) // Popira 2 is a set of 2 blue & green linked units (2nd unit is just a controller, no CPU or TV out)

// たいこでポピラ
CONS( 2003,  taikodp,   0,        0, xavix_taikodp,       taikodp,   xavix_i2c_cart_state,         empty_init,  "Takara / SSD Company Ltd.", "Taiko de Popira (Japan)", 0 )

// ジャンピンポピラ
CONS( 2004,  jpopira,   0,        0, xavix_jpopira,       jpopira,   xavix_i2c_cart_state,         empty_init,  "Takara / SSD Company Ltd.", "Jumping Popira (Japan)", 0 )

// evio
CONS( 2003,  evio,      0,        0, xavix_evio,          evio,      xavix_evio_state,             empty_init,  "Tomy / SSD Company Ltd.", "Evio (Japan)", MACHINE_NOT_WORKING )

// 爆闘宣言 ダイガンダーDX
/* 2 ROM expansion cartridges were available for this, in the form of other robots (the heads acts as cartridges) see daig_cart.xml
   a number of other robots were also available, but those act as controllers and don't plug into the ROM slot (presumably the signal sent is used to determine the character) */
CONS( 2002,  tak_daig,  0,        0, xavix_tak_daig,      daig,      xavix_tak_daig_state,         empty_init,  "Takara / SSD Company Ltd.", "Bakutou Sengen Daigunder DX (Japan)", MACHINE_NOT_WORKING )

// ガチンコ勝負！パチスロTV
CONS( 2002,  gcslottv,  0,        0, xavix_gcslottv,      gcslottv,  xavix_gcslottv_state,         empty_init,  "Takara / Sammy / DCT / SSD Company Ltd.", "Gachinko Shoubu! PachisloTV (Japan)", MACHINE_NOT_WORKING )

// Let’s!TVプレイ 超にんきスポット!ころがしほーだい たまごっちりぞーと   (Let's! TV Play Chou Ninki Spot! Korogashi-Houdai Tamagotchi Resort) (only on the Japanese list? http://test.shinsedai.co.jp/english/products/Applied/list.html )   This also allows you to use an IR reciever to import a Tamagotchi from compatible games
CONS( 2006,  ltv_tam,   0,        0, xavix_ltv_tam,       ltv_tam,   xavix_ltv_tam_state,          empty_init,  "Bandai / SSD Company Ltd.", "Let's! TV Play Chou Ninki Spot! Korogashi-Houdai Tamagotchi Resort (Japan)", MACHINE_NOT_WORKING )

CONS( 2008,  hikara,    0,        0, xavix_hikara,        hikara,    xavix_hikara_state,           empty_init,  "Takara Tomy / SSD Company Ltd.", "Hi-kara (Japan)", MACHINE_NOT_WORKING )

// 東京フレンドパーク2
CONS( 2003,  epo_tfp2,  0,        0, xavix_i2c_24c08_4mb, epo_tfp2,  xavix_i2c_power_state,        empty_init,  "Epoch / SSD Company Ltd.", "Tokyo Friend Park II (Japan)", 0 ) // uses in24lc08b

// 東京フレンドパーク2スペシャル
CONS( 2005,  epo_tp2s,  0,        0, xavix_4mb,           epo_tp2s,  xavix_state,                  empty_init,  "Epoch / SSD Company Ltd.", "Tokyo Friend Park II Special! (Japan)", 0 )

// 東京フレンドパークⅡ パーフェクト!めざせ!グランドスラム‼︎
CONS( 2007,  epo_tp2p,  0,        0, xavix,               epo_tp2p,  xavix_power_state,            empty_init,  "Epoch / SSD Company Ltd.", "Tokyo Friend Park II Perfect! Mezase! Grand Slam!! (Japan)", 0 )

// TV-PC Series
// きかんしゃトーマス テレビパソコン
CONS( 2005,  tvpc_tom,  0,        0, xavix_tvpc,          tvpc,      xavix_tvpc_state,             empty_init,  "Epoch / SSD Company Ltd.", "TV-PC Thomas & Friends (Japan)", MACHINE_NOT_WORKING )
// ドラえもん テレビパソコン
CONS( 2003,  tvpc_dor,  0,        0, xavix_tvpc,          tvpc,      xavix_tvpc_state,             empty_init,  "Epoch / SSD Company Ltd.", "TV-PC Doraemon (Japan)", MACHINE_NOT_WORKING )
// とっとこハム太郎 テレビパソコン
CONS( 2003,  tvpc_ham,  0,        0, xavix_tvpc,          tvpc,      xavix_tvpc_state,             empty_init,  "Epoch / SSD Company Ltd.", "TV-PC Tottoko Hamutaro (Japan)", MACHINE_NOT_WORKING )
// ハローキティテレビパソコン
CONS( 2004,  tvpc_hk,   0,        0, xavix_tvpc_hk,       tvpc,      xavix_tvpc_state,             empty_init,  "Epoch / SSD Company Ltd.", "TV-PC Hello Kitty (Japan)", MACHINE_NOT_WORKING )

// TVメールPC mail*cot メルコット
CONS( 2004,  epo_mail,  0,        0, xavix_epo_mail,      epo_mail,  xavix_epo_mail_state,         empty_init,  "Epoch / SSD Company Ltd.", "TV Mail PC mail*cot (Japan)", MACHINE_NOT_WORKING ) // A 74HC541D buffers the keyboard matrix onto the external data bus.
