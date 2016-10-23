// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i6300ESB southbridge

#ifndef I6300ESB_H
#define I6300ESB_H

#include "pci.h"
#include "lpc-acpi.h"
#include "lpc-rtc.h"
#include "lpc-pit.h"

#define MCFG_I6300ESB_LPC_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, I6300ESB_LPC, 0x808625a1, 0x02, 0x060100, 0x00000000)

#define MCFG_I6300ESB_WATCHDOG_ADD(_tag, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, I6300ESB_WATCHDOG, 0x808625ab, 0x02, 0x088000, _subdevice_id)

class i6300esb_lpc_device : public pci_device {
public:
	i6300esb_lpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;


protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<lpc_acpi_device> acpi;
	required_device<lpc_rtc_device> rtc;
	required_device<lpc_pit_device> pit;
	required_memory_region m_region;

	DECLARE_ADDRESS_MAP(internal_io_map, 32);

	uint32_t pmbase, gpio_base, fwh_sel1, gen_cntl, etr1, rst_cnt2, gpi_rout;
	uint16_t bios_cntl, pci_dma_cfg, gen1_dec, lpc_en, gen2_dec, fwh_sel2, func_dis, gen_pmcon_1;
	uint16_t mon_trp_rng[4], mon_trp_msk;
	uint8_t pirq_rout[8];
	uint8_t acpi_cntl, tco_cntl, gpio_cntl, serirq_cntl, d31_err_cfg, d31_err_sts, gen_sta, back_cntl, rtc_conf;
	uint8_t lpc_if_com_range, lpc_if_fdd_lpt_range, lpc_if_sound_range, fwh_dec_en1, fwh_dec_en2, siu_config_port;
	uint8_t gen_pmcon_2, gen_pmcon_3, apm_cnt, apm_sts, mon_fwd_en, nmi_sc;
	int siu_config_state;

	void nop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// configuration space registers
	uint32_t pmbase_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);               // 40
	void pmbase_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t acpi_cntl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);            // 44
	void acpi_cntl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t bios_cntl_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);            // 4e
	void bios_cntl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t tco_cntl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);             // 54
	void tco_cntl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t gpio_base_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);            // 58
	void gpio_base_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t gpio_cntl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);            // 5c
	void gpio_cntl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pirq_rout_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);            // 60-63
	void pirq_rout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t serirq_cntl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);          // 64
	void serirq_cntl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pirq2_rout_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);           // 68-6b
	void pirq2_rout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t d31_err_cfg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);          // 88
	void d31_err_cfg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t d31_err_sts_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);          // 8a
	void d31_err_sts_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t pci_dma_cfg_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);          // 90
	void pci_dma_cfg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gen_pmcon_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);          // a0
	void gen_pmcon_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t gen_pmcon_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);           // a2
	void gen_pmcon_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gen_pmcon_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);          // a4
	void gen_pmcon_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t rst_cnt2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);             // ac
	void rst_cnt2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t apm_cnt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);              // b2
	void apm_cnt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t apm_sts_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);              // b3
	void apm_sts_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t gpi_rout_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);             // b8
	void gpi_rout_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t mon_fwd_en_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);           // c0
	void mon_fwd_en_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t mon_trp_rng_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);          // c4-ca
	void mon_trp_rng_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mon_trp_msk_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);          // cc
	void mon_trp_msk_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t gen_cntl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);             // d0
	void gen_cntl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t gen_sta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);              // d4
	void gen_sta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t back_cntl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);            // d5
	void back_cntl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rtc_conf_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);             // d8
	void rtc_conf_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lpc_if_com_range_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);     // e0
	void lpc_if_com_range_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lpc_if_fdd_lpt_range_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff); // e1
	void lpc_if_fdd_lpt_range_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lpc_if_sound_range_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);   // e2
	void lpc_if_sound_range_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fwh_dec_en1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);          // e3
	void fwh_dec_en1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t gen1_dec_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);             // e4
	void gen1_dec_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t lpc_en_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);               // e6
	void lpc_en_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t fwh_sel1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);             // e8
	void fwh_sel1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t gen2_dec_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);             // ec
	void gen2_dec_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t fwh_sel2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);             // ee
	void fwh_sel2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t fwh_dec_en2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);          // f0
	void fwh_dec_en2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t func_dis_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);             // f2
	void func_dis_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t etr1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);                 // f4
	void etr1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t mfid_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);                 // f8
	uint32_t unk_fc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);               // fc
	void unk_fc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);


	// i/o space registers
	uint8_t siu_config_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);      // 4e
	void siu_config_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t siu_data_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);        // 4f
	void siu_data_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nmi_sc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);               // 61
	void nmi_sc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void map_bios(address_space *memory_space, uint32_t start, uint32_t end, int idsel);
};

class i6300esb_watchdog_device : public pci_device {
public:
	i6300esb_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);
};

extern const device_type I6300ESB_LPC;
extern const device_type I6300ESB_WATCHDOG;

#endif
