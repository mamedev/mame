/*************************************************************************

    Top Speed / Full Throttle

*************************************************************************/

#include "sound/msm5205.h"
#include "machine/taitoio.h"
#include "video/pc080sn.h"

class topspeed_state : public driver_device
{
public:
	enum
	{
		TIMER_TOPSPEED_INTERRUPT6,
		TIMER_TOPSPEED_CPUB_INTERRUPT6
	};

	topspeed_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spritemap(*this, "spritemap"),
		m_raster_ctrl(*this, "raster_ctrl"),
		m_spriteram(*this, "spriteram"),
		m_sharedram(*this, "sharedram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "subcpu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_pc080sn_1(*this, "pc080sn_1"),
		m_pc080sn_2(*this, "pc080sn_2"),
		m_tc0220ioc(*this, "tc0220ioc") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spritemap;
	required_shared_ptr<UINT16> m_raster_ctrl;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_sharedram;

	/* adpcm */
	UINT8 *m_msm_rom[2];
	UINT16 m_msm_start[2];
	UINT16 m_msm_loop[2];
	UINT16 m_msm_pos[2];
	UINT8 m_msm_sel[2];

	/* misc */
	UINT16     m_cpua_ctrl;
	INT32      m_ioc220_port;
	INT32      m_banknum;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<pc080sn_device> m_pc080sn_1;
	required_device<pc080sn_device> m_pc080sn_2;
	required_device<tc0220ioc_device> m_tc0220ioc;

	UINT8 m_dislayer[5];
	DECLARE_READ16_MEMBER(sharedram_r);
	DECLARE_WRITE16_MEMBER(sharedram_w);
	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_READ8_MEMBER(topspeed_input_bypass_r);
	DECLARE_READ16_MEMBER(topspeed_motor_r);
	DECLARE_WRITE16_MEMBER(topspeed_motor_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(topspeed_msm5205_command_w);
	DECLARE_CUSTOM_INPUT_MEMBER(topspeed_pedal_r);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_topspeed(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(topspeed_interrupt);
	INTERRUPT_GEN_MEMBER(topspeed_cpub_interrupt);
	void topspeed_postload();
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void parse_control(  )   /* assumes Z80 sandwiched between 68Ks */;
	void reset_sound_region(  );
	void topspeed_msm5205_clock(int chip);
	DECLARE_WRITE_LINE_MEMBER(topspeed_msm5205_vck_1);
	DECLARE_WRITE_LINE_MEMBER(topspeed_msm5205_vck_2);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
