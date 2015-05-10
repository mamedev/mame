// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
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

// ======================> isa8_hdc_device

class isa8_hdc_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
		isa8_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		isa8_hdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;
		DECLARE_READ8_MEMBER(pc_hdc_r);
		DECLARE_WRITE8_MEMBER(pc_hdc_w);
		virtual ioport_constructor device_input_ports() const;

protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();
		virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
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

		int csb;                /* command status byte */
		int status;         /* drive status */
		int error;          /* error code */
		int dip;                /* dip switches */
		emu_timer *timer;

		int data_cnt;                /* data count */
		dynamic_buffer buffer;                  /* data buffer */
		UINT8 *buffer_ptr;          /* data pointer */
		UINT8 hdc_control;

		UINT8 hdcdma_data[512];
		UINT8 *hdcdma_src;
		UINT8 *hdcdma_dst;
		int hdcdma_read;
		int hdcdma_write;
		int hdcdma_size;

		// internal state
public:
		virtual UINT8 dack_r(int line);
		virtual void dack_w(int line,UINT8 data);
protected:
		hard_disk_file *pc_hdc_file(int id);
		void pc_hdc_result(int set_error_info);
		int no_dma(void);
		int get_lbasector();
		int pc_hdc_dack_r();
		void pc_hdc_dack_w(int data);
		void pc_hdc_dack_ws(int data);
		void execute_read();
		void execute_write();
		void execute_writesbuff();
		void get_drive();
		void get_chsn();
		int test_ready();

		enum {
			STANDARD,
			EC1841
		};
		int m_type;
public:
		void hdc_command();
		void pc_hdc_data_w(int data);
		void pc_hdc_reset_w(int data);
		void pc_hdc_select_w(int data);
		void pc_hdc_control_w(int data);
		UINT8 pc_hdc_data_r();
		UINT8 pc_hdc_status_r();
		UINT8 pc_hdc_dipswitch_r();
};


class isa8_hdc_ec1841_device : public isa8_hdc_device
{
public:
	isa8_hdc_ec1841_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

// device type definition
extern const device_type ISA8_HDC;
extern const device_type ISA8_HDC_EC1841;

#endif  /* ISA_HDC_H */
