#ifndef __GSTRIKER_H
#define __GSTRIKER_H

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

/*** CG10103 **********************************************/

#define MAX_CG10103 2

struct tCG10103 
{
	UINT16* vram;
	UINT16 pal_base;
	UINT8 gfx_region;
	UINT8 transpen;

};

class gstriker_state : public driver_device
{
public:
	gstriker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_MB60553_vram(*this, "mb60553_vram"),
		m_CG10103_vram(*this, "cg10103_vram"),
		m_VS920A_vram(*this, "vs920a_vram"),
		m_work_ram(*this, "work_ram"),
		m_lineram(*this, "lineram"){ }

	virtual void machine_start()
	{
		m_MB60553[0].vram = m_MB60553_vram;
		m_CG10103[0].vram = m_CG10103_vram;
		m_VS920A[0].vram = m_VS920A_vram;
	}

	required_shared_ptr<UINT16> m_MB60553_vram;
	required_shared_ptr<UINT16> m_CG10103_vram;
	required_shared_ptr<UINT16> m_VS920A_vram;
	UINT16 m_dmmy_8f_ret;
	int m_pending_command;
	required_shared_ptr<UINT16> m_work_ram;
	int m_gametype;
	UINT16 m_mcu_data;
	UINT16 m_prot_reg[2];
	required_shared_ptr<UINT16> m_lineram;
	sVS920A m_VS920A[MAX_VS920A];
	tMB60553 m_MB60553[MAX_MB60553];
	tCG10103 m_CG10103[MAX_CG10103];
	sVS920A* m_VS920A_cur_chip;
	tMB60553 *m_MB60553_cur_chip;
	tCG10103* m_CG10103_cur_chip;
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
};


/*----------- defined in video/gstriker.c -----------*/


SCREEN_UPDATE_IND16( gstriker );



#endif
