// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/tms5110.h"

class bagman_state : public driver_device
{
public:
	bagman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_tmsprom(*this, "tmsprom"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_video_enable(*this, "video_enable"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<tmsprom_device> m_tmsprom;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_video_enable;
	required_shared_ptr<uint8_t> m_spriteram;

	uint8_t m_irq_mask;
	uint8_t m_ls259_buf[8];
	uint8_t m_p1_res;
	uint8_t m_p1_old_val;
	uint8_t m_p2_res;
	uint8_t m_p2_old_val;

	/*table holds outputs of all ANDs (after AND map)*/
	uint8_t m_andmap[64];

	/*table holds inputs (ie. not x, x, not q, q) to the AND map*/
	uint8_t m_columnvalue[32];

	/*8 output pins (actually 6 output and 2 input/output)*/
	uint8_t m_outvalue[8];

	tilemap_t *m_bg_tilemap;

	// common
	void coincounter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// bagman
	void ls259_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pal16r6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pal16r6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// squaitsa
	uint8_t dial_input_p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dial_input_p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	void machine_start_bagman();
	void machine_start_squaitsa();
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_bagman(palette_device &palette);
	void init_bagman();

	void vblank_irq(device_t &device);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_pal();
};

/*----------- timings -----------*/

#define BAGMAN_MAIN_CLOCK   XTAL_18_432MHz
#define BAGMAN_HCLK         (BAGMAN_MAIN_CLOCK / 3)
#define BAGMAN_H0           (BAGMAN_HCLK / 2)
#define BAGMAN_H1           (BAGMAN_H0   / 2)
#define HTOTAL              ((0x100-0x40)*2)
#define HBEND               (0x00)
#define HBSTART             (0x100)
#define VTOTAL              ((0x100-0x7c)*2)

/* the following VBEND/VBSTART are used for compsync
 * #define VBEND                (0x08)
 * #define VBSTART              (0x100)
 *
 * However VBSYQ (and INTQ) is generated using the following values:
 */
#define VBEND               (0x10)
#define VBSTART             (0xf0)
