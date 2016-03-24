// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA Cosmac VIP Expansion Interface emulation

**********************************************************************

                  CLOCK       1      A       _MWR
                   _EF4       2      B       TPA
                   _EF3       3      C       MA0
                   XTAL       4      D       MA1
                   _EF1       5      E       MA2
                     N0       6      F       MA3
                     N1       7      H       MA4
                     N2       8      J       MA5
                   SPOT       9      K       MA6
                  _SYNC      10      L       MA7
                    TPB      11      M       BUS 0
                    SC0      12      N       BUS 1
             _INTERRUPT      13      P       BUS 2
                    SC1      14      R       BUS 3
               _DMA-OUT      15      S       BUS 4
                      Q      16      T       BUS 5
                _DMA-IN      17      U       BUS 6
                    RUN      18      V       BUS 7
                   MINH      19      W       _MRD
                  _CDEF      20      X       CS
                   +5 V      21      Y       +5 V
                    GND      22      Z       GND

**********************************************************************/

#pragma once

#ifndef __VIP_EXPANSION_SLOT__
#define __VIP_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VIP_EXPANSION_SLOT_TAG      "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VIP_EXPANSION_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, VIP_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_VIP_EXPANSION_SLOT_INT_CALLBACK(_write) \
	devcb = &vip_expansion_slot_device::set_int_wr_callback(*device, DEVCB_##_write);

#define MCFG_VIP_EXPANSION_SLOT_DMA_OUT_CALLBACK(_write) \
	devcb = &vip_expansion_slot_device::set_dma_out_wr_callback(*device, DEVCB_##_write);

#define MCFG_VIP_EXPANSION_SLOT_DMA_IN_CALLBACK(_write) \
	devcb = &vip_expansion_slot_device::set_dma_in_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vip_expansion_slot_device

class device_vip_expansion_card_interface;

class vip_expansion_slot_device : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	vip_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_int_wr_callback(device_t &device, _Object object) { return downcast<vip_expansion_slot_device &>(device).m_write_int.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_out_wr_callback(device_t &device, _Object object) { return downcast<vip_expansion_slot_device &>(device).m_write_dma_out.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_in_wr_callback(device_t &device, _Object object) { return downcast<vip_expansion_slot_device &>(device).m_write_dma_in.set_callback(object); }

	// computer interface
	UINT8 program_r(address_space &space, offs_t offset, int cs, int cdef, int *minh);
	void program_w(address_space &space, offs_t offset, UINT8 data, int cdef, int *minh);
	UINT8 io_r(address_space &space, offs_t offset);
	void io_w(address_space &space, offs_t offset, UINT8 data);
	UINT8 dma_r(address_space &space, offs_t offset);
	void dma_w(address_space &space, offs_t offset, UINT8 data);
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_READ_LINE_MEMBER( ef1_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_READ_LINE_MEMBER( ef4_r );
	void sc_w(int data);
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_WRITE_LINE_MEMBER( run_w );

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( interrupt_w ) { m_write_int(state); }
	DECLARE_WRITE_LINE_MEMBER( dma_out_w ) { m_write_dma_out(state); }
	DECLARE_WRITE_LINE_MEMBER( dma_in_w ) { m_write_dma_in(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

	devcb_write_line m_write_int;
	devcb_write_line m_write_dma_out;
	devcb_write_line m_write_dma_in;

	device_vip_expansion_card_interface *m_card;
};


// ======================> device_vip_expansion_card_interface

class device_vip_expansion_card_interface : public device_slot_card_interface
{
	friend class vip_expansion_slot_device;

public:
	// construction/destruction
	device_vip_expansion_card_interface(const machine_config &mconfig, device_t &device);

protected:
	// runtime
	virtual UINT8 vip_program_r(address_space &space, offs_t offset, int cs, int cdef, int *minh) { return 0xff; };
	virtual void vip_program_w(address_space &space, offs_t offset, UINT8 data, int cdef, int *minh) { };

	virtual UINT8 vip_io_r(address_space &space, offs_t offset) { return 0xff; };
	virtual void vip_io_w(address_space &space, offs_t offset, UINT8 data) { };

	virtual UINT8 vip_dma_r(address_space &space, offs_t offset) { return 0xff; };
	virtual void vip_dma_w(address_space &space, offs_t offset, UINT8 data) { };

	virtual UINT32 vip_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; }

	virtual int vip_ef1_r() { return CLEAR_LINE; }
	virtual int vip_ef3_r() { return CLEAR_LINE; }
	virtual int vip_ef4_r() { return CLEAR_LINE; }

	virtual void vip_sc_w(int data) { };

	virtual void vip_q_w(int state) { };

	virtual void vip_run_w(int state) { };

	vip_expansion_slot_device *m_slot;
};


// device type definition
extern const device_type VIP_EXPANSION_SLOT;


SLOT_INTERFACE_EXTERN( vip_expansion_cards );



#endif
