// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca
#include "sound/discrete.h"


/* Discrete Sound Input Nodes */
#define NORAUTP_SND_EN                  NODE_01
#define NORAUTP_FREQ_DATA               NODE_02


class norautp_state : public driver_device
{
public:
	norautp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	std::unique_ptr<uint16_t[]> m_np_vram;
	uint16_t m_np_addr;
	uint8_t test_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vram_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vram_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_addr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t test2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mainlamps_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void soundlamps_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void counterlamps_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_ssa();
	void init_enc();
	void init_deb();
	virtual void video_start() override;
	void palette_init_norautp(palette_device &palette);
	uint32_t screen_update_norautp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

/*----------- defined in audio/norautp.c -----------*/
DISCRETE_SOUND_EXTERN( norautp );
DISCRETE_SOUND_EXTERN( dphl );
DISCRETE_SOUND_EXTERN( kimble );
