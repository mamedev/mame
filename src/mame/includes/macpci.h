// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 * includes/macpci.h
 *
 * PCI-based Power Macintosh driver declarations
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_MACPCI_H
#define MAME_INCLUDES_MACPCI_H

#include "machine/8530scc.h"
#include "machine/6522via.h"
#include "machine/ram.h"
#include "machine/cuda.h"
#include "machine/ncr539x.h"
#include "sound/awacs.h"

#define C7M     (7833600)
#define C15M    (C7M*2)

#define MAC_SCREEN_NAME "screen"
#define MAC_539X_1_TAG "539x_1"
#define MAC_539X_2_TAG "539x_2"

/* Mac driver data */

class macpci_state : public driver_device
{
public:
	macpci_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via6522_0"),
		m_awacs(*this, "awacs"),
		m_cuda(*this, CUDA_TAG),
		m_ram(*this, RAM_TAG),
		m_scc(*this, "scc"),
		m_539x_1(*this, MAC_539X_1_TAG),
		m_539x_2(*this, MAC_539X_2_TAG)
	{ }

	void pippin(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via1;
	optional_device<awacs_device> m_awacs;
	required_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;
	optional_device<scc8530_legacy_device> m_scc;
	optional_device<ncr539x_device> m_539x_1;
	optional_device<ncr539x_device> m_539x_2;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	/* tells which model is being emulated (set by macxxx_init) */
	enum model_t
	{
		PCIMODEL_MAC_PM5200,
		PCIMODEL_MAC_PM6200,
		PCIMODEL_MAC_PM5300,
		PCIMODEL_MAC_PM7200,
		PCIMODEL_MAC_PM7500,
		PCIMODEL_MAC_PM8500,
		PCIMODEL_MAC_PM9500,
		PCIMODEL_MAC_PM7215,
		PCIMODEL_MAC_PM5260,
		PCIMODEL_MAC_PM5400,
		PCIMODEL_MAC_PM7600,
		PCIMODEL_MAC_PM8200,
		PCIMODEL_MAC_PM6300,
		PCIMODEL_MAC_PM6400,
		PCIMODEL_MAC_PM4400,
		PCIMODEL_MAC_PM5500,
		PCIMODEL_MAC_PM7220,
		PCIMODEL_MAC_PM7300,
		PCIMODEL_MAC_PM6500,
		PCIMODEL_MAC_PM8600,
		PCIMODEL_MAC_PM9600,

		PCIMODEL_MAC_20TH,

		PCIMODEL_MAC_G3_GOSSAMER,
		PCIMODEL_MAC_G3_ALLINONE,

		PCIMODEL_MAC_PB5x0PPC,
		PCIMODEL_MAC_PB1400,
		PCIMODEL_MAC_PB2300,
		PCIMODEL_MAC_PB2400,
		PCIMODEL_MAC_PB3400,
		PCIMODEL_MAC_PB5300,

		PCIMODEL_MAC_PBG3KANGA,
		PCIMODEL_MAC_PBG3WALLST1,
		PCIMODEL_MAC_PBG3WALLST2,

		PCIMODEL_MAC_PIPPIN    // Apple/Bandai Pippin

	};

	model_t m_model{};

	// 60.15 Hz timer for RBV/V8/Sonora/Eagle/VASP/etc.
	emu_timer *m_6015_timer = nullptr;

	// RBV and friends (V8, etc)
	uint8_t m_rbv_regs[256]{}, m_rbv_ier = 0, m_rbv_ifr = 0, m_rbv_type = 0, m_rbv_montype = 0, m_rbv_vbltime = 0;
	uint32_t m_rbv_colors[3]{}, m_rbv_count = 0, m_rbv_clutoffs = 0, m_rbv_immed10wr = 0;
	uint32_t m_rbv_palette[256]{};
	uint8_t m_sonora_vctl[8]{};
	emu_timer *m_vbl_timer = nullptr, *m_cursor_timer = nullptr;
	uint16_t m_cursor_line = 0;
	uint16_t m_dafb_int_status = 0;
	int m_dafb_scsi1_drq = 0, m_dafb_scsi2_drq = 0;
	uint8_t m_dafb_mode = 0;
	uint32_t m_dafb_base = 0, m_dafb_stride = 0;

	// this is shared among all video setups with vram
	uint32_t *m_vram = nullptr;

	uint16_t mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mac_scc_r(offs_t offset);
	void mac_scc_w(offs_t offset, uint16_t data);
	void mac_scc_2_w(offs_t offset, uint16_t data);

	uint32_t mac_read_id();

	uint8_t mac_5396_r(offs_t offset);
	void mac_5396_w(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(irq_539x_1_w);
	DECLARE_WRITE_LINE_MEMBER(drq_539x_1_w);

	DECLARE_WRITE_LINE_MEMBER(cuda_reset_w);
	DECLARE_WRITE_LINE_MEMBER(cuda_adb_linechange_w);

	// hack functions
	uint64_t unk1_r();
	uint64_t unk2_r(offs_t offset, uint64_t mem_mask = ~0);

	void init_pippin();
	void pippin_mem(address_map &map);
	void cdmcu_mem(address_map &map);
	void cdmcu_data(address_map &map);
	// wait states for accessing the VIA
	int m_via_cycles = 0;

	// hack
	uint16_t m_unk1_test = 0;

	emu_timer *m_scanline_timer = nullptr;
	uint32_t screen_update_pippin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);
	uint8_t mac_via_in_a();
	uint8_t mac_via_in_b();
	void mac_via_out_a(uint8_t data);
	void mac_via_out_b(uint8_t data);
	DECLARE_READ_LINE_MEMBER(mac_adb_via_in_cb2);
	DECLARE_WRITE_LINE_MEMBER(mac_adb_via_out_cb2);
	DECLARE_WRITE_LINE_MEMBER(mac_via_irq);
	void mac_driver_init(model_t model);
};

#endif // MAME_INCLUDES_MACPCI_H
