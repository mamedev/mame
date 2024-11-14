// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_PSX_MULTITAP_H
#define MAME_BUS_PSX_MULTITAP_H

#include "ctlrport.h"

void psx_controllers_nomulti(device_slot_interface &device);

class psx_multitap_device : public device_t,
							public device_psx_controller_interface
{
public:
	psx_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override { device_psx_controller_interface::m_owner->disable_card(false); }
	virtual void device_reset() override { device_psx_controller_interface::m_owner->disable_card(true); }
	virtual void interface_pre_reset() override;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	virtual bool get_pad(int count, uint8_t *odata, uint8_t idata) override;
	virtual void do_pad() override;
	void ack();
	void set_tx_line(bool tx, int port);
	bool get_rx_line(int port);

	int m_activeport;
	bool m_cack[4];
	bool m_singlemode, m_nextmode, m_tapmc;
	uint8_t m_data[3][8]; // port a is passed though
	required_device<psx_controller_port_device> m_porta;
	required_device<psx_controller_port_device> m_portb;
	required_device<psx_controller_port_device> m_portc;
	required_device<psx_controller_port_device> m_portd;
};

DECLARE_DEVICE_TYPE(PSX_MULTITAP, psx_multitap_device)

#endif // MAME_BUS_PSX_MULTITAP_H
