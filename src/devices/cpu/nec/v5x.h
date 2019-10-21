// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_NEC_V5X_H
#define MAME_CPU_NEC_V5X_H

#pragma once

#include "nec.h"

#include "machine/am9517a.h"
#include "machine/i8251.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"

class device_v5x_interface : public device_interface
{
public:
	// TCU
	template <unsigned Timer> void set_clk(double clk) { device().subdevice<pit8253_device>("tcu")->set_clk<Timer>(clk); }
	template <unsigned Timer> void set_clk(const XTAL &xtal) { device().subdevice<pit8253_device>("tcu")->set_clk<Timer>(xtal.dvalue()); }
	template <unsigned Timer> auto out_handler() { return device().subdevice<pit8253_device>("tcu")->out_handler<Timer>(); }

	// DMAU
	auto out_hreq_cb() { return device().subdevice<v5x_dmau_device>("dmau")->out_hreq_callback(); }
	auto out_eop_cb() { return device().subdevice<v5x_dmau_device>("dmau")->out_eop_callback(); }
	auto in_memr_cb() { return device().subdevice<v5x_dmau_device>("dmau")->in_memr_callback(); }
	auto in_mem16r_cb() { return device().subdevice<v5x_dmau_device>("dmau")->in_mem16r_callback(); }
	auto out_memw_cb() { return device().subdevice<v5x_dmau_device>("dmau")->out_memw_callback(); }
	auto out_mem16w_cb() { return device().subdevice<v5x_dmau_device>("dmau")->out_mem16w_callback(); }
	template <unsigned Channel> auto in_ior_cb() { return device().subdevice<v5x_dmau_device>("dmau")->in_ior_callback<Channel>(); }
	template <unsigned Channel> auto in_io16r_cb() { return device().subdevice<v5x_dmau_device>("dmau")->in_io16r_callback<Channel>(); }
	template <unsigned Channel> auto out_iow_cb() { return device().subdevice<v5x_dmau_device>("dmau")->out_iow_callback<Channel>(); }
	template <unsigned Channel> auto out_io16w_cb() { return device().subdevice<v5x_dmau_device>("dmau")->out_io16w_callback<Channel>(); }
	template <unsigned Channel> auto out_dack_cb() { return device().subdevice<v5x_dmau_device>("dmau")->out_dack_callback<Channel>(); }

	// SCU
	auto txd_handler_cb() { return device().subdevice<v5x_scu_device>("scu")->txd_handler(); }
	auto dtr_handler_cb() { return device().subdevice<v5x_scu_device>("scu")->dtr_handler(); }
	auto rts_handler_cb() { return device().subdevice<v5x_scu_device>("scu")->rts_handler(); }
	auto rxrdy_handler_cb() { return device().subdevice<v5x_scu_device>("scu")->rxrdy_handler(); }
	auto txrdy_handler_cb() { return device().subdevice<v5x_scu_device>("scu")->txrdy_handler(); }
	auto txempty_handler_cb() { return device().subdevice<v5x_scu_device>("scu")->txempty_handler(); }
	auto syndet_handler_cb() { return device().subdevice<v5x_scu_device>("scu")->syndet_handler(); }

protected:
	device_v5x_interface(const machine_config &mconfig, nec_common_device &device);

	// device_interface overrides
	virtual void interface_post_start() override;
	virtual void interface_pre_reset() override;
	virtual void interface_post_load() override;

	void v5x_set_input(int inputnum, int state);
	void v5x_add_mconfig(machine_config &config);

	virtual void install_peripheral_io() = 0;

	DECLARE_WRITE8_MEMBER(BSEL_w) {}
	DECLARE_WRITE8_MEMBER(BADR_w) {}
	DECLARE_WRITE8_MEMBER(BRC_w) {}
	DECLARE_WRITE8_MEMBER(WMB0_w) {}
	DECLARE_WRITE8_MEMBER(WCY1_w) {}
	DECLARE_WRITE8_MEMBER(WCY0_w) {}
	DECLARE_WRITE8_MEMBER(WAC_w) {}
	DECLARE_WRITE8_MEMBER(TCKS_w) {}
	DECLARE_WRITE8_MEMBER(SBCR_w) {}
	DECLARE_WRITE8_MEMBER(RFC_w) {}
	DECLARE_WRITE8_MEMBER(WMB1_w) {}
	DECLARE_WRITE8_MEMBER(WCY2_w) {}
	DECLARE_WRITE8_MEMBER(WCY3_w) {}
	DECLARE_WRITE8_MEMBER(WCY4_w) {}
	DECLARE_WRITE8_MEMBER(SULA_w);
	DECLARE_WRITE8_MEMBER(TULA_w);
	DECLARE_WRITE8_MEMBER(IULA_w);
	DECLARE_WRITE8_MEMBER(DULA_w);
	DECLARE_WRITE8_MEMBER(OPHA_w);
	DECLARE_WRITE8_MEMBER(OPSEL_w);
	DECLARE_READ8_MEMBER(get_pic_ack) { return 0; }
	DECLARE_WRITE_LINE_MEMBER(internal_irq_w);

	required_device<pit8253_device> m_tcu;
	required_device<v5x_dmau_device> m_dmau;
	required_device<v5x_icu_device> m_icu;
	required_device<v5x_scu_device> m_scu;

	enum opsel_mask
	{
		OPSEL_DS = 0x01, // dmau enabled
		OPSEL_IS = 0x02, // icu enabled
		OPSEL_TS = 0x04, // tcu enabled
		OPSEL_SS = 0x08, // scu enabled
	};
	u8 m_OPSEL;

	u8 m_SULA;
	u8 m_TULA;
	u8 m_IULA;
	u8 m_DULA;
	u8 m_OPHA;
};

class v50_base_device : public nec_common_device, public device_v5x_interface
{
public:
	template <unsigned Channel> DECLARE_WRITE_LINE_MEMBER(dreq_w) { m_dmau->dreq_w<Channel>(state); }
	DECLARE_WRITE_LINE_MEMBER(hack_w) { m_dmau->hack_w(state); }

protected:
	v50_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool is_16bit, uint8_t prefetch_size, uint8_t prefetch_cycles, uint32_t chip_type);

	// device-specific overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_set_input(int inputnum, int state) override;

	void internal_port_map(address_map &map);

	DECLARE_WRITE8_MEMBER(OPCN_w);

private:
	u8 m_OPCN;
};

class v40_device : public v50_base_device
{
public:
	v40_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void install_peripheral_io() override;
};

class v50_device : public v50_base_device
{
public:
	v50_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void install_peripheral_io() override;
};

class v53_device : public v33_base_device, public device_v5x_interface
{
public:
	v53_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <unsigned Channel> DECLARE_WRITE_LINE_MEMBER(dreq_w)
	{
		// dreq0 could be wrong / nonexistent
		if (!(m_SCTL & 0x02))
		{
			m_dmau->dreq_w<Channel>(state);
		}
		else
		{
			logerror("dreq%d not in 71071mode\n", Channel);
		}
	}
	DECLARE_WRITE_LINE_MEMBER(hack_w);

protected:
	v53_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-specific overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_set_input(int inputnum, int state) override;

	void internal_port_map(address_map &map);
	virtual void install_peripheral_io() override;

	DECLARE_WRITE8_MEMBER(SCTL_w);

private:
	u8 m_SCTL;
};

class v53a_device : public v53_device
{
public:
	v53a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(V40,  v40_device)
DECLARE_DEVICE_TYPE(V50,  v50_device)
DECLARE_DEVICE_TYPE(V53,  v53_device)
DECLARE_DEVICE_TYPE(V53A, v53a_device)

#endif // MAME_CPU_NEC_V5X_H
