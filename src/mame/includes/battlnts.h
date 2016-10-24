// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Konami Battlantis Hardware

*************************************************************************/

#include "video/k007342.h"
#include "video/k007420.h"

class battlnts_state : public driver_device
{
public:
	battlnts_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007342(*this, "k007342"),
		m_k007420(*this, "k007420"),
		m_gfxdecode(*this, "gfxdecode"),
		m_rombank(*this, "rombank") { }

	/* video-related */
	int m_spritebank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007342_device> m_k007342;
	required_device<k007420_device> m_k007420;
	required_device<gfxdecode_device> m_gfxdecode;
	required_memory_bank m_rombank;

	void battlnts_sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlnts_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlnts_spritebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_battlnts(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void battlnts_interrupt(device_t &device);
	K007342_CALLBACK_MEMBER(battlnts_tile_callback);
	K007420_CALLBACK_MEMBER(battlnts_sprite_callback);
};
