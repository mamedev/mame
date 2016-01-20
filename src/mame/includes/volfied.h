// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Nicola Salmoria
/*************************************************************************

    Volfied

*************************************************************************/

#include "video/pc090oj.h"

class volfied_state : public driver_device
{
public:
	enum
	{
		TIMER_VOLFIED
	};

	volfied_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_pc090oj(*this, "pc090oj"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	std::unique_ptr<UINT16[]>    m_video_ram;
	std::unique_ptr<UINT8[]>    m_cchip_ram;

	/* video-related */
	UINT16      m_video_ctrl;
	UINT16      m_video_mask;

	/* c-chip */
	UINT8       m_current_bank;
	UINT8       m_current_flag;
	UINT8       m_cc_port;
	UINT8       m_current_cmd;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<pc090oj_device> m_pc090oj;
	required_device<screen_device> m_screen;

	DECLARE_WRITE16_MEMBER(volfied_cchip_ctrl_w);
	DECLARE_WRITE16_MEMBER(volfied_cchip_bank_w);
	DECLARE_WRITE16_MEMBER(volfied_cchip_ram_w);
	DECLARE_READ16_MEMBER(volfied_cchip_ctrl_r);
	DECLARE_READ16_MEMBER(volfied_cchip_ram_r);
	DECLARE_READ16_MEMBER(volfied_video_ram_r);
	DECLARE_WRITE16_MEMBER(volfied_video_ram_w);
	DECLARE_WRITE16_MEMBER(volfied_video_ctrl_w);
	DECLARE_READ16_MEMBER(volfied_video_ctrl_r);
	DECLARE_WRITE16_MEMBER(volfied_video_mask_w);
	DECLARE_WRITE16_MEMBER(volfied_sprite_ctrl_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_volfied(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(volfied_timer_callback);
	void refresh_pixel_layer( bitmap_ind16 &bitmap );
	void volfied_cchip_init(  );
	void volfied_cchip_reset(  );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
