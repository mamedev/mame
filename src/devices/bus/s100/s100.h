// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    S-100 (IEEE Std 696-1983) bus emulation

**********************************************************************

                +8 V (B)     1      51      +8 V (B)
                +16 V (B)    2      52      -16 V (B)
                XRDY (S)     3      53      0 V
                VI0* (S)     4      54      SLAVE CLR* (B)
                VI1* (S)     5      55      TMA0* (M)
                VI2* (S)     6      56      TMA1* (M)
                VI3* (S)     7      57      TMA2* (M)
                VI4* (S)     8      58      sXTRQ* (M)
                VI5* (S)     9      59      A19 (M)
                VI6* (S)    10      60      SIXTN* (S)
                VI7* (S)    11      61      A20 (M)
                NMI* (S)    12      62      A21 (M)
            PWRFAIL* (B)    13      63      A22 (M)
                TMA3* (M)   14      64      A23 (M)
                A18 (M)     15      65      NDEF
                A16 (M)     16      66      NDEF
                A17 (M)     17      67      PHANTOM* (M/S)
                SDSB* (M)   18      68      MWRT (B)
                CDSB* (M)   19      69      RFU
                0 V         20      70      0 V
                NDEF        21      71      RFU
                ADSB* (M)   22      72      RDY (S)
                DODSB* (M)  23      73      INT* (S)
                phi (B)     24      74      HOLD* (M)
            pSTVAL* (M)     25      75      RESET* (B)
                pHLDA (M)   26      76      pSYNC (M)
                RFU         27      77      pWR* (M)
                RFU         28      78      pDBIN (M)
                A5 (M)      29      79      A0 (M)
                A4 (M)      30      80      A1 (M)
                A3 (M)      31      81      A2 (M)
                A15 (M)     32      82      A6 (M)
                A12 (M)     33      83      A7 (M)
                A9 (M)      34      84      A8 (M)
        DO1 (M)/ED1 (M/S)   35      85      A13 (M)
        DO0 (M)/ED0 (M/S)   36      86      A14 (M)
                A10 (M)     37      87      A11 (M)
        DO4 (M)/ED4 (M/S)   38      88      DO2 (M)/ED2 (M/S)
        DO5 (M)/ED5 (M/S)   39      89      DO3 (M)/ED3 (M/S)
        DO6 (M)/ED6 (M/S)   40      90      DO7 (M)/ED7 (M/S)
        DI2 (M)/OD2 (M/S)   41      91      DI4 (M)/OD4 (M/S)
        DI3 (M)/OD3 (M/S)   42      92      DI5 (M)/OD5 (M/S)
        DI7 (M)/OD7 (M/S)   43      93      DI6 (M)/OD6 (M/S)
                sM1 (M)     44      94      DI1 (M)/OD1 (M/S)
                sOUT (M)    45      95      DI0 (M)/OD0 (M/S)
                sINP (M)    46      96      sINTA (M)
                sMEMR (M)   47      97      sWO* (M)
                sHLTA (M)   48      98      ERROR* (S)
                CLOCK (B)   49      99      POC* (B)
                0 V         50      100     0 V

**********************************************************************/

#pragma once

#ifndef __S100__
#define __S100__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define S100_TAG        "s100"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_S100_BUS_ADD() \
	MCFG_DEVICE_ADD(S100_TAG, S100_BUS, 0)

#define MCFG_S100_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, S100_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_S100_IRQ_CALLBACK(_write) \
	devcb = &s100_bus_t::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_NMI_CALLBACK(_write) \
	devcb = &s100_bus_t::set_nmi_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_VI0_CALLBACK(_write) \
	devcb = &s100_bus_t::set_vi0_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_VI1_CALLBACK(_write) \
	devcb = &s100_bus_t::set_vi1_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_VI2_CALLBACK(_write) \
	devcb = &s100_bus_t::set_vi2_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_VI3_CALLBACK(_write) \
	devcb = &s100_bus_t::set_vi3_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_VI4_CALLBACK(_write) \
	devcb = &s100_bus_t::set_vi4_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_VI5_CALLBACK(_write) \
	devcb = &s100_bus_t::set_vi5_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_VI6_CALLBACK(_write) \
	devcb = &s100_bus_t::set_vi6_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_VI7_CALLBACK(_write) \
	devcb = &s100_bus_t::set_vi7_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_DMA0_CALLBACK(_write) \
	devcb = &s100_bus_t::set_dma0_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_DMA1_CALLBACK(_write) \
	devcb = &s100_bus_t::set_dma1_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_DMA2_CALLBACK(_write) \
	devcb = &s100_bus_t::set_dma2_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_DMA3_CALLBACK(_write) \
	devcb = &s100_bus_t::set_dma3_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_RDY_CALLBACK(_write) \
	devcb = &s100_bus_t::set_rdy_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_HOLD_CALLBACK(_write) \
	devcb = &s100_bus_t::set_hold_wr_callback(*device, DEVCB_##_write);

#define MCFG_S100_ERROR_CALLBACK(_write) \
	devcb = &s100_bus_t::set_error_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class s100_bus_t;

// ======================> device_s100_card_interface

class device_s100_card_interface : public device_slot_card_interface
{
	friend class s100_bus_t;

public:
	// construction/destruction
	device_s100_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_s100_card_interface() { }

	device_s100_card_interface *next() const { return m_next; }

	// interrupts
	virtual void s100_int_w(int state) { };
	virtual void s100_nmi_w(int state) { };
	virtual UINT8 s100_sinta_r(offs_t offset) { return 0; };

	// vectored interrupts
	virtual void s100_vi0_w(int state) { };
	virtual void s100_vi1_w(int state) { };
	virtual void s100_vi2_w(int state) { };
	virtual void s100_vi3_w(int state) { };
	virtual void s100_vi4_w(int state) { };
	virtual void s100_vi5_w(int state) { };
	virtual void s100_vi6_w(int state) { };
	virtual void s100_vi7_w(int state) { };

	// memory access
	virtual UINT8 s100_smemr_r(address_space &space, offs_t offset) { return 0; };
	virtual void s100_mwrt_w(address_space &space, offs_t offset, UINT8 data) { };

	// I/O access
	virtual UINT8 s100_sinp_r(address_space &space, offs_t offset) { return 0; };
	virtual void s100_sout_w(address_space &space, offs_t offset, UINT8 data) { };

	// configuration access
	virtual void s100_phlda_w(int state) { }
	virtual void s100_shalta_w(int state) { }
	virtual void s100_phantom_w(int state) { }
	virtual void s100_sxtrq_w(int state) { }
	virtual int s100_sixtn_r() { return 1; }

	// reset
	virtual void s100_poc_w(int state) { }
	virtual void s100_reset_w(int state) { }
	virtual void s100_slave_clr_w(int state) { }

public:
	s100_bus_t  *m_bus;
	device_s100_card_interface *m_next;
};



// ======================> s100_bus_t

class s100_bus_t : public device_t
{
public:
	// construction/destruction
	s100_bus_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~s100_bus_t() { m_device_list.detach_all(); }

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_nmi_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_nmi.set_callback(object); }
	template<class _Object> static devcb_base &set_vi0_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_vi0.set_callback(object); }
	template<class _Object> static devcb_base &set_vi1_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_vi1.set_callback(object); }
	template<class _Object> static devcb_base &set_vi2_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_vi2.set_callback(object); }
	template<class _Object> static devcb_base &set_vi3_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_vi3.set_callback(object); }
	template<class _Object> static devcb_base &set_vi4_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_vi4.set_callback(object); }
	template<class _Object> static devcb_base &set_vi5_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_vi5.set_callback(object); }
	template<class _Object> static devcb_base &set_vi6_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_vi6.set_callback(object); }
	template<class _Object> static devcb_base &set_vi7_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_vi7.set_callback(object); }
	template<class _Object> static devcb_base &set_dma0_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_dma0.set_callback(object); }
	template<class _Object> static devcb_base &set_dma1_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_dma1.set_callback(object); }
	template<class _Object> static devcb_base &set_dma2_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_dma2.set_callback(object); }
	template<class _Object> static devcb_base &set_dma3_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_dma3.set_callback(object); }
	template<class _Object> static devcb_base &set_rdy_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_rdy.set_callback(object); }
	template<class _Object> static devcb_base &set_hold_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_hold.set_callback(object); }
	template<class _Object> static devcb_base &set_error_wr_callback(device_t &device, _Object object) { return downcast<s100_bus_t &>(device).m_write_error.set_callback(object); }

	void add_card(device_s100_card_interface *card);

	DECLARE_READ8_MEMBER( smemr_r );
	DECLARE_WRITE8_MEMBER( mwrt_w );

	DECLARE_READ8_MEMBER( sinp_r );
	DECLARE_WRITE8_MEMBER( sout_w );

	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_write_nmi(state); }
	DECLARE_WRITE_LINE_MEMBER( vi0_w ) { m_write_vi0(state); }
	DECLARE_WRITE_LINE_MEMBER( vi1_w ) { m_write_vi1(state); }
	DECLARE_WRITE_LINE_MEMBER( vi2_w ) { m_write_vi2(state); }
	DECLARE_WRITE_LINE_MEMBER( vi3_w ) { m_write_vi3(state); }
	DECLARE_WRITE_LINE_MEMBER( vi4_w ) { m_write_vi4(state); }
	DECLARE_WRITE_LINE_MEMBER( vi5_w ) { m_write_vi5(state); }
	DECLARE_WRITE_LINE_MEMBER( vi6_w ) { m_write_vi6(state); }
	DECLARE_WRITE_LINE_MEMBER( vi7_w ) { m_write_vi7(state); }
	DECLARE_WRITE_LINE_MEMBER( dma0_w ) { m_write_dma0(state); }
	DECLARE_WRITE_LINE_MEMBER( dma1_w ) { m_write_dma1(state); }
	DECLARE_WRITE_LINE_MEMBER( dma2_w ) { m_write_dma2(state); }
	DECLARE_WRITE_LINE_MEMBER( dma3_w ) { m_write_dma3(state); }
	DECLARE_WRITE_LINE_MEMBER( rdy_w ) { m_write_rdy(state); }
	DECLARE_WRITE_LINE_MEMBER( hold_w ) { m_write_hold(state); }
	DECLARE_WRITE_LINE_MEMBER( error_w ) { m_write_error(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_nmi;
	devcb_write_line   m_write_vi0;
	devcb_write_line   m_write_vi1;
	devcb_write_line   m_write_vi2;
	devcb_write_line   m_write_vi3;
	devcb_write_line   m_write_vi4;
	devcb_write_line   m_write_vi5;
	devcb_write_line   m_write_vi6;
	devcb_write_line   m_write_vi7;
	devcb_write_line   m_write_dma0;
	devcb_write_line   m_write_dma1;
	devcb_write_line   m_write_dma2;
	devcb_write_line   m_write_dma3;
	devcb_write_line   m_write_rdy;
	devcb_write_line   m_write_hold;
	devcb_write_line   m_write_error;

	simple_list<device_s100_card_interface> m_device_list;
};


// ======================> s100_slot_t

class s100_slot_t : public device_t,
					public device_slot_interface
{
public:
	// construction/destruction
	s100_slot_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;

private:
	s100_bus_t  *m_bus;
};



// device type definition
extern const device_type S100_BUS;
extern const device_type S100_SLOT;




#endif
