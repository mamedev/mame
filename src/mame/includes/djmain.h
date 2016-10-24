// license:BSD-3-Clause
// copyright-holders:smf
#include "machine/ataintf.h"
#include "video/konami_helper.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k055555.h"

class djmain_state : public driver_device
{
public:
	djmain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_obj_ram(*this, "obj_ram"),
		m_maincpu(*this, "maincpu"),
		m_k056832(*this, "k056832"),
		m_k055555(*this, "k055555"),
		m_ata(*this, "ata"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_turntable(*this, {"TT1", "TT2"})
	{
	}

	int m_sndram_bank;
	uint8_t *m_sndram;
	int m_turntable_select;
	uint8_t m_turntable_last_pos[2];
	uint16_t m_turntable_pos[2];
	uint8_t m_pending_vb_int;
	uint16_t m_v_ctrl;
	uint32_t m_obj_regs[0xa0/4];
	const uint8_t *m_ata_user_password;
	const uint8_t *m_ata_master_password;
	required_shared_ptr<uint32_t> m_obj_ram;
	void sndram_bank_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t sndram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void sndram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t obj_ctrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void obj_ctrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t obj_rom_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void v_ctrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t v_rom_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint8_t inp1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t inp2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint32_t turntable_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void turntable_select_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void light_ctrl_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void light_ctrl_2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void unknown590000_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void unknown802000_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void unknownc02000_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void init_bm7thmix();
	void init_bm6thmix();
	void init_hmcompmx();
	void init_bmfinal();
	void init_hmcompm2();
	void init_bm5thmix();
	void init_bm4thmix();
	void init_beatmania();
	void init_bmdct();
	void init_bmcompm2();
	void init_bmcorerm();
	void init_bmclubmx();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_djmain(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vb_interrupt(device_t &device);
	void ide_interrupt(int state);
	void sndram_set_bank();
	void draw_sprites( bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<k055555_device> m_k055555;
	required_device<ata_interface_device> m_ata;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	K056832_CB_MEMBER(tile_callback);
	optional_ioport_array<2> m_turntable;
};
