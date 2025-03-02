// license:BSD-3-Clause
// copyright-holders:Golden Child
/***************************************************************************

    Epson FX-80 Dot Matrix Printer (skeleton)

        Main CPU is a UPD7810H6 running at 10 MHz.
        Slave CPU is an 8042AH running at 11 Mhz.

    Epson JX-80 Dot Matrix Printer (skeleton)
        based on same hardware, adds an expansion board VX0B
        that provides ram expansion and ribbon motor control

*****************************************************************************/

#include "emu.h"
#include "epson_fx80.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/upd7810/upd7810.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class epson_fx80_device : public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_fx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	/* Centronics stuff */
	virtual void input_init(int state) override;
	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { if (state) m_centronics_data |= 0x01; else m_centronics_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_centronics_data |= 0x02; else m_centronics_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_centronics_data |= 0x04; else m_centronics_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_centronics_data |= 0x08; else m_centronics_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_centronics_data |= 0x10; else m_centronics_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_centronics_data |= 0x20; else m_centronics_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_centronics_data |= 0x40; else m_centronics_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_centronics_data |= 0x80; else m_centronics_data &= ~0x80; }

protected:
	epson_fx80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool supports_pin35_5v() override { return true; }

	void epson_fx80_mem(address_map &map) ATTR_COLD;

public:
	uint8_t slave_r(offs_t offset);
	void slave_w(offs_t offset, uint8_t data);
	uint8_t slave_p1_r();
	void slave_p2_w(uint8_t data);
	uint8_t slave_t0_r();
	uint8_t slave_t1_r();
	uint8_t centronics_data_r(offs_t offset);

protected:
	TIMER_CALLBACK_MEMBER(slave_write_data_sync);
	TIMER_CALLBACK_MEMBER(slave_write_command_sync);

	uint8_t pts_r() { return 0; };
	uint8_t home_sensor() { return 0; };

	required_device<upd7810_device> m_maincpu;
	required_device<i8042ah_device> m_slavecpu;

	uint8_t m_centronics_data;
};


class epson_jx80_device : public epson_fx80_device
{
public:
	// construction/destruction
	epson_jx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void epson_jx80_mem(address_map &map) ATTR_COLD;
	uint8_t vxob_r(offs_t offset);
	void vxob_w(offs_t offset, uint8_t data);
};



//-------------------------------------------------
//  ROM( epson_fx80 )
//-------------------------------------------------

ROM_START( epson_fx80 )
	ROM_REGION(0x4000, "maincpu", 0)  // 16K rom for upd7810 FX-80
	// this is from an fx80+ (found with a M20214GA which according to the tech manual goes with an fx80+)
	ROM_LOAD("epson_8426k9_m1206ba029_read_as_27c128.bin", 0x0000, 0x4000, CRC(ff5c2b1e) SHA1(e1e38c3e4864e60f701939e23331360b76603a24))

	ROM_REGION(0x800, "slavecpu", 0)  // 2K rom for 8042
	ROM_LOAD("epson_fx_c42040kb_8042ah.bin", 0x0000, 0x800, CRC(3e9d08c1) SHA1(d5074f60497cc75d40996e6cef63231d3a3697f1))

	ROM_REGION(0x2000, "dots_4a", 0) // 2k rom for dotsperfect upgrade kit for FX-80/JX-80
	ROM_LOAD("dotsperfect_4a_417_as_2764.bin", 0x0000, 0x2000, CRC(4ab92737) SHA1(31ea93cb8aee8622f160d0ac9d3341e6434687cc))

	ROM_REGION(0x4000, "dots_5a", 0) // 4k rom for dotsperfect upgrade kit for FX-80/JX-80
	ROM_LOAD("dotsperfect_5a_417_as_27128.bin", 0x0000, 0x4000, CRC(97bba4c8) SHA1(16703dbbc80d2ef78e29421817cdee97af53e0a5))

	ROM_REGION(0x4000, "jx80", 0)  // JX-80 rom
	ROM_LOAD("jx80_a4_fs5_27128.bin", 0x0000, 0x4000, CRC(2925a47b) SHA1(1864d3561491d7dca78ac2cd13a023460f551184))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *epson_fx80_device::device_rom_region() const
{
	return ROM_NAME( epson_fx80 );
}


//-------------------------------------------------
//  ADDRESS_MAP( epson_fx80_mem )
//-------------------------------------------------

void epson_fx80_device::epson_fx80_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
//  map(0x6000, 0x7fff).ram(); // external RAM VXOB board (optional)
	map(0x8000, 0x97ff).ram(); // external RAM 4K + 2K = 0x1800
	map(0xd000, 0xd001).mirror(0x08fe).rw(FUNC(epson_fx80_device::slave_r), FUNC(epson_fx80_device::slave_w));
	map(0xd800, 0xd800).r(FUNC(epson_fx80_device::centronics_data_r));
}

void epson_jx80_device::epson_jx80_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("jx80", 0);
	map(0x6000, 0x7fff).ram(); // external RAM VXOB board (optional)
	map(0x8000, 0x97ff).ram(); // external RAM 4K + 2K = 0x1800
	map(0xc800, 0xc801).rw(FUNC(epson_jx80_device::vxob_r), FUNC(epson_jx80_device::vxob_w));
	map(0xd000, 0xd001).mirror(0x08fe).rw(FUNC(epson_fx80_device::slave_r), FUNC(epson_fx80_device::slave_w));
	map(0xd800, 0xd800).r(FUNC(epson_fx80_device::centronics_data_r));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void epson_fx80_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	upd7810_device &upd(UPD7810(config, m_maincpu, 10000000)); // 10 Mhz
	upd.set_addrmap(AS_PROGRAM, &epson_fx80_device::epson_fx80_mem);

	/* upi41 i8042 slave cpu */
	i8042ah_device &sla(I8042AH(config, m_slavecpu, 11000000)); // 11 Mhz
	sla.p1_in_cb().set(FUNC(epson_fx80_device::slave_p1_r));
	sla.p2_out_cb().set(FUNC(epson_fx80_device::slave_p2_w));
	sla.t0_in_cb().set(FUNC(epson_fx80_device::slave_t0_r));  // home sensor
	sla.t1_in_cb().set(FUNC(epson_fx80_device::slave_t1_r));  // pts sensor
}

void epson_jx80_device::device_add_mconfig(machine_config &config)
{
	epson_fx80_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &epson_jx80_device::epson_jx80_mem);
}


//-------------------------------------------------
//  INPUT_PORTS( epson_fx80 )
//-------------------------------------------------

INPUT_PORTS_START( epson_fx80 )
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor epson_fx80_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_fx80 );
}

//-------------------------------------------------
//  epson_fx80_device - constructor
//-------------------------------------------------

// constructor that passes device type
epson_fx80_device::epson_fx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	epson_fx80_device(mconfig, EPSON_FX80, tag, owner, clock)
{
}

// constructor to pass through the device type to device_t
epson_fx80_device::epson_fx80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_slavecpu(*this, "slavecpu")
{
}

// constructor that pass device type
epson_jx80_device::epson_jx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: epson_fx80_device(mconfig, EPSON_JX80, tag, owner, clock)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_fx80_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_fx80_device::device_reset()
{
}

//-------------------------------------------------
//  Slave CPU functions
//-------------------------------------------------

uint8_t epson_fx80_device::slave_r(offs_t offset)
{
	u8 const data = m_slavecpu->upi41_master_r(offset);
	return data;
}

void epson_fx80_device::slave_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		machine().scheduler().synchronize(
			timer_expired_delegate(FUNC(epson_fx80_device::slave_write_data_sync), this), unsigned(data));
	else if (offset == 1)
		machine().scheduler().synchronize(
			timer_expired_delegate(FUNC(epson_fx80_device::slave_write_command_sync), this), unsigned(data));
}

TIMER_CALLBACK_MEMBER(epson_fx80_device::slave_write_data_sync)
{
	m_slavecpu->upi41_master_w(0U, u8(u32(param)));
}

TIMER_CALLBACK_MEMBER(epson_fx80_device::slave_write_command_sync)
{
	m_slavecpu->upi41_master_w(1U, u8(u32(param)));
}

uint8_t epson_fx80_device::slave_t0_r()
{
	return home_sensor();
}

uint8_t epson_fx80_device::slave_t1_r()
{
	return pts_r();  // print timing sensor
}

uint8_t epson_fx80_device::slave_p1_r()
{
	// read DIPSW1
	return 0;
}

void epson_fx80_device::slave_p2_w(uint8_t data)
{
	// p20 - p21 are cr_stepper
	// p22, p24 stepper power
	// p25 not err
	// p26 online lamp
	// p27 buzzer
}


//-------------------------------------------------
//  VX0B expansion board (for JX-80)
//-------------------------------------------------

uint8_t epson_jx80_device::vxob_r(offs_t offset)
{
	// read color ribbon home position status, MSB is high if in home position zone
	return 0;  // not hooked up
}

void epson_jx80_device::vxob_w(offs_t offset, uint8_t data)
{
	// bit 6 and 7 control the color ribbon stepper motor
}

//-------------------------------------------------
//  Centronics data read
//-------------------------------------------------

uint8_t epson_fx80_device::centronics_data_r(offs_t offset)
{
	return 0;
}

//-------------------------------------------------
//  Centronics device
//-------------------------------------------------

void epson_fx80_device::input_strobe(int state)
{
}

void epson_fx80_device::input_init(int state)
{
}

} // anonymous namespace

// GLOBAL
DEFINE_DEVICE_TYPE_PRIVATE(EPSON_FX80, device_centronics_peripheral_interface, epson_fx80_device, "epson_fx80", "Epson FX-80")
DEFINE_DEVICE_TYPE_PRIVATE(EPSON_JX80, device_centronics_peripheral_interface, epson_jx80_device, "epson_jx80", "Epson JX-80")
