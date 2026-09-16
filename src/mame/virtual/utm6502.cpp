// license:BSD-3-Clause
// copyright-holders: Stuart Inglis

// 6502 interruptible access unit test

#include "emu.h"

#include "cpu/m6502/m6502.h"


namespace {

class utm6502_state : public driver_device
{
public:
	utm6502_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
	{
	}

	void utm6502(machine_config &config);

private:
	enum phase : u8
	{
		PHASE_RESET_LO,
		PHASE_RESET_HI,
		PHASE_OPCODE,
		PHASE_OPERAND_LO,
		PHASE_OPERAND_HI,
		PHASE_DATA,
		PHASE_NEXT_FETCH,
		PHASE_FINISHED
	};

	required_device<m6502_device> m_maincpu;
	required_shared_ptr<u8> m_ram;

	memory_passthrough_handler m_read_tap;
	emu_timer *m_slice_timer = nullptr;

	std::array<u64, 2> m_delay_cycles{};
	u64 m_base_cycle = 0;
	u64 m_opcode_cycle = 0;
	u64 m_last_read_cycle = 0;
	u32 m_delay_hits = 0;
	u16 m_last_read_address = 0;
	u8 m_last_read_data = 0;
	phase m_phase = PHASE_RESET_LO;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void program_map(address_map &map) ATTR_COLD;

	u32 data_read_delay(offs_t offset);
	void observe_read(offs_t offset, u8 data);
	void fail(char const *message);
	char const *phase_name() const;

	TIMER_CALLBACK_MEMBER(slice_tick);
};


void utm6502_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("ram");
}


u32 utm6502_state::data_read_delay(offs_t offset)
{
	if (offset == 0x1234)
	{
		if (m_delay_hits < m_delay_cycles.size())
			m_delay_cycles[m_delay_hits] = m_maincpu->total_cycles() - m_base_cycle;
		m_delay_hits++;
	}
	return 2;
}


void utm6502_state::observe_read(offs_t offset, u8 data)
{
	u64 const cycle = m_maincpu->total_cycles() - m_base_cycle;

	m_last_read_cycle = cycle;
	m_last_read_address = u16(offset);
	m_last_read_data = data;

	switch (m_phase)
	{
	case PHASE_RESET_LO:
		if ((offset == 0xfffc) && (data == 0x00))
			m_phase = PHASE_RESET_HI;
		break;

	case PHASE_RESET_HI:
		if ((offset == 0xfffd) && (data == 0xc0))
			m_phase = PHASE_OPCODE;
		break;

	case PHASE_OPCODE:
		if ((offset == 0xc000) && (data == 0xad))
		{
			m_opcode_cycle = cycle;
			m_phase = PHASE_OPERAND_LO;
		}
		break;

	case PHASE_OPERAND_LO:
		if ((cycle != (m_opcode_cycle + 1)) || (offset != 0xc001) || (data != 0x34))
			fail("expected low operand fetch at opcode+1");
		m_phase = PHASE_OPERAND_HI;
		break;

	case PHASE_OPERAND_HI:
		if ((cycle != (m_opcode_cycle + 2)) || (offset != 0xc002) || (data != 0x12))
			fail("expected high operand fetch at opcode+2");
		m_phase = PHASE_DATA;
		break;

	case PHASE_DATA:
		if (m_delay_hits != 2)
			fail("expected two delay-hook hits before final data read");
		if ((m_delay_cycles[0] != (m_opcode_cycle + 3)) || (m_delay_cycles[1] != (m_opcode_cycle + 5)))
			fail("delay-hook cycles did not match expected redo timing");
		if ((cycle != (m_opcode_cycle + 5)) || (offset != 0x1234) || (data != 0x56))
			fail("expected delayed data read at opcode+5");
		m_phase = PHASE_NEXT_FETCH;
		break;

	case PHASE_NEXT_FETCH:
		if ((cycle != (m_opcode_cycle + 6)) || (offset != 0xc003) || (data != 0xea))
			fail("expected next opcode fetch at opcode+6");
		m_phase = PHASE_FINISHED;
		break;

	case PHASE_FINISHED:
		osd_printf_info("PASS\n");
		machine().schedule_exit();
		break;
	}
}


void utm6502_state::fail(char const *message)
{
	throw emu_fatalerror(
			"%s; phase=%s delay_hits=%u delay0=%u delay1=%u last=%u:%04X:%02X opcode=%u",
			message,
			phase_name(),
			m_delay_hits,
			u32(m_delay_cycles[0]),
			u32(m_delay_cycles[1]),
			u32(m_last_read_cycle),
			m_last_read_address,
			m_last_read_data,
			u32(m_opcode_cycle));
}


char const *utm6502_state::phase_name() const
{
	switch (m_phase)
	{
	case PHASE_RESET_LO:   return "RESET_LO";
	case PHASE_RESET_HI:   return "RESET_HI";
	case PHASE_OPCODE:     return "OPCODE";
	case PHASE_OPERAND_LO: return "OPERAND_LO";
	case PHASE_OPERAND_HI: return "OPERAND_HI";
	case PHASE_DATA:       return "DATA";
	case PHASE_NEXT_FETCH: return "NEXT_FETCH";
	case PHASE_FINISHED:   return "FINISHED";
	}

	return "UNKNOWN";
}


TIMER_CALLBACK_MEMBER(utm6502_state::slice_tick)
{
	if ((m_maincpu->total_cycles() - m_base_cycle) > 32)
		fail("timed out waiting for expected read sequence");
}


void utm6502_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_read_before_delay(0x1234, 0x1234, ws_delay_delegate(*this, FUNC(utm6502_state::data_read_delay)));
	m_read_tap = program.install_read_tap(
			0x0000, 0xffff,
			"utm6502_read_tap",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
					observe_read(offset, data);
			},
			&m_read_tap);
	m_slice_timer = timer_alloc(FUNC(utm6502_state::slice_tick), this);
	save_item(NAME(m_delay_cycles));
	save_item(NAME(m_base_cycle));
	save_item(NAME(m_opcode_cycle));
	save_item(NAME(m_last_read_cycle));
	save_item(NAME(m_delay_hits));
	save_item(NAME(m_last_read_address));
	save_item(NAME(m_last_read_data));
}


void utm6502_state::machine_reset()
{
	std::fill_n(&m_ram[0], 0x10000, 0xea);

	m_delay_cycles.fill(0);
	m_base_cycle = 0;
	m_opcode_cycle = 0;
	m_last_read_cycle = 0;
	m_delay_hits = 0;
	m_last_read_address = 0;
	m_last_read_data = 0;
	m_phase = PHASE_RESET_LO;

	m_ram[0xc000] = 0xad; // LDA $1234
	m_ram[0xc001] = 0x34;
	m_ram[0xc002] = 0x12;
	m_ram[0xc003] = 0xea; // NOP
	m_ram[0x1234] = 0x56;
	m_ram[0xfffc] = 0x00;
	m_ram[0xfffd] = 0xc0;

	attotime const cycle = attotime::from_hz(m_maincpu->clock());
	m_slice_timer->adjust(cycle, 0, cycle);
}


void utm6502_state::utm6502(machine_config &config)
{
	M6502(config, m_maincpu, 1'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &utm6502_state::program_map);
}

} // anonymous namespace


ROM_START( utm6502 )
	ROM_REGION(0x10, "user1", ROMREGION_ERASEFF)
ROM_END


SYST(2026, utm6502, 0, 0, utm6502, 0, utm6502_state, empty_init, "MAME", "6502 interruptible access unit test", MACHINE_NO_SOUND_HW)
