// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    AMD AM9517A/8237A Multimode DMA Controller emulation

****************************************************************************
                            _____   _____
                  _IOR   1 |*    \_/     | 40  A7
                  _IOW   2 |             | 39  A6
                 _MEMR   3 |             | 38  A5
                 _MEMW   4 |             | 37  A4
                     *   5 |             | 36  _EOP
                 READY   6 |             | 35  A3
                  HACK   7 |             | 34  A2
                 ADSTB   8 |             | 33  A1
                   AEN   9 |             | 32  A0
                  HREQ  10 |   AM9517A   | 31  Vcc
                   _CS  11 |    8237A    | 30  DB0
                   CLK  12 |             | 29  DB1
                 RESET  13 |             | 28  DB2
                 DACK2  14 |             | 27  DB3
                 DACK3  15 |             | 26  DB4
                 DREQ3  16 |             | 25  DACK0
                 DREQ2  17 |             | 24  DACK1
                 DREQ1  18 |             | 23  DB5
                 DREQ0  19 |             | 22  DB6
                   Vss  20 |_____________| 21  DB7

***************************************************************************/

#pragma once

#ifndef __AM9517A__
#define __AM9517A__

#include "emu.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> am9517a_device

class am9517a_device :  public device_t,
						public device_execute_interface
{
public:
	// construction/destruction
	am9517a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);
	am9517a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_hreq_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_hreq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_eop_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_eop_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_in_memr_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_in_memr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_memw_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_memw_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_in_ior_0_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_in_ior_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_1_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_in_ior_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_2_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_in_ior_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_3_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_in_ior_3_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_iow_0_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_iow_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_1_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_iow_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_2_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_iow_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_3_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_iow_3_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_dack_0_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_dack_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_1_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_dack_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_2_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_dack_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_3_callback(device_t &device, _Object object) { return downcast<am9517a_device &>(device).m_out_dack_3_cb.set_callback(object); }

	virtual DECLARE_READ8_MEMBER( read );
	virtual DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( hack_w );
	DECLARE_WRITE_LINE_MEMBER( ready_w );
	DECLARE_WRITE_LINE_MEMBER( eop_w );

	DECLARE_WRITE_LINE_MEMBER( dreq0_w );
	DECLARE_WRITE_LINE_MEMBER( dreq1_w );
	DECLARE_WRITE_LINE_MEMBER( dreq2_w );
	DECLARE_WRITE_LINE_MEMBER( dreq3_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_run() override;

	int m_icount;
	UINT32 m_address_mask;

	struct
	{
		UINT32 m_address;
		UINT16 m_count;
		UINT32 m_base_address;
		UINT16 m_base_count;
		UINT8 m_mode;
	} m_channel[4];

	int m_msb;
	int m_hreq;
	int m_hack;
	int m_ready;
	int m_eop;
	int m_state;
	int m_current_channel;
	int m_last_channel;
	UINT8 m_command;
	UINT8 m_mask;
	UINT8 m_status;
	UINT16 m_temp;
	UINT8 m_request;

private:
	inline void dma_request(int channel, int state);
	inline bool is_request_active(int channel);
	inline bool is_software_request_active(int channel);
	inline void set_hreq(int state);
	inline void set_dack();
	inline void set_eop(int state);
	inline int get_state1(bool msb_changed);
	inline void dma_read();
	inline void dma_write();
	inline void dma_advance();
	inline void end_of_process();

	devcb_write_line   m_out_hreq_cb;
	devcb_write_line   m_out_eop_cb;

	devcb_read8        m_in_memr_cb;
	devcb_write8       m_out_memw_cb;

	devcb_read8        m_in_ior_0_cb;
	devcb_read8        m_in_ior_1_cb;
	devcb_read8        m_in_ior_2_cb;
	devcb_read8        m_in_ior_3_cb;
	devcb_write8       m_out_iow_0_cb;
	devcb_write8       m_out_iow_1_cb;
	devcb_write8       m_out_iow_2_cb;
	devcb_write8       m_out_iow_3_cb;
	devcb_write_line   m_out_dack_0_cb;
	devcb_write_line   m_out_dack_1_cb;
	devcb_write_line   m_out_dack_2_cb;
	devcb_write_line   m_out_dack_3_cb;





};


class upd71071_v53_device :  public am9517a_device
{
public:
	// construction/destruction
	upd71071_v53_device(const machine_config &mconfig,  const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER( read ) override;
	virtual DECLARE_WRITE8_MEMBER( write ) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	int m_selected_channel;
	int m_base;
	UINT8 m_command_high;

};




// device type definition
extern const device_type AM9517A;
extern const device_type V53_DMAU;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_AM9517A_OUT_HREQ_CB(_devcb) \
	devcb = &am9517a_device::set_out_hreq_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_EOP_CB(_devcb) \
	devcb = &am9517a_device::set_out_eop_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_IN_MEMR_CB(_devcb) \
	devcb = &am9517a_device::set_in_memr_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_MEMW_CB(_devcb) \
	devcb = &am9517a_device::set_out_memw_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_IN_IOR_0_CB(_devcb) \
	devcb = &am9517a_device::set_in_ior_0_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_IN_IOR_1_CB(_devcb) \
	devcb = &am9517a_device::set_in_ior_1_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_IN_IOR_2_CB(_devcb) \
	devcb = &am9517a_device::set_in_ior_2_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_IN_IOR_3_CB(_devcb) \
	devcb = &am9517a_device::set_in_ior_3_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_IOW_0_CB(_devcb) \
	devcb = &am9517a_device::set_out_iow_0_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_IOW_1_CB(_devcb) \
	devcb = &am9517a_device::set_out_iow_1_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_IOW_2_CB(_devcb) \
	devcb = &am9517a_device::set_out_iow_2_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_IOW_3_CB(_devcb) \
	devcb = &am9517a_device::set_out_iow_3_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_DACK_0_CB(_devcb) \
	devcb = &am9517a_device::set_out_dack_0_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_DACK_1_CB(_devcb) \
	devcb = &am9517a_device::set_out_dack_1_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_DACK_2_CB(_devcb) \
	devcb = &am9517a_device::set_out_dack_2_callback(*device, DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_DACK_3_CB(_devcb) \
	devcb = &am9517a_device::set_out_dack_3_callback(*device, DEVCB_##_devcb);

#define MCFG_I8237_OUT_HREQ_CB MCFG_AM9517A_OUT_HREQ_CB
#define MCFG_I8237_OUT_EOP_CB MCFG_AM9517A_OUT_EOP_CB
#define MCFG_I8237_IN_MEMR_CB MCFG_AM9517A_IN_MEMR_CB
#define MCFG_I8237_OUT_MEMW_CB MCFG_AM9517A_OUT_MEMW_CB
#define MCFG_I8237_IN_IOR_0_CB MCFG_AM9517A_IN_IOR_0_CB
#define MCFG_I8237_IN_IOR_1_CB MCFG_AM9517A_IN_IOR_1_CB
#define MCFG_I8237_IN_IOR_2_CB MCFG_AM9517A_IN_IOR_2_CB
#define MCFG_I8237_IN_IOR_3_CB MCFG_AM9517A_IN_IOR_3_CB
#define MCFG_I8237_OUT_IOW_0_CB MCFG_AM9517A_OUT_IOW_0_CB
#define MCFG_I8237_OUT_IOW_1_CB MCFG_AM9517A_OUT_IOW_1_CB
#define MCFG_I8237_OUT_IOW_2_CB MCFG_AM9517A_OUT_IOW_2_CB
#define MCFG_I8237_OUT_IOW_3_CB MCFG_AM9517A_OUT_IOW_3_CB
#define MCFG_I8237_OUT_DACK_0_CB MCFG_AM9517A_OUT_DACK_0_CB
#define MCFG_I8237_OUT_DACK_1_CB MCFG_AM9517A_OUT_DACK_1_CB
#define MCFG_I8237_OUT_DACK_2_CB MCFG_AM9517A_OUT_DACK_2_CB
#define MCFG_I8237_OUT_DACK_3_CB MCFG_AM9517A_OUT_DACK_3_CB

#endif
