// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
// thanks-to:Mike Miller
/**********************************************************************

   SPECIFICATION
       Processor: 6809 (1MHz)
             RAM: upto 256K on board, and 512K with expansion unit.
             ROM: upto 128K
     Text screen: teletext-type 40 x 24, 8 colours.
  Graphic screen: 240 x 240 pixels, 8 colours.
        Keyboard: 88 keys in unit with CPU, numeric/cursor pad. 10 function
                  keys.
      Interfaces: 4 x RS232, IEEE-488, cassette

    TODO:
    - fuse timer for switching tasks needs some attention.
    - nothing is known so emulation is very incomplete.

**********************************************************************/


#include "emu.h"

#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/bankdev.h"
#include "machine/clock.h"
#include "machine/ds75160a.h"
#include "machine/input_merger.h"
#include "machine/ram.h"
#include "machine/tms9914.h"
#include "video/saa5050.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "utf8.h"


// Debugging
#define VERBOSE 0
#include "logmacro.h"


namespace {

class positron_state : public driver_device
{
public:
	positron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mmu_bankdev(*this, "mmu")
		, m_ram(*this, RAM_TAG)
		, m_keyboard(*this, "COL%u", 0)
		, m_hires_ram(*this, "hires_ram")
		, m_lores_ram(*this, "lores_ram")
		, m_cassette(*this, "cassette")
	{ }

	void positron(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(fuse_update); // TODO: Does nothing

	// disassembly override
	offs_t os9_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

	emu_timer *m_fuse_timer = nullptr;
	bool m_fuse_timer_running = false;

	void positron_map(address_map &map) ATTR_COLD;
	void positron_fetch(address_map &map) ATTR_COLD;
	void mmu_map(address_map &map) ATTR_COLD;

	uint8_t mmu_r(offs_t offset);
	void mmu_w(offs_t offset, uint8_t data);

	// sockets for upto 8 x SC67476, only 2 actually fitted in this machine
	struct mmu {
		uint16_t access_reg[1024]{};
		uint8_t key_value[8]{};
		uint8_t access_key = 0;
		uint8_t operate_key = 0;
		uint8_t active_key = 0;
		bool sbit;
	} m_mmu;

	uint8_t opcode_r(offs_t offset);
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);

	bool m_irq_ack = 0;

	required_device<mc6809_device> m_maincpu;
	required_device<address_map_bank_device> m_mmu_bankdev;
	required_device<ram_device> m_ram;
	required_ioport_array<14> m_keyboard;
	required_shared_ptr<uint8_t> m_hires_ram;
	required_shared_ptr<uint8_t> m_lores_ram;
	required_device<cassette_image_device> m_cassette;

	memory_passthrough_handler m_mmu_shadow_tap;

	uint8_t m_prev_opcode = 0;
};


void positron_state::machine_start()
{
	std::fill(std::begin(m_mmu.access_reg), std::end(m_mmu.access_reg), 0x3ff);
	m_mmu.sbit = true;

	// select task 0
	m_mmu.active_key = 0;

	m_fuse_timer = timer_alloc(FUNC(positron_state::fuse_update), this);
	m_fuse_timer->adjust(attotime::never);
	m_fuse_timer_running = false;
	m_irq_ack = false;

	save_item(STRUCT_MEMBER(m_mmu, access_reg));
	save_item(STRUCT_MEMBER(m_mmu, key_value));
	save_item(STRUCT_MEMBER(m_mmu, access_key));
	save_item(STRUCT_MEMBER(m_mmu, operate_key));
	save_item(STRUCT_MEMBER(m_mmu, active_key));
	save_item(STRUCT_MEMBER(m_mmu, sbit));
}


void positron_state::machine_reset()
{
	std::fill(std::begin(m_mmu.access_reg), std::end(m_mmu.access_reg), 0x3ff);
	std::fill(std::begin(m_mmu.key_value), std::end(m_mmu.key_value), 0x00);
	m_mmu.access_key = 0x00;
	m_mmu.operate_key = 0x00;
	m_mmu.sbit = true;

	// select task 0
	m_mmu.active_key = 0;

	address_space &program = m_maincpu->space(AS_PROGRAM);
	m_mmu_shadow_tap.remove();
	m_mmu_shadow_tap = program.install_read_tap(
			0x0000, 0xffff,
			"mmu_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					if (m_fuse_timer_running && m_prev_opcode == 0x3b) // RTI
					{
						m_mmu.active_key = m_mmu.operate_key;
						logerror("mmu_shadow_r: switched to task %d\n", m_mmu.active_key);
						m_mmu.sbit = false;
						m_fuse_timer_running = false;
					}
					else if (m_irq_ack && offset >= 0xfff0 && offset != 0xffff)
					{
						m_mmu.active_key = 0;
						logerror("irq_callback: switched to task %d\n", m_mmu.active_key);
						m_mmu.sbit = true;
						m_irq_ack = false;
						data = m_maincpu->space(AS_PROGRAM).read_byte(offset);
					}
				}
			},
			&m_mmu_shadow_tap);
}


TIMER_CALLBACK_MEMBER(positron_state::fuse_update)
{
	//m_mmu.active_key = m_mmu.operate_key;
	//m_mmu.sbit = false;
	m_fuse_timer->adjust(attotime::never);
}

//-------------------------------------------------
//  ADDRESS_MAP( positron_map )
//-------------------------------------------------

void positron_state::positron_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(positron_state::mmu_r), FUNC(positron_state::mmu_w));
}

void positron_state::positron_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(positron_state::opcode_r));
}

void positron_state::mmu_map(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x00000, 0x3ffff).rw(FUNC(positron_state::ram_r), FUNC(positron_state::ram_w));
	map(0x40000, 0x7ffff).noprw(); // Expansion board RAM
	map(0x80000, 0x81fff).ram().share("lores_ram");
	//map(0x82000, 0x82000)
	map(0x83000, 0x8300d).lr8([this](offs_t offset) { return m_keyboard[offset]->read(); }, "keyboard_r");
	//map(0x84000, 0x84003)
	map(0x90000, 0x97fff).ram().share("hires_ram");
	map(0xa0000, 0xa0007).rw("timer", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xa2000, 0xa2000).portr("DSW1");
	map(0xa2001, 0xa2001).portr("DSW2");
	map(0xa2002, 0xa2002).portr("DSW3");
	map(0xa2003, 0xa2003).portr("DSW4");
	map(0xa3000, 0xa3001).rw("acia0", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xa4000, 0xa4001).rw("acia1", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xa4002, 0xa4002).lw8([this](uint8_t data) { LOG("acia1_clock_w: %02x\n", data); }, "acia1_clock_w");
	map(0xa5000, 0xa5001).rw("acia2", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xa5002, 0xa5002).lw8([this](uint8_t data) { LOG("acia2_clock_w: %02x\n", data); }, "acia2_clock_w");
	map(0xa6000, 0xa6001).rw("acia3", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xa6002, 0xa6002).lw8([this](uint8_t data) { LOG("acia3_clock_w: %02x\n", data); }, "acia3_clock_w");
	map(0xa7000, 0xa7001).rw("acia4", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xa7002, 0xa7002).lw8([this](uint8_t data) { LOG("acia4_clock_w: %02x\n", data); }, "acia4_clock_w");
	map(0xa8000, 0xa8007).rw("hpib", FUNC(tms9914_device::read), FUNC(tms9914_device::write));
	map(0xe0000, 0xfffff).rom().region("maincpu", 0);
}


uint8_t positron_state::opcode_r(offs_t offset)
{
	uint8_t data = m_maincpu->space(AS_PROGRAM).read_byte(offset);

	if (m_fuse_timer_running && !machine().side_effects_disabled())
	{
		logerror("opcode_r: %04x = %02x\n", offset, data);
	}

	m_prev_opcode = data;

	return data;
}


uint8_t positron_state::mmu_r(offs_t offset)
{
	uint8_t data = 0x00;

	if (offset >= 0xff80 && offset < 0xffcc && m_mmu.active_key == 0x00)
	{
		switch (offset & 0x40)
		{
		case 0x00:
		{
			uint16_t task_reg = (m_mmu.access_key << 5) | ((offset >> 1) & 0x1f);
			if (offset & 1)
				data = m_mmu.access_reg[task_reg] & 0xff;
			else
				data = m_mmu.access_reg[task_reg] >> 8;
			break;
		}
		case 0x40:
			switch (offset & 0x0f)
			{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
				data = m_mmu.key_value[offset & 7];
				if (!machine().side_effects_disabled())
					LOG("(%04X) mmu_r: key %d = %02x\n", m_maincpu->pc(), offset & 7, data);
				break;
			case 0x08:
				data = m_mmu.sbit ? 1 : 0;
				if (!machine().side_effects_disabled())
					LOG("(%04X) mmu_r: sbit = %d\n", m_maincpu->pc(), data);
				break;
			case 0x0a:
				data = m_mmu.access_key;
				if (!machine().side_effects_disabled())
					LOG("(%04X) mmu_r: access_key = %02x\n", m_maincpu->pc(), data);
				break;
			case 0x0b:
				data = m_mmu.operate_key;
				if (!machine().side_effects_disabled())
					LOG("(%04X) mmu_r: operate_key = %02x\n", m_maincpu->pc(), data);
				break;
			}
			break;
		}
	}
	else
	{
		uint16_t task_reg = (m_mmu.active_key << 5) | ((offset >> 11) & 0x1f);
		offs_t mmu_addr = (m_mmu.access_reg[task_reg] << 11) | (offset & 0x7ff);
		data = m_mmu_bankdev->read8(mmu_addr);
	}

	return data;
}

void positron_state::mmu_w(offs_t offset, uint8_t data)
{
	if (offset >= 0xff80 && offset < 0xffcc && m_mmu.active_key == 0x00 && m_mmu.sbit)
	{
		switch (offset & 0x40)
		{
		case 0x00:
		{
			uint16_t task_reg = (m_mmu.access_key << 5) | ((offset >> 1) & 0x1f);
			if (offset & 1)
				m_mmu.access_reg[task_reg] = (m_mmu.access_reg[task_reg] & 0x300) | data;
			else
				m_mmu.access_reg[task_reg] = (m_mmu.access_reg[task_reg] & 0x0ff) | (data << 8);
			break;
		}
		case 0x40:
			switch (offset & 0x0f)
			{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
				m_mmu.key_value[offset & 7] = data & 0x07;
				LOG("(%04X) mmu_w: key %d = %02x\n", m_maincpu->pc(), offset & 7, data);
				break;
			case 0x09:
				LOG("(%04X) mmu_w: fuse = %02x\n", m_maincpu->pc(), data);
				if (data == 0x00)
				{
					m_mmu.active_key = m_mmu.operate_key;
					logerror("mmu_w: switched to task %d\n", m_mmu.active_key);
					m_mmu.sbit = false;
					m_fuse_timer_running = false;
				}
				else
				{
					//m_fuse_timer->adjust(attotime::from_ticks(data & 0x07, 24_MHz_XTAL / 24));
					m_fuse_timer_running = true;
				}
				break;
			case 0x0a:
				m_mmu.access_key = data & 0x1f;
				LOG("(%04X) mmu_w: access_key = %02x\n", m_maincpu->pc(), data);
				break;
			case 0x0b:
				m_mmu.operate_key = data & 0x1f;
				LOG("(%04X) mmu_w: operate_key = %02x\n", m_maincpu->pc(), data);
				break;
			}
			break;
		}
	}
	else
	{
		uint16_t task_reg = (m_mmu.active_key << 5) | ((offset >> 11) & 0x1f);
		offs_t mmu_addr = (m_mmu.access_reg[task_reg] << 11) | (offset & 0x7ff);
		m_mmu_bankdev->write8(mmu_addr, data);
	}
}


uint8_t positron_state::ram_r(offs_t offset)
{
	if (offset < m_ram->size())
		return m_ram->pointer()[offset];
	else
		return 0xff;
}

void positron_state::ram_w(offs_t offset, uint8_t data)
{
	if (offset < m_ram->size())
	{
		m_ram->pointer()[offset] = data;
	}
}


static INPUT_PORTS_START(positron)
	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RETURN")             PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REPEAT")             PORT_CODE(KEYCODE_INSERT)       PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESC")                PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONTROL")            PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT")              PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT")              PORT_CODE(KEYCODE_RSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT LOCK")         PORT_CODE(KEYCODE_LALT)         PORT_CHAR(UCHAR_MAMEKEY(LALT))      PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAPS LOCK")          PORT_CODE(KEYCODE_CAPSLOCK)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  PORT_TOGGLE

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)           PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)            PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)            PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)              PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL")                PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_0)            PORT_CHAR('0')      PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_1)            PORT_CHAR('1')      PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_2)            PORT_CHAR('2')      PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_3)            PORT_CHAR('3')      PORT_CHAR(0xA3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_4)            PORT_CHAR('4')      PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_5)            PORT_CHAR('5')      PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_6)            PORT_CHAR('6')      PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_7)            PORT_CHAR('7')      PORT_CHAR('\'')

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_8)            PORT_CHAR('8')      PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_9)            PORT_CHAR('9')      PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR(':')      PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';')      PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',')      PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('=')      PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.')      PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/')      PORT_CHAR('?')

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('@')      PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_A)            PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_B)            PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_C)            PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_D)            PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_E)            PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_F)            PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_G)            PORT_CHAR('G')

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_H)            PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_I)            PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_J)            PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_K)            PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_L)            PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_M)            PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_N)            PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_O)            PORT_CHAR('O')

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_P)            PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_Q)            PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_R)            PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_S)            PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_T)            PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_U)            PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_V)            PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_W)            PORT_CHAR('W')

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_X)            PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_Y)            PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_Z)            PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR(0x2190)   PORT_CHAR(0xbc)    // ←  ¼
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR(0xbd)     PORT_CHAR(0x2016)  // ½  ‖
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(0x2192)   PORT_CHAR(0xbe)    // →  ¾
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('^')      PORT_CHAR(0xf7)    // ÷
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPACE")              PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')

	PORT_START("COL8") // Video Attributes
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Red Alpha Gr")       PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Green Alpha Gr")     PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Yellow Alpha Gr")    PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Blue Alpha Gr")      PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Magenta Alpha Gr")   PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cyan Alpha Gr")      PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("White Alpha Gr")     PORT_CODE(KEYCODE_F7)

	PORT_START("COL9") // Video Attributes
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FC SC")              PORT_CODE(KEYCODE_F8)  // Flashing/Steady
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DH NH")              PORT_CODE(KEYCODE_F9)  // Double/Normal Height
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SGR CGR")            PORT_CODE(KEYCODE_F10) // Separated/Contiguous Graphics
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NB BB")              PORT_CODE(KEYCODE_F11) // New/Black Background
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HGR RGR")            PORT_CODE(KEYCODE_F12) // Hold/Release Graphics
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL10") // Graphics Encoding
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_TOGGLE // unlabelled Viewdata 2x3 matrix toggle
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_TOGGLE // unlabelled Viewdata 2x3 matrix toggle
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_TOGGLE // unlabelled Viewdata 2x3 matrix toggle
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_TOGGLE // unlabelled Viewdata 2x3 matrix toggle
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_TOGGLE // unlabelled Viewdata 2x3 matrix toggle
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_TOGGLE // unlabelled Viewdata 2x3 matrix toggle
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REP")                PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENC")                PORT_CODE(KEYCODE_PGDN)

	PORT_START("COL11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 0")           PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 1")           PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 2")           PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 3")           PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 4")           PORT_CODE(KEYCODE_4_PAD)        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 5")           PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 6")           PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 7")           PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("COL12")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 8")           PORT_CODE(KEYCODE_8_PAD)        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 9")           PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad *")           PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad #")           PORT_CODE(KEYCODE_NUMLOCK)      PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL13") // same as COL1?
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) // right
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED) // left
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) // down
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) // up
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // del
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DSW0") //IEEE-488
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW0:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW0:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("DSW1") // RS232 75-4800 baud
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("DSW2") // RS232 75-4800 baud
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("DSW3") // RS232 75-4800 baud
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("DSW4") // RS232 75-4800 baud
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("DSW5") // Video display mode
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
INPUT_PORTS_END


/***************************************************************************
  DISASSEMBLY OVERRIDE (OS9 syscalls)
 ***************************************************************************/

static const char *const os9syscalls[] =
{
	"F$Link",          // Link to Module
	"F$Load",          // Load Module from File
	"F$UnLink",        // Unlink Module
	"F$Fork",          // Start New Process
	"F$Wait",          // Wait for Child Process to Die
	"F$Chain",         // Chain Process to New Module
	"F$Exit",          // Terminate Process
	"F$Mem",           // Set Memory Size
	"F$Send",          // Send Signal to Process
	"F$Icpt",          // Set Signal Intercept
	"F$Sleep",         // Suspend Process
	"F$SSpd",          // Suspend Process
	"F$ID",            // Return Process ID
	"F$SPrior",        // Set Process Priority
	"F$SSWI",          // Set Software Interrupt
	"F$PErr",          // Print Error
	"F$PrsNam",        // Parse Pathlist Name
	"F$CmpNam",        // Compare Two Names
	"F$SchBit",        // Search Bit Map
	"F$AllBit",        // Allocate in Bit Map
	"F$DelBit",        // Deallocate in Bit Map
	"F$Time",          // Get Current Time
	"F$STime",         // Set Current Time
	"F$CRC",           // Generate CRC
	"F$GPrDsc",        // get Process Descriptor copy
	"F$GBlkMp",        // get System Block Map copy
	"F$GModDr",        // get Module Directory copy
	"F$CpyMem",        // Copy External Memory
	"F$SUser",         // Set User ID number
	"F$UnLoad",        // Unlink Module by name
	"F$Alarm",         // Color Computer Alarm Call (system wide)
	nullptr,
	nullptr,
	"F$NMLink",        // Color Computer NonMapping Link
	"F$NMLoad",        // Color Computer NonMapping Load
	nullptr,
	nullptr,
	"F$TPS",           // Return System's Ticks Per Second
	"F$TimAlm",        // COCO individual process alarm call
	"F$VIRQ",          // Install/Delete Virtual IRQ
	"F$SRqMem",        // System Memory Request
	"F$SRtMem",        // System Memory Return
	"F$IRQ",           // Enter IRQ Polling Table
	"F$IOQu",          // Enter I/O Queue
	"F$AProc",         // Enter Active Process Queue
	"F$NProc",         // Start Next Process
	"F$VModul",        // Validate Module
	"F$Find64",        // Find Process/Path Descriptor
	"F$All64",         // Allocate Process/Path Descriptor
	"F$Ret64",         // Return Process/Path Descriptor
	"F$SSvc",          // Service Request Table Initialization
	"F$IODel",         // Delete I/O Module
	"F$SLink",         // System Link
	"F$Boot",          // Bootstrap System
	"F$BtMem",         // Bootstrap Memory Request
	"F$GProcP",        // Get Process ptr
	"F$Move",          // Move Data (low bound first)
	"F$AllRAM",        // Allocate RAM blocks
	"F$AllImg",        // Allocate Image RAM blocks
	"F$DelImg",        // Deallocate Image RAM blocks
	"F$SetImg",        // Set Process DAT Image
	"F$FreeLB",        // Get Free Low Block
	"F$FreeHB",        // Get Free High Block
	"F$AllTsk",        // Allocate Process Task number
	"F$DelTsk",        // Deallocate Process Task number
	"F$SetTsk",        // Set Process Task DAT registers
	"F$ResTsk",        // Reserve Task number
	"F$RelTsk",        // Release Task number
	"F$DATLog",        // Convert DAT Block/Offset to Logical
	"F$DATTmp",        // Make temporary DAT image (Obsolete)
	"F$LDAXY",         // Load A [X,[Y]]
	"F$LDAXYP",        // Load A [X+,[Y]]
	"F$LDDDXY",        // Load D [D+X,[Y]]
	"F$LDABX",         // Load A from 0,X in task B
	"F$STABX",         // Store A at 0,X in task B
	"F$AllPrc",        // Allocate Process Descriptor
	"F$DelPrc",        // Deallocate Process Descriptor
	"F$ELink",         // Link using Module Directory Entry
	"F$FModul",        // Find Module Directory Entry
	"F$MapBlk",        // Map Specific Block
	"F$ClrBlk",        // Clear Specific Block
	"F$DelRAM",        // Deallocate RAM blocks
	"F$GCMDir",        // Pack module directory
	"F$AlHRam",        // Allocate HIGH RAM Blocks
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"F$RegDmp",        // Ron Lammardo's debugging register dump call
	"F$NVRAM",         // Non Volatile RAM (RTC battery backed static) read/write
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"I$Attach",        // Attach I/O Device
	"I$Detach",        // Detach I/O Device
	"I$Dup",           // Duplicate Path
	"I$Create",        // Create New File
	"I$Open",          // Open Existing File
	"I$MakDir",        // Make Directory File
	"I$ChgDir",        // Change Default Directory
	"I$Delete",        // Delete File
	"I$Seek",          // Change Current Position
	"I$Read",          // Read Data
	"I$Write",         // Write Data
	"I$ReadLn",        // Read Line of ASCII Data
	"I$WritLn",        // Write Line of ASCII Data
	"I$GetStt",        // Get Path Status
	"I$SetStt",        // Set Path Status
	"I$Close",         // Close Path
	"I$DeletX"         // Delete from current exec dir
};


//-------------------------------------------------
//  os9_dasm_override
//-------------------------------------------------

offs_t positron_state::os9_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	unsigned call;
	offs_t result = 0;

	// Microware OS-9 and a number of other 6x09 based systems used the SWI2
	// instruction for syscalls.  This checks for a SWI2 and looks up the syscall as appropriate
	if ((opcodes.r8(pc) == 0x10) && (opcodes.r8(pc+1) == 0x3F))
	{
		call = opcodes.r8(pc+2);
		if ((call < std::size(os9syscalls)) && (os9syscalls[call] != nullptr))
		{
			util::stream_format(stream, "OS9   %s", os9syscalls[call]);
			result = 3;
		}
	}
	return result;
}


void positron_state::positron(machine_config &config)
{
	MC6809(config, m_maincpu, 24_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &positron_state::positron_map);
	m_maincpu->set_addrmap(AS_OPCODES, &positron_state::positron_fetch);
	m_maincpu->set_dasm_override(FUNC(positron_state::os9_dasm_override));
	m_maincpu->set_irq_acknowledge_callback(NAME([this](device_t&, int) -> int { m_irq_ack = true; return 0; }));

	ADDRESS_MAP_BANK(config, m_mmu_bankdev); // SC67476
	m_mmu_bankdev->set_addrmap(0, &positron_state::mmu_map);
	m_mmu_bankdev->set_data_width(8);
	m_mmu_bankdev->set_addr_width(20);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	RAM(config, m_ram).set_default_size("192K").set_extra_options("64K,128K,256K");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(17.73447_MHz_XTAL*2, 1135, 0, 960, 625, 0, 480); // clk doubled for interlace
	screen.set_screen_update("saa5050", FUNC(saa5050_device::screen_update));

	saa5050_device &saa5050(SAA5050(config, "saa5050", 24_MHz_XTAL / 4));
	saa5050.d_cb().set([this](offs_t offset) { return m_lores_ram[offset & 0x1fff]; });
	saa5050.set_screen_size(80, 24, 256);

	PALETTE(config, "palette", palette_device::RGB_3BIT);

	ptm6840_device &timer(PTM6840(config, "timer", 24_MHz_XTAL / 24));
	//timer.o3_callback().set("acia0", FUNC(acia6850_device::write_txc));
	//timer.o3_callback().append("acia0", FUNC(acia6850_device::write_rxc));
	timer.irq_callback().set("irqs", FUNC(input_merger_device::in_w<6>));

	acia6850_device &acia0(ACIA6850(config, "acia0"));
	//m_acia[0]->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	//m_acia[0]->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	acia0.irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	acia6850_device &acia1(ACIA6850(config, "acia1"));
	acia1.txd_handler().set("serial1", FUNC(rs232_port_device::write_txd));
	acia1.rts_handler().set("serial1", FUNC(rs232_port_device::write_rts));
	acia1.irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));

	clock_device &acia1_clock(CLOCK(config, "acia1_clock", 4800));
	acia1_clock.signal_handler().set("acia1", FUNC(acia6850_device::write_txc));
	acia1_clock.signal_handler().append("acia1", FUNC(acia6850_device::write_rxc));

	rs232_port_device &serial1(RS232_PORT(config, "serial1", default_rs232_devices, nullptr));
	serial1.rxd_handler().set("acia1", FUNC(acia6850_device::write_rxd));
	serial1.cts_handler().set("acia1", FUNC(acia6850_device::write_cts));

	acia6850_device &acia2(ACIA6850(config, "acia2"));
	acia2.txd_handler().set("serial2", FUNC(rs232_port_device::write_txd));
	acia2.rts_handler().set("serial2", FUNC(rs232_port_device::write_rts));
	acia2.irq_handler().set("irqs", FUNC(input_merger_device::in_w<2>));

	clock_device &acia2_clock(CLOCK(config, "acia2_clock", 4800));
	acia2_clock.signal_handler().set("acia2", FUNC(acia6850_device::write_txc));
	acia2_clock.signal_handler().append("acia2", FUNC(acia6850_device::write_rxc));

	rs232_port_device &serial2(RS232_PORT(config, "serial2", default_rs232_devices, nullptr));
	serial2.rxd_handler().set("acia2", FUNC(acia6850_device::write_rxd));
	serial2.cts_handler().set("acia2", FUNC(acia6850_device::write_cts));

	acia6850_device &acia3(ACIA6850(config, "acia3"));
	acia3.txd_handler().set("serial3", FUNC(rs232_port_device::write_txd));
	acia3.rts_handler().set("serial3", FUNC(rs232_port_device::write_rts));
	acia3.irq_handler().set("irqs", FUNC(input_merger_device::in_w<3>));

	clock_device &acia3_clock(CLOCK(config, "acia3_clock", 4800));
	acia3_clock.signal_handler().set("acia3", FUNC(acia6850_device::write_txc));
	acia3_clock.signal_handler().append("acia3", FUNC(acia6850_device::write_rxc));

	rs232_port_device &serial3(RS232_PORT(config, "serial3", default_rs232_devices, nullptr));
	serial3.rxd_handler().set("acia3", FUNC(acia6850_device::write_rxd));
	serial3.cts_handler().set("acia3", FUNC(acia6850_device::write_cts));

	acia6850_device &acia4(ACIA6850(config, "acia4"));
	acia4.txd_handler().set("serial4", FUNC(rs232_port_device::write_txd));
	acia4.rts_handler().set("serial4", FUNC(rs232_port_device::write_rts));
	acia4.irq_handler().set("irqs", FUNC(input_merger_device::in_w<4>));

	clock_device &acia4_clock(CLOCK(config, "acia4_clock", 4800));
	acia4_clock.signal_handler().set("acia4", FUNC(acia6850_device::write_txc));
	acia4_clock.signal_handler().append("acia4", FUNC(acia6850_device::write_rxc));

	rs232_port_device &serial4(RS232_PORT(config, "serial4", default_rs232_devices, nullptr));
	serial4.rxd_handler().set("acia4", FUNC(acia6850_device::write_rxd));
	serial4.cts_handler().set("acia4", FUNC(acia6850_device::write_cts));

	tms9914_device &hpib(TMS9914(config, "hpib", 16_MHz_XTAL / 4));
	hpib.int_write_cb().set("irqs", FUNC(input_merger_device::in_w<5>));
	hpib.dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	hpib.dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	hpib.eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	hpib.dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	hpib.nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	hpib.ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	hpib.ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	hpib.srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	hpib.atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	hpib.ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));

	//DS75160A(config, m_ieee1, 0);
	//m_ieee1->read_callback().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	//m_ieee1->write_callback().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));

	ieee488_device &ieee(IEEE488(config, IEEE488_TAG));
	ieee.eoi_callback().set("hpib", FUNC(tms9914_device::eoi_w));
	ieee.dav_callback().set("hpib", FUNC(tms9914_device::dav_w));
	ieee.nrfd_callback().set("hpib", FUNC(tms9914_device::nrfd_w));
	ieee.ndac_callback().set("hpib", FUNC(tms9914_device::ndac_w));
	ieee.ifc_callback().set("hpib", FUNC(tms9914_device::ifc_w));
	ieee.srq_callback().set("hpib", FUNC(tms9914_device::srq_w));
	ieee.atn_callback().set("hpib", FUNC(tms9914_device::atn_w));
	ieee.ren_callback().set("hpib", FUNC(tms9914_device::ren_w));
	IEEE488_SLOT(config, "ieee_dev", 0, cbm_ieee488_devices, nullptr);
}


ROM_START(positron)
	ROM_REGION(0x20000, "maincpu", ROMREGION_ERASE00) // supports upto 128K ROM
	ROM_LOAD("os9_5_0007.bin", 0x16000, 0x2000, CRC(f6a24c1d) SHA1(443de539deeec5785872b295dab8d93acd60d19c))
	ROM_LOAD("os9_4_0007.bin", 0x18000, 0x2000, CRC(19f73846) SHA1(525aa101b34f03a5c312ec5aad7c1b9062fe9284))
	ROM_LOAD("os9_3_0007.bin", 0x1a000, 0x2000, CRC(62d47630) SHA1(3719f4fb8ebc8cc64c37702f34532f76178a94d7))
	ROM_LOAD("os9_2_0007.bin", 0x1c000, 0x2000, CRC(63380de9) SHA1(53a58f7c60ebbbc294d94c406ec0fc6c5cb0fb7b))
	ROM_LOAD("os9_1_0007.bin", 0x1e000, 0x2000, CRC(63f0fb57) SHA1(7c50c0cbc7c0dbaf8da186ccb474d263f71416ea))
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT   MACHINE   INPUT     CLASS           INIT        COMPANY                   FULLNAME         FLAGS
COMP( 1982, positron, 0,      0,       positron, positron, positron_state, empty_init, "Positron Computers Ltd", "Positron 9000", MACHINE_NOT_WORKING )
