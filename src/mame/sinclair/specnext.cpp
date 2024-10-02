// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next

    Versions: TBBlue 1.2, Issue 0, Issue 1, Issue 2,
              Issue 2B (Kickstarter 1), Issue 2D, Issue 2E, Issue 2H,
              Issue 4 (Kickstarter 2)
    Current implementation is based on Issue 4. Only limited difference
    tracked through PORT_CONFIG

    TODO:
    * improve zxnDMA
    * contention
    * internal_port_enable() support
    * (1) invalidate tiles/sprites caches on region w, not every frame

**********************************************************************/

#include "emu.h"

#include "beta_m.h"
#include "screen_ula.h"
#include "spec128.h"
#include "specnext_copper.h"
#include "specnext_ctc.h"
#include "specnext_divmmc.h"
#include "specnext_multiface.h"
#include "specnext_layer2.h"
#include "specnext_lores.h"
#include "specnext_sprites.h"
#include "specnext_tiles.h"

#include "bus/spectrum/zxbus.h"
#include "cpu/z80/z80n.h"
#include "machine/i2cmem.h"
#include "machine/spi_sdcard.h"
#include "machine/z80dma.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "speaker.h"


#define LOG_IO    (1U << 1)
#define LOG_MEM   (1U << 2)
#define LOG_WARN  (1U << 3)

#define VERBOSE ( LOG_GENERAL | LOG_IO | /*LOG_MEM |*/ LOG_WARN )
#include "logmacro.h"

#define LOGIO(...)    LOGMASKED(LOG_IO,    __VA_ARGS__)
#define LOGMEM(...)   LOGMASKED(LOG_MEM,   __VA_ARGS__)
#define LOGWARN(...)  LOGMASKED(LOG_WARN,  __VA_ARGS__)


namespace {

#define TIMINGS_PERFECT     0

static const u16 CYCLES_HORIZ = (228 * 2) << 1;
static const u16 CYCLES_VERT = 311;
static const rectangle SCR_256x192 = { 48 << 1, (304 << 1) - 1, 40, 231 };
static const rectangle SCR_320x256 = {  16 << 1, (336 << 1) - 1,  8, 263 };

class specnext_state : public spectrum_128_state
{
public:
	specnext_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bank_boot_rom(*this, "bootrom")
		, m_bank_ram(*this, "bank_ram%u", 0U)
		, m_view0(*this, "mem_view0")
		, m_view1(*this, "mem_view1")
		, m_view2(*this, "mem_view2")
		, m_view3(*this, "mem_view3")
		, m_view4(*this, "mem_view4")
		, m_view5(*this, "mem_view5")
		, m_view6(*this, "mem_view6")
		, m_view7(*this, "mem_view7")
		, m_copper(*this, "copper")
		, m_ctc(*this, "ctc")
		, m_dma(*this, "dma")
		, m_i2cmem(*this, "i2cmem")
		, m_sdcard(*this, "sdcard")
		, m_ay(*this, "ay%u", 0U)
		, m_dac(*this, "dac%u", 0U)
		, m_palette(*this, "palette")
		, m_regs_map(*this, "regs_map")
		, m_mf(*this, "multiface")
		, m_divmmc(*this, "divmmc")
		, m_ula(*this, "ula")
		, m_tiles(*this, "tiles")
		, m_layer2(*this, "layer2")
		, m_lores(*this, "lores")
		, m_sprites(*this, "sprites")
		, m_io_issue(*this, "ISSUE")
		, m_io_mouse(*this, "mouse_input%u", 1U)
	{}

	void tbblue(machine_config &config);

	INPUT_CHANGED_MEMBER(on_mf_nmi);
	INPUT_CHANGED_MEMBER(on_divmmc_nmi);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void reset_hard();
	virtual void video_start() override ATTR_COLD;
	virtual void spectrum_128_update_memory() override {}

	u8 do_m1(offs_t offset);
	void do_mf_nmi();
	void leave_nmi(int status);
	void map_fetch(address_map &map) ATTR_COLD;
	void map_mem(address_map &map) ATTR_COLD;
	void map_io(address_map &map) ATTR_COLD;
	void map_regs(address_map &map) ATTR_COLD;
	u8 reg_r(offs_t reg);
	void reg_w(offs_t reg, u8 data);
	void mmu_w(offs_t bank, u8 data);
	void mmu_x2_w(offs_t bank, u8 data);
	u8 dma_r(bool dma_mode);
	void dma_w(bool dma_mode, u8 data);
	u8 spi_data_r();
	void spi_data_w(u8 data);
	void spi_miso_w(u8 data);
	void i2c_scl_w(u8 data);
	void palette_val_w(u8 nr_palette_priority, u16 nr_palette_value);
	u8 port_ff_r();
	void port_ff_w(u8 data);
	void turbosound_address_w(u8 data);
	u8 mf_port_r(offs_t addr);
	void mf_port_w(offs_t addr, u8 data);
	attotime cooper_until_pos_r(u16 pos);

	void bank_update(u8 bank, u8 count);
	void bank_update(u8 bank);
	void memory_change(u16 port, u8 data);
	u16 get_layer2_active_page(u8 bank);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<z80n_device> m_maincpu;

private:
	static const u8 MACHINE_TBBLUE = 0x08;
	static const u8 MACHINE_NEXT = 0x0a;
	static const u8 MACHINE_NEXT_AB = 0xfa; // Anti Brick (reset disabled, bootrom)
	static const u8 G_VERSION = 0x32; // 3.02
	static const u8 G_SUB_VERSION = 0x01;
	static const u8 G_VIDEO_INC = 0b11;
	static const u16 UTM_FALLBACK_PEN = 0x800;

	virtual TIMER_CALLBACK_MEMBER(irq_off) override;
	virtual TIMER_CALLBACK_MEMBER(irq_on) override;
	INTERRUPT_GEN_MEMBER(specnext_interrupt);
	TIMER_CALLBACK_MEMBER(line_irq_on);
	INTERRUPT_GEN_MEMBER(line_interrupt);
	IRQ_CALLBACK_MEMBER(irq_callback);
	TIMER_CALLBACK_MEMBER(spi_clock);
	void line_irq_adjust();

	u8 g_machine_id() { return m_io_issue->read() ? MACHINE_NEXT : MACHINE_TBBLUE; }
	u8 g_board_issue() { return m_io_issue->read(); }
	bool machine_type_48() const { return m_nr_03_machine_type == 0 || m_nr_03_machine_type == 1; }
	bool machine_type_128() const { return m_nr_03_machine_type == 2 || m_nr_03_machine_type == 4; }
	bool machine_type_p3() const { return !machine_type_48() && !machine_type_128(); }

	void nr_02_w(u8 nr_wr_dat);
	bool nr_02_iotrap() { return m_nr_da_iotrap_cause & 3; }
	void nr_07_cpu_speed_w(u8 data);

	void nr_0a_mf_type_w(u8 data) { m_nr_0a_mf_type = data; m_mf->mf_mode_w(m_nr_0a_mf_type); }
	void nr_12_layer2_active_bank_w(u8 data) { m_nr_12_layer2_active_bank = data; bank_update(0, 6); m_layer2->layer2_active_bank_w(m_nr_12_layer2_active_bank); }
	void nr_13_layer2_shadow_bank_w(u8 data) { m_nr_13_layer2_shadow_bank = data; bank_update(0, 6); }
	void nr_14_global_transparent_rgb_w(u8 data);
	void nr_15_sprite_priority_w(bool data) { m_nr_15_sprite_priority = data; m_sprites->zero_on_top_w(m_nr_15_sprite_priority); }
	void nr_15_sprite_border_clip_en_w(bool data) { m_nr_15_sprite_border_clip_en = data; m_sprites->border_clip_en_w(m_nr_15_sprite_border_clip_en); }
	void nr_15_sprite_over_border_en_w(bool data) { m_nr_15_sprite_over_border_en = data; m_sprites->over_border_w(m_nr_15_sprite_over_border_en); }
	void nr_16_layer2_scrollx_w(u8 data) { m_nr_16_layer2_scrollx = data; m_layer2->scroll_x_w((m_nr_71_layer2_scrollx_msb << 8) | m_nr_16_layer2_scrollx); }
	void nr_17_layer2_scrolly_w(u8 data) { m_nr_17_layer2_scrolly = data; m_layer2->scroll_y_w(m_nr_17_layer2_scrolly); }
	void nr_18_layer2_clip_x1_w(u8 data) { m_nr_18_layer2_clip_x1 = data; m_layer2->clip_x1_w(m_nr_18_layer2_clip_x1); }
	void nr_18_layer2_clip_x2_w(u8 data) { m_nr_18_layer2_clip_x2 = data; m_layer2->clip_x2_w(m_nr_18_layer2_clip_x2); }
	void nr_18_layer2_clip_y1_w(u8 data) { m_nr_18_layer2_clip_y1 = data; m_layer2->clip_y1_w(m_nr_18_layer2_clip_y1); }
	void nr_18_layer2_clip_y2_w(u8 data) { m_nr_18_layer2_clip_y2 = data; m_layer2->clip_y2_w(m_nr_18_layer2_clip_y2); }
	void nr_19_sprite_clip_x1_w(u8 data) { m_nr_19_sprite_clip_x1 = data; m_sprites->clip_x1_w(m_nr_19_sprite_clip_x1); }
	void nr_19_sprite_clip_x2_w(u8 data) { m_nr_19_sprite_clip_x2 = data; m_sprites->clip_x2_w(m_nr_19_sprite_clip_x2); }
	void nr_19_sprite_clip_y1_w(u8 data) { m_nr_19_sprite_clip_y1 = data; m_sprites->clip_y1_w(m_nr_19_sprite_clip_y1); }
	void nr_19_sprite_clip_y2_w(u8 data) { m_nr_19_sprite_clip_y2 = data; m_sprites->clip_y2_w(m_nr_19_sprite_clip_y2); }
	void nr_1a_ula_clip_x1_w(u8 data) { m_nr_1a_ula_clip_x1 = data; m_ula->ula_clip_x1_w(m_nr_1a_ula_clip_x1); m_lores->clip_x1_w(m_nr_1a_ula_clip_x1); }
	void nr_1a_ula_clip_x2_w(u8 data) { m_nr_1a_ula_clip_x2 = data; m_ula->ula_clip_x2_w(m_nr_1a_ula_clip_x2); m_lores->clip_x2_w(m_nr_1a_ula_clip_x2); }
	void nr_1a_ula_clip_y1_w(u8 data) { m_nr_1a_ula_clip_y1 = data; m_ula->ula_clip_y1_w(m_nr_1a_ula_clip_y1); m_lores->clip_y1_w(m_nr_1a_ula_clip_y1); }
	void nr_1a_ula_clip_y2_w(u8 data);
	void nr_1b_tm_clip_x1_w(u8 data) { m_nr_1b_tm_clip_x1 = data; m_tiles->clip_x1_w(m_nr_1b_tm_clip_x1); }
	void nr_1b_tm_clip_x2_w(u8 data) { m_nr_1b_tm_clip_x2 = data; m_tiles->clip_x2_w(m_nr_1b_tm_clip_x2); }
	void nr_1b_tm_clip_y1_w(u8 data) { m_nr_1b_tm_clip_y1 = data; m_tiles->clip_y1_w(m_nr_1b_tm_clip_y1); }
	void nr_1b_tm_clip_y2_w(u8 data) { m_nr_1b_tm_clip_y2 = data; m_tiles->clip_y2_w(m_nr_1b_tm_clip_y2); }
	void nr_26_ula_scrollx_w(u8 data) { m_nr_26_ula_scrollx = data; m_ula->ula_scroll_x_w(m_nr_26_ula_scrollx); }
	void nr_27_ula_scrolly_w(u8 data) { m_nr_27_ula_scrolly = data; m_ula->ula_scroll_y_w(m_nr_27_ula_scrolly); }

	void nr_30_tm_scrollx_w(u16 data) { m_nr_30_tm_scrollx = data; m_tiles->tm_scroll_x_w(m_nr_30_tm_scrollx); }
	void nr_31_tm_scrolly_w(u8 data) { m_nr_31_tm_scrolly = data; m_tiles->tm_scroll_y_w(m_nr_31_tm_scrolly); }
	void nr_32_lores_scrollx_w(u8 data) { m_nr_32_lores_scrollx = data; m_lores->scroll_x_w(m_nr_32_lores_scrollx); }
	void nr_33_lores_scrolly_w(u8 data) { m_nr_33_lores_scrolly = data; m_lores->scroll_y_w(m_nr_33_lores_scrolly); }

	void nr_42_ulanext_format_w(u8 data) { m_nr_42_ulanext_format = data; m_ula->ulanext_format_w(m_nr_42_ulanext_format); }
	void nr_43_ulanext_en_w(bool data) { m_nr_43_ulanext_en = data; m_ula->ulanext_en_w(m_nr_43_ulanext_en); m_lores->ulap_en_w(m_port_ff3b_ulap_en && !m_nr_43_ulanext_en); }
	void nr_43_active_ula_palette_w(bool data) { m_nr_43_active_ula_palette = data; m_ula->ula_palette_select_w(m_nr_43_active_ula_palette); m_lores->lores_palette_select_w(m_nr_43_active_ula_palette); }
	void nr_43_active_layer2_palette_w(bool data) { m_nr_43_active_layer2_palette = data; m_layer2->layer2_palette_select_w(m_nr_43_active_layer2_palette); }
	void nr_43_active_sprite_palette_w(bool data) { m_nr_43_active_sprite_palette = data; m_sprites->sprite_palette_select_w(m_nr_43_active_sprite_palette); }

	void nr_4b_sprite_transparent_index_w(u8 data) { m_nr_4b_sprite_transparent_index = data; m_sprites->transp_colour_w(m_nr_4b_sprite_transparent_index); }
	void nr_4c_tm_transparent_index_w(u8 data) { m_nr_4c_tm_transparent_index = data; m_tiles->transp_colour_w(m_nr_4c_tm_transparent_index); }

	void nr_62_copper_mode_w(u8 data) { m_nr_62_copper_mode = data; m_copper->copper_en_w(m_nr_62_copper_mode); }
	void nr_68_ula_fine_scroll_x_w(bool data) { m_nr_68_ula_fine_scroll_x = data; m_ula->ula_fine_scroll_x_w(m_nr_68_ula_fine_scroll_x); }

	void nr_6a_lores_radastan_w(bool data) { m_nr_6a_lores_radastan = data; m_lores->mode_w(m_nr_6a_lores_radastan); }
	void nr_6a_lores_radastan_xor_w(bool data) { m_nr_6a_lores_radastan_xor = data; m_lores->dfile_w(BIT(m_port_ff_data, 0) != m_nr_6a_lores_radastan_xor); }
	void nr_6a_lores_palette_offset_w(u8 data) { m_nr_6a_lores_palette_offset = data; m_lores->lores_palette_offset_w(m_nr_6a_lores_palette_offset); }
	void nr_6b_tm_control_w(u8 data) { m_nr_6b_tm_control = data; m_tiles->control_w(m_nr_6b_tm_control); }
	void nr_6c_tm_default_attr_w(u8 data) { m_nr_6c_tm_default_attr = data; m_tiles->default_flags_w(m_nr_6c_tm_default_attr); }
	void nr_6e_tilemap_base_w(bool data7, u8 data5_0) { m_nr_6e_tilemap_base_7 = data7; m_nr_6e_tilemap_base = data5_0; m_tiles->tm_map_base_w((m_nr_6e_tilemap_base_7 << 6) | m_nr_6e_tilemap_base); }
	void nr_6f_tilemap_tiles_w(bool data7, u8 data5_0) { m_nr_6f_tilemap_tiles_7 = data7; m_nr_6f_tilemap_tiles = data5_0; m_tiles->tm_tile_base_w((m_nr_6f_tilemap_tiles_7 << 6) | m_nr_6f_tilemap_tiles); }

	void nr_70_layer2_resolution_w(u8 data) { m_nr_70_layer2_resolution = data; m_layer2->resolution_w(m_nr_70_layer2_resolution); }
	void nr_70_layer2_palette_offset_w(u8 data) { m_nr_70_layer2_palette_offset = data; m_layer2->palette_offset_w(m_nr_70_layer2_palette_offset); }
	void nr_71_layer2_scrollx_msb_w(bool data) { m_nr_71_layer2_scrollx_msb = data; m_layer2->scroll_x_w((m_nr_71_layer2_scrollx_msb << 8) | m_nr_16_layer2_scrollx); }

	bool nr_8c_altrom_en() const { return BIT(m_nr_8c_altrom, 7); }
	bool nr_8c_altrom_rw() const { return BIT(m_nr_8c_altrom, 6); }
	bool nr_8c_altrom_lock_rom1() const { return BIT(m_nr_8c_altrom, 5); }
	bool nr_8c_altrom_lock_rom0() const { return BIT(m_nr_8c_altrom, 4); }

	bool nr_8f_mapping_mode_profi() const { return 0; }
	bool nr_8f_mapping_mode_pentagon() const { return m_nr_8f_mapping_mode == 0b10 || nr_8f_mapping_mode_pentagon_1024_en(); }
	bool nr_8f_mapping_mode_pentagon_1024() const { return m_nr_8f_mapping_mode == 0b11; }
	bool nr_8f_mapping_mode_pentagon_1024_en() const { return nr_8f_mapping_mode_pentagon_1024() && BIT(~m_port_eff7_data, 2); }

	void nr_c0_im2_vector_w(u8 data) { m_nr_c0_im2_vector = data; m_ctc->write(0, m_nr_c0_im2_vector << 5); }

	u32 internal_port_enable() const;
	bool port_ff_io_en() const { return BIT(internal_port_enable(), 0); }
	bool port_7ffd_io_en() const { return BIT(internal_port_enable(), 1); }
	bool port_dffd_io_en() const { return BIT(internal_port_enable(), 2); }
	bool port_1ffd_io_en() const { return BIT(internal_port_enable(), 3); }
	bool port_p3_floating_bus_io_en() const { return BIT(internal_port_enable(), 4); }
	bool port_dma_6b_io_en() const { return BIT(internal_port_enable(), 5); }
	bool port_1f_io_en() const { return BIT(internal_port_enable(), 6); }
	bool port_37_io_en() const { return BIT(internal_port_enable(), 7); }
	bool port_divmmc_io_en() const { return BIT(internal_port_enable(), 8); }
	bool port_multiface_io_en() const { return BIT(internal_port_enable(), 9); }
	bool port_i2c_io_en() const { return BIT(internal_port_enable(), 10); }
	bool port_spi_io_en() const { return BIT(internal_port_enable(), 11); }
	bool port_uart_io_en() const { return BIT(internal_port_enable(), 12); }
	bool port_mouse_io_en() const { return BIT(internal_port_enable(), 13); }
	bool port_sprite_io_en() const { return BIT(internal_port_enable(), 14); }
	bool port_layer2_io_en() const { return BIT(internal_port_enable(), 15); }
	bool port_ay_io_en() const { return BIT(internal_port_enable(), 16); }
	bool port_dac_sd1_ABCD_1f0f4f5f_io_en() const { return BIT(internal_port_enable(), 17); }
	bool port_dac_sd2_ABCD_f1f3f9fb_io_en() const { return BIT(internal_port_enable(), 18); }
	bool port_dac_stereo_AD_3f5f_io_en() const { return BIT(internal_port_enable(), 19); }
	bool port_dac_stereo_BC_0f4f_io_en() const { return BIT(internal_port_enable(), 20); }
	bool port_dac_mono_AD_fb_io_en() const { return BIT(internal_port_enable(), 21); }
	bool port_dac_mono_BC_b3_io_en() const { return BIT(internal_port_enable(), 22); }
	bool port_dac_mono_AD_df_io_en() const { return BIT(internal_port_enable(), 23); }
	bool port_ulap_io_en() const { return BIT(internal_port_enable(), 24); }
	bool port_dma_0b_io_en() const { return BIT(internal_port_enable(), 25); }
	bool port_eff7_io_en() const { return BIT(internal_port_enable(), 26); }
	bool port_ctc_io_en() const { return BIT(internal_port_enable(), 27); }

	u8 port_7ffd_bank() const { return (((nr_8f_mapping_mode_pentagon() || nr_8f_mapping_mode_profi()) ? 0 : BIT(m_port_dffd_data, 3)) << 6) | ((!nr_8f_mapping_mode_pentagon() ? BIT(m_port_dffd_data, 2) : (nr_8f_mapping_mode_pentagon_1024_en() && BIT(m_port_7ffd_data, 5))) << 5) | ((nr_8f_mapping_mode_pentagon() ? BIT(m_port_7ffd_data, 6, 2) : (m_port_dffd_data & 3)) << 3) | (m_port_7ffd_data & 7); }
	bool port_7ffd_shadow() const { return BIT(m_port_7ffd_data, 3); }
	bool port_7ffd_locked() const { return (nr_8f_mapping_mode_pentagon_1024_en() || (nr_8f_mapping_mode_profi() && BIT(m_port_dffd_data, 4))) ? 0 : BIT(m_port_7ffd_data, 5); }
	bool port_1ffd_special() const { return BIT(m_port_1ffd_data, 0); }
	u8 port_1ffd_rom() const { return (BIT(m_port_1ffd_data, 2) << 1) | BIT(m_port_7ffd_data, 4); }
	void port_7ffd_reg_w(u8 data);
	bool port_ff_interrupt_disable() { return BIT(m_port_ff_data, 6); }

	void port_123b_layer2_en_w(bool data) { m_screen->update_now(); m_port_123b_layer2_en = data; m_layer2->layer2_en_w(m_port_123b_layer2_en); }

	void port_ff3b_ulap_en_w(bool data) { m_port_ff3b_ulap_en = data; m_ula->ulap_en_w(m_port_ff3b_ulap_en); m_lores->ulap_en_w(m_port_ff3b_ulap_en && !m_nr_43_ulanext_en); }
	u16 nr_palette_dat();

	void port_e3_reg_w(u8 data);
	void port_e7_reg_w(u8 data);

	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_next_regs;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;
	memory_bank_creator m_bank_boot_rom;
	memory_bank_array_creator<8> m_bank_ram;
	memory_view m_view0, m_view1, m_view2, m_view3, m_view4, m_view5, m_view6, m_view7;
	required_device<specnext_copper_device> m_copper;
	required_device<specnext_ctc_device> m_ctc;
	required_device<specnext_dma_device> m_dma;
	required_device<i2cmem_device> m_i2cmem;
	required_device<spi_sdcard_device> m_sdcard;
	required_device_array<ym2149_device, 3> m_ay;
	required_device_array<dac_byte_interface, 4> m_dac;
	required_device<device_palette_interface> m_palette;
	required_device<address_map_bank_device> m_regs_map;
	required_device<specnext_multiface_device> m_mf;
	required_device<specnext_divmmc_device> m_divmmc;
	required_device<screen_ula_device> m_ula;
	required_device<specnext_tiles_device> m_tiles;
	required_device<specnext_layer2_device> m_layer2;
	required_device<specnext_lores_device> m_lores;
	required_device<specnext_sprites_device> m_sprites;
	required_ioport m_io_issue;
	required_ioport_array<3> m_io_mouse;

	int m_page_shadow[8];
	bool m_bootrom_en;
	u8 m_port_ff_data;
	bool m_port_1ffd_special_old;
	u8 m_port_1ffd_data;
	u8 m_port_7ffd_data;
	u8 m_port_dffd_data;
	u8 m_port_eff7_data;
	u8 m_port_e7_reg;
	u8 m_nr_register;
	u8 m_port_e3_reg;
	bool m_divmmc_delayed_check;
	u16 m_global_transparent;

	u8 m_sram_rom;
	bool m_sram_rom3;
	bool m_sram_alt_128_n;

	u8 m_mmu[8];
	bool m_nr_02_bus_reset;
	bool m_nr_02_generate_mf_nmi;
	bool m_nr_02_generate_divmmc_nmi;
	bool m_nr_02_hard_reset;
	u8 m_nr_02_reset_type; // u3
	u8 m_nr_03_machine_type; // u3
	bool m_nr_03_user_dt_lock;
	u8 m_nr_03_machine_timing; // u3
	bool m_nr_03_config_mode;
	u8 m_nr_04_romram_bank; // u7
	u8 m_nr_05_joy1; // u2
	u8 m_nr_05_joy0; // u2
	u8 m_nr_06_psg_mode; // u2
	bool m_nr_06_ps2_mode;
	bool m_nr_06_button_m1_nmi_en;
	bool m_nr_06_button_drive_nmi_en;
	bool m_nr_06_hotkey_5060_en;
	bool m_nr_06_internal_speaker_beep;
	bool m_nr_06_hotkey_cpu_speed_en;
	u8 m_nr_07_cpu_speed; // u2
	bool m_nr_08_keyboard_issue2;
	bool m_nr_08_psg_turbosound_en;
	bool m_nr_08_port_ff_rd_en;
	bool m_nr_08_dac_en;
	bool m_nr_08_internal_speaker_en;
	bool m_nr_08_psg_stereo_mode;
	bool m_nr_08_contention_disable;
	bool m_nr_09_hdmi_audio_en;
	bool m_nr_09_sprite_tie;
	u8 m_nr_09_psg_mono; // u3
	u8 m_nr_0a_mouse_dpi; // u2
	bool m_nr_0a_mouse_button_reverse;
	bool m_nr_0a_divmmc_automap_en;
	u8 m_nr_0a_mf_type; // u2
	bool m_nr_0b_joy_iomode_0;
	u8 m_nr_0b_joy_iomode; // u2
	bool m_nr_0b_joy_iomode_en;
	u8 m_nr_10_coreid; // u5
	u8 m_nr_11_video_timing; // u3
	bool m_nr_10_flashboot;
	u8 m_nr_12_layer2_active_bank; // u7
	u8 m_nr_13_layer2_shadow_bank; // u7
	u8 m_nr_14_global_transparent_rgb;
	bool m_nr_15_sprite_en;
	bool m_nr_15_sprite_over_border_en;
	u8 m_nr_15_layer_priority; // u3
	bool m_nr_15_sprite_border_clip_en;
	bool m_nr_15_sprite_priority;
	bool m_nr_15_lores_en;
	u8 m_nr_16_layer2_scrollx;
	u8 m_nr_17_layer2_scrolly;
	u8 m_nr_18_layer2_clip_x1;
	u8 m_nr_18_layer2_clip_x2;
	u8 m_nr_18_layer2_clip_y1;
	u8 m_nr_18_layer2_clip_y2;
	u8 m_nr_18_layer2_clip_idx; // u2
	u8 m_nr_19_sprite_clip_x1;
	u8 m_nr_19_sprite_clip_x2;
	u8 m_nr_19_sprite_clip_y1;
	u8 m_nr_19_sprite_clip_y2;
	u8 m_nr_19_sprite_clip_idx; // u2
	u8 m_nr_1a_ula_clip_x1;
	u8 m_nr_1a_ula_clip_x2;
	u8 m_nr_1a_ula_clip_y1;
	u8 m_nr_1a_ula_clip_y2;
	u8 m_nr_1a_ula_clip_idx; // u2
	u8 m_nr_1b_tm_clip_x1;
	u8 m_nr_1b_tm_clip_x2;
	u8 m_nr_1b_tm_clip_y1;
	u8 m_nr_1b_tm_clip_y2;
	u8 m_nr_1b_tm_clip_idx; // u2
	bool m_nr_22_line_interrupt_en;
	u16 m_nr_23_line_interrupt; // u9
	u8 m_nr_26_ula_scrollx;
	u8 m_nr_27_ula_scrolly;
	u8 m_nr_2d_i2s_sample; // u2
	u16 m_nr_30_tm_scrollx; // u10
	u8 m_nr_31_tm_scrolly;
	u8 m_nr_32_lores_scrollx;
	u8 m_nr_33_lores_scrolly;
	u8 m_nr_palette_idx;
	bool m_nr_palette_sub_idx;
	u8 m_nr_42_ulanext_format;
	bool m_nr_43_palette_autoinc_disable;
	u8 m_nr_43_palette_write_select; // u3
	bool m_nr_43_active_sprite_palette;
	bool m_nr_43_active_layer2_palette;
	bool m_nr_43_active_ula_palette;
	bool m_nr_43_ulanext_en;
	u8 m_nr_stored_palette_value;
	u8 m_nr_4a_fallback_rgb;
	u8 m_nr_4b_sprite_transparent_index;
	u8 m_nr_4c_tm_transparent_index; // u4
	u16 m_nr_copper_addr; // u11
	u8 m_nr_copper_data_stored;
	u8 m_nr_62_copper_mode; // u2
	u8 m_nr_64_copper_offset;
	bool m_nr_68_ula_en;
	u8 m_nr_68_blend_mode; // u2
	bool m_nr_68_cancel_extended_keys;
	bool m_nr_68_ula_fine_scroll_x;
	bool m_nr_68_ula_stencil_mode;
	bool m_nr_6a_lores_radastan;
	bool m_nr_6a_lores_radastan_xor;
	u8 m_nr_6a_lores_palette_offset; // u4
	bool m_nr_6b_tm_en;
	u8 m_nr_6b_tm_control; // u7
	u8 m_nr_6c_tm_default_attr;
	u8 m_nr_6e_tilemap_base; // u6
	bool m_nr_6e_tilemap_base_7;
	u8 m_nr_6f_tilemap_tiles; // u6
	bool m_nr_6f_tilemap_tiles_7;
	u8 m_nr_70_layer2_resolution; // u2
	u8 m_nr_70_layer2_palette_offset; // u4
	bool m_nr_71_layer2_scrollx_msb;
	u8 m_nr_7f_user_register_0;
	u8 m_nr_80_expbus;
	u8 m_nr_81_expbus_speed; // u2
	bool m_nr_81_expbus_clken;
	bool m_nr_81_expbus_nmi_debounce_disable;
	bool m_nr_81_expbus_ula_override;
	u8 m_nr_82_internal_port_enable;
	u8 m_nr_83_internal_port_enable;
	u8 m_nr_84_internal_port_enable;
	u8 m_nr_85_internal_port_enable; // u4
	bool m_nr_85_internal_port_reset_type;
	u8 m_nr_86_bus_port_enable;
	u8 m_nr_87_bus_port_enable;
	u8 m_nr_88_bus_port_enable;
	u8 m_nr_89_bus_port_enable; // u4
	bool m_nr_89_bus_port_reset_type;
	u8 m_nr_8a_bus_port_propagate; // u6
	u8 m_nr_8c_altrom;
	u8 m_nr_8f_mapping_mode; // u2
	u8 m_nr_90_pi_gpio_o_en;
	u8 m_nr_91_pi_gpio_o_en;
	u8 m_nr_92_pi_gpio_o_en;
	u8 m_nr_93_pi_gpio_o_en; // u4
	u8 m_nr_98_pi_gpio_o;
	u8 m_nr_99_pi_gpio_o;
	u8 m_nr_9a_pi_gpio_o;
	u8 m_nr_9b_pi_gpio_o; // u4
	u8 m_nr_a0_pi_peripheral_en;
	u8 m_nr_a2_pi_i2s_ctl;
	bool m_nr_a8_esp_gpio0_en;
	bool m_nr_a9_esp_gpio0;
	u8 m_nr_b8_divmmc_ep_0;
	u8 m_nr_b9_divmmc_ep_valid_0;
	u8 m_nr_ba_divmmc_ep_timing_0;
	u8 m_nr_bb_divmmc_ep_1;
	u8 m_nr_c0_im2_vector; // u3
	bool m_nr_c0_stackless_nmi;
	bool m_nr_c0_int_mode_pulse_0_im2_1;
	u8 m_nr_c2_retn_address_lsb;
	u8 m_nr_c3_retn_address_msb;
	bool m_nr_c4_int_en_0_expbus;
	u8 m_nr_c6_int_en_2_654; // u3
	u8 m_nr_c6_int_en_2_210; // u3
	bool m_nr_cc_dma_int_en_0_7;
	u8 m_nr_cc_dma_int_en_0_10; // u2
	u8 m_nr_cd_dma_int_en_1;
	u8 m_nr_ce_dma_int_en_2_654; // u3
	u8 m_nr_ce_dma_int_en_2_210; // u3
	bool m_nr_d8_io_trap_fdc_en;
	u8 m_nr_d9_iotrap_write;
	u8 m_nr_da_iotrap_cause; // u2
	bool m_nr_f0_select;
	bool m_nr_f0_xdna_en;
	bool m_nr_f0_xadc_en;
	u8 m_nr_f0_xdev_cmd;
	bool m_nr_f0_xadc_eoc;
	bool m_nr_f0_xadc_eos;
	bool m_nr_f8_xadc_dwe;
	u8 m_nr_f8_xadc_daddr; // u7
	bool m_nr_f8_xadc_den;
	u8 m_nr_f9_xadc_d0;
	u8 m_nr_fa_xadc_d1;

	bool m_pulse_int_n;
	u8 m_nr_09_scanlines; // u2

	u8 m_eff_nr_03_machine_timing; // u3
	bool m_eff_nr_05_5060;
	bool m_eff_nr_05_scandouble_en;
	bool m_eff_nr_08_contention_disable;
	u8 m_eff_nr_09_scanlines; // u2

	bool m_port_123b_layer2_en;
	bool m_port_123b_layer2_map_wr_en;
	bool m_port_123b_layer2_map_rd_en;
	bool m_port_123b_layer2_map_shadow;
	u8 m_port_123b_layer2_map_segment; // u2
	u8 m_port_123b_layer2_offset; // u3

	u8 m_port_bf3b_ulap_mode; // u2
	u8 m_port_bf3b_ulap_index; // u6
	bool m_port_ff3b_ulap_en;

	u8 m_ay_select; // u2

	emu_timer *m_irq_line_timer;
	emu_timer *m_spi_clock;
	int m_spi_clock_cycles;
	bool m_spi_clock_state;
	u8 m_spi_mosi_dat;
	u8 m_spi_miso_dat;
	bool m_i2c_scl_data;

	u16 m_irq_mask;
};

void specnext_state::bank_update(u8 bank, u8 count)
{
	for (auto b = bank; count; ++b, --count)
		bank_update(b);
}

void specnext_state::bank_update(u8 bank)
{
	using views_link = std::reference_wrapper<memory_view>;
	views_link views[] = { m_view0, m_view1, m_view2, m_view3, m_view4, m_view5, m_view6, m_view7 };

	const bool is_rom = (bank >> 1) == 0;
	if (m_bootrom_en && is_rom)
		return views[bank].get().select(1);

	if (machine_type_48())
	{
		m_sram_rom = 0b00;
		m_sram_rom3 = 1;
		m_sram_alt_128_n = !(!nr_8c_altrom_lock_rom1() && nr_8c_altrom_lock_rom0());
	}
	else if (machine_type_p3())
	{
		if (nr_8c_altrom_lock_rom1() || nr_8c_altrom_lock_rom0())
		{
			m_sram_rom = (nr_8c_altrom_lock_rom1() << 1) | nr_8c_altrom_lock_rom0();
			m_sram_rom3 = nr_8c_altrom_lock_rom1() && nr_8c_altrom_lock_rom0();
			m_sram_alt_128_n = nr_8c_altrom_lock_rom1();
		}
		else
		{
			m_sram_rom = port_1ffd_rom();
			m_sram_rom3 = port_1ffd_rom() == 0x03;
			m_sram_alt_128_n = port_1ffd_rom() & 1;
		}
	}
	else
	{
		if (nr_8c_altrom_lock_rom1() || nr_8c_altrom_lock_rom0())
		{
			m_sram_rom = nr_8c_altrom_lock_rom1();
			m_sram_rom3 = nr_8c_altrom_lock_rom1();
			m_sram_alt_128_n = nr_8c_altrom_lock_rom1();
		}
		else
		{
			m_sram_rom = port_1ffd_rom() & 1;
			m_sram_rom3 = port_1ffd_rom() & 1;
			m_sram_alt_128_n = port_1ffd_rom() & 1;
		}
	}

	const u8 mem_active_page = m_mmu[bank];
	const bool mem_active_bank5 = mem_active_page == 0x0a || mem_active_page == 0x0b;
	const bool mem_active_bank7 = mem_active_page == 0x0e;
	const u16 mmu_A21_A13 = ((0b0001 + BIT(mem_active_page, 5, 3)) << 5) | BIT(mem_active_page, 0, 5);

	const u8 layer2_active_bank_offset_pre = m_port_123b_layer2_map_segment == 0b11 ? BIT(bank, 1, 2) : m_port_123b_layer2_map_segment;
	const u8 layer2_active_bank_offset = layer2_active_bank_offset_pre + m_port_123b_layer2_offset;
	const u8 layer2_active_bank = m_port_123b_layer2_map_shadow ? m_nr_13_layer2_shadow_bank : m_nr_12_layer2_active_bank;
	const u8 layer2_active_page = ((layer2_active_bank + layer2_active_bank_offset) << 1) | BIT(bank, 0);
	const u16 layer2_A21_A13 = ((0b0001 + BIT(layer2_active_page, 5, 3)) << 5) | BIT(layer2_active_page, 0, 5);

	// sram_pre_romcs_replace = expbus_romcs_replace;
	const bool sram_pre_alt_en = nr_8c_altrom_en();
	const bool sram_pre_alt_128_n = m_sram_alt_128_n;
	const bool sram_pre_rom3 = m_sram_rom3;
	const bool sram_pre_layer2_rd_en = m_port_123b_layer2_map_rd_en;
	const bool sram_pre_layer2_wr_en = m_port_123b_layer2_map_wr_en;
	const u16 sram_pre_layer2_A21_A13 = layer2_A21_A13;

	u8 sram_pre_A20_A13, sram_pre_override; // u3: divmmc & layer 2 & romcs
	bool sram_pre_active, sram_pre_bank5, sram_pre_bank7, sram_pre_rdonly;
	if (is_rom) // 0-1
	{
		const bool cpu_a13 = bank & 1;
		m_mf->enable_w(port_multiface_io_en());
		if (m_mf->mf_enabled_r())
		{
			sram_pre_A20_A13 = 0b00001010 | cpu_a13;
			sram_pre_active = 1;
			sram_pre_bank5 = sram_pre_bank7 = 0;
			sram_pre_rdonly = !cpu_a13;
			sram_pre_override = 0b000;
		}
		else if (BIT(~mmu_A21_A13, 8))
		{
			sram_pre_A20_A13 = mmu_A21_A13 & 0xff;
			sram_pre_active = !mem_active_bank5 && !mem_active_bank7;
			sram_pre_bank5 = mem_active_bank5;
			sram_pre_bank7 = mem_active_bank7;
			sram_pre_rdonly = 0;
			sram_pre_override = 0b110;
		}
		else if (m_nr_03_config_mode)
		{
			sram_pre_A20_A13 = (m_nr_04_romram_bank << 1) | cpu_a13;
			sram_pre_active = 1;
			sram_pre_bank5 = sram_pre_bank7 = 0;
			sram_pre_rdonly = 0;
			sram_pre_override = 0b110;
		}
		else
		{
			sram_pre_A20_A13 = (m_sram_rom << 1) | cpu_a13;
			sram_pre_active = 1;
			sram_pre_bank5 = sram_pre_bank7 = 0;
			sram_pre_rdonly = !(nr_8c_altrom_en() && nr_8c_altrom_rw());
			sram_pre_override = 0b111;
		}
	}
	else
	{
		sram_pre_A20_A13 = mmu_A21_A13 & 0xff;
		sram_pre_active = BIT(~mmu_A21_A13, 8) && !mem_active_bank5 && !mem_active_bank7;
		sram_pre_bank5 = mem_active_bank5;
		sram_pre_bank7 = mem_active_bank7;
		sram_pre_rdonly = 0;
		sram_pre_override = (((bank & 0x06) != 0x06) && (m_port_123b_layer2_map_segment == 0x03)) << 1;
	}

	const bool sram_pre_romcs_n = 0; // expbus_eff_en && !expbus_eff_disable_mem;
	const bool sram_romcs = BIT(sram_pre_override, 0) && sram_pre_romcs_n;
	const bool sram_divmmc_automap_en = BIT(sram_pre_override, 2);

	m_divmmc->cpu_a_15_13_w(bank);
	m_divmmc->en_w(port_divmmc_io_en());
	m_divmmc->automap_reset_w(!port_divmmc_io_en() || !m_nr_0a_divmmc_automap_en);
	m_divmmc->automap_active_w(sram_divmmc_automap_en);
	m_divmmc->retn_seen_w(0);
	m_divmmc->divmmc_button_w(m_nr_02_generate_divmmc_nmi);

	for (s8 cpu_rd_n = 1; cpu_rd_n >= 0; --cpu_rd_n) // check W then R
	{
		const bool sram_layer2_map_en = BIT(sram_pre_override, 1) && ((sram_pre_layer2_wr_en && cpu_rd_n) || (sram_pre_layer2_rd_en && !cpu_rd_n));
		const bool sram_altrom_en = !(BIT(~sram_pre_override, 0) || !sram_pre_alt_en || (sram_pre_rdonly && cpu_rd_n) || (!sram_pre_rdonly && !cpu_rd_n));
		const bool sram_divmmc_automap_rom3_en = ((sram_pre_override & 0x05) == 0x05) && !sram_layer2_map_en && !sram_romcs && ((sram_altrom_en && sram_pre_alt_128_n) || (sram_pre_rom3 && !sram_altrom_en));

		m_divmmc->automap_rom3_active_w(sram_divmmc_automap_rom3_en);
		m_divmmc->clock_w();

		u8 sram_A20_A13;
		bool sram_active, sram_bank5, sram_bank7, sram_rdonly, sram_romcs_en, sram_mem_hide_n;
		if (BIT(sram_pre_override, 2) && m_divmmc->divmmc_rom_en_r())
		{
			sram_A20_A13 = 0b00001000;
			sram_active = 1;
			sram_bank5 = sram_bank7 = 0;
			sram_rdonly = 1;
			sram_romcs_en = 0;
			sram_mem_hide_n = 0;
		}
		else if (BIT(sram_pre_override, 2) && m_divmmc->divmmc_ram_en_r())
		{
			sram_A20_A13 = 0b00010000 | m_divmmc->divmmc_ram_bank_r();
			sram_active = 1;
			sram_bank5 = sram_bank7 = 0;
			sram_rdonly = m_divmmc->divmmc_rdonly_r();
			sram_romcs_en = 0;
			sram_mem_hide_n = 0;
		}
		else if (sram_layer2_map_en)
		{
			sram_A20_A13 = sram_pre_layer2_A21_A13 & 0xff;
			sram_active = BIT(~sram_pre_layer2_A21_A13, 8);
			sram_bank5 = sram_bank7 = 0;
			sram_rdonly = 0;
			sram_romcs_en = 0;
			sram_mem_hide_n = 0;
		}
		else if (sram_romcs)
		{
			sram_A20_A13 = 0b00011110 | BIT(sram_pre_A20_A13, 0);
			sram_active = 1;
			sram_bank5 = sram_bank7 = 0;
			sram_rdonly = 1;
			sram_romcs_en = 1;
			sram_mem_hide_n = 1;
		}
		else if (sram_altrom_en)
		{
			sram_A20_A13 = 0b00001100 | (sram_pre_alt_128_n << 1) | BIT(sram_pre_A20_A13, 0);
			sram_active = 1;
			sram_bank5 = sram_bank7 = 0;
			sram_rdonly = sram_pre_rdonly;
			sram_romcs_en = 0;
			sram_mem_hide_n = 1;
		}
		else
		{
			sram_A20_A13 = sram_pre_A20_A13;
			sram_active = sram_pre_active;
			sram_bank5 = sram_pre_bank5;
			sram_bank7 = sram_pre_bank7;
			sram_rdonly = sram_pre_rdonly;
			sram_romcs_en = 0;
			sram_mem_hide_n = BIT(sram_pre_override, 0);
		}

		if (false) // unused in current implementation, makes compiler happy
			printf("%d", sram_active + sram_bank5 + sram_romcs_en + sram_mem_hide_n);

		if (cpu_rd_n)
		{
			m_page_shadow[bank] = sram_rdonly ? ~0 : sram_A20_A13;
		}
		else
		{
			m_bank_ram[bank]->set_entry(sram_A20_A13);
			if (sram_rdonly || (m_page_shadow[bank] != sram_A20_A13))
			{
				views[bank].get().select(0);
				LOGMEM("ROM%d = %x\n", bank, sram_A20_A13);
			}
			else
			{
				if (m_page_shadow[bank] == sram_A20_A13)
					m_page_shadow[bank] = ~0;
				views[bank].get().disable();
				LOGMEM("RAM%d = %x\n", bank, sram_A20_A13);
			}
		}
	}
}

void specnext_state::memory_change(u16 port, u8 data) // port_memory_change_dly
{
	if (port_1ffd_special())
	{
		u8 b3 = (BIT(m_port_1ffd_data, 2) || BIT(m_port_1ffd_data, 1)) << 3;
		mmu_x2_w(0, b3 | 0b000);
		const u8 b2 = (BIT(m_port_1ffd_data, 2) && BIT(m_port_1ffd_data, 1)) << 2;
		mmu_x2_w(2, b3 | b2 | 0b10);
		mmu_x2_w(4, b3 | 0b100);
		b3 = (BIT(~m_port_1ffd_data, 2) && BIT(m_port_1ffd_data, 1)) << 3;
		mmu_x2_w(6, b3 | 0b110);
	}
	else
	{
		const bool mode_profi = nr_8f_mapping_mode_profi();

		if (BIT(m_port_eff7_data, 3) || (mode_profi && BIT(m_port_dffd_data, 4)))
			mmu_x2_w(0, 0x00);
		else
			mmu_x2_w(0, 0xff);

		if (mode_profi && BIT(m_port_dffd_data, 3))
			mmu_x2_w(2, (port_7ffd_bank() << 1) | 0);
		else if (mode_profi || m_port_1ffd_special_old)
			mmu_x2_w(2, 0x0a);

		if (mode_profi && BIT(m_port_dffd_data, 6))
			mmu_x2_w(4, 0x0c);
		else if (mode_profi || m_port_1ffd_special_old)
			mmu_x2_w(4, 0x04);

		const bool port_memory_ram_change_dly = !(port == 0x8e && !BIT(data, 3));
		if (mode_profi && BIT(m_port_dffd_data, 3))
			mmu_x2_w(6, 0x0e);
		else if (m_port_1ffd_special_old || port_memory_ram_change_dly)
			mmu_x2_w(6, (port_7ffd_bank() << 1) | 0);
	}

	m_port_1ffd_special_old = port_1ffd_special();
}

u32 specnext_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle clip256x192 = SCR_256x192;
	clip256x192 &= cliprect;
	rectangle clip320x256 = SCR_320x256;
	clip320x256 &= cliprect;

	screen.priority().fill(0, cliprect);
	const bool flash = u64(screen.frame_number() / m_frame_invert_count) & 1;
	if (m_nr_15_layer_priority < 0b110)
	{
		// background
		if (m_nr_68_ula_en)
		{
			m_ula->draw_border(bitmap, cliprect, m_port_fe_data & 0x07);
		}
		else {
			bitmap.fill(m_palette->pen_color(UTM_FALLBACK_PEN), cliprect);
		}

		static const u8 lcfg[][3] =
		{
			// tiles+ula priority; l2 prioryty; l2 mask
			// + l2 pushes priority colors to 8 (foreground)
			{ 1, 1, 0 }, // SLU
			{ 1, 8, 0 }, // LSU
			{ 1, 1, 1 }, // SUL
			{ 8, 8, 0 }, // LUS
			{ 8, 1, 8 }, // USL
			{ 8, 8, 8 }  // ULS
		};

		const u8 (&l)[3] = lcfg[m_nr_15_layer_priority];
		if (m_nr_6b_tm_en) m_tiles->draw(screen, bitmap, clip320x256, TILEMAP_DRAW_CATEGORY(1), l[0]);
		if (m_nr_68_ula_en && BIT(~m_nr_6b_tm_control, 3))
		{
			if (m_nr_15_lores_en) m_lores->draw(screen, bitmap, clip256x192, l[0]);
			else m_ula->draw(screen, bitmap, clip256x192, flash, l[0]);
		}
		if (m_nr_6b_tm_en) m_tiles->draw(screen, bitmap, clip320x256, TILEMAP_DRAW_CATEGORY(2), l[0]);
		m_layer2->draw(screen, bitmap, clip320x256, l[1], l[2]);
	}
	else // colors mixing case
	{
		bitmap.fill(m_palette->pen_color(UTM_FALLBACK_PEN), cliprect);

		if (m_nr_68_blend_mode != 0b11)
		//if (m_nr_68_blend_mode == 0b00)
		{
			if (m_nr_68_ula_en && BIT(~m_nr_6b_tm_control, 3))
			{
				if (m_nr_15_lores_en) m_lores->draw(screen, bitmap, clip256x192, 1);
				else m_ula->draw(screen, bitmap, clip256x192, flash, 1);
			}
			if (m_nr_6b_tm_en) m_tiles->draw(screen, bitmap, clip320x256, TILEMAP_DRAW_CATEGORY(1), 2);
			if (m_nr_6b_tm_en) m_tiles->draw(screen, bitmap, clip320x256, TILEMAP_DRAW_CATEGORY(2), 8);
		}
		/* TODO No tests for modes below yet
		else if (m_nr_68_blend_mode == 0b10)
		{
		}
		else if (m_nr_68_blend_mode == 0b11)
		{
		}
		*/
		else
		{
			if (m_nr_68_ula_en && BIT(~m_nr_6b_tm_control, 3))
			{
				if (m_nr_15_lores_en) m_lores->draw(screen, bitmap, clip256x192, 1);
				else m_ula->draw(screen, bitmap, clip256x192, flash, 1);
			}
			if (m_nr_6b_tm_en) m_tiles->draw(screen, bitmap, clip320x256, TILEMAP_DRAW_CATEGORY(1), 2);
			if (m_nr_6b_tm_en) m_tiles->draw(screen, bitmap, clip320x256, TILEMAP_DRAW_CATEGORY(2), 2);
		}
		// mixes only to 1
		m_layer2->draw_mix(screen, bitmap, clip320x256, m_nr_15_layer_priority & 1);
	}
	// sprites below foreground
	if (m_nr_15_sprite_en) m_sprites->draw(screen, bitmap, clip320x256, GFX_PMASK_8);

	return 0;
}

u32 specnext_state::internal_port_enable() const
{
	u32 ports_en = (m_nr_85_internal_port_enable << 24) | (m_nr_84_internal_port_enable << 16) | (m_nr_83_internal_port_enable << 8) | (m_nr_82_internal_port_enable << 0);
	if (m_nr_80_expbus & 0x80)
	{
		ports_en &= (m_nr_89_bus_port_enable << 24) | (m_nr_88_bus_port_enable << 16) | (m_nr_87_bus_port_enable << 8) | (m_nr_86_bus_port_enable << 0);
	}
	return ports_en;
}

u16 specnext_state::nr_palette_dat()
{
	u16 nr_palette_index = (BIT(m_nr_43_palette_write_select, 1) << 9) | (BIT(m_nr_43_palette_write_select, 2) << 8) | m_nr_palette_idx;
	if ((BIT(m_nr_43_palette_write_select, 1) != BIT(m_nr_43_palette_write_select, 0)))
		nr_palette_index |= 0x400;

	rgb_t rgb = m_palette->pen_color(nr_palette_index);
	return ((rgb.r() >> 5) << 6) | ((rgb.g() >> 5) << 3) | ((rgb.b() >> 5) << 0);
}

void specnext_state::palette_val_w(u8 nr_palette_priority, u16 nr_palette_value)
{
	m_screen->update_now();
	u16 nr_palette_index = (BIT(m_nr_43_palette_write_select, 1) << 9) | (BIT(m_nr_43_palette_write_select, 2) << 8) | m_nr_palette_idx;
	if ((BIT(m_nr_43_palette_write_select, 1) xor BIT(m_nr_43_palette_write_select, 0)))
		nr_palette_index |= 0x400;

	m_palette->set_pen_color(nr_palette_index, rgbexpand<3,3,3>(nr_palette_value, 6, 3, 0));
	m_layer2->pen_priority_w(nr_palette_index, BIT(nr_palette_priority, 1));
}

u8 specnext_state::port_ff_r()
{
	return (m_nr_08_port_ff_rd_en && port_ff_io_en())
		? m_port_ff_data // ==port_ff_dat_tmx
		: floating_bus_r();
}

void specnext_state::port_ff_w(u8 data)
{
	m_port_ff_data = data; // ==port_ff_dat_tmx
	m_ula->port_ff_reg_w(m_port_ff_data);
	nr_6a_lores_radastan_xor_w(m_nr_6a_lores_radastan_xor);

	// TODO confirm this
	u16 nr_palette_value = (data << 1) | BIT(data, 1) | BIT(data, 0);
	u16 nr_palette_index_utm = (BIT(m_nr_43_palette_write_select, 2) << 8) | (0b11 << 6) | m_port_bf3b_ulap_index;

	m_palette->set_pen_color(nr_palette_index_utm, rgbexpand<3,3,3>(nr_palette_value, 6, 3, 0));
}

void specnext_state::port_7ffd_reg_w(u8 data)
{
	m_port_7ffd_data = data;
	m_ula->ula_shadow_en_w(port_7ffd_shadow());
}

void specnext_state::port_e3_reg_w(u8 data)
{
	m_port_e3_reg = data & ~0x30;
	m_divmmc->divmmc_reg_w(m_port_e3_reg & ~0x30);
	bank_update(0, 2);
}

void specnext_state::port_e7_reg_w(u8 data)
{
	if ((data & 3) == 0b10)
		m_port_e7_reg = 0xfe;
	else if ((data & 3) == 0x01)
		m_port_e7_reg = 0xfd;
	else if ((data == 0xfb) || (data == 0xf7))
		m_port_e7_reg = data;
	else if ((data == 0x7f) && (m_nr_03_config_mode || BIT(m_nr_02_reset_type, 2)))
		m_port_e7_reg = 0x7f;
	else
		m_port_e7_reg = 0xff;

	// bit 7 = fpga flash, bit 3 = rpi1, bit 2 = rpi0, bit 1 = sd1, bit 0 = sd0
	m_sdcard->spi_ss_w(BIT(~m_port_e7_reg, 0));
}

u8 specnext_state::spi_data_r()
{
	u8 din = m_spi_miso_dat;
	if (!machine().side_effects_disabled())
		spi_data_w(0xff);

	return din;
}

void specnext_state::spi_data_w(u8 data)
{
	m_spi_mosi_dat = data;
#if TIMINGS_PERFECT
	m_spi_clock_cycles = 8;
	m_spi_clock->adjust(m_maincpu->clocks_to_attotime(1) / 4, 0, m_maincpu->clocks_to_attotime(1) / 4);
#else
	for (u8 m = 0x80; m; m >>= 1)
	{
		m_sdcard->spi_mosi_w(m_spi_mosi_dat & m ? 1 : 0);
		m_sdcard->spi_clock_w(CLEAR_LINE);
		m_sdcard->spi_clock_w(ASSERT_LINE);
	}
#endif
}

void specnext_state::spi_miso_w(u8 data)
{
	m_spi_miso_dat <<= 1;
	m_spi_miso_dat |= data;
}

void specnext_state::i2c_scl_w(u8 data)
{
	if (port_i2c_io_en())
	{
		m_i2c_scl_data = data & 1;
		m_i2cmem->write_scl(m_i2c_scl_data);
	}
}

void specnext_state::turbosound_address_w(u8 data)
{
	if ((data & 0x9c) == 0x9c)
		m_ay_select = (data & 3) % 3;
	else
		m_ay[m_nr_08_psg_turbosound_en ? m_ay_select : 0]->address_w(data);
}

u8 specnext_state::mf_port_r(offs_t addr)
{
	if (!machine().side_effects_disabled())
	{
		const u8 port = addr & 0xff;
		u8 port_mf_enable_io_a = 0x3f;
		u8 port_mf_disable_io_a = 0xbf;
		if (m_nr_0a_mf_type & 2)
		{
			port_mf_enable_io_a = 0x9f;
			port_mf_disable_io_a = 0x1f;
		}
		else if (m_nr_0a_mf_type & 1)
		{
			port_mf_enable_io_a = 0xbf;
			port_mf_disable_io_a = 0x3f;
		}

		m_mf->port_mf_enable_rd_w(port_multiface_io_en() && (port == port_mf_enable_io_a));
		m_mf->port_mf_disable_rd_w(port_multiface_io_en() && (port == port_mf_disable_io_a));
		m_mf->port_mf_enable_wr_w(0);
		m_mf->port_mf_disable_wr_w(0);
		m_mf->clock_w();

		bank_update(0, 2);
	}

	u8 data;
	if (!m_mf->mf_port_en_r())
		data = 0x00;
	else if (m_nr_0a_mf_type != 0b00)
		data = (BIT(m_port_7ffd_data, 3) << 7) | 0x7f;
	else
	{
		switch (BIT(addr, 12, 4))
		{
			case 0b0001: data = m_port_1ffd_data; break;
			case 0b0111: data = m_port_7ffd_data; break;
			case 0b1101: data = m_port_dffd_data; break;
			case 0b1110: data = m_port_eff7_data & 0xc0; break;
			default: data = m_port_fe_data & 0x07;
		}
	}

	if (!machine().side_effects_disabled())
	{
		m_mf->port_mf_enable_rd_w(0);
		m_mf->port_mf_disable_rd_w(0);
	}
	return data;
}

void specnext_state::mf_port_w(offs_t addr, u8 data)
{
	const u8 port = addr & 0xff;
	u8 port_mf_enable_io_a = 0x3f;
	u8 port_mf_disable_io_a = 0xbf;
	if (m_nr_0a_mf_type & 2)
	{
		port_mf_enable_io_a = 0x9f;
		port_mf_disable_io_a = 0x1f;
	}
	else if (m_nr_0a_mf_type & 1)
	{
		port_mf_enable_io_a = 0xbf;
		port_mf_disable_io_a = 0x3f;
	}

	m_mf->port_mf_enable_rd_w(0);
	m_mf->port_mf_disable_rd_w(0);
	m_mf->port_mf_enable_wr_w(port_multiface_io_en() && (port == port_mf_enable_io_a));
	m_mf->port_mf_disable_wr_w(port_multiface_io_en() && (port == port_mf_disable_io_a));
	m_mf->clock_w();
	bank_update(0, 2);
	m_mf->port_mf_enable_rd_w(0);
	m_mf->port_mf_disable_rd_w(0);
}

attotime specnext_state::cooper_until_pos_r(u16 pos)
{
	const u16 vcount = BIT(pos, 0, 9);
	const u16 hcount = ((BIT(pos, 9, 6) << 3) + (BIT(pos, 15) ? 12 : 0)) << 1;
	return ((vcount < m_screen->height()) && (hcount < m_screen->width()))
		? m_screen->time_until_pos(SCR_256x192.top() + vcount + m_nr_64_copper_offset, SCR_256x192.left() + hcount)
		: attotime::never;
}


TIMER_CALLBACK_MEMBER(specnext_state::spi_clock)
{
	if (m_spi_clock_cycles > 0)
	{
		if (m_spi_clock_state)
		{
			m_sdcard->spi_clock_w(ASSERT_LINE);
			m_spi_clock_cycles--;
		}
		else
		{
			m_sdcard->spi_mosi_w(BIT(m_spi_mosi_dat, m_spi_clock_cycles - 1));
			m_sdcard->spi_clock_w(CLEAR_LINE);
		}

		m_spi_clock_state = !m_spi_clock_state;
	}
	else
	{
		m_spi_clock_state = false;
		m_spi_clock->reset();
	}
}

void specnext_state::mmu_w(offs_t bank, u8 data)
{
	m_mmu[bank] = data;
	bank_update(bank);
}

void specnext_state::mmu_x2_w(offs_t bank, u8 data)
{
	mmu_w(bank, data);
	mmu_w(bank | 1, data == 0xff ? 0xff : (data | 1));
}

u8 specnext_state::dma_r(bool dma_mode)
{
	m_dma->set_dma_mode(dma_mode ? z80dma_device::dma_mode::ZILOG : z80dma_device::dma_mode::SPEC_NEXT);
	return m_dma->read();
}

void specnext_state::dma_w(bool dma_mode, u8 data)
{
	m_dma->set_dma_mode(dma_mode ? z80dma_device::dma_mode::ZILOG : z80dma_device::dma_mode::SPEC_NEXT);
	m_dma->write(data);
}

u8 specnext_state::reg_r(offs_t nr_register)
{
	u8 port_253b_dat;

	switch (nr_register)
	{
	case 0x00:
		port_253b_dat = g_machine_id();
		break;
	case 0x01:
		port_253b_dat = G_VERSION;
		break;
	case 0x02:
		port_253b_dat = (m_nr_02_bus_reset << 7) | (0b00 << 5) | (nr_02_iotrap() << 4) | (m_nr_02_generate_mf_nmi << 3) | (m_nr_02_generate_divmmc_nmi << 2) | BIT(m_nr_02_reset_type, 0, 2);
		break;
	case 0x03:
		port_253b_dat = (m_nr_palette_sub_idx << 7) | (0b00 << 5) | (m_nr_03_machine_timing << 4) | (m_nr_03_user_dt_lock << 3) | m_nr_03_machine_type;
		break;
	case 0x05:
		port_253b_dat = (BIT(m_nr_05_joy0, 0, 2) << 6) | (BIT(m_nr_05_joy1, 0, 2) << 4) | (BIT(m_nr_05_joy0, 2) << 3) | (m_eff_nr_05_5060 << 2) | (BIT(m_nr_05_joy1, 2) << 1) | m_eff_nr_05_scandouble_en;
		break;
	case 0x06:
		port_253b_dat = (m_nr_06_hotkey_cpu_speed_en << 7) | (m_nr_06_internal_speaker_beep << 6) | (m_nr_06_hotkey_5060_en << 5) | (m_nr_06_button_drive_nmi_en << 4) | (m_nr_06_button_m1_nmi_en << 3) | (m_nr_06_ps2_mode << 2) | m_nr_06_psg_mode;
		break;
	case 0x07:
		{
			u8 cpu_speed = 0;
			for (u8 clock_scale = BIT(u8(m_maincpu->clock_scale()), 0, 3) >> 1; clock_scale; clock_scale >>= 1, ++cpu_speed);
			port_253b_dat = (0b00 << 6) | (cpu_speed << 4) | (0b00 << 2) | m_nr_07_cpu_speed;
		}
		break;
	case 0x08:
		port_253b_dat = ((!port_7ffd_locked()) << 7) | (m_eff_nr_08_contention_disable << 6) | (m_nr_08_psg_stereo_mode << 5) | (m_nr_08_internal_speaker_en << 4) | (m_nr_08_dac_en << 3) | (m_nr_08_port_ff_rd_en << 2) | (m_nr_08_psg_turbosound_en << 1) | m_nr_08_keyboard_issue2;
		break;
	case 0x09:
		port_253b_dat = (m_nr_09_psg_mono  << 5) | (m_nr_09_sprite_tie << 4) | (0 << 3) | ((!m_nr_09_hdmi_audio_en) << 2) | m_nr_09_scanlines; // m_eff_nr_09_scanlines
		break;
	case 0x0a:
		port_253b_dat = (m_nr_0a_mf_type << 6) | (0 << 5) | (m_nr_0a_divmmc_automap_en << 4) | (m_nr_0a_mouse_button_reverse << 3) | (0 << 2) | m_nr_0a_mouse_dpi;
		break;
	case 0x0b:
		port_253b_dat = (m_nr_0b_joy_iomode_en << 7) | (0 << 6) | (m_nr_0b_joy_iomode << 4) | (0b000 << 1) | m_nr_0b_joy_iomode_0;
		break;
	case 0x0e:
		port_253b_dat = G_SUB_VERSION;
		break;
	case 0x0f:
		port_253b_dat = (0b0000 << 4) | g_board_issue();
		break;
	case 0x10:
		port_253b_dat = (0 << 7) | (m_nr_10_coreid << 2) | 0b00;// | i_SPKEY_BUTTONS(1 downto 0);
		break;
	case 0x11:
		port_253b_dat = (0b00000 << 3) | m_nr_11_video_timing;
		break;
	case 0x12:
		port_253b_dat = (0 << 7) | m_nr_12_layer2_active_bank;
		break;
	case 0x13:
		port_253b_dat = (0 << 7) | m_nr_13_layer2_shadow_bank;
		break;
	case 0x14:
		port_253b_dat = m_nr_14_global_transparent_rgb;
		break;
	case 0x15:
		port_253b_dat = (m_nr_15_lores_en << 7) | (m_nr_15_sprite_priority << 6) | (m_nr_15_sprite_border_clip_en << 5) | (m_nr_15_layer_priority << 2) | (m_nr_15_sprite_over_border_en << 1) | m_nr_15_sprite_en;
		break;
	case 0x16:
		port_253b_dat = m_nr_16_layer2_scrollx;
		break;
	case 0x17:
		port_253b_dat = m_nr_17_layer2_scrolly;
		break;
	case 0x18:
		switch (m_nr_18_layer2_clip_idx)
		{
		case 0b00:
			port_253b_dat = m_nr_18_layer2_clip_x1;
			break;
		case 0b01:
			port_253b_dat = m_nr_18_layer2_clip_x2;
			break;
		case 0b10:
			port_253b_dat = m_nr_18_layer2_clip_y1;
			break;
		default:
			port_253b_dat = m_nr_18_layer2_clip_y2;
		}
		break;
	case 0x19:
		switch (m_nr_19_sprite_clip_idx)
		{
		case 0b00:
			port_253b_dat = m_nr_19_sprite_clip_x1;
			break;
		case 0b01:
			port_253b_dat = m_nr_19_sprite_clip_x2;
			break;
		case 0b10:
			port_253b_dat = m_nr_19_sprite_clip_y1;
			break;
		default:
			port_253b_dat = m_nr_19_sprite_clip_y2;
		}
		break;
	case 0x1a:
		switch (m_nr_1a_ula_clip_idx)
		{
		case 0b00:
			port_253b_dat = m_nr_1a_ula_clip_x1;
			break;
		case 0b01:
			port_253b_dat = m_nr_1a_ula_clip_x2;
			break;
		case 0b10:
			port_253b_dat = m_nr_1a_ula_clip_y1;
			break;
		default:
			port_253b_dat = m_nr_1a_ula_clip_y2;
		}
		break;
	case 0x1b:
		switch (m_nr_1b_tm_clip_idx)
		{
		case 0b00:
			port_253b_dat = m_nr_1b_tm_clip_x1;
			break;
		case 0b01:
			port_253b_dat = m_nr_1b_tm_clip_x2;
			break;
		case 0b10:
			port_253b_dat = m_nr_1b_tm_clip_y1;
			break;
		default:
			port_253b_dat = m_nr_1b_tm_clip_y2;
		}
		break;
	case 0x1c:
		port_253b_dat = (m_nr_1b_tm_clip_idx << 6) | (m_nr_1a_ula_clip_idx << 4) | (m_nr_19_sprite_clip_idx << 2) | m_nr_18_layer2_clip_idx;
		break;
	case 0x1e:
		port_253b_dat = BIT((m_screen->vpos() - 40) % m_screen->height(), 8);
		break;
	case 0x1f:
		port_253b_dat = ((m_screen->vpos() - 40) % m_screen->height()) & 0xff;
		break;
	case 0x20:
		port_253b_dat = 0;//(BIT(im2_int_status, 0) <<) | (BIT(im2_int_status, 11) <<) | (0b00 <<) | BIT(im2_int_status, 3, 4);
		break;
	case 0x22:
		port_253b_dat = ((!m_pulse_int_n) << 7) | (0b0000 << 3) | (port_ff_interrupt_disable() << 2) | (m_nr_22_line_interrupt_en << 1) | BIT(m_nr_23_line_interrupt, 8);
		break;
	case 0x23:
		port_253b_dat = BIT(m_nr_23_line_interrupt, 0, 8);
		break;
	case 0x26:
		port_253b_dat = m_nr_26_ula_scrollx;
		break;
	case 0x27:
		port_253b_dat = m_nr_27_ula_scrolly;
		break;
	case 0x28:
		port_253b_dat = m_nr_stored_palette_value;
		break;
	case 0x2c:
		port_253b_dat = 0;//pi_audio_L(9 downto 2);
		//m_nr_2d_i2s_sample = pi_audio_L(1 downto 0);
		break;
	case 0x2d:
		port_253b_dat = (m_nr_2d_i2s_sample << 6) | 0b000000;
		break;
	case 0x2e:
		port_253b_dat = 0;//pi_audio_R(9 downto 2);
		//m_nr_2d_i2s_sample = pi_audio_R(1 downto 0);
		break;
	case 0x2f:
		port_253b_dat = (0b000000 << 2) | BIT(m_nr_30_tm_scrollx, 8, 2);
		break;
	case 0x30:
		port_253b_dat = BIT(m_nr_30_tm_scrollx, 0, 8);
		break;
	case 0x31:
		port_253b_dat = m_nr_31_tm_scrolly;
		break;
	case 0x32:
		port_253b_dat = m_nr_32_lores_scrollx;
		break;
	case 0x33:
		port_253b_dat = m_nr_33_lores_scrolly;
		break;
	case 0x34:
		port_253b_dat = m_sprites->mirror_num_r() & 0x7f;
		break;
	case 0x40:
		port_253b_dat = m_nr_palette_idx;
		break;
	case 0x41:
		port_253b_dat = BIT(nr_palette_dat(), 1, 8);
		break;
	case 0x42:
		port_253b_dat = m_nr_42_ulanext_format;
		break;
	case 0x43:
		port_253b_dat = (m_nr_43_palette_autoinc_disable << 7) | (m_nr_43_palette_write_select << 4) | (m_nr_43_active_sprite_palette << 3) | (m_nr_43_active_layer2_palette << 2) | (m_nr_43_active_ula_palette << 1) | m_nr_43_ulanext_en;
		break;
	case 0x44:
		port_253b_dat = (BIT(nr_palette_dat(), 9, 2) << 6) | (0b00000 << 1) | BIT(nr_palette_dat(), 0);
		break;
	case 0x4a:
		port_253b_dat = m_nr_4a_fallback_rgb;
		break;
	case 0x4b:
		port_253b_dat = m_nr_4b_sprite_transparent_index;
		break;
	case 0x4c:
		port_253b_dat = (0b0000 << 4) | m_nr_4c_tm_transparent_index;
		break;
	case 0x50: case 0x51: case 0x52: case 0x53:
	case 0x54: case 0x55: case 0x56: case 0x57:
		port_253b_dat = m_mmu[nr_register - 0x50];
		break;
	case 0x61:
		port_253b_dat = BIT(m_nr_copper_addr, 0, 8);
		break;
	case 0x62:
		port_253b_dat = (m_nr_62_copper_mode << 6) | (0b000 << 3) | BIT(m_nr_copper_addr, 8, 3);
		break;
	case 0x64:
		port_253b_dat = m_nr_64_copper_offset;
		break;
	case 0x68:
		port_253b_dat = ((!m_nr_68_ula_en) << 7) | (m_nr_68_blend_mode << 5) | (m_nr_68_cancel_extended_keys << 4) | (m_port_ff3b_ulap_en << 3) | (m_nr_68_ula_fine_scroll_x << 2) | (0 << 1) | m_nr_68_ula_stencil_mode;
		break;
	case 0x69:
		port_253b_dat = (m_port_123b_layer2_en << 7) | (port_7ffd_shadow() << 6) | BIT(m_port_ff_data, 0, 6);
		break;
	case 0x6a:
		port_253b_dat = (0b00 << 6) | (m_nr_6a_lores_radastan << 5) | (m_nr_6a_lores_radastan_xor << 4) | m_nr_6a_lores_palette_offset;
		break;
	case 0x6b:
		port_253b_dat = (m_nr_6b_tm_en << 7) | m_nr_6b_tm_control;
		break;
	case 0x6c:
		port_253b_dat = m_nr_6c_tm_default_attr;
		break;
	case 0x6e:
		port_253b_dat = (m_nr_6e_tilemap_base_7 << 7) | (0 << 6) | m_nr_6e_tilemap_base;
		break;
	case 0x6f:
		port_253b_dat = (m_nr_6f_tilemap_tiles_7 << 7) | (0 << 6) | m_nr_6f_tilemap_tiles;
		break;
	case 0x70:
		port_253b_dat = (0b00 << 6) | (m_nr_70_layer2_resolution << 4) | m_nr_70_layer2_palette_offset;
		break;
	case 0x71:
		port_253b_dat = (0b0000000 << 1) | m_nr_71_layer2_scrollx_msb;
		break;
	case 0x7f:
		port_253b_dat = m_nr_7f_user_register_0;
		break;
	case 0x80:
		port_253b_dat = m_nr_80_expbus;
		break;
	case 0x81:
		port_253b_dat = /*(i_BUS_ROMCS_n <<) |*/ (m_nr_81_expbus_ula_override << 6) | (m_nr_81_expbus_nmi_debounce_disable << 5) | (m_nr_81_expbus_clken << 4) | (0b00 << 2) | m_nr_81_expbus_speed;
		break;
	case 0x82:
		port_253b_dat = m_nr_82_internal_port_enable;
		break;
	case 0x83:
		port_253b_dat = m_nr_83_internal_port_enable;
		break;
	case 0x84:
		port_253b_dat = m_nr_84_internal_port_enable;
		break;
	case 0x85:
		port_253b_dat = (m_nr_85_internal_port_reset_type << 7) | (0b000 << 4) | m_nr_85_internal_port_enable;
		break;
	case 0x86:
		port_253b_dat = m_nr_86_bus_port_enable;
		break;
	case 0x87:
		port_253b_dat = m_nr_87_bus_port_enable;
		break;
	case 0x88:
		port_253b_dat = m_nr_88_bus_port_enable;
		break;
	case 0x89:
		port_253b_dat = (m_nr_89_bus_port_reset_type << 7) | (0b000 << 4) | m_nr_89_bus_port_enable;
		break;
	case 0x8a:
		port_253b_dat = (0b00 << 6) | m_nr_8a_bus_port_propagate;
		break;
	case 0x8c:
		port_253b_dat = m_nr_8c_altrom;
		break;
	case 0x8e:
		port_253b_dat = (BIT(m_port_dffd_data, 0) << 7) | (BIT(m_port_7ffd_data, 0, 3) << 4) | (1 << 3) | (BIT(m_port_1ffd_data, 0) << 2)
			| (BIT(m_port_1ffd_data, 2) << 1) | ((BIT(m_port_7ffd_data, 4) && !BIT(m_port_1ffd_data, 0)) || (BIT(m_port_1ffd_data, 1) && BIT(m_port_1ffd_data, 0)));;
		break;
	case 0x8f:
		port_253b_dat = (0b000000 << 2) | m_nr_8f_mapping_mode;
		break;
	case 0x90:
		port_253b_dat = m_nr_90_pi_gpio_o_en;
		break;
	case 0x91:
		port_253b_dat = m_nr_91_pi_gpio_o_en;
		break;
	case 0x92:
		port_253b_dat = m_nr_92_pi_gpio_o_en;
		break;
	case 0x93:
		port_253b_dat = (0b0000 << 4) | m_nr_93_pi_gpio_o_en;
		break;
	case 0x98:
		port_253b_dat = 0;//i_GPIO(7 downto 0);
		break;
	case 0x99:
		port_253b_dat = 0;//i_GPIO(15 downto 8);
		break;
	case 0x9a:
		port_253b_dat = 0;//i_GPIO(23 downto 16);
		break;
	case 0x9b:
		port_253b_dat = (0b0000 << 4);// | i_GPIO(27 downto 24);
		break;
	case 0xa0:
		port_253b_dat = (0b00 << 6) | (BIT(m_nr_a0_pi_peripheral_en, 3, 3) << 2) | (0b00 << 1) | BIT(m_nr_a0_pi_peripheral_en, 0);
		break;
	case 0xa2:
		port_253b_dat = (BIT(m_nr_a2_pi_i2s_ctl, 6, 2) << 6) | (0 << 5) | (BIT(m_nr_a2_pi_i2s_ctl, 2, 3) << 2) | (1 << 1) | BIT(m_nr_a2_pi_i2s_ctl, 0);
		break;
	case 0xa8:
		port_253b_dat = (0b0000000 << 7) | m_nr_a8_esp_gpio0_en;
		break;
	case 0xa9:
		port_253b_dat = (0b00000 << 3);// | (i_ESP_GPIO_20(2) <<) | (0 << 1) | i_ESP_GPIO_20(0);
		break;
	case 0xb0:
		port_253b_dat = 0;//(i_KBD_EXTENDED_KEYS(8) <<) | (i_KBD_EXTENDED_KEYS(9) <<) | (i_KBD_EXTENDED_KEYS(10) <<) | (i_KBD_EXTENDED_KEYS(11) <<) | (i_KBD_EXTENDED_KEYS(1) <<) | i_KBD_EXTENDED_KEYS(15 downto 13);
		break;
	case 0xb1:
		port_253b_dat = 0;//(i_KBD_EXTENDED_KEYS(12) <<) | (i_KBD_EXTENDED_KEYS(7 downto 2) <<) | i_KBD_EXTENDED_KEYS(0);
		break;
	case 0xb2:
		port_253b_dat = 0;//(i_JOY_RIGHT(10 downto 8) <<) | (i_JOY_RIGHT(11) <<) | (i_JOY_LEFT(10 downto 8) <<) | i_JOY_LEFT(11);
		break;
	case 0xb8:
		port_253b_dat = m_nr_b8_divmmc_ep_0;
		break;
	case 0xb9:
		port_253b_dat = m_nr_b9_divmmc_ep_valid_0;
		break;
	case 0xba:
		port_253b_dat = m_nr_ba_divmmc_ep_timing_0;
		break;
	case 0xbb:
		port_253b_dat = m_nr_bb_divmmc_ep_1;
		break;
	case 0xc0:
		port_253b_dat = (m_nr_c0_im2_vector << 5) | (0 << 4) | (m_nr_c0_stackless_nmi << 3) | /*(z80_im_mode << 1) |*/ m_nr_c0_int_mode_pulse_0_im2_1;
		break;
	case 0xc2:
		port_253b_dat = m_nr_c2_retn_address_lsb;
		break;
	case 0xc3:
		port_253b_dat = m_nr_c3_retn_address_msb;
		break;
	case 0xc4:
		{
			const u8 ula_int_en = (m_nr_22_line_interrupt_en << 1) | !port_ff_interrupt_disable();
			port_253b_dat = (m_nr_c4_int_en_0_expbus << 7) | (0b00000 << 2) | ula_int_en;
		}
		break;
	case 0xc5:
		port_253b_dat = 0;//ctc_int_en;
		break;
	case 0xc6:
		port_253b_dat = (0 << 7) | (m_nr_c6_int_en_2_654 << 4) | (0 << 3) | m_nr_c6_int_en_2_210;
		break;
	case 0xc8:
		port_253b_dat = (0b000000 << 2);// | (im2_int_status(0) << 1) | im2_int_status(11);
		break;
	case 0xc9:
		port_253b_dat = 0;//im2_int_status(10 downto 3);
		break;
	case 0xca:
		port_253b_dat = (0 << 7);// | (im2_int_status(13) <<  6) | (im2_int_status(2) << 5) | (im2_int_status(2) << 4) | (0 << 3) | (im2_int_status(12) << 2) | (im2_int_status(1) << 1) | im2_int_status(1);
		break;
	case 0xcc:
		port_253b_dat = (m_nr_cc_dma_int_en_0_7 << 7) | (0b00000 << 2) | m_nr_cc_dma_int_en_0_10;
		break;
	case 0xcd:
		port_253b_dat = m_nr_cd_dma_int_en_1;
		break;
	case 0xce:
		port_253b_dat = (0 << 7) | (m_nr_ce_dma_int_en_2_654 << 4) | (0 << 3) | m_nr_ce_dma_int_en_2_210;
		break;
	case 0xd8:
		port_253b_dat = (0b0000000 << 1) | m_nr_d8_io_trap_fdc_en;
		break;
	case 0xd9:
		port_253b_dat = m_nr_d9_iotrap_write;
		break;
	case 0xda:
		port_253b_dat = (0b000000 << 2) | m_nr_da_iotrap_cause;
		break;
	case 0xf0:
		port_253b_dat = m_nr_f0_xdev_cmd;
		break;
	case 0xf8:
		port_253b_dat = (0 << 7) | m_nr_f8_xadc_daddr;
		break;
	case 0xf9:
		port_253b_dat = m_nr_f9_xadc_d0;
		break;
	case 0xfa:
		port_253b_dat = m_nr_fa_xadc_d1;
		break;
	default:
		port_253b_dat = 0x00;
		if (!machine().side_effects_disabled())
			LOGWARN("rR: %X -> %x\n", nr_register, port_253b_dat);
	}

	return port_253b_dat;
}

void specnext_state::reg_w(offs_t nr_wr_reg, u8 nr_wr_dat)
{
	switch (nr_wr_reg)
	{
	case 0x02:
		nr_02_w(nr_wr_dat);
		break;
	case 0x03:
		m_bootrom_en = 0;

		if (BIT(nr_wr_dat, 7) == 1 && m_nr_03_user_dt_lock == 0 && BIT(nr_wr_dat, 3) == 0)
		{
			switch (BIT(nr_wr_dat, 4, 3))
			{
			case 0b000:
				m_nr_03_machine_timing = 0b001;
				break;
			case 0b001:
				m_nr_03_machine_timing = 0b001;
				break;
			case 0b010:
				m_nr_03_machine_timing = 0b010;
				break;
			case 0b011:
				m_nr_03_machine_timing = 0b011;
				break;
			case 0b100:
				m_nr_03_machine_timing = 0b100;
				break;
			default:
				m_nr_03_machine_timing = 0b011;
			}
		}

		m_nr_03_user_dt_lock = m_nr_03_user_dt_lock ^ BIT(nr_wr_dat, 3);

		if (m_nr_03_config_mode == 1)
		{
			switch (BIT(nr_wr_dat, 0, 3))
			{
			case 0b001:
				m_nr_03_machine_type = 0b001;
				break;
			case 0b010:
				m_nr_03_machine_type = 0b010;
				break;
			case 0b011:
				m_nr_03_machine_type = 0b011;
				break;
			case 0b100:
				m_nr_03_machine_type = 0b100;
				break;
			}
		}

		if (BIT(nr_wr_dat, 0, 3) == 0b111)
			m_nr_03_config_mode = 1;
		else if (BIT(nr_wr_dat, 0, 3) != 0b000)
			m_nr_03_config_mode = 0;

		break;
	case 0x04:
		m_nr_04_romram_bank = BIT(nr_wr_dat, 0, 7);
		bank_update(0, 2);
		break;
	case 0x05:
		m_nr_05_joy0 = (BIT(nr_wr_dat, 3) << 2) | BIT(nr_wr_dat, 6, 2);
		m_nr_05_joy1 = (BIT(nr_wr_dat, 1) << 2) | BIT(nr_wr_dat, 4, 2);
		break;
	case 0x06:
		m_nr_06_hotkey_cpu_speed_en = BIT(nr_wr_dat, 7);
		m_nr_06_internal_speaker_beep = BIT(nr_wr_dat, 6);
		m_nr_06_hotkey_5060_en = BIT(nr_wr_dat, 5);
		m_nr_06_button_drive_nmi_en = BIT(nr_wr_dat, 4);
		m_nr_06_button_m1_nmi_en = BIT(nr_wr_dat, 3);
		if (m_nr_03_config_mode == 1)
			m_nr_06_ps2_mode = BIT(nr_wr_dat, 2);
		m_nr_06_psg_mode = BIT(nr_wr_dat, 0, 2);
		break;
	case 0x07:
		nr_07_cpu_speed_w(BIT(nr_wr_dat, 0, 2));
		break;
	case 0x08:
		m_nr_08_contention_disable = BIT(nr_wr_dat, 6);
		m_eff_nr_08_contention_disable = m_nr_08_contention_disable;
		m_nr_08_psg_stereo_mode = BIT(nr_wr_dat, 5);
		m_nr_08_internal_speaker_en = BIT(nr_wr_dat, 4);
		m_nr_08_dac_en = BIT(nr_wr_dat, 3);
		m_nr_08_port_ff_rd_en = BIT(nr_wr_dat, 2);
		m_nr_08_psg_turbosound_en = BIT(nr_wr_dat, 1);
		m_nr_08_keyboard_issue2 = BIT(nr_wr_dat, 0);

		if (BIT(nr_wr_dat, 7))
			port_7ffd_reg_w(m_port_7ffd_data &= ~0x20);
		break;
	case 0x09:
		m_nr_09_psg_mono = BIT(nr_wr_dat, 5, 3);
		m_nr_09_sprite_tie = BIT(nr_wr_dat, 4);
		m_sprites->mirror_tie_w(m_nr_09_sprite_tie);
		m_nr_09_hdmi_audio_en = not BIT(nr_wr_dat, 2);

		if (BIT(nr_wr_dat, 3))
			port_e3_reg_w(m_port_e3_reg & ~0x40);
		m_nr_09_scanlines = BIT(nr_wr_dat, 0, 2);
		break;
	case 0x0a:
		if (m_nr_03_config_mode == 1)
			nr_0a_mf_type_w(BIT(nr_wr_dat, 6, 2));
		m_nr_0a_divmmc_automap_en = BIT(nr_wr_dat, 4);
		m_nr_0a_mouse_button_reverse = BIT(nr_wr_dat, 3);
		m_nr_0a_mouse_dpi = BIT(nr_wr_dat, 0, 2);
		break;
	case 0x0b:
		m_nr_0b_joy_iomode_en = BIT(nr_wr_dat, 7);
		m_nr_0b_joy_iomode = BIT(nr_wr_dat, 4, 2);
		m_nr_0b_joy_iomode_0 = BIT(nr_wr_dat, 0);
		break;
	case 0x10:
		m_nr_10_flashboot = BIT(nr_wr_dat, 7);
		if (m_nr_03_config_mode)
		{
			if (g_board_issue() != 2)
			{
				m_nr_10_coreid = BIT(nr_wr_dat, 0, 5);
			}
			else if (g_board_issue() == 2)
			{
				if (BIT(~nr_wr_dat, 4) && BIT(nr_wr_dat, 0, 4) != 0b1111)
					m_nr_10_coreid = BIT(nr_wr_dat, 0, 4);
			}
		}
		break;
	case 0x11:
		if (m_nr_03_config_mode == 1)
		{
			if (BIT(nr_wr_dat, 0, 3) == 0b111)
				m_nr_11_video_timing = 0b000;
			else if (G_VIDEO_INC == 0b10)
				m_nr_11_video_timing = (0b00 << 1) | BIT(nr_wr_dat, 0);
			else
				m_nr_11_video_timing = BIT(nr_wr_dat, 0, 3);
		}
		break;
	case 0x12:
		nr_12_layer2_active_bank_w(nr_wr_dat);
		break;
	case 0x13:
		nr_13_layer2_shadow_bank_w(nr_wr_dat);
		break;
	case 0x14:
		nr_14_global_transparent_rgb_w(nr_wr_dat);
		break;
	case 0x15:
		m_screen->update_now();
		m_nr_15_lores_en = BIT(nr_wr_dat, 7);
		nr_15_sprite_priority_w(BIT(nr_wr_dat, 6));
		nr_15_sprite_border_clip_en_w(BIT(nr_wr_dat, 5));
		m_nr_15_layer_priority = BIT(nr_wr_dat, 2, 3);
		nr_15_sprite_over_border_en_w(BIT(nr_wr_dat, 1));
		m_nr_15_sprite_en = BIT(nr_wr_dat, 0);
		break;
	case 0x16:
		m_screen->update_now();
		nr_16_layer2_scrollx_w(nr_wr_dat);
		break;
	case 0x17:
		m_screen->update_now();
		nr_17_layer2_scrolly_w(nr_wr_dat);
		break;
	case 0x18:
		switch (m_nr_18_layer2_clip_idx)
		{
		case 0b00:
			nr_18_layer2_clip_x1_w(nr_wr_dat);
			break;
		case 0b01:
			nr_18_layer2_clip_x2_w(nr_wr_dat);
			break;
		case 0b10:
			nr_18_layer2_clip_y1_w(nr_wr_dat);
			break;
		default:
			nr_18_layer2_clip_y2_w(nr_wr_dat);
		}
		++m_nr_18_layer2_clip_idx &= 0x03;
		break;
	case 0x19:
		switch (m_nr_19_sprite_clip_idx)
		{
		case 0b00:
			nr_19_sprite_clip_x1_w(nr_wr_dat);
			break;
		case 0b01:
			nr_19_sprite_clip_x2_w(nr_wr_dat);
			break;
		case 0b10:
			nr_19_sprite_clip_y1_w(nr_wr_dat);
			break;
		default:
			nr_19_sprite_clip_y2_w(nr_wr_dat);
		}
		++m_nr_19_sprite_clip_idx &= 0x03;
		break;
	case 0x1a:
		switch (m_nr_1a_ula_clip_idx)
		{
		case 0b00:
			nr_1a_ula_clip_x1_w(nr_wr_dat);
			break;
		case 0b01:
			nr_1a_ula_clip_x2_w(nr_wr_dat);
			break;
		case 0b10:
			nr_1a_ula_clip_y1_w(nr_wr_dat);
			break;
		default:
			nr_1a_ula_clip_y2_w(nr_wr_dat);
		}
		++m_nr_1a_ula_clip_idx &= 0x03;
		break;
	case 0x1b:
		switch (m_nr_1b_tm_clip_idx)
		{
		case 0b00:
			nr_1b_tm_clip_x1_w(nr_wr_dat);
			break;
		case 0b01:
			nr_1b_tm_clip_x2_w(nr_wr_dat);
			break;
		case 0b10:
			nr_1b_tm_clip_y1_w(nr_wr_dat);
			break;
		default:
			nr_1b_tm_clip_y2_w(nr_wr_dat);
		}
		++m_nr_1b_tm_clip_idx &= 0x03;
		break;
	case 0x1c:
		if (BIT(nr_wr_dat, 0) == 1)
			m_nr_18_layer2_clip_idx = 0b00;
		if (BIT(nr_wr_dat, 1) == 1)
			m_nr_19_sprite_clip_idx = 0b00;
		if (BIT(nr_wr_dat, 2) == 1)
			m_nr_1a_ula_clip_idx = 0b00;
		if (BIT(nr_wr_dat, 3) == 1)
			m_nr_1b_tm_clip_idx = 0b00;
		break;
	case 0x22:
		m_nr_22_line_interrupt_en = BIT(nr_wr_dat, 1);
		m_nr_23_line_interrupt = (m_nr_23_line_interrupt & ~0x0100) | (BIT(nr_wr_dat, 0) << 8);
		line_irq_adjust();
		m_port_ff_data = (m_port_ff_data & 0xbf) | (BIT(nr_wr_dat, 1) << 6);
		break;
	case 0x23:
		m_nr_23_line_interrupt = (m_nr_23_line_interrupt & ~0x00ff) | nr_wr_dat;
		line_irq_adjust();
		break;
	case 0x26:
		m_screen->update_now();
		nr_26_ula_scrollx_w(nr_wr_dat);
		break;
	case 0x27:
		m_screen->update_now();
		nr_27_ula_scrolly_w(nr_wr_dat);
		break;
	case 0x28:
		//nr_keymap_sel <= nr_wr_dat(7);
		//nr_keymap_addr(8) <= nr_wr_dat(0);
		break;
	case 0x29:
		//nr_keymap_addr(7 downto 0) <= nr_wr_dat;
		break;
	case 0x2a:
		break;
	case 0x2b:
		// nr_keymap_addr <= nr_keymap_addr + 1;
		break;
	case 0x2c:
		m_dac[1]->data_w(nr_wr_dat);
		break;
	case 0x2d:
		m_dac[0]->data_w(nr_wr_dat);
		m_dac[3]->data_w(nr_wr_dat);
		break;
	case 0x2e:
		m_dac[2]->data_w(nr_wr_dat);
		break;
	case 0x2f:
		m_screen->update_now();
		nr_30_tm_scrollx_w((m_nr_30_tm_scrollx & ~0x0300) | (BIT(nr_wr_dat, 0, 2) << 8));
		break;
	case 0x30:
		m_screen->update_now();
		nr_30_tm_scrollx_w((m_nr_30_tm_scrollx & ~0x00ff) | nr_wr_dat);
		break;
	case 0x31:
		m_screen->update_now();
		nr_31_tm_scrolly_w(nr_wr_dat);
		break;
	case 0x32:
		m_screen->update_now();
		nr_32_lores_scrollx_w(nr_wr_dat);
		break;
	case 0x33:
		m_screen->update_now();
		nr_33_lores_scrolly_w(nr_wr_dat);
		break;
	case 0x34:
		m_sprites->mirror_data_w(nr_wr_dat);
		break;
	case 0x35: case 0x36:  case 0x37: case 0x38: case 0x39:
	case 0x75: case 0x76:  case 0x77: case 0x78: case 0x79:
		m_sprites->mirror_inc_w(BIT(nr_wr_reg, 6));
		m_sprites->mirror_index_w((nr_wr_reg & 0x3f) - 0x35);
		m_sprites->mirror_data_w(nr_wr_dat);
		break;
	case 0x40:
		m_nr_palette_idx = nr_wr_dat;
		m_nr_palette_sub_idx = 0;
		break;
	case 0x41:
		palette_val_w(0b00, (nr_wr_dat << 1) | BIT(nr_wr_dat, 1) | BIT(nr_wr_dat, 0));
		if (m_nr_43_palette_autoinc_disable == 0)
			++m_nr_palette_idx;
		m_nr_palette_sub_idx = 0;
		break;
	case 0x42:
		nr_42_ulanext_format_w(nr_wr_dat);
		break;
	case 0x43:
		m_nr_43_palette_autoinc_disable = BIT(nr_wr_dat, 7);
		m_nr_43_palette_write_select = BIT(nr_wr_dat, 4, 3);
		nr_43_active_sprite_palette_w(BIT(nr_wr_dat, 3));
		nr_43_active_layer2_palette_w(BIT(nr_wr_dat, 2));
		nr_43_active_ula_palette_w(BIT(nr_wr_dat, 1));
		nr_43_ulanext_en_w(BIT(nr_wr_dat, 0));
		m_nr_palette_sub_idx = 0;
		break;
	case 0x44:
		if (m_nr_palette_sub_idx == 0)
			m_nr_stored_palette_value = nr_wr_dat;
		else
		{
			palette_val_w(BIT(nr_wr_dat, 6, 2), (m_nr_stored_palette_value << 1) | BIT(nr_wr_dat, 0));
			if (m_nr_43_palette_autoinc_disable == 0)
				++m_nr_palette_idx;
		}
		m_nr_palette_sub_idx = !m_nr_palette_sub_idx;
		break;
	case 0x4a:
		{
			m_nr_4a_fallback_rgb = nr_wr_dat;
			rgb_t fb = rgbexpand<3,3,3>((m_nr_4a_fallback_rgb << 1) | BIT(m_nr_4a_fallback_rgb, 1) | BIT(m_nr_4a_fallback_rgb, 0), 6, 3, 0);
			m_palette->set_pen_color(UTM_FALLBACK_PEN, fb);
		}
		break;
	case 0x4b:
		nr_4b_sprite_transparent_index_w(nr_wr_dat);
		break;
	case 0x4c:
		nr_4c_tm_transparent_index_w(BIT(nr_wr_dat, 0, 4));
		break;
	case 0x50: case 0x51: case 0x52: case 0x53:
	case 0x54: case 0x55: case 0x56: case 0x57:
		mmu_w(nr_wr_reg - 0x50, nr_wr_dat);
		break;
	case 0x60:
	case 0x63:
		{
			const bool nr_copper_write_8 = nr_wr_reg == 0x60;
			if (BIT(m_nr_copper_addr, 0) == 0)
			{
				if (nr_copper_write_8)
					m_copper->data_w(m_nr_copper_addr, nr_wr_dat); // msb
			}
			else
			{
				if (!nr_copper_write_8)
					m_copper->data_w(m_nr_copper_addr & ~1, m_nr_copper_data_stored); // msb
				m_copper->data_w(m_nr_copper_addr, nr_wr_dat); // lsb
			}

			if (BIT(m_nr_copper_addr, 0) == 0)
				m_nr_copper_data_stored = nr_wr_dat;
			++m_nr_copper_addr %= 0x800;
		}
		break;
	case 0x61:
		m_nr_copper_addr = (m_nr_copper_addr & ~0x00ff) | nr_wr_dat;
		break;
	case 0x62:
		nr_62_copper_mode_w(BIT(nr_wr_dat, 6, 2));
		m_nr_copper_addr = (m_nr_copper_addr & ~0x0700) | (BIT(nr_wr_dat, 0, 3) << 8);
		break;
	case 0x64:
		m_nr_64_copper_offset = nr_wr_dat;
		break;
	case 0x68:
		m_screen->update_now();
		m_nr_68_ula_en = not BIT(nr_wr_dat, 7);
		m_nr_68_blend_mode = BIT(nr_wr_dat, 5, 2);
		m_nr_68_cancel_extended_keys = BIT(nr_wr_dat, 4);
		port_ff3b_ulap_en_w(BIT(nr_wr_dat, 3));
		nr_68_ula_fine_scroll_x_w(BIT(nr_wr_dat, 2));
		m_nr_68_ula_stencil_mode = BIT(nr_wr_dat, 0);
		break;
	case 0x69:
		m_port_ff_data = (m_port_ff_data & 0xc0) | (nr_wr_dat & 0x3f);
		port_7ffd_reg_w((m_port_7ffd_data & ~0x08) | (BIT(nr_wr_dat, 6) << 3));
		port_123b_layer2_en_w(BIT(nr_wr_dat, 7));
		break;
	case 0x6a:
		nr_6a_lores_radastan_w(BIT(nr_wr_dat, 5));
		nr_6a_lores_radastan_xor_w(BIT(nr_wr_dat, 4));
		nr_6a_lores_palette_offset_w(BIT(nr_wr_dat, 0, 4));
		break;
	case 0x6b:
		m_screen->update_now();
		m_nr_6b_tm_en = BIT(nr_wr_dat, 7);
		nr_6b_tm_control_w(BIT(nr_wr_dat, 0, 7));
		break;
	case 0x6c:
		nr_6c_tm_default_attr_w(nr_wr_dat);
		break;
	case 0x6e:
		nr_6e_tilemap_base_w(BIT(nr_wr_dat, 7), BIT(nr_wr_dat, 0, 6));
		break;
	case 0x6f:
		nr_6f_tilemap_tiles_w(BIT(nr_wr_dat, 7), BIT(nr_wr_dat, 0, 6));
		break;
	case 0x70:
		nr_70_layer2_resolution_w(BIT(nr_wr_dat, 4, 2));
		nr_70_layer2_palette_offset_w(BIT(nr_wr_dat, 0, 4));
		break;
	case 0x71:
		nr_71_layer2_scrollx_msb_w(BIT(nr_wr_dat, 0));
		break;
	case 0x7f:
		m_nr_7f_user_register_0 = nr_wr_dat;
		break;
	case 0x80:
		m_nr_80_expbus = nr_wr_dat;
		break;
	case 0x81:
		m_nr_81_expbus_ula_override = BIT(nr_wr_dat, 6);
		m_nr_81_expbus_nmi_debounce_disable = BIT(nr_wr_dat, 5);
		m_nr_81_expbus_clken = BIT(nr_wr_dat, 4);
		m_nr_81_expbus_speed = 0b00;
		break;
	case 0x82:
		m_nr_82_internal_port_enable = nr_wr_dat;
		break;
	case 0x83:
		m_nr_83_internal_port_enable = nr_wr_dat;
		break;
	case 0x84:
		m_nr_84_internal_port_enable = nr_wr_dat;
		break;
	case 0x85:
		m_nr_85_internal_port_enable = BIT(nr_wr_dat, 0, 4);
		m_nr_85_internal_port_reset_type = BIT(nr_wr_dat, 7);
		break;
	case 0x86:
		m_nr_86_bus_port_enable = nr_wr_dat;
		break;
	case 0x87:
		m_nr_87_bus_port_enable = nr_wr_dat;
		break;
	case 0x88:
		m_nr_88_bus_port_enable = nr_wr_dat;
		break;
	case 0x89:
		m_nr_89_bus_port_enable = BIT(nr_wr_dat, 0, 4);
		m_nr_89_bus_port_reset_type = BIT(nr_wr_dat, 7);
		break;
	case 0x8a:
		m_nr_8a_bus_port_propagate = BIT(nr_wr_dat, 0, 6);
		break;
	case 0x8c:
		m_nr_8c_altrom = nr_wr_dat;
		bank_update(0, 2);
		break;
	case 0x8e:
		LOGMEM("8e (%x): 1f=%x 7f=%x df=%x\n", nr_wr_dat, m_port_1ffd_data, m_port_7ffd_data, m_port_dffd_data);
		if (BIT(nr_wr_dat, 3))
		{
			if (!nr_8f_mapping_mode_profi())
				m_port_dffd_data &= ~0x08;
			m_port_dffd_data = (m_port_dffd_data & ~0x07) | BIT(nr_wr_dat, 7);
			port_7ffd_reg_w((m_port_7ffd_data & ~0x07) | BIT(nr_wr_dat, 4, 3));
		}

		if (BIT(~nr_wr_dat, 2))
			port_7ffd_reg_w((m_port_7ffd_data & ~0x10) | ((nr_wr_dat & 1) << 4));

		m_port_1ffd_data = (m_port_1ffd_data & ~0x04) | (BIT(nr_wr_dat, 1) << 2);
		m_port_1ffd_data = (m_port_1ffd_data & ~0x02) | (BIT(nr_wr_dat, 0) << 1);
		m_port_1ffd_data = (m_port_1ffd_data & ~0x01) | BIT(nr_wr_dat, 2);

		memory_change(0x8e, nr_wr_dat);
		LOGMEM("8e: 1f=%x 7f=%x df=%x\n", m_port_1ffd_data, m_port_7ffd_data, m_port_dffd_data);
		break;
	case 0x8f:
		m_nr_8f_mapping_mode = BIT(nr_wr_dat, 0, 2);
		memory_change(0x8f, nr_wr_dat);
		break;
	case 0x90:
		m_nr_90_pi_gpio_o_en = (BIT(nr_wr_dat, 2, 6) << 2) | 0b00;
		break;
	case 0x91:
		m_nr_91_pi_gpio_o_en = nr_wr_dat;
		break;
	case 0x92:
		m_nr_92_pi_gpio_o_en = nr_wr_dat;
		break;
	case 0x93:
		m_nr_93_pi_gpio_o_en = BIT(nr_wr_dat, 0, 4);
		break;
	case 0x98:
		m_nr_98_pi_gpio_o = nr_wr_dat;
		break;
	case 0x99:
		m_nr_99_pi_gpio_o = nr_wr_dat;
		break;
	case 0x9a:
		m_nr_9a_pi_gpio_o = nr_wr_dat;
		break;
	case 0x9b:
		m_nr_9b_pi_gpio_o = BIT(nr_wr_dat, 0, 4);
		break;
	case 0xa0:
		m_nr_a0_pi_peripheral_en = nr_wr_dat;
		break;
	case 0xa2:
		m_nr_a2_pi_i2s_ctl = nr_wr_dat;
		break;
	case 0xa8:
		m_nr_a8_esp_gpio0_en = BIT(nr_wr_dat, 0);
		break;
	case 0xa9:
		m_nr_a9_esp_gpio0 = BIT(nr_wr_dat, 0);
		break;
	case 0xb8:
		m_nr_b8_divmmc_ep_0 = nr_wr_dat;
		break;
	case 0xb9:
		m_nr_b9_divmmc_ep_valid_0 = nr_wr_dat;
		break;
	case 0xba:
		m_nr_ba_divmmc_ep_timing_0 = nr_wr_dat;
		break;
	case 0xbb:
		m_nr_bb_divmmc_ep_1 = nr_wr_dat;
		break;
	case 0xc0:
		nr_c0_im2_vector_w(BIT(nr_wr_dat, 5, 3));
		m_nr_c0_stackless_nmi = BIT(nr_wr_dat, 3);
		m_maincpu->nmi_stackless_w(m_nr_c0_stackless_nmi);
		m_nr_c0_int_mode_pulse_0_im2_1 = BIT(nr_wr_dat, 0);
		break;
	case 0xc2:
		m_nr_c2_retn_address_lsb = nr_wr_dat;
		break;
	case 0xc3:
		m_nr_c3_retn_address_msb = nr_wr_dat;
		break;
	case 0xc4:
		m_nr_c4_int_en_0_expbus = BIT(nr_wr_dat, 7);
		m_nr_22_line_interrupt_en = BIT(nr_wr_dat, 1);
		m_port_ff_data = (m_port_ff_data & 0xbf) | (BIT(~nr_wr_dat, 0) << 6);
		break;
	case 0xc5:
		{
			u8 active = BIT(nr_wr_dat, 0, 4);
			for (auto ch = 0; ch < 4; ++ch, active >>= 1)
				m_ctc->write(ch, 1 | ((active & 1) ? 0 : 2)); // for inactive: CONTROL + RESET
		}
		break;
	case 0xc6:
		m_nr_c6_int_en_2_654 = (BIT(nr_wr_dat, 6) << 2) | (BIT(nr_wr_dat, 5) << 1) | BIT(nr_wr_dat, 4);
		m_nr_c6_int_en_2_210 = (BIT(nr_wr_dat, 2) << 2) | (BIT(nr_wr_dat, 1) << 1) | BIT(nr_wr_dat, 0);
		break;
	case 0xcc:
		m_nr_cc_dma_int_en_0_7 = BIT(nr_wr_dat, 7);
		m_nr_cc_dma_int_en_0_10 = BIT(nr_wr_dat, 0, 2);
		break;
	case 0xcd:
		m_nr_cd_dma_int_en_1 = nr_wr_dat;
		break;
	case 0xce:
		m_nr_ce_dma_int_en_2_654 = BIT(nr_wr_dat, 4, 3);
		m_nr_ce_dma_int_en_2_210 = BIT(nr_wr_dat, 0, 3);
		break;
	case 0xd8:
		m_nr_d8_io_trap_fdc_en = BIT(nr_wr_dat, 0);
		break;
	case 0xd9:
		m_nr_d9_iotrap_write = nr_wr_dat;
		break;
	case 0xff:
		popmessage("Debug: #%02X\n", nr_wr_dat); // LED
		break;
	default:
		LOGWARN("wR: %X <- %x\n", nr_wr_reg, nr_wr_dat);
		break;
	}

	m_sprites->mirror_index_w(0b111);
	m_sprites->mirror_inc_w(0);
}

void specnext_state::nr_02_w(u8 nr_wr_dat)
{
	m_nr_02_bus_reset = BIT(nr_wr_dat, 7);
	if (BIT(~nr_wr_dat, 4))
		m_nr_da_iotrap_cause = 0;

	m_nr_02_generate_mf_nmi = BIT(nr_wr_dat, 3);
	do_mf_nmi();

	if (BIT(nr_wr_dat, 2))
	{
		if (m_nr_06_button_drive_nmi_en)
		{
			m_nr_02_generate_divmmc_nmi = 1;
			m_maincpu->nmi();
		}
	}
	else
	{
		m_nr_02_generate_divmmc_nmi = 0;
	}

	if (BIT(nr_wr_dat, 1)) // hard reset
	{
		m_nr_02_hard_reset = 1;
		machine().schedule_soft_reset();
	}
	else if (BIT(nr_wr_dat, 0)) // soft reset
	{
		m_nr_02_reset_type = (BIT(m_nr_02_reset_type, 2) << 1) | BIT(m_nr_02_reset_type, 1) | BIT(m_nr_02_reset_type, 0);
		machine().schedule_soft_reset();
	}
}

void specnext_state::nr_07_cpu_speed_w(u8 data)
{
	m_nr_07_cpu_speed = data & 3;
	m_maincpu->set_clock_scale(1 << m_nr_07_cpu_speed);
	m_dma->set_clock_scale(1 << m_nr_07_cpu_speed);
}

void specnext_state::nr_14_global_transparent_rgb_w(u8 data)
{
	m_nr_14_global_transparent_rgb = data;
	m_global_transparent = (m_nr_14_global_transparent_rgb << 1) | BIT(m_nr_14_global_transparent_rgb, 1) | BIT(m_nr_14_global_transparent_rgb, 0);
	m_ula->set_global_transparent(data);
	m_lores->set_global_transparent(data);
	m_layer2->set_global_transparent(data);
}

void specnext_state::nr_1a_ula_clip_y2_w(u8 data)
{
	m_nr_1a_ula_clip_y2 = data;
	const u8 ula_clip_y2_0 = ((m_nr_1a_ula_clip_y2 & 0xc0) == 0xc0) ? 0xbf : m_nr_1a_ula_clip_y2;
	m_ula->ula_clip_y2_w(ula_clip_y2_0);
	m_lores->clip_y2_w(ula_clip_y2_0);
}

static const z80_daisy_config z80_daisy_chain[] =
{
	{ "dma" },
	{ "ctc" },
	{ nullptr }
};

TIMER_CALLBACK_MEMBER(specnext_state::irq_off)
{
	spectrum_state::irq_off(param);
	m_irq_mask = 0;
}

TIMER_CALLBACK_MEMBER(specnext_state::irq_on)
{
	spectrum_state::irq_on(param);
	m_irq_mask |= 1 << 11;
	m_irq_off_timer->adjust(m_maincpu->clocks_to_attotime(32));
}

TIMER_CALLBACK_MEMBER(specnext_state::line_irq_on)
{
	m_screen->update_now();
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	m_irq_mask |= 1 << 0;
	m_irq_off_timer->adjust(m_maincpu->clocks_to_attotime(32));
}

INTERRUPT_GEN_MEMBER(specnext_state::specnext_interrupt)
{
	m_tiles->control_w(m_nr_6b_tm_control); // TODO (1): Santa's Pressie, The Next War

	line_irq_adjust();
	if (!port_ff_interrupt_disable())
	{
		m_irq_on_timer->adjust(m_screen->time_until_pos(SCR_256x192.top(), SCR_256x192.left())
			- attotime::from_ticks(14365, m_maincpu->unscaled_clock()));
	}
}

void specnext_state::line_irq_adjust()
{
	if (m_nr_22_line_interrupt_en) {
		u16 line = m_nr_64_copper_offset + m_nr_23_line_interrupt;
		if (line < m_screen->width())
			m_irq_line_timer->adjust(m_screen->time_until_pos(
				(SCR_256x192.top() - 1 + line) % m_screen->height(),
				SCR_256x192.right() + 2));
		else
			m_irq_line_timer->reset();
	}
}

IRQ_CALLBACK_MEMBER(specnext_state::irq_callback)
{
	if (!m_nr_02_bus_reset)
	{
		return 0xff;
	}
	else
	{
		u8 vector = 11;
		if (m_irq_mask)
		{
			vector = 0;
			u16 i = 1;
			for (; ~m_irq_mask & i; ++vector, i <<= 1);
			m_irq_mask &= ~i;

		}
		return (m_nr_c0_im2_vector << 5) | (vector << 2);
	}
}

INPUT_CHANGED_MEMBER(specnext_state::on_mf_nmi)
{
	m_nr_02_generate_mf_nmi = newval & 1;
	do_mf_nmi();
	m_nr_02_generate_mf_nmi = 0;
}

INPUT_CHANGED_MEMBER(specnext_state::on_divmmc_nmi)
{
	m_nr_02_generate_divmmc_nmi = newval & 1;
	if (m_nr_06_button_drive_nmi_en && m_nr_02_generate_divmmc_nmi)
		m_maincpu->nmi();
}

void specnext_state::do_mf_nmi()
{
	if (m_nr_06_button_m1_nmi_en && m_nr_02_generate_mf_nmi)
	{
		m_maincpu->nmi();
		m_mf->button_w(1);
		m_mf->clock_w();
		m_mf->button_w(0);
	}
}

void specnext_state::leave_nmi(int status)
{
	m_mf->cpu_retn_seen_w(1);
	m_mf->clock_w();

	m_mf->cpu_retn_seen_w(0);
	m_mf->clock_w();
	bank_update(0, 2);
}

u8 specnext_state::do_m1(offs_t offset)
{
	u8 bank = offset >> 13;
	// pre M1
	m_divmmc->cpu_mreq_n_w(1);
	m_divmmc->clock_w();

	// M1
	m_divmmc->cpu_mreq_n_w(0);
	m_divmmc->cpu_m1_n_w(0);
	m_divmmc->clock_w();
	if (bank < 2)
		bank_update(bank);
	const u8 data = m_program.read_byte(offset);

	// after M1
	m_divmmc->automap_instant_on_w(0);
	m_divmmc->automap_delayed_on_w(0);
	m_divmmc->automap_delayed_off_w(0);
	m_divmmc->automap_rom3_instant_on_w(0);
	m_divmmc->automap_rom3_delayed_on_w(0);
	m_divmmc->automap_nmi_instant_on_w(0);
	m_divmmc->automap_nmi_delayed_on_w(0);

	m_divmmc->cpu_m1_n_w(1);
	m_divmmc->clock_w();
	bank_update(0, 2);

	m_divmmc_delayed_check = 1;
	return data;
}

void specnext_state::map_fetch(address_map &map)
{
	map(0x0000, 0xffff).lr8(NAME([this](offs_t offset)
	{
		if (m_divmmc_delayed_check && !machine().side_effects_disabled())
		{
			/* Happens after RW cycles (before next M1 fetch).
			Fell like side effects check must be ignored here,
			because doesn't matter who reset this lines and such
			approach gives better experience in debugger UI. */
			do_m1(offset);
			m_divmmc_delayed_check = 0;
		}

		return m_program.read_byte(offset);
	}));
	map(0x0000, 0x0000).select(0x0038).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			const u8 b = offset >> 3;
			const bool divmmc_rst_ep = BIT(m_nr_b8_divmmc_ep_0, b);
			const bool divmmc_rst_ep_valid = BIT(m_nr_b9_divmmc_ep_valid_0, b);
			const bool divmmc_rst_ep_timing = BIT(m_nr_ba_divmmc_ep_timing_0, b);

			m_divmmc->automap_instant_on_w(divmmc_rst_ep && divmmc_rst_ep_valid && divmmc_rst_ep_timing);
			m_divmmc->automap_delayed_on_w(divmmc_rst_ep && divmmc_rst_ep_valid && !divmmc_rst_ep_timing);
			m_divmmc->automap_rom3_instant_on_w(divmmc_rst_ep && !divmmc_rst_ep_valid && divmmc_rst_ep_timing);
			m_divmmc->automap_rom3_delayed_on_w(divmmc_rst_ep && !divmmc_rst_ep_valid && !divmmc_rst_ep_timing);

			return do_m1(offset);
		}
		else
			return m_program.read_byte(offset);
	}));
	map(0x1ff8, 0x1fff).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			m_divmmc->automap_delayed_off_w(BIT(m_nr_bb_divmmc_ep_1, 6));
			return do_m1(0x1ff8 + offset);
		}
		else
			return m_program.read_byte(0x1ff8 + offset);
	}));
	map(0x3d00, 0x3dff).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			m_divmmc->automap_rom3_instant_on_w(BIT(m_nr_bb_divmmc_ep_1, 7));
			return do_m1(0x3d00 + offset);
		}
		else
			return m_program.read_byte(0x3d00 + offset);
	}));
	map(0x04c6, 0x04c6).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			m_divmmc->automap_rom3_delayed_on_w(BIT(m_nr_bb_divmmc_ep_1, 2));
			return do_m1(0x04c6 + offset);
		}
		else
			return m_program.read_byte(0x04c6 + offset);
	}));
	map(0x0562, 0x0562).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			m_divmmc->automap_rom3_delayed_on_w(BIT(m_nr_bb_divmmc_ep_1, 3));
			return do_m1(0x0562 + offset);
		}
		else
			return m_program.read_byte(0x0562 + offset);
	}));
	map(0x04d7, 0x04d7).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			m_divmmc->automap_rom3_delayed_on_w(BIT(m_nr_bb_divmmc_ep_1, 4));
			return do_m1(0x04d7 + offset);
		}
		else
			return m_program.read_byte(0x04d7 + offset);
	}));
	map(0x056a, 0x056a).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			m_divmmc->automap_rom3_delayed_on_w(BIT(m_nr_bb_divmmc_ep_1, 5));
			return do_m1(0x056a + offset);
		}
		else
			return m_program.read_byte(0x056a + offset);
	}));
	map(0x0066, 0x0066).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			m_mf->cpu_m1_n_w(0);
			m_mf->cpu_a_0066_w(1);
			m_mf->cpu_mreq_n_w(0);
			m_mf->clock_w();

			m_divmmc->automap_nmi_instant_on_w(BIT(m_nr_bb_divmmc_ep_1, 1));
			m_divmmc->automap_nmi_delayed_on_w(BIT(m_nr_bb_divmmc_ep_1, 0));
			u8 data = do_m1(0x0066 + offset);

			m_mf->cpu_a_0066_w(0);
			m_mf->cpu_m1_n_w(1);
			m_mf->clock_w();

			return data;
		}
		else
			return m_program.read_byte(0x0066 + offset);
	}));
}

void specnext_state::map_mem(address_map &map)
{
	using views_link = std::reference_wrapper<memory_view>;
	views_link views[] = { m_view0, m_view1, m_view2, m_view3, m_view4, m_view5, m_view6, m_view7 };

	for (auto i = 0; i < 8; i++)
	{
		map(0x0000 + i * 0x2000, 0x1fff + i * 0x2000).bankrw(m_bank_ram[i]);
		map(0x0000 + i * 0x2000, 0x1fff + i * 0x2000).view(views[i].get());
		views[i].get()[0](0x0000 + i * 0x2000, 0x1fff + i * 0x2000).nopw();
	}
	views[0].get()[1](0x0000, 0x1fff).bankr(m_bank_boot_rom);
	views[1].get()[1](0x2000, 0x3fff).bankr(m_bank_boot_rom);
}

void specnext_state::map_io(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0xffff).unmaprw();

	map(0x0000, 0x0000).select(0xfffe).rw(FUNC(specnext_state::spectrum_ula_r), FUNC(specnext_state::spectrum_ula_w));
	map(0x00ff, 0x00ff).mirror(0xff00).rw(FUNC(specnext_state::port_ff_r), FUNC(specnext_state::port_ff_w));

	map(0x001f, 0x001f).select(0xff00).lrw8(NAME([this](offs_t offset)
	{
		return mf_port_r(offset | 0x1f);
	}), NAME([this](offs_t offset, u8 data) {
		mf_port_w(offset | 0x1f, data);
	}));
	map(0x003f, 0x003f).select(0xff00).lrw8(NAME([this](offs_t offset)
	{
		return mf_port_r(offset | 0x3f);
	}), NAME([this](offs_t offset, u8 data) {
		mf_port_w(offset | 0x3f, data);
	}));
	map(0x009f, 0x009f).select(0xff00).lrw8(NAME([this](offs_t offset)
	{
		return mf_port_r(offset | 0x8f);
	}), NAME([this](offs_t offset, u8 data) {
		mf_port_w(offset | 0x8f, data);
	}));
	map(0x00bf, 0x00bf).select(0xff00).lrw8(NAME([this](offs_t offset)
	{
		return mf_port_r(offset | 0xbf);
	}), NAME([this](offs_t offset, u8 data) {
		mf_port_w(offset | 0xbf, data);
	}));

	map(0x0001, 0x0001).mirror(0xfff4).lr8(NAME([this]() { // #bff5
		return m_nr_08_psg_turbosound_en ? m_ay_select : 0;
	}));
	map(0xc005, 0xc005).mirror(0x3ff8).lr8(NAME([this]() { // #fffd
		return m_ay[m_nr_08_psg_turbosound_en ? m_ay_select : 0]->data_r();
	})).w(FUNC(specnext_state::turbosound_address_w));
	map(0x8005, 0x8005).mirror(0x3ff8).lw8(NAME([this](u8 data) { // #bffd
		m_ay[m_nr_08_psg_turbosound_en ? m_ay_select : 0]->data_w(data);
	}));

	map(0x0001, 0x0001).select(0x7ffc).lw8(NAME([this](offs_t offset, u8 data) {
		const bool p3_timing_hw_en = (m_nr_03_machine_timing & 3) == 0b11;
		if (port_7ffd_io_en() && (BIT(offset, 14) || !p3_timing_hw_en) && !port_7ffd_locked())
		{
			port_7ffd_reg_w(data);
			memory_change(0x7ffd, data);
		}
	}));
	map(0xd001, 0xd001).mirror(0x0ffc).lw8(NAME([this](u8 data) {
		if (port_dffd_io_en() && (!port_7ffd_locked() || nr_8f_mapping_mode_profi()))
		{
			m_port_dffd_data = data;
			memory_change(0xdffd, data);
		}
	}));
	map(0x1001, 0x1001).mirror(0x0ffc).lw8(NAME([this](u8 data) {
		if (port_1ffd_io_en() && !port_7ffd_locked())
		{
			m_port_1ffd_data = data;
			memory_change(0x1ffd, data);
		}
	}));
	map(0xe0f7, 0xe0f7).mirror(0x0f00).lw8(NAME([this](u8 data) {
		if(port_eff7_io_en())
		{
			m_port_eff7_data = data;
			memory_change(0xeff7, data);
		}
	}));

	map(0x2001, 0x2001).mirror(0x0ffc).lr8(NAME([]() {
		return /*m_nr_d8_io_trap_fdc_en ? ... :*/ 0x00;
	}));
	map(0x3001, 0x3001).mirror(0x0ffc).lrw8(NAME([]() {
		return /*m_nr_d8_io_trap_fdc_en ? ... :*/ 0x00;
	}), NAME([this](u8 data) {
		if (m_nr_d8_io_trap_fdc_en)
			;
	}));


	map(0x103b, 0x103b).lr8(NAME([this]() {
		return port_i2c_io_en() ? (0xfe | m_i2c_scl_data) : 0x00;
	})).w(FUNC(specnext_state::i2c_scl_w));
	map(0x113b, 0x113b).lrw8(NAME([this]() {
		return port_i2c_io_en() ? (0xfe | (m_i2cmem->read_sda() & 1)) : 0x00;
	}), NAME([this](u8 data) {
		if(port_i2c_io_en())
			m_i2cmem->write_sda(data & 1);
	}));
	map(0x183b, 0x183b).select(0x0700).lrw8(NAME([this](offs_t offset) {
		u8 chanel = offset >> 8;
		return port_ctc_io_en() && (chanel < 4) ? m_ctc->read(chanel) : 0x00;
	}), NAME([this](offs_t offset, u8 data) {
		u8 chanel = offset >> 8;
		if(port_ctc_io_en() && (chanel < 4))
			m_ctc->write(chanel, data);
	}));
	map(0x123b, 0x123b).lrw8(NAME([this]() {
		return (m_port_123b_layer2_map_segment << 6) | (0b00 << 4) | (m_port_123b_layer2_map_shadow << 3) | (m_port_123b_layer2_map_rd_en << 2) | (m_port_123b_layer2_en << 1) | m_port_123b_layer2_map_wr_en;
	}), NAME([this](u8 data) {
		if (BIT(~data, 4))
		{
			port_123b_layer2_en_w(BIT(data, 1));
			m_port_123b_layer2_map_wr_en = BIT(data, 0);
			m_port_123b_layer2_map_rd_en = BIT(data, 2);
			m_port_123b_layer2_map_shadow = BIT(data, 3);
			m_port_123b_layer2_map_segment = BIT(data, 6, 2);
		}
		else
			m_port_123b_layer2_offset = BIT(data, 0, 3);

		bank_update(0, 6);
	}));
	map(0x243b, 0x243b).lrw8(NAME([this]() { return m_nr_register; })
		, NAME([this](u8 data) { m_nr_register = data; }));
	map(0x253b, 0x253b).lrw8(NAME([this]() { return m_next_regs.read_byte(m_nr_register); })
		, NAME([this](u8 data) { m_next_regs.write_byte(m_nr_register, data); }));
	map(0x303b, 0x303b).lrw8(NAME([this]() {
		return port_sprite_io_en() ? m_sprites->mirror_num_r() : 0x00;
	}), NAME([this](u8 data) {
		if (port_sprite_io_en())
			m_sprites->io_w(0x303b, data);
	}));
	map(0xbf3b, 0xbf3b).lw8(NAME([this](u8 data) {
		if (port_ulap_io_en())
		{
			m_port_bf3b_ulap_mode = BIT(data, 6, 2);
			if (BIT(data, 6, 2) == 0b00)
				m_port_bf3b_ulap_index = BIT(data, 0, 6);
		}
	})); // ULA+ Register
	map(0xff3b, 0xff3b).lrw8(NAME([this]() {
		u8 port_ff3b_dat = 0x00;
		if (port_ulap_io_en())
		{
			port_ff3b_dat = m_port_bf3b_ulap_mode == 0b00
				? bitswap<8>(nr_palette_dat(), 5, 4, 3, 8, 7, 6, 2, 1)
				: m_port_ff3b_ulap_en;
		}
		return port_ff3b_dat;
	}), NAME([this](u8 data) {
		if (port_ulap_io_en())
		{
			if (m_port_bf3b_ulap_mode == 0b01)
				port_ff3b_ulap_en_w(BIT(data, 0));
		}
	})); // ULA+ Data
	map(0x0057, 0x0057).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (port_sprite_io_en())
			m_sprites->io_w(0x0057, data);
	}));
	map(0x005b, 0x005b).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (port_sprite_io_en())
			m_sprites->io_w(0x005b, data);
	}));
	map(0x00e7, 0x00e7).mirror(0xff00).w(FUNC(specnext_state::port_e7_reg_w));
	map(0x00e3, 0x00e3).mirror(0xff00).lrw8(NAME([this]() { return port_divmmc_io_en() ? m_port_e3_reg & ~0x30 : 0x00; })
		, NAME([this](u8 data) { if (port_divmmc_io_en()) { port_e3_reg_w((m_port_e3_reg & 0x40) | data); }}));
	map(0x00eb, 0x00eb).mirror(0xff00).rw(FUNC(specnext_state::spi_data_r), FUNC(specnext_state::spi_data_w));

	map(0x000b, 0x000b).mirror(0xff00).lrw8(NAME([this]() { return dma_r(1); }), NAME([this](u8 data) { dma_w(1, data); }));
	map(0x006b, 0x006b).mirror(0xff00).lrw8(NAME([this]() { return dma_r(0); }), NAME([this](u8 data) { dma_w(0, data); }));

	map(0x0bdf, 0x0bdf).mirror(0xf000).lr8(NAME([this]() -> u8 { return m_io_mouse[0]->read(); }));                 // #fbdf
	map(0x0fdf, 0x0fdf).mirror(0xf000).lr8(NAME([this]() -> u8 { return ~m_io_mouse[1]->read(); }));                // #ffdf
	map(0x0adf, 0x0adf).mirror(0xf000).lr8(NAME([this]() -> u8 { return 0x80 | (m_io_mouse[2]->read() & 0x07); })); // #fadf

	// TODO resolve conflicts mf+joy+DAC: 1f, 3f
	//map(0x001f, 0x001f).mirror(0xff00).lr8(NAME([]() -> u8 { return 0x00; /* Joy1,2*/ })).lw8(NAME([this](u8 data) {
	//  if (m_nr_08_dac_en)
	//      m_dac[0]->data_w(data);
	//}));
	map(0x00f1, 0x00f1).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (m_nr_08_dac_en)
			m_dac[0]->data_w(data);
	}));
	//map(0x003f, 0x003f).mirror(0xff00).lw8(NAME([this](u8 data) {
	//  if (m_nr_08_dac_en)
	//      m_dac[0]->data_w(data);
	//}));
	map(0x000f, 0x000f).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (m_nr_08_dac_en)
			m_dac[1]->data_w(data);
	}));
	map(0x00f3, 0x00f3).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (m_nr_08_dac_en)
			m_dac[1]->data_w(data);
	}));
	map(0x00df, 0x00df).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (m_nr_08_dac_en && port_dac_mono_AD_df_io_en())
		{
			m_dac[0]->data_w(data);
			m_dac[3]->data_w(data);
		}
	}));
	map(0x00fb, 0x00fb).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (m_nr_08_dac_en && port_dac_mono_AD_fb_io_en())
		{
			m_dac[0]->data_w(data);
			m_dac[3]->data_w(data);
		}
	}));
	map(0x00b3, 0x00b3).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (m_nr_08_dac_en && port_dac_mono_BC_b3_io_en())
		{
			m_dac[1]->data_w(data);
			m_dac[2]->data_w(data);
		}
	}));
	map(0x004f, 0x004f).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (m_nr_08_dac_en)
			m_dac[2]->data_w(data);
	}));
	map(0x00f9, 0x00f9).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (m_nr_08_dac_en)
			m_dac[2]->data_w(data);
	}));
	map(0x005f, 0x005f).mirror(0xff00).lw8(NAME([this](u8 data) {
		if (m_nr_08_dac_en)
			m_dac[3]->data_w(data);
	}));
}

void specnext_state::map_regs(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(specnext_state::reg_r), FUNC(specnext_state::reg_w));
}

INPUT_PORTS_START(specnext)
	PORT_INCLUDE(spec_plus)

	PORT_START("ISSUE")
	PORT_CONFNAME(0x03, 0x02, "Hardware Version" )
	PORT_CONFSETTING(0x00, "Issue 1 (TBBLUE)" )
	PORT_CONFSETTING(0x01, "Issue 2 (KS 1)" )
	PORT_CONFSETTING(0x02, "Issue 4 (KS 2)" )
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("mouse_input1")
	PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(40)

	PORT_START("mouse_input2")
	PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(40)

	PORT_START("mouse_input3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Middle mouse button") PORT_CODE(MOUSECODE_BUTTON3)

	PORT_MODIFY("NMI")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("NMI MF") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, specnext_state, on_mf_nmi, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("NMI DivMMC") PORT_CODE(KEYCODE_F11) PORT_CHANGED_MEMBER(DEVICE_SELF, specnext_state, on_divmmc_nmi, 0)
INPUT_PORTS_END

void specnext_state::machine_start()
{
	spectrum_128_state::machine_start();

	m_irq_line_timer = timer_alloc(FUNC(specnext_state::line_irq_on), this);
	m_spi_clock = timer_alloc(FUNC(specnext_state::spi_clock), this);

	m_regs_map->space(AS_PROGRAM).specific(m_next_regs);
	m_maincpu->space(AS_PROGRAM).specific(m_program);
	m_maincpu->space(AS_IO).specific(m_io);

	for (auto i = 0; i < 8; i++)
		m_bank_ram[i]->configure_entries(0, m_ram->size() / 0x2000, m_ram->pointer(), 0x2000);
	m_bank_boot_rom->configure_entry(0, memregion("maincpu")->base());

	const u8 *ram = m_ram->pointer() + 0x40000;
	m_ula->set_host_ram_ptr(ram);
	m_tiles->set_host_ram_ptr(ram);
	m_layer2->set_host_ram_ptr(ram);
	m_lores->set_host_ram_ptr(ram);

	m_nr_02_hard_reset = 1;

	// Save
	save_pointer(NAME(m_page_shadow), 8);
	save_item(NAME(m_bootrom_en));
	save_item(NAME(m_port_ff_data));
	save_item(NAME(m_port_1ffd_special_old));
	save_item(NAME(m_port_1ffd_data));
	//save_item(NAME(m_port_7ffd_data));
	save_item(NAME(m_port_dffd_data));
	save_item(NAME(m_port_eff7_data));
	save_item(NAME(m_port_e7_reg));
	save_item(NAME(m_nr_register));
	save_item(NAME(m_port_e3_reg));
	save_item(NAME(m_divmmc_delayed_check));
	save_item(NAME(m_global_transparent));
	save_item(NAME(m_sram_rom));
	save_item(NAME(m_sram_rom3));
	save_item(NAME(m_sram_alt_128_n));
	save_pointer(NAME(m_mmu), 8);
	save_item(NAME(m_nr_02_bus_reset));
	save_item(NAME(m_nr_02_generate_mf_nmi));
	save_item(NAME(m_nr_02_generate_divmmc_nmi));
	save_item(NAME(m_nr_02_hard_reset));
	save_item(NAME(m_nr_02_reset_type));
	save_item(NAME(m_nr_03_machine_type));
	save_item(NAME(m_nr_03_user_dt_lock));
	save_item(NAME(m_nr_03_machine_timing));
	save_item(NAME(m_nr_03_config_mode));
	save_item(NAME(m_nr_04_romram_bank));
	save_item(NAME(m_nr_05_joy1));
	save_item(NAME(m_nr_05_joy0));
	save_item(NAME(m_nr_06_psg_mode));
	save_item(NAME(m_nr_06_ps2_mode));
	save_item(NAME(m_nr_06_button_m1_nmi_en));
	save_item(NAME(m_nr_06_button_drive_nmi_en));
	save_item(NAME(m_nr_06_hotkey_5060_en));
	save_item(NAME(m_nr_06_internal_speaker_beep));
	save_item(NAME(m_nr_06_hotkey_cpu_speed_en));
	save_item(NAME(m_nr_07_cpu_speed));
	save_item(NAME(m_nr_08_keyboard_issue2));
	save_item(NAME(m_nr_08_psg_turbosound_en));
	save_item(NAME(m_nr_08_port_ff_rd_en));
	save_item(NAME(m_nr_08_dac_en));
	save_item(NAME(m_nr_08_internal_speaker_en));
	save_item(NAME(m_nr_08_psg_stereo_mode));
	save_item(NAME(m_nr_08_contention_disable));
	save_item(NAME(m_nr_09_hdmi_audio_en));
	save_item(NAME(m_nr_09_sprite_tie));
	save_item(NAME(m_nr_09_psg_mono));
	save_item(NAME(m_nr_0a_mouse_dpi));
	save_item(NAME(m_nr_0a_mouse_button_reverse));
	save_item(NAME(m_nr_0a_divmmc_automap_en));
	save_item(NAME(m_nr_0a_mf_type));
	save_item(NAME(m_nr_0b_joy_iomode_0));
	save_item(NAME(m_nr_0b_joy_iomode));
	save_item(NAME(m_nr_0b_joy_iomode_en));
	save_item(NAME(m_nr_10_coreid));
	save_item(NAME(m_nr_11_video_timing));
	save_item(NAME(m_nr_10_flashboot));
	save_item(NAME(m_nr_12_layer2_active_bank));
	save_item(NAME(m_nr_13_layer2_shadow_bank));
	save_item(NAME(m_nr_14_global_transparent_rgb));
	save_item(NAME(m_nr_15_sprite_en));
	save_item(NAME(m_nr_15_sprite_over_border_en));
	save_item(NAME(m_nr_15_layer_priority));
	save_item(NAME(m_nr_15_sprite_border_clip_en));
	save_item(NAME(m_nr_15_sprite_priority));
	save_item(NAME(m_nr_15_lores_en));
	save_item(NAME(m_nr_16_layer2_scrollx));
	save_item(NAME(m_nr_17_layer2_scrolly));
	save_item(NAME(m_nr_18_layer2_clip_x1));
	save_item(NAME(m_nr_18_layer2_clip_x2));
	save_item(NAME(m_nr_18_layer2_clip_y1));
	save_item(NAME(m_nr_18_layer2_clip_y2));
	save_item(NAME(m_nr_18_layer2_clip_idx));
	save_item(NAME(m_nr_19_sprite_clip_x1));
	save_item(NAME(m_nr_19_sprite_clip_x2));
	save_item(NAME(m_nr_19_sprite_clip_y1));
	save_item(NAME(m_nr_19_sprite_clip_y2));
	save_item(NAME(m_nr_19_sprite_clip_idx));
	save_item(NAME(m_nr_1a_ula_clip_x1));
	save_item(NAME(m_nr_1a_ula_clip_x2));
	save_item(NAME(m_nr_1a_ula_clip_y1));
	save_item(NAME(m_nr_1a_ula_clip_y2));
	save_item(NAME(m_nr_1a_ula_clip_idx));
	save_item(NAME(m_nr_1b_tm_clip_x1));
	save_item(NAME(m_nr_1b_tm_clip_x2));
	save_item(NAME(m_nr_1b_tm_clip_y1));
	save_item(NAME(m_nr_1b_tm_clip_y2));
	save_item(NAME(m_nr_1b_tm_clip_idx));
	save_item(NAME(m_nr_22_line_interrupt_en));
	save_item(NAME(m_nr_23_line_interrupt));
	save_item(NAME(m_nr_26_ula_scrollx));
	save_item(NAME(m_nr_27_ula_scrolly));
	save_item(NAME(m_nr_2d_i2s_sample));
	save_item(NAME(m_nr_30_tm_scrollx));
	save_item(NAME(m_nr_31_tm_scrolly));
	save_item(NAME(m_nr_32_lores_scrollx));
	save_item(NAME(m_nr_33_lores_scrolly));
	save_item(NAME(m_nr_palette_idx));
	save_item(NAME(m_nr_palette_sub_idx));
	save_item(NAME(m_nr_42_ulanext_format));
	save_item(NAME(m_nr_43_palette_autoinc_disable));
	save_item(NAME(m_nr_43_palette_write_select));
	save_item(NAME(m_nr_43_active_sprite_palette));
	save_item(NAME(m_nr_43_active_layer2_palette));
	save_item(NAME(m_nr_43_active_ula_palette));
	save_item(NAME(m_nr_43_ulanext_en));
	save_item(NAME(m_nr_stored_palette_value));
	save_item(NAME(m_nr_4a_fallback_rgb));
	save_item(NAME(m_nr_4b_sprite_transparent_index));
	save_item(NAME(m_nr_4c_tm_transparent_index));
	save_item(NAME(m_nr_copper_addr));
	save_item(NAME(m_nr_copper_data_stored));
	save_item(NAME(m_nr_62_copper_mode));
	save_item(NAME(m_nr_64_copper_offset));
	save_item(NAME(m_nr_68_ula_en));
	save_item(NAME(m_nr_68_blend_mode));
	save_item(NAME(m_nr_68_cancel_extended_keys));
	save_item(NAME(m_nr_68_ula_fine_scroll_x));
	save_item(NAME(m_nr_68_ula_stencil_mode));
	save_item(NAME(m_nr_6a_lores_radastan));
	save_item(NAME(m_nr_6a_lores_radastan_xor));
	save_item(NAME(m_nr_6a_lores_palette_offset));
	save_item(NAME(m_nr_6b_tm_en));
	save_item(NAME(m_nr_6b_tm_control));
	save_item(NAME(m_nr_6c_tm_default_attr));
	save_item(NAME(m_nr_6e_tilemap_base));
	save_item(NAME(m_nr_6e_tilemap_base_7));
	save_item(NAME(m_nr_6f_tilemap_tiles));
	save_item(NAME(m_nr_6f_tilemap_tiles_7));
	save_item(NAME(m_nr_70_layer2_resolution));
	save_item(NAME(m_nr_70_layer2_palette_offset));
	save_item(NAME(m_nr_71_layer2_scrollx_msb));
	save_item(NAME(m_nr_7f_user_register_0));
	save_item(NAME(m_nr_80_expbus));
	save_item(NAME(m_nr_81_expbus_speed));
	save_item(NAME(m_nr_81_expbus_clken));
	save_item(NAME(m_nr_81_expbus_nmi_debounce_disable));
	save_item(NAME(m_nr_81_expbus_ula_override));
	save_item(NAME(m_nr_82_internal_port_enable));
	save_item(NAME(m_nr_83_internal_port_enable));
	save_item(NAME(m_nr_84_internal_port_enable));
	save_item(NAME(m_nr_85_internal_port_enable));
	save_item(NAME(m_nr_85_internal_port_reset_type));
	save_item(NAME(m_nr_86_bus_port_enable));
	save_item(NAME(m_nr_87_bus_port_enable));
	save_item(NAME(m_nr_88_bus_port_enable));
	save_item(NAME(m_nr_89_bus_port_enable));
	save_item(NAME(m_nr_89_bus_port_reset_type));
	save_item(NAME(m_nr_8a_bus_port_propagate));
	save_item(NAME(m_nr_8c_altrom));
	save_item(NAME(m_nr_8f_mapping_mode));
	save_item(NAME(m_nr_90_pi_gpio_o_en));
	save_item(NAME(m_nr_91_pi_gpio_o_en));
	save_item(NAME(m_nr_92_pi_gpio_o_en));
	save_item(NAME(m_nr_93_pi_gpio_o_en));
	save_item(NAME(m_nr_98_pi_gpio_o));
	save_item(NAME(m_nr_99_pi_gpio_o));
	save_item(NAME(m_nr_9a_pi_gpio_o));
	save_item(NAME(m_nr_9b_pi_gpio_o));
	save_item(NAME(m_nr_a0_pi_peripheral_en));
	save_item(NAME(m_nr_a2_pi_i2s_ctl));
	save_item(NAME(m_nr_a8_esp_gpio0_en));
	save_item(NAME(m_nr_a9_esp_gpio0));
	save_item(NAME(m_nr_b8_divmmc_ep_0));
	save_item(NAME(m_nr_b9_divmmc_ep_valid_0));
	save_item(NAME(m_nr_ba_divmmc_ep_timing_0));
	save_item(NAME(m_nr_bb_divmmc_ep_1));
	save_item(NAME(m_nr_c0_im2_vector));
	save_item(NAME(m_nr_c0_stackless_nmi));
	save_item(NAME(m_nr_c0_int_mode_pulse_0_im2_1));
	save_item(NAME(m_nr_c2_retn_address_lsb));
	save_item(NAME(m_nr_c3_retn_address_msb));
	save_item(NAME(m_nr_c4_int_en_0_expbus));
	save_item(NAME(m_nr_c6_int_en_2_654));
	save_item(NAME(m_nr_c6_int_en_2_210));
	save_item(NAME(m_nr_cc_dma_int_en_0_7));
	save_item(NAME(m_nr_cc_dma_int_en_0_10));
	save_item(NAME(m_nr_cd_dma_int_en_1));
	save_item(NAME(m_nr_ce_dma_int_en_2_654));
	save_item(NAME(m_nr_ce_dma_int_en_2_210));
	save_item(NAME(m_nr_d8_io_trap_fdc_en));
	save_item(NAME(m_nr_d9_iotrap_write));
	save_item(NAME(m_nr_da_iotrap_cause));
	save_item(NAME(m_nr_f0_select));
	save_item(NAME(m_nr_f0_xdna_en));
	save_item(NAME(m_nr_f0_xadc_en));
	save_item(NAME(m_nr_f0_xdev_cmd));
	save_item(NAME(m_nr_f0_xadc_eoc));
	save_item(NAME(m_nr_f0_xadc_eos));
	save_item(NAME(m_nr_f8_xadc_dwe));
	save_item(NAME(m_nr_f8_xadc_daddr));
	save_item(NAME(m_nr_f8_xadc_den));
	save_item(NAME(m_nr_f9_xadc_d0));
	save_item(NAME(m_nr_fa_xadc_d1));
	save_item(NAME(m_pulse_int_n));
	save_item(NAME(m_nr_09_scanlines));
	save_item(NAME(m_eff_nr_03_machine_timing));
	save_item(NAME(m_eff_nr_05_5060));
	save_item(NAME(m_eff_nr_05_scandouble_en));
	save_item(NAME(m_eff_nr_08_contention_disable));
	save_item(NAME(m_eff_nr_09_scanlines));
	save_item(NAME(m_port_123b_layer2_en));
	save_item(NAME(m_port_123b_layer2_map_wr_en));
	save_item(NAME(m_port_123b_layer2_map_rd_en));
	save_item(NAME(m_port_123b_layer2_map_shadow));
	save_item(NAME(m_port_123b_layer2_map_segment));
	save_item(NAME(m_port_123b_layer2_offset));
	save_item(NAME(m_port_bf3b_ulap_mode));
	save_item(NAME(m_port_bf3b_ulap_index));
	save_item(NAME(m_port_ff3b_ulap_en));
	save_item(NAME(m_ay_select));
	save_item(NAME(m_spi_clock_cycles));
	save_item(NAME(m_spi_clock_state));
	save_item(NAME(m_spi_mosi_dat));
	save_item(NAME(m_spi_miso_dat));
	save_item(NAME(m_i2c_scl_data));
	save_item(NAME(m_irq_mask));
}

void specnext_state::reset_hard()
{
	m_nr_02_hard_reset = 0;
	m_bootrom_en = 1;

	m_dma->set_dma_mode(z80dma_device::dma_mode::SPEC_NEXT);
	// nmi_mf = 0;
	// nmi_divmmc = 0;
	// nmi_expbus = 0;
	m_nr_80_expbus = 0;
	// expbus_eff_en = 0;
	// expbus_eff_disable_io = 0;
	// expbus_eff_disable_mem = 0;
	// expbus_eff_clken = 0;
	m_nr_8c_altrom = 0;
	// bus_iorq_ula = 0;
	// port_253b_rd_qq = 0;
	// sram_req_d = 1;
	// sram_wait_n = 1;
	// m_port_103b_dat = 0xff;
	// m_port_113b_dat = 0xff;
	port_e7_reg_w(0xff);
	// joy_iomode_pin7 = 1;
	// port_fe_border = 0;
	m_nr_8f_mapping_mode = 0;
	// m_port_123b_dat = 0;
	m_nr_02_bus_reset = 0;
	m_nr_03_machine_timing = 0b011;
	m_nr_03_user_dt_lock = 0;
	m_nr_03_config_mode = 1;
	m_nr_03_machine_type = 0b011;
	m_nr_04_romram_bank = 0;
	m_nr_05_joy0 = 0b001;
	m_nr_05_joy1 = 0b000;
	m_nr_06_hotkey_cpu_speed_en = 1;
	m_nr_06_hotkey_5060_en = 1;
	m_nr_06_button_drive_nmi_en = 0;
	m_nr_06_button_m1_nmi_en = 0;
	m_nr_06_ps2_mode = 0;
	m_nr_06_psg_mode = 0b00;
	nr_07_cpu_speed_w(0b00);
	m_nr_06_internal_speaker_beep = 0;
	m_nr_08_contention_disable = 0;
	m_nr_08_psg_stereo_mode = 0;
	m_nr_08_internal_speaker_en = 1;
	m_nr_08_dac_en = 0;
	m_nr_08_port_ff_rd_en = 0;
	m_nr_08_psg_turbosound_en = 0;
	m_nr_08_keyboard_issue2 = 0;
	m_nr_09_psg_mono = 0;
	m_nr_09_hdmi_audio_en = 1;
	m_nr_09_sprite_tie = 0;
	nr_0a_mf_type_w(0b00);
	m_nr_0a_divmmc_automap_en = 0;
	m_nr_0a_mouse_button_reverse = 0;
	m_nr_0a_mouse_dpi = 0b01;
	m_nr_10_flashboot = 0;
	m_nr_10_coreid = 0b00001;
	m_nr_7f_user_register_0 = 0xff;
	m_nr_81_expbus_ula_override = 0;
	m_nr_81_expbus_nmi_debounce_disable = 0;
	m_nr_81_expbus_clken = 0;
	m_nr_81_expbus_speed = 0b00;
	m_nr_82_internal_port_enable = 0xff;
	m_nr_83_internal_port_enable = 0xff;
	m_nr_84_internal_port_enable = 0xff;
	m_nr_85_internal_port_enable = 0x0f;
	m_nr_85_internal_port_reset_type = 1;
	m_nr_86_bus_port_enable = 0xff;
	m_nr_87_bus_port_enable = 0xff;
	m_nr_88_bus_port_enable = 0xff;
	m_nr_89_bus_port_enable = 0x0f;
	m_nr_89_bus_port_reset_type = 1;
	m_nr_8a_bus_port_propagate = 0x3f;
	m_nr_90_pi_gpio_o_en = 0;
	m_nr_91_pi_gpio_o_en = 0;
	m_nr_92_pi_gpio_o_en = 0;
	m_nr_93_pi_gpio_o_en = 0;
	m_nr_a8_esp_gpio0_en = 0;
	m_nr_a9_esp_gpio0 = 1;
	m_nr_f0_select = 1;
	m_nr_f0_xdna_en = 0;
	m_nr_f0_xadc_en = 0;
	m_nr_f0_xdev_cmd = 0x00;
	m_nr_f0_xadc_eoc = 0;
	m_nr_f0_xadc_eos = 0;
	m_nr_f8_xadc_dwe = 0;
	m_nr_f8_xadc_daddr = 0x00;
	m_nr_f8_xadc_den = 0;
	m_nr_f9_xadc_d0 = 0;
	m_nr_fa_xadc_d1 = 0;
	//m_nr_05_5060 = 0;
	//m_nr_05_scandouble_en = 1;
	m_nr_09_scanlines = 0b00;
	m_nr_02_reset_type = 0b100;
	// nr_keymap_sel = 0;
	// nr_keymap_addr = 0;
	// hotkeys_1 = 0;
	// hotkeys_0 = 0;
	// video_timing_change_d = 0;
	m_eff_nr_03_machine_timing = 0b011;
	m_eff_nr_08_contention_disable = 0;
	// xdna_load = 0;
	// xdna_shift = 0;
	// xadc_reset = 0;
	// xadc_convst = 0;
	m_eff_nr_05_5060 = 0;
	m_eff_nr_05_scandouble_en = 0;
	m_eff_nr_09_scanlines = 0;
	m_nr_2d_i2s_sample = 0b00;

	// m_port_00_data = 0;
	m_nr_0a_divmmc_automap_en = 1;
}

void specnext_state::machine_reset()
{
	// TODO prevent from soft reset in config mode?
	spectrum_128_state::machine_reset();

	m_irq_line_timer->reset();

	if (m_nr_02_hard_reset)
		reset_hard();

	m_spi_clock->reset();
	m_spi_clock_cycles = 0;
	m_spi_clock_state = false;

	m_port_ff_data = 0;
	m_divmmc_delayed_check = 0;

	m_dma->set_dma_mode(z80dma_device::dma_mode::SPEC_NEXT);
	//z80_retn_seen_28_d  = 0;
	//im2_dma_delay  = 0;
	m_pulse_int_n  = 1;
	m_nr_c2_retn_address_lsb  = 0x00;
	m_nr_c3_retn_address_msb  = 0x00;
	//z80_stackless_retn_en  = 0;
	//nmi_mf  = 0;
	//nmi_divmmc  = 0;
	//nmi_expbus  = 0;
	//nmi_state  = S_NMI_IDLE;
	m_nr_80_expbus = (m_nr_80_expbus << 4) | (m_nr_80_expbus & 0x0f);
	m_nr_8c_altrom = (m_nr_8c_altrom << 4) | (m_nr_8c_altrom & 0x0f);
	//port_253b_rd_qq  = 0b00;
	//sram_req_d  = 0;
	m_i2c_scl_data = 1;
	port_e7_reg_w(0xff);
	//joy_iomode_pin7  = 1;

	port_7ffd_reg_w(0x00);
	m_port_1ffd_data = 0;
	m_port_1ffd_special_old = 0;
	m_port_dffd_data = 0;
	m_port_eff7_data = 0;
/* TODO don't use inherited
    port_fe_reg  = 0x00;
    port_ff_reg  = 0x00;
    port_7ffd_reg  = 0x00;
    port_dffd_reg  = 0x00;
    port_dffd_reg_6  = 0;
    port_1ffd_reg  = 0x00;
    port_1ffd_special_old  = 0;
    port_eff7_reg_2  = 0;
    port_eff7_reg_3  = 0;
*/
	m_nr_02_generate_mf_nmi  = 0;
	m_nr_02_generate_divmmc_nmi  = 0;
	m_nr_da_iotrap_cause  = 0x00;
	m_nr_d9_iotrap_write  = 0x00;
	port_123b_layer2_en_w(0);
	m_port_123b_layer2_map_wr_en  = 0;
	m_port_123b_layer2_map_rd_en  = 0;
	m_port_123b_layer2_map_shadow = 0;
	m_port_123b_layer2_map_segment  = 0x00;
	m_port_123b_layer2_offset  = 0x00;
	port_e3_reg_w(0x00);
	m_port_bf3b_ulap_mode  = 0x00;
	m_port_bf3b_ulap_index  = 0x00;
	port_ff3b_ulap_en_w(0);
	m_nr_register = 0x24;
	//copper_requester_d  = 0;
	//copper_req  = 0;
	//copper_nr_reg  = 0x00;
	//copper_nr_dat  = 0x00;
	//cpu_requester_d  = 0;
	//cpu_req  = 0;
	//cpu_nr_reg  = 0x00;
	//cpu_nr_dat  = 0x00;
	nr_07_cpu_speed_w(0b00);
	m_eff_nr_08_contention_disable  = 0;

	//expbus_eff_en  = expbus_en;
	//expbus_eff_disable_io  = expbus_disable_io;
	//expbus_eff_disable_mem  = expbus_disable_mem;
	//expbus_eff_clken  = expbus_clken;


	m_nr_06_hotkey_cpu_speed_en = 1;
	m_nr_06_hotkey_5060_en = 1;

	m_nr_08_contention_disable = 0;

	m_nr_09_sprite_tie = 0;

	m_nr_0b_joy_iomode_en = 0;
	m_nr_0b_joy_iomode = 0b00;
	m_nr_0b_joy_iomode_0 = 1;

	nr_12_layer2_active_bank_w(0b0001000);
	nr_13_layer2_shadow_bank_w(0b0001011);

	nr_14_global_transparent_rgb_w(0xe3);

	m_nr_15_lores_en = 0;
	nr_15_sprite_priority_w(0);
	nr_15_sprite_border_clip_en_w(0);
	m_nr_15_layer_priority = 0x00;
	nr_15_sprite_over_border_en_w(0);
	m_nr_15_sprite_en = 0;

	nr_16_layer2_scrollx_w(0x00);
	nr_17_layer2_scrolly_w(0x00);
	nr_18_layer2_clip_x1_w(0x00);
	nr_18_layer2_clip_x2_w(0xff);
	nr_18_layer2_clip_y1_w(0x00);
	nr_18_layer2_clip_y2_w(0xbf);
	m_nr_18_layer2_clip_idx = 0b00;

	nr_19_sprite_clip_x1_w(0x00);
	nr_19_sprite_clip_x2_w(0xff);
	nr_19_sprite_clip_y1_w(0x00);
	nr_19_sprite_clip_y2_w(0xbf);
	m_nr_19_sprite_clip_idx = 0b00;

	nr_1a_ula_clip_x1_w(0x00);
	nr_1a_ula_clip_x2_w(0xff);
	nr_1a_ula_clip_y1_w(0x00);
	nr_1a_ula_clip_y2_w(0xbf);
	m_nr_1a_ula_clip_idx = 0b00;

	nr_1b_tm_clip_x1_w(0x00);
	nr_1b_tm_clip_x2_w(0x9f);
	nr_1b_tm_clip_y1_w(0x00);
	nr_1b_tm_clip_y2_w(0xff);
	m_nr_1b_tm_clip_idx = 0b00;

	m_nr_22_line_interrupt_en = 0;
	m_nr_23_line_interrupt = 0x00;
	nr_26_ula_scrollx_w(0x00);
	nr_27_ula_scrolly_w(0x00);
	nr_30_tm_scrollx_w(0x00);
	nr_31_tm_scrolly_w(0x00);
	nr_32_lores_scrollx_w(0x00);
	nr_33_lores_scrolly_w(0x00);

	m_nr_palette_idx = 0x00;
	m_nr_palette_sub_idx = 0;

	nr_42_ulanext_format_w(0x07);

	m_nr_43_palette_autoinc_disable = 0;
	m_nr_43_palette_write_select = 0b000;
	nr_43_active_sprite_palette_w(0);
	nr_43_active_layer2_palette_w(0);
	nr_43_active_ula_palette_w(0);
	nr_43_ulanext_en_w(0);

	m_nr_stored_palette_value = 0x00;

	reg_w(0x4a, 0xe3); // m_nr_4a_fallback_rgb = 0xe3;
	nr_4b_sprite_transparent_index_w(0xe3);
	nr_4c_tm_transparent_index_w(0xf);

	nr_62_copper_mode_w(0b00);
	m_nr_copper_addr = 0x00;
	m_nr_copper_data_stored = 0x00;

	m_nr_64_copper_offset = 0x00;

	m_nr_68_ula_en = 1;
	m_nr_68_blend_mode = 0b00;
	m_nr_68_cancel_extended_keys = 0;
	nr_68_ula_fine_scroll_x_w(0);
	m_nr_68_ula_stencil_mode = 0;

	nr_6a_lores_radastan_w(0);
	nr_6a_lores_palette_offset_w(0x00);
	nr_6a_lores_radastan_xor_w(0);

	m_nr_6b_tm_en = 0;
	nr_6b_tm_control_w(0x00);
	nr_6c_tm_default_attr_w(0x00);
	nr_6e_tilemap_base_w(0, 0b101100);
	nr_6f_tilemap_tiles_w(0, 0b001100);

	nr_70_layer2_resolution_w(0b00);
	nr_70_layer2_palette_offset_w(0x00);

	nr_71_layer2_scrollx_msb_w(0);

	if (m_nr_85_internal_port_reset_type)
	{
		m_nr_82_internal_port_enable = 0xff;
		m_nr_83_internal_port_enable = 0xff;
		m_nr_84_internal_port_enable = 0xff;
		m_nr_85_internal_port_enable = 0xf;
	}

	if (!m_nr_89_bus_port_reset_type)
	{
		m_nr_86_bus_port_enable = 0xff;
		m_nr_87_bus_port_enable = 0xff;
		m_nr_88_bus_port_enable = 0xff;
		m_nr_89_bus_port_enable = 0xf;
	}

	m_nr_98_pi_gpio_o = 0xff;
	m_nr_99_pi_gpio_o = 0x01;
	m_nr_9a_pi_gpio_o = 0x00;
	m_nr_9b_pi_gpio_o = 0x00;

	m_nr_90_pi_gpio_o_en = 0x00;
	m_nr_91_pi_gpio_o_en = 0x00;
	m_nr_92_pi_gpio_o_en = 0x00;
	m_nr_93_pi_gpio_o_en = 0x00;

	m_nr_a0_pi_peripheral_en = 0x00;
	m_nr_a2_pi_i2s_ctl = 0x00;

	m_nr_a8_esp_gpio0_en = 0;
	m_nr_a9_esp_gpio0 = 1;

	m_nr_b8_divmmc_ep_0 = 0x83;
	m_nr_b9_divmmc_ep_valid_0 = 0x01;
	m_nr_ba_divmmc_ep_timing_0 = 0x00;
	m_nr_bb_divmmc_ep_1 = 0xcd;

	nr_c0_im2_vector_w(0x00);
	m_nr_c0_stackless_nmi = 0;
	m_maincpu->nmi_stackless_w(m_nr_c0_stackless_nmi);
	m_nr_c0_int_mode_pulse_0_im2_1 = 0;

	m_nr_c4_int_en_0_expbus = 1;

	m_nr_c6_int_en_2_654 = 0x00;
	m_nr_c6_int_en_2_210 = 0x00;

	m_nr_cc_dma_int_en_0_7 = 0;
	m_nr_cc_dma_int_en_0_10 = 0x00;
	m_nr_cd_dma_int_en_1 = 0x00;
	m_nr_ce_dma_int_en_2_654 = 0x00;
	m_nr_ce_dma_int_en_2_210 = 0x00;

	m_nr_d8_io_trap_fdc_en = 0;

	if (m_nr_03_config_mode)
		m_bootrom_en = 1;

	mmu_x2_w(0, 0xff);
	mmu_x2_w(2, 0x0a);
	mmu_x2_w(4, 0x04);
	mmu_x2_w(6, 0x00);

	m_ay_select = 0;
	m_irq_mask = 0;
}

static const gfx_layout bootrom_charlayout =
{
	8, 8,          // 8 x 8 characters */
	96,            // 96 characters */
	1,             // 1 bits per pixel */
	{ 0 },           // no bitplanes
	{ STEP8(0, 1) }, // x offsets
	{ STEP8(0, 8) }, // y offsets
	8 * 8          // every char takes 8 bytes
};

static GFXDECODE_START(gfx_tbblue)
	GFXDECODE_ENTRY("maincpu", 0x1584, bootrom_charlayout, 0xf7, 1)
GFXDECODE_END

void specnext_state::video_start()
{
	spectrum_128_state::video_start();
	m_contention_pattern = {}; // No contention for now

	address_space &prg = m_maincpu->space(AS_PROGRAM);
	prg.install_write_tap(0x0000, 0xbfff, "shadow_w", [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		u8 bank8 = offset >> 13;
		if (~m_page_shadow[bank8])
		{
			u8 *to = m_ram->pointer() + (m_page_shadow[bank8] << 13);
			to[offset & 0x1fff] = data;
		}
	});
}

void specnext_state::tbblue(machine_config &config)
{
	spectrum_128(config);
	config.device_remove("exp");
	// m_ram->set_default_size("1M").set_extra_options("2M");
	m_ram->set_default_size("2M").set_default_value(0);

	Z80N(config.replace(), m_maincpu, 28_MHz_XTAL / 8);
	m_maincpu->set_daisy_config(z80_daisy_chain);
	m_maincpu->set_m1_map(&specnext_state::map_fetch);
	m_maincpu->set_memory_map(&specnext_state::map_mem);
	m_maincpu->set_io_map(&specnext_state::map_io);
	m_maincpu->set_vblank_int("screen", FUNC(specnext_state::specnext_interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(specnext_state::irq_callback));
	m_maincpu->out_nextreg_cb().set(FUNC(specnext_state::reg_w));
	m_maincpu->in_nextreg_cb().set(FUNC(specnext_state::reg_r));
	m_maincpu->out_retn_seen_cb().set(FUNC(specnext_state::leave_nmi));
	m_maincpu->nomreq_cb().set_nop();
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	SPECNEXT_CTC(config, m_ctc, 28_MHz_XTAL / 8);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SPECNEXT_DMA(config, m_dma, 28_MHz_XTAL / 8);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dma->in_mreq_callback().set([this](offs_t offset) { return m_program.read_byte(offset); });
	m_dma->out_mreq_callback().set([this](offs_t offset, u8 data) { m_program.write_byte(offset, data); });
	m_dma->in_iorq_callback().set([this](offs_t offset) { return m_io.read_byte(offset); });
	m_dma->out_iorq_callback().set([this](offs_t offset, u8 data) { m_io.write_byte(offset, data); });

	ADDRESS_MAP_BANK(config, m_regs_map).set_map(&specnext_state::map_regs).set_options(ENDIANNESS_LITTLE, 8, 8, 0);

	I2C_24C01(config, m_i2cmem).set_address(0xd0); // RTC + DEFAULT_ALL_0; confitm size

	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();
	m_sdcard->spi_miso_callback().set(FUNC(specnext_state::spi_miso_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DAC_8BIT_R2R(config, m_dac[0], 0).add_route(ALL_OUTPUTS, "lspeaker", 0.75);
	DAC_8BIT_R2R(config, m_dac[1], 0).add_route(ALL_OUTPUTS, "lspeaker", 0.75);
	DAC_8BIT_R2R(config, m_dac[2], 0).add_route(ALL_OUTPUTS, "rspeaker", 0.75);
	DAC_8BIT_R2R(config, m_dac[3], 0).add_route(ALL_OUTPUTS, "rspeaker", 0.75);

	config.device_remove("ay8912");
	for (auto i = 0; i < 3; ++i)
	{
		YM2149(config, m_ay[i], 14_MHz_XTAL / 8)
			.add_route(0, "lspeaker", 0.50)
			.add_route(1, "lspeaker", 0.25)
			.add_route(1, "rspeaker", 0.25)
			.add_route(2, "rspeaker", 0.50);
	}

	SPECNEXT_MULTIFACE(config, m_mf, 0);
	SPECNEXT_DIVMMC(config, m_divmmc, 0);

	zxbus_device &zxbus(ZXBUS(config, "zxbus", 0));
	zxbus.set_iospace("maincpu", AS_IO);
	ZXBUS_SLOT(config, "zxbus:1", 0, "zxbus", zxbus_cards, nullptr);

	const rectangle scr_full = { SCR_320x256.left() - 16, SCR_320x256.right() + 16, SCR_320x256.top() - 8, SCR_320x256.bottom() + 8 };
	m_screen->set_raw(28_MHz_XTAL / 2, CYCLES_HORIZ, CYCLES_VERT, scr_full);
	m_screen->set_screen_update(FUNC(specnext_state::screen_update));
	m_screen->set_no_palette();

	PALETTE(config.replace(), m_palette, palette_device::BLACK, 512 * 4 + 1); // ulatm, l2s, +1 == fallback
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_tbblue);

	const u16 left = SCR_256x192.left();
	const u16 top = SCR_256x192.top();
	SCREEN_ULA_NEXT (config, m_ula,     0).set_raster_offset(left, top).set_palette(m_palette->device().tag(), 0x000, 0x100);
	SPECNEXT_LORES  (config, m_lores,   0).set_raster_offset(left, top).set_palette(m_palette->device().tag(), 0x000, 0x100);
	SPECNEXT_TILES  (config, m_tiles,   0).set_raster_offset(left, top).set_palette(m_palette->device().tag(), 0x200, 0x300);
	SPECNEXT_LAYER2 (config, m_layer2,  0).set_raster_offset(left, top).set_palette(m_palette->device().tag(), 0x400, 0x500);
	SPECNEXT_SPRITES(config, m_sprites, 0).set_raster_offset(left, top).set_palette(m_palette->device().tag(), 0x600, 0x700);

	SPECNEXT_COPPER(config, m_copper, 28_MHz_XTAL);
	m_copper->out_nextreg_cb().set(FUNC(specnext_state::reg_w));
	m_copper->set_in_until_pos_cb(FUNC(specnext_state::cooper_until_pos_r));

	config.device_remove("snapshot");
}

ROM_START(tbblue)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("boot")

	ROM_SYSTEM_BIOS(0, "boot", "BootROM")
	ROMX_LOAD( "bootrom.fa55357d.bin", 0x0000, 0x2000, CRC(ccbd55ba) SHA1(8b3c2a301f486904d1c74929b94845a7731bf230), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "bootab", "BootROM - AntiBrick")
	ROMX_LOAD( "bootrom-ab.cfffa702.bin", 0x0000, 0x2000, CRC(1d16e9d4) SHA1(6f9c8771e5a9ef5a6b52a31b2e65f0698f0f5cfa), ROM_BIOS(1))
ROM_END

} // Anonymous namespace

/*    YEAR   NAME     PARENT    COMPAT  MACHINE  INPUT      CLASS            INIT         COMPANY                                            FULLNAME                     FLAGS */
COMP( 2017,  tbblue,  spec128,  0,      tbblue,  specnext,  specnext_state,  empty_init,  "SpecNext Ltd., Victor Trucco, Fabio Belavenuto",  "ZX Spectrum Next: TBBlue",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
