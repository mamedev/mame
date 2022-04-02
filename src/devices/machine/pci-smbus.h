// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_PCI_SMBUS_H
#define MAME_MACHINE_PCI_SMBUS_H

#include "pci.h"

class smbus_device : public pci_device {
public:
	smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision, uint32_t subdevice_id)
		: smbus_device(mconfig, tag, owner, clock)
	{
		set_ids(main_id, revision, 0x0c0500, subdevice_id);
	}
	smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map(address_map &map);

	uint8_t hst_sts_r();
	void hst_sts_w(uint8_t data);
	uint8_t hst_cnt_r();
	void hst_cnt_w(uint8_t data);
	uint8_t hst_cmd_r();
	void hst_cmd_w(uint8_t data);
	uint8_t xmit_slva_r();
	void xmit_slva_w(uint8_t data);
	uint8_t hst_d0_r();
	void hst_d0_w(uint8_t data);
	uint8_t hst_d1_r();
	void hst_d1_w(uint8_t data);
	uint8_t host_block_db_r();
	void host_block_db_w(uint8_t data);
	uint8_t pec_r();
	void pec_w(uint8_t data);
	uint8_t rcv_slva_r();
	void rcv_slva_w(uint8_t data);
	uint16_t slv_data_r();
	void slv_data_w(uint16_t data);
	uint8_t aux_sts_r();
	void aux_sts_w(uint8_t data);
	uint8_t aux_ctl_r();
	void aux_ctl_w(uint8_t data);
	uint8_t smlink_pin_ctl_r();
	void smlink_pin_ctl_w(uint8_t data);
	uint8_t smbus_pin_ctl_r();
	void smbus_pin_ctl_w(uint8_t data);
	uint8_t slv_sts_r();
	void slv_sts_w(uint8_t data);
	uint8_t slv_cmd_r();
	void slv_cmd_w(uint8_t data);
	uint8_t notify_daddr_r();
	uint8_t notify_dlow_r();
	uint8_t notify_dhigh_r();

	uint16_t slv_data = 0;

	uint8_t hst_sts = 0, hst_cnt = 0, hst_cmd = 0, xmit_slva = 0, hst_d0 = 0, hst_d1 = 0;
	uint8_t host_block_db = 0, pec = 0, rcv_slva = 0, aux_sts = 0, aux_ctl = 0;
	uint8_t smlink_pin_ctl = 0, smbus_pin_ctl = 0, slv_sts = 0, slv_cmd = 0, notify_daddr = 0, notify_dlow = 0, notify_dhigh = 0;
};

DECLARE_DEVICE_TYPE(SMBUS, smbus_device)

#endif // MAME_MACHINE_PCI_SMBUS_H
