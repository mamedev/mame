// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

/**************** Machine stuff ******************/
//#define TRUXTON2_STEREO       /* Uncomment to hear truxton2 music in stereo */

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/nmk112.h"
#include "machine/ticket.h"
#include "machine/upd4992.h"
#include "video/gp9001.h"
#include "sound/okim6295.h"
#include "screen.h"

// We encode priority with colour in the tilemaps, so need a larger palette
#define T2PALETTE_LENGTH 0x10000

class toaplan2_state : public driver_device
{
public:
	toaplan2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_shared_ram(*this, "shared_ram"),
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
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_hopper(*this, "hopper") { }

	optional_shared_ptr<uint8_t> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	optional_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint16_t> m_tx_videoram;
	optional_shared_ptr<uint16_t> m_tx_lineselect;
	optional_shared_ptr<uint16_t> m_tx_linescroll;
	optional_shared_ptr<uint16_t> m_tx_gfxram16;
	optional_shared_ptr<uint16_t> m_mainram16;

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
	optional_device<generic_latch_8_device> m_soundlatch; // tekipaki, batrider, bgaregga, batsugun
	optional_device<generic_latch_8_device> m_soundlatch2;
	optional_device<ticket_dispenser_device> m_hopper;

	int8_t m_old_p1_paddle_h; /* For Ghox */
	int8_t m_old_p2_paddle_h;
	uint8_t m_v25_reset_line; /* 0x20 for dogyuun/batsugun, 0x10 for vfive, 0x08 for fixeight */
	uint8_t m_sndirq_line;        /* IRQ4 for batrider, IRQ2 for bbakraid */
	uint8_t m_z80_busreq;

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
	DECLARE_READ16_MEMBER(ghox_p1_h_analog_r);
	DECLARE_READ16_MEMBER(ghox_p2_h_analog_r);
	DECLARE_WRITE16_MEMBER(fixeight_subcpu_ctrl_w);
	DECLARE_WRITE16_MEMBER(fixeightbl_oki_bankswitch_w);
	DECLARE_WRITE8_MEMBER(raizing_z80_bankswitch_w);
	DECLARE_WRITE8_MEMBER(raizing_oki_bankswitch_w);
	DECLARE_READ8_MEMBER(bgaregga_E01D_r);
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
	DECLARE_WRITE16_MEMBER(enmadaio_oki_bank_w);
	DECLARE_DRIVER_INIT(bbakraid);
	DECLARE_DRIVER_INIT(pipibibsbl);
	DECLARE_DRIVER_INIT(dogyuun);
	DECLARE_DRIVER_INIT(fixeight);
	DECLARE_DRIVER_INIT(bgaregga);
	DECLARE_DRIVER_INIT(fixeightbl);
	DECLARE_DRIVER_INIT(vfive);
	DECLARE_DRIVER_INIT(batrider);
	DECLARE_DRIVER_INIT(enmadaio);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(toaplan2);
	DECLARE_VIDEO_START(toaplan2);
	DECLARE_MACHINE_RESET(ghox);
	DECLARE_VIDEO_START(truxton2);
	DECLARE_VIDEO_START(fixeightbl);
	DECLARE_VIDEO_START(bgaregga);
	DECLARE_VIDEO_START(bgareggabl);
	DECLARE_VIDEO_START(batrider);

	// Teki Paki sound
	DECLARE_READ8_MEMBER(tekipaki_cmdavailable_r);

	uint32_t screen_update_toaplan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dogyuun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_toaplan2);
	IRQ_CALLBACK_MEMBER(fixeightbl_irq_ack);
	IRQ_CALLBACK_MEMBER(pipibibsbl_irq_ack);
	INTERRUPT_GEN_MEMBER(bbakraid_snd_interrupt);
	void truxton2_postload();
	void create_tx_tilemap(int dx = 0, int dx_flipped = 0);

	DECLARE_WRITE8_MEMBER(pwrkick_coin_w);
	DECLARE_WRITE8_MEMBER(pwrkick_coin_lockout_w);

	DECLARE_WRITE_LINE_MEMBER(toaplan2_reset);
	void dogyuun(machine_config &config);
	void othldrby(machine_config &config);
	void snowbro2(machine_config &config);
	void bgareggabl(machine_config &config);
	void pwrkick(machine_config &config);
	void mahoudai(machine_config &config);
	void tekipaki(machine_config &config);
	void bbakraid(machine_config &config);
	void fixeightbl(machine_config &config);
	void fixeight(machine_config &config);
	void ghox(machine_config &config);
	void bgaregga(machine_config &config);
	void batrider(machine_config &config);
	void shippumd(machine_config &config);
	void kbash(machine_config &config);
	void pipibibs(machine_config &config);
	void pipibibsbl(machine_config &config);
	void batsugun(machine_config &config);
	void enmadaio(machine_config &config);
	void truxton2(machine_config &config);
	void vfive(machine_config &config);
	void kbash2(machine_config &config);
	void batrider_68k_mem(address_map &map);
	void batrider_sound_z80_mem(address_map &map);
	void batrider_sound_z80_port(address_map &map);
	void batsugun_68k_mem(address_map &map);
	void bbakraid_68k_mem(address_map &map);
	void bbakraid_sound_z80_mem(address_map &map);
	void bbakraid_sound_z80_port(address_map &map);
	void bgaregga_68k_mem(address_map &map);
	void bgaregga_sound_z80_mem(address_map &map);
	void dogyuun_68k_mem(address_map &map);
	void enmadaio_68k_mem(address_map &map);
	void enmadaio_oki(address_map &map);
	void fixeight_68k_mem(address_map &map);
	void fixeight_v25_mem(address_map &map);
	void fixeightbl_68k_mem(address_map &map);
	void fixeightbl_oki(address_map &map);
	void ghox_68k_mem(address_map &map);
	void ghox_hd647180_mem_map(address_map &map);
	void hd647180_io_map(address_map &map);
	void hd647180_mem_map(address_map &map);
	void kbash2_68k_mem(address_map &map);
	void kbash_68k_mem(address_map &map);
	void kbash_v25_mem(address_map &map);
	void mahoudai_68k_mem(address_map &map);
	void othldrby_68k_mem(address_map &map);
	void pipibibi_bootleg_68k_mem(address_map &map);
	void pipibibs_68k_mem(address_map &map);
	void pipibibs_sound_z80_mem(address_map &map);
	void pwrkick_68k_mem(address_map &map);
	void raizing_sound_z80_mem(address_map &map);
	void shippumd_68k_mem(address_map &map);
	void snowbro2_68k_mem(address_map &map);
	void tekipaki_68k_mem(address_map &map);
	void truxton2_68k_mem(address_map &map);
	void v25_mem(address_map &map);
	void vfive_68k_mem(address_map &map);
	void vfive_v25_mem(address_map &map);
};
