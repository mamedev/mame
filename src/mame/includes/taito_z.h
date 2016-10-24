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
		m_steer(*this, "STEER") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	int         m_sci_spriteframe;
	int         m_road_palbank;

	/* misc */
	uint16_t      m_cpua_ctrl;
	int32_t       m_sci_int6;
	int32_t       m_ioc220_port;
	uint16_t      m_eep_latch;

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
	optional_ioport m_steer;

	void cpua_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bshark_cpua_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void chasehq_cpua_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dblaxle_cpua_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void spacegun_output_bypass_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t contcirc_input_bypass_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t chasehq_input_bypass_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint16_t bshark_stick_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t nightstr_stick_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bshark_stick_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sci_steer_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t spacegun_input_bypass_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t spacegun_lightgun_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void spacegun_lightgun_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void spacegun_gun_output_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dblaxle_steer_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t chasehq_motor_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void chasehq_motor_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nightstr_motor_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t aquajack_unknown_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void taitoz_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t taitoz_sound_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void taitoz_pancontrol(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t sci_spriteframe_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sci_spriteframe_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void contcirc_out_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value taitoz_pedal_r(ioport_field &field, void *param);
	void init_taitoz();
	void init_bshark();
	void machine_start_taitoz();
	void machine_reset_taitoz();
	void video_start_taitoz();
	void machine_start_bshark();
	uint32_t screen_update_contcirc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_chasehq(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bshark(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sci(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_aquajack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spacegun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dblaxle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_racingb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sci_interrupt(device_t &device);
	void contcirc_draw_sprites_16x8( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	void chasehq_draw_sprites_16x16( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	void bshark_draw_sprites_16x8( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	void sci_draw_sprites_16x8( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	void aquajack_draw_sprites_16x8(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int y_offs);
	void spacegun_draw_sprites_16x8(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int y_offs);
	void parse_cpu_control();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
