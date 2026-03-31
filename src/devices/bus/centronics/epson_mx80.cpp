// license:BSD-3-Clause
// copyright-holders:Golden Child
/******************************************************************************

    Epson MX-80 Dot Matrix printer (skeleton)

******************************************************************************/

#include "emu.h"
#include "ctronics.h"
#include "epson_mx80.h"
#include "machine/i8155.h"
#include "cpu/mcs48/mcs48.h"

namespace {

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

class epson_mx80_device :  public device_t, public device_centronics_peripheral_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	epson_mx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		epson_mx80_device(mconfig, EPSON_MX80, tag, owner, clock)
	{
	}

	/* Centronics stuff */

	virtual void input_init(int state) override {};
	virtual void input_strobe(int state) override {};
	virtual void input_data0(int state) override { if (state) m_centronics_data |= 0x01; else m_centronics_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_centronics_data |= 0x02; else m_centronics_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_centronics_data |= 0x04; else m_centronics_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_centronics_data |= 0x08; else m_centronics_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_centronics_data |= 0x10; else m_centronics_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_centronics_data |= 0x20; else m_centronics_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_centronics_data |= 0x40; else m_centronics_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_centronics_data |= 0x80; else m_centronics_data &= ~0x80; }

protected:
	// constructor to pass along a device type
	epson_mx80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool supports_pin35_5v() override { return true; }

protected:
		required_device<i8039_device> m_maincpu;
	required_device<i8041ah_device> m_slavecpu;
	required_device<i8155_device> m_i8155;

	uint8_t t0_r();
	uint8_t t1_r();
	uint8_t port1_r();
	uint8_t port2_r();
	uint8_t bus_r();
	void port1_w(uint8_t data);
	void port2_w(uint8_t data);
	void bus_w(uint8_t data);

	void mx80_io_mem(address_map &map);
	// void mx80_data_mem(address_map &map); // 8039 ram is internal
	void mx80_prog_mem(address_map &map);

	//uint8_t data_r(offs_t offset);  // 8039 ram is internal
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t prog_mem_r(offs_t offset);

	uint8_t i8155_pa_r();
	void i8155_pa_w(uint8_t data);
	uint8_t i8155_pb_r();
	void i8155_pb_w(uint8_t data);
	uint8_t i8155_pc_r();
	void i8155_pc_w(uint8_t data);

	uint8_t slave_bus_r();
	void slave_bus_w(uint8_t data);
	uint8_t slave_port1_r();
	void slave_port1_w(uint8_t data);
	uint8_t slave_port2_r();
	void slave_port2_w(uint8_t data);
	uint8_t slave_t0_r();
	uint8_t slave_t1_r();

	TIMER_CALLBACK_MEMBER(slave_write_data_sync);
	TIMER_CALLBACK_MEMBER(slave_write_command_sync);
	[[maybe_unused]] uint8_t slave_r(offs_t offset);
	[[maybe_unused]] void slave_w(offs_t offset, uint8_t data);

	u8 m_8049_p1 = 0;
	u8 m_8049_p2 = 0;

	u8 m_slave_p1 = 0;
	u8 m_slave_p2 = 0;

	u8 m_8155_pa = 0;
	u8 m_8155_pb = 0;
	u8 m_8155_pc = 0;

	u8 m_centronics_data = 0;

	uint8_t home_sensor();
	uint8_t pts_r();
};

class epson_mx80_dots_device : public epson_mx80_device
{
public:
	epson_mx80_dots_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		epson_mx80_device(mconfig, EPSON_MX80_DOTS, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void mx80dots_prog_mem(address_map &map);
	uint8_t dots_perfect_prog_mem_r(offs_t offset);

	virtual void prog_w(int state);

	u8 m_dots_bank = 0;
	uint8_t m_prog_line = 1;  // status of prog_line
};

class epson_mx80_iii_device : public epson_mx80_device
{
public:
	epson_mx80_iii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		epson_mx80_device(mconfig, EPSON_MX80_III, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

//-------------------------------------------------
//  epson_mx80_device - constructor
//-------------------------------------------------

// constructor that passes device type to device_t
epson_mx80_device::epson_mx80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_slavecpu(*this, "i8041_slave"),
	m_i8155(*this, "i8155_io")
{
}


//-------------------------------------------------
//  ROM( epson_mx80 )
//-------------------------------------------------

ROM_START( epson_mx80 )
	ROM_REGION(0x1800, "mx80_rom", 0)
	ROM_LOAD("mx80_graftrax_rom_1.bin", 0x0000, 0x800, CRC(0843ea56) SHA1(23948acb55760ddbfc59c57c1553b2e733983201))
	ROM_LOAD("mx80_graftrax_rom_2.bin", 0x0800, 0x800, CRC(8435f945) SHA1(c1f0a74bf66d114d0f349bec27c7756997352602))
	ROM_LOAD("mx80_graftrax_rom_3.bin", 0x1000, 0x800, CRC(f1d1c0e7) SHA1(1f4e24575c412368f0b628d2a276c9943debd0ae))

	ROM_REGION(0x400, "i8041_slave", 0)
	ROM_LOAD("8041_mx80.bin", 0x0000, 0x400, CRC(5844ef51) SHA1(1025d34b3ab684a06589b5890c604ce114399d23))
ROM_END

uint8_t epson_mx80_device::prog_mem_r(offs_t offset)
{
	u8 *mx80_rom_ptr = memregion("mx80_rom")->base();

	[[maybe_unused]] auto offset_low = offset & ((1 << 11) - 1); // get low 11 bits  not 1<<12 - 1 but 1<<11 - 1
	auto offset_upper = (offset & (0xf800)) >> 11;

	u8 retval = (offset_upper == 0) ?
		mx80_rom_ptr[ offset_low | (0x800 * 2)] :
			(m_8049_p2 & 0x10) ?
				mx80_rom_ptr[offset_low | 0x0] : mx80_rom_ptr[offset_low | 0x800];
	return retval;
}

ROM_START( epson_mx80_iii )

	ROM_REGION(0x1800, "mx80_rom", 0)
	ROM_LOAD("a2_ha1_1b.bin", 0x0000, 0x800, CRC(5a8a8dec) SHA1(c7f7505e0e6a5916fe17c0a614207a5d1c97e130))
	ROM_LOAD("a1_ha2_2b.bin", 0x0800, 0x800, CRC(a6b448bc) SHA1(bbe8c211b726fbd2fcc92891b8fd475120678e34))
	ROM_LOAD("a2_ha3_3b.bin", 0x1000, 0x800, CRC(a9a2ac25) SHA1(4f66bf77b628c6ca8e61373d49efd28133463228))

	ROM_REGION(0x400, "i8041_slave", 0)
	ROM_LOAD("8041_mx80.bin", 0x0000, 0x400, CRC(5844ef51) SHA1(1025d34b3ab684a06589b5890c604ce114399d23))
ROM_END

ROM_START( epson_mx80_dots )
	ROM_REGION(0x4000, "dots_perfect", 0)
	ROM_LOAD("dots_perfect_mx80_27128.bin", 0x0000, 0x4000, CRC(49bae08f) SHA1(702b3fd3e8fefd983306028915a49cb26880bb16))

	ROM_REGION(0x400, "i8041_slave", 0)
	ROM_LOAD("8041_mx80.bin", 0x0000, 0x400, CRC(5844ef51) SHA1(1025d34b3ab684a06589b5890c604ce114399d23))
ROM_END

uint8_t epson_mx80_dots_device::dots_perfect_prog_mem_r(offs_t offset)
{
	u8 *dots_ptr = memregion("dots_perfect")->base();
	u8 retval = dots_ptr[offset | (m_dots_bank * 0x1000)];
	return retval;
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *epson_mx80_device::device_rom_region() const
{
	return ROM_NAME( epson_mx80 );
}

const tiny_rom_entry *epson_mx80_iii_device::device_rom_region() const
{
	return ROM_NAME( epson_mx80_iii );
}

const tiny_rom_entry *epson_mx80_dots_device::device_rom_region() const
{
	return ROM_NAME( epson_mx80_dots );
}

//-------------------------------------------------
//  ADDRESS_MAP( mx80_mem )
//-------------------------------------------------

void epson_mx80_device::mx80_io_mem(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(epson_mx80_device::io_r),FUNC(epson_mx80_device::io_w));
}

void epson_mx80_device::mx80_prog_mem(address_map &map)
{
	map(0x000, 0xfff).r(FUNC(epson_mx80_device::prog_mem_r));
}

void epson_mx80_dots_device::mx80dots_prog_mem(address_map &map)
{
	map(0x000, 0xfff).r(FUNC(epson_mx80_dots_device::dots_perfect_prog_mem_r));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void epson_mx80_device::device_add_mconfig(machine_config &config)
{
	// 8039/8049 main cpu
	i8039_device &main(I8039(config, m_maincpu, 6000000)); // 6 Mhz can be 8039 or 8049 according to schematic

	main.set_addrmap(AS_PROGRAM, &epson_mx80_device::mx80_prog_mem);
	//main.set_addrmap(AS_DATA, &epson_mx80_device::mx80_data_mem);  // don't need to set the data mem as it's internal
	main.set_addrmap(AS_IO, &epson_mx80_device::mx80_io_mem);

	main.p1_in_cb().set(FUNC(epson_mx80_device::port1_r));
	main.p1_out_cb().set(FUNC(epson_mx80_device::port1_w));
	main.p2_in_cb().set(FUNC(epson_mx80_device::port2_r));
	main.p2_out_cb().set(FUNC(epson_mx80_device::port2_w));
	main.bus_in_cb().set(FUNC(epson_mx80_device::bus_r));
	main.bus_out_cb().set(FUNC(epson_mx80_device::bus_w));
	main.t0_in_cb().set(FUNC(epson_mx80_device::t0_r));  // home sensor
	main.t1_in_cb().set(FUNC(epson_mx80_device::t1_r));  // pts sensor

	[[maybe_unused]] i8041ah_device &slave(I8041AH(config, m_slavecpu, 6000000));
	slave.p1_in_cb().set(FUNC(epson_mx80_device::slave_port1_r));
	slave.p1_out_cb().set(FUNC(epson_mx80_device::slave_port1_w));
	slave.p2_in_cb().set(FUNC(epson_mx80_device::slave_port2_r));
	slave.p2_out_cb().set(FUNC(epson_mx80_device::slave_port2_w));
	slave.bus_in_cb().set(FUNC(epson_mx80_device::slave_bus_r));
	slave.bus_out_cb().set(FUNC(epson_mx80_device::slave_bus_w));
	slave.t0_in_cb().set(FUNC(epson_mx80_device::slave_t0_r));  // home sensor
	slave.t1_in_cb().set(FUNC(epson_mx80_device::slave_t1_r));  // pts sensor

	// 8155 timer input is connected to the PTS sensor, not emulated
	[[maybe_unused]] i8155_device &i8155(I8155(config, m_i8155, 0));
	i8155.in_pa_callback().set(FUNC(epson_mx80_device::i8155_pa_r));
	i8155.out_pa_callback().set(FUNC(epson_mx80_device::i8155_pa_w));
	i8155.in_pb_callback().set(FUNC(epson_mx80_device::i8155_pb_r));
	i8155.out_pb_callback().set(FUNC(epson_mx80_device::i8155_pb_w));
	i8155.in_pc_callback().set(FUNC(epson_mx80_device::i8155_pc_r));
	i8155.out_pc_callback().set(FUNC(epson_mx80_device::i8155_pc_w));
}

void epson_mx80_dots_device::device_add_mconfig(machine_config &config)
{
	epson_mx80_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &epson_mx80_dots_device::mx80dots_prog_mem);
	m_maincpu->prog_out_cb().set(FUNC(epson_mx80_dots_device::prog_w));  // prog output to switch banks (dots perfect)
}

uint8_t epson_mx80_device::i8155_pa_r()
{
	return m_8155_pa;
}

void epson_mx80_device::i8155_pa_w(uint8_t data)
{
	m_8155_pa = data;
}

uint8_t epson_mx80_device::i8155_pb_r()
{
	return m_8155_pb;
}
void epson_mx80_device::i8155_pb_w(uint8_t data)
{
	m_8155_pb = data;
}
uint8_t epson_mx80_device::i8155_pc_r()
{
	return m_8155_pc;
}
void epson_mx80_device::i8155_pc_w(uint8_t data)
{
	m_8155_pc = data;
}

uint8_t epson_mx80_device::slave_t1_r()
{
	return home_sensor();
}

uint8_t epson_mx80_device::slave_t0_r() // print timing sensor
{
	return pts_r();
}

uint8_t epson_mx80_device::t0_r()
{
	return 0;
}

uint8_t epson_mx80_device::t1_r() // print timing sensor
{
	return pts_r();
}

uint8_t epson_mx80_device::io_r(offs_t offset)
{
	return 0;
}

void epson_mx80_device::io_w(offs_t offset, uint8_t data)
{
}

uint8_t epson_mx80_device::slave_port1_r()
{
	return 0;
}

void epson_mx80_device::slave_port1_w(uint8_t data)
{
}

uint8_t epson_mx80_device::slave_port2_r()
{
	return 0;
}

void epson_mx80_device::slave_port2_w(uint8_t data)
{
	m_slave_p2 = data;
}

uint8_t epson_mx80_device::slave_bus_r()
{
	return 0;
}

void epson_mx80_device::slave_bus_w(uint8_t data)
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_mx80_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_mx80_device::device_reset()
{
}

uint8_t epson_mx80_device::port1_r()
{
	return 0;
}

uint8_t epson_mx80_device::port2_r()
{
	return 0;
}

uint8_t epson_mx80_device::bus_r()
{
	return 0;
}

void epson_mx80_device::port1_w(uint8_t data)
{
}

void epson_mx80_device::port2_w(uint8_t data)
{
	m_8049_p2 = data;
}

void epson_mx80_dots_device::prog_w(int state)
{
}

void epson_mx80_device::bus_w(uint8_t data)
{
}

u8 epson_mx80_device::home_sensor()
{
	return 0;
}

uint8_t epson_mx80_device::pts_r() // print timing sensor
{
	return 0;
}

uint8_t epson_mx80_device::slave_r(offs_t offset)
{
		u8 const data = m_slavecpu->upi41_master_r(offset);
		logerror("slave_r (%x)=%02x (%s)\n", offset, data, machine().describe_context());
		return data;
}

void epson_mx80_device::slave_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(epson_mx80_device::slave_write_data_sync), this), unsigned(data));
	else if (offset == 1)
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(epson_mx80_device::slave_write_command_sync), this), unsigned(data));
	logerror("slave_w (%x)=%x %s with offset %x %s\n", offset, data, machine().describe_context(), offset, offset ? "COMMAND" : "DATA");
}

TIMER_CALLBACK_MEMBER(epson_mx80_device::slave_write_data_sync)
{
	m_slavecpu->upi41_master_w(0U, u8(u32(param)));
}

TIMER_CALLBACK_MEMBER(epson_mx80_device::slave_write_command_sync)
{
	m_slavecpu->upi41_master_w(1U, u8(u32(param)));
}

INPUT_PORTS_START( epson_mx80 )
INPUT_PORTS_END

INPUT_PORTS_START( epson_mx80_dots )
INPUT_PORTS_END

ioport_constructor epson_mx80_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_mx80 );
}

ioport_constructor epson_mx80_dots_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_mx80_dots );
}


} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(EPSON_MX80, device_centronics_peripheral_interface, epson_mx80_device, "epson_mx80", "Epson MX-80")
DEFINE_DEVICE_TYPE_PRIVATE(EPSON_MX80_DOTS, device_centronics_peripheral_interface, epson_mx80_dots_device, "epson_mx80_dots", "Epson MX-80 with Dots Perfect Upgrade")
DEFINE_DEVICE_TYPE_PRIVATE(EPSON_MX80_III, device_centronics_peripheral_interface, epson_mx80_iii_device, "epson_mx80_iii", "Epson MX-80 III")

