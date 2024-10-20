// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_MULTIFACE_H
#define MAME_SINCLAIR_SPECNEXT_MULTIFACE_H

#pragma once

class specnext_multiface_device : public device_t
{
public:
	specnext_multiface_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void cpu_a_0066_w(bool data) { m_cpu_a_0066 = data; }

	void cpu_mreq_n_w(bool data) { m_cpu_mreq_n = data; }
	void cpu_m1_n_w(bool data) { m_cpu_m1_n = data; }
	void cpu_retn_seen_w(bool data) { m_cpu_retn_seen = data; }

	void enable_w(bool data) { m_enable = data; }
	void button_w(bool data) { m_button = data; }

	void mf_mode_w(u8 data) { m_mf_mode = data; }

	void port_mf_enable_rd_w(bool data) { m_port_mf_enable_rd = data; }
	void port_mf_enable_wr_w(bool data) { m_port_mf_enable_wr = data; }
	void port_mf_disable_rd_w(bool data) { m_port_mf_disable_rd = data; }
	void port_mf_disable_wr_w(bool data) { m_port_mf_disable_wr = data; }

	bool nmi_disable_r() { return m_enable && m_nmi_active; };
	bool mf_enabled_r() { return m_enable && mf_enable_eff(); };
	bool mf_port_en_r() { return m_enable && (m_port_mf_enable_rd && !invisible_eff() && (mode_128() || mode_p3())); };

	void clock_w(); // called on active clock edge

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// in
	bool m_cpu_a_0066;
	bool m_cpu_mreq_n;
	bool m_cpu_m1_n;
	bool m_cpu_retn_seen;

	bool m_enable;
	bool m_button;

	u8 m_mf_mode;  // u2: 00 = mf +3, 11 = mf 48, else mf 128

	bool m_port_mf_enable_rd;
	bool m_port_mf_enable_wr;
	bool m_port_mf_disable_rd;
	bool m_port_mf_disable_wr;

	// internal
	bool m_nmi_active;
	bool m_invisible;
	bool m_mf_enable;

	// signal
	bool mode_48();
	bool mode_128();
	bool mode_p3();

	bool button_pulse();
	bool invisible_eff();
	bool fetch_66();
	bool mf_enable_eff();
};


DECLARE_DEVICE_TYPE(SPECNEXT_MULTIFACE, specnext_multiface_device)

#endif // MAME_SINCLAIR_SPECNEXT_MULTIFACE_H
