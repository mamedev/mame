// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
// thanks-to:Happy

/*
 * Silicon Graphics GE5 and HQ1 devices.
 *
 * This board handles the interface to the host, and mainly consists of the HQ1
 * instruction sequencer and the WTL3132 floating-point accelerator. The board
 * also contains instruction and data RAM for the HQ1, and a FIFO for host
 * communication.
 *
 * The undocumented HQ1 microcode instruction format is relatively well decoded
 * now, but the exact timing and function of some operations remains unknown.
 *
 * TODO:
 *   - implement host dma
 *   - verify some operations
 *   - tidy up latch/timing logic
 *   - implement single stepping
 *   - redo disassembly
 *   - save state
 */

#include "emu.h"
#include "debugger.h"
#include "sgi_ge5.h"

#define LOG_GENERAL   (1U << 0)
#define LOG_TOKEN     (1U << 1)
#define LOG_MEMORY    (1U << 2)

//#define VERBOSE       (LOG_GENERAL)
#include "logmacro.h"

static char const *const token_diag[] =
{
	"DIAG_DATA",          "DIAG_INIT",          "DIAG_DRAMTEST",      "DIAG_DMA_IG",
	"DIAG_DMA_IB",        "DIAG_DMA_GB",        "DIAG_CHARPOS",       "DIAG_WRITEFULLPIX",
	"DIAG_DRAWLINE",      "DIAG_DK3_FIFO",      "DIAG_DK3_FINFLGS",   "DIAG_DRAW4SPANS",
	"DIAG_DRAWFLATSPAN",  "DIAG_DRAWSPAN",      "DIAG_LIFECHECK",     "DIAG_LOADRE",
	"DIAG_READPIXDMA",    "DIAG_READPIXELS",    "DIAG_SCREENCLEAR",   "DIAG_WRITEPIXDMA",
	"DIAG_FASTCLEAR20",   "DIAG_DRAWCHAR",      "DIAG_STRINGINIT",    "DIAG_STRINGEND",
	"DIAG_FASTCHAR",      "DIAG_DRAWLONGSPANS", nullptr,              nullptr,

	// turbo option
	nullptr,              nullptr,              "DIAG_DSPLOAD",       "DIAG_DSPRAMDATA",
	"DIAG_DSPRAMADDR",    "DIAG_DSPFIFO",       "DIAG_DSPSPAN",       "DIAG_DSPRD",
	"DIAG_DSPWR",         "DIAG_DSPINTRAM",     "DIAG_DSPSCOPE",      "DIAG_RESCOPE",
};

static char const *const token_puc[] =
{
	"PUC_DATA",           "PUC_INIT",           nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              "PUC_COLOR",          "PUC_FINISH",
	"PUC_PNT2I",          "PUC_RECTI2D",        "PUC_CMOV2I",         "PUC_DRAWCHAR",
	"PUC_HAND",           "PUC_FBOPT",          "PUC_ZBOPT",          "PUC_TOPSCAN",
	"DIAG_READPIXELS",    "DIAG_WRITEFULLPIX",  "DIAG_LOADRE",        "DIAG_CHARPOS",
};

static char const *const token_gl[] =
{
	"GE_DATA",            "GE_INIT",            nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              "GE_COLOR",           "GE_FINISH0",
	"GE_PNT2I",           "GE_SBOXI",           "GE_CMOV2I",          "GE_DRAWCHAR",
	"GE_HAND",            "GE_FBOPT",           "GE_ZBOPT",           "GE_TOPSCAN",
	"GE_READPIXELS",      "GE_WRITEPIXELS",     "GE_LOADRE",          "GE_GETCPOS",
	"GE_PICKMODE",        "GE_PIXTYPE",         "GE_PIXWRITEMASK",    "GE_POPNAME",
	"GE_PUSHNAME",        "GE_READBLOCK",       "GE_RECTREAD",        "GE_READBUF",
	"GE_READPIXDMA",      "GE_AUXWRITEMASK",    "GE_READRGB",         "GE_RECTCOPY",
	"GE_RGBCOLOR",        "GE_RGBSHADERANGE",   "GE_RWMODE",          "GE_SCREENCLEAR",
	"GE_SHADEMODEL",      "GE_SHADERANGE",      "GE_WRITEBLOCK",      "GE_RECTWRITE",
	"GE_WRITEPIXDMA",     "GE_BEGINBBOX",       "GE_ZBUFFER",         "GE_ZCLEAR",
	"GE_ZOOMFACTOR",      "GE_READSOURCE",      "GE_DRAWMODE",        "GE_CZCLEAR",
	"GE_HQMSAV",          "GE_ZFUNCTION",       "GE_SETPIECES",       "GE_FLATMODE",
	"GE_LMCOLOR",         "GE_LOADAMBIENT",     "GE_DEPTHFN",         "GE_LOADDIFFUSE",
	"GE_LOADMATRIX",      "GE_MULTMATRIX",      "GE_PUSHMATRIX",      "GE_POPMATRIX",
	"GE_LOADSPECULAR",    "GE_LOADEMISSION",    "GE_LOADASUM",        "GE_LOADLCOLOR",
	nullptr,              nullptr,              "GE_CURVEIT",         "GE_LOADVIEWP",
	"GE_POLYGON",         "GE_ENDPOLYGON",      "GE_TRANSLATEI",      "GE_TRANSLATE",
	"GE_LINESTYLE",       "GE_LINEWIDTH",       "GE_VERTEX2I",        "GE_VERTEX2",
	"GE_VERTEX3I",        "GE_VERTEX3",         "GE_VERTEX4I",        "GE_VERTEX4",
	"GE_RVERTEX2I",       "GE_RVERTEX2",        "GE_RVERTEX3I",       "GE_RVERTEX3",
	"GE_CLOSEDLINE",      "GE_ENDCLOSEDLINE",   "GE_LSREPEAT",        "GE_ANTIALIAS",
	"GE_COLORF",          "GE_PNT2",            "GE_PNT3I",           "GE_PNT3",
	"GE_PNT4I",           "GE_PNT4",            nullptr,              nullptr,
	"GE_MOVE2I",          "GE_MOVE2",           "GE_MOVE3I",          "GE_MOVE3",
	"GE_MOVE4I",          "GE_MOVE4",           "GE_RMOVE2I",         "GE_RMOVE2",
	"GE_RMOVE3I",         "GE_RMOVE3",          "GE_DRAW2I",          "GE_DRAW2",
	"GE_DRAW3I",          "GE_DRAW3",           "GE_DRAW4I",          "GE_DRAW4",
	"GE_RDRAW2I",         "GE_RDRAW2",          "GE_RDRAW3I",         "GE_RDRAW3",
	"GE_ENABLWID",        "GE_LOADGE",          nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              "GE_FRONTFACE",       "GE_BACKFACE",        "GE_CONCAVE",
	"GE_PATTERN",         "GE_SETPATTERN",      "GE_LOADNORMAL",      "GE_MULTNORMAL",
	"GE_INITSTACK",       "GE_MMODE",           "GE_NORMAL",          "GE_ABNORMAL",
	"GE_LIGHTATTR1",      "GE_LIGHTATTR2",      "GE_LIGHTATTR3",      "GE_BINDLIGHT",
	"GE_LIGHTDATA4",      "GE_LIGHTMEMPTR",     "GE_LIGHTDIRECTION",  "GE_LIGHTPOSITION",
	"GE_LIGHTMOVEDATA",   "GE_BEGINMESH",       "GE_ENDMESH",         "GE_SWAPMESH",
	"GE_SBOXF",           "GE_SBOXFI",          "GE_FATPOLY",         "GE_ENDOLDPOLYGON",
	"GE_SBOX",            "GE_CURRENTWID",      nullptr,              nullptr,
	"GE_DEPTHCUE",        "GE_CMOV2",           "GE_CMOV3I",          "GE_CMOV3",
	"GE_CMOV4I",          "GE_CMOV4",           "GE_ENABDITH",        "GE_ENABWID",
	nullptr,              nullptr,              "GE_SETMATRIX",       "GE_COMPOSEMATRIX",
	"GE_LOADTOPMATRIX",   "GE_COPYMATRIX",      "GE_FEEDBACK",        "GE_ENDFEEDBACK",
	"GE_PASSTHROUGH",     "GE_FMOVE",           "GE_FDRAW",           "GE_FLINE",
	"GE_SCRMASK",         "GE_ZSOURCE",         "GE_SUBPIXEL",        "GE_SMOOTHPOINT",
	"GE_RASTEROP",        "GE_RESETLS",         nullptr,              nullptr,
	nullptr,              nullptr,              "GE_SETSURFSCALE",    "GE_SETV",
	"GE_PUSHV",           "GE_SURFP1",          "GE_SURFNTURF",       "GE_SETVHI",
	"GE_STRIP",           "GE_1LOAD1",          "GE_1LOAD3",          "GE_1LOAD4",
	"GE_SURFMODE",        "GE_DSPRD",           "GE_DSPWR",           "GE_DSPNEXT",
	"GE_DSPDUMMY",        "GE_PICKTYPE",        "GE_VERSION",         "GE_ENDBBOX",
	"GE_ENDPICKMODE",     "GE_INITNAMES",       "GE_LOADNAME",        nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	"GE_CTX0",            nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              nullptr,
	nullptr,              nullptr,              nullptr,              "GE_CTX1",
};

DEFINE_DEVICE_TYPE(SGI_GE5, sgi_ge5_device, "ge5", "SGI Geometry Engine 5")

sgi_ge5_device::sgi_ge5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, SGI_GE5, tag, owner, clock)
	, m_code_config("code", ENDIANNESS_BIG, 64, 15, -3, address_map_constructor(FUNC(sgi_ge5_device::code_map), this))
	, m_data_config("data", ENDIANNESS_BIG, 32, 13, -2, address_map_constructor(FUNC(sgi_ge5_device::data_map), this))
	, m_fpu(*this, "fpu")
	, m_int_cb(*this)
	, m_fifo_empty(*this)
	, m_fifo_read(*this)
	, m_re_r(*this)
	, m_re_w(*this)
	, m_icount(0)
{
}

void sgi_ge5_device::device_add_mconfig(machine_config &config)
{
	WTL3132(config, m_fpu, clock());
	m_fpu->out_fpcn().set([this](int state) { m_fpu_c = bool(state); });
	m_fpu->out_port_x().set([this](u32 data) { m_bus = data; });
}


void sgi_ge5_device::code_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("code");
}

void sgi_ge5_device::data_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("data");
}

void sgi_ge5_device::device_start()
{
	m_int_cb.resolve_safe();
	m_fifo_empty.resolve();
	m_fifo_read.resolve();
	m_re_r.resolve();
	m_re_w.resolve();

	// TODO: save state
	state_add(STATE_GENPC,     "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(0, "PC", m_pc).formatstr("%04X");

	state_add(1, "MEMPTR", m_memptr).formatstr("%04X");
	state_add(2, "REPTR",  m_reptr).formatstr("%04X");
	state_add(3, "BUS",    m_bus).formatstr("%08X");
	state_add(4, "DMACNT", m_dma_count).formatstr("%04X");

	m_fpu->state_add(*this, 5);

	set_icountptr(m_icount);
}

void sgi_ge5_device::device_reset()
{
	m_sp = 0;
	m_reptr = 0;
	m_memptr = 0;
	m_memptr_temp = 0;

	set_int(false);

	m_state = DECODE;
	suspend(SUSPEND_REASON_HALT, false);
}

device_memory_interface::space_config_vector sgi_ge5_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(0, &m_code_config),
		std::make_pair(1, &m_data_config),
	};
}

std::unique_ptr<util::disasm_interface> sgi_ge5_device::create_disassembler()
{
	return std::make_unique<sgi_ge5_disassembler>();
}

void sgi_ge5_device::execute_run()
{
	while (m_icount > 0)
	{
		switch (m_state)
		{
		case DECODE:
			debugger_instruction_hook(m_pc);

			// decode instruction
			decode();

			// execute secondary operation
			if (m_decode.secondary)
				secondary(m_bus_latch);

			// increment memptr
			if (m_decode.inc_memptr)
				m_memptr = (m_memptr + 1) & 0x7fff;

			// increment reptr
			if (m_decode.inc_reptr)
				m_reptr = (m_reptr + 1) & 0x3f;

			// update pc to next sequential instruction
			m_pc += m_decode.secondary ? 2 : 1;
			m_state = READ;
			break;

		case READ:
			m_state = CONTROL;

			// fetch source
			switch (m_decode.source)
			{
			case 0: // reptr
				if (m_reptr == 0x20 && !m_re_drq)
				{
					// re read stall
					m_state = READ;
					m_icount = 0;
				}
				else
					m_bus = m_re_r(m_reptr);
				break;
			case 1: // fifo
				if (m_fifo_empty())
				{
					// fifo read stall
					m_state = READ;
					m_icount = 0;
					suspend(SUSPEND_REASON_TRIGGER, false);
				}
				else
					m_bus = m_fifo_read();
				break;
			case 2: // memptr
				m_bus = space(1).read_dword(m_memptr);
				break;
			case 3: // fpu
				m_decode.fpu |= (2ULL << wtl3132_device::S_IOCT);
				break;
			}
			break;

		case CONTROL:
			m_state = WRITE;

			switch (m_decode.control)
			{
			case 0x0: // sequential execution
				break;
			case 0x1: // unconditional branch
				m_pc = m_decode.immediate;
				break;
			case 0x2: // branch fpu less than
				if (m_fpu_c_latch)
					m_pc = m_decode.immediate;
				break;
			case 0x3: // branch fpu greater or equal
				if (!m_fpu_c_latch)
					m_pc = m_decode.immediate;
				break;
			case 0x4: // unconditional call
				m_stack[m_sp] = m_pc;
				m_sp = (m_sp + 1) & 7;
				m_pc = m_decode.immediate;
				break;
			case 0x5: // call fpu less than
				if (m_fpu_c_latch)
				{
					m_stack[m_sp] = m_pc;
					m_sp = (m_sp + 1) & 7;
					m_pc = m_decode.immediate;
				}
				break;
			case 0x6: // call fpu greater or equal
				if (!m_fpu_c_latch)
				{
					m_stack[m_sp] = m_pc;
					m_sp = (m_sp + 1) & 7;
					m_pc = m_decode.immediate;
				}
				break;
			case 0x7: // return
				m_sp = (m_sp + 7) & 7;
				m_pc = m_stack[m_sp];
				break;
			case 0x8: // fetch
				m_pc = (m_bus >> 31) & 0x1fe;

				debugger_exception_hook(m_bus >> 32);

				if (VERBOSE & LOG_TOKEN)
				{
					auto const suppressor(machine().disable_side_effects());

					u8 const token = m_bus >> 32;
					char const *string = nullptr;

					/*
					 * Magic numbers stored at specific data memory locations
					 * are used to identify specific microcode programs. Other
					 * variations may exist but are not known at this time.
					 */
					if (space(1).read_dword(0x50b) == 0x004d0003)
					{
						if (token < ARRAY_LENGTH(token_puc))
							string = token_puc[token];
					}
					else if (space(1).read_dword(0x50d) == 0x004d0005 || space(1).read_dword(0x536) == 0x12345678)
					{
						if (token < ARRAY_LENGTH(token_gl))
							string = token_gl[token];
					}
					else if (token < ARRAY_LENGTH(token_diag))
						string = token_diag[token];

					if (string)
						LOGMASKED(LOG_TOKEN, "fetch 0x%02x (%s)\n", token, string);
					else
						LOGMASKED(LOG_TOKEN, "fetch 0x%02x (unknown)\n", token);
				}
				else
					LOG("fetch 0x%02x\n", m_bus >> 32);

				// neutralize previous instruction writeback
				m_fpu->neut_w(0);
				break;
			case 0x9: // branch indirect
				// TODO: verify value
				m_pc = m_bus;
				break;
			case 0xa: // branch less than
				if (BIT(m_bus, 31))
					m_pc = m_decode.immediate;
				break;
			case 0xb: // branch greater or equal
				if (!BIT(m_bus, 31))
					m_pc = m_decode.immediate;
				break;
			case 0xc: // stall
				LOG("stall\n");
				suspend(SUSPEND_REASON_HALT, false);
				m_icount = 0;
				break;
			case 0xd: // call less than
				if (BIT(m_bus, 31))
				{
					m_stack[m_sp] = m_pc;
					m_sp = (m_sp + 1) & 7;
					m_pc = m_decode.immediate;
				}
				break;
			case 0xe: // call greater or equal
				if (!BIT(m_bus, 31))
				{
					m_stack[m_sp] = m_pc;
					m_sp = (m_sp + 1) & 7;
					m_pc = m_decode.immediate;
				}
				break;
			case 0xf: // dma cycle
				if (--m_dma_count)
					m_pc -= m_decode.secondary ? 2 : 1;
				break;
			}
			break;

		case WRITE:
			m_state = COMPLETE;

			// store destination
			switch (m_decode.destination)
			{
			case 0: // reptr
				if (m_reptr == 0x20 && !m_re_rdy && !m_re_drq)
				{
					// re write stall
					m_state = WRITE;
					m_icount = 0;
				}
				else if (m_reptr > 0x20 && !m_re_rdy)
				{
					// re unbuffered register write stall
					m_state = WRITE;
					m_icount = 0;
				}
				else
					m_re_w(m_reptr, m_bus);
				break;
			case 1: // TODO: bus?
				break;
			case 2: // memptr
				space(1).write_dword(m_memptr, m_bus);
				break;
			case 3: // fpu
				m_fpu->x_port_w(m_bus);
				m_decode.fpu |= (3ULL << wtl3132_device::S_IOCT);
				break;
			}
			break;

		case COMPLETE:
			m_state = DECODE;

			// restore memptr
			if (m_memptr_temp & 0x8000)
			{
				m_memptr = m_memptr_temp & 0x7fff;
				m_memptr_temp = 0;
			}

			// FIXME: fpu condition has additional 1 cycle latency
			m_bus_latch = m_bus;
			m_fpu_c_latch = m_fpu_c;

			// fpu operation
			m_fpu->c_port_w(m_decode.fpu);
			m_fpu->clk_w(1);
			m_fpu->neut_w(1);

			m_icount--;
			break;
		}
	}
}

void sgi_ge5_device::decode()
{
	// fetch primary word
	u64 const primary = space(0).read_qword(m_pc + 0);

	// decode primary word
	m_decode.source = (primary >> 38) & 3;
	m_decode.inc_reptr = BIT(primary, 37);
	m_decode.secondary = BIT(primary, 36);
	m_decode.inc_memptr = BIT(primary, 35);
	m_decode.destination = (primary >> 33) & 3;
	m_decode.control = (primary >> 29) & 0xf;

	// decode fpu instruction
	m_decode.fpu = ((primary & 0x1fff'f800ULL) << 5) | ((primary & 0x0000'07ffULL) << 2) | (2ULL << wtl3132_device::S_ENCN);
	if (m_cwen)
		m_decode.fpu |= wtl3132_device::M_CWEN;

	// decode secondary word
	if (m_decode.secondary)
	{
		u64 const secondary = space(0).read_qword(m_pc + 1);

		m_decode.operation = (secondary >> 32) & 0xfe;
		m_decode.immediate = (secondary >> 19) & 0x3fff;
	}
}

void sgi_ge5_device::secondary(u64 bus)
{
	switch (m_decode.operation)
	{
	case 0x3c: // store register
		switch (m_decode.immediate)
		{
		case 0: // TODO: store pcsave?
			break;

		case 1: // store memptr
			m_bus = m_memptr;
			break;

		case 2: // store reptr
			m_bus = m_reptr;
		}
		break;

	case 0x8c: // load reptr
		m_reptr = m_bus & 0x3f;
		break;

	case 0x90: // load memptr; set reptr
		m_memptr = m_bus & 0x7fff;
		m_reptr = m_decode.immediate & 0x3f;
		break;

	case 0x9c: // set reptr
		m_reptr = m_decode.immediate & 0x3f;
		break;

	case 0xb0: // load memptr
		m_memptr = bus & 0x7fff;
		break;

	case 0xb4: // set memptr
		m_memptr = m_decode.immediate & 0x7fff;
		break;

	case 0xb6: // set memptr; set finish flag
		m_memptr = m_decode.immediate & 0x7fff;
		m_finish[m_decode.immediate & 1] = 1;
		break;

	case 0xb8: // set memptr_temp
		m_memptr_temp = m_memptr | 0x8000;
		m_memptr = m_decode.immediate & 0x7fff;
		break;

	case 0xbc: // nop?
		break;

	case 0xfc:
		switch (m_decode.immediate)
		{
		case 0: // TODO: assert dma ready
			break;

		default: // assert interrupt
			LOG("interrupt asserted\n");
			set_int(true);
			break;
		}
		break;

	case 0xfe:
		switch (m_decode.immediate)
		{
		case 0: // TODO: reset dma?
			break;

		default: // load dma count
			m_dma_count = m_bus;
			break;
		}
		break;

	default:
		logerror("unknown secondary operation 0x%02x\n", m_decode.operation);
		break;
	}
}

void sgi_ge5_device::command_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
	case 0x00: // clear stall
		LOG("clear stall\n");
		resume(SUSPEND_REASON_HALT);
		debugger_exception_hook(0);
		break;

	case 0x10: // set single step
	case 0x20: // clear single step
	case 0x30: // execute single step
		break;

	case 0x50: // clear interrupt
		LOG("interrupt cleared\n");
		set_int(false);
		break;
	}
}

u32 sgi_ge5_device::code_r(offs_t offset)
{
	m_pc = offset | offs_t(m_mar & 0x7f) << 8;
	u64 const data = space(0).read_qword(m_pc);

	return m_mar_msb ? u32(data >> 32) : u32(data);
}

void sgi_ge5_device::code_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_pc = offset | offs_t(m_mar & 0x7f) << 8;

	LOGMASKED(LOG_MEMORY, "code_w msb %d offset 0x%08x data 0x%08x mask 0x%08x (%s)\n", m_mar_msb, m_pc, data, mem_mask, machine().describe_context());

	if (m_mar_msb)
	{
		u64 const mask = u64(mem_mask & 0x000000ffU) << 32;

		if (BIT(data, 8) && !BIT(data, 4))
		{
			// FIXME: this is required, but not very satisfactory
			LOGMASKED(LOG_MEMORY, "correcting unset secondary instruction bit\n");
			data |= 0x10;
		}

		space(0).write_qword(m_pc, u64(data) << 32, mask);
	}
	else
		space(0).write_qword(m_pc, data, mem_mask);
}

u32 sgi_ge5_device::data_r(offs_t offset)
{
	// FIXME: 5 or 6 bits from MAR?
	m_memptr = offset | offs_t(m_mar & 0x1f) << 8;

	return space(1).read_dword(m_memptr);
}

void sgi_ge5_device::data_w(offs_t offset, u32 data, u32 mem_mask)
{
	// FIXME: 5 or 6 bits from MAR?
	m_memptr = offset | offs_t(m_mar & 0x1f) << 8;

	space(1).write_dword(m_memptr, data, mem_mask);
}

offs_t sgi_ge5_disassembler::disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params)
{
	std::string src, dst;
	u16 immediate = 0;
	u32 flags = 0;

	u64 const primary = opcodes.r64(pc);

	if (BIT(primary, 36))
	{
		std::string prefix;

		u64 const secondary = opcodes.r64(pc + 1);
		u8 const opcode = (secondary >> 32) & 0xfe;
		immediate = (secondary >> 19) & 0x3fff;

		switch (opcode)
		{
		case 0x3c:
			switch (immediate)
			{
			case 0: prefix = std::string("STORE PCSAVE"); break;
			case 1: prefix = std::string("STORE MEMPTR"); break;
			case 2: prefix = std::string("STORE REPTR"); break;
			}
			break;
		case 0x8c: prefix = std::string("LOAD REPTR"); break;
		case 0x8e: prefix = std::string("LOAD MEMPTR"); break;
		case 0x90: prefix = util::string_format("LOAD MEMPTR; SET REPTR,0x%04x", immediate); break;
		case 0x9c: prefix = util::string_format("SET REPTR,0x%04x", immediate); break;
		case 0xb0: prefix = std::string("LOAD MEMPTR"); break;
		case 0xb4: prefix = util::string_format("SET MEMPTR,0x%04x", immediate); break;
		case 0xb6: prefix = util::string_format("SET MEMPTR,0x%04x; SET FF%d", immediate, immediate & 1); break;
		case 0xb8: prefix = util::string_format("SET MEMPTR_TEMP,0x%04x", immediate); break;
		case 0xbc: break;
		case 0xfc: prefix = immediate ? std::string("SET INT") : std::string("SET DMARDY"); break;
		case 0xfe: prefix = immediate ? std::string("LOAD DMACNT") : std::string("RESET DMA"); break;
		}

		if (!prefix.empty())
			stream << prefix << "; ";
	}

	u8 const opcode = (primary >> 32) & 0xff;

	u64 fpu_ctrl = 0; // ENCN=0, IOCT=0

	switch ((primary >> 29) & 0xf)
	{
	case 2:
	case 3:
	case 5:
	case 6:
		fpu_ctrl |= 0x2'0000'0000; // ENCN=2
		break;
	}

	// fstore
	if (((primary >> 38) & 3) == 3)
		fpu_ctrl |= 0x8000'0000;

	// fload
	if (((primary >> 33) & 3) == 3)
		fpu_ctrl |= 0xc000'0000;

	switch (opcode >> 6)
	{
	case 0: src = std::string("RE"); break;
	case 1: src = std::string("FIFO"); break;
	case 2: src = std::string("MEM"); break;
	case 3: src = std::string("FPU"); break;
	}

	switch ((opcode >> 1) & 3)
	{
	case 0: dst = std::string("RE"); break;
	case 1: dst = std::string("BUS"); break;
	case 2: dst = std::string("MEM"); break;
	case 3: dst = std::string("FPU"); break;
	}

	stream << wtl3132_device::disassemble(
		bitswap<34>((primary & 0x0fff'ffff) | fpu_ctrl,
			28, 27, 26,          // f
			25, 24, 23, 22, 21,  // aadd
			20, 19, 18, 17, 16,  // badd
			15, 14, 13, 12, 11,  // cadd
			29,                  // cwen*
			31, 30,              // ioct*
			10, 9, 8, 7, 6,      // dadd
			5, 4, 3,             // abin
			2, 1,                // adst
			0,                   // mbin
			33, 32))             // encn*
		<< "; ";

	switch ((primary >> 29) & 0x140)
	{
	case 0x040: stream << std::string("MEMPTR++; "); break;
	case 0x100: stream << std::string("REPTR++; "); break;
	case 0x140: stream << std::string("MEMPTR++; REPTR++; "); break;
	}

	stream << util::string_format("R:%s; W:%s", src, dst);

	// branch
	switch ((primary >> 29) & 0xf)
	{
	case 0x1: stream << util::string_format("; BRA 0x%04x", immediate); break;
	case 0x2: stream << util::string_format("; BLTF 0x%04x", immediate); break;
	case 0x3: stream << util::string_format("; BGEF 0x%04x", immediate); break;
	case 0x4: stream << util::string_format("; CALL 0x%04x", immediate); flags = STEP_OVER; break;
	case 0x5: stream << util::string_format("; CLTF 0x%04x", immediate); flags = STEP_OVER; break;
	case 0x6: stream << util::string_format("; CGEF 0x%04x", immediate); flags = STEP_OVER; break;
	case 0x7: stream << "; RET"; flags = STEP_OUT; break;
	case 0x8: stream << "; FETCH"; break;
	case 0x9: stream << util::string_format("; BRI 0x%04x", immediate); break;
	case 0xa: stream << util::string_format("; BLT 0x%04x", immediate); break;
	case 0xb: stream << util::string_format("; BGE 0x%04x", immediate); break;
	case 0xc: stream << "; STALL"; break;
	case 0xd: stream << util::string_format("; CLT 0x%04x", immediate); flags = STEP_OVER; break;
	case 0xe: stream << util::string_format("; CGE 0x%04x", immediate); flags = STEP_OVER; break;
	case 0xf: stream << "; DMA?"; flags = STEP_OVER; break;
	}

	return SUPPORTED | flags | (BIT(primary, 36) ? 2 : 1);
}
