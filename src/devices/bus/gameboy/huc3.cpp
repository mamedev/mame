// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Hudson Soft HuC-3 Memory Controller

 Provides ROM and RAM banking, infrared I/O, a real-time clock and a melody
 generator.

 The HuC-3 controller appears to only respond to A15-A13 and D6-D0, i.e.
 addresses are effectively masked with 0xE000 and data is effectively masked
 with 0x7F.

 Major components in the cartridge include:
 * U1 program ROM
 * U2 HuC-3 controller
 * U3 LH52256CT-10LL 32K*8 static RAM
 * U4 MM1026A or MM1134A system reset/backup power switch
 * U5 TC74LVX04FT hex inverter
 * D1 infrared LED
 * Q1 infrared phototransistor
 * X1 real-time clock crystal
 * BATT CR2025 coin cell (user-replaceable)

 HuC-3 48-pin QFP known connectons:
  1                 13  D1             25                 37
  2  Audio out      14  D0             26                 38  XTAL out
  3  Audio out      15                 27                 39  XTAL in
  4  Audio out      16                 28                 40  GND
  5                 17                 29                 41  GND
  6                 18  A15            30                 42  GND
  7  GND            19  GND            31  GND            43
  8  D6             20                 32                 44
  9  D5             21                 33                 45  /WR
 10  D4             22                 34                 46  /RD
 11  D3             23  A13            35  GND            47
 12  D2             24  IR out         36  GND            48

 0x0000-3FFF R  - Fixed ROM bank, always first page of ROM.
 0x4000-7FFF R  - Selectable ROM bank, page 0-255 of ROM.
 0xA000-BFFF RW - Static RAM or I/O.

 0x0000-1FFF W  - Select RAM or I/O at 0xA000.
 0x2000-3FFF W  - Select ROM page mapped at 0x4000.
 0x4000-5FFF W  - Select RAM page mapped at 0xA000.

 Only the four least significant bits of the RAM or I/O selection value are
 significant.  The ten unused values will map nothing into 0xA000-0xBFFF.
 Used values:
 0x0 - RAM (read-only)
 0xA - RAM (read/write)
 0xB - Write command/data
 0xC - Read command/data
 0xD - Clear least significant bit to execute command
 0xE - Infrared I/O

 The HuC-3 chip likely contains a 4-bit microcontroller that implements the
 real-time clock and melody generator functionality.  The game communicates
 with the microcontroller via I/O 0xB, 0xC and 0xD.

 Conceptually, the value written to 0xB contains two values: a 3-bit command
 in bits 6-4, and a 4-bit value in bits 3-0.  Reading 0x0C gives the same
 command in bits 6-4 and a response value in bits 3-0.  Writing to 0xB or
 reading from 0xC has no immediate side effects.

 Bit 0 for 0xD reads high when the microcontroller is ready to accept a
 command.  Writing with bit 0 clear causes the microcontroller to execute
 the command previously written to 0xB.  The microcontroller will set bit 0
 when it has completed the command and is ready to execute another command.

 Five of the eight possible commands are used by the games:
 0x1 - Read register and increment address (value put in bits 3-0 of 0xC)
 0x3 - Write register and increment address (value from bits 3-0 of 0xB)
 0x4 - Set register address low nybble
 0x5 - Set register address high nybble
 0x6 - Execute extended command (selector from bits 3-0 of 0xB)

 The games use four of the sixteen possible extended commands:
 0x0 - Atomically read real-time clock to registers 0-7
 0x1 - Atomically write real-time clock from registers 0-7
 0x2 - Some kind of handshake/status request - sets result to 0x1
 0xe - Sent twice to trigger melody generator

 Registers are likely a window into the microcontroller's memory.  Known
 registers:
 0x00-02 - Minute counter read/write (least significant nybble low)
 0x03-05 - Day counter read/write (least significant nybble low)
 0x10-12 - Minute counter (least significant nybble low)
 0x13-15 - Day counter (least significant nybble low)
 0x26    - Bits 1-0 select melody
 0x27    - Enable (0x1) or disable (not 0x1) melody

 TODO:
 * Implement real-time clock.
 * Simulate melody generator?
 * Which of the internal registers are battery-backed?
 * What is the default state for banking and infrared select on reset?
 * Does ROM bank 0 map to bank 1 like MBC1?
 * How many RAM page lines are there?  No games use more than 2.

 ***************************************************************************/

#include "emu.h"
#include "huc3.h"

#include "cartbase.ipp"

#include <algorithm>
#include <iterator>
#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class huc3_device : public mbc_ram_device_base<mbc_dual_device_base>
{
public:
	static constexpr feature_type unemulated_features() { return feature::SOUND | feature::COMMS; }

	huc3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void io_select(u8 data);
	void bank_switch_fine(u8 data);
	void bank_switch_coarse(u8 data);
	void write_command(u8 data);
	u8 read_command(address_space &space);
	u8 read_status(address_space &space);
	void write_control(u8 data);
	u8 read_ir(address_space &space);
	void write_ir(u8 data);

	void execute_instruction()
	{
		switch (m_ctrl_data & 0x0f)
		{
		case 0x0:
			LOG("Instruction 0x0 - atomic RTC read\n");
			std::copy_n(&m_registers[0x10], 7, &m_registers[0x00]);
			break;
		case 0x1:
			LOG("Instruction 0x2 - atomic RTC write\n");
			std::copy_n(&m_registers[0x00], 7, &m_registers[0x10]);
			break;
		case 0x2:
			logerror("Instruction 0x2 - setting data to 0x1\n");
			m_ctrl_data = 0x01U;
			break;
		case 0xe:
			logerror("Instruction 0xE - play melody\n");
			break;
		default:
			logerror(
					"%s: Unknown instruction 0x%X\n",
					machine().describe_context(),
					m_ctrl_data);
		}
	}

	memory_view m_view_io;

	u8 m_ctrl_cmd;
	u8 m_ctrl_data;
	u8 m_ctrl_addr;
	u8 m_registers[0x100];
};


huc3_device::huc3_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_ram_device_base<mbc_dual_device_base>(mconfig, GB_ROM_HUC3, tag, owner, clock),
	m_view_io(*this, "io"),
	m_ctrl_cmd(0U),
	m_ctrl_data(0U),
	m_ctrl_addr(0U)
{
}


image_init_result huc3_device::load(std::string &message)
{
	// check for valid ROM/RAM regions
	set_bank_bits_rom(2, 7);
	set_bank_bits_ram(2);
	if (!check_rom(message) || !check_ram(message))
		return image_init_result::FAIL;

	// if that checked out, install memory
	cart_space()->install_view(0xa000, 0xbfff, m_view_io);
	install_rom();
	install_ram(m_view_io[0], m_view_io[1]);

	// install memory controller handlers
	cart_space()->install_write_handler(
			0x0000, 0x1fff,
			write8smo_delegate(*this, FUNC(huc3_device::io_select)));
	cart_space()->install_write_handler(
			0x2000, 0x3fff,
			write8smo_delegate(*this, FUNC(huc3_device::bank_switch_fine)));
	cart_space()->install_write_handler(
			0x4000, 0x5fff,
			write8smo_delegate(*this, FUNC(huc3_device::bank_switch_coarse)));

	// install I/O handlers
	m_view_io[2].install_write_handler(
			0xa000, 0xbfff,
			write8smo_delegate(*this, FUNC(huc3_device::write_command)));
	m_view_io[3].install_read_handler(
			0xa000, 0xbfff,
			read8mo_delegate(*this, FUNC(huc3_device::read_command)));
	m_view_io[4].install_read_handler(
			0xa000, 0xbfff,
			read8mo_delegate(*this, FUNC(huc3_device::read_status)));
	m_view_io[4].install_write_handler(
			0xa000, 0xbfff,
			write8smo_delegate(*this, FUNC(huc3_device::write_control)));
	m_view_io[5].install_read_handler(
			0xa000, 0xbfff,
			read8mo_delegate(*this, FUNC(huc3_device::read_ir)));
	m_view_io[5].install_write_handler(
			0xa000, 0xbfff,
			write8smo_delegate(*this, FUNC(huc3_device::write_ir)));

	// all good
	return image_init_result::PASS;
}


void huc3_device::device_start()
{
	mbc_ram_device_base<mbc_dual_device_base>::device_start();

	std::fill(std::begin(m_registers), std::end(m_registers), 0U);

	save_item(NAME(m_ctrl_cmd));
	save_item(NAME(m_ctrl_data));
	save_item(NAME(m_ctrl_addr));
	save_item(NAME(m_registers));
}


void huc3_device::device_reset()
{
	mbc_ram_device_base<mbc_dual_device_base>::device_reset();

	// TODO: what's the proper reset state?
	m_view_io.disable();

	set_bank_rom_fine(0);
	set_bank_rom_coarse(0);
	set_bank_ram(0);

	m_ctrl_cmd = 0U;
	m_ctrl_data = 0U;
	m_ctrl_addr = 0U;
}


void huc3_device::io_select(u8 data)
{
	switch (data & 0x0f)
	{
	case 0x00:
		LOG("%s: Select RAM (read-only)\n", machine().describe_context());
		m_view_io.select(0);
		break;
	case 0x0a:
		LOG("%s: Select RAM (read/write)\n", machine().describe_context());
		m_view_io.select(1);
		break;
	case 0x0b:
		LOG("%s: Select control data write\n", machine().describe_context());
		m_view_io.select(2);
		break;
	case 0x0c:
		LOG("%s: Select control data read\n", machine().describe_context());
		m_view_io.select(3);
		break;
	case 0x0d:
		LOG("%s: Select control command/status\n", machine().describe_context());
		m_view_io.select(4);
		break;
	case 0x0e:
		LOG("%s: Select infrared I/O\n", machine().describe_context());
		m_view_io.select(5);
		break;
	default:
		LOG("%s: Select unused I/O 0x%X\n", machine().describe_context(), data & 0x0f);
		m_view_io.disable();
	}
}


void huc3_device::bank_switch_fine(u8 data)
{
	// TODO: does zero map to bank 1 like MBC1?
	set_bank_rom_fine(data & 0x7f);
}


void huc3_device::bank_switch_coarse(u8 data)
{
	// TODO: how many output lines are physically present?
	set_bank_rom_coarse(data & 0x03);
	set_bank_ram(data & 0x03);
}


void huc3_device::write_command(u8 data)
{
	m_ctrl_cmd = BIT(data, 4, 3);
	m_ctrl_data = BIT(data, 0, 4);
	LOG(
			"%s: Write command = 0x%X data = 0x%X\n",
			machine().describe_context(),
			m_ctrl_cmd,
			m_ctrl_data);
}


u8 huc3_device::read_command(address_space &space)
{
	LOG(
			"%s: Read command = 0x%X data = 0x%X\n",
			machine().describe_context(),
			m_ctrl_cmd,
			m_ctrl_data);
	return (space.unmap() & 0x80) | (m_ctrl_cmd << 4) | m_ctrl_data;
}


u8 huc3_device::read_status(address_space &space)
{
	LOG("%s: Read status\n", machine().describe_context());
	return (space.unmap() & 0x80) | 0x7f; // least significant bit set when ready to receive a command
}


void huc3_device::write_control(u8 data)
{
	// TODO: Is there more to this?
	LOG("%s: Write control = 0x%02X\n", machine().describe_context(), data);
	if (!BIT(data, 0))
	{
		switch (m_ctrl_cmd)
		{
		case 0x1:
			LOG("Command 0x1 - read register 0x%02X\n", m_ctrl_addr);
			m_ctrl_data = m_registers[m_ctrl_addr++] & 0x0f;
			break;
		case 0x3:
			LOG("Command 0x3 - write register 0x%02X = 0x%X\n", m_ctrl_addr, m_ctrl_data);
			m_registers[m_ctrl_addr++] = m_ctrl_data & 0x0f;
			break;
		case 0x4:
			m_ctrl_addr = (m_ctrl_addr & 0xf0) | (m_ctrl_data & 0x0f);
			LOG("Command 0x4 - set register address = 0x%02X\n", m_ctrl_addr);
			break;
		case 0x5:
			m_ctrl_addr = (m_ctrl_addr & 0x0f) | (m_ctrl_data << 4);
			LOG("Command 0x5 - set register address = 0x%02X\n", m_ctrl_addr);
			break;
		case 0x6:
			LOG("Command 0x6 - execute instruction 0x%X\n", m_ctrl_data);
			execute_instruction();
			break;
		default:
			logerror(
					"%s: Unknown command 0x%X data = 0x%X\n",
					machine().describe_context(),
					m_ctrl_cmd,
					m_ctrl_data);
		}
	}
}


u8 huc3_device::read_ir(address_space &space)
{
	LOG("%s: Infrared read\n", machine().describe_context());
	return (space.unmap() & 0xc0) | 0x00; // least significant bit clear - dark
}


void huc3_device::write_ir(u8 data)
{
	// bit zero high to turn on the IR LED, or low to turn it off
	LOG("%s: Infrared write 0x%02X\n", machine().describe_context(), data);
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_HUC3, device_gb_cart_interface, bus::gameboy::huc3_device, "gb_rom_huc3", "Game Boy Hudson Soft HuC-3 Cartridge")
