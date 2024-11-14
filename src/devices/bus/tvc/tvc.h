// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    tvc.h

**********************************************************************

                   +----------+
          GND      | A01  B01 |       GND
          +5V      | A02  B02 |       +5V
         +12V      | A03  B03 |       +12V
         -12V      | A04  B04 |       -12V
         N.C.      | A05  B05 |       N.C.
         N.C.      | A06  B06 |       /HSYNC
    3.125 CLK      | A07  B07 |       /VSYNC
          DMA      | A08  B08 |       BORDER
         LPEN      | A09  B09 |       N.C.
          DEB      | A10  B10 |       MUX
    6.25 CLK       | A11  B11 |       A3
          A1       | A12  B12 |       EC3
         EC0       | A13  B13 |       EC1
       /EXTC       | A14  B14 |       EC2
    /PHANTOM       | A15  B15 |       TXRXCLK
        N.C.       | A16  B16 |       SOUND
         BD0       | A17  B17 |       N.C.
         BD1       | A18  B18 |       N.C.
         BD2       | A19  B19 |       N.C.
         BD3       | A20  B20 |       N.C.
         BD4       | A21  B21 |       N.C.
         BD5       | A22  B22 |       N.C.
         BD6       | A23  B23 |       N.C.
         BD7       | A24  B24 |       A15
         A17       | A25  B25 |       A5
          A8       | A26  B26 |       A12
          A7       | A27  B27 |       A6
          A9       | A28  B28 |       A11
          A4       | A29  B29 |       A10
         A13       | A30  B30 |       A2
     /VIDLSB       | A31  B31 |       A0
        /EXP       | A32  B32 |       P3
        N.C.       | A33  B33 |       /BUSRQ
         /P2       | A34  B34 |       N.C.
     /CLRINT       | A35  B35 |       RESET
         IDA       | A36  B36 |       /NMI
        /INT       | A37  B37 |       /WAIT
         IDB       | A38  B38 |       /INT
        /EXP       | A39  B39 |       N.C.
         /BM       | A40  B40 |       /BIORQ
     /BRESET       | A41  B41 |       /B0
        /BWR       | A42  B42 |       /BMREQ
        /WRD       | A43  B43 |       /BRFSH
                   +----------+
*********************************************************************/

#ifndef MAME_BUS_TVC_TVC_H
#define MAME_BUS_TVC_TVC_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> device_tvcexp_interface

class device_tvcexp_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_tvcexp_interface();

	// reading and writing
	virtual uint8_t id_r() { return 0x00; }   // ID_A and ID_B lines
	virtual void int_ack() { }
	virtual uint8_t int_r() { return 1; }
	virtual uint8_t read(offs_t offset) { return 0x00; }
	virtual void write(offs_t offset, uint8_t data) { }
	virtual uint8_t io_read(offs_t offset) { return 0x00; }
	virtual void io_write(offs_t offset, uint8_t data) { }

protected:
	device_tvcexp_interface(const machine_config &mconfig, device_t &device);
};

// ======================> tvcexp_slot_device

class tvcexp_slot_device : public device_t, public device_single_card_slot_interface<device_tvcexp_interface>
{
public:
	// construction/destruction
	template <typename T>
	tvcexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: tvcexp_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	tvcexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~tvcexp_slot_device();

	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }

	// reading and writing
	uint8_t id_r();
	void int_ack();
	uint8_t int_r();
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t io_read(offs_t offset);
	void io_write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	devcb_write_line            m_out_irq_cb;
	devcb_write_line            m_out_nmi_cb;

	device_tvcexp_interface*    m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(TVCEXP_SLOT, tvcexp_slot_device)


#endif // MAME_BUS_TVC_TVC_H
