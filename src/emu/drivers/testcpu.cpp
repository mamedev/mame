// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    testcpu.c

    Example driver for performing CPU stress tests.

**************************************************************************/


#include "emu.h"
#include "cpu/powerpc/ppc.h"


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
	testcpu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_cpu(*this, "maincpu"),
			m_ram(*this, "ram"),
			m_space(nullptr)
	{
	}

	// timer callback; used to wrest control of the system
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override
	{
		static const UINT32 sample_instructions[] =
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
		for (auto & sample_instruction : sample_instructions)
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
			printf("==================================================\n");
			printf("Initial state:\n");
			dump_state(true);

			// execute one instruction
			*m_cpu->m_icountptr = 0;
			m_cpu->run();

			// dump the final register state
			printf("Final state:\n");
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
		timer_set(attotime::zero);
	}

	// dump the current CPU state
	void dump_state(bool disassemble)
	{
		char buffer[256];
		UINT8 instruction[32];
		buffer[0] = 0;
		int bytes = 0;
		if (disassemble)
		{
			// fill in an array of bytes in the CPU's natural order
			int maxbytes = m_cpu->max_opcode_bytes();
			for (int bytenum = 0; bytenum < maxbytes; bytenum++)
				instruction[bytenum] = m_space->read_byte(RAM_BASE + bytenum);

			// disassemble the current instruction
			bytes = m_cpu->disassemble(buffer, RAM_BASE, instruction, instruction) & DASMFLAG_LENGTHMASK;
		}

		// output the registers
		printf("PC : %08X", UINT32(m_cpu->state_int(PPC_PC)));
		if (disassemble && bytes > 0)
		{
			printf(" => ");
			for (int bytenum = 0; bytenum < bytes; bytenum++)
				printf("%02X", instruction[bytenum]);
			printf("  %s", buffer);
		}
		printf("\n");
		for (int regnum = 0; regnum < 32; regnum++)
		{
			printf("R%-2d: %08X   ", regnum, UINT32(m_cpu->state_int(PPC_R0 + regnum)));
			if (regnum % 4 == 3) printf("\n");
		}
		printf("CR : %08X   LR : %08X   CTR: %08X   XER: %08X\n",
				UINT32(m_cpu->state_int(PPC_CR)), UINT32(m_cpu->state_int(PPC_LR)),
				UINT32(m_cpu->state_int(PPC_CTR)), UINT32(m_cpu->state_int(PPC_XER)));
		for (int regnum = 0; regnum < 32; regnum++)
		{
			printf("F%-2d: %10g   ", regnum, u2d(m_cpu->state_int(PPC_F0 + regnum)));
			if (regnum % 4 == 3) printf("\n");
		}
	}

	// report reads from anywhere
	READ64_MEMBER( general_r )
	{
		UINT64 fulloffs = offset;
		UINT64 result = fulloffs + (fulloffs << 8) + (fulloffs << 16) + (fulloffs << 24) + (fulloffs << 32);
		printf("Read from %08X & %08X%08X = %08X%08X\n", offset * 8, (int)((mem_mask&0xffffffff00000000LL) >> 32) , (int)(mem_mask&0xffffffff), (int)((result&0xffffffff00000000LL) >> 32), (int)(result&0xffffffff));
		return result;
	}

	// report writes to anywhere
	WRITE64_MEMBER( general_w )
	{
		printf("Write to %08X & %08X%08X = %08X%08X\n", offset * 8, (int)((mem_mask&0xffffffff00000000LL) >> 32) , (int)(mem_mask&0xffffffff), (int)((data&0xffffffff00000000LL) >> 32), (int)(data&0xffffffff));
	}

private:
	// internal state
	required_device<ppc603e_device> m_cpu;
	required_shared_ptr<UINT64> m_ram;
	address_space *m_space;
};



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( ppc_mem, AS_PROGRAM, 64, testcpu_state )
	AM_RANGE(RAM_BASE, RAM_BASE+7) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(general_r, general_w)
ADDRESS_MAP_END



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static MACHINE_CONFIG_START( testcpu, testcpu_state )

	// CPUs
	MCFG_CPU_ADD("maincpu", PPC603E, 66000000)
	MCFG_PPC_BUS_FREQUENCY(66000000)  // Multiplier 1, Bus = 66MHz, Core = 66MHz
	MCFG_CPU_PROGRAM_MAP(ppc_mem)
MACHINE_CONFIG_END



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( testcpu )
	ROM_REGION( 0x10, "user1", ROMREGION_ERASEFF )
ROM_END



//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

GAME( 2012, testcpu, 0, testcpu, 0, driver_device, 0, ROT0, "MAME", "CPU Tester", MACHINE_NO_SOUND )
