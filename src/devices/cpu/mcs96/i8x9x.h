// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8x9x.h

    MCS96, 8x9x branch, the original version

***************************************************************************/

#ifndef __I8X9X_H__
#define __I8X9X_H__

#include "mcs96.h"

class i8x9x_device : public mcs96_device {
public:
	enum {
		A0, A1, A2, A3, A4, A5, A6, A7,
		SERIAL,
		P0, P1, P2
	};

	i8x9x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	void serial_w(UINT8 val);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;
	virtual void internal_update(UINT64 current_time) override;
	virtual void io_w8(UINT8 adr, UINT8 data) override;
	virtual void io_w16(UINT8 adr, UINT16 data) override;
	virtual UINT8 io_r8(UINT8 adr) override;
	virtual UINT16 io_r16(UINT8 adr) override;

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
		UINT8 command;
		UINT16 time;
	};

	address_space_config io_config;
	address_space *io;

	hso_cam_entry hso_info[8];
	hso_cam_entry hso_cam_hold;

	UINT64 base_timer2, ad_done;
	UINT8 hso_command, ad_command;
	UINT16 hso_time, ad_result;
	UINT8 ios0, ios1, ioc0, ioc1;
	UINT8 sbuf, sp_stat;
	UINT8 serial_send_buf;
	UINT64 serial_send_timer;

	UINT16 timer_value(int timer, UINT64 current_time) const;
	UINT64 timer_time_until(int timer, UINT64 current_time, UINT16 timer_value) const;
	void commit_hso_cam();
	void trigger_cam(int id, UINT64 current_time);
	void ad_start(UINT64 current_time);
	void serial_send(UINT8 data);
	void serial_send_done();
};

class c8095_device : public i8x9x_device {
public:
	c8095_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class p8098_device : public i8x9x_device {
public:
	p8098_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type C8095;
extern const device_type P8098;

#endif
