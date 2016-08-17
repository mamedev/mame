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

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_video_enable;
	required_shared_ptr<UINT8> m_spriteram;

	UINT8 m_irq_mask;
	UINT8 m_ls259_buf[8];
	UINT8 m_p1_res;
	UINT8 m_p1_old_val;
	UINT8 m_p2_res;
	UINT8 m_p2_old_val;

	/*table holds outputs of all ANDs (after AND map)*/
	UINT8 m_andmap[64];

	/*table holds inputs (ie. not x, x, not q, q) to the AND map*/
	UINT8 m_columnvalue[32];

	/*8 output pins (actually 6 output and 2 input/output)*/
	UINT8 m_outvalue[8];

	tilemap_t *m_bg_tilemap;

	// common
	DECLARE_WRITE8_MEMBER(coincounter_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);

	// bagman
	DECLARE_WRITE8_MEMBER(ls259_w);
	DECLARE_WRITE8_MEMBER(pal16r6_w);
	DECLARE_READ8_MEMBER(pal16r6_r);

	// squaitsa
	DECLARE_READ8_MEMBER(dial_input_p1_r);
	DECLARE_READ8_MEMBER(dial_input_p2_r);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	DECLARE_MACHINE_START(bagman);
	DECLARE_MACHINE_START(squaitsa);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(bagman);
	DECLARE_DRIVER_INIT(bagman);

	INTERRUPT_GEN_MEMBER(vblank_irq);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
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
