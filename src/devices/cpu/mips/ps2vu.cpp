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

#include <array>
#include <bit>

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
	, m_micro_config("micro", ENDIANNESS_BIG, 64, chiptype == CHIP_TYPE_VU0 ? 12 : 14, 0, micro_cons)
	, m_vu_config("vu", ENDIANNESS_BIG, 32, chiptype == CHIP_TYPE_VU0 ? 15 : 14, 0, vu_cons)
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
	, m_vi_old{}
	, m_vi_cycle{}
	, m_vi_chain{}
	, m_link_pc(0)
	, m_upper_dest(0)
	, m_end_delay(0)
	, m_draining(false)
	, m_mbit(false)
	, m_debug_control(0)
	, m_stop_flags(0)
	, m_interrupt_pending(false)
	, m_irq(*this)
	, m_icount(0)
{
}

sonyvu0_device::sonyvu0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sonyvu_device(mconfig, SONYPS2_VU0, tag, owner, clock, address_map_constructor(FUNC(sonyvu0_device::micro_map), this), address_map_constructor(FUNC(sonyvu0_device::vu_map), this), CHIP_TYPE_VU0, 0x1000)
	, m_vu1(*this, finder_base::DUMMY_TAG)
{
}

sonyvu1_device::sonyvu1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sonyvu_device(mconfig, SONYPS2_VU1, tag, owner, clock, address_map_constructor(FUNC(sonyvu1_device::micro_map), this), address_map_constructor(FUNC(sonyvu1_device::vu_map), this), CHIP_TYPE_VU1, 0x4000)
	, m_gs(*this, finder_base::DUMMY_TAG)
	, m_vif(*this, "vif")
	, m_kick_pending(false)
	, m_kick_address(0)
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
	save_item(NAME(m_pipeline.cycle));
	save_item(NAME(m_pipeline.due));
	save_item(NAME(m_pipeline.value));
	save_item(NAME(m_pipeline.mac));
	save_item(NAME(m_pipeline.status));
	save_item(NAME(m_pipeline.reg));
	save_item(NAME(m_pipeline.mask));
	save_item(NAME(m_pipeline.flags));
	save_item(NAME(m_pipeline.head));
	save_item(NAME(m_pipeline.tail));
	save_item(NAME(m_pipeline.count));
	save_item(NAME(m_pipeline.acc_overflow));
	save_item(NAME(m_pipeline.q_due));
	save_item(NAME(m_pipeline.q_value));
	save_item(NAME(m_pipeline.q_flags));
	save_item(NAME(m_pipeline.vi_due));
	save_item(NAME(m_pipeline.vi_value));
	save_item(NAME(m_vi_old));
	save_item(NAME(m_vi_cycle));
	save_item(NAME(m_vi_chain));
	save_item(NAME(m_link_pc));
	save_item(NAME(m_upper_dest));
	save_item(NAME(m_end_delay));
	save_item(NAME(m_draining));
	save_item(NAME(m_mbit));
	save_item(NAME(m_debug_control));
	save_item(NAME(m_stop_flags));
	save_item(NAME(m_interrupt_pending));

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
	m_pipeline = {};
	std::fill(std::begin(m_vi_old), std::end(m_vi_old), 0);
	std::fill(std::begin(m_vi_cycle), std::end(m_vi_cycle), ~uint64_t(0));
	std::fill(std::begin(m_vi_chain), std::end(m_vi_chain), 0);
	m_link_pc = m_upper_dest = m_end_delay = 0;
	m_draining = m_mbit = m_interrupt_pending = false;
	m_debug_control = m_stop_flags = 0;
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

bool sonyvu_device::pair_hazard(uint64_t op) const
{
	const uint32_t upper = op >> 32;
	if (macro_hazard((upper & 63) >= 0x3c ? upper & ~0x400U : upper))
		return true;
	if (op & OP_UPPER_I)
		return false;
	const unsigned rs = (op >> 11) & 31, rt = (op >> 16) & 31, rd = (op >> 6) & 31;
	const unsigned mask = (op >> 21) & 15;
	const auto busy = [this](unsigned r) { return r && m_pipeline.vi_due[r] > m_pipeline.cycle; };
	switch ((op >> 25) & 0x7f)
	{
		case 0x00: return busy(rs); // LQ
		case 0x01: return busy(rt) || m_pipeline.ready(rs, mask) > m_pipeline.cycle; // SQ
		case 0x08: case 0x09: case 0x1a: case 0x25: case 0x28: case 0x29:
			return busy(rs) || busy(rt);
		case 0x21: return busy(rt); // BAL
		case 0x24: case 0x2c: case 0x2d: case 0x2e: case 0x2f: return busy(rs);
		case 0x40:
			if ((op & 0x3c) != 0x3c)
				return busy(rs) || busy(rt) || ((op & 63) != 0x32 && busy(rd));
			switch (((op >> 4) & 0x7c) | (op & 3))
			{
				case 0x34: case 0x3d: case 0x6c: return busy(rs);
				case 0x35: return busy(rt) || m_pipeline.ready(rs, mask) > m_pipeline.cycle;
				case 0x38: case 0x39: case 0x3b: return macro_hazard(uint32_t(op));
				case 0x3c: return busy(rt) || m_pipeline.ready(rs, 8 >> ((op >> 21) & 3)) > m_pipeline.cycle;
				case 0x3e: return busy(rs) || busy(rt);
				case 0x68: case 0x69: return busy(rt);
			}
	}
	return false;
}

void sonyvu_device::write_vi(unsigned reg, uint16_t value, bool dependent)
{
	if (!reg)
		return;
	// a branch reads the value from before an adjacent producer, for up to four dependent issues
	if (!dependent || m_vi_cycle[reg] + 1 != m_pipeline.cycle || m_vi_chain[reg] == 4)
	{
		m_vi_old[reg] = m_vcr[reg];
		m_vi_chain[reg] = 0;
	}
	++m_vi_chain[reg];
	m_vi_cycle[reg] = m_pipeline.cycle;
	m_vcr[reg] = value;
}

uint16_t sonyvu_device::branch_vi(unsigned reg) const
{
	return reg && m_vi_chain[reg] && m_vi_cycle[reg] + 1 == m_pipeline.cycle ? m_vi_old[reg] : m_vcr[reg];
}

void sonyvu_device::execute_run()
{
	while (m_icount > 0)
	{
		m_pipeline.retire(m_vfr, m_status_flag, m_mac_flag, m_q, m_vcr);
		if (!m_running)
		{
			m_icount = 0;
			return;
		}

		const bool kick_ready = service_xgkick();
		if (m_draining)
		{
			if (!m_pipeline.count && !m_pipeline.q_due && !m_pipeline.integers_pending() && kick_ready)
			{
				m_running = m_draining = m_mbit = false;
				if (m_interrupt_pending)
				{
					m_interrupt_pending = false;
					m_irq(1);
					m_irq(0);
				}
			}
		}
		else if (kick_ready)
		{
			const uint64_t op = m_micro_mem[(m_pc & m_mem_mask) >> 3];
			if (!pair_hazard(op))
			{
				debugger_instruction_hook(m_pc);
				const uint32_t redirect = m_delay_pc;
				m_delay_pc = ~0U;
				m_pc = (m_pc + 8) & m_mem_mask;
				m_link_pc = ((redirect != ~0U ? redirect : m_pc) + 8) & m_mem_mask;
				m_mbit = (op & OP_UPPER_M) != 0;

				const unsigned upper_slot = m_pipeline.tail;
				execute_upper(op >> 32);
				// an upper VF write wins over the lower one
				m_upper_dest = m_pipeline.tail != upper_slot && m_pipeline.mask[upper_slot] ? m_pipeline.reg[upper_slot] : 0;
				if (op & OP_UPPER_I)
					m_i = std::bit_cast<float>(uint32_t(op));
				else
					execute_lower(uint32_t(op));
				m_upper_dest = 0;
				if (redirect != ~0U)
					m_pc = redirect;

				if (m_end_delay && !--m_end_delay)
					m_draining = true;
				if ((op & OP_UPPER_E) && !m_draining && !m_end_delay)
					m_end_delay = 1;
				const bool d = (op & OP_UPPER_D) && (m_debug_control & 4);
				const bool t = (op & OP_UPPER_T) && (m_debug_control & 8);
				if (d || t)
				{
					m_stop_flags |= (d ? 2 : 0) | (t ? 4 : 0);
					m_interrupt_pending = true;
					const unsigned lower = (op >> 25) & 0x7f;
					const bool branch = !(op & OP_UPPER_I) && (lower == 0x20 || lower == 0x21 || lower == 0x24 || lower == 0x25 || lower == 0x28 || lower == 0x29 || (lower >= 0x2c && lower <= 0x2f));
					m_end_delay = branch ? 1 : 0;
					m_draining = !branch;
				}
			}
		}
		++m_pipeline.cycle;
		--m_icount;
	}
	m_pipeline.retire(m_vfr, m_status_flag, m_mac_flag, m_q, m_vcr);
}

void sonyvu_device::execute_upper(const uint32_t op)
{
	const auto decoded = ps2vu::decode_fmac(op, false);
	if (decoded)
	{
		m_pipeline.fmac(decoded, m_vfr, m_acc, m_q);
		return;
	}

	switch (op & 0x3f)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: // ADDbc
			fatalerror("%s: unsupported VU instruction ADDbc\n", machine().describe_context());
		case 0x04: case 0x05: case 0x06: case 0x07: // SUBbc
			fatalerror("%s: unsupported VU instruction SUBbc\n", machine().describe_context());
		case 0x0c: case 0x0d: case 0x0e: case 0x0f: // MSUBbc
			fatalerror("%s: unsupported VU instruction MSUBbc\n", machine().describe_context());
		case 0x10: case 0x11: case 0x12: case 0x13: // MAXbc
			execute_macro((op & 63) >= 0x3c ? op & ~0x400U : op);
			break;
		case 0x14: case 0x15: case 0x16: case 0x17: // MINIbc
			execute_macro((op & 63) >= 0x3c ? op & ~0x400U : op);
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: // MULbc
			fatalerror("%s: unsupported VU instruction MULbc\n", machine().describe_context());
		case 0x1c: // MULq
			fatalerror("%s: unsupported VU instruction MULq\n", machine().describe_context());
		case 0x1d: // MAXi
			fatalerror("%s: unsupported VU instruction MAXi\n", machine().describe_context());
		case 0x1e: // MULi
			fatalerror("%s: unsupported VU instruction MULi\n", machine().describe_context());
		case 0x1f: // MINIi
			fatalerror("%s: unsupported VU instruction MINIi\n", machine().describe_context());
		case 0x20: // ADDq
			fatalerror("%s: unsupported VU instruction ADDq\n", machine().describe_context());
		case 0x21: // MADDq
			fatalerror("%s: unsupported VU instruction MADDq\n", machine().describe_context());
		case 0x22: // ADDi
			fatalerror("%s: unsupported VU instruction ADDi\n", machine().describe_context());
		case 0x23: // MADDi
			fatalerror("%s: unsupported VU instruction MADDi\n", machine().describe_context());
		case 0x24: // SUBq
			fatalerror("%s: unsupported VU instruction SUBq\n", machine().describe_context());
		case 0x25: // MSUBq
			fatalerror("%s: unsupported VU instruction MSUBq\n", machine().describe_context());
		case 0x26: // SUBi
			fatalerror("%s: unsupported VU instruction SUBi\n", machine().describe_context());
		case 0x27: // MSUBi
			fatalerror("%s: unsupported VU instruction MSUBi\n", machine().describe_context());
		case 0x28: // ADD
			fatalerror("%s: unsupported VU instruction ADD\n", machine().describe_context());
		case 0x29: // MADD
			fatalerror("%s: unsupported VU instruction MADD\n", machine().describe_context());
		case 0x2a: // MUL
			fatalerror("%s: unsupported VU instruction MUL\n", machine().describe_context());
		case 0x2b: // MAX
			fatalerror("%s: unsupported VU instruction MAX\n", machine().describe_context());
		case 0x2c: // SUB
			fatalerror("%s: unsupported VU instruction SUB\n", machine().describe_context());
		case 0x2d: // MSUB
			fatalerror("%s: unsupported VU instruction MSUB\n", machine().describe_context());
		case 0x2e: // OPMSUB
			fatalerror("%s: unsupported VU instruction OPMSUB\n", machine().describe_context());
		case 0x2f: // MINI
			fatalerror("%s: unsupported VU instruction MINI\n", machine().describe_context());
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		{
			const uint8_t type2_op = ((op & 0x3c0) >> 4) | (op & 3);
			switch (type2_op)
			{
				case 0x00: case 0x01: case 0x02: case 0x03: // ADDAbc
					fatalerror("%s: unsupported VU instruction ADDAb\n", machine().describe_context());
				case 0x04: case 0x05: case 0x06: case 0x07: // SUBAbc
					fatalerror("%s: unsupported VU instruction SUBAbc\n", machine().describe_context());
				case 0x0c: case 0x0d: case 0x0e: case 0x0f: // MSUBAbc
					fatalerror("%s: unsupported VU instruction MSUBAbc\n", machine().describe_context());
				case 0x10: // ITOF0
					fatalerror("%s: unsupported VU instruction ITOF0\n", machine().describe_context());
				case 0x11: // ITOF4
					fatalerror("%s: unsupported VU instruction ITOF4\n", machine().describe_context());
				case 0x12: // ITOF12
					fatalerror("%s: unsupported VU instruction ITOF12\n", machine().describe_context());
				case 0x13: // ITOF15
					fatalerror("%s: unsupported VU instruction ITOF15\n", machine().describe_context());
				case 0x14: // FTOI0
					execute_macro((op & 63) >= 0x3c ? op & ~0x400U : op);
					break;
				case 0x15: // FTOI4
					execute_macro((op & 63) >= 0x3c ? op & ~0x400U : op);
					break;
				case 0x16: // FTOI12
					fatalerror("%s: unsupported VU instruction FTOI12\n", machine().describe_context());
				case 0x17: // FTOI15
					fatalerror("%s: unsupported VU instruction FTOI15\n", machine().describe_context());
				case 0x1c: // MULAq
					fatalerror("%s: unsupported VU instruction MULAq\n", machine().describe_context());
				case 0x1d: // ABS
					fatalerror("%s: unsupported VU instruction ABS\n", machine().describe_context());
				case 0x1e: // MULAi
					fatalerror("%s: unsupported VU instruction MULAi\n", machine().describe_context());
				case 0x1f: // CLIP
					fatalerror("%s: unsupported VU instruction CLIP\n", machine().describe_context());
				case 0x20: // ADDAq
					fatalerror("%s: unsupported VU instruction ADDAq\n", machine().describe_context());
				case 0x21: // MADDAq
					fatalerror("%s: unsupported VU instruction MADDAq\n", machine().describe_context());
				case 0x22: // ADDAi
					fatalerror("%s: unsupported VU instruction ADDAi\n", machine().describe_context());
				case 0x23: // MADDAi
					fatalerror("%s: unsupported VU instruction MADDAi\n", machine().describe_context());
				case 0x24: // SUBAq
					fatalerror("%s: unsupported VU instruction SUBAq\n", machine().describe_context());
				case 0x25: // MSUBAq
					fatalerror("%s: unsupported VU instruction MSUBAq\n", machine().describe_context());
				case 0x26: // SUBAi
					fatalerror("%s: unsupported VU instruction SUBAi\n", machine().describe_context());
				case 0x27: // MSUBAi
					fatalerror("%s: unsupported VU instruction MSUBAi\n", machine().describe_context());
				case 0x28: // ADDA
					fatalerror("%s: unsupported VU instruction ADDA\n", machine().describe_context());
				case 0x29: // MADDA
					fatalerror("%s: unsupported VU instruction MADDA\n", machine().describe_context());
				case 0x2a: // MULA
					fatalerror("%s: unsupported VU instruction MULA\n", machine().describe_context());
				case 0x2c: // SUBA
					fatalerror("%s: unsupported VU instruction SUBA\n", machine().describe_context());
				case 0x2d: // MSUBA
					fatalerror("%s: unsupported VU instruction MSUBA\n", machine().describe_context());
				case 0x2e: // OPMULA
					fatalerror("%s: unsupported VU instruction OPMULA\n", machine().describe_context());
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
			{
				const uint32_t addr = uint32_t(m_vcr[rs] + immediate_s11(op)) << 4;
				if (rt && rt != m_upper_dest)
				{
					const unsigned slot = m_pipeline.enqueue(rt, dest);
					for (unsigned field = 0; field < 4; ++field)
					{
						if (dest & (8 >> field))
							m_pipeline.value[slot][field] = read_data(addr + field * 4);
					}
				}
				break;
			}
		case 0x01: // SQ
			{
				const uint32_t addr = uint32_t(m_vcr[rt] + immediate_s11(op)) << 4;
				for (unsigned field = 0; field < 4; ++field)
				{
					if (dest & (8 >> field))
						write_data(addr + field * 4, vf_r(rs, field));
				}
				break;
			}
		case 0x04: // ILW
			fatalerror("%s: unsupported VU instruction ILW\n", machine().describe_context());
		case 0x05: // ISW
			fatalerror("%s: unsupported VU instruction ISW\n", machine().describe_context());
		case 0x08: // IADDIU
			if (rt)
				write_vi(rt, uint16_t(m_vcr[rs] + (BIT(op, 21, 4) << 11) + (op & 0x7ff)), rt == rs);
			break;
		case 0x09: // ISUBIU
			write_vi(rt, uint16_t(m_vcr[rs] - ((BIT(op, 21, 4) << 11) | (op & 0x7ff))), rt == rs);
			break;
		case 0x10: // FCEQ
			fatalerror("%s: unsupported VU instruction FCEQ\n", machine().describe_context());
		case 0x11: // FCSET
			fatalerror("%s: unsupported VU instruction FCSET\n", machine().describe_context());
		case 0x12: // FCAND
			fatalerror("%s: unsupported VU instruction FCAND\n", machine().describe_context());
		case 0x13: // FCOR
			fatalerror("%s: unsupported VU instruction FCOR\n", machine().describe_context());
		case 0x14: // FSEQ
			fatalerror("%s: unsupported VU instruction FSEQ\n", machine().describe_context());
		case 0x15: // FSSET
			fatalerror("%s: unsupported VU instruction FSSET\n", machine().describe_context());
		case 0x16: // FSAND
			fatalerror("%s: unsupported VU instruction FSAND\n", machine().describe_context());
		case 0x17: // FSOR
			fatalerror("%s: unsupported VU instruction FSOR\n", machine().describe_context());
		case 0x18: // FMEQ
			fatalerror("%s: unsupported VU instruction FMEQ\n", machine().describe_context());
		case 0x1a: // FMAND
			if (rt)
			{
				m_vcr[rt] = m_mac_flag & m_vcr[rs];
				// Flag transfers are visible to the immediately following branch.
				m_vi_chain[rt] = 0;
			}
			break;
		case 0x1b: // FMOR
			fatalerror("%s: unsupported VU instruction FMOR\n", machine().describe_context());
		case 0x1c: // FCGET
			fatalerror("%s: unsupported VU instruction FCGET\n", machine().describe_context());
		case 0x20: // B
			m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x21: // BAL
			if (rt)
				m_vcr[rt] = uint16_t(m_link_pc >> 3);
			m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x24: // JR
			m_delay_pc = (m_vcr[rs] << 3) & m_mem_mask;
			break;
		case 0x25: // JALR
		{
			const uint32_t target = (m_vcr[rs] << 3) & m_mem_mask;
			if (rt)
				m_vcr[rt] = uint16_t(m_link_pc >> 3);
			m_delay_pc = target;
			break;
		}
		case 0x28: // IBEQ
			if (branch_vi(rs) == branch_vi(rt))
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x29: // IBNE
			if (branch_vi(rs) != branch_vi(rt))
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x2c: // IBLTZ
			if ((int16_t)branch_vi(rs) < 0)
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x2d: // IBGTZ
			if ((int16_t)branch_vi(rs) > 0)
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x2e: // IBLEZ
			if ((int16_t)branch_vi(rs) <= 0)
				m_delay_pc = (m_pc + immediate_s11(op) * 8) & m_mem_mask;
			break;
		case 0x2f: // IBGEZ
			if ((int16_t)branch_vi(rs) >= 0)
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
							fatalerror("%s: unsupported VU instruction MOVE\n", machine().describe_context());
						}
						break;
					case 0x31: // MR32
						fatalerror("%s: unsupported VU instruction MR32\n", machine().describe_context());
					case 0x34: // LQI
					{
						if (rt && rt != m_upper_dest)
						{
							const unsigned slot = m_pipeline.enqueue(rt, dest);
							const uint32_t addr = m_vcr[rs] << 4;
							for (int field = 0; field < 4; field++)
							{
								if (BIT(op, 24-field))
									m_pipeline.value[slot][field] = read_data(addr + field * 4);
							}
						}
						if (rs)
							write_vi(rs, uint16_t(m_vcr[rs] + 1), true);
						break;
					}
					case 0x35: // SQI
					{
						const uint32_t addr = m_vcr[rt] << 4;
						for (int field = 0; field < 4; field++)
						{
							if (BIT(op, 24-field))
								write_data(addr + field * 4, std::bit_cast<uint32_t>(m_vfr[rs][field]));
						}
						if (rt)
							write_vi(rt, uint16_t(m_vcr[rt] + 1), true);
						break;
					}
					case 0x36: // LQD
						fatalerror("%s: unsupported VU instruction LQD\n", machine().describe_context());
					case 0x37: // SQD
						fatalerror("%s: unsupported VU instruction SQD\n", machine().describe_context());
					case 0x38: // DIV
						execute_macro(op);
						break;
					case 0x39: // SQRT
						execute_macro(op);
						break;
					case 0x3a: // RSQRT
						fatalerror("%s: unsupported VU instruction RSQRT\n", machine().describe_context());
					case 0x3b: // WAITQ
						execute_macro(op);
						break;
					case 0x3c: // MTIR
						if (rt)
							write_vi(rt, uint16_t(std::bit_cast<uint32_t>(m_vfr[rs][fsf])), false);
						break;
					case 0x3d: // MFIR
						if (rt && rt != m_upper_dest)
						{
							const unsigned slot = m_pipeline.enqueue(rt, dest);
							int32_t value = (int16_t)(m_vcr[rs] & 0xffff);
							for (int field = 0; field < 4; field++)
							{
								if (BIT(op, 24-field))
								{
									m_pipeline.value[slot][field] = value;
								}
							}
						}
						break;
					case 0x3e: // ILWR
						if (rt && dest)
						{
							// Multiple selected components are architecturally undefined.
							unsigned field = 0;
							while (!(dest & (8 >> field)))
								++field;
							m_pipeline.vi_value[rt] = read_data((m_vcr[rs] << 4) + field * 4);
							m_pipeline.vi_due[rt] = m_pipeline.cycle + 4;
							m_vi_chain[rt] = 0;
						}
						break;
					case 0x3f: // ISWR
						fatalerror("%s: unsupported VU instruction ISWR\n", machine().describe_context());
					case 0x40: // RNEXT
						fatalerror("%s: unsupported VU instruction RNEXT\n", machine().describe_context());
					case 0x41: // RGET
						fatalerror("%s: unsupported VU instruction RGET\n", machine().describe_context());
					case 0x42: // RINIT
						fatalerror("%s: unsupported VU instruction RINIT\n", machine().describe_context());
					case 0x43: // RXOR
						fatalerror("%s: unsupported VU instruction RXOR\n", machine().describe_context());
					case 0x64: // MFP
						fatalerror("%s: unsupported VU instruction MFP\n", machine().describe_context());
					case 0x68: // XTOP
						write_vi(rt, vif_top(false), false);
						break;
					case 0x69: // XITOP
						write_vi(rt, vif_top(true), false);
						break;
					case 0x6c: // XGKICK
						execute_xgkick(rs);
						break;
					case 0x70: // ESADD
						fatalerror("%s: unsupported VU instruction ESADD\n", machine().describe_context());
					case 0x71: // ERSADD
						fatalerror("%s: unsupported VU instruction ERSADD\n", machine().describe_context());
					case 0x72: // ELENG
						fatalerror("%s: unsupported VU instruction ELENG\n", machine().describe_context());
					case 0x73: // ERLENG
						fatalerror("%s: unsupported VU instruction ERLENG\n", machine().describe_context());
					case 0x74: // EATANxy
						fatalerror("%s: unsupported VU instruction EATANxy\n", machine().describe_context());
					case 0x75: // EATANxz
						fatalerror("%s: unsupported VU instruction EATANxz\n", machine().describe_context());
					case 0x76: // ESUM
						fatalerror("%s: unsupported VU instruction ESUM\n", machine().describe_context());
					case 0x78: // ESQRT
						fatalerror("%s: unsupported VU instruction ESQRT\n", machine().describe_context());
					case 0x79: // ERSQRT
						fatalerror("%s: unsupported VU instruction ERSQRT\n", machine().describe_context());
					case 0x7a: // ERCPR
						fatalerror("%s: unsupported VU instruction ERCPR\n", machine().describe_context());
					case 0x7b: // WAITP
						fatalerror("%s: unsupported VU instruction WAITP\n", machine().describe_context());
					case 0x7c: // ESIN
						fatalerror("%s: unsupported VU instruction ESIN\n", machine().describe_context());
					case 0x7d: // EATAN
						fatalerror("%s: unsupported VU instruction EATAN\n", machine().describe_context());
					case 0x7e: // EEXP
						fatalerror("%s: unsupported VU instruction EEXP\n", machine().describe_context());
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
							write_vi(rd, uint16_t(m_vcr[rs] + m_vcr[rt]), rd == rs || rd == rt);
						break;
					case 0x31: // ISUB
						if (rd)
							write_vi(rd, uint16_t(m_vcr[rs] - m_vcr[rt]), rd == rs || rd == rt);
						break;
					case 0x32: // IADDI
						write_vi(rt, uint16_t(m_vcr[rs] + int(rd ^ 16) - 16), rt == rs);
						break;
					case 0x34: // IAND
						if (rd)
							write_vi(rd, uint16_t(m_vcr[rs] & m_vcr[rt]), rd == rs || rd == rt);
						break;
					case 0x35: // IOR
						if (rd)
							write_vi(rd, uint16_t(m_vcr[rs] | m_vcr[rt]), rd == rs || rd == rt);
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
	map(0x0000, 0x0fff).mirror(0x3000).ram().share(m_vu_mem);
	map(0x4000, 0x43ff).mirror(0x3c00).rw(m_vu1, FUNC(sonyvu1_device::reg_r), FUNC(sonyvu1_device::reg_w));
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
	state_add(SONYVU0_VPU_STAT, "VPU_STAT", m_vpu_stat).callexport();
}

void sonyvu0_device::device_reset()
{
	sonyvu_device::device_reset();

	m_cmsar0 = 0;
	m_control = 0;
	m_vpu_stat = 0;
	m_cmsar1 = 0;
}

void sonyvu0_device::execute_xgkick(uint32_t rs)
{
	fatalerror("Unsupported VU0 instruction: XGKICK\n");
}

void sonyvu1_device::device_start()
{
	sonyvu_device::device_start();

	save_item(NAME(m_p));
	save_item(NAME(m_kick_pending));
	save_item(NAME(m_kick_address));

	state_add(SONYVU1_P, "P", *(uint32_t*)&m_p).formatstr("%17s");
}

void sonyvu1_device::device_reset()
{
	sonyvu_device::device_reset();

	m_p = 0.0f;
	m_kick_pending = false;
	m_kick_address = 0;
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
	map(0x0000, 0x3fff).ram().share(m_micro_mem);
}

void sonyvu1_device::vu_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share(m_vu_mem);
}

uint32_t sonyvu_device::read_data(uint32_t address)
{
	return m_vu_space->read_dword(address);
}

uint64_t sonyvu_device::micro_r(offs_t offset)
{
	return m_micro_mem[offset & (m_mem_mask >> 3)];
}

void sonyvu_device::micro_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	uint64_t &word = m_micro_mem[offset & (m_mem_mask >> 3)];
	word = (word & ~mem_mask) | (data & mem_mask);
}

uint64_t sonyvu_device::data_r(offs_t offset)
{
	return uint64_t(read_data(offset * 8)) | (uint64_t(read_data(offset * 8 + 4)) << 32);
}

void sonyvu_device::data_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (uint32_t(mem_mask))
		m_vu_space->write_dword(offset * 8, uint32_t(data), uint32_t(mem_mask));
	if (uint32_t(mem_mask >> 32))
		m_vu_space->write_dword(offset * 8 + 4, uint32_t(data >> 32), uint32_t(mem_mask >> 32));
}

uint32_t sonyvu_device::vf_r(unsigned reg, unsigned field) const
{
	return reg ? std::bit_cast<uint32_t>(m_vfr[reg][field]) : field == 3 ? 0x3f800000 : 0;
}

void sonyvu_device::vf_w(unsigned reg, unsigned field, uint32_t data)
{
	if (reg)
		m_vfr[reg][field] = std::bit_cast<float>(data);
}

uint32_t sonyvu_device::control_r(unsigned reg) const
{
	if (reg < 16)
		return reg ? uint16_t(m_vcr[reg]) : 0;
	switch (reg)
	{
		case 16: return m_status_flag;
		case 17: return m_mac_flag;
		case 18: return m_clip_flag;
		case 20: return m_r & 0x7fffff;
		case 21: return std::bit_cast<uint32_t>(m_i);
		case 22: return std::bit_cast<uint32_t>(m_q);
		case 26: return (m_pc & m_mem_mask) >> 3;
		default: return 0;
	}
}

void sonyvu_device::control_w(unsigned reg, uint32_t data)
{
	if (reg < 16)
	{
		if (reg)
		{
			m_vcr[reg] = uint16_t(data);
			m_vi_chain[reg] = 0;
		}
		return;
	}
	switch (reg)
	{
		case 16: m_status_flag = (m_status_flag & 0x3f) | (data & 0xfc0); break;
		case 18: m_clip_flag = data & 0xffffff; break;
		case 20: m_r = data & 0x7fffff; break;
		case 21: m_i = std::bit_cast<float>(data); break;
		case 22: m_q = std::bit_cast<float>(data); break;
		default: break; // MAC/TPC are read-only; unused slots are reserved.
	}
}

uint32_t sonyvu0_device::control_r(unsigned reg) const
{
	switch (reg)
	{
		case 27: return m_cmsar0;
		case 28: return m_control;
		case 29: return vpu_status();
		case 31: return 0; // CMSAR1 is write-only.
		default: return sonyvu_device::control_r(reg);
	}
}

void sonyvu0_device::control_w(unsigned reg, uint32_t data)
{
	switch (reg)
	{
		case 27: m_cmsar0 = data & 0xffff; break;
		case 28:
			debug_control_w(data);
			if (data & 2)
			{
				reset_control();
				m_control &= ~0x0c;
			}
			else if (data & 1)
			{
				force_break();
			}
			if (data & 0x200)
			{
				m_vu1->reset_control();
				m_control &= ~0xc00;
			}
			else if (data & 0x100)
			{
				m_vu1->force_break();
			}
			break;
		case 29: break;
		case 31:
			if (!m_vu1->running())
			{
				m_cmsar1 = data & 0xffff;
				m_vu1->start(m_cmsar1 << 3);
			}
			break;
		default: sonyvu_device::control_w(reg, data); break;
	}
}

void sonyvu_device::synchronize_macro(uint64_t cycle)
{
	if (!m_running)
	{
		m_pipeline.cycle = std::max(m_pipeline.cycle, cycle);
		m_pipeline.retire(m_vfr, m_status_flag, m_mac_flag, m_q, m_vcr);
	}
}

void sonyvu_device::force_break()
{
	m_running = m_draining = m_mbit = m_interrupt_pending = false;
	m_stop_flags = 8;
	m_end_delay = 0;
	m_delay_pc = ~0U;
	m_pipeline.count = m_pipeline.head = m_pipeline.tail = 0;
	m_pipeline.q_due = 0;
	std::fill(std::begin(m_pipeline.vi_due), std::end(m_pipeline.vi_due), 0);
	std::fill(std::begin(m_vi_chain), std::end(m_vi_chain), 0);
}

void sonyvu_device::reset_control()
{
	// FBRST resets execution and control, preserving VF/VI/ACC and both RAMs.
	force_break();
	m_stop_flags = m_debug_control = 0;
	m_status_flag = m_mac_flag = m_clip_flag = 0;
}

void sonyvu1_device::force_break()
{
	sonyvu_device::force_break();
	m_kick_pending = false;
}

void sonyvu1_device::reset_control()
{
	sonyvu_device::reset_control();
	m_kick_pending = false;
}

void sonyvu_device::write_data(uint32_t address, uint32_t data)
{
	m_vu_space->write_dword(address, data);
}

// VU1 registers appear in VU0 data space as 16-byte slots, integer and control registers in the low word
uint32_t sonyvu1_device::reg_r(offs_t offset)
{
	if (offset < 0x80)
		return vf_r(offset >> 2, offset & 3);
	if (offset & 3)
		return 0;
	if (offset == 0xdc)
		return std::bit_cast<uint32_t>(m_p);
	return control_r((offset - 0x80) >> 2);
}

void sonyvu1_device::reg_w(offs_t offset, uint32_t data)
{
	if (offset < 0x80)
		vf_w(offset >> 2, offset & 3, data);
	else if (!(offset & 3))
	{
		if (offset == 0xdc)
			m_p = std::bit_cast<float>(data);
		else
			control_w((offset - 0x80) >> 2, data);
	}
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
	// latch once; the next pair waits for the GIF
	m_kick_address = m_vcr[rs];
	m_kick_pending = true;
	service_xgkick();
}

bool sonyvu1_device::service_xgkick()
{
	if (m_kick_pending && m_gs->interface()->path1_available())
	{
		m_gs->interface()->kick_path1(m_kick_address);
		m_kick_pending = false;
	}
	return !m_kick_pending;
}

void sonyvu_device::start(uint32_t address)
{
	if (m_running)
		return;
	m_pc = address & m_mem_mask;
	m_delay_pc = ~0U;
	m_end_delay = 0;
	m_draining = m_mbit = m_interrupt_pending = false;
	m_stop_flags = 0;
	std::fill(std::begin(m_vi_chain), std::end(m_vi_chain), 0);
	m_running = true;
}

void sonyvu0_device::debug_control_w(uint32_t data)
{
	m_control = data & 0xc0c;
	set_debug_control(data);
	m_vu1->set_debug_control(data >> 8);
}

int16_t sonyvu_device::immediate_s11(const uint32_t op)
{
	int16_t sval = (int16_t)(op << 5);
	return sval >> 5;
}

bool sonyvu_device::macro_hazard(uint32_t op) const
{
	const auto d = ps2vu::decode_fmac(op, true);
	if (d)
		return m_pipeline.hazard(d);
	const unsigned function = op & 63, ext = ((op >> 4) & 0x7c) | (op & 3);
	const unsigned rs = (op >> 11) & 31, rt = (op >> 16) & 31;
	const unsigned mask = (op >> 21) & 15;
	unsigned rs_mask = 0, rt_mask = 0;
	if (function >= 0x10 && function <= 0x17) // MIN/MAX broadcast
	{
		rs_mask = mask;
		rt_mask = mask ? 8 >> (op & 3) : 0;
	}
	else if (function >= 0x3c)
	{
		switch (ext)
		{
			case 0x14: case 0x15: case 0x30: case 0x35: rs_mask = mask; break;
			case 0x31: rs_mask = ((mask << 3) | (mask >> 1)) & 15; break;
			case 0x38: rs_mask = 8 >> ((op >> 21) & 3); [[fallthrough]];
			case 0x39: rt_mask = 8 >> ((op >> 23) & 3); [[fallthrough]];
			case 0x3b:
				if (m_pipeline.q_due > m_pipeline.cycle)
					return true;
				break;
		}
	}
	return m_pipeline.ready(rs, rs_mask) > m_pipeline.cycle || m_pipeline.ready(rt, rt_mask) > m_pipeline.cycle;
}

bool sonyvu_device::execute_macro(uint32_t op)
{
	const auto decoded = ps2vu::decode_fmac(op, true);
	if (decoded)
	{
		m_pipeline.fmac(decoded, m_vfr, m_acc, m_q);
		return true;
	}
	const int rd   = (op >>  6) & 31;
	const int rs   = (op >> 11) & 31;
	const int rt   = (op >> 16) & 31;
	const int ext = ((op >> 4) & 0x7c) | (op & 3);

	switch (op & 0x3f)
	{
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			fatalerror("%s: unsupported VU instruction VMSUBbc\n", machine().describe_context());
		case 0x10: case 0x11: case 0x12: case 0x13: /* VMAXbc */
			if (rd)
			{
				const uint32_t bc = op & 3;
				float *fs = m_vfr[rs];
				const uint32_t ft = std::bit_cast<uint32_t>(m_vfr[rt][bc]);
				const unsigned slot = m_pipeline.enqueue(rd, (op >> 21) & 15);
				for (int field = 0; field < 4; field++)
				{
					if (BIT(op, 24-field))
					{
						m_pipeline.value[slot][field] = ps2vu::maximum(std::bit_cast<uint32_t>(fs[field]), ft);
					}
				}
			}
			break;
		case 0x14: case 0x15: case 0x16: case 0x17: /* VMINIbc */
			if (rd)
			{
				const uint32_t bc = op & 3;
				float *fs = m_vfr[rs];
				const uint32_t ft = std::bit_cast<uint32_t>(m_vfr[rt][bc]);
				const unsigned slot = m_pipeline.enqueue(rd, (op >> 21) & 15);
				for (int field = 0; field < 4; field++)
				{
					if (BIT(op, 24-field))
					{
						m_pipeline.value[slot][field] = ps2vu::minimum(std::bit_cast<uint32_t>(fs[field]), ft);
					}
				}
			}
			break;
		case 0x1d: fatalerror("%s: unsupported VU instruction VMAXi\n", machine().describe_context());
		case 0x1e: fatalerror("%s: unsupported VU instruction VMULi\n", machine().describe_context());
		case 0x1f: fatalerror("%s: unsupported VU instruction VMINIi\n", machine().describe_context());
		case 0x21: fatalerror("%s: unsupported VU instruction VMADDq\n", machine().describe_context());
		case 0x22: fatalerror("%s: unsupported VU instruction VADDi\n", machine().describe_context());
		case 0x23: fatalerror("%s: unsupported VU instruction VMADDi\n", machine().describe_context());
		case 0x24: fatalerror("%s: unsupported VU instruction VSUBq\n", machine().describe_context());
		case 0x25: fatalerror("%s: unsupported VU instruction VMSUBq\n", machine().describe_context());
		case 0x26: fatalerror("%s: unsupported VU instruction VSUBi\n", machine().describe_context());
		case 0x27: fatalerror("%s: unsupported VU instruction VMSUBi\n", machine().describe_context());
		case 0x29: fatalerror("%s: unsupported VU instruction VMADD\n", machine().describe_context());
		case 0x2b: fatalerror("%s: unsupported VU instruction VMAX\n", machine().describe_context());
		case 0x2d: fatalerror("%s: unsupported VU instruction VMSUB\n", machine().describe_context());
		case 0x2f: fatalerror("%s: unsupported VU instruction VMINI\n", machine().describe_context());
		case 0x30:
			if (rd)
			{
				m_vcr[rd] = (m_vcr[rs] + m_vcr[rt]) & 0xffff;
			}
			break;
		case 0x31: fatalerror("%s: unsupported VU instruction VISUB\n", machine().describe_context());
		case 0x32: fatalerror("%s: unsupported VU instruction VIADDI\n", machine().describe_context());
		case 0x34: fatalerror("%s: unsupported VU instruction VIAND\n", machine().describe_context());
		case 0x35: fatalerror("%s: unsupported VU instruction VIOR\n", machine().describe_context());
		case 0x38: fatalerror("%s: unsupported VU instruction VCALLMS\n", machine().describe_context());
		case 0x39: fatalerror("%s: unsupported VU instruction VCALLMSR\n", machine().describe_context());
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			switch (ext)
			{
				case 0x00: case 0x01: case 0x02: case 0x03:
					fatalerror("%s: unsupported VU instruction VADDAbc\n", machine().describe_context());
				case 0x04: case 0x05: case 0x06: case 0x07:
					fatalerror("%s: unsupported VU instruction VSUBAbc\n", machine().describe_context());
				case 0x0c: case 0x0d: case 0x0e: case 0x0f:
					fatalerror("%s: unsupported VU instruction VMSUBAbc\n", machine().describe_context());
				case 0x10: fatalerror("%s: unsupported VU instruction VITOF0\n", machine().describe_context());
				case 0x11: fatalerror("%s: unsupported VU instruction VITOF4\n", machine().describe_context());
				case 0x12: fatalerror("%s: unsupported VU instruction VITOF12\n", machine().describe_context());
				case 0x13: fatalerror("%s: unsupported VU instruction VITOF15\n", machine().describe_context());
				case 0x14: /* VFTOI0 */
					if (rt)
					{
						float *fs = m_vfr[rs];
						const unsigned slot = m_pipeline.enqueue(rt, (op >> 21) & 15);
						for (int field = 0; field < 4; field++)
						{
							if (BIT(op, 24-field))
							{
								m_pipeline.value[slot][field] = ps2vu::ftoi(std::bit_cast<uint32_t>(fs[field]), 0);
							}
						}
					}
					break;
				case 0x15: /* VFTOI4 */
					if (rt)
					{
						float *fs = m_vfr[rs];
						const unsigned slot = m_pipeline.enqueue(rt, (op >> 21) & 15);
						for (int field = 0; field < 4; field++)
						{
							if (BIT(op, 24-field))
							{
								m_pipeline.value[slot][field] = ps2vu::ftoi(std::bit_cast<uint32_t>(fs[field]), 4);
							}
						}
					}
					break;
				case 0x16: fatalerror("%s: unsupported VU instruction VFTOI12\n", machine().describe_context());
				case 0x17: fatalerror("%s: unsupported VU instruction VFTOI15\n", machine().describe_context());
				case 0x1c: fatalerror("%s: unsupported VU instruction VMULAq\n", machine().describe_context());
				case 0x1d: fatalerror("%s: unsupported VU instruction VABS\n", machine().describe_context());
				case 0x1e: fatalerror("%s: unsupported VU instruction VMULAi\n", machine().describe_context());
				case 0x1f: fatalerror("%s: unsupported VU instruction VCLIP\n", machine().describe_context());
				case 0x20: fatalerror("%s: unsupported VU instruction VADDAq\n", machine().describe_context());
				case 0x21: fatalerror("%s: unsupported VU instruction VMADDAq\n", machine().describe_context());
				case 0x22: fatalerror("%s: unsupported VU instruction VADDAi\n", machine().describe_context());
				case 0x23: fatalerror("%s: unsupported VU instruction VMADDAi\n", machine().describe_context());
				case 0x24: fatalerror("%s: unsupported VU instruction VSUBAq\n", machine().describe_context());
				case 0x25: fatalerror("%s: unsupported VU instruction VMSUBAq\n", machine().describe_context());
				case 0x26: fatalerror("%s: unsupported VU instruction VSUBAi\n", machine().describe_context());
				case 0x27: fatalerror("%s: unsupported VU instruction VMSUBAi\n", machine().describe_context());
				case 0x28: fatalerror("%s: unsupported VU instruction VADDA\n", machine().describe_context());
				case 0x29: fatalerror("%s: unsupported VU instruction VMADDA\n", machine().describe_context());
				case 0x2a: fatalerror("%s: unsupported VU instruction VMULA\n", machine().describe_context());
				// 2b?
				case 0x2c: fatalerror("%s: unsupported VU instruction VSUBA\n", machine().describe_context());
				case 0x2d: fatalerror("%s: unsupported VU instruction VMSUBA\n", machine().describe_context());
				case 0x2f: /* VNOP */
					break;
				case 0x30: /* VMOVE */
					if (rt)
					{
						float *fs = m_vfr[rs];
						const unsigned slot = m_pipeline.enqueue(rt, (op >> 21) & 15);
						for (int field = 0; field < 4; field++)
						{
							if (BIT(op, 24-field))
							{
								m_pipeline.value[slot][field] = std::bit_cast<uint32_t>(fs[field]);
							}
						}
					}
					break;
				case 0x31: /* VMR32 */
					if (rt)
					{
						const auto fs = std::bit_cast<std::array<uint32_t, 4>>(m_vfr[rs]);
						const unsigned slot = m_pipeline.enqueue(rt, (op >> 21) & 15);
						for (int field = 0; field < 4; field++)
						{
							if (BIT(op, 24-field))
							{
								m_pipeline.value[slot][field] = fs[(field + 1) & 3];
							}
						}
					}
					break;
				// 32?
				// 33?
				case 0x34: fatalerror("%s: unsupported VU instruction VLQI\n", machine().describe_context());
				case 0x35: /* VSQI */
				{
					const uint32_t base = m_vcr[rt] << 4;
					const float *fs = m_vfr[rs];
					for (int field = 0; field < 4; field++)
					{
						if (BIT(op, 24-field))
						{
							write_data(base + field * 4, std::bit_cast<uint32_t>(fs[field]));
						}
					}
					if (rt)
					{
						m_vcr[rt]++;
						m_vcr[rt] &= 0xffff;
					}
					break;
				}
				case 0x36: fatalerror("%s: unsupported VU instruction VLQD\n", machine().describe_context());
				case 0x37: fatalerror("%s: unsupported VU instruction VSQD\n", machine().describe_context());
				case 0x38: /* VDIV */
					{
						const uint32_t fsf = (op >> 21) & 3;
						const uint32_t ftf = (op >> 23) & 3;
						const float *fs = m_vfr[rs];
						const float *ft = m_vfr[rt];
						uint32_t status = 0;
						m_pipeline.q_value = ps2vu::divide(std::bit_cast<uint32_t>(fs[fsf]),
							std::bit_cast<uint32_t>(ft[ftf]), status);
						m_pipeline.q_flags = status & 0x30;
						m_pipeline.q_due = m_pipeline.cycle + 7;
					}
					break;
				case 0x39: /* VSQRT */
					{
						const uint32_t ftf = (op >> 23) & 3;
						uint32_t status = 0;
						m_pipeline.q_value = ps2vu::square_root(std::bit_cast<uint32_t>(m_vfr[rt][ftf]), status);
						m_pipeline.q_flags = status & 0x30;
						m_pipeline.q_due = m_pipeline.cycle + 7;
					}
					break;
				case 0x3a: fatalerror("%s: unsupported VU instruction VRSQRT\n", machine().describe_context());
				case 0x3b: /* VWAITQ */
					// The instruction interlock waits for the pending FDIV result.
					break;
				case 0x3c: fatalerror("%s: unsupported VU instruction VMTIR\n", machine().describe_context());
				case 0x3d: fatalerror("%s: unsupported VU instruction VMFIR\n", machine().describe_context());
				case 0x3e: fatalerror("%s: unsupported VU instruction VILWR\n", machine().describe_context());
				case 0x3f: /* VISWR */
				{
					const uint32_t val = m_vcr[rt] & 0xffff;
					const uint32_t base = m_vcr[rs] << 4;
					for (int field = 0; field < 4; field++)
					{
						if (BIT(op, 24-field))
						{
							write_data(base + field * 4, val);
						}
					}
					break;
				}
				case 0x40: fatalerror("%s: unsupported VU instruction VRNEXT\n", machine().describe_context());
				case 0x41: fatalerror("%s: unsupported VU instruction VRGET\n", machine().describe_context());
				case 0x42: fatalerror("%s: unsupported VU instruction VRINIT\n", machine().describe_context());
				case 0x43: fatalerror("%s: unsupported VU instruction VRXOR\n", machine().describe_context());
				default:   return false;
			}
			break;
		default:
			return false;
	}
	return true;
}

uint16_t sonyvu_device::vif_top(bool itop) const
{
	fatalerror("VU0 XITOP requires VIF0 support\n");
}

uint16_t sonyvu1_device::vif_top(bool itop) const
{
	return itop ? m_vif->itop() : m_vif->top();
}

void sonyvu0_device::state_export(const device_state_entry &entry)
{
	if (entry.index() == SONYVU0_VPU_STAT)
		m_vpu_stat = vpu_status();
	else
		sonyvu_device::state_export(entry);
}
