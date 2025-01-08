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

#ifndef MAME_BUS_S100_S100_H
#define MAME_BUS_S100_S100_H

#pragma once

#include <functional>
#include <vector>



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class s100_bus_device;

// ======================> device_s100_card_interface

class device_s100_card_interface : public device_interface
{
	friend class s100_bus_device;

public:
	// interrupts
	virtual void s100_int_w(int state) { }
	virtual void s100_nmi_w(int state) { }
	virtual uint8_t s100_sinta_r(offs_t offset) { return 0xff; }

	// vectored interrupts
	virtual void s100_vi0_w(int state) { }
	virtual void s100_vi1_w(int state) { }
	virtual void s100_vi2_w(int state) { }
	virtual void s100_vi3_w(int state) { }
	virtual void s100_vi4_w(int state) { }
	virtual void s100_vi5_w(int state) { }
	virtual void s100_vi6_w(int state) { }
	virtual void s100_vi7_w(int state) { }

	// memory access
	virtual uint8_t s100_smemr_r(offs_t offset) { return 0xff; }
	virtual void s100_mwrt_w(offs_t offset, uint8_t data) { }

	// I/O access
	virtual uint8_t s100_sinp_r(offs_t offset) { return 0xff; }
	virtual void s100_sout_w(offs_t offset, uint8_t data) { }

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

protected:
	// construction/destruction
	device_s100_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	s100_bus_device  *m_bus;
};



// ======================> s100_bus_device

class s100_bus_device : public device_t
{
public:
	// construction/destruction
	s100_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~s100_bus_device();

	auto irq() { return m_write_irq.bind(); }
	auto nmi() { return m_write_nmi.bind(); }
	auto vi0() { return m_write_vi0.bind(); }
	auto vi1() { return m_write_vi1.bind(); }
	auto vi2() { return m_write_vi2.bind(); }
	auto vi3() { return m_write_vi3.bind(); }
	auto vi4() { return m_write_vi4.bind(); }
	auto vi5() { return m_write_vi5.bind(); }
	auto vi6() { return m_write_vi6.bind(); }
	auto vi7() { return m_write_vi7.bind(); }
	auto dma0() { return m_write_dma0.bind(); }
	auto dma1() { return m_write_dma1.bind(); }
	auto dma2() { return m_write_dma2.bind(); }
	auto dma3() { return m_write_dma3.bind(); }
	auto rdy() { return m_write_rdy.bind(); }
	auto hold() { return m_write_hold.bind(); }
	auto error() { return m_write_error.bind(); }

	void add_card(device_s100_card_interface &card);

	uint8_t smemr_r(offs_t offset);
	void mwrt_w(offs_t offset, uint8_t data);

	uint8_t sinp_r(offs_t offset);
	void sout_w(offs_t offset, uint8_t data);

	void irq_w(int state) { m_write_irq(state); }
	void nmi_w(int state) { m_write_nmi(state); }
	void vi0_w(int state) { m_write_vi0(state); }
	void vi1_w(int state) { m_write_vi1(state); }
	void vi2_w(int state) { m_write_vi2(state); }
	void vi3_w(int state) { m_write_vi3(state); }
	void vi4_w(int state) { m_write_vi4(state); }
	void vi5_w(int state) { m_write_vi5(state); }
	void vi6_w(int state) { m_write_vi6(state); }
	void vi7_w(int state) { m_write_vi7(state); }
	void dma0_w(int state) { m_write_dma0(state); }
	void dma1_w(int state) { m_write_dma1(state); }
	void dma2_w(int state) { m_write_dma2(state); }
	void dma3_w(int state) { m_write_dma3(state); }
	void rdy_w(int state) { m_write_rdy(state); }
	void hold_w(int state) { m_write_hold(state); }
	void error_w(int state) { m_write_error(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	using card_vector = std::vector<std::reference_wrapper<device_s100_card_interface> >;

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

	card_vector m_device_list;
};


// ======================> s100_slot_device

class s100_slot_device : public device_t, public device_single_card_slot_interface<device_s100_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	s100_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: s100_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1, 1))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	s100_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_bus(T &&tag) { m_bus.set_tag(std::forward<T>(tag)); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	required_device<s100_bus_device> m_bus;
};



// device type definition
DECLARE_DEVICE_TYPE(S100_BUS,  s100_bus_device)
DECLARE_DEVICE_TYPE(S100_SLOT, s100_slot_device)

#endif // MAME_BUS_S100_S100_H
