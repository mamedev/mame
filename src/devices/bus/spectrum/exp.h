// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        ZX Spectrum Expansion Port emulation

**********************************************************************

    Pinout:

             48K                          128K/+2                          +2A/+2B/+3/3B

         A         B                    A         B                         A         B
        A15   1   A14                  A15   1   A14                       A15   1   A14
        A13   2   A12                  A13   2   A12                       A13   2   A12
         D7   3   +5V                   D7   3   +5v                        D7   3   +5v
         NC   4   +9V                   NC   4   +9V                  /OE ROM1   4   NC
       SLOT   5   SLOT                SLOT   5   SLOT                     SLOT   5   SLOT
         D0   6   GND                   D0   6   GND                        D0   6   GND
         D1   7   GND                   D1   7   GND                        D1   7   GND
         D2   8   CLK                   D2   8   CLK                        D2   8   CKEXT
         D6   9   A0                    D6   9   A0                         D6   9   A0
         D5  10   A1                    D5  10   A1                         D5  10   A1
         D3  11   A2                    D3  11   A2                         D3  11   A2
         D4  12   A3                    D4  12   A3                         D4  12   A3
       /INT  13   /IORQGE             /INT  13   /IORQGE (+2 only)        /INT  13   NC
       /NMI  14   GND                 /NMI  14   GND                      /NMI  14   GND
      /HALT  15   VIDEO              /HALT  15   NC                      /HALT  15   /OE ROM2
      /MREQ  16   /Y                 /MREQ  16   NC                      /MREQ  16   /DRD
      /IORQ  17   V                  /IORQ  17   NC                      /IORQ  17   /DWR
        /RD  18   U                    /RD  18   NC                        /RD  18   /MTR
        /WR  19   /BUSRQ               /WR  19   /BUSRQ                    /WR  19   /BUSRQ
        -5V  20   /RESET               -5V  20   /RESET                     NC  20   /RESET
      /WAIT  21   A7                 /WAIT  21   A7                      /WAIT  21   A7
       +12V  22   A6                  +12V  22   A6                       +12V  22   A6
      12VAC  23   A5                 12VAC  23   A5                       -12V  23   A5
        /M1  24   A4                   /M1  24   A4                        /M1  24   A4
      /RFSH  25   /ROMCS             /RFSH  25   /ROMCS                  /RFSH  25   NC
         A8  26   /BUSACK               A8  26   /BUSACK                    A8  26   /BUSACK
        A10  27   A9                   A10  27   A9                        A10  27   A9
         NC  28   A11                   NC  28   A11                     RESET  28   A11


**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_EXP_H
#define MAME_BUS_SPECTRUM_EXP_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_expansion_slot_device

class device_spectrum_expansion_interface;

class spectrum_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_spectrum_expansion_interface>
{
public:
	// construction/destruction
	template <typename T>
	spectrum_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: spectrum_expansion_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	spectrum_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }
	auto fb_r_handler() { return m_fb_r_handler.bind(); }

	void pre_opcode_fetch(offs_t offset);
	void post_opcode_fetch(offs_t offset);
	void pre_data_fetch(offs_t offset);
	void post_data_fetch(offs_t offset);

	uint8_t mreq_r(offs_t offset);
	void mreq_w(offs_t offset, uint8_t data);
	uint8_t iorq_r(offs_t offset);
	void iorq_w(offs_t offset, uint8_t data);
	bool romcs();

	void irq_w(int state) { m_irq_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }
	uint8_t fb_r() { return m_fb_r_handler(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_spectrum_expansion_interface *m_card;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;
	devcb_read8      m_fb_r_handler;
};


// ======================> device_spectrum_expansion_interface

class device_spectrum_expansion_interface : public device_interface
{
public:
	// reading and writing
	virtual void pre_opcode_fetch(offs_t offset) { }
	virtual void post_opcode_fetch(offs_t offset) { }
	virtual void pre_data_fetch(offs_t offset) { }
	virtual void post_data_fetch(offs_t offset) { }
	virtual uint8_t mreq_r(offs_t offset) { return 0xff; }
	virtual void mreq_w(offs_t offset, uint8_t data) { }
	virtual uint8_t iorq_r(offs_t offset) { return offset & 1 ? m_slot->fb_r() : 0xff; }
	virtual void iorq_w(offs_t offset, uint8_t data) { }
	virtual bool romcs() { return 0; }

protected:
	// construction/destruction
	device_spectrum_expansion_interface(const machine_config &mconfig, device_t &device);

	spectrum_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_EXPANSION_SLOT, spectrum_expansion_slot_device)

void spectrum_expansion_devices(device_slot_interface &device);
void spec128_expansion_devices(device_slot_interface &device);
void specpls3_expansion_devices(device_slot_interface &device);


#endif // MAME_BUS_SPECTRUM_EXP_H
