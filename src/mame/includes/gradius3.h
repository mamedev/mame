// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Gradius 3

*************************************************************************/
#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/konami_helper.h"

class gradius3_state : public driver_device
{
public:
	gradius3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gfxram(*this, "k052109"),
		m_gfxrom(*this, "k051960"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_gfxram;
	required_region_ptr<uint8_t> m_gfxrom;

	/* misc */
	int         m_priority;
	int         m_irqAen;
	int         m_irqBmask;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;

	uint16_t k052109_halfword_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void k052109_halfword_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t k051937_halfword_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void k051937_halfword_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t k051960_halfword_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void k051960_halfword_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cpuA_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cpuB_irqenable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cpuB_irqtrigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gradius3_gfxrom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gradius3_gfxram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_gradius3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cpuA_interrupt(device_t &device);
	void gradius3_sub_scanline(timer_device &timer, void *ptr, int32_t param);
	void gradius3_postload();
	void volume_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
};
