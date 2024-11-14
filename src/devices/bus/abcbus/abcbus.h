// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC (Databoard 4680) Bus emulation

**********************************************************************

                              ABC 80

                              A     B
                -12 V   <--   *  1  *   --> -12V
                0 V     ---   *  2  *   --- 0 V
                RESIN_  -->   *  3  *   --> XMEMWR_
                0 V     ---   *  4  *   --> XMEMFL_
                INT_    -->   *  5  *   --> phi
                D7      <->   *  6  *   --- 0 V
                D6      <->   *  7  *   --- 0 V
                D5      <->   *  8  *   --- 0 V
                D4      <->   *  9  *   --- 0 V
                D3      <->   * 10  *   --- 0 V
                D2      <->   * 11  *   --- 0 V
                D1      <->   * 12  *   --- 0 V
                D0      <->   * 13  *   --- 0 V
                              * 14  *   --> A15
                RST_    <--   * 15  *   --> A14
                IN1     <--   * 16  *   --> A13
                IN0     <--   * 17  *   --> A12
                OUT5    <--   * 18  *   --> A11
                OUT4    <--   * 19  *   --> A10
                OUT3    <--   * 20  *   --> A9
                OUT2    <--   * 21  *   --> A8
                OUT0    <--   * 22  *   --> A7
                OUT1    <--   * 23  *   --> A6
                NMI_    -->   * 24  *   --> A5
                INP2_   <--   * 25  *   --> A4
               XINPSTB_ <--   * 26  *   --> A3
              XOUTPSTB_ <--   * 27  *   --> A2
                XM_     -->   * 28  *   --> A1
                RFSH_   <--   * 29  *   --> A0
                RDY     -->   * 30  *   --> MEMRQ_
                +5 V    <--   * 31  *   --> +5 V
                +12 V   <--   * 32  *   --> +12 V

    OUT 0   _OUT    data output
    OUT 1   _CS     card select
    OUT 2   _C1     command 1
    OUT 3   _C2     command 2
    OUT 4   _C3     command 3
    OUT 5   _C4     command 4

    IN 0    _INP    data input
    IN 1    _STAT   status in
    IN 7    RST     reset

**********************************************************************

                             ABC 1600

                              A     B
                -12 V   ---   *  1  *   --- -12V
                0 V     ---   *  2  *   --- 0 V
                BPCLK*  ---   *  3  *   --- BPCLK
                0 V     ---   *  4  *   --- 0 V
                INT*    ---   *  5  *   --- 0 V
                D7      ---   *  6  *   --- 0 V
                D6      ---   *  7  *   ---
                D5      ---   *  8  *   ---
                D4      ---   *  9  *   ---
                D3      ---   * 10  *   --- XINT*5^
                D2      ---   * 11  *   --- XINT*4^
                D1      ---   * 12  *   --- XINT*3^
                D0      ---   * 13  *   --- XINT*2^
                CSB*    ---   * 14  *   --- XCSB*2^
                BRST*   ---   * 15  *   --- XCSB*3^
                STAT*   ---   * 16  *   --- XCSB*4^
                INP*    ---   * 17  *   --- XCSB*5^
                C4*     ---   * 18  *   ---
                C3*     ---   * 19  *   ---
                C2*     ---   * 20  *   ---
                C1*     ---   * 21  *   --- EXP*^
                OUT*    ---   * 22  *   --- BUSEN*^
                CS*     ---   * 23  *   --- DSTB*
                NMI*^   ---   * 24  *   --- 0 V
                OPS*    ---   * 25  *   --- A4
                R/W*    ---   * 26  *   --- A3
                TREN*   ---   * 27  *   --- A2
                TRRQ*   ---   * 28  *   --- A1
                PRAC*   ---   * 29  *   --- A0
                PREN*   ---   * 30  *   --- DIRW/R*
                +5 V    ---   * 31  *   --- +5 V
                +12 V   ---   * 32  *   --- +12 V

                    ^ only connected on BUS0X

**********************************************************************/

#ifndef MAME_DEVICES_ABCBUS_ABCBUS_H
#define MAME_DEVICES_ABCBUS_ABCBUS_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_abcbus_card_interface

class abcbus_slot_device;

class device_abcbus_card_interface : public device_interface
{
public:
	// required operation overrides
	virtual void abcbus_cs(uint8_t data) = 0;

	// optional operation overrides
	virtual uint8_t abcbus_inp() { return 0xff; }
	virtual void abcbus_out(uint8_t data) { }
	virtual uint8_t abcbus_stat() { return 0xff; }
	virtual void abcbus_c1(uint8_t data) { }
	virtual void abcbus_c2(uint8_t data) { }
	virtual void abcbus_c3(uint8_t data) { }
	virtual void abcbus_c4(uint8_t data) { }

	// optional operation overrides for ABC 80
	virtual uint8_t abcbus_xmemfl(offs_t offset) { return 0xff; }
	virtual void abcbus_xmemw(offs_t offset, uint8_t data) { }

	// optional operation overrides for ABC 1600
	virtual int abcbus_csb() { return 1; }
	virtual uint8_t abcbus_ops() { return 0xff; }
	virtual uint8_t abcbus_tren() { return 0xff; }
	virtual void abcbus_tren(uint8_t data) { }
	virtual void abcbus_prac(int state) { }
	virtual uint8_t abcbus_exp() { return 0xff; }
	virtual int abcbus_xcsb2() { return 1; }
	virtual int abcbus_xcsb3() { return 1; }
	virtual int abcbus_xcsb4() { return 1; }
	virtual int abcbus_xcsb5() { return 1; }

protected:
	// construction/destruction
	device_abcbus_card_interface(const machine_config &mconfig, device_t &device);

	abcbus_slot_device  *m_slot;

	friend class abcbus_slot_device;
};


// ======================> abcbus_slot_device

class abcbus_slot_device : public device_t,
							public device_single_card_slot_interface<device_abcbus_card_interface>
{
public:
	// construction/destruction
	abcbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T>
	abcbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: abcbus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	auto irq_callback() { return m_write_irq.bind(); }
	auto nmi_callback() { return m_write_nmi.bind(); }
	auto rdy_callback() { return m_write_rdy.bind(); }
	auto resin_callback() { return m_write_resin.bind(); }
	auto pren_callback() { return m_write_pren.bind(); }
	auto trrq_callback() { return m_write_trrq.bind(); }
	auto xint2_callback() { return m_write_xint2.bind(); }
	auto xint3_callback() { return m_write_xint3.bind(); }
	auto xint4_callback() { return m_write_xint4.bind(); }
	auto xint5_callback() { return m_write_xint5.bind(); }

	// computer interface
	void write_cs(uint8_t data) { if (m_card) m_card->abcbus_cs(data); }
	uint8_t read_rst() { device_reset(); return 0xff; }
	uint8_t read_inp() { return m_card ? m_card->abcbus_inp() : 0xff; }
	void write_out(uint8_t data) { if (m_card) m_card->abcbus_out(data); }
	uint8_t read_stat() { return m_card ? m_card->abcbus_stat() : 0xff; }
	void write_c1(uint8_t data) { if (m_card) m_card->abcbus_c1(data); }
	void write_c2(uint8_t data) { if (m_card) m_card->abcbus_c2(data); }
	void write_c3(uint8_t data) { if (m_card) m_card->abcbus_c3(data); }
	void write_c4(uint8_t data) { if (m_card) m_card->abcbus_c4(data); }
	uint8_t xmemfl_r(offs_t offset) { return m_card ? m_card->abcbus_xmemfl(offset) : 0xff; }
	void xmemw_w(offs_t offset, uint8_t data) { if (m_card) m_card->abcbus_xmemw(offset, data); }
	int csb_r() { return m_card ? m_card->abcbus_csb() : 1; }
	uint8_t ops_r() { return m_card ? m_card->abcbus_ops() : 0xff; }
	uint8_t exp_r() { return m_card ? m_card->abcbus_exp() : 0xff; }
	int xcsb2_r() { return m_card ? m_card->abcbus_xcsb2() : 1; }
	int xcsb3_r() { return m_card ? m_card->abcbus_xcsb3() : 1; }
	int xcsb4_r() { return m_card ? m_card->abcbus_xcsb4() : 1; }
	int xcsb5_r() { return m_card ? m_card->abcbus_xcsb5() : 1; }
	uint8_t read_tren() { return m_card ? m_card->abcbus_tren() : 0xff; }
	void write_tren(uint8_t data) { if (m_card) m_card->abcbus_tren(data); }
	void prac_w(int state) { if (m_card) m_card->abcbus_prac(state); }

	void cs_w(uint8_t data) { write_cs(data); }
	uint8_t rst_r() { return read_rst(); }
	uint8_t inp_r() { return read_inp(); }
	void out_w(uint8_t data) { write_out(data); }
	uint8_t stat_r() { return read_stat(); }
	void c1_w(uint8_t data) { write_c1(data); }
	void c2_w(uint8_t data) { write_c2(data); }
	void c3_w(uint8_t data) { write_c3(data); }
	void c4_w(uint8_t data) { write_c4(data); }

	int irq_r() { return m_irq; }
	int nmi_r() { return m_nmi; }
	int pren_r() { return m_pren; }
	int trrq_r() { return m_trrq; }
	int xint2_r() { return m_xint2; }
	int xint3_r() { return m_xint3; }
	int xint4_r() { return m_xint4; }
	int xint5_r() { return m_xint5; }

	// card interface
	void irq_w(int state) { if (m_irq != state) { m_irq = state; m_write_irq(state); } }
	void nmi_w(int state) { m_nmi = state; m_write_nmi(state); }
	void rdy_w(int state) { m_write_rdy(state); }
	void resin_w(int state) { m_write_resin(state); }
	void pren_w(int state) { if (m_pren != state) { m_pren = state; m_write_pren(state); } }
	void trrq_w(int state) { if (m_trrq != state) { m_trrq = state; m_write_trrq(state); } }
	void xint2_w(int state) { m_xint2 = state; m_write_xint2(state); }
	void xint3_w(int state) { m_xint3 = state; m_write_xint3(state); }
	void xint4_w(int state) { m_xint4 = state; m_write_xint4(state); }
	void xint5_w(int state) { m_xint5 = state; m_write_xint5(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_nmi;
	devcb_write_line   m_write_rdy;
	devcb_write_line   m_write_resin;
	devcb_write_line   m_write_pren;
	devcb_write_line   m_write_trrq;
	devcb_write_line   m_write_xint2;
	devcb_write_line   m_write_xint3;
	devcb_write_line   m_write_xint4;
	devcb_write_line   m_write_xint5;

	device_abcbus_card_interface *m_card;

	int m_irq;
	int m_nmi;
	int m_pren;
	int m_trrq;
	int m_xint2;
	int m_xint3;
	int m_xint4;
	int m_xint5;
};


// device type definition
DECLARE_DEVICE_TYPE(ABCBUS_SLOT, abcbus_slot_device)


void abc80_cards(device_slot_interface &device);
void abcbus_cards(device_slot_interface &device);
void abc1600bus_cards(device_slot_interface &device);


typedef device_type_enumerator<abcbus_slot_device> abcbus_slot_device_enumerator;


#endif // MAME_DEVICES_ABCBUS_ABCBUS_H
