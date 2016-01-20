// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood
/**************** Machine stuff ******************/
//#define USE_HD64x180          /* Define if CPU support is available */
//#define TRUXTON2_STEREO       /* Uncomment to hear truxton2 music in stereo */

// We encode priority with colour in the tilemaps, so need a larger palette
#define T2PALETTE_LENGTH 0x10000

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/nmk112.h"
#include "machine/upd4992.h"
#include "video/gp9001.h"
#include "sound/okim6295.h"

class toaplan2_state : public driver_device
{
public:
	enum
	{
		TIMER_RAISE_IRQ
	};

	toaplan2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_shared_ram(*this, "shared_ram"),
		m_shared_ram16(*this, "shared_ram16"),
		m_paletteram(*this, "palette"),
		m_tx_videoram(*this, "tx_videoram"),
		m_tx_lineselect(*this, "tx_lineselect"),
		m_tx_linescroll(*this, "tx_linescroll"),
		m_tx_gfxram16(*this, "tx_gfxram16"),
		m_mainram16(*this, "mainram16"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_vdp0(*this, "gp9001"),
		m_vdp1(*this, "gp9001_1"),
		m_nmk112(*this, "nmk112"),
		m_oki(*this, "oki"),
		m_oki1(*this, "oki1"),
		m_eeprom(*this, "eeprom"),
		m_rtc(*this, "rtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	optional_shared_ptr<UINT8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	optional_shared_ptr<UINT16> m_shared_ram16;     // Really 8 bit RAM connected to Z180
	optional_shared_ptr<UINT16> m_paletteram;
	optional_shared_ptr<UINT16> m_tx_videoram;
	optional_shared_ptr<UINT16> m_tx_lineselect;
	optional_shared_ptr<UINT16> m_tx_linescroll;
	optional_shared_ptr<UINT16> m_tx_gfxram16;
	optional_shared_ptr<UINT16> m_mainram16;

	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gp9001vdp_device> m_vdp0;
	optional_device<gp9001vdp_device> m_vdp1;
	optional_device<nmk112_device> m_nmk112;
	optional_device<okim6295_device> m_oki;
	optional_device<okim6295_device> m_oki1;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<upd4992_device> m_rtc;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT16 m_mcu_data;
	INT8 m_old_p1_paddle_h; /* For Ghox */
	INT8 m_old_p2_paddle_h;
	UINT8 m_v25_reset_line; /* 0x20 for dogyuun/batsugun, 0x10 for vfive, 0x08 for fixeight */
	UINT8 m_sndirq_line;        /* IRQ4 for batrider, IRQ2 for bbakraid */
	UINT8 m_z80_busreq;

	bitmap_ind8 m_custom_priority_bitmap;
	bitmap_ind16 m_secondary_render_bitmap;

	tilemap_t *m_tx_tilemap;    /* Tilemap for extra-text-layer */
	DECLARE_READ16_MEMBER(video_count_r);
	DECLARE_WRITE8_MEMBER(toaplan2_coin_w);
	DECLARE_WRITE16_MEMBER(toaplan2_coin_word_w);
	DECLARE_WRITE16_MEMBER(toaplan2_v25_coin_word_w);
	DECLARE_WRITE16_MEMBER(shippumd_coin_word_w);
	DECLARE_READ16_MEMBER(shared_ram_r);
	DECLARE_WRITE16_MEMBER(shared_ram_w);
	DECLARE_WRITE16_MEMBER(toaplan2_hd647180_cpu_w);
	DECLARE_READ16_MEMBER(ghox_p1_h_analog_r);
	DECLARE_READ16_MEMBER(ghox_p2_h_analog_r);
	DECLARE_READ16_MEMBER(ghox_mcu_r);
	DECLARE_WRITE16_MEMBER(ghox_mcu_w);
	DECLARE_READ16_MEMBER(ghox_shared_ram_r);
	DECLARE_WRITE16_MEMBER(ghox_shared_ram_w);
	DECLARE_WRITE16_MEMBER(fixeight_subcpu_ctrl_w);
	DECLARE_WRITE16_MEMBER(fixeightbl_oki_bankswitch_w);
	DECLARE_READ8_MEMBER(v25_dswa_r);
	DECLARE_READ8_MEMBER(v25_dswb_r);
	DECLARE_READ8_MEMBER(v25_jmpr_r);
	DECLARE_READ8_MEMBER(fixeight_region_r);
	DECLARE_WRITE8_MEMBER(raizing_z80_bankswitch_w);
	DECLARE_WRITE8_MEMBER(raizing_oki_bankswitch_w);
	DECLARE_WRITE16_MEMBER(bgaregga_soundlatch_w);
	DECLARE_READ8_MEMBER(bgaregga_E01D_r);
	DECLARE_WRITE8_MEMBER(bgaregga_E00C_w);
	DECLARE_READ16_MEMBER(batrider_z80_busack_r);
	DECLARE_WRITE16_MEMBER(batrider_z80_busreq_w);
	DECLARE_READ16_MEMBER(batrider_z80rom_r);
	DECLARE_WRITE16_MEMBER(batrider_soundlatch_w);
	DECLARE_WRITE16_MEMBER(batrider_soundlatch2_w);
	DECLARE_WRITE16_MEMBER(batrider_unknown_sound_w);
	DECLARE_WRITE16_MEMBER(batrider_clear_sndirq_w);
	DECLARE_WRITE8_MEMBER(batrider_sndirq_w);
	DECLARE_WRITE8_MEMBER(batrider_clear_nmi_w);
	DECLARE_READ16_MEMBER(bbakraid_eeprom_r);
	DECLARE_WRITE16_MEMBER(bbakraid_eeprom_w);
	DECLARE_WRITE16_MEMBER(toaplan2_tx_videoram_w);
	DECLARE_WRITE16_MEMBER(toaplan2_tx_linescroll_w);
	DECLARE_WRITE16_MEMBER(toaplan2_tx_gfxram16_w);
	DECLARE_WRITE16_MEMBER(batrider_textdata_dma_w);
	DECLARE_WRITE16_MEMBER(batrider_unknown_dma_w);
	DECLARE_WRITE16_MEMBER(batrider_objectbank_w);
	DECLARE_CUSTOM_INPUT_MEMBER(c2map_r);
	DECLARE_WRITE16_MEMBER(oki_bankswitch_w);
	DECLARE_WRITE16_MEMBER(oki1_bankswitch_w);
	DECLARE_DRIVER_INIT(bbakraid);
	DECLARE_DRIVER_INIT(pipibibsbl);
	DECLARE_DRIVER_INIT(dogyuun);
	DECLARE_DRIVER_INIT(fixeight);
	DECLARE_DRIVER_INIT(bgaregga);
	DECLARE_DRIVER_INIT(fixeightbl);
	DECLARE_DRIVER_INIT(vfive);
	DECLARE_DRIVER_INIT(batrider);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	DECLARE_MACHINE_START(toaplan2);
	DECLARE_MACHINE_RESET(toaplan2);
	DECLARE_VIDEO_START(toaplan2);
	DECLARE_MACHINE_RESET(ghox);
	DECLARE_VIDEO_START(truxton2);
	DECLARE_VIDEO_START(fixeightbl);
	DECLARE_VIDEO_START(bgaregga);
	DECLARE_VIDEO_START(bgareggabl);
	DECLARE_VIDEO_START(batrider);
	UINT32 screen_update_toaplan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_dogyuun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_toaplan2(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(toaplan2_vblank_irq1);
	INTERRUPT_GEN_MEMBER(toaplan2_vblank_irq2);
	INTERRUPT_GEN_MEMBER(toaplan2_vblank_irq4);
	INTERRUPT_GEN_MEMBER(bbakraid_snd_interrupt);
	void truxton2_postload();
	void create_tx_tilemap(int dx = 0, int dx_flipped = 0);
	void toaplan2_vblank_irq(int irq_line);

	UINT8 m_pwrkick_hopper;
	DECLARE_CUSTOM_INPUT_MEMBER(pwrkick_hopper_status_r);
	DECLARE_WRITE8_MEMBER(pwrkick_coin_w);

	DECLARE_WRITE_LINE_MEMBER(toaplan2_reset);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
