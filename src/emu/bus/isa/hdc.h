// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    ISA 8 bit XT Hard Disk Controller

**********************************************************************/
#pragma once

#ifndef ISA_HDC_H
#define ISA_HDC_H

#include "emu.h"
#include "isa.h"
#include "imagedev/harddriv.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// XT HD controller device

#define MCFG_XTHDC_IRQ_HANDLER(_devcb) \
	devcb = &xt_hdc_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_XTHDC_DRQ_HANDLER(_devcb) \
	devcb = &xt_hdc_device::set_drq_handler(*device, DEVCB_##_devcb);

class xt_hdc_device :
		public device_t
{
public:
	// construction/destruction
	xt_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	xt_hdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<xt_hdc_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_handler(device_t &device, _Object object) { return downcast<xt_hdc_device &>(device).m_drq_handler.set_callback(object); }

	int dack_r();
	void dack_w(int data);
	void dack_ws(int data);

	virtual void command();
	void data_w(int data);
	void reset_w(int data);
	void select_w(int data);
	void control_w(int data);
	UINT8 data_r();
	UINT8 status_r();
	void set_ready();
	UINT8 get_command() { return buffer[0]; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	hard_disk_file *pc_hdc_file(int id);
	void pc_hdc_result(int set_error_info);
	int no_dma(void);
	int get_lbasector();
	void execute_read();
	void execute_write();
	void execute_writesbuff();
	void get_drive();
	void get_chsn();
	int test_ready();

	dynamic_buffer buffer;                  /* data buffer */
	UINT8 *buffer_ptr;          /* data pointer */
	int csb;                /* command status byte */
	int status;         /* drive status */
	int error;          /* error code */

	enum {
		STANDARD,
		EC1841,
		ST11M
	};
	int m_type;
	UINT8 m_current_cmd;
	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;

private:
	int drv;                            /* 0 master, 1 slave drive */
	int cylinders[2];       /* number of cylinders */
	int rwc[2];         /* reduced write current from cyl */
	int wp[2];          /* write precompensation from cyl */
	int heads[2];           /* heads */
	int ecc[2];         /* ECC bytes */

	/* indexes */
	int cylinder[2];            /* current cylinder */
	int head[2];                /* current head */
	int sector[2];          /* current sector */
	int sector_cnt[2];      /* sector count */
	int control[2];         /* control */

	emu_timer *timer;

	int data_cnt;                /* data count */
	UINT8 hdc_control;

	UINT8 hdcdma_data[512];
	UINT8 *hdcdma_src;
	UINT8 *hdcdma_dst;
	int hdcdma_read;
	int hdcdma_write;
	int hdcdma_size;
};

class ec1841_device : public xt_hdc_device
{
public:
	ec1841_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;
};

class st11m_device : public xt_hdc_device
{
public:
	st11m_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;
};

extern const device_type XT_HDC;
extern const device_type EC1841_HDC;
extern const device_type ST11M_HDC;

// ======================> isa8_hdc_device

class isa8_hdc_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
		isa8_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		isa8_hdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

		DECLARE_READ8_MEMBER(pc_hdc_r);
		DECLARE_WRITE8_MEMBER(pc_hdc_w);
		DECLARE_WRITE_LINE_MEMBER(irq_w);
		DECLARE_WRITE_LINE_MEMBER(drq_w);
		required_device<xt_hdc_device> m_hdc;

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;
		virtual ioport_constructor device_input_ports() const;
protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();
public:
		virtual UINT8 dack_r(int line);
		virtual void dack_w(int line,UINT8 data);
		UINT8 pc_hdc_dipswitch_r();

		int dip;                /* dip switches */
};


class isa8_hdc_ec1841_device : public isa8_hdc_device
{
public:
	isa8_hdc_ec1841_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	required_device<ec1841_device> m_hdc;
};

// device type definition
extern const device_type ISA8_HDC;
extern const device_type ISA8_HDC_EC1841;

#endif  /* ISA_HDC_H */
