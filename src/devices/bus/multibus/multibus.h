// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    multibus.h

    Intel Multibus
    
    P1 Multibus connector:

    Power supplies
    1   GND        2   GND
    3   +5Vdc      4   +5Vdc
    5   +5Vdc      6   +5Vdc
    7   +12Vdc     8   +12Vdc
    9   -5Vdc      10  -5Vdc
    11  GND        12  GND
    Bus controls
    13  BCLK/      14  INIT/
    15  BPRN/      16  BPRO/
    17  BUSY/      18  BREQ/
    19  MRDC/      20  MWTC/
    21  IORC/      22  IOWC/
    23  XACK/      24  INH1/
    Bus controls and address
    25  LOCK/      26  INH2/
    27  BHEN/      28  ADR10/
    29  CBRQ/      30  ADR11/
    31  CCLK/      32  ADR12/
    33  INTA/      34  ADR13/
    Interrupts
    35  INT6/      36  INT7/
    37  INT4/      38  INT5/
    39  INT2/      40  INT3/
    41  INT0/      42  INT1/
    Address
    43  ADRE/      44  ADRF/
    45  ADRC/      46  ADRD/
    47  ADRA/      48  ADRB/
    49  ADR8/      50  ADR9/
    51  ADR6/      52  ADR7/
    53  ADR4/      54  ADR5/
    55  ADR2/      56  ADR3/
    57  ADR0/      58  ADR1/
    Data
    59  DATE/      60  DATF/
    61  DATC/      62  DATD/
    63  DATA/      64  DATB/
    65  DAT8/      66  DAT9/
    67  DAT6/      68  DAT7/
    69  DAT4/      70  DAT5/
    71  DAT2/      72  DAT3/
    73  DAT0/      74  DAT1/
    Power supplies
    75  GND        76  GND
    77  reserved   78  reserved
    79  -12Vdc     80  -12Vdc
    81  +5Vdc      82  +5Vdc
    83  +5Vdc      84  +5Vdc
    85  GND        86  GND
    
    P2 Multibus connector:
    
    1-54 reserved for iLBX bus
    
    Address
    55  ADR16/     56  ADR17/
    57  ADR14/     58  ADR15/
    
    59-60 reserved for iLBx bus
        
*********************************************************************/

#ifndef MAME_BUS_MULTIBUS_MULTIBUS_H
#define MAME_BUS_MULTIBUS_MULTIBUS_H

#pragma once

class device_multibus_interface;

class multibus_slot_device : public device_t,
							 public device_single_card_slot_interface<device_multibus_interface>
{
public:
	multibus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~multibus_slot_device();

	static constexpr unsigned BUS_CLOCK = 9830400;

	// Install r/w functions in I/O space
	void install_io_rw(address_space& space);

	// Set memory space
	void install_mem_rw(address_space& space);
	
	auto irq0_callback() { return m_irq_cb[0].bind(); }
	auto irq1_callback() { return m_irq_cb[1].bind(); }
	auto irq2_callback() { return m_irq_cb[2].bind(); }
	auto irq3_callback() { return m_irq_cb[3].bind(); }
	auto irq4_callback() { return m_irq_cb[4].bind(); }
	auto irq5_callback() { return m_irq_cb[5].bind(); }
	auto irq6_callback() { return m_irq_cb[6].bind(); }
	auto irq7_callback() { return m_irq_cb[7].bind(); }
	auto irqa_callback() { return m_irqa_cb.bind(); }
	
	DECLARE_WRITE_LINE_MEMBER( irq0_w );
	DECLARE_WRITE_LINE_MEMBER( irq1_w );
	DECLARE_WRITE_LINE_MEMBER( irq2_w );
	DECLARE_WRITE_LINE_MEMBER( irq3_w );
	DECLARE_WRITE_LINE_MEMBER( irq4_w );
	DECLARE_WRITE_LINE_MEMBER( irq5_w );
	DECLARE_WRITE_LINE_MEMBER( irq6_w );
	DECLARE_WRITE_LINE_MEMBER( irq7_w );
	DECLARE_WRITE_LINE_MEMBER( irqa_w );
	
protected:
	virtual void device_start() override;
	
	devcb_write_line::array<8> m_irq_cb;
	devcb_write_line           m_irqa_cb;

};

class device_multibus_interface : public device_interface
{
public:
	// Install r/w functions in I/O space
	virtual void install_io_rw(address_space& space) = 0;

	// Set CPU memory space
	virtual void install_mem_rw(address_space& space) = 0;
	
protected:
	device_multibus_interface(const machine_config &mconfig , device_t &device);
	virtual ~device_multibus_interface();
};

// device type declaration
DECLARE_DEVICE_TYPE(MULTIBUS_SLOT, multibus_slot_device)

#endif /* MAME_BUS_MULTIBUS_MULTIBUS_H */
