// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68kexp.h
 *
 * Expansion slots for the X680x0 series
 *
 * Pinout: (from http://www.amy.hi-ho.ne.jp/shimada/neptune/x68k.html)
                +-----+
    GND     B1  |[] []| A1      GND
    10MHz   B2  |[] []| A2      20MHz
    #10MHz  B3  |[] []| A3      GND
    E       B4  |[] []| A4      DB0
    AB1     B5  |[] []| A5      DB1
    AB2     B6  |[] []| A6      DB2
    AB3     B7  |[] []| A7      DB3
    AB4     B8  |[] []| A8      DB4
    AB5     B9  |[] []| A9      DB5
    AB6     B10 |[] []| A10     DB6
    GND     B11 |[] []| A11     GND
    AB7     B12 |[] []| A12     DB7
    AB8     B13 |[] []| A13     DB8
    AB9     B14 |[] []| A14     DB9
    AB10    B15 |[] []| A15     DB10
    AB11    B16 |[] []| A16     DB11
    AB12    B17 |[] []| A17     DB12
    AB13    B18 |[] []| A18     DB13
    AB14    B19 |[] []| A19     DB14
    AB15    B20 |[] []| A20     DB15
    GND     B21 |[] []| A21     GND
    AB16    B22 |[] []| A22     +12V
    AB17    B23 |[] []| A23     +12V
    AB18    B24 |[] []| A24     FC0
    AB19    B25 |[] []| A25     FC1
    AB20    B26 |[] []| A26     FC2
    AB21    B27 |[] []| A27     #AS
    AB22    B28 |[] []| A28     #LDS
    AB23    B29 |[] []| A29     #UDS
    IDDIR   B30 |[] []| A30     R/#W
    N.C.    B31 |[] []| A31     N.C.
    HSYNC   B32 |[] []| A32     -12V
    VSYNC   B33 |[] []| A33     -12V
    #DONE   B34 |[] []| A34     #VMA
    #DTC    B35 |[] []| A35     #EXVPA
    #EXREQ  B36 |[] []| A36     #DTACK
    #EXACK  B37 |[] []| A37     #EXRESET
    #EXPCL  B38 |[] []| A38     #HALT
    #EXOWN  B39 |[] []| A39     #EXBERR
    #EXNMI  B40 |[] []| A40     #EXPWON
    GND     B41 |[] []| A41     GND
    #IRQ2   B42 |[] []| A42     Vcc2
    #IRQ4   B43 |[] []| A43     Vcc2
    #IACK2  B44 |[] []| A44     SELEN
    #IACK4  B45 |[] []| A45     CASRDEN
    #BR     B46 |[] []| A46     CASWRL
    #BG     B47 |[] []| A47     CASWRU
    #BGACK  B48 |[] []| A48     INH2
    Vcc1    B49 |[] []| A49     Vcc1
    Vcc1    B50 |[] []| A50     Vcc1
                +-----+
 *
 */

#ifndef MAME_BUS_X68K_X68KEXP_H
#define MAME_BUS_X68K_X68KEXP_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_x68k_expansion_card_interface

// class representing interface-specific live x68k_expansion card
class device_x68k_expansion_card_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_x68k_expansion_card_interface();

	// reset
	virtual void x68k_reset_w() { }

	virtual uint8_t iack2();
	virtual uint8_t iack4();

protected:
	device_x68k_expansion_card_interface(const machine_config &mconfig, device_t &device);
};


// ======================> x68k_expansion_slot_device

class x68k_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_x68k_expansion_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	x68k_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: x68k_expansion_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	x68k_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~x68k_expansion_slot_device();

	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }

	auto out_irq2_callback() { return m_out_irq2_cb.bind(); }
	auto out_irq4_callback() { return m_out_irq4_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }
	auto out_reset_callback() { return m_out_reset_cb.bind(); }

	address_space &space() { return *m_space; }

	void irq2_w(int state);
	void irq4_w(int state);
	void nmi_w(int state);
	void reset_w(int state);

	uint8_t iack2() { return (m_card != nullptr) ? m_card->iack2() : 0x18; }
	uint8_t iack4() { return (m_card != nullptr) ? m_card->iack4() : 0x18; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	required_address_space m_space;

	devcb_write_line    m_out_irq2_cb;
	devcb_write_line    m_out_irq4_cb;
	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_reset_cb;

	device_x68k_expansion_card_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(X68K_EXPANSION_SLOT, x68k_expansion_slot_device)

#endif // MAME_BUS_X68K_X68KEXP_H
