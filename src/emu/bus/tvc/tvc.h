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

#ifndef __TVCEXP_H__
#define __TVCEXP_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> device_tvcexp_interface

class device_tvcexp_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_tvcexp_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_tvcexp_interface();

	// reading and writing
	virtual UINT8 id_r() { return 0x00; }   // ID_A and ID_B lines
	virtual void int_ack() { }
	virtual UINT8 int_r() { return 1; }
	virtual DECLARE_READ8_MEMBER(read) { return 0x00; }
	virtual DECLARE_WRITE8_MEMBER(write) {}
	virtual DECLARE_READ8_MEMBER(io_read) { return 0x00; }
	virtual DECLARE_WRITE8_MEMBER(io_write) {}
};

// ======================> tvcexp_slot_device

class tvcexp_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	tvcexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~tvcexp_slot_device();

	template<class _Object> static devcb_base &set_out_irq_callback(device_t &device, _Object object) { return downcast<tvcexp_slot_device &>(device).m_out_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_nmi_callback(device_t &device, _Object object) { return downcast<tvcexp_slot_device &>(device).m_out_nmi_cb.set_callback(object); }

	// device-level overrides
	virtual void device_start();

	// reading and writing
	virtual UINT8 id_r();
	virtual void int_ack();
	virtual UINT8 int_r();
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);
	virtual DECLARE_READ8_MEMBER(io_read);
	virtual DECLARE_WRITE8_MEMBER(io_write);

	devcb_write_line                m_out_irq_cb;
	devcb_write_line                m_out_nmi_cb;

	device_tvcexp_interface*    m_cart;
};

// device type definition
extern const device_type TVCEXP_SLOT;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TVCEXP_SLOT_OUT_IRQ_CB(_devcb) \
	devcb = &tvcexp_slot_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_TVCEXP_SLOT_OUT_NMI_CB(_devcb) \
	devcb = &tvcexp_slot_device::set_out_nmi_callback(*device, DEVCB_##_devcb);

#endif /* __TVCEXP_H__ */
