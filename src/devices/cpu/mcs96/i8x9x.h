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
		A0, A1, A2, A3, A4, A5, A6, A7,
		SERIAL,
		P0, P1, P2
	};

	void serial_w(uint8_t val);

protected:
	i8x9x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual space_config_vector memory_space_config() const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual void io_w8(uint8_t adr, uint8_t data) override;
	virtual void io_w16(uint8_t adr, uint16_t data) override;
	virtual uint8_t io_r8(uint8_t adr) override;
	virtual uint16_t io_r16(uint8_t adr) override;

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
		bool active;
		uint8_t command;
		uint16_t time;
	};

	address_space_config io_config;
	address_space *io;

	hso_cam_entry hso_info[8];
	hso_cam_entry hso_cam_hold;

	uint64_t base_timer2, ad_done;
	uint8_t hso_command, ad_command;
	uint16_t hso_time, ad_result;
	uint8_t ios0, ios1, ioc0, ioc1;
	uint8_t sbuf, sp_stat;
	uint8_t serial_send_buf;
	uint64_t serial_send_timer;

	uint16_t timer_value(int timer, uint64_t current_time) const;
	uint64_t timer_time_until(int timer, uint64_t current_time, uint16_t timer_value) const;
	void commit_hso_cam();
	void trigger_cam(int id, uint64_t current_time);
	void ad_start(uint64_t current_time);
	void serial_send(uint8_t data);
	void serial_send_done();
};

class c8095_device : public i8x9x_device {
public:
	c8095_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class p8098_device : public i8x9x_device {
public:
	p8098_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(C8095, c8095_device)
DECLARE_DEVICE_TYPE(P8098, p8098_device)

#endif // MAME_CPU_MCS96_I8X9X_H
