// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*
    QX-10 Option bus

    1   GND           2   GND
    3   DTB0          4   DTB1
    5   DTB2          6   DTB3
    7   DTB4          8   DTB5
    9   DTB6          10  DTB7
    11  -12V          12  -12V
    13  ADR0          14  ADR1
    15  ADR2          16  ADR3
    17  ADR4          18  ADR5
    19  ADR6          20  ADR7
    21  ADR8          22  ADR9
    23  ADR10         24  ADR11
    25  ADR12         26  ADR13
    27  ADR14         28  ADR15
    29  GND           30  GND
    31  CLK           32  GND
    33  /BSAK         34  /MEMX
    35  /IRD          36  /IWR
    37  /MRD          38  /MWR
    39  /RSIN         40  INTH1
    41  INTH2         42  INTL
    43  +5V           44  /RSET
    45  +5V           46  +5V
    47  /DRQF         48  /DRQS
    49  /RDYF         50  /RDYS
    51  /WAIT         52  /IWS
    53  /DAKF         54  /DAKS
    55  /EOPF         56  /EOPS
    57  +12V          58  +12V
    59  GND           60  GND

    The INTH1 and INTH2 singals are the two hihg priority interrupts and are
    global to the entire option bus. The INTL signal is the low priority interrupt
    and is local to each of the 5 option slots.

      5      4      3      2      1
    |---|  |---|  |---|  |---|  |---|
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  |   |  |   |
    | *-|--|-*-|--|-*-|--|-*-|--|-*-|--------IRQH1 (Master IR2)
    |   |  |   |  |   |  |   |  |   |
    | *-|--|-*-|--|-*-|--|-*-|--|-*-|--------IRQH2 (Master IR3)
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  |   |  | *-|--------IRQL  (Slave  IR1)
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  | *-|--|---|--------IRQL  (Slave  IR3)
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  | *-|--|---|--|---|--------IRQL  (Slave  IR4)
    |   |  |   |  |   |  |   |  |   |
    |   |  | *-|--|---|--|---|--|---|--------IRQL  (Slave  IR6)
    |   |  |   |  |   |  |   |  |   |
    | *-|--|---|--|---|--|---|--|---|--------IRQL  (Slave  IR7)
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  |   |  |   |
    |---|  |---|  |---|  |---|  |---|

    The DREQ/DACK(F) signals are the high priorty DMA signals and are shared between all option slots.
    The DREQ/DACK(S) signals are the lower priority DMA signals and are local to each slot, with the
    exception of slot 5 which does not provide low priority DMA signals. The RDY/EOP(S) signals are
    also connected to all 5 options slots since the DREQ(S) signals are all connected to the same
    DMA controller.

      5      4      3      2      1
    |---|  |---|  |---|  |---|  |---|
    |   |  |   |  |   |  |   |  |   |
    | *-|--|-*-|--|-*-|--|-*-|--|-*-|--------DREQF (Master CH3)
    | *-|--|-*-|--|-*-|--|-*-|--|-*-|--------DACKF (Master Ch3)
    |   |  |   |  |   |  |   |  |   |
    | *-|--|-*-|--|-*-|--|-*-|--|-*-|--------RDYF  (Master RDY)
    | *-|--|-*-|--|-*-|--|-*-|--|-*-|--------EOPF  (Master EOP)
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  |   |  | *-|--------DREQS (Slave  CH0)
    |   |  |   |  |   |  |   |  | *-|--------DACKS (Slave  CH0)
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  |   |  | *-|--|---|--------DREQS (Slave  CH1)
    |   |  |   |  |   |  | *-|--|---|--------DACKS (Slave  CH1)
    |   |  |   |  |   |  |   |  |   |
    |   |  |   |  | *-|--|---|--|---|--------DREQS (Slave  CH2)
    |   |  |   |  | *-|--|---|--|---|--------DACKS (Slave  CH2)
    |   |  |   |  |   |  |   |  |   |
    |   |  | *-|--|---|--|---|--|---|--------DREQS (Slave  CH3)
    |   |  | *-|--|---|--|---|--|---|--------DACKS (Slave  CH3)
    |   |  |   |  |   |  |   |  |   |
    | *-|--|-*-|--|-*-|--|-*-|--|-*-|--------RDYS  (Slave RDY)
    | *-|--|-*-|--|-*-|--|-*-|--|-*-|--------EOPS  (Slave EOP)
    |   |  |   |  |   |  |   |  |   |
    |---|  |---|  |---|  |---|  |---|

*/
#ifndef MAME_BUS_EPSON_QX_BUS_H
#define MAME_BUS_EPSON_QX_BUS_H

#pragma once

#include <vector>

namespace bus::epson_qx {

//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

class option_bus_device;
class device_option_expansion_interface;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* Epson Slot Device */

class option_slot_device : public device_t, public device_single_card_slot_interface<device_option_expansion_interface>
{
public:
	friend class option_bus_device;
	// construction/destruction
	template <typename T, typename U>
	option_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&bus_tag, int slot, U &&opts, const char *dflt)
		: option_slot_device(mconfig, tag, owner, bus_tag->clock())
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_bus.set_tag(std::forward<T>(bus_tag));
		m_slot = slot;
	}
	option_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void inth1_w(int state);
	void inth2_w(int state);
	void intl_w(int state);

	void drqf_w(int state);
	void drqs_w(int state);

	void rdyf_w(int state);
	void rdys_w(int state);

	void eopf(int state);
	void eops(int state);

	auto dmas_w_callback() { m_dmas_w_cb.bind(); }
	auto dmas_r_callback() { m_dmas_r_cb.bind(); }
	auto dmaf_w_callback() { m_dmaf_w_cb.bind(); }
	auto dmaf_r_callback() { m_dmaf_r_cb.bind(); }

	auto in_eopf_callback() { return m_eopf_cb.bind(); }
	auto in_eops_callback() { return m_eops_cb.bind(); }
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<option_bus_device> m_bus;
	devcb_write8 m_dmas_w_cb;
	devcb_read8  m_dmas_r_cb;
	devcb_write8 m_dmaf_w_cb;
	devcb_read8  m_dmaf_r_cb;

	devcb_write_line m_eopf_cb;
	devcb_write_line m_eops_cb;

	int m_slot;
};


/* Epson Bus Device */

class option_bus_device :  public device_t
{
public:
	// construction/destruction
	option_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }
	void set_memview(memory_view::memory_view_entry &view) { m_view = &view; }

	auto out_inth1_callback() { return m_inth1_cb.bind(); }
	auto out_inth2_callback() { return m_inth2_cb.bind(); }
	template <unsigned Slot> auto out_intl_callback() { return m_intl_cb[Slot].bind(); }

	auto out_drqf_callback() { return m_drqf_cb.bind(); }
	template <unsigned Slot> auto out_drqs_callback() { return m_drqs_cb[Slot].bind(); }

	auto out_rdyf_callback() { return m_rdyf_cb.bind(); }
	auto out_rdys_callback() { return m_rdys_cb.bind(); }

	memory_view::memory_view_entry& memview() const { return *m_view; }
	address_space &iospace() const { return *m_iospace; }

	void dackf_w(uint8_t data);
	uint8_t dackf_r();
	template <unsigned Slot> void dacks_w(uint8_t data) {  (*this)[Slot]->m_dmas_w_cb(data);  }
	template <unsigned Slot> uint8_t dacks_r() { return (*this)[Slot]->m_dmas_r_cb(); }

	template <void (option_slot_device::*slot_callback)(int)> void slots_w(int state) {
		for (option_slot_device *slot : m_slot_list) {
			(slot->*slot_callback)(state);
		}
	}

	void set_inth1_line(uint8_t state, uint8_t slot);
	void set_inth2_line(uint8_t state, uint8_t slot);
	void set_intl_line(uint8_t state, uint8_t slot) { m_intl_cb[slot](state); }
	void set_drqf_line(uint8_t state, uint8_t slot);
	void set_drqs_line(uint8_t state, uint8_t slot) { m_drqs_cb[slot](state); }
	void set_rdyf_line(uint8_t state, uint8_t slot);
	void set_rdys_line(uint8_t state, uint8_t slot);

	void add_slot(option_slot_device &slot);
	option_slot_device* operator[](int index) const {assert(index < m_slot_list.size()); return m_slot_list[index]; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_address_space m_iospace;
	memory_view::memory_view_entry *m_view;

	devcb_write_line m_inth1_cb;
	devcb_write_line m_inth2_cb;
	devcb_write_line::array<5> m_intl_cb;

	devcb_write_line m_drqf_cb;
	devcb_write_line::array<4> m_drqs_cb;

	devcb_write_line m_rdyf_cb;
	devcb_write_line m_rdys_cb;

	std::vector<option_slot_device *> m_slot_list;

	uint8_t m_inth1;
	uint8_t m_inth2;

	uint8_t m_drqf;

	uint8_t m_rdyf;
	uint8_t m_rdys;
};


/* Epson Option Card interface */
class device_option_expansion_interface : public device_interface
{
public:
	void set_option_bus(option_bus_device &bus, int slot) { assert(!device().started()); m_bus = &bus; m_slot = slot; }

protected:
	device_option_expansion_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	option_slot_device* get_slot() { return (*m_bus)[m_slot]; }

	option_bus_device  *m_bus;
	int m_slot;
};

void option_bus_devices(device_slot_interface &device);

} // namespace bus::epson_qx


// device type definition
DECLARE_DEVICE_TYPE_NS(EPSON_QX_OPTION_BUS_SLOT, bus::epson_qx, option_slot_device)
DECLARE_DEVICE_TYPE_NS(EPSON_QX_OPTION_BUS, bus::epson_qx, option_bus_device)

#endif
