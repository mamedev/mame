// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Brad Oliver, Fabio Priuli
/******************************************************************************

Nintendo 2C0x PPU emulation.

Written by Ernesto Corvi.
This code is heavily based on Brad Oliver's MESS implementation.

Total rewrite by Matthew Sutton for Accuracy NTSC and PAL

2009-04: Changed NES PPU to be a device (Nathan Woods)
2009-07: Changed NES PPU to use a device memory map (Robert Bohms)
2026-04: Total rewrite to be cycle accurate

******************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"
#include "cpu/m6502/m6502.h"
#include "sound/nes_apu.h"
//#include "bus/nes/mmc5.h"
//#include "bus/nes/mmc3.h"
//#include "bus/nes/mmc1.h"
//#include "bus/nes/tengen.h"
//#include "bus/nes/bootleg.h"
//#include "bus/nes/batlab.h"

#include "screen.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(PPU_2C02, ppu2c02_device, "ppu2c02", "2C02 PPU")
DEFINE_DEVICE_TYPE(PPU_2C03B, ppu2c03b_device, "ppu2c03b", "2C03B PPC")
DEFINE_DEVICE_TYPE(PPU_2C04, ppu2c04_device, "ppu2c04", "2C04 PPU")
DEFINE_DEVICE_TYPE(PPU_2C07, ppu2c07_device, "ppu2c07", "2C07 PPU")
DEFINE_DEVICE_TYPE(PPU_PALC, ppupalc_device, "ppupalc", "Generic PAL Clone PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_01, ppu2c05_01_device, "ppu2c05_01", "2C05_01 PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_02, ppu2c05_02_device, "ppu2c05_02", "2C05_02 PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_03, ppu2c05_03_device, "ppu2c05_03", "2C05_03 PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_04, ppu2c05_04_device, "ppu2c05_04", "2C05_04 PPU")
DEFINE_DEVICE_TYPE(PPU_2C04C, ppu2c04_clone_device, "ppu2c04c", "2C04 Clone PPU")

// default address map
void ppu2c0x_device::ppu2c0x(address_map& map) {
	if (!has_configured_map(0)) {
		map(0x0000, 0x3eff).ram();
		map(0x3f00, 0x3fff).rw(FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
	}
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ppu2c0x_device::memory_space_config() const {
	return space_config_vector {
		std::make_pair(0, &m_space_config)};
}

//-------------------------------------------------
//  ppu2c0x_device - constructor
//-------------------------------------------------

void ppu2c0x_device::device_config_complete() {
	/* reset the callbacks */
	m_scanline_callback_proc.set(nullptr);
	m_hblank_callback_proc.set(nullptr);
	m_vidaccess_callback_proc.set(nullptr);
	m_latch.set(nullptr);
	m_ppu_to_mapper.set(nullptr);
	m_ppu_bus_address_callback.set(nullptr);
	m_ppu_odd_frame_skip.set(nullptr);
	m_mmc1_ppu_phase.set(nullptr);
	m_mmc5_ppu_read.set(nullptr);
	m_mmc5_reset_scanline_irq.set(nullptr);
}

ppu2c0x_device::ppu2c0x_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock, address_map_constructor internal_map) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 17, 0, internal_map),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_scanline(0), // reset the scanline count
	m_videoram_addr_mask(0x3fff),
	m_global_refresh_mask(0x7fff),
	m_line_write_increment_large(32),
	m_paletteram_in_ppuspace(false),
	m_tile_page(0),
	m_back_color(0),
	m_refresh_data(0),
	m_x_fine(0),
	m_toggle(0),
	m_tilecount(0),
	m_latch(*this),
	m_ppu_to_mapper(*this),
	m_ppu_bus_address_callback(*this),
	m_ppu_odd_frame_skip(*this),
	m_mmc1_ppu_phase(*this),
	m_mmc5_ppu_read(*this),
	m_mmc5_reset_scanline_irq(*this),
	m_scanline_callback_proc(*this),
	m_hblank_callback_proc(*this),
	m_vidaccess_callback_proc(*this),
	m_int_callback(*this),
	m_refresh_latch(0),
	m_add(1),
	m_videomem_addr(0),
	m_data_latch(0),
	m_buffered_data(0),
	m_sprite_page(0),
	m_scan_scale(1) { // set the scan scale (this is for dual monitor vertical setups)
	for (auto& elem : m_regs)
		elem = 0;

	m_scanlines_per_frame = NTSC_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE;
	m_prerender_line = NTSC_SCANLINES_PER_FRAME - 1;

	/* usually, no security value... */
	m_security_value = 0;
}

ppu2c0x_device::ppu2c0x_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(ppu2c0x_device::ppu2c0x), this)) {
	m_paletteram_in_ppuspace = true;
}

ppu2c0x_rgb_device::ppu2c0x_rgb_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock),
	m_palette_data(*this, "palette") {
}

// NTSC NES
ppu2c02_device::ppu2c02_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, PPU_2C02, tag, owner, clock) {
}

// Playchoice 10
ppu2c03b_device::ppu2c03b_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C03B, tag, owner, clock) {}

// Vs. Unisystem
ppu2c04_device::ppu2c04_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C04, tag, owner, clock) {}

// PAL NES
ppu2c07_device::ppu2c07_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, PPU_2C07, tag, owner, clock) {
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE;
}

// PAL clones
ppupalc_device::ppupalc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, PPU_PALC, tag, owner, clock) {
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_PALC;
}

// The PPU_2C05 variants have different protection value, set at device start, but otherwise are all the same...
// Vs. Unisystem (Ninja Jajamaru Kun)
ppu2c05_01_device::ppu2c05_01_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_01, tag, owner, clock) {
	m_security_value = 0x1b; // game (jajamaru) doesn't seem to ever actually check it
}

// Vs. Unisystem (Mighty Bomb Jack)
ppu2c05_02_device::ppu2c05_02_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_02, tag, owner, clock) {
	m_security_value = 0x3d;
}

// Vs. Unisystem (Gumshoe)
ppu2c05_03_device::ppu2c05_03_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_03, tag, owner, clock) {
	m_security_value = 0x1c;
}

// Vs. Unisystem (Top Gun)
ppu2c05_04_device::ppu2c05_04_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_04, tag, owner, clock) {
	m_security_value = 0x1b;
}

// Vs. Unisystem (Super Mario Bros. bootlegs)
ppu2c04_clone_device::ppu2c04_clone_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, PPU_2C04C, tag, owner, clock),
	m_palette_data(*this, "palette") {
	m_scanlines_per_frame = VS_CLONE_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_VS_CLONE;

	// background and sprites are always enabled; monochrome and color emphasis aren't supported
	m_regs[PPU_CONTROL1] = u8(~(PPU_CONTROL1_COLOR_EMPHASIS | PPU_CONTROL1_DISPLAY_MONO));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void ppu2c0x_device::init_runtime_reset_state() {
	// --------------------------------------------------
	// VRAM address / scroll registers
	// --------------------------------------------------
	fine_x = 0;
	t = 0;
	v = 0;
	w = false;

	// --------------------------------------------------
	// $2000 / PPUCTRL decoded state
	// --------------------------------------------------
	ppuctrl_vram_increment = 1;
	ppuctrl_sprite_pattern_base = 0x0000;
	ppuctrl_bg_pattern_base = 0x0000;
	ppuctrl_sprite_size = EIGHT_BY_EIGHT;
	ppuctrl_nmi_enable = false;

	// --------------------------------------------------
	// $2001 / PPUMASK decoded state
	// --------------------------------------------------
	ppumask_grayscale_mask = 0x3F; // Grayscale off
	ppumask_show_bg_left = false;
	ppumask_show_spr_left = false;
	ppumask_emphasis_bits = 0;
	bg_left_clip = 256;
	spr_left_clip = 256;

	bg_output_enabled = 0;
	spr_output_enabled = 0;
	bg_pipeline_enabled = 0;
	spr_pipeline_enabled = 0;

	retro_ppumask_color = false;
	retro_ppumask_render = false;

	// --------------------------------------------------
	// $2002 / PPUSTATUS state
	// --------------------------------------------------
	ppustatus_sprite_overflow = false;
	ppustatus_sprite0_hit = false;
	sprite0_hit_pending = false;
	sprite0_hit_delay = 0;
	ppustatus_vblank = false;
	suppress_vblank_flag = false;

	// --------------------------------------------------
	// NMI state
	// --------------------------------------------------
	nmi_delay = 0;
	nmi_pending = false;

	// --------------------------------------------------
	// PPU timing / scan position
	// --------------------------------------------------
	dot = 0;
	scanline = 0;
	m_scanline = 0;
	
	ppu_tick_in_cpu_cycle = 0;
	frame_start_ppu_phase = 0;

	// --------------------------------------------------
	// PPU external bus / open bus / $2007 buffer
	// --------------------------------------------------
	ppu_address_bus = 0;
	ppu_ad_latch = 0;
	ppudata_read_buffer = 0;
	ppu_ale_low_latch = 0;

	ppu2007_buffer_fill_armed = false;
	ppu2007_buffer_fill_arm_pending = false;
	ppu_bus_read_can_fill_2007 = false;

	ppu2007_post_bump_pending = false;
	ppu2007_post_bump_delay = 0;

	ppu2007_ale_read_addr_latch_poison = false;
	ppu2007_ale_read_low_latch = 0;

	// --------------------------------------------------
	// Delayed $2007 write state
	// --------------------------------------------------
	m_2007_write.pending = false;
	m_2007_write.delay = 0;
	m_2007_write.addr = 0;
	m_2007_write.data = 0;

	// --------------------------------------------------
	// Delayed $2007 read state
	// --------------------------------------------------
	m_2007_read.pending = false;
	m_2007_read.delay = 0;
	m_2007_read.addr = 0;
	m_2007_read.use_next_ppu_read_for_refill = false;
	m_2007_read.waiting_for_refill_bus_read = false;

	// --------------------------------------------------
	// Pending delayed CPU register effects
	// --------------------------------------------------
	pending_2000.has_pending = false;
	pending_2000.value = 0;
	//pending_2000.value16 = 0;
	//pending_2000.apply_dot = 0;
	//pending_2000.apply_scanline = 0;
	pending_2000.apply_ppu = 0;

	pending_2001.has_pending = false;
	pending_2001.value = 0;
	//pending_2001.value16 = 0;
	//pending_2001.apply_dot = 0;
	//pending_2001.apply_scanline = 0;
	pending_2001.apply_ppu = 0;

	pending_2006.has_pending = false;
	pending_2006.value = 0;
	pending_2006.value16 = 0;
	//pending_2006.apply_dot = 0;
	//pending_2006.apply_scanline = 0;
	pending_2006.apply_ppu = 0;

	// --------------------------------------------------
	// OAM memory / OAMDATA state
	// --------------------------------------------------
	memset(primary_oam, 0x00, sizeof(primary_oam));
	memset(secondary_oam, 0x00, sizeof(secondary_oam));

	oamaddr = 0;
	oam2addr = 0;
	oam_eval_addr = 0;
	oam_latch_addr = 0;

	oamdata_read_buffer = 0;
	oamdata_latch = 0xFF;
	oam2_last_write = 0xFF;
	oam2_full = false;
	oam2_increment_frozen = false;
	sprite0_in_oam2_next_valid = false;

	// --------------------------------------------------
	// Sprite evaluation state
	// --------------------------------------------------
	eval_sprite_y = 0;
	eval_sprite_tile = 0;
	eval_candidate_in_range = false;
	sprite_eval_in_range = false;
	sprite_eval_initialized = false;

	oam_copy_done = false;
	overflow_bug_counter = 0;
	overflow_eval_phase = 0;
	overflow_finish_bytes = 0;

	m_eval_wrap_byte = 0;
	m_eval_prev_oam_latch_addr = 0xFF;
	m_oam_eval_realigned = false;

	// --------------------------------------------------
	// Sprite zero / stale sprite state
	// --------------------------------------------------
	sprite0_in_oam2_next = false;
	sprite0_in_oam2_current = false;
	sprite0_eval_addr = 0;
	sprite0_pat = 0;

	sprite_go_this_line = true;
	sprite_go_next_line = true;
	sprite_sl0_early_shift_pending = false;
	sl0_stale_s0_loaded = false;
	sl0_stale_sprite0_identity = false;

	// --------------------------------------------------
	// Sprite output units
	// --------------------------------------------------
	memset(spr_attr_latch, 0x00, sizeof(spr_attr_latch));
	memset(spr_x_latch, 0x00, sizeof(spr_x_latch));
	memset(spr_x_counter, 0, sizeof(spr_x_counter));
	memset(spr_pt_l_shift, 0x00, sizeof(spr_pt_l_shift));
	memset(spr_pt_h_shift, 0x00, sizeof(spr_pt_h_shift));
	memset(spr_shift_count, 0, sizeof(spr_shift_count));

	memset(sec_oam_source, 0xFF, sizeof(sec_oam_source));
	memset(sprite_oam_source, 0xFF, sizeof(sprite_oam_source));

	// --------------------------------------------------
	// Sprite/OAM corruption state
	// --------------------------------------------------
	oam_corrupt_pending = false;
	oam_corrupt_seed = 0;
	corrupt_resume_high_lane = 0xFF;

	// --------------------------------------------------
	// Background fetch latches / shift registers
	// --------------------------------------------------
	bg_nt_latch = 0;
	bg_at_latch = 0;
	bg_pt_l_latch = 0;
	bg_pt_h_latch = 0;

	bg_pt_l_shift = 0;
	bg_pt_h_shift = 0;
	bg_at_l_shift = 0;
	bg_at_h_shift = 0;

	bg_at_latch_l = 0;
	bg_at_latch_h = 0;

	// --------------------------------------------------
	// Background/sprite fetch helper state
	// --------------------------------------------------
	skip_bg_reload_once = false;

	spr_fetch_v_old = 0;
	spr_fetch_v_new = 0;
	spr_pt_addr_h = 0;
	spr_pt_addr_l = 0;

	bg_fetch_nt_addr = 0;
	bg_fetch_at_addr = 0;
	bg_fetch_v_nt = 0;
	bg_fetch_v_at = 0;
	bg_fetch_v_pt = 0;
	bg_fetch_pt_base = 0;

	// --------------------------------------------------
	// Previous-pixel latch for retroactive PPUMASK behavior
	// --------------------------------------------------
	prev_pixel_valid = false;
	prev_pixel_scanline = 0;
	prev_pixel_x = 0;
	prev_bg_pixel_pat = 0;
	prev_attr_bits = 0;
	prev_spr_pat = 0;
	prev_spr_pal = 0;
	prev_spr_behind_bg = false;
	prev_spr_is_s0 = false;
	prev_sprite0_pat = 0;
	prev_backdrop_pal_index = 0;
}

void ppu2c0x_device::init_startup_only_state() {
	// --------------------------------------------------
	// Mapper / cartridge-side device pointers
	// --------------------------------------------------
	m_mapper_number = -1;
	//m_mmc5 = nullptr;
	//m_mmc3 = nullptr;
	//m_mmc1_sxrom = nullptr;
	//m_rambo1 = nullptr;
	//m_sc127 = nullptr;
	//m_batmap_srrx = nullptr;

	// --------------------------------------------------
	// Mapper feature flags
	// --------------------------------------------------
	//m_has_mmc3_a12 = false;
	//m_has_rambo1_a12 = false;
	//m_has_sc127_a12 = false;
	//m_has_mmc5_ppu = false;
	//m_has_mmc1_phase = false;
	//m_has_chr_latch = false;
	//m_has_batmap_srrx_a12 = false;

	// --------------------------------------------------
	// PPU model / board configuration
	// --------------------------------------------------
	m_security_value = 0;
	m_prerender_line = 0;

	// --------------------------------------------------
	// Frame / timing startup state
	// --------------------------------------------------
	frame = 0;
	skip_dot = false;
	s_after_wrap = false;
	odd_frame = false;
}

void ppu2c0x_device::reset() {
	init_runtime_reset_state();
}

void ppu2c0x_device::start_nopalram() {
	// --------------------------------------------------
	// Video allocation / palette initialization
	// --------------------------------------------------
	/* allocate a screen bitmap, videomem and spriteram, a dirtychar array and the monochromatic colortable */
	m_bitmap.allocate(VISIBLE_SCREEN_WIDTH, VISIBLE_SCREEN_HEIGHT);
	init_palette_tables();

	// --------------------------------------------------
	// Base MAME save state
	// --------------------------------------------------
	save_item(NAME(m_scanline));
	save_item(NAME(m_refresh_data));
	save_item(NAME(m_x_fine));
	save_item(NAME(m_toggle));
	save_item(NAME(m_back_color));
	save_item(NAME(m_scan_scale));
	save_item(NAME(m_scanlines_per_frame));
	save_item(NAME(m_vblank_first_scanline));
	save_item(NAME(m_regs));
	save_item(NAME(m_bitmap));

	// --------------------------------------------------
	// Initial PPU state
	// --------------------------------------------------
	init_startup_only_state();
	init_runtime_reset_state();
}

void ppu2c0x_device::device_start() {
	start_nopalram();

	// --------------------------------------------------
	// Device pointers
	// --------------------------------------------------
	m_maincpu6502 = machine().root_device().subdevice<m6502_device>("maincpu");
	//m_mmc5 = machine().root_device().subdevice<nes_exrom_device>("nes_slot:exrom");

	// --------------------------------------------------
	// Power-up palette RAM
	// --------------------------------------------------
	// pass blargg_ppu_tests/power_up_palette.nes
	m_palette_ram = {
		0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D,
		0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
		0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14,
		0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08};

	// --------------------------------------------------
	// Save callbacks
	// --------------------------------------------------
	save_item(NAME(m_palette_ram));
	machine().save().register_presave(save_prepost_delegate(FUNC(ppu2c0x_device::presave), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(ppu2c0x_device::postload), this));

	// --------------------------------------------------
	// Core PPU / mapper state
	// --------------------------------------------------
	save_item(NAME(m_mapper_number));
	save_item(NAME(m_security_value));
	save_item(NAME(m_prerender_line));
	save_item(NAME(frame));
	//save_item(NAME(m_has_mmc3_a12));
	//save_item(NAME(m_has_rambo1_a12));
	//save_item(NAME(m_has_sc127_a12));
	//save_item(NAME(m_has_mmc5_ppu));
	//save_item(NAME(m_has_mmc1_phase));
	//save_item(NAME(m_has_chr_latch));
	//save_item(NAME(m_has_batmap_srrx_a12));

	// --------------------------------------------------
	// OAM memory
	// --------------------------------------------------
	save_item(NAME(primary_oam));
	save_item(NAME(secondary_oam));

	// --------------------------------------------------
	// VRAM address / scroll state
	// --------------------------------------------------
	save_item(NAME(t));
	save_item(NAME(v));
	save_item(NAME(fine_x));
	save_item(NAME(w));

	// --------------------------------------------------
	// $2000 / PPUCTRL state
	// --------------------------------------------------
	save_item(NAME(ppuctrl_vram_increment));
	save_item(NAME(ppuctrl_sprite_pattern_base));
	save_item(NAME(ppuctrl_bg_pattern_base));
	save_item(NAME(m_save_sprite_size));
	save_item(NAME(ppuctrl_nmi_enable));

	// --------------------------------------------------
	// $2001 / PPUMASK state
	// --------------------------------------------------
	save_item(NAME(ppumask_grayscale_mask));
	save_item(NAME(ppumask_show_bg_left));
	save_item(NAME(ppumask_show_spr_left));
	save_item(NAME(ppumask_emphasis_bits));
	save_item(NAME(bg_left_clip));
	save_item(NAME(spr_left_clip));

	// --------------------------------------------------
	// $2002 / PPUSTATUS state
	// --------------------------------------------------
	save_item(NAME(ppustatus_sprite_overflow));
	save_item(NAME(ppustatus_sprite0_hit));
	save_item(NAME(ppustatus_vblank));

	// --------------------------------------------------
	// NMI / vblank suppression state
	// --------------------------------------------------
	save_item(NAME(nmi_pending));
	save_item(NAME(nmi_delay));
	save_item(NAME(suppress_vblank_flag));

	// --------------------------------------------------
	// OAM registers / sprite evaluation state
	// --------------------------------------------------
	save_item(NAME(oamaddr));
	save_item(NAME(oam2addr));
	save_item(NAME(oamdata_read_buffer));
	save_item(NAME(oam_eval_addr));
	save_item(NAME(oam_copy_done));
	save_item(NAME(overflow_bug_counter));
	save_item(NAME(sprite_eval_in_range));

	save_item(NAME(eval_sprite_y));
	save_item(NAME(eval_sprite_tile));
	save_item(NAME(eval_candidate_in_range));

	// --------------------------------------------------
	// PPU timing state
	// --------------------------------------------------
	save_item(NAME(odd_frame));
	save_item(NAME(scanline));
	save_item(NAME(dot));
	save_item(NAME(skip_dot));
	save_item(NAME(s_after_wrap));
	save_item(NAME(ppu_tick_in_cpu_cycle));
	save_item(NAME(frame_start_ppu_phase));

	// --------------------------------------------------
	// PPU data/bus state
	// --------------------------------------------------
	save_item(NAME(ppudata_read_buffer));
	save_item(NAME(ppu_address_bus));
	save_item(NAME(ppu_ale_low_latch));

	// --------------------------------------------------
	// Background render pipeline
	// --------------------------------------------------
	save_item(NAME(bg_nt_latch));
	save_item(NAME(bg_at_latch));
	save_item(NAME(bg_pt_l_latch));
	save_item(NAME(bg_pt_h_latch));
	save_item(NAME(bg_pt_l_shift));
	save_item(NAME(bg_pt_h_shift));
	save_item(NAME(bg_at_l_shift));
	save_item(NAME(bg_at_h_shift));
	save_item(NAME(bg_at_latch_l));
	save_item(NAME(bg_at_latch_h));

	// --------------------------------------------------
	// Sprite output units
	// --------------------------------------------------
	save_item(NAME(spr_attr_latch));
	save_item(NAME(spr_x_latch));
	save_item(NAME(spr_x_counter));
	save_item(NAME(spr_pt_l_shift));
	save_item(NAME(spr_pt_h_shift));
	save_item(NAME(spr_shift_count));

	// --------------------------------------------------
	// Sprite zero / stale sprite state
	// --------------------------------------------------
	save_item(NAME(sprite0_in_oam2_next));
	save_item(NAME(sprite0_in_oam2_current));
	save_item(NAME(sprite0_eval_addr));
	save_item(NAME(sprite0_pat));
	save_item(NAME(sprite_go_this_line));
	save_item(NAME(sprite_go_next_line));
	save_item(NAME(sprite_sl0_early_shift_pending));
	save_item(NAME(sl0_stale_s0_loaded));
	save_item(NAME(sl0_stale_sprite0_identity));

	// --------------------------------------------------
	// Pending delayed register effects
	// --------------------------------------------------
	save_item(NAME(pending_2000.has_pending));
	save_item(NAME(pending_2000.value));
	//save_item(NAME(pending_2000.value16));
	//save_item(NAME(pending_2000.apply_dot));
	//save_item(NAME(pending_2000.apply_scanline));
	save_item(NAME(pending_2000.apply_ppu));

	save_item(NAME(pending_2001.has_pending));
	save_item(NAME(pending_2001.value));
	//save_item(NAME(pending_2001.value16));
	//save_item(NAME(pending_2001.apply_dot));
	//save_item(NAME(pending_2001.apply_scanline));
	save_item(NAME(pending_2001.apply_ppu));

	save_item(NAME(pending_2006.has_pending));
	save_item(NAME(pending_2006.value));
	save_item(NAME(pending_2006.value16));
	//save_item(NAME(pending_2006.apply_dot));
	//save_item(NAME(pending_2006.apply_scanline));
	save_item(NAME(pending_2006.apply_ppu));

	// --------------------------------------------------
	// Render enable / pipeline decoded state
	// --------------------------------------------------
	save_item(NAME(bg_output_enabled));
	save_item(NAME(spr_output_enabled));
	save_item(NAME(bg_pipeline_enabled));
	save_item(NAME(spr_pipeline_enabled));
	save_item(NAME(skip_bg_reload_once));

	// --------------------------------------------------
	// OAM corruption / OAM latch state
	// --------------------------------------------------
	save_item(NAME(oam_corrupt_pending));
	save_item(NAME(oam_corrupt_seed));
	save_item(NAME(oam_latch_addr));
	save_item(NAME(oam2_full));
	save_item(NAME(oam2_increment_frozen));
	save_item(NAME(sprite0_in_oam2_next_valid));
	save_item(NAME(oamdata_latch));
	save_item(NAME(oam2_last_write));
	save_item(NAME(overflow_eval_phase));
	save_item(NAME(overflow_finish_bytes));

	// --------------------------------------------------
	// Retroactive PPUMASK previous-pixel state
	// --------------------------------------------------
	save_item(NAME(prev_pixel_valid));
	save_item(NAME(prev_pixel_scanline));
	save_item(NAME(prev_pixel_x));
	save_item(NAME(prev_bg_pixel_pat));
	save_item(NAME(prev_attr_bits));
	save_item(NAME(prev_spr_pat));
	save_item(NAME(prev_spr_pal));
	save_item(NAME(prev_spr_behind_bg));
	save_item(NAME(prev_spr_is_s0));
	save_item(NAME(prev_sprite0_pat));
	save_item(NAME(prev_backdrop_pal_index));
	save_item(NAME(retro_ppumask_color));
	save_item(NAME(retro_ppumask_render));

	// --------------------------------------------------
	// Delayed fine-X / scroll side effects
	// --------------------------------------------------
	//save_item(NAME(pending_fine_x));

	// --------------------------------------------------
	// PPU internal I/O latch / open-bus decay
	// --------------------------------------------------
	save_item(NAME(m_ppu_io_db));
	save_item(NAME(m_ppu_io_db_decay_at));

	// --------------------------------------------------
	// $2007 delayed read/write machinery
	// --------------------------------------------------
	save_item(NAME(ppu2007_buffer_fill_armed));
	save_item(NAME(ppu2007_buffer_fill_arm_pending));
	save_item(NAME(ppu_bus_read_can_fill_2007));
	save_item(NAME(ppu2007_post_bump_pending));
	save_item(NAME(ppu2007_post_bump_delay));

	save_item(NAME(m_2007_write.pending));
	save_item(NAME(m_2007_write.delay));
	save_item(NAME(m_2007_write.addr));
	save_item(NAME(m_2007_write.data));

	save_item(NAME(m_2007_read.pending));
	save_item(NAME(m_2007_read.delay));
	save_item(NAME(m_2007_read.addr));
	save_item(NAME(m_2007_read.use_next_ppu_read_for_refill));
	save_item(NAME(m_2007_read.waiting_for_refill_bus_read));

	// --------------------------------------------------
	// Fetch helper state
	// --------------------------------------------------
	save_item(NAME(spr_fetch_v_old));
	save_item(NAME(spr_fetch_v_new));
	save_item(NAME(spr_pt_addr_h));
	save_item(NAME(spr_pt_addr_l));
	save_item(NAME(bg_fetch_v_nt));
	save_item(NAME(bg_fetch_v_at));
	save_item(NAME(bg_fetch_v_pt));
	save_item(NAME(bg_fetch_pt_base));
	save_item(NAME(bg_fetch_nt_addr));
	save_item(NAME(bg_fetch_at_addr));

	// --------------------------------------------------
	// Replacement state for previous function-local statics
	// --------------------------------------------------
	save_item(NAME(m_eval_wrap_byte));
	save_item(NAME(m_eval_prev_oam_latch_addr));
	save_item(NAME(sprite_eval_initialized));

	// --------------------------------------------------
	// ALE / $2007 address latch state
	// --------------------------------------------------
	save_item(NAME(ppu_ad_latch));
	save_item(NAME(ppu2007_ale_read_addr_latch_poison));
	save_item(NAME(ppu2007_ale_read_low_latch));

	// --------------------------------------------------
	// Resume-render OAM corruption state
	// --------------------------------------------------
	save_item(NAME(corrupt_resume_high_lane));
	save_item(NAME(sec_oam_source));
	save_item(NAME(sprite_oam_source));
	save_item(NAME(m_oam_eval_realigned));
}

void ppu2c0x_device::presave() {
	m_save_sprite_size = int(ppuctrl_sprite_size);
}

void ppu2c0x_device::postload() {
	ppuctrl_sprite_size = Sprite_size(m_save_sprite_size);
	//resolve_mapper_ppu_devices();
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

uint8_t ppu2c0x_device::readbyte(offs_t address) {
	return space().read_byte(address); // works
}

inline uint8_t ppu2c0x_device::readbyte(uint16_t bus_addr, uint16_t mem_addr) {
	return space().read_byte(mem_addr);
}

//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void ppu2c0x_device::writebyte(offs_t address, uint8_t data) {
	space().write_byte(address, data);
}

inline uint16_t ppu2c0x_device::apply_grayscale_and_emphasis(uint8_t color) {
	return (color & ppumask_grayscale_mask) | ppumask_emphasis_bits;
}

/***************************************************************************
IMPLEMENTATION
***************************************************************************/

/*************************************
*
*  PPU Palette Initialization
*
*************************************/

void ppu2c0x_device::apply_color_emphasis_and_clamp(bool is_pal_or_dendy, int color_emphasis, double& R, double& G, double& B) {
	if (is_pal_or_dendy) { // PAL machines swap the colour emphasis bits, this means the red/blue highlighting on rampart tally bar doesn't look as good
		color_emphasis = bitswap<3>(color_emphasis, 2, 0, 1);
	}

	static constexpr double rgb_mod[8][3] = {
		//  R      G      B
		{1.0, 1.0, 1.0},
		{1.24, 0.915, 0.743},
		{0.794, 1.09, 0.882},
		{0.905, 1.03, 1.28},
		{0.741, 0.987, 1.0},
		{1.02, 0.908, 0.979},
		{1.02, 0.98, 0.653},
		{0.75, 0.75, 0.75}};

	// Clipping, in case of saturation
	R = std::clamp(R * rgb_mod[color_emphasis][0], 0.0, 255.0);
	G = std::clamp(G * rgb_mod[color_emphasis][1], 0.0, 255.0);
	B = std::clamp(B * rgb_mod[color_emphasis][2], 0.0, 255.0);
}

rgb_t ppu2c0x_device::nespal_to_RGB(int color_intensity, int color_num, int color_emphasis, bool is_pal_or_dendy) {
	const double tint = 0.22; /* adjust to taste */
	const double hue = 287.0;

	const double Kr = 0.2989;
	const double Kb = 0.1145;
	const double Ku = 2.029;
	const double Kv = 1.140;

	static const double brightness[3][4] = {
		{0.50, 0.75, 1.0, 1.0},
		{0.29, 0.45, 0.73, 0.9},
		{0, 0.24, 0.47, 0.77}};

	double sat;
	double y, u, v;
	double rad;

	switch (color_num) {
		case 0:
			sat = 0;
			rad = 0;
			y = brightness[0][color_intensity];
			break;

		case 13:
			sat = 0;
			rad = 0;
			y = brightness[2][color_intensity];
			break;

		case 14:
		case 15:
			sat = 0;
			rad = 0;
			y = 0;
			break;

		default:
			sat = tint;
			rad = M_PI * ((color_num * 30 + hue) / 180.0);
			y = brightness[1][color_intensity];
			break;
	}

	u = sat * cos(rad);
	v = sat * sin(rad);

	/* Transform to RGB */
	double R = (y + Kv * v) * 255.0;
	double G = (y - (Kb * Ku * u + Kr * Kv * v) / (1 - Kb - Kr)) * 255.0;
	double B = (y + Ku * u) * 255.0;

	apply_color_emphasis_and_clamp(is_pal_or_dendy, color_emphasis, R, G, B);

	return rgb_t(floor(R + .5), floor(G + .5), floor(B + .5));
}

///this is the color for regular nes
void ppu2c0x_device::init_palette_tables() {
	const bool is_pal = m_scanlines_per_frame != NTSC_SCANLINES_PER_FRAME;

	// Build the NES palette using a transformation from YUV to RGB.
	//
	// Layout:
	//   8 emphasis modes
	//   4 luminance/intensity levels
	//   16 colors per intensity
	int entry = 0;

	for (int color_emphasis = 0; color_emphasis < 8; ++color_emphasis) {
		for (int color_intensity = 0; color_intensity < 4; ++color_intensity) {
			for (int color_num = 0; color_num < 16; ++color_num)
				m_nespens[entry++] = uint32_t(nespal_to_RGB(color_intensity, color_num, color_emphasis, is_pal));
		}
	}
}

void ppu2c0x_rgb_device::init_palette_tables() {
	// Build 8 emphasis variants of the 64-entry RGB palette.
	//
	// Each base color is stored as 3 packed 3-bit components in m_palette_data:
	//   R, G, B.
	//
	// For RGB PPUs, emphasis forces the selected channel to max intensity.
	int entry = 0;

	for (int color_emphasis = 0; color_emphasis < 8; ++color_emphasis) {
		const bool emph_r = (color_emphasis & 1) != 0;
		const bool emph_g = (color_emphasis & 2) != 0;
		const bool emph_b = (color_emphasis & 4) != 0;

		for (int color_num = 0; color_num < 64; ++color_num) {
			const int base = color_num * 3;

			const int r = emph_r ? 7 : m_palette_data[base + 0];
			const int g = emph_g ? 7 : m_palette_data[base + 1];
			const int b = emph_b ? 7 : m_palette_data[base + 2];

			m_nespens[entry++] = (pal3bit(r) << 16) | (pal3bit(g) << 8) | pal3bit(b);
		}
	}
}

/*************************************
*
*  PPU Bus Helpers
*
*************************************/

void ppu2c0x_device::ppu_bus_address_drive(uint16_t addr, ppu_bus_source source)
{
	addr &= 0x3fff;

	ppu_address_bus = addr;
	ppu_ad_latch = addr & 0xff;

	const uint64_t cpu_cycle = m_cpu->total_cycles() + (source == ppu_bus_source::CPU_ACCESS ? 1 : 0);
	const uint64_t ppu_cycle = (cpu_cycle * 3) + ppu_tick_in_cpu_cycle;

	if (!m_ppu_bus_address_callback.isnull())
		m_ppu_bus_address_callback(addr, ppu_cycle, ppu_tick_in_cpu_cycle, odd_frame);
}

uint8_t ppu2c0x_device::ppu_bus_read(uint16_t addr, ppu_fetch_phase phase) {
	addr &= 0x3FFF;

	bool is_nt = false;
	bool is_at = false;
	bool is_bg_pattern = false;
	bool is_spr_pattern = false;

	switch (phase) {
		case ppu_fetch_phase::NT:
			is_nt = true;
			break;

		case ppu_fetch_phase::AT:
			is_at = true;
			break;

		case ppu_fetch_phase::PTL:
		case ppu_fetch_phase::PTH:
			is_bg_pattern = true;
			break;

		case ppu_fetch_phase::SPR_PTL:
		case ppu_fetch_phase::SPR_PTH:
			is_spr_pattern = true;
			break;

		default:
			break;
	}

	const bool chr_fetch = is_bg_pattern || is_spr_pattern;

	// Background fetches use the high address lines currently supplied by
	// the PPU together with the low address byte previously captured by ALE
	// on the setup dot.
	//
	// Sprite and dummy fetches remain on the existing full-address path for
	// now so this change is contained to the background fetch pipeline.
	if (is_nt || is_at || is_bg_pattern)
		addr = (addr & 0x3F00) | ppu_ale_low_latch;

	addr &= 0x3FFF;

	ppu_bus_address_drive(addr, ppu_bus_source::PPU);

	if (!m_mmc5_ppu_read.isnull()) {
		m_mmc5_ppu_read(addr);
	}

	const uint8_t data = readbyte(addr);

	// AD0-7 now contain returned memory data. The external ALE latch keeps
	// the low address byte captured during the preceding setup phase.
	ppu_ad_latch = data;

	if (addr < 0x2000 && chr_fetch) {
		if (!m_latch.isnull())
			m_latch(addr);

		if (!m_mmc1_ppu_phase.isnull()) {
			const bool upper_chr = BIT(addr, 12);
			m_mmc1_ppu_phase(upper_chr, addr & 0x1fff);
		}
	}

	if (ppu2007_buffer_fill_armed && ppu_bus_read_can_fill_2007) {
		ppudata_read_buffer = data;
		ppu2007_buffer_fill_armed = false;

		if (m_2007_read.pending && m_2007_read.waiting_for_refill_bus_read) {
			m_2007_read.pending = false;
			m_2007_read.waiting_for_refill_bus_read = false;
		}
	}

	return data;
}

void ppu2c0x_device::schedule_2007_read(uint16_t addr, int delay, bool use_next_ppu_read_for_refill) {
	addr &= 0x3FFF;

	m_2007_read.pending = true;
	m_2007_read.delay = (delay > 0) ? delay : 0;
	m_2007_read.addr = addr;
	m_2007_read.use_next_ppu_read_for_refill = use_next_ppu_read_for_refill;
	m_2007_read.waiting_for_refill_bus_read = false;
}

void ppu2c0x_device::schedule_2007_write(uint16_t addr, uint8_t data, int delay) {
	addr &= 0x3FFF;

	m_2007_write.pending = true;
	m_2007_write.delay = (delay > 0) ? delay : 0;
	m_2007_write.addr = addr;
	m_2007_write.data = data;
}

void ppu2c0x_device::schedule_2007_post_access_bump() {
	const bool rendering_now = (bg_pipeline_enabled || spr_pipeline_enabled) && is_visible_scanline();

	if (rendering_now) {
		ppu2007_post_bump_pending = true;
	} else {
		do_2007_post_access_bump(ppu_bus_source::PPU);
	}
}

void ppu2c0x_device::set_mapper(int mapper)
{
	m_mapper_number = mapper;

	//m_has_mmc5_ppu = mapper == 5;
	//m_has_mmc1_phase = mapper == 1;

	//resolve_mapper_ppu_devices();
}

/*void ppu2c0x_device::resolve_mapper_ppu_devices() {
	// MMC5 has special PPU read observation for scanline detection / ExROM behavior.
	if (m_has_mmc5_ppu && !m_mmc5)
		m_mmc5 = machine().root_device().subdevice<nes_exrom_device>("nes_slot:exrom");

	// MMC3-family boards watch PPU A12.  This includes official TxROM/MMC6
	// boards and many bootleg/multicart MMC3 clones.
	//
	// Do not resolve this by hardcoded board tags.  The NES slot contains the
	// loaded PCB device, and MMC3 clones derive from nes_txrom_device.  Walk the
	// children of "nes_slot" and find the active nes_txrom_device.
	if (m_has_mmc3_a12 && !m_mmc3) {
		device_t* const slot = machine().root_device().subdevice("nes_slot");

		if (slot) {
			for (device_t& dev : slot->subdevices()) {
				m_mmc3 = dynamic_cast<nes_txrom_device*>(&dev);
				if (m_mmc3)
					break;
			}
		}
	}

	// If no TxROM/MMC3-family PCB resolved, disable the A12 observer so the hot
	// PPU path does not keep doing useless checks.
	if (m_has_mmc3_a12 && !m_mmc3) {
		m_has_mmc3_a12 = false;
	}

	if (m_has_rambo1_a12 && !m_rambo1)
	{
		device_t *const slot = machine().root_device().subdevice("nes_slot");

		if (slot)
		{
			for (device_t &dev : slot->subdevices())
			{
				m_rambo1 = dynamic_cast<nes_tengen032_device *>(&dev);

				if (m_rambo1)
					break;
			}
		}
	}

	if (m_has_rambo1_a12 && !m_rambo1)
		m_has_rambo1_a12 = false;

	// Mapper 35 uses the J.Y. ASIC's unfiltered PPU A12 IRQ source.
	if (m_has_sc127_a12 && !m_sc127) {
		device_t *const slot = machine().root_device().subdevice("nes_slot");

		if (slot) {
			for (device_t &dev : slot->subdevices()) {
				m_sc127 = dynamic_cast<nes_sc127_device *>(&dev);

				if (m_sc127) {
					break;
				}
			}
		}
	}

	if (m_has_sc127_a12 && !m_sc127) {
		m_has_sc127_a12 = false;
	}
	
	if (m_has_batmap_srrx_a12 && !m_batmap_srrx)
	{
		device_t *const slot = machine().root_device().subdevice("nes_slot");

		if (slot)
		{
			for (device_t &dev : slot->subdevices())
			{
				m_batmap_srrx = dynamic_cast<nes_batmap_srrx_device *>(&dev);

				if (m_batmap_srrx)
					break;
			}
		}
	}

	if (m_has_batmap_srrx_a12 && !m_batmap_srrx)
		m_has_batmap_srrx_a12 = false;

	// MMC1 special SxROM-family phase feed.
	//
	// Only boards that repurpose MMC1 CHR register bits for PRG-RAM enable/banking
	// need to know which CHR register was last active on the PPU side.
	//
	// Plain SxROM/SLROM does normal MMC1 CHR banking through set_chr(); it does not
	// need per-fetch PPU phase feedback.
	if (m_has_mmc1_phase && !m_mmc1_sxrom) {
		m_mmc1_sxrom = machine().root_device().subdevice<nes_sxrom_device>("nes_slot:snrom");
		if (!m_mmc1_sxrom)
			m_mmc1_sxrom = machine().root_device().subdevice<nes_sxrom_device>("nes_slot:sorom");
		if (!m_mmc1_sxrom)
			m_mmc1_sxrom = machine().root_device().subdevice<nes_sxrom_device>("nes_slot:surom");
		if (!m_mmc1_sxrom)
			m_mmc1_sxrom = machine().root_device().subdevice<nes_sxrom_device>("nes_slot:sxrom_ext");
		if (!m_mmc1_sxrom)
			m_mmc1_sxrom = machine().root_device().subdevice<nes_sxrom_device>("nes_slot:szrom");
	}

	// Plain MMC1 boards don't need the phase feed. Turn it off if no special
	// SxROM-family device was found.
	if (m_has_mmc1_phase && !m_mmc1_sxrom)
		m_has_mmc1_phase = false;

	m_has_chr_latch = !m_latch.isnull();
}*/

void ppu2c0x_device::tick(int x) {

	ppu_tick_in_cpu_cycle = x;

	// Sprite-0 hit is detected by pixel overlap now, but $2002 bit 6
	// becomes visible two PPU dots later.
	if (sprite0_hit_pending) {
		if (sprite0_hit_delay > 0)
			--sprite0_hit_delay;

		if (sprite0_hit_delay == 0) {
			ppustatus_sprite0_hit = true;
			sprite0_hit_pending = false;
		}
	}

	// --------------------------------------------------
	// Delayed $2007 write
	//
	// Rendering $2007 writes do not hit VRAM immediately.  The CPU access places
	// the address on the PPU bus now, but the effective write reaches the PPU bus
	// after the delay recorded in m_2007_write.
	//
	// Non-rendering writes still happen immediately in the $2007 write handler;
	// this path is only for delayed rendering writes.
	// --------------------------------------------------
	if (m_2007_write.pending) {
		if (m_2007_write.delay > 0) {
			--m_2007_write.delay;
		} else {
			const uint16_t bus_addr = m_2007_write.addr & 0x3FFF;

			if ((bus_addr & 0x3F00) == 0x3F00) {
				ppu_address_bus = bus_addr;
				//m_palette_ram[bus_addr & 0x1F] = m_2007_write.data & 0x3F;
				palette_write(bus_addr & 0x1F, m_2007_write.data);
			} else {
				ppu_bus_address_drive(bus_addr, ppu_bus_source::PPU);
				writebyte(bus_addr, m_2007_write.data);
			}

			ppu2007_post_bump_pending = true;
			ppu2007_post_bump_delay = 1;

			m_2007_write.pending = false;
		}
	}

	// --------------------------------------------------
	// Delayed $2007 read/refill
	//
	// CPU $2007 reads return immediately from either:
	//   - ppudata_read_buffer for normal VRAM reads, or
	//   - palette RAM for palette reads.
	//
	// The internal read buffer refill happens later:
	//   - outside rendering, refill directly from the accessed address;
	//   - during rendering, wait for the next allowed real PPU fetch.
	// --------------------------------------------------
	if (m_2007_read.pending && !m_2007_read.waiting_for_refill_bus_read) {
		if (m_2007_read.delay > 0) {
			--m_2007_read.delay;
		} else {
			const uint16_t delayed_bus_addr = m_2007_read.addr & 0x3FFF;

			// Palette reads return palette data immediately, but the internal buffer
			// refills from the mirrored non-palette address.
			const uint16_t read_addr = ((delayed_bus_addr >= 0x3F00 && delayed_bus_addr <= 0x3FFF) ? (delayed_bus_addr & 0x2FFF) : delayed_bus_addr) & 0x3FFF;

			if (m_2007_read.use_next_ppu_read_for_refill) {
				m_2007_read.waiting_for_refill_bus_read = true;
				ppu2007_buffer_fill_arm_pending = true;

				const bool render_line = is_render_scanline();

				const bool bg_fetch_dot = render_line && (bg_pipeline_enabled || spr_pipeline_enabled) && (dot >= 1 && dot <= 256);

				const int bg_phase = (dot - 1) & 7;

				// General ALE+Read collision:
				//
				// If the delayed CPU $2007 read matures on a BG pattern address setup dot,
				// the normal fetch provides the high address bits, but the low external
				// latch may still be whatever the shared AD bus held from the previous data
				// phase.
				ppu2007_ale_read_addr_latch_poison = false;

				if (bg_fetch_dot && (bg_phase == 4 || bg_phase == 6)) {
					const uint16_t normal_addr = (ppuctrl_bg_pattern_base + (16 * bg_nt_latch) + (v >> 12) + ((bg_phase == 6) ? 8 : 0)) & 0x3FFF;

					const uint16_t poisoned_addr = (normal_addr & 0x3F00) | ppu_ad_latch;

					// Approximation of ALE+Read feedback stability.
					// Hardware does not perform this compare; this prevents unstable
					// feedback cases from becoming deterministic in the emulator.
					const uint8_t feedback_data = readbyte(poisoned_addr);

					if (feedback_data == ppu_ad_latch) {
						ppu2007_ale_read_addr_latch_poison = true;
						ppu2007_ale_read_low_latch = ppu_ad_latch;
					}
				}

				schedule_2007_post_access_bump();
			/*} else {
				// Non-rendering $2007 read:
				// The mapper sees the original $2007 access address.
				// The read buffer data may come from the palette mirror.
				ppu_bus_address_drive(delayed_bus_addr, ppu_bus_source::PPU);
				ppudata_read_buffer = readbyte(read_addr);

				m_2007_read.pending = false;
			}*/
			} else {
				// Non-rendering $2007 read:
				// The mapper sees the original $2007 access address.
				// The read buffer data may come from the palette mirror.
				ppu_bus_address_drive(delayed_bus_addr, ppu_bus_source::PPU);

				if (!m_mmc5_ppu_read.isnull()) {
					m_mmc5_ppu_read(delayed_bus_addr);
				}

				ppudata_read_buffer = readbyte(read_addr);
				
				// MMC2/MMC4 observe pattern-table reads performed through $2007.
				// Apply the latch only after the triggering byte has been fetched.
				if (read_addr < 0x2000 && !m_latch.isnull()) {
					//logerror("MMC2/MMC4 $2007 CHR read: %04X\n", read_addr);
					m_latch(read_addr);
				}
				
				m_2007_read.pending = false;
			}
		}
	}

	//log when a key is pressed, helps debugging of test roms
	/*if (g_nes_p1_a_pressed_edge) {
		logerror("[PPU] A pressed at sl=%d dot=%d\n", scanline, dot);
		g_nes_p1_a_pressed_edge = false;
	}*/

	// Start-of-scanline bookkeeping
	// dot 0 bookkeeping
	if (dot == 0) {
		sprite_eval_initialized = false;
		ppu_bus_read_can_fill_2007 = false;

		sprite_go_this_line = sprite_go_next_line;

		prev_pixel_valid = false;
		retro_ppumask_color = false;
		retro_ppumask_render = false;

		// One-shot scanline-0 stale sprite0 identity.
		// This is only armed by prerender sprite loading.
		if (scanline == 0) {
			sprite0_in_oam2_current = sl0_stale_sprite0_identity;
		
			sl0_stale_sprite0_identity = false;
			sl0_stale_s0_loaded = false;
		}
	}

	if (pending_2000.has_pending) {
		if (pending_2000.apply_ppu > 0)
			pending_2000.apply_ppu--;

		if (pending_2000.apply_ppu <= 0) {
			apply_delayed_2000(pending_2000.value);
			pending_2000.has_pending = false;
		}
	}

	if (pending_2006.has_pending) {
		if (pending_2006.apply_ppu > 0)
			pending_2006.apply_ppu--;

		if (pending_2006.apply_ppu <= 0) {
			const uint16_t ppuaddr_reload = pending_2006.value16 & 0x7FFF;

			const bool rendering_now =
				(bg_pipeline_enabled || spr_pipeline_enabled) &&
				is_visible_scanline();

			if (rendering_now) {
				t = ppuaddr_reload;
				copy_horiz();
				copy_vert();

				// W6/2 only replaces the external high-address source when it
				// matures between the NT setup/ALE dot and the NT read dot.
				//
				// Phase 0 already captured the old NT low byte in
				// ppu_ale_low_latch. ppu_bus_read() will combine that retained
				// low byte with the new high bits supplied here.
				if (dot >= 1 && dot <= 256 && ((dot - 1) & 7) == 1) {
					ppu_address_bus = v & 0x3FFF;
				}
			} else {
				v = ppuaddr_reload;

				if ((v & 0x3F00) == 0x3F00) {
					ppu_address_bus = v & 0x3FFF;
					ppu_ad_latch = v & 0xFF;
				} else {
					ppu_bus_address_drive(v, ppu_bus_source::PPU);
				}
			}

			pending_2006.has_pending = false;
		}
	}

	// Apply pending $2001 writes using a local countdown.
	if (pending_2001.has_pending) {
		if (pending_2001.apply_ppu > 0)
			pending_2001.apply_ppu--;

		if (pending_2001.apply_ppu <= 0) {
			apply_delayed_2001(pending_2001.value);
			pending_2001.has_pending = false;
		}
	}

	// 2C02 pre-render OAMADDR row copy.
	//
	// This is separate from render-disable OAM corruption. If render-disable
	// corruption is pending, that operation takes priority on this PPU cycle.
	if (is_prerender_scanline() && dot == 0 && (bg_pipeline_enabled || spr_pipeline_enabled) && !oam_corrupt_pending && (oamaddr & 0xF8)) {
		memcpy(&primary_oam[0], &primary_oam[oamaddr & 0xF8], 8);
	}

	// Apply OAM corruption exactly once:
	// on the first PPU cycle that occurs with rendering enabled on a visible/pre-render line.
	if (oam_corrupt_pending && (bg_pipeline_enabled || spr_pipeline_enabled)) {
		if (is_render_scanline()) {
			uint8_t row = oam_corrupt_seed & 0x1F;

			if (row != 0) {
				memcpy(primary_oam + row * 8, primary_oam, 8); // copy row 0 -> row 'row'
			}

			// Secondary OAM side effect mentioned by the test
			secondary_oam[row] = secondary_oam[0];

			oam_corrupt_pending = false;
		}
	}

	if (nmi_delay > 0 && --nmi_delay == 0) {
		if (nmi_pending) {
			m_maincpu6502->queue_delayed_nmi(1);
			nmi_pending = false;
		}
	}

	if (skip_dot && scanline == 0 && dot == 0) {
		dot++;
		skip_dot = false;
		screen().reset_origin(scanline, dot);
	}

	if (is_visible_scanline())
		run_visible_scanline_dot();
	else if (is_vblank_start_scanline())
		run_scanline_241_dot();
	else if (is_prerender_scanline())
		run_prerender_scanline_dot();

	//mmc3/mmc6 needs no variables but still needs the call to countdown IRQ
	//mmc5 needs scanline, (bg_output_enabled || spr_output_enabled) and dot
	//vrc6 needs no variables but still needs the call to countdown IRQ
	// Only mapper devices with a PPU tick callback need this.
	// Avoid call_mapper() overhead for plain carts.
	if (!m_ppu_to_mapper.isnull())
		m_ppu_to_mapper(scanline, dot, ppu_tick_in_cpu_cycle, ppu_address_bus);

	if (ppu2007_post_bump_pending) {
		if (ppu2007_post_bump_delay > 0) {
			--ppu2007_post_bump_delay;
		} else {
			do_2007_post_access_bump(ppu_bus_source::PPU);
			ppu2007_post_bump_pending = false;
			ppu2007_post_bump_delay = 0;
		}
	}

	// Odd-frame skip: skip dot 340, not 339
	//337 for no 2001 delay - 338
	//338 for 1 ppu cycle delay - 339
	//339 for 2 ppu cycle delay - 340
	if (is_ntsc_timing() && is_prerender_scanline() && dot == 338 && odd_frame && (bg_output_enabled || spr_output_enabled)) {
		skip_dot = true;
		sprite_sl0_early_shift_pending = sl0_stale_s0_loaded;

		//if (m_has_mmc3_a12 && m_mmc3)
		//	m_mmc3->notify_ppu_odd_skip();
		if (!m_ppu_odd_frame_skip.isnull())
			m_ppu_odd_frame_skip();
	}

	++dot;

	// Rendering $2007 reads mature at the start of the tick, but must not be
	// filled by bus reads that already occurred during this same dot. Arm the
	// fill only after the dot's PPU work is complete.
	if (ppu2007_buffer_fill_arm_pending) {
		ppu2007_buffer_fill_arm_pending = false;
		ppu2007_buffer_fill_armed = true;
	}

	//Each scanline lasts for 341 PPU clock cycles, with each clock cycle producing one pixel(dot).
	if (dot > 340) {
		dot = 0;
		++scanline;
		if (scanline == 240 && dot == 0) {
			ppu_address_bus = v & 0x3FFF;
		}
		if (scanline >= m_scanlines_per_frame) {
			scanline = 0;
			frame_start_ppu_phase = ppu_tick_in_cpu_cycle;
			suppress_vblank_flag = false;
			odd_frame = !odd_frame;
			screen().reset_origin(scanline, dot);
			frame++;
		}
	}

	m_scanline = scanline;

	//Start HBlank
	if (scanline <= 239 && dot == 257) {
		if (!m_hblank_callback_proc.isnull())
			m_hblank_callback_proc(scanline, ppustatus_vblank, (bg_pipeline_enabled || spr_pipeline_enabled));
	}
}

/*************************************
*
*  Scanline Rendering and Update
*
*************************************/
void ppu2c0x_device::retro_fix_previous_pixel_after_ppumask_write() {
	if (!prev_pixel_valid)
		return;

	if (prev_pixel_scanline != scanline)
		return;

	bitmap_rgb32& bitmap = m_bitmap;
	const unsigned pixel = prev_pixel_x;
	unsigned pal_index = 0;
	// Use the current visible PPUMASK state after the write.
	const bool bg_visible = bg_output_enabled && (ppumask_show_bg_left || pixel >= 8);
	const bool spr_visible = spr_output_enabled && (ppumask_show_spr_left || pixel >= 8);
	const bool rendering_disabled = (!bg_output_enabled && !spr_output_enabled);
	// Internal BG path for sprite-zero hit.
	// Do NOT include skip_bg_reload_once here.
	const unsigned bg_pat_internal = bg_visible ? prev_bg_pixel_pat : 0;
	// Display BG path. This is allowed to suppress the enable-edge stale pixel.
	const bool bg_display_visible = bg_visible && !skip_bg_reload_once;
	const unsigned bg_pat_display = bg_display_visible ? prev_bg_pixel_pat : 0;
	// IMPORTANT:
	// prev_spr_pat must be the raw sprite pixel from get_sprite_pixel(),
	// not the already visibility-masked visible_spr_pat.
	const unsigned spr_pat_display = spr_visible ? prev_spr_pat : 0;
	// Re-evaluate sprite 0 hit for the previous pixel under the new PPUMASK state.
	// Use internal BG, not display-suppressed BG.
	const bool right_edge_ok = (pixel != 255);

	/*if (sprite0_in_oam2_current &&
			prev_sprite0_pat && bg_pat_internal && bg_visible && spr_visible && right_edge_ok) {
		ppustatus_sprite0_hit = true;
	}*/
	if (sprite0_in_oam2_current && prev_sprite0_pat && bg_pat_internal && bg_visible && spr_visible && right_edge_ok) {
		if (!ppustatus_sprite0_hit && !sprite0_hit_pending) {
			sprite0_hit_pending = true;
			sprite0_hit_delay = 2;
		}
	}

	if (rendering_disabled) {
		pal_index = prev_backdrop_pal_index;
	} else if (spr_pat_display && !(prev_spr_behind_bg && bg_pat_display)) {
		pal_index = 0x10 + (prev_spr_pal << 2) + spr_pat_display;
	} else {
		if (!bg_pat_display)
			pal_index = 0;
		else
			pal_index = (prev_attr_bits << 2) | bg_pat_display;
	}

	//bitmap.pix(scanline, pixel) = m_nespens[apply_grayscale_and_emphasis(palette_read(pal_index))];
	bitmap.pix(prev_pixel_scanline, pixel) = m_nespens[apply_grayscale_and_emphasis(palette_read(pal_index))];

	retro_ppumask_color = false;
	retro_ppumask_render = false;
}

void ppu2c0x_device::set_nmi(bool s) {
	if (s) {
		if (!nmi_pending && nmi_delay == 0) {
			// Queue delayed arrival into CPU.
			// we delay 2 ppu clicks, gives enough time to cancel it
			nmi_pending = true;
			nmi_delay = 2;
		}
	} else {
		if (nmi_delay > 0 && nmi_pending) {
			m_maincpu6502->cancel_delayed_nmi();
		}
		nmi_pending = false;
		nmi_delay = 0;
	}
}

void ppu2c0x_device::run_bg_fetch_dot() {
	if (!(bg_pipeline_enabled || spr_pipeline_enabled))
		return;

	switch ((dot - 1) % 8) {
		case 0: {
			bg_fetch_v_nt = v & 0x7FFF;
			bg_fetch_nt_addr = (0x2000 | (bg_fetch_v_nt & 0x0FFF)) & 0x3FFF;

			// Normal NT ALE phase:
			// AD0-7 carry the low address and the external latch captures it.
			ppu_address_bus = bg_fetch_nt_addr;
			ppu_ad_latch = bg_fetch_nt_addr & 0xFF;
			ppu_ale_low_latch = ppu_ad_latch;
			break;
		}

		case 1: {
			ppu_bus_read_can_fill_2007 = true;
			bg_nt_latch = ppu_bus_read(ppu_address_bus, ppu_fetch_phase::NT);
			ppu_bus_read_can_fill_2007 = false;
			break;
		}

		case 2: {
			bg_fetch_v_at = v & 0x7FFF;
			bg_fetch_at_addr = 0x23C0 |	(bg_fetch_v_at & 0x0C00) | ((bg_fetch_v_at >> 4) & 0x38) | ((bg_fetch_v_at >> 2) & 0x07);
			bg_fetch_at_addr &= 0x3FFF;

			// Normal attribute ALE phase.
			ppu_address_bus = bg_fetch_at_addr;
			ppu_ad_latch = bg_fetch_at_addr & 0xFF;
			ppu_ale_low_latch = ppu_ad_latch;
			break;
		}

		case 3: {
			ppu_bus_read_can_fill_2007 = true;
			bg_at_latch = ppu_bus_read(ppu_address_bus, ppu_fetch_phase::AT);
			ppu_bus_read_can_fill_2007 = false;

			bg_at_latch = (bg_at_latch >> (((bg_fetch_v_at >> 4) & 4) | (bg_fetch_v_at & 2))) & 0x03;
			break;
		}

		case 4: {
			bg_fetch_v_pt = v & 0x7FFF;
			bg_fetch_pt_base = ppuctrl_bg_pattern_base;

			ppu_address_bus = (bg_fetch_pt_base + (16 * bg_nt_latch) + (bg_fetch_v_pt >> 12)) & 0x3FFF;

			if (ppu2007_ale_read_addr_latch_poison) {
				// ALE and $2007 Read are active together.
				//
				// PAR supplies the high address lines, but the external low
				// latch remains in the stable feedback state produced by the
				// preceding data value. Do not capture PAR's normal low byte.
				ppu_address_bus = (ppu_address_bus & 0x3F00) | ppu2007_ale_read_low_latch;

				ppu_ad_latch = ppu2007_ale_read_low_latch;
				ppu_ale_low_latch = ppu2007_ale_read_low_latch;
			} else {
				// Normal pattern-low ALE phase.
				ppu_ad_latch = ppu_address_bus & 0xFF;
				ppu_ale_low_latch = ppu_ad_latch;
			}

			break;
		}

		case 5:
			ppu_bus_read_can_fill_2007 = true;
			bg_pt_l_latch = ppu_bus_read(ppu_address_bus, ppu_fetch_phase::PTL);
			ppu_bus_read_can_fill_2007 = false;

			ppu2007_ale_read_addr_latch_poison = false;
			break;

		case 6:
			ppu_address_bus = bg_fetch_pt_base + (16 * bg_nt_latch) + (bg_fetch_v_pt >> 12) + 8;

			ppu_address_bus &= 0x3FFF;

			if (ppu2007_ale_read_addr_latch_poison) {
				// Same ALE+Read feedback behavior for pattern-high setup.
				ppu_address_bus = (ppu_address_bus & 0x3F00) | ppu2007_ale_read_low_latch;

				ppu_ad_latch = ppu2007_ale_read_low_latch;
				ppu_ale_low_latch = ppu2007_ale_read_low_latch;
			} else {
				// Normal pattern-high ALE phase.
				ppu_ad_latch = ppu_address_bus & 0xFF;
				ppu_ale_low_latch = ppu_ad_latch;
			}

			break;

		case 7:
			ppu_bus_read_can_fill_2007 = true;
			bg_pt_h_latch = ppu_bus_read(ppu_address_bus, ppu_fetch_phase::PTH);
			ppu_bus_read_can_fill_2007 = false;

			ppu2007_ale_read_addr_latch_poison = false;

			bump_horiz();
			break;
	}
}

void ppu2c0x_device::run_visible_scanline_dot() {
	if (!(bg_pipeline_enabled || spr_pipeline_enabled)) {
		uint8_t ob_addr = oamaddr;

		// While rendering is off in sprite-fetch time, the PPU is still reading OAM1.
		// The OAM1 row that matters on resume is selected by the preserved OAM2ADDR,
		// not by whichever output shifter we later notice has stale pattern bits.
		if (is_render_scanline() && dot >= 257 && dot <= 320) {
			const unsigned resume_oam2_slot = (corrupt_resume_oam2addr >> 2) & 7;

			uint8_t src = sprite_oam_source[resume_oam2_slot];

			if (src == 0xFF)
				src = sec_oam_source[resume_oam2_slot];

			if (src != 0xFF)
				ob_addr = src & 0xFC;
		}

		oam_latch_addr = ob_addr;
		oamdata_read_buffer = primary_oam[ob_addr];
		oamdata_latch = oamdata_read_buffer;
	}
	// AccuracyCoin Stale Sprite Shift Registers:
	//
	// Dot 257 normally transfers the sprite0 identity discovered by evaluation
	// into the current-line sprite unit state.  Forced blank must not perform that
	// transfer or clear the pending identity, because stale sprite shifter contents
	// and their sprite0 identity can survive until rendering is re-enabled.
	//
	// Sprite eval/reload follows the delayed pipeline-enable state, while the
	// sprite pattern shifters follow the immediate output-visible state.
	if (bg_pipeline_enabled || spr_pipeline_enabled) {
		switch (dot) {
			case 0:
				oam2addr = 0;
				break;

			case 1 ... 64:
				// OAM2 init/clear phase:
				// $2004 must see $FF through this whole window.
				oamdata_latch = 0xFF;
				oam2_last_write = 0xFF;
				oam_latch_addr = oam2addr & 0x1F;
				if (dot & 1) {
					oamdata_read_buffer = 0xFF;
				} else {
					// A new four-byte OAM2 slot begins at offsets 0, 4, 8...
					if ((oam2addr & 3) == 0)
						sec_oam_source[(oam2addr >> 2) & 7] = 0xFF;

					// Write the forced $FF into secondary OAM.
					secondary_oam[oam2addr & 0x1F] = 0xFF;
					oam2addr = (oam2addr + 1) & 0x1F;
				}
				break;

			case 65 ... 256:
				do_sprite_evaluation();
				break;
		}
	}

	if (dot == 257 && (bg_pipeline_enabled || spr_pipeline_enabled) && sprite0_in_oam2_next_valid) {
		sprite0_in_oam2_current = sprite0_in_oam2_next;
		sprite0_in_oam2_next = false;
		sprite0_in_oam2_next_valid = false;
	}

	if (dot >= 2 && dot <= 257) {
		do_pixel_output_and_sprite_zero();
	}

	if ((bg_pipeline_enabled || spr_pipeline_enabled) && ((dot >= 2 && dot <= 257) || (dot >= 322 && dot <= 337))) {
		clock_bg_shifters_only();
	}

	run_render_pipeline_dot();
}

void ppu2c0x_device::run_render_pipeline_dot() {
	if (dot == 0) {
		if (bg_pipeline_enabled || spr_pipeline_enabled) {
			ppu_address_bus = (ppuctrl_bg_pattern_base + (16 * bg_nt_latch) + (v >> 12)) & 0x3FFF;
		} else {
			ppu_address_bus = v & 0x3FFF;
		}
	}

	const bool pipe_render = (bg_pipeline_enabled || spr_pipeline_enabled);
	
	if (pipe_render && (dot == 63 || dot == 255 || dot == 339)) {
		oam2_increment_frozen = false;
	}

	switch (dot) {
		case 1 ... 256:
		case 321 ... 336:
			if (pipe_render) {
				run_bg_fetch_dot();

				if (dot == 256) {
					bump_vert();
				}
			}
			break;
		case 257: {
			//sprite_fetch_active_this_line = pipe_render;

			if (pipe_render) {
				// Latch OLD v before horizontal reload
				spr_fetch_v_old = v & 0x7FFF;

				// Precompute what v will become after copy_horiz()
				spr_fetch_v_new = (v & ~0x041F) | (t & 0x041F);
				spr_fetch_v_new &= 0x7FFF;

				oamaddr = 0;
				oam2addr = 0;
				copy_horiz();

				do_sprite_loading();
			}
			break;
		}

		case 258 ... 320:
			if (pipe_render) {
				do_sprite_loading();
			}
			break;

			// --- Dummy Nametable Fetches (Dots 337 & 339) ---
		case 337:
			if (pipe_render)
				ppu_address_bus = 0x2000 | (v & 0x0FFF);
			break;
		case 338:
			if (pipe_render) {
				ppu_bus_read_can_fill_2007 = true;
				(void)ppu_bus_read(ppu_address_bus, ppu_fetch_phase::DUMMY338);
				ppu_bus_read_can_fill_2007 = false;
			}
			break;
		case 339:
			// AccuracyCoin Stale Sprite Shift Registers:
			//
			// Dot 339 is the point where sprite shifter counters are told to
			// enter their normal "counting" mode for the next visible line.
			//
			// This decision must be recorded even when rendering is forced
			// blanked.  If dot 339 occurs during forced blank, do NOT leave the
			// previous value of sprite_go_next_line alive.  Explicitly record
			// false so the stale/halted sprite state can survive and draw
			// immediately when rendering is re-enabled.
			if (is_render_scanline()) {
				sprite_go_next_line = (bg_pipeline_enabled || spr_pipeline_enabled);
			}

			if (pipe_render)
				ppu_address_bus = 0x2000 | (v & 0x0FFF);

			break;
		case 340:
			if (pipe_render) {
				ppu_bus_read_can_fill_2007 = true;
				(void)ppu_bus_read(ppu_address_bus, ppu_fetch_phase::DUMMY340);
				ppu_bus_read_can_fill_2007 = false;
			}
			break;
	}

	if (pipe_render && ((dot >= 2 && dot <= 257) || (dot >= 322 && dot <= 337))) {
		reload_bg_shift_registers();
	}
}

unsigned ppu2c0x_device::get_sprite_pixel(unsigned& spr_pal, bool& spr_behind_bg, bool& spr_is_s0) {
	if (scanline >= 240 || dot < 2 || dot > 257) {
		sprite0_pat = 0;
		return 0;
	}

	unsigned const pixel = dot - 2;

	// /VIS for sprite pattern shifters is output-visible rendering.
	// Counters may continue through forced blank, but pattern shifters only
	// shift when rendering output is actually visible.
	const bool vis = (bg_output_enabled || spr_output_enabled);

	// Output is still gated by sprite-enable + left-8 rules (your existing behavior)
	const bool output_allowed = spr_output_enabled && (ppumask_show_spr_left || pixel >= 8);

	// Scanline-0 odd-frame skip glitch:
	// one early shift/output at X=0, then counting effectively offset by 1.
	if (scanline == 0 && dot == 2 && sprite_sl0_early_shift_pending) {
		sprite_sl0_early_shift_pending = false;

		if (!vis) {
			sprite0_pat = 0;
			return 0;
		}

		uint8_t slot0_pat = 0;

		if (spr_shift_count[0] < 8) {
			const unsigned p1 = (spr_pt_h_shift[0] & 0x80) ? 1 : 0;
			const unsigned p0 = (spr_pt_l_shift[0] & 0x80) ? 1 : 0;
			slot0_pat = uint8_t((p1 << 1) | p0);
		}

		sprite0_pat = (sprite0_in_oam2_current && output_allowed) ? slot0_pat : 0;

		if (!output_allowed || !slot0_pat)
			return 0;

		spr_pal = spr_attr_latch[0] & 3;
		spr_behind_bg = spr_attr_latch[0] & 0x20;
		spr_is_s0 = true;
		return slot0_pat;
	}

	uint8_t slot0_pat = 0;
	uint8_t first_pat = 0;
	unsigned first_index = 8;

	for (unsigned i = 0; i < 8; ++i) {
		uint8_t pat = 0;

		if (spr_shift_count[i] < 8) {
			// X counter counts down even if rendering disabled once started.
			if (spr_x_counter[i] > 0 && sprite_go_this_line) {
				spr_x_counter[i]--;
			} else if (vis) {
				const unsigned p1 = (spr_pt_h_shift[i] & 0x80) ? 1 : 0;
				const unsigned p0 = (spr_pt_l_shift[i] & 0x80) ? 1 : 0;
				pat = uint8_t((p1 << 1) | p0);

				spr_pt_h_shift[i] <<= 1;
				spr_pt_l_shift[i] <<= 1;
				spr_shift_count[i]++;
			}
		}

		// Slot0 pixel for sprite0-hit logic.
		if (i == 0)
			slot0_pat = pat;

		// Keep first non-transparent sprite by priority, but do not return yet.
		// All sprite shifters/counters above must still be serviced.
		if (pat && first_index == 8) {
			first_pat = pat;
			first_index = i;
		}
	}

	sprite0_pat = (output_allowed && sprite0_in_oam2_current) ? slot0_pat : 0;

	if (!output_allowed || first_index == 8)
		return 0;

	spr_pal = spr_attr_latch[first_index] & 3;
	spr_behind_bg = spr_attr_latch[first_index] & 0x20;
	spr_is_s0 = (first_index == 0);
	return first_pat;
}

void ppu2c0x_device::do_pixel_output_and_sprite_zero() {
	bitmap_rgb32& bitmap = m_bitmap;
	unsigned pixel = dot - 2;
	unsigned pal_index;

	const bool render_line = is_render_scanline();
	const bool rendering_disabled = (!bg_output_enabled && !spr_output_enabled);

	// If rendering disabled, we still allow sprite unit bookkeeping to run,
	// but we force the output color.
	if (render_line && rendering_disabled) {
		{
			bool spr_behind_bg = false;
			bool spr_is_s0 = false;
			unsigned spr_pal = 0;
			(void)get_sprite_pixel(spr_pal, spr_behind_bg, spr_is_s0);
			// Do NOT set ppustatus_sprite0_hit here; rendering is disabled.
		}

		// Apply retroactive PPUMASK fix to the PREVIOUS pixel first
		if (retro_ppumask_render || retro_ppumask_color)
			retro_fix_previous_pixel_after_ppumask_write();

		uint16_t va = v & 0x3FFF;
		pal_index = ((va & 0x3F00) == 0x3F00) ? (va & 0x1F) : 0;

		bitmap.pix(scanline, pixel) = m_nespens[apply_grayscale_and_emphasis(palette_read(pal_index))];

		// Latch this pixel as the "previous pixel" for any later retroactive fix
		prev_pixel_valid = true;
		prev_pixel_scanline = scanline;
		prev_pixel_x = pixel;

		prev_bg_pixel_pat = 0;
		prev_attr_bits = 0;
		prev_spr_pat = 0;
		prev_spr_pal = 0;
		prev_spr_behind_bg = false;
		prev_spr_is_s0 = false;
		prev_sprite0_pat = sprite0_pat;
		prev_backdrop_pal_index = pal_index;

		return;
	}

	unsigned bg_pixel_pat = 0;
	bool spr_behind_bg = false;
	bool spr_is_s0 = false;
	unsigned spr_pal = 0;
	unsigned attr_bits = 0;

	// IMPORTANT: BG visibility must use delayed (architectural) enable
	bool bg_visible = bg_output_enabled && (ppumask_show_bg_left || pixel >= 8);

	unsigned const spr_pat = get_sprite_pixel(spr_pal, spr_behind_bg, spr_is_s0);

	if (bg_visible) {
		bg_pixel_pat = (NTH_BIT(bg_pt_h_shift, 15 - fine_x) << 1) | NTH_BIT(bg_pt_l_shift, 15 - fine_x);

		attr_bits = (NTH_BIT(bg_at_h_shift, 15 - fine_x) << 1) | NTH_BIT(bg_at_l_shift, 15 - fine_x);
	}

	// Sprite zero hit conditions
	bool const left8_ok = (pixel >= 8) || (ppumask_show_bg_left && ppumask_show_spr_left);
	bool const right_edge_ok = (dot != 257);

	// Use sprite0_pat (slot0 pixel), NOT the "winning" sprite pixel.
	/*if (sprite0_in_oam2_current &&
		sprite0_pat && bg_pixel_pat && bg_output_enabled && spr_output_enabled && left8_ok && right_edge_ok) {
		ppustatus_sprite0_hit = true;
	}*/
	if (sprite0_in_oam2_current && sprite0_pat && bg_pixel_pat && bg_output_enabled && spr_output_enabled && left8_ok && right_edge_ok) {
		ppustatus_sprite0_hit = true;
	}

	if (spr_pat && !(spr_behind_bg && bg_pixel_pat)) {
		pal_index = 0x10 + (spr_pal << 2) + spr_pat;
	} else {
		if (!bg_pixel_pat)
			pal_index = 0;
		else
			pal_index = (attr_bits << 2) | bg_pixel_pat;
	}

	// Apply retroactive PPUMASK fix to the PREVIOUS pixel first
	if (retro_ppumask_render || retro_ppumask_color)
		retro_fix_previous_pixel_after_ppumask_write();

	bitmap.pix(scanline, pixel) = m_nespens[apply_grayscale_and_emphasis(palette_read(pal_index))];

	// Latch this pixel as the "previous pixel" for any later retroactive fix
	prev_pixel_valid = true;
	prev_pixel_scanline = scanline;
	prev_pixel_x = pixel;

	prev_bg_pixel_pat = bg_pixel_pat;
	prev_attr_bits = attr_bits;

	prev_spr_pat = spr_pat;
	prev_spr_pal = spr_pal;
	prev_spr_behind_bg = spr_behind_bg;
	prev_spr_is_s0 = spr_is_s0;

	prev_sprite0_pat = sprite0_pat;
	{
		uint16_t va = v & 0x3FFF;
		prev_backdrop_pal_index = ((va & 0x3F00) == 0x3F00) ? (va & 0x1F) : 0;
	}
}

inline void ppu2c0x_device::clock_bg_shifters_only() {
	bg_pt_l_shift = uint16_t(bg_pt_l_shift << 1); // low plane serial-in is 0
	bg_pt_h_shift = uint16_t((bg_pt_h_shift << 1) | 1);

	bg_at_l_shift = uint16_t((bg_at_l_shift << 1) | bg_at_latch_l);
	bg_at_h_shift = uint16_t((bg_at_h_shift << 1) | bg_at_latch_h);
}

// only reloads the upper eight bits and
// the attribute bits every eight pixels
void ppu2c0x_device::reload_bg_shift_registers() {
	if (!(bg_pipeline_enabled || spr_pipeline_enabled)) {
		return;
	}

	// Reload happens once per tile, after the high pattern byte fetch
	// Using your existing timing: dot % 8 == 1
	if ((dot & 7) == 1) {
		// Skip exactly ONE reload when rendering is enabled mid-scanline
		if (skip_bg_reload_once) {
			skip_bg_reload_once = false;
			return;
		}

		// ---- Pattern shifter reload (low 8 bits) ----
		bg_pt_l_shift = (bg_pt_l_shift & 0xFF00) | bg_pt_l_latch;
		bg_pt_h_shift = (bg_pt_h_shift & 0xFF00) | bg_pt_h_latch;

		// ---- Attribute latch + reload (8 copies) ----
		bg_at_latch_l = bg_at_latch & 1;
		bg_at_latch_h = (bg_at_latch >> 1) & 1;

		bg_at_l_shift = (bg_at_l_shift & 0xFF00) | (bg_at_latch_l ? 0xFF : 0x00);
		bg_at_h_shift = (bg_at_h_shift & 0xFF00) | (bg_at_latch_h ? 0xFF : 0x00);
	}
}

void ppu2c0x_device::do_sprite_evaluation() {
	// ------------------------------------------------------------------------
	// Evaluation setup (dot 65)
	//
	// This is the start of the sprite-evaluation band for the scanline.
	// We initialize per-scanline evaluation state here.
	// ------------------------------------------------------------------------
	if (!sprite_eval_initialized) {
		sprite_eval_initialized = true;
		sprite0_in_oam2_next = false;
		sprite0_in_oam2_next_valid = true;
		// ppustatus_sprite_overflow is intentionally NOT cleared here.
		// It is the PPUSTATUS-visible flag and is cleared elsewhere.

		sprite_eval_in_range = false;
		oam2addr = 0;
		overflow_bug_counter = 0;
		oam_copy_done = false;

		oam_eval_addr = oamaddr;

		spr_pt_addr_h = (oam_eval_addr >> 2) & 0x3F;
		spr_pt_addr_l = oam_eval_addr & 0x03;

		// This is the part that got lost:
		// If evaluation starts aligned, we are already in byte-0 compare mode.
		// If it starts misaligned, we stay in the misaligned walk until the
		// first failed compare realigns us.
		m_oam_eval_realigned = (spr_pt_addr_l == 0);

		sprite0_eval_addr = oamaddr;
		oam2_full = false;
		oam2_last_write = 0xFF;

		s_after_wrap = false;
		m_eval_wrap_byte = 0x00;
		m_eval_prev_oam_latch_addr = 0xFF;

		overflow_eval_phase = 0;
		overflow_finish_bytes = 0;
	}

	// ------------------------------------------------------------------------
	// Odd dots: read from primary OAM.
	//
	// In this implementation, odd dots perform the primary OAM read and drive
	// the internal OAM bus / $2004-visible latch.
	// ------------------------------------------------------------------------
	if (dot & 0x01) {
		oam_latch_addr = (uint8_t)oam_eval_addr;
		oamdata_read_buffer = primary_oam[oam_eval_addr];

		// Primary OAM drives the bus on odd dots.
		oamdata_latch = oamdata_read_buffer;

		// Capture the byte read at $FC so post-wrap behavior can replay it.
		if (oam_latch_addr == 0xFC)
			m_eval_wrap_byte = oamdata_latch;

		// Detect internal OAM address wrap from $FC -> $00.
		if (m_eval_prev_oam_latch_addr == 0xFC && oam_latch_addr == 0x00)
			s_after_wrap = true;

		m_eval_prev_oam_latch_addr = oam_latch_addr;
		return;
	}

	// ------------------------------------------------------------------------
	// Even dots: act on the previously-read primary OAM byte.
	// ------------------------------------------------------------------------
	if (s_after_wrap) {
		// After FC->00 wrap, the even-dot bus/latch is forced to the captured
		// wrap byte.
		oam_latch_addr = 0;
		oamdata_latch = m_eval_wrap_byte;
		oam2_last_write = oamdata_latch;
	}

	uint8_t const orig_oam_data = oamdata_read_buffer;
	int const sprite_check_y = scanline & 0xFF;
	int const spr_h = (ppuctrl_sprite_size == EIGHT_BY_EIGHT) ? 8 : 16;

	// Sprite-0 identity comes from the original evaluation start.
	uint8_t const startH = (sprite0_eval_addr >> 2) & 0x3F;
	uint8_t const startL = sprite0_eval_addr & 0x03;
	bool const misaligned_start = (startL != 0);

	auto set_eval_addr = [&]() {
		spr_pt_addr_h = (oam_eval_addr >> 2) & 0x3F;
		spr_pt_addr_l = oam_eval_addr & 0x03;

		if (oam_eval_addr == 0)
			oam_copy_done = true;
	};

	auto step_copy_addr = [&]() {
		// Copy-chain increment: +1 with normal carry through the low bits.
		oam_eval_addr = (oam_eval_addr + 1) & 0xFF;
		set_eval_addr();
	};

	auto step_failed_compare_addr = [&]() {
		// Failed Y compare: +4 mode clears the low two OAM address bits.
		oam_eval_addr = (oam_eval_addr + 4) & 0xFC;
		m_oam_eval_realigned = true;
		set_eval_addr();
	};

	auto step_no_compare_addr = [&]() {
		// No comparator result this cycle. Keep the existing diagonal walk
		// while still in the original misaligned phase; once +4 mode has
		// realigned evaluation, failed scans compare byte 0 entries.
		spr_pt_addr_h = (spr_pt_addr_h + 1) & 0x3F;

		if (misaligned_start && !m_oam_eval_realigned)
			spr_pt_addr_l = (spr_pt_addr_l + 1) & 0x03;
		else
			spr_pt_addr_l = 0;

		oam_eval_addr = (spr_pt_addr_l & 0x03) | ((spr_pt_addr_h & 0x3F) << 2);

		if (oam_eval_addr == 0)
			oam_copy_done = true;
	};

	// ------------------------------------------------------------------------
	// End-of-copy handling
	//
	// Once copying is done, evaluation does not necessarily stop immediately.
	// If secondary OAM filled, the PPU continues the failed-copy / OAM2-readback
	// behavior until HBlank.
	// ------------------------------------------------------------------------
	if (oam_copy_done) {
		if (oam2_full) {
			// Keep the OAM2 side visible on $2004 during the remaining eval cycles.
			oam_latch_addr = 0x00;
			oamdata_latch = secondary_oam[0];
			oam2_last_write = oamdata_latch;

			// Failed-copy phase scans Y bytes only: n++, m=0.
			spr_pt_addr_h = (spr_pt_addr_h + 1) & 0x3F;
			spr_pt_addr_l = 0;

			oam_eval_addr = (spr_pt_addr_l & 0x03) | ((spr_pt_addr_h & 0x3F) << 2);
			return;
		}

		spr_pt_addr_h = (spr_pt_addr_h + 1) & 0x3F;
		oam_eval_addr = (spr_pt_addr_l & 0x03) | ((spr_pt_addr_h & 0x3F) << 2);
		return;
	}

	// ------------------------------------------------------------------------
	// Secondary OAM FULL path
	//
	// Once OAM2 is full, writes become OAM2 readback and evaluation enters the
	// sprite-overflow search / tail behavior.
	// ------------------------------------------------------------------------
	if (oam2addr >= 0x20) {
		oam2_full = true;

		// With OAM2 full, the OAM2 side becomes readback instead of write.
		// For this logic, keep $2004 seeing OAM2[0]'s Y byte.
		oam_latch_addr = 0x00;
		oamdata_latch = secondary_oam[0];
		oam2_last_write = oamdata_latch;

		switch (overflow_eval_phase) {
				// ------------------------------------------------------------
				// Phase 0: diagonal overflow search
				//
				// Every current primary OAM byte is treated as a potential Y byte.
				// If not in range: increment n and m without carry.
				// If in range: set overflow and move to phase 1.
				// ------------------------------------------------------------
			case 0: {
				// Attribute byte (byte 2) is compared as readback-visible data.
				uint8_t compare_data = orig_oam_data;
				if (spr_pt_addr_l == 2)
					compare_data &= 0xE3;

				bool const nowInRange = (sprite_check_y >= compare_data) && (sprite_check_y < (int(compare_data) + spr_h));

				if (nowInRange) {
					ppustatus_sprite_overflow = true;
					overflow_eval_phase = 1;
					overflow_finish_bytes = 3;

					// Move to the next entry after the one that matched.
					spr_pt_addr_l = (spr_pt_addr_l + 1) & 0x03;
					if (spr_pt_addr_l == 0)
						spr_pt_addr_h = (spr_pt_addr_h + 1) & 0x3F;
				} else {
					// Hardware bug: increment n and m without carry.
					spr_pt_addr_h = (spr_pt_addr_h + 1) & 0x3F;
					spr_pt_addr_l = (spr_pt_addr_l + 1) & 0x03;
				}
				break;
			}

				// ------------------------------------------------------------
				// Phase 1: finish the found sprite's next 3 entries.
				// ------------------------------------------------------------
			case 1: {
				if (overflow_finish_bytes > 0)
					overflow_finish_bytes--;

				spr_pt_addr_l = (spr_pt_addr_l + 1) & 0x03;
				if (spr_pt_addr_l == 0)
					spr_pt_addr_h = (spr_pt_addr_h + 1) & 0x3F;

				if (overflow_finish_bytes == 0) {
					// After those 3 entries, switch to failed-copy scan.
					overflow_eval_phase = 2;
					spr_pt_addr_l = 0;
				}
				break;
			}

				// ------------------------------------------------------------
				// Phase 2: failed-copy scan (Y-only).
				//
				// Attempt and fail to copy OAM[n][0], then increment n only.
				// ------------------------------------------------------------
			case 2:
			default: {
				spr_pt_addr_h = (spr_pt_addr_h + 1) & 0x3F;
				spr_pt_addr_l = 0;
				break;
			}
		}

		if (spr_pt_addr_h == 0)
			oam_copy_done = true;

		oam_eval_addr = (spr_pt_addr_l & 0x03) | ((spr_pt_addr_h & 0x3F) << 2);
		return;
	}

	// ------------------------------------------------------------------------
	// NOT FULL path
	//
	// Normal sprite evaluation while OAM2 still has room.
	// ------------------------------------------------------------------------
	bool eval_compare_done = false;

	if (!sprite_eval_in_range) {
		bool const is_first_eval_byte = (spr_pt_addr_h == startH) && (spr_pt_addr_l == startL);

		bool const do_compare = is_first_eval_byte || (m_oam_eval_realigned ? (spr_pt_addr_l == 0) : (!misaligned_start ? (spr_pt_addr_l == 0) : (spr_pt_addr_l == startL)));

		if (do_compare) {
			eval_compare_done = true;

			bool const nowInRange = (sprite_check_y >= orig_oam_data) && (sprite_check_y < (int(orig_oam_data) + spr_h));

			if (nowInRange) {
				sprite_eval_in_range = true;
				overflow_bug_counter = 0;

				// Sprite 0 is identified when the original first eval byte matches.
				if (is_first_eval_byte)
					sprite0_in_oam2_next = true;
			}
		}
	}

	// ------------------------------------------------------------------------
	// In-range sprite: copy 4 bytes into secondary OAM.
	// ------------------------------------------------------------------------
	if (sprite_eval_in_range) {
		if ((oam2addr & 3) == 0)
			sec_oam_source[(oam2addr >> 2) & 7] = oam_eval_addr & 0xFC;

		oam_latch_addr = (uint8_t)oam2addr;

		if (spr_pt_addr_l == 2) {
			uint8_t const w = orig_oam_data & 0xE3;
			secondary_oam[oam2addr] = w;
			oam2_last_write = w;
			oamdata_latch = w;
		} else {
			secondary_oam[oam2addr] = orig_oam_data;
			oam2_last_write = orig_oam_data;
			oamdata_latch = orig_oam_data;
		}

		oam2addr++;

		if (!oam2_full && oam2addr >= 0x20) {
			oam2_full = true;
			oam2_increment_frozen = true;
		}

		step_copy_addr();

		overflow_bug_counter++;
		if (overflow_bug_counter >= 4) {
			bool const x_as_y_in_range = (sprite_check_y >= orig_oam_data) && (sprite_check_y < (int(orig_oam_data) + spr_h));

			sprite_eval_in_range = false;
			overflow_bug_counter = 0;

			if (!x_as_y_in_range) {
				oam_eval_addr &= 0xFC;
				m_oam_eval_realigned = true;
				set_eval_addr();
			}
		}
	} else {
		// --------------------------------------------------------------------
		// Not in range: repeatedly overwrite the current OAM2 slot without
		// incrementing oam2addr.
		// --------------------------------------------------------------------
		if (!s_after_wrap) {
			uint8_t w = orig_oam_data;
			if (spr_pt_addr_l == 2)
				w &= 0xE3;

			oam_latch_addr = (uint8_t)oam2addr;
			secondary_oam[oam2addr] = w;
			oam2_last_write = w;
			oamdata_latch = w;
		}

		if (eval_compare_done)
			step_failed_compare_addr();
		else
			step_no_compare_addr();
	}

	// Rebuild the flat primary OAM address from n/m.
	// spr_pt_addr_h/L are just cached bitfields of the internal eval address:
	//   H = bits 7..2
	//   L = bits 1..0
	oam_eval_addr = (spr_pt_addr_l & 0x03) | ((spr_pt_addr_h & 0x3F) << 2);
}

// Returns 'true' if the sprite is in range
bool ppu2c0x_device::calc_sprite_tile_addr(uint8_t y, uint8_t index, uint8_t attrib, bool is_high) {
	unsigned const diff = (scanline & 0xFF) - y;

	if (ppuctrl_sprite_size == EIGHT_BY_EIGHT) {
		if (diff >= 8) {
			ppu_address_bus = ppuctrl_sprite_pattern_base + (16 * index) + (8 * is_high);
			ppu_address_bus &= 0x3FFF;
			return false;
		}

		unsigned const row = (attrib & 0x80) ? (7 - diff) : diff;

		ppu_address_bus = ppuctrl_sprite_pattern_base + (16 * index) + (8 * is_high) + row;
		ppu_address_bus &= 0x3FFF;
		return true;
	}

	if (diff >= 16) {
		ppu_address_bus = 0x1000 * (index & 1) + (16 * (index & 0xFE)) + (8 * is_high);
		ppu_address_bus &= 0x3FFF;
		return false;
	}

	unsigned const row = (attrib & 0x80) ? (15 - diff) : diff;

	ppu_address_bus = 0x1000 * (index & 1) + (16 * (index & 0xFE)) + ((row & 8) << 1) + (8 * is_high) + (row & 7);
	ppu_address_bus &= 0x3FFF;
	return true;
}

// Initializes the sprite output units with the sprites that were copied into
// secondary OAM during sprite evaluation.
void ppu2c0x_device::do_sprite_loading() {
	const unsigned sprite_n = (dot - 257) / 8;
	const unsigned base = (sprite_n * 4) & 0x1F;
	const unsigned spr_load_phase = (dot - 1) & 7;

	// --------------------------------------------------
	// OAM2 visibility / $2004 latch side
	// --------------------------------------------------
	//
	// During sprite-fetch dots 257-320:
	//
	//   - OAM2 is read every dot.
	//   - OAM2ADDR advances after phases 0, 1, 2, and 7.
	//   - Dot 257 clears OAM2ADDR before this function is called.
	//
	// This models OAM2 read/latch visibility only.
	//
	// Do not use OAM2ADDR to determine when the sprite output units receive
	// Y, tile, attributes, or X. Those transfers remain tied to the fixed
	// eight-dot sprite-fetch phase below.
	const uint8_t oam2_read = secondary_oam[oam2addr & 0x1F];

	// --------------------------------------------------
	// Forced-blank sprite FIFO resume
	// --------------------------------------------------
	//
	// The rendering-enable edge only arms corrupt_resume_fifo_pending.
	//
	// Destination selection comes from the current sprite-fetch slot.
	// Source selection comes from the OAM2ADDR value preserved when
	// rendering was disabled.
	//
	// At this point, oamdata_read_buffer still contains the previous dot's
	// OAM1 access. The current OAM2 read must not replace it until after
	// the resumed transfer consumes that old OB value.
	if (corrupt_resume_fifo_pending) {
		const unsigned src_n = (corrupt_resume_oam2addr >> 2) & 7;

		// Attribute load occurs during phase 2.
		//
		// From phase 3 onward, the normal attribute load strobe has already
		// passed, so the destination retains stale source-lane attributes.
		if (spr_load_phase >= 3) {
			spr_attr_latch[sprite_n] = spr_attr_latch[src_n];
		}

		// X/down-counter load occurs during phase 3.
		//
		// From phase 4 onward, the normal X load strobe has already passed,
		// so previous OB becomes the destination X/down-counter.
		if (spr_load_phase >= 4) {
			spr_x_latch[sprite_n] = oamdata_read_buffer;
			spr_x_counter[sprite_n] = oamdata_read_buffer;
		}

		// Pattern-low load occurs during phase 5.
		//
		// During phases 6-7, the normal low-pattern load strobe has already
		// passed, so the destination retains the stale source-lane low plane.
		if (spr_load_phase >= 6) {
			spr_pt_l_shift[sprite_n] = spr_pt_l_shift[src_n];
		}

		// The stale high plane enters the destination lane. The normal phase-7
		// CHR read still occurs, but its destination write is inhibited once.
		spr_pt_h_shift[sprite_n] = spr_pt_h_shift[src_n];
		corrupt_resume_high_lane = sprite_n;

		spr_shift_count[sprite_n] = 0;
		corrupt_resume_fifo_pending = false;
	}

	// Publish the current OAM2 byte after the resume path has consumed
	// the previous OAM buffer value.
	oamdata_read_buffer = oam2_read;
	oam_latch_addr = oam2addr & 0x1F;
	oamdata_latch = oam2_read;

	// --------------------------------------------------
	// Eight-dot sprite-fetch sequence
	// --------------------------------------------------
	switch (spr_load_phase) {
		case 0: {
			// Dot 257, 265, 273...
			//
			// Set up garbage nametable fetch #1 and load the sprite Y byte into
			// the render-unit staging latch.
			sprite_oam_source[sprite_n] = sec_oam_source[sprite_n];

			if (dot == 257) {
				const uint16_t old_nt_addr = (0x2000 | (spr_fetch_v_old & 0x0FFF)) & 0x3FFF;
				const uint16_t new_nt_addr = (0x2000 | (spr_fetch_v_new & 0x0FFF)) & 0x3FFF;
				ppu_address_bus = (new_nt_addr & 0x3F00) | (old_nt_addr & 0x00FF);
				ppu_address_bus &= 0x3FFF;
			} else {
				ppu_address_bus = (0x2000 | (spr_fetch_v_new & 0x0FFF)) & 0x3FFF;
			}

			//eval_sprite_y = secondary_oam[(base + 0) & 0x1F];
			eval_sprite_y = oam2_read;
			break;
		}

		case 1: {
			// Dot 258, 266, 274...
			//
			// Read garbage nametable fetch #1 and load the sprite tile byte.
			ppu_bus_read_can_fill_2007 = true;
			(void)ppu_bus_read(ppu_address_bus, ppu_fetch_phase::SPR_NT);
			ppu_bus_read_can_fill_2007 = false;
			//eval_sprite_tile = secondary_oam[(base + 1) & 0x1F];
			eval_sprite_tile = oam2_read;
			break;
		}

		case 2: {
			// Dot 259, 267, 275...
			//
			// Set up garbage nametable fetch #2 and assert the normal
			// attribute-latch load strobe.
			ppu_address_bus = (0x2000 | (spr_fetch_v_new & 0x0FFF)) & 0x3FFF;
			//spr_attr_latch[sprite_n] = secondary_oam[(base + 2) & 0x1F];
			spr_attr_latch[sprite_n] = oam2_read;
			break;
		}

		case 3: {
			// Dot 260, 268, 276...
			//
			// Read garbage nametable fetch #2 and assert the normal
			// X/down-counter load strobe.
			ppu_bus_read_can_fill_2007 = true;
			(void)ppu_bus_read(ppu_address_bus, ppu_fetch_phase::SPR_AT);
			ppu_bus_read_can_fill_2007 = false;
			
			// A forced-blank resume has already selected the corrupted output
			// unit and armed its one-shot high-pattern write inhibit. Preserve
			// that unit's known-good StarTropics X behavior.
			//
			// Every normal output unit loads X from the OAM2 read bus. When
			// OAM2ADDR is frozen at zero, this supplies OAM2[0] as required.
			if (sprite_n == corrupt_resume_high_lane) {
				spr_x_latch[sprite_n] = secondary_oam[(base + 3) & 0x1F];
			} else {
				spr_x_latch[sprite_n] = oam2_read;
			}

			spr_x_counter[sprite_n] = spr_x_latch[sprite_n];
			
			//spr_x_latch[sprite_n] = secondary_oam[(base + 3) & 0x1F];
			//spr_x_counter[sprite_n] = spr_x_latch[sprite_n];
			break;
		}

		case 4: {
			// Dot 261, 269, 277...
			//
			// Set up the sprite pattern-low fetch address.
			eval_candidate_in_range = calc_sprite_tile_addr(eval_sprite_y, eval_sprite_tile, spr_attr_latch[sprite_n], false);
			break;
		}

		case 5: {
			// Dot 262, 270, 278...
			//
			// Read sprite pattern-low data and assert the normal
			// low-plane load strobe.
			ppu_bus_read_can_fill_2007 = true;
			const uint8_t pat = ppu_bus_read(ppu_address_bus, ppu_fetch_phase::SPR_PTL);
			ppu_bus_read_can_fill_2007 = false;
			spr_pt_l_shift[sprite_n] = eval_candidate_in_range ? pat : 0;

			if (eval_candidate_in_range && (spr_attr_latch[sprite_n] & 0x40)) {
				spr_pt_l_shift[sprite_n] = rev_byte(spr_pt_l_shift[sprite_n]);
			}

			break;
		}

		case 6: {
			// Dot 263, 271, 279...
			//
			// Set up the sprite pattern-high fetch address.
			eval_candidate_in_range = calc_sprite_tile_addr(eval_sprite_y, eval_sprite_tile, spr_attr_latch[sprite_n], true);
			break;
		}

		case 7: {
			// Dot 264, 272, 280...
			//
			// The high-pattern CHR read always occurs. A forced-blank resume
			// can inhibit only the destination FIFO's high-plane load strobe.
			ppu_bus_read_can_fill_2007 = true;
			const uint8_t pat = ppu_bus_read(ppu_address_bus, ppu_fetch_phase::SPR_PTH);
			ppu_bus_read_can_fill_2007 = false;

			// The pre-render line reuses the secondary OAM left by the final visible
			// scanline. Any in-range lane-0 entry can reach output, but sprite-0
			// identity remains the identity produced by the original evaluation.
			if (is_prerender_scanline() && eval_candidate_in_range && sprite_n == 0) {
				sl0_stale_s0_loaded = true;
				sl0_stale_sprite0_identity = sprite0_in_oam2_current;
			}

			if (sprite_n != corrupt_resume_high_lane) {
				spr_pt_h_shift[sprite_n] = eval_candidate_in_range ? pat : 0;

				if (eval_candidate_in_range && (spr_attr_latch[sprite_n] & 0x40)) {
					spr_pt_h_shift[sprite_n] = rev_byte(spr_pt_h_shift[sprite_n]);
				}
			} else {
				// The stale high byte is already present in this lane.
				// Suppress this one FIFO write, then release the inhibit.
				corrupt_resume_high_lane = 0xFF;
			}

			spr_shift_count[sprite_n] = 0;
			break;
		}

		default:
			break;
	}

	// --------------------------------------------------
	// OAM2ADDR increment timing during sprite loading
	// --------------------------------------------------
	//
	// Each eight-dot sprite slot exposes:
	//
	//   byte 0, byte 1, byte 2, byte 3,
	//   byte 3, byte 3, byte 3, byte 3
	if (!oam2_increment_frozen) {
		if (spr_load_phase < 3 || spr_load_phase == 7) {
			oam2addr = (oam2addr + 1) & 0x1F;

			if (oam2addr == 0) {
				oam2_increment_frozen = true;
			}
		}
	}
}

// Called for dots on the pre-render line
void ppu2c0x_device::run_prerender_scanline_dot() {
	if (dot == 1) {
		ppustatus_sprite_overflow = ppustatus_sprite0_hit = false;
		sprite0_hit_pending = false;
		sprite0_hit_delay = 0;
		ppustatus_vblank = false;
		set_nmi(false);
	}

	// === BG shift clocking happens regardless of rendering (composite) ===
	if ((bg_pipeline_enabled || spr_pipeline_enabled) && ((dot >= 2 && dot <= 257) || (dot >= 322 && dot <= 337))) {
		clock_bg_shifters_only();
	}

	run_render_pipeline_dot();
	if (bg_pipeline_enabled || spr_pipeline_enabled) {
		switch (dot) {
			case 65 ... 256:
				do_prerender_oam_sweep();
				break;
		}

		if (dot >= 280 && dot <= 304) {
			copy_vert();
		}
	}
}

void ppu2c0x_device::do_prerender_oam_sweep() {
	// /VIS is deasserted for the whole pre-render scanline (Fiskbit).
	// So: OAM2 selected, automatic writes become reads, and OAM1 is not touched.

	if (dot == 65) {
		// Don't touch oamaddr / oam_eval_addr at all.
		// Just start a harmless OAM2 read sweep
		oam2addr = 0;
	}

	// Read OAM2 every dot in this phase.
	oamdata_read_buffer = secondary_oam[oam2addr & 0x1F];
	oamdata_latch = oamdata_read_buffer;
	oam_latch_addr = oam2addr;

	// Optionally advance OAM2ADDR occasionally to "sweep" and refresh for decay behavior.
	// The exact increment cadence is analog-weird and probably not test-critical,
	// but advancing every other dot is a reasonable approximation.
	if (!(dot & 1)) {
		oam2addr = (oam2addr + 1) & 0x1F;
	}
}

// Called for dots on line 241
void ppu2c0x_device::run_scanline_241_dot() {
	if (!suppress_vblank_flag) {
		if (dot == 1 && !ppustatus_vblank) {
			ppustatus_vblank = true;

			if (ppuctrl_nmi_enable)
				set_nmi(true);
		}
	}
}

void ppu2c0x_device::do_2007_post_access_bump(ppu_bus_source source) {
	if ((bg_pipeline_enabled || spr_pipeline_enabled) && is_visible_scanline()) {
		// Accessing $2007 during rendering performs this glitch. Used by Young
		// Indiana Jones Chronicles to shake the screen.
		bump_horiz();
		bump_vert();
	} else {
		// The $2007 access increments v after the access.
		// MMC3 A12 tests expect this increment to be visible when it changes A12,
		// e.g. $0FFF -> $1000 after a $2007 read/write.
		const uint16_t old_v = v & 0x3FFF;

		v = (v + ppuctrl_vram_increment) & 0x7FFF;

		const uint16_t new_v = v & 0x3FFF;

		// Palette space is internal to the PPU. This is the Steins fix:
		// do not let $3Fxx palette-space bumps create MMC3 clocks.
		if ((old_v & 0x3F00) == 0x3F00 || (new_v & 0x3F00) == 0x3F00) {
			ppu_address_bus = new_v;
			ppu_ad_latch = new_v & 0xFF;
		} else {
			ppu_bus_address_drive(new_v, source);
		}
	}
}

void ppu2c0x_device::write_oam_dma_byte(uint8_t val) {
	primary_oam[oamaddr] = val;
	oamaddr = (oamaddr + 1) & 0xFF;
}

void ppu2c0x_device::write_oam_data_reg(uint8_t val) {
	// $2004 writes during rendering:
	//   - visible scanlines 0-239
	//   - pre-render scanline 261
	//   - only if BG or sprites are enabled
	//
	// They do not write to primary OAM, but they perform the glitchy
	// OAMADDR increment. For this test/model, bump the sprite index by one:
	// +4, then force byte index to 0 with & $FC.
	const bool rendering_on = (bg_pipeline_enabled || spr_pipeline_enabled);
	const bool render_line = is_render_scanline();

	if (render_line && rendering_on) {
		oamdata_latch = val;
		oamaddr = (uint8_t)(oamaddr + 4);
		oamaddr &= 0xFC;
		return;
	}

	primary_oam[oamaddr] = val;
	oamaddr = (uint8_t)(oamaddr + 1);
}

//The coarse X component of v needs to be incremented when the next tile is reached. Bits 0-4 are incremented, with overflow toggling bit 10.
//This means that bits 0-4 count from 0 to 31 across a single nametable, and bit 10 selects the current nametable horizontally.
void ppu2c0x_device::bump_horiz() {
	// Coarse x equal to 31?
	if ((v & 0x1F) == 0x1F) {
		// Set coarse x to 0 and switch horizontal nametable. The bit twiddling
		// to clear the lower five bits relies on them being 1.
		v ^= 0x041F;
	} else {
		++v;
	}
	v &= 0x7FFF;
}

//If rendering is enabled, fine Y is incremented at dot 256 of each scanline, overflowing to coarse Y, and finally adjusted to wrap among the nametables vertically
//Bits 12-14 are fine Y. Bits 5-9 are coarse Y. Bit 11 selects the vertical nametable.
//If rendering is enabled, fine Y is incremented at dot 256 of each scanline, overflowing to coarse Y, and finally adjusted to wrap among the nametables vertically
//Bits 12-14 are fine Y. Bits 5-9 are coarse Y. Bit 11 selects the vertical nametable.
void ppu2c0x_device::bump_vert() {
	// Fine y equal to 7?
	if ((v & 0x7000) == 0x7000) {
		// Check coarse y
		switch (v & 0x03E0) {
				// Coarse y equal to 29. Switch vertical nametable (XOR by 0x0800) and
				// clear fine y and coarse y in the same operation (possible since we
				// know their value).
			case 29 << 5:
				v ^= 0x7800 | (29 << 5);
				break;

				// Coarse y equal to 31. Clear fine y and coarse y without switching
				// vertical nametable (this occurs for vertical scroll values > 240).
			case 31 << 5:
				v &= ~0x73E0;
				break;

				// Clear fine y and increment coarse y
			default:
				v = (v & ~0x7000) + 0x0020;
				break;
		}
	} else {
		// Bump fine y
		v += 0x1000;
	}

	v &= 0x7FFF;
}

// Restores the horizontal bits in v from t at the end of each scanline during
// rendering
void ppu2c0x_device::copy_horiz() {
	// v: ... .H.. ...E DCBA = t: ... .H.. ...E DCBA
	v = (v & ~0x041F) | (t & 0x041F);
	v &= 0x7FFF;
}

// Initializes the vertical bits in v from t on the pre-render line
void ppu2c0x_device::copy_vert() {
	// v: IHG F.ED CBA. .... = t: IHG F.ED CBA. ....
	v = (v & ~0x7BE0) | (t & 0x7BE0);
	v &= 0x7FFF;
}

/*************************************
*
*   PPU Memory functions
*
*************************************/

void ppu2c0x_device::palette_write(offs_t offset, uint8_t val) {
	// 1. Address Masking: Mask the address to the 32-entry palette RAM range ($3F00-$3F1F).
	offset &= 0x1F;

	// 2. Value Masking: Mask the value to the available 6 bits (64 colors total).
	val &= 0x3F;

	// 3. Perform the primary write to the target address.
	m_palette_ram[offset] = val;

	// 4. Handle Backdrop Mirroring:
	// Check if the offset is a backdrop address (multiples of 4: xF00, xF04, xF08, xF0C, xF10, xF14, xF18, xF1C).
	// These addresses are electrically linked.
	if (!(offset & 0x3))
		// Mirror the written value to its linked address (XOR 0x10 toggles between
		// the Background Palette 0x0X and the Sprite Palette 0x1X spaces).
		// This ensures $3F00 and $3F10 (and their mirrors) always contain the same universal color.
		m_palette_ram[offset ^ 0x10] = val;
}

uint8_t ppu2c0x_device::palette_read(offs_t offset) {
	// 1. Address Masking: Mask the address to the 32-entry palette RAM range ($3F00-$3F1F).
	offset &= 0x1f;

	// 2. Sprite Backdrop Mirroring:
	// If the address is a Sprite Backdrop entry (0x10, 0x14, 0x18, 0x1C),
	// the hardware forces a read from the corresponding Background Palette index (0x00, 0x04, etc.).
	// Clearing bit 4 (0x10) performs this mapping.
	// Note: This logic relies on the palette_write function to ensure all backdrop entries are linked.
	if (offset == 0x10 || offset == 0x14 || offset == 0x18 || offset == 0x1C)
		offset &= ~0x10;

	// 3. Perform the read and apply grayscale if enabled (ppumask_grayscale_mask is set by $2001 bit 0).
	return m_palette_ram[offset] & 0x3F;
}

/*************************************
	*
	*  PPU Registers Read
	*
	*************************************/

uint8_t ppu2c0x_device::read(offs_t offset) {
	switch (offset & 7) {
		case 0:
		case 1:
		case 3:
		case 5:
		case 6:
			return ppu_open_bus_peek();

		case PPU_STATUS: /* 2 */ {
			// Read-start sample for vblank
			bool vblank_read = ppustatus_vblank;
			bool spr0_read = ppustatus_sprite0_hit;
			bool ovf_read = ppustatus_sprite_overflow;

			// Scheduler convention: dot is the next PPU dot to execute.
			// dot == 1 here means hardware dot 0 has completed, but dot 1 has not run yet.
			// This models the $2002 read-on-241,0 case that suppresses the upcoming vblank set.
			if (is_vblank_start_scanline() && dot == 1) {
				vblank_read = false;
				suppress_vblank_flag = true;
			}
			// Readback quirk: at prerender clear edge, sprite flags read as cleared here
			if (is_prerender_scanline() && dot == 1) {
				spr0_read = false;
				ovf_read = false;
			}

			w = false;

			const uint8_t old_bus = ppu_open_bus_peek();

			const uint8_t ret = m_security_value ? uint8_t((vblank_read ? 0x80 : 0x00) | (spr0_read ? 0x40 : 0x00) | m_security_value) :
												   uint8_t((vblank_read ? 0x80 : 0x00) | (spr0_read ? 0x40 : 0x00) | (ovf_read ? 0x20 : 0x00) | (old_bus & 0x1f));

			if (BIT(ret, 7) && !m_mmc5_reset_scanline_irq.isnull()) {
				m_mmc5_reset_scanline_irq();
			}

			ppustatus_vblank = false;
			set_nmi(false);

			// $2002 read drives only status bits 7-5.
			// Low bits 4-0 are open bus and must keep their existing decay timers.
			if (m_security_value)
				ppu_open_bus_drive(ret);
			else
				ppu_open_bus_drive_masked(ret, 0xe0);

			return ret;
		}

		case PPU_SPRITE_DATA: /* $2004 */ {
			const bool render_line = is_render_scanline();
			const bool rendering_enabled = (bg_pipeline_enabled || spr_pipeline_enabled);

			uint8_t ret = 0;

			if (render_line && rendering_enabled) {
				if (dot == 1) {
					//Reading from $2004 (with rendering enabled) on dot 0 should return Secondary_OAM Index 0.
					ret = secondary_oam[0];
				} else if (dot >= 2 && dot <= 65) {
					//Reading from $2004 (with rendering enabled) from dots 1 through dots 64 should return #$FF.
					ret = oamdata_latch;
				} else if (dot >= 66 && dot <= 257) {
					//Reading from $2004 (with rendering enabled) from dots 65 through 256 should read from the "OAM Latch" used during OAM Evaluation.
					ret = oamdata_latch;
					if ((oam_latch_addr & 3) == 2)
						ret &= 0xE3;
				} else if (dot >= 258 && dot <= 321) {
					//Reading from $2004 (with rendering enabled) from dots 257 through 320 should read from secondary OAM.
					ret = oamdata_latch;
				} else {
					// During dots 321 through 340, $2004 reads secondary OAM at
					// the current OAM2 address. Normally that address is zero,
					// but interrupted sprite fetch can leave it misaligned.
					ret = secondary_oam[oam2addr & 0x1F];
				}
			} else {
				// Non-rendering: CPU reads primary OAM at OAMADDR
				ret = primary_oam[oamaddr];
				if ((oamaddr & 3) == 2)
					ret &= 0xE3;
			}

			// $2004 read drives the PPU internal data bus.
			ppu_open_bus_drive(ret);

			return ppu_open_bus_peek();
		}

		case PPU_DATA: /* $2007 */ {
			const uint16_t bus_addr = v & 0x3FFF;

			// $2007 read is a real PPU address-bus event.
			// mmc3_test_2/3-A12_clocking expects this to clock when A12 rises.
			if ((bus_addr & 0x3F00) == 0x3F00) {
				ppu_address_bus = bus_addr;
			} else {
				ppu_bus_address_drive(bus_addr, ppu_bus_source::CPU_ACCESS);
			}

			uint8_t ret;

			if (bus_addr >= 0x3F00 && bus_addr <= 0x3FFF) {
				const uint8_t old_bus = ppu_open_bus_peek();

				ret = (old_bus & 0xC0) | (m_palette_ram[bus_addr & 0x1F] & ppumask_grayscale_mask);

				// Palette reads drive only bits 5-0.
				// Bits 7-6 remain open bus.
				ppu_open_bus_drive_masked(ret, 0x3F);
			} else {
				// Normal reads return the existing buffered value.
				ret = ppudata_read_buffer;
				ppu_open_bus_drive(ret);
			}

			const bool rendering_for_access = (bg_pipeline_enabled || spr_pipeline_enabled) && is_visible_scanline();

			if (!rendering_for_access) {
				// Non-rendering:
				// The CPU access increments v now.
				do_2007_post_access_bump(ppu_bus_source::CPU_ACCESS);

				// The internal read buffer refills later from the accessed address.
				schedule_2007_read(bus_addr, 5, false);
			} else {
				// Rendering:
				// Do not perform a hidden memory read here.
				// This is only for CPU $2007 reads during rendering; normal rendering fetches
				// do not update ppudata_read_buffer unless a pending $2007 refill is armed.
				// The rendering H+V increment glitch is scheduled when the delayed read matures.
				schedule_2007_read(bus_addr, 4, true);
			}

			return ppu_open_bus_peek();
		}

		default:
			logerror("invalid ppu read at $%04X\n", offset);
			break;
	}
	return ppu_open_bus_peek();
}

void ppu2c0x_device::apply_delayed_2000(uint8_t val) {
	// Only apply the PPUCTRL bits that affect delayed fetch/render pipeline state.

	// Bit 3: sprite pattern table select for 8x8 sprites.
	ppuctrl_sprite_pattern_base = (val & 0x08) ? 0x1000 : 0x0000;
	// Bit 4: background pattern table select.
	ppuctrl_bg_pattern_base = (val & 0x10) ? 0x1000 : 0x0000;
	// Bit 5: sprite size.
	ppuctrl_sprite_size = (val & 0x20) ? EIGHT_BY_SIXTEEN : EIGHT_BY_EIGHT;
}

void ppu2c0x_device::apply_delayed_2001(uint8_t val) {
	// Fetch pipeline enable.
	const bool prev_pipe = bg_pipeline_enabled || spr_pipeline_enabled;
	const bool new_bg_on = (val & 0x08) != 0;
	const bool new_spr_on = (val & 0x10) != 0;

	bg_pipeline_enabled = new_bg_on;
	spr_pipeline_enabled = new_spr_on;

	// Output ON is delayed to the same edge as the pipe.
	// Output OFF already happened immediately in the $2001 write handler.
	if (new_bg_on)
		bg_output_enabled = true;

	if (new_spr_on)
		spr_output_enabled = true;

	// Clip vars follow output domain.
	bg_left_clip = !bg_output_enabled ? 256 : ppumask_show_bg_left ? 0 : 8;
	spr_left_clip = !spr_output_enabled ? 256 : ppumask_show_spr_left ? 0 :8;

	const bool new_pipe = bg_pipeline_enabled || spr_pipeline_enabled;

	// --------------------------------------------------
	// BGSerialIn behavior
	// --------------------------------------------------
	//
	// If rendering is enabled mid-scanline, the next BG reload slot must be
	// skipped once, so the shifters keep shifting and the serial input becomes
	// visible.
	if (!prev_pipe && new_pipe) {
		const bool render_line = is_render_scanline();
		const bool in_fetch_region = ((dot >= 2 && dot <= 257) || (dot >= 322 && dot <= 337));

		if (render_line && in_fetch_region)
			skip_bg_reload_once = true;
	}

	// --------------------------------------------------
	// OAM row corruption seed
	// --------------------------------------------------
	if (prev_pipe && !new_pipe) {
		skip_bg_reload_once = false;

		// OAM2ADDR is preserved when rendering is disabled. This selects
		// the stale source lane when rendering later resumes.
		corrupt_resume_oam2addr = oam2addr & 0x1F;

		const bool render_line = is_render_scanline();
		const bool early_window = dot >= 1 && dot <= 64;
		const bool late_window = dot >= 257 && dot <= 320;

		if (render_line && (early_window || late_window)) {
			uint8_t seed = 0;

			if (early_window) {
				// During OAM2 clear, the secondary-OAM address advances every
				// other dot. Row corruption follows the active OAM2 row.
				seed = uint8_t((dot >> 1) & 0x1F);
			} else {
				// Dots 257-320 are sprite-load time.
				//
				// The first four phases expose bytes 0, 1, 2, and 3.
				// The address then remains effectively on byte 3 through
				// the remainder of the eight-dot slot.
				const unsigned load_dot = unsigned(dot - 257);
				const unsigned slot = load_dot >> 3;
				const unsigned phase = load_dot & 0x07;
				const unsigned byte = phase <= 3 ? phase : 3;

				seed = uint8_t(((slot << 2) + byte) & 0x1F);
			}

			oam_corrupt_seed = seed;
			oam_corrupt_pending = true;
		}
	}
}

/*************************************
*
*  PPU Registers Write
*
*************************************/
void ppu2c0x_device::write(offs_t offset, uint8_t val) {
	ppu_open_bus_drive(val);

	/* on the RC2C05, PPU_CONTROL0 and PPU_CONTROL1 are swapped (protection) */
	if (m_security_value && !(offset & 6))
		offset ^= 1;

	switch (offset & 7) {
		case PPU_CONTROL0: /* 0 */ {
			t = (t & 0x73FF) | ((val & 0x03) << 10);
			ppuctrl_vram_increment = (val & 0x04) ? 32 : 1;

			bool new_nmi_on_vblank = val & 0x80;

			// In this scheduler, $2000 writes are handled before the per-dot PPU work,
			// so a write on the exact pre-render clear tick can still see ppustatus_vblank = true
			// even though hardware would already be past the enable window.
			bool vblank_seen_for_enable = ppustatus_vblank;
			if (is_prerender_scanline() && dot == 1) { //needed for nmi "on" timing
				vblank_seen_for_enable = false;
			}

			// Enabling NMI while VBL is already set retriggers NMI, except on the clear-edge tick.
			if (new_nmi_on_vblank && !ppuctrl_nmi_enable && vblank_seen_for_enable) {
				nmi_pending = false;
				nmi_delay = 0;
				m_maincpu6502->queue_delayed_nmi(2);
			}

			// Disabling NMI near the VBL-set edge suppresses the pending NMI for one more
			// scheduler tick than the raw hardware dot because writes are processed before
			// the PPU edge in this core.
			if (!new_nmi_on_vblank && ppuctrl_nmi_enable && is_vblank_start_scanline() && dot >= 1 && dot <= 3) {
				set_nmi(false);
			}

			ppuctrl_nmi_enable = new_nmi_on_vblank;
			
			m_regs[PPU_CONTROL0] = val;
			pending_2000.has_pending = true;
			pending_2000.value = val;
			pending_2000.apply_ppu = 3;
			break;
		}

		case PPU_CONTROL1: {
			uint8_t old_ppumask = m_regs[PPU_CONTROL1];

			const bool old_render = (old_ppumask & 0x18) != 0;
			const bool new_render = (val & 0x18) != 0;

			if (!old_render && new_render) {
				corrupt_resume_fifo_pending = false;
				corrupt_resume_high_lane = 0xFF;

				if (is_render_scanline() && dot >= 257 && dot <= 320) {
					corrupt_resume_fifo_pending = true;
				}
			}

			bg_output_enabled = (val & 0x08) != 0;
			spr_output_enabled = (val & 0x10) != 0;

			// Delay pipeline/render-domain change by 3 PPU cycles.
			pending_2001.has_pending = true;
			pending_2001.value = val;
			pending_2001.apply_ppu = 3;

			// Immediate visual-only properties.
			m_regs[PPU_CONTROL1] = val;

			ppumask_show_bg_left = (val & 0x02) != 0;
			ppumask_show_spr_left = (val & 0x04) != 0;

			ppumask_grayscale_mask = (val & PPU_CONTROL1_DISPLAY_MONO) ? 0x30 : 0x3F;
			ppumask_emphasis_bits = uint16_t(val & PPU_CONTROL1_COLOR_EMPHASIS) << 1;

			// Clip vars follow output domain.
			bg_left_clip = !bg_output_enabled ? 256 : ppumask_show_bg_left ? 0 :
																			 8;
			spr_left_clip = !spr_output_enabled ? 256 : ppumask_show_spr_left ? 0 :
																				8;

			// Retroactive color/render correction.
			if (scanline <= 239 && dot >= 2 && dot <= 257) {
				uint8_t diff = old_ppumask ^ val;
				const int late_delta = dot - prev_pixel_x;
				const bool late_prev_pixel = prev_pixel_valid && prev_pixel_scanline == scanline && prev_pixel_x >= 0 && prev_pixel_x < 256 && late_delta == 3;

				if ((diff & (0x01 | 0xE0)) && late_prev_pixel) {
					retro_ppumask_color = true;
				}

				if ((diff & 0x18) && frame_start_ppu_phase == 3 && late_prev_pixel) {
					retro_ppumask_render = true;
				}
			}

			break;
		}

		case 2:
			break;

		case PPU_SPRITE_ADDRESS: /* 3 */ {
			m_regs[PPU_SPRITE_ADDRESS] = val;
			oamaddr = val;

			const bool render_line = is_render_scanline(); //(scanline < 240) || (scanline == 261);
			const bool rendering_enabled = (bg_pipeline_enabled || spr_pipeline_enabled);

			if (render_line && rendering_enabled && sprite_eval_initialized && dot >= 65 && dot <= 256) {
				oam_eval_addr = val;

				spr_pt_addr_h = (oam_eval_addr >> 2) & 0x3F;
				spr_pt_addr_l = oam_eval_addr & 0x03;

				sprite_eval_in_range = false;
				overflow_bug_counter = 0;

				// Do not force this true. $01/$11/$83/$93 must begin misaligned.
				m_oam_eval_realigned = (spr_pt_addr_l == 0);
			}

			break;
		}

		case PPU_SPRITE_DATA:
			write_oam_data_reg(val);
			break;

		case PPU_SCROLL: /* 5 */ {
			if (!w) {
				// First write
				// fine_x = val: .... .ABC
				// t: ... .... ...D EFGH = val: DEFG H...
				fine_x = val & 7;
				t = (t & 0x7FE0) | ((val & 0xF8) >> 3);
			} else {
				// Second write
				// t: ABC ..DE FGH. .... = val: DEFG HABC
				t = (t & 0x0C1F) | ((val & 0xF8) << 2) | ((val & 7) << 12);
			}
			w = !w;
			break;
		}

		case PPU_ADDRESS: /* $2006 */ {
			if (!w) {
				t = (t & 0x00FF) | ((val & 0x3F) << 8);
			} else {
				t = (t & 0x7F00) | val;
				pending_2006.has_pending = true;
				pending_2006.value16 = t & 0x7FFF;
				pending_2006.apply_ppu = 4;
			}

			w = !w;
			break;
		}

		case PPU_DATA: /* 7 */ {
			uint16_t bus_addr = v & 0x3FFF;

			// Bus sees the access address now.
			// Palette space is internal; don't clock cartridge/MMC3 A12 from it.
			if ((bus_addr & 0x3F00) == 0x3F00) {
				ppu_address_bus = bus_addr & 0x3FFF;
				//ppu_ad_latch = bus_addr & 0xFF;
			} else {
				ppu_bus_address_drive(bus_addr, ppu_bus_source::CPU_ACCESS);
			}

			const bool rendering_for_access = (bg_pipeline_enabled || spr_pipeline_enabled) && is_visible_scanline();

			if (!rendering_for_access) {
				// Non-rendering:
				// $2007 write takes effect at the current address now.
				writebyte(bus_addr & 0x3FFF, val);

				// The $2007 post-access increment is NOT immediate.
				// Hardware leaves the current palette/address visible briefly after
				// the write becomes effective. This lets a palette write show the newly
				// written color for 1 dot before v advances.
				ppu2007_post_bump_pending = true;
				ppu2007_post_bump_delay = 1;

				// Keep the bus shadow at the access address until the delayed bump runs.
				// Palette space is internal; don't clock cartridge/MMC3 A12 from it.
				if ((bus_addr & 0x3F00) == 0x3F00) {
					ppu_address_bus = bus_addr & 0x3FFF;
					//ppu_ad_latch = bus_addr & 0xFF;
				} else {
					ppu_bus_address_drive(bus_addr, ppu_bus_source::CPU_ACCESS);
				}
			} else {
				// Rendering:
				// The write itself is delayed. Do NOT bump v here.
				// The bump is armed when the delayed write actually reaches the PPU bus,
				// so the increment is delayed relative to the effective write, not merely
				// relative to the CPU register write.
				schedule_2007_write(bus_addr & 0x3FFF, val, 5);

				// Keep the bus shadow at the access address for now.
				// Palette space is internal; don't clock cartridge/MMC3 A12 from it.
				if ((bus_addr & 0x3F00) == 0x3F00) {
					ppu_address_bus = bus_addr & 0x3FFF;
					//ppu_ad_latch = bus_addr & 0xFF;
				} else {
					ppu_bus_address_drive(bus_addr, ppu_bus_source::CPU_ACCESS);
				}
			}
			break;
		}

		default:
			logerror("invalid ppu write: $%02X at $%04X\n", val, offset);
			/* ignore other registers writes */
			break;
	}
}

uint64_t ppu2c0x_device::ppudecay_now() const {
	// Use whatever your CPU-cycle source is here.
	// In your tree this may be m_maincpu6502->total_cycles().
	return m_maincpu6502 ? m_maincpu6502->total_cycles() : 0;
}

uint64_t ppu2c0x_device::ppudecay_count_to_cpu_cycles(uint16_t count) const {
	// The inner loop is roughly 15 CPU cycles per successful compare loop.
	// The +8 is the small setup/read-position fudge needed because the first
	// CMP does not happen immediately after STA $2002.
	return uint64_t(count) * 15 + 8;
}

void ppu2c0x_device::ppu_open_bus_drive(uint8_t data) {
	const uint64_t now = ppudecay_now();

	m_ppu_io_db = data;

	for (int bit = 0; bit < 8; bit++) {
		const uint8_t mask = 1U << bit;

		if (data & mask)
			m_ppu_io_db_decay_at[bit] = now + ppudecay_count_to_cpu_cycles(ppudecay_count_for_bit[bit]);
		else
			m_ppu_io_db_decay_at[bit] = 0;
	}
}

uint8_t ppu2c0x_device::ppu_open_bus_peek() {
	const uint64_t now = ppudecay_now();

	for (int bit = 0; bit < 8; bit++) {
		const uint8_t mask = 1U << bit;

		if ((m_ppu_io_db & mask) && m_ppu_io_db_decay_at[bit] && now >= m_ppu_io_db_decay_at[bit])
			m_ppu_io_db &= ~mask;
	}

	return m_ppu_io_db;
}

void ppu2c0x_device::ppu_open_bus_drive_masked(uint8_t data, uint8_t mask) {
	const uint64_t now = ppudecay_now();

	// Only masked bits are newly driven/refreshed.
	m_ppu_io_db = (m_ppu_io_db & ~mask) | (data & mask);

	for (int bit = 0; bit < 8; bit++) {
		const uint8_t b = 1U << bit;

		if (!(mask & b))
			continue;

		if (data & b)
			m_ppu_io_db_decay_at[bit] = now + ppudecay_count_to_cpu_cycles(ppudecay_count_for_bit[bit]);
		else
			m_ppu_io_db_decay_at[bit] = 0;
	}
}

uint16_t ppu2c0x_device::get_vram_dest() {
	return v;
}

void ppu2c0x_device::set_vram_dest(uint16_t dest) {
	v = dest;
}

/*************************************
*
*  PPU Rendering
*
*************************************/

void ppu2c0x_device::render(bitmap_rgb32& bitmap, int flipx, int flipy, int sx, int sy, const rectangle& cliprect) {
	copybitmap(bitmap, m_bitmap, flipx, flipy, sx, sy, cliprect);
}

uint32_t ppu2c0x_device::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect) {
	render(bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void ppu2c0x_device::read_tile_plane_data(int address, int color) {}
void ppu2c0x_device::shift_tile_plane_data(uint8_t& pix) {}
void ppu2c0x_device::draw_tile_pixel(uint8_t pix, int color, uint32_t back_pen, uint32_t*& dest) {}
void ppu2c0x_device::draw_tile(uint8_t* line_priority, int color_byte, int color_bits, int address, int start_x, uint32_t back_pen, uint32_t*& dest) {}
void ppu2c0x_device::draw_background(uint8_t* line_priority) {}
void ppu2c04_clone_device::draw_background(uint8_t* line_priority) {}
void ppu2c0x_device::draw_back_pen(uint32_t* dest, int back_pen) {}
void ppu2c0x_device::draw_background_pen() {}
void ppu2c0x_device::read_sprite_plane_data(int address) {}
void ppu2c0x_device::make_sprite_pixel_data(uint8_t& pixel_data, bool flipx) {}

void ppu2c0x_device::write_to_spriteram_with_increment(uint8_t data)
{
	write_oam_data_reg(data);
}

void ppu2c0x_device::reload_refresh_data()
{
	m_refresh_data = m_refresh_latch;
}
void ppu2c0x_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32& bitmap) {}
void ppu2c04_clone_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32& bitmap) {}
void ppu2c0x_device::read_extra_sprite_bits(int eval_sprite_tile) {}
bool ppu2c0x_device::is_spritepixel_opaque(int pixel_data, int color) {
	return false;
}
void ppu2c0x_device::draw_sprite_pixel_low(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int eval_sprite_tile, uint8_t* line_priority) {}
void ppu2c0x_device::draw_sprite_pixel_high(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int eval_sprite_tile, uint8_t* line_priority) {}
int ppu2c0x_device::apply_sprite_pattern_page(int index1, int size) {
	return 0;
}
void ppu2c0x_device::draw_sprites(uint8_t* line_priority) {}
void ppu2c04_clone_device::draw_sprites(uint8_t* line_priority) {}
void ppu2c0x_device::render_scanline() {}
void ppu2c0x_device::scanline_increment_fine_ycounter() {}
void ppu2c0x_device::update_visible_enabled_scanline() {}
void ppu2c0x_device::update_visible_disabled_scanline() {}
void ppu2c0x_device::update_visible_scanline() {}
void ppu2c0x_device::update_scanline() {}
void ppu2c04_clone_device::write(offs_t offset, uint8_t data) {}
void ppu2c04_clone_device::device_start() {}
void ppu2c04_clone_device::init_palette_tables() {}	
uint8_t ppu2c04_clone_device::read(offs_t offset) {
	return 0;
}
