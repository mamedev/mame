// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Nintendo Game Boy Memory Bank Controller 7

 Only used for two games, the cartridges contain an ADXL202E two-axis
 accelerometer and a 93LC56 or 93LC66 serial EEPROM, as well as the MBC7
 chip and program ROM.  The MBC7 never seems to have been fully utilised.
 It appears to have inputs for an additional axis sensor, and an unused
 8-bit port of some kind.  Supports up to 2 MiB ROM (128 16 KiB pages).

 0x0000-3FFF R  - Fixed ROM bank, always first page of ROM.
 0x4000-7FFF R  - Selectable ROM bank.

 0x0000-1FFF W  - I/O enable - write 0x0A on low nybble to enable, any
                  other value to disable.
 0x2000-3FFF W  - Select ROM page mapped at 0x4000.  Only low seven bits are
                  significant.
 0x4000-5FFF W  - I/O select - presumably switches between sensors and
                  EEPROM, and some unpopulated peripheral.  Write 0x40 to
                  select sensors and EEPROM.  Values seen include 0x00,
                  0x3F, 0x40 and 0x54.

 When I/O is enabled:
 0xA.0.      W  - Write 0x55 to clear accelerometer values (subsequent
                  reads will return 0x8000).
 0xA.1.      W  - Write 0xAA to acquire and latch accelerometer values
                  (must be in cleared state, or no effect).
 0xA.2.      R  - Accelerometer X value least significant byte.
 0xA.3.      R  - Accelerometer X value most significant byte.
 0xA.4.      R  - Accelerometer Y value least significant byte.
 0xA.5.      R  - Accelerometer Y value most significant byte.
 0xA.6.      R  - Always 0x00 (presumably for unpopulated Z axis sensor).
 0xA.7.      R  - Always 0xFF (presumably for unpopulated Z axis sensor).
 0xA.8.      RW - EEPROM: bit 7 = CS, bit 6 = CLK, bit 1 = DI, bit 0 = DO.

 Accelerometers read 0x81D0 in neutral orientation, and 1 g produces a
 displacement of about 0x70 in either direction.  The ADXL202E is rated for
 full-scale output at 2 g in either direction.  Inputs are set up to
 simulate up to about 1.5 g.

 TODO:
 * Can ROM page 0 be selected like MBC5, or does it map to 1 like MBC1?
 * EEPROM initialisation from software list item 'nvram' region.
 * Reset state for EEPROM outputs.
 * How is direction controlled for I/O lines?

 ***************************************************************************/

#include "emu.h"
#include "mbc7.h"

#include "cartbase.h"

#include "machine/eepromser.h"

#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

//**************************************************************************
//  Class declarations
//**************************************************************************

class mbc7_device_base : public mbc_device_base
{
public:
	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	mbc7_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<eeprom_serial_93cxx_device> m_eeprom;

private:
	void bank_rom_switch(u8 data);
	template <unsigned N, u8 Value, u8 Mask> void enable_io(u8 data);
	void clear_accel(u8 data);
	void acquire_accel(u8 data);
	template <unsigned N> u8 read_accel(offs_t offset);
	u8 read_unknown(offs_t offset);
	u8 read_eeprom();
	void write_eeprom(u8 data);

	required_ioport_array<2> m_accel;
	memory_view m_view_io;

	u8 m_io_enable[2];

	u16 m_accel_val[2];
	u8 m_accel_latched;

	u8 m_eeprom_out;
};


class mbc7_2k_device : public mbc7_device_base
{
public:
	mbc7_2k_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		mbc7_device_base(mconfig, GB_ROM_MBC7_2K, tag, owner, clock)
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		EEPROM_93C56_16BIT(config, m_eeprom, 0);
	}
};


class mbc7_4k_device : public mbc7_device_base
{
public:
	mbc7_4k_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		mbc7_device_base(mconfig, GB_ROM_MBC7_4K, tag, owner, clock)
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		EEPROM_93C66_16BIT(config, m_eeprom, 0);
	}
};



//**************************************************************************
//  Input port definitions
//**************************************************************************

INPUT_PORTS_START(mbc7)
	PORT_START("ACCELX")
	PORT_BIT(0x01ff, 0x00c0, IPT_AD_STICK_X) PORT_REVERSE PORT_MINMAX(0x0000, 0x0180) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)

	PORT_START("ACCELY")
	PORT_BIT(0x01ff, 0x00c0, IPT_AD_STICK_Y) PORT_REVERSE PORT_MINMAX(0x0000, 0x0180) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END



//**************************************************************************
//  mbc7_device_base
//**************************************************************************

image_init_result mbc7_device_base::load(std::string &message)
{
	// first check ROM
	set_bank_bits_rom(7);
	if (!check_rom(message))
		return image_init_result::FAIL;

	// install ROM and handlers
	install_rom();
	cart_space()->install_write_handler(
			0x0000, 0x1fff,
			write8smo_delegate(*this, NAME((&mbc7_device_base::enable_io<0, 0x0a, 0x0f>))));
	cart_space()->install_write_handler(
			0x2000, 0x3fff,
			write8smo_delegate(*this, FUNC(mbc7_device_base::bank_rom_switch)));
	cart_space()->install_write_handler(
			0x4000, 0x5fff,
			write8smo_delegate(*this, NAME((&mbc7_device_base::enable_io<1, 0x40, 0xff>))));
	cart_space()->install_view(
			0xa000, 0xafff,
			m_view_io);
	m_view_io[0].install_write_handler(
			0xa000, 0xa00f, 0x0000, 0x0f00, 0x0000,
			write8smo_delegate(*this, FUNC(mbc7_device_base::clear_accel)));
	m_view_io[0].install_write_handler(
			0xa010, 0xa01f, 0x0000, 0x0f00, 0x0000,
			write8smo_delegate(*this, FUNC(mbc7_device_base::acquire_accel)));
	m_view_io[0].install_read_handler(
			0xa020, 0xa03f, 0x0000, 0x0f00, 0x0000,
			read8sm_delegate(*this, FUNC(mbc7_device_base::read_accel<0>)));
	m_view_io[0].install_read_handler(
			0xa040, 0xa05f, 0x0000, 0x0f00, 0x0000,
			read8sm_delegate(*this, FUNC(mbc7_device_base::read_accel<1>)));
	m_view_io[0].install_read_handler(
			0xa060, 0xa07f, 0x0000, 0x0f00, 0x0000,
			read8sm_delegate(*this, FUNC(mbc7_device_base::read_unknown)));
	m_view_io[0].install_readwrite_handler(
			0xa080, 0xa08f, 0x0000, 0x0f00, 0x0000,
			read8smo_delegate(*this, FUNC(mbc7_device_base::read_eeprom)),
			write8smo_delegate(*this, FUNC(mbc7_device_base::write_eeprom)));

	// all good
	return image_init_result::PASS;
}


mbc7_device_base::mbc7_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_device_base(mconfig, type, tag, owner, clock),
	m_eeprom(*this, "eeprom"),
	m_accel(*this, "ACCEL%c", 'X'),
	m_view_io(*this, "io"),
	m_io_enable{ 0U, 0U },
	m_accel_val{ 0U, 0U },
	m_accel_latched(0U),
	m_eeprom_out(0U)
{
}


ioport_constructor mbc7_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME(mbc7);
}


void mbc7_device_base::device_start()
{
	mbc_device_base::device_start();

	save_item(NAME(m_io_enable));
	save_item(NAME(m_accel_val));
	save_item(NAME(m_accel_latched));
	save_item(NAME(m_eeprom_out));
}


void mbc7_device_base::device_reset()
{
	mbc_device_base::device_reset();

	std::fill(std::begin(m_io_enable), std::end(m_io_enable), 0U);
	std::fill(std::begin(m_io_enable), std::end(m_io_enable), 0x8000U);
	m_accel_latched = 0U;

	set_bank_rom(0);
	m_view_io.disable();
}


void mbc7_device_base::bank_rom_switch(u8 data)
{
	set_bank_rom(data & 0x7f);
}


template <unsigned N, u8 Value, u8 Mask>
void mbc7_device_base::enable_io(u8 data)
{
	m_io_enable[N] = (Value == (data & Mask)) ? 1U : 0U;
	bool const enable(m_io_enable[0] && m_io_enable[1]);
	LOG(
			"%s: I/O enable %u = 0x%02X, I/O %s\n",
			machine().describe_context(),
			N,
			data,
			enable ? "enabled" : "disabled");
	if (enable)
		m_view_io.select(0);
	else
		m_view_io.disable();
}


void mbc7_device_base::clear_accel(u8 data)
{
	if (0x55 != data)
	{
		logerror(
				"%s: Ignoring unknown command 0x%02X written to accelerometer clear register\n",
				machine().describe_context(),
				data);
	}
	else
	{
		m_accel_val[0] = 0x8000U;
		m_accel_val[1] = 0x8000U;
		m_accel_latched = 0U;
		LOG("%s: Accelerometer data cleared\n", machine().describe_context());
	}
}


void mbc7_device_base::acquire_accel(u8 data)
{
	if (0xaa != data)
	{
		logerror(
				"%s: Ignoring unknown command 0x%02X written to accelerometer acquire register\n",
				machine().describe_context(),
				data);
	}
	else if (m_accel_latched)
	{
		logerror(
				"%s: Ignoring accelerometer acquire command while data latched\n",
				machine().describe_context());
	}
	else
	{
		m_accel_val[0] = 0x81d0 - 0x00c0 + m_accel[0]->read();
		m_accel_val[1] = 0x81d0 - 0x00c0 + m_accel[1]->read();
		m_accel_latched = 1U;
		LOG(
				machine().describe_context(),
				"%s: Accelerometer data acquired X = 0x%04X Y = 0x%04X\n",
				m_accel_val[0],
				m_accel_val[1]);
	}
}


template <unsigned N>
u8 mbc7_device_base::read_accel(offs_t offset)
{
	u8 const result(m_accel_val[N] >> (BIT(offset, 4) ? 8 : 0));
	LOG(
			"Read accelerometer %c %cSB = 0x%02X\n",
			machine().describe_context(),
			'X' + N,
			BIT(offset, 4) ? 'M' : 'L',
			result);
	return result;
}


u8 mbc7_device_base::read_unknown(offs_t offset)
{
	return BIT(offset, 4) ? 0xff : 0x00;
}


u8 mbc7_device_base::read_eeprom()
{
	u8 const dout(m_eeprom->do_read() ? 0x01U : 0x00U);
	LOG("%s: Read EEPROM DO = %u\n", machine().describe_context(), dout);
	return (m_eeprom_out & 0xfe) | dout;
}


void mbc7_device_base::write_eeprom(u8 data)
{
	LOG(
			"%s: Write EEPROM CS = %u CLK = %u DI = %u\n",
			machine().describe_context(),
			BIT(data, 7),
			BIT(data, 6),
			BIT(data, 1));
	m_eeprom_out = data;
	m_eeprom->cs_write(BIT(data, 7));
	m_eeprom->clk_write(BIT(data, 6));
	m_eeprom->di_write(BIT(data, 1));
}

} // anonymous namespace

} // namespace bus::gameboy



//**************************************************************************
//  Device type definitions
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_MBC7_2K, device_gb_cart_interface, bus::gameboy::mbc7_2k_device, "gb_rom_mbc7_2k", "Game Boy MBC7 Cartridge with 93LC56")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_MBC7_4K, device_gb_cart_interface, bus::gameboy::mbc7_4k_device, "gb_rom_mbc7_4k", "Game Boy MBC7 Cartridge with 93LC66")
