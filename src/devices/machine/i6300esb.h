// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i6300ESB southbridge

#ifndef MAME_MACHINE_I6300ESB_H
#define MAME_MACHINE_I6300ESB_H

#include "pci.h"
#include "lpc-acpi.h"
#include "lpc-rtc.h"
#include "lpc-pit.h"

class i6300esb_lpc_device : public pci_device {
public:
	i6300esb_lpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<lpc_acpi_device> acpi;
	required_device<lpc_rtc_device> rtc;
	required_device<lpc_pit_device> pit;
	required_memory_region m_region;

	void internal_io_map(address_map &map);

	uint32_t pmbase, gpio_base, fwh_sel1, gen_cntl, etr1, rst_cnt2, gpi_rout;
	uint16_t bios_cntl, pci_dma_cfg, gen1_dec, lpc_en, gen2_dec, fwh_sel2, func_dis, gen_pmcon_1;
	uint16_t mon_trp_rng[4], mon_trp_msk;
	uint8_t pirq_rout[8];
	uint8_t acpi_cntl, tco_cntl, gpio_cntl, serirq_cntl, d31_err_cfg, d31_err_sts, gen_sta, back_cntl, rtc_conf;
	uint8_t lpc_if_com_range, lpc_if_fdd_lpt_range, lpc_if_sound_range, fwh_dec_en1, fwh_dec_en2, siu_config_port;
	uint8_t gen_pmcon_2, gen_pmcon_3, apm_cnt, apm_sts, mon_fwd_en, nmi_sc;
	int siu_config_state;

	DECLARE_WRITE8_MEMBER (nop_w);

	// configuration space registers
	DECLARE_READ32_MEMBER (pmbase_r);               // 40
	DECLARE_WRITE32_MEMBER(pmbase_w);
	DECLARE_READ8_MEMBER  (acpi_cntl_r);            // 44
	DECLARE_WRITE8_MEMBER (acpi_cntl_w);
	DECLARE_READ16_MEMBER (bios_cntl_r);            // 4e
	DECLARE_WRITE16_MEMBER(bios_cntl_w);
	DECLARE_READ8_MEMBER  (tco_cntl_r);             // 54
	DECLARE_WRITE8_MEMBER (tco_cntl_w);
	DECLARE_READ32_MEMBER (gpio_base_r);            // 58
	DECLARE_WRITE32_MEMBER(gpio_base_w);
	DECLARE_READ8_MEMBER  (gpio_cntl_r);            // 5c
	DECLARE_WRITE8_MEMBER (gpio_cntl_w);
	DECLARE_READ8_MEMBER  (pirq_rout_r);            // 60-63
	DECLARE_WRITE8_MEMBER (pirq_rout_w);
	DECLARE_READ8_MEMBER  (serirq_cntl_r);          // 64
	DECLARE_WRITE8_MEMBER (serirq_cntl_w);
	DECLARE_READ8_MEMBER  (pirq2_rout_r);           // 68-6b
	DECLARE_WRITE8_MEMBER (pirq2_rout_w);
	DECLARE_READ8_MEMBER  (d31_err_cfg_r);          // 88
	DECLARE_WRITE8_MEMBER (d31_err_cfg_w);
	DECLARE_READ8_MEMBER  (d31_err_sts_r);          // 8a
	DECLARE_WRITE8_MEMBER (d31_err_sts_w);
	DECLARE_READ16_MEMBER (pci_dma_cfg_r);          // 90
	DECLARE_WRITE16_MEMBER(pci_dma_cfg_w);
	DECLARE_READ16_MEMBER (gen_pmcon_1_r);          // a0
	DECLARE_WRITE16_MEMBER(gen_pmcon_1_w);
	DECLARE_READ8_MEMBER (gen_pmcon_2_r);           // a2
	DECLARE_WRITE8_MEMBER(gen_pmcon_2_w);
	DECLARE_READ8_MEMBER  (gen_pmcon_3_r);          // a4
	DECLARE_WRITE8_MEMBER (gen_pmcon_3_w);
	DECLARE_READ32_MEMBER (rst_cnt2_r);             // ac
	DECLARE_WRITE32_MEMBER(rst_cnt2_w);
	DECLARE_READ8_MEMBER  (apm_cnt_r);              // b2
	DECLARE_WRITE8_MEMBER (apm_cnt_w);
	DECLARE_READ8_MEMBER  (apm_sts_r);              // b3
	DECLARE_WRITE8_MEMBER (apm_sts_w);
	DECLARE_READ32_MEMBER (gpi_rout_r);             // b8
	DECLARE_WRITE32_MEMBER(gpi_rout_w);
	DECLARE_READ8_MEMBER  (mon_fwd_en_r);           // c0
	DECLARE_WRITE8_MEMBER (mon_fwd_en_w);
	DECLARE_READ16_MEMBER (mon_trp_rng_r);          // c4-ca
	DECLARE_WRITE16_MEMBER(mon_trp_rng_w);
	DECLARE_READ16_MEMBER (mon_trp_msk_r);          // cc
	DECLARE_WRITE16_MEMBER(mon_trp_msk_w);
	DECLARE_READ32_MEMBER (gen_cntl_r);             // d0
	DECLARE_WRITE32_MEMBER(gen_cntl_w);
	DECLARE_READ8_MEMBER  (gen_sta_r);              // d4
	DECLARE_WRITE8_MEMBER (gen_sta_w);
	DECLARE_READ8_MEMBER  (back_cntl_r);            // d5
	DECLARE_WRITE8_MEMBER (back_cntl_w);
	DECLARE_READ8_MEMBER  (rtc_conf_r);             // d8
	DECLARE_WRITE8_MEMBER (rtc_conf_w);
	DECLARE_READ8_MEMBER  (lpc_if_com_range_r);     // e0
	DECLARE_WRITE8_MEMBER (lpc_if_com_range_w);
	DECLARE_READ8_MEMBER  (lpc_if_fdd_lpt_range_r); // e1
	DECLARE_WRITE8_MEMBER (lpc_if_fdd_lpt_range_w);
	DECLARE_READ8_MEMBER  (lpc_if_sound_range_r);   // e2
	DECLARE_WRITE8_MEMBER (lpc_if_sound_range_w);
	DECLARE_READ8_MEMBER  (fwh_dec_en1_r);          // e3
	DECLARE_WRITE8_MEMBER (fwh_dec_en1_w);
	DECLARE_READ16_MEMBER (gen1_dec_r);             // e4
	DECLARE_WRITE16_MEMBER(gen1_dec_w);
	DECLARE_READ16_MEMBER (lpc_en_r);               // e6
	DECLARE_WRITE16_MEMBER(lpc_en_w);
	DECLARE_READ32_MEMBER (fwh_sel1_r);             // e8
	DECLARE_WRITE32_MEMBER(fwh_sel1_w);
	DECLARE_READ16_MEMBER (gen2_dec_r);             // ec
	DECLARE_WRITE16_MEMBER(gen2_dec_w);
	DECLARE_READ16_MEMBER (fwh_sel2_r);             // ee
	DECLARE_WRITE16_MEMBER(fwh_sel2_w);
	DECLARE_READ8_MEMBER  (fwh_dec_en2_r);          // f0
	DECLARE_WRITE8_MEMBER (fwh_dec_en2_w);
	DECLARE_READ16_MEMBER (func_dis_r);             // f2
	DECLARE_WRITE16_MEMBER(func_dis_w);
	DECLARE_READ32_MEMBER (etr1_r);                 // f4
	DECLARE_WRITE32_MEMBER(etr1_w);
	DECLARE_READ32_MEMBER (mfid_r);                 // f8
	DECLARE_READ32_MEMBER (unk_fc_r);               // fc
	DECLARE_WRITE32_MEMBER(unk_fc_w);


	// i/o space registers
	DECLARE_READ8_MEMBER  (siu_config_port_r);      // 4e
	DECLARE_WRITE8_MEMBER (siu_config_port_w);
	DECLARE_READ8_MEMBER  (siu_data_port_r);        // 4f
	DECLARE_WRITE8_MEMBER (siu_data_port_w);
	DECLARE_READ8_MEMBER  (nmi_sc_r);               // 61
	DECLARE_WRITE8_MEMBER (nmi_sc_w);

	void map_bios(address_space *memory_space, uint32_t start, uint32_t end, int idsel);
};

class i6300esb_watchdog_device : public pci_device {
public:
	i6300esb_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subdevice_id)
		: i6300esb_watchdog_device(mconfig, tag, owner, clock)
	{
		set_ids(0x808625ab, 0x02, 0x088000, subdevice_id);
	}
	i6300esb_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map(address_map &map);
};

DECLARE_DEVICE_TYPE(I6300ESB_LPC,      i6300esb_lpc_device)
DECLARE_DEVICE_TYPE(I6300ESB_WATCHDOG, i6300esb_watchdog_device)

#endif // MAME_MACHINE_I6300ESB_H
