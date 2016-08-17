// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef PSXMULTITAP_H_
#define PSXMULTITAP_H_

#include "ctlrport.h"

SLOT_INTERFACE_EXTERN(psx_controllers_nomulti);

class psx_multitap_device : public device_t,
							public device_psx_controller_interface
{
public:
	psx_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	virtual void device_start() override;
	virtual void device_stop() override { device_psx_controller_interface::m_owner->disable_card(false); }
	virtual void device_reset() override { device_psx_controller_interface::m_owner->disable_card(true); }
	virtual void device_config_complete() override { m_shortname = "psx_multitap"; }
	virtual void interface_pre_reset() override;

private:
	virtual bool get_pad(int count, UINT8 *odata, UINT8 idata) override;
	virtual void do_pad() override;
	void ack();
	void set_tx_line(bool tx, int port);
	bool get_rx_line(int port);

	int m_activeport;
	bool m_cack[4];
	bool m_singlemode, m_nextmode, m_tapmc;
	UINT8 m_data[3][8]; // port a is passed though
	required_device<psx_controller_port_device> m_porta;
	required_device<psx_controller_port_device> m_portb;
	required_device<psx_controller_port_device> m_portc;
	required_device<psx_controller_port_device> m_portd;
};

extern const device_type PSX_MULTITAP;

#endif /* PSXMULTITAP_H_ */
