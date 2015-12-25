// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 * includes/macpci.h
 *
 * PCI-based Power Macintosh driver declarations
 *
 ****************************************************************************/

#ifndef MACPCI_H_
#define MACPCI_H_

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


/* Mac driver data */

class macpci_state : public driver_device
{
public:
	macpci_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via6522_0"),
		m_awacs(*this, "awacs"),
		m_cuda(*this, CUDA_TAG),
		m_ram(*this, RAM_TAG),
		m_539x_1(*this, MAC_539X_1_TAG),
		m_539x_2(*this, MAC_539X_2_TAG)
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via1;
	optional_device<awacs_device> m_awacs;
	required_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;
	optional_device<ncr539x_device> m_539x_1;
	optional_device<ncr539x_device> m_539x_2;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	model_t m_model;

	// 60.15 Hz timer for RBV/V8/Sonora/Eagle/VASP/etc.
	emu_timer *m_6015_timer;

	// RBV and friends (V8, etc)
	UINT8 m_rbv_regs[256], m_rbv_ier, m_rbv_ifr, m_rbv_type, m_rbv_montype, m_rbv_vbltime;
	UINT32 m_rbv_colors[3], m_rbv_count, m_rbv_clutoffs, m_rbv_immed10wr;
	UINT32 m_rbv_palette[256];
	UINT8 m_sonora_vctl[8];
	emu_timer *m_vbl_timer, *m_cursor_timer;
	UINT16 m_cursor_line;
	UINT16 m_dafb_int_status;
	int m_dafb_scsi1_drq, m_dafb_scsi2_drq;
	UINT8 m_dafb_mode;
	UINT32 m_dafb_base, m_dafb_stride;

	// this is shared among all video setups with vram
	UINT32 *m_vram;

	// defined in machine/mac.c
	void set_via_interrupt(int value);
	void vblank_irq();
	void mac_adb_newaction(int state);

	DECLARE_READ16_MEMBER ( mac_via_r );
	DECLARE_WRITE16_MEMBER ( mac_via_w );
	DECLARE_READ16_MEMBER ( mac_scc_r );
	DECLARE_WRITE16_MEMBER ( mac_scc_w );
	DECLARE_WRITE16_MEMBER ( mac_scc_2_w );

	DECLARE_READ32_MEMBER(mac_read_id);

	DECLARE_READ8_MEMBER(mac_5396_r);
	DECLARE_WRITE8_MEMBER(mac_5396_w);

	DECLARE_WRITE_LINE_MEMBER(irq_539x_1_w);
	DECLARE_WRITE_LINE_MEMBER(drq_539x_1_w);

	DECLARE_WRITE_LINE_MEMBER(cuda_reset_w);
	DECLARE_WRITE_LINE_MEMBER(cuda_adb_linechange_w);

	// hack functions
	DECLARE_READ64_MEMBER ( unk1_r );
	DECLARE_READ64_MEMBER ( unk2_r );

	DECLARE_DRIVER_INIT(pippin);
private:
	// wait states for accessing the VIA
	int m_via_cycles;

	// hack
	UINT16 m_unk1_test;

public:
	emu_timer *m_scanline_timer;
	UINT32 screen_update_pippin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);
	DECLARE_READ8_MEMBER(mac_via_in_a);
	DECLARE_READ8_MEMBER(mac_via_in_b);
	DECLARE_WRITE8_MEMBER(mac_via_out_a);
	DECLARE_WRITE8_MEMBER(mac_via_out_b);
	DECLARE_READ_LINE_MEMBER(mac_adb_via_in_cb2);
	DECLARE_WRITE_LINE_MEMBER(mac_adb_via_out_cb2);
	DECLARE_WRITE_LINE_MEMBER(mac_via_irq);
	void mac_driver_init(model_t model);
};

#endif /* PCIMAC_H_ */
