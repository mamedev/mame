// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************


    Taito Z system

*************************************************************************/

#include "audio/taitosnd.h"
#include "machine/eepromser.h"
#include "machine/taitoio.h"
#include "video/tc0100scn.h"
#include "video/tc0110pcr.h"
#include "video/tc0150rod.h"
#include "video/tc0480scp.h"


class taitoz_state : public driver_device
{
public:
	enum
	{
		TIMER_TAITOZ_INTERRUPT6,
		TIMER_TAITOZ_CPUB_INTERRUPT5
	};

	taitoz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_eeprom(*this, "eeprom"),
		m_tc0480scp(*this, "tc0480scp"),
		m_tc0150rod(*this, "tc0150rod"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0110pcr(*this, "tc0110pcr"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_tc0510nio(*this, "tc0510nio"),
		m_tc0140syt(*this, "tc0140syt"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	int         m_sci_spriteframe;
	int         m_road_palbank;

	/* misc */
	UINT16      m_cpua_ctrl;
	INT32       m_sci_int6;
	INT32       m_ioc220_port;
	UINT16      m_eep_latch;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<tc0480scp_device> m_tc0480scp;
	optional_device<tc0150rod_device> m_tc0150rod;
	optional_device<tc0100scn_device> m_tc0100scn;
	optional_device<tc0110pcr_device> m_tc0110pcr;
	optional_device<tc0220ioc_device> m_tc0220ioc;
	optional_device<tc0510nio_device> m_tc0510nio;
	optional_device<tc0140syt_device> m_tc0140syt;  // bshark & spacegun miss the CPUs which shall use TC0140
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_WRITE16_MEMBER(chasehq_cpua_ctrl_w);
	DECLARE_WRITE16_MEMBER(dblaxle_cpua_ctrl_w);
	DECLARE_READ16_MEMBER(eep_latch_r);
	DECLARE_WRITE16_MEMBER(spacegun_output_bypass_w);
	DECLARE_READ8_MEMBER(contcirc_input_bypass_r);
	DECLARE_READ8_MEMBER(chasehq_input_bypass_r);
	DECLARE_READ16_MEMBER(bshark_stick_r);
	DECLARE_READ16_MEMBER(nightstr_stick_r);
	DECLARE_WRITE16_MEMBER(bshark_stick_w);
	DECLARE_READ16_MEMBER(sci_steer_input_r);
	DECLARE_READ16_MEMBER(spacegun_input_bypass_r);
	DECLARE_READ16_MEMBER(spacegun_lightgun_r);
	DECLARE_WRITE16_MEMBER(spacegun_lightgun_w);
	DECLARE_WRITE16_MEMBER(spacegun_gun_output_w);
	DECLARE_READ16_MEMBER(dblaxle_steer_input_r);
	DECLARE_READ16_MEMBER(chasehq_motor_r);
	DECLARE_WRITE16_MEMBER(chasehq_motor_w);
	DECLARE_WRITE16_MEMBER(nightstr_motor_w);
	DECLARE_READ16_MEMBER(aquajack_unknown_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(taitoz_sound_w);
	DECLARE_READ16_MEMBER(taitoz_sound_r);
	DECLARE_WRITE16_MEMBER(taitoz_msb_sound_w);
	DECLARE_READ16_MEMBER(taitoz_msb_sound_r);
	DECLARE_WRITE8_MEMBER(taitoz_pancontrol);
	DECLARE_READ16_MEMBER(sci_spriteframe_r);
	DECLARE_WRITE16_MEMBER(sci_spriteframe_w);
	DECLARE_WRITE16_MEMBER(contcirc_out_w);
	DECLARE_CUSTOM_INPUT_MEMBER(taitoz_pedal_r);
	DECLARE_DRIVER_INIT(taitoz);
	DECLARE_DRIVER_INIT(bshark);
	DECLARE_MACHINE_START(taitoz);
	DECLARE_MACHINE_RESET(taitoz);
	DECLARE_VIDEO_START(taitoz);
	DECLARE_MACHINE_START(bshark);
	UINT32 screen_update_contcirc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_chasehq(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bshark(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sci(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_aquajack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spacegun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_dblaxle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_racingb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sci_interrupt);
	void contcirc_draw_sprites_16x8( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	void chasehq_draw_sprites_16x16( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	void bshark_draw_sprites_16x8( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	void sci_draw_sprites_16x8( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	void aquajack_draw_sprites_16x8(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int y_offs);
	void spacegun_draw_sprites_16x8(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int y_offs);
	void parse_cpu_control(  );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	DECLARE_WRITE_LINE_MEMBER(irqhandlerb);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
