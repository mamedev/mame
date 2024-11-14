// license:BSD-3-Clause
// copyright-holders:AJR

/* Sega Sawatte / S-Pico

a sound-only Pico type system (one of the boards even says S-PICO)

CPU is unknown (MCU with internal ROM?) cartridge dumps contain 6502 code
and have been tested as working using a flash cart.


images supplied by Team Europe

http://mamedev.emulab.it/haze/reference/sawatte/cartridge_pcb_front.jpg
http://mamedev.emulab.it/haze/reference/sawatte/cartridge_pcb_back.jpg

http://mamedev.emulab.it/haze/reference/sawatte/PCB_Front.jpg
http://mamedev.emulab.it/haze/reference/sawatte/PCB_Back.jpg

http://mamedev.emulab.it/haze/reference/sawatte/Console_Front.JPG
http://mamedev.emulab.it/haze/reference/sawatte/Console_Back.JPG

http://mamedev.emulab.it/haze/reference/sawatte/cartridge_example.jpg

*/

#include "emu.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m6502.h"
#include "softlist_dev.h"


namespace {

class sawatte_state : public driver_device
{
public:
	sawatte_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cartslot(*this, "cartslot")
		, m_soundram(*this, "soundram")
		, m_irq3_timer(nullptr)
		, m_irq4_timer(nullptr)
		, m_data_bank(0)
		, m_prog_bank(0)
		, m_irq_status(0)
		, m_irq_mask(0)
	{ }

	void sawatte(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(irq3_timer);
	TIMER_CALLBACK_MEMBER(irq4_timer);

	void bank08_w(u8 data);
	void bank09_w(u8 data);
	void bank0a_w(u8 data);
	u8 irq_status_r();
	void irq_mask_w(u8 data);
	void irq3_timer_w(u8 data);
	void irq4_timer_w(u8 data);
	u8 fixed_r(offs_t offset);
	u8 data_bank_r(offs_t offset);
	u8 prog_bank_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cartslot;
	required_shared_ptr<u8> m_soundram;

	emu_timer *m_irq3_timer;
	emu_timer *m_irq4_timer;

	u16 m_data_bank;
	u8 m_prog_bank;
	u8 m_irq_status;
	u8 m_irq_mask;
};

void sawatte_state::machine_start()
{
	m_irq3_timer = timer_alloc(FUNC(sawatte_state::irq3_timer), this);
	m_irq4_timer = timer_alloc(FUNC(sawatte_state::irq4_timer), this);

	save_item(NAME(m_data_bank));
	save_item(NAME(m_prog_bank));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_irq_mask));
}

void sawatte_state::machine_reset()
{
	m_data_bank = 0;
	m_prog_bank = 0;

	m_irq_status = 0;
	m_irq_mask = 0;
	m_maincpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);

	m_irq3_timer->adjust(attotime::never);
	m_irq4_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(sawatte_state::irq3_timer)
{
	if (BIT(m_irq_mask, 3))
	{
		m_irq_status |= 0x08;
		m_maincpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);
	}
}

TIMER_CALLBACK_MEMBER(sawatte_state::irq4_timer)
{
	if (BIT(m_irq_mask, 4))
	{
		m_irq_status |= 0x10;
		m_maincpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);
	}
}


void sawatte_state::bank08_w(u8 data)
{
	m_data_bank = (m_data_bank & 0xff00) | data;
}

void sawatte_state::bank09_w(u8 data)
{
	m_data_bank = u16(data) << 8 | (m_data_bank & 0x00ff);
}

void sawatte_state::bank0a_w(u8 data)
{
	m_prog_bank = data;
}

u8 sawatte_state::irq_status_r()
{
	return m_irq_status;
}

void sawatte_state::irq_mask_w(u8 data)
{
	m_irq_mask = data;
	if (m_irq_status != 0)
	{
		m_irq_status &= data;
		if (m_irq_status == 0)
			m_maincpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
	}
}

void sawatte_state::irq3_timer_w(u8 data)
{
	if (data == 0)
		m_irq3_timer->adjust(attotime::never);
	else
	{
		attotime period = attotime::from_hz(10'000); // probably incorrect
		m_irq3_timer->adjust(period, 0, period);
	}
}

void sawatte_state::irq4_timer_w(u8 data)
{
	if (!BIT(data, 0))
		m_irq4_timer->adjust(attotime::never);
	else
	{
		attotime period = attotime::from_hz(8'000); // probably incorrect
		m_irq4_timer->adjust(period, 0, period);
	}
}

u8 sawatte_state::fixed_r(offs_t offset)
{
	return m_cartslot->read_rom(offset + 0x200);
}

u8 sawatte_state::data_bank_r(offs_t offset)
{
	return m_cartslot->read_rom(offset | offs_t(m_data_bank) << 11);
}

u8 sawatte_state::prog_bank_r(offs_t offset)
{
	return m_cartslot->read_rom(offset | offs_t(m_prog_bank) << 12);
}


void sawatte_state::mem_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x0000).nopr();
	map(0x0008, 0x0008).w(FUNC(sawatte_state::bank08_w));
	map(0x0009, 0x0009).w(FUNC(sawatte_state::bank09_w));
	map(0x000a, 0x000a).w(FUNC(sawatte_state::bank0a_w));
	map(0x0011, 0x0011).portr("IN1");
	map(0x0012, 0x0012).portr("IN2");
	map(0x0016, 0x0016).nopr();
	map(0x0017, 0x0017).r(FUNC(sawatte_state::irq_status_r));
	map(0x0020, 0x0020).nopw(); // matrix scanning?
	map(0x0022, 0x0022).nopw(); // matrix scanning?
	map(0x0026, 0x0026).w(FUNC(sawatte_state::irq3_timer_w));
	map(0x0027, 0x0027).w(FUNC(sawatte_state::irq_mask_w));
	map(0x002f, 0x002f).w(FUNC(sawatte_state::irq4_timer_w));
	map(0x0060, 0x007f).ram();
	map(0x0080, 0x009f).ram().share("soundram");
	map(0x00a0, 0x01ff).ram(); // might not all exist
	map(0x0200, 0x07ff).r(FUNC(sawatte_state::fixed_r));
	map(0x0800, 0x0fff).r(FUNC(sawatte_state::data_bank_r));
	map(0x1000, 0x1fff).r(FUNC(sawatte_state::prog_bank_r));
}


static INPUT_PORTS_START( sawatte )
	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON5)

	PORT_START("IN2")
	PORT_BIT(0x1f, IP_ACTIVE_HIGH, IPT_UNKNOWN) // matrix inputs?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON6)
INPUT_PORTS_END


void sawatte_state::sawatte(machine_config &config)
{
	M6502(config, m_maincpu, 4'000'000); // could be some stock SoC, type and clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &sawatte_state::mem_map);

	GENERIC_CARTSLOT(config, m_cartslot, generic_plain_slot, "sawatte_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("sawatte");
}

ROM_START( sawatte )
ROM_END

} // anonymous namespace


CONS( 1996?, sawatte, 0, 0, sawatte,  sawatte, sawatte_state, empty_init, "Sega", "Sawatte", MACHINE_IS_SKELETON )
