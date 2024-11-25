// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    GAYLE

    Gate array used in the Amiga 600 and Amiga 1200 computers.

    84-pin

     1  PE12         43  A14
     2  PE5          44  A15
     3  GND          45  GND
     4  NOISE        46  A16
     5  CC-RESET     47  A17
     6  _CC_ENA      48  A18
     7  _CC_REG      49  A19
     8  _CC_CEL      50  A20
     9  _CC_CEU      51  A21
    10  E            52  A22
    11  _FLASH       53  A23
    12  _IDE_IRQ     54  D7
    13  _IDE_CS1     55  D6
    14  _IDE_CS2     56  D5
    15  _SPARE_CS    57  D4
    16  _NET_CS      58  D3
    17  _RTC_CS      59  D2
    18  _IOWR        60  D1
    19  _IORD        61  D0
    20  VCC          62  VCC
    21  _ROMEN       63  _KBRESET
    22  C14M         64  DKWEB
    23  CCK          65  DKWDB
    24  GND          66  GND
    25  XRDY         67  MTRON
    26  _OVR         68  MTRX
    27  _OEL         69  DKWE
    28  _OEB         70  _DKWD
    29  _DBR         71  _MTR
    30  _BLS         72  _SEL
    31  _REGEN       73  _ODD_CIA
    32  _RAMEN       74  _EVEN_CIA
    33  _AS          75  _CC_CD1
    34  _UDS         76  _CC_CD2
    35  _LDS         77  _CC_BVD1
    36  R_W          78  _CC_BVD2
    37  _DTACK       79  CC_WP
    38  _BGACK       80  _CC_BUSY_IREQ
    39  _HLT         81  _WAIT
    40  _RST         82  _BERR
    41  A12          83  _INT6
    42  A13          84  _INT2

***************************************************************************/

#ifndef MAME_MACHINE_GAYLE_H
#define MAME_MACHINE_GAYLE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gayle_device

class gayle_device : public device_t
{
public:
	// construction/destruction
	gayle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto int2_handler() { return m_int2_w.bind(); }
	auto int6_handler() { return m_int6_w.bind(); }
	auto rst_handler() { return m_rst_w.bind(); }
	template<int N> auto ide_cs_r_cb() { return m_ide_cs_r_cb[N].bind(); }
	template<int N> auto ide_cs_w_cb() { return m_ide_cs_w_cb[N].bind(); }

	// interface
	void ide_interrupt_w(int state);

	// credit card signals
	void cc_cd1_w(int state);
	void cc_cd2_w(int state);
	void cc_bvd1_w(int state);
	void cc_bvd2_w(int state);
	void cc_wp_w(int state);

	void register_map(address_map &map) ATTR_COLD;
	uint16_t gayle_id_r(offs_t offset, uint16_t mem_mask = ~0);
	void gayle_id_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// inline configuration
	void set_id(uint8_t id) { m_gayle_id = id; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void dump_register();
	void line_change(int line, int state, int level);

	template<int N> uint16_t ide_cs_r(offs_t offset, uint16_t mem_mask);
	template<int N> void ide_cs_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t status_r();
	void status_w(uint16_t data);
	uint16_t change_r();
	void change_w(uint16_t data);
	uint16_t int_r();
	void int_w(uint16_t data);
	uint16_t control_r();
	void control_w(uint16_t data);

	enum
	{
		REG_STATUS = 0,
		REG_CHANGE,
		REG_INT,
		REG_CONTROL
	};

	enum
	{
		LINE_IDE = 7,
		LINE_CC_DET = 6,
		LINE_CC_BVD1_SC = 5,
		LINE_CC_BVD2_DA = 4,
		LINE_CC_WP = 3,
		LINE_CC_BUSY_IREQ = 2
	};

	devcb_write_line m_int2_w;
	devcb_write_line m_int6_w;
	devcb_write_line m_rst_w;

	devcb_read16::array<2> m_ide_cs_r_cb;
	devcb_write16::array<2> m_ide_cs_w_cb;

	// gayle id, bitwise readable by the cpu
	uint8_t m_gayle_id;
	uint8_t m_gayle_id_count;

	// gayle register, readable/writeable by the cpu
	uint8_t m_gayle_reg[4];

	// internal latched line state
	uint8_t m_line_state;
	std::array<int8_t, 2> m_cd;
};

// device type definition
DECLARE_DEVICE_TYPE(GAYLE, gayle_device)

#endif // MAME_MACHINE_GAYLE_H
