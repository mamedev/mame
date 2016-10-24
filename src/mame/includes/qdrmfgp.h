// license:BSD-3-Clause
// copyright-holders:Hau
#include "machine/ataintf.h"
#include "sound/k054539.h"
#include "machine/k053252.h"
#include "video/konami_helper.h"
#include "video/k054156_k054157_k056832.h"

class qdrmfgp_state : public driver_device
{
public:
	qdrmfgp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_workram(*this, "workram"),
		m_k056832(*this, "k056832"),
		m_k054539(*this, "k054539"),
		m_k053252(*this, "k053252"),
		m_ata(*this, "ata"),
		m_inputs_port(*this, "INPUTS"),
		m_dsw_port(*this, "DSW"),
		m_palette(*this, "palette")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_nvram;
	required_shared_ptr<uint16_t> m_workram;
	required_device<k056832_device> m_k056832;
	required_device<k054539_device> m_k054539;
	required_device<k053252_device> m_k053252;
	required_device<ata_interface_device> m_ata;
	required_ioport m_inputs_port;
	required_ioport m_dsw_port;
	required_device<palette_device> m_palette;

	uint8_t *m_sndram;
	uint16_t m_control;
	int32_t m_gp2_irq_control;
	int32_t m_pal;

	void gp_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gp2_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t v_rom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t gp2_vram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t gp2_vram_mirror_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gp2_vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gp2_vram_mirror_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sndram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sndram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gp2_ide_std_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t inputs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	ioport_value battery_sensor_r(ioport_field &field, void *param);

	virtual void machine_reset() override;

	void machine_start_qdrmfgp();
	void video_start_qdrmfgp();
	void machine_start_qdrmfgp2();
	void video_start_qdrmfgp2();
	uint32_t screen_update_qdrmfgp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void qdrmfgp2_interrupt(device_t &device);
	void gp2_timer_callback(void *ptr, int32_t param);
	void qdrmfgp_interrupt(timer_device &timer, void *ptr, int32_t param);
	void ide_interrupt(int state);
	void gp2_ide_interrupt(int state);
	void k054539_irq1_gen(int state);
	K056832_CB_MEMBER(qdrmfgp_tile_callback);
	K056832_CB_MEMBER(qdrmfgp2_tile_callback);
};
