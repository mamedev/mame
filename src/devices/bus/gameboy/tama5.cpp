// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Wilbert Pol
/***************************************************************************

 TAMA5 Mapper (Used by Tamagotchi 3)
 ============

 Cartridge contains the following chips:
 * TAMA5 Game Boy bus interface
 * TAMA6 Toshiba TMP47C243M TLCS-47 microcontroller
 * TAMA7 mask ROM
 * Toshiba TC8521AM real-time clock
 * Toshiba M62021P system reset IC with switch for memory backup

 The MCU acts as an interface between the TAMA5 and the RTC.  The MCU can
 drive a piezo speaker (in response to alarms from the RTC).  There is a
 mechanical switch that disconnects power from the piezo speaker.  The RTC,
 MCU and piezo speaker have backup power supplied by a user-replaceable
 CR2016 coin cell.

 The TAMA5 chip only responds to A15, A14, A0, and D3-D0, i.e. addresses are
 effectively masked with 0xC001, and data is effectively masked with 0x0F.

 Status: partially supported.

 The TAMA5 mapper includes a special RTC chip which communicates through the
 RAM area (0xA000-0xBFFF); most notably addresses 0xA000 and 0xA001 seem to
 be used. In this setup 0xA001 acts like a control register and 0xA000 like
 a data register.

 Accepted values by the TAMA5 control register:
 0x00 - Writing to 0xA000 will set bits 3-0 for ROM bank selection.
 0x01 - Writing to 0xA000 will set bits (7-?)4 for ROM bank selection.

 0x04 - Bits 3-0 of the value to write
 0x05 - Bits 4-7 of the value to write
 0x06 - Address control hi
        bit 0 - Bit 4 for the address
        bit 3-1 - 000 - Write a byte to the 32 byte memory. The data to be
                        written must be set in registers 0x04 (lo nibble) and
                        0x05 (hi nibble).
                - 001 - Read a byte from the 32 byte memory. The data read
                        will be available in registers 0x0C (lo nibble) and
                        0x0D (hi nibble).
                - 010 - Unknown (occurs just after having started a game and
                        entered a date) (execution at address 1A19)
                - 011 - Unknown (not encountered yet)
                - 100 - Unknown (occurs during booting a game; appears to be
                        some kind of read command as it is followed by a read
                        of the 0x0C register) (execution at address 1B5B)
                - 101 - Unknown (not encountered yet)
                - 110 - Unknown (not encountered yet)
                - 111 - Unknown (not encountered yet)
 0x07 - Address control lo
        bit 3-0 - bits 3-0 for the address

 0x0A - After writing this the lowest 2 bits of A000 determine whether the
        TAMA5 chip is ready to accept the next command. If the lowest 2 bits
        hold the value 01 then the TAMA5 chip is ready for the next command.

 0x0C - Reading from A000 will return bits 3-0 of the data
 0x0D - Reading from A000 will return bits 7-4 of the data

 0x04 - RTC controls? -> RTC/memory?
 0x05 - Write time/memomry?
 0x06 - RTC controls?
 0x07 - RTC controls?

 Unknown sequences:
 During booting a game (1B5B:
 04 <- 00, 06 <- 08, 07 <- 01, followed by read 0C
 when value read from 0C equals 00 followed by the sequence:
 04 <- 01, 06 <- 08, 07 <- 01, followed by read 0C
 the value read from 0C is checked for non-zero, don't know the consequences for either
 yet.

 Initialization after starting a game:
 At address 1A19:
 06 <- 05, 07 <- 02, followed by read 0C, if != 0F => OK, otherwise do something.

 ***************************************************************************/

#include "emu.h"
#include "tama5.h"

#include "cartbase.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class tama5_device : public mbc_device_base
{
public:
	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	tama5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 data_r(address_space &space);
	void data_w(u8 data);
	void command_w(u8 data);

	u8 m_bank_sel_rom;

	u8 m_registers[0x20];
	u8 m_response;
	u8 m_command;
	u8 m_data;
	u8 m_address;
};


tama5_device::tama5_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_device_base(mconfig, GB_ROM_TAMA5, tag, owner, clock),
	m_bank_sel_rom(0U),
	m_response(0U),
	m_command(0U),
	m_data(0U),
	m_address(0U)
{
}


image_init_result tama5_device::load(std::string &message)
{
	// set up ROM
	set_bank_bits_rom(5);
	if (!check_rom(message))
		return image_init_result::FAIL;
	install_rom();

	// install I/O
	cart_space()->install_read_handler(0xa000, 0xa000, 0x0000, 0x1fff, 0x0000, read8mo_delegate(*this, FUNC(tama5_device::data_r)));
	cart_space()->install_write_handler(0xa000, 0xa000, 0x0000, 0x1ffe, 0x0000, write8smo_delegate(*this, FUNC(tama5_device::data_w)));
	cart_space()->install_write_handler(0xa001, 0xa001, 0x0000, 0x1ffe, 0x0000, write8smo_delegate(*this, FUNC(tama5_device::command_w)));

	// all good
	return image_init_result::PASS;
}


void tama5_device::device_start()
{
	mbc_device_base::device_start();

	save_item(NAME(m_bank_sel_rom));
	save_item(NAME(m_registers));
	save_item(NAME(m_response));
	save_item(NAME(m_command));
	save_item(NAME(m_data));
	save_item(NAME(m_address));
}


void tama5_device::device_reset()
{
	mbc_device_base::device_reset();

	m_bank_sel_rom = 1;

	std::fill(std::begin(m_registers), std::end(m_registers), 0xffU);
	m_response = 0xffU;
	m_command = 0x00U;
	m_data = 0x00U;
	m_address = 0x00U;

	set_bank_rom(m_bank_sel_rom);
}


u8 tama5_device::data_r(address_space &space)
{
	return (space.unmap() & 0xf0) | (m_response & 0x0f);
}


void tama5_device::data_w(u8 data)
{
	switch (m_command)
	{
	case 0x00:      // ROM bank low
		m_bank_sel_rom = (m_bank_sel_rom & 0x10) | (data & 0x0f);
		set_bank_rom(m_bank_sel_rom);
		break;
	case 0x01:      // ROM bank high
		m_bank_sel_rom = (m_bank_sel_rom & 0x0f) | ((data & 0x01) << 4);
		set_bank_rom(m_bank_sel_rom);
		break;
	case 0x04:      // value low
		m_data = (m_data & 0xf0) | (data & 0x0f);
		break;
	case 0x05:      // value high
		m_data = (m_data & 0x0f) | (data << 4);
		break;
	case 0x06:      // address high
		m_address = (m_address & 0x0f) | (data << 4);
		break;
	case 0x07:      // address low/trigger
		m_address = (m_address & 0xf0) | (data & 0x0f);
		switch (m_address & 0xe0)
		{
		case 0x00:
			LOG("%s: write register 0x%02X = 0x%02X\n", machine().describe_context(), m_address & 0x1f, m_data);
			m_registers[m_address & 0x1f] = m_data;
			break;
		case 0x20:
			LOG("%s: read register 0x%02X\n", machine().describe_context(), m_address & 0x1f);
			m_data = m_registers[m_address & 0x1f];
			break;
		case 0x40:  // unknown - some kind of read
			if ((m_address & 0x1f) == 0x12)
				m_data = 0xff;
			[[fallthrough]];
		case 0x80:  // unknown - some kind of read when 0x07=0x01, or write when 0x07=0x00 or 0x07=0x02
		default:
			logerror("%s: Unknown access 0x%02X = 0x%02X\n", machine().describe_context(), m_address, m_data);
		}
	}
}


void tama5_device::command_w(u8 data)
{
	switch (data)
	{
	case 0x00:      // ROM bank low
	case 0x01:      // ROM bank high
	case 0x04:      // value low
	case 0x05:      // value high
	case 0x06:      // address high
	case 0x07:      // address low/trigger
		break;
	case 0x0a:      // ready status in least significant bit
		m_response = 0x01U;
		break;
	case 0x0c:      // read value low
		m_response = m_data & 0x0f;
		break;
	case 0x0d:      // read value high
		m_response = m_data >> 4;
		break;
	default:
		logerror("%s: Unknown command 0x%02X\n", machine().describe_context(), data);
	}
	m_command = data;
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_TAMA5, device_gb_cart_interface, bus::gameboy::tama5_device, "gb_rom_tama5", "Game Boy Bandai Tamagotchi Cartridge")
