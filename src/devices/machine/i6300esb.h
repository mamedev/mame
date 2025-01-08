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

	virtual void config_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<lpc_acpi_device> acpi;
	required_device<lpc_rtc_device> rtc;
	required_device<lpc_pit_device> pit;
	required_memory_region m_region;

	void internal_io_map(address_map &map) ATTR_COLD;

	uint32_t pmbase, gpio_base, fwh_sel1, gen_cntl, etr1, rst_cnt2, gpi_rout;
	uint16_t bios_cntl, pci_dma_cfg, gen1_dec, lpc_en, gen2_dec, fwh_sel2, func_dis, gen_pmcon_1;
	uint16_t mon_trp_rng[4], mon_trp_msk;
	uint8_t pirq_rout[8];
	uint8_t acpi_cntl, tco_cntl, gpio_cntl, serirq_cntl, d31_err_cfg, d31_err_sts, gen_sta, back_cntl, rtc_conf;
	uint8_t lpc_if_com_range, lpc_if_fdd_lpt_range, lpc_if_sound_range, fwh_dec_en1, fwh_dec_en2, siu_config_port;
	uint8_t gen_pmcon_2, gen_pmcon_3, apm_cnt, apm_sts, mon_fwd_en, nmi_sc;
	int siu_config_state;

	void nop_w(uint8_t data);

	// configuration space registers
	uint32_t pmbase_r();                   // 40
	void pmbase_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t acpi_cntl_r();                 // 44
	void acpi_cntl_w(uint8_t data);
	uint16_t bios_cntl_r();                // 4e
	void bios_cntl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t tco_cntl_r();                  // 54
	void tco_cntl_w(uint8_t data);
	uint32_t gpio_base_r();                // 58
	void gpio_base_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t gpio_cntl_r();                 // 5c
	void gpio_cntl_w(uint8_t data);
	uint8_t pirq_rout_r(offs_t offset);    // 60-63
	void pirq_rout_w(offs_t offset, uint8_t data);
	uint8_t serirq_cntl_r();               // 64
	void serirq_cntl_w(uint8_t data);
	uint8_t pirq2_rout_r(offs_t offset);   // 68-6b
	void pirq2_rout_w(offs_t offset, uint8_t data);
	uint8_t d31_err_cfg_r();               // 88
	void d31_err_cfg_w(uint8_t data);
	uint8_t d31_err_sts_r();               // 8a
	void d31_err_sts_w(uint8_t data);
	uint16_t pci_dma_cfg_r();              // 90
	void pci_dma_cfg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gen_pmcon_1_r();              // a0
	void gen_pmcon_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t gen_pmcon_2_r();               // a2
	void gen_pmcon_2_w(uint8_t data);
	uint8_t gen_pmcon_3_r();               // a4
	void gen_pmcon_3_w(uint8_t data);
	uint32_t rst_cnt2_r();                 // ac
	void rst_cnt2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t apm_cnt_r();                   // b2
	void apm_cnt_w(uint8_t data);
	uint8_t apm_sts_r();                   // b3
	void apm_sts_w(uint8_t data);
	uint32_t gpi_rout_r();                 // b8
	void gpi_rout_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t mon_fwd_en_r();                // c0
	void mon_fwd_en_w(uint8_t data);
	uint16_t mon_trp_rng_r(offs_t offset); // c4-ca
	void mon_trp_rng_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mon_trp_msk_r();              // cc
	void mon_trp_msk_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t gen_cntl_r();                 // d0
	void gen_cntl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t gen_sta_r();                   // d4
	void gen_sta_w(uint8_t data);
	uint8_t back_cntl_r();                 // d5
	void back_cntl_w(uint8_t data);
	uint8_t rtc_conf_r();                  // d8
	void rtc_conf_w(uint8_t data);
	uint8_t lpc_if_com_range_r();          // e0
	void lpc_if_com_range_w(uint8_t data);
	uint8_t lpc_if_fdd_lpt_range_r();      // e1
	void lpc_if_fdd_lpt_range_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t lpc_if_sound_range_r();        // e2
	void lpc_if_sound_range_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t fwh_dec_en1_r();               // e3
	void fwh_dec_en1_w(uint8_t data);
	uint16_t gen1_dec_r();                 // e4
	void gen1_dec_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t lpc_en_r();                   // e6
	void lpc_en_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t fwh_sel1_r();                 // e8
	void fwh_sel1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t gen2_dec_r();                 // ec
	void gen2_dec_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t fwh_sel2_r();                 // ee
	void fwh_sel2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t fwh_dec_en2_r();               // f0
	void fwh_dec_en2_w(uint8_t data);
	uint16_t func_dis_r();                 // f2
	void func_dis_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t etr1_r();                     // f4
	void etr1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t mfid_r();                     // f8
	uint32_t unk_fc_r();                   // fc
	void unk_fc_w(uint32_t data);


	// i/o space registers
	uint8_t siu_config_port_r();      // 4e
	void siu_config_port_w(uint8_t data);
	uint8_t siu_data_port_r();        // 4f
	void siu_data_port_w(uint8_t data);
	uint8_t nmi_sc_r();               // 61
	void nmi_sc_w(uint8_t data);

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(I6300ESB_LPC,      i6300esb_lpc_device)
DECLARE_DEVICE_TYPE(I6300ESB_WATCHDOG, i6300esb_watchdog_device)

#endif // MAME_MACHINE_I6300ESB_H
