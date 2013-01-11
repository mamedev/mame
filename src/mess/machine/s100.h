/**********************************************************************

    S-100 (IEEE Std 696-1983) bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#define MCFG_S100_BUS_ADD(_cpu_tag, _config) \
	MCFG_DEVICE_ADD(S100_TAG, S100, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	s100_device::static_set_cputag(*device, _cpu_tag);

#define S100_INTERFACE(_name) \
	const s100_bus_interface (_name) =


#define MCFG_S100_SLOT_ADD(_tag, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, S100_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	s100_slot_device::static_set_s100_slot(*device, S100_TAG);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_slot_device

class s100_device;

class s100_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	s100_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

	// inline configuration
	static void static_set_s100_slot(device_t &device, const char *tag);

private:
	// configuration
	const char *m_bus_tag;
	s100_device  *m_bus;
};


// device type definition
extern const device_type S100_SLOT;


// ======================> s100_bus_interface

struct s100_bus_interface
{
	devcb_write_line    m_out_int_cb;
	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_vi0_cb;
	devcb_write_line    m_out_vi1_cb;
	devcb_write_line    m_out_vi2_cb;
	devcb_write_line    m_out_vi3_cb;
	devcb_write_line    m_out_vi4_cb;
	devcb_write_line    m_out_vi5_cb;
	devcb_write_line    m_out_vi6_cb;
	devcb_write_line    m_out_vi7_cb;
	devcb_write_line    m_out_dma0_cb;
	devcb_write_line    m_out_dma1_cb;
	devcb_write_line    m_out_dma2_cb;
	devcb_write_line    m_out_dma3_cb;
	devcb_write_line    m_out_rdy_cb;
	devcb_write_line    m_out_hold_cb;
	devcb_write_line    m_out_error_cb;
	devcb_write8        m_out_terminal_cb;
};

class device_s100_card_interface;


// ======================> s100_device

class s100_device : public device_t,
					public s100_bus_interface
{
public:
	// construction/destruction
	s100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);

	void add_s100_card(device_s100_card_interface *card);

	DECLARE_READ8_MEMBER( smemr_r );
	DECLARE_WRITE8_MEMBER( mwrt_w );

	DECLARE_READ8_MEMBER( sinp_r );
	DECLARE_WRITE8_MEMBER( sout_w );

	DECLARE_WRITE_LINE_MEMBER( int_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( vi0_w );
	DECLARE_WRITE_LINE_MEMBER( vi1_w );
	DECLARE_WRITE_LINE_MEMBER( vi2_w );
	DECLARE_WRITE_LINE_MEMBER( vi3_w );
	DECLARE_WRITE_LINE_MEMBER( vi4_w );
	DECLARE_WRITE_LINE_MEMBER( vi5_w );
	DECLARE_WRITE_LINE_MEMBER( vi6_w );
	DECLARE_WRITE_LINE_MEMBER( vi7_w );
	DECLARE_WRITE_LINE_MEMBER( dma0_w );
	DECLARE_WRITE_LINE_MEMBER( dma1_w );
	DECLARE_WRITE_LINE_MEMBER( dma2_w );
	DECLARE_WRITE_LINE_MEMBER( dma3_w );
	DECLARE_WRITE_LINE_MEMBER( rdy_w );
	DECLARE_WRITE_LINE_MEMBER( hold_w );
	DECLARE_WRITE_LINE_MEMBER( error_w );

	DECLARE_WRITE8_MEMBER( terminal_receive_w );
	DECLARE_WRITE8_MEMBER( terminal_transmit_w );
	void terminal_transmit_w(UINT8 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

private:
	// internal state
	cpu_device   *m_maincpu;

	devcb_resolved_write_line   m_out_int_func;
	devcb_resolved_write_line   m_out_nmi_func;
	devcb_resolved_write_line   m_out_vi0_func;
	devcb_resolved_write_line   m_out_vi1_func;
	devcb_resolved_write_line   m_out_vi2_func;
	devcb_resolved_write_line   m_out_vi3_func;
	devcb_resolved_write_line   m_out_vi4_func;
	devcb_resolved_write_line   m_out_vi5_func;
	devcb_resolved_write_line   m_out_vi6_func;
	devcb_resolved_write_line   m_out_vi7_func;
	devcb_resolved_write_line   m_out_dma0_func;
	devcb_resolved_write_line   m_out_dma1_func;
	devcb_resolved_write_line   m_out_dma2_func;
	devcb_resolved_write_line   m_out_dma3_func;
	devcb_resolved_write_line   m_out_rdy_func;
	devcb_resolved_write_line   m_out_hold_func;
	devcb_resolved_write_line   m_out_error_func;
	devcb_resolved_write8       m_out_terminal_func;

	simple_list<device_s100_card_interface> m_device_list;
	const char *m_cputag;
};


// device type definition
extern const device_type S100;


// ======================> device_s100_card_interface

// class representing interface-specific live s100 card
class device_s100_card_interface : public device_slot_card_interface
{
	friend class s100_device;

public:
	// construction/destruction
	device_s100_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_s100_card_interface();

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

	// terminal
	virtual bool s100_has_terminal() { return false; }
	virtual void s100_terminal_w(UINT8 data) { }

	// reset
	virtual void s100_poc_w(int state) { }
	virtual void s100_reset_w(int state) { }
	virtual void s100_slave_clr_w(int state) { }

public:
	s100_device  *m_bus;
	device_s100_card_interface *m_next;
};

#endif
