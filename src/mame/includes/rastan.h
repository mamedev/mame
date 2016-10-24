// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/*************************************************************************

    Rastan

*************************************************************************/
#include "sound/msm5205.h"
#include "video/pc080sn.h"
#include "video/pc090oj.h"

class rastan_state : public driver_device
{
public:
	rastan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj") { }

	/* video-related */
	uint16_t      m_sprite_ctrl;
	uint16_t      m_sprites_flipscreen;

	/* misc */
	int         m_adpcm_pos;
	int         m_adpcm_data;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<pc080sn_device> m_pc080sn;
	required_device<pc090oj_device> m_pc090oj;
	void rastan_msm5205_address_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rastan_spritectrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rastan_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rastan_msm5205_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rastan_msm5205_stop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_rastan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void rastan_msm5205_vck(int state);
};
