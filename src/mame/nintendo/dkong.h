// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

  Nintendo Donkey Kong hardware

***************************************************************************/
#ifndef MAME_NINTENDO_DKONG_H
#define MAME_NINTENDO_DKONG_H

#pragma once

#include "cpu/m6502/rp2a03.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/eepromser.h"
#include "machine/i8257.h"
#include "machine/latch8.h"
#include "machine/tms6100.h"
#include "machine/watchdog.h"
#include "machine/z80dma.h"
#include "sound/discrete.h"
#include "sound/tms5110.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


/*
 * From the schematics:
 *
 * XTAL is 61,44 MHZ. There is some oscillator logic around it. The oscillating circuit
 * transfers the signal with a transformator. Onwards, it is fed through a M(B/C)10136. This
 * is a programmable counter which is used as a divisor by 5.
 * Cascaded 74LS161 further divide the signal. The following signals are generated:
 * 1/2H: 61,44MHZ/5/2 - pixel clock
 * 1H  : 61,44MHZ/5/4 - cpu-clock
 * 2H  : 61,44MHZ/5/8
 * ....
 * 128H: 61,44MHZ/5/512
 * The horizontal circuit counts till 384=256+128, thus 256H only being high for 128H/2
 *
 * Signal 16H,32H,64H and 256H are combined using a LS00, LS04 and a D-Flipflop to produce
 * a signal with Freq 16H/12. This is only possible because a 220pf capacitor with the
 * impedance of the LS-Family of 10K delays the 16H signal by about half a cycle.
 * This signal is divided by two by another D-Flipflop(74LS74) to give:
 * 1VF: 61,44MHZ/5/64/12/2 = 8KHZ
 * 2VF: 1VF/2 - Noise frequency: 4Khz
 * ...
 * The vertical circuit counts from 248 till 512 giving 264 lines.
 * 256VF is not being used, so counting is from 248...255, 0...255, ....
 */

#define MASTER_CLOCK            XTAL(61'440'000)
#define CLOCK_1H                (MASTER_CLOCK / 5 / 4)
#define CLOCK_16H               (CLOCK_1H / 16)
#define CLOCK_1VF               ((CLOCK_16H) / 12 / 2)
#define CLOCK_2VF               ((CLOCK_1VF) / 2)

#define PIXEL_CLOCK             (MASTER_CLOCK/10)
#define HTOTAL                  (384)
#define HBSTART                 (256)
#define HBEND                   (0)
#define VTOTAL                  (264)
#define VBSTART                 (240)
#define VBEND                   (16)

#define I8035_CLOCK             (XTAL(6'000'000))

/****************************************************************************
 * CONSTANTS
 ****************************************************************************/

enum
{
	HARDWARE_TKG04 = 0,
	HARDWARE_TRS01,
	HARDWARE_TRS02,
	HARDWARE_TKG02
};

enum
{
	DKONG_RADARSCP_CONVERSION = 0,
	DKONG_BOARD = 1
};

enum
{
	DK2650_HERBIEDK = 0,
	DK2650_HUNCHBKD,
	DK2650_EIGHTACT,
	DK2650_SHOOTGAL,
	DK2650_SPCLFORC
};

#define DK2B_PALETTE_LENGTH     (256+256+8+1) /*  (256) */
#define DK4B_PALETTE_LENGTH     (256+256+8+1) /*  (256) */
#define DK3_PALETTE_LENGTH      (256+256+8+1) /*  (256) */
#define RS_PALETTE_LENGTH       (256+256+8+1)

class dkong_state : public driver_device
{
public:
	dkong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_eeprom(*this, "eeprom")
		, m_dev_rp2a03a(*this, "rp2a03a")
		, m_dev_rp2a03b(*this, "rp2a03b")
		, m_dev_vp2(*this, "virtual_p2")
		, m_dev_6h(*this, "ls259.6h")
		, m_ls175_3d(*this, "ls175.3d")
		, m_discrete(*this, "discrete")
		, m_m58817(*this, "tms")
		, m_watchdog(*this, "watchdog")
		, m_video_ram(*this,"video_ram")
		, m_sprite_ram(*this,"sprite_ram")
		, m_snd_rom(*this, "soundcpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_z80dma(*this, "z80dma")
		, m_dma8257(*this, "dma8257")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
	{
	}

	void dkong_base(machine_config &config);
	void dk_braze(machine_config &config);
	void dkj_braze(machine_config &config);
	void ddk_braze(machine_config &config);
	void dk3_braze(machine_config &config);
	void strtheat(machine_config &config);
	void s2650(machine_config &config);
	void spclforc(machine_config &config);
	void dkongjr(machine_config &config);
	void radarscp1(machine_config &config);
	void drktnjr(machine_config &config);
	void dkong2b(machine_config &config);
	void drakton(machine_config &config);
	void radarscp(machine_config &config);
	void pestplce(machine_config &config);
	void herbiedk(machine_config &config);
	void dkong3(machine_config &config);
	void dkong3b(machine_config &config);
	void radarscp_audio(machine_config &config);
	void dkong2b_audio(machine_config &config);
	void dkongjr_audio(machine_config &config);
	void dkong3_audio(machine_config &config);
	void radarscp1_audio(machine_config &config);

	void init_strtheat();
	void init_herodk();
	void init_dkingjr();
	void init_drakton();
	void init_dkonghs();
	void init_dkongx();
	void init_dkong3();
	void init_dkong3hs();

	void dk_braze_a15(int state);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<mcs48_cpu_device> m_soundcpu;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<rp2a03_device> m_dev_rp2a03a; /* dkong3 */
	optional_device<rp2a03_device> m_dev_rp2a03b; /* dkong3 */
	optional_device<latch8_device> m_dev_vp2;   /* dkong2, virtual port 2 */
	optional_device<latch8_device> m_dev_6h;    /* dkong2 */
	optional_device<latch8_device> m_ls175_3d;  /* dkong2b_audio */
	optional_device<discrete_device> m_discrete;
	optional_device<m58817_device> m_m58817;    /* radarscp1 */
	optional_device<watchdog_timer_device> m_watchdog;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;

	/* machine states */
	uint8_t           m_hardware_type = 0;
	uint8_t           m_nmi_mask = 0U;

	std::unique_ptr<uint8_t[]> m_decrypted;

	/* sound state */
	optional_region_ptr<uint8_t> m_snd_rom;

	/* video state */
	tilemap_t         *m_bg_tilemap = nullptr;

	bitmap_ind16      m_bg_bits;
	const uint8_t     *m_color_codes = nullptr;
	emu_timer         *m_scanline_timer = nullptr;
	int8_t            m_vidhw = DKONG_BOARD; // Selected video hardware RS Conversion / TKG04

	/* radar scope */
	uint8_t           *m_gfx4 = nullptr;
	uint8_t           *m_gfx3 = nullptr;
	int               m_gfx3_len = 0;

	uint8_t           m_sig30Hz = 0;
	uint8_t           m_lfsr_5I = 0;
	uint8_t           m_grid_sig = 0;
	uint8_t           m_rflip_sig = 0;
	uint8_t           m_star_ff = 0;
	uint8_t           m_blue_level = 0;
	double            m_cd4049_a = 0.0;
	double            m_cd4049_b = 0.0;

	/* Specific states */
	int8_t            m_decrypt_counter = 0;

	/* 2650 protection */
	uint8_t           m_protect_type = 0;
	uint8_t           m_hunchloopback = 0;
	uint8_t           m_prot_cnt = 0;
	uint8_t           m_main_fo = 0;

	/* Save state relevant */
	uint8_t           m_gfx_bank = 0;
	uint8_t           m_palette_bank = 0;
	uint8_t           m_grid_on = 0;
	uint16_t          m_grid_col = 0;
	uint8_t           m_sprite_bank = 0;
	uint8_t           m_dma_latch = 0;
	uint8_t           m_flip = 0;

	/* radarscp_step */
	double            m_cv1 = 0.0;
	double            m_cv2 = 0.0;
	double            m_vg1 = 0.0;
	double            m_vg2 = 0.0;
	double            m_vg3 = 0.0;
	double            m_cv3 = 0.0;
	double            m_cv4 = 0.0;
	double            m_vc17 = 0.0;
	int               m_pixelcnt = 0;

	/* radarscp_scanline */
	int               m_counter = 0;

	/* reverse address lookup map - hunchbkd */
	int16_t           m_rev_map[0x200] = { };

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<z80dma_device> m_z80dma;
	optional_device<i8257_device> m_dma8257;
	memory_bank_creator m_bank1;
	memory_bank_creator m_bank2;
	memory_passthrough_handler m_dkong3_tap[2];

	uint8_t hb_dma_read_byte(offs_t offset);
	void hb_dma_write_byte(offs_t offset, uint8_t data);
	void dkong3_coin_counter_w(offs_t offset, uint8_t data);
	uint8_t dkong_in2_r(offs_t offset);
	uint8_t epos_decrypt_rom(offs_t offset);
	void s2650_data_w(uint8_t data);
	void s2650_fo_w(int state);
	uint8_t s2650_port0_r();
	uint8_t s2650_port1_r();
	void dkong3_2a03_reset_w(uint8_t data);
	uint8_t strtheat_inputport_0_r();
	uint8_t strtheat_inputport_1_r();
	void nmi_mask_w(uint8_t data);
	void dk_braze_a15_w(uint8_t data);
	void dkong_videoram_w(offs_t offset, uint8_t data);
	void dkongjr_gfxbank_w(uint8_t data);
	void dkong3_gfxbank_w(uint8_t data);
	void dkong_palettebank_w(offs_t offset, uint8_t data);
	void radarscp_grid_enable_w(uint8_t data);
	void radarscp_grid_color_w(uint8_t data);
	void dkong_flipscreen_w(uint8_t data);
	void dkong_spritebank_w(uint8_t data);
	void dkong_voice_w(uint8_t data);
	void dkong_audio_irq_w(uint8_t data);
	uint8_t p8257_ctl_r();
	void p8257_ctl_w(uint8_t data);
	void p8257_drq_w(uint8_t data);
	void dkong_z80dma_rdy_w(uint8_t data);
	uint8_t braze_eeprom_r();
	void braze_eeprom_w(uint8_t data);
	TILE_GET_INFO_MEMBER(dkong_bg_tile_info);
	TILE_GET_INFO_MEMBER(radarscp1_bg_tile_info);
	DECLARE_MACHINE_START(dkong2b);
	DECLARE_MACHINE_RESET(dkong);
	DECLARE_MACHINE_RESET(ddk);
	DECLARE_VIDEO_START(dkong);
	DECLARE_VIDEO_START(dkong_base);
	void dkong2b_palette(palette_device &palette);
	DECLARE_MACHINE_START(dkong3);
	void dkong3_palette(palette_device &palette);
	[[maybe_unused]] void dkong4b_palette(palette_device &palette);
	DECLARE_MACHINE_START(radarscp);
	void radarscp_palette(palette_device &palette);
	DECLARE_MACHINE_START(radarscp1);
	void radarscp1_palette(palette_device &palette);
	DECLARE_MACHINE_START(s2650);
	DECLARE_MACHINE_RESET(strtheat);
	DECLARE_MACHINE_RESET(drakton);
	void m58817_command_w(uint8_t data);
	uint8_t dkong_voice_status_r();
	uint8_t dkong_tune_r(offs_t offset);
	void dkong_p1_w(uint8_t data);
	uint32_t screen_update_dkong(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pestplce(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spclforc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void s2650_interrupt(int state);
	void vblank_irq(int state);
	TIMER_CALLBACK_MEMBER(scanline_callback);

	void braze_decrypt_rom(uint8_t *dest);
	void dk_braze_decrypt();
	void drakton_decrypt_rom(uint8_t mod, int offs, int *bs);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	double CD4049(double x);

	void dkong3_io_map(address_map &map) ATTR_COLD;
	void dkong3_map(address_map &map) ATTR_COLD;
	void dkong3_sound1_map(address_map &map) ATTR_COLD;
	void dkong3_sound2_map(address_map &map) ATTR_COLD;
	void dkong_map(address_map &map) ATTR_COLD;
	void dkong_sound_io_map(address_map &map) ATTR_COLD;
	void dkong_sound_map(address_map &map) ATTR_COLD;
	void dkongjr_map(address_map &map) ATTR_COLD;
	void dkongjr_sound_io_map(address_map &map) ATTR_COLD;
	void epos_readport(address_map &map) ATTR_COLD;
	void radarscp1_sound_io_map(address_map &map) ATTR_COLD;

	void s2650_map(address_map &map) ATTR_COLD;
	void s2650_io_map(address_map &map) ATTR_COLD;
	void s2650_data_map(address_map &map) ATTR_COLD;
	void spclforc_data_map(address_map &map) ATTR_COLD;

	// video/dkong.c
	void radarscp_step(int line_cnt);
	void radarscp_scanline(int scanline);
	void check_palette(void);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t mask_bank, uint32_t shift_bits);
	void radarscp_draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#endif // MAME_NINTENDO_DKONG_H
