// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    TS-Configuration Copper

    Mirrors pentevo/fpga/current/common/copper.v:
      * F5nn SIGNAL: bit0 -> CPU INT (IM2 0xF7, INT_MASK.4),
                     bit1 -> RDY flag (bit7 of V_CONFIG read, cleared on read)
      * WREG: TS reg write; CPU access has priority over PC advance in HW
      * JUMP FF = RET; JUMP to self = busy loop (halt)
**********************************************************************/

#include "emu.h"
#include "tsconf_copper.h"
#include "tsconf_copper_dasm.h"


#define LOG_CTRL  (1U << 1)
#define LOG_DATA  (1U << 2)

//#define VERBOSE ( LOG_GENERAL /*| LOG_CTRL | LOG_DATA*/ )
#include "logmacro.h"

#define LOGCTRL(...) LOGMASKED(LOG_CTRL, __VA_ARGS__)
#define LOGDATA(...) LOGMASKED(LOG_DATA, __VA_ARGS__)


// device type definition
DEFINE_DEVICE_TYPE(TSCONF_COPPER, tsconf_copper_device, "tsconf_copper", "TS-Configuration Copper")


tsconf_copper_device::tsconf_copper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TSCONF_COPPER, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_disasm_interface(mconfig, *this)
	, device_state_interface(mconfig, *this)
	, m_space_config("program", ENDIANNESS_LITTLE, 16, 8, -1, address_map_constructor(FUNC(tsconf_copper_device::copper_map), this))
	, m_cl_data(*this, "cl_data", 0x200, ENDIANNESS_LITTLE)
	, m_out_wreg_cb(*this)
	, m_out_int_cb(*this)
	, m_out_rdy_cb(*this)
	, m_in_until_pos_cb(*this)
{
}

void tsconf_copper_device::copper_en_w(u8 data)
{
	m_en = data != 0xff;
	m_pc = data;

	if (m_en)
	{
		LOGCTRL("START\n");
		m_timer->adjust(attotime::zero);
	}
	else
	{
		LOGCTRL("STOP\n");
		m_timer->reset();
	}
}

void tsconf_copper_device::cl_data_w(u16 addr, u8 data)
{
	addr &= 0x1ff;
	LOGDATA("data(W) %02x %04x\n", addr, data);
	m_cl_data[addr >> 1] = (addr & 1)
		? (m_cl_data[addr >> 1] & 0x00ff) | (data << 8)
		: (m_cl_data[addr >> 1] & 0xff00) | data;
}

void tsconf_copper_device::dma_ready_w(int status)
{
	if(m_en && (status & 1) && (m_cl_data[m_pc] & 0xffff) == 0b1111'1011'1111'0001) // WAIT DMA
	{
		++m_pc;
		m_timer->adjust(attotime::from_hz(clock()), 0);
	}
}

TIMER_CALLBACK_MEMBER(tsconf_copper_device::timer_callback)
{
	const u16 data = m_cl_data[m_pc];
	if (BIT(data, 12, 4) == 0b1111) // all instructions except WREG
	{
		switch (BIT(data, 8, 4))
		{
			case 0b0110: //SETA
				m_a = data & 0xff;
				++m_pc;
				break;

			case 0b1000: //SETB
			case 0b1001:
				m_b = data & 0x1ff;
				++m_pc;
				break;

			case 0b0111: // WAITX
				if (param == 0b1111'0111)
					++m_pc;
				else
				{
					const u8 x = data & 0xff;
					if (x < 224)
						m_timer->adjust(m_in_until_pos_cb(0x8000 | x), 0b1111'0111);
					else
						m_timer->reset();
					return;
				}
				break;

			case 0b1011: // WAITY
				if (BIT(data, 4, 4) == 0b1111) // WAIT event
				{
					const u8 event = data & 0x0f;
					if (event == param)
						++m_pc;
					else
					{
						switch (event)
						{
							case 1: // DMA
								m_timer->reset();
								return;
								break;
							case 2: // frame
								m_timer->adjust(m_in_until_pos_cb(0b00 << 14), 2);
								return;
								break;
							case 3: // line start
								m_timer->adjust(m_in_until_pos_cb(0b11 << 14), 3);
								return;
								break;
							default:
								++m_pc;
								break;

						}
					}
					break;
				}
				[[fallthrough]];
			case 0b1010:
				if (param == 0b1111'1010)
					++m_pc;
				else
				{
					const u16 y = data & 0x1ff;
					if (y < 320)
						m_timer->adjust(m_in_until_pos_cb(0x4000 | y), 0b1111'1010);
					else
						m_timer->reset();
					return;
				}
				break;

			case 0b1100: // DJNZA
				--m_a;
				m_pc = m_a ? data & 0xff : (m_pc + 1);
				break;

			case 0b1101: // DJNZB
				m_b = (m_b - 1) & 0x1ff;
				m_pc = m_b ? data & 0xff : (m_pc + 1);
				break;

			case 0b1110: // CALL
				m_ret_pc = m_pc + 1;
				m_pc = data & 0xff;
				break;

			case 0b1111: // JUMP
				{
					const u8 to = data & 0xff;
					if (to != 0xff && to == m_pc) // JUMP to self: busy loop
					{
						m_en = false;
						m_timer->reset();
						return;
					}

					m_pc = (to == 0xff) ? m_ret_pc : to; // FF = RET
				}
				break;

			case 0b0101: // SIGNAL
				m_out_int_cb(BIT(data, 0));
				m_out_rdy_cb(BIT(data, 1));
				++m_pc;
				break;

			default: // reserved instructions
				++m_pc;
				break;
		}
	}
	else
	{
		m_out_wreg_cb((data & 0xff00) | 0xaf, data & 0x00ff);
		++m_pc;
	}

	m_timer->adjust(attotime::from_hz(clock()), 0);
}

device_memory_interface::space_config_vector tsconf_copper_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config)
	};
}

std::unique_ptr<util::disasm_interface> tsconf_copper_device::create_disassembler()
{
	return std::make_unique<tsconf_copper_disassembler>();
}

void tsconf_copper_device::copper_map(address_map &map)
{
	map(0x000, 0x0ff).ram().share(m_cl_data);
}

void tsconf_copper_device::device_start()
{
	m_timer = timer_alloc(FUNC(tsconf_copper_device::timer_callback), this);

	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow(); // disassembly view cursor

	m_in_until_pos_cb.resolve_safe(attotime::zero);

	save_item(NAME(m_en));
	save_item(NAME(m_pc));
	save_item(NAME(m_ret_pc));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
}

void tsconf_copper_device::device_reset()
{
	m_timer->reset();

	m_en = false;
	m_pc = 0;
	m_ret_pc = 0;
	m_a = 0;
	m_b = 0;
}
