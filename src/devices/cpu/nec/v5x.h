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
	void set_tclk(double clk) { m_tclk = clk; }
	void set_tclk(const XTAL &xtal) { set_tclk(xtal.dvalue()); }
	void tclk_w(int state);

	// DMAU
	auto out_hreq_cb() { return m_dmau.lookup()->out_hreq_callback(); }
	auto out_eop_cb() { return m_dmau.lookup()->out_eop_callback(); }
	auto in_memr_cb() { return m_dmau.lookup()->in_memr_callback(); }
	auto in_mem16r_cb() { return m_dmau.lookup()->in_mem16r_callback(); }
	auto out_memw_cb() { return m_dmau.lookup()->out_memw_callback(); }
	auto out_mem16w_cb() { return m_dmau.lookup()->out_mem16w_callback(); }
	template <unsigned Channel> auto in_ior_cb() { return m_dmau.lookup()->in_ior_callback<Channel>(); }
	template <unsigned Channel> auto in_io16r_cb() { return m_dmau.lookup()->in_io16r_callback<Channel>(); }
	template <unsigned Channel> auto out_iow_cb() { return m_dmau.lookup()->out_iow_callback<Channel>(); }
	template <unsigned Channel> auto out_io16w_cb() { return m_dmau.lookup()->out_io16w_callback<Channel>(); }
	template <unsigned Channel> auto out_dack_cb() { return m_dmau.lookup()->out_dack_callback<Channel>(); }

	// SCU
	auto txd_handler_cb() { return m_scu.lookup()->txd_handler(); }
	auto dtr_handler_cb() { return m_scu.lookup()->dtr_handler(); }
	auto rts_handler_cb() { return m_scu.lookup()->rts_handler(); }
	auto rxrdy_handler_cb() { return m_scu.lookup()->rxrdy_handler(); }
	auto txrdy_handler_cb() { return m_scu.lookup()->txrdy_handler(); }
	auto txempty_handler_cb() { return m_scu.lookup()->txempty_handler(); }
	auto syndet_handler_cb() { return m_scu.lookup()->syndet_handler(); }

protected:
	device_v5x_interface(const machine_config &mconfig, nec_common_device &device, bool is_16bit);

	// device_interface overrides
	virtual void interface_post_start() override;
	virtual void interface_pre_reset() override;
	virtual void interface_post_load() override;
	virtual void interface_clock_changed(bool sync_on_new_clock_domain) override;

	void v5x_set_input(int inputnum, int state);
	void v5x_add_mconfig(machine_config &config);

	virtual void install_peripheral_io() = 0;

	const int AS_INTERNAL_IO = AS_OPCODES + 1;
	const u8 INTERNAL_IO_ADDR_WIDTH = (1 << 3);
	const u8 INTERNAL_IO_ADDR_MASK = (1 << INTERNAL_IO_ADDR_WIDTH) - 1;
	const u16 OPHA_MASK = INTERNAL_IO_ADDR_MASK << INTERNAL_IO_ADDR_WIDTH;

	inline u16 OPHA() { return (m_OPHA << INTERNAL_IO_ADDR_WIDTH) & OPHA_MASK; }
	inline u16 io_mask(u8 base) { return 0x00ff << ((base & 1) << 3); }
	inline bool check_OPHA(offs_t a)
	{
		return ((m_OPSEL & OPSEL_MASK) != 0) && (m_OPHA != 0xff) && ((a & OPHA_MASK) == OPHA()); // 256 bytes boundary, ignore system io area
	}

	inline u8 internal_io_read_byte(offs_t a) { return m_internal_io->read_byte(a & INTERNAL_IO_ADDR_MASK); }
	inline u16 internal_io_read_word(offs_t a) { return m_internal_io->read_word_unaligned(a & INTERNAL_IO_ADDR_MASK); }
	inline void internal_io_write_byte(offs_t a, u8 v) { m_internal_io->write_byte(a & INTERNAL_IO_ADDR_MASK, v); }
	inline void internal_io_write_word(offs_t a, u16 v) { m_internal_io->write_word_unaligned(a & INTERNAL_IO_ADDR_MASK, v); }

	void remappable_io_map(address_map &map) ATTR_COLD;
	virtual u8 temp_io_byte_r(offs_t offset) = 0;
	virtual void temp_io_byte_w(offs_t offset, u8 data) = 0;

	void BSEL_w(u8 data) {}
	void BADR_w(u8 data) {}
	void BRC_w(u8 data) {}
	void WMB0_w(u8 data) {}
	void WCY1_w(u8 data) {}
	void WCY0_w(u8 data) {}
	void WAC_w(u8 data) {}
	u8 TCKS_r();
	void TCKS_w(u8 data);
	void SBCR_w(u8 data) {}
	void RFC_w(u8 data) {}
	void WMB1_w(u8 data) {}
	void WCY2_w(u8 data) {}
	void WCY3_w(u8 data) {}
	void WCY4_w(u8 data) {}
	u8 SULA_r();
	void SULA_w(u8 data);
	u8 TULA_r();
	void TULA_w(u8 data);
	u8 IULA_r();
	void IULA_w(u8 data);
	u8 DULA_r();
	void DULA_w(u8 data);
	u8 OPHA_r();
	void OPHA_w(u8 data);
	u8 OPSEL_r();
	void OPSEL_w(u8 data);
	virtual u8 get_pic_ack(offs_t offset) { return 0; }
	void internal_irq_w(int state);

	void tcu_clock_update();

	required_device<pit8253_device> m_tcu;
	required_device<v5x_dmau_device> m_dmau;
	required_device<v5x_icu_device> m_icu;
	required_device<v5x_scu_device> m_scu;

	address_space_config m_internal_io_config;
	address_space *m_internal_io;

	double m_tclk;

	enum opsel_mask
	{
		OPSEL_DS = 0x01, // dmau enabled
		OPSEL_IS = 0x02, // icu enabled
		OPSEL_TS = 0x04, // tcu enabled
		OPSEL_SS = 0x08, // scu enabled
		OPSEL_MASK = OPSEL_DS | OPSEL_IS | OPSEL_TS | OPSEL_SS
	};
	u8 m_OPSEL;

	u8 m_SULA;
	u8 m_TULA;
	u8 m_IULA;
	u8 m_DULA;
	u8 m_OPHA;
	u8 m_TCKS;
};

class v50_base_device : public nec_common_device, public device_v5x_interface
{
public:
	template <unsigned Channel> void dreq_w(int state) { m_dmau->dreq_w<Channel>(state); }
	void hack_w(int state) { m_dmau->hack_w(state); }
	void tctl2_w(int state) { m_tcu->write_gate2(state); }

	auto tout1_cb() { return m_tout1_callback.bind(); }
	auto tout2_cb() { return m_tcu.lookup()->out_handler<2>(); }

	auto icu_slave_ack_cb() { return m_icu_slave_ack.bind(); }

protected:
	v50_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool is_16bit, u8 prefetch_size, u8 prefetch_cycles, u32 chip_type);

	// device-specific overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks / 2); }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	virtual u8 temp_io_byte_r(offs_t offset) override { return nec_common_device::io_read_byte(OPHA() | (offset & INTERNAL_IO_ADDR_MASK)); }
	virtual void temp_io_byte_w(offs_t offset, u8 data) override { nec_common_device::io_write_byte(OPHA() | (offset & INTERNAL_IO_ADDR_MASK), data); }

	virtual u8 io_read_byte(offs_t a) override;
	virtual u16 io_read_word(offs_t a) override;
	virtual void io_write_byte(offs_t a, u8 v) override;
	virtual void io_write_word(offs_t a, u16 v) override;

	void internal_port_map(address_map &map) ATTR_COLD;

	u8 OPCN_r();
	void OPCN_w(u8 data);

	// TODO: non-offset 7 configuration
	// Currently used by pc88va only, which uses the canonical IRQ7 for cascading an external PIC to the internal one.
	virtual u8 get_pic_ack(offs_t offset) override
	{
		if (offset == 7)
			return m_icu_slave_ack(0);
		return 0;
	}

private:
	void tout1_w(int state);

	devcb_write_line m_tout1_callback;
	devcb_read8 m_icu_slave_ack;

	u8 m_OPCN;
	bool m_tout1;
	bool m_intp1;
	bool m_intp2;
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

	template <unsigned Channel> void dreq_w(int state)
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
	void hack_w(int state);

	template <unsigned Timer> auto tout_handler() { return m_tcu.lookup()->out_handler<Timer>(); }

protected:
	v53_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-specific overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	virtual u8 temp_io_byte_r(offs_t offset) override { return nec_common_device::io_read_byte(OPHA() | (offset & INTERNAL_IO_ADDR_MASK)); }
	virtual void temp_io_byte_w(offs_t offset, u8 data) override { nec_common_device::io_write_byte(OPHA() | (offset & INTERNAL_IO_ADDR_MASK), data); }

	virtual u8 io_read_byte(offs_t a) override;
	virtual u16 io_read_word(offs_t a) override;
	virtual void io_write_byte(offs_t a, u8 v) override;
	virtual void io_write_word(offs_t a, u16 v) override;

	void internal_port_map(address_map &map) ATTR_COLD;
	virtual void install_peripheral_io() override;

	u8 SCTL_r();
	void SCTL_w(u8 data);

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
