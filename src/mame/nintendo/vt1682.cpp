// license:BSD-3-Clause
// copyright-holders:David Haywood

/*  VT1682 - NOT compatible with NES, different video system, sound CPU (4x
             main CPU clock), optional internal ROM etc.  The design is somewhat
             based on the NES but the video / sound system is significantly
             changed

    Internal ROM can be mapped to Main CPU, or Sound CPU at 0x3000-0x3fff if used
    can also be configured as boot device
*/

/*
    UNIMPLEMENTED / TODO

    General VT1862:

    Sound Quality (currently crackles)
    Verify timer enable / disable behavior
    Line Modes, High Colour Line Mode
    Tile rowscroll modes
    0x8000 bit in palette is 'cut through' mode, which isn't the same as transpen, some kind of palette manipulation
    **DONE** It seems Pal1 and Pal2 should actually be separate render buffers for each palette, on which layers / sprites can be enabled, that are mixed later and can be output independently to LCD and TV?
        (how does this work with high colour line mode?)
    CCIR effects (only apply to 'palette 2'?)
    LCD Control registers
    Internal to External DMA (glitchy)
    Sprite limits
    Other hardware limits (video DMA should be delayed until Vblank, some registers only take effect at Hblank)
    Verify raster timing (might be off by a line)
    Hardware glitches (scroll layers + sprites get offset under specific conditions, sprites sometimes missing in 2 rightmost column, bk sometimes missing in rightmost column during scroll)
    Sleep functionality on sound cpu (broken on hardware?)
    Interrupt controller / proper interrupt support (currently a bit hacky, only main timer and sub-timer a supported)
    Proper IO support (enables / disables) UART, I2C etc.
    'Capture' mode
    Gain (zoom) for Tilemaps
    Refactor into a device
    Verify that internal ROMs are blank (it isn't used for any set we have)

    + more

    -----------

    Intec InterAct:

    Is there meant to be a 2nd player? (many games prompt a 2nd player to start, but inputs don't appear to be read?)

    -----------

    Excite Sports 48-in-1:

    Why are the rasters broken on MX Motorstorm when the game game works in other collections? does the alt input reading throw the timing off enough that the current hookup fails
    or is there a different PAL/NTSC detection method that we're failing?

    Why is the priority incorrect in Ping Pong, again it was fine in the other collections

    No sound in Archery?

*/

#include "emu.h"
#include "m6502_swap_op_d2_d7.h"
#include "m6502_swap_op_d5_d6.h"
#include "vt1682_io.h"
#include "vt1682_uio.h"
#include "vt1682_alu.h"
#include "vt1682_timer.h"
#include "machine/bankdev.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_VRAM_WRITES      (1U << 1)
#define LOG_SRAM_WRITES      (1U << 2)
#define LOG_OTHER            (1U << 3)

#define LOG_ALL           (LOG_VRAM_WRITES | LOG_SRAM_WRITES | LOG_OTHER)

#define VERBOSE             (0)
#include "logmacro.h"


namespace {

// NTSC uses XTAL(21'477'272) Sound CPU runs at exactly this, Main CPU runs at this / 4
// PAL  uses XTAL(26'601'712) Sound CPU runs at exactly this, Main CPU runs at this / 5

// can also be used with the following
// PAL M 21.453669MHz
// PAL N 21.492336MHz

#define MAIN_CPU_CLOCK_NTSC XTAL(21'477'272)/4
#define SOUND_CPU_CLOCK_NTSC XTAL(21'477'272)
#define TIMER_ALT_SPEED_NTSC (15746)

#define MAIN_CPU_CLOCK_PAL XTAL(26'601'712)/5
#define SOUND_CPU_CLOCK_PAL XTAL(26'601'712)
#define TIMER_ALT_SPEED_PAL (15602)



class vt_vt1682_state : public driver_device
{
public:
	vt_vt1682_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_io(*this, "io"),
		m_uio(*this, "uio"),
		m_leftdac(*this, "leftdac"),
		m_rightdac(*this, "rightdac"),
		m_maincpu(*this, "maincpu"),
		m_fullrom(*this, "fullrom"),
		m_bank(*this, "cartbank"),
		m_screen(*this, "screen"),
		m_soundcpu(*this, "soundcpu"),
		m_soundcpu_timer_a_dev(*this, "snd_timera_dev"),
		m_soundcpu_timer_b_dev(*this, "snd_timerb_dev"),
		m_system_timer_dev(*this, "sys_timer_dev"),
		m_maincpu_alu(*this, "mainalu"),
		m_soundcpu_alu(*this, "soundalu"),
		m_spriteram(*this, "spriteram"),
		m_vram(*this, "vram"),
		m_sound_share(*this, "sound_share"),
		m_gfxdecode(*this, "gfxdecode2"),
		m_palette(*this, "palette"),
		m_render_timer(*this, "render_timer")
	{ }

	[[maybe_unused]] void vt_vt1682(machine_config& config);
	void regular_init();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<vrt_vt1682_io_device> m_io;
	required_device<vrt_vt1682_uio_device> m_uio;
	required_device<dac_12bit_r2r_device> m_leftdac;
	required_device<dac_12bit_r2r_device> m_rightdac;
	required_device<cpu_device> m_maincpu;

	void vt_vt1682_map(address_map &map) ATTR_COLD;
	void vt_vt1682_sound_map(address_map &map) ATTR_COLD;

	required_device<address_map_bank_device> m_fullrom;
	required_memory_bank m_bank;
	required_device<screen_device> m_screen;
	required_device<cpu_device> m_soundcpu;

	void soundcpu_timera_irq(int state);
	void soundcpu_timerb_irq(int state);

	void maincpu_timer_irq(int state);

	required_device<vrt_vt1682_timer_device> m_soundcpu_timer_a_dev;
	required_device<vrt_vt1682_timer_device> m_soundcpu_timer_b_dev;
	required_device<vrt_vt1682_timer_device> m_system_timer_dev;

	void vt_vt1682_ntscbase(machine_config& config);
	void vt_vt1682_palbase(machine_config& config);
	void vt_vt1682_common(machine_config& config);

private:
	required_device<vrt_vt1682_alu_device> m_maincpu_alu;
	required_device<vrt_vt1682_alu_device> m_soundcpu_alu;


	required_device<address_map_bank_device> m_spriteram;
	required_device<address_map_bank_device> m_vram;
	required_shared_ptr<uint8_t> m_sound_share;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_render_timer;

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	void rom_map(address_map &map) ATTR_COLD;

	void spriteram_map(address_map &map) ATTR_COLD;
	void vram_map(address_map &map) ATTR_COLD;


	/* Video */
	uint8_t m_2000;
	uint8_t m_2001;

	uint8_t m_2002_sprramaddr_2_0; // address attribute
	uint8_t m_2003_sprramaddr_10_3; // address sprite number
	uint8_t m_2005_vramaddr_7_0;
	uint8_t m_2006_vramaddr_15_8;

	uint8_t m_201a_sp_segment_7_0;
	uint8_t m_201b_sp_segment_11_8;

	uint8_t m_segment_7_0_bk[2];
	uint8_t m_segment_11_8_bk[2];

	uint8_t m_main_control_bk[2];

	uint8_t m_scroll_control_bk[2];

	uint8_t m_xscroll_7_0_bk[2];
	uint8_t m_yscroll_7_0_bk[2];

	uint8_t m_200e_blend_pal_sel;
	uint8_t m_200f_bk_pal_sel;

	uint8_t m_2008_lcd_vs_delay;
	uint8_t m_2009_lcd_hs_delay_7_0;
	uint8_t m_200a_lcd_fr_delay_7_0;

	uint8_t m_200d_misc_vregs2;
	uint8_t m_200c_misc_vregs1;
	uint8_t m_200b_misc_vregs0;

	uint8_t m_2018_spregs;
	uint8_t m_2019_bkgain;

	uint8_t m_2020_bk_linescroll;
	uint8_t m_2021_lum_offset;
	uint8_t m_2022_saturation_misc;

	uint8_t m_2023_lightgun_reset;
	uint8_t m_2024_lightgun1_y;
	uint8_t m_2025_lightgun1_x;
	uint8_t m_2026_lightgun2_y;
	uint8_t m_2027_lightgun2_x;

	uint8_t m_2031_red_dac;
	uint8_t m_2032_green_dac;
	uint8_t m_2033_blue_dac;

	uint8_t m_2028;
	uint8_t m_2029;
	uint8_t m_202a;
	uint8_t m_202b;
	uint8_t m_202e;
	uint8_t m_2030;


	uint8_t vt1682_2000_r();
	void vt1682_2000_w(uint8_t data);

	uint8_t vt1682_2001_vblank_r();
	void vt1682_2001_w(uint8_t data);

	uint8_t vt1682_2002_sprramaddr_2_0_r();
	void vt1682_2002_sprramaddr_2_0_w(uint8_t data);
	uint8_t vt1682_2003_sprramaddr_10_3_r();
	void vt1682_2003_sprramaddr_10_3_w(uint8_t data);
	uint8_t vt1682_2004_sprram_data_r();
	void vt1682_2004_sprram_data_w(uint8_t data);

	uint8_t vt1682_2005_vramaddr_7_0_r();
	void vt1682_2005_vramaddr_7_0_w(uint8_t data);
	uint8_t vt1682_2006_vramaddr_15_8_r();
	void vt1682_2006_vramaddr_15_8_w(uint8_t data);
	uint8_t vt1682_2007_vram_data_r();
	void vt1682_2007_vram_data_w(uint8_t data);

	uint8_t vt1682_201a_sp_segment_7_0_r();
	void vt1682_201a_sp_segment_7_0_w(uint8_t data);
	uint8_t vt1682_201b_sp_segment_11_8_r();
	void vt1682_201b_sp_segment_11_8_w(uint8_t data);

	uint8_t vt1682_201c_bk1_segment_7_0_r();
	void vt1682_201c_bk1_segment_7_0_w(uint8_t data);
	uint8_t vt1682_201d_bk1_segment_11_8_r();
	void vt1682_201d_bk1_segment_11_8_w(uint8_t data);
	uint8_t vt1682_201e_bk2_segment_7_0_r();
	void vt1682_201e_bk2_segment_7_0_w(uint8_t data);
	uint8_t vt1682_201f_bk2_segment_11_8_r();
	void vt1682_201f_bk2_segment_11_8_w(uint8_t data);

	uint8_t vt1682_2013_bk1_main_control_r();
	void vt1682_2013_bk1_main_control_w(uint8_t data);
	uint8_t vt1682_2017_bk2_main_control_r();
	void vt1682_2017_bk2_main_control_w(uint8_t data);

	uint8_t vt1682_2012_bk1_scroll_control_r();
	void vt1682_2012_bk1_scroll_control_w(uint8_t data);
	uint8_t vt1682_2016_bk2_scroll_control_r();
	void vt1682_2016_bk2_scroll_control_w(uint8_t data);

	uint8_t vt1682_2010_bk1_xscroll_7_0_r();
	void vt1682_2010_bk1_xscroll_7_0_w(uint8_t data);
	uint8_t vt1682_2011_bk1_yscoll_7_0_r();
	void vt1682_2011_bk1_yscoll_7_0_w(uint8_t data);
	uint8_t vt1682_2014_bk2_xscroll_7_0_r();
	void vt1682_2014_bk2_xscroll_7_0_w(uint8_t data);
	uint8_t vt1682_2015_bk2_yscoll_7_0_r();
	void vt1682_2015_bk2_yscoll_7_0_w(uint8_t data);

	uint8_t vt1682_200e_blend_pal_sel_r();
	void vt1682_200e_blend_pal_sel_w(uint8_t data);
	uint8_t vt1682_200f_bk_pal_sel_r();
	void vt1682_200f_bk_pal_sel_w(uint8_t data);

	uint8_t vt1682_2008_lcd_vs_delay_r();
	void vt1682_2008_lcd_vs_delay_w(uint8_t data);
	uint8_t vt1682_2009_lcd_hs_delay_7_0_r();
	void vt1682_2009_lcd_hs_delay_7_0_w(uint8_t data);
	uint8_t vt1682_200a_lcd_fr_delay_7_0_r();
	void vt1682_200a_lcd_fr_delay_7_0_w(uint8_t data);

	uint8_t vt1682_200d_misc_vregs2_r();
	void vt1682_200d_misc_vregs2_w(uint8_t data);
	uint8_t vt1682_200c_misc_vregs1_r();
	void vt1682_200c_misc_vregs1_w(uint8_t data);
	uint8_t vt1682_200b_misc_vregs0_r();
	void vt1682_200b_misc_vregs0_w(uint8_t data);

	uint8_t vt1682_2018_spregs_r();
	void vt1682_2018_spregs_w(uint8_t data);
	uint8_t vt1682_2019_bkgain_r();
	void vt1682_2019_bkgain_w(uint8_t data);

	uint8_t vt1682_2020_bk_linescroll_r();
	void vt1682_2020_bk_linescroll_w(uint8_t data);
	uint8_t vt1682_2021_lum_offset_r();
	void vt1682_2021_lum_offset_w(uint8_t data);
	uint8_t vt1682_2022_saturation_misc_r();
	void vt1682_2022_saturation_misc_w(uint8_t data);

	uint8_t vt1682_2023_lightgun_reset_r();
	void vt1682_2023_lightgun_reset_w(uint8_t data);
	uint8_t vt1682_2024_lightgun1_y_r();
	void vt1682_2024_lightgun1_y_w(uint8_t data);
	uint8_t vt1682_2025_lightgun1_x_r();
	void vt1682_2025_lightgun1_x_w(uint8_t data);
	uint8_t vt1682_2026_lightgun2_y_r();
	void vt1682_2026_lightgun2_y_w(uint8_t data);
	uint8_t vt1682_2027_lightgun2_x_r();
	void vt1682_2027_lightgun2_x_w(uint8_t data);

	uint8_t vt1682_2031_red_dac_r();
	void vt1682_2031_red_dac_w(uint8_t data);
	uint8_t vt1682_2032_green_dac_r();
	void vt1682_2032_green_dac_w(uint8_t data);
	uint8_t vt1682_2033_blue_dac_r();
	void vt1682_2033_blue_dac_w(uint8_t data);

	uint8_t vt1682_2028_r();
	void vt1682_2028_w(uint8_t data);
	uint8_t vt1682_2029_r();
	void vt1682_2029_w(uint8_t data);
	uint8_t vt1682_202a_r();
	void vt1682_202a_w(uint8_t data);
	uint8_t vt1682_202b_r();
	void vt1682_202b_w(uint8_t data);
	uint8_t vt1682_202e_r();
	void vt1682_202e_w(uint8_t data);
	uint8_t vt1682_2030_r();
	void vt1682_2030_w(uint8_t data);

	/* Video Helpers */

	uint16_t get_spriteram_addr()
	{
		return (m_2002_sprramaddr_2_0 & 0x07) | (m_2003_sprramaddr_10_3 << 3);
	}


	void set_spriteram_addr(uint16_t addr)
	{
		m_2002_sprramaddr_2_0 = addr & 0x07;
		m_2003_sprramaddr_10_3 = addr >> 3;
	}


	void inc_spriteram_addr()
	{
		// there is some strange logic here, sources state on DMA only so this might not be correct
		// it is unclear what happens if an address where the lower bits are 0x6/0x7 is set directly
		// the ii8in1 set clearly only writes 0x600 bytes worth of data, without using DMA suggesting
		// that this 'skipping' applies to non-DMA writes too.
		int addr = get_spriteram_addr();
		addr++;
		if ((addr & 0x07) >= 0x6)
		{
			addr += 0x8;
			addr &= ~0x7;
		}
		set_spriteram_addr(addr);
	}

	uint16_t get_vram_addr()
	{
		return (m_2005_vramaddr_7_0) | (m_2006_vramaddr_15_8 << 8);
	}

	void set_vram_addr(uint16_t addr)
	{
		m_2005_vramaddr_7_0 = addr & 0xff;
		m_2006_vramaddr_15_8 = addr >> 8;
	}

	/* System */
	uint8_t m_prgbank1_r0;
	uint8_t m_prgbank1_r1;
	uint8_t m_210c_prgbank1_r2;
	uint8_t m_2100_prgbank1_r3;
	uint8_t m_2118_prgbank1_r4_r5;

	uint8_t m_2107_prgbank0_r0;
	uint8_t m_2108_prgbank0_r1;
	uint8_t m_2109_prgbank0_r2;
	uint8_t m_210a_prgbank0_r3;
	uint8_t m_prgbank0_r4;
	uint8_t m_prgbank0_r5;

	uint8_t m_210b_misc_cs_prg0_bank_sel;

	uint8_t m_2105_vt1682_2105_comr6_tvmodes;
	uint8_t m_211c_regs_ext2421;

	uint8_t m_2122_dma_dt_addr_7_0;
	uint8_t m_2123_dma_dt_addr_15_8;

	uint8_t m_2124_dma_sr_addr_7_0;
	uint8_t m_2125_dma_sr_addr_15_8;

	uint8_t m_2126_dma_sr_bank_addr_22_15;
	uint8_t m_2128_dma_sr_bank_addr_24_23;

	uint8_t m_2106_enable_reg;

	uint8_t vt1682_2100_prgbank1_r3_r();
	void vt1682_2100_prgbank1_r3_w(uint8_t data);
	uint8_t vt1682_210c_prgbank1_r2_r();
	void vt1682_210c_prgbank1_r2_w(uint8_t data);

	uint8_t vt1682_2107_prgbank0_r0_r();
	void vt1682_2107_prgbank0_r0_w(uint8_t data);
	uint8_t vt1682_2108_prgbank0_r1_r();
	void vt1682_2108_prgbank0_r1_w(uint8_t data);
	uint8_t vt1682_2109_prgbank0_r2_r();
	void vt1682_2109_prgbank0_r2_w(uint8_t data);
	uint8_t vt1682_210a_prgbank0_r3_r();
	void vt1682_210a_prgbank0_r3_w(uint8_t data);

	uint8_t vt1682_prgbank0_r4_r();
	uint8_t vt1682_prgbank0_r5_r();
	uint8_t vt1682_prgbank1_r0_r();
	uint8_t vt1682_prgbank1_r1_r();

	void vt1682_prgbank1_r0_w(uint8_t data);
	void vt1682_prgbank1_r1_w(uint8_t data);
	void vt1682_prgbank0_r4_w(uint8_t data);
	void vt1682_prgbank0_r5_w(uint8_t data);

	uint8_t vt1682_2118_prgbank1_r4_r5_r();
	void vt1682_2118_prgbank1_r4_r5_w(uint8_t data);

	uint8_t vt1682_210b_misc_cs_prg0_bank_sel_r();
	void vt1682_210b_misc_cs_prg0_bank_sel_w(uint8_t data);

	void vt1682_2105_comr6_tvmodes_w(uint8_t data);

	void vt1682_211c_regs_ext2421_w(uint8_t data);

	uint8_t vt1682_2122_dma_dt_addr_7_0_r();
	void vt1682_2122_dma_dt_addr_7_0_w(uint8_t data);
	uint8_t vt1682_2123_dma_dt_addr_15_8_r();
	void vt1682_2123_dma_dt_addr_15_8_w(uint8_t data);

	uint8_t vt1682_2124_dma_sr_addr_7_0_r();
	void vt1682_2124_dma_sr_addr_7_0_w(uint8_t data);
	uint8_t vt1682_2125_dma_sr_addr_15_8_r();
	void vt1682_2125_dma_sr_addr_15_8_w(uint8_t data);

	uint8_t vt1682_2126_dma_sr_bank_addr_22_15_r();
	void vt1682_2126_dma_sr_bank_addr_22_15_w(uint8_t data);
	uint8_t vt1682_2128_dma_sr_bank_addr_24_23_r();
	void vt1682_2128_dma_sr_bank_addr_24_23_w(uint8_t data);

	uint8_t vt1682_2127_dma_status_r();
	void vt1682_2127_dma_size_trigger_w(uint8_t data);

	uint8_t vt1682_2106_enable_regs_r();
	void vt1682_2106_enable_regs_w(uint8_t data);

	uint8_t vt1682_212c_prng_r();
	void vt1682_212c_prng_seed_w(uint8_t data);

	virtual void clock_joy2();

	uint8_t inteact_212a_send_joy_clock2_r();

	/* Hacky */

	uint8_t soundcpu_irq_vector_hack_r(offs_t offset);
	uint8_t maincpu_irq_vector_hack_r(offs_t offset);
	void vt1682_sound_reset_hack_w(offs_t offset, uint8_t data);
	bool m_scpu_is_in_reset;

	/* System Helpers */

	uint16_t get_dma_sr_addr()
	{
		return ((m_2124_dma_sr_addr_7_0 ) | (m_2125_dma_sr_addr_15_8 << 8)) & 0x7fff;
	}

	void set_dma_sr_addr(uint16_t addr)
	{
		addr &= 0x7fff;

		m_2124_dma_sr_addr_7_0 = addr & 0xff;
		m_2125_dma_sr_addr_15_8 = (m_2125_dma_sr_addr_15_8 & 0x80) | (addr >> 8); // don't change the external flag
	}

	uint16_t get_dma_dt_addr()
	{
		return ((m_2122_dma_dt_addr_7_0 ) | (m_2123_dma_dt_addr_15_8 << 8)) & 0x7fff;
	}

	void set_dma_dt_addr(uint16_t addr)
	{
		m_2122_dma_dt_addr_7_0 = addr & 0xff;
		m_2123_dma_dt_addr_15_8 = (m_2123_dma_dt_addr_15_8 & 0x80) | (addr >> 8); // don't change the external flag
	}

	bool get_dma_sr_isext()
	{
		return m_2125_dma_sr_addr_15_8 & 0x80 ? true : false;
	}

	bool get_dma_dt_isext()
	{
		return m_2123_dma_dt_addr_15_8 & 0x80 ? true : false;
	}

	bool get_dma_dt_is_video()
	{
		if (get_dma_dt_isext())
			return false;

		if (get_dma_dt_addr() == 0x2004)
			return true;

		if (get_dma_dt_addr() == 0x2007)
			return true;

		return false;
	}

	uint16_t get_dma_sr_bank_ddr()
	{
		return ((m_2126_dma_sr_bank_addr_22_15 ) | (m_2128_dma_sr_bank_addr_24_23 << 8)) & 0x3ff;
	}

	void do_dma_external_to_internal(int data, bool is_video);
	void do_dma_internal_to_internal(int data, bool is_video);

	/* Sound CPU Related*/

	uint8_t m_soundcpu_2118_dacleft_7_0;
	uint8_t m_soundcpu_2119_dacleft_15_8;
	uint8_t m_soundcpu_211a_dacright_7_0;
	uint8_t m_soundcpu_211b_dacright_15_8;

	void vt1682_soundcpu_211c_reg_irqctrl_w(uint8_t data);

	uint8_t vt1682_soundcpu_2118_dacleft_7_0_r();
	void vt1682_soundcpu_2118_dacleft_7_0_w(uint8_t data);
	uint8_t vt1682_soundcpu_2119_dacleft_15_8_r();
	void vt1682_soundcpu_2119_dacleft_15_8_w(uint8_t data);
	uint8_t vt1682_soundcpu_211a_dacright_7_0_r();
	void vt1682_soundcpu_211a_dacright_7_0_w(uint8_t data);
	uint8_t vt1682_soundcpu_211b_dacright_15_8_r();
	void vt1682_soundcpu_211b_dacright_15_8_w(uint8_t data);

	/* Support */

	void vt1682_timer_enable_trampoline_w(uint8_t data)
	{
		// this is used for raster interrupt effects, despite not being a scanline timer, so knowing when it triggers is useful, so trampoline it to avoid passing m_screen to the device
		LOGMASKED(LOG_OTHER, "%s: vt1682_timer_enable_trampoline_w: %02x @ position y%d, x%d\n", machine().describe_context(), data, m_screen->vpos(), m_screen->hpos());
		m_system_timer_dev->vt1682_timer_enable_w(data);
	};

	void vt1682_timer_preload_15_8_trampoline_w(uint8_t data)
	{
		LOGMASKED(LOG_OTHER, "%s: vt1682_timer_preload_15_8_trampoline_w: %02x @ position y%d, x%d\n", machine().describe_context(), data, m_screen->vpos(), m_screen->hpos());
		m_system_timer_dev->vt1682_timer_preload_15_8_w(data);
	};


	void update_banks();
	uint8_t translate_prg0select(uint8_t tp20_tp13);
	uint32_t translate_address_4000_to_7fff(uint16_t address);
	uint32_t translate_address_8000_to_ffff(uint16_t address);

	uint8_t rom_4000_to_7fff_r(offs_t offset);
	uint8_t rom_8000_to_ffff_r(offs_t offset);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(line_render_start);

	bitmap_ind8 m_pal2_priority_bitmap;
	bitmap_ind8 m_pal1_priority_bitmap;
	bitmap_ind8 m_pal2_pix_bitmap;
	bitmap_ind8 m_pal1_pix_bitmap;

	void setup_video_pages(int which, int tilesize, int vs, int hs, int y8, int x8, uint16_t* pagebases);
	int get_address_for_tilepos(int x, int y, int tilesize, uint16_t* pagebases);

	void draw_tile_pixline(int segment, int tile, int yy, int x, int y, int palselect, int pal, int is16pix_high, int is16pix_wide, int bpp, int depth, int opaque, int flipx, int flipy, const rectangle& cliprect);
	[[maybe_unused]] void draw_tile(int segment, int tile, int x, int y, int palselect, int pal, int is16pix_high, int is16pix_wide, int bpp, int depth, int opaque, int flipx, int flipy, const rectangle& cliprect);
	void draw_layer(int which, int opaque, const rectangle& cliprect);
	void draw_sprites(const rectangle& cliprect);
};


class intec_interact_state : public vt_vt1682_state
{
public:
	intec_interact_state(const machine_config& mconfig, device_type type, const char* tag) :
		vt_vt1682_state(mconfig, type, tag),
		m_io_p1(*this, "IN0"),
		m_io_p2(*this, "IN1"),
		m_io_p3(*this, "IN2"),
		m_io_p4(*this, "IN3")
	{ }

	void banked_init();

	void intech_interact(machine_config& config);
	void intech_interact_bank(machine_config& config);

	virtual uint8_t porta_r();
	virtual uint8_t portb_r() { return 0x00;/*uint8_t ret = machine().rand() & 0xf; LOGMASKED(LOG_OTHER, "%s: portb_r returning: %1x\n", machine().describe_context(), ret); return ret;*/ };
	virtual uint8_t portc_r();
	virtual uint8_t portd_r() { return 0x00;/*uint8_t ret = machine().rand() & 0xf; LOGMASKED(LOG_OTHER, "%s: portd_r returning: %1x\n", machine().describe_context(), ret); return ret;*/ };

	void porta_w(uint8_t data);
	void portb_w(uint8_t data);
	void portc_w(uint8_t data) { LOGMASKED(LOG_OTHER, "%s: portc_w writing: %1x\n", machine().describe_context(), data & 0xf); };
	void portd_w(uint8_t data) { LOGMASKED(LOG_OTHER, "%s: portd_w writing: %1x\n", machine().describe_context(), data & 0xf); };

	void ext_rombank_w(uint8_t data);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:

	uint8_t m_previous_port_b;
	int m_input_sense;
	int m_input_pos;
	int m_current_bank;

	required_ioport m_io_p1;
	required_ioport m_io_p2;
	required_ioport m_io_p3;
	required_ioport m_io_p4;
};

class vt1682_dance_state : public vt_vt1682_state
{
public:
	vt1682_dance_state(const machine_config& mconfig, device_type type, const char* tag) :
		vt_vt1682_state(mconfig, type, tag),
		m_io_p1(*this, "IN0")
	{ }

	void vt1682_dance(machine_config& config);

protected:
	uint8_t uio_porta_r();
	void uio_porta_w(uint8_t data);

private:
	required_ioport m_io_p1;
};

class vt1682_lxts3_state : public vt_vt1682_state
{
public:
	vt1682_lxts3_state(const machine_config& mconfig, device_type type, const char* tag) :
		vt_vt1682_state(mconfig, type, tag),
		m_io_p1(*this, "IN0")
	{ }

	void vt1682_lxts3(machine_config& config);
	void vt1682_unk1682(machine_config& config);

	void unk1682_init();
	void njp60in1_init();
	void pgs268_init();

protected:
	uint8_t uio_porta_r();

private:
	required_ioport m_io_p1;
};

class vt1682_exsport_state : public vt_vt1682_state
{
public:
	vt1682_exsport_state(const machine_config& mconfig, device_type type, const char* tag) :
		vt_vt1682_state(mconfig, type, tag),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2")
	{ }

	void vt1682_exsport(machine_config& config);
	void vt1682_exsportp(machine_config& config);

	virtual uint8_t uiob_r();
	void uiob_w(uint8_t data);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	int m_old_portb;
	int m_portb_shiftpos = 0;
	int m_p1_latch;
	int m_p2_latch;
	virtual void clock_joy2() override;

	required_ioport m_io_p1;
	required_ioport m_io_p2;
};

class vt1682_wow_state : public vt1682_exsport_state
{
public:
	vt1682_wow_state(const machine_config& mconfig, device_type type, const char* tag) :
		vt1682_exsport_state(mconfig, type, tag)
	{ }

	void vt1682_wow(machine_config& config);

protected:

private:
};


void vt_vt1682_state::video_start()
{
	m_screen->register_screen_bitmap(m_pal2_priority_bitmap);
	m_screen->register_screen_bitmap(m_pal1_priority_bitmap);
	m_screen->register_screen_bitmap(m_pal2_pix_bitmap);
	m_screen->register_screen_bitmap(m_pal1_pix_bitmap);

	m_pal2_priority_bitmap.fill(0xff);
	m_pal1_priority_bitmap.fill(0xff);
	m_pal2_pix_bitmap.fill(0xff);
	m_pal1_pix_bitmap.fill(0xff);
}



void vt_vt1682_state::machine_start()
{
	/* Video */
	save_item(NAME(m_2000));
	save_item(NAME(m_2001));

	save_item(NAME(m_2002_sprramaddr_2_0));
	save_item(NAME(m_2003_sprramaddr_10_3));

	save_item(NAME(m_2005_vramaddr_7_0));
	save_item(NAME(m_2006_vramaddr_15_8));

	save_item(NAME(m_201a_sp_segment_7_0));
	save_item(NAME(m_201b_sp_segment_11_8));
	save_item(NAME(m_segment_7_0_bk));
	save_item(NAME(m_segment_11_8_bk));

	save_item(NAME(m_main_control_bk));

	save_item(NAME(m_scroll_control_bk));

	save_item(NAME(m_xscroll_7_0_bk));
	save_item(NAME(m_yscroll_7_0_bk));

	save_item(NAME(m_200e_blend_pal_sel));
	save_item(NAME(m_200f_bk_pal_sel));

	save_item(NAME(m_2008_lcd_vs_delay));
	save_item(NAME(m_2009_lcd_hs_delay_7_0));
	save_item(NAME(m_200a_lcd_fr_delay_7_0));

	save_item(NAME(m_200d_misc_vregs2));
	save_item(NAME(m_200c_misc_vregs1));
	save_item(NAME(m_200b_misc_vregs0));

	save_item(NAME(m_2018_spregs));
	save_item(NAME(m_2019_bkgain));

	save_item(NAME(m_2020_bk_linescroll));
	save_item(NAME(m_2021_lum_offset));
	save_item(NAME(m_2022_saturation_misc));

	save_item(NAME(m_2023_lightgun_reset));
	save_item(NAME(m_2024_lightgun1_y));
	save_item(NAME(m_2025_lightgun1_x));
	save_item(NAME(m_2026_lightgun2_y));
	save_item(NAME(m_2027_lightgun2_x));

	save_item(NAME(m_2031_red_dac));
	save_item(NAME(m_2032_green_dac));
	save_item(NAME(m_2033_blue_dac));

	save_item(NAME(m_2028));
	save_item(NAME(m_2029));
	save_item(NAME(m_202a));
	save_item(NAME(m_202b));
	save_item(NAME(m_202e));
	save_item(NAME(m_2030));

	/* System */

	save_item(NAME(m_prgbank1_r0));
	save_item(NAME(m_prgbank1_r1));
	save_item(NAME(m_210c_prgbank1_r2));
	save_item(NAME(m_2100_prgbank1_r3));
	save_item(NAME(m_2118_prgbank1_r4_r5));

	save_item(NAME(m_2107_prgbank0_r0));
	save_item(NAME(m_2108_prgbank0_r1));
	save_item(NAME(m_2109_prgbank0_r2));
	save_item(NAME(m_210a_prgbank0_r3));
	save_item(NAME(m_prgbank0_r4));
	save_item(NAME(m_prgbank0_r5));

	save_item(NAME(m_210b_misc_cs_prg0_bank_sel));
	save_item(NAME(m_2105_vt1682_2105_comr6_tvmodes));
	save_item(NAME(m_211c_regs_ext2421));

	save_item(NAME(m_2122_dma_dt_addr_7_0));
	save_item(NAME(m_2123_dma_dt_addr_15_8));
	save_item(NAME(m_2124_dma_sr_addr_7_0));
	save_item(NAME(m_2125_dma_sr_addr_15_8));

	save_item(NAME(m_2126_dma_sr_bank_addr_22_15));
	save_item(NAME(m_2128_dma_sr_bank_addr_24_23));

	save_item(NAME(m_2106_enable_reg));

	/* Sound CPU */

	save_item(NAME(m_soundcpu_2118_dacleft_7_0));
	save_item(NAME(m_soundcpu_2119_dacleft_15_8));
	save_item(NAME(m_soundcpu_211a_dacright_7_0));
	save_item(NAME(m_soundcpu_211b_dacright_15_8));
}

void vt_vt1682_state::machine_reset()
{
	/* Video */
	m_2000 = 0;
	m_2001 = 0;

	m_2002_sprramaddr_2_0 = 0;
	m_2003_sprramaddr_10_3 = 0;

	m_2005_vramaddr_7_0 = 0;
	m_2006_vramaddr_15_8 = 0;

	m_201a_sp_segment_7_0 = 0;
	m_201b_sp_segment_11_8 = 0;
	m_segment_7_0_bk[0] = 0;
	m_segment_11_8_bk[0] = 0;
	m_segment_7_0_bk[1] = 0;
	m_segment_11_8_bk[1] = 0;

	m_main_control_bk[0] = 0;
	m_main_control_bk[1] = 0;

	m_scroll_control_bk[0] = 0;
	m_scroll_control_bk[1] = 0;

	m_xscroll_7_0_bk[0] = 0;
	m_yscroll_7_0_bk[0] = 0;
	m_xscroll_7_0_bk[1] = 0;
	m_yscroll_7_0_bk[1] = 0;

	m_200e_blend_pal_sel = 0;
	m_200f_bk_pal_sel = 0;

	m_2008_lcd_vs_delay = 0;
	m_2009_lcd_hs_delay_7_0 = 0;
	m_200a_lcd_fr_delay_7_0 = 0;

	m_200d_misc_vregs2 = 0;
	m_200c_misc_vregs1 = 0;
	m_200b_misc_vregs0 = 0;

	m_2018_spregs = 0;
	m_2019_bkgain = 0;

	m_2020_bk_linescroll = 0;
	m_2021_lum_offset = 0;
	m_2022_saturation_misc = 0;

	m_2023_lightgun_reset = 0;
	m_2024_lightgun1_y = 0;
	m_2025_lightgun1_x = 0;
	m_2026_lightgun2_y = 0;
	m_2027_lightgun2_x = 0;

	m_2031_red_dac = 0;
	m_2032_green_dac = 0;
	m_2033_blue_dac = 0;

	m_2028 = 0;
	m_2029 = 0;
	m_202a = 0;
	m_202b = 0;
	m_202e = 0;
	m_2030 = 0;

	/* System */
	m_prgbank1_r0 = 0;
	m_prgbank1_r1 = 0;
	m_210c_prgbank1_r2 = 0;
	m_2100_prgbank1_r3 = 0;
	m_2118_prgbank1_r4_r5 = 0;

	m_2107_prgbank0_r0 = 0x3f;
	m_2108_prgbank0_r1 = 0;
	m_2109_prgbank0_r2 = 0;
	m_210a_prgbank0_r3 = 0;
	m_prgbank0_r4 = 0;
	m_prgbank0_r5 = 0;

	m_210b_misc_cs_prg0_bank_sel = 0;
	m_2105_vt1682_2105_comr6_tvmodes = 0;
	m_211c_regs_ext2421 = 0;

	m_2122_dma_dt_addr_7_0 = 0;
	m_2123_dma_dt_addr_15_8 = 0;

	m_2124_dma_sr_addr_7_0 = 0;
	m_2125_dma_sr_addr_15_8 = 0;

	m_2126_dma_sr_bank_addr_22_15 = 0;
	m_2128_dma_sr_bank_addr_24_23 = 0;

	m_2106_enable_reg = 0;

	/* Sound CPU */

	m_soundcpu_2118_dacleft_7_0 = 0;
	m_soundcpu_2119_dacleft_15_8 = 0;
	m_soundcpu_211a_dacright_7_0 = 0;
	m_soundcpu_211b_dacright_15_8 = 0;

	/* Misc */

	update_banks();

	m_bank->set_entry(0);

	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_scpu_is_in_reset = true;
}

/*

Address translation

----------------------------------------------------------------

First table uses bits from PB0r0, PB0r1, PB0r2 (0x8000 and above) or PB0r4, PB0r5 (below 0x8000)

PB0r0 = Program Bank 0 Register 0
PB0r1 = Program Bank 0 Register 1
PB0r2 = Program Bank 0 Register 2

PB0r4 = Program Bank 0 Register 4
PB0r5 = Program Bank 0 Register 5

PQ2EN   COMR6   A:15    A:14    A:13    |   TP:20   TP:19   TP:18   TP:17   TP:16   TP:15   TP:14   TP:13
-----------------------------------------------------------------------------------------------------------
0       0       1       0       0       |   PB0r0:7 PB0r0:6 PB0r0:5 PB0r0:4 PB0r0:3 PB0r0:2 PB0r0:1 PB0r0:0   (all PB0r0)
0       0       1       0       1       |   PB0r1:7 PB0r1:6 PB0r1:5 PB0r1:4 PB0r1:3 PB0r1:2 PB0r1:1 PB0r1:0   (all PB0r1)
0       0       1       1       0       |   1       1       1       1       1       1       1       0
0       0       1       1       1       |   1       1       1       1       1       1       1       1
0       1       1       0       0       |   1       1       1       1       1       1       1       0
0       1       1       0       1       |   PB0r1:7 PB0r1:6 PB0r1:5 PB0r1:4 PB0r1:3 PB0r1:2 PB0r1:1 PB0r1:0   (all PB0r1)
0       1       1       1       0       |   PB0r0:7 PB0r0:6 PB0r0:5 PB0r0:4 PB0r0:3 PB0r0:2 PB0r0:1 PB0r0:0   (all PB0r0)
0       1       1       1       1       |   1       1       1       1       1       1       1       1
1       0       1       0       0       |   PB0r0:7 PB0r0:6 PB0r0:5 PB0r0:4 PB0r0:3 PB0r0:2 PB0r0:1 PB0r0:0   (all PB0r0)
1       0       1       0       1       |   PB0r1:7 PB0r1:6 PB0r1:5 PB0r1:4 PB0r1:3 PB0r1:2 PB0r1:1 PB0r1:0   (all PB0r1)
1       0       1       1       0       |   PB0r2:7 PB0r2:6 PB0r2:5 PB0r2:4 PB0r2:3 PB0r2:2 PB0r2:1 PB0r2:0   (all PB0r2)
1       0       1       1       1       |   1       1       1       1       1       1       1       1
1       1       1       0       0       |   PB0r2:7 PB0r2:6 PB0r2:5 PB0r2:4 PB0r2:3 PB0r2:2 PB0r2:1 PB0r2:0   (all PB0r2)
1       1       1       0       1       |   PB0r1:7 PB0r1:6 PB0r1:5 PB0r1:4 PB0r1:3 PB0r1:2 PB0r1:1 PB0r1:0   (all PB0r1)
1       1       1       1       0       |   PB0r0:7 PB0r0:6 PB0r0:5 PB0r0:4 PB0r0:3 PB0r0:2 PB0r0:1 PB0r0:0   (all PB0r0)
1       1       1       1       1       |   1       1       1       1       1       1       1       1
-----------------------------------------------------------------------------------------------------------
-       -       0       1       1       |   PB0r5:7 PB0r5:6 PB0r5:5 PB0r5:4 PB0r5:3 PB0r5:2 PB0r5:1 PB0r5:0   (all PB0r5)
-       -       0       1       0       |   PB0r4:7 PB0r4:6 PB0r4:5 PB0r4:4 PB0r4:3 PB0r4:2 PB0r4:1 PB0r4:0   (all PB0r4)

----------------------------------------------------------------

second table uses bits from above, and PB0r3

Program Bank 0 Select   |   PA:20   PA:19   PA:18   PA:17   PA:16   PA:15   PA:14   PA:13
-------------------------------------------------------------------------------------------
0       0       0       |   PB0r3:7 PB0r3:6 TP:18   TP:17   TP:16   TP:15   TP:14   TP:13
0       0       1       |   PB0r3:7 PB0r3:6 PB0r3:5 TP:17   TP:16   TP:15   TP:14   TP:13
0       1       0       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 TP:16   TP:15   TP:14   TP:13
0       1       1       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 PB0r3:3 TP:15   TP:14   TP:13
1       0       0       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 PB0r3:3 PB0r3:2 TP:14   TP:13
1       0       1       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 PB0r3:3 PB0r3:2 PB0r3:1 TP:13
1       1       0       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 PB0r3:3 PB0r3:2 PB0r3:1 PB0r3:0
1       1       1       |   TP:20   TP:19   TP:18   TP:17   TP:16   TP:15   TP:14   TP:13

PB0r3 = Program Bank 0 Register 3
TP = Address translated by 1st table

----------------------------------------------------------------

third table uses bits from PB1r0, PB1r1, PB1r2, PB1r3 (0x8000 and above) or PB1r4, PB1r5 (below 0x8000)

PB1r0 = Program Bank 1 Register 0
PB1r1 = Program Bank 1 Register 1
PB1r2 = Program Bank 1 Register 2
PB1r3 = Program Bank 1 Register 3

PB1r4 = Program Bank 1 Register 4
PB1r5 = Program Bank 1 Register 5

EXT2421 PQ2EN   COMR6   A:15    A:14    A:13    |   PA:24   PA:23   PA:22   PA:21
------------------------------------------------------------------------------------
1       -       -       1       -       -       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
------------------------------------------------------------------------------------
0       0       0       1       0       0       |   PB1r0:3 PB1r0:2 PB1r0:1 PB1r0:0    (all PB1r0)
0       0       0       1       0       1       |   PB1r1:3 PB1r1:2 PB1r1:1 PB1r1:0    (all PB1r1)
0       0       0       1       1       0       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       0       0       1       1       1       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       0       1       1       0       0       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       0       1       1       0       1       |   PB1r1:3 PB1r1:2 PB1r1:1 PB1r1:0    (all PB1r1)
0       0       1       1       1       0       |   PB1r0:3 PB1r0:2 PB1r0:1 PB1r0:0    (all PB1r0)
0       0       1       1       1       1       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       1       0       1       0       0       |   PB1r0:3 PB1r0:2 PB1r0:1 PB1r0:0    (all PB1r0)
0       1       0       1       0       1       |   PB1r1:3 PB1r1:2 PB1r1:1 PB1r1:0    (all PB1r1)
0       1       0       1       1       0       |   PB1r2:3 PB1r2:2 PB1r2:1 PB1r2:0    (all PB1r2)
0       1       0       1       1       1       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       1       1       1       0       0       |   PB1r2:3 PB1r2:2 PB1r2:1 PB1r2:0    (all PB1r2)
0       1       1       1       0       1       |   PB1r1:3 PB1r1:2 PB1r1:1 PB1r1:0    (all PB1r1)
0       1       1       1       1       0       |   PB1r0:3 PB1r0:2 PB1r0:1 PB1r0:0    (all PB1r0)
0       1       1       1       1       1       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
------------------------------------------------------------------------------------
-       -       -       0       1       1       |   PB1r5:3 PB1r5:2 PB1r5:1 PB1r5:0    (all PB1r5)
-       -       -       0       1       0       |   PB1r4:3 PB1r4:2 PB1r4:1 PB1r4:0    (all PB1r4)


*/
void vt_vt1682_state::update_banks()
{
	/* must use

	m_prgbank1_r0
	m_prgbank1_r1
	m_210c_prgbank1_r2
	m_2100_prgbank1_r3
	m_2118_prgbank1_r4_r5

	m_2107_prgbank0_r0
	m_2108_prgbank0_r1
	m_2109_prgbank0_r2
	m_210a_prgbank0_r3
	m_prgbank0_r4
	m_prgbank0_r5

	m_2105_vt1682_2105_comr6_tvmodes
	m_211c_regs_ext2421
	m_210b_misc_cs_prg0_bank_sel

	everything that changes these calls here, so if we wanted to do this with actual
	banks then here would be the place

	*/
}

uint8_t vt_vt1682_state::translate_prg0select(uint8_t tp20_tp13)
{
	uint8_t bank = m_210b_misc_cs_prg0_bank_sel & 0x07;

	uint8_t ret = 0x00;

	switch (bank)
	{
	case 0x0: ret = (m_210a_prgbank0_r3 & 0xc0) | (tp20_tp13 & 0x3f); break;
	case 0x1: ret = (m_210a_prgbank0_r3 & 0xe0) | (tp20_tp13 & 0x1f); break;
	case 0x2: ret = (m_210a_prgbank0_r3 & 0xf0) | (tp20_tp13 & 0x0f); break;
	case 0x3: ret = (m_210a_prgbank0_r3 & 0xf8) | (tp20_tp13 & 0x07); break;
	case 0x4: ret = (m_210a_prgbank0_r3 & 0xfc) | (tp20_tp13 & 0x03); break;
	case 0x5: ret = (m_210a_prgbank0_r3 & 0xfe) | (tp20_tp13 & 0x01); break;
	case 0x6: ret = m_210a_prgbank0_r3; break;
	case 0x7: ret = tp20_tp13;  break;
	}

	return ret;
}

uint32_t vt_vt1682_state::translate_address_4000_to_7fff(uint16_t address)
{
	uint32_t realaddress = 0x00000000;

	uint8_t prgbank1_r4 = (m_2118_prgbank1_r4_r5 & 0x0f);
	uint8_t prgbank1_r5 = (m_2118_prgbank1_r4_r5 & 0xf0)>>4;

	int tp20_tp13 = 0;
	int pa24_pa21 = 0;

	switch (address & 0x6000)
	{
	case 0x4000:
		tp20_tp13 = m_prgbank0_r4;
		pa24_pa21 = prgbank1_r4;
		break;

	case 0x6000:
		tp20_tp13 = m_prgbank0_r5;
		pa24_pa21 = prgbank1_r5;
		break;

	// invalid cases
	default:
	case 0x0000:
	case 0x2000:
		break;

	}

	int pa20_pa13 = translate_prg0select(tp20_tp13);

	realaddress = address & 0x1fff;
	realaddress |= pa20_pa13 << 13;
	realaddress |= pa24_pa21 << 21;

	return realaddress;
}

uint32_t vt_vt1682_state::translate_address_8000_to_ffff(uint16_t address)
{
	uint32_t realaddress = 0x00000000;

	int tp20_tp13 = 0;
	int pa24_pa21 = 0;

	const int pq2en = (m_210b_misc_cs_prg0_bank_sel & 0x40)>>6;
	const int comr6 = (m_2105_vt1682_2105_comr6_tvmodes & 0x40)>>6;
	const int a14_a13 = (address & 0x6000) >> 13;
	const int lookup = a14_a13 | (comr6 << 2) | (pq2en << 3);

	switch (lookup)
	{
	// PQ2EN disabled, COMR6 disabled (0,1,2,3 order)
	case 0x0: tp20_tp13 = m_2107_prgbank0_r0;   pa24_pa21 = m_prgbank1_r0;      break;
	case 0x1: tp20_tp13 = m_2108_prgbank0_r1;   pa24_pa21 = m_prgbank1_r1;      break;
	case 0x2: tp20_tp13 = 0xfe;                 pa24_pa21 = m_2100_prgbank1_r3; break;
	case 0x3: tp20_tp13 = 0xff;                 pa24_pa21 = m_2100_prgbank1_r3; break;
	// PQ2EN disabled, COMR6 enabled (2,1,0,3 order)
	case 0x4: tp20_tp13 = 0xfe;                 pa24_pa21 = m_2100_prgbank1_r3; break;
	case 0x5: tp20_tp13 = m_2108_prgbank0_r1;   pa24_pa21 = m_prgbank1_r1;      break;
	case 0x6: tp20_tp13 = m_2107_prgbank0_r0;   pa24_pa21 = m_prgbank1_r0;      break;
	case 0x7: tp20_tp13 = 0xff;                 pa24_pa21 = m_2100_prgbank1_r3; break;
	// PQ2EN enabled, COMR6 disabled (0,1,2,3 order) (2 is now m_2109_prgbank0_r2, not 0xfe)
	case 0x8: tp20_tp13 = m_2107_prgbank0_r0;   pa24_pa21 = m_prgbank1_r0;      break;
	case 0x9: tp20_tp13 = m_2108_prgbank0_r1;   pa24_pa21 = m_prgbank1_r1;      break;
	case 0xa: tp20_tp13 = m_2109_prgbank0_r2;   pa24_pa21 = m_210c_prgbank1_r2; break;
	case 0xb: tp20_tp13 = 0xff;                 pa24_pa21 = m_2100_prgbank1_r3; break;
	// PQ2EN enabled, COMR6 enabled (2,1,0,3 order) (2 is now m_2109_prgbank0_r2, not 0xfe)
	case 0xc: tp20_tp13 = m_2109_prgbank0_r2;   pa24_pa21 = m_210c_prgbank1_r2; break;
	case 0xd: tp20_tp13 = m_2108_prgbank0_r1;   pa24_pa21 = m_prgbank1_r1;      break;
	case 0xe: tp20_tp13 = m_2107_prgbank0_r0;   pa24_pa21 = m_prgbank1_r0;      break;
	case 0xf: tp20_tp13 = 0xff;                 pa24_pa21 = m_2100_prgbank1_r3; break;
	}

	// override selection above
	const int ext2421 = (m_211c_regs_ext2421 & 0x20) >> 5;
	if (ext2421)
	{
		pa24_pa21 = m_2100_prgbank1_r3;
	}

	const int pa20_pa13 = translate_prg0select(tp20_tp13);

	realaddress = address & 0x1fff;
	realaddress |= pa20_pa13 << 13;
	realaddress |= pa24_pa21 << 21;

	return realaddress;
}

uint8_t vt_vt1682_state::rom_4000_to_7fff_r(offs_t offset)
{
	const uint32_t address = translate_address_4000_to_7fff(offset + 0x4000);
	return m_fullrom->read8(address);
}

uint8_t vt_vt1682_state::rom_8000_to_ffff_r(offs_t offset)
{
	const uint32_t address = translate_address_8000_to_ffff(offset + 0x8000);
	return m_fullrom->read8(address);
}

/************************************************************************************************************************************
 VT1682 PPU Registers
************************************************************************************************************************************/

/*
    Address 0x2000 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - Capture
    0x08 - SLAVE
    0x04 - (unused)
    0x02 - (unused)
    0x01 - NMI_EN
*/

uint8_t vt_vt1682_state::vt1682_2000_r()
{
	uint8_t ret = m_2000;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2000_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2000_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2000_w writing: %02x (Capture:%1x Slave:%1x NMI_Enable:%1x)\n", machine().describe_context(), data, (data & 0x10)>>4, (data & 0x08)>>3, (data & 0x01)>>0 );
	m_2000 = data;
}

/*
    Address 0x2001 READ (MAIN CPU)

    0x80 - VBLANK
    0x40 - SP ERR
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - (unused)
    0x01 - (unused)

    Address 0x2001 WRITE (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - EXT CLK DIV
    0x04 - EXT CLK DIV
    0x02 - SP INI (blank sprites on left 8 pixels)
    0x01 - BK INI (blank bg on left 8 pixels)
*/

uint8_t vt_vt1682_state::vt1682_2001_vblank_r()
{
	uint8_t ret = 0x00;

	int sp_err = 0; // too many sprites per lien
	int vblank = m_screen->vpos() > 239 ? 1 : 0; // in vblank, the pinball game in miwi2_16 under 'drum master' requires this to become set before the VBL interrupt fires

	ret |= sp_err << 6;
	ret |= vblank << 7;

	LOGMASKED(LOG_OTHER, "%s: vt1682_2001_vblank_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2001_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2001_w writing: %02x (ext_clk_div:%1x sp_ini:%1x bk_ini:%1x)\n", machine().describe_context(), data,
		(data & 0x0c) >> 2, (data & 0x02) >> 1, (data & 0x01) >> 0);

	m_2001 = data;
}


/*
    Address 0x2002 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - SPRAM ADDR:2
    0x02 - SPRAM ADDR:1
    0x01 - SPRAM ADDR:0
*/

uint8_t vt_vt1682_state::vt1682_2002_sprramaddr_2_0_r()
{
	uint8_t ret = m_2002_sprramaddr_2_0;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2002_sprramaddr_2_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2002_sprramaddr_2_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2002_sprramaddr_2_0_w writing: %02x\n", machine().describe_context(), data);
	m_2002_sprramaddr_2_0 = data & 0x07;
}

/*
    Address 0x2003 r/w (MAIN CPU)

    0x80 - SPRAM ADDR:10
    0x40 - SPRAM ADDR:9
    0x20 - SPRAM ADDR:8
    0x10 - SPRAM ADDR:7
    0x08 - SPRAM ADDR:6
    0x04 - SPRAM ADDR:5
    0x02 - SPRAM ADDR:4
    0x01 - SPRAM ADDR:3
*/

uint8_t vt_vt1682_state::vt1682_2003_sprramaddr_10_3_r()
{
	uint8_t ret = m_2003_sprramaddr_10_3;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2003_sprramaddr_10_3_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2003_sprramaddr_10_3_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2003_sprramaddr_10_3_w writing: %02x\n", machine().describe_context(), data);
	m_2003_sprramaddr_10_3 = data;
}

/*
    Address 0x2004 r/w (MAIN CPU)

    0x80 - SPRAM DATA:7
    0x40 - SPRAM DATA:6
    0x20 - SPRAM DATA:5
    0x10 - SPRAM DATA:4
    0x08 - SPRAM DATA:3
    0x04 - SPRAM DATA:2
    0x02 - SPRAM DATA:1
    0x01 - SPRAM DATA:0
*/

uint8_t vt_vt1682_state::vt1682_2004_sprram_data_r()
{
	uint16_t spriteram_address = get_spriteram_addr();
	uint8_t ret = m_spriteram->read8(spriteram_address);
	LOGMASKED(LOG_OTHER, "%s: vt1682_2004_sprram_data_r returning: %02x from SpriteRam Address %04x\n", machine().describe_context(), ret, spriteram_address);
	// no increment on read?
	// documentation indicates this doesn't work
	return ret;
}

void vt_vt1682_state::vt1682_2004_sprram_data_w(uint8_t data)
{
	uint16_t spriteram_address = get_spriteram_addr();
	m_spriteram->write8(spriteram_address, data);

	LOGMASKED(LOG_SRAM_WRITES, "%s: vt1682_2004_sprram_data_w writing: %02x to SpriteRam Address %04x\n", machine().describe_context(), data, spriteram_address);
	inc_spriteram_addr();
}


/*
    Address 0x2005 r/w (MAIN CPU)

    0x80 - VRAM ADDR:7
    0x40 - VRAM ADDR:6
    0x20 - VRAM ADDR:5
    0x10 - VRAM ADDR:4
    0x08 - VRAM ADDR:3
    0x04 - VRAM ADDR:2
    0x02 - VRAM ADDR:1
    0x01 - VRAM ADDR:0
*/

uint8_t vt_vt1682_state::vt1682_2005_vramaddr_7_0_r()
{
	uint8_t ret = m_2005_vramaddr_7_0;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2005_vramaddr_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2005_vramaddr_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2005_vramaddr_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_2005_vramaddr_7_0 = data;
}

/*
    Address 0x2006 r/w (MAIN CPU)

    0x80 - VRAM ADDR:15
    0x40 - VRAM ADDR:14
    0x20 - VRAM ADDR:13
    0x10 - VRAM ADDR:12
    0x08 - VRAM ADDR:11
    0x04 - VRAM ADDR:10
    0x02 - VRAM ADDR:9
    0x01 - VRAM ADDR:8
*/

uint8_t vt_vt1682_state::vt1682_2006_vramaddr_15_8_r()
{
	uint8_t ret = m_2006_vramaddr_15_8;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2006_vramaddr_15_8 returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2006_vramaddr_15_8_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2006_vramaddr_15_8 writing: %02x\n", machine().describe_context(), data);
	m_2006_vramaddr_15_8 = data;
}


/*
    Address 0x2007 r/w (MAIN CPU)

    0x80 - VRAM DATA:7
    0x40 - VRAM DATA:6
    0x20 - VRAM DATA:5
    0x10 - VRAM DATA:4
    0x08 - VRAM DATA:3
    0x04 - VRAM DATA:2
    0x02 - VRAM DATA:1
    0x01 - VRAM DATA:0
*/

uint8_t vt_vt1682_state::vt1682_2007_vram_data_r()
{
	uint16_t vram_address = get_vram_addr();
	uint8_t ret = m_vram->read8(vram_address);
	LOGMASKED(LOG_OTHER, "%s: vt1682_2007_vram_data_r returning: %02x from VideoRam Address %04x\n", machine().describe_context(), ret, vram_address);
	// no increment on read?
	// documentation indicates this doesn't work
	return ret;
}

void vt_vt1682_state::vt1682_2007_vram_data_w(uint8_t data)
{
	uint16_t vram_address = get_vram_addr();
	m_vram->write8(vram_address, data);
	LOGMASKED(LOG_VRAM_WRITES, "%s: vt1682_2007_vram_data_w writing: %02x to VideoRam Address %04x\n", machine().describe_context(), data, vram_address);
	vram_address++; // auto inc
	set_vram_addr(vram_address); // update registers
}


/*
    Address 0x2008 r/w (MAIN CPU)

    0x80 - LCD VS DELAY
    0x40 - LCD VS DELAY
    0x20 - LCD VS DELAY
    0x10 - LCD VS DELAY
    0x08 - LCD VS DELAY
    0x04 - LCD VS DELAY
    0x02 - LCD VS DELAY
    0x01 - LCD VS DELAY
*/

uint8_t vt_vt1682_state::vt1682_2008_lcd_vs_delay_r()
{
	uint8_t ret = m_2008_lcd_vs_delay;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2008_lcd_vs_delay_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2008_lcd_vs_delay_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2008_lcd_vs_delay_w writing: %02x\n", machine().describe_context(), data);
	m_2008_lcd_vs_delay = data;
}

/*
    Address 0x2009 r/w (MAIN CPU)

    0x80 - LCD HS DELAY:7
    0x40 - LCD HS DELAY:6
    0x20 - LCD HS DELAY:5
    0x10 - LCD HS DELAY:4
    0x08 - LCD HS DELAY:3
    0x04 - LCD HS DELAY:2
    0x02 - LCD HS DELAY:1
    0x01 - LCD HS DELAY:0
*/

uint8_t vt_vt1682_state::vt1682_2009_lcd_hs_delay_7_0_r()
{
	uint8_t ret = m_2009_lcd_hs_delay_7_0;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2009_lcd_hs_delay_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2009_lcd_hs_delay_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2009_lcd_hs_delay_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_2009_lcd_hs_delay_7_0 = data;
}

/*
    Address 0x200a r/w (MAIN CPU)

    0x80 - LCD FR DELAY:7
    0x40 - LCD FR DELAY:6
    0x20 - LCD FR DELAY:5
    0x10 - LCD FR DELAY:4
    0x08 - LCD FR DELAY:3
    0x04 - LCD FR DELAY:2
    0x02 - LCD FR DELAY:1
    0x01 - LCD FR DELAY:0
*/

uint8_t vt_vt1682_state::vt1682_200a_lcd_fr_delay_7_0_r()
{
	uint8_t ret = m_200a_lcd_fr_delay_7_0;
	LOGMASKED(LOG_OTHER, "%s: vt1682_200a_lcd_fr_delay_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_200a_lcd_fr_delay_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_200a_lcd_fr_delay_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_200a_lcd_fr_delay_7_0 = data;
}


/*
    Address 0x200b r/w (MAIN CPU)

    0x80 - CH2 Odd Line Colour
    0x40 - CH2 Odd Line Colour
    0x20 - CH2 Even Line Colour
    0x10 - CH2 Even Line Colour
    0x08 - CH2 SEL
    0x04 - CH2 REV
    0x02 - LCD FR:8
    0x01 - LCD HS:8
*/

uint8_t vt_vt1682_state::vt1682_200b_misc_vregs0_r()
{
	uint8_t ret = m_200b_misc_vregs0;
	LOGMASKED(LOG_OTHER, "%s: vt1682_200b_misc_vregs0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_200b_misc_vregs0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_200b_misc_vregs0_w writing: %02x\n", machine().describe_context(), data);
	m_200b_misc_vregs0 = data;
}

/*
    Address 0x200c r/w (MAIN CPU)

    0x80 - FRate
    0x40 - DotODR
    0x20 - LCD CLOCK
    0x10 - LCD CLOCK
    0x08 - UPS 052
    0x04 - Field AC
    0x02 - LCD MODE
    0x01 - LCD MODE
*/

uint8_t vt_vt1682_state::vt1682_200c_misc_vregs1_r()
{
	uint8_t ret = m_200c_misc_vregs1;
	LOGMASKED(LOG_OTHER, "%s: vt1682_200c_misc_vregs1_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_200c_misc_vregs1_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_200c_misc_vregs1_w writing: %02x\n", machine().describe_context(), data);
	m_200c_misc_vregs1 = data;
}

/*
    Address 0x200d r/w (MAIN CPU)

    0x80 - LCD ENABLE
    0x40 - Dot 240
    0x20 - Reverse
    0x10 - Vcom
    0x08 - Odd Line Color
    0x04 - Odd Line Color
    0x02 - Even Line Color
    0x01 - Even Line Color
*/

uint8_t vt_vt1682_state::vt1682_200d_misc_vregs2_r()
{
	uint8_t ret = m_200d_misc_vregs2;
	LOGMASKED(LOG_OTHER, "%s: vt1682_200d_misc_vregs2_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_200d_misc_vregs2_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_200d_misc_vregs2_w writing: %02x\n", machine().describe_context(), data);
	m_200d_misc_vregs2 = data;
}


/*
    Address 0x200e r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Blend2 - LCD output blending  0 = Overlapped (use depth) 1 = 50% blend Pal1/Pal2
    0x10 - Blend1 - TV output blending   0 = Overlapped (use depth) 1 = 50% blend Pal1/Pal2
    0x08 - Palette 2 Out Sel 'SB4' \
    0x04 - Palette 2 Out Sel 'SB6' /- 0 = output Palette 2 Disable, 1 = output Palette 2 to LCD only, 2 = Output Palette 2 to TV only, 3 = Output Palette 2 to both
    0x02 - Palette 1 Out Sel 'SB3' \
    0x01 - Palette 1 Out Sel 'SB5' /- 0 = output Palette 1 Disable, 1 = output Palette 1 to LCD only, 2 = Output Palette 1 to TV only, 3 = Output Palette 1 to both
*/

uint8_t vt_vt1682_state::vt1682_200e_blend_pal_sel_r()
{
	uint8_t ret = m_200e_blend_pal_sel;
	LOGMASKED(LOG_OTHER, "%s: vt1682_200e_blend_pal_sel_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_200e_blend_pal_sel_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_200e_blend_pal_sel_w writing: %02x\n", machine().describe_context(), data);
	m_200e_blend_pal_sel = data;
}

/*
    Address 0x200f r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Bk2 Palette Select 'BK2 SB2'
    0x04 - Bk2 Palette Select 'BK2 SB1'
    0x02 - Bk1 Palette Select 'BK1 SB2'
    0x01 - Bk1 Palette Select 'BK1 SB1'
*/

uint8_t vt_vt1682_state::vt1682_200f_bk_pal_sel_r()
{
	uint8_t ret = m_200f_bk_pal_sel;
	LOGMASKED(LOG_OTHER, "%s: vt1682_200f_bk_pal_sel_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_200f_bk_pal_sel_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_200f_bk_pal_sel_w writing: %02x\n", machine().describe_context(), data);
	m_200f_bk_pal_sel = data;
}

/*
    Address 0x2010 r/w (MAIN CPU)

    0x80 - BK1X:7
    0x40 - BK1X:6
    0x20 - BK1X:5
    0x10 - BK1X:4
    0x08 - BK1X:3
    0x04 - BK1X:2
    0x02 - BK1X:1
    0x01 - BK1X:0
*/

uint8_t vt_vt1682_state::vt1682_2010_bk1_xscroll_7_0_r()
{
	uint8_t ret = m_xscroll_7_0_bk[0];
	LOGMASKED(LOG_OTHER, "%s: vt1682_2010_bk1_xscroll_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2010_bk1_xscroll_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2010_bk1_xscroll_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_xscroll_7_0_bk[0] = data;
}

/*
    Address 0x2011 r/w (MAIN CPU)

    0x80 - BK1Y:7
    0x40 - BK1Y:6
    0x20 - BK1Y:5
    0x10 - BK1Y:4
    0x08 - BK1Y:3
    0x04 - BK1Y:2
    0x02 - BK1Y:1
    0x01 - BK1Y:0
*/

uint8_t vt_vt1682_state::vt1682_2011_bk1_yscoll_7_0_r()
{
	uint8_t ret = m_yscroll_7_0_bk[0];
	LOGMASKED(LOG_OTHER, "%s: vt1682_2011_bk1_yscoll_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2011_bk1_yscoll_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2011_bk1_yscoll_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_yscroll_7_0_bk[0] = data;
}


/*
    Address 0x2012 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - BK1 HCLR
    0x08 - BK1 Scroll Enable (page layout)
    0x04 - BK1 Scroll Enable (page layout)
    0x02 - BK1Y:8
    0x01 - BK1X:8
*/

uint8_t vt_vt1682_state::vt1682_2012_bk1_scroll_control_r()
{
	uint8_t ret = m_scroll_control_bk[0];
	LOGMASKED(LOG_OTHER, "%s: vt1682_2012_bk1_scroll_control_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}


void vt_vt1682_state::vt1682_2012_bk1_scroll_control_w(uint8_t data)
{

	LOGMASKED(LOG_OTHER, "%s: vt1682_2012_bk1_scroll_control_w writing: %02x (hclr: %1x page_layout:%1x ymsb:%1x xmsb:%1x)\n", machine().describe_context(), data,
		(data & 0x10) >> 4, (data & 0x0c) >> 2, (data & 0x02) >> 1, (data & 0x01) >> 0);

	m_scroll_control_bk[0] = data;
}


/*
    Address 0x2013 r/w (MAIN CPU)

    0x80 - BK1 Enable
    0x40 - BK1 Palette
    0x20 - BK1 Depth
    0x10 - BK1 Depth
    0x08 - BK1 Colour (bpp)
    0x04 - BK1 Colour (bpp)
    0x02 - BK1 Line
    0x01 - BK1 Size
*/

uint8_t vt_vt1682_state::vt1682_2013_bk1_main_control_r()
{
	uint8_t ret = m_main_control_bk[0];
	LOGMASKED(LOG_OTHER, "%s: vt1682_2013_bk1_main_control_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2013_bk1_main_control_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2013_bk1_main_control_w writing: %02x (enable:%01x palette:%01x depth:%01x bpp:%01x linemode:%01x tilesize:%01x)\n", machine().describe_context(), data,
		(data & 0x80) >> 7, (data & 0x40) >> 6, (data & 0x30) >> 4, (data & 0x0c) >> 2, (data & 0x02) >> 1, (data & 0x01) >> 0 );

	m_main_control_bk[0] = data;
}

/*
    Address 0x2014 r/w (MAIN CPU)

    0x80 - BK2X:7
    0x40 - BK2X:6
    0x20 - BK2X:5
    0x10 - BK2X:4
    0x08 - BK2X:3
    0x04 - BK2X:2
    0x02 - BK2X:1
    0x01 - BK2X:0
*/


uint8_t vt_vt1682_state::vt1682_2014_bk2_xscroll_7_0_r()
{
	uint8_t ret = m_xscroll_7_0_bk[1];
	LOGMASKED(LOG_OTHER, "%s: vt1682_2014_bk2_xscroll_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2014_bk2_xscroll_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2014_bk2_xscroll_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_xscroll_7_0_bk[1] = data;
}

/*
    Address 0x2015 r/w (MAIN CPU)

    0x80 - BK2Y:7
    0x40 - BK2Y:6
    0x20 - BK2Y:5
    0x10 - BK2Y:4
    0x08 - BK2Y:3
    0x04 - BK2Y:2
    0x02 - BK2Y:1
    0x01 - BK2Y:0
*/

uint8_t vt_vt1682_state::vt1682_2015_bk2_yscoll_7_0_r()
{
	uint8_t ret = m_yscroll_7_0_bk[1];
	LOGMASKED(LOG_OTHER, "%s: vt1682_2015_bk2_yscoll_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2015_bk2_yscoll_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2015_bk2_yscoll_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_yscroll_7_0_bk[1] = data;
}


/*
    Address 0x2016 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK2 Scroll Enable (page layout)
    0x04 - BK2 Scroll Enable (page layout)
    0x02 - BK2Y:8
    0x01 - BK2X:8
*/

uint8_t vt_vt1682_state::vt1682_2016_bk2_scroll_control_r()
{
	uint8_t ret = m_scroll_control_bk[1];
	LOGMASKED(LOG_OTHER, "%s: vt1682_2016_bk2_scroll_control_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}


void vt_vt1682_state::vt1682_2016_bk2_scroll_control_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2016_bk2_scroll_control_w writing: %02x ((invalid): %1x page_layout:%1x ymsb:%1x xmsb:%1x)\n", machine().describe_context(), data,
		(data & 0x10) >> 4, (data & 0x0c) >> 2, (data & 0x02) >> 1, (data & 0x01) >> 0);

	m_scroll_control_bk[1] = data;
}


/*
    Address 0x2017 r/w (MAIN CPU)

    0x80 - BK2 Enable
    0x40 - BK2 Palette
    0x20 - BK2 Depth
    0x10 - BK2 Depth
    0x08 - BK2 Colour (bpp)
    0x04 - BK2 Colour (bpp)
    0x02 - (unused)
    0x01 - BK2 Size
*/

uint8_t vt_vt1682_state::vt1682_2017_bk2_main_control_r()
{
	uint8_t ret = m_main_control_bk[1];
	LOGMASKED(LOG_OTHER, "%s: vt1682_2017_bk2_main_control_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2017_bk2_main_control_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2017_bk2_main_control_w writing: %02x (enable:%01x palette:%01x depth:%01x bpp:%01x (invalid):%01x tilesize:%01x)\n", machine().describe_context(), data,
		(data & 0x80) >> 7, (data & 0x40) >> 6, (data & 0x30) >> 4, (data & 0x0c) >> 2, (data & 0x02) >> 1, (data & 0x01) >> 0 );

	m_main_control_bk[1] = data;
}


/*
    Address 0x2018 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - SP PALSEL
    0x04 - SP ENABLE
    0x02 - SP SIZE
    0x01 - SP SIZE
*/

uint8_t vt_vt1682_state::vt1682_2018_spregs_r()
{
	uint8_t ret = m_2018_spregs;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2018_spregs_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2018_spregs_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2018_spregs_w writing: %02x\n", machine().describe_context(), data);
	m_2018_spregs = data;
}

/*
    Address 0x2019 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK2 Gain (vertical zoom 0 = 1x, 1= 1x, 2= 1.5x, 3= 2x)
    0x04 - BK2 Gain
    0x02 - BK1 Gain (same but for BK1)
    0x01 - BK1 Gain
*/

uint8_t vt_vt1682_state::vt1682_2019_bkgain_r()
{
	uint8_t ret = m_2019_bkgain;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2019_bkgain_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2019_bkgain_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2019_bkgain_w writing: %02x\n", machine().describe_context(), data);
	m_2019_bkgain = data;
}


/*
    Address 0x201a r/w (MAIN CPU)

    0x80 - SP SEGMENT:7
    0x40 - SP SEGMENT:6
    0x20 - SP SEGMENT:5
    0x10 - SP SEGMENT:4
    0x08 - SP SEGMENT:3
    0x04 - SP SEGMENT:2
    0x02 - SP SEGMENT:1
    0x01 - SP SEGMENT:0
*/

uint8_t vt_vt1682_state::vt1682_201a_sp_segment_7_0_r()
{
	uint8_t ret = m_201a_sp_segment_7_0;
	LOGMASKED(LOG_OTHER, "%s: vt1682_201a_sp_segment_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_201a_sp_segment_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_201a_sp_segment_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_201a_sp_segment_7_0 = data;
}

/*
    Address 0x201b r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - SP SEGMENT:11
    0x04 - SP SEGMENT:10
    0x02 - SP SEGMENT:9
    0x01 - SP SEGMENT:8
*/

uint8_t vt_vt1682_state::vt1682_201b_sp_segment_11_8_r()
{
	uint8_t ret = m_201b_sp_segment_11_8;
	LOGMASKED(LOG_OTHER, "%s: vt1682_201b_sp_segment_11_8_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_201b_sp_segment_11_8_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_201b_sp_segment_11_8_w writing: %02x\n", machine().describe_context(), data);
	m_201b_sp_segment_11_8 = data & 0x0f;
}


/*
    Address 0x201c r/w (MAIN CPU)

    0x80 - BK1 SEGMENT:7
    0x40 - BK1 SEGMENT:6
    0x20 - BK1 SEGMENT:5
    0x10 - BK1 SEGMENT:4
    0x08 - BK1 SEGMENT:3
    0x04 - BK1 SEGMENT:2
    0x02 - BK1 SEGMENT:1
    0x01 - BK1 SEGMENT:0
*/

uint8_t vt_vt1682_state::vt1682_201c_bk1_segment_7_0_r()
{
	uint8_t ret = m_segment_7_0_bk[0];
	LOGMASKED(LOG_OTHER, "%s: vt1682_201c_bk1_segment_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_201c_bk1_segment_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_201c_bk1_segment_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_segment_7_0_bk[0] = data;
}

/*
    Address 0x201d r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK1 SEGMENT:11
    0x04 - BK1 SEGMENT:10
    0x02 - BK1 SEGMENT:9
    0x01 - BK1 SEGMENT:8
*/

uint8_t vt_vt1682_state::vt1682_201d_bk1_segment_11_8_r()
{
	uint8_t ret = m_segment_11_8_bk[0];
	LOGMASKED(LOG_OTHER, "%s: vt1682_201d_bk1_segment_11_8_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_201d_bk1_segment_11_8_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_201d_bk1_segment_11_8_w writing: %02x\n", machine().describe_context(), data);
	m_segment_11_8_bk[0] = data & 0x0f;
}


/*
    Address 0x201e r/w (MAIN CPU)

    0x80 - BK2 SEGMENT:7
    0x40 - BK2 SEGMENT:6
    0x20 - BK2 SEGMENT:5
    0x10 - BK2 SEGMENT:4
    0x08 - BK2 SEGMENT:3
    0x04 - BK2 SEGMENT:2
    0x02 - BK2 SEGMENT:1
    0x01 - BK2 SEGMENT:0
*/

uint8_t vt_vt1682_state::vt1682_201e_bk2_segment_7_0_r()
{
	uint8_t ret = m_segment_7_0_bk[1];
	LOGMASKED(LOG_OTHER, "%s: vt1682_201e_bk2_segment_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_201e_bk2_segment_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_201e_bk2_segment_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_segment_7_0_bk[1] = data;
}

/*
    Address 0x201f r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK2 SEGMENT:11
    0x04 - BK2 SEGMENT:10
    0x02 - BK2 SEGMENT:9
    0x01 - BK2 SEGMENT:8
*/

uint8_t vt_vt1682_state::vt1682_201f_bk2_segment_11_8_r()
{
	uint8_t ret = m_segment_11_8_bk[1];
	LOGMASKED(LOG_OTHER, "%s: vt1682_201f_bk2_segment_11_8_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_201f_bk2_segment_11_8_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_201f_bk2_segment_11_8_w writing: %02x\n", machine().describe_context(), data);
	m_segment_11_8_bk[1] = data & 0x0f;
}

/*
    Address 0x2020 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - BK2 L EN (Linescroll enable)
    0x10 - BK1 L EN (Linescroll enable)
    0x08 - Scroll Bank
    0x04 - Scroll Bank
    0x02 - Scroll Bank
    0x01 - Scroll Bank
*/

uint8_t vt_vt1682_state::vt1682_2020_bk_linescroll_r()
{
	uint8_t ret = m_2020_bk_linescroll;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2020_bk_linescroll_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2020_bk_linescroll_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2020_bk_linescroll_w writing: %02x\n", machine().describe_context(), data);
	m_2020_bk_linescroll = data;

	if (data)
		popmessage("linescroll %02x!\n", data);
}

/*
    Address 0x2021 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Luminance_offset
    0x10 - Luminance_offset
    0x08 - Luminance_offset
    0x04 - Luminance_offset
    0x02 - Luminance_offset
    0x01 - Luminance_offset
*/

uint8_t vt_vt1682_state::vt1682_2021_lum_offset_r()
{
	uint8_t ret = m_2021_lum_offset;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2021_lum_offset_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2021_lum_offset_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2021_lum_offset_w writing: %02x\n", machine().describe_context(), data);
	m_2021_lum_offset = data;
}


/*
    Address 0x2022 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - VCOMIO
    0x10 - RGB DAC
    0x08 - CCIR Out
    0x04 - Saturation
    0x02 - Saturation
    0x01 - Saturation
*/

uint8_t vt_vt1682_state::vt1682_2022_saturation_misc_r()
{
	uint8_t ret = m_2022_saturation_misc;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2022_saturation_misc_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2022_saturation_misc_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2022_saturation_misc_w writing: %02x\n", machine().describe_context(), data);
	m_2022_saturation_misc = data;
}

/*
    Address 0x2023 r/w (MAIN CPU)

    0x80 - Light Gun Reset
    0x40 - Light Gun Reset
    0x20 - Light Gun Reset
    0x10 - Light Gun Reset
    0x08 - Light Gun Reset
    0x04 - Light Gun Reset
    0x02 - Light Gun Reset
    0x01 - Light Gun Reset
*/

uint8_t vt_vt1682_state::vt1682_2023_lightgun_reset_r()
{
	uint8_t ret = m_2023_lightgun_reset;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2023_lightgun_reset_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2023_lightgun_reset_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2023_lightgun_reset_w writing: %02x\n", machine().describe_context(), data);
	m_2023_lightgun_reset = data;
}

/*
    Address 0x2024 r/w (MAIN CPU)

    0x80 - Light Gun 1 Y
    0x40 - Light Gun 1 Y
    0x20 - Light Gun 1 Y
    0x10 - Light Gun 1 Y
    0x08 - Light Gun 1 Y
    0x04 - Light Gun 1 Y
    0x02 - Light Gun 1 Y
    0x01 - Light Gun 1 Y
*/

uint8_t vt_vt1682_state::vt1682_2024_lightgun1_y_r()
{
	uint8_t ret = m_2024_lightgun1_y;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2024_lightgun1_y_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2024_lightgun1_y_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2024_lightgun1_y_w writing: %02x\n", machine().describe_context(), data);
	m_2024_lightgun1_y = data;
}

/*
    Address 0x2025 r/w (MAIN CPU)

    0x80 - Light Gun 1 X
    0x40 - Light Gun 1 X
    0x20 - Light Gun 1 X
    0x10 - Light Gun 1 X
    0x08 - Light Gun 1 X
    0x04 - Light Gun 1 X
    0x02 - Light Gun 1 X
    0x01 - Light Gun 1 X
*/

uint8_t vt_vt1682_state::vt1682_2025_lightgun1_x_r()
{
	uint8_t ret = m_2025_lightgun1_x;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2025_lightgun1_x_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2025_lightgun1_x_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2025_lightgun1_x_w writing: %02x\n", machine().describe_context(), data);
	m_2025_lightgun1_x = data;
}

/*
    Address 0x2026 r/w (MAIN CPU)

    0x80 - Light Gun 2 Y
    0x40 - Light Gun 2 Y
    0x20 - Light Gun 2 Y
    0x10 - Light Gun 2 Y
    0x08 - Light Gun 2 Y
    0x04 - Light Gun 2 Y
    0x02 - Light Gun 2 Y
    0x01 - Light Gun 2 Y
*/

uint8_t vt_vt1682_state::vt1682_2026_lightgun2_y_r()
{
	uint8_t ret = m_2026_lightgun2_y;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2026_lightgun2_y_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2026_lightgun2_y_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2026_lightgun2_y_w writing: %02x\n", machine().describe_context(), data);
	m_2026_lightgun2_y = data;
}


/*
    Address 0x2027 r/w (MAIN CPU)

    0x80 - Light Gun 2 X
    0x40 - Light Gun 2 X
    0x20 - Light Gun 2 X
    0x10 - Light Gun 2 X
    0x08 - Light Gun 2 X
    0x04 - Light Gun 2 X
    0x02 - Light Gun 2 X
    0x01 - Light Gun 2 X
*/

uint8_t vt_vt1682_state::vt1682_2027_lightgun2_x_r()
{
	uint8_t ret = m_2027_lightgun2_x;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2027_lightgun2_x_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2027_lightgun2_x_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2027_lightgun2_x_w writing: %02x\n", machine().describe_context(), data);
	m_2027_lightgun2_x = data;
}


/*
    Address 0x2028 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - CCIR Y
    0x10 - CCIR Y
    0x08 - CCIR Y
    0x04 - CCIR Y
    0x02 - CCIR Y
    0x01 - CCIR Y
*/

uint8_t vt_vt1682_state::vt1682_2028_r()
{
	uint8_t ret = m_2028;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2028_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2028_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2028_w writing: %02x\n", machine().describe_context(), data);
	m_2028 = data;
}

/*
    Address 0x2029 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - CCIR X
    0x08 - CCIR X
    0x04 - CCIR X
    0x02 - CCIR X
    0x01 - CCIR X
*/

uint8_t vt_vt1682_state::vt1682_2029_r()
{
	uint8_t ret = m_2029;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2029_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2029_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2029_w writing: %02x\n", machine().describe_context(), data);
	m_2029 = data;
}


/*
    Address 0x202a r/w (MAIN CPU)

    0x80 - VS Phase
    0x40 - HS Phase
    0x20 - YC Swap
    0x10 - CbCr Swap
    0x08 - SyncMod
    0x04 - YUV_RGB
    0x02 - Field O En
    0x01 - Field On
*/


uint8_t vt_vt1682_state::vt1682_202a_r()
{
	uint8_t ret = m_202a;
	LOGMASKED(LOG_OTHER, "%s: vt1682_202a_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_202a_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_202a_w writing: %02x\n", machine().describe_context(), data);
	m_202a = data;
}


/*
    Address 0x202b r/w (MAIN CPU)

    0x80 - R En
    0x40 - G En
    0x20 - B En
    0x10 - Halftone
    0x08 - B/W
    0x04 - CCIR Depth
    0x02 - CCIR Depth
    0x01 - CCIR Depth
*/


uint8_t vt_vt1682_state::vt1682_202b_r()
{
	uint8_t ret = m_202b;
	LOGMASKED(LOG_OTHER, "%s: vt1682_202b_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_202b_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_202b_w writing: %02x\n", machine().describe_context(), data);
	m_202b = data;
}


/* Address 0x202c Unused */
/* Address 0x202d Unused */

/*
    Address 0x202e r/w (MAIN CPU)

    0x80 - TRC EN
    0x40 - CCIR EN
    0x20 - Bluescreen EN
    0x10 - Touch EN
    0x08 - CCIR TH
    0x04 - CCIR TH
    0x02 - CCIR TH
    0x01 - CCIR TH
*/


uint8_t vt_vt1682_state::vt1682_202e_r()
{
	uint8_t ret = m_202e;
	LOGMASKED(LOG_OTHER, "%s: vt1682_202e_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_202e_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_202e_w writing: %02x\n", machine().describe_context(), data);
	m_202e = data;
}


/* Address 0x202f Unused */

/*
    Address 0x2030 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - VDACSW
    0x20 - VDACOUT:5
    0x10 - VDACOUT:4
    0x08 - VDACOUT:3
    0x04 - VDACOUT:2
    0x02 - VDACOUT:1
    0x01 - VDACOUT:0
*/


uint8_t vt_vt1682_state::vt1682_2030_r()
{
	uint8_t ret = m_2030;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2030_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2030_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2030_w writing: %02x\n", machine().describe_context(), data);
	m_2030 = data;
}


/*
    Address 0x2031 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - R DAC SW
    0x10 - R DAC OUT:4
    0x08 - R DAC OUT:3
    0x04 - R DAC OUT:2
    0x02 - R DAC OUT:1
    0x01 - R DAC OUT:0
*/

uint8_t vt_vt1682_state::vt1682_2031_red_dac_r()
{
	uint8_t ret = m_2031_red_dac;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2031_red_dac_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2031_red_dac_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2031_red_dac_w writing: %02x\n", machine().describe_context(), data);
	m_2031_red_dac = data;
}

/*
    Address 0x2032 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - G DAC SW
    0x10 - G DAC OUT:4
    0x08 - G DAC OUT:3
    0x04 - G DAC OUT:2
    0x02 - G DAC OUT:1
    0x01 - G DAC OUT:0
*/

uint8_t vt_vt1682_state::vt1682_2032_green_dac_r()
{
	uint8_t ret = m_2032_green_dac;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2032_green_dac_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2032_green_dac_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2032_green_dac_w writing: %02x\n", machine().describe_context(), data);
	m_2032_green_dac = data;
}

/*
    Address 0x2033 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - B DAC SW
    0x10 - B DAC OUT:4
    0x08 - B DAC OUT:3
    0x04 - B DAC OUT:2
    0x02 - B DAC OUT:1
    0x01 - B DAC OUT:0
*/

uint8_t vt_vt1682_state::vt1682_2033_blue_dac_r()
{
	uint8_t ret = m_2033_blue_dac;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2033_blue_dac_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2033_blue_dac_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2033_blue_dac_w writing: %02x\n", machine().describe_context(), data);
	m_2033_blue_dac = data;
}


/************************************************************************************************************************************
 VT1682 Sys Registers
************************************************************************************************************************************/

/*
    Address 0x2100 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 3
    0x04 - Program Bank 1 Register 3
    0x02 - Program Bank 1 Register 3
    0x01 - Program Bank 1 Register 3
*/

uint8_t vt_vt1682_state::vt1682_2100_prgbank1_r3_r()
{
	uint8_t ret = m_2100_prgbank1_r3;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2100_prgbank1_r3_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2100_prgbank1_r3_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2100_prgbank1_r3_w writing: %02x (4-bits)\n", machine().describe_context(), data);
	m_2100_prgbank1_r3 = data;
	update_banks();
}

/* Address 0x2101 - 0x2104 (MAIN CPU) - see vt1682_timer.cpp */

/*
    Address 0x2105 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - COMR6
    0x20 - TV SYS SE:1
    0x10 - TV SYS SE:0
    0x08 - CCIR SEL
    0x04 - Double
    0x02 - ROM SEL
    0x01 - PRAM

    TV Mode settings 0 = NTSC, 1 = PAL M, 2 = PAL N, 3 = PAL
    see clocks near machine_config

    ROM SEL is which CPU the internal ROM maps to (if used)  0 = Main CPU, 1 = Sound CPU

*/

void vt_vt1682_state::vt1682_2105_comr6_tvmodes_w(uint8_t data)
{
	// COMR6 is used for banking
	LOGMASKED(LOG_OTHER, "%s: vt1682_2105_comr6_tvmodes_w writing: %02x\n", machine().describe_context(), data);
	m_2105_vt1682_2105_comr6_tvmodes = data;
	update_banks();
}


/*
    Address 0x2106 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - SCPU RN (Sound CPU Reset Line Control)
    0x10 - SCPU ON (Sound CPU Enable)
    0x08 - SPI ON
    0x04 - UART ON
    0x02 - TV ON (TV display encoder enable)
    0x01 - LCD ON (LCD display controller enable)
*/

uint8_t vt_vt1682_state::vt1682_2106_enable_regs_r()
{
	uint8_t ret = m_2106_enable_reg;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2106_enable_regs_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2106_enable_regs_w(uint8_t data)
{
	// COMR6 is used for banking
	LOGMASKED(LOG_OTHER, "%s: vt1682_2106_enable_regs_w writing: %02x (scpurn:%1x scpuon:%1x spion:%1x uarton:%1x tvon:%1x lcdon:%1x)\n", machine().describe_context().c_str(), data,
		(data & 0x20) >> 5, (data & 0x10) >> 4, (data & 0x08) >> 3, (data & 0x04) >> 2, (data & 0x02) >> 1, (data & 0x01));
	m_2106_enable_reg = data;

	if (data & 0x20)
	{
		m_soundcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_scpu_is_in_reset = false;
	}
	else
	{
		m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_scpu_is_in_reset = true;
	}
}


/*
    Address 0x2107 r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 0
    0x40 - Program Bank 0 Register 0
    0x20 - Program Bank 0 Register 0
    0x10 - Program Bank 0 Register 0
    0x08 - Program Bank 0 Register 0
    0x04 - Program Bank 0 Register 0
    0x02 - Program Bank 0 Register 0
    0x01 - Program Bank 0 Register 0
*/

uint8_t vt_vt1682_state::vt1682_2107_prgbank0_r0_r()
{
	uint8_t ret = m_2107_prgbank0_r0;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2107_prgbank0_r0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2107_prgbank0_r0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2107_prgbank0_r0_w writing: %02x\n", machine().describe_context(), data);
	m_2107_prgbank0_r0 = data;
	update_banks();
}

/*
    Address 0x2108 r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 1
    0x40 - Program Bank 0 Register 1
    0x20 - Program Bank 0 Register 1
    0x10 - Program Bank 0 Register 1
    0x08 - Program Bank 0 Register 1
    0x04 - Program Bank 0 Register 1
    0x02 - Program Bank 0 Register 1
    0x01 - Program Bank 0 Register 1
*/

uint8_t vt_vt1682_state::vt1682_2108_prgbank0_r1_r()
{
	uint8_t ret = m_2108_prgbank0_r1;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2108_prgbank0_r1_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2108_prgbank0_r1_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2108_prgbank0_r1_w writing: %02x\n", machine().describe_context(), data);
	m_2108_prgbank0_r1 = data;
	update_banks();
}


/*
    Address 0x2109 r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 2
    0x40 - Program Bank 0 Register 2
    0x20 - Program Bank 0 Register 2
    0x10 - Program Bank 0 Register 2
    0x08 - Program Bank 0 Register 2
    0x04 - Program Bank 0 Register 2
    0x02 - Program Bank 0 Register 2
    0x01 - Program Bank 0 Register 2
*/


uint8_t vt_vt1682_state::vt1682_2109_prgbank0_r2_r()
{
	uint8_t ret = m_2109_prgbank0_r2;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2109_prgbank0_r2_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2109_prgbank0_r2_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2109_prgbank0_r2_w writing: %02x\n", machine().describe_context(), data);
	m_2109_prgbank0_r2 = data;
	update_banks();
}

/*
    Address 0x210a r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 3
    0x40 - Program Bank 0 Register 3
    0x20 - Program Bank 0 Register 3
    0x10 - Program Bank 0 Register 3
    0x08 - Program Bank 0 Register 3
    0x04 - Program Bank 0 Register 3
    0x02 - Program Bank 0 Register 3
    0x01 - Program Bank 0 Register 3
*/

uint8_t vt_vt1682_state::vt1682_210a_prgbank0_r3_r()
{
	uint8_t ret = m_210a_prgbank0_r3;
	LOGMASKED(LOG_OTHER, "%s: vt1682_210a_prgbank0_r3_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_210a_prgbank0_r3_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_210a_prgbank0_r3_w writing: %02x\n", machine().describe_context(), data);
	m_210a_prgbank0_r3 = data;
	update_banks();
}

/*
    Address 0x210b r/w (MAIN CPU)

    0x80 - TSYSN En (Timer Clock Select)
    0x40 - PQ2 Enable
    0x20 - BUS Tristate
    0x10 - CS Control:1
    0x08 - CS Control:0
    0x04 - Program Bank 0 Select
    0x02 - Program Bank 0 Select
    0x01 - Program Bank 0 Select
*/

uint8_t vt_vt1682_state::vt1682_210b_misc_cs_prg0_bank_sel_r()
{
	uint8_t ret = m_210b_misc_cs_prg0_bank_sel;
	LOGMASKED(LOG_OTHER, "%s: vt1682_210b_misc_cs_prg0_bank_sel_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_210b_misc_cs_prg0_bank_sel_w(uint8_t data)
{
	// PQ2 Enable is also used for ROM banking along with Program Bank 0 select
	uint32_t clock = m_maincpu->clock();

	LOGMASKED(LOG_OTHER, "%s: vt1682_210b_misc_cs_prg0_bank_sel_w writing: %02x\n", machine().describe_context(), data);
	m_210b_misc_cs_prg0_bank_sel = data;

	if (data & 0x80)
	{
		if (clock == 21477272/4)
			m_system_timer_dev->set_clock(TIMER_ALT_SPEED_NTSC);
		else if (clock == 26601712/5)
			m_system_timer_dev->set_clock(TIMER_ALT_SPEED_PAL);
		else
			logerror("setting alt timings with unknown main CPU frequency %d\n", clock);
	}
	else
	{
		if (clock == 21477272/4)
			m_system_timer_dev->set_clock(MAIN_CPU_CLOCK_NTSC);
		else if (clock == 26601712/5)
			m_system_timer_dev->set_clock(MAIN_CPU_CLOCK_PAL);
		else
			logerror("setting alt timings with unknown main CPU frequency %d\n", clock);
	}

	update_banks();
}


/*
    Address 0x210c r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 2
    0x04 - Program Bank 1 Register 2
    0x02 - Program Bank 1 Register 2
    0x01 - Program Bank 1 Register 2
*/

uint8_t vt_vt1682_state::vt1682_210c_prgbank1_r2_r()
{
	uint8_t ret = m_210c_prgbank1_r2;
	LOGMASKED(LOG_OTHER, "%s: vt1682_210c_prgbank1_r2_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_210c_prgbank1_r2_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_210c_prgbank1_r2_w writing: %02x (4-bits)\n", machine().describe_context(), data);
	m_210c_prgbank1_r2 = data;
	update_banks();
}


/* 0x210d - see vt1682_io.cpp */
/* 0x210e - see vt1682_io.cpp */
/* 0x210f - see vt1682_io.cpp */


/*
   Address 0x2110 READ (MAIN CPU)

    0x80 - Program Bank 0 Register 4
    0x40 - Program Bank 0 Register 4
    0x20 - Program Bank 0 Register 4
    0x10 - Program Bank 0 Register 4
    0x08 - Program Bank 0 Register 4
    0x04 - Program Bank 0 Register 4
    0x02 - Program Bank 0 Register 4
    0x01 - Program Bank 0 Register 4

    Address 0x2110 WRITE (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 0
    0x04 - Program Bank 1 Register 0
    0x02 - Program Bank 1 Register 0
    0x01 - Program Bank 1 Register 0
*/

uint8_t vt_vt1682_state::vt1682_prgbank0_r4_r()
{
	uint8_t ret = m_prgbank0_r4;
	LOGMASKED(LOG_OTHER, "%s: (2110) vt1682_prgbank0_r4_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_prgbank1_r0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: (2110) vt1682_prgbank1_r0_w writing: %02x (4-bits)\n", machine().describe_context(), data);
	m_prgbank1_r0 = data;
	update_banks();
}

/*
   Address 0x2111 READ (MAIN CPU)

    0x80 - Program Bank 0 Register 5
    0x40 - Program Bank 0 Register 5
    0x20 - Program Bank 0 Register 5
    0x10 - Program Bank 0 Register 5
    0x08 - Program Bank 0 Register 5
    0x04 - Program Bank 0 Register 5
    0x02 - Program Bank 0 Register 5
    0x01 - Program Bank 0 Register 5

    Address 0x2111 WRITE (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 1
    0x04 - Program Bank 1 Register 1
    0x02 - Program Bank 1 Register 1
    0x01 - Program Bank 1 Register 1
*/

uint8_t vt_vt1682_state::vt1682_prgbank0_r5_r()
{
	uint8_t ret = m_prgbank0_r5;
	LOGMASKED(LOG_OTHER, "%s: (2111) vt1682_prgbank0_r5_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}


void vt_vt1682_state::vt1682_prgbank1_r1_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: (2111) vt1682_prgbank1_r1_w writing: %02x (4-bits)\n", machine().describe_context(), data);
	m_prgbank1_r1 = data;
	update_banks();
}


/*
    Address 0x2112 READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 0
    0x04 - Program Bank 1 Register 0
    0x02 - Program Bank 1 Register 0
    0x01 - Program Bank 1 Register 0

    Address 0x2112 WRITE (MAIN CPU)

    0x80 - Program Bank 0 Register 4
    0x40 - Program Bank 0 Register 4
    0x20 - Program Bank 0 Register 4
    0x10 - Program Bank 0 Register 4
    0x08 - Program Bank 0 Register 4
    0x04 - Program Bank 0 Register 4
    0x02 - Program Bank 0 Register 4
    0x01 - Program Bank 0 Register 4
*/

uint8_t vt_vt1682_state::vt1682_prgbank1_r0_r()
{
	uint8_t ret = m_prgbank1_r0;
	LOGMASKED(LOG_OTHER, "%s: (2112) vt1682_prgbank1_r0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}


void vt_vt1682_state::vt1682_prgbank0_r4_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: (2112) vt1682_prgbank0_r4_w writing: %02x (8-bits)\n", machine().describe_context(), data);
	m_prgbank0_r4 = data;
	update_banks();
}


/*
    Address 0x2113 READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 1
    0x04 - Program Bank 1 Register 1
    0x02 - Program Bank 1 Register 1
    0x01 - Program Bank 1 Register 1

    Address 0x2113 WRITE (MAIN CPU)

    0x80 - Program Bank 0 Register 5
    0x40 - Program Bank 0 Register 5
    0x20 - Program Bank 0 Register 5
    0x10 - Program Bank 0 Register 5
    0x08 - Program Bank 0 Register 5
    0x04 - Program Bank 0 Register 5
    0x02 - Program Bank 0 Register 5
    0x01 - Program Bank 0 Register 5
*/

uint8_t vt_vt1682_state::vt1682_prgbank1_r1_r()
{
	uint8_t ret = m_prgbank1_r1;
	LOGMASKED(LOG_OTHER, "%s: (2113) vt1682_prgbank1_r1_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_prgbank0_r5_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: (2113) vt1682_prgbank0_r5_w writing: %02x (8-bits)\n", machine().describe_context(), data);
	m_prgbank0_r5 = data;
	update_banks();
}

/*
    Address 0x2114 r/w (MAIN CPU)

    0x80 - Baud Rate:7
    0x40 - Baud Rate:6
    0x20 - Baud Rate:5
    0x10 - Baud Rate:4
    0x08 - Baud Rate:3
    0x04 - Baud Rate:2
    0x02 - Baud Rate:1
    0x01 - Baud Rate:0
*/

/*
    Address 0x2115 r/w (MAIN CPU)

    0x80 - Baud Rate:15
    0x40 - Baud Rate:14
    0x20 - Baud Rate:13
    0x10 - Baud Rate:12
    0x08 - Baud Rate:11
    0x04 - Baud Rate:10
    0x02 - Baud Rate:9
    0x01 - Baud Rate:8
*/

/*
    Address 0x2116 r/w (MAIN CPU)

    0x80 - 16bit SPI
    0x40 - SPIEN
    0x20 - SPI RST
    0x10 - M/SB
    0x08 - CLK PHASE
    0x04 - CLK POLARITY
    0x02 - CLK FREQ:1
    0x01 - CLK FREQ:0
*/

/*
    Address 0x2117 WRITE (MAIN CPU)

    0x80 - SPI TX Data
    0x40 - SPI TX Data
    0x20 - SPI TX Data
    0x10 - SPI TX Data
    0x08 - SPI TX Data
    0x04 - SPI TX Data
    0x02 - SPI TX Data
    0x01 - SPI TX Data

    Address 0x2117 READ (MAIN CPU)

    0x80 - SPI RX Data
    0x40 - SPI RX Data
    0x20 - SPI RX Data
    0x10 - SPI RX Data
    0x08 - SPI RX Data
    0x04 - SPI RX Data
    0x02 - SPI RX Data
    0x01 - SPI RX Data
*/

/*
    Address 0x2118 r/w (MAIN CPU)

    0x80 - Program Bank 1 Register 5
    0x40 - Program Bank 1 Register 5
    0x20 - Program Bank 1 Register 5
    0x10 - Program Bank 1 Register 5
    0x08 - Program Bank 1 Register 4
    0x04 - Program Bank 1 Register 4
    0x02 - Program Bank 1 Register 4
    0x01 - Program Bank 1 Register 4
*/

uint8_t vt_vt1682_state::vt1682_2118_prgbank1_r4_r5_r()
{
	uint8_t ret = m_2118_prgbank1_r4_r5;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2118_prgbank1_r4_r5_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2118_prgbank1_r4_r5_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2118_prgbank1_r4_r5_w writing: %02x (2x 4-bits)\n", machine().describe_context(), data);
	m_2118_prgbank1_r4_r5 = data;
	update_banks();
}


/*
    Address 0x2119 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - Carrier En
    0x20 - UART En
    0x10 - Tx IRQ En
    0x08 - Rx IRQ En
    0x04 - Parity En
    0x02 - Odd/Even
    0x01 - 9bit Mode
*/

/*
    Address 0x211a WRITE (MAIN CPU)

    0x80 - TX Data
    0x40 - TX Data
    0x20 - TX Data
    0x10 - TX Data
    0x08 - TX Data
    0x04 - TX Data
    0x02 - TX Data
    0x01 - TX Data

    Address 0x211a READ (MAIN CPU)

    0x80 - RX Data
    0x40 - RX Data
    0x20 - RX Data
    0x10 - RX Data
    0x08 - RX Data
    0x04 - RX Data
    0x02 - RX Data
    0x01 - RX Data
*/

/*
    Address 0x211b WRITE (MAIN CPU)

    0x80 - Carrier Freq
    0x40 - Carrier Freq
    0x20 - Carrier Freq
    0x10 - Carrier Freq
    0x08 - Carrier Freq
    0x04 - Carrier Freq
    0x02 - Carrier Freq
    0x01 - Carrier Freq

    Address 0x211b READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Rx Error
    0x10 - Tx Status
    0x08 - Rx Status
    0x04 - Parity Error
    0x02 - (unused)
    0x01 - (unused)
*/

/*
    Address 0x211c WRITE (MAIN CPU)

    0x80 - AutoWake
    0x40 - KeyWake
    0x20 - EXT2421EN
    0x10 - SCPUIRQ
    0x08 - SLEEPM
    0x04 - (unused)
    0x02 - SLEEP SEL
    0x01 - CLK SEL

    Address 0x211c READ (MAIN CPU)

    0x80 - Clear_SCPU_IRQ
    0x40 - Clear_SCPU_IRQ
    0x20 - Clear_SCPU_IRQ
    0x10 - Clear_SCPU_IRQ
    0x08 - Clear_SCPU_IRQ
    0x04 - Clear_SCPU_IRQ
    0x02 - Clear_SCPU_IRQ
    0x01 - Clear_SCPU_IRQ
*/

void vt_vt1682_state::vt1682_211c_regs_ext2421_w(uint8_t data)
{
	// EXT2421EN is used for ROM banking
	LOGMASKED(LOG_OTHER, "%s: vt1682_211c_regs_ext2421_w writing: %02x\n", machine().describe_context(), data);
	m_211c_regs_ext2421 = data;
	update_banks();

	if (data & 0x10)
	{
		// not seen used
		logerror("Sound CPU IRQ Request\n");
	}
}


/*
    Address 0x211d WRITE (MAIN CPU)

    0x80 - LVDEN
    0x40 - LVDS1
    0x20 - LVDS0
    0x10 - VDAC_EN
    0x08 - ADAC_EN
    0x04 - PLL_EN
    0x02 - LCDACEN
    0x01 - (unused)

    Address 0x211d READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - (unused)
    0x01 - LVD
*/

/*
    Address 0x211e WRITE (MAIN CPU)

    0x80 - ADCEN
    0x40 - ADCS1
    0x20 - ADCS0
    0x10 - (unused)
    0x08 - IOFOEN3
    0x04 - IOFOEN2
    0x02 - IOFOEN1
    0x01 - IOFOEN0

    Address 0x211e READ (MAIN CPU)

    0x80 - ADC DATA:7
    0x40 - ADC DATA:6
    0x20 - ADC DATA:5
    0x10 - ADC DATA:4
    0x08 - ADC DATA:3
    0x04 - ADC DATA:2
    0x02 - ADC DATA:1
    0x01 - ADC DATA:0
*/

/*
    Address 0x211f r/w (MAIN CPU)

    0x80 - VGCEN
    0x40 - VGCA6
    0x20 - VGCA5
    0x10 - VGCA4
    0x08 - VGCA3
    0x04 - VGCA2
    0x02 - VGCA1
    0x01 - VGCA0
*/

/*
    Address 0x2120 r/w (MAIN CPU)

    0x80 - Sleep Period
    0x40 - Sleep Period
    0x20 - Sleep Period
    0x10 - Sleep Period
    0x08 - Sleep Period
    0x04 - Sleep Period
    0x02 - Sleep Period
    0x01 - Sleep Period
*/

/*
    Address 0x2121 READ (MAIN CPU) (maybe)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - SPI MSK
    0x08 - UART MSK
    0x04 - SPU MSK
    0x02 - TMR MSK
    0x01 - Ext MSK

    Address 0x2121 WRITE (MAIN CPU) (maybe)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - Clear SPU
    0x02 - (unused)
    0x01 - Clear Ext
*/

/*
    Address 0x2122 r/w (MAIN CPU)

    0x80 - DMA DT ADDR:7
    0x40 - DMA DT ADDR:6
    0x20 - DMA DT ADDR:5
    0x10 - DMA DT ADDR:4
    0x08 - DMA DT ADDR:3
    0x04 - DMA DT ADDR:2
    0x02 - DMA DT ADDR:1
    0x01 - DMA DT ADDR:0
*/

uint8_t vt_vt1682_state::vt1682_2122_dma_dt_addr_7_0_r()
{
	uint8_t ret = m_2122_dma_dt_addr_7_0;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2122_dma_dt_addr_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2122_dma_dt_addr_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2122_dma_dt_addr_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_2122_dma_dt_addr_7_0 = data;
}


/*
    Address 0x2123 r/w (MAIN CPU)

    0x80 - DMA DT ADDR:15
    0x40 - DMA DT ADDR:14
    0x20 - DMA DT ADDR:13
    0x10 - DMA DT ADDR:12
    0x08 - DMA DT ADDR:11
    0x04 - DMA DT ADDR:10
    0x02 - DMA DT ADDR:9
    0x01 - DMA DT ADDR:8
*/

uint8_t vt_vt1682_state::vt1682_2123_dma_dt_addr_15_8_r()
{
	uint8_t ret = m_2123_dma_dt_addr_15_8;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2123_dma_dt_addr_15_8_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2123_dma_dt_addr_15_8_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2123_dma_dt_addr_15_8_w writing: %02x\n", machine().describe_context(), data);
	m_2123_dma_dt_addr_15_8 = data;
}


/*
    Address 0x2124 r/w (MAIN CPU)

    0x80 - DMA SR ADDR:7
    0x40 - DMA SR ADDR:6
    0x20 - DMA SR ADDR:5
    0x10 - DMA SR ADDR:4
    0x08 - DMA SR ADDR:3
    0x04 - DMA SR ADDR:2
    0x02 - DMA SR ADDR:1
    0x01 - DMA SR ADDR:0
*/

uint8_t vt_vt1682_state::vt1682_2124_dma_sr_addr_7_0_r()
{
	uint8_t ret = m_2124_dma_sr_addr_7_0;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2124_dma_sr_addr_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2124_dma_sr_addr_7_0_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2124_dma_sr_addr_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_2124_dma_sr_addr_7_0 = data;
}


/*
    Address 0x2125 r/w (MAIN CPU)

    0x80 - DMA SR ADDR:15
    0x40 - DMA SR ADDR:14
    0x20 - DMA SR ADDR:13
    0x10 - DMA SR ADDR:12
    0x08 - DMA SR ADDR:11
    0x04 - DMA SR ADDR:10
    0x02 - DMA SR ADDR:9
    0x01 - DMA SR ADDR:8
*/

uint8_t vt_vt1682_state::vt1682_2125_dma_sr_addr_15_8_r()
{
	uint8_t ret = m_2125_dma_sr_addr_15_8;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2125_dma_sr_addr_15_8_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2125_dma_sr_addr_15_8_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2125_dma_sr_addr_15_8_w writing: %02x\n", machine().describe_context(), data);
	m_2125_dma_sr_addr_15_8 = data;
}



/*
    Address 0x2126 r/w (MAIN CPU)

    0x80 - DMA SR BANK:22
    0x40 - DMA SR BANK:21
    0x20 - DMA SR BANK:20
    0x10 - DMA SR BANK:19
    0x08 - DMA SR BANK:18
    0x04 - DMA SR BANK:17
    0x02 - DMA SR BANK:16
    0x01 - DMA SR BANK:15
*/

uint8_t vt_vt1682_state::vt1682_2126_dma_sr_bank_addr_22_15_r()
{
	uint8_t ret = m_2126_dma_sr_bank_addr_22_15;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2126_dma_sr_bank_addr_22_15_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2126_dma_sr_bank_addr_22_15_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2126_dma_sr_bank_addr_22_15_w writing: %02x\n", machine().describe_context(), data);
	m_2126_dma_sr_bank_addr_22_15 = data;
}

/*
    Address 0x2127 WRITE (MAIN CPU)

    0x80 - DMA Number
    0x40 - DMA Number
    0x20 - DMA Number
    0x10 - DMA Number
    0x08 - DMA Number
    0x04 - DMA Number
    0x02 - DMA Number
    0x01 - DMA Number

    Address 0x2127 READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - (unused)
    0x01 - DMA Status
*/

uint8_t vt_vt1682_state::vt1682_2127_dma_status_r()
{
	uint8_t ret = 0x00;

	int dma_status = 0; // 1 would be 'busy'
	ret |= dma_status;

	LOGMASKED(LOG_OTHER, "%s: vt1682_2127_dma_status_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::do_dma_external_to_internal(int data, bool is_video)
{
	int count = data * 2;
	if (count == 0)
		count = 0x200;

	int srcbank = get_dma_sr_bank_ddr();
	int srcaddr = get_dma_sr_addr();
	uint16_t dstaddr = get_dma_dt_addr();

	if (is_video)
		LOGMASKED(LOG_OTHER, "Doing DMA, External to Internal (VRAM/SRAM) src: %08x dest: %04x length: %03x\n", srcaddr | srcbank<<15, dstaddr, count);
	else
		LOGMASKED(LOG_OTHER, "Doing DMA, External to Internal src: %08x dest: %04x length: %03x\n", srcaddr | srcbank<<15, dstaddr, count);

	for (int i = 0; i < count; i++)
	{
		srcaddr = get_dma_sr_addr();
		dstaddr = get_dma_dt_addr();
		uint8_t dat = m_fullrom->read8(srcaddr | srcbank<<15);
		srcaddr++;

		address_space &mem = m_maincpu->space(AS_PROGRAM);
		mem.write_byte(dstaddr, dat);

		if (!is_video)
			dstaddr++;

		// update registers
		set_dma_dt_addr(dstaddr);
		set_dma_sr_addr(srcaddr);
	}
}

void vt_vt1682_state::do_dma_internal_to_internal(int data, bool is_video)
{
	int count = data * 2;
	if (count == 0)
		count = 0x200;

	int srcaddr = get_dma_sr_addr();
	uint16_t dstaddr = get_dma_dt_addr();

	if (is_video)
		LOGMASKED(LOG_OTHER, "Doing DMA, Internal to Internal (VRAM/SRAM) src: %04x dest: %04x length: %03x\n", srcaddr, dstaddr, count);
	else
		LOGMASKED(LOG_OTHER, "Doing DMA, Internal to Internal src: %04x dest: %04x length: %03x\n", srcaddr, dstaddr, count);

	for (int i = 0; i < count; i++)
	{
		address_space &mem = m_maincpu->space(AS_PROGRAM);
		dstaddr = get_dma_dt_addr();

		srcaddr = get_dma_sr_addr();
		uint8_t dat = mem.read_byte(srcaddr);
		srcaddr++;

		mem.write_byte(dstaddr, dat);

		if (!is_video)
			dstaddr++;

		// update registers
		set_dma_dt_addr(dstaddr);
		set_dma_sr_addr(srcaddr);
	}
}



void vt_vt1682_state::vt1682_2127_dma_size_trigger_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2127_dma_size_trigger_w writing: %02x\n", machine().describe_context(), data);

	// hw waits until VBLANK before actually doing the DMA! (TODO)

	if (get_dma_sr_isext())
	{
		if (get_dma_dt_isext())
		{
			// Source External
			// Dest External
			LOGMASKED(LOG_OTHER, "Invalid DMA, both Source and Dest are 'External'\n");
			return;
		}
		else
		{
			// Source External
			// Dest Internal

			uint16_t dstaddr = get_dma_dt_addr();
			int srcaddr = get_dma_sr_addr();

			if ((srcaddr & 1) || ((dstaddr & 1) && (!get_dma_dt_is_video())) )
			{
				LOGMASKED(LOG_OTHER, "Invalid DMA, low bit of address set\n");
				return;
			}


			do_dma_external_to_internal(data, get_dma_dt_is_video());

			return;
		}
	}
	else
	{
		if (get_dma_dt_isext())
		{
			// this is only likely if there is RAM in the usual ROM space

			// Source Internal
			// Dest External
			int dstbank = get_dma_sr_bank_ddr();
			int dstaddr = get_dma_dt_addr() | (dstbank << 15);
			uint16_t srcaddr = get_dma_sr_addr();

			if ((srcaddr & 1) || (dstaddr & 1))
			{
				LOGMASKED(LOG_OTHER, "Invalid DMA, low bit of address set\n");
				return;
			}

			LOGMASKED(LOG_OTHER, "Unhandled DMA, Dest is 'External'\n");
			return;
		}
		else
		{
			// Source Internal
			// Dest Internal

			uint16_t srcaddr = get_dma_sr_addr();
			uint16_t dstaddr = get_dma_dt_addr();

			if ((srcaddr & 1) || ((dstaddr & 1) && (!get_dma_dt_is_video())) )
			{
				LOGMASKED(LOG_OTHER, "Invalid DMA, low bit of address set\n");
				return;
			}

			do_dma_internal_to_internal(data, get_dma_dt_is_video());
			return;
		}
	}
}

/*
    Address 0x2128 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - DMA SR BANK:24
    0x01 - DMA SR BANK:23
*/

uint8_t vt_vt1682_state::vt1682_2128_dma_sr_bank_addr_24_23_r()
{
	uint8_t ret = m_2128_dma_sr_bank_addr_24_23;
	LOGMASKED(LOG_OTHER, "%s: vt1682_2128_dma_sr_bank_addr_24_23_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_2128_dma_sr_bank_addr_24_23_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_2128_dma_sr_bank_addr_24_23_w writing: %02x\n", machine().describe_context(), data);
	m_2128_dma_sr_bank_addr_24_23 = data & 0x03;
}


/*
    Address 0x2129 READ (MAIN CPU)

    0x80 - UIOA DATA IN / Send Joy CLK
    0x40 - UIOA DATA IN / Send Joy CLK
    0x20 - UIOA DATA IN / Send Joy CLK
    0x10 - UIOA DATA IN / Send Joy CLK
    0x08 - UIOA DATA IN / Send Joy CLK
    0x04 - UIOA DATA IN / Send Joy CLK
    0x02 - UIOA DATA IN / Send Joy CLK
    0x01 - UIOA DATA IN / Send Joy CLK

    Address 0x2129 WRITE (MAIN CPU)

    0x80 - UIOA DATA OUT
    0x40 - UIOA DATA OUT
    0x20 - UIOA DATA OUT
    0x10 - UIOA DATA OUT
    0x08 - UIOA DATA OUT
    0x04 - UIOA DATA OUT
    0x02 - UIOA DATA OUT
    0x01 - UIOA DATA OUT

*/

/*
    Address 0x212a READ (MAIN CPU)

    0x80 - Send Joy CLK 2
    0x40 - Send Joy CLK 2
    0x20 - Send Joy CLK 2
    0x10 - Send Joy CLK 2
    0x08 - Send Joy CLK 2
    0x04 - Send Joy CLK 2
    0x02 - Send Joy CLK 2
    0x01 - Send Joy CLK 2

    Address 0x212a WRITE (MAIN CPU)

    0x80 - UIOA DIRECTION
    0x40 - UIOA DIRECTION
    0x20 - UIOA DIRECTION
    0x10 - UIOA DIRECTION
    0x08 - UIOA DIRECTION
    0x04 - UIOA DIRECTION
    0x02 - UIOA DIRECTION
    0x01 - UIOA DIRECTION
*/

void vt_vt1682_state::clock_joy2()
{
}

uint8_t vt_vt1682_state::inteact_212a_send_joy_clock2_r()
{
	uint8_t ret = m_uio->inteact_212a_uio_a_direction_r();
	clock_joy2();
	return ret;
}

/*
    Address 0x212b r/w (MAIN CPU)

    0x80 - UIOA ATTRIBUTE
    0x40 - UIOA ATTRIBUTE
    0x20 - UIOA ATTRIBUTE
    0x10 - UIOA ATTRIBUTE
    0x08 - UIOA ATTRIBUTE
    0x04 - UIOA ATTRIBUTE
    0x02 - UIOA ATTRIBUTE
    0x01 - UIOA ATTRIBUTE
*/

/*
    Address 0x212c READ (MAIN CPU)

    0x80 - Pseudo Random Number
    0x40 - Pseudo Random Number
    0x20 - Pseudo Random Number
    0x10 - Pseudo Random Number
    0x08 - Pseudo Random Number
    0x04 - Pseudo Random Number
    0x02 - Pseudo Random Number
    0x01 - Pseudo Random Number

    Address 0x212c WRITE (MAIN CPU)

    0x80 - Pseudo Random Number Seed
    0x40 - Pseudo Random Number Seed
    0x20 - Pseudo Random Number Seed
    0x10 - Pseudo Random Number Seed
    0x08 - Pseudo Random Number Seed
    0x04 - Pseudo Random Number Seed
    0x02 - Pseudo Random Number Seed
    0x01 - Pseudo Random Number Seed
*/

uint8_t vt_vt1682_state::vt1682_212c_prng_r()
{
	uint8_t ret = machine().rand();
	LOGMASKED(LOG_OTHER, "%s: vt1682_212c_prng_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_212c_prng_seed_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: vt1682_212c_prng_seed_w writing: %02x\n", machine().describe_context(), data);
	// don't know the algorithm
}


/*
    Address 0x212d WRITE ONLY (MAIN CPU)

    0x80 - PLL B
    0x40 - PLL B
    0x20 - PLL B
    0x10 - PLL B
    0x08 - PLL M
    0x04 - PLL A
    0x02 - PLL A
    0x01 - PLL A
*/

/* Address 0x212e Unused */
/* Address 0x212f Unused */

/* Address 0x2130 - 0x2137 - see v1682_alu.cpp */

/* Address 0x2138 Unused */
/* Address 0x2139 Unused */
/* Address 0x213a Unused */
/* Address 0x213b Unused */
/* Address 0x213c Unused */
/* Address 0x213d Unused */
/* Address 0x213e Unused */
/* Address 0x213f Unused */

/*
    Address 0x2140 r/w (MAIN CPU)

    0x80 - I2C ID
    0x40 - I2C ID
    0x20 - I2C ID
    0x10 - I2C ID
    0x08 - I2C ID
    0x04 - I2C ID
    0x02 - I2C ID
    0x01 - I2C ID
*/

/*
    Address 0x2141 r/w (MAIN CPU)

    0x80 - I2C ADDR
    0x40 - I2C ADDR
    0x20 - I2C ADDR
    0x10 - I2C ADDR
    0x08 - I2C ADDR
    0x04 - I2C ADDR
    0x02 - I2C ADDR
    0x01 - I2C ADDR
*/

/*
    Address 0x2142 r/w (MAIN CPU)

    0x80 - I2C DATA
    0x40 - I2C DATA
    0x20 - I2C DATA
    0x10 - I2C DATA
    0x08 - I2C DATA
    0x04 - I2C DATA
    0x02 - I2C DATA
    0x01 - I2C DATA
*/

/*
    Address 0x2143 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - I2C CLK SELECT
    0x01 - I2C CLK SELECT
*/

/* Address 0x2144 Unused */
/* Address 0x2145 Unused */
/* Address 0x2146 Unused */
/* Address 0x2147 Unused */

/*
    Address 0x2148 WRITE ONLY (MAIN CPU)

    0x80 - UIOB SEL:7
    0x40 - UIOB SEL:6
    0x20 - UIOB SEL:5
    0x10 - UIOB SEL:4
    0x08 - UIOB SEL:3
    0x04 - (unused)
    0x02 - UIOA MODE
    0x01 - UIOA MODE
*/

/*
    Address 0x2149 WRITE (MAIN CPU)

    0x80 - UIOB DATA OUT
    0x40 - UIOB DATA OUT
    0x20 - UIOB DATA OUT
    0x10 - UIOB DATA OUT
    0x08 - UIOB DATA OUT
    0x04 - UIOB DATA OUT
    0x02 - UIOB DATA OUT
    0x01 - UIOB DATA OUT

    Address 0x2149 READ (MAIN CPU)

    0x80 - UIOB DATA IN
    0x40 - UIOB DATA IN
    0x20 - UIOB DATA IN
    0x10 - UIOB DATA IN
    0x08 - UIOB DATA IN
    0x04 - UIOB DATA IN
    0x02 - UIOB DATA IN
    0x01 - UIOB DATA IN
*/

/*
    Address 0x214a r/w (MAIN CPU)

    0x80 - UIOB DIRECTION
    0x40 - UIOB DIRECTION
    0x20 - UIOB DIRECTION
    0x10 - UIOB DIRECTION
    0x08 - UIOB DIRECTION
    0x04 - UIOB DIRECTION
    0x02 - UIOB DIRECTION
    0x01 - UIOB DIRECTION
*/

/*
    Address 0x214b r/w (MAIN CPU)

    0x80 - UIOB ATTRIBUTE
    0x40 - UIOB ATTRIBUTE
    0x20 - UIOB ATTRIBUTE
    0x10 - UIOB ATTRIBUTE
    0x08 - UIOB ATTRIBUTE
    0x04 - UIOB ATTRIBUTE
    0x02 - UIOB ATTRIBUTE
    0x01 - UIOB ATTRIBUTE
*/

/*
    Address 0x214c r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Keychange Enable
    0x10 - Keychange Enable
    0x08 - IOFEN
    0x04 - (unused)
    0x02 - (unused)
    0x01 - IOEOEN
*/

/*
    Address 0x214d r/w (MAIN CPU)

    0x80 - IOF:3
    0x40 - IOF:2
    0x20 - IOF:1
    0x10 - IOF:0
    0x08 - IOE:3
    0x04 - IOE:2
    0x02 - IOE:1
    0x01 - IOE:0
*/


/************************************************************************************************************************************
 VT1682 Sound CPU Registers
************************************************************************************************************************************/

/* Address 0x2100 - 0x2103 (SOUND CPU) - see vt1682_timer.cpp */

/* Address 0x2104 Unused (SOUND CPU) */
/* Address 0x2105 Unused (SOUND CPU) */
/* Address 0x2106 Unused (SOUND CPU) */
/* Address 0x2107 Unused (SOUND CPU) */
/* Address 0x2108 Unused (SOUND CPU) */
/* Address 0x2109 Unused (SOUND CPU) */
/* Address 0x210a Unused (SOUND CPU) */
/* Address 0x210b Unused (SOUND CPU) */
/* Address 0x210c Unused (SOUND CPU) */
/* Address 0x210d Unused (SOUND CPU) */
/* Address 0x210e Unused (SOUND CPU) */
/* Address 0x210f Unused (SOUND CPU) */

/* Address 0x2110 - 0x2113 (SOUND CPU) - see vt1682_timer.cpp */

/* Address 0x2114 Unused (SOUND CPU) */
/* Address 0x2115 Unused (SOUND CPU) */
/* Address 0x2116 Unused (SOUND CPU) */
/* Address 0x2117 Unused (SOUND CPU) */

/*
    Address 0x2118 r/w (SOUND CPU)

    0x80 - Audio DAC Left:7
    0x40 - Audio DAC Left:6
    0x20 - Audio DAC Left:5
    0x10 - Audio DAC Left:4
    0x08 - Audio DAC Left:3
    0x04 - Audio DAC Left:2
    0x02 - Audio DAC Left:1
    0x01 - Audio DAC Left:0

    actually 12 bits precision so only 15 to 4 are used
*/

uint8_t vt_vt1682_state::vt1682_soundcpu_2118_dacleft_7_0_r()
{
	uint8_t ret = m_soundcpu_2118_dacleft_7_0;
	//LOGMASKED(LOG_OTHER, "%s: vt1682_soundcpu_2118_dacleft_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_soundcpu_2118_dacleft_7_0_w(uint8_t data)
{
	//LOGMASKED(LOG_OTHER, "%s: vt1682_soundcpu_2118_dacleft_7_0_r writing: %02x\n", machine().describe_context(), data);
	m_soundcpu_2118_dacleft_7_0 = data;
}

/*
    Address 0x2119 r/w (SOUND CPU)

    0x80 - Audio DAC Left:15
    0x40 - Audio DAC Left:14
    0x20 - Audio DAC Left:13
    0x10 - Audio DAC Left:12
    0x08 - Audio DAC Left:11
    0x04 - Audio DAC Left:10
    0x02 - Audio DAC Left:9
    0x01 - Audio DAC Left:8
*/

uint8_t vt_vt1682_state::vt1682_soundcpu_2119_dacleft_15_8_r()
{
	uint8_t ret = m_soundcpu_2119_dacleft_15_8;
	//LOGMASKED(LOG_OTHER, "%s: vt1682_soundcpu_2119_dacleft_15_8_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_soundcpu_2119_dacleft_15_8_w(uint8_t data)
{
	//LOGMASKED(LOG_OTHER, "%s: vt1682_soundcpu_2119_dacleft_15_8_r writing: %02x\n", machine().describe_context(), data);
	m_soundcpu_2119_dacleft_15_8 = data;

	uint16_t dacdata = (m_soundcpu_2119_dacleft_15_8 << 8) | m_soundcpu_2118_dacleft_7_0;
	m_leftdac->write(dacdata >> 4);
}

/*
    Address 0x211a r/w (SOUND CPU)

    0x80 - Audio DAC Right:7
    0x40 - Audio DAC Right:6
    0x20 - Audio DAC Right:5
    0x10 - Audio DAC Right:4
    0x08 - Audio DAC Right:3
    0x04 - Audio DAC Right:2
    0x02 - Audio DAC Right:1
    0x01 - Audio DAC Right:0
*/

uint8_t vt_vt1682_state::vt1682_soundcpu_211a_dacright_7_0_r()
{
	uint8_t ret = m_soundcpu_211a_dacright_7_0;
	//LOGMASKED(LOG_OTHER, "%s: vt1682_soundcpu_211a_dacright_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_soundcpu_211a_dacright_7_0_w(uint8_t data)
{
	//LOGMASKED(LOG_OTHER, "%s: vt1682_soundcpu_211a_dacright_7_0_r writing: %02x\n", machine().describe_context(), data);
	m_soundcpu_211a_dacright_7_0 = data;
}

/*
    Address 0x211b r/w (SOUND CPU)

    0x80 - Audio DAC Right:15
    0x40 - Audio DAC Right:14
    0x20 - Audio DAC Right:13
    0x10 - Audio DAC Right:12
    0x08 - Audio DAC Right:11
    0x04 - Audio DAC Right:10
    0x02 - Audio DAC Right:9
    0x01 - Audio DAC Right:8
*/

uint8_t vt_vt1682_state::vt1682_soundcpu_211b_dacright_15_8_r()
{
	uint8_t ret = m_soundcpu_211b_dacright_15_8;
	//LOGMASKED(LOG_OTHER, "%s: vt1682_soundcpu_211b_dacright_15_8_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vt_vt1682_state::vt1682_soundcpu_211b_dacright_15_8_w(uint8_t data)
{
	//LOGMASKED(LOG_OTHER, "%s: vt1682_soundcpu_211b_dacright_15_8_r writing: %02x\n", machine().describe_context(), data);
	m_soundcpu_211b_dacright_15_8 = data;

	uint16_t dacdata = (m_soundcpu_211b_dacright_15_8 << 8) | m_soundcpu_211a_dacright_7_0;
	m_rightdac->write(dacdata >> 4);
}


/*
    Address 0x211c WRITE (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - IRQ_OUT
    0x08 - SLEEP
    0x04 - ExtIRQSel
    0x02 - NMI_WAKEUP_EN
    0x01 - ExtMask

    Address 0x211c READ (SOUND CPU)

    0x80 - Clear_CPU_IRQ
    0x40 - Clear_CPU_IRQ
    0x20 - Clear_CPU_IRQ
    0x10 - Clear_CPU_IRQ
    0x08 - Clear_CPU_IRQ
    0x04 - Clear_CPU_IRQ
    0x02 - Clear_CPU_IRQ
    0x01 - Clear_CPU_IRQ
*/

void vt_vt1682_state::vt1682_soundcpu_211c_reg_irqctrl_w(uint8_t data)
{
	// EXT2421EN is used for ROM banking
	LOGMASKED(LOG_OTHER, "%s: vt1682_soundcpu_211c_reg_irqctrl_w writing: %02x\n", machine().describe_context(), data);

	if (data & 0x10)
	{
		// not seen used
		logerror("Main CPU IRQ Request from Sound CPU\n");
	}

	if (data & 0x08)
	{
		// documentation indicates that Sleep mode is buggy, so this probably never gets used
		popmessage("SCU Sleep\n");
	}
}

/*
    Address 0x211d r/w (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - IIS Mode
    0x01 - IIS EN
*/

/* Address 0x211E Unused? (maybe) (SOUND CPU) */
/* Address 0x211F Unused (SOUND CPU) */
/* Address 0x2120 Unused (SOUND CPU) */
/* Address 0x2121 Unused (SOUND CPU) */
/* Address 0x2122 Unused (SOUND CPU) */
/* Address 0x2123 Unused (SOUND CPU) */
/* Address 0x2124 Unused (SOUND CPU) */
/* Address 0x2125 Unused (SOUND CPU) */
/* Address 0x2126 Unused (SOUND CPU) */
/* Address 0x2127 Unused (SOUND CPU) */
/* Address 0x2128 Unused (SOUND CPU) */
/* Address 0x2129 Unused (SOUND CPU) */
/* Address 0x212a Unused (SOUND CPU) */
/* Address 0x212b Unused (SOUND CPU) */
/* Address 0x212c Unused (SOUND CPU) */
/* Address 0x212d Unused (SOUND CPU) */
/* Address 0x212e Unused (SOUND CPU) */
/* Address 0x212f Unused (SOUND CPU) */

/* Address 0x2130 - 0x2137 - see v1682_alu.cpp (device identical to main CPU device) */

/* Address 0x2138 Unused (SOUND CPU) */
/* Address 0x2139 Unused (SOUND CPU) */
/* Address 0x213a Unused (SOUND CPU) */
/* Address 0x213b Unused (SOUND CPU) */
/* Address 0x213c Unused (SOUND CPU) */
/* Address 0x213d Unused (SOUND CPU) */
/* Address 0x213e Unused (SOUND CPU) */
/* Address 0x213f Unused (SOUND CPU) */

/*
    Address 0x2140 r/w (SOUND CPU)

    0x80 - IOA DATA
    0x40 - IOA DATA
    0x20 - IOA DATA
    0x10 - IOA DATA
    0x08 - IOA DATA
    0x04 - IOA DATA
    0x02 - IOA DATA
    0x01 - IOA DATA
*/

/*
    Address 0x2141 r/w (SOUND CPU)

    0x80 - IOA DIR
    0x40 - IOA DIR
    0x20 - IOA DIR
    0x10 - IOA DIR
    0x08 - IOA DIR
    0x04 - IOA DIR
    0x02 - IOA DIR
    0x01 - IOA DIR
*/

/*
    Address 0x2142 r/w (SOUND CPU)

    0x80 - IOA PLH
    0x40 - IOA PLH
    0x20 - IOA PLH
    0x10 - IOA PLH
    0x08 - IOA PLH
    0x04 - IOA PLH
    0x02 - IOA PLH
    0x01 - IOA PLH
*/

/* Address 0x2143 Unused (SOUND CPU) */

/*
    Address 0x2144 r/w (SOUND CPU)

    0x80 - IOB DATA
    0x40 - IOB DATA
    0x20 - IOB DATA
    0x10 - IOB DATA
    0x08 - IOB DATA
    0x04 - IOB DATA
    0x02 - IOB DATA
    0x01 - IOB DATA
*/

/*
    Address 0x2145 r/w (SOUND CPU)

    0x80 - IOB DIR
    0x40 - IOB DIR
    0x20 - IOB DIR
    0x10 - IOB DIR
    0x08 - IOB DIR
    0x04 - IOB DIR
    0x02 - IOB DIR
    0x01 - IOB DIR
*/

/*
    Address 0x2146 r/w (SOUND CPU)

    0x80 - IOB PLH
    0x40 - IOB PLH
    0x20 - IOB PLH
    0x10 - IOB PLH
    0x08 - IOB PLH
    0x04 - IOB PLH
    0x02 - IOB PLH
    0x01 - IOB PLH
*/

void vt_vt1682_state::draw_tile_pixline(int segment, int tile, int tileline, int x, int y, int palselect, int pal, int is16pix_high, int is16pix_wide, int bpp, int depth, int opaque, int flipx, int flipy, const rectangle& cliprect)
{
	int tilesize_high = is16pix_high ? 16 : 8;

	if (y >= cliprect.min_y && y <= cliprect.max_y)
	{

		if (bpp == 3) pal = 0x0;
		if (bpp == 2) pal &= 0xc;

		int startaddress = segment;
		int linebytes;

		if (bpp == 3)
		{
			if (is16pix_wide)
			{
				linebytes = 16;
			}
			else
			{
				linebytes = 8;
			}
		}
		else if (bpp == 2)
		{
			if (is16pix_wide)
			{
				linebytes = 12;
			}
			else
			{
				linebytes = 6;
			}
		}
		else //if (bpp == 1) // or 0
		{
			if (is16pix_wide)
			{
				linebytes = 8;
			}
			else
			{
				linebytes = 4;
			}
		}
		int tilesize_wide = is16pix_wide ? 16 : 8;

		int tilebytes = linebytes * tilesize_high;

		startaddress += tilebytes * tile;

		int currentaddress;

		if (!flipy)
			currentaddress = startaddress + tileline * linebytes;
		else
			currentaddress = startaddress + ((tilesize_high - 1) - tileline) * linebytes;

		uint8_t *const pri2ptr = &m_pal2_priority_bitmap.pix(y);
		uint8_t *const pri1ptr = &m_pal1_priority_bitmap.pix(y);

		uint8_t *const pix2ptr = &m_pal2_pix_bitmap.pix(y);
		uint8_t *const pix1ptr = &m_pal1_pix_bitmap.pix(y);


		int shift_amount, mask, bytes_in;
		if (bpp == 3) // (8bpp)
		{
			shift_amount = 8;
			mask = 0xff;
			bytes_in = 4;
		}
		else if (bpp == 2) // (6bpp)
		{
			shift_amount = 6;
			mask = 0x3f;
			bytes_in = 3;
		}
		else // 1 / 0 (4bpp)
		{
			shift_amount = 4;
			mask = 0x0f;
			bytes_in = 2;
		}

		int xbase = x;

		for (int xx = 0; xx < tilesize_wide; xx += 4) // tile x pixels
		{
			// draw 4 pixels
			uint32_t pixdata = 0;

			int shift = 0;
			for (int i = 0; i < bytes_in; i++)
			{
				pixdata |= m_fullrom->read8(currentaddress) << shift; currentaddress++;
				shift += 8;
			}

			shift = 0;
			for (int ii = 0; ii < 4; ii++)
			{
				uint8_t pen = (pixdata >> shift)& mask;
				if (opaque || pen)
				{
					int xdraw_real;
					if (!flipx)
						xdraw_real = xbase + xx + ii; // pixel position
					else
						xdraw_real = xbase + ((tilesize_wide - 1) - xx - ii);

					if (xdraw_real >= cliprect.min_x && xdraw_real <= cliprect.max_x)
					{
						if (palselect & 1)
						{
							if (depth < pri1ptr[xdraw_real])
							{
								pix1ptr[xdraw_real] = pen | (pal << 4);
								pri1ptr[xdraw_real] = depth;
							}
						}
						if (palselect & 2)
						{
							if (depth < pri2ptr[xdraw_real])
							{
								pix2ptr[xdraw_real] = pen | (pal << 4);
								pri2ptr[xdraw_real] = depth;
							}
						}

					}
				}
				shift += shift_amount;
			}
		}
	}
}
void vt_vt1682_state::draw_tile(int segment, int tile, int x, int y, int palselect, int pal, int is16pix_high, int is16pix_wide, int bpp, int depth, int opaque, int flipx, int flipy, const rectangle& cliprect)
{
	int tilesize_high = is16pix_high ? 16 : 8;

	for (int yy = 0; yy < tilesize_high; yy++) // tile y lines
	{
		draw_tile_pixline(segment, tile, yy, x, y+yy, palselect, pal, is16pix_high, is16pix_wide, bpp, depth, opaque, flipx, flipy, cliprect);
	}
}

void vt_vt1682_state::setup_video_pages(int which, int tilesize, int vs, int hs, int y8, int x8, uint16_t* pagebases)
{
	int vs_hs = (vs << 1) | hs;
	int y8_x8 = (y8 << 1) | x8;

	pagebases[0] = 0xffff;
	pagebases[1] = 0xffff;
	pagebases[2] = 0xffff;
	pagebases[3] = 0xffff;


	if (!tilesize) // 8x8 mode
	{
		if (vs_hs == 0)
		{
			// 1x1 mode
			switch (y8_x8)
			{
			case 0x0:
				pagebases[0] = 0x000; /* 0x000-0x7ff */
				break;
			case 0x1:
				pagebases[0] = 0x800; /* 0x800-0xfff */
				break;
			case 0x2:
				pagebases[0] = 0x800; /* 0x800-0xfff */ // technically invalid?
				break;
			case 0x3:
				pagebases[0] = 0x800; /* 0x800-0xfff */ // technically invalid?
				break;
			}

			// mirror for rendering
			pagebases[1] = pagebases[0];
			pagebases[2] = pagebases[0];
			pagebases[3] = pagebases[0];
		}
		else if (vs_hs == 1)
		{
			// 2x1 mode
			switch (y8_x8)
			{
			case 0x0:
				pagebases[0] = 0x000; /* 0x000-0x7ff */ pagebases[1] = 0x800; /* 0x800-0xfff */
				break;
			case 0x1:
				pagebases[0] = 0x800; /* 0x800-0xfff */ pagebases[1] = 0x000; /* 0x000-0x7ff */
				break;
			case 0x2:
				pagebases[0] = 0x000; /* 0x000-0x7ff */ pagebases[1] = 0x800; /* 0x800-0xfff */
				break;
			case 0x3:
				pagebases[0] = 0x800; /* 0x800-0xfff */ pagebases[1] = 0x000; /* 0x000-0x7ff */
				break;
			}

			// mirror for rendering
			pagebases[2] = pagebases[0];
			pagebases[3] = pagebases[1];
		}
		else if (vs_hs == 2)
		{
			// 1x2 mode
			switch (y8_x8)
			{
			case 0x0:
				pagebases[0] = 0x000; /* 0x000-0x7ff */
				pagebases[2] = 0x800; /* 0x800-0xfff */
				break;
			case 0x1:
				pagebases[0] = 0x000; /* 0x000-0x7ff */
				pagebases[2] = 0x800; /* 0x800-0xfff */
				break;
			case 0x2:
				pagebases[0] = 0x800; /* 0x800-0xfff */
				pagebases[2] = 0x000; /* 0x000-0x7ff */
				break;
			case 0x3:
				pagebases[0] = 0x800; /* 0x800-0xfff */
				pagebases[2] = 0x000; /* 0x000-0x7ff */
				break;
			}

			// mirror for rendering
			pagebases[1] = pagebases[0];
			pagebases[3] = pagebases[2];
		}
		else if (vs_hs == 3)
		{
			// 2x2 mode

			// 4 pages in 8x8 is an INVALID MODE, set all bases to 0?
			pagebases[0] = 0x000;
			pagebases[1] = 0x000;
			pagebases[2] = 0x000;
			pagebases[3] = 0x000;
		}
	}
	else // 16x16 mode
	{
		if (vs_hs == 0)
		{
			// 1x1 mode
			switch (y8_x8)
			{
			case 0x0:
				pagebases[0] = 0x000; /* 0x000 - 0x1ff */
				break;
			case 0x1:
				pagebases[0] = 0x200; /* 0x200 - 0x3ff */
				break;
			case 0x2:
				pagebases[0] = 0x400; /* 0x400 - 0x5ff */
				break;
			case 0x3:
				pagebases[0] = 0x600; /* 0x600 - 0x7ff */
				break;
			}

			// mirror for rendering
			pagebases[1] = pagebases[0];
			pagebases[2] = pagebases[0];
			pagebases[3] = pagebases[0];
		}
		else if (vs_hs == 1)
		{
			// 2x1 mode
			switch (y8_x8)
			{
			case 0x0:
				pagebases[0] = 0x000; /* 0x000 - 0x1ff */ pagebases[1] = 0x200; /* 0x200 - 0x3ff */
				break;
			case 0x1:
				pagebases[0] = 0x200; /* 0x200 - 0x3ff */ pagebases[1] = 0x000; /* 0x000 - 0x1ff */
				break;
			case 0x2:
				pagebases[0] = 0x000; /* 0x000 - 0x1ff */ pagebases[1] = 0x200; /* 0x200 - 0x3ff */
				break;
			case 0x3:
				pagebases[0] = 0x200; /* 0x200 - 0x3ff */ pagebases[1] = 0x000; /* 0x000 - 0x1ff */
				break;
			}

			// mirror for rendering
			pagebases[2] = pagebases[0];
			pagebases[3] = pagebases[1];
		}
		else if (vs_hs == 2)
		{
			// 1x2 mode
			switch (y8_x8)
			{
			case 0x0:
				pagebases[0] = 0x000; /* 0x000 - 0x1ff */
				pagebases[2] = 0x200; /* 0x200 - 0x3ff */
				break;
			case 0x1:
				pagebases[0] = 0x000; /* 0x000 - 0x1ff */
				pagebases[2] = 0x200; /* 0x200 - 0x3ff */
				break;
			case 0x2:
				pagebases[0] = 0x200; /* 0x200 - 0x3ff */
				pagebases[2] = 0x000; /* 0x000 - 0x1ff */
				break;
			case 0x3:
				pagebases[0] = 0x200; /* 0x200 - 0x3ff */
				pagebases[2] = 0x000; /* 0x000 - 0x1ff */
				break;
			}

			// mirror for rendering
			pagebases[1] = pagebases[0];
			pagebases[3] = pagebases[2];
		}
		else if (vs_hs == 3)
		{
			// 2x2 mode
			switch (y8_x8)
			{
			case 0x0:
				pagebases[0] = 0x000; /* 0x000 - 0x1ff */ pagebases[1] = 0x200; /* 0x200 - 0x3ff */
				pagebases[2] = 0x400; /* 0x400 - 0x5ff */ pagebases[3] = 0x600; /* 0x600 - 0x7ff */
				break;
			case 0x1:
				pagebases[0] = 0x200; /* 0x200 - 0x3ff */ pagebases[1] = 0x000; /* 0x000 - 0x1ff */
				pagebases[2] = 0x600; /* 0x600 - 0x7ff */ pagebases[3] = 0x400; /* 0x400 - 0x5ff */
				break;
			case 0x2:
				pagebases[0] = 0x400; /* 0x400 - 0x5ff */ pagebases[1] = 0x600; /* 0x600 - 0x7ff */
				pagebases[2] = 0x000; /* 0x000 - 0x1ff */ pagebases[3] = 0x200; /* 0x200 - 0x3ff */
				break;
			case 0x3:
				pagebases[0] = 0x600; /* 0x600 - 0x7ff */ pagebases[1] = 0x400; /* 0x400 - 0x5ff */
				pagebases[2] = 0x200; /* 0x200 - 0x3ff */ pagebases[3] = 0x000; /* 0x000 - 0x1ff */
				break;
			}
		}
	}

	// for BK2 layer, in 16x16 mode, all tilebases are 0x800 higher
	if (tilesize && (which == 1))
	{
		pagebases[0] += 0x800;
		pagebases[1] += 0x800;
		pagebases[2] += 0x800;
		pagebases[3] += 0x800;
	}

	/*
	if ((pagebases[0] == 0xffff) || (pagebases[1] == 0xffff) || (pagebases[2] == 0xffff) || (pagebases[3] == 0xffff))
	{
	    fatalerror("failed to set config for tilemap:%1x, size:%1x vs:%1x hs:%1x y8:%1x x8:%1x", which, tilesize, vs, hs, y8, x8);
	}
	*/
}

int vt_vt1682_state::get_address_for_tilepos(int x, int y, int tilesize, uint16_t* pagebases)
{
	if (!tilesize) // 8x8 mode
	{
		// in 8x8 mode each page is 32 tiles wide and 32 tiles high
		// the pagebases structure is for 2x2 pages, so 64 tiles in each direction, pre-mirrored for smaller sizes
		// each page is 0x800 bytes
		// 0x40 bytes per line (0x2 bytes per tile, 0x20 tiles)

		x &= 0x3f;
		y &= 0x3f;

		if (x & 0x20) // right set of pages
		{
			x &= 0x1f;
			if (y & 0x20)// bottom set of pages
			{
				y &= 0x1f;
				return pagebases[3] + (y * 0x20 * 0x02) + (x * 0x2);
			}
			else // top set of pages
			{
				y &= 0x1f;
				return pagebases[1] + (y * 0x20 * 0x02) + (x * 0x2);
			}
		}
		else // left set of pages
		{
			x &= 0x1f;
			if (y & 0x20)// bottom set of pages
			{
				y &= 0x1f;
				return pagebases[2] + (y * 0x20 * 0x02) + (x * 0x2);
			}
			else // top set of pages
			{
				y &= 0x1f;
				return pagebases[0] + (y * 0x20 * 0x02) + (x * 0x2);
			}
		}
	}
	else // 16x16 mode
	{
		// in 16x16 mode each page is 16 tiles wide and 16 tiles high
		// the pagebases structure is for 2x2 pages, so 32 tiles in each direction, pre-mirrored for smaller sizes
		// each page is 0x100 bytes
		// 0x10 bytes per line (0x2 bytes per tile, 0x10 tiles)
		x &= 0x1f;
		y &= 0x1f;

		if (x & 0x10) // right set of pages
		{
			x &= 0x0f;
			if (y & 0x10)// bottom set of pages
			{
				y &= 0x0f;
				return pagebases[3] + (y * 0x10 * 0x02) + (x * 0x2);
			}
			else // top set of pages
			{
				y &= 0x0f;
				return pagebases[1] + (y * 0x10 * 0x02) + (x * 0x2);
			}
		}
		else // left set of pages
		{
			x &= 0x0f;
			if (y & 0x10)// bottom set of pages
			{
				y &= 0x0f;
				return pagebases[2] + (y * 0x10 * 0x02) + (x * 0x2);
			}
			else // top set of pages
			{
				y &= 0x0f;
				return pagebases[0] + (y * 0x10 * 0x02) + (x * 0x2);
			}
		}
	}
	// should never get here
	return 0x00;
}

/*
    Page Setups

    8x8 Mode  (Note, BK2 RAM arrangements are the same as BK1 in 8x8 mode)

    ---------------------------------------------------------------------------------------------------------------------------------
    |   Bk1 Reg |   Bk1 Reg |   Layout                              |   Bk2 Reg |   Bk2 Reg |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |   Vs  Hs  |   Y8  X8  |   resulting config                    |   Vs  Hs  |   Y8  X8  |   resulting config                    |
    ---------------------------------------------------------------------------------------------------------------------------------
    |   0   0   |   0   0   |   0x000 - 0x7ff                       |   0   0   |   0   0   |   0x000 - 0x7ff                       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   0   1   |   0x800 - 0x800                       |           |   0   1   |   0x800 - 0x800                       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   0   |   0x800 - 0x800                       |           |   1   0   |   0x800 - 0x800                       |
    |           |           |   (technically invalid?)              |           |           |   (technically invalid?)              |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   1   |   0x800 - 0x800                       |           |   1   1   |   0x800 - 0x800                       |
    |           |           |   (technically invalid?)              |           |           |   (technically invalid?)              |
    =================================================================================================================================
    |   0   1   |   0   0   |   0x000 - 0x7ff   0x800 - 0xfff       |   0   1   |   0   0   |   0x000 - 0x7ff   0x800 - 0xfff       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   0   1   |   0x800 - 0xfff   0x000 - 0xfff       |           |   0   1   |   0x800 - 0xfff   0x000 - 0xfff       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   0   |   0x000 - 0x7ff   0x800 - 0xfff       |           |   1   0   |   0x000 - 0x7ff   0x800 - 0xfff       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   1   |   0x800 - 0xfff   0x000 - 0xfff       |           |   1   1   |   0x800 - 0xfff   0x000 - 0xfff       |
    |           |           |                                       |           |           |                                       |
    =================================================================================================================================
    |   1   0   |   0   0   |   0x000 - 0x7ff                       |   1   0   |   0   0   |   0x000 - 0x7ff                       |
    |           |           |   0x800 - 0xfff                       |           |           |   0x800 - 0xfff                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   0   1   |   0x000 - 0x7ff                       |           |   0   1   |   0x000 - 0x7ff                       |
    |           |           |   0x800 - 0xfff                       |           |           |   0x800 - 0xfff                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   0   |   0x800 - 0xfff                       |           |   1   0   |   0x800 - 0xfff                       |
    |           |           |   0x000 - 0x7ff                       |           |           |   0x000 - 0x7ff                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   1   |   0x800 - 0xfff                       |           |   1   1   |   0x800 - 0xfff                       |
    |           |           |   0x000 - 0x7ff                       |           |           |   0x000 - 0x7ff                       |
    =================================================================================================================================
    |   1   1   |   0   0   |   Invalid (each page is 0x800 bytes,  |   1   1   |   0   0   |   Invalid (each page is 0x800 bytes,  |
    |           |           |    so not enough RAM for 4 pages)     |           |           |    so not enough RAM for 4 pages)     |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   0   1   |   Invalid (each page is 0x800 bytes,  |           |   0   1   |   Invalid (each page is 0x800 bytes,  |
    |           |           |    so not enough RAM for 4 pages)     |           |           |    so not enough RAM for 4 pages)     |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   0   |   Invalid (each page is 0x800 bytes,  |           |   1   0   |   Invalid (each page is 0x800 bytes,  |
    |           |           |    so not enough RAM for 4 pages)     |           |           |    so not enough RAM for 4 pages)     |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   1   |   Invalid (each page is 0x800 bytes,  |           |   1   1   |   Invalid (each page is 0x800 bytes,  |
    |           |           |    so not enough RAM for 4 pages)     |           |           |    so not enough RAM for 4 pages)     |
    =================================================================================================================================

    16x16 Mode  (Note, BK2 RAM base is different, with 0x800 added, compared to BK1 in 16x16 mode)

    ---------------------------------------------------------------------------------------------------------------------------------
    |   Bk1 Reg |   Bk1 Reg |   Layout                              |   Bk2 Reg |   Bk2 Reg |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |   Vs  Hs  |   Y8  X8  |   resulting config                    |   Vs  Hs  |   Y8  X8  |   resulting config                    |
    ---------------------------------------------------------------------------------------------------------------------------------
    |   0   0   |   0   0   |   0x000 - 0x1ff                       |   0   0   |   0   0   |   0x800 - 0x9ff                       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   0   1   |   0x200 - 0x3ff                       |           |   0   1   |   0xa00 - 0xbff                       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   0   |   0x400 - 0x5ff                       |           |   1   0   |   0xc00 - 0xdff                       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   1   |   0x600 - 0x7ff                       |           |   1   1   |   0xe00 - 0xfff                       |
    |           |           |                                       |           |           |                                       |
    =================================================================================================================================
    |   0   1   |   0   0   |   0x000 - 0x1ff   0x200 - 0x3ff       |   0   1   |   0   0   |   0x800 - 0x9ff   0xa00 - 0xbff       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   0   1   |   0x200 - 0x3ff   0x000 - 0x1ff       |           |   0   1   |   0xa00 - 0xbff   0x800 - 0x9ff       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   0   |   0x000 - 0x1ff   0x200 - 0x3ff       |           |   1   0   |   0x800 - 0x9ff   0xa00 - 0xbff       |
    |           |           |                                       |           |           |                                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   1   |   0x200 - 0x3ff   0x000 - 0x1ff       |           |   1   1   |   0xa00 - 0xbff   0x800 - 0x9ff       |
    |           |           |                                       |           |           |                                       |
    =================================================================================================================================
    |   1   0   |   0   0   |   0x000 - 0x1ff                       |   1   0   |   0   0   |   0x800 - 0x9ff                       |
    |           |           |   0x200 - 0x3ff                       |           |           |   0xa00 - 0xbff                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   0   1   |   0x000 - 0x1ff                       |           |   0   1   |   0x800 - 0x9ff                       |
    |           |           |   0x200 - 0x3ff                       |           |           |   0xa00 - 0xbff                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   0   |   0x200 - 0x3ff                       |           |   1   0   |   0xa00 - 0xbff                       |
    |           |           |   0x000 - 0x1ff                       |           |           |   0x800 - 0x9ff                       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   1   |   0x200 - 0x3ff                       |           |   1   1   |   0xa00 - 0xbff                       |
    |           |           |   0x000 - 0x1ff                       |           |           |   0x800 - 0x9ff                       |
    =================================================================================================================================
    |   1   1   |   0   0   |   0x000 - 0x1ff   0x200 - 0x3ff       |   1   1   |   0   0   |   0x800 - 0x9ff   0xa00 - 0xbff       |
    |           |           |   0x400 - 0x5ff   0x600 - 0x7ff       |           |           |   0xc00 - 0xdff   0xe00 - 0xfff       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   0   1   |   0x200 - 0x3ff   0x000 - 0x1ff       |           |   0   1   |   0xa00 - 0xbff   0x800 - 0x9ff       |
    |           |           |   0x600 - 0x7ff   0x400 - 0x5ff       |           |           |   0xe00 - 0xfff   0xc00 - 0xdff       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   0   |   0x400 - 0x5ff   0x600 - 0x7ff       |           |   1   0   |   0xc00 - 0xdff   0xe00 - 0xfff       |
    |           |           |   0x000 - 0x1ff   0x200 - 0x3ff       |           |           |   0x800 - 0x9ff   0xa00 - 0xbff       |
    ---------------------------------------------------------------------------------------------------------------------------------
    |           |   1   1   |   0x600 - 0x7ff   0x400 - 0x5ff       |           |   1   1   |   0xe00 - 0xfff   0xc00 - 0xdff       |
    |           |           |   0x200 - 0x3ff   0x000 - 0x1ff       |           |           |   0xa00 - 0xbff   0x800 - 0x9ff       |
    =================================================================================================================================
*/

void vt_vt1682_state::draw_layer(int which, int opaque, const rectangle& cliprect)
{
	int bk_tilesize = (m_main_control_bk[which] & 0x01);
	int bk_line = (m_main_control_bk[which] & 0x02) >> 1;
	int bk_tilebpp = (m_main_control_bk[which] & 0x0c) >> 2;
	int bk_depth = (m_main_control_bk[which] & 0x30) >> 4;
	int bk_paldepth_mode = (m_main_control_bk[which] & 0x40) >> 5; // called bkpal in places, bk_pal_select in others (in conflict with palselect below)
	int bk_enable = (m_main_control_bk[which] & 0x80) >> 7;

	if (bk_enable)
	{
		int xscroll = m_xscroll_7_0_bk[which];
		int yscroll = m_yscroll_7_0_bk[which];
		int xscrollmsb = (m_scroll_control_bk[which] & 0x01);
		int yscrollmsb = (m_scroll_control_bk[which] & 0x02) >> 1;
		int page_layout_h = (m_scroll_control_bk[which] & 0x04) >> 2;
		int page_layout_v = (m_scroll_control_bk[which] & 0x08) >> 3;
		int high_color = (m_scroll_control_bk[which] & 0x10) >> 4;

		/* must be some condition for this, as Maze Pac does not want this offset (confirmed no offset on hardware) but some others do (see Snake title for example)
		   documentation says it's a hw bug, for bk2 (+2 pixels), but conditions aren't understood, and bk1 clearly needs offset too
		   sprites and tilemaps on the select menu need to align too, without left edge scrolling glitches
		   judging this from videos is tricky, because there's another bug that causes the right-most column of pixels to not render for certain scroll values
		   and the right-most 2 columns of sprites to not render

		   does this come down to pal1/pal2 output mixing rather than specific layers?
		*/
		//if (which == 0)
		//  xscroll += 1;

		//if (which == 1)
		//  xscroll += 1;

		int segment = m_segment_7_0_bk[which];
		segment |= m_segment_11_8_bk[which] << 8;

		segment = segment * 0x2000;

		//xscroll |= xscrollmsb << 8;
		//yscroll |= yscrollmsb << 8;

		uint16_t bases[4];

		setup_video_pages(which, bk_tilesize, page_layout_v, page_layout_h, yscrollmsb, xscrollmsb, bases);

		//LOGMASKED(LOG_OTHER, "layer %d bases %04x %04x %04x %04x (scrolls x:%02x y:%02x)\n", which, bases[0], bases[1], bases[2], bases[3], xscroll, yscroll);

		if (!bk_line)
		{
			int palselect;
			if (which == 0) palselect = m_200f_bk_pal_sel & 0x03;
			else palselect = (m_200f_bk_pal_sel & 0x0c) >> 2;

			// Character Mode
			LOGMASKED(LOG_OTHER, "DRAWING ----- bk, Character Mode Segment base %08x, TileSize %1x Bpp %1x, Depth %1x PalDepthMode:%1x PalSelect:%1 PageLayout_V:%1x PageLayout_H:%1x XScroll %04x YScroll %04x\n", segment, bk_tilesize, bk_tilebpp, bk_depth, bk_paldepth_mode, palselect, page_layout_v, page_layout_h, xscroll, yscroll);

			for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				int ytile, ytileline;

				int ywithscroll = y - yscroll;

				if (bk_tilesize)
				{
					ytileline = ywithscroll & 0xf;
					ytile = ywithscroll >> 4;

				}
				else
				{
					ytileline = ywithscroll & 0x07;
					ytile = ywithscroll >> 3;
				}

				for (int xtile = -1; xtile < (bk_tilesize ? (16) : (32)); xtile++) // -1 due to possible need for partial tile during scrolling
				{
					int xscrolltile_part;
					int xscrolltile;
					if (bk_tilesize)
					{
						xscrolltile = xscroll >> 4;
						xscrolltile_part = xscroll & 0x0f;
					}
					else
					{
						xscrolltile = xscroll >> 3;
						xscrolltile_part = xscroll & 0x07;
					}


					int count = get_address_for_tilepos(xtile - xscrolltile, ytile, bk_tilesize, bases);

					uint16_t word = m_vram->read8(count);
					count++;
					word |= m_vram->read8(count) << 8;
					count++;

					int tile = word & 0x0fff;

					if (!tile) // verified
						continue;

					uint8_t pal = (word & 0xf000) >> 12;

					int xpos = xtile * (bk_tilesize ? 16 : 8);

					uint8_t realpal, realdepth;

					if (bk_paldepth_mode)
					{
						// this mode isn't tested, not seen it used
						//if (bk_paldepth_mode)
						//  popmessage("bk_paldepth_mode set\n");
						realdepth = pal & 0x03;

						// depth might instead be the high 2 bits in 4bpp mode
						realpal = (pal & 0x0c) | bk_depth;
					}
					else
					{
						realpal = pal;
						realdepth = bk_depth;
					}

					draw_tile_pixline(segment, tile, ytileline, xpos + xscrolltile_part, y, palselect, realpal, bk_tilesize, bk_tilesize, bk_tilebpp, (realdepth * 2) + 1, opaque, 0, 0, cliprect);
				}
			}
		}
		else
		{
			// Line Mode

			if (high_color)
			{
				popmessage("high colour line mode\n");
			}
			else
			{
				popmessage("line mode\n");
			}
		}
	}
}

void vt_vt1682_state::draw_sprites(const rectangle& cliprect)
{
	int sp_en = (m_2018_spregs & 0x04) >> 2;
	int sp_pal_sel = (m_2018_spregs & 0x08) >> 3;
	int sp_size = (m_2018_spregs & 0x03);

	int segment = m_201a_sp_segment_7_0;
	segment |= m_201b_sp_segment_11_8 << 8;
	segment = segment * 0x2000;
	// if we don't do the skipping in inc_spriteram_addr this would need to be 5 instead
	const int SPRITE_STEP = 8;


	if (sp_en)
	{
		for (int line = cliprect.min_y; line <= cliprect.max_y; line++)
		{
			for (int i = 0; i < 240; i++)
			{
				int attr2 = m_spriteram->read8((i * SPRITE_STEP) + 5);

				int ystart = m_spriteram->read8((i * SPRITE_STEP) + 4);

				if (attr2 & 0x01)
					ystart -= 256;

				int yend = ystart + ((sp_size & 0x2) ? 16 : 8);

				// TODO, cache first 16 sprites per scanline which meet the critera to a list during hblank, set overflow flag if more requested
				// (do tilenum = 0 sprites count against this limit?)

				if (line >= ystart && line < yend)
				{
					int ytileline = line - ystart;

					int tilenum = m_spriteram->read8((i * SPRITE_STEP) + 0);
					int attr0 = m_spriteram->read8((i * SPRITE_STEP) + 1);
					int x = m_spriteram->read8((i * SPRITE_STEP) + 2);
					int attr1 = m_spriteram->read8((i * SPRITE_STEP) + 3);

					tilenum |= (attr0 & 0x0f) << 8;

					if (!tilenum) // verified
						continue;

					int pal = (attr0 & 0xf0) >> 4;

					int flipx = (attr1 & 0x02) >> 1; // might not function correctly on hardware
					int flipy = (attr1 & 0x04) >> 2;

					int depth = (attr1 & 0x18) >> 3;

					if (attr1 & 0x01)
						x -= 256;

					// guess! Maze Pac needs sprites shifted left by 1, but actual conditions might be more complex
					//if ((!sp_size & 0x01))
					//  x -= 1;

					int palselect = 0;
					if (sp_pal_sel)
					{
						// sprites are rendered to both buffers
						palselect = 3;
					}
					else
					{
						if (attr2 & 0x02)
							palselect = 2;
						else
							palselect = 1;
					}

					draw_tile_pixline(segment, tilenum, ytileline, x, line, palselect, pal, sp_size & 0x2, sp_size & 0x1, 0, depth * 2, 0, flipx, flipy, cliprect);

				}
			}
		}
		// if more than 16 sprites on any line 0x2001 bit 0x40 (SP_ERR) should be set (updated every line, can only be read in HBLANK)
	}
}

uint32_t vt_vt1682_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	m_pal2_priority_bitmap.fill(0xff, cliprect);
	m_pal1_priority_bitmap.fill(0xff, cliprect);
	m_pal2_pix_bitmap.fill(0x00, cliprect);
	m_pal1_pix_bitmap.fill(0x00, cliprect);

	bitmap.fill(0, cliprect);

	draw_layer(0, 0, cliprect);

	draw_layer(1, 0, cliprect);

	draw_sprites(cliprect);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		pen_t const *const paldata = m_palette->pens();
		uint8_t const *const pri2ptr = &m_pal2_priority_bitmap.pix(y);
		uint8_t const *const pri1ptr = &m_pal1_priority_bitmap.pix(y);
		uint8_t const *const pix2ptr = &m_pal2_pix_bitmap.pix(y);
		uint8_t const *const pix1ptr = &m_pal1_pix_bitmap.pix(y);
		uint32_t *const dstptr = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint8_t pix1 = pix1ptr[x];
			uint8_t pix2 = pix2ptr[x];
			uint8_t pri1 = pri1ptr[x];
			uint8_t pri2 = pri2ptr[x];

			// TODO: bit 0x8000 in palette can cause the layer to 'dig through'
			// palette layers can also be turned off, or just sent to lcd / just sent to tv
			// layers can also blend 50/50 rather than using depth

			// the transparency fallthrough here works for Boxing, but appears to be incorrect for Lawn Purge title screen (assuming it isn't an offset issue)

			if (pri1 <= pri2)
			{
				if (pix1) dstptr[x] = paldata[pix1 | 0x100];
				else
				{
					if (pix2) dstptr[x] = paldata[pix2];
					else dstptr[x] = paldata[0x100];
				}
			}
			else
			{
				if (pix2) dstptr[x] = paldata[pix2];
				else
				{
					if (pix1) dstptr[x] = paldata[pix1 | 0x100];
					else dstptr[x] = paldata[0x000];
				}
			}
		}
	}

	return 0;
}

// VT1682 can address 25-bit address space (32MB of ROM)
void vt_vt1682_state::rom_map(address_map &map)
{
	map(0x0000000, 0x1ffffff).bankr("cartbank");
}

// 11-bits (0x800 bytes) for sprites
void vt_vt1682_state::spriteram_map(address_map &map)
{
	map(0x000, 0x7ff).ram();
}

// 16-bits (0x10000 bytes) for vram (maybe mirrors at 0x2000?)
void vt_vt1682_state::vram_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1bff).ram(); // this gets cleared, but apparently is 'reserved'
	map(0x1c00, 0x1fff).ram().w("palette", FUNC(palette_device::write8)).share("palette"); // palette 2
}

// for the 2nd, faster, CPU
void vt_vt1682_state::vt_vt1682_sound_map(address_map& map)
{
	map(0x0000, 0x0fff).ram().share("sound_share");
	map(0x1000, 0x1fff).ram().share("sound_share");
	// 3000-3fff internal ROM if enabled

	map(0x2100, 0x2100).rw(m_soundcpu_timer_a_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_7_0_r),  FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_7_0_w));
	map(0x2101, 0x2101).rw(m_soundcpu_timer_a_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_15_8_r), FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_15_8_w));
	map(0x2102, 0x2102).rw(m_soundcpu_timer_a_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_enable_r),       FUNC(vrt_vt1682_timer_device::vt1682_timer_enable_w));
	map(0x2103, 0x2103).w( m_soundcpu_timer_a_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_irqclear_w));

	map(0x2110, 0x2110).rw(m_soundcpu_timer_b_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_7_0_r),  FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_7_0_w));
	map(0x2111, 0x2111).rw(m_soundcpu_timer_b_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_15_8_r), FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_15_8_w));
	map(0x2112, 0x2112).rw(m_soundcpu_timer_b_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_enable_r),       FUNC(vrt_vt1682_timer_device::vt1682_timer_enable_w));
	map(0x2113, 0x2113).w( m_soundcpu_timer_b_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_irqclear_w));

	map(0x2118, 0x2118).rw(FUNC(vt_vt1682_state::vt1682_soundcpu_2118_dacleft_7_0_r), FUNC(vt_vt1682_state::vt1682_soundcpu_2118_dacleft_7_0_w));
	map(0x2119, 0x2119).rw(FUNC(vt_vt1682_state::vt1682_soundcpu_2119_dacleft_15_8_r), FUNC(vt_vt1682_state::vt1682_soundcpu_2119_dacleft_15_8_w));
	map(0x211a, 0x211a).rw(FUNC(vt_vt1682_state::vt1682_soundcpu_211a_dacright_7_0_r), FUNC(vt_vt1682_state::vt1682_soundcpu_211a_dacright_7_0_w));
	map(0x211b, 0x211b).rw(FUNC(vt_vt1682_state::vt1682_soundcpu_211b_dacright_15_8_r), FUNC(vt_vt1682_state::vt1682_soundcpu_211b_dacright_15_8_w));

	map(0x211c, 0x211c).w(FUNC(vt_vt1682_state::vt1682_soundcpu_211c_reg_irqctrl_w));

	map(0x2130, 0x2130).rw(m_soundcpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_1_r), FUNC(vrt_vt1682_alu_device::alu_oprand_1_w));
	map(0x2131, 0x2131).rw(m_soundcpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_2_r), FUNC(vrt_vt1682_alu_device::alu_oprand_2_w));
	map(0x2132, 0x2132).rw(m_soundcpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_3_r), FUNC(vrt_vt1682_alu_device::alu_oprand_3_w));
	map(0x2133, 0x2133).rw(m_soundcpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_4_r), FUNC(vrt_vt1682_alu_device::alu_oprand_4_w));
	map(0x2134, 0x2134).rw(m_soundcpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_5_r), FUNC(vrt_vt1682_alu_device::alu_oprand_5_mult_w));
	map(0x2135, 0x2135).rw(m_soundcpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_6_r), FUNC(vrt_vt1682_alu_device::alu_oprand_6_mult_w));
	map(0x2136, 0x2136).w(m_soundcpu_alu, FUNC(vrt_vt1682_alu_device::alu_oprand_5_div_w));
	map(0x2137, 0x2137).w(m_soundcpu_alu, FUNC(vrt_vt1682_alu_device::alu_oprand_6_div_w));

	map(0xf000, 0xffff).ram().share("sound_share"); // doesn't actually map here, the CPU fetches vectors from 0x0ff0 - 0x0fff!

	map(0xfffe, 0xffff).r(FUNC(vt_vt1682_state::soundcpu_irq_vector_hack_r)); // probably need custom IRQ support in the core instead...
}

void vt_vt1682_state::vt_vt1682_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().share("sound_share");
	map(0x1ff4, 0x1fff).w(FUNC(vt_vt1682_state::vt1682_sound_reset_hack_w));

	/* Video */
	map(0x2000, 0x2000).rw(FUNC(vt_vt1682_state::vt1682_2000_r), FUNC(vt_vt1682_state::vt1682_2000_w));
	map(0x2001, 0x2001).rw(FUNC(vt_vt1682_state::vt1682_2001_vblank_r), FUNC(vt_vt1682_state::vt1682_2001_w));
	map(0x2002, 0x2002).rw(FUNC(vt_vt1682_state::vt1682_2002_sprramaddr_2_0_r), FUNC(vt_vt1682_state::vt1682_2002_sprramaddr_2_0_w));
	map(0x2003, 0x2003).rw(FUNC(vt_vt1682_state::vt1682_2003_sprramaddr_10_3_r), FUNC(vt_vt1682_state::vt1682_2003_sprramaddr_10_3_w));
	map(0x2004, 0x2004).rw(FUNC(vt_vt1682_state::vt1682_2004_sprram_data_r), FUNC(vt_vt1682_state::vt1682_2004_sprram_data_w));
	map(0x2005, 0x2005).rw(FUNC(vt_vt1682_state::vt1682_2005_vramaddr_7_0_r), FUNC(vt_vt1682_state::vt1682_2005_vramaddr_7_0_w));
	map(0x2006, 0x2006).rw(FUNC(vt_vt1682_state::vt1682_2006_vramaddr_15_8_r), FUNC(vt_vt1682_state::vt1682_2006_vramaddr_15_8_w));
	map(0x2007, 0x2007).rw(FUNC(vt_vt1682_state::vt1682_2007_vram_data_r), FUNC(vt_vt1682_state::vt1682_2007_vram_data_w));
	map(0x2008, 0x2008).rw(FUNC(vt_vt1682_state::vt1682_2008_lcd_vs_delay_r), FUNC(vt_vt1682_state::vt1682_2008_lcd_vs_delay_w));
	map(0x2009, 0x2009).rw(FUNC(vt_vt1682_state::vt1682_2009_lcd_hs_delay_7_0_r), FUNC(vt_vt1682_state::vt1682_2009_lcd_hs_delay_7_0_w));
	map(0x200a, 0x200a).rw(FUNC(vt_vt1682_state::vt1682_200a_lcd_fr_delay_7_0_r), FUNC(vt_vt1682_state::vt1682_200a_lcd_fr_delay_7_0_w));
	map(0x200b, 0x200b).rw(FUNC(vt_vt1682_state::vt1682_200b_misc_vregs0_r), FUNC(vt_vt1682_state::vt1682_200b_misc_vregs0_w));
	map(0x200c, 0x200c).rw(FUNC(vt_vt1682_state::vt1682_200c_misc_vregs1_r), FUNC(vt_vt1682_state::vt1682_200c_misc_vregs1_w));
	map(0x200d, 0x200d).rw(FUNC(vt_vt1682_state::vt1682_200d_misc_vregs2_r), FUNC(vt_vt1682_state::vt1682_200d_misc_vregs2_w));
	map(0x200e, 0x200e).rw(FUNC(vt_vt1682_state::vt1682_200e_blend_pal_sel_r), FUNC(vt_vt1682_state::vt1682_200e_blend_pal_sel_w));
	map(0x200f, 0x200f).rw(FUNC(vt_vt1682_state::vt1682_200f_bk_pal_sel_r), FUNC(vt_vt1682_state::vt1682_200f_bk_pal_sel_w));
	map(0x2010, 0x2010).rw(FUNC(vt_vt1682_state::vt1682_2010_bk1_xscroll_7_0_r), FUNC(vt_vt1682_state::vt1682_2010_bk1_xscroll_7_0_w));
	map(0x2011, 0x2011).rw(FUNC(vt_vt1682_state::vt1682_2011_bk1_yscoll_7_0_r), FUNC(vt_vt1682_state::vt1682_2011_bk1_yscoll_7_0_w));
	map(0x2012, 0x2012).rw(FUNC(vt_vt1682_state::vt1682_2012_bk1_scroll_control_r), FUNC(vt_vt1682_state::vt1682_2012_bk1_scroll_control_w));
	map(0x2013, 0x2013).rw(FUNC(vt_vt1682_state::vt1682_2013_bk1_main_control_r), FUNC(vt_vt1682_state::vt1682_2013_bk1_main_control_w));
	map(0x2014, 0x2014).rw(FUNC(vt_vt1682_state::vt1682_2014_bk2_xscroll_7_0_r), FUNC(vt_vt1682_state::vt1682_2014_bk2_xscroll_7_0_w));
	map(0x2015, 0x2015).rw(FUNC(vt_vt1682_state::vt1682_2015_bk2_yscoll_7_0_r), FUNC(vt_vt1682_state::vt1682_2015_bk2_yscoll_7_0_w));
	map(0x2016, 0x2016).rw(FUNC(vt_vt1682_state::vt1682_2016_bk2_scroll_control_r), FUNC(vt_vt1682_state::vt1682_2016_bk2_scroll_control_w));
	map(0x2017, 0x2017).rw(FUNC(vt_vt1682_state::vt1682_2017_bk2_main_control_r), FUNC(vt_vt1682_state::vt1682_2017_bk2_main_control_w));
	map(0x2018, 0x2018).rw(FUNC(vt_vt1682_state::vt1682_2018_spregs_r), FUNC(vt_vt1682_state::vt1682_2018_spregs_w));
	map(0x2019, 0x2019).rw(FUNC(vt_vt1682_state::vt1682_2019_bkgain_r), FUNC(vt_vt1682_state::vt1682_2019_bkgain_w));
	map(0x201a, 0x201a).rw(FUNC(vt_vt1682_state::vt1682_201a_sp_segment_7_0_r), FUNC(vt_vt1682_state::vt1682_201a_sp_segment_7_0_w));
	map(0x201b, 0x201b).rw(FUNC(vt_vt1682_state::vt1682_201b_sp_segment_11_8_r), FUNC(vt_vt1682_state::vt1682_201b_sp_segment_11_8_w));
	map(0x201c, 0x201c).rw(FUNC(vt_vt1682_state::vt1682_201c_bk1_segment_7_0_r), FUNC(vt_vt1682_state::vt1682_201c_bk1_segment_7_0_w));
	map(0x201d, 0x201d).rw(FUNC(vt_vt1682_state::vt1682_201d_bk1_segment_11_8_r), FUNC(vt_vt1682_state::vt1682_201d_bk1_segment_11_8_w));
	map(0x201e, 0x201e).rw(FUNC(vt_vt1682_state::vt1682_201e_bk2_segment_7_0_r), FUNC(vt_vt1682_state::vt1682_201e_bk2_segment_7_0_w));
	map(0x201f, 0x201f).rw(FUNC(vt_vt1682_state::vt1682_201f_bk2_segment_11_8_r), FUNC(vt_vt1682_state::vt1682_201f_bk2_segment_11_8_w));
	map(0x2020, 0x2020).rw(FUNC(vt_vt1682_state::vt1682_2020_bk_linescroll_r), FUNC(vt_vt1682_state::vt1682_2020_bk_linescroll_w));
	map(0x2021, 0x2021).rw(FUNC(vt_vt1682_state::vt1682_2021_lum_offset_r), FUNC(vt_vt1682_state::vt1682_2021_lum_offset_w));
	map(0x2022, 0x2022).rw(FUNC(vt_vt1682_state::vt1682_2022_saturation_misc_r), FUNC(vt_vt1682_state::vt1682_2022_saturation_misc_w));
	map(0x2023, 0x2023).rw(FUNC(vt_vt1682_state::vt1682_2023_lightgun_reset_r), FUNC(vt_vt1682_state::vt1682_2023_lightgun_reset_w));
	map(0x2024, 0x2024).rw(FUNC(vt_vt1682_state::vt1682_2024_lightgun1_y_r), FUNC(vt_vt1682_state::vt1682_2024_lightgun1_y_w));
	map(0x2025, 0x2025).rw(FUNC(vt_vt1682_state::vt1682_2025_lightgun1_x_r), FUNC(vt_vt1682_state::vt1682_2025_lightgun1_x_w));
	map(0x2026, 0x2026).rw(FUNC(vt_vt1682_state::vt1682_2026_lightgun2_y_r), FUNC(vt_vt1682_state::vt1682_2026_lightgun2_y_w));
	map(0x2027, 0x2027).rw(FUNC(vt_vt1682_state::vt1682_2027_lightgun2_x_r), FUNC(vt_vt1682_state::vt1682_2027_lightgun2_x_w));
	map(0x2028, 0x2028).rw(FUNC(vt_vt1682_state::vt1682_2028_r), FUNC(vt_vt1682_state::vt1682_2028_w));
	map(0x2029, 0x2029).rw(FUNC(vt_vt1682_state::vt1682_2029_r), FUNC(vt_vt1682_state::vt1682_2029_w));
	map(0x202a, 0x202a).rw(FUNC(vt_vt1682_state::vt1682_202a_r), FUNC(vt_vt1682_state::vt1682_202a_w));
	map(0x202b, 0x202b).rw(FUNC(vt_vt1682_state::vt1682_202b_r), FUNC(vt_vt1682_state::vt1682_202b_w));
	// 202c unused
	// 202d unused
	map(0x202e, 0x202e).rw(FUNC(vt_vt1682_state::vt1682_202e_r), FUNC(vt_vt1682_state::vt1682_202e_w));
	// 202f unused
	map(0x2030, 0x2030).rw(FUNC(vt_vt1682_state::vt1682_2030_r), FUNC(vt_vt1682_state::vt1682_2030_w));
	map(0x2031, 0x2031).rw(FUNC(vt_vt1682_state::vt1682_2031_red_dac_r), FUNC(vt_vt1682_state::vt1682_2031_red_dac_w));
	map(0x2032, 0x2032).rw(FUNC(vt_vt1682_state::vt1682_2032_green_dac_r), FUNC(vt_vt1682_state::vt1682_2032_green_dac_w));
	map(0x2033, 0x2033).rw(FUNC(vt_vt1682_state::vt1682_2033_blue_dac_r), FUNC(vt_vt1682_state::vt1682_2033_blue_dac_w));

	/* System */
	map(0x2100, 0x2100).rw(FUNC(vt_vt1682_state::vt1682_2100_prgbank1_r3_r), FUNC(vt_vt1682_state::vt1682_2100_prgbank1_r3_w));
	map(0x2101, 0x2101).rw(m_system_timer_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_7_0_r),  FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_7_0_w));
	map(0x2102, 0x2102).r(m_system_timer_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_enable_r));
	map(0x2102, 0x2102).w(FUNC(vt_vt1682_state::vt1682_timer_enable_trampoline_w));
	map(0x2103, 0x2103).w( m_system_timer_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_irqclear_w));
	map(0x2104, 0x2104).r(m_system_timer_dev, FUNC(vrt_vt1682_timer_device::vt1682_timer_preload_15_8_r));
	map(0x2104, 0x2104).w(FUNC(vt_vt1682_state::vt1682_timer_preload_15_8_trampoline_w));
	map(0x2105, 0x2105).w(FUNC(vt_vt1682_state::vt1682_2105_comr6_tvmodes_w));
	map(0x2106, 0x2106).rw(FUNC(vt_vt1682_state::vt1682_2106_enable_regs_r), FUNC(vt_vt1682_state::vt1682_2106_enable_regs_w));
	map(0x2107, 0x2107).rw(FUNC(vt_vt1682_state::vt1682_2107_prgbank0_r0_r), FUNC(vt_vt1682_state::vt1682_2107_prgbank0_r0_w));
	map(0x2108, 0x2108).rw(FUNC(vt_vt1682_state::vt1682_2108_prgbank0_r1_r), FUNC(vt_vt1682_state::vt1682_2108_prgbank0_r1_w));
	map(0x2109, 0x2109).rw(FUNC(vt_vt1682_state::vt1682_2109_prgbank0_r2_r), FUNC(vt_vt1682_state::vt1682_2109_prgbank0_r2_w));
	map(0x210a, 0x210a).rw(FUNC(vt_vt1682_state::vt1682_210a_prgbank0_r3_r), FUNC(vt_vt1682_state::vt1682_210a_prgbank0_r3_w));
	map(0x210b, 0x210b).rw(FUNC(vt_vt1682_state::vt1682_210b_misc_cs_prg0_bank_sel_r), FUNC(vt_vt1682_state::vt1682_210b_misc_cs_prg0_bank_sel_w));
	map(0x210c, 0x210c).rw(FUNC(vt_vt1682_state::vt1682_210c_prgbank1_r2_r), FUNC(vt_vt1682_state::vt1682_210c_prgbank1_r2_w));
	map(0x210d, 0x210d).rw(m_io, FUNC(vrt_vt1682_io_device::vt1682_210d_ioconfig_r),FUNC(vrt_vt1682_io_device::vt1682_210d_ioconfig_w));
	map(0x210e, 0x210e).rw(m_io, FUNC(vrt_vt1682_io_device::vt1682_210e_io_ab_r),FUNC(vrt_vt1682_io_device::vt1682_210e_io_ab_w));
	map(0x210f, 0x210f).rw(m_io, FUNC(vrt_vt1682_io_device::vt1682_210f_io_cd_r),FUNC(vrt_vt1682_io_device::vt1682_210f_io_cd_w));
	map(0x2110, 0x2110).rw(FUNC(vt_vt1682_state::vt1682_prgbank0_r4_r), FUNC(vt_vt1682_state::vt1682_prgbank1_r0_w)); // either reads/writes are on different addresses or our source info is incorrect
	map(0x2111, 0x2111).rw(FUNC(vt_vt1682_state::vt1682_prgbank0_r5_r), FUNC(vt_vt1682_state::vt1682_prgbank1_r1_w)); // ^
	map(0x2112, 0x2112).rw(FUNC(vt_vt1682_state::vt1682_prgbank1_r0_r), FUNC(vt_vt1682_state::vt1682_prgbank0_r4_w)); // ^
	map(0x2113, 0x2113).rw(FUNC(vt_vt1682_state::vt1682_prgbank1_r1_r), FUNC(vt_vt1682_state::vt1682_prgbank0_r5_w)); // ^
	// 2114 baud rade
	// 2115 baud rate
	// 2116 SPI
	// 2117 SPI
	map(0x2118, 0x2118).rw(FUNC(vt_vt1682_state::vt1682_2118_prgbank1_r4_r5_r), FUNC(vt_vt1682_state::vt1682_2118_prgbank1_r4_r5_w));
	// 2119 UART
	// 211a UART
	// 211b UART
	map(0x211c, 0x211c).w(FUNC(vt_vt1682_state::vt1682_211c_regs_ext2421_w));
	// 211d misc enable regs
	// 211e ADC
	// 211f voice gain
	// 2120 sleep period
	// 2121 misc interrupt masks / clears
	map(0x2122, 0x2122).rw(FUNC(vt_vt1682_state::vt1682_2122_dma_dt_addr_7_0_r), FUNC(vt_vt1682_state::vt1682_2122_dma_dt_addr_7_0_w));
	map(0x2123, 0x2123).rw(FUNC(vt_vt1682_state::vt1682_2123_dma_dt_addr_15_8_r), FUNC(vt_vt1682_state::vt1682_2123_dma_dt_addr_15_8_w));
	map(0x2124, 0x2124).rw(FUNC(vt_vt1682_state::vt1682_2124_dma_sr_addr_7_0_r), FUNC(vt_vt1682_state::vt1682_2124_dma_sr_addr_7_0_w));
	map(0x2125, 0x2125).rw(FUNC(vt_vt1682_state::vt1682_2125_dma_sr_addr_15_8_r), FUNC(vt_vt1682_state::vt1682_2125_dma_sr_addr_15_8_w));
	map(0x2126, 0x2126).rw(FUNC(vt_vt1682_state::vt1682_2126_dma_sr_bank_addr_22_15_r), FUNC(vt_vt1682_state::vt1682_2126_dma_sr_bank_addr_22_15_w));
	map(0x2127, 0x2127).rw(FUNC(vt_vt1682_state::vt1682_2127_dma_status_r), FUNC(vt_vt1682_state::vt1682_2127_dma_size_trigger_w));
	map(0x2128, 0x2128).rw(FUNC(vt_vt1682_state::vt1682_2128_dma_sr_bank_addr_24_23_r), FUNC(vt_vt1682_state::vt1682_2128_dma_sr_bank_addr_24_23_w));
	map(0x2129, 0x2129).rw(m_uio, FUNC(vrt_vt1682_uio_device::inteact_2129_uio_a_data_r), FUNC(vrt_vt1682_uio_device::inteact_2129_uio_a_data_w));
	map(0x212a, 0x212a).w(m_uio, FUNC(vrt_vt1682_uio_device::inteact_212a_uio_a_direction_w));
	map(0x212a, 0x212a).r(FUNC(vt_vt1682_state::inteact_212a_send_joy_clock2_r));
	map(0x212b, 0x212b).rw(m_uio, FUNC(vrt_vt1682_uio_device::inteact_212b_uio_a_attribute_r), FUNC(vrt_vt1682_uio_device::inteact_212b_uio_a_attribute_w));
	map(0x212c, 0x212c).rw(FUNC(vt_vt1682_state::vt1682_212c_prng_r), FUNC(vt_vt1682_state::vt1682_212c_prng_seed_w));
	// 212d PLL
	// 212e unused
	// 212f unused
	map(0x2130, 0x2130).rw(m_maincpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_1_r), FUNC(vrt_vt1682_alu_device::alu_oprand_1_w));
	map(0x2131, 0x2131).rw(m_maincpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_2_r), FUNC(vrt_vt1682_alu_device::alu_oprand_2_w));
	map(0x2132, 0x2132).rw(m_maincpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_3_r), FUNC(vrt_vt1682_alu_device::alu_oprand_3_w));
	map(0x2133, 0x2133).rw(m_maincpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_4_r), FUNC(vrt_vt1682_alu_device::alu_oprand_4_w));
	map(0x2134, 0x2134).rw(m_maincpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_5_r), FUNC(vrt_vt1682_alu_device::alu_oprand_5_mult_w));
	map(0x2135, 0x2135).rw(m_maincpu_alu, FUNC(vrt_vt1682_alu_device::alu_out_6_r), FUNC(vrt_vt1682_alu_device::alu_oprand_6_mult_w));
	map(0x2136, 0x2136).w(m_maincpu_alu, FUNC(vrt_vt1682_alu_device::alu_oprand_5_div_w));
	map(0x2137, 0x2137).w(m_maincpu_alu, FUNC(vrt_vt1682_alu_device::alu_oprand_6_div_w));

	map(0x2149, 0x2149).rw(m_uio, FUNC(vrt_vt1682_uio_device::inteact_2149_uio_b_data_r), FUNC(vrt_vt1682_uio_device::inteact_2149_uio_b_data_w));
	map(0x214a, 0x214a).rw(m_uio, FUNC(vrt_vt1682_uio_device::inteact_214a_uio_b_direction_r), FUNC(vrt_vt1682_uio_device::inteact_214a_uio_b_direction_w));
	map(0x214b, 0x214b).rw(m_uio, FUNC(vrt_vt1682_uio_device::inteact_214b_uio_b_attribute_r), FUNC(vrt_vt1682_uio_device::inteact_214b_uio_b_attribute_w));


	// 3000-3fff internal ROM if enabled
	map(0x4000, 0x7fff).r(FUNC(vt_vt1682_state::rom_4000_to_7fff_r));
	map(0x8000, 0xffff).r(FUNC(vt_vt1682_state::rom_8000_to_ffff_r));

	map(0xfffe, 0xffff).r(FUNC(vt_vt1682_state::maincpu_irq_vector_hack_r)); // probably need custom IRQ support in the core instead...
}

/*

Vectors / IRQ Levels

MAIN CPU:

SPI IRQ         0x7fff2 - 0x7fff3 (0xfff2 - 0xfff3)
UART IRQ        0x7fff4 - 0x7fff5 (0xfff4 - 0xfff5)
SCPU IRQ        0x7fff6 - 0x7fff7 (0xfff6 - 0xfff7)
Timer IRQ       0x7fff8 - 0x7fff9 (0xfff8 - 0xfff9)
NMI             0x7fffa - 0x7fffb (0xfffa - 0xfffb)
RESET           0x7fffc - 0x7fffd (0xfffc - 0xfffd)
Ext IRQ         0x7fffe - 0x7ffff (0xfffe - 0xffff)

SOUND CPU:

CPU IRQ         0x0ff4 - 0x0ff5
Timer2 IRQ      0x0ff6 - 0x0ff7
Timer1 IRQ      0x0ff8 - 0x0ff9
NMI             0x0ffa - 0x0ffb
RESET           0x0ffc - 0x0ffd
Ext IRQ         0x0ffe - 0x0fff

*/


uint8_t vt_vt1682_state::soundcpu_irq_vector_hack_r(offs_t offset)
{
	// redirect to Timer IRQ!
	return m_sound_share[0x0ff8 + offset];
}

uint8_t vt_vt1682_state::maincpu_irq_vector_hack_r(offs_t offset)
{
	// redirect to Timer IRQ!
	return rom_8000_to_ffff_r((0xfff8 - 0x8000)+offset);
}

// intg5410 writes a new program without resetting the CPU when selecting from the 'arcade' game main menu, this is problematic
// it does appear to rewrite the vectors first, so maybe there is some hardware side-effect of this putting the CPU in reset state??
void vt_vt1682_state::vt1682_sound_reset_hack_w(offs_t offset, uint8_t data)
{
	m_sound_share[0x0ff4 + offset] = data;
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void vt_vt1682_state::soundcpu_timera_irq(int state)
{
	if (state && !m_scpu_is_in_reset)
		m_soundcpu->set_input_line(0, ASSERT_LINE);
	else
		m_soundcpu->set_input_line(0, CLEAR_LINE);
}

void vt_vt1682_state::soundcpu_timerb_irq(int state)
{
// need to set proper vector (need IRQ priority manager function?)
/*
    if (state)
        m_soundcpu->set_input_line(0, ASSERT_LINE);
    else
        m_soundcpu->set_input_line(0, CLEAR_LINE);
*/
}

void vt_vt1682_state::maincpu_timer_irq(int state)
{
	// need to set proper vector (need IRQ priority manager function?)

	/* rasters are used on:

	   Highway Racing (title screen - scrolling split)
	   Fire Man (title screen - scrolling split)
	   Bee Fighting (title screen - scrolling split)
	   Over Speed (ingame rendering - road)
	   Motor Storm (ingame rendering - road)
	   Fish War (ingame rendering - status bar split)
	   Duel Soccer (ingame rendering - status bar split)
	*/

	if (state)
		m_maincpu->set_input_line(0, ASSERT_LINE);
	else
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(vt_vt1682_state::line_render_start)
{
	// some video reigsters latched in hblank, exact signal timings of irqs etc. is unknown
	// note Fireman titlescreen effect is off by one line on real hardware, as it is with this setup
	if ((param>=0) && (param<240))
		m_screen->update_partial(m_screen->vpos());

	m_render_timer->adjust(attotime::never);
}

TIMER_DEVICE_CALLBACK_MEMBER(vt_vt1682_state::scanline)
{
	int scanline = param;

	m_render_timer->adjust(m_screen->time_until_pos(m_screen->vpos(), 156), scanline);

	if (scanline == 240)
	{
		if (m_2000 & 0x01)
		{
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			if (!m_scpu_is_in_reset)
				m_soundcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero); // same enable? (NMI_EN on sub is 'wakeup NMI')
		}
	}
}

static const gfx_layout helper_8bpp_8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8 * 8 * 8
};

static const gfx_layout helper_8bpp_16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16 * 16 * 8
};

// hardware has line modes, so these views might be useful
static const uint32_t texlayout_xoffset_8bpp[256] = { STEP256(0,8) };
static const uint32_t texlayout_yoffset_8bpp[256] = { STEP256(0,256*8) };
static const gfx_layout texture_helper_8bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*8,
	texlayout_xoffset_8bpp,
	texlayout_yoffset_8bpp
};

static const uint32_t texlayout_xoffset_4bpp[256] = { STEP256(0,4) };
static const uint32_t texlayout_yoffset_4bpp[256] = { STEP256(0,256*4) };
static const gfx_layout texture_helper_4bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*4,
	texlayout_xoffset_4bpp,
	texlayout_yoffset_4bpp
};

// there are 6bpp gfx too, but they can't be decoded cleanly due to endian and alignment issues (start on what would be non-tile boundaries etc.)
static GFXDECODE_START( gfx_test )
	GFXDECODE_ENTRY( "mainrom", 0, texture_helper_4bpp_layout,  0x0, 2  )
	GFXDECODE_ENTRY( "mainrom", 0, helper_8bpp_8x8_layout,  0x0, 2  )
	GFXDECODE_ENTRY( "mainrom", 0, helper_8bpp_16x16_layout,  0x0, 2  )
	GFXDECODE_ENTRY( "mainrom", 0, texture_helper_8bpp_layout,  0x0, 2  )
GFXDECODE_END


void vt_vt1682_state::vt_vt1682_ntscbase(machine_config& config)
{
	/* basic machine hardware */
	M6502_SWAP_OP_D2_D7(config, m_maincpu, MAIN_CPU_CLOCK_NTSC);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt_vt1682_state::vt_vt1682_map);
	//m_maincpu->set_vblank_int("screen", FUNC(vt_vt1682_state::nmi));

	M6502(config, m_soundcpu, SOUND_CPU_CLOCK_NTSC);
	m_soundcpu->set_addrmap(AS_PROGRAM, &vt_vt1682_state::vt_vt1682_sound_map);

	VT_VT1682_TIMER(config, m_soundcpu_timer_a_dev, SOUND_CPU_CLOCK_NTSC);
	m_soundcpu_timer_a_dev->write_irq_callback().set(FUNC(vt_vt1682_state::soundcpu_timera_irq));
	m_soundcpu_timer_a_dev->set_sound_timer(); // different logging conditions
	VT_VT1682_TIMER(config, m_soundcpu_timer_b_dev, SOUND_CPU_CLOCK_NTSC);
	m_soundcpu_timer_b_dev->write_irq_callback().set(FUNC(vt_vt1682_state::soundcpu_timerb_irq));
	VT_VT1682_TIMER(config, m_system_timer_dev, MAIN_CPU_CLOCK_NTSC);
	m_system_timer_dev->write_irq_callback().set(FUNC(vt_vt1682_state::maincpu_timer_irq));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_refresh_hz(60);
	m_screen->set_size(300, 262); // 262 for NTSC, might be 261 if Vblank line is changed
	m_screen->set_visarea(0, 256-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(vt_vt1682_state::screen_update));
}

void vt_vt1682_state::vt_vt1682_palbase(machine_config& config)
{
	M6502_SWAP_OP_D2_D7(config, m_maincpu, MAIN_CPU_CLOCK_PAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt_vt1682_state::vt_vt1682_map);
	//m_maincpu->set_vblank_int("screen", FUNC(vt_vt1682_state::nmi));

	M6502(config, m_soundcpu, SOUND_CPU_CLOCK_PAL);
	m_soundcpu->set_addrmap(AS_PROGRAM, &vt_vt1682_state::vt_vt1682_sound_map);

	VT_VT1682_TIMER(config, m_soundcpu_timer_a_dev, SOUND_CPU_CLOCK_PAL);
	m_soundcpu_timer_a_dev->write_irq_callback().set(FUNC(vt_vt1682_state::soundcpu_timera_irq));
	m_soundcpu_timer_a_dev->set_sound_timer(); // different logging conditions
	VT_VT1682_TIMER(config, m_soundcpu_timer_b_dev, SOUND_CPU_CLOCK_PAL);
	m_soundcpu_timer_b_dev->write_irq_callback().set(FUNC(vt_vt1682_state::soundcpu_timerb_irq));
	VT_VT1682_TIMER(config, m_system_timer_dev, MAIN_CPU_CLOCK_PAL);
	m_system_timer_dev->write_irq_callback().set(FUNC(vt_vt1682_state::maincpu_timer_irq));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_refresh_hz(50.0070);
	m_screen->set_size(300, 312); // 312? for PAL
	m_screen->set_visarea(0, 256-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(vt_vt1682_state::screen_update));
}

void vt_vt1682_state::vt_vt1682_common(machine_config& config)
{
	TIMER(config, "scantimer").configure_scanline(FUNC(vt_vt1682_state::scanline), "screen", 0, 1);
	TIMER(config, m_render_timer).configure_generic(FUNC(vt_vt1682_state::line_render_start));

	VT_VT1682_ALU(config, m_maincpu_alu, 0);
	VT_VT1682_ALU(config, m_soundcpu_alu, 0);
	m_soundcpu_alu->set_sound_alu(); // different logging conditions

	config.set_maximum_quantum(attotime::from_hz(6000));

	ADDRESS_MAP_BANK(config, m_fullrom).set_map(&vt_vt1682_state::rom_map).set_options(ENDIANNESS_NATIVE, 8, 25, 0x2000000);

	ADDRESS_MAP_BANK(config, m_spriteram).set_map(&vt_vt1682_state::spriteram_map).set_options(ENDIANNESS_NATIVE, 8, 11, 0x800);
	ADDRESS_MAP_BANK(config, m_vram).set_map(&vt_vt1682_state::vram_map).set_options(ENDIANNESS_NATIVE, 8, 16, 0x10000);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x200).set_endianness(ENDIANNESS_LITTLE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_test);

	VT_VT1682_IO(config, m_io, 0);
	VT_VT1682_UIO(config, m_uio, 0);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DAC_12BIT_R2R(config, m_leftdac, 0).add_route(0, "lspeaker", 0.5); // unknown 12-bit DAC
	DAC_12BIT_R2R(config, m_rightdac, 0).add_route(0, "rspeaker", 0.5); // unknown 12-bit DAC
}


void vt_vt1682_state::vt_vt1682(machine_config &config)
{
	vt_vt1682_ntscbase(config);
	vt_vt1682_common(config);
}

void intec_interact_state::machine_start()
{
	vt_vt1682_state::machine_start();

	save_item(NAME(m_previous_port_b));
	save_item(NAME(m_input_sense));
	save_item(NAME(m_input_pos));
	save_item(NAME(m_current_bank));
}

void intec_interact_state::machine_reset()
{
	vt_vt1682_state::machine_reset();
	m_previous_port_b = 0x0;
	m_input_sense = 0;
	m_input_pos = 0;
	m_current_bank = 0;
	if (m_bank)
		m_bank->set_entry(m_current_bank & 0x03);
}


void vt1682_exsport_state::machine_start()
{
	vt_vt1682_state::machine_start();

	save_item(NAME(m_old_portb));
	save_item(NAME(m_portb_shiftpos));
	save_item(NAME(m_p1_latch));
	save_item(NAME(m_p2_latch));
}

void vt1682_exsport_state::machine_reset()
{
	vt_vt1682_state::machine_reset();

	m_old_portb = 0;
	m_portb_shiftpos = 0;
	m_p1_latch = 0;
	m_p2_latch = 0;
}

void intec_interact_state::ext_rombank_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: ext_rombank_w writing: %1x\n", machine().describe_context(), data);

	// Seems no way to unset a bank once set? program will write 0 here, and even taking into account direction
	// registers that would result in the bank bits being cleared, when running from a higher bank, which
	// crashes the program.  The game offers no 'back' option, so maybe this really is the correct logic.

	if (data & 0x01)
		m_current_bank |= 1;

	if (data & 0x02)
		m_current_bank |= 2;

	m_bank->set_entry(m_current_bank & 0x03);
};


void intec_interact_state::porta_w(uint8_t data)
{
	if (data != 0xf)
	{
		LOGMASKED(LOG_OTHER, "%s: porta_w writing: %1x\n", machine().describe_context(), data & 0xf);
	}
}


static INPUT_PORTS_START( intec )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) // Selects games
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Select") // used on first screen to choose which set of games
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // Fires in Tank

	PORT_START("IN2") // are these used? 2 player games all seem to be turn based? (Aqua-Mix looks like it should be 2 player but nothing here starts a 2 player game, maybe mapped in some other way?)
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
INPUT_PORTS_END

static INPUT_PORTS_START( miwi2 )
	PORT_INCLUDE( intec )

	PORT_MODIFY("IN3") // the 2nd drum appears to act like a single 2nd player controller? (even if none of the player 2 controls work in this port for intec?)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // Pink Drum in Drum Master
INPUT_PORTS_END

static INPUT_PORTS_START( 110dance )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Pad Up-Right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pad Up-Left")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Back")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Select / Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Pad Up") PORT_16WAY // NOT A JOYSTICK!!
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Pad Down") PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Pad Left") PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Pad Right") PORT_16WAY
INPUT_PORTS_END

static INPUT_PORTS_START( gm235upc )
	PORT_START("IN0")
INPUT_PORTS_END

static INPUT_PORTS_START( lxts3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END

static INPUT_PORTS_START( njp60in1 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )
INPUT_PORTS_END

static INPUT_PORTS_START( exsprt48 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( dance555 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Pad Up") PORT_16WAY // NOT A JOYSTICK!!
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Pad Down") PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Pad Left") PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Pad Right") PORT_16WAY

	PORT_START("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


// this controller code is just designed to feed the games with data they're happy with, it probably has no grounds in reality
// as I don't know how they really work.  presumably wireless with timeouts, sending signals for brief periods that need to be
// picked up on, although that said, there are some very short (128 read on status) timeout loops in the code that will force
// input to 0 if they fail

// note, the real hardware has multiple 'motion' accessories, but in reality they all just act like a button press

// inputs aren't working correctly on ii8in1, you can change to the bowling game, and select that, but select doesn't continue
// to move between games, why not?  ram address 0x6c contains current selection if you want to manually change it to start
// other games.  maybe it's waiting on some status from the sound cpu?

uint8_t intec_interact_state::porta_r()
{
	uint8_t ret = 0x0;// = machine().rand() & 0xf;

	switch (m_input_pos)
	{
	case 0x1: ret = m_io_p1->read(); break;
	case 0x2: ret = m_io_p2->read(); break;
	case 0x3: ret = m_io_p3->read(); break;
	case 0x4: ret = m_io_p4->read(); break;
	}

	//LOGMASKED(LOG_OTHER, "%s: porta_r returning: %1x (INPUTS) (with input position %d)\n", machine().describe_context(), ret, m_input_pos);
	return ret;
}

uint8_t intec_interact_state::portc_r()
{
	uint8_t ret = 0x0;
	ret |= m_input_sense ^1;
	//LOGMASKED(LOG_OTHER, "%s: portc_r returning: %1x (CONTROLLER INPUT SENSE)\n", machine().describe_context(), ret);
	return ret;
}

void intec_interact_state::portb_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: portb_w writing: %1x\n", machine().describe_context(), data & 0xf);

	if ((m_previous_port_b & 0x1) && (!(data & 0x1)))
	{
		// 0x1 high -> low
		LOGMASKED(LOG_OTHER, "high to low\n");

		if (m_input_sense == 1)
		{
			m_input_pos++;
		}
		else
		{
			m_input_sense = 1;
		}
		LOGMASKED(LOG_OTHER, "input pos is %d\n", m_input_pos);

	}
	else if ((m_previous_port_b & 0x1) && (data & 0x1))
	{
		// 0x1 high -> high
		LOGMASKED(LOG_OTHER, "high to high\n");
		m_input_pos = 0;
	}
	else if ((!(m_previous_port_b & 0x1)) && (!(data & 0x1)))
	{
		// 0x1 low -> low
		LOGMASKED(LOG_OTHER, "low to low\n");

		if (m_input_sense == 1)
		{
			m_input_pos = 0;
		}
	}
	else if ((!(m_previous_port_b & 0x1)) && (data & 0x1))
	{
		// 0x1 low -> high
		LOGMASKED(LOG_OTHER, "low to high\n");

		if (m_input_sense == 1)
		{
			m_input_pos++;
		}

		if (m_input_pos == 5)
		{
			m_input_sense = 0;
		}

		LOGMASKED(LOG_OTHER, "input pos is %d\n", m_input_pos);

	}

	m_previous_port_b = data;
};

void vt1682_exsport_state::clock_joy2()
{
	m_portb_shiftpos++;
}

uint8_t vt1682_exsport_state::uiob_r()
{
	int p1bit = (m_p1_latch >> m_portb_shiftpos) & 1;
	int p2bit = (m_p2_latch >> m_portb_shiftpos) & 1;

	return (p1bit << 1) | (p2bit << 3);
};

void vt1682_exsport_state::uiob_w(uint8_t data)
{
	if ((m_old_portb & 0x01) != (data & 0x01))
	{
		if (!(data & 0x01))
		{
			m_portb_shiftpos = 0;

			//logerror("%s: reset shift\n", machine().describe_context());

			m_p1_latch = m_io_p1->read();
			m_p2_latch = m_io_p2->read();
		}
	}
	m_old_portb = data;
}


void intec_interact_state::intech_interact(machine_config& config)
{
	vt_vt1682_ntscbase(config);
	vt_vt1682_common(config);

	m_io->porta_in().set(FUNC(intec_interact_state::porta_r));
	m_io->porta_out().set(FUNC(intec_interact_state::porta_w));

	m_io->portb_in().set(FUNC(intec_interact_state::portb_r));
	m_io->portb_out().set(FUNC(intec_interact_state::portb_w));

	m_io->portc_in().set(FUNC(intec_interact_state::portc_r));
	m_io->portc_out().set(FUNC(intec_interact_state::portc_w));

	m_io->portd_in().set(FUNC(intec_interact_state::portd_r));
	m_io->portd_out().set(FUNC(intec_interact_state::portd_w));

	m_leftdac->reset_routes();
	m_rightdac->reset_routes();

	config.device_remove(":lspeaker");
	config.device_remove(":rspeaker");

	SPEAKER(config, "mono").front_center();
	m_leftdac->add_route(0, "mono", 0.5);
	m_rightdac->add_route(0, "mono", 0.5);
}

uint8_t vt1682_lxts3_state::uio_porta_r()
{
	uint8_t ret = m_io_p1->read();
	logerror("%s: porta_r returning: %02x (INPUTS)\n", machine().describe_context(), ret);
	return ret;
}

uint8_t vt1682_dance_state::uio_porta_r()
{
	uint8_t ret = m_io_p1->read();
	logerror("%s: porta_r returning: %02x (INPUTS)\n", machine().describe_context(), ret);
	return ret;
}

void vt1682_dance_state::uio_porta_w(uint8_t data)
{
	logerror("%s: porta_w writing: %02x (INPUTS)\n", machine().describe_context(), data);
}

void intec_interact_state::intech_interact_bank(machine_config& config)
{
	intech_interact(config);

	m_uio->porta_out().set(FUNC(intec_interact_state::ext_rombank_w));
}

void vt1682_exsport_state::vt1682_exsport(machine_config& config)
{
	vt_vt1682_ntscbase(config);
	vt_vt1682_common(config);

	m_uio->portb_in().set(FUNC(vt1682_exsport_state::uiob_r));
	m_uio->portb_out().set(FUNC(vt1682_exsport_state::uiob_w));
}

void vt1682_exsport_state::vt1682_exsportp(machine_config& config)
{
	vt_vt1682_palbase(config);
	vt_vt1682_common(config);

	m_uio->portb_in().set(FUNC(vt1682_exsport_state::uiob_r));
	m_uio->portb_out().set(FUNC(vt1682_exsport_state::uiob_w));
}

void vt1682_dance_state::vt1682_dance(machine_config& config)
{
	vt_vt1682_palbase(config);
	vt_vt1682_common(config);

	M6502(config.replace(), m_maincpu, MAIN_CPU_CLOCK_PAL); // no opcode bitswap
	m_maincpu->set_addrmap(AS_PROGRAM, &vt1682_dance_state::vt_vt1682_map);

	m_leftdac->reset_routes();
	m_rightdac->reset_routes();

	config.device_remove(":lspeaker");
	config.device_remove(":rspeaker");

	SPEAKER(config, "mono").front_center();
	m_leftdac->add_route(0, "mono", 0.5);
	m_rightdac->add_route(0, "mono", 0.5);

	m_uio->porta_in().set(FUNC(vt1682_dance_state::uio_porta_r));
	m_uio->porta_out().set(FUNC(vt1682_dance_state::uio_porta_w));
}

void vt1682_lxts3_state::vt1682_lxts3(machine_config& config)
{
	vt_vt1682_ntscbase(config);
	vt_vt1682_common(config);

	M6502(config.replace(), m_maincpu, MAIN_CPU_CLOCK_NTSC); // no opcode bitswap
	m_maincpu->set_addrmap(AS_PROGRAM, &vt1682_lxts3_state::vt_vt1682_map);

	m_leftdac->reset_routes();
	m_rightdac->reset_routes();

	config.device_remove(":lspeaker");
	config.device_remove(":rspeaker");

	SPEAKER(config, "mono").front_center();
	m_leftdac->add_route(0, "mono", 0.5);
	m_rightdac->add_route(0, "mono", 0.5);

	m_uio->porta_in().set(FUNC(vt1682_lxts3_state::uio_porta_r));
}

void vt1682_lxts3_state::vt1682_unk1682(machine_config& config)
{
	vt_vt1682_palbase(config);
	vt_vt1682_common(config);

	M6502(config.replace(), m_maincpu, MAIN_CPU_CLOCK_PAL); // no opcode bitswap
	m_maincpu->set_addrmap(AS_PROGRAM, &vt1682_lxts3_state::vt_vt1682_map);

	m_leftdac->reset_routes();
	m_rightdac->reset_routes();

	config.device_remove(":lspeaker");
	config.device_remove(":rspeaker");

	SPEAKER(config, "mono").front_center();
	m_leftdac->add_route(0, "mono", 0.5);
	m_rightdac->add_route(0, "mono", 0.5);

	m_uio->porta_in().set(FUNC(vt1682_lxts3_state::uio_porta_r));
}


void vt1682_wow_state::vt1682_wow(machine_config& config)
{
	vt_vt1682_palbase(config);
	vt_vt1682_common(config);

	m_uio->portb_in().set(FUNC(vt1682_exsport_state::uiob_r));
	m_uio->portb_out().set(FUNC(vt1682_exsport_state::uiob_w));

	M6502_SWAP_OP_D5_D6(config.replace(), m_maincpu, MAIN_CPU_CLOCK_NTSC); // doesn't use the same bitswap as the other VT1682 games...
	m_maincpu->set_addrmap(AS_PROGRAM, &vt1682_wow_state::vt_vt1682_map);
}


void vt_vt1682_state::regular_init()
{
	m_bank->configure_entry(0, memregion("mainrom")->base() + 0x0000000);
}



void intec_interact_state::banked_init()
{
	int size = memregion("mainrom")->bytes();
	for (int i = 0; i < 4; i++)
	{
		m_bank->configure_entry(i, memregion("mainrom")->base() + ((i*0x2000000) & (size-1)));
	}
}


void vt1682_lxts3_state::unk1682_init()
{
	regular_init();

	uint8_t* ROM = memregion("mainrom")->base();
	// this jumps to a function on startup that has a bunch of jumps / accesses to the 3xxx region, which is internal ROM
	// but bypassing it allows the unit to boot.
	ROM[0x7ef43] = 0xea;
	ROM[0x7ef44] = 0xea;
	ROM[0x7ef45] = 0xea;
}

void vt1682_lxts3_state::njp60in1_init()
{
	regular_init();

	uint8_t* ROM = memregion("mainrom")->base();
	// first jsr in the code is for some port based security(?) check, might be SEEPROM
	ROM[0x7ff44] = 0xea;
	ROM[0x7ff45] = 0xea;
	ROM[0x7ff46] = 0xea;
}

void vt1682_lxts3_state::pgs268_init()
{
	regular_init();

	uint8_t* ROM = memregion("mainrom")->base();
	// patch out the first JSR again
	ROM[0x7ff65] = 0xea;
	ROM[0x7ff65] = 0xea;
	ROM[0x7ff66] = 0xea;
}


ROM_START( ii8in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "ii8in1.bin", 0x00000, 0x2000000, CRC(7aee7464) SHA1(7a9cf7f54a350f0853a17459f2dcbef34f4f7c30) ) // 2ND HALF EMPTY
ROM_END

ROM_START( ii32in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "ii32in1.bin", 0x00000, 0x2000000, CRC(ddee4eac) SHA1(828c0c18a66bb4872299f9a43d5e3647482c5925) )
ROM_END

ROM_START( zone7in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "zone.bin", 0x000000, 0x1000000, CRC(50726ae8) SHA1(bcedcd61728dce7b430784585be14109af542cc2) )
ROM_END

ROM_START( zone7in1p )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "zone7in1.bin", 0x000000, 0x1000000, CRC(40bbfb80) SHA1(f65a900abea13977713bbe3b5e736e6d4d106f2c) )
ROM_END

ROM_START( dance555 )
	ROM_REGION( 0x2000000, "mainrom", ROMREGION_ERASE00 )
	ROM_LOAD( "39vf6401.u3", 0x000000, 0x800000, CRC(13b1ccef) SHA1(3eb494816a1781a5e6a45bd0562b2b8326598ef7) )
ROM_END

ROM_START( miwi2_16 )
	ROM_REGION( 0x2000000, "mainrom", ROMREGION_ERASE00 )
	ROM_LOAD( "miwi 2 16 arcade games and drum master vt168.bin", 0x00000, 0x1000000, CRC(00c115c5) SHA1(fa5fdb448dd9b963351d71fe94e2072f5c872a18) )
ROM_END

ROM_START( miwi2_7 )
	ROM_REGION( 0x2000000, "mainrom", ROMREGION_ERASE00 )
	ROM_LOAD( "miwi 2 sports 7 in 1 vt168.bin", 0x00000, 0x1000000, CRC(fcefb956) SHA1(fea8f041d42bcbae3716ce8b942a01e64504061e) )
ROM_END

ROM_START( intact89 )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "89n1.bin", 0x00000, 0x4000000, CRC(bbcba068) SHA1(0ec1ecc55e9a7050ca20b1349b9712319fd21629) )
ROM_END

ROM_START( intg5410 )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "interact_intg5410_111games_plus_42songs.bin", 0x00000, 0x8000000, CRC(d32dc914) SHA1(269fa262bb036ad5246dee9f83ee33dbb1543210) )
ROM_END

ROM_START( exsprt48 )
	ROM_REGION( 0x2000000, "mainrom", ROMREGION_ERASE00 )
	ROM_LOAD( "excitesportgames_48.bin", 0x00000, 0x2000000, CRC(1bf239a0) SHA1(d69c16bac5fb15c62abb5a0c0920405647205539) ) // original dump had upper 2 address lines swapped, unmarked chip, so lines were guessed when dumping
ROM_END

// differs by 2 bytes from above, the rasters glitch in MotorStorm in a different way, so it's likely an NTSC/PAL difference?
ROM_START( itvg48 )
	ROM_REGION( 0x2000000, "mainrom", ROMREGION_ERASE00 )
	ROM_LOAD( "48in1sports.bin", 0x00000, 0x2000000, CRC(8e490541) SHA1(aeb01b3d7229fc888b36aaa924fe6b10597a7783) )
ROM_END

ROM_START( xing48 )
	ROM_REGION( 0x2000000, "mainrom", ROMREGION_ERASE00 )
	ROM_LOAD( "xing48in1.bin", 0x00000, 0x0800000, CRC(c601a4ae) SHA1(ec1219ede01a48df6bfd01675e715f6b13d2b43e) )
	ROM_CONTINUE(0x1000000, 0x0800000)
	ROM_CONTINUE(0x0800000, 0x0800000)
	ROM_CONTINUE(0x1800000, 0x0800000)
ROM_END


ROM_START( wowwg )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "msp55lv128.bin", 0x00000, 0x1000000, CRC(f607c40c) SHA1(66d3960c3b8fbab06a88cf039419c79a6c8633f0) )
	ROM_RELOAD(0x1000000,0x1000000)
ROM_END

ROM_START( unk1682 )
	ROM_REGION( 0x1000, "internal", 0 )
	// this appears to use the internal ROM on startup, so mark it as missing
	ROM_LOAD( "101in1.internal.rom", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "vt1682_101in1.bin", 0x00000, 0x2000000, CRC(82879200) SHA1(c1977d1733f8849326286102c0755629d0406ec4) )

	// also has a 24c02n SEEPROM, no accesses noted (maybe accessed from 'internal ROM' code?)
	// note, this could be mismatched, it came from a VT1682-896 20120410 PCB with ROM already removed
	ROM_REGION( 0x100, "seeprom", 0 )
	ROM_LOAD( "24c02.u2", 0x00000, 0x100, CRC(ee89332c) SHA1(aaa90b6bb47a60e44a98795c4e1ee0c64408ec92) )
ROM_END


ROM_START( njp60in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "60-in-1.bin", 0x00000, 0x2000000, CRC(7b2ee951) SHA1(fc7c214704908b85676efc64a21930483d24a457) )

	// also has a 24c02n SEEPROM, seems to access it on startup (security check?)
ROM_END

ROM_START( pgs268 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "4000-256a.u7", 0x00000, 0x2000000, CRC(4af648a8) SHA1(c60677ac0a31814bad4aeb80b2605dc7e767a3f6) )

	// 24c02 SEEPROM, seems to access it on startup (security check?)
	ROM_REGION( 0x100, "seeprom", 0 )
	ROM_LOAD( "24c02.bin", 0x00000, 0x100, CRC(db0e3f75) SHA1(23328dcff6f46f1ec0297752a654d43e2650b2e4) )
ROM_END


ROM_START( 110dance )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "110songdancemat.bin", 0x00000, 0x2000000, CRC(cd668e41) SHA1(975bfe05f4cce047860b05766bc8539218f6014f) )
ROM_END

ROM_START( lxts3 )
	ROM_REGION( 0x2000000, "mainrom", ROMREGION_ERASE00 )
	ROM_LOAD( "lexibooktoystory_mx29lv640mt_00c2227e.bin", 0x00000, 0x800000, CRC(91344ae7) SHA1(597fc4a27dd1fb6e6f5fda1c4ea237c07e9dba71))
ROM_END


ROM_START( gm235upc )
	ROM_REGION( 0x2000000, "mainrom", ROMREGION_ERASE00 )
	ROM_LOAD( "39vf3201.u3", 0x00000, 0x400000, CRC(182f8a2c) SHA1(7be56e1063cc8dbb78c419f5adc05b8cd65c8e2f))
	// also has RAM
ROM_END

} // anonymous namespace


// TODO: this is a cartridge based system (actually, verify this, it seems some versions simply had built in games) move these to SL if verified as from cartridge config
//  actually it appears that for the cart based systems these are 'fake systems' anyway, where the base unit is just a Famiclone but as soon as you plug in a cart none of
//  the internal hardware gets used at all.

CONS( 200?, ii8in1,    0,  0,  intech_interact,    intec, intec_interact_state, regular_init,  "Intec", "InterAct 8-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 200?, ii32in1,   0,  0,  intech_interact,    intec, intec_interact_state, regular_init,  "Intec", "InterAct 32-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
// a 40-in-1 also exists which combines the above

CONS( 200?, zone7in1,  0,         0,  intech_interact,    miwi2, intec_interact_state, regular_init,  "Ultimate Products Ltd.", "Zone 7-in-1 Sports (NTSC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, zone7in1p, zone7in1,  0,  intech_interact,    miwi2, intec_interact_state, regular_init,  "Ultimate Products Ltd.", "Zone 7-in-1 Sports (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // has Fishing instead of Baseball, and Ultimate Products banners in the Football game



CONS( 200?, miwi2_16,  0,  0,  intech_interact,    miwi2, intec_interact_state, regular_init,  "Macro Winners", "MiWi2 16-in-1 + Drum Master", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // clearly older code, Highway has uncensored title screen, selection screen has 'Arcase' instead of 'Arcade'
CONS( 200?, miwi2_7,   0,  0,  intech_interact,    miwi2, intec_interact_state, regular_init,  "Macro Winners", "MiWi2 7-in-1 Sports", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
// ViMax seems to be identical software to MiWi2

CONS( 200?, intact89,  0,  0,  intech_interact_bank, miwi2, intec_interact_state, banked_init,  "Intec", "InterAct Complete Video Game - 89-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

/*
Box shows

InterAct
Complete Video Game System
Sistema Completo De Video Juegos
111 Games & 42 Songs

96 Arcade Games:
8 of them are Sports Games,
& 3 of the are Drum Master Games.
Plus 15 Shooting Games

Unit has 'InfraZone' text on it, but this isn't used anywhere in product description.

*/
CONS( 200?, intg5410,  0,  0,  intech_interact_bank, miwi2, intec_interact_state, banked_init,  "Intec", "InterAct Complete Video Game - 111 Games & 42 Songs (G5410)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // need to hook up gun controls etc. and verify others, also sometimes crashes on game change (due to crashing sound CPU?)

// Other standalone Mi Kara units should fit here as well


// the timing code for MotorStorm differs between these sets (although fails wiht our emulation in both cases, even if the game runs fine in other collections)
CONS( 200?, exsprt48,   0,         0,  vt1682_exsport,    exsprt48, vt1682_exsport_state, regular_init,  "Excite", "Excite Sports Wireless Interactive TV Game - 48-in-1 (NTSC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // "32 Arcade, 8 Sports, 8 Stadium"
CONS( 200?, itvg48,     exsprt48,  0,  vt1682_exsportp,   exsprt48, vt1682_exsport_state, regular_init,  "TaiKee", "Interactive TV Games 48-in-1 (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // ^

// This has a different selection of games to the above, Dancing as extra under Music, Doesn't have Poker under Brain, Ball Shoot instead of 'Noshery' under Arcade
// imported by Cathay Product Sourcing Ltd. (Ireland) no other manufacturer information on box, not sure if Xing is name of manufacturer or product
CONS( 200?, xing48,     0,         0,  vt1682_exsportp,   exsprt48, vt1682_exsport_state, regular_init,  "Xing", "Xing Wireless Interactive TV Game 'Wi TV Zone' 48-in-1 (Europe, PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // ^
/*
The above was also released in the US as Excite Sports Wireless Interactive TV Game - 48-in-1 with an almost identical box to exsprt48 unit, but with the different games noted.

It is still advertised as 48-in-1, 8 Interactive Sports Games, 8 Olympic games, 32 Arcade Games
see https://www.youtube.com/watch?v=tHMX71daHAk

This might be a regional / store thing if some places didn't want to sell a unit with a Poker game in it?
*/

// Timings are broken in the Bomberman game ('Explosion') even on real hardware (raster effect to keep status bar in place doesn't work) because the game is still coded to use NTSC timings even if this is a PAL unit.  This was fixed in other PAL releases (eg. 110dance)
// 'Riding Horse' on the other hand actually needs PAL timings, so this unit clearly was designed for PAL regions, however 'Explosion' was left broken.
CONS( 200?, wowwg,  0,  0,  vt1682_wow, exsprt48, vt1682_wow_state, regular_init, "Wow", "Wow Wireless Gaming (PAL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND) // needs high colour line mode for main menu


CONS( 200?, 110dance,  0,  0,  vt1682_dance, 110dance, vt1682_dance_state, regular_init, "<unknown>", "Retro Dance Mat (110 song Super StepMania + 9-in-1 games) (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)

// songs 5-8 are repeats of songs 1-4, but probably not a bug?
CONS( 200?, dance555,  0,  0,  vt1682_exsportp,   dance555, vt1682_exsport_state, regular_init,  "Subor", "Sports and Dance Fit Games Mat D-555 (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )


// NJ Pocket 60-in-1 (NJ-250) is meant to have similar games to the mini-games found in wowwg and 110dance, so almost certainly fits here

// manual explicitly states it has NTSC output only (unit can be connected to a TV) and both Ranning Horse + Explosion (Bomberman) are the NTSC versions
// has 21.477 Mhz XTAL
CONS( 200?, njp60in1,  0,  0,   vt1682_lxts3, njp60in1, vt1682_lxts3_state, njp60in1_init, "<unknown>", "NJ Pocket 60-in-1 handheld 'X zero' (NTSC)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND) // linescroll issues

// fewer than 268 games, there are repeats
CONS( 200?, pgs268,  0,  0,   vt1682_lxts3, njp60in1, vt1682_lxts3_state, pgs268_init, "<unknown>", "Portable Game Station 268-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND) // linescroll issues

// this appears to be related to the NJ Pocket, claims 101-in-1 but has some duplicates.
// Like the 'Wow Wireless gaming' it incorrectly mixes the PAL version of 'Ranning Horse' with the NTSC version of 'Bomberman', it has no TV output.
// has 26.6017 Mhz (6xPAL) XTAL
CONS( 200?, unk1682,  0,  0,   vt1682_unk1682, lxts3, vt1682_lxts3_state, unk1682_init, "<unknown>", "unknown VT1682-based 101-in-1 handheld (PAL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND) // linescroll issues

CONS( 2010, lxts3,    0,  0,   vt1682_lxts3, lxts3, vt1682_lxts3_state, regular_init,  "Lexibook", "Toy Story 3 (Lexibook)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // linescroll issues

// there are products on SunPlus type hardware with nearly identical shells 'Mi DiGi World' / 'Mi Digi Diary'
// needs IO ports on sound CPU side, needs write access to space for RAM (inputs are 'mini-keyboard' style)
CONS( 200?, gm235upc,  0,  0,  vt1682_dance, gm235upc, vt1682_dance_state, regular_init, "TimeTop", "Ultimate Pocket Console GM-235", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
