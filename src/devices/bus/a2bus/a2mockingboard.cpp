// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2mockingboard.cpp

    Implementation of the Sweet Micro Systems Mockingboard card
    and friends.

*********************************************************************/

#include "emu.h"
#include "a2mockingboard.h"

#include "machine/6522via.h"
#include "sound/ay8910.h"
#include "sound/tms5220.h"
#include "sound/votrax.h"

#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ayboard_device:
		public device_t,
		public device_a2bus_card_interface
{
protected:
	// construction/destruction
	a2bus_ayboard_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock) ATTR_COLD;

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override { return 0xff; }
	virtual void write_c0nx(u8 offset, u8 data) override { }
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

	static void via_psg_ctrl(ay8913_device &psg, u8 &latch, u8 data);

	void via1_irq_w(int state);
	void via2_irq_w(int state);
	u8 via1_in_a() { return m_porta1; }
	u8 via2_in_a() { return m_porta2; }
	void via1_out_a(u8 data);
	void via1_out_b(u8 data);
	void via2_out_a(u8 data);
	void via2_out_b(u8 data);

	// machine configuration fragments
	void single_via_devices(machine_config &config) ATTR_COLD;

	required_device<via6522_device> m_via1;
	optional_device<via6522_device> m_via2;
	required_device<ay8913_device> m_ay1;
	required_device<ay8913_device> m_ay2;

	u8 m_porta1;
	u8 m_porta2;
};

class a2bus_mockingboard_device : public a2bus_ayboard_device
{
public:
	a2bus_mockingboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void via1_out_b(u8 data);

	required_device<votrax_sc01_device> m_sc01;

private:
	void write_via1_cb2(int state);

	u8 m_portb1;
	int m_last_cb2_state;
};

class a2bus_phasor_device : public a2bus_ayboard_device
{
public:
	a2bus_phasor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

	void via1_out_b(u8 data);
	void via2_out_b(u8 data);

	required_device<ay8913_device> m_ay3;
	required_device<ay8913_device> m_ay4;

private:
	void set_clocks();

	bool m_native;
};

class a2bus_echoplus_device : public a2bus_ayboard_device
{
public:
	a2bus_echoplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

	void via1_out_b(u8 data);

	required_device<tms5220_device> m_tms;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_ayboard_device::single_via_devices(machine_config &config)
{
	MOS6522(config, m_via1, 1022727);
	m_via1->readpa_handler().set(FUNC(a2bus_ayboard_device::via1_in_a));
	m_via1->writepa_handler().set(FUNC(a2bus_ayboard_device::via1_out_a));
	m_via1->writepb_handler().set(FUNC(a2bus_ayboard_device::via1_out_b));
	m_via1->irq_handler().set(FUNC(a2bus_ayboard_device::via1_irq_w));

	SPEAKER(config, "speaker", 2).front();

	AY8913(config, m_ay1, 1022727);
	m_ay1->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);

	AY8913(config, m_ay2, 1022727);
	m_ay2->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
}

void a2bus_ayboard_device::device_add_mconfig(machine_config &config)
{
	single_via_devices(config);

	MOS6522(config, m_via2, 1022727);
	m_via2->readpa_handler().set(FUNC(a2bus_ayboard_device::via2_in_a));
	m_via2->writepa_handler().set(FUNC(a2bus_ayboard_device::via2_out_a));
	m_via2->writepb_handler().set(FUNC(a2bus_ayboard_device::via2_out_b));
	m_via2->irq_handler().set(FUNC(a2bus_ayboard_device::via2_irq_w));
}

void a2bus_mockingboard_device::device_add_mconfig(machine_config &config)
{
	a2bus_ayboard_device::device_add_mconfig(config);

	m_via1->writepb_handler().set(FUNC(a2bus_mockingboard_device::via1_out_b));
	m_via1->cb2_handler().set(FUNC(a2bus_mockingboard_device::write_via1_cb2));

	VOTRAX_SC01A(config, m_sc01, 1022727);
	m_sc01->ar_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_sc01->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_sc01->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

void a2bus_phasor_device::device_add_mconfig(machine_config &config)
{
	a2bus_ayboard_device::device_add_mconfig(config);

	m_via1->writepb_handler().set(FUNC(a2bus_phasor_device::via1_out_b));
	m_via2->writepb_handler().set(FUNC(a2bus_phasor_device::via2_out_b));

	SPEAKER(config, "speaker2", 2).front();

	m_ay2->reset_routes().add_route(ALL_OUTPUTS, "speaker2", 0.5, 0);

	AY8913(config, m_ay3, 1022727);
	m_ay3->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);

	AY8913(config, m_ay4, 1022727);
	m_ay4->add_route(ALL_OUTPUTS, "speaker2", 0.5, 1);
}

void a2bus_echoplus_device::device_add_mconfig(machine_config &config)
{
	single_via_devices(config);

	m_via1->writepb_handler().set(FUNC(a2bus_echoplus_device::via1_out_b));

	SPEAKER(config, "echosp").front_center();
	TMS5220C(config, m_tms, 640000);
	// echo+ has a TSP5220C soldered down on it
	m_tms->add_route(ALL_OUTPUTS, "echosp", 1.0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ayboard_device::a2bus_ayboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_via1(*this, "via1"),
	m_via2(*this, "via2"),
	m_ay1(*this, "ay1"),
	m_ay2(*this, "ay2"),
	m_porta1(0),
	m_porta2(0)
{
}

a2bus_mockingboard_device::a2bus_mockingboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ayboard_device(mconfig, A2BUS_MOCKINGBOARD, tag, owner, clock),
	m_sc01(*this, "sc01")
{
}

a2bus_phasor_device::a2bus_phasor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ayboard_device(mconfig, A2BUS_PHASOR, tag, owner, clock),
	m_ay3(*this, "ay3"),
	m_ay4(*this, "ay4"),
	m_native(false)
{
}

a2bus_echoplus_device::a2bus_echoplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ayboard_device(mconfig, A2BUS_ECHOPLUS, tag, owner, clock),
	m_tms(*this, "tms5220")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ayboard_device::device_start()
{
	save_item(NAME(m_porta1));
	save_item(NAME(m_porta2));
}

void a2bus_ayboard_device::device_reset()
{
	m_porta1 = m_porta2 = 0;
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

u8 a2bus_ayboard_device::read_cnxx(u8 offset)
{
	if (offset <= 0x7f)
	{
		return m_via1->read(offset & 0xf);
	}
	else
	{
		return m_via2->read(offset & 0xf);
	}

	return 0;
}

u8 a2bus_echoplus_device::read_cnxx(u8 offset)
{
	return m_via1->read(offset & 0xf);
}

u8 a2bus_phasor_device::read_cnxx(u8 offset)
{
	u8 ret = 0;
	int via_sel;

	if (m_native)
	{
		via_sel = ((offset & 0x80) >> 6) | ((offset & 0x10) >> 4);
	}
	else
	{
		via_sel = (offset & 0x80) ? 2 : 1;
	}

	if ((offset < 0x20) || (offset >= 0x80 && offset < 0xa0))
	{
		if (BIT(via_sel, 0))
		{
			ret |= m_via1->read(offset & 0xf);
		}
		if (BIT(via_sel, 1))
		{
			ret |= m_via2->read(offset & 0xf);
		}
	}

	return ret;
}

/*-------------------------------------------------
    write_cnxx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_ayboard_device::write_cnxx(u8 offset, u8 data)
{
	// When the SSI-263 is emulated, Cn40 will write to both the VIA and
	// the first SSI-263, and Cn20 will write to both the VIA and the
	// second SSI-263.  Reads only select the VIA.
	if (offset <= 0x7f)
	{
		m_via1->write(offset & 0xf, data);
	}
	else
	{
		m_via2->write(offset & 0xf, data);
	}
}

void a2bus_echoplus_device::write_cnxx(u8 offset, u8 data)
{
	m_via1->write(offset & 0xf, data);
}

void a2bus_phasor_device::write_cnxx(u8 offset, u8 data)
{
	if ((offset < 0x20) || (offset >= 0x80 && offset < 0xa0))
	{
		int via_sel;

		if (m_native)
		{
			via_sel = ((offset & 0x80) >> 6) | ((offset & 0x10) >> 4);
		}
		else
		{
			via_sel = (offset & 0x80) ? 2 : 1;
		}

		if (BIT(via_sel, 0))
		{
			m_via1->write(offset & 0xf, data);
		}
		if (BIT(via_sel, 1))
		{
			m_via2->write(offset & 0xf, data);
		}
	}
}

inline void a2bus_ayboard_device::via_psg_ctrl(ay8913_device &psg, u8 &latch, u8 data)
{
	switch (data & 3)
	{
		case 0: // BDIR=0, BC1=0 (inactive)
			break;

		case 1: // BDIR=0, BC1=1 (read PSG)
			latch = psg.data_r();
			break;

		case 2: // BDIR=1, BC1=0 (write PSG)
			psg.data_w(latch);
			break;

		case 3: // BDIR=1, BC1=1 (latch)
			psg.address_w(latch);
			break;
	}
}

void a2bus_ayboard_device::via1_irq_w(int state)
{
	if (state)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

void a2bus_ayboard_device::via2_irq_w(int state)
{
	if (state)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

void a2bus_ayboard_device::via1_out_a(u8 data)
{
	m_porta1 = data;
}

void a2bus_ayboard_device::via1_out_b(u8 data)
{
	if (!BIT(data, 2))
	{
		m_ay1->reset_w();
	}
	else
	{
		via_psg_ctrl(*m_ay1, m_porta1, data);
	}
}

void a2bus_echoplus_device::via1_out_b(u8 data)
{
	if (!BIT(data, 2))
	{
		m_ay1->reset_w();
		m_ay2->reset_w();
	}
	else
	{
		// TODO: reading with both PSGs selected should fight for the latch
		if (BIT(data, 3)) // BC2_1=1 (PSG1 active)
		{
			via_psg_ctrl(*m_ay1, m_porta1, data);
		}
		if (BIT(data, 4)) // BC2_2_=1 (PSG2 active)
		{
			via_psg_ctrl(*m_ay2, m_porta1, data);
		}
	}
}

void a2bus_phasor_device::via1_out_b(u8 data)
{
	if (!(data & 4))
	{
		m_ay1->reset_w();
		if (m_native)
		{
			m_ay2->reset_w();
		}
	}
	else
	{
		int chip_sel;

		if (m_native)
			chip_sel = (~(data >> 3) & 3);
		else
			chip_sel = 1;

//      logerror("Phasor: %02x to AY1/2 CS %02x (BDIR/BC1 %02x, data %02x)\n", m_porta1, chipSel, data & 3, data);
		// TODO: reading with both PSGs selected should fight for the latch
		if (BIT(chip_sel, 0))
			via_psg_ctrl(*m_ay1, m_porta1, data);
		if (BIT(chip_sel, 1))
			via_psg_ctrl(*m_ay2, m_porta1, data);
	}
}

void a2bus_ayboard_device::via2_out_a(u8 data)
{
	m_porta2 = data;
}

void a2bus_ayboard_device::via2_out_b(u8 data)
{
	if (!BIT(data, 2))
	{
		m_ay2->reset_w();
	}
	else
	{
		via_psg_ctrl(*m_ay2, m_porta2, data);
	}
}

void a2bus_phasor_device::via2_out_b(u8 data)
{
	if (!BIT(data, 2))
	{
		if (m_native)
		{
			m_ay3->reset_w();
			m_ay4->reset_w();
		}
		else
		{
			m_ay2->reset_w();
		}
	}
	else
	{
		int chip_sel;

		if (m_native)
			chip_sel = (~(data >> 3) & 3);
		else
			chip_sel = 1;

//      logerror("Phasor: %02x to AY3/4 CS %02x (BDIR/BC1 %02x, data %02x)\n", m_porta2, chipSel, data & 3, data);
		// TODO: reading with both PSGs selected should fight for the latch
		if (BIT(chip_sel, 0))
			via_psg_ctrl(*m_ay3, m_porta2, data);
		if (BIT(chip_sel, 1))
			via_psg_ctrl(*m_ay4, m_porta2, data);
	}
}

void a2bus_phasor_device::set_clocks()
{
	if (m_native)
	{
		m_ay1->set_clock(1022727*2);
		m_ay2->set_clock(1022727*2);
		m_ay3->set_clock(1022727*2);
		m_ay4->set_clock(1022727*2);
	}
	else
	{
		m_ay1->set_clock(1022727);
		m_ay2->set_clock(1022727);
		m_ay3->set_clock(1022727);
		m_ay4->set_clock(1022727);
	}
}

u8 a2bus_phasor_device::read_c0nx(u8 offset)
{
	m_native = BIT(offset, 0);
	set_clocks();
	return 0xff;
}

void a2bus_phasor_device::write_c0nx(u8 offset, u8 data)
{
	m_native = BIT(offset, 0);
	set_clocks();
}

u8 a2bus_echoplus_device::read_c0nx(u8 offset)
{
	switch (offset)
	{
		case 0:
			return 0x1f | m_tms->status_r();
	}

	return 0;
}

void a2bus_echoplus_device::write_c0nx(u8 offset, u8 data)
{
	switch (offset)
	{
		case 0:
			m_tms->data_w(data);
			break;
	}
}

void a2bus_mockingboard_device::device_reset()
{
	m_porta1 = m_porta2 = m_portb1 = 0;
	m_last_cb2_state = ASSERT_LINE;
}

void a2bus_mockingboard_device::write_via1_cb2(int state)
{
	if ((state == CLEAR_LINE) && (m_last_cb2_state == ASSERT_LINE))
	{
		m_sc01->write(m_portb1);
		m_sc01->inflection_w(m_portb1 >> 6);
	}
	m_last_cb2_state = state;
}

void a2bus_mockingboard_device::via1_out_b(u8 data)
{
	m_portb1 = data;

	if (!BIT(data, 2))
	{
		m_ay1->reset_w();
	}
	else
	{
		via_psg_ctrl(*m_ay1, m_porta1, data);
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_MOCKINGBOARD, device_a2bus_card_interface, a2bus_mockingboard_device, "a2mockbd", "Sweet Micro Systems Mockingboard Sound/Speech I")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_PHASOR,       device_a2bus_card_interface, a2bus_phasor_device,       "a2phasor", "Applied Engineering Phasor")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ECHOPLUS,     device_a2bus_card_interface, a2bus_echoplus_device,     "a2echop",  "Street Electronics Echo Plus")
