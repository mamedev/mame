// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

TODO:
- Only upd1771c supported
- Noise, time, and external interrupts not implemented
- CH1/CH2 not implemented
- Not all instructions implemented

*/

#include "emu.h"
#include "upd177x.h"
#include "upd177xd.h"

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(UPD1771C, upd1771c_cpu_device, "upd1771c", "NEC uPD1771C")


enum
{
	UPD177X_PC, UPD177X_SP,
	UPD177X_A, UPD177X_A_SAVE, UPD177X_H, UPD177X_N, UPD177X_X, UPD177X_Y,
	UPD177X_TIMER, UPD177X_PNC1, UPD177X_PNC2,
	UPD177X_PA, UPD177X_PB, UPD177X_MD0, UPD177X_MD1, UPD177X_SKIP, UPD177X_SKIP_SAVE
};


static constexpr u8 INT_TONE1 = 1 << 0;
static constexpr u8 INT_TONE2 = 1 << 1;
static constexpr u8 INT_TONE3 = 1 << 2;
static constexpr u8 INT_TONE4 = 1 << 3;
static constexpr u8 INT_NOISE = 1 << 4;
static constexpr u8 INT_EXT = 1 << 5;
static constexpr u8 INT_TIME = 1 << 6;

static constexpr u8 MD0_3264_BIT = 0;
static constexpr u8 MD0_TN_BIT = 1;
static constexpr u8 MD0_NS_BIT = 2;
static constexpr u8 MD0_NSF1_BIT = 3;
static constexpr u8 MD0_NSF2_BIT = 4;
static constexpr u8 MD0_OUT_BIT = 5;
static constexpr u8 MD0_IF_BIT = 6;
static constexpr u8 MD1_NSF3_BIT = 0;
static constexpr u8 MD1_TIME_BIT = 1;
static constexpr u8 MD1_EXT_BIT = 2;

static constexpr u16 VECTOR_TONE1 = 0x20;
static constexpr u16 VECTOR_TONE2 = 0x24;
static constexpr u16 VECTOR_TONE3 = 0x28;
static constexpr u16 VECTOR_TONE4 = 0x2c;
static constexpr u16 VECTOR_NOISE = 0x48;
static constexpr u16 VECTOR_EXT   = 0x60;
static constexpr u16 VECTOR_TIME  = 0x80;

static constexpr u8 STACK_START = 0x20;


upd177x_cpu_device::upd177x_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 16, -1, address_map_constructor(FUNC(upd177x_cpu_device::program_map), this))
	, m_pb_out_cb(*this)
{ }


upd1771c_cpu_device::upd1771c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd177x_cpu_device(mconfig, UPD1771C, tag, owner, clock)
{ }


device_memory_interface::space_config_vector upd177x_cpu_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


void upd177x_cpu_device::program_map(address_map &map)
{
	map(0, 0x1ff).rom();
}


void upd177x_cpu_device::device_start()
{
	space(AS_PROGRAM).specific(m_program);

	state_add(UPD177X_PC,        "PC",    m_pc);
	state_add(STATE_GENPC,       "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE,   "CURPC", m_pc).noshow();
	state_add(UPD177X_SP,        "SP",    m_sp);
	state_add(UPD177X_A,         "A",     m_a);
	state_add(UPD177X_A_SAVE,    "A'",    m_a_save);
	state_add(UPD177X_H,         "H",     m_h);
	state_add(UPD177X_N,         "N",     m_n);
	state_add(UPD177X_TIMER,     "Timer", m_timer);
	state_add(UPD177X_X,         "X",     m_x);
	state_add(UPD177X_Y,         "Y",     m_y);
	state_add(UPD177X_PA,        "PA",    m_pa);
	state_add(UPD177X_PB,        "PB",    m_pb);
	state_add(UPD177X_MD0,       "MD0",   m_md0);
	state_add(UPD177X_MD1,       "MD1",   m_md1);
	state_add(UPD177X_PNC1,      "PNC1",  m_pnc1);
	state_add(UPD177X_PNC2,      "PNC2",  m_pnc2);
	state_add(UPD177X_SKIP,      "SKIP",  m_skip);
	state_add(UPD177X_SKIP_SAVE, "SKIP'", m_skip_save);

	save_item(NAME(m_pc));
	save_item(NAME(m_sp));
	save_item(NAME(m_a));
	save_item(NAME(m_a_save));
	save_item(NAME(m_h));
	save_item(NAME(m_n));
	save_item(NAME(m_timer));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_pa));
	save_item(NAME(m_pb));
	save_item(NAME(m_ram));
	save_item(NAME(m_md0));
	save_item(NAME(m_md1));
	save_item(NAME(m_pnc1));
	save_item(NAME(m_pnc2));
	save_item(NAME(m_skip));
	save_item(NAME(m_skip_save));
	save_item(NAME(m_ts));
	save_item(NAME(m_ns));
	save_item(NAME(m_ss));
	save_item(NAME(m_dac));
	save_item(NAME(m_dac_sign));
	save_item(NAME(m_tn_int));
	save_item(NAME(m_counter));

	set_icountptr(m_icount);

	m_channel = stream_alloc(0, 1, clock() / 4);
}


void upd177x_cpu_device::device_reset()
{
	m_pc = 0;
	m_sp = 0;
	m_a = 0;
	m_a_save = 0;
	m_h = 0;
	m_n = 0;
	m_x = 0;
	m_y = 0;
	m_pa = 0;
	m_pb = 0;
	m_md0 = 0;
	m_md1 = 0;
	m_skip = false;
	m_skip_save = false;
	m_ts = false;
	m_ns = false;
	m_ss = false;
	m_dac = 0;
	m_dac_sign = true;
	m_tn_int = false;
	m_counter = 0;
	m_timer = 0;
	set_noise_counter_bit();
}


std::unique_ptr<util::disasm_interface> upd177x_cpu_device::create_disassembler()
{
	return std::make_unique<upd177x_disassembler>();
}


void upd177x_cpu_device::op0xxx(u16 opcode)
{
	switch (opcode)
	{
	case 0x0000:  // NOP
		break;
	case 0x0002:  // OUT PA
		m_pa = m_a;
		break;
	case 0x0004:  // OUT PB
		m_pb = m_a;
		m_pb_out_cb(m_pb);
		break;
	case 0x0008:  // MOV X,RG
		m_x = m_pnc1 & 0x7f;
		break;
	case 0x0201:  // MOV N,A
		m_n = m_a;
		break;
	case 0x0208:  // MOV X,A
		m_x = m_a & 0x7f;
		break;
	case 0x0401:  // IN PA
		m_a = m_pa;
		break;
	case 0x0402:  // IN PB
		m_a = m_pb;
		break;
	case 0x0404:  // RAR
		m_a = ((m_a & 0x01) << 7) | (m_a >> 1);
		break;
	case 0x0408:  // RAL
		m_a = ((m_a & 0x80) >> 7) | (m_a << 1);
		break;
	case 0x0502:  // OUT DA
		m_channel->update();
		m_dac = (m_ss ^ m_ts) ? ~m_a : m_a;
		m_dac_sign = m_ss;
		break;
	case 0x0504:  // MUL1
		if (!BIT(m_y, 0))
		{
			m_a = m_a >> 1;
		}
		m_y = m_y >> 1;
		break;
	case 0x050c:  // MUL2, usually gets executed 5 times to calculate X * (Y / 32)
		if (BIT(m_y, 0))
		{
			m_a = (m_a + m_x) >> 1;
		}
		else
		{
			m_a = m_a >> 1;
		}
		m_y = m_y >> 1;
		break;
	case 0x0800:  // RET
		m_sp = (m_sp - 1) & 0x07;
		m_pc = (m_ram[STACK_START + (m_sp * 2) + 1] << 8) | m_ram[STACK_START + (m_sp * 2)];
		break;
	case 0x090f:  // RETI
		m_sp = (m_sp - 1) & 0x07;
		m_pc = (m_ram[STACK_START + (m_sp * 2) + 1] << 8) | m_ram[STACK_START + (m_sp * 2)];
		m_a = m_a_save;
		m_skip = m_skip_save;
		m_tn_int = false;
		break;

	case 0x0005:  // STF
	case 0x0101:  // MON
	case 0x0501:  // JMPA
	case 0x0602:  // OFF
	case 0x0801:  // RETS
	default:
		fatalerror("%s, %04x: Unsupported instruction %04x\n", tag(), m_pc - 1, opcode);
	}
}


void upd177x_cpu_device::op1xxx(u16 opcode)
{
	switch (opcode & 0x0e0f)
	{
	case 0x0000:  // MOV Y,Rr
		m_y = m_ram[(opcode >> 4) & 0x1f] & 0x1f;
		break;
	case 0x0005:  // MOV A,Rr
		m_a = m_ram[(opcode >> 4) & 0x1f];
		break;
	case 0x0201:  // MOV Rr,A
		m_ram[(opcode >> 4) & 0x1f] = m_a;
		break;
	case 0x0409:  // MIX Rr
	case 0x0509:  // MIX Rr
		m_ns = BIT(m_md0, MD0_NSF1_BIT) ? BIT(m_pnc2 , 2) : false;
		if (m_ts ^ m_ns)
		{
			m_ss = m_ts ^ ((m_ram[(opcode >> 4) & 0x1f] < m_a) ? true : false);
			m_a = m_ram[(opcode >> 4) & 0x1f] - m_a;
		}
		else
		{
			m_ss = m_ts ^ ((m_ram[(opcode >> 4) & 0x1f] + m_a > 255) ? true : false);
			m_a = m_ram[(opcode >> 4) & 0x1f] + m_a;
		}
		break;
	case 0x0601:  // MOV (H),A
		m_ram[m_h] = m_a;
		break;
	case 0x0801:  // TBL0 A,(Rr)
	case 0x0901:  // TBL0 A,(Rr)
		{
			const u8 reg = (opcode >> 4) & 0x1e;
			const u16 addr = (m_ram[reg + 1] << 8) | m_ram[reg];
			const u16 data = m_program.read_word(addr >> 1);
			m_a = BIT(addr, 0) ? data >> 8 : data & 0xff;
		}
		m_cycle();
		break;
	case 0x0802:  // TBL0 X,(Rr)
	case 0x0902:  // TBL0 X,(Rr)
		{
			const u8 reg = (opcode >> 4) & 0x1e;
			const u16 addr = (m_ram[reg + 1] << 8) | m_ram[reg];
			const u16 data = m_program.read_word(addr >> 1);
			m_x = BIT(addr, 0) ? (data >> 8) : data;
			m_ts = BIT(m_x, 7);
			m_x &= 0x7f;
		}
		m_cycle();
		break;

	case 0x000a:  // MOV H,Rr
	case 0x0202:  // MOV Rr,H
	case 0x0205:  // XCHG Rr,A
	case 0x020a:  // XCHG Rr,H
	case 0x0405:  // MOV A,(H)
	case 0x0605:  // XCHG (H),A
	case 0x0804:  // TBL0 Y,(Rr)
	case 0x0904:  // TBL0 Y,(Rr)
	case 0x0808:  // CALL0 (Rr)
	case 0x0908:  // CALL0 (Rr)
	case 0x0a01:  // TBL1 A,(Rr)
	case 0x0b01:  // TBL1 A,(Rr)
	case 0x0a02:  // TBL1 X,(Rr)
	case 0x0b02:  // TBL1 X,(Rr)
	case 0x0a04:  // TBL1 Y,(Rr)
	case 0x0b04:  // TBL1 Y,(Rr)
	case 0x0a08:  // CALL1 (Rr)
	case 0x0b08:  // CALL1 (Rr)
	default:
		fatalerror("%s, %04x: Unsupported instruction %04x\n", tag(), m_pc - 1, opcode);
	}
}


void upd177x_cpu_device::op2xxx(u16 opcode)
{
	switch (opcode & 0x0f00)
	{
	case 0x0100:  // MVI MD0,A
		m_md0 = opcode & 0x7f;
		set_noise_counter_bit();
		break;
	case 0x0000:  // JPP n
	case 0x8000:  // JMPFZ n
	default:
		fatalerror("%s, %04x: Unsupported instruction %04x\n", tag(), m_pc - 1, opcode);
	}
}


void upd177x_cpu_device::op3xxx(u16 opcode)
{
	switch (opcode & 0x0f00)
	{
	case 0x0400:  // MVI A,n
		m_a = opcode & 0xff;
		break;
	case 0x0800:
		if (!(opcode & 0x00c0))
		{
			m_h = opcode & 0x3f;
		}
		break;
	case 0x0100:  // MVI MD1,n
	case 0x0200:  // MVI (H),n
	default:
		fatalerror("%s, %04x: Unsupported instruction %04x\n", tag(), m_pc - 1, opcode);
	}
}


void upd177x_cpu_device::op7xxx(u16 opcode)
{
	// CALL
	m_ram[STACK_START + (m_sp * 2) + 1] = m_pc >> 8;
	m_ram[STACK_START + (m_sp * 2)] = m_pc & 0xff;
	m_sp = (m_sp + 1) & 0x07;
	m_pc = (m_pc & 0xf000) | (opcode & 0x0fff);
}


void upd177x_cpu_device::op89xxx(u16 opcode)
{
	switch (opcode & 0x1f00)
	{
	case 0x0000:  // ADI A,n
		m_a = m_a + (opcode & 0xff);
		break;
	case 0x0200:  // ANDI A,n
		m_a = m_a & (opcode & 0xff);
		break;
	case 0x0600:  // ORI A,n
		m_a = m_a | (opcode & 0xff);
		break;
	case 0x0800:  // ADIS A,n
		m_a = m_a + (opcode & 0xff);
		m_skip = (m_a < (opcode & 0xff));
		break;
	case 0x0c00:  // SBIS A,n
		m_skip = m_a < (opcode & 0xff);
		m_a = m_a - (opcode & 0xff);
		break;
	case 0x0e00:  // XORI A,n
		m_a = m_a ^ (opcode & 0xff);
		break;
	case 0x1000:  // TADI NC A,n
		m_skip = (m_a + (opcode & 0xff) < 0x100);
		break;
	case 0x1400:  // TSBI NC A,n
		m_skip = (m_a >= (opcode & 0xff));
		break;
	case 0x1600:  // TSBI NZ A,n
		m_skip = (m_a != (opcode & 0xff));
		break;
	case 0x1a00:  // TANDI Z A,n
		m_skip = (m_a & (opcode & 0xff)) == 0;
		break;
	case 0x1c00:  // TSBI C A,n
		m_skip = (m_a < (opcode & 0xff));
		break;

	case 0x0100:  // ADI MD1,n
	case 0x1100:  // ADI MD0,n
	case 0x0300:  // ANDI MD1,n
	case 0x1300:  // ANDI MD0,n
	case 0x0400:  // SBI A,n
	case 0x0500:  // SBI MD1,n
	case 0x1500:  // SBI MD0,n
	case 0x0700:  // ORI MD1,n
	case 0x1700:  // ORI MD0,n
	case 0x0900:  // ADIS MD1,n
	case 0x1900:  // ADIS MD0,n
	case 0x0a00:  // ANDIS A,n
	case 0x0b00:  // ANDIS MD1,n
	case 0x1b00:  // ANDIS MD0,n
	case 0x0d00:  // SBIS MD1,n
	case 0x1d00:  // SBIS MD0,n
	case 0x0f00:  // XORI MD1,n
	case 0x1f00:  // XORI MD0,n
	case 0x1200:  // TANDI NZ A,n
	case 0x1800:  // TADI C A,n
	case 0x1e00:  // TSBI Z A,n
	default:
		fatalerror("%s, %04x: Unsupported instruction %04x\n", tag(), m_pc - 1, opcode);
	}
}


void upd177x_cpu_device::opabxxx(u16 opcode)
{
	switch (opcode & 0x1f00)
	{
	case 0x0d00:  // SBS H,n
		{
			u8 old_h = m_h;
			m_h = (m_h - (opcode & 0x3f)) & 0x3f;
			if (old_h < m_h)
			{
				m_skip = true;
			}
		}
		break;
	case 0x1200:  // TANDI NZ (H),n
		m_skip = (m_ram[m_h] & (opcode & 0xff));
		break;
	case 0x1a00:  // TANDI Z (H),n
		m_skip = (m_ram[m_h] & (opcode & 0xff)) == 0;
		break;
	case 0x1f00:  // TSB Z H,n
		m_skip = (m_h == (opcode & 0xff));
		break;

	case 0x0000:  // AD (H),n
	case 0x0100:  // AD H,n
	case 0x0200:  // AND (H),n
	case 0x0300:  // AND H,n
	case 0x0400:  // SB (H),n
	case 0x0500:  // SB H,n
	case 0x0600:  // OR (H),n
	case 0x0700:  // OR H,n
	case 0x0800:  // ADS (H),n
	case 0x0900:  // ADS H,n
	case 0x0a00:  // ANDS (H),n
	case 0x0b00:  // ANDS H,n
	case 0x0c00:  // SBS (H),n
	case 0x0e00:  // XOR (H),n
	case 0x0f00:  // XOR H,n
	case 0x1000:  // TAD NC (H),n
	case 0x1100:  // TAD NC H,n
	case 0x1300:  // TAND NZ H,n
	case 0x1400:  // TSB NC (H),n
	case 0x1500:  // TSB NC H,n
	case 0x1600:  // TSB NZ (H),n
	case 0x1700:  // TSB NZ H,n
	case 0x1800:  // TAD C (H),n
	case 0x1900:  // TAD C H,n
	case 0x1b00:  // TAND Z H,n
	case 0x1c00:  // TSB C (H),n
	case 0x1d00:  // TSB C H,n
	case 0x1e00:  // TSB Z (H),n
	default:
		fatalerror("%s, %04x: Unsupported instruction %04x\n", tag(), m_pc - 1, opcode);
	}
}


void upd177x_cpu_device::opcdxxx(u16 opcode)
{
	switch (opcode & 0x1e0f)
	{
	case 0x0008:  // AD Rr,A
		m_ram[(opcode >> 4) & 0x1f] += m_a;
		break;
	case 0x1e00:  // TSB Z A,Rr
		m_skip = (m_a == m_ram[(opcode >> 4) & 0x1f]);
		break;
	case 0x0000:  // AD A,Rr
		m_a += m_ram[(opcode >> 4) & 0x1f];
		break;

	case 0x0001:  // AD A,(H)
	case 0x0009:  // AD (H),A
	case 0x0200:  // AND A,Rr
	case 0x0201:  // AND A,(H)
	case 0x0208:  // AND Rr,A
	case 0x0209:  // AND (H),A
	case 0x0400:  // SB A,Rr
	case 0x0401:  // SB A,(H)
	case 0x0408:  // SB Rr,A
	case 0x0409:  // SB (H),A
	case 0x0600:  // OR A,Rr
	case 0x0601:  // OR A,(H)
	case 0x0608:  // OR Rr,A
	case 0x0609:  // OR (H),A
	case 0x0800:  // ADS A,Rr
	case 0x0801:  // ADS A,(H)
	case 0x0808:  // ADS Rr,A
	case 0x0809:  // ADS (H),A
	case 0x0a00:  // ANDS A,Rr
	case 0x0a01:  // ANDS A,(H)
	case 0x0a08:  // ANDS Rr,A
	case 0x0a09:  // ANDS (H),A
	case 0x0c00:  // SBS A,Rr
	case 0x0c01:  // SBS A,(H)
	case 0x0c08:  // SBS Rr,A
	case 0x0c09:  // SBS (H),A
	case 0x0e00:  // XOR A,Rr
	case 0x0e01:  // XOR A,(H)
	case 0x0e08:  // XOR Rr,A
	case 0x0e09:  // XOR (H),A
	case 0x1000:  // TAD NC A,Rr
	case 0x1001:  // TAD NC A,(H)
	case 0x1008:  // TAD NC Rr,A
	case 0x1009:  // TAD NC (H),A
	case 0x1200:  // TAND NZ A,Rr
	case 0x1201:  // TAND NZ A,(H)
	case 0x1208:  // TAND NZ Rr,A
	case 0x1209:  // TAND NZ (H),A
	case 0x1400:  // TSB NC A,Rr
	case 0x1401:  // TSB NC A,(H)
	case 0x1408:  // TSB NC Rr,A
	case 0x1409:  // TSB NC (H),A
	case 0x1600:  // TSB NZ A,Rr
	case 0x1601:  // TSB NZ A,(H)
	case 0x1608:  // TSB NZ Rr,A
	case 0x1609:  // TSB NZ (H),A
	case 0x1800:  // TAD C A,Rr
	case 0x1801:  // TAD C A,(H)
	case 0x1808:  // TAD C Rr,A
	case 0x1809:  // TAD C (H),A
	case 0x1a00:  // TAND Z A,Rr
	case 0x1a01:  // TAND Z A,(H)
	case 0x1a08:  // TAND Z Rr,A
	case 0x1a09:  // TAND Z (H),A
	case 0x1c00:  // TSB C A,Rr
	case 0x1c01:  // TSB C A,(H)
	case 0x1c08:  // TSB C Rr,A
	case 0x1c09:  // TSB C (H),A
	case 0x1e01:  // TSB Z A,(H)
	case 0x1e08:  // TSB Z Rr,A
	case 0x1e09:  // TSB Z (H),A
	default:
		fatalerror("%s, %04x: Unsupported instruction %04x\n", tag(), m_pc - 1, opcode);
	}
}


void upd177x_cpu_device::opefxxx(u16 opcode)
{
	switch (opcode & 0x1e00)
	{
	case 0x0000:  // ADI Rr,n
		m_ram[(opcode >> 4) & 0x1f] += (opcode & 0x0f);
		break;
	case 0x0200:  // ADIS Rr,n
		m_ram[(opcode >> 4) & 0x1f] += (opcode & 0x0f);
		m_skip = (m_ram[(opcode >> 4) & 0x1f] < (opcode & 0x0f));
		break;
	case 0x0400:  // SBI Rr,n
		m_ram[(opcode >> 4) & 0x1f] -= (opcode & 0x0f);
		break;
	case 0x0600:  // SBIS Rr,n
		m_skip = (m_ram[(opcode >> 4) & 0x1f] < (opcode & 0x0f));
		m_ram[(opcode >> 4) & 0x1f] -= (opcode & 0x0f);
		break;
	case 0x0800:  // TADI NC Rr,n
		m_skip = (m_ram[(opcode >> 4) & 0x1f] + (opcode & 0x0f) < 0x100);
		break;
	case 0x0c00:  // TSBI NC Rr,n
		m_skip = (m_ram[(opcode >> 4) & 0x1f] >= (opcode & 0x0f));
		break;
	case 0x0e00:  // TSBI C Rr,n
		m_skip = (m_ram[(opcode >> 4) & 0x1f] < (opcode & 0x0f));
		break;
	case 0x1200:  // ADIMS Rr,n
		{
			const u8 reg = (opcode >> 4) & 0x1f;
			if (BIT(m_md0, 0))
			{
				m_skip = ((m_ram[reg] & 0x3f) + (opcode & 0x0f)) > 0x3f;
				m_ram[reg] = (m_ram[reg] & 0xc0) | ((m_ram[reg] + (opcode & 0x0f)) & 0x3f);
			}
			else
			{
				m_skip = ((m_ram[reg] & 0x1f) + (opcode & 0x0f)) > 0x1f;
				m_ram[reg] = (m_ram[reg] & 0xe0) | ((m_ram[reg] + (opcode & 0x0f)) & 0x1f);
			}
		}
		break;

	case 0x0a00:  // TADI C Rr,n
	case 0x1000:  // ADI5 Rr,n
	case 0x1800:  // TADI5 Rr,n
	default:
		if (opcode != 0xffff)
		{
			fatalerror("%s, %04x: Unsupported instruction %04x\n", tag(), m_pc - 1, opcode);
		}
	}
}


void upd177x_cpu_device::handle_timer()
{
	if (m_n >= 8)
	{
		if (m_timer == 0 || !--m_timer)
		{
			if (BIT(m_md0, MD0_TN_BIT) && !m_tn_int)
			{
				// Trigger TN IRQ
				m_ram[STACK_START + (m_sp * 2) + 1] = m_pc >> 8;
				m_ram[STACK_START + (m_sp * 2)] = m_pc & 0xff;
				m_sp = (m_sp + 1) & 0x07;
				m_skip_save = m_skip;
				m_skip = false;
				m_a_save = m_a;
				if (m_n >= 0x40)
				{
					m_pc = VECTOR_TONE4;
				}
				else if (m_n >= 0x20)
				{
					m_pc = VECTOR_TONE3;
				}
				else if (m_n >= 0x10)
				{
					m_pc = VECTOR_TONE2;
				}
				else
				{
					m_pc = VECTOR_TONE1;
				}
				m_tn_int = true;
			}
			if (m_n >= 0x40)
			{
				m_timer = m_n;
			}
			else if (m_n >= 0x20)
			{
				m_timer = m_n * 2;
			}
			else if (m_n >= 0x10)
			{
				m_timer = m_n * 4;
			}
			else
			{
				m_timer = m_n * 8;
			}
		}
	}
}


void upd177x_cpu_device::set_noise_counter_bit()
{
	if (BIT(m_md0, MD0_NSF2_BIT))
	{
		m_noise_counter_bit = (BIT(m_md1, MD1_NSF3_BIT)) ? 5 : 8;
	}
	else
	{
		m_noise_counter_bit = (BIT(m_md1, MD1_NSF3_BIT)) ? 6 : 9;
	}
}


void upd177x_cpu_device::m_cycle()
{
	m_icount -= 8;
	const u16 old_counter = m_counter;
	m_counter++;
	const u16 counter_diff = old_counter ^ m_counter;
	if (BIT(counter_diff, 9))
	{
		if (BIT(m_md1, MD1_TIME_BIT))
		{
			fatalerror("%s: TM int not supported\n", tag());
		}
	}
	if (BIT(counter_diff, m_noise_counter_bit))
	{
		m_pnc1 = ((m_pnc1 << 1) | ((BIT(m_pnc1, 6) ^ BIT(m_pnc1, 5)) ? 1 : 0)) & 0x7f;
		if (BIT(m_md0, MD0_NSF1_BIT))
		{
			m_pnc2 = (m_pnc2 << 1) | (BIT(m_pnc2, 0) ? 0 : 1);
		}
		else
		{
			m_pnc2 = (m_pnc2 << 1) | ((BIT(m_pnc2, 2) ^ BIT(m_pnc2, 1)) ? 0 : 1);
		}

		if (BIT(m_md0, MD0_NS_BIT))
		{
			fatalerror("%s: NS int not supported\n", tag());
		}
	}
}


void upd177x_cpu_device::execute_run()
{
	do
	{
		handle_timer();
		debugger_instruction_hook(m_pc);
		if (!m_skip)
		{
			u16 opcode = m_program.read_word(m_pc++);
			switch (opcode & 0xf000)
			{
			case 0x0000: op0xxx(opcode); break;
			case 0x1000: op1xxx(opcode); break;
			case 0x2000: op2xxx(opcode); break;
			case 0x3000: op3xxx(opcode); break;
			case 0x4000: m_ram[(opcode >> 8) & 0x1f] = opcode & 0xff; break;  // MVI Rr,n
			case 0x5000: m_ram[(opcode >> 8) & 0x1f] = opcode & 0xff; break;  // MVI Rr,n
			case 0x6000: m_pc = (m_pc & 0xf000) | (opcode & 0x0fff); break;  // JMP n
			case 0x7000: op7xxx(opcode); break;
			case 0x8000: op89xxx(opcode); break;
			case 0x9000: op89xxx(opcode); break;
			case 0xa000: opabxxx(opcode); break;
			case 0xb000: opabxxx(opcode); break;
			case 0xc000: opcdxxx(opcode); break;
			case 0xd000: opcdxxx(opcode); break;
			case 0xe000: opefxxx(opcode); break;
			case 0xf000: opefxxx(opcode); break;
			}
		}
		else
		{
			m_pc++;
			m_skip = false;
		}
		m_cycle();
	} while (m_icount > 0);
}


void upd177x_cpu_device::sound_stream_update(sound_stream &stream)
{
	const int smpl = m_dac_sign ? -m_dac : m_dac;
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		stream.put_int(0, sampindex, smpl, 256);
	}
}
