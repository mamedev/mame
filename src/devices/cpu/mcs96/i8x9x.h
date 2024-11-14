// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8x9x.h

    MCS96, 8x9x branch, the original version

***************************************************************************/

#ifndef MAME_CPU_MCS96_I8X9X_H
#define MAME_CPU_MCS96_I8X9X_H

#include "mcs96.h"

class i8x9x_device : public mcs96_device {
public:
	enum {
		EXTINT_LINE = 0,
		HSI0_LINE,
		HSI1_LINE,
		HSI2_LINE,
		HSI3_LINE
	};

	enum {
		I8X9X_HSI_MODE = MCS96_LAST_REG + 1,
		I8X9X_HSI_STATUS,
		I8X9X_HSO_TIME,
		I8X9X_HSO_COMMAND,
		I8X9X_AD_COMMAND,
		I8X9X_AD_RESULT,
		I8X9X_PORT1,
		I8X9X_PORT2,
		I8X9X_PWM_CONTROL,
		I8X9X_SBUF_RX,
		I8X9X_SBUF_TX,
		I8X9X_SP_CON,
		I8X9X_SP_STAT,
		I8X9X_BAUD_RATE,
		I8X9X_IOC0,
		I8X9X_IOC1,
		I8X9X_IOS0,
		I8X9X_IOS1
	};

	auto ach0_cb() { return m_ach_cb[0].bind(); }
	auto ach1_cb() { return m_ach_cb[1].bind(); }
	auto ach2_cb() { return m_ach_cb[2].bind(); }
	auto ach3_cb() { return m_ach_cb[3].bind(); }
	auto ach4_cb() { return m_ach_cb[4].bind(); }
	auto ach5_cb() { return m_ach_cb[5].bind(); }
	auto ach6_cb() { return m_ach_cb[6].bind(); }
	auto ach7_cb() { return m_ach_cb[7].bind(); }
	auto hso_cb() { return m_hso_cb.bind(); }
	auto serial_tx_cb() { return m_serial_tx_cb.bind(); }

	auto in_p0_cb() { return m_in_p0_cb.bind(); }
	auto out_p1_cb() { return m_out_p1_cb.bind(); }
	auto in_p1_cb() { return m_in_p1_cb.bind(); }
	auto out_p2_cb() { return m_out_p2_cb.bind(); }
	auto in_p2_cb() { return m_in_p2_cb.bind(); }

	void serial_w(u8 val);

	virtual u8 i8x9x_p0_mask() const noexcept { return 0xff; }
	virtual bool i8x9x_has_p1() const noexcept { return true; }
	virtual u8 i8x9x_p2_mask() const noexcept { return 0xff; }

protected:
	i8x9x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int data_width);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_set_input(int linenum, int state) override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;
	virtual void internal_update(u64 current_time) override;

	void internal_regs(address_map &map) ATTR_COLD;
	void ad_command_w(u8 data);
	u8 ad_result_r(offs_t offset);
	void hsi_mode_w(u8 data);
	void hso_time_w(u16 data);
	u16 hsi_time_r();
	void hso_command_w(u8 data);
	u8 hsi_status_r();
	void sbuf_w(u8 data);
	u8 sbuf_r();
	void watchdog_w(u8 data);
	u16 timer1_r();
	u16 timer2_r();
	void baud_rate_w(u8 data);
	u8 port0_r();
	void port1_w(u8 data);
	u8 port1_r();
	void port2_w(u8 data);
	u8 port2_r();
	void sp_con_w(u8 data);
	u8 sp_stat_r();
	void ioc0_w(u8 data);
	u8 ios0_r();
	void ios0_w(u8 data);
	void ioc1_w(u8 data);
	u8 ios1_r();
	void pwm_control_w(u8 data);

private:
	enum {
		IRQ_TIMER  = 0x01,
		IRQ_AD     = 0x02,
		IRQ_HSI    = 0x04,
		IRQ_HSO    = 0x08,
		IRQ_HSI0   = 0x10,
		IRQ_SOFT   = 0x20,
		IRQ_SERIAL = 0x40,
		IRQ_EXTINT = 0x80
	};

	struct hso_cam_entry {
		u8 command;
		u16 time;
	};

	devcb_read16::array<8> m_ach_cb;
	devcb_write8 m_hso_cb;
	devcb_write8 m_serial_tx_cb;

	devcb_read8 m_in_p0_cb;
	devcb_write8 m_out_p1_cb;
	devcb_read8 m_in_p1_cb;
	devcb_write8 m_out_p2_cb;
	devcb_read8 m_in_p2_cb;
	//devcb_write16 m_out_p3_p4_cb;
	//devcb_read16 m_in_p3_p4_cb;

	hso_cam_entry hso_info[8];
	hso_cam_entry hso_cam_hold;

	u64 base_timer2, ad_done;
	u8 hsi_mode, hsi_status, hso_command, ad_command, hso_active;
	u16 hso_time, ad_result;
	u8 pwm_control;
	u8 port1, port2;
	u8 ios0, ios1, ioc0, ioc1;
	bool extint;
	u8 sbuf, sp_con, sp_stat;
	u8 serial_send_buf;
	u64 serial_send_timer;
	u16 baud_reg;
	bool brh;

	u16 timer_value(int timer, u64 current_time) const;
	u64 timer_time_until(int timer, u64 current_time, u16 timer_value) const;
	void timer2_reset(u64 current_time);
	void set_hsi_state(int pin, bool state);
	void commit_hso_cam();
	void trigger_cam(int id, u64 current_time);
	void set_hso(u8 mask, bool state);
	void ad_start(u64 current_time);
	void serial_send(u8 data);
	void serial_send_done();
};

class c8095_90_device : public i8x9x_device {
public:
	c8095_90_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual u8 i8x9x_p0_mask() const noexcept override { return 0xf0; }
	virtual bool i8x9x_has_p1() const noexcept override { return false; }
	virtual u8 i8x9x_p2_mask() const noexcept override { return 0x27; }
};

class n8097bh_device : public i8x9x_device {
public:
	n8097bh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class p8098_device : public i8x9x_device {
public:
	p8098_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual u8 i8x9x_p0_mask() const noexcept override { return 0xf0; }
	virtual bool i8x9x_has_p1() const noexcept override { return false; }
	virtual u8 i8x9x_p2_mask() const noexcept override { return 0x27; }
};

class p8798_device : public i8x9x_device {
public:
	p8798_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual u8 i8x9x_p0_mask() const noexcept override { return 0xf0; }
	virtual bool i8x9x_has_p1() const noexcept override { return false; }
	virtual u8 i8x9x_p2_mask() const noexcept override { return 0x27; }
};

DECLARE_DEVICE_TYPE(C8095_90, c8095_90_device)
DECLARE_DEVICE_TYPE(N8097BH, n8097bh_device)
DECLARE_DEVICE_TYPE(P8098, p8098_device)
DECLARE_DEVICE_TYPE(P8798, p8798_device)

#endif // MAME_CPU_MCS96_I8X9X_H
