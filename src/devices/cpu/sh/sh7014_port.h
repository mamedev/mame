// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 Bus State Controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH7014_PORT_H
#define MAME_CPU_SH_SH7014_PORT_H

#pragma once


class sh7014_port_device : public device_t
{
public:
	sh7014_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto port_a_read_callback() { return m_port_a_read_cb.bind(); }
	auto port_a_write_callback() { return m_port_a_write_cb.bind(); }

	auto port_b_read_callback() { return m_port_b_read_cb.bind(); }
	auto port_b_write_callback() { return m_port_b_write_cb.bind(); }

	auto port_e_read_callback() { return m_port_e_read_cb.bind(); }
	auto port_e_write_callback() { return m_port_e_write_cb.bind(); }

	auto port_f_read_callback() { return m_port_f_read_cb.bind(); }

	// port A
	uint16_t padrl_r();
	void padrl_w(uint16_t data);

	uint16_t paiorl_r();
	void paiorl_w(uint16_t data);

	uint16_t pacrl1_r();
	void pacrl1_w(uint16_t data);

	uint16_t pacrl2_r();
	void pacrl2_w(uint16_t data);

	// port b
	uint16_t pbdr_r();
	void pbdr_w(uint16_t data);

	uint16_t pbior_r();
	void pbior_w(uint16_t data);

	uint16_t pbcr1_r();
	void pbcr1_w(uint16_t data);

	uint16_t pbcr2_r();
	void pbcr2_w(uint16_t data);

	// port e
	uint16_t pedr_r();
	void pedr_w(uint16_t data);

	uint16_t peior_r();
	void peior_w(uint16_t data);

	uint16_t pecr1_r();
	void pecr1_w(uint16_t data);

	uint16_t pecr2_r();
	void pecr2_w(uint16_t data);

	// port f
	uint8_t pfdr_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read16 m_port_a_read_cb;
	devcb_write16 m_port_a_write_cb;

	devcb_read16 m_port_b_read_cb;
	devcb_write16 m_port_b_write_cb;

	devcb_read16 m_port_e_read_cb;
	devcb_write16 m_port_e_write_cb;

	devcb_read16 m_port_f_read_cb;

	uint16_t m_padr, m_paior, m_pacr1, m_pacr2;
	uint16_t m_pbdr, m_pbior, m_pbcr1, m_pbcr2;
	uint16_t m_pedr, m_peior, m_pecr1, m_pecr2;
	uint8_t m_pfdr;
};


DECLARE_DEVICE_TYPE(SH7014_PORT, sh7014_port_device)

#endif // MAME_CPU_SH_SH7014_PORT_H
