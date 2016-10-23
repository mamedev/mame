// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Manuel Abadia
/*************************************************************************

    Chequered Flag

*************************************************************************/
#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "sound/k007232.h"
#include "video/k051960.h"
#include "video/k051316.h"
#include "video/k051733.h"
#include "video/konami_helper.h"

class chqflag_state : public driver_device
{
public:
	chqflag_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bank1000(*this, "bank1000"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k051960(*this, "k051960"),
		m_k051316_1(*this, "k051316_1"),
		m_k051316_2(*this, "k051316_2"),
		m_palette(*this, "palette"),
		m_soundlatch2(*this, "soundlatch2"),
		m_rombank(*this, "rombank") { }

	/* misc */
	int        m_k051316_readroms;
	int        m_last_vreg;
	int        m_analog_ctrl;
	int        m_accel;
	int        m_wheel;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<address_map_bank_device> m_bank1000;
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k051960_device> m_k051960;
	required_device<k051316_device> m_k051316_1;
	required_device<k051316_device> m_k051316_2;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch2;

	/* memory pointers */
	required_memory_bank m_rombank;

	uint8_t k051316_1_ramrom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t k051316_2_ramrom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void chqflag_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void chqflag_vreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void select_analog_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t analog_read_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void chqflag_sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void k007232_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void k007232_extvolume_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_chqflag(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void volume_callback0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void volume_callback1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	K051316_CB_MEMBER(zoom_callback_1);
	K051316_CB_MEMBER(zoom_callback_2);
	K051960_CB_MEMBER(sprite_callback);
};
