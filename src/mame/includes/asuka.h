// license:BSD-3-Clause
// copyright-holders:David Graves, Brian Troha
/*************************************************************************

    Asuka & Asuka  (+ Taito/Visco games on similar hardware)

*************************************************************************/

#include "machine/taitoio.h"
#include "sound/msm5205.h"
#include "video/pc090oj.h"
#include "video/tc0100scn.h"
#include "video/tc0110pcr.h"

class asuka_state : public driver_device
{
public:
	enum
	{
		TIMER_CADASH_INTERRUPT5
	};

	asuka_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cadash_shared_ram(*this, "sharedram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_pc090oj(*this, "pc090oj"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0110pcr(*this, "tc0110pcr"),
		m_tc0220ioc(*this, "tc0220ioc") { }

	/* video-related */
	uint16_t      m_video_ctrl;
	uint16_t      m_video_mask;

	/* c-chip */
	int         m_current_round;
	int         m_current_bank;

	uint8_t       m_cval[26];
	uint8_t       m_cc_port;
	uint8_t       m_restart_status;

	/* misc */
	int         m_adpcm_pos;
	int         m_adpcm_data;

	optional_shared_ptr<uint8_t> m_cadash_shared_ram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<pc090oj_device> m_pc090oj;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0110pcr_device> m_tc0110pcr;
	optional_device<tc0220ioc_device> m_tc0220ioc;
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void asuka_msm5205_address_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t cadash_share_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cadash_share_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void asuka_spritectrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_bankswitch_2151_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void asuka_msm5205_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void asuka_msm5205_stop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_bonzeadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_asuka(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_asuka(screen_device &screen, bool state);
	void cadash_interrupt(device_t &device);

	/*----------- defined in machine/bonzeadv.c -----------*/
	void WriteLevelData();
	void WriteRestartPos(int level );

	uint16_t bonzeadv_cchip_ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t bonzeadv_cchip_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bonzeadv_cchip_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bonzeadv_cchip_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bonzeadv_cchip_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void asuka_msm5205_vck(int state);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
