#ifndef __GSTRIKER_H
#define __GSTRIKER_H

#include "video/vsystem_spr.h"

/*** VS920A **********************************************/

#define MAX_VS920A 2

struct sVS920A
{
	tilemap_t* tmap;
	UINT16* vram;
	UINT16 pal_base;
	UINT8 gfx_region;

};

/*** MB60553 **********************************************/

#define MAX_MB60553 2

struct tMB60553
{
	tilemap_t* tmap;
	UINT16* vram;
	UINT16 regs[8];
	UINT8 bank[8];
	UINT16 pal_base;
	UINT8 gfx_region;

};



class gstriker_state : public driver_device
{
public:
	gstriker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_MB60553_vram(*this, "mb60553_vram"),
		m_CG10103_vram(*this, "cg10103_vram"),
		m_VS920A_vram(*this, "vs920a_vram"),
		m_work_ram(*this, "work_ram"),
		m_lineram(*this, "lineram"),
		m_spr(*this, "vsystem_spr"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	virtual void machine_start()
	{
		m_MB60553[0].vram = m_MB60553_vram;
		m_VS920A[0].vram = m_VS920A_vram;
	}

	required_shared_ptr<UINT16> m_MB60553_vram;
	required_shared_ptr<UINT16> m_CG10103_vram;
	required_shared_ptr<UINT16> m_VS920A_vram;
	required_shared_ptr<UINT16> m_work_ram;
	required_shared_ptr<UINT16> m_lineram;
	required_device<vsystem_spr_device> m_spr;

	UINT16 m_dmmy_8f_ret;
	int m_pending_command;

	int m_gametype;
	UINT16 m_mcu_data;
	UINT16 m_prot_reg[2];

	sVS920A m_VS920A[MAX_VS920A];
	tMB60553 m_MB60553[MAX_MB60553];
	sVS920A* m_VS920A_cur_chip;
	tMB60553 *m_MB60553_cur_chip;
	DECLARE_READ16_MEMBER(dmmy_8f);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_READ16_MEMBER(pending_command_r);
	DECLARE_WRITE8_MEMBER(gs_sh_pending_command_clear_w);
	DECLARE_WRITE8_MEMBER(gs_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(twrldc94_mcu_w);
	DECLARE_READ16_MEMBER(twrldc94_mcu_r);
	DECLARE_WRITE16_MEMBER(twrldc94_prot_reg_w);
	DECLARE_READ16_MEMBER(twrldc94_prot_reg_r);
	DECLARE_READ16_MEMBER(vbl_toggle_r);
	DECLARE_WRITE16_MEMBER(vbl_toggle_w);
	DECLARE_WRITE16_MEMBER(VS920A_0_vram_w);
	DECLARE_WRITE16_MEMBER(VS920A_1_vram_w);
	DECLARE_WRITE16_MEMBER(MB60553_0_regs_w);
	DECLARE_WRITE16_MEMBER(MB60553_1_regs_w);
	DECLARE_WRITE16_MEMBER(MB60553_0_vram_w);
	DECLARE_WRITE16_MEMBER(MB60553_1_vram_w);
	DECLARE_WRITE16_MEMBER(gsx_videoram3_w);
	void MB60553_reg_written(int numchip, int num_reg);
	DECLARE_DRIVER_INIT(twrldc94a);
	DECLARE_DRIVER_INIT(vgoalsoc);
	DECLARE_DRIVER_INIT(twrldc94);
	TILE_GET_INFO_MEMBER(VS920A_get_tile_info);
	TILE_GET_INFO_MEMBER(MB60553_get_tile_info);
	TILEMAP_MAPPER_MEMBER(twc94_scan);
	DECLARE_VIDEO_START(gstriker);
	DECLARE_VIDEO_START(vgoalsoc);
	DECLARE_VIDEO_START(twrldc94);
	UINT32 screen_update_gstriker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void VS920A_init(int numchips);
	tilemap_t* VS920A_get_tilemap(int numchip);
	void VS920A_set_pal_base(int numchip, int pal_base);
	void VS920A_set_gfx_region(int numchip, int gfx_region);
	void VS920A_draw(int numchip, screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority);
	void MB60553_init(int numchips);
	void MB60553_set_pal_base(int numchip, int pal_base);
	void MB60553_set_gfx_region(int numchip, int gfx_region);
	void MB60553_draw(int numchip, screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority);
	tilemap_t* MB60553_get_tilemap(int numchip);
	void mcu_init(  );
	DECLARE_WRITE_LINE_MEMBER(gs_ym2610_irq);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

#endif
