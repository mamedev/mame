// license:BSD-3-Clause
// copyright-holders:smf

#pragma once

#ifndef _SCSI_H_
#define _SCSI_H_

#include "emu.h"
#include "machine/buffer.h"
#include "machine/latch.h"

#define SCSI_PORT_DEVICE1 "1"
#define SCSI_PORT_DEVICE2 "2"
#define SCSI_PORT_DEVICE3 "3"
#define SCSI_PORT_DEVICE4 "4"
#define SCSI_PORT_DEVICE5 "5"
#define SCSI_PORT_DEVICE6 "6"
#define SCSI_PORT_DEVICE7 "7"

#define MCFG_SCSI_BSY_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_bsy_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_SEL_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_sel_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_CD_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_cd_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_IO_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_io_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_MSG_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_msg_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_REQ_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_req_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_ACK_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_ack_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_ATN_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_atn_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_RST_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_rst_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_DATA0_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_data0_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_DATA1_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_data1_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_DATA2_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_data2_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_DATA3_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_data3_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_DATA4_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_data4_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_DATA5_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_data5_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_DATA6_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_data6_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_DATA7_HANDLER(_devcb) \
	devcb = &SCSI_PORT_DEVICE::set_data7_handler(*device, DEVCB_##_devcb);

#define MCFG_SCSI_OUTPUT_LATCH_ADD(_tag, scsi_port_tag) \
	MCFG_DEVICE_ADD(_tag, OUTPUT_LATCH, 0) \
	MCFG_OUTPUT_LATCH_BIT0_HANDLER(DEVWRITELINE(scsi_port_tag, SCSI_PORT_DEVICE, write_data0)) \
	MCFG_OUTPUT_LATCH_BIT1_HANDLER(DEVWRITELINE(scsi_port_tag, SCSI_PORT_DEVICE, write_data1)) \
	MCFG_OUTPUT_LATCH_BIT2_HANDLER(DEVWRITELINE(scsi_port_tag, SCSI_PORT_DEVICE, write_data2)) \
	MCFG_OUTPUT_LATCH_BIT3_HANDLER(DEVWRITELINE(scsi_port_tag, SCSI_PORT_DEVICE, write_data3)) \
	MCFG_OUTPUT_LATCH_BIT4_HANDLER(DEVWRITELINE(scsi_port_tag, SCSI_PORT_DEVICE, write_data4)) \
	MCFG_OUTPUT_LATCH_BIT5_HANDLER(DEVWRITELINE(scsi_port_tag, SCSI_PORT_DEVICE, write_data5)) \
	MCFG_OUTPUT_LATCH_BIT6_HANDLER(DEVWRITELINE(scsi_port_tag, SCSI_PORT_DEVICE, write_data6)) \
	MCFG_OUTPUT_LATCH_BIT7_HANDLER(DEVWRITELINE(scsi_port_tag, SCSI_PORT_DEVICE, write_data7))

#define MCFG_SCSI_DATA_INPUT_BUFFER(_tag) \
	MCFG_SCSI_DATA0_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit0)) \
	MCFG_SCSI_DATA1_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit1)) \
	MCFG_SCSI_DATA2_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit2)) \
	MCFG_SCSI_DATA3_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit3)) \
	MCFG_SCSI_DATA4_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit4)) \
	MCFG_SCSI_DATA5_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit5)) \
	MCFG_SCSI_DATA6_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit6)) \
	MCFG_SCSI_DATA7_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit7))

class SCSI_PORT_SLOT_device;
class scsi_port_interface;

class SCSI_PORT_DEVICE : public device_t
{
	friend class scsi_port_interface;

public:
	// construction/destruction
	SCSI_PORT_DEVICE(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_bsy_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_bsy_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_sel_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_sel_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_cd_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_cd_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_io_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_io_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_msg_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_msg_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_req_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_req_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ack_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_ack_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_atn_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_atn_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rst_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_rst_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data0_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_data0_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data1_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_data1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data2_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_data2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data3_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_data3_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data4_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_data4_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data5_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_data5_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data6_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_data6_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data7_handler(device_t &device, _Object object) { return downcast<SCSI_PORT_DEVICE &>(device).m_data7_handler.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( write_bsy );
	DECLARE_WRITE_LINE_MEMBER( write_sel );
	DECLARE_WRITE_LINE_MEMBER( write_cd );
	DECLARE_WRITE_LINE_MEMBER( write_io );
	DECLARE_WRITE_LINE_MEMBER( write_msg );
	DECLARE_WRITE_LINE_MEMBER( write_req );
	DECLARE_WRITE_LINE_MEMBER( write_ack );
	DECLARE_WRITE_LINE_MEMBER( write_atn );
	DECLARE_WRITE_LINE_MEMBER( write_rst );
	DECLARE_WRITE_LINE_MEMBER( write_data0 );
	DECLARE_WRITE_LINE_MEMBER( write_data1 );
	DECLARE_WRITE_LINE_MEMBER( write_data2 );
	DECLARE_WRITE_LINE_MEMBER( write_data3 );
	DECLARE_WRITE_LINE_MEMBER( write_data4 );
	DECLARE_WRITE_LINE_MEMBER( write_data5 );
	DECLARE_WRITE_LINE_MEMBER( write_data6 );
	DECLARE_WRITE_LINE_MEMBER( write_data7 );

protected:
	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;

	void update_bsy();
	void update_sel();
	void update_cd();
	void update_io();
	void update_msg();
	void update_req();
	void update_ack();
	void update_atn();
	void update_rst();
	void update_data0();
	void update_data1();
	void update_data2();
	void update_data3();
	void update_data4();
	void update_data5();
	void update_data6();
	void update_data7();

private:
	devcb_write_line m_bsy_handler;
	devcb_write_line m_sel_handler;
	devcb_write_line m_cd_handler;
	devcb_write_line m_io_handler;
	devcb_write_line m_msg_handler;
	devcb_write_line m_req_handler;
	devcb_write_line m_ack_handler;
	devcb_write_line m_atn_handler;
	devcb_write_line m_rst_handler;
	devcb_write_line m_data0_handler;
	devcb_write_line m_data1_handler;
	devcb_write_line m_data2_handler;
	devcb_write_line m_data3_handler;
	devcb_write_line m_data4_handler;
	devcb_write_line m_data5_handler;
	devcb_write_line m_data6_handler;
	devcb_write_line m_data7_handler;

	SCSI_PORT_SLOT_device *m_slot[7];
	int m_device_count;

	int m_bsy_in;
	int m_sel_in;
	int m_cd_in;
	int m_io_in;
	int m_msg_in;
	int m_req_in;
	int m_ack_in;
	int m_atn_in;
	int m_rst_in;
	int m_data0_in;
	int m_data1_in;
	int m_data2_in;
	int m_data3_in;
	int m_data4_in;
	int m_data5_in;
	int m_data6_in;
	int m_data7_in;
	int m_bsy_out;
	int m_sel_out;
	int m_cd_out;
	int m_io_out;
	int m_msg_out;
	int m_req_out;
	int m_ack_out;
	int m_atn_out;
	int m_rst_out;
	int m_data0_out;
	int m_data1_out;
	int m_data2_out;
	int m_data3_out;
	int m_data4_out;
	int m_data5_out;
	int m_data6_out;
	int m_data7_out;
};

extern const device_type SCSI_PORT;

class scsi_port_interface;

class SCSI_PORT_SLOT_device : public device_t,
	public device_slot_interface
{
	friend class SCSI_PORT_DEVICE;
	friend class scsi_port_interface;

public:
	SCSI_PORT_SLOT_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	scsi_port_interface *dev() { return m_dev; }
	SCSI_PORT_DEVICE *port() { return m_port; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

protected:
	scsi_port_interface *m_dev;
	SCSI_PORT_DEVICE *m_port;

	int m_bsy;
	int m_sel;
	int m_cd;
	int m_io;
	int m_msg;
	int m_req;
	int m_ack;
	int m_atn;
	int m_rst;
	int m_data0;
	int m_data1;
	int m_data2;
	int m_data3;
	int m_data4;
	int m_data5;
	int m_data6;
	int m_data7;
};

extern const device_type SCSI_PORT_SLOT;

class scsi_port_interface : public device_slot_card_interface
{
public:
	scsi_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~scsi_port_interface();

	virtual DECLARE_WRITE_LINE_MEMBER( input_bsy ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_sel ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_cd ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_io ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_msg ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_req ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_ack ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_atn ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_rst ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_data7 ) {}

	DECLARE_WRITE_LINE_MEMBER( output_bsy ) { if (m_slot->m_bsy != state) { m_slot->m_bsy = state; m_slot->port()->update_bsy(); } }
	DECLARE_WRITE_LINE_MEMBER( output_sel ) { if (m_slot->m_sel != state) { m_slot->m_sel = state; m_slot->port()->update_sel(); } }
	DECLARE_WRITE_LINE_MEMBER( output_cd ) { if (m_slot->m_cd != state) { m_slot->m_cd = state; m_slot->port()->update_cd(); } }
	DECLARE_WRITE_LINE_MEMBER( output_io ) { if (m_slot->m_io != state) { m_slot->m_io = state; m_slot->port()->update_io(); } }
	DECLARE_WRITE_LINE_MEMBER( output_msg ) { if (m_slot->m_msg != state) { m_slot->m_msg = state; m_slot->port()->update_msg(); } }
	DECLARE_WRITE_LINE_MEMBER( output_req ) { if (m_slot->m_req != state) { m_slot->m_req = state; m_slot->port()->update_req(); } }
	DECLARE_WRITE_LINE_MEMBER( output_ack ) { if (m_slot->m_ack != state) { m_slot->m_ack = state; m_slot->port()->update_ack(); } }
	DECLARE_WRITE_LINE_MEMBER( output_atn ) { if (m_slot->m_atn != state) { m_slot->m_atn = state; m_slot->port()->update_atn(); } }
	DECLARE_WRITE_LINE_MEMBER( output_rst ) { if (m_slot->m_rst != state) { m_slot->m_rst = state; m_slot->port()->update_rst(); } }
	DECLARE_WRITE_LINE_MEMBER( output_data0 ) { if (m_slot->m_data0 != state) { m_slot->m_data0 = state; m_slot->port()->update_data0(); } }
	DECLARE_WRITE_LINE_MEMBER( output_data1 ) { if (m_slot->m_data1 != state) { m_slot->m_data1 = state; m_slot->port()->update_data1(); } }
	DECLARE_WRITE_LINE_MEMBER( output_data2 ) { if (m_slot->m_data2 != state) { m_slot->m_data2 = state; m_slot->port()->update_data2(); } }
	DECLARE_WRITE_LINE_MEMBER( output_data3 ) { if (m_slot->m_data3 != state) { m_slot->m_data3 = state; m_slot->port()->update_data3(); } }
	DECLARE_WRITE_LINE_MEMBER( output_data4 ) { if (m_slot->m_data4 != state) { m_slot->m_data4 = state; m_slot->port()->update_data4(); } }
	DECLARE_WRITE_LINE_MEMBER( output_data5 ) { if (m_slot->m_data5 != state) { m_slot->m_data5 = state; m_slot->port()->update_data5(); } }
	DECLARE_WRITE_LINE_MEMBER( output_data6 ) { if (m_slot->m_data6 != state) { m_slot->m_data6 = state; m_slot->port()->update_data6(); } }
	DECLARE_WRITE_LINE_MEMBER( output_data7 ) { if (m_slot->m_data7 != state) { m_slot->m_data7 = state; m_slot->port()->update_data7(); } }

private:
	SCSI_PORT_SLOT_device *m_slot;
};

#endif
