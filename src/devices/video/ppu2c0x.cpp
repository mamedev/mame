// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Brad Oliver, Fabio Priuli, Brian H. Gilbert
/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

    2009-04: Changed NES PPU to be a device (Nathan Woods)
    2009-07: Changed NES PPU to use a device memory map (Robert Bohms)

    What is not known, and how it is handled

    * Every PPU draws dot by dot. The VT03 family, the SH6578 and the 2C04
      clone do so through hooks (their own pattern buses and planes, name
      table formats, palettes, transparency, VRAM address); nothing documents
      their fetch timing, so they fetch in the 2C02's slots, extra pattern
      planes in the same access as the first two.

    * The VT3xx's extended video ($201E non-zero: packed 4/8bpp and 16x16
      tiles, the high-resolution sprite engine) is drawn by its own engine
      for a whole line at the end of that line; its fetch timing is not
      documented (NESdev's VT369 pages give its registers only).

    * The NTSC VT03 family (VT03, VT32, VT3xx) keeps its own dot timing: VBL
      set and cleared on dot 1, /NMI as a signal that its 2A03 core samples.
      The PAL VT variants, the SH6578 and the 2C04 clone follow the screen
      and drive /NMI from a timer 4 CPU cycles after the VBL flag: the PAL VT
      dot grid is not known, and the SH6578 and the 2C04 clone have neither a
      2A03 nor documented timing (given the 2C02's own timing, the SH6578
      multicarts show a grey screen and the clone's frame changes).

    * The 2C04 clone's sprites are shown one frame late from a second OAM
      buffer; when the clone reads their CHR data is not known, so they take
      the CHR of the frame they are shown in (the Vs. SMB bootlegs that use
      it have no CHR bank switching).

******************************************************************************/

#include "emu.h"
#include "ppu2c0x.h"

#include "cpu/m6502/rp2a03.h"

#include "screen.h"

#include "corefloat.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(PPU_2C02,    ppu2c02_device,       "ppu2c02",    "2C02 PPU")
DEFINE_DEVICE_TYPE(PPU_2C03B,   ppu2c03b_device,      "ppu2c03b",   "2C03B PPC")
DEFINE_DEVICE_TYPE(PPU_2C04,    ppu2c04_device,       "ppu2c04",    "2C04 PPU")
DEFINE_DEVICE_TYPE(PPU_2C07,    ppu2c07_device,       "ppu2c07",    "2C07 PPU")
DEFINE_DEVICE_TYPE(PPU_PALC,    ppupalc_device,       "ppupalc",    "Generic PAL Clone PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_01, ppu2c05_01_device,    "ppu2c05_01", "2C05_01 PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_02, ppu2c05_02_device,    "ppu2c05_02", "2C05_02 PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_03, ppu2c05_03_device,    "ppu2c05_03", "2C05_03 PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_04, ppu2c05_04_device,    "ppu2c05_04", "2C05_04 PPU")
DEFINE_DEVICE_TYPE(PPU_2C04C,   ppu2c04_clone_device, "ppu2c04c",   "2C04 Clone PPU")


// default address map
void ppu2c0x_device::ppu2c0x(address_map& map)
{
	if (!has_configured_map(0))
	{
		map(0x0000, 0x3eff).ram();
		map(0x3f00, 0x3fff).rw(FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
		//map(0x0000, 0x3fff).ram();
	}
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ppu2c0x_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  ppu2c0x_device - constructor
//-------------------------------------------------

ppu2c0x_device::ppu2c0x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 17, 0, internal_map),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_scanline(0),  // reset the scanline count
	m_videoram_addr_mask(0x3fff),
	m_global_refresh_mask(0x7fff),
	m_line_write_increment_large(32),
	m_paletteram_in_ppuspace(false),
	m_tile_page(0),
	m_back_color(0),
	m_refresh_data(0),
	m_x_fine(0),
	m_toggle(false),
	m_render_enabled_hblank(false),
	m_tilecount(0),
	m_latch(*this),
	m_read_latch(*this),
	m_ale_cb(*this),
	m_rd_cb(*this),
	m_wr_cb(*this),
	m_spriteramsize(0x100),
	m_scanline_callback_proc(*this),
	m_hblank_callback_proc(*this),
	m_a12_callback_proc(*this)
	, m_a12_level_proc(*this)
	, m_addr_bus_proc(*this)
	, m_bus_a13a12(0)
	, m_a12_filter_proc(*this),
	m_int_callback(*this),
	m_ext_in_cb(*this, 0),
	m_ext_out_cb(*this),
	m_refresh_latch(0),
	m_add(1),
	m_videomem_addr(0),
	m_data_latch(0),
	m_buffered_data(0),
	m_sprite_page(0),
	m_scan_scale(1), // set the scan scale (this is for dual monitor vertical setups)
	m_draw_phase(0)
{
	for (auto &elem : m_regs)
		elem = 0;

	m_scanlines_per_frame = NTSC_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE;

	/* usually, no security value... */
	m_security_value = 0;
}

ppu2c0x_device::ppu2c0x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(ppu2c0x_device::ppu2c0x), this))
{
	m_paletteram_in_ppuspace = true;
}

ppu2c0x_rgb_device::ppu2c0x_rgb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock),
	m_palette_data(*this, "palette")
{
}

// NTSC NES
ppu2c02_device::ppu2c02_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_device(mconfig, PPU_2C02, tag, owner, clock)
{
}

// Playchoice 10
ppu2c03b_device::ppu2c03b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C03B, tag, owner, clock)
{
}

// Vs. Unisystem
ppu2c04_device::ppu2c04_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C04, tag, owner, clock)
{
}

// PAL NES
ppu2c07_device::ppu2c07_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_device(mconfig, PPU_2C07, tag, owner, clock)
{
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
}

// PAL clones
ppupalc_device::ppupalc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_device(mconfig, PPU_PALC, tag, owner, clock)
{
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_PALC;
}

// The PPU_2C05 variants have different protection value, set at device start, but otherwise are all the same...
// Vs. Unisystem (Ninja Jajamaru Kun)
ppu2c05_01_device::ppu2c05_01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_01, tag, owner, clock)
{
	m_security_value = 0x1b;    // game (jajamaru) doesn't seem to ever actually check it
}
// Vs. Unisystem (Mighty Bomb Jack)
ppu2c05_02_device::ppu2c05_02_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_02, tag, owner, clock)
{
	m_security_value = 0x3d;
}
// Vs. Unisystem (Gumshoe)
ppu2c05_03_device::ppu2c05_03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_03, tag, owner, clock)
{
	m_security_value = 0x1c;
}
// Vs. Unisystem (Top Gun)
ppu2c05_04_device::ppu2c05_04_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_04, tag, owner, clock)
{
	m_security_value = 0x1b;
}

// Vs. Unisystem (Super Mario Bros. bootlegs)
ppu2c04_clone_device::ppu2c04_clone_device(const machine_config &mconfig, const char *tag, device_t* owner, u32 clock) :
	ppu2c0x_device(mconfig, PPU_2C04C, tag, owner, clock),
	m_palette_data(*this, "palette")
{
	m_scanlines_per_frame = VS_CLONE_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_VS_CLONE;

	// background and sprites are always enabled; monochrome and color emphasis aren't supported
	m_regs[PPU_CONTROL1] = u8(~(PPU_CONTROL1_COLOR_EMPHASIS | PPU_CONTROL1_DISPLAY_MONO));
	m_out_mask = m_regs[PPU_CONTROL1];
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ppu2c0x_device::start_nopalram()
{
	// allocate timers
	m_hblank_timer = timer_alloc(FUNC(ppu2c0x_device::hblank_tick), this);
	m_hsync_timer = timer_alloc(FUNC(ppu2c0x_device::hsync_tick), this);
	m_nmi_timer = timer_alloc(FUNC(ppu2c0x_device::nmi_tick), this);
	m_pipe_timer = timer_alloc(FUNC(ppu2c0x_device::pipe_tick), this);
	m_scanline_timer = timer_alloc(FUNC(ppu2c0x_device::scanline_tick), this);
	m_vbl_timer = timer_alloc(FUNC(ppu2c0x_device::vbl_tick), this);
	m_a12_timer = timer_alloc(FUNC(ppu2c0x_device::a12_tick), this);
	m_a12_catchup_timer = timer_alloc(FUNC(ppu2c0x_device::a12_catchup_tick), this);
	m_vbl_suppress = false;
	m_vbl_set_pending = false;
	m_nmi_line = false;
	m_nmi_vbl = false;
	m_vbl_set_at = attotime::never;
	m_vbl_clr_at = attotime::never;
	m_nmi_en_at = attotime::zero;
	m_vbl_nmi_end_at = attotime::never;
	m_nmi_update_last = attotime::zero;

	/* initialize the scanline handling portion */
	// own timing: the PPU keeps its own dot clock (VBL set and cleared on dot 1, /NMI from the VBL flag and enable as a
	// signal, sampled by a 2A03) instead of following the screen; the scanline renderer (2C04 clone, VT03, SH6578) gets
	// it where it is known to apply (keeps_own_timing()) and still draws each line whole at its start
	m_own_timing = keeps_own_timing() && (m_scan_scale == 1);

	// A 2A03 samples /NMI itself at the end of every cycle (NESdev "CPU interrupts"): it reads /NMI from the VBL flag and
	// NMI enable as functions of time, and is told when an IRQ an A12 edge causes really happened. Any other CPU sees
	// the pin, which the PPU drives either way.
	if (m_own_timing)
	{
		if (auto *const cpu = dynamic_cast<rp2a03_core_device *>(m_cpu.target()))
		{
			m_nmi_sampled = true;
			cpu->set_nmi_level_func([this] (const attotime &when) { return nmi_asserted_at(when); });
			m_irq_assert_time = [cpu] (const attotime &when) { cpu->set_irq_assert_time(when); };
		}
	}
	// NTSC and Dendy-style clones: 3 PPU clocks per CPU clock; the 2C07 (PAL): 3.2
	// The dot grid is counted in master clocks with exact integer arithmetic: the CPU divides the master clock by 12 and the
	// PPU by 4 (3 dots per CPU cycle); on the 2C07 the divisors are 16 and 5.  CPU and PPU share this one grid, so a bus
	// access can never land on a dot edge by rounding luck.
	const u32 master_hz = m_cpu->clock() * cpu_master_clocks();
	const u32 dot_clocks = dot_master_clocks();
	m_dot_period = attotime::from_ticks(dot_clocks, master_hz);
	m_odd_frame = false;
	// CPU/PPU alignment: the dot grid leads the CPU cycle grid by m_cpu_ppu_align master clocks; 0 keeps both grids on the
	// same edge
	// A Dendy-style clone's frame is a whole number of CPU cycles, so the phase between the PPU's frame and the CPU cycles
	// never drifts: its alignment covers a whole CPU cycle (15 master clocks), not only the offset within one dot
	const u32 align_range = (type() == PPU_PALC) ? cpu_master_clocks() : dot_clocks;
	const attotime align_shift = attotime::from_ticks(m_cpu_ppu_align % align_range, master_hz);
	m_line_start = machine().time() - align_shift;
	m_next_line_start = m_line_start + m_dot_period * 341;
	m_vbl_event_time = attotime::never;
	if (m_own_timing)
		m_scanline_timer->adjust(m_dot_period * 341 - align_shift);
	else
		m_scanline_timer->adjust(screen().time_until_pos(1));
	m_hblank_timer->adjust(m_dot_period * 260); // dot 260 of the first line, from the PPU's own dot clock
	m_nmi_timer->adjust(attotime::never);

	/* allocate a screen bitmap, videomem and spriteram, a dirtychar array and the monochromatic colortable */
	m_bitmap.allocate(VISIBLE_SCREEN_WIDTH, VISIBLE_SCREEN_HEIGHT);
	m_spriteram = make_unique_clear<u8[]>(m_spriteramsize);

	m_line_dot = 0;
	m_bg_shift_lo = m_bg_shift_hi = m_at_shift_lo = m_at_shift_hi = 0;
	m_bg_next_nt = m_bg_next_at = m_bg_next_lo = m_bg_next_hi = 0;
	m_spr_count = 0;
	std::fill(std::begin(m_spr_x), std::end(m_spr_x), 0);
	std::fill(std::begin(m_spr_attr), std::end(m_spr_attr), 0);
	std::fill(std::begin(m_spr_lo), std::end(m_spr_lo), 0);
	std::fill(std::begin(m_spr_hi), std::end(m_spr_hi), 0);
	std::fill(std::begin(m_sp_ctr), std::end(m_sp_ctr), 0);
	std::fill(std::begin(m_sp_attr), std::end(m_sp_attr), 0);
	std::fill(std::begin(m_sp_lo), std::end(m_sp_lo), 0);
	std::fill(std::begin(m_sp_hi), std::end(m_sp_hi), 0);
	m_at_latch = 0;
	m_spr_zero_in_line = false;
	std::fill(std::begin(m_sec_oam), std::end(m_sec_oam), 0xff);
	std::fill(std::begin(m_sec_oam_src), std::end(m_sec_oam_src), 0xff);
	std::fill(std::begin(m_decay_refresh), std::end(m_decay_refresh), attotime::zero);
	m_a12 = false;
	m_a12_low_since = attotime::zero;
	m_ovf_dot = -1;
	m_eval_n = m_eval_m = m_eval_sec = m_eval_latch = m_eval_found = m_eval_sprite0_mask = 0;
	m_eval_copying = m_eval_done = false;
	m_warmup_until = attotime::zero;
	std::fill(std::begin(m_spr_addr), std::end(m_spr_addr), 0x1ff0);

	init_palette_tables();

	// register for state saving
	save_item(NAME(m_scanline));
	save_item(NAME(m_refresh_data));
	save_item(NAME(m_refresh_latch));
	save_item(NAME(m_x_fine));
	save_item(NAME(m_toggle));
	save_item(NAME(m_render_enabled_hblank));
	save_item(NAME(m_add));
	save_item(NAME(m_videomem_addr));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_buffered_data));
	save_item(NAME(m_tile_page));
	save_item(NAME(m_sprite_page));
	save_item(NAME(m_back_color));
	save_item(NAME(m_scan_scale));
	save_item(NAME(m_scanlines_per_frame));
	save_item(NAME(m_vblank_first_scanline));
	save_item(NAME(m_regs));
	save_item(NAME(m_draw_phase));
	save_item(NAME(m_tilecount));
	save_item(NAME(m_line_dot));
	save_item(NAME(m_line_v)); save_item(NAME(m_line_v_next));
	save_item(NAME(m_at_shift_x0)); save_item(NAME(m_at_shift_x1));
	save_item(NAME(m_bg_next_x0)); save_item(NAME(m_bg_next_x1)); save_item(NAME(m_bg_shift_x0)); save_item(NAME(m_bg_shift_x1));
	save_item(NAME(m_sp_x0)); save_item(NAME(m_sp_x1)); save_item(NAME(m_sp_half));
	save_item(NAME(m_bg_shift_lo));
	save_item(NAME(m_bg_shift_hi));
	save_item(NAME(m_at_shift_lo));
	save_item(NAME(m_at_shift_hi));
	save_item(NAME(m_bg_next_nt));
	save_item(NAME(m_bg_next_at));
	save_item(NAME(m_bg_next_lo));
	save_item(NAME(m_bg_next_hi));
	save_item(NAME(m_spr_count));
	save_item(NAME(m_spr_x));
	save_item(NAME(m_spr_attr));
	save_item(NAME(m_spr_lo));
	save_item(NAME(m_spr_hi));
	save_item(NAME(m_sp_ctr));
	save_item(NAME(m_sp_attr));
	save_item(NAME(m_sp_lo));
	save_item(NAME(m_sp_hi));
	save_item(NAME(m_at_latch));
	save_item(NAME(m_oam_buf));
	save_item(NAME(m_out_mask));
	save_item(NAME(m_out_mask_next));
	save_item(NAME(m_out_mask_delay));
	save_item(NAME(m_r_sr)); save_item(NAME(m_r_req)); save_item(NAME(m_rl)); save_item(NAME(m_pd_rb)); save_item(NAME(m_r_ale));
	save_item(NAME(m_w_sr)); save_item(NAME(m_w_req)); save_item(NAME(m_wl)); save_item(NAME(m_w_ale)); save_item(NAME(m_db_par));
	save_item(NAME(m_tstep_latch)); save_item(NAME(m_blnk_latch)); save_item(NAME(m_pal_en)); save_item(NAME(m_bus_read));
	save_item(NAME(m_bus_ale)); save_item(NAME(m_w_data)); save_item(NAME(m_r_start)); save_item(NAME(m_w_start));
	save_item(NAME(m_2006_delay)); save_item(NAME(m_2006_new)); save_item(NAME(m_2006_old)); save_item(NAME(m_copyv));
	save_item(NAME(m_fetch_addr));
	m_r_start = m_w_start = attotime::never;
	save_item(NAME(m_bus_addr));
	save_item(NAME(m_octal));
	save_item(NAME(m_pat_addr));
	save_item(NAME(m_oam2_addr));
	save_item(NAME(m_oam2_ovf));
	save_item(NAME(m_oam2_reset));
	save_item(NAME(m_eval_tick));
	save_item(NAME(m_eval_nine));
	save_item(NAME(m_eval_addr_ovf));
	save_item(NAME(m_fetch_y));
	save_item(NAME(m_fetch_tile));
	save_item(NAME(m_fetch_attr));
	save_item(NAME(m_fetch_x));
	save_item(NAME(m_spr_zero_in_line));
	save_item(NAME(m_vbl_suppress));
	save_item(NAME(m_vbl_set_pending));
	save_item(NAME(m_nmi_line));
	save_item(NAME(m_line_start));
	save_item(NAME(m_next_line_start));
	save_item(NAME(m_vbl_event_time));
	save_item(NAME(m_render_prev));
	save_item(NAME(m_s0hit_pending));
	save_item(NAME(m_s0hit_time));
	save_item(NAME(m_odd_frame));
	save_item(NAME(m_idle_nt));
	save_item(NAME(m_idle_dot_skipped));
	save_item(NAME(m_nmi_vbl));
	save_item(NAME(m_vbl_set_at));
	save_item(NAME(m_vbl_set_blocked));
	save_item(NAME(m_vbl_clr_at));
	save_item(NAME(m_nmi_en_at));
	save_item(NAME(m_nmi_en_prev));
	save_item(NAME(m_vbl_nmi_end_at));
	save_item(NAME(m_nmi_update_last));
	save_item(NAME(m_sec_oam));
	save_item(NAME(m_sec_oam_src));
	save_item(NAME(m_decay_refresh));
	save_item(NAME(m_a12));
	save_item(NAME(m_bus_a13a12));
	save_item(NAME(m_a12_low_since));
	save_item(NAME(m_a12_armed));
	save_item(NAME(m_powered));
	save_item(NAME(m_render_eff));
	save_item(NAME(m_render_pending));
	save_item(NAME(m_render_delay));
	save_item(NAME(m_rdbuf_value));
	save_item(NAME(m_rdbuf_delay));
	save_item(NAME(m_eval_start));
	save_item(NAME(m_oamw_pending));
	save_item(NAME(m_oamw_data));
	save_item(NAME(m_oamw_delay));
	save_item(NAME(m_oam_start_copied));
	save_item(NAME(m_oam_corrupt_rows));
	save_item(NAME(m_warmup_until));
	save_item(NAME(m_spr_addr));
	save_item(NAME(m_ovf_dot));
	save_item(NAME(m_eval_n));
	save_item(NAME(m_eval_m));
	save_item(NAME(m_eval_sec));
	save_item(NAME(m_eval_latch));
	save_item(NAME(m_eval_found));
	save_item(NAME(m_eval_sprite0_mask));
	save_item(NAME(m_eval_copying));
	save_item(NAME(m_eval_done));
	save_pointer(NAME(m_spriteram), m_spriteramsize);

	save_item(NAME(m_bitmap));
}

void ppu2c0x_device::device_start()
{
	// bind the callbacks set before the device started (later ones are bound when they are set)
	m_scanline_callback_proc.resolve();
	m_hblank_callback_proc.resolve();
	m_a12_callback_proc.resolve();
	m_latch.resolve();
	m_read_latch.resolve();
	m_ale_cb.resolve();
	m_rd_cb.resolve();
	m_wr_cb.resolve();

	start_nopalram();

	m_palette_ram.resize(0x20, 0x00);
	// Palette RAM power-on contents vary between consoles; the NTSC 2C02 starts with the values blargg measured on his
	// NES (blargg_ppu_tests power_up_palette), a real console's state rather than all zeros
	if (type() == PPU_2C02)
	{
		static const u8 power_on[0x20] = {
			0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0d, 0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2c,
			0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14, 0x08, 0x3a, 0x00, 0x02, 0x00, 0x20, 0x2c, 0x08 };
		std::copy(std::begin(power_on), std::end(power_on), m_palette_ram.begin());
		m_back_color = power_on[0];
	}

	save_item(NAME(m_palette_ram));
	// the line being drawn: sprite rows in range, and its palette values before greyscale and emphasis
	save_item(NAME(m_spr_in_range));
	save_item(NAME(m_line_pal));
	save_item(NAME(m_line_pal_y));
}

void ppu2c04_clone_device::device_start()
{
	ppu2c0x_device::device_start();

	/* this PPU clone draws sprites into a frame buffer before displaying them,
	causing sprite rendering to be one frame behind tile/background rendering
	(mainly noticeable during scrolling)
	to simulate that, we can just have a secondary OAM buffer and swap them
	at the end of each frame.

	(theoretically this can cause the wrong sprite tiles to be drawn for
	one frame after changing CHR banks, but the Vs. SMB bootlegs that use
	this clone hardware don't actually have CHR bank switching anyway.
	also generally affects PPU-side read timings involving the OAM, but
	this still doesn't seem to matter for Vs. SMB specifically)
	*/
	m_spritebuf = make_unique_clear<u8[]>(m_spriteramsize);
	save_pointer(NAME(m_spritebuf), m_spriteramsize);
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

u8 ppu2c0x_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}

u8 ppu2c0x_device::ppu_vram_direct_read(offs_t address)
{
	return readbyte(address);
}

void ppu2c0x_device::ppu_vram_direct_write(offs_t address, u8 data)
{
	writebyte(address, data);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void ppu2c0x_device::writebyte(offs_t address, u8 data)
{
	space().write_byte(address, data);
}


inline u16 ppu2c0x_device::apply_grayscale_and_emphasis(u8 color)
{
	// the dot renderer takes greyscale/emphasis from the bits as they reach the pixel output (m_out_mask)
	const u8 mask = use_dot_renderer() ? m_out_mask : m_regs[PPU_CONTROL1];
	u16 palval = color;
	palval &= (mask & PPU_CONTROL1_DISPLAY_MONO) ? 0x30 : 0x3f;
	palval |= (mask & PPU_CONTROL1_COLOR_EMPHASIS) << 1;

	return palval;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*************************************
 *
 *  PPU Palette Initialization
 *
 *************************************/

// Palette from the composite signal, as documented on the NESdev wiki "NTSC video" page: lidnariq's terminated voltage
// levels (including the attenuated levels the emphasis bits produce) generate the PPU's 12-phase square wave for the
// colour, which is decoded to YUV against the colour burst and converted to RGB, black ($1D) to white ($20) normalised
// to 0..1
//
// The 2C07 (PAL) and the UMC 6538 (Dendy and other PAL famiclones) use the same levels and the same 12 oscillators, see
// the NESdev wiki "PAL video" page.  They differ in where the decoder takes its reference: the PAL colour burst is
// phase 7 and defined as -U+V (135 degrees), while the NTSC burst is phase 8 and -U (180 degrees), which moves the
// reference by half a phase step (15 degrees) and gives the PAL palette its hue shift.  PAL alternates the sign of V
// from line to line, and an ideal decoder undoes that, so the two lines decode to the same colour and only this
// reference changes.  (Treating the Dendy's DAC as "brighter and more saturated", which the wiki notes without
// figures, is not modelled.)
static constexpr double PAL_REFERENCE_SHIFT = 0.5;   // phase steps (30 degrees each)

// one sample of the PPU's composite signal for colour index 'pixel' (eeellcccc) at subcarrier phase p (0-11), normalised
// so that black ($1D) is 0 and white ($20) is 1: the same levels and square wave the palette below is built from
static double nes_composite_sample(int pixel, int p)
{
	static const double levels[16] = {
		0.228, 0.312, 0.552, 0.880,   // signal low
		0.616, 0.840, 1.100, 1.100,   // signal high
		0.192, 0.256, 0.448, 0.712,   // signal low, attenuated
		0.500, 0.676, 0.896, 0.896 }; // signal high, attenuated
	const int color = pixel & 0x0f;
	const int level = (color > 13) ? 1 : ((pixel >> 4) & 3);
	const int emphasis = (pixel >> 6) & 7;
	const double black = 0.312, white = 1.100;
	auto in_phase = [p] (int c) { return ((c + p) % 12) < 6; };
	const int att = ((((emphasis & 1) && in_phase(0xc)) || ((emphasis & 2) && in_phase(0x4)) || ((emphasis & 4) && in_phase(0x8))) && (color < 0xe)) ? 8 : 0;
	double low = levels[0 + level + att], high = levels[4 + level + att];
	if (color == 0) low = high;
	if (color > 12) high = low;
	return ((in_phase(color) ? high : low) - black) / (white - black);
}

static rgb_t nespal_signal_to_RGB(int pixel, bool pal)
{
	constexpr double pi = 3.14159265358979323846;   // not M_PI: MinGW does not define it in MAME's build

	double y = 0.0, u = 0.0, v = 0.0;
	for (int p = 0; p < 12; p++)
	{
		const double s = nes_composite_sample(pixel, p) / 12.0;
		y += s;
		const double ref = p + 3.0 - 0.5 + (pal ? PAL_REFERENCE_SHIFT : 0.0);   // +3: carrier reference 90 degrees off, -0.5: another 15
		u += s * sin(pi * ref / 6.0) * 2.0;
		v += s * cos(pi * ref / 6.0) * 2.0;
	}
	auto clamp8 = [] (double c) { c = floor(255.0 * c + 0.5); return u8((c < 0.0) ? 0.0 : (c > 255.0) ? 255.0 : c); };
	return rgb_t(clamp8(y + 1.139883 * v), clamp8(y - 0.394642 * u - 0.580622 * v), clamp8(y + 2.032062 * u));
}

rgb_t ppu2c0x_device::nespal_to_RGB(int color_intensity, int color_num, int color_emphasis, bool is_pal_or_dendy)
{
	// the PAL and Dendy PPUs swap the red and green emphasis bits (NESdev "Colour emphasis"); this is how the PPUs are
	// wired, it does not follow from the PAL line alternation
	if (is_pal_or_dendy)
		color_emphasis = bitswap<3>(color_emphasis, 2, 0, 1);

	return nespal_signal_to_RGB((color_emphasis << 6) | (color_intensity << 4) | color_num, is_pal_or_dendy);
}

// Composite video (NTSC 2C02, when enabled in the machine configuration): the PPU puts out its colour as a square wave
// sampled 8 times per pixel, 12 samples per colour subcarrier cycle (NESdev "NTSC video": 4 master clocks per pixel,
// the subcarrier is the master clock / 6). Each pixel of the finished line is decoded from the 12 samples centred on
// it with the same reference and YUV matrix as the palette above, so a run of one colour decodes to exactly its palette
// colour and only colour changes along the line show the artifacts of the composite signal. The line's subcarrier phase
// comes from the master clock at its start, which gives the line-to-line and frame-to-frame phase steps (dot crawl).
void ppu2c0x_device::composite_decode_line()
{
	constexpr double pi = 3.14159265358979323846;
	static double sample[512][12];
	static double sinref[2][12], cosref[2][12];
	static bool tables = false;
	if (!tables)
	{
		for (int pix = 0; pix < 512; pix++)
			for (int p = 0; p < 12; p++)
				sample[pix][p] = nes_composite_sample(pix, p);
		for (int pal = 0; pal < 2; pal++)
			for (int p = 0; p < 12; p++)
			{
				const double ref = p + 3.0 - 0.5 + (pal ? PAL_REFERENCE_SHIFT : 0.0);
				sinref[pal][p] = sin(pi * ref / 6.0) * 2.0;
				cosref[pal][p] = cos(pi * ref / 6.0) * 2.0;
			}
		tables = true;
	}

	// NTSC: 4 master clocks (8 samples) per pixel. PAL (2C07, and the 6538 of the Dendy): 5 master clocks (10 samples),
	// and on every other line the V component is sent inverted: hue h is generated by the oscillator paired with it
	// (3 and 2, 4 and 1, 5 and C, 6 and B, 7 and A, 8 and 9: (5 - h) mod 12) and the TV decodes that line with V negated
	// (NESdev "PAL video")
	const bool pal = (type() == PPU_2C07) || (type() == PPU_PALC);
	const int spp = pal ? 10 : 8;
	const bool vinv = pal && (m_scanline & 1);
	// the master clock: the 2C02's is the CPU clock times 12; PAL's (5 per dot) comes from the dot period, the PAL and
	// Dendy CPU dividers differing
	const u32 master = pal ? u32(5.0 / m_dot_period.as_double() + 0.5) : u32(m_cpu->clock()) * 12;
	const int phase0 = int((m_line_start.as_ticks(master) * 2) % 12);
	auto clamp8 = [] (double c) { c = floor(255.0 * c + 0.5); return u8((c < 0.0) ? 0.0 : (c > 255.0) ? 255.0 : c); };
	u32 *const row = &m_bitmap.pix(m_scanline, 0);
	for (int x = 0; x < 256; x++)
	{
		double y = 0.0, u = 0.0, v = 0.0;
		for (int k = 0; k < 12; k++)
		{
			const int t = x * spp + spp / 2 - 6 + k;   // the 12 samples centred on the pixel
			const int px = (t < 0) ? 0 : (t >= 256 * spp) ? 255 : (t / spp);
			const int p = (((phase0 + t) % 12) + 12) % 12;
			u16 idx = m_line_idx[px];
			if (vinv)
			{
				const int hue = idx & 0x0f;
				if ((hue >= 1) && (hue <= 12))
					idx = (idx & ~0x0f) | ((((5 - hue) % 12) + 12) % 12 == 0 ? 12 : (((5 - hue) % 12) + 12) % 12);
			}
			const double s = sample[idx][p] / 12.0;
			y += s;
			u += s * sinref[pal][p];
			v += s * cosref[pal][p];
		}
		if (vinv)
			v = -v;
		if (pal && (m_composite == 2))
		{
			// delay-line decoding: this line's chroma averaged with the line above
			const double cu = u, cv = v;
			if (m_scanline > 0)
			{
				u = (u + m_comp_prev_u[x]) / 2.0;
				v = (v + m_comp_prev_v[x]) / 2.0;
			}
			m_comp_prev_u[x] = cu;
			m_comp_prev_v[x] = cv;
		}
		row[x] = rgb_t(clamp8(y + 1.139883 * v), clamp8(y - 0.394642 * u - 0.580622 * v), clamp8(y + 2.032062 * u));
	}
}

void ppu2c0x_device::init_palette_tables()
{
	const bool is_pal = m_scanlines_per_frame != NTSC_SCANLINES_PER_FRAME;

	/* This routine builds a palette using a transformation from */
	/* the YUV (Y, B-Y, R-Y) to the RGB color space */

	/* The NES has a 64 color palette                        */
	/* 16 colors, with 4 luminance levels for each color     */
	/* The 16 colors circle around the YUV color space,      */

	int entry = 0;

	/* Loop through the emphasis modes (8 total) */
	for (int color_emphasis = 0; color_emphasis < 8; color_emphasis++)
	{
		/* loop through the 4 intensities */
		for (int color_intensity = 0; color_intensity < 4; color_intensity++)
		{
			/* loop through the 16 colors */
			for (int color_num = 0; color_num < 16; color_num++)
			{
				const rgb_t col = nespal_to_RGB(color_intensity, color_num, color_emphasis, is_pal);

				set_pen_color(entry, col);
				entry++;

			}
		}
	}
}

void ppu2c0x_rgb_device::init_palette_tables()
{
	/* Loop through the emphasis modes (8 total) */
	int entry = 0;
	for (int color_emphasis = 0; color_emphasis < 8; color_emphasis++)
	{
		for (int color_num = 0; color_num < 64; color_num++)
		{
			const int R = ((color_emphasis & 1) ? 7 : m_palette_data[color_num * 3]);
			const int G = ((color_emphasis & 2) ? 7 : m_palette_data[color_num * 3 + 1]);
			const int B = ((color_emphasis & 4) ? 7 : m_palette_data[color_num * 3 + 2]);

			set_pen_color(entry, rgb_t(pal3bit(R), pal3bit(G), pal3bit(B)));

			//set_pen_color(entry++, pal3bit(R), pal3bit(G), pal3bit(B));
			entry++;
		}
	}
}

void ppu2c04_clone_device::init_palette_tables()
{
	/* clone HW doesn't use color emphasis bits.
	   however, it does have two separate palettes: colors 0-63 for background, and 64-127 for sprites
	   (although the tile and sprite colors are identical in the Vs. SMB bootleg ROMs)
	*/
	for (int color_num = 0; color_num < 64*2; color_num++)
	{
		/* A7 line on palette ROMs is always high, color bits are in reverse order */
		const u8 color = m_palette_data[color_num | 0x80];
		const int R = bitswap<3>(color, 0, 1, 2);
		const int G = bitswap<3>(color, 3, 4, 5);
		const int B = bitswap<2>(color, 6, 7);

		set_pen_color(color_num, rgb_t(pal3bit(R), pal3bit(G), pal2bit(B)));
	}
}

/*************************************
 *
 *  PPU Initialization and Disposal
 *
 *************************************/

//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ppu2c0x_device::hsync_tick)
{
	if (use_dot_renderer())
		dot_run_to(current_line_dot());
	const bool blanked = (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) == 0;
	const bool vblank = (m_scanline >= m_vblank_first_scanline - 1) && (m_scanline < m_scanlines_per_frame - 1);
	if (!m_hblank_callback_proc.isnull())
		m_hblank_callback_proc(m_scanline, vblank, blanked);
}

TIMER_CALLBACK_MEMBER(ppu2c0x_device::hblank_tick)
{
	const bool blanked = (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) == 0;
	const bool vblank = (m_scanline >= m_vblank_first_scanline - 1) && (m_scanline < m_scanlines_per_frame - 1);

	// the vertical scroll increment happens at dot 256, before hblank, so remember whether rendering was on by now
	m_render_enabled_hblank = !blanked;

	//update_scanline();

	if (use_dot_renderer())
	{
		dot_run_to(current_line_dot());

		// with 8x8 sprites and both pattern tables at $0000, PPU A12 never rises, so the mapper scanline counter is not clocked
		if (hblank_dot() < 0)
		{
			m_hblank_timer->adjust(attotime::never);
			return;
		}
	}

	// with the callback at HSYNC (own timing) it is called by hsync_tick instead
	if (!m_hblank_callback_proc.isnull() && !(m_hblank_at_hsync && m_own_timing))
		m_hblank_callback_proc(m_scanline, vblank, blanked);

	m_hblank_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(ppu2c0x_device::nmi_tick)
{
	if (use_dot_renderer())
	{
		update_nmi_line();
		m_nmi_timer->adjust(attotime::never);
		if (m_own_timing && (m_nmi_update_last > machine().time()))
			m_nmi_timer->adjust(m_nmi_update_last - machine().time());
		return;
	}

	// Actually fire the NMI, unless it was disabled again during the short delay
	if (m_regs[PPU_CONTROL0] & PPU_CONTROL0_NMI)
	{
		m_int_callback(ASSERT_LINE);
		m_int_callback(CLEAR_LINE);
	}

	m_nmi_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(ppu2c0x_device::scanline_tick)
{
	const bool blanked = (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) == 0;
	const bool vblank = ((m_scanline >= m_vblank_first_scanline - 1) && (m_scanline < m_scanlines_per_frame - 1)) ? 1 : 0;

	/* if a callback is available, call it */
	if (!m_scanline_callback_proc.isnull())
		m_scanline_callback_proc(m_scanline, vblank, blanked);

	/* update the scanline that just went by */
	if (use_dot_renderer())
		dot_run_to(341);
	else
		update_scanline();

	/* increment our scanline count */
	m_scanline++;

	//logerror("starting scanline %d (MAME %d, beam %d)\n", m_scanline, device->screen().vpos(), device->screen().hpos());

	/* Note: this is called at the _end_ of each scanline */
	if (m_own_timing && (m_scanline == m_vblank_first_scanline - 1))
	{
		// this frame's VBL set pulse, on dot 1 of the next line: known now for the $2002 reads that race it
		m_vbl_set_at = m_next_line_start + m_dot_period * 342;
		m_vbl_set_blocked = false;
		m_vbl_nmi_end_at = attotime::never;
	}
	if (m_scanline == m_vblank_first_scanline)
	{
		// logerror("vblank starting\n");
		if (use_dot_renderer() || m_own_timing)
		{
			/* the VBL flag is set on dot 1 of the first vblank line */
			m_vbl_set_pending = true;
			if (m_own_timing)
			{
				m_vbl_event_time = m_next_line_start + m_dot_period;   // the line that starts now (m_line_start is updated below)
				arm_event(m_vbl_timer, m_vbl_event_time, 1);
			}
			else
				m_vbl_timer->adjust(dot_time(1), 1);
		}
		else
		{
		/* We just entered VBLANK */
		m_regs[PPU_STATUS] |= PPU_STATUS_VBLANK;

		/* If NMI's are set to be triggered, go for it */
		if (m_regs[PPU_CONTROL0] & PPU_CONTROL0_NMI)
		{
			// We need an ever-so-slight delay between entering vblank and firing an NMI - enough so that
			// a game can read the high bit of $2002 before the NMI is called (potentially resetting the bit
			// via a read from $2002 in the NMI handler).
			// B-Wings is an example game that needs this.
			m_nmi_timer->adjust(m_cpu->cycles_to_attotime(4));
		}
		}
	}

	if (m_scanline == m_scanlines_per_frame - 1)
	{
		//logerror("vblank ending\n");
		/* clear the vblank & sprite hit flag (dot renderer and own timing: on dot 1 of the pre-render line) */
		if (use_dot_renderer() || m_own_timing)
		{
			m_nmi_vbl = false;
			if (m_own_timing)
			{
				m_vbl_nmi_end_at = m_next_line_start;   // the pre-render line starts now
				nmi_line_update_due(m_vbl_nmi_end_at);
			}
			else
				update_nmi_line();
			if (m_own_timing)
			{
				m_vbl_event_time = m_next_line_start + m_dot_period;   // the line that starts now (m_line_start is updated below)
				arm_event(m_vbl_timer, m_vbl_event_time, 0);
			}
			else
				m_vbl_timer->adjust(dot_time(1), 0);
		}
		else
			m_regs[PPU_STATUS] &= ~(PPU_STATUS_VBLANK | PPU_STATUS_SPRITE0_HIT | PPU_STATUS_8SPRITES);
	}

	/* see if we rolled */
	else if (m_scanline == m_scanlines_per_frame)
	{
		/* if background or sprites are enabled, copy the ppu address latch (the dot renderer does this on the pre-render line) */
		if (!blanked && !use_dot_renderer())
			m_refresh_data = m_refresh_latch;

		/* reset the scanline count */
		m_scanline = 0;
		if (m_own_timing)
		{
			m_odd_frame = !m_odd_frame;
			screen().reset_origin(0, 0);
		}
		//logerror("sprite 0 x: %d y: %d num: %d\n", m_spriteram[3], m_spriteram[0] + 1, m_spriteram[1]);
	}

	int next_scanline = m_scanline + 1;
	if (next_scanline == m_scanlines_per_frame)
		next_scanline = 0;

	m_line_dot = 0;

	// Call us back when the hblank starts for this scanline (dot renderer: at the dot where PPU A12 first rises)
	// (timed from the line's logical start, which can be up to a CPU cycle before this callback)
	const attotime line_late = m_own_timing ? (machine().time() - m_next_line_start) : attotime::zero;
	if (m_own_timing && !m_a12_callback_proc.isnull())
	{
		// just after the line's first counted A12 edge. Each is aimed half a dot past the dot boundary it has to reach: a
		// timer aimed at the boundary itself can fire an attosecond short of it, and the PPU, brought up to the dot that
		// is then current, stops one dot short - leaving the edge's dot (and the mapper IRQ it causes) for the end of the
		// line, ~80 dots late (blargg mmc3_test 4-scanline_timing)
		m_hblank_timer->adjust(m_dot_period * (((hblank_dot() == 324) ? 326 : 262) + (m_a12_on_fall ? 4 : 0)) + m_dot_period / 2 - line_late);
		m_a12_catchup_timer->adjust(m_dot_period * 6 + m_dot_period / 2 - line_late);
	}
	else if (m_own_timing && hblank_dot() > 0)
		m_hblank_timer->adjust(m_dot_period * hblank_dot() - line_late);
	else if (use_dot_renderer() && hblank_dot() > 0)
		m_hblank_timer->adjust(m_cpu->cycles_to_attotime(hblank_dot()) / 3);
	else
		m_hblank_timer->adjust(m_dot_period * 260); // dot 260, from the PPU's own dot clock (also right for PAL)

	// a device that counts horizontal sync (the VT02+ timer with TSYNEN = 1) gets the hblank callback when HSYNC begins:
	// after the 256 visible dots, the 11-dot right border and the 9-dot front porch, at dot 277 (NESdev "NTSC video",
	// horizontal timing), not at the dot an MMC3 would see A12 rise
	if (m_own_timing && m_hblank_at_hsync)
		m_hsync_timer->adjust(m_dot_period * 277 + m_dot_period / 2 - line_late);

	// trigger again at the start of the next scanline
	if (m_own_timing)
		schedule_next_line();
	else
		m_scanline_timer->adjust(screen().time_until_pos(next_scanline * m_scan_scale));
}


void ppu2c0x_device::read_tile_plane_data(int address, int color)
{
	m_planebuf[0] = readbyte(address & 0x1fff);
	m_planebuf[1] = readbyte((address + 8) & 0x1fff);
}

void ppu2c0x_device::shift_tile_plane_data(u8& pix)
{
	pix = BIT(m_planebuf[0], 7) | (BIT(m_planebuf[1], 7) << 1);
	m_planebuf[0] <<= 1;
	m_planebuf[1] <<= 1;
}

void ppu2c0x_device::draw_tile_pixel(u8 pix, int color, u32 back_pen, u32*& dest)
{
	const u16 palval = pix ? m_palette_ram[((4 * color) + pix) & 0x1f] : back_pen;

	*dest = pen_color(apply_grayscale_and_emphasis(palval));
}

void ppu2c0x_device::draw_tile(u8* line_priority, int color_byte, int color_bits, int address, int start_x, u32 back_pen, u32*& dest)
{
	const int color = (color_byte >> color_bits) & 0x03;

	read_tile_plane_data(address, color);

	/* render the pixel */
	for (int i = 0; i < 8; i++)
	{
		u8 pix;
		shift_tile_plane_data(pix);

		if ((start_x + i) >= 0 && (start_x + i) < VISIBLE_SCREEN_WIDTH)
		{
			draw_tile_pixel(pix, color, back_pen, dest);

			// priority marking
			if (pix)
				line_priority[start_x + i] |= 0x02;
		}
		dest++;
	}
}


// m_refresh_data is important as it is updated during rendering, and overwritten when you write new scroll values
// making raster effects more complex than on other systems
// https://retrocomputing.stackexchange.com/questions/1898/how-can-i-create-a-split-scroll-effect-in-an-nes-game

void ppu2c0x_device::draw_background(u8* line_priority)
{
	/* determine where in the nametable to start drawing from */
	/* based on the current scanline and scroll regs */
	const u8  scroll_x_coarse = m_refresh_data & 0x001f;
	const u8  scroll_y_coarse = (m_refresh_data & 0x03e0) >> 5;
	const u16 nametable = m_refresh_data & 0x0c00;
	const u8  scroll_y_fine = (m_refresh_data & 0x7000) >> 12;

	int x = scroll_x_coarse;

	/* get the tile index */
	int tile_index = (nametable | 0x2000) + scroll_y_coarse * 32;

	/* set up dest */
	int start_x = (m_x_fine ^ 0x07) - 7;
	u32* dest = &m_bitmap.pix(m_scanline, start_x);

	m_tilecount = 0;

	/* draw the 32 or 33 tiles that make up a line */
	while (m_tilecount < 34)
	{
		const int index1 = tile_index + x;

		// page2 is the output of the nametable read (this section is the FIRST read per tile!)
		const int page2 = readbyte(index1);

		// this is attribute table stuff! (actually read 2 in PPUspeak)!
		/* Figure out which byte in the color table to use */
		const int pos = ((index1 & 0x380) >> 4) | ((index1 & 0x1f) >> 2);
		const int page = (index1 & 0x0c00) >> 10;
		int address = 0x3c0 + pos;
		const int color_byte = readbyte((((page * 0x400) + address) & 0xfff) + 0x2000);

		/* figure out which bits in the color table to use */
		const int color_bits = ((index1 & 0x40) >> 4) + (index1 & 0x02);

		// 27/12/2002
		if (!m_latch.isnull())
			m_latch((m_tile_page << 10) | (page2 << 4));

		if (start_x < VISIBLE_SCREEN_WIDTH)
		{
			// need to read 0x0000 or 0x1000 + 16*nametable data
			address = (m_tile_page ? 0x1000 : 0) + (page2 * 16);
			// plus something that accounts for y
			address += scroll_y_fine;

			draw_tile(line_priority, color_byte, color_bits, address, start_x, m_back_color, dest);

			start_x += 8;

			/* move to next tile over and toggle the horizontal name table if necessary */
			x++;
			if (x > 31)
			{
				x = 0;
				tile_index ^= 0x400;
			}
		}
		m_tilecount++;
	}

	/* if the left 8 pixels for the background are off, blank 'em */
	if (!(m_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND_L8))
	{
		dest = &m_bitmap.pix(m_scanline);
		for (int i = 0; i < 8; i++)
		{
			draw_back_pen(dest, m_back_color);
			dest++;

			line_priority[i] &= ~0x02;
		}
	}
}

// $2001 writes do nothing on this clone: background and sprites are always on. The dot renderer acts on the effective
// mask a $2001 write sets up, so the fixed value goes through that path at every reset
void ppu2c04_clone_device::device_reset()
{
	ppu2c0x_device::device_reset();
	ppu2c0x_device::write(PPU_CONTROL1, u8(~(PPU_CONTROL1_COLOR_EMPHASIS | PPU_CONTROL1_DISPLAY_MONO)));
}

// name table selection is ignored above the hardwired split at line 31 (as in draw_background below): v's name table
// bits are cleared before the prefetch of each such line
void ppu2c04_clone_device::dot_before_prefetch()
{
	const int next = (m_scanline == m_scanlines_per_frame - 1) ? 0 : (m_scanline + 1);
	if (next < 31)
		m_refresh_data &= ~0x0c00;
}

// the sprite buffer of this frame is cleared after being displayed, and the one filled this frame is shown next frame
// (as in draw_sprites below). Not modelled, and not known: when the clone reads the sprites' CHR data into its frame
// buffer; the sprites take the CHR of the frame they are shown in, which matters only with CHR bank switching, which
// the Vs. SMB bootlegs using this clone do not have
void ppu2c04_clone_device::dot_line_done()
{
	if (m_scanline == BOTTOM_VISIBLE_SCANLINE)
	{
		m_spriteram.swap(m_spritebuf);
		memset(m_spritebuf.get(), 0, m_spriteramsize);
	}
}

// sprites are clipped at the screen edges and take the second half of the clone's palette (draw_sprite_pixel below)
bool ppu2c04_clone_device::dot_pixel_rgb(int x, u8 bg_pix, u8 bg_pal, bool bg_opaque, u8 sp_pix, u8 sp_pal, bool sprite_wins, u32 *dest)
{
	if (!sp_pix)
		return false;
	if (sprite_wins && (x >= 8) && (x < VISIBLE_SCREEN_WIDTH - 6))
	{
		*dest = pen_color(m_palette_ram[((4 * sp_pal) | sp_pix) & 0x1f] | 0x40);
		return true;
	}
	// a clipped sprite pixel shows what is behind it
	// (this clone has neither greyscale nor colour emphasis, so the palette value is the pen)
	*dest = pen_color(bg_opaque ? m_palette_ram[((4 * bg_pal) | bg_pix) & 0x1f] : m_back_color);
	return true;
}

void ppu2c04_clone_device::draw_background(u8* line_priority)
{
	// nametable selection is ignored below the hardwired scroll split position
	if (m_scanline < 31)
		m_refresh_data &= ~0x0c00;

	ppu2c0x_device::draw_background(line_priority);
}

void ppu2c0x_device::draw_back_pen(u32* dest, int back_pen)
{
	*dest = pen_color(apply_grayscale_and_emphasis(back_pen));
}

void ppu2c0x_device::draw_background_pen()
{
	// Fill this scanline with the background pen.
	for (int i = 0; i < m_bitmap.width(); i++)
		draw_back_pen(&m_bitmap.pix(m_scanline, i), m_back_color);
}

void ppu2c0x_device::read_sprite_plane_data(int address)
{
	m_planebuf[0] = readbyte(address & 0x1fff);
	m_planebuf[1] = readbyte((address + 8) & 0x1fff);
}

void ppu2c0x_device::make_sprite_pixel_data(u8 &pixel_data, bool flipx)
{
	if (flipx)
	{
		pixel_data = (m_planebuf[0] & 1) | ((m_planebuf[1] & 1) << 1);
		m_planebuf[0] >>= 1;
		m_planebuf[1] >>= 1;
	}
	else
	{
		pixel_data = BIT(m_planebuf[0], 7) | (BIT(m_planebuf[1], 7) << 1);
		m_planebuf[0] <<= 1;
		m_planebuf[1] <<= 1;
	}
}

void ppu2c0x_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, u8 pixel_data, bitmap_rgb32 &bitmap)
{
	const u16 palval = m_palette_ram[((4 * color) | pixel_data) & 0x1f];
	bitmap.pix(m_scanline, sprite_xpos + pixel) = pen_color(apply_grayscale_and_emphasis(palval));
}

void ppu2c04_clone_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, u8 pixel_data, bitmap_rgb32 &bitmap)
{
	/* clone PPU clips sprites at the screen edges */
	if ((sprite_xpos + pixel < 8) || (sprite_xpos + pixel) >= (VISIBLE_SCREEN_WIDTH - 6))
		return;

	const u16 palval = m_palette_ram[((4 * color) | pixel_data) & 0x1f];
	bitmap.pix(m_scanline, sprite_xpos + pixel) = pen_color(palval | 0x40);
}

void ppu2c0x_device::read_extra_sprite_bits(int sprite_index)
{
	// needed for some clones
}

bool ppu2c0x_device::is_spritepixel_opaque(int pixel_data, int color)
{
	if (pixel_data)
		return true;
	else
		return false;
}

void ppu2c0x_device::draw_sprite_pixel_low(bitmap_rgb32 &bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, u8 *line_priority)
{
	if (is_spritepixel_opaque(pixel_data, color))
	{
		/* has the background (or another sprite) already been drawn here? */
		if ((sprite_xpos + pixel) < VISIBLE_SCREEN_WIDTH)
		{
			if (!line_priority[sprite_xpos + pixel])
			{
				/* no, draw */
				draw_sprite_pixel(sprite_xpos, color, pixel, pixel_data, bitmap);
			}
			/* indicate that a sprite was drawn at this location, even if it's not seen */
			line_priority[sprite_xpos + pixel] |= 0x01;
		}
	}

	/* set the "sprite 0 hit" flag if appropriate */
	if (sprite_index == 0 && (pixel_data & 0x03) && ((sprite_xpos + pixel) < 255) && (line_priority[sprite_xpos + pixel] & 0x02))
		m_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
}

void ppu2c0x_device::draw_sprite_pixel_high(bitmap_rgb32 &bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, u8 *line_priority)
{
	if (is_spritepixel_opaque(pixel_data, color))
	{
		if ((sprite_xpos + pixel) < VISIBLE_SCREEN_WIDTH)
		{
			/* has another sprite been drawn here? */
			if (BIT(~line_priority[sprite_xpos + pixel], 0))
			{
				/* no, draw */
				draw_sprite_pixel(sprite_xpos, color, pixel, pixel_data, bitmap);
				line_priority[sprite_xpos + pixel] |= 0x01;
			}
		}
	}

	/* set the "sprite 0 hit" flag if appropriate */
	if (sprite_index == 0 && (pixel_data & 0x03) && ((sprite_xpos + pixel) < 255) && (line_priority[sprite_xpos + pixel] & 0x02))
		m_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
}

int ppu2c0x_device::apply_sprite_pattern_page(int index1, int size)
{
	if (size == 8)
		index1 += (m_sprite_page == 0) ? 0 : 0x1000;

	return index1;
}

void ppu2c0x_device::draw_sprites(u8 *line_priority)
{
	int sprite_count = 0;

	/* determine if the sprites are 8x8 or 8x16 */
	int size = (m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE) ? 16 : 8;

	int first_pixel = (m_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES_L8) ? 0 : 8;

	// use 0x100 instead of m_spriteramsize because this is for old handling
	for (int sprite_index = 0; sprite_index < 0x100; sprite_index += 4)
	{
		int sprite_ypos = m_spriteram[sprite_index] + 1;
		int sprite_xpos = m_spriteram[sprite_index + 3];

		// The sprite collision acts funny on the last pixel of a scanline.
		// The various scanline latches update while the last few pixels
		// are being drawn. Since we don't do cycle-by-cycle PPU emulation,
		// we fudge it a bit here so that sprite 0 collisions are detected
		// when, e.g., sprite x is 254, sprite y is 29 and we're rendering
		// at the end of scanline 28.
		// Battletoads needs this level of precision to be playable.
		if ((sprite_index == 0) && (sprite_xpos == 254))
		{
			sprite_ypos--;
		}

		/* if the sprite isn't visible, skip it */
		if ((sprite_ypos + size <= m_scanline) || (sprite_ypos > m_scanline))
			continue;

		int tile = m_spriteram[sprite_index + 1];
		const int color = (m_spriteram[sprite_index + 2] & 0x03) + 4;
		const bool pri = BIT(m_spriteram[sprite_index + 2], 5);
		const bool flipx = BIT(m_spriteram[sprite_index + 2], 6);
		const bool flipy = BIT(m_spriteram[sprite_index + 2], 7);
		read_extra_sprite_bits(sprite_index);

		if (size == 16)
		{
			/* if it's 8x16 and odd-numbered, draw the other half instead */
			if (BIT(tile, 0))
			{
				tile &= ~0x01;
				tile |= 0x100;
			}
		}

		if (!m_latch.isnull())
			m_latch((m_sprite_page << 10) | ((tile & 0xff) << 4));

		/* compute the character's line to draw */
		int sprite_line = m_scanline - sprite_ypos;

		if (flipy)
			sprite_line = (size - 1) - sprite_line;

		if (size == 16 && sprite_line > 7)
		{
			tile++;
			sprite_line -= 8;
		}

		int index1 = tile * 16;

		index1 = apply_sprite_pattern_page(index1, size);

		read_sprite_plane_data(index1 + sprite_line);

		/* if there are more than 8 sprites on this line, set the flag */
		if (sprite_count == 8)
		{
			m_regs[PPU_STATUS] |= PPU_STATUS_8SPRITES;
			//logerror ("> 8 sprites, scanline: %d\n", m_scanline);

			/* the real NES only draws up to 8 sprites - the rest should be invisible */
			break;
		}

		sprite_count++;

		/* abort drawing if sprites aren't rendered */
		if (!(m_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES))
			continue;

		if (pri)
		{
			/* draw the low-priority sprites */
			for (int pixel = 0; pixel < 8; pixel++)
			{
				u8 pixel_data;
				make_sprite_pixel_data(pixel_data, flipx);

				/* is this pixel non-transparent? */
				if (sprite_xpos + pixel >= first_pixel)
				{
					draw_sprite_pixel_low(m_bitmap, pixel_data, pixel, sprite_xpos, color, sprite_index, line_priority);
				}
			}
		}
		else
		{
			/* draw the high-priority sprites */
			for (int pixel = 0; pixel < 8; pixel++)
			{
				u8 pixel_data;
				make_sprite_pixel_data(pixel_data, flipx);

				/* is this pixel non-transparent? */
				if (sprite_xpos + pixel >= first_pixel)
				{
					draw_sprite_pixel_high(m_bitmap, pixel_data, pixel, sprite_xpos, color, sprite_index, line_priority);
				}
			}
		}
	}
}

void ppu2c04_clone_device::draw_sprites(u8 *line_priority)
{
	ppu2c0x_device::draw_sprites(line_priority);

	if (m_scanline == BOTTOM_VISIBLE_SCANLINE)
	{
		/* this frame's sprite buffer is cleared after being displayed
		and the other one that was filled this frame will be displayed next frame */
		m_spriteram.swap(m_spritebuf);
		memset(m_spritebuf.get(), 0, m_spriteramsize);
	}
}

/*************************************
 *
 *  Scanline Rendering and Update
 *
 *************************************/

void ppu2c0x_device::render_scanline()
{
	u8 line_priority[VISIBLE_SCREEN_WIDTH];

	// lets see how long it takes
	auto profile = g_profiler.start(PROFILER_USER1);

	// clear the line priority for this scanline
	memset(line_priority, 0, VISIBLE_SCREEN_WIDTH);

	m_draw_phase = PPU_DRAW_BG;

	// see if we need to render the background
	if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND)
		draw_background(line_priority);
	else
		draw_background_pen();

	m_draw_phase = PPU_DRAW_OAM;

	// if sprites are on, draw them, but we call always to process them
	draw_sprites(line_priority);

	m_draw_phase = PPU_DRAW_BG;
}

void ppu2c0x_device::scanline_increment_fine_ycounter()
{
	/* increment the fine y-scroll */
	m_refresh_data += 0x1000;

	/* if it's rolled, increment the coarse y-scroll */
	if (m_refresh_data & 0x8000)
	{
		const u16 tmp = (m_refresh_data & 0x03e0) + 0x20;
		m_refresh_data &= 0x7c1f;

		/* handle bizarro scrolling rollover at the 30th (not 32nd) vertical tile */
		if (tmp == 0x03c0)
			m_refresh_data ^= 0x0800;
		else
			m_refresh_data |= tmp & 0x03e0;

		//logerror("updating refresh_data: %04x\n", m_refresh_data);
	}
}

void ppu2c0x_device::update_visible_enabled_scanline()
{
	if (m_scanline_timer->remaining() == attotime::zero)
	{
		/* If background or sprites are enabled, copy the ppu address latch */
		/* Copy only the scroll x-coarse and the x-overflow bit */
		m_refresh_data &= ~0x041f;
		m_refresh_data |= m_refresh_latch & 0x041f;
	}

	//logerror("updating refresh_data: %04x (scanline: %d)\n", m_refresh_data, m_scanline);
	render_scanline();
}

void ppu2c0x_device::update_visible_disabled_scanline()
{
	u32 back_pen = m_back_color;

	if (m_paletteram_in_ppuspace)
	{
		/* cache the background pen */
		if (m_videomem_addr >= 0x3f00)
		{
			// If the PPU's VRAM address happens to point into palette ram space while
			// both the sprites and background are disabled, the PPU paints the scanline
			// with the palette entry at the VRAM address instead of the usual background
			// pen. Micro Machines makes use of this feature.
			// Note, the PPU DOES access normally unused background palette colors.
			back_pen = m_palette_ram[m_videomem_addr & 0x1f];
		}
	}

	// Fill this scanline with the background pen.
	for (int i = 0; i < m_bitmap.width(); i++)
		draw_back_pen(&m_bitmap.pix(m_scanline, i), back_pen);
}

void ppu2c0x_device::update_visible_scanline()
{
	/* Render this scanline if appropriate */
	if (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES))
	{
		update_visible_enabled_scanline();
	}
	else
	{
		update_visible_disabled_scanline();
	}

	/* the vertical scroll increment only happens if rendering was enabled when the line's visible part ended */
	if (m_render_enabled_hblank && (m_scanline_timer->remaining() == attotime::zero))
	{
		scanline_increment_fine_ycounter();
	}
}

/*************************************
 *
 *  Dot-based renderer
 *
 *  Follows the NESdev "PPU rendering" frame timing: tile fetches every 8 dots (nametable, attribute,
 *  pattern low, pattern high) on dots 1-256 and 321-336, shifters moving on dots 2-257 and 322-337 and
 *  reloading on dots 9, 17, ..., 257, 329 and 337, coarse X increments every 8 dots, the vertical increment
 *  on dot 256, the horizontal copy on dot 257 and the vertical copy on dots 280-304 of the pre-render line.
 *  All of these only happen while rendering (background or sprites) is enabled at that dot.
 *
 *************************************/

attotime ppu2c0x_device::dot_time(int dots)
{
	if (m_own_timing)
		return m_dot_period * dots;
	return attotime::from_double(screen().scan_period().as_double() * m_scan_scale * dots / 341.0);
}

// event_fire_time() below moves an event that falls after the CPU's access point to the end of that cycle, and its
// boundary is the M2 rise, where a read is made. On a CPU whose writes are made later in the cycle than its M2 rise (the
// original 2A03, see rp2a03_core_device::set_m2_write_point) an event between the two points has been moved although it
// precedes the write: it is applied here, before the write is handled. On every other CPU the two points are the same
// and nothing is ever found.
void ppu2c0x_device::apply_due_events()
{
	if (!m_own_timing || m_applying_events)
		return;
	const rp2a03_core_device *const cpu = dynamic_cast<const rp2a03_core_device *>(m_cpu.target());
	if (!cpu || !cpu->m2_state() || !cpu->m2_is_write() || !(cpu->m2_write_offset() > cpu->m2_rise_offset()))
		return;

	m_applying_events = true;
	const attotime now = machine().time();
	for (int guard = 0; guard < 4; guard++)
	{
		const bool vbl_due = m_vbl_timer->enabled() && !m_vbl_timer->expire().is_never() && (m_vbl_timer->expire() > now) && (m_vbl_event_time <= now);
		const bool line_due = m_scanline_timer->enabled() && !m_scanline_timer->expire().is_never() && (m_scanline_timer->expire() > now) && (m_next_line_start <= now);
		if (vbl_due && (!line_due || (m_vbl_event_time <= m_next_line_start)))
		{
			const int param = m_vbl_timer->param();
			m_vbl_timer->adjust(attotime::never);
			vbl_tick(param);
		}
		else if (line_due)
		{
			m_scanline_timer->adjust(attotime::never);
			scanline_tick(0);
		}
		else
			break;
	}
	m_applying_events = false;
}

// The CPU makes one bus access per cycle, at M2 rise, but MAME only stops it on cycle boundaries. A PPU event (line
// start, VBL set/clear) that falls in a CPU cycle after that cycle's access would otherwise be applied before the
// access and show it the PPU's future; such an event is applied at the end of that CPU cycle instead. Its logical time
// (what dot positions are computed from) is unchanged.
attotime ppu2c0x_device::event_fire_time(const attotime &when)
{
	if (!m_own_timing)
		return when;
	const attotime period = m_cpu->cycles_to_attotime(1);
	const attotime cpu_at = m_cpu->local_time() - m_cpu->bus_offset();   // a cycle boundary outside an access
	if (when <= cpu_at)
		return when;
	const u64 k = (when - cpu_at).as_ticks(m_cpu->clock()) ;            // whole CPU cycles from cpu_at to 'when'
	const attotime cycle_start = cpu_at + period * u32(k);
	const attotime access = cycle_start + m2_rise_offset();   // M2 rise
	return (when <= access) ? when : (cycle_start + period);
}

void ppu2c0x_device::arm_event(emu_timer *timer, const attotime &when, int param)
{
	const attotime fire = event_fire_time(when);
	const attotime now = machine().time();
	timer->adjust((fire > now) ? (fire - now) : attotime::zero, param);
}

void ppu2c0x_device::schedule_next_line()
{
	m_line_start = m_next_line_start;
	m_next_line_start = m_line_start + m_dot_period * 341;
	arm_event(m_scanline_timer, m_next_line_start);
}

void ppu2c0x_device::nmi_line_update_due(const attotime &event)
{
	// the pin follows the model nmi_output_delay() after a change of the flag or the enable
	const attotime due = event + nmi_output_delay();
	const attotime now = machine().time();
	if (due > m_nmi_update_last)
		m_nmi_update_last = due;
	if (!m_nmi_timer->enabled() || (m_nmi_timer->expire() > due))
		m_nmi_timer->adjust((due > now) ? (due - now) : attotime::zero);
}

attotime ppu2c0x_device::nmi_output_delay() const
{
	// the single-chip Dendy-style clones (UMC UM6561: NESdev pinout, no /NMI pin) take the NMI straight from the flag
	if (type() == PPU_PALC)
		return attotime::zero;
	return m_dot_period * 3 / dot_master_clocks();
}

bool ppu2c0x_device::rgb_ppu() const
{
	return (type() == PPU_2C03B) || (type() == PPU_2C04) || (type() == PPU_2C05_01) || (type() == PPU_2C05_02)
			|| (type() == PPU_2C05_03) || (type() == PPU_2C05_04);
}

bool ppu2c0x_device::oam_palette_readable() const
{
	if (rgb_ppu())
		return false;
	// NESdev PPU registers: OAMDATA readability was added on the RP2C02G; palette reads are supported by the 2C02G, 2C02H
	// and the PAL PPUs
	return !((type() == PPU_2C02) && (m_revision < REV_2C02G));
}

bool ppu2c0x_device::oam_corruption() const
{
	// NESdev: present on the 2C02G ("PPU registers", OAMADDR precautions); "OAM corruption added in revision E" ("PPU
	// variants", RC2C05-99); the 2C03, 2C04, 2C05 and 2C07 do not have it. Emulated here: the row copy when rendering
	// starts with OAMADDR at 8 or more (the corruption on OAMADDR writes is not; see the OAMADDR write)
	return (type() == PPU_2C02) && (m_revision >= REV_2C02E);
}

bool ppu2c0x_device::pal_oam_refresh_line() const
{
	return (type() == PPU_2C07) && (m_scanline >= 265) && (m_scanline <= 310);
}

// The CPU's access is made when M2 rises (NESdev "CPU pinout": M2 is high for 15/24 of a cycle on the 2A03E/G/H, 17/24
// on the original 2A03 and 19/32 on the 2A07), so the offset of the access in the CPU cycle depends on the chip.
attotime ppu2c0x_device::m2_rise_offset() const
{
	if (const rp2a03_core_device *const cpu = dynamic_cast<const rp2a03_core_device *>(m_cpu.target()))
		return cpu->m2_rise_offset();
	return attotime::from_ticks(9, m_cpu->clock() * 24);
}

// From the access to the point where it ends for the PPU. On the 2A03G that is 7 master clocks after M2 rises (see
// sprite_flags_at_m2_fall()): half a master clock before M2 falls, 7.5 master clocks after it rose. The other chips are
// given the same half master clock before their own M2 fall: 8 master clocks on the original 2A03, 9 on the 2A07.
attotime ppu2c0x_device::access_end_offset() const
{
	const rp2a03_core_device *const cpu = dynamic_cast<const rp2a03_core_device *>(m_cpu.target());
	const u32 master = cpu_master_clocks();
	if (!cpu || ((cpu->m2_low() == 9) && (cpu->m2_period() == 24) && (master == 12)))
		return attotime::from_ticks(7, m_cpu->clock() * 12);
	const attotime end = attotime::from_ticks(cpu->m2_period() - cpu->m2_low(), m_cpu->clock() * cpu->m2_period())
			- attotime::from_ticks(1, m_cpu->clock() * master * 2);
	// a write made after the M2 rise (see rp2a03_core_device::set_m2_write_point) ends at the same point of the cycle
	if (cpu->m2_state() && cpu->m2_is_write() && (cpu->m2_write_offset() > cpu->m2_rise_offset()))
		return end - (cpu->m2_write_offset() - cpu->m2_rise_offset());
	return end;
}

u32 ppu2c0x_device::cpu_master_clocks() const
{
	return (type() == PPU_2C07) ? 16 : (type() == PPU_PALC) ? 15 : 12;
}

u32 ppu2c0x_device::dot_master_clocks() const
{
	return ((type() == PPU_2C07) || (type() == PPU_PALC)) ? 5 : 4;
}

bool ppu2c0x_device::vbl_flag_at(const attotime &when) const
{
	// set from the start of the set pulse, unless a $2002 read blocked it, until it is next cleared
	if ((m_vbl_set_at == attotime::never) || m_vbl_set_blocked || !(m_vbl_set_at < when))
		return false;
	return !((m_vbl_clr_at != attotime::never) && (m_vbl_clr_at > m_vbl_set_at) && (m_vbl_clr_at <= when));
}

bool ppu2c0x_device::nmi_asserted_at(const attotime &when) const
{
	// /NMI is low while the VBL flag and the NMI enable are both set, 3 master clocks behind them: in the Visual 2C02
	// chart of $2002 reads around the set (NESdev, User:Ulfalizer) a CPU whose phi2 falls 3.5 master clocks after the set
	// pulse begins sees /NMI low, 2.5 clocks after it does not.  For /NMI, vblank ends when the pre-render line starts,
	// one PPU clock before the readable flag clears on its dot 1: blargg's ppu_vbl_nmi 07-nmi_on_timing (hardware
	// results) needs /NMI high 1 to 5 master clocks before the flag clears; NESdev does not describe the gating
	const attotime delay = nmi_output_delay();
	if (when < delay)
		return false;
	const attotime at = when - delay;
	if ((m_vbl_nmi_end_at != attotime::never) && (at >= m_vbl_nmi_end_at))
		return false;
	const bool enabled = (at >= m_nmi_en_at) ? ((m_regs[PPU_CONTROL0] & PPU_CONTROL0_NMI) != 0) : m_nmi_en_prev;
	return enabled && vbl_flag_at(at);
}

void ppu2c0x_device::update_nmi_line()
{
	// the PPU drives /NMI low while the VBL flag and the NMI enable bit are both set; the CPU detects the edge
	const bool want = m_own_timing ? nmi_asserted_at(machine().time()) : (m_nmi_vbl && (m_regs[PPU_CONTROL0] & PPU_CONTROL0_NMI));
	if (want != m_nmi_line)
	{
		m_nmi_line = want;
		m_int_callback(want ? ASSERT_LINE : CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(ppu2c0x_device::vbl_tick)
{
	if (param == 1)
	{
		if (m_own_timing)
		{
			// /NMI follows the flag's own timing (nmi_asserted_at)
			m_vbl_set_pending = false;
			if (!m_vbl_set_blocked)
			{
				m_regs[PPU_STATUS] |= PPU_STATUS_VBLANK;
				nmi_line_update_due(m_vbl_set_at);
			}
			return;
		}

		// NESdev PPU frame timing: a $2002 read one PPU clock before this point reads the flag clear and
		// suppresses both the flag and the NMI for this frame
		m_vbl_set_pending = false;
		if (m_vbl_suppress)
		{
			m_vbl_suppress = false;
			return;
		}
		m_regs[PPU_STATUS] |= PPU_STATUS_VBLANK;
		m_nmi_vbl = true;

		// without its own timing the PPU drives /NMI 2 dots after the flag, once a $2002 read on this clock or the next
		// (which suppresses the NMI) has had its chance
		m_nmi_timer->adjust(dot_time(2));
	}
	else if (param == 0)
	{
		if (m_own_timing && (m_regs[PPU_STATUS] & PPU_STATUS_VBLANK))
			m_vbl_clr_at = m_vbl_event_time;
		m_regs[PPU_STATUS] &= ~(PPU_STATUS_VBLANK | PPU_STATUS_SPRITE0_HIT | PPU_STATUS_8SPRITES);
		m_s0hit_pending = false;
		update_nmi_line();

		// NTSC: on odd frames with rendering enabled, the pre-render line is one PPU clock shorter (dot 339 is skipped)
		if (m_own_timing && (type() == PPU_2C02))
		{
			m_vbl_event_time = m_line_start + m_dot_period * 338;
			arm_event(m_vbl_timer, m_vbl_event_time, 2);
		}
	}
	else
	{
		// the odd-frame skip follows PPUMASK as written, without the delay of the internal rendering enable: NESdev does
		// not say which signal gates it, and blargg's ppu_vbl_nmi 10-even_odd_timing (hardware results) shows this one
		const bool rendering = (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) != 0;
		if (m_odd_frame && rendering)
		{
			m_idle_dot_skipped = true;
			m_next_line_start -= m_dot_period;
			arm_event(m_scanline_timer, m_next_line_start);
		}
	}
}

int ppu2c0x_device::line_dot_at(const attotime &when)
{
	// exact integer division of the elapsed master clocks on the shared grid
	const attotime elapsed = when - m_line_start;
	if (elapsed < attotime::zero)
		return 0;
	const u64 dots = elapsed.as_ticks(m_cpu->clock() * cpu_master_clocks()) / dot_master_clocks();
	return int(std::min<u64>(dots, 341));
}

int ppu2c0x_device::current_line_dot()
{
	if (m_own_timing)
		return line_dot_at(machine().time());

	const double period = screen().scan_period().as_double() * m_scan_scale;
	const double remaining = m_scanline_timer->remaining().as_double();
	if (period <= 0.0 || remaining > period)
		return 0;
	return std::clamp(int((1.0 - remaining / period) * 341.0), 0, 341);
}

int ppu2c0x_device::hblank_dot()
{
	// NESdev MMC3: with 8x8 sprites, BG at $0000 and sprites at $1000 -> A12 rises at dot 260;
	// BG at $1000 and sprites at $0000 -> dot 324 (counted for the next line); both at $0000 -> never
	if (m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE)
		return 260;
	const bool bg_hi = m_regs[PPU_CONTROL0] & PPU_CONTROL0_CHR_SELECT;
	const bool spr_hi = m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPR_SELECT;
	if (!bg_hi && spr_hi)
		return 260;
	if (bg_hi && !spr_hi)
		return 324;
	// both at $1000: A12 is only low for the nametable and attribute fetches, too short for the filter, so no rise is
	// counted, as with both at $0000. A board that takes its scanline clock from this timer gets none; a board that
	// counts the PPU's own A12 edges still needs the PPU brought up to date here, and gets no clock from it.
	if (bg_hi && spr_hi)
		return m_a12_callback_proc.isnull() ? -1 : 260;
	return -1;
}

// PPU open bus (blargg ppu_open_bus): the I/O latch is a "decay register"; writes load it, reads refresh only the bits
// the register actually drives, and a bit that is not refreshed with a 1 for about 600 ms decays to 0
// PPU address bus: MMC3-style counters count rising edges of A12 that follow a long enough low period
// (NESdev MMC3: "a rising edge after the line has remained low for three falling edges of M2")
attotime ppu2c0x_device::dot_when(int dot)
{
	return m_own_timing ? (m_line_start + m_dot_period * dot) : machine().time();
}

void ppu2c0x_device::ppu_bus(u16 addr, attotime when)
{
	if (!m_addr_bus_proc.isnull() && BIT(addr, 12, 2) != m_bus_a13a12)
	{
		m_bus_a13a12 = BIT(addr, 12, 2);
		m_addr_bus_proc(addr);
	}
	const bool a12 = BIT(addr, 12);
	if (a12 == m_a12)
		return;
	m_a12 = a12;
	if (!m_a12_level_proc.isnull())
		m_a12_level_proc(a12 ? 1 : 0);
	if (!a12)
	{
		m_a12_low_since = when;
		if (m_a12_armed)
		{
			m_a12_armed = false;
			a12_report(when);
		}
		return;
	}
	// a board can supply its own rule (MMC3 counts M2 falling edges); otherwise A12 has to have been low for 10 PPU
	// clocks. While rendering, A12 is low for 4 clocks between two pattern fetches from $1000, also from the unused
	// nametable fetches (dots 337-340) to the next line's first tile, as the idle dot carries a pattern address.
	const bool low_long_enough = m_a12_filter_proc.isnull()
			? (when - m_a12_low_since >= m_dot_period * 10)
			: m_a12_filter_proc(m_a12_low_since, when);
	if (!m_a12_callback_proc.isnull() && low_long_enough)
	{
		if (m_a12_on_fall)
			m_a12_armed = true;
		else
			a12_report(when);
	}
}

void ppu2c0x_device::a12_report(attotime when)
{
	// With the CPU sampling IRQ itself (nmi_sampled: the 2A03 samples both interrupt inputs when phi2 falls), the IRQ an
	// A12 edge causes reaches the CPU 2 master clocks after the start of the fetch dot the edge is placed on, and an edge
	// the PPU finds after that is reported with that time. NESdev does not document the mapper's output timing or where
	// in the fetch A12 rises to this precision: blargg's 4-scanline_timing (hardware results) and the rest of the three
	// MMC3 test suites pass with 1 to 4 master clocks (0 is a PPU clock early, 5 one late); 2, half a PPU
	// clock, is used.
	// Otherwise the IRQ is delivered 4 PPU clocks late, which stands in for that sampling.
	if (m_nmi_sampled)
		when += m_dot_period / 2;
	const attotime due = m_nmi_sampled ? when : (when + m_dot_period * 4);
	const attotime now = machine().time();
	if (due > now)
		m_a12_timer->adjust(due - now);
	else
	{
		// the CPU's input line change is queued until the PPU returns, so the time is kept until the CPU takes it
		if (m_nmi_sampled && m_irq_assert_time)
			m_irq_assert_time(when);
		m_a12_callback_proc();
	}
}

TIMER_CALLBACK_MEMBER(ppu2c0x_device::a12_tick)
{
	m_a12_callback_proc();
}

// with the background at $1000 the first counted rise of a line can be its first pattern fetch (dot 5),
// so the PPU is brought up to date there as well
TIMER_CALLBACK_MEMBER(ppu2c0x_device::a12_catchup_tick)
{
	dot_run_to(current_line_dot());
}

int ppu2c0x_device::sprite_fetch_slot() const
{
	if (!m_own_timing || !use_dot_renderer() || !rendering_now())
		return -1;
	const int dot = m_line_dot - 1;   // dot_run_to steps dot m_line_dot - 1 while the cartridge is called
	return ((dot >= 257) && (dot <= 320) && (m_scanline <= BOTTOM_VISIBLE_SCANLINE || m_scanline == m_scanlines_per_frame - 1)) ? ((dot - 257) >> 3) : -1;
}

int ppu2c0x_device::sprite_fetch_oam_index() const
{
	const int slot = sprite_fetch_slot();
	return ((slot < 0) || (m_sec_oam_src[slot] == 0xff)) ? -1 : m_sec_oam_src[slot];
}

bool ppu2c0x_device::rendering_now() const
{
	return m_render_eff
			&& ((m_scanline <= BOTTOM_VISIBLE_SCANLINE) || (m_scanline == m_scanlines_per_frame - 1));
}

// with rendering off, the address bus shows the VRAM address
void ppu2c0x_device::ppu_bus_idle()
{
	if (use_dot_renderer() && !rendering_now())
		ppu_bus(m_refresh_data & 0x3fff, machine().time());
}

// NESdev PPU power up state (blargg, NTSC): after a reset an internal reset signal clears PPUCTRL, PPUMASK, PPUSCROLL,
// PPUADDR, their shared write latch and the PPUDATA read buffer; writes to PPUCTRL, PPUMASK, PPUSCROLL and PPUADDR are
// ignored for ~29658 CPU clocks (~33132 on PAL); the PPU comes out of reset at the top of the picture and the VBL flag
// is unchanged. On the Famicom the reset button does not reach the PPU.
// At power-on the same holds where the CPU and the PPU leave reset together (NES-001: "writes are ignored until ~33132
// CPU clocks after power and reset" on a PAL NES, and Magic John, which enables NMI 9217 cycles after power-on, does
// not boot on an NTSC NES). On the Famicom the PPU starts about a frame before the CPU, so its writes are accepted
// right away.
void ppu2c0x_device::device_reset()
{
	if (!m_own_timing)
		return;
	if (!m_powered)
	{
		m_powered = true;
		m_warmup_until = m_power_warmup
				? (machine().time() + m_cpu->cycles_to_attotime((type() == PPU_2C07) ? 33132 : 29658))
				: attotime::zero;
		return;
	}
	if (!m_console_reset)
		return;

	m_regs[PPU_CONTROL0] = 0;
	m_regs[PPU_CONTROL1] = 0;
	m_out_mask = m_out_mask_next = m_out_mask_delay = 0;
	m_render_eff = m_render_pending = false;
	m_render_delay = 0;
	m_rdbuf_delay = 0;
	// PPUDATA / PPUADDR access machines idle
	m_r_sr = m_r_req = m_pd_rb = m_r_ale = false;
	m_w_sr = m_w_req = m_w_ale = m_db_par = m_tstep_latch = false;
	std::fill(std::begin(m_rl), std::end(m_rl), false);
	std::fill(std::begin(m_wl), std::end(m_wl), false);
	m_r_start = m_w_start = attotime::never;
	m_2006_delay = 0;
	m_copyv = false;
	m_tile_page = 0;
	m_sprite_page = 0;
	m_add = 1;
	m_refresh_latch = 0;
	m_x_fine = 0;
	m_toggle = false;
	m_buffered_data = 0;
	m_odd_frame = false;   // NESdev "PPU power up state": odd frame "no" after a reset as at power
	m_warmup_until = machine().time() + m_cpu->cycles_to_attotime((type() == PPU_2C07) ? 33132 : 29658);

	m_vbl_timer->adjust(attotime::never);
	m_nmi_timer->adjust(attotime::never);
	m_hblank_timer->adjust(attotime::never);
	m_nmi_vbl = false;
	update_nmi_line();
	m_vbl_set_at = attotime::never;
	m_vbl_set_blocked = false;
	m_vbl_clr_at = attotime::never;
	m_nmi_en_at = attotime::zero;
	m_nmi_en_prev = false;
	m_vbl_nmi_end_at = attotime::never;
	m_nmi_update_last = attotime::zero;
	m_scanline = 0;
	m_line_dot = 0;
	// A reset restarts the PPU's counters, but its dots stay on the master clock grid it shares with the CPU: the next
	// line starts on a dot edge of the grid set up at power-on, so the CPU/PPU alignment survives a reset (as on
	// the console). Starting it at the reset's own instant, which is usually mid CPU cycle, picked a new alignment.
	{
		const u32 master_hz = m_cpu->clock() * cpu_master_clocks();
		const u32 dot_clocks = dot_master_clocks();
		const u64 align = m_cpu_ppu_align % dot_clocks;
		const attotime now = machine().time();
		const u64 now_ticks = now.as_ticks(master_hz) + align;
		// the last dot edge at or before the reset, so the restarted line never starts in the future
		u64 edge = (now_ticks / dot_clocks) * dot_clocks;
		while ((edge >= dot_clocks) && (attotime::from_ticks(edge, master_hz) - attotime::from_ticks(align, master_hz) > now))
			edge -= dot_clocks;
		m_next_line_start = attotime::from_ticks(edge, master_hz) - attotime::from_ticks(align, master_hz);
	}
	schedule_next_line();
}

// NESdev PPU scrolling: PPUDATA uses the same v register as rendering. Outside of rendering an access adds 1 or 32 to v;
// during rendering (visible and pre-render lines with background or sprites enabled) it triggers a coarse X increment and
// a Y increment at the same time. MAME's separate $2007 address is kept equal to v's address bits.
void ppu2c0x_device::data_port_increment()
{
	if (rendering_now())
	{
		dot_increment_x();
		scanline_increment_fine_ycounter();
	}
	else
		m_refresh_data = (m_refresh_data + m_add) & 0x7fff;
	m_videomem_addr = m_refresh_data & m_videoram_addr_mask;
}

u8 ppu2c0x_device::io_latch()
{
	const attotime now = machine().time();
	for (int b = 0; b < 8; b++)
		if (BIT(m_data_latch, b) && (now - m_decay_refresh[b] >= attotime::from_msec(600)))
			m_data_latch &= ~(1 << b);
	return m_data_latch;
}

void ppu2c0x_device::io_latch_refresh(u8 value, u8 mask)
{
	const attotime now = machine().time();
	m_data_latch = (m_data_latch & ~mask) | (value & mask);
	for (int b = 0; b < 8; b++)
		if (BIT(mask, b))
			m_decay_refresh[b] = now;
}

u8 ppu2c0x_device::dot_oam_bus(int dot)
{
	// dots 1-64: secondary OAM clear, reads return $FF
	if (dot >= 1 && dot <= 64)
		return 0xff;

	// dots 257-320: sprite fetches, 8 dots per sprite: Y, tile, attribute, then X four times, from secondary OAM
	if (dot >= 257 && dot <= 320)
	{
		const int slot = (dot - 257) / 8;
		const int cyc = (dot - 257) % 8;
		return m_sec_oam[slot * 4 + std::min(cyc, 3)];
	}

	// dots 321-340 and 0: the first byte of secondary OAM
	if (dot >= 321 || dot == 0)
		return m_sec_oam[0];

	// dots 65-256: sprite evaluation reads primary OAM on odd dots; replay the scan for the next line up to this dot
	const int next = (m_scanline == m_scanlines_per_frame - 1) ? 0 : m_scanline + 1;
	const int size = (m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE) ? 16 : 8;
	int t = 65, n = 0, found = 0;
	while (n < 64)
	{
		const u8 y = m_spriteram[n * 4];
		const int row = next - (y + 1);
		const bool in_range = (row >= 0) && (row < size) && (found < 8);
		const int cost = in_range ? 8 : 2;
		if (dot < t + cost)
		{
			const int byte = in_range ? std::min((dot - t) / 2, 3) : 0;
			const u8 data = m_spriteram[n * 4 + byte];
			return (byte == 2) ? (data & oam_attr_mask()) : data;
		}
		t += cost;
		if (in_range)
			found++;
		n++;
	}
	return m_spriteram[0];
}

void ppu2c0x_device::dot_run_to(int target)
{
	if (!use_dot_renderer())   // the scanline renderer draws whole lines; only the dot renderer catches up dot by dot
		return;
	while (m_line_dot < target)
		dot_step(m_line_dot++);
}

void ppu2c0x_device::dot_reload_shifters()
{
	m_bg_shift_lo = (m_bg_shift_lo & 0xff00) | m_bg_next_lo;
	m_bg_shift_hi = (m_bg_shift_hi & 0xff00) | m_bg_next_hi;
	m_bg_shift_x0 = (m_bg_shift_x0 & 0xff00) | m_bg_next_x0;
	m_bg_shift_x1 = (m_bg_shift_x1 & 0xff00) | m_bg_next_x1;
	m_at_shift_lo = (m_at_shift_lo & 0xff00) | ((m_bg_next_at & 1) ? 0xff : 0x00);
	m_at_shift_hi = (m_at_shift_hi & 0xff00) | ((m_bg_next_at & 2) ? 0xff : 0x00);
	m_at_shift_x0 = (m_at_shift_x0 & 0xff00) | ((m_bg_next_at & 4) ? 0xff : 0x00);
	m_at_shift_x1 = (m_at_shift_x1 & 0xff00) | ((m_bg_next_at & 8) ? 0xff : 0x00);
	m_at_latch = m_bg_next_at & 0x0f;
}

void ppu2c0x_device::bus_ale(u16 addr)
{
	if (!m_ale_cb.isnull())
		m_ale_cb(addr & 0x3fff);
	if (!use_data_pipeline())
	{
		m_bus_addr = addr & 0x3fff;
		m_octal = u8(addr);
		return;
	}
	// TriCNES: with a read on the same dot (a PPUDATA read strobe) the octal latch is fed back from the bus first and
	// does not take the new address afterwards (AccuracyCoin "ALE + Read")
	const bool read = m_bus_read;
	if (m_bus_ale && read)
		m_octal = u8(m_bus_addr);
	m_bus_addr = addr & 0x3fff;
	if (m_bus_ale && !read)
		m_octal = u8(m_bus_addr);
}

// the read dot: high address bits as they are now, low byte from the octal latch
u8 ppu2c0x_device::bus_read(u16 addr_now)
{
	if (use_data_pipeline() && m_bus_ale)
		m_octal = u8(m_bus_addr);
	m_bus_addr = (addr_now & 0x3f00) | m_octal;
	m_fetch_addr = m_bus_addr;
	if (!m_rd_cb.isnull())
		m_rd_cb(m_bus_addr);
	const u8 data = readbyte(m_bus_addr);
	m_bus_addr = (m_bus_addr & 0xff00) | data;   // the data comes back on the low bus lines
	return data;
}

// TriCNES FetchVideoMemory: the memory sees the bus's high bits and the octal latch; the data read comes back on the
// low bus lines. The palette is not read here (palette reads go straight to the CPU): below it lies the nametable mirror.
u8 ppu2c0x_device::bus_fetch()
{
	u16 addr = (m_bus_addr & 0x3f00) | m_octal;
	if (addr >= 0x3f00)
		addr &= 0x2fff;
	if (!m_latch.isnull() && m_read_latch.isnull())
		m_latch(addr);
	if (!m_rd_cb.isnull())
		m_rd_cb(addr);
	const u8 data = readbyte(addr);
	if (!m_read_latch.isnull())
		m_read_latch(addr);
	m_bus_addr = (m_bus_addr & 0xff00) | data;
	return data;
}

// TriCNES WriteVideoMemory: palette RAM (only while blanking) from the full bus address, VRAM through the octal latch
void ppu2c0x_device::bus_write_vram(u8 data)
{
	if (m_pal_en)
		writebyte(0x3f00 | (m_bus_addr & 0x1f), data);
	else
	{
		u16 addr = (m_bus_addr & 0x3f00) | m_octal;
		if (addr >= 0x3f00)
			addr &= 0x2fff;
		if (!m_wr_cb.isnull())
			m_wr_cb(addr, data);
		writebyte(addr, data);
	}
}

// make sure the PPU is run up to where a PPUADDR/PPUDATA access takes effect
void ppu2c0x_device::pipe_catchup(int dots)
{
	const attotime due = machine().time() + m_dot_period * dots;
	if (!m_pipe_timer->enabled() || (m_pipe_timer->expire() > due))
		m_pipe_timer->adjust(m_dot_period * dots);
}

TIMER_CALLBACK_MEMBER(ppu2c0x_device::pipe_tick)
{
	dot_run_to(current_line_dot());
	// still something under way (e.g. a write's second half): look again shortly
	if (m_2006_delay || m_r_sr || m_w_sr || (m_r_start != attotime::never) || (m_w_start != attotime::never)
			|| m_rl[0] || m_rl[2] || m_wl[0] || m_wl[2] || m_pd_rb || m_db_par || m_tstep_latch)
		m_pipe_timer->adjust(m_dot_period);
}

// Y increment of a PPUDATA access during rendering (TriCNES PPU_IncrementScrollY, including the $2006 collision)
void ppu2c0x_device::data_increment_y()
{
	if (m_copyv)
	{
		m_refresh_data = m_2006_old & m_2006_new;
		return;
	}
	scanline_increment_fine_ycounter();
}

// full-dot part, before rendering's own fetch (TriCNES PPU_DATA_StateMachine)
void ppu2c0x_device::data_sm_full(int dot)
{
	const attotime when = dot_when(dot);
	if ((m_r_start != attotime::never) && (when > m_r_start)) { m_r_sr = m_r_req = true; m_r_start = attotime::never; }
	if ((m_w_start != attotime::never) && (when > m_w_start)) { m_w_sr = m_w_req = true; m_w_start = attotime::never; }

	// a $2006 second write reaches v (and the bus) here
	m_copyv = false;
	if (m_2006_delay && (--m_2006_delay == 0))
	{
		m_2006_old = m_refresh_data;
		m_refresh_data = m_2006_new;
		m_bus_addr = m_refresh_data & 0x3fff;
		m_copyv = true;
		if (!rendering_now())
			ppu_bus(m_bus_addr, when);
	}

	const bool blnk = !m_render_eff || ((m_scanline >= m_vblank_first_scanline) && (m_scanline < m_scanlines_per_frame - 1));
	m_blnk_latch = blnk;
	const bool h0_dash = ((dot - 1) & 1) != 0;
	m_pal_en = ((m_bus_addr & 0x3f00) == 0x3f00) && m_blnk_latch;

	m_rl[0] = m_r_sr;
	m_r_req = false;
	m_rl[2] = !m_rl[1];
	m_rl[4] = !m_rl[3];
	m_pd_rb = m_rl[4] && !m_rl[2];
	m_r_ale = !m_rl[4] && m_rl[2];
	m_bus_read = m_pd_rb || (!blnk && h0_dash);

	m_wl[0] = m_w_sr;
	m_w_req = false;
	m_wl[2] = !m_wl[1];
	m_wl[4] = !m_wl[3];
	m_w_ale = !m_wl[4] && m_wl[2];
	m_tstep_latch = m_db_par;

	m_bus_ale = m_r_ale || m_w_ale || (!blnk && !h0_dash);
	if ((m_r_ale || m_w_ale) && !m_bus_read)
	{
		m_bus_addr = m_refresh_data & 0x3fff;
		m_octal = u8(m_bus_addr);
		if (!rendering_now())
			ppu_bus(m_bus_addr, when);
	}
}

// end of the full dot (TriCNES PPU_DATA_StateMachine2)
void ppu2c0x_device::data_sm_full_end()
{
	if (m_pd_rb)
	{
		m_buffered_data = bus_fetch();
		if (m_bus_ale)
			m_octal = u8(m_bus_addr);
	}
}

// half dot (TriCNES PPU_DATA_StateMachine_Half)
void ppu2c0x_device::data_sm_half(int dot)
{
	const attotime when = dot_when(dot) + m_dot_period / 2;
	if ((m_r_start != attotime::never) && (when > m_r_start)) { m_r_sr = m_r_req = true; m_r_start = attotime::never; }
	if ((m_w_start != attotime::never) && (when > m_w_start)) { m_w_sr = m_w_req = true; m_w_start = attotime::never; }

	if (m_tstep_latch || m_pd_rb)
	{
		if (!m_blnk_latch)
			data_increment_y();
		else
			m_refresh_data = (m_refresh_data + m_add) & 0x7fff;
	}
	m_bus_ale = m_r_ale || m_w_ale;
	if (m_pd_rb)
	{
		m_buffered_data = bus_fetch();
		if (m_bus_ale)
			m_octal = u8(m_bus_addr);
	}
	m_rl[1] = !m_rl[0];
	m_rl[3] = !m_rl[2];
	if (!m_rl[3])
		m_r_sr = false;
	m_wl[1] = !m_wl[0];
	m_wl[3] = !m_wl[2];
	if (!m_wl[3])
		m_w_sr = false;
	m_db_par = m_wl[1] && !m_wl[3];
	if (m_db_par)
	{
		bus_write_vram(m_w_data);
		if (m_bus_ale)
		{
			// the write's data passed through the octal latch: the write is made again with it as the low address
			m_octal = m_w_data;
			bus_write_vram(m_w_data);
		}
	}
}

// Background fetches, two dots each (TriCNES PPU_Render_ShiftRegistersAndBitPlanes): nametable on dots 1-2, attribute
// 3-4, pattern low 5-6, pattern high 7-8 of every tile. On a read dot the nametable and attribute addresses are formed
// again from v; the pattern fetch keeps its pattern address register.
void ppu2c0x_device::dot_fetch_nt(int dot)
{
	// MMC5 numbers the 34 fetched tiles of a line 0-33 in drawing order; tiles 0 and 1 are prefetched on the previous line
	m_tilecount = (dot <= 256) ? (2 + (dot - 1) / 8) : ((dot - 321) / 8);
	m_draw_phase = PPU_DRAW_BG;
	const u16 addr = dot_nt_addr();
	ppu_bus(addr, dot_when(dot));
	bus_ale(addr);
}

void ppu2c0x_device::dot_read_nt()
{
	m_bg_next_nt = bus_read(dot_nt_addr());
}

void ppu2c0x_device::dot_fetch_at()
{
	const u16 addr = dot_at_addr();
	// Attribute fetches drive the external PPU address bus just like nametable and pattern fetches.
	// Report the address phase so mapper-visible A12 activity follows the complete background fetch sequence.
	ppu_bus(addr, dot_when(m_line_dot - 1));
	bus_ale(addr);
}

void ppu2c0x_device::dot_read_at()
{
	m_bg_next_at = dot_at_decode(bus_read(dot_at_addr()));
}

void ppu2c0x_device::dot_fetch_pattern(bool high)
{
	if (!high && m_read_latch.isnull() && !m_latch.isnull())
		m_latch((m_tile_page << 10) | (m_bg_next_nt << 4));

	const u16 addr = dot_pattern_addr(high);
	// Put the complete pattern address on the external bus, including fine Y and plane.
	ppu_bus(addr, dot_when(m_line_dot - 1));
	m_pat_addr = addr;
	bus_ale(addr);
}

void ppu2c0x_device::dot_read_pattern(bool high)
{
	u8 extra;
	const u8 data = dot_bg_pattern(m_pat_addr, high, bus_read(m_pat_addr), m_bg_next_at, extra);
	if (!m_read_latch.isnull())
		m_read_latch(m_fetch_addr);
	if (high)
	{
		m_bg_next_hi = data;
		m_bg_next_x1 = extra;
	}
	else
	{
		m_bg_next_lo = data;
		m_bg_next_x0 = extra;
	}
}

void ppu2c0x_device::dot_increment_x()
{
	if ((m_refresh_data & 0x001f) == 0x001f)
	{
		m_refresh_data &= ~0x001f;
		m_refresh_data ^= 0x0400;
	}
	else
		m_refresh_data++;
}

// NESdev PPU sprite evaluation: secondary OAM is cleared on dots 1-64, then primary OAM is read on odd dots
// and the read latch is processed/written on even dots 65-256.  Once eight sprites have been copied, writes are
// disabled and the 2C02's diagonal n/m overflow scan continues.
void ppu2c0x_device::dot_sprite_evaluation_step(int dot)
{
	// on the pre-render line evaluation only reads: secondary OAM is neither cleared nor written, so the sprite fetch
	// that follows loads line 0's sprites from what line 239 left there (TriCNES; AccuracyCoin "Sprites On Scanline 0")
	if (m_scanline == m_scanlines_per_frame - 1)
		return;
	if (dot >= 1 && dot <= 64)
	{
		if (!(dot & 1))
			m_sec_oam[(dot >> 1) - 1] = 0xff;
		return;
	}
	if (dot < 65 || dot > 256 || m_eval_done)
		return;

	if (dot & 1)
	{
		m_eval_latch = m_spriteram[((m_eval_n << 2) | m_eval_m) & 0xff];
		if (m_eval_m == 2)
			m_eval_latch &= oam_attr_mask();
		return;
	}

	const int size = (m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE) ? 16 : 8;
	// The pre-render line prepares the pipeline for line 0, where sprites cannot be displayed.  Keep every
	// primary-OAM value out of range there; otherwise Y=$ff wraps into a visible line-0 sprite and can hit.
	const bool in_range = (m_scanline <= BOTTOM_VISIBLE_SCANLINE)
			&& (m_scanline - m_eval_latch) >= 0 && (m_scanline - m_eval_latch) < size;
	if (m_eval_found < 8)
	{
		if (m_eval_sec < 32)
			m_sec_oam[m_eval_sec++] = m_eval_latch;
		const bool first_byte = (m_eval_found == 0) && (m_eval_sec == 1)
				&& (m_eval_n == (m_eval_start >> 2)) && (m_eval_m == (m_eval_start & 3));
		if (!m_eval_copying && ((m_eval_m == 0) || first_byte))
		{
			if (in_range)
			{
				m_eval_copying = true;
				if (m_eval_n == (m_eval_start >> 2))
					m_eval_sprite0_mask |= 1U << m_eval_found;
				m_eval_m = (m_eval_m + 1) & 3;
				if (m_eval_m == 0) m_eval_n = (m_eval_n + 1) & 0x3f;
			}
			else
			{
				m_eval_sec--; // an out-of-range Y is tested but not retained in secondary OAM
				m_eval_n = (m_eval_n + 1) & 0x3f;
				m_eval_m = 0;
				if (m_eval_n == 0) m_eval_done = true;
			}
		}
		else if (m_eval_copying)
		{
			m_eval_m = (m_eval_m + 1) & 3;
			if (m_eval_m == 0) m_eval_n = (m_eval_n + 1) & 0x3f;
			if ((m_eval_sec & 3) == 0)
			{
				m_eval_copying = false;
				m_eval_found++;
				if (m_eval_n == 0) m_eval_done = true;
			}
		}
	}
	else
	{
		// After secondary OAM is full, an in-range byte advances m normally (carrying into n); a miss
		// advances n and m without carry, producing the 2C02 diagonal overflow scan.
		if (in_range)
		{
			// the overflow flag is set by this even-dot range check of the byte read on the odd dot before it
			// (TriCNES; AccuracyCoin "$2002 flag timing" test 2: overflow becomes visible after a same-line sprite 0 hit)
			m_regs[PPU_STATUS] |= PPU_STATUS_8SPRITES;
			m_eval_m = (m_eval_m + 1) & 3;
			if (m_eval_m == 0)
				m_eval_n = (m_eval_n + 1) & 0x3f;
		}
		else
		{
			m_eval_m = (m_eval_m + 1) & 3;
			m_eval_n = (m_eval_n + 1) & 0x3f;
		}
		if (m_eval_n == 0) m_eval_done = true;
	}
}

void ppu2c0x_device::oam2_increment()
{
	if (!m_oam2_ovf && (++m_oam2_addr == 0x20))
	{
		m_oam2_ovf = true;     // wrapped: frozen at 0 until the next reset
		m_oam2_addr = 0;
	}
}

// One dot of the 2C02's OAM side on a visible or the pre-render line (after TriCNES): odd dots read into the OAM buffer,
// even dots act on what the buffer latched the dot before. On the pre-render line it only reads.
void ppu2c0x_device::dot_oam_step(int dot, bool rendering)
{
	if (!rendering)
		return;
	const bool prerender = (m_scanline == m_scanlines_per_frame - 1);
	u8 &oamaddr = m_regs[PPU_SPRITE_ADDRESS];
	const u8 prev = m_oam_buf;
	u8 next = prev;
	const int size = (m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE) ? 16 : 8;

	if (dot == 63 || dot == 255 || dot == 339)
		m_oam2_reset = 2;

	if (dot >= 1 && dot <= 64)
	{
		// clear secondary OAM to $FF (the pre-render line only reads it)
		if (dot & 1)
		{
			next = prerender ? m_sec_oam[m_oam2_addr] : 0xff;
			if (dot == 1)
			{
				if (!prerender)
					std::fill(std::begin(m_sec_oam_src), std::end(m_sec_oam_src), 0xff);
				m_oam2_addr = 0;
				m_eval_tick = 0;
				m_eval_addr_ovf = false;
			}
		}
		else
		{
			if (!prerender)
				m_sec_oam[m_oam2_addr] = prev;
			oam2_increment();
		}
	}
	else if (dot >= 65 && dot <= 256)
	{
		if (dot == 65)
		{
			m_eval_nine = false;
			m_eval_addr_ovf = false;
			m_eval_found = 0;
			m_eval_start = oamaddr;
		}
		// NESdev forum "Sprite rendering research" (Fiskbit, from the Breaks circuit diagrams): /VIS is deasserted for the
		// whole pre-render line, and "OAM2ADDR is selected during rendering whenever /VIS is not asserted. So, the PPU
		// doesn't touch OAM1 at all on pre-render": the read dots of evaluation read secondary OAM there
		if (dot & 1)
			next = prerender ? m_sec_oam[m_oam2_addr] : m_spriteram[oamaddr];
		else if (!m_eval_addr_ovf)
		{
			const u8 pre = oamaddr;
			if (!m_oam2_ovf && !prerender)
				m_sec_oam[m_oam2_addr] = prev;
			const u8 oam2read = m_sec_oam[m_oam2_addr];
			if (m_eval_tick == 0)
			{
				// Y: in range of this line?
				const int voff = (m_scanline & 0xff) - prev;
				if (!m_eval_nine && !prerender && (voff >= 0) && (voff < size))
				{
					if (!m_oam2_ovf)
					{
						m_sec_oam_src[(m_oam2_addr >> 2) & 7] = pre >> 2;   // for sprite_fetch_oam_index()
						oamaddr++;
						oam2_increment();
						m_eval_found++;
						// "sprite 0" is whatever object is in range on dot 66, wherever OAMADDR started
						if (dot == 66)
							m_eval_sprite0_mask = 1;
					}
					else
					{
						// a ninth object: sprite overflow (set by this even-dot check)
						m_eval_nine = true;
						m_regs[PPU_STATUS] |= PPU_STATUS_8SPRITES;
						oamaddr++;
					}
					m_eval_tick++;
				}
				else
				{
					if (dot == 66)
						m_eval_sprite0_mask = 0;
					if (!prerender)
					{
						if (m_oam2_ovf && !m_eval_nine)
							oamaddr += ((oamaddr & 3) == 3) ? 1 : 5;   // the 2C02 sprite overflow bug
						else
							oamaddr = (oamaddr + 4) & 0xfc;
					}
				}
			}
			else
			{
				if (m_eval_tick == 3)
				{
					// X: the range comparator runs again; an object whose 4th byte is not "in range" realigns OAMADDR
					const int voff = m_scanline - prev;
					if ((voff >= 0) && (voff < size))
					{
						if (!prerender)
							oamaddr += m_oam2_ovf ? 4 : 1;
					}
					else if (!prerender || m_oam2_ovf)
						oamaddr = (oamaddr + 1) & 0xfc;
				}
				else if (!prerender)
					oamaddr++;
				m_eval_tick = (m_eval_tick + 1) & 3;
				if (!prerender)
					oam2_increment();
			}
			if ((oamaddr < pre) && (oamaddr < 4))
				m_eval_addr_ovf = true;   // OAMADDR wrapped: evaluation stops writing secondary OAM
			next = oam2read;              // a full secondary OAM is read instead of written
		}
		else
		{
			if (!prerender)
				oamaddr = (oamaddr + 4) & 0xfc;
			next = m_sec_oam[m_oam2_addr];
		}
	}
	else if (dot >= 257 && dot <= 320)
	{
		// sprite fetch: each slot's Y, tile, attribute and X pass through the buffer from the secondary OAM address
		oamaddr = 0;
		next = m_sec_oam[m_oam2_addr];
		switch ((dot - 257) & 7)
		{
		case 0: oam2_increment(); break;
		case 1: m_fetch_y = prev; oam2_increment(); break;
		case 2: m_fetch_tile = prev; oam2_increment(); break;
		case 3: m_fetch_attr = prev; break;
		case 4: m_fetch_x = prev; break;
		case 7: oam2_increment(); break;
		default: break;
		}
	}
	else if (dot >= 321)
		next = m_sec_oam[m_oam2_addr];

	m_oam_buf = next;
}

void ppu2c0x_device::dot_evaluate_sprites()
{
	// the sprite fetches of dots 257-320 read each slot from secondary OAM themselves (dot_fetch_sprite_slot)
	m_spr_count = std::min<u8>(m_eval_found, 8);
	m_spr_zero_in_line = BIT(m_eval_sprite0_mask, 0);
}

// first pattern fetch of a sprite slot: the slot's Y, tile, attribute and X come from secondary OAM, and the range check
// uses the line number's low 8 bits and the sprite size in effect now (the pre-render line checks as line 5); an
// out-of-range slot - every empty slot, whose Y is $FF - loads a blank pattern (TriCNES)
void ppu2c0x_device::dot_fetch_sprite_slot(int slot)
{
	const int size = (m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE) ? 16 : 8;
	const u8 y = m_fetch_y;
	const u8 tile = m_fetch_tile;
	const u8 attr = m_fetch_attr;
	const int row = (m_scanline & 0xff) - y;   // not wrapped to 8 bits: Y = $FF is never in range
	m_spr_in_range[slot] = (row >= 0) && (row < size);
	int line = (attr & 0x80) ? (size - 1 - row) : row;
	line &= size - 1;
	int base;
	if (size == 16)
	{
		base = ((tile & 1) ? 0x1000 : 0) + ((tile & 0xfe) * 16);
		if (line > 7) { base += 16; line -= 8; }
	}
	else
		base = (m_sprite_page ? 0x1000 : 0) + (tile * 16);
	m_spr_addr[slot] = u16((base + line) & 0x1fff);
	m_spr_x[slot] = m_fetch_x;
	m_spr_attr[slot] = attr;
}

// pixel output: the pixel itself, then the 2C07's border, then (at the end of the line) the composite decode
void ppu2c0x_device::dot_output_pixel(int x, bool rendering)
{
	dot_output_pixel_inner(x, rendering);

	// 2C07: the border is always black, not the backdrop, and covers the top scanline and the left and right 2 pixels
	// of the others (NESdev "PAL video": 252 active pixels, line 0 entirely cropped; timings from the 2C07-0). The UA6538
	// (Dendy) is not given it: the page's timings are the 2C07-0's
	if ((type() == PPU_2C07) && ((m_scanline == 0) || (x < 2) || (x >= 254)))
	{
		m_line_idx[x] = 0x0f;
		m_bitmap.pix(m_scanline, x) = pen_color(0x0f);
	}

	if (m_composite && (x == 255) && ((type() == PPU_2C02) || (type() == PPU_2C07) || (type() == PPU_PALC)))
		composite_decode_line();
}

void ppu2c0x_device::dot_output_pixel_inner(int x, bool rendering)
{
	u32 *const dest = &m_bitmap.pix(m_scanline, x);

	if (!rendering)
	{
		// with rendering off the PPU shows the backdrop, or the palette entry the VRAM address points at
		u8 pal = m_back_color;
		// it is the current VRAM address v that picks the colour, not just $2007's own idea of it
		const u16 vaddr = m_refresh_data & 0x3fff;
		if (m_paletteram_in_ppuspace && (vaddr >= 0x3f00))
			pal = m_palette_ram[vaddr & 0x1f];
		m_line_pal[x] = pal;
		m_line_pal_y = m_scanline;
		m_line_idx[x] = apply_grayscale_and_emphasis(pal);
		*dest = pen_color(m_line_idx[x]);
		// "When rendering is disabled, EXT output is always index 0 regardless of backdrop override" (NESdev)
		if ((m_regs[PPU_CONTROL0] & PPU_CONTROL0_EXT_OUTPUT) && !m_ext_out_cb.isunset())
			m_ext_out_cb(x, 0);
		return;
	}

	u8 bg_pix = 0, bg_pal = 0;
	bool bg_opaque = false;
	if ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND) && ((x >= 8) || (m_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND_L8)))
	{
		const u16 bit = 0x8000 >> m_x_fine;
		bg_pix = ((m_bg_shift_lo & bit) ? 1 : 0) | ((m_bg_shift_hi & bit) ? 2 : 0)
				| ((m_bg_shift_x0 & bit) ? 0x20 : 0) | ((m_bg_shift_x1 & bit) ? 0x40 : 0);   // planes 2-3: VT03 4bpp
		bg_pal = ((m_at_shift_lo & bit) ? 1 : 0) | ((m_at_shift_hi & bit) ? 2 : 0)
				| ((m_at_shift_x0 & bit) ? 4 : 0) | ((m_at_shift_x1 & bit) ? 8 : 0);   // palette bits 2-3: SH6578
		bg_opaque = dot_bg_opaque(bg_pix, bg_pal);
	}

	u8 sp_pix = 0, sp_pal = 0;
	bool sp_behind = false, sp_zero = false;
	if ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES) && ((x >= 8) || (m_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES_L8)))
	{
		for (int i = 0; i < 8; i++)
		{
			// a unit draws while its X counter is zero, from the top bits of its shift registers
			if (m_sp_ctr[i])
				continue;
			const u8 p = BIT(m_sp_lo[i], 7) | (BIT(m_sp_hi[i], 7) << 1)
					| (dot_sprite_16px() ? 0 : ((BIT(m_sp_x0[i], 7) << 5) | (BIT(m_sp_x1[i], 7) << 6)));   // VT03 4bpp
			if (!p)
				continue;
			sp_pix = p;
			sp_pal = (m_sp_attr[i] & 0x03) + 4;
			sp_behind = BIT(m_sp_attr[i], 5);
			sp_zero = (i == 0) && m_spr_zero_in_line;
			break;
		}
	}

	// sprite 0 hit: opaque sprite 0 pixel over an opaque background pixel, never at x = 255
	if (sp_zero && bg_opaque && (x != 255) && !(m_regs[PPU_STATUS] & PPU_STATUS_SPRITE0_HIT) && !m_s0hit_pending)
	{
		if (m_own_timing)
		{
			// the flag becomes visible 1.5 dots after the pixel is decided (TriCNES's two half-dot latch stages;
			// AccuracyCoin "$2002 flag timing" test 2 at CPU/PPU alignment 0 reads neither flag on the critical sample)
			m_s0hit_pending = true;
			m_s0hit_time = dot_when(x + 1) + (m_dot_period * 3) / 2;
		}
		else
			m_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
	}

	// a PPU with a pixel format of its own (VT03 family 4bpp and extended palettes) gives the colour itself
	if (dot_pixel_rgb(x, bg_pix, bg_pal, bg_opaque, sp_pix, sp_pal, sp_pix && (!bg_opaque || !sp_behind), dest))
		return;

	u8 pal;
	u8 index;
	const bool ext_output = m_regs[PPU_CONTROL0] & PPU_CONTROL0_EXT_OUTPUT;
	if (!bg_opaque && !sp_pix)
	{
		// the backdrop: palette entry 0, or with PPUCTRL bit 6 clear the entry the EXT pins select (NESdev PPUCTRL)
		const u8 ext = (!ext_output && !m_ext_in_cb.isunset()) ? (m_ext_in_cb(x) & 0x0f) : 0;
		pal = ext ? m_palette_ram[ext] : m_back_color;
		// what EXT outputs for a transparent pixel is entry 0 of that background sliver's palette, not the backdrop
		index = (4 * bg_pal) & 0x0f;
	}
	else if (!bg_opaque || (sp_pix && !sp_behind))
	{
		index = ((4 * sp_pal) | sp_pix) & 0x1f;
		pal = m_palette_ram[index];
	}
	else
	{
		index = ((4 * bg_pal) | bg_pix) & 0x1f;
		pal = m_palette_ram[index];
	}
	// with PPUCTRL bit 6 set the low four bits of the palette index go out on EXT (sprites and background alike)
	if (ext_output && !m_ext_out_cb.isunset())
		m_ext_out_cb(x, index & 0x0f);

	m_line_pal[x] = pal;
	m_line_pal_y = m_scanline;
	m_line_idx[x] = apply_grayscale_and_emphasis(pal);
	*dest = pen_color(m_line_idx[x]);
}

void ppu2c0x_device::dot_oam_row_corruption(int dot, bool apply)
{
	// Turning rendering off during sprite evaluation or the sprite fetches corrupts an OAM row once rendering is turned
	// on again (NESdev Errata; Fiskbit: believed present on every NTSC PPU revision, PAL and RGB PPUs not tested). The
	// 2C07 is included: its OAM address logic is the 2C02's with a delay added (Breaking NES, 2C07 analysis), not
	// removed. The NESdev statement that OAMADDR works as intended on the 2C07 concerns the corruption by OAMADDR (see
	// oam_corruption()), not this one. The 2C02's row mapping is used for the 2C07 as well (VERIFY on a 2C07).
	if (type() != PPU_2C02 && type() != PPU_2C07)
		return;

	if (apply)
	{
		for (int row = 1; row < 32; row++)
			if (m_oam_corrupt_rows & (u32(1) << row))
				for (int byte = 0; byte < 8; byte++)
					m_spriteram[row * 8 + byte] = m_spriteram[byte];
		m_oam_corrupt_rows = 0;
	}
	else if (dot >= 0 && dot < 64)
		m_oam_corrupt_rows |= u32(1) << (dot >> 1);
	else if (dot >= 256 && dot < 320)
	{
		const int row = ((dot - 256) >> 3) * 4 + std::min((dot - 256) & 7, 3);
		m_oam_corrupt_rows |= u32(1) << row;
	}
}

void ppu2c0x_device::dot_step(int dot)
{
	dot_step_main(dot);
	if (use_data_pipeline())
	{
		data_sm_full_end();
		data_sm_half(dot);
	}
}

void ppu2c0x_device::dot_step_main(int dot)
{
	// a PPUMASK write changes the internal rendering-enabled state 3 dots later (written at dot N, effective at N+3;
	// NESdev PPUMASK: toggling rendering takes effect approximately 3-4 dots after the write) - this is what decides
	// fetches and scroll increments
	if (m_out_mask_delay && (--m_out_mask_delay == 0))
		m_out_mask = m_out_mask_next;
	if (m_render_delay && (--m_render_delay == 0))
	{
		if (m_render_eff != m_render_pending &&
			(m_scanline <= BOTTOM_VISIBLE_SCANLINE || m_scanline == m_scanlines_per_frame - 1))
			dot_oam_row_corruption(dot, m_render_pending);
		m_render_eff = m_render_pending;
	}

	// PPUDATA: the fetched byte enters the read buffer 5 dots after the read (MesenCE, NESdev org fork); a second read
	// before then still returns the old buffer and replaces the pending byte (blargg's double_2007_read). The byte is
	// fetched at the read, so a later CHR bank switch does not affect it (test_ppu_read_buffer #71).
	if (!use_data_pipeline() && m_rdbuf_delay && (--m_rdbuf_delay == 0))
		m_buffered_data = m_rdbuf_value;

	// sprite 0 hit: the flag decided at a pixel becomes visible 1.5 dots later (see dot_output_pixel_inner())
	if (m_s0hit_pending && (dot_when(dot) >= m_s0hit_time))
	{
		m_s0hit_pending = false;
		m_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
	}

	// OAMDATA: the write reaches OAM a few dots after the CPU write; whether rendering is on is decided when it lands
	if (m_oamw_pending && (--m_oamw_delay <= 0))
	{
		m_oamw_pending = false;
		oam_data_write_apply(m_oamw_data);
	}

	// 2C07 forced OAM refresh (NESdev "PPU OAM": it "begins refreshing OAM on scanline 265, 24 scanlines after the start of
	// NMI", so PAL OAM never decays): on scanlines 265-310 the PPU does no sprite evaluation and instead increments OAMADDR
	// every 2 dots, at dots 2, 4, ..., 340 but not 0 (lidnariq's measurements, NESdev forum t=15763, cited by the wiki)
	if (pal_oam_refresh_line() && (dot >= 2) && !(dot & 1))
		m_regs[PPU_SPRITE_ADDRESS] = (m_regs[PPU_SPRITE_ADDRESS] + 1) & 0xff;

	const bool render_prev = m_render_prev;
	m_render_prev = m_render_eff;

	// the secondary OAM address and its overflow flag reset 1.5 dots after dots 63, 255 and 339 (here: before dot N+2)
	if (m_oam2_reset && (--m_oam2_reset == 0))
	{
		m_oam2_addr = 0;
		m_oam2_ovf = false;
	}

	// Scroll updates at the very start of the dot, before anything else of it (TriCNES): the Y increment of the line on
	// dot 257, the horizontal copy from t on dot 258, the pre-render line's vertical copy on dots 281-305 - so dot 257's
	// garbage nametable fetch still sees the old coarse X (AccuracyCoin "$2007 Stress Test")
	// (TriCNES also needs rendering to have been on for a dot already: its PPU_RenderingCounter >= 1)
	if (use_data_pipeline() && m_render_eff && render_prev
			&& ((m_scanline <= BOTTOM_VISIBLE_SCANLINE) || (m_scanline == m_scanlines_per_frame - 1)))
	{
		if (dot == 257)
			data_increment_y();
		else if (dot == 258)
			m_refresh_data = (m_refresh_data & ~0x041f) | (m_refresh_latch & 0x041f);
		if ((m_scanline == m_scanlines_per_frame - 1) && (dot >= 281) && (dot <= 305))
			m_refresh_data = (m_refresh_data & ~0x7be0) | (m_refresh_latch & 0x7be0);
	}

	// with rendering off the PPU address bus simply carries v (TriCNES); the cartridge sees it change, so a PPUDATA
	// access whose increment takes v across $1000 raises A12 (blargg mmc3_test "3-A12_clocking" tests 5 and 6)
	if (use_data_pipeline() && !m_render_eff)
	{
		m_bus_addr = m_refresh_data & 0x3fff;
		ppu_bus(m_bus_addr, dot_when(dot));
	}

	// PPUDATA state machines, before rendering's own fetch (TriCNES order)
	if (use_data_pipeline())
		data_sm_full(dot);

	const bool visible = m_scanline <= BOTTOM_VISIBLE_SCANLINE;
	const bool prerender = m_scanline == m_scanlines_per_frame - 1;
	if (!visible && !prerender)
		return;

	const bool rendering = m_render_eff;

	if (prerender && dot == 0)
	{
		m_oam_start_copied = false;
		if (rendering)
			dot_oam_row_corruption(dot, true);
	}

	// NESdev "PPU rendering", cycle 0 of a visible scanline: "This is an idle cycle. The value on the PPU address bus
	// during this cycle appears to be the same CHR address that is later used to fetch the low background tile byte
	// starting at dot 5 (possibly calculated during the two unused NT fetches at the end of the previous scanline)."
	// With the background at $1000 that raises A12 for this one dot. On an odd frame the idle dot of scanline 0 is the
	// one the shortened pre-render line leaves out: the last dot of its second unused nametable fetch stands there.
	if (visible && (dot == 0) && rendering && m_own_timing && !((m_scanline == 0) && m_idle_dot_skipped))
		ppu_bus(((m_tile_page ? 0x1000 : 0) + (m_idle_nt * 16) + ((m_refresh_data >> 12) & 7)) & 0x1fff, dot_when(dot));
	if ((m_scanline == 0) && (dot == 0))
		m_idle_dot_skipped = false;

	if (rendering)
	{
		// 2C02G/H (NESdev PPU sprite evaluation, Errata): starting sprite evaluation with OAMADDR at 8 or more copies the
		// eight bytes at OAMADDR & $F8 over OAM[0-7]; it happens at the first dot of the pre-render line, or at the dot
		// rendering is turned on if that is later in the line (RC Pro-Am II relies on it)
		if (prerender && !m_oam_start_copied && dot >= 1 && dot <= 256 && oam_corruption())
		{
			m_oam_start_copied = true;
			const u8 src = m_regs[PPU_SPRITE_ADDRESS] & 0xf8;
			if (src)
				for (int b = 0; b < 8; b++)
					m_spriteram[b] = m_spriteram[src + b];
		}

		if ((dot >= 2 && dot <= 257) || (dot >= 322 && dot <= 337))
		{
			// the pattern shifters' serial inputs are tied low (plane 0) and high (plane 1): a reload skipped by
			// toggling rendering leaves 1s in the high plane (AccuracyCoin "BG Serial In")
			m_bg_shift_lo <<= 1;
			m_bg_shift_hi = (m_bg_shift_hi << 1) | 1;
			m_bg_shift_x0 <<= 1;
			m_bg_shift_x1 <<= 1;
			m_at_shift_lo = (m_at_shift_lo << 1) | (m_at_latch & 1);          // serial input: the attribute latch
			m_at_shift_hi = (m_at_shift_hi << 1) | ((m_at_latch >> 1) & 1);
			m_at_shift_x0 = (m_at_shift_x0 << 1) | ((m_at_latch >> 2) & 1);
			m_at_shift_x1 = (m_at_shift_x1 << 1) | ((m_at_latch >> 3) & 1);
		}
	}

	// sprite evaluation / secondary OAM / OAM buffer (before the sprite fetch below uses what it latched)
	dot_oam_step(dot, rendering);

	// shifter reload: the pattern high byte is read on the dot before (MAME fetches it a dot earlier still, atomically),
	// so whether the reload happens follows rendering on that dot - turning rendering on exactly at a reload dot leaves
	// the shifters unloaded (TriCNES's delayed-mask fetch; AccuracyCoin "BG Serial In")
	if (render_prev && (((dot >= 9) && (dot <= 257)) || (dot == 329) || (dot == 337)) && (((dot - 1) % 8) == 0))
		dot_reload_shifters();

	if (rendering)
	{


		if ((dot >= 1 && dot <= 256) || (dot >= 321 && dot <= 336))
		{
			switch ((dot - 1) % 8)
			{
			case 0: dot_fetch_nt(dot); break;
			case 1: dot_read_nt(); break;
			case 2: dot_fetch_at(); break;
			case 3: dot_read_at(); break;
			case 4: dot_fetch_pattern(false); break;
			case 5: dot_read_pattern(false); break;
			case 6: dot_fetch_pattern(true); break;
			case 7: dot_read_pattern(true); dot_increment_x(); break;
			}
		}


		if ((dot == 256) && !use_data_pipeline())
			scanline_increment_fine_ycounter();

		if (dot == 257)
		{
			if (!use_data_pipeline())
				m_refresh_data = (m_refresh_data & ~0x041f) | (m_refresh_latch & 0x041f);
			if (visible || prerender)
				dot_evaluate_sprites();
			else
			{
				m_spr_count = 0;
				m_spr_zero_in_line = false;
			}
		}

		if (dot >= 257 && dot <= 320)
		{
			m_regs[PPU_SPRITE_ADDRESS] = 0;
			// Each sprite slot performs two garbage nametable fetches, then pattern low and high.
			const int slot = (dot - 257) >> 3;
			const int phase = (dot - 257) & 7;
			if (phase == 0 || phase == 2)
			{
				const u16 addr = 0x2000 | (m_refresh_data & 0x0fff);
				ppu_bus(addr, dot_when(dot));
				bus_ale(addr);
			}
			else if (phase == 1 || phase == 3)
				(void)bus_read(0x2000 | (m_refresh_data & 0x0fff));   // garbage nametable fetch; data is discarded
			else if (phase == 4 || phase == 6)
			{
				if (phase == 4)
					dot_fetch_sprite_slot(slot);
				const u16 addr = (m_spr_addr[slot] + (phase == 6 ? 8 : 0)) & 0x1fff;
				if ((phase == 4) && m_read_latch.isnull() && !m_latch.isnull())
					m_latch(addr & 0x1ff0);
				ppu_bus(addr, dot_when(dot));
				m_pat_addr = addr;
				bus_ale(addr);
			}
			else
			{
				m_draw_phase = PPU_DRAW_OAM;
				u8 extra;
				u8 data = dot_sp_pattern(m_pat_addr, phase != 5, bus_read(m_pat_addr), m_spr_attr[slot], extra);
				if (!m_read_latch.isnull())
					m_read_latch(m_fetch_addr);
				if (m_spr_attr[slot] & 0x40)
				{
					data = bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7);
					extra = bitswap<8>(extra, 0, 1, 2, 3, 4, 5, 6, 7);
				}
				if (phase == 5)
					m_spr_lo[slot] = data;
				else
					m_spr_hi[slot] = data;
				// load this slot's output unit; an empty slot (secondary OAM $FF: out of range) gets a blank pattern
				const bool used = m_spr_in_range[slot];
				if (phase == 5)
				{
					m_sp_ctr[slot] = m_spr_x[slot];
					m_sp_attr[slot] = m_spr_attr[slot];
					m_sp_lo[slot] = used ? data : 0;
					m_sp_x0[slot] = used ? extra : 0;
					m_sp_half[slot] = 0;
				}
				else
				{
					m_sp_hi[slot] = used ? data : 0;
					m_sp_x1[slot] = used ? extra : 0;
				}
				// 16-pixel sprites (VT03): planes 2 and 3 are the sprite's second 8 pixels; flipped, they come first
				if ((phase != 5) && used && dot_sprite_16px() && (m_spr_attr[slot] & 0x40))
				{
					std::swap(m_sp_lo[slot], m_sp_x0[slot]);
					std::swap(m_sp_hi[slot], m_sp_x1[slot]);
				}
				m_draw_phase = PPU_DRAW_BG;
			}
		}
		if (dot == 337 || dot == 339)
		{
			const u16 addr = 0x2000 | (m_refresh_data & 0x0fff);
			ppu_bus(addr, dot_when(dot));
			bus_ale(addr);
		}
		else if (dot == 338 || dot == 340)
			m_idle_nt = bus_read(0x2000 | (m_refresh_data & 0x0fff));   // unused, but see dot 0

		if (prerender && (dot >= 280) && (dot <= 304) && !use_data_pipeline())
			m_refresh_data = (m_refresh_data & ~0x7be0) | (m_refresh_latch & 0x7be0);
	}
	else if (dot == 257)
	{
		// with rendering off the "sprite 0 is on this line" flag still takes the (stale) result of the last evaluation
		// that ran, and the output units keep what they hold (TriCNES; AccuracyCoin "Stale Sprite Shift Registers")
		m_spr_zero_in_line = BIT(m_eval_sprite0_mask, 0);
	}

	if (visible && (dot >= 1) && (dot <= 256))
	{
		dot_output_pixel(dot - 1, rendering);
		if (dot == 256)
			dot_line_done();
	}
	if (dot == 320)
	{
		dot_before_prefetch();
		m_line_v = m_line_v_next;
		m_line_v_next = m_refresh_data;
	}

	// sprite output units, after the pixel: a counter that is not zero counts down every dot (rendering or not); a unit
	// at zero shifts, but only while rendering, so a sprite left at X=0 draws as soon as rendering resumes (AccuracyCoin
	// "Stale BG Shift Registers" test 3)
	if ((dot >= 1) && (dot <= 256))
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_sp_ctr[i])
				m_sp_ctr[i]--;
			else if (rendering)
			{
				m_sp_lo[i] <<= 1;
				m_sp_hi[i] <<= 1;
				if (dot_sprite_16px())
				{
					// after its first 8 pixels a 16-pixel sprite unit draws planes 2 and 3 as its second half
					if (++m_sp_half[i] == 8)
					{
						m_sp_lo[i] = m_sp_x0[i];
						m_sp_hi[i] = m_sp_x1[i];
						m_sp_x0[i] = m_sp_x1[i] = 0;
					}
				}
				else
				{
					m_sp_x0[i] <<= 1;
					m_sp_x1[i] <<= 1;
				}
			}
		}
	}
	// with rendering off on dot 339 every unit's counter ends up zero (AccuracyCoin "Stale BG Shift Registers" test 4)
	if ((dot == 339) && !rendering)
		std::fill(std::begin(m_sp_ctr), std::end(m_sp_ctr), 0);
}

void ppu2c0x_device::update_scanline()
{
	if (m_scanline <= BOTTOM_VISIBLE_SCANLINE)
	{
		update_visible_scanline();
	}
}

/*************************************
*
*   PPU Memory functions
*
*************************************/

void ppu2c0x_device::palette_write(offs_t offset, u8 data)
{
	// palette RAM is only 6 bits wide
	data &= 0x3f;

	if (offset & 0x3)
	{
		// regular pens, no mirroring
		m_palette_ram[offset & 0x1f] = data;
	}
	else
	{
		// transparent pens are mirrored!
		if (0 == (offset & 0xf))
		{
			m_back_color = data;
		}
		m_palette_ram[offset & 0xf] = m_palette_ram[(offset & 0xf) + 0x10] = data;
	}
}

u8 ppu2c0x_device::palette_read(offs_t offset)
{
	if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
		return m_palette_ram[offset & 0x1f] & 0x30;
	else
		return m_palette_ram[offset & 0x1f];
}

/*************************************
 *
 *  PPU Registers Read
 *
 *************************************/

// sprite 0 hit / overflow as seen at the falling edge of M2 for the CPU access in progress: the PPU is brought up to
// that point (at most two dots ahead, still before the next CPU access), and the clear on dot 1 of the pre-render line,
// which is a timer event, is applied if it falls inside the window
u8 ppu2c0x_device::sprite_flags_at_m2_fall()
{
	const attotime now = machine().time();
	// sampled 7 master clocks after M2 rises: in the Visual 2C02 charts of reads around the sprite 0 hit and around the
	// sprite flags' clear (NESdev, User:Ulfalizer) the boundary read is the one whose high phase began 7 master clocks
	// before the change (TriCNES EmulateUntilEndOfRead agrees)
	const attotime fall = now + access_end_offset();
	if (fall <= now)
		return m_regs[PPU_STATUS] & 0x60;
	dot_run_to(line_dot_at(fall));
	if ((m_scanline == m_scanlines_per_frame - 1) && m_vbl_timer->enabled() && (m_vbl_timer->param() == 0) && (m_vbl_event_time <= fall))
		return 0;
	if ((m_scanline == m_scanlines_per_frame - 2) && ((m_next_line_start + m_dot_period) <= fall))
		return 0;   // the pre-render line starts (and clears the flags on its dot 1) before this access ends
	u8 flags = m_regs[PPU_STATUS] & 0x60;
	if (m_s0hit_pending && (m_s0hit_time <= fall))
		flags |= PPU_STATUS_SPRITE0_HIT;
	return flags;
}

u8 ppu2c0x_device::read(offs_t offset)
{
	if (use_dot_renderer())
		dot_run_to(current_line_dot());

	if (offset >= PPU_MAX_REG)
	{
		logerror("PPU %s: Attempting to read past the chip: offset %x\n", this->tag(), offset);
		offset &= PPU_MAX_REG - 1;
	}

	if (use_dot_renderer())
		io_latch();

	// see which register to read
	switch (offset & 7)
	{
	case PPU_STATUS: /* 2 */
		// a read in the last PPU clock before the VBL flag is set suppresses the flag and NMI for this frame; the set is
		// on dot 1 of the first vblank line, so with the access at M2 rise that read can still fall on the line before
		if (m_own_timing)
		{
			// The read latches the flag when M2 rises (now) and holds its clear input while M2 is high, until the cycle
			// ends: the flag reads set if its set pulse began before now, a pulse that ends while the clear is held
			// does not set it this frame, and a set flag is cleared from now (NESdev User:Ulfalizer, Visual 2C02)
			const attotime now = machine().time();
			const attotime fall = now - m_cpu->bus_offset() + m_cpu->cycles_to_attotime(1);
			const bool seen = vbl_flag_at(now);
			const attotime set_end = m_vbl_set_at + m_dot_period / 2;
			if (type() == PPU_PALC)
			{
				// The single-chip Dendy-style clones have no /NMI pin between PPU and CPU (NESdev, UM6561 pinout), so the
				// Visual 2C02 timing of that pin does not apply; what NESdev states for reads around the set does: a read one
				// PPU clock before it reads the flag clear and keeps the flag and the NMI from happening that frame
				if ((m_vbl_set_at != attotime::never) && (now >= m_vbl_set_at - m_dot_period) && (now < m_vbl_set_at))
					m_vbl_set_blocked = true;
			}
			else if ((m_vbl_set_at != attotime::never) && (now <= set_end) && (set_end < fall))
				m_vbl_set_blocked = true;
			if (seen)
			{
				m_vbl_clr_at = now;
				nmi_line_update_due(now);
			}
			m_regs[PPU_STATUS] = (m_regs[PPU_STATUS] & ~PPU_STATUS_VBLANK) | (seen ? PPU_STATUS_VBLANK : 0);
		}
		else if (use_dot_renderer() && m_vbl_set_pending && (m_scanline == m_vblank_first_scanline) && (current_line_dot() >= 0))
			m_vbl_suppress = true;

		// The top 3 bits of the status register are the only ones that report data. The
		// remainder contain whatever was last in the PPU data latch, except on the RC2C05 (protection)
		if (m_security_value)
			m_data_latch = (m_regs[PPU_STATUS] & 0xc0) | m_security_value;
		else if (use_dot_renderer())
		{
			// the VBL flag is latched when M2 rises (now), but the sprite 0 hit and overflow flags are not latched and
			// read as they are when M2 falls at the end of the cycle, ~1.9 dots later on a 2A03G (AccuracyCoin "$2002
			// flag timing")
			const u8 vbl = m_regs[PPU_STATUS] & PPU_STATUS_VBLANK;
			const u8 sprite_flags = (m_own_timing && use_dot_renderer()) ? sprite_flags_at_m2_fall() : (m_regs[PPU_STATUS] & 0x60);
			io_latch_refresh(vbl | sprite_flags, 0xe0);
		}
		else
			m_data_latch = m_regs[PPU_STATUS] | (m_data_latch & 0x1f);

		// Reset hi/lo scroll toggle
		m_toggle = false;

		// If the vblank bit is set, clear all status bits but the 2 sprite flags
		if (m_data_latch & PPU_STATUS_VBLANK)
		{
			m_regs[PPU_STATUS] &= 0x60;
			m_nmi_vbl = false;
		}
		if (use_dot_renderer() && !m_own_timing)
			update_nmi_line();
		break;

	case PPU_SPRITE_DATA: /* 4 */
		if (use_dot_renderer())
			oam_data_write_flush();
		// the RGB PPUs and 2C02s before G cannot read OAM: like any write-only port, the read returns the I/O latch (NESdev)
		if (!oam_palette_readable())
			break;
		if (use_dot_renderer() && m_render_eff
				&& ((m_scanline <= BOTTOM_VISIBLE_SCANLINE) || (m_scanline == m_scanlines_per_frame - 1)))
		{
			// the pre-render line is a rendering line for OAM too: there the PPU has secondary OAM selected for the whole
			// line (dot_oam_step), so the read does not return OAM[OAMADDR]. Hardware: Sour's OAM read tests on thefox's
			// consoles (NESdev forum "PAL NES, sprite evaluation and $2004 reads/writes") read OAM correctly on the 20
			// (2C02) or 24 (2C07) lines after VBlank starts and wrongly from then on, the pre-render line included
			// while rendering, OAMDATA reads return the OAM buffer: whatever the evaluation/fetch logic last moved through it
			// (2C02G and later; Micro Machines relies on this). The buffer is not latched by the read: the CPU takes it as
			// it is when M2 falls, at the end of the access (AccuracyCoin "$2004 Stress Test": "the read cycle will END on dot 0")
			if (m_own_timing)
			{
				const attotime now = machine().time();
				const attotime fall = now - m_cpu->bus_offset() + m_cpu->cycles_to_attotime(1);
				if (fall > now)
					dot_run_to(line_dot_at(fall));
			}
			m_data_latch = m_oam_buf;
		}
		else
		{
			const u8 addr = m_regs[PPU_SPRITE_ADDRESS];
			m_data_latch = m_spriteram[addr];
			if ((addr & 3) == 2)
				m_data_latch &= oam_attr_mask();
		}
		if (use_dot_renderer())
			io_latch_refresh(m_data_latch, 0xff);
		break;

	case PPU_DATA: /* 7 */
		if (use_data_pipeline())
		{
			// TriCNES: the CPU gets the read buffer, or palette RAM when the PPU bus is in palette space (bits 6-7 open
			// bus); the read machine starts when the access ends, 7 master clocks after M2 rises
			// the RGB PPUs cannot read palette RAM: the CPU gets the read buffer as for any other address (NESdev "PPU
			// variants"; the buffer itself is refilled from the PPU bus, i.e. the nametable below, on every PPU type)
			if (((m_bus_addr & 0x3fff) >= 0x3f00) && oam_palette_readable())
				io_latch_refresh(readbyte(0x3f00 | (m_refresh_data & 0x1f)), 0x3f);
			else
				io_latch_refresh(m_buffered_data, 0xff);
			m_r_start = machine().time() + access_end_offset();
			pipe_catchup(4);
			break;
		}
		if (use_dot_renderer() && vram_addr_is_v())
			m_videomem_addr = m_refresh_data & m_videoram_addr_mask;
		// Outside rendering, a CPU PPUDATA read drives the current VRAM address
		// before the access and subsequent increment.  Palette reads still perform
		// this external bus cycle even though palette RAM supplies the CPU result.
		if (use_dot_renderer() && !rendering_now())
			ppu_bus(m_videomem_addr & 0x3fff, machine().time());
		if (!(use_dot_renderer() && !m_read_latch.isnull()) && !m_latch.isnull())
			m_latch(m_videomem_addr & 0x3fff);

		if ((m_videomem_addr >= 0x3f00) && m_paletteram_in_ppuspace && oam_palette_readable())
		{
			if (use_dot_renderer())
				io_latch_refresh(readbyte(m_videomem_addr), 0x3f);   // palette reads drive bits 0-5 only
			else
				m_data_latch = readbyte(m_videomem_addr);
			// buffer the mirrored NT data
			if (use_dot_renderer())
			{
				m_rdbuf_value = readbyte(m_videomem_addr & 0x2fff);
				if (!m_read_latch.isnull())
					m_read_latch(m_videomem_addr & 0x2fff);
				m_rdbuf_delay = 6;   // counted down at the start of dots N .. N+5, so it lands at N+5
			}
			else
				m_buffered_data = readbyte(m_videomem_addr & 0x2fff);
		}
		else
		{
			if (use_dot_renderer())
				io_latch_refresh(m_buffered_data, 0xff);
			else
				m_data_latch = m_buffered_data;
			// an RGB PPU reading in palette space buffers the nametable below it, like the 2C02
			const u16 bufaddr = (!oam_palette_readable() && (m_videomem_addr >= 0x3f00)) ? (m_videomem_addr & 0x2fff) : m_videomem_addr;
			if (use_dot_renderer())
			{
				m_rdbuf_value = readbyte(bufaddr);
				if (!m_read_latch.isnull())
					m_read_latch(bufaddr & 0x3fff);
				m_rdbuf_delay = 6;
			}
			else
				m_buffered_data = readbyte(bufaddr);
		}

		if (use_dot_renderer() && vram_addr_is_v())
			data_port_increment();
		else
			m_videomem_addr = (m_videomem_addr + m_add) & m_videoram_addr_mask;
		ppu_bus_idle();
		break;

	default:
		break;
	}

	return m_data_latch;
}

u8 ppu2c04_clone_device::read(offs_t offset)
{
	switch (offset & 7)
	{
	case PPU_STATUS: /* 2 */
		// $2002 on this clone only contains the sprite 0 hit flag,
		// and it's hardwired to trigger after a specific scanline, not based on actual sprite positions
		if (m_scanline < 31 || m_scanline >= (m_vblank_first_scanline - 1))
			return ~PPU_STATUS_SPRITE0_HIT;
		return 0xff;

	case PPU_SPRITE_DATA: /* 4 */
		return m_spritebuf[m_regs[PPU_SPRITE_ADDRESS]];
	}

	return ppu2c0x_device::read(offset);
}

/*************************************
 *
 *  PPU Registers Write
 *
 *************************************/

// NESdev OAMDATA: writes during rendering (visible and pre-render lines) do not modify OAM but bump OAMADDR by 4 and
// clear its low two bits (AccuracyCoin "Address $2004 behavior" error A); bits 2-4 of each sprite's attribute byte do
// not exist and always read back as 0
void ppu2c0x_device::oam_data_write_apply(u8 data)
{
	// 2C07: OAM cannot be written during the forced refresh (NESdev Errata: "OAM cannot be accessed from scanlines 21
	// through 70, after the NMI would have happened"; an OAM DMA that late corrupts the whole transfer). What the write
	// does to OAMADDR then is not documented; it is left to the refresh, which is incrementing it
	if (pal_oam_refresh_line())
		return;

	const bool rendering = m_render_eff
			&& ((m_scanline <= BOTTOM_VISIBLE_SCANLINE) || (m_scanline == m_scanlines_per_frame - 1));
	if (rendering)
		m_regs[PPU_SPRITE_ADDRESS] = (m_regs[PPU_SPRITE_ADDRESS] + 4) & 0xfc;
	else
	{
		const u8 addr = m_regs[PPU_SPRITE_ADDRESS];
		m_spriteram[addr] = ((addr & 3) == 2) ? (data & oam_attr_mask()) : data;
		m_regs[PPU_SPRITE_ADDRESS] = (addr + 1) & 0xff;
	}
}

void ppu2c0x_device::oam_data_write_flush()
{
	if (m_oamw_pending)
	{
		m_oamw_pending = false;
		m_oamw_delay = 0;
		oam_data_write_apply(m_oamw_data);
	}
}

void ppu2c0x_device::write_to_spriteram_with_increment(u8 data)
{
	if (use_dot_renderer())
	{
		// the write lands m_oamw_latency dots later: hardware needs "a delay of a few PPU cycles" so a write on dot 319 of
		// the pre-render line still survives the OAMADDR reset of dots 257-320 (AccuracyCoin "Address $2004 behavior"
		// test 6); the exact count is not documented, and 3 is bounded by that test, not measured; a write still
		// pending from before lands first
		oam_data_write_flush();
		if (m_oamw_latency <= 0)
			oam_data_write_apply(data);
		else
		{
			m_oamw_pending = true;
			m_oamw_data = data;
			m_oamw_delay = m_oamw_latency + 1;   // counted down at the start of dots N .. N+latency
		}
		return;
	}

	// writes to sprite data during rendering do not modify memory, but bump the high six bits of the address
	// (NESdev, PPU registers: OAMDATA), as the dot renderer above does
	if (m_scanline > BOTTOM_VISIBLE_SCANLINE || !(m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)))
	{
		m_spriteram[m_regs[PPU_SPRITE_ADDRESS]] = data;
		m_regs[PPU_SPRITE_ADDRESS] = (m_regs[PPU_SPRITE_ADDRESS] + 1) & 0xff;
	}
	else
		m_regs[PPU_SPRITE_ADDRESS] = (m_regs[PPU_SPRITE_ADDRESS] + 4) & 0xff;
}

void ppu2c0x_device::write(offs_t offset, u8 data)
{
	apply_due_events();
	if (use_dot_renderer())
		dot_run_to(current_line_dot());

	// after a reset, PPUCTRL, PPUMASK, PPUSCROLL and PPUADDR ignore writes until the warm-up is over
	if (use_dot_renderer() && (machine().time() < m_warmup_until))
	{
		const offs_t reg = offset & 7;
		if (reg == PPU_CONTROL0 || reg == PPU_CONTROL1 || reg == PPU_SCROLL || reg == PPU_ADDRESS)
			return;
	}

	if (offset >= PPU_MAX_REG)
	{
		logerror("PPU %s: Attempting to write past the chip: offset %x, data %x\n", this->tag(), offset, data);
		offset &= PPU_MAX_REG - 1;
	}

#if 0
	if (m_scanline <= BOTTOM_VISIBLE_SCANLINE)
	{
		logerror("PPU register %d write %02x during non-vblank scanline %d (MAME %d, beam pos: %d)\n", offset, data, m_scanline, screen().vpos(), screen().hpos());
	}
#endif

	/* on the RC2C05, PPU_CONTROL0 and PPU_CONTROL1 are swapped (protection) */
	if (m_security_value && !(offset & 6))
		offset ^= 1;

	switch (offset & 7)
	{
	case PPU_CONTROL0: /* 0 */
	{
		const u8 old_control0 = m_regs[PPU_CONTROL0];
		m_regs[PPU_CONTROL0] = data;

		/* update the name table number on our refresh latches */
		m_refresh_latch &= 0x73ff;
		m_refresh_latch |= (data & 3) << 10;

		/* the char ram bank points either 0x0000 or 0x1000 (page 0 or page 4) */
		m_tile_page = (data & PPU_CONTROL0_CHR_SELECT) >> 2;
		m_sprite_page = (data & PPU_CONTROL0_SPR_SELECT) >> 1;

		m_add = (data & PPU_CONTROL0_INC) ? m_line_write_increment_large : 1;
		//logerror("control0 write: %02x (scanline: %d)\n", data, m_scanline);

		/* enabling NMI while the vblank flag is already set generates an NMI right away */
		if (m_own_timing)
		{
			// /NMI follows the enable's own timing (nmi_asserted_at)
			if ((old_control0 ^ data) & PPU_CONTROL0_NMI)
			{
				m_nmi_en_prev = (old_control0 & PPU_CONTROL0_NMI) != 0;
				m_nmi_en_at = machine().time();
				nmi_line_update_due(m_nmi_en_at);
			}
		}
		else if (use_dot_renderer())
		{
			// /NMI must stay low long enough for the CPU to sample it: an enable that the VBL clear follows
			// within the same window produces no NMI, so drive the line after the same settling time as on VBL set
			if (!(old_control0 & PPU_CONTROL0_NMI) && (data & PPU_CONTROL0_NMI) && (m_regs[PPU_STATUS] & PPU_STATUS_VBLANK))
				m_nmi_timer->adjust(dot_time(2));
			else
				update_nmi_line();
		}
		else if (!(old_control0 & PPU_CONTROL0_NMI) && (data & PPU_CONTROL0_NMI) && (m_regs[PPU_STATUS] & PPU_STATUS_VBLANK))
		{
			m_int_callback(ASSERT_LINE);
			m_int_callback(CLEAR_LINE);
		}
	}
	break;

	case PPU_CONTROL1: /* 1 */
		//logerror("control1 write: %02x (scanline: %d)\n", data, m_scanline);
		if (use_dot_renderer() && m_own_timing && (type() != PPU_2C07)
				&& ((m_regs[PPU_CONTROL1] ^ data) & (PPU_CONTROL1_DISPLAY_MONO | PPU_CONTROL1_COLOR_EMPHASIS))
				&& (m_scanline <= BOTTOM_VISIBLE_SCANLINE) && (m_line_pal_y == m_scanline))
		{
			// Greyscale and emphasis are applied where the pixel leaves the PPU, 3 dots after its colour is chosen, and
			// a PPUMASK write reaches them 2 dots after it lands (TriCNES: 3-dot output pipeline, 2-dot emphasis delay):
			// the new bits already apply to the pixel whose colour was chosen on the dot before the write - drawn here
			// before the write came in, so it is coloured again (blargg nmi_sync: the timed greyscale line starts on the
			// reference sprites' column)
			const int x = current_line_dot() - 2;
			if ((x >= 0) && (x < 256))
			{
				u16 v = m_line_pal[x];
				v &= (data & PPU_CONTROL1_DISPLAY_MONO) ? 0x30 : 0x3f;
				v |= (data & PPU_CONTROL1_COLOR_EMPHASIS) << 1;
				m_bitmap.pix(m_scanline, x) = pen_color(v);
			}
		}
		m_regs[PPU_CONTROL1] = data;
		// greyscale/emphasis at the pixel output: at once on the 2C02 (with the pixel already drawn, above); on the 2C07 a
		// dot later. NESdev does not document the 2C07's timing here: with the 2C02's mechanism blargg's nmi_sync PAL line
		// starts 2 pixels left of the farthest-left position the hardware shows (its upper reference line); a dot later
		// keeps it within the hardware range on CPU/PPU alignments 0-3, and a pixel past it on alignment 4 - the 2C07's
		// real behaviour is finer than whole dots
		if (use_dot_renderer() && m_own_timing && (type() == PPU_2C07))
		{
			m_out_mask_next = data;
			m_out_mask_delay = 2;   // counted down at the start of dots N and N+1: in effect from dot N+1
		}
		else
		{
			m_out_mask = data;
			m_out_mask_delay = 0;
		}
		if (use_dot_renderer())
		{
			const bool enable = (data & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) != 0;
			// Re-enabling rendering outside vblank with an OAM corruption pending: the corruption does not happen when the
			// write reaches the PPU on master clock 1 or 2 of a dot (TriCNES PPU_OAMCorruptionRenderingEnabledOutOfVBlank,
			// PPUClock 1/2). TriCNES makes CPU writes at the start of the CPU cycle; here the write is made when M2 rises,
			// mid master clock, so the first PPU master clock that sees it is the next whole one - the phase is rounded up.
			// (blargg sprite_overflow_tests 5.Emulator re-enables rendering mid-frame after a corrupting disable)
			if (enable && !m_render_pending && m_oam_corrupt_rows && m_own_timing
					&& ((m_scanline < m_vblank_first_scanline) || (m_scanline == m_scanlines_per_frame - 1)))
			{
				const u32 master = m_cpu->clock() * cpu_master_clocks();
				const attotime now = machine().time();
				// position in the dot to the nearest half master clock: the write sits exactly on a half clock, and plain
				// truncation of the attotime difference can land a hair short of it and pick the wrong phase
				const u64 quarter_ticks = (now > m_line_start) ? (now - m_line_start).as_ticks(master * 4) : 0;
				const u64 half_ticks = (quarter_ticks + 1) / 2;
				const int phase = int(((half_ticks + 1) / 2) % dot_master_clocks());
				if ((phase == 1) || (phase == 2))
					m_oam_corrupt_rows = 0;
			}
			m_render_pending = enable;
			m_render_delay = m_mask_latency + 1;   // counted down at the start of dots N .. N+latency
		}
		ppu_bus_idle();
		break;

	case PPU_SPRITE_ADDRESS: /* 3 */
		if (use_dot_renderer())
			oam_data_write_flush();
		// Not emulated: on the 2C02G a write to OAMADDR corrupts OAM. NESdev ("PPU registers", OAMADDR precautions) calls
		// the corruption "not fully described": usually the row at $20 is copied over the row OAMADDR selects, the source
		// seeming to come from the previous value on the CPU bus. Modelled that way it fails AccuracyCoin's "Address $2004
		// Behavior" test (error A), which passes on a 2C02G, so the description is not complete enough to emulate.
		m_regs[PPU_SPRITE_ADDRESS] = data;
		break;

	case PPU_SPRITE_DATA: /* 4 */
		write_to_spriteram_with_increment(data);
		break;

	case PPU_SCROLL: /* 5 */
		if (m_toggle)
		{
			/* second write */
			m_refresh_latch &= 0x0c1f;
			m_refresh_latch |= (data & 0xf8) << 2;
			m_refresh_latch |= (data & 0x07) << 12;
			//logerror("scroll write 2: %d, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
		}
		else
		{
			/* first write */
			m_refresh_latch &= (0xffe0 & m_global_refresh_mask);
			m_refresh_latch |= (data & 0xf8) >> 3;

			m_x_fine = data & 7;
			//logerror("scroll write 1: %d, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
		}

		m_toggle = !m_toggle;
		break;

	case PPU_ADDRESS: /* 6 */
		if (m_toggle)
		{
			/* second write */
			m_refresh_latch &= (0xff00 & m_global_refresh_mask);
			m_refresh_latch |= data;
			if (use_data_pipeline())
			{
				// TriCNES: v (and the PPU bus) take the new address 4 dots later
				m_2006_new = m_refresh_latch;
				m_2006_delay = 4;
				pipe_catchup(5);
			}
			else
			{
				m_refresh_data = m_refresh_latch;
				m_videomem_addr = m_refresh_latch;
				ppu_bus_idle();
			}
			//logerror("vram addr write 2: %02x, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
		}
		else
		{
			/* first write */
			m_refresh_latch &= 0x00ff;
			m_refresh_latch |= (data & (m_videoram_addr_mask >> 8)) << 8;
			//logerror("vram addr write 1: %02x, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
		}

		m_toggle = !m_toggle;
		break;

	case PPU_DATA: /* 7 */
	{
		if (use_data_pipeline())
		{
			// TriCNES: the write machine starts when the access ends (7 master clocks after M2 rises)
			m_w_data = data;
			m_w_start = machine().time() + access_end_offset();
			pipe_catchup(4);
			break;
		}
		if (use_dot_renderer() && vram_addr_is_v())
			m_videomem_addr = m_refresh_data & m_videoram_addr_mask;
		const int tempAddr = m_videomem_addr & m_videoram_addr_mask;

		// Outside rendering, PPUDATA writes drive the current VRAM address before
		// the write and subsequent increment.  Palette writes expose the address
		// even though the PPU does not assert external /WR for internal palette RAM.
		if (use_dot_renderer() && !rendering_now())
			ppu_bus(tempAddr & 0x3fff, machine().time());

		// the MMC2/MMC4 latches react to PPU reads only, so with the read latch in use PPUDATA writes do not trigger it
		if (!(use_dot_renderer() && !m_read_latch.isnull()) && !m_latch.isnull())
			m_latch(tempAddr);

		/* store the data */
		writebyte(tempAddr, data);

		/* increment the address */
		if (use_dot_renderer() && vram_addr_is_v())
			data_port_increment();
		else
			m_videomem_addr = (m_videomem_addr + m_add) & m_videoram_addr_mask;
		ppu_bus_idle();
	}
	break;

	default:
		/* ignore other registers writes */
		break;
	}

	if (use_dot_renderer())
		io_latch_refresh(data, 0xff);
	else
		m_data_latch = data;
}

void ppu2c04_clone_device::write(offs_t offset, u8 data)
{
	switch (offset & 7)
	{
	case PPU_CONTROL0: /* 0 */
		data &= 0x01 | PPU_CONTROL0_INC | PPU_CONTROL0_NMI; /* other bits of $2000 are ignored by this clone */
		data |= PPU_CONTROL0_CHR_SELECT;
		break;

	case PPU_CONTROL1: /* 1 */
	case PPU_SPRITE_ADDRESS: /* 3 */
		return; /* $2001 and $2003 do nothing on this clone */

	case PPU_SPRITE_DATA: /* 4 */
		m_spritebuf[m_regs[PPU_SPRITE_ADDRESS]] = data;
		m_regs[PPU_SPRITE_ADDRESS] = (m_regs[PPU_SPRITE_ADDRESS] + 1) & 0xff;
		return;

	case PPU_SCROLL: /* 5 */
		if (m_toggle)
			data = 0; /* no vertical scroll */
		break;

	case PPU_ADDRESS: /* 6 */
		/* $2006 doesn't affect scroll latching */
		if (m_toggle)
		{
			/* second write */
			set_vram_dest((get_vram_dest() & 0xff00) | data);
		}
		else
		{
			/* first write */
			set_vram_dest((get_vram_dest() & 0x00ff) | (data << 8));
		}

		m_toggle = !m_toggle;
		return;
	}

	ppu2c0x_device::write(offset, data);
}

u16 ppu2c0x_device::get_vram_dest()
{
	return m_videomem_addr;
}

void ppu2c0x_device::set_vram_dest(u16 dest)
{
	m_videomem_addr = dest;
}

void ppu2c0x_device::reload_refresh_data()
{
	m_refresh_data = m_refresh_latch;
}

/*************************************
 *
 *  Sprite DMA
 *
 *************************************/

void ppu2c0x_device::spriteram_dma(address_space& space, const u8 page)
{
	const int address = page << 8;

	for (int i = 0; i < 0x100; i++)
	{
		const u8 spriteData = space.read_byte(address + i);
		space.write_byte(0x2004, spriteData);
	}

	// OAM DMA takes 513 CPU cycles when already aligned to a get cycle and
	// 514 when an additional alignment cycle is required.
	const int dma_cycles = 513 + BIT(space.device().execute().total_cycles(), 0);
	space.device().execute().adjust_icount(-dma_cycles);
}

/*************************************
 *
 *  PPU Rendering
 *
 *************************************/

void ppu2c0x_device::render(bitmap_rgb32& bitmap, bool flipx, bool flipy, int sx, int sy, const rectangle& cliprect)
{
	if (use_dot_renderer())
		dot_run_to(current_line_dot());
	else if (m_scanline_timer->remaining() != attotime::zero)
	{
		// Partial line update, need to render first (especially for light gun emulation).
		update_scanline();
	}
	copybitmap(bitmap, m_bitmap, flipx, flipy, sx, sy, cliprect);
}

u32 ppu2c0x_device::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	render(bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
