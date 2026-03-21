// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi, Ville Linde, Matthew Daniels
/********************************************************************************
 *
 * MB86235 "TGPx4" (c) Fujitsu
 *
 * TODO:
 * - rewrite ALU integer/floating point functions, use templates etc;
 * - rewrite fifo hookups, remove them from the actual opcodes.
 * - illegal delay slots unsupported, and no idea about what is supposed to happen;
 * - externalize PDR (error LED flags on Model 2);
 *
 ********************************************************************************/

#include "emu.h"
#include "mb86235.h"

#include "mb86235d.h"
#include "mb86235fe.h"

#include "emuopts.h"


#define ENABLE_DRC      0


#define CACHE_SIZE                      (1 * 1024 * 1024)
#define COMPILE_BACKWARDS_BYTES         128
#define COMPILE_FORWARDS_BYTES          512
#define COMPILE_MAX_INSTRUCTIONS        ((COMPILE_BACKWARDS_BYTES/4) + (COMPILE_FORWARDS_BYTES/4))
#define COMPILE_MAX_SEQUENCE            64



DEFINE_DEVICE_TYPE(MB86235, mb86235_device, "mb86235", "Fujitsu MB86235 \"TGPx4\"")


void mb86235_device::internal_abus(address_map &map)
{
	map(0x000000, 0x0003ff).ram();
}

void mb86235_device::internal_bbus(address_map &map)
{
	map(0x000000, 0x0003ff).ram();
}



/* Execute cycles */
void mb86235_device::execute_run()
{
#if ENABLE_DRC
	run_drc();
#else
	uint64_t opcode;
	while(m_core->icount > 0)
	{
		uint32_t curpc;

		curpc = check_previous_op_stall() ? m_core->cur_fifo_state.pc : m_core->pc;

		debugger_instruction_hook(curpc);
		opcode = m_pcache.read_qword(curpc);

		m_core->ppc = curpc;

		if(m_core->delay_slot == true)
		{
			m_core->pc = m_core->delay_pc;
			m_core->delay_slot = false;
		}
		else
			handle_single_step_execution();

		m_core->cur_fifo_state.has_stalled = false;

		execute_op(opcode);

		m_core->icount--;
	}

#endif
}


void mb86235_device::device_start()
{
	space(AS_PROGRAM).cache(m_pcache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_external);
	space(AS_BUSA).specific(m_dataa);
	space(AS_BUSB).specific(m_datab);

	m_cache.allocate_cache(mconfig().options().drc_rwx());
	m_core = m_cache.alloc_near<mb86235_internal_state>();
	memset(m_core, 0, sizeof(mb86235_internal_state));


#if ENABLE_DRC
	// init UML generator

	uint32_t umlflags = 0;
	m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, umlflags, 1, 24, 0);

	// add UML symbols
	m_drcuml->symbol_add(&m_core->pc, sizeof(m_core->pc), "pc");
	m_drcuml->symbol_add(&m_core->icount, sizeof(m_core->icount), "icount");

	for (int i = 0; i < 8; i++)
	{
		char buf[10];
		sprintf(buf, "aa%d", i);
		m_drcuml->symbol_add(&m_core->aa[i], sizeof(m_core->aa[i]), buf);
		sprintf(buf, "ab%d", i);
		m_drcuml->symbol_add(&m_core->ab[i], sizeof(m_core->ab[i]), buf);
		sprintf(buf, "ma%d", i);
		m_drcuml->symbol_add(&m_core->ma[i], sizeof(m_core->ma[i]), buf);
		sprintf(buf, "mb%d", i);
		m_drcuml->symbol_add(&m_core->mb[i], sizeof(m_core->mb[i]), buf);
		sprintf(buf, "ar%d", i);
		m_drcuml->symbol_add(&m_core->ar[i], sizeof(m_core->ar[i]), buf);
	}

	m_drcuml->symbol_add(&m_core->flags.az, sizeof(m_core->flags.az), "flags_az");
	m_drcuml->symbol_add(&m_core->flags.an, sizeof(m_core->flags.an), "flags_an");
	m_drcuml->symbol_add(&m_core->flags.av, sizeof(m_core->flags.av), "flags_av");
	m_drcuml->symbol_add(&m_core->flags.au, sizeof(m_core->flags.au), "flags_au");
	m_drcuml->symbol_add(&m_core->flags.ad, sizeof(m_core->flags.ad), "flags_ad");
	m_drcuml->symbol_add(&m_core->flags.zc, sizeof(m_core->flags.zc), "flags_zc");
	m_drcuml->symbol_add(&m_core->flags.il, sizeof(m_core->flags.il), "flags_il");
	m_drcuml->symbol_add(&m_core->flags.nr, sizeof(m_core->flags.nr), "flags_nr");
	m_drcuml->symbol_add(&m_core->flags.zd, sizeof(m_core->flags.zd), "flags_zd");
	m_drcuml->symbol_add(&m_core->flags.mn, sizeof(m_core->flags.mn), "flags_mn");
	m_drcuml->symbol_add(&m_core->flags.mz, sizeof(m_core->flags.mz), "flags_mz");
	m_drcuml->symbol_add(&m_core->flags.mv, sizeof(m_core->flags.mv), "flags_mv");
	m_drcuml->symbol_add(&m_core->flags.mu, sizeof(m_core->flags.mu), "flags_mu");
	m_drcuml->symbol_add(&m_core->flags.md, sizeof(m_core->flags.md), "flags_md");


	m_drcuml->symbol_add(&m_core->arg0, sizeof(m_core->arg0), "arg0");
	m_drcuml->symbol_add(&m_core->arg1, sizeof(m_core->arg1), "arg1");
	m_drcuml->symbol_add(&m_core->arg2, sizeof(m_core->arg2), "arg2");
	m_drcuml->symbol_add(&m_core->arg3, sizeof(m_core->arg3), "arg3");
	m_drcuml->symbol_add(&m_core->alutemp, sizeof(m_core->alutemp), "alutemp");
	m_drcuml->symbol_add(&m_core->multemp, sizeof(m_core->multemp), "multemp");

	m_drcuml->symbol_add(&m_core->pcp, sizeof(m_core->pcp), "pcp");


	m_drcfe = std::make_unique<mb86235_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, COMPILE_MAX_SEQUENCE);

	for (int i = 0; i < 8; i++)
	{
		m_regmap[i] = uml::mem(&m_core->aa[i]);
		m_regmap[i + 8] = uml::mem(&m_core->ab[i]);
		m_regmap[i + 16] = uml::mem(&m_core->ma[i]);
		m_regmap[i + 24] = uml::mem(&m_core->mb[i]);
	}
#endif

	// Register state for debugger
	state_add(MB86235_PC, "PC", m_core->pc).formatstr("%08X");
	state_add(MB86235_AR0, "AR0", m_core->ar[0]).formatstr("%08X");
	state_add(MB86235_AR1, "AR1", m_core->ar[1]).formatstr("%08X");
	state_add(MB86235_AR2, "AR2", m_core->ar[2]).formatstr("%08X");
	state_add(MB86235_AR3, "AR3", m_core->ar[3]).formatstr("%08X");
	state_add(MB86235_AR4, "AR4", m_core->ar[4]).formatstr("%08X");
	state_add(MB86235_AR5, "AR5", m_core->ar[5]).formatstr("%08X");
	state_add(MB86235_AR6, "AR6", m_core->ar[6]).formatstr("%08X");
	state_add(MB86235_AR7, "AR7", m_core->ar[7]).formatstr("%08X");
	state_add(MB86235_AA0, "AA0", m_core->aa[0]).formatstr("%08X");
	state_add(MB86235_AA1, "AA1", m_core->aa[1]).formatstr("%08X");
	state_add(MB86235_AA2, "AA2", m_core->aa[2]).formatstr("%08X");
	state_add(MB86235_AA3, "AA3", m_core->aa[3]).formatstr("%08X");
	state_add(MB86235_AA4, "AA4", m_core->aa[4]).formatstr("%08X");
	state_add(MB86235_AA5, "AA5", m_core->aa[5]).formatstr("%08X");
	state_add(MB86235_AA6, "AA6", m_core->aa[6]).formatstr("%08X");
	state_add(MB86235_AA7, "AA7", m_core->aa[7]).formatstr("%08X");
	state_add(MB86235_AB0, "AB0", m_core->ab[0]).formatstr("%08X");
	state_add(MB86235_AB1, "AB1", m_core->ab[1]).formatstr("%08X");
	state_add(MB86235_AB2, "AB2", m_core->ab[2]).formatstr("%08X");
	state_add(MB86235_AB3, "AB3", m_core->ab[3]).formatstr("%08X");
	state_add(MB86235_AB4, "AB4", m_core->ab[4]).formatstr("%08X");
	state_add(MB86235_AB5, "AB5", m_core->ab[5]).formatstr("%08X");
	state_add(MB86235_AB6, "AB6", m_core->ab[6]).formatstr("%08X");
	state_add(MB86235_AB7, "AB7", m_core->ab[7]).formatstr("%08X");
	state_add(MB86235_MA0, "MA0", m_core->ma[0]).formatstr("%08X");
	state_add(MB86235_MA1, "MA1", m_core->ma[1]).formatstr("%08X");
	state_add(MB86235_MA2, "MA2", m_core->ma[2]).formatstr("%08X");
	state_add(MB86235_MA3, "MA3", m_core->ma[3]).formatstr("%08X");
	state_add(MB86235_MA4, "MA4", m_core->ma[4]).formatstr("%08X");
	state_add(MB86235_MA5, "MA5", m_core->ma[5]).formatstr("%08X");
	state_add(MB86235_MA6, "MA6", m_core->ma[6]).formatstr("%08X");
	state_add(MB86235_MA7, "MA7", m_core->ma[7]).formatstr("%08X");
	state_add(MB86235_MB0, "MB0", m_core->mb[0]).formatstr("%08X");
	state_add(MB86235_MB1, "MB1", m_core->mb[1]).formatstr("%08X");
	state_add(MB86235_MB2, "MB2", m_core->mb[2]).formatstr("%08X");
	state_add(MB86235_MB3, "MB3", m_core->mb[3]).formatstr("%08X");
	state_add(MB86235_MB4, "MB4", m_core->mb[4]).formatstr("%08X");
	state_add(MB86235_MB5, "MB5", m_core->mb[5]).formatstr("%08X");
	state_add(MB86235_MB6, "MB6", m_core->mb[6]).formatstr("%08X");
	state_add(MB86235_MB7, "MB7", m_core->mb[7]).formatstr("%08X");
	state_add(MB86235_ST,  "ST", m_core->st).formatstr("%08X");

	state_add(MB86235_EB,  "EB",  m_core->eb).formatstr("%08X");
	state_add(MB86235_EO,  "EO",  m_core->eo).formatstr("%08X");
	state_add(MB86235_SP,  "SP",  m_core->sp).formatstr("%08X");
	state_add(MB86235_RPC, "RPC", m_core->rpc).formatstr("%08X");
	state_add(MB86235_LPC, "LPC", m_core->lpc).formatstr("%08X");
	state_add(MB86235_PDR, "PDR", m_core->pdr).formatstr("%08X");
	state_add(MB86235_DDR, "DDR", m_core->ddr).formatstr("%08X");
	state_add(MB86235_MOD, "MOD", m_core->mod).formatstr("%04X");
	state_add(MB86235_PRP, "PRP", m_core->prp).formatstr("%02X");
	state_add(MB86235_PWP, "PWP", m_core->pwp).formatstr("%02X");
	state_add(STATE_GENPC, "GENPC", m_core->pc ).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_core->pc).noshow();

	set_icountptr(m_core->icount);

	m_core->fp0 = 0.0f;
	save_pointer(NAME(m_core->pr), 24);
}

void mb86235_device::device_reset()
{
#if ENABLE_DRC
	flush_cache();
#endif

	m_core->pc = 0;
	m_core->delay_pc = 0;
	m_core->ppc = 0;
	m_core->delay_slot = false;
}

#if 0
void mb86235_cpu_device::execute_set_input(int irqline, int state)
{
	switch(irqline)
	{
		case MB86235_INT_INTRM:
			m_intrm_pending = (state == ASSERT_LINE);
			m_intrm_state = state;
			break;
		case MB86235_RESET:
			if (state == ASSERT_LINE)
				m_reset_pending = 1;
			m_reset_state = state;
			break;
		case MB86235_INT_INTR:
			if (state == ASSERT_LINE)
				m_intr_pending = 1;
			m_intr_state = state;
			break;
	}
}
#endif

mb86235_device::mb86235_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, MB86235, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 64, 12, -3)
	, m_external_config("external", ENDIANNESS_LITTLE, 32, 24, -2)
	, m_dataa_config("data_a", ENDIANNESS_LITTLE, 32, 10, -2, address_map_constructor(FUNC(mb86235_device::internal_abus), this))
	, m_datab_config("data_b", ENDIANNESS_LITTLE, 32, 10, -2, address_map_constructor(FUNC(mb86235_device::internal_bbus), this))
	, m_fifoin(*this, finder_base::DUMMY_TAG)
	, m_fifoout(*this, finder_base::DUMMY_TAG)
	, m_cache(CACHE_SIZE + sizeof(mb86235_internal_state))
	, m_drcuml(nullptr)
	, m_drcfe(nullptr)
{
}

mb86235_device::~mb86235_device()
{
}

device_memory_interface::space_config_vector mb86235_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_external_config),
		std::make_pair(AS_BUSA,    &m_dataa_config),
		std::make_pair(AS_BUSB,    &m_datab_config)
	};
}

void mb86235_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("?");
			break;
	}
}

std::unique_ptr<util::disasm_interface> mb86235_device::create_disassembler()
{
	return std::make_unique<mb86235_disassembler>();
}
