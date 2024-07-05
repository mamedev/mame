// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    testcpu.cpp

    Example driver for performing CPU stress tests.

**************************************************************************/


#include "emu.h"
#include "cpu/powerpc/ppc.h"

#include <sstream>


namespace {

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define RAM_BASE 0x80000000



//**************************************************************************
//  DRIVER STATE
//**************************************************************************

class testcpu_state : public driver_device
{
public:
	// constructor
	testcpu_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_space(nullptr)
	{
	}

	void testcpu(machine_config &config);

private:
	class disasm_data_buffer : public util::disasm_interface::data_buffer
	{
	public:
		disasm_data_buffer(address_space &space) : m_space(space)
		{
		}

		virtual u8 r8(offs_t pc) const override { return m_space.read_byte(pc); }
		virtual u16 r16(offs_t pc) const override { return m_space.read_word(pc); }
		virtual u32 r32(offs_t pc) const override { return m_space.read_dword(pc); }
		virtual u64 r64(offs_t pc) const override { return m_space.read_qword(pc); }

	private:
		address_space &m_space;
	};

	// timer callback; used to wrest control of the system
	TIMER_CALLBACK_MEMBER(timer_tick)
	{
		static constexpr u32 sample_instructions[] =
		{
			0x3d40f900,     // li r10,0xf9000000
			0x394af000,     // addi r10,r10,-0x1000
			0x38600146,     // li r3,0x00000146
			0x38800004,     // li r4,0x00000004
			0x7c64572c,     // sthbrx r3,r4,r10
			0x38600000,     // li r3,0x00000000
			0x986a0070      // stb r3,0x0070(r10)
		};

		// iterate over instructions
		for (auto &sample_instruction : sample_instructions)
		{
			// write the instruction to execute, followed by a BLR which will terminate the
			// basic block in the DRC
			m_space->write_dword(RAM_BASE, sample_instruction);
			m_space->write_dword(RAM_BASE + 4, 0x4e800020);

			// initialize the register state
			m_cpu->set_state_int(PPC_PC, RAM_BASE);
			for (int regnum = 0; regnum < 32; regnum++)
				m_cpu->set_state_int(PPC_R0 + regnum, regnum | (regnum << 8) | (regnum << 16) | (regnum << 24));
			m_cpu->set_state_int(PPC_CR, 0);
			m_cpu->set_state_int(PPC_LR, 0x12345678);
			m_cpu->set_state_int(PPC_CTR, 0x1000);
			m_cpu->set_state_int(PPC_XER, 0);
			for (int regnum = 0; regnum < 32; regnum++)
			{
				double value = double(regnum | (regnum << 8) | (regnum << 16) | (regnum << 24));
				m_cpu->set_state_int(PPC_F0 + regnum, d2u(value));
			}

			// output initial state
			osd_printf_info("==================================================\n");
			osd_printf_info("Initial state:\n");
			dump_state(true);

			// execute one instruction
			*m_cpu->m_icountptr = 0;
			m_cpu->run();

			// dump the final register state
			osd_printf_info("Final state:\n");
			dump_state(false);
		}

		// all done; just bail
		throw emu_fatalerror(0, "All done");
	}

	// startup code; do basic configuration and set a timer to go off immediately
	virtual void machine_start() override
	{
		// find the CPU's address space
		m_space = &m_cpu->space(AS_PROGRAM);

		// configure DRC in the most compatible mode
		m_cpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

		// set a timer to go off right away
		timer_alloc(FUNC(testcpu_state::timer_tick), this)->adjust(attotime::zero);
	}

	// dump the current CPU state
	void dump_state(bool disassemble)
	{
		std::ostringstream buffer;
		int bytes = 0;
		if (disassemble)
		{
			// disassemble the current instruction
			disasm_data_buffer databuf(*m_space);
			bytes = m_cpu->get_disassembler().disassemble(buffer, RAM_BASE, databuf, databuf) & util::disasm_interface::LENGTHMASK;
		}

		// output the registers
		osd_printf_info("PC : %08X", u32(m_cpu->state_int(PPC_PC)));
		if (bytes > 0)
		{
			osd_printf_info(" => ");
			for (int bytenum = 0; bytenum < bytes; bytenum++)
				osd_printf_info("%02X", m_space->read_byte(RAM_BASE + bytenum));
			osd_printf_info("  %s", buffer.str());
		}
		osd_printf_info("\n");
		for (int regnum = 0; regnum < 32; regnum++)
		{
			osd_printf_info("R%-2d: %08X   ", regnum, u32(m_cpu->state_int(PPC_R0 + regnum)));
			if (regnum % 4 == 3) osd_printf_info("\n");
		}
		osd_printf_info("CR : %08X   LR : %08X   CTR: %08X   XER: %08X\n",
				u32(m_cpu->state_int(PPC_CR)), u32(m_cpu->state_int(PPC_LR)),
				u32(m_cpu->state_int(PPC_CTR)), u32(m_cpu->state_int(PPC_XER)));
		for (int regnum = 0; regnum < 32; regnum++)
		{
			osd_printf_info("F%-2d: %10g   ", regnum, u2d(m_cpu->state_int(PPC_F0 + regnum)));
			if (regnum % 4 == 3) osd_printf_info("\n");
		}
	}

	// report reads from anywhere
	u64 general_r(offs_t offset, u64 mem_mask = ~0)
	{
		u64 fulloffs = offset;
		u64 result = fulloffs + (fulloffs << 8) + (fulloffs << 16) + (fulloffs << 24) + (fulloffs << 32);
		osd_printf_info("Read from %08X & %016X = %016X\n", offset * 8, mem_mask, result);
		return result;
	}

	// report writes to anywhere
	void general_w(offs_t offset, u64 data, u64 mem_mask = ~0)
	{
		osd_printf_info("Write to %08X & %016X = %016X\n", offset * 8, mem_mask, data);
	}

	void ppc_mem(address_map &map);

	// internal state
	required_device<ppc603e_device> m_cpu;
	required_shared_ptr<u64> m_ram;
	address_space *m_space;
};



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void testcpu_state::ppc_mem(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(testcpu_state::general_r), FUNC(testcpu_state::general_w));
	map(RAM_BASE, RAM_BASE+7).ram().share("ram");
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void testcpu_state::testcpu(machine_config &config)
{
	// CPUs
	PPC603E(config, m_cpu, 66'000'000);
	m_cpu->set_bus_frequency(66'000'000);  // Multiplier 1, Bus = 66MHz, Core = 66MHz
	m_cpu->set_addrmap(AS_PROGRAM, &testcpu_state::ppc_mem);
}



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( testcpu )
	ROM_REGION( 0x10, "user1", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

GAME( 2012, testcpu, 0, testcpu, 0, testcpu_state, empty_init, ROT0, "MAME", "CPU Tester", MACHINE_NO_SOUND_HW )
