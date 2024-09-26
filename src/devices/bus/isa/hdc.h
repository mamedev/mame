// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    ISA 8 bit XT Hard Disk Controller

**********************************************************************/
#ifndef MAME_BUS_ISA_HDC_H
#define MAME_BUS_ISA_HDC_H

#pragma once

#include "isa.h"
#include "imagedev/harddriv.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// XT HD controller device


class xt_hdc_device :
		public device_t
{
public:
	// construction/destruction
	xt_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_handler() { return m_irq_handler.bind(); }
	auto drq_handler() { return m_drq_handler.bind(); }

	int dack_r();
	int dack_rs();
	void dack_w(int data);
	void dack_ws(int data);

	void data_w(uint8_t data);
	void reset_w(uint8_t data);
	void select_w(uint8_t data);
	void control_w(uint8_t data);
	uint8_t data_r();
	uint8_t status_r();
	void set_ready();
	uint8_t get_command() { return m_buffer[0]; }
	bool install_rom() { return (m_type != EC1841); }

protected:
	xt_hdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	harddisk_image_device *pc_hdc_file(int id);
	void pc_hdc_result(bool set_error_info);
	bool no_dma();
	int get_lbasector();
	void execute_read();
	void execute_readsbuff();
	void execute_write();
	void execute_writesbuff();
	void get_drive();
	void get_chsn();
	bool test_ready();

	TIMER_CALLBACK_MEMBER(process_command);

	std::unique_ptr<uint8_t[]> m_buffer;  /* data buffer */
	uint8_t *m_buffer_ptr;  /* data pointer */
	int m_csb;              /* command status byte */
	int m_status;           /* drive status */
	int m_error;            /* error code */

	enum {
		STANDARD,
		EC1841,
		ST11M
	};
	int m_type;
	uint8_t m_current_cmd;
	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;

private:
	int m_drv;              /* 0 master, 1 slave drive */
	int m_cylinders[2];     /* number of cylinders */
	int m_rwc[2];           /* reduced write current from cyl */
	int m_wp[2];            /* write precompensation from cyl */
	int m_heads[2];         /* heads */
	int m_ecc[2];           /* ECC bytes */

	/* indexes */
	int m_cylinder[2];      /* current cylinder */
	int m_head[2];          /* current head */
	int m_sector[2];        /* current sector */
	int m_sector_cnt[2];    /* sector count */
	int m_control[2];       /* control */

	emu_timer *m_timer;

	int m_data_cnt;         /* data count */
	uint8_t m_hdc_control;

	uint8_t m_hdcdma_data[512];
	uint8_t *m_hdcdma_src;
	uint8_t *m_hdcdma_dst;
	int m_hdcdma_read;
	int m_hdcdma_write;
	int m_hdcdma_size;
};

class ec1841_device : public xt_hdc_device
{
public:
	ec1841_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;
};

class st11m_device : public xt_hdc_device
{
public:
	st11m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;
};

DECLARE_DEVICE_TYPE(XT_HDC,     xt_hdc_device)
DECLARE_DEVICE_TYPE(EC1841_HDC, ec1841_device)
DECLARE_DEVICE_TYPE(ST11M_HDC,  st11m_device)

// ======================> isa8_hdc_device

class isa8_hdc_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t pc_hdc_r(offs_t offset);
	void pc_hdc_w(offs_t offset, uint8_t data);
	required_device<xt_hdc_device> m_hdc;

protected:
	isa8_hdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void irq_w(int state);
	void drq_w(int state);

public:
	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line,uint8_t data) override;
	uint8_t pc_hdc_dipswitch_r();

	int m_dip;                /* dip switches */
};


class isa8_hdc_ec1841_device : public isa8_hdc_device
{
public:
	isa8_hdc_ec1841_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<ec1841_device> m_hdc;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_HDC,        isa8_hdc_device)
DECLARE_DEVICE_TYPE(ISA8_HDC_EC1841, isa8_hdc_ec1841_device)

#endif // MAME_BUS_ISA_HDC_H
