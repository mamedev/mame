// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, Mr. Lars
/*******************************************************************************

Saitek OSA: Kasparov Brute Force Module (1992)

When Brute Force Module was announced, it was supposed to include a program by
Ulf Rathsman (known from Conchess and Mephisto MM II). It got canceled for some
reason, and the final version is by Frans Morsch.

Hardware notes:
- Hitachi H8/325 MCU, 20MHz resonator
- 256KB DRAM (2*NEC D424256C-80)
- PCA EP9604-100 delay line (for DRAM CAS)

*******************************************************************************/

#include "emu.h"
#include "bruteforce.h"

#include "cpu/h8/h8325.h"

namespace {

//-------------------------------------------------
//  initialization
//-------------------------------------------------

class saitekosa_bforce_device : public device_t, public device_saitekosa_expansion_interface
{
public:
	// construction/destruction
	saitekosa_bforce_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// from host
	virtual u8 data_r() override;
	virtual void nmi_w(int state) override;
	virtual void ack_w(int state) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<h8325_device> m_maincpu;
	memory_share_creator<u8> m_ram;
	required_memory_bank m_rambank;

	u8 m_data_out = 0;

	void main_map(address_map &map) ATTR_COLD;

	u8 p4_r();
	void p4_w(u8 data);
	u8 p5_r();
	void p5_w(u8 data);
	void p6_w(u8 data);
	u8 p7_r();
};

saitekosa_bforce_device::saitekosa_bforce_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, OSA_BFORCE, tag, owner, clock),
	device_saitekosa_expansion_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_ram(*this, "ram", 0x40000, ENDIANNESS_BIG),
	m_rambank(*this, "rambank")
{ }

void saitekosa_bforce_device::device_start()
{
	m_rambank->configure_entries(0, 16, m_ram, 0x4000);

	save_item(NAME(m_data_out));
}


//-------------------------------------------------
//  host i/o
//-------------------------------------------------

u8 saitekosa_bforce_device::data_r()
{
	return m_data_out;
}

void saitekosa_bforce_device::nmi_w(int state)
{
	m_maincpu->set_input_line(0, state ? CLEAR_LINE : ASSERT_LINE);
}

void saitekosa_bforce_device::ack_w(int state)
{
	if (state != m_expansion->ack_state())
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
}


//-------------------------------------------------
//  internal i/o
//-------------------------------------------------

u8 saitekosa_bforce_device::p4_r()
{
	// P40-P43: DC0-DC3
	return m_expansion->data_state() & 0xf;
}

void saitekosa_bforce_device::p4_w(u8 data)
{
	// P40-P43: DC0-DC3
	m_data_out = (m_data_out & 0xf0) | (data & 0x0f);
}

u8 saitekosa_bforce_device::p5_r()
{
	// P50-P53: DC4-DC7
	return m_expansion->data_state() >> 4;
}

void saitekosa_bforce_device::p5_w(u8 data)
{
	// P50-P53: DC4-DC7
	m_data_out = (m_data_out & 0x0f) | (data << 4 & 0xf0);

	// P54: RTS-P
	m_expansion->rts_w(BIT(data, 4));

	// P55: STB-P
	m_expansion->stb_w(BIT(data, 5));
}

void saitekosa_bforce_device::p6_w(u8 data)
{
	// P60-P63: RAM bank
	m_rambank->set_entry(data & 0xf);
}

u8 saitekosa_bforce_device::p7_r()
{
	// P70: ACK-P
	return m_expansion->ack_state();
}

void saitekosa_bforce_device::main_map(address_map &map)
{
	map(0x8000, 0xbfff).bankrw(m_rambank);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void saitekosa_bforce_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	H8325(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_mode(2);
	m_maincpu->set_addrmap(AS_PROGRAM, &saitekosa_bforce_device::main_map);
	m_maincpu->read_port4().set(FUNC(saitekosa_bforce_device::p4_r));
	m_maincpu->write_port4().set(FUNC(saitekosa_bforce_device::p4_w));
	m_maincpu->read_port5().set(FUNC(saitekosa_bforce_device::p5_r));
	m_maincpu->write_port5().set(FUNC(saitekosa_bforce_device::p5_w));
	m_maincpu->write_port6().set(FUNC(saitekosa_bforce_device::p6_w));
	m_maincpu->read_port7().set(FUNC(saitekosa_bforce_device::p7_r));
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( bforce )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("h8_325_hd6473258p10_sm18_b16_u_2.u2", 0x0000, 0x8000, CRC(f22e46d2) SHA1(7baa809ffc09117a27bdeede655f92bc3a6d1626) )
ROM_END

const tiny_rom_entry *saitekosa_bforce_device::device_rom_region() const
{
	return ROM_NAME(bforce);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(OSA_BFORCE, device_saitekosa_expansion_interface, saitekosa_bforce_device, "osa_bforce", "Saitek OSA: Kasparov Brute Force Module")
