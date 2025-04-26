// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

#include "emu.h"

#include "xbox.h"
#include "xbox_pci.h"

#include "machine/pci.h"
#include "machine/idectrl.h"

#include "cpu/i386/i386.h"

#include "debug/debugcon.h"
#include "debugger.h"
#include "romload.h"
#include "screen.h"

#include <functional>

const xbox_base_state::debugger_constants xbox_base_state::debugp[] = {
	{ 0x66232714, {0x8003aae0, 0x5c, 0x1c, 0x28, 0x210, 8, 0x28, 0x1c} },
	{ 0x49d8055a, {0x8003aae0, 0x5c, 0x1c, 0x28, 0x210, 8, 0x28, 0x1c} }
};

const struct {
	const char *name;
	const int value;
} vertex_format_names[] = {
	{"NONE", 0x02}, {"NORMSHORT1", 0x11}, {"FLOAT1", 0x12}, {"PBYTE1", 0x14},
	{"SHORT1", 0x15}, {"NORMPACKED3", 0x16}, {"NORMSHORT2", 0x21},
	{"FLOAT2", 0x22}, {"PBYTE2", 0x24}, {"SHORT2", 0x25}, {"NORMSHORT3", 0x31},
	{"FLOAT3", 0x32}, {"PBYTE3", 0x34}, {"SHORT3", 0x35}, {"D3DCOLOR", 0x40},
	{"FLOAT4", 0x42}, {"NORMSHORT4", 0x41}, {"PBYTE4", 0x44}, {"SHORT4", 0x45},
	{"FLOAT2H", 0x72}, { nullptr, 0}
};

int xbox_base_state::find_bios_index()
{
	u8 sb = system_bios();
	return sb;
}

bool xbox_base_state::find_bios_hash(int bios, uint32_t &crc32)
{
	uint32_t crc = 0;
	const std::vector<rom_entry> &rev = rom_region_vector();

	for (rom_entry const &re : rev)
	{
		if (ROMENTRY_ISFILE(re))
		{
			if (ROM_GETBIOSFLAGS(re) == (uint32_t)(bios + 1))
			{
				const std::string &h = re.hashdata();
				util::hash_collection hc(h.c_str());
				if (hc.crc(crc) == true)
				{
					crc32 = crc;
					return true;
				}
			}
		}
	}
	return false;
}

void xbox_base_state::find_debug_params()
{
	uint32_t crc;
	int sb;

	sb = (int)find_bios_index();
	debugc_bios = debugp;
	if (find_bios_hash(sb - 1, crc) == true)
	{
		for (int n = 0; n < 2; n++)
			if (debugp[n].id == crc)
			{
				debugc_bios = &debugp[n];
				break;
			}
	}
}

void xbox_base_state::dump_string_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	uint64_t addr;
	offs_t address;

	if (params.size() < 2)
		return;

	if (!con.validate_number_parameter(params[1], addr))
		return;

	address = (offs_t)addr;
	address_space *tspace;
	if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
	{
		con.printf("Address is unmapped.\n");
		return;
	}

	uint32_t length = tspace->read_word_unaligned(address);
	uint32_t maximumlength = tspace->read_word_unaligned(address + 2);
	offs_t buffer = tspace->read_dword_unaligned(address + 4);
	con.printf("Length %d word\n", length);
	con.printf("MaximumLength %d word\n", maximumlength);
	con.printf("Buffer %08X byte* ", buffer);

	// limit the number of characters to avoid flooding
	if (length > 256)
		length = 256;

	if (m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, buffer, tspace))
	{
		for (int a = 0; a < length; a++)
		{
			uint8_t c = tspace->read_byte(buffer + a);
			con.printf("%c", c);
		}
	}
	con.printf("\n");
}

void xbox_base_state::dump_process_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	uint64_t addr;
	offs_t address;

	if (params.size() < 2)
		return;

	if (!con.validate_number_parameter(params[1], addr))
		return;

	address = (offs_t)addr;
	address_space *tspace;
	if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
	{
		con.printf("Address is unmapped.\n");
		return;
	}

	con.printf("ReadyListHead {%08X,%08X} _LIST_ENTRY\n", tspace->read_dword(address), tspace->read_dword_unaligned(address + 4));
	con.printf("ThreadListHead {%08X,%08X} _LIST_ENTRY\n", tspace->read_dword(address + 8), tspace->read_dword_unaligned(address + 12));
	con.printf("StackCount %d dword\n", tspace->read_dword_unaligned(address + 16));
	con.printf("ThreadQuantum %d dword\n", tspace->read_dword_unaligned(address + 20));
	con.printf("BasePriority %d byte\n", tspace->read_byte(address + 24));
	con.printf("DisableBoost %d byte\n", tspace->read_byte(address + 25));
	con.printf("DisableQuantum %d byte\n", tspace->read_byte(address + 26));
	con.printf("_padding %d byte\n", tspace->read_byte(address + 27));
}

void xbox_base_state::dump_list_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	uint64_t addr;
	offs_t address;

	if (params.size() < 2)
		return;

	if (!con.validate_number_parameter(params[1], addr))
		return;

	uint64_t offs = 0;
	offs_t offset = 0;
	if (params.size() >= 3)
	{
		if (!con.validate_number_parameter(params[2], offs))
			return;
		offset = (offs_t)offs;
	}

	uint64_t start = addr;
	address_space *tspace;
	address = (offs_t)addr;
	if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
	{
		con.printf("Address is unmapped.\n");
		return;
	}
	if (params.size() >= 3)
		con.printf("Entry    Object\n");
	else
		con.printf("Entry\n");

	uint64_t old;
	for (int num = 0; num < 32; num++)
	{
		if (params.size() >= 3)
			con.printf("%08X %08X\n", (uint32_t)addr, (offs_t)addr - offset);
		else
			con.printf("%08X\n", (uint32_t)addr);
		old = addr;
		addr = tspace->read_dword_unaligned(address);
		if (addr == start)
			break;
		if (addr == old)
			break;
		address = (offs_t)addr;
		if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
			break;
	}
}

void xbox_base_state::dump_dpc_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	uint64_t addr;
	offs_t address;
	address_space *tspace;

	if (params.size() < 2)
		return;

	if (!con.validate_number_parameter(params[1], addr))
		return;

	address = (offs_t)addr;
	if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
	{
		con.printf("Address is unmapped.\n");
		return;
	}
	con.printf("Type %d word\n", tspace->read_word_unaligned(address));
	con.printf("Inserted %d byte\n", tspace->read_byte(address + 2));
	con.printf("Padding %d byte\n", tspace->read_byte(address + 3));
	con.printf("DpcListEntry {%08X,%08X} _LIST_ENTRY\n", tspace->read_dword_unaligned(address + 4), tspace->read_dword_unaligned(address + 8, true));
	con.printf("DeferredRoutine %08X dword\n", tspace->read_dword_unaligned(address + 12));
	con.printf("DeferredContext %08X dword\n", tspace->read_dword_unaligned(address + 16));
	con.printf("SystemArgument1 %08X dword\n", tspace->read_dword_unaligned(address + 20));
	con.printf("SystemArgument2 %08X dword\n", tspace->read_dword_unaligned(address + 24));
}

void xbox_base_state::dump_timer_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	uint64_t addr;
	offs_t address;

	if (params.size() < 2)
		return;

	if (!con.validate_number_parameter(params[1], addr))
		return;

	address = (offs_t)addr;
	address_space *tspace;
	if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
	{
		con.printf("Address is unmapped.\n");
		return;
	}
	con.printf("Header.Type %d byte\n", tspace->read_byte(address));
	con.printf("Header.Absolute %d byte\n", tspace->read_byte(address + 1));
	con.printf("Header.Size %d byte\n", tspace->read_byte(address + 2));
	con.printf("Header.Inserted %d byte\n", tspace->read_byte(address + 3));
	con.printf("Header.SignalState %08X dword\n", tspace->read_dword_unaligned(address + 4));
	con.printf("Header.WaitListEntry {%08X,%08X} _LIST_ENTRY\n", tspace->read_dword_unaligned(address + 8), tspace->read_dword_unaligned(address + 12));
	con.printf("%s", string_format("DueTime %x qword\n", (int64_t)tspace->read_qword_unaligned(address + 16)).c_str());
	con.printf("TimerListEntry {%08X,%08X} _LIST_ENTRY\n", tspace->read_dword_unaligned(address + 24), tspace->read_dword_unaligned(address + 28));
	con.printf("Dpc %08X dword\n", tspace->read_dword_unaligned(address + 32));
	con.printf("Period %d dword\n", tspace->read_dword_unaligned(address + 36));
}

void xbox_base_state::curthread_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	offs_t address;

	uint64_t fsbase = m_maincpu->state_int(44); // base of FS register
	address = (offs_t)fsbase + (offs_t)debugc_bios->parameter[7-1];
	address_space *tspace;
	if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
	{
		con.printf("Address is unmapped.\n");
		return;
	}

	uint32_t kthrd = tspace->read_dword_unaligned(address);
	con.printf("Current thread is %08X\n", kthrd);
	address = (offs_t)(kthrd + debugc_bios->parameter[8-1]);
	if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
		return;
	uint32_t topstack = tspace->read_dword_unaligned(address);
	con.printf("Current thread stack top is %08X\n", topstack);
	address = (offs_t)(kthrd + debugc_bios->parameter[4-1]);
	if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
		return;
	uint32_t tlsdata = tspace->read_dword_unaligned(address);
	if (tlsdata == 0)
		address = (offs_t)(topstack - debugc_bios->parameter[5-1] - debugc_bios->parameter[6-1]);
	else
		address = (offs_t)(tlsdata - debugc_bios->parameter[6-1]);
	if (m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
		con.printf("Current thread function is %08X\n", tspace->read_dword_unaligned(address));
}

void xbox_base_state::threadlist_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	address_space *tspace;

	con.printf("Pri. _KTHREAD   Stack  Function\n");
	con.printf("-------------------------------\n");
	for (int pri = 0; pri < 16; pri++)
	{
		uint32_t curr = debugc_bios->parameter[1 - 1] + pri * 8;
		uint32_t addr = curr;
		if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, addr, tspace))
			continue;
		uint32_t next = tspace->read_dword_unaligned(addr);

		while ((next != curr) && (next != 0))
		{
			uint32_t kthrd = next - debugc_bios->parameter[2 - 1];
			if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, kthrd, tspace))
				break;
			uint32_t topstack = tspace->read_dword_unaligned(kthrd + debugc_bios->parameter[3 - 1]);
			uint32_t tlsdata = tspace->read_dword_unaligned(kthrd + debugc_bios->parameter[4 - 1]);
			uint32_t function = 0;
			if (tlsdata == 0)
				addr = topstack - debugc_bios->parameter[5 - 1] - debugc_bios->parameter[6 - 1];
			else
				addr = tlsdata - debugc_bios->parameter[6 - 1];
			if (m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, addr, tspace))
				function = tspace->read_dword_unaligned(addr);
			con.printf(" %02d  %08x %08x %08x\n", pri, kthrd, topstack, function);
			addr = next;
			if (m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, addr, tspace))
				next = tspace->read_dword_unaligned(addr);
			else
				break;
		}
	}
}

void xbox_base_state::generate_irq_command(const std::vector<std::string_view> &params)
{
	uint64_t irq;

	if (params.size() < 2)
		return;
	if (!machine().debugger().console().validate_number_parameter(params[1], irq))
		return;
	if (irq > 15)
		return;
	if (irq == 2)
		return;
	debug_generate_irq((int)irq, true);
}

void xbox_base_state::nv2a_combiners_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	bool en = nvidia_nv2a->toggle_register_combiners_usage();
	if (en == true)
		con.printf("Register combiners enabled\n");
	else
		con.printf("Register combiners disabled\n");
}

void xbox_base_state::nv2a_wclipping_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	bool en = nvidia_nv2a->toggle_clipping_w_support();
	if (en == true)
		con.printf("W clipping enabled\n");
	else
		con.printf("W clipping disabled\n");
}

void xbox_base_state::waitvblank_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	bool en = nvidia_nv2a->toggle_wait_vblank_support();
	if (en == true)
		con.printf("Vblank method enabled\n");
	else
		con.printf("Vblank method disabled\n");
}

void xbox_base_state::grab_texture_command(const std::vector<std::string_view> &params)
{
	uint64_t type;

	if (params.size() < 3)
		return;
	if (!machine().debugger().console().validate_number_parameter(params[1], type))
		return;
	if (params[2].empty() || params[2].length() > 127)
		return;
	std::string filename(params[2]);
	nvidia_nv2a->debug_grab_texture((int)type, filename.c_str());
}

void xbox_base_state::grab_vprog_command(const std::vector<std::string_view> &params)
{
	uint32_t instruction[4];
	FILE *fil;

	if (params.size() < 2)
		return;
	if (params[1].empty() || params[1].length() > 127)
		return;
	std::string filename(params[1]);
	if ((fil = fopen(filename.c_str(), "wb")) == nullptr)
		return;
	for (int n = 0; n < 136; n++)
	{
		nvidia_nv2a->debug_grab_vertex_program_slot(n, instruction);
		fwrite(instruction, sizeof(uint32_t), 4, fil);
	}
	fclose(fil);
}

void xbox_base_state::vprogdis_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();

	if (params.size() < 3)
		return;

	uint64_t addr;
	if (!con.validate_number_parameter(params[1], addr))
		return;

	uint64_t length;
	if (!con.validate_number_parameter(params[2], length))
		return;

	uint64_t type = 0;
	if (params.size() > 3)
		if (!con.validate_number_parameter(params[3], type))
			return;

	vertex_program_disassembler vd;
	address_space *tspace;
	while (length > 0)
	{
		uint32_t instruction[4];
		if (type == 1)
		{
			offs_t address = (offs_t)addr;
			if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
				return;
			instruction[0] = tspace->read_dword_unaligned(address);
			instruction[1] = tspace->read_dword_unaligned(address + 4);
			instruction[2] = tspace->read_dword_unaligned(address + 8);
			instruction[3] = tspace->read_dword_unaligned(address + 12);
		}
		else
		{
			nvidia_nv2a->debug_grab_vertex_program_slot((int)addr, instruction);
		}

		char line[64];
		while (vd.disassemble(instruction, line) != 0)
			con.printf("%s\n", line);

		if (type == 1)
			addr = addr + 4 * 4;
		else
			addr++;

		length--;
	}
}

void xbox_base_state::vdeclaration_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();
	address_space *tspace;

	if (params.size() < 1)
		return;

	uint64_t addr;
	if (!con.validate_number_parameter(params[1], addr))
		return;

	for (int n = 128; n > 0; n--)
	{
		offs_t address = (offs_t)addr;
		if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
			return;
		uint32_t w = tspace->read_dword_unaligned(address);

		if (w == 0xffffffff)
		{
			con.printf("D3DVSD_END()\n");
			break;
		}
		switch ((w >> 29) & 7)
		{
		case 0:
			con.printf("D3DVSD_NOP()\n");
			break;
		case 1:
			if (w & (1 << 28))
				con.printf("D3DVSD_STREAM_TESS()\n");
			else
				con.printf("D3DVSD_STREAM(%d)\n", w & 0x1fffffff);
			break;
		case 2:
			if (w & 0x18000000)
				con.printf("D3DVSD_SKIPBYTES(%d)\n", (w >> 16) & 0xfff);
			else if (w & 0x10000000)
				con.printf("D3DVSD_SKIP(%d)\n", (w >> 16) & 0xfff);
			else
			{
				const char *t = "???";

				for (int s = 0; vertex_format_names[s].value != 0; s++)
				{
					if (vertex_format_names[s].value == ((w >> 16) & 0xfff))
					{
						t = vertex_format_names[s].name;
						break;
					}
				}
				con.printf("D3DVSD_REG(%d, D3DVSDT_%s)\n", w & 0xffff, t);
			}
			break;
		case 3:
			if (w & 0x10000000)
				con.printf("D3DVSD_TESSUV(%d)\n", w & 0xf);
			else
				con.printf("D3DVSD_TESSNORMAL(%d, %d)\n", (w >> 20) & 0xf, w & 0xf);
			break;
		case 4:
			con.printf("D3DVSD_CONST(%d, %d)\n", (w & 0xff) - 96, (w >> 25) & 0xf);
			for (int n = 0; n < ((w >> 23) & 0x3c); n++)
			{
				addr += 4;
				address = (offs_t)addr;
				if (!m_maincpu->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
					return;
				w = tspace->read_dword_unaligned(address);
				con.printf("%08x\n", w);
			}
			break;
		default:
			con.printf("??? %08x\n", w);
			n = 0;
		}
		addr += 4;
	}
}

void xbox_base_state::help_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();

	con.printf("Available Xbox commands:\n");
	con.printf("  xbox dump_string,<address> -- Dump _STRING object at <address>\n");
	con.printf("  xbox dump_process,<address> -- Dump _PROCESS object at <address>\n");
	con.printf("  xbox dump_list,<address>[,<offset>] -- Dump _LIST_ENTRY chain starting at <address>\n");
	con.printf("  xbox dump_dpc,<address> -- Dump _KDPC object at <address>\n");
	con.printf("  xbox dump_timer,<address> -- Dump _KTIMER object at <address>\n");
	con.printf("  xbox curthread -- Print information about current thread\n");
	con.printf("  xbox threadlist -- list of currently active threads\n");
	con.printf("  xbox irq,<number> -- Generate interrupt with irq number 0-15\n");
	con.printf("  xbox nv2a_combiners -- Toggle use of register combiners\n");
	con.printf("  xbox nv2a_wclipping -- Toggle use of negative w vertex clipping\n");
	con.printf("  xbox waitvblank -- Toggle support for wait vblank method\n");
	con.printf("  xbox grab_texture,<type>,<filename> -- Save to <filename> the next used texture of type <type>\n");
	con.printf("  xbox grab_vprog,<filename> -- save current vertex program instruction slots to <filename>\n");
	con.printf("  xbox vprogdis,<address>,<length>[,<type>] -- disassemble <length> vertex program instructions at <address> of <type>\n");
	con.printf("  xbox vdeclaration,<address> -- decode vertex program declaration at <address>\n");
	con.printf("  xbox help -- this list\n");
}

void xbox_base_state::xbox_debug_commands(const std::vector<std::string_view> &params)
{
	if (params.size() < 1)
		return;
	if (params[0] == "dump_string")
		dump_string_command(params);
	else if (params[0] == "dump_process")
		dump_process_command(params);
	else if (params[0] == "dump_list")
		dump_list_command(params);
	else if (params[0] == "dump_dpc")
		dump_dpc_command(params);
	else if (params[0] == "dump_timer")
		dump_timer_command(params);
	else if (params[0] == "curthread")
		curthread_command(params);
	else if (params[0] == "threadlist")
		threadlist_command(params);
	else if (params[0] == "irq")
		generate_irq_command(params);
	else if (params[0] == "nv2a_combiners")
		nv2a_combiners_command(params);
	else if (params[0] == "nv2a_wclipping")
		nv2a_wclipping_command(params);
	else if (params[0] == "waitvblank")
		waitvblank_command(params);
	else if (params[0] == "grab_texture")
		grab_texture_command(params);
	else if (params[0] == "grab_vprog")
		grab_vprog_command(params);
	else if (params[0] == "vprogdis")
		vprogdis_command(params);
	else if (params[0] == "vdeclaration")
		vdeclaration_command(params);
	else
		help_command(params);
}

void xbox_base_state::debug_generate_irq(int irq, bool active)
{
	int state;

	if (active)
	{
		debug_irq_active = true;
		debug_irq_number = irq;
		state = 1;
	}
	else
	{
		debug_irq_active = false;
		state = 0;
	}
	mcpxlpc->debug_generate_irq(irq, state);
}

void xbox_base_state::vblank_callback(int state)
{
	nvidia_nv2a->vblank_callback(state);
}

uint32_t xbox_base_state::screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return nvidia_nv2a->screen_update_callback(screen, bitmap, cliprect);
}

/*
 * PIC & PIT
 */

void xbox_base_state::maincpu_interrupt(int state)
{
	m_maincpu->set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

IRQ_CALLBACK_MEMBER(xbox_base_state::irq_callback)
{
	int r;
	r = mcpxlpc->acknowledge();
	if (debug_irq_active)
		debug_generate_irq(debug_irq_number, false);
	return r;
}

/*
 * SMbus devices
 */

/*
 * PIC16LC
 */

DEFINE_DEVICE_TYPE(XBOX_PIC16LC, xbox_pic16lc_device, "pic16lc", "XBOX PIC16LC")

xbox_pic16lc_device::xbox_pic16lc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XBOX_PIC16LC, tag, owner, clock)
{
}

int xbox_pic16lc_device::execute_command(int command, int rw, int data)
{
	if (rw == 1) { // read
		if (command == 0)
		{
			if (buffer[0] == 'D')
				buffer[0] = 'X';
			else if (buffer[0] == 'X')
				buffer[0] = 'B';
			else if (buffer[0] == 'B')
				buffer[0] = 'D';
		}
		logerror("pic16lc: %d %d %d\n", command, rw, buffer[command]);
		return buffer[command];
	}
	else
		if (command == 0)
			buffer[0] = 'B';
		else
			buffer[command] = (uint8_t)data;
	logerror("pic16lc: %d %d %d\n", command, rw, data);
	return 0;
}

void xbox_pic16lc_device::device_start()
{
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = 'B';
	buffer[4] = 0; // A/V connector, 0=scart 2=vga 4=svideo 7=none
	// PIC challenge handshake data
	buffer[0x1c] = 0x0c;
	buffer[0x1d] = 0x0d;
	buffer[0x1e] = 0x0e;
	buffer[0x1f] = 0x0f;
}

void xbox_pic16lc_device::device_reset()
{
}

/*
 * CX25871
 */

DEFINE_DEVICE_TYPE(XBOX_CX25871, xbox_cx25871_device, "cx25871", "XBOX CX25871")

xbox_cx25871_device::xbox_cx25871_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XBOX_CX25871, tag, owner, clock)
{
}

int xbox_cx25871_device::execute_command(int command, int rw, int data)
{
	logerror("cx25871: %d %d %d\n", command, rw, data);
	return 0;
}

void xbox_cx25871_device::device_start()
{
}

void xbox_cx25871_device::device_reset()
{
}

/*
 * EEPROM
 */

// let's try to fake the missing eeprom, make sure its ntsc, otherwise chihiro will show an error
static int dummyeeprom[256] = {
	0x39, 0xe3, 0xcc, 0x81, 0xb0, 0xa9, 0x97, 0x09, 0x57, 0xac, 0x57, 0x12, 0xf7, 0xc2, 0xc0, 0x21, 0xce, 0x0d, 0x0a, 0xdb, 0x20, 0x7a, 0xf3, 0xff,
	0xdf, 0x67, 0xed, 0xf4, 0xf8, 0x95, 0x5c, 0xd0, 0x9b, 0xef, 0x7b, 0x81, 0xda, 0xd5, 0x98, 0xc1, 0xb1, 0xb3, 0x74, 0x18, 0x86, 0x05, 0xe2, 0x7c,
	0xd1, 0xad, 0xc9, 0x90, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x00, 0x00,
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xab, 0xcd, 0xef, 0xba, 0xdc, 0xfe, 0xa1, 0xb2, 0xc3, 0xd3, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

DEFINE_DEVICE_TYPE(XBOX_EEPROM, xbox_eeprom_device, "eeprom", "XBOX EEPROM")

xbox_eeprom_device::xbox_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XBOX_EEPROM, tag, owner, clock)
{
}

int xbox_eeprom_device::execute_command(int command, int rw, int data)
{
	if (command >= 112)
		return 0;
	if (rw == 1) // if reading
	{
		// hack to avoid hanging if eeprom contents are not correct
		// removing this would need dumping the serial eeprom on the chihiro xbox board
		if (command == 0)
		{
			hack_eeprom();
		}
		data = dummyeeprom[command] + dummyeeprom[command + 1] * 256;
		logerror("eeprom: %d %d %d\n", command, rw, data);
		return data;
	}
	logerror("eeprom: %d %d %d\n", command, rw, data);
	dummyeeprom[command] = data;
	return 0;
}

void xbox_eeprom_device::device_start()
{
}

void xbox_eeprom_device::device_reset()
{
}

/*
 * Super-io connected to lpc bus
 */

DEFINE_DEVICE_TYPE(XBOX_SUPERIO, xbox_superio_device, "superio_device", "XBOX debug SuperIO")

xbox_superio_device::xbox_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XBOX_SUPERIO, tag, owner, clock)
	, configuration_mode(false)
	, index(0)
	, selected(0)
{

}

void xbox_superio_device::internal_io_map(address_map &map)
{
	map(0x002e, 0x002f).rw(FUNC(xbox_superio_device::read), FUNC(xbox_superio_device::write));
	map(0x03f8, 0x03ff).rw(FUNC(xbox_superio_device::read_rs232), FUNC(xbox_superio_device::write_rs232));
}

void xbox_superio_device::map_extra(address_space *memory_space, address_space *io_space)
{
	memspace = memory_space;
	iospace = io_space;
	io_space->install_device(0, 0xffff, *this, &xbox_superio_device::internal_io_map);
}

void xbox_superio_device::set_host(int device_index, lpcbus_host_interface *host)
{
	lpchost = host;
	lpcindex = device_index;
}

uint32_t xbox_superio_device::dma_transfer(int channel, dma_operation operation, dma_size size, uint32_t data)
{
	logerror("LPC dma transfer attempted on channel %d\n", channel);
	return 0;
}

void xbox_superio_device::device_start()
{
	memset(registers, 0, sizeof(registers));
	registers[0][0x26] = 0x2e; // Configuration port address byte 0
}

uint8_t xbox_superio_device::read(offs_t offset)
{
	if (configuration_mode == false)
		return 0;
	if (offset == 0) // index port 0x2e
		return index;
	if (offset == 1)
	{
		// data port 0x2f
		if (index < 0x30)
			return registers[0][index];
		return registers[selected][index];
	}
	return 0;
}

void xbox_superio_device::write(offs_t offset, uint8_t data)
{
	if (configuration_mode == false)
	{
		// config port 0x2e
		if ((offset == 0) && (data == 0x55))
			configuration_mode = true;
		return;
	}
	if ((offset == 0) && (data == 0xaa))
	{
		// config port 0x2e
		configuration_mode = false;
		return;
	}
	if (offset == 0)
	{
		// index port 0x2e
		index = data;
	}
	if (offset == 1)
	{
		// data port 0x2f
		if (index < 0x30)
		{
			registers[0][index] = data;
			selected = registers[0][7];
		}
		else
		{
			registers[selected][index] = data;
			//if ((superiost.selected == 4) && (superiost.index == 0x30) && (data != 0))
			//  ; // add handlers 0x3f8- +7
		}
	}
}

uint8_t xbox_superio_device::read_rs232(offs_t offset)
{
	if (offset == 5)
		return 0x20;
	return 0;
}

void xbox_superio_device::write_rs232(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		printf("%c", data);
	}
}

void xbox_base_state::machine_start()
{
	find_debug_params();
	nvidia_nv2a = subdevice<nv2a_gpu_device>("pci:1e.0:00.0")->debug_get_renderer();
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("xbox", CMDFLAG_CUSTOM_HELP, 1, 4, std::bind(&xbox_base_state::xbox_debug_commands, this, _1));
	}
	subdevice<xbox_eeprom_device>("pci:01.1:154")->hack_eeprom =
		[&](void)
	{
		hack_eeprom();
	};
	subdevice<mcpx_ohci_device>("pci:02.0")->set_hack_callback(
		[&](void)
	{
		hack_usb();
	}
	);
	// savestates
	save_item(NAME(debug_irq_active));
	save_item(NAME(debug_irq_number));
}

#if 0
void xbox_base_state::xbox_base_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram(); // 128 megabytes
	map(0xf0000000, 0xf7ffffff).ram().share("nv2a_share"); // 3d accelerator wants this
	map(0xfd000000, 0xfdffffff).ram().rw(FUNC(xbox_base_state::geforce_r), FUNC(xbox_base_state::geforce_w));
	map(0xfed00000, 0xfed003ff).rw(FUNC(xbox_base_state::ohci_usb_r), FUNC(xbox_base_state::ohci_usb_w));
	map(0xfed08000, 0xfed083ff).rw(FUNC(xbox_base_state::ohci_usb2_r), FUNC(xbox_base_state::ohci_usb2_w));
	map(0xfe800000, 0xfe87ffff).rw(FUNC(xbox_base_state::audio_apu_r), FUNC(xbox_base_state::audio_apu_w));
	map(0xfec00000, 0xfec00fff).rw(FUNC(xbox_base_state::audio_ac93_r), FUNC(xbox_base_state::audio_ac93_w));
	map(0xfef00000, 0xfef003ff).rw(FUNC(xbox_base_state::network_r), FUNC(xbox_base_state::network_w));
}

void xbox_base_state::xbox_base_map_io(address_map &map)
{
	map(0x01f0, 0x01f7).rw("pci:09.0:ide1", FUNC(bus_master_ide_controller_device::cs0_r), FUNC(bus_master_ide_controller_device::cs0_w));
	map(0x002e, 0x002f).rw(FUNC(xbox_base_state::superio_read), FUNC(xbox_base_state::superio_write));
	map(0x03f8, 0x03ff).rw(FUNC(xbox_base_state::superiors232_read), FUNC(xbox_base_state::superiors232_write));
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
	map(0x8000, 0x80ff).rw(FUNC(xbox_base_state::dummy_r), FUNC(xbox_base_state::dummy_w)); // lpc bridge
	map(0xc000, 0xc00f).rw(FUNC(xbox_base_state::smbus_r), FUNC(xbox_base_state::smbus_w));
	map(0xc200, 0xc21f).rw(FUNC(xbox_base_state::smbus2_r), FUNC(xbox_base_state::smbus2_w));
	map(0xd000, 0xd0ff).noprw(); // ac97
	map(0xd200, 0xd27f).noprw(); // ac97
	map(0xe000, 0xe007).rw(FUNC(xbox_base_state::networkio_r), FUNC(xbox_base_state::networkio_w));
	map(0xff60, 0xff6f).rw("ide", FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
}
#endif

void xbox_base_state::xbox_base(machine_config &config)
{
	/* basic machine hardware */
	PENTIUM3(config, m_maincpu, 733333333); /* Wrong! family 6 model 8 stepping 10 */
	m_maincpu->set_irq_acknowledge_callback(FUNC(xbox_base_state::irq_callback));

	config.set_maximum_quantum(attotime::from_hz(6000));

	PCI_ROOT(config,        "pci", 0);
	NV2A_HOST(config,       "pci:00.0", 0, m_maincpu);
	NV2A_RAM(config,        "pci:00.3", 0, 128); // 128 megabytes
	MCPX_ISALPC(config,     "pci:01.0", 0, 0).interrupt_output().set(FUNC(xbox_base_state::maincpu_interrupt));
	XBOX_SUPERIO(config,    "pci:01.0:0", 0);
	subdevice<mcpx_isalpc_device>("pci:01.0")->set_dma_space(m_maincpu, AS_PROGRAM);
	MCPX_SMBUS(config,      "pci:01.1", 0, 0).interrupt_handler().set("pci:01.0", FUNC(mcpx_isalpc_device::irq11)); //.set(FUNC(xbox_base_state::smbus_interrupt_changed));
	XBOX_PIC16LC(config,    "pci:01.1:110", 0); // these 3 are on smbus number 1
	XBOX_CX25871(config,    "pci:01.1:145", 0);
	XBOX_EEPROM(config,     "pci:01.1:154", 0);
	MCPX_OHCI(config,       "pci:02.0", 0, 0).interrupt_handler().set("pci:01.0", FUNC(mcpx_isalpc_device::irq1));  //.set(FUNC(xbox_base_state::ohci_usb_interrupt_changed));
	MCPX_OHCI(config,       "pci:03.0", 0, 0);
	MCPX_ETH(config,        "pci:04.0", 0);
	MCPX_APU(config,        "pci:05.0", 0, 0, m_maincpu);
	MCPX_AC97_AUDIO(config, "pci:06.0", 0, 0);
	MCPX_AC97_MODEM(config, "pci:06.1", 0);
	PCI_BRIDGE(config,      "pci:08.0", 0, 0x10de01b8, 0);
	MCPX_IDE(config,        "pci:09.0", 0, 0).pri_interrupt_handler().set("pci:01.0", FUNC(mcpx_isalpc_device::irq14));  //.set(FUNC(xbox_base_state::ide_interrupt_changed));
	subdevice<mcpx_ide_device>("pci:09.0")->set_bus_master_space(m_maincpu, AS_PROGRAM);
	NV2A_AGP(config,        "pci:1e.0", 0, 0x10de01b7, 0);
	NV2A_GPU(config,        "pci:1e.0:00.0", 0, m_maincpu).interrupt_handler().set("pci:01.0", FUNC(mcpx_isalpc_device::irq3)); //.set(FUNC(xbox_base_state::nv2a_interrupt_changed));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));  /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_screen_update(FUNC(xbox_base_state::screen_update_callback));
	screen.screen_vblank().set(FUNC(xbox_base_state::vblank_callback));
}
