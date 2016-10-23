// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef PCI_SMBUS_H
#define PCI_SMBUS_H

#include "pci.h"

#define MCFG_SMBUS_ADD(_tag, _main_id, _revision, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, SMBUS, _main_id, _revision, 0x0c0500, _subdevice_id)

class smbus_device : public pci_device {
public:
	smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t hst_sts_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hst_sts_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hst_cnt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hst_cnt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hst_cmd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hst_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t xmit_slva_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void xmit_slva_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hst_d0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hst_d0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hst_d1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hst_d1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t host_block_db_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void host_block_db_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pec_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pec_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rcv_slva_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rcv_slva_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t slv_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void slv_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t aux_sts_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void aux_sts_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t aux_ctl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void aux_ctl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t smlink_pin_ctl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void smlink_pin_ctl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t smbus_pin_ctl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void smbus_pin_ctl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t slv_sts_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void slv_sts_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t slv_cmd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void slv_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t notify_daddr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t notify_dlow_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t notify_dhigh_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);

	uint16_t slv_data;

	uint8_t hst_sts, hst_cnt, hst_cmd, xmit_slva, hst_d0, hst_d1;
	uint8_t host_block_db, pec, rcv_slva, aux_sts, aux_ctl;
	uint8_t smlink_pin_ctl, smbus_pin_ctl, slv_sts, slv_cmd, notify_daddr, notify_dlow, notify_dhigh;
};

extern const device_type SMBUS;

#endif
