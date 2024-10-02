// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_MACHINE_SIS950_SMBUS_H
#define MAME_MACHINE_SIS950_SMBUS_H

#include "emu.h"

class sis950_smbus_device : public device_t {
public:

	sis950_smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 smb_sts_r();
	void smb_sts_w(u8 data);
	u8 smbhost_cnt_r();
	void smbhost_cnt_w(u8 data);

	u8 m_status = 0;
	u8 m_cmd = 0;
};

DECLARE_DEVICE_TYPE(SIS950_SMBUS, sis950_smbus_device)

#endif // MAME_MACHINE_PCI_SMBUS_H
