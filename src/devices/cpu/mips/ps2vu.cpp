// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 Vector Unit device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "ps2vu.h"

#include "video/ps2gif.h"
#include "vudasm.h"

DEFINE_DEVICE_TYPE(SONYPS2_VU0, sonyvu0_device, "sonyvu0", "Sony PlayStation 2 VU0")
DEFINE_DEVICE_TYPE(SONYPS2_VU1, sonyvu1_device, "sonyvu1", "Sony PlayStation 2 VU1")

sonyvu_device::sonyvu_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		address_map_constructor micro_cons,
		address_map_constructor vu_cons,
		chip_type chiptype,
		uint32_t mem_size)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_micro_config("micro", ENDIANNESS_BIG, 64, 12, 0, micro_cons)
	, m_vu_config("vu", ENDIANNESS_BIG, 32, 12, 0, vu_cons)
	, m_micro_space(nullptr)
	, m_vu_space(nullptr)
	, m_mem_size(mem_size)
	, m_mem_mask(mem_size-1)
	, m_micro_mem(*this, "micro")
	, m_vu_mem(*this, "vu")
	, m_vfmem(nullptr)
	, m_vimem(nullptr)
	, m_v(nullptr)
	, m_status_flag(0)
	, m_mac_flag(0)
	, m_clip_flag(0)
	, m_r(0)
	, m_i(0.0f)
	, m_q(0.0f)
	, m_pc(0)
	, m_delay_pc(0)
	, m_start_pc(0)
	, m_running(false)
	, m_icount(0)
{
}

sonyvu0_device::sonyvu0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sonyvu_device(mconfig, SONYPS2_VU0, tag, owner, clock, address_map_constructor(FUNC(sonyvu0_device::micro_map), this), address_map_constructor(FUNC(sonyvu0_device::vu_map), this), CHIP_TYPE_VU0, 0x1000)
	, m_vu1(*this, finder_base::DUMMY_TAG)
{
}

sonyvu1_device::sonyvu1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sonyvu_device(mconfig, SONYPS2_VU1, tag, owner, clock, address_map_constructor(FUNC(sonyvu1_device::micro_map), this), address_map_constructor(FUNC(sonyvu1_device::vu_map), this), CHIP_TYPE_VU0, 0x4000)
	, m_gs(*this, finder_base::DUMMY_TAG)
	, m_vif(*this, "vif")
{
}

void sonyvu1_device::device_add_mconfig(machine_config &config)
{
	SONYPS2_VIF1(config, m_vif, 294912000/2, m_gs, DEVICE_SELF);
}

ps2_vif1_device* sonyvu1_device::interface()
{
	return m_vif.target();
}

uint64_t sonyvu1_device::vif_r(offs_t offset)
{
	return m_vif->mmio_r(offset);
}

void sonyvu1_device::vif_w(offs_t offset, uint64_t data)
{
	m_vif->mmio_w(offset, data);
}

void sonyvu_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	m_micro_space = &space(AS_PROGRAM);
	m_vu_space  = &space(AS_DATA);

	/* register for save states */
	for (int i = 0; i < 32; i++)
	{
		save_item(NAME(m_vfr[i][0]), i);
		save_item(NAME(m_vfr[i][1]), i);
		save_item(NAME(m_vfr[i][2]), i);
		save_item(NAME(m_vfr[i][3]), i);
	}
	save_item(NAME(m_vcr));
	save_item(NAME(m_acc));
	save_item(NAME(m_running));
	save_item(NAME(m_icount));

	save_item(NAME(m_status_flag));
	save_item(NAME(m_mac_flag));
	save_item(NAME(m_clip_flag));
	save_item(NAME(m_r));
	save_item(NAME(m_i));
	save_item(NAME(m_q));
	save_item(NAME(m_pc));
	save_item(NAME(m_delay_pc));
	save_item(NAME(m_start_pc));

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(SONYVU_TPC,  "TPC",   m_pc);
	state_add(SONYVU_SF,   "SF",    m_status_flag);
	state_add(SONYVU_MF,   "MF",    m_mac_flag);
	state_add(SONYVU_CF,   "CF",    m_clip_flag);
	state_add(SONYVU_R,    "R",     m_r);
	state_add(SONYVU_I,    "I",     *(uint32_t*)&m_i).formatstr("%17s");
	state_add(SONYVU_Q,    "Q",     *(uint32_t*)&m_q).formatstr("%17s");

	char elements[4] = { 'x', 'y', 'z', 'w' };
	char regname[6];

	for (int i = 0; i < 4; i++)
	{
		snprintf(regname, 6, "ACC%c", elements[i]);
		state_add(SONYVU_ACCx + i, regname, *(uint32_t*)&m_acc[i]).formatstr("%17s");
	}
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			snprintf(regname, 6, "VF%02d%c", i, elements[j]);
			state_add(SONYVU_VF00x + i*4 + j, regname, *(uint32_t*)&m_vfr[i][j]).formatstr("%17s");
		}
	}
	for (int i = 0; i < 16; i++)
	{
		snprintf(regname, 6, "VI%02d", i);
		state_add(SONYVU_VI00 + i, regname, m_vcr[i]);
	}
}

void sonyvu_device::device_reset()
{
	m_vfmem = reinterpret_cast<float*>(&m_vu_mem[0]);
	m_vimem = &m_vu_mem[0];

	// clear some additional state
	memset(m_vfr, 0, sizeof(float) * 32 * 4);
	memset(m_vcr, 0, sizeof(float) * 32);
	memset(m_acc, 0, sizeof(float) * 4);

	m_v = reinterpret_cast<float*>(m_vfr);

	m_status_flag = 0;
	m_mac_flag = 0;
	m_clip_flag = 0;
	m_r = 0;
	m_i = 0.0f;
	m_q = 0.0f;
	m_pc = 0;
	m_delay_pc = ~0;
	m_start_pc = 0;

	m_v[3] = 1.0f;

	m_running = false;
}

device_memory_interface::space_config_vector sonyvu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_micro_config),
		std::make_pair(AS_DATA,    &m_vu_config)
	};
}

void sonyvu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			break;

		default:
			fatalerror("sonyvu_device::state_import called for unexpected value\n");
	}
}

void sonyvu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			break;

		default:
			fatalerror("sonyvu_device::state_export called for unexpected value\n");
	}
}

void sonyvu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case SONYVU_I: str = string_format("!%16g", m_i); break;
		case SONYVU_Q: str = string_format("!%16g", m_q); break;
		case SONYVU_ACCx: str = string_format("!%16g", m_acc[0]); break;
		case SONYVU_ACCy: str = string_format("!%16g", m_acc[1]); break;
		case SONYVU_ACCz: str = string_format("!%16g", m_acc[2]); break;
		case SONYVU_ACCw: str = string_format("!%16g", m_acc[3]); break;
		case SONYVU_VF00x: str = string_format("!%16g", m_vfr[0][0]); break;
		case SONYVU_VF00y: str = string_format("!%16g", m_vfr[0][1]); break;
		case SONYVU_VF00z: str = string_format("!%16g", m_vfr[0][2]); break;
		case SONYVU_VF00w: str = string_format("!%16g", m_vfr[0][3]); break;
		case SONYVU_VF01x: str = string_format("!%16g", m_vfr[1][0]); break;
		case SONYVU_VF01y: str = string_format("!%16g", m_vfr[1][1]); break;
		case SONYVU_VF01z: str = string_format("!%16g", m_vfr[1][2]); break;
		case SONYVU_VF01w: str = string_format("!%16g", m_vfr[1][3]); break;
		case SONYVU_VF02x: str = string_format("!%16g", m_vfr[2][0]); break;
		case SONYVU_VF02y: str = string_format("!%16g", m_vfr[2][1]); break;
		case SONYVU_VF02z: str = string_format("!%16g", m_vfr[2][2]); break;
		case SONYVU_VF02w: str = string_format("!%16g", m_vfr[2][3]); break;
		case SONYVU_VF03x: str = string_format("!%16g", m_vfr[3][0]); break;
		case SONYVU_VF03y: str = string_format("!%16g", m_vfr[3][1]); break;
		case SONYVU_VF03z: str = string_format("!%16g", m_vfr[3][2]); break;
		case SONYVU_VF03w: str = string_format("!%16g", m_vfr[3][3]); break;
		case SONYVU_VF04x: str = string_format("!%16g", m_vfr[4][0]); break;
		case SONYVU_VF04y: str = string_format("!%16g", m_vfr[4][1]); break;
		case SONYVU_VF04z: str = string_format("!%16g", m_vfr[4][2]); break;
		case SONYVU_VF04w: str = string_format("!%16g", m_vfr[4][3]); break;
		case SONYVU_VF05x: str = string_format("!%16g", m_vfr[5][0]); break;
		case SONYVU_VF05y: str = string_format("!%16g", m_vfr[5][1]); break;
		case SONYVU_VF05z: str = string_format("!%16g", m_vfr[5][2]); break;
		case SONYVU_VF05w: str = string_format("!%16g", m_vfr[5][3]); break;
		case SONYVU_VF06x: str = string_format("!%16g", m_vfr[6][0]); break;
		case SONYVU_VF06y: str = string_format("!%16g", m_vfr[6][1]); break;
		case SONYVU_VF06z: str = string_format("!%16g", m_vfr[6][2]); break;
		case SONYVU_VF06w: str = string_format("!%16g", m_vfr[6][3]); break;
		case SONYVU_VF07x: str = string_format("!%16g", m_vfr[7][0]); break;
		case SONYVU_VF07y: str = string_format("!%16g", m_vfr[7][1]); break;
		case SONYVU_VF07z: str = string_format("!%16g", m_vfr[7][2]); break;
		case SONYVU_VF07w: str = string_format("!%16g", m_vfr[7][3]); break;
		case SONYVU_VF08x: str = string_format("!%16g", m_vfr[8][0]); break;
		case SONYVU_VF08y: str = string_format("!%16g", m_vfr[8][1]); break;
		case SONYVU_VF08z: str = string_format("!%16g", m_vfr[8][2]); break;
		case SONYVU_VF08w: str = string_format("!%16g", m_vfr[8][3]); break;
		case SONYVU_VF09x: str = string_format("!%16g", m_vfr[9][0]); break;
		case SONYVU_VF09y: str = string_format("!%16g", m_vfr[9][1]); break;
		case SONYVU_VF09z: str = string_format("!%16g", m_vfr[9][2]); break;
		case SONYVU_VF09w: str = string_format("!%16g", m_vfr[9][3]); break;
		case SONYVU_VF10x: str = string_format("!%16g", m_vfr[10][0]); break;
		case SONYVU_VF10y: str = string_format("!%16g", m_vfr[10][1]); break;
		case SONYVU_VF10z: str = string_format("!%16g", m_vfr[10][2]); break;
		case SONYVU_VF10w: str = string_format("!%16g", m_vfr[10][3]); break;
		case SONYVU_VF11x: str = string_format("!%16g", m_vfr[11][0]); break;
		case SONYVU_VF11y: str = string_format("!%16g", m_vfr[11][1]); break;
		case SONYVU_VF11z: str = string_format("!%16g", m_vfr[11][2]); break;
		case SONYVU_VF11w: str = string_format("!%16g", m_vfr[11][3]); break;
		case SONYVU_VF12x: str = string_format("!%16g", m_vfr[12][0]); break;
		case SONYVU_VF12y: str = string_format("!%16g", m_vfr[12][1]); break;
		case SONYVU_VF12z: str = string_format("!%16g", m_vfr[12][2]); break;
		case SONYVU_VF12w: str = string_format("!%16g", m_vfr[12][3]); break;
		case SONYVU_VF13x: str = string_format("!%16g", m_vfr[13][0]); break;
		case SONYVU_VF13y: str = string_format("!%16g", m_vfr[13][1]); break;
		case SONYVU_VF13z: str = string_format("!%16g", m_vfr[13][2]); break;
		case SONYVU_VF13w: str = string_format("!%16g", m_vfr[13][3]); break;
		case SONYVU_VF14x: str = string_format("!%16g", m_vfr[14][0]); break;
		case SONYVU_VF14y: str = string_format("!%16g", m_vfr[14][1]); break;
		case SONYVU_VF14z: str = string_format("!%16g", m_vfr[14][2]); break;
		case SONYVU_VF14w: str = string_format("!%16g", m_vfr[14][3]); break;
		case SONYVU_VF15x: str = string_format("!%16g", m_vfr[15][0]); break;
		case SONYVU_VF15y: str = string_format("!%16g", m_vfr[15][1]); break;
		case SONYVU_VF15z: str = string_format("!%16g", m_vfr[15][2]); break;
		case SONYVU_VF15w: str = string_format("!%16g", m_vfr[15][3]); break;
		case SONYVU_VF16x: str = string_format("!%16g", m_vfr[16][0]); break;
		case SONYVU_VF16y: str = string_format("!%16g", m_vfr[16][1]); break;
		case SONYVU_VF16z: str = string_format("!%16g", m_vfr[16][2]); break;
		case SONYVU_VF16w: str = string_format("!%16g", m_vfr[16][3]); break;
		case SONYVU_VF17x: str = string_format("!%16g", m_vfr[17][0]); break;
		case SONYVU_VF17y: str = string_format("!%16g", m_vfr[17][1]); break;
		case SONYVU_VF17z: str = string_format("!%16g", m_vfr[17][2]); break;
		case SONYVU_VF17w: str = string_format("!%16g", m_vfr[17][3]); break;
		case SONYVU_VF18x: str = string_format("!%16g", m_vfr[18][0]); break;
		case SONYVU_VF18y: str = string_format("!%16g", m_vfr[18][1]); break;
		case SONYVU_VF18z: str = string_format("!%16g", m_vfr[18][2]); break;
		case SONYVU_VF18w: str = string_format("!%16g", m_vfr[18][3]); break;
		case SONYVU_VF19x: str = string_format("!%16g", m_vfr[19][0]); break;
		case SONYVU_VF19y: str = string_format("!%16g", m_vfr[19][1]); break;
		case SONYVU_VF19z: str = string_format("!%16g", m_vfr[19][2]); break;
		case SONYVU_VF19w: str = string_format("!%16g", m_vfr[19][3]); break;
		case SONYVU_VF20x: str = string_format("!%16g", m_vfr[20][0]); break;
		case SONYVU_VF20y: str = string_format("!%16g", m_vfr[20][1]); break;
		case SONYVU_VF20z: str = string_format("!%16g", m_vfr[20][2]); break;
		case SONYVU_VF20w: str = string_format("!%16g", m_vfr[20][3]); break;
		case SONYVU_VF21x: str = string_format("!%16g", m_vfr[21][0]); break;
		case SONYVU_VF21y: str = string_format("!%16g", m_vfr[21][1]); break;
		case SONYVU_VF21z: str = string_format("!%16g", m_vfr[21][2]); break;
		case SONYVU_VF21w: str = string_format("!%16g", m_vfr[21][3]); break;
		case SONYVU_VF22x: str = string_format("!%16g", m_vfr[22][0]); break;
		case SONYVU_VF22y: str = string_format("!%16g", m_vfr[22][1]); break;
		case SONYVU_VF22z: str = string_format("!%16g", m_vfr[22][2]); break;
		case SONYVU_VF22w: str = string_format("!%16g", m_vfr[22][3]); break;
		case SONYVU_VF23x: str = string_format("!%16g", m_vfr[23][0]); break;
		case SONYVU_VF23y: str = string_format("!%16g", m_vfr[23][1]); break;
		case SONYVU_VF23z: str = string_format("!%16g", m_vfr[23][2]); break;
		case SONYVU_VF23w: str = string_format("!%16g", m_vfr[23][3]); break;
		case SONYVU_VF24x: str = string_format("!%16g", m_vfr[24][0]); break;
		case SONYVU_VF24y: str = string_format("!%16g", m_vfr[24][1]); break;
		case SONYVU_VF24z: str = string_format("!%16g", m_vfr[24][2]); break;
		case SONYVU_VF24w: str = string_format("!%16g", m_vfr[24][3]); break;
		case SONYVU_VF25x: str = string_format("!%16g", m_vfr[25][0]); break;
		case SONYVU_VF25y: str = string_format("!%16g", m_vfr[25][1]); break;
		case SONYVU_VF25z: str = string_format("!%16g", m_vfr[25][2]); break;
		case SONYVU_VF25w: str = string_format("!%16g", m_vfr[25][3]); break;
		case SONYVU_VF26x: str = string_format("!%16g", m_vfr[26][0]); break;
		case SONYVU_VF26y: str = string_format("!%16g", m_vfr[26][1]); break;
		case SONYVU_VF26z: str = string_format("!%16g", m_vfr[26][2]); break;
		case SONYVU_VF26w: str = string_format("!%16g", m_vfr[26][3]); break;
		case SONYVU_VF27x: str = string_format("!%16g", m_vfr[27][0]); break;
		case SONYVU_VF27y: str = string_format("!%16g", m_vfr[27][1]); break;
		case SONYVU_VF27z: str = string_format("!%16g", m_vfr[27][2]); break;
		case SONYVU_VF27w: str = string_format("!%16g", m_vfr[27][3]); break;
		case SONYVU_VF28x: str = string_format("!%16g", m_vfr[28][0]); break;
		case SONYVU_VF28y: str = string_format("!%16g", m_vfr[28][1]); break;
		case SONYVU_VF28z: str = string_format("!%16g", m_vfr[28][2]); break;
		case SONYVU_VF28w: str = string_format("!%16g", m_vfr[28][3]); break;
		case SONYVU_VF29x: str = string_format("!%16g", m_vfr[29][0]); break;
		case SONYVU_VF29y: str = string_format("!%16g", m_vfr[29][1]); break;
		case SONYVU_VF29z: str = string_format("!%16g", m_vfr[29][2]); break;
		case SONYVU_VF29w: str = string_format("!%16g", m_vfr[29][3]); break;
		case SONYVU_VF30x: str = string_format("!%16g", m_vfr[30][0]); break;
		case SONYVU_VF30y: str = string_format("!%16g", m_vfr[30][1]); break;
		case SONYVU_VF30z: str = string_format("!%16g", m_vfr[30][2]); break;
		case SONYVU_VF30w: str = string_format("!%16g", m_vfr[30][3]); break;
		case SONYVU_VF31x: str = string_format("!%16g", m_vfr[31][0]); break;
		case SONYVU_VF31y: str = string_format("!%16g", m_vfr[31][1]); break;
		case SONYVU_VF31z: str = string_format("!%16g", m_vfr[31][2]); break;
		case SONYVU_VF31w: str = string_format("!%16g", m_vfr[31][3]); break;
	}
}

std::unique_ptr<util::disasm_interface> sonyvu_device::create_disassembler()
{
	return std::make_unique<sonyvu_disassembler>();
}

void sonyvu_device::execute_run()
{
	while (m_icount > 0)
	{
		if (!m_running)
		{
			m_icount = 0;
			return;
		}

		debugger_instruction_hook(m_pc);

		const uint64_t op = m_micro_mem[(m_pc & m_mem_mask) >> 3];
		m_pc += 8;
		if (m_delay_pc != ~0)
		{
			m_pc = m_delay_pc;
			m_delay_pc = ~0;
		}

		execute_upper((uint32_t)(op >> 32));
		if (op & OP_UPPER_I)
		{
			uint32_t lower_op = (uint32_t)op;
			m_i = *reinterpret_cast<float*>(&lower_op);
		}
		else
		{
			execute_lower((uint32_t)op);
		}

		m_icount--;
	}
}

void sonyvu_device::execute_upper(const uint32_t op)
{
	const int rs   = (op >> 11) & 31;
	const int rt   = (op >> 16) & 31;
	const int rd   = (op >>  6) & 31;

	switch (op & 0x3f)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: // ADDbc
			printf("Unsupported VU instruction: ADDbc\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x04: case 0x05: case 0x06: case 0x07: // SUBbc
			printf("Unsupported VU instruction: SUBbc\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b: // MADDbc
			if (rd)
			{
				const uint32_t bc = op & 3;
				float *fs = m_vfr[rs];
				float *ft = m_vfr[rt];
				float *fd = m_vfr[rd];
				for (int field = 0; field < 4; field++)
				{
					if (BIT(op, 24-field))
					{
						fd[field] = m_acc[field] + fs[field] * ft[bc];
					}
				}
			}
			break;
		case 0x0c: case 0x0d: case 0x0e: case 0x0f: // MSUBbc
			printf("Unsupported VU instruction: MSUBbc\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x10: case 0x11: case 0x12: case 0x13: // MAXbc
			printf("Unsupported VU instruction: MAXbc\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x14: case 0x15: case 0x16: case 0x17: // MINIbc
			printf("Unsupported VU instruction: MINIbc\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: // MULbc
			printf("Unsupported VU instruction: MULbc\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x1c: // MULq
			printf("Unsupported VU instruction: MULq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x1d: // MAXi
			printf("Unsupported VU instruction: MAXi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x1e: // MULi
			printf("Unsupported VU instruction: MULi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x1f: // MINIi
			printf("Unsupported VU instruction: MINIi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x20: // ADDq
			printf("Unsupported VU instruction: ADDq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x21: // MADDq
			printf("Unsupported VU instruction: MADDq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x22: // ADDi
			printf("Unsupported VU instruction: ADDi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x23: // MADDi
			printf("Unsupported VU instruction: MADDi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x24: // SUBq
			printf("Unsupported VU instruction: SUBq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x25: // MSUBq
			printf("Unsupported VU instruction: MSUBq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x26: // SUBi
			printf("Unsupported VU instruction: SUBi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x27: // MSUBi
			printf("Unsupported VU instruction: MSUBi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x28: // ADD
			printf("Unsupported VU instruction: ADD\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x29: // MADD
			printf("Unsupported VU instruction: MADD\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x2a: // MUL
			printf("Unsupported VU instruction: MUL\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x2b: // MAX
			printf("Unsupported VU instruction: MAX\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x2c: // SUB
			printf("Unsupported VU instruction: SUB\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x2d: // MSUB
			printf("Unsupported VU instruction: MSUB\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x2e: // OPMSUB
			printf("Unsupported VU instruction: OPMSUB\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x2f: // MINI
			printf("Unsupported VU instruction: MINI\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		{
			const uint8_t type2_op = ((op & 0x3c0) >> 4) | (op & 3);
			switch (type2_op)
			{
				case 0x00: case 0x01: case 0x02: case 0x03: // ADDAbc
					printf("Unsupported VU instruction: ADDAb\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x04: case 0x05: case 0x06: case 0x07: // SUBAbc
					printf("Unsupported VU instruction: SUBAbc\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x08: case 0x09: case 0x0a: case 0x0b: // MADDAbc
					if (rd)
					{
						const uint32_t bc = op & 3;
						float *fs = m_vfr[rs];
						float *ft = m_vfr[rt];
						for (int field = 0; field < 4; field++)
						{
							if (BIT(op, 24-field))
							{
								m_acc[field] += fs[field] * ft[bc];
							}
						}
					}
					break;
				case 0x0c: case 0x0d: case 0x0e: case 0x0f: // MSUBAbc
					printf("Unsupported VU instruction: MSUBAbc\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x18: case 0x19: case 0x1a: case 0x1b: // MULAbc
				{
					const uint32_t bc = op & 3;
					float *fs = m_vfr[rs];
					float *ft = m_vfr[rt];
					for (int field = 0; field < 4; field++)
					{
						if (BIT(op, 24-field))
						{
							m_acc[field] = fs[field] * ft[bc];
						}
					}
					break;
				}
				case 0x10: // ITOF0
					printf("Unsupported VU instruction: ITOF0\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x11: // ITOF4
					printf("Unsupported VU instruction: ITOF4\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x12: // ITOF12
					printf("Unsupported VU instruction: ITOF12\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x13: // ITOF15
					printf("Unsupported VU instruction: ITOF15\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x14: // FTOI0
					printf("Unsupported VU instruction: FTOI0\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x15: // FTOI4
					printf("Unsupported VU instruction: FTOI4\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x16: // FTOI12
					printf("Unsupported VU instruction: FTOI12\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x17: // FTOI15
					printf("Unsupported VU instruction: FTOI15\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x1c: // MULAq
					printf("Unsupported VU instruction: MULAq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x1d: // ABS
					printf("Unsupported VU instruction: ABS\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x1e: // MULAi
					printf("Unsupported VU instruction: MULAi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x1f: // CLIP
					printf("Unsupported VU instruction: CLIP\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x20: // ADDAq
					printf("Unsupported VU instruction: ADDAq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x21: // MADDAq
					printf("Unsupported VU instruction: MADDAq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x22: // ADDAi
					printf("Unsupported VU instruction: ADDAi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x23: // MADDAi
					printf("Unsupported VU instruction: MADDAi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x24: // SUBAq
					printf("Unsupported VU instruction: SUBAq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x25: // MSUBAq
					printf("Unsupported VU instruction: MSUBAq\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x26: // SUBAi
					printf("Unsupported VU instruction: SUBAi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x27: // MSUBAi
					printf("Unsupported VU instruction: MSUBAi\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x28: // ADDA
					printf("Unsupported VU instruction: ADDA\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x29: // MADDA
					printf("Unsupported VU instruction: MADDA\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x2a: // MULA
					printf("Unsupported VU instruction: MULA\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x2c: // SUBA
					printf("Unsupported VU instruction: SUBA\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x2d: // MSUBA
					printf("Unsupported VU instruction: MSUBA\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x2e: // OPMULA
					printf("Unsupported VU instruction: OPMULA\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
					break;
				case 0x2f: // NOP
					break;
				default:
					logerror("%s: %08x: Unknown upper opcode %08x\n", machine().describe_context(), m_pc, op);
					break;
			}
			break;
		}
		default:
			logerror("%s: %08x: Unknown upper opcode %08x\n", machine().describe_context(), m_pc, op);
			break;
	}
}

void sonyvu_device::execute_lower(const uint32_t op)
{
	const int rd = (op >>  6) & 31;
	const int rs = (op >> 11) & 31;
	const int rt = (op >> 16) & 31;
	const int dest = (op >> 21) & 15;
	const int fsf = (op >> 21) & 3;
	//const int ftf = (op >> 23) & 3;

	switch ((op >> 25) & 0x7f)
	{
		case 0x00: // LQ
			printf("Unsupported VU instruction: LQ\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x01: // SQ
			printf("Unsupported VU instruction: SQ\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x04: // ILW
			printf("Unsupported VU instruction: ILW\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x05: // ISW
			printf("Unsupported VU instruction: ISW\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x08: // IADDIU
			if (rt)
				m_vcr[rt] = (uint16_t)(m_vcr[rs] + (op & 0x7ff));
			break;
		case 0x09: // ISUBIU
			printf("Unsupported VU instruction: ISUBIU\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x10: // FCEQ
			printf("Unsupported VU instruction: FCEQ\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x11: // FCSET
			printf("Unsupported VU instruction: FCSET\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x12: // FCAND
			printf("Unsupported VU instruction: FCAND\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x13: // FCOR
			printf("Unsupported VU instruction: FCOR\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x14: // FSEQ
			printf("Unsupported VU instruction: FSEQ\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x15: // FSSET
			printf("Unsupported VU instruction: FSSET\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x16: // FSAND
			printf("Unsupported VU instruction: FSAND\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x17: // FSOR
			printf("Unsupported VU instruction: FSOR\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x18: // FMEQ
			printf("Unsupported VU instruction: FMEQ\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x1a: // FMAND
			printf("Unsupported VU instruction: FMAND\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x1b: // FMOR
			printf("Unsupported VU instruction: FMOR\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x1c: // FCGET
			printf("Unsupported VU instruction: FCGET\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
			break;
		case 0x20: // B
			m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x21: // BAL
			if (rt)
				m_vcr[rt] = m_pc + 8;
			m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x24: // JR
			m_delay_pc = m_vcr[rs] & m_mem_mask;
			break;
		case 0x25: // JALR
			if (rt)
				m_vcr[rt] = m_pc + 8;
			m_delay_pc = m_vcr[rs] & m_mem_mask;
			break;
		case 0x28: // IBEQ
			if (m_vcr[rs] == m_vcr[rt])
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x29: // IBNE
			if (m_vcr[rs] != m_vcr[rt])
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x2c: // IBLTZ
			if ((int16_t)m_vcr[rs] < 0)
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x2d: // IBGTZ
			if ((int16_t)m_vcr[rs] > 0)
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x2e: // IBLEZ
			if ((int16_t)m_vcr[rs] <= 0)
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x2f: // IBGEZ
			if ((int16_t)m_vcr[rs] >= 0)
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x40: // SPECIAL
		{
			if ((op & 0x3c) == 0x3c)
			{
				uint8_t type4_op = ((op & 0x7c0) >> 4) | (op & 3);
				switch (type4_op)
				{
					case 0x30:
						if (rs == 0 && rt == 0 && dest == 0)
						{   // NOP
						}
						else
						{   // MOVE
							printf("Unsupported VU instruction: MOVE\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						}
						break;
					case 0x31: // MR32
						printf("Unsupported VU instruction: MR32\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x34: // LQI
					{
						if (rt)
						{
							uint16_t addr = (m_vcr[rs] << 2) & m_mem_mask;
							float* vumem = reinterpret_cast<float*>(&m_vu_mem[0]);
							float* base = m_vfr[rt];
							for (int field = 0; field < 4; field++)
							{
								if (BIT(op, 24-field))
								{
									base[field] = vumem[addr + field];
								}
							}
							m_vcr[rs] = (uint16_t)(m_vcr[rs] + 1);
						}
						break;
					}
					case 0x35: // SQI
						if (rt)
						{
							uint16_t addr = (m_vcr[rt] << 2) & m_mem_mask;
							float* vumem = reinterpret_cast<float*>(&m_vu_mem[0]);
							float* base = m_vfr[rs];
							for (int field = 0; field < 4; field++)
							{
								if (BIT(op, 24-field))
								{
									vumem[addr + field] = base[field];
								}
							}
							m_vcr[rt] = (uint16_t)(m_vcr[rt] + 1);
						}
						break;
					case 0x36: // LQD
						printf("Unsupported VU instruction: LQD\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x37: // SQD
						printf("Unsupported VU instruction: SQD\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x38: // DIV
						printf("Unsupported VU instruction: DIV\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x39: // SQRT
						printf("Unsupported VU instruction: SQRT\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x3a: // RSQRT
						printf("Unsupported VU instruction: RSQRT\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x3b: // WAITQ
						printf("Unsupported VU instruction: WAITQ\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x3c: // MTIR
						if (rt)
							m_vcr[rt] = (uint16_t)*reinterpret_cast<uint32_t*>(&m_vfr[rs][fsf]);
						break;
					case 0x3d: // MFIR
						if (rt)
						{
							int32_t* base = reinterpret_cast<int32_t*>(m_vfr[rt]);
							int32_t value = (int16_t)(m_vcr[rs] & 0xffff);
							for (int field = 0; field < 4; field++)
							{
								if (BIT(op, 24-field))
								{
									base[field] = value;
								}
							}
						}
						break;
					case 0x3e: // ILWR
						printf("Unsupported VU instruction: ILWR\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x3f: // ISWR
						printf("Unsupported VU instruction: ISWR\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x40: // RNEXT
						printf("Unsupported VU instruction: RNEXT\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x41: // RGET
						printf("Unsupported VU instruction: RGET\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x42: // RINIT
						printf("Unsupported VU instruction: RINIT\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x43: // RXOR
						printf("Unsupported VU instruction: RXOR\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x64: // MFP
						printf("Unsupported VU instruction: MFP\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x68: // XTOP
						printf("Unsupported VU instruction: XTOP\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x69: // XITOP
						printf("Unsupported VU instruction: XITOP\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x6c: // XGKICK
						execute_xgkick(rs);
						break;
					case 0x70: // ESADD
						printf("Unsupported VU instruction: ESADD\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x71: // ERSADD
						printf("Unsupported VU instruction: ERSADD\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x72: // ELENG
						printf("Unsupported VU instruction: ELENG\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x73: // ERLENG
						printf("Unsupported VU instruction: ERLENG\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x74: // EATANxy
						printf("Unsupported VU instruction: EATANxy\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x75: // EATANxz
						printf("Unsupported VU instruction: EATANxz\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x76: // ESUM
						printf("Unsupported VU instruction: ESUM\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x78: // ESQRT
						printf("Unsupported VU instruction: ESQRT\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x79: // ERSQRT
						printf("Unsupported VU instruction: ERSQRT\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x7a: // ERCPR
						printf("Unsupported VU instruction: ERCPR\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x7b: // WAITP
						printf("Unsupported VU instruction: WAITP\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x7c: // ESIN
						printf("Unsupported VU instruction: ESIN\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x7d: // EATAN
						printf("Unsupported VU instruction: EATAN\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x7e: // EEXP
						printf("Unsupported VU instruction: EEXP\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					default:
						logerror("%s: %08x: Unknown lower opcode %08x\n", machine().describe_context(), m_pc, op);
						break;
				}
				break;
			}
			else
			{
				switch (op & 0x3f)
				{
					case 0x30: // IADD
						if (rd)
							m_vcr[rd] = (uint16_t)(m_vcr[rs] + m_vcr[rt]);
						break;
					case 0x31: // ISUB
						if (rd)
							m_vcr[rd] = (uint16_t)(m_vcr[rs] - m_vcr[rt]);
						break;
					case 0x32: // IADDI
						printf("Unsupported VU instruction: IADDI\n"); fflush(stdout); fatalerror("Unsupported VU instruction\n"); break;
						break;
					case 0x34: // IAND
						if (rd)
							m_vcr[rd] = (uint16_t)(m_vcr[rs] & m_vcr[rt]);
						break;
					case 0x35: // IOR
						if (rd)
							m_vcr[rd] = (uint16_t)(m_vcr[rs] | m_vcr[rt]);
						break;
					default:
						logerror("%s: %08x: Unknown lower opcode %08x\n", machine().describe_context(), m_pc, op);
						break;
				}
			}
			break;
		}
		default:
			logerror("%s: %08x: Unknown lower opcode %08x\n", machine().describe_context(), m_pc, op);
			break;
	}
}

void sonyvu0_device::micro_map(address_map &map)
{
	map(0x000, 0xfff).ram().share(m_micro_mem);
}

void sonyvu0_device::vu_map(address_map &map)
{
	map(0x000, 0xfff).ram().share(m_vu_mem);
}

void sonyvu0_device::device_start()
{
	sonyvu_device::device_start();

	save_item(NAME(m_cmsar0));
	save_item(NAME(m_cmsar1));
	save_item(NAME(m_control));
	save_item(NAME(m_vpu_stat));

	state_add(SONYVU0_CMSAR0,   "CMSAR0",   m_cmsar0);
	state_add(SONYVU0_CMSAR1,   "CMSAR1",   m_cmsar1);
	state_add(SONYVU0_FBRST,    "FBRST",    m_control);
	state_add(SONYVU0_VPU_STAT, "VPU_STAT", m_vpu_stat);
}

void sonyvu0_device::device_reset()
{
	sonyvu_device::device_reset();

	m_vu1_regs = m_vu1->vector_regs();

	m_cmsar0 = 0;
	m_control = 0;
	m_vpu_stat = 0;
	m_cmsar1 = 0;
}

uint32_t sonyvu0_device::vu1_reg_r(offs_t offset)
{
	return m_vu1_regs[offset];
}

void sonyvu0_device::vu1_reg_w(offs_t offset, uint32_t data)
{
	m_vu1_regs[offset] = data;
}

void sonyvu0_device::execute_xgkick(uint32_t rs)
{
	fatalerror("Unsupported VU0 instruction: XGKICK\n");
}

void sonyvu1_device::device_start()
{
	sonyvu_device::device_start();

	save_item(NAME(m_p));

	state_add(SONYVU1_P, "P", *(uint32_t*)&m_p).formatstr("%17s");
}

void sonyvu1_device::device_reset()
{
	sonyvu_device::device_reset();

	m_p = 0.0f;
}

void sonyvu1_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case SONYVU1_P: str = string_format("!%16g", m_p); break;
		default: sonyvu_device::state_string_export(entry, str); break;
	}
}

void sonyvu1_device::micro_map(address_map &map)
{
	map(0x000, 0xfff).ram().share(m_micro_mem);
}

void sonyvu1_device::vu_map(address_map &map)
{
	map(0x000, 0xfff).ram().share(m_vu_mem);
}

void sonyvu_device::write_vu_mem(uint32_t address, uint32_t data)
{
	m_vu_mem[(address & m_mem_mask) >> 2] = data;
}

void sonyvu_device::write_micro_mem(uint32_t address, uint64_t data)
{
	m_micro_mem[(address & m_mem_mask) >> 3] = data;
}

void sonyvu1_device::execute_xgkick(uint32_t rs)
{
	if (m_gs->interface()->path1_available())
	{
		m_gs->interface()->kick_path1(m_vcr[rs]);
	}
	else
	{
		m_pc -= 8;
	}
}

void sonyvu_device::start(uint32_t address)
{
	m_pc = address & m_mem_mask;
	m_running = true;
}

int16_t sonyvu_device::immediate_s11(const uint32_t op)
{
	int16_t sval = (int16_t)(op << 5);
	return sval >> 5;
}
