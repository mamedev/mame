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

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:

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

	uint16_t slv_data;

	uint8_t hst_sts, hst_cnt, hst_cmd, xmit_slva, hst_d0, hst_d1;
	uint8_t host_block_db, pec, rcv_slva, aux_sts, aux_ctl;
	uint8_t smlink_pin_ctl, smbus_pin_ctl, slv_sts, slv_cmd, notify_daddr, notify_dlow, notify_dhigh;
};

DECLARE_DEVICE_TYPE(SMBUS, smbus_device)

#endif // MAME_MACHINE_PCI_SMBUS_H
