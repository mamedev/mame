// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  kim1bus.h - KIM-1 expansion bus

  Basically 6502 bus brought out to a connector.

  44-pin edge connector, 1-22 on top, A-Z on bottom

           SYNC   1  A  AB0
           /RDY   2  B  AB1
  clock phase 1   3  C  AB2
           /IRQ   4  D  AB3
             RO   5  E  AB4
           /NMI   6  F  AB5
           /RES   7  H  AB6
            DB7   8  J  AB7
            DB6   9  K  AB8
            DB5  10  L  AB9
            DB4  11  M  AB10
            DB3  12  N  AB11
            DB2  13  P  AB12
            DB1  14  R  AB13
            DB0  15  S  AB14
             K6  16  T  AB15
        SST out  17  U  clock phase 2
            N/C  18  V  R/W
            N/C  19  W  inverted R/W
            N/C  20  X  PLL test
            VCC  21  Y  inverted clock phase 2
            GND  22  Z  RAM R/W

  * RO is connected to 6502 SO pin
  * K6 is asserted (low) when accessing 0x1800-0x1bff (mirror 0xe000)

  Rockwell AIM65 expansion only differs in five pins:

           -12V  16
           +12V  17
           /CS8  18
           /CS9  19
           /CSA  20

  Comelta DRAC-1 expansion modules use a 96-pin Eurocard connector with row
  b unused.  Has an external manual reset input and more power rails, but
  omits some control signals:

                   +25V  a  1 c  +25V
  external manual reset  a  2 c  reserved
               reserved  a  3 c  +5V battery
                   +12V  a  4 c  +15V
                    -5V  a  5 c  -15V/-12V
                    +5V  a  6 c  +5V
                    GND  a  7 c  GND
                    GND  a  8 c  GND
                    N/C  a  9 c  N/C
                    N/C  a 10 c  N/C
                    N/C  a 11 c  N/C
                    N/C  a 12 c  N/C
               reserved  a 13 c  SYNC
               reserved  a 14 c  reserved
                    R/W  a 15 c  /RDY
          clock phase 2  a 16 c  clock phase 1
                   /IRQ  a 17 c  reserved
                   /NMI  a 18 c  /RES
               reserved  a 19 c  reserved
                    DB7  a 20 c  DB6
                    DB5  a 21 c  DB4
                    DB3  a 22 c  DB2
                    DB1  a 23 c  DB0
               reserved  a 24 c  reserved
                   AB15  a 25 c  AB14
                   AB13  a 26 c  AB12
                   AB11  a 27 c  AB10
                    AB9  a 28 c  AB8
                    AB7  a 29 c  AB6
                    AB5  a 30 c  AB4
                    AB3  a 31 c  AB2
                    AB1  a 32 c  AB0

  * No RO, RAM R/W, inverted clock phase 2 and R/W, K6 or SST out

***************************************************************************/

#ifndef MAME_BUS_KIM1_KIM1BUS_H
#define MAME_BUS_KIM1_KIM1BUS_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kim1bus_device;
class device_kim1bus_card_interface;

class kim1bus_slot_device : public device_t, public device_single_card_slot_interface<device_kim1bus_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	kim1bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&kim1bus_tag, U &&opts, const char *dflt)
		: kim1bus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_kim1bus.set_tag(std::forward<T>(kim1bus_tag));
	}
	kim1bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	kim1bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<kim1bus_device> m_kim1bus;
};

// device type definition
DECLARE_DEVICE_TYPE(KIM1BUS_SLOT, kim1bus_slot_device)


// ======================> kim1bus_device
class kim1bus_device : public device_t
{
public:
	// construction/destruction
	kim1bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }
	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }

	void add_kim1bus_card(device_kim1bus_card_interface *card);
	device_kim1bus_card_interface *get_kim1bus_card();

	void set_irq_line(int state);
	void set_nmi_line(int state);

	void install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler);
	void install_bank(offs_t start, offs_t end, uint8_t *data);

	void irq_w(int state);
	void nmi_w(int state);

protected:
	kim1bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	required_address_space m_space;

	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;

	device_kim1bus_card_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(KIM1BUS, kim1bus_device)

// ======================> device_kim1bus_card_interface

// class representing interface-specific live kim1bus card
class device_kim1bus_card_interface : public device_interface
{
	friend class kim1bus_device;
public:
	// construction/destruction
	virtual ~device_kim1bus_card_interface();

	// inline configuration
	void set_kim1bus(kim1bus_device *kim1bus, const char *slottag) { m_kim1bus = kim1bus; m_kim1bus_slottag = slottag; }
	template <typename T> void set_onboard(T &&kim1bus) { m_kim1bus_finder.set_tag(std::forward<T>(kim1bus)); m_kim1bus_slottag = device().tag(); }

protected:
	void raise_slot_irq() { m_kim1bus->set_irq_line(ASSERT_LINE); }
	void lower_slot_irq() { m_kim1bus->set_irq_line(CLEAR_LINE); }
	void raise_slot_nmi() { m_kim1bus->set_nmi_line(ASSERT_LINE); }
	void lower_slot_nmi() { m_kim1bus->set_nmi_line(CLEAR_LINE); }

	void install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler);
	void install_bank(offs_t start, offs_t end, uint8_t *data);

	device_kim1bus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;

private:
	optional_device<kim1bus_device> m_kim1bus_finder;
	kim1bus_device *m_kim1bus;
	const char *m_kim1bus_slottag;
};

#endif  // MAME_BUS_KIM1_KIM1BUS_H
