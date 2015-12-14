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
		m_tilemap(*this, "tilemap"),
		m_mixer(*this, "mixer"),
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
	DECLARE_WRITE32_MEMBER(sndram_bank_w);
	DECLARE_READ32_MEMBER(sndram_r);
	DECLARE_WRITE32_MEMBER(sndram_w);
	DECLARE_READ32_MEMBER(obj_ctrl_r);
	DECLARE_WRITE32_MEMBER(obj_ctrl_w);
	DECLARE_READ32_MEMBER(obj_rom_r);
	DECLARE_WRITE32_MEMBER(v_ctrl_w);
	DECLARE_READ32_MEMBER(v_rom_r);
	DECLARE_READ8_MEMBER(inp1_r);
	DECLARE_READ8_MEMBER(inp2_r);
	DECLARE_READ32_MEMBER(turntable_r);
	DECLARE_WRITE32_MEMBER(turntable_select_w);
	DECLARE_WRITE32_MEMBER(light_ctrl_1_w);
	DECLARE_WRITE32_MEMBER(light_ctrl_2_w);
	DECLARE_WRITE32_MEMBER(unknown590000_w);
	DECLARE_WRITE32_MEMBER(unknown802000_w);
	DECLARE_WRITE32_MEMBER(unknownc02000_w);
	DECLARE_DRIVER_INIT(bm7thmix);
	DECLARE_DRIVER_INIT(bm6thmix);
	DECLARE_DRIVER_INIT(hmcompmx);
	DECLARE_DRIVER_INIT(bmfinal);
	DECLARE_DRIVER_INIT(hmcompm2);
	DECLARE_DRIVER_INIT(bm5thmix);
	DECLARE_DRIVER_INIT(bm4thmix);
	DECLARE_DRIVER_INIT(beatmania);
	DECLARE_DRIVER_INIT(bmdct);
	DECLARE_DRIVER_INIT(bmcompm2);
	DECLARE_DRIVER_INIT(bmcorerm);
	DECLARE_DRIVER_INIT(bmclubmx);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_djmain(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vb_interrupt);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	void sndram_set_bank();
	void draw_sprites( bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<k058143_056832_device> m_tilemap;
	required_device<k055555_device> m_mixer;
	required_device<ata_interface_device> m_ata;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_ioport_array<2> m_turntable;
};
