// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

#include <functional>

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/idectrl.h"
#include "video/poly.h"
#include "bitmap.h"
#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "includes/chihiro.h"
#include "includes/xbox.h"
#include "includes/xbox_usb.h"

#define LOG_PCI
//#define LOG_AUDIO
#define USB_HACK_ENABLED

void xbox_base_state::dump_string_command(int ref, int params, const char **param)
{
	address_space &space = m_maincpu->space();

	if (params < 1)
		return;

	UINT64 addr;
	if (!machine().debugger().commands().validate_number_parameter(param[0], &addr))
		return;

	offs_t address = (offs_t)addr;
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
	{
		machine().debugger().console().printf("Address is unmapped.\n");
		return;
	}

	UINT32 length = space.read_word_unaligned(address);
	UINT32 maximumlength = space.read_word_unaligned(address + 2);
	offs_t buffer = space.read_dword_unaligned(address + 4);
	machine().debugger().console().printf("Length %d word\n", length);
	machine().debugger().console().printf("MaximumLength %d word\n", maximumlength);
	machine().debugger().console().printf("Buffer %08X byte* ", buffer);
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, buffer))
	{
		machine().debugger().console().printf("\nBuffer is unmapped.\n");
		return;
	}

	if (length > 256)
		length = 256;

	for (int a = 0; a < length; a++)
	{
		UINT8 c = space.read_byte(buffer + a);
		machine().debugger().console().printf("%c", c);
	}
	machine().debugger().console().printf("\n");
}

void xbox_base_state::dump_process_command(int ref, int params, const char **param)
{
	address_space &space = m_maincpu->space();

	if (params < 1)
		return;

	UINT64 addr;
	if (!machine().debugger().commands().validate_number_parameter(param[0], &addr))
		return;

	offs_t address = (offs_t)addr;
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
	{
		machine().debugger().console().printf("Address is unmapped.\n");
		return;
	}
	machine().debugger().console().printf("ReadyListHead {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address), space.read_dword_unaligned(address + 4));
	machine().debugger().console().printf("ThreadListHead {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address + 8), space.read_dword_unaligned(address + 12));
	machine().debugger().console().printf("StackCount %d dword\n", space.read_dword_unaligned(address + 16));
	machine().debugger().console().printf("ThreadQuantum %d dword\n", space.read_dword_unaligned(address + 20));
	machine().debugger().console().printf("BasePriority %d byte\n", space.read_byte(address + 24));
	machine().debugger().console().printf("DisableBoost %d byte\n", space.read_byte(address + 25));
	machine().debugger().console().printf("DisableQuantum %d byte\n", space.read_byte(address + 26));
	machine().debugger().console().printf("_padding %d byte\n", space.read_byte(address + 27));
}

void xbox_base_state::dump_list_command(int ref, int params, const char **param)
{
	address_space &space = m_maincpu->space();

	if (params < 1)
		return;

	UINT64 addr;
	if (!machine().debugger().commands().validate_number_parameter(param[0], &addr))
		return;

	UINT64 offs = 0;
	offs_t offset = 0;
	if (params >= 2)
	{
		if (!machine().debugger().commands().validate_number_parameter(param[1], &offs))
			return;
		offset = (offs_t)offs;
	}

	UINT64 start = addr;
	offs_t address = (offs_t)addr;
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
	{
		machine().debugger().console().printf("Address is unmapped.\n");
		return;
	}
	if (params >= 2)
		machine().debugger().console().printf("Entry    Object\n");
	else
		machine().debugger().console().printf("Entry\n");

	UINT64 old;
	for (int num = 0; num < 32; num++)
	{
		if (params >= 2)
			machine().debugger().console().printf("%08X %08X\n", (UINT32)addr, (offs_t)addr - offset);
		else
			machine().debugger().console().printf("%08X\n", (UINT32)addr);
		old = addr;
		addr = space.read_dword_unaligned(address);
		if (addr == start)
			break;
		if (addr == old)
			break;
		address = (offs_t)addr;
		if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
			break;
	}
}

void xbox_base_state::dump_dpc_command(int ref, int params, const char **param)
{
	address_space &space = m_maincpu->space();

	if (params < 1)
		return;

	UINT64 addr;
	if (!machine().debugger().commands().validate_number_parameter(param[0], &addr))
		return;

	offs_t address = (offs_t)addr;
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
	{
		machine().debugger().console().printf("Address is unmapped.\n");
		return;
	}
	machine().debugger().console().printf("Type %d word\n", space.read_word_unaligned(address));
	machine().debugger().console().printf("Inserted %d byte\n", space.read_byte(address + 2));
	machine().debugger().console().printf("Padding %d byte\n", space.read_byte(address + 3));
	machine().debugger().console().printf("DpcListEntry {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address + 4), space.read_dword_unaligned(address + 8));
	machine().debugger().console().printf("DeferredRoutine %08X dword\n", space.read_dword_unaligned(address + 12));
	machine().debugger().console().printf("DeferredContext %08X dword\n", space.read_dword_unaligned(address + 16));
	machine().debugger().console().printf("SystemArgument1 %08X dword\n", space.read_dword_unaligned(address + 20));
	machine().debugger().console().printf("SystemArgument2 %08X dword\n", space.read_dword_unaligned(address + 24));
}

void xbox_base_state::dump_timer_command(int ref, int params, const char **param)
{
	address_space &space = m_maincpu->space();

	if (params < 1)
		return;

	UINT64 addr;
	if (!machine().debugger().commands().validate_number_parameter(param[0], &addr))
		return;

	offs_t address = (offs_t)addr;
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
	{
		machine().debugger().console().printf("Address is unmapped.\n");
		return;
	}
	machine().debugger().console().printf("Header.Type %d byte\n", space.read_byte(address));
	machine().debugger().console().printf("Header.Absolute %d byte\n", space.read_byte(address + 1));
	machine().debugger().console().printf("Header.Size %d byte\n", space.read_byte(address + 2));
	machine().debugger().console().printf("Header.Inserted %d byte\n", space.read_byte(address + 3));
	machine().debugger().console().printf("Header.SignalState %08X dword\n", space.read_dword_unaligned(address + 4));
	machine().debugger().console().printf("Header.WaitListEntry {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address + 8), space.read_dword_unaligned(address + 12));
	machine().debugger().console().printf("%s", string_format("DueTime %I64x qword\n", (INT64)space.read_qword_unaligned(address + 16)).c_str());
	machine().debugger().console().printf("TimerListEntry {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address + 24), space.read_dword_unaligned(address + 28));
	machine().debugger().console().printf("Dpc %08X dword\n", space.read_dword_unaligned(address + 32));
	machine().debugger().console().printf("Period %d dword\n", space.read_dword_unaligned(address + 36));
}

void xbox_base_state::curthread_command(int ref, int params, const char **param)
{
	address_space &space = m_maincpu->space();

	UINT64 fsbase = m_maincpu->state_int(44);
	offs_t address = (offs_t)fsbase + 0x28;
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
	{
		machine().debugger().console().printf("Address is unmapped.\n");
		return;
	}

	UINT32 kthrd = space.read_dword_unaligned(address);
	machine().debugger().console().printf("Current thread is %08X\n", kthrd);

	address = (offs_t)kthrd + 0x1c;
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
		return;

	UINT32 topstack = space.read_dword_unaligned(address);
	machine().debugger().console().printf("Current thread stack top is %08X\n", topstack);

	address = (offs_t)kthrd + 0x28;
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
		return;

	UINT32 tlsdata = space.read_dword_unaligned(address);
	if (tlsdata == 0)
		address = (offs_t)topstack - 0x210 - 8;
	else
		address = (offs_t)tlsdata - 8;
	if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, address))
		return;
	machine().debugger().console().printf("Current thread function is %08X\n", space.read_dword_unaligned(address));
}

void xbox_base_state::generate_irq_command(int ref, int params, const char **param)
{
	UINT64 irq;

	if (params < 1)
		return;
	if (!machine().debugger().commands().validate_number_parameter(param[0], &irq))
		return;
	if (irq > 15)
		return;
	if (irq == 2)
		return;
	debug_generate_irq((int)irq, true);
}

void xbox_base_state::nv2a_combiners_command(int ref, int params, const char **param)
{
	int en = nvidia_nv2a->toggle_register_combiners_usage();
	if (en != 0)
		machine().debugger().console().printf("Register combiners enabled\n");
	else
		machine().debugger().console().printf("Register combiners disabled\n");
}

void xbox_base_state::waitvblank_command(int ref, int params, const char **param)
{
	int en = nvidia_nv2a->toggle_wait_vblank_support();
	if (en != 0)
		machine().debugger().console().printf("Vblank method enabled\n");
	else
		machine().debugger().console().printf("Vblank method disabled\n");
}

void xbox_base_state::grab_texture_command(int ref, int params, const char **param)
{
	UINT64 type;

	if (params < 2)
		return;
	if (!machine().debugger().commands().validate_number_parameter(param[0], &type))
		return;
	if ((param[1][0] == 0) || (strlen(param[1]) > 127))
		return;
	nvidia_nv2a->debug_grab_texture((int)type, param[1]);
}

void xbox_base_state::grab_vprog_command(int ref, int params, const char **param)
{
	UINT32 instruction[4];
	FILE *fil;

	if (params < 1)
		return;
	if ((param[0][0] == 0) || (strlen(param[0]) > 127))
		return;
	if ((fil = fopen(param[0], "wb")) == nullptr)
		return;
	for (int n = 0; n < 136; n++) {
		nvidia_nv2a->debug_grab_vertex_program_slot(n, instruction);
		fwrite(instruction, sizeof(UINT32), 4, fil);
	}
	fclose(fil);
}

void xbox_base_state::vprogdis_command(int ref, int params, const char **param)
{
	address_space &space = m_maincpu->space();

	if (params < 2)
		return;

	UINT64 address;
	if (!machine().debugger().commands().validate_number_parameter(param[0], &address))
		return;

	UINT64 length;
	if (!machine().debugger().commands().validate_number_parameter(param[1], &length))
		return;

	UINT64 type = 0;
	if (params > 2)
		if (!machine().debugger().commands().validate_number_parameter(param[2], &type))
			return;

	vertex_program_disassembler vd;
	while (length > 0)
	{
		UINT32 instruction[4];
		if (type == 1)
		{
			offs_t addr = (offs_t)address;
			if (!m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, addr))
				return;
			instruction[0] = space.read_dword_unaligned(address);
			instruction[1] = space.read_dword_unaligned(address + 4);
			instruction[2] = space.read_dword_unaligned(address + 8);
			instruction[3] = space.read_dword_unaligned(address + 12);
		}
		else
		{
			nvidia_nv2a->debug_grab_vertex_program_slot((int)address, instruction);
		}

		char line[64];
		while (vd.disassemble(instruction, line) != 0)
			machine().debugger().console().printf("%s\n", line);

		if (type == 1)
			address = address + 4 * 4;
		else
			address++;

		length--;
	}
}

void xbox_base_state::help_command(int ref, int params, const char **param)
{
	machine().debugger().console().printf("Available Xbox commands:\n");
	machine().debugger().console().printf("  xbox dump_string,<address> -- Dump _STRING object at <address>\n");
	machine().debugger().console().printf("  xbox dump_process,<address> -- Dump _PROCESS object at <address>\n");
	machine().debugger().console().printf("  xbox dump_list,<address>[,<offset>] -- Dump _LIST_ENTRY chain starting at <address>\n");
	machine().debugger().console().printf("  xbox dump_dpc,<address> -- Dump _KDPC object at <address>\n");
	machine().debugger().console().printf("  xbox dump_timer,<address> -- Dump _KTIMER object at <address>\n");
	machine().debugger().console().printf("  xbox curthread -- Print information about current thread\n");
	machine().debugger().console().printf("  xbox irq,<number> -- Generate interrupt with irq number 0-15\n");
	machine().debugger().console().printf("  xbox nv2a_combiners -- Toggle use of register combiners\n");
	machine().debugger().console().printf("  xbox waitvblank -- Toggle support for wait vblank method\n");
	machine().debugger().console().printf("  xbox grab_texture,<type>,<filename> -- Save to <filename> the next used texture of type <type>\n");
	machine().debugger().console().printf("  xbox grab_vprog,<filename> -- save current vertex program instruction slots to <filename>\n");
	machine().debugger().console().printf("  xbox vprogdis,<address>,<length>[,<type>] -- disassemble <lenght> vertex program instructions at <address> of <type>\n");
	machine().debugger().console().printf("  xbox help -- this list\n");
}

void xbox_base_state::xbox_debug_commands(int ref, int params, const char **param)
{
	if (params < 1)
		return;
	if (strcmp("dump_string", param[0]) == 0)
		dump_string_command(ref, params - 1, param + 1);
	else if (strcmp("dump_process", param[0]) == 0)
		dump_process_command(ref, params - 1, param + 1);
	else if (strcmp("dump_list", param[0]) == 0)
		dump_list_command(ref, params - 1, param + 1);
	else if (strcmp("dump_dpc", param[0]) == 0)
		dump_dpc_command(ref, params - 1, param + 1);
	else if (strcmp("dump_timer", param[0]) == 0)
		dump_timer_command(ref, params - 1, param + 1);
	else if (strcmp("curthread", param[0]) == 0)
		curthread_command(ref, params - 1, param + 1);
	else if (strcmp("irq", param[0]) == 0)
		generate_irq_command(ref, params - 1, param + 1);
	else if (strcmp("nv2a_combiners", param[0]) == 0)
		nv2a_combiners_command(ref, params - 1, param + 1);
	else if (strcmp("waitvblank", param[0]) == 0)
		waitvblank_command(ref, params - 1, param + 1);
	else if (strcmp("grab_texture", param[0]) == 0)
		grab_texture_command(ref, params - 1, param + 1);
	else if (strcmp("grab_vprog", param[0]) == 0)
		grab_vprog_command(ref, params - 1, param + 1);
	else if (strcmp("vprogdis", param[0]) == 0)
		vprogdis_command(ref, params - 1, param + 1);
	else
		help_command(ref, params - 1, param + 1);
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
	switch (irq)
	{
	case 0:
		xbox_base_devs.pic8259_1->ir0_w(state);
		break;
	case 1:
		xbox_base_devs.pic8259_1->ir1_w(state);
		break;
	case 3:
		xbox_base_devs.pic8259_1->ir3_w(state);
		break;
	case 4:
		xbox_base_devs.pic8259_1->ir4_w(state);
		break;
	case 5:
		xbox_base_devs.pic8259_1->ir5_w(state);
		break;
	case 6:
		xbox_base_devs.pic8259_1->ir6_w(state);
		break;
	case 7:
		xbox_base_devs.pic8259_1->ir7_w(state);
		break;
	case 8:
		xbox_base_devs.pic8259_2->ir0_w(state);
		break;
	case 9:
		xbox_base_devs.pic8259_2->ir1_w(state);
		break;
	case 10:
		xbox_base_devs.pic8259_2->ir2_w(state);
		break;
	case 11:
		xbox_base_devs.pic8259_2->ir3_w(state);
		break;
	case 12:
		xbox_base_devs.pic8259_2->ir4_w(state);
		break;
	case 13:
		xbox_base_devs.pic8259_2->ir5_w(state);
		break;
	case 14:
		xbox_base_devs.pic8259_2->ir6_w(state);
		break;
	case 15:
		xbox_base_devs.pic8259_2->ir7_w(state);
		break;
	}
}

void xbox_base_state::vblank_callback(screen_device &screen, bool state)
{
	nvidia_nv2a->vblank_callback(screen, state);
}

UINT32 xbox_base_state::screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return nvidia_nv2a->screen_update_callback(screen, bitmap, cliprect);
}

READ32_MEMBER(xbox_base_state::geforce_r)
{
	return nvidia_nv2a->geforce_r(space, offset, mem_mask);
}

WRITE32_MEMBER(xbox_base_state::geforce_w)
{
	nvidia_nv2a->geforce_w(space, offset, data, mem_mask);
}

static UINT32 geforce_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
#ifdef LOG_PCI
	busdevice->logerror("  bus:1 device:NV_2A function:%d register:%d mask:%08X\n",function,reg,mem_mask);
#endif
	return 0;
}

static void geforce_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
#ifdef LOG_PCI
	busdevice->logerror("  bus:1 device:NV_2A function:%d register:%d data:%08X mask:%08X\n",function,reg,data,mem_mask);
#endif
}

/*
 * Audio
 */

READ32_MEMBER(xbox_base_state::audio_apu_r)
{
#ifdef LOG_AUDIO
	logerror("Audio_APU: read from %08X mask %08X\n", 0xfe800000 + offset * 4, mem_mask);
#endif
	if (offset == 0x20010 / 4) // some kind of internal counter or state value
		return 0x20 + 4 + 8 + 0x48 + 0x80;
	return apust.memory[offset];
}

WRITE32_MEMBER(xbox_base_state::audio_apu_w)
{
	//UINT32 old;
	UINT32 v;

#ifdef LOG_AUDIO
	logerror("Audio_APU: write at %08X mask %08X value %08X\n", 0xfe800000 + offset * 4, mem_mask, data);
#endif
	//old = apust.memory[offset];
	apust.memory[offset] = data;
	if (offset == 0x02040 / 4) // address of memory area with scatter-gather info (gpdsp scratch dma)
		apust.gpdsp_sgaddress = data;
	if (offset == 0x020d4 / 4) { // block count (gpdsp)
		apust.gpdsp_sgblocks = data;
		apust.gpdsp_address = apust.space->read_dword(apust.gpdsp_sgaddress); // memory address of first block
		apust.timer->enable();
		apust.timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
	}
	if (offset == 0x02048 / 4) // (epdsp scratch dma)
		apust.epdsp_sgaddress = data;
	if (offset == 0x020dc / 4) // (epdsp)
		apust.epdsp_sgblocks = data;
	if (offset == 0x0204c / 4) // address of memory area with information about blocks
		apust.unknown_sgaddress = data;
	if (offset == 0x020e0 / 4) // block count - 1
		apust.unknown_sgblocks = data;
	if (offset == 0x0202c / 4) { // address of memory area with 0x80 bytes for each voice
		apust.voicedata_address = data;
		return;
	}
	if (offset == 0x04024 / 4) // offset in memory area indicated by 0x204c (analog output ?)
		return;
	if (offset == 0x04034 / 4) // size
		return;
	if (offset == 0x04028 / 4) // offset in memory area indicated by 0x204c (digital output ?)
		return;
	if (offset == 0x04038 / 4) // size
		return;
	if (offset == 0x20804 / 4) { // block number for scatter-gather heap that stores sampled audio to be played
		if (data >= 1024) {
			logerror("Audio_APU: sg block number too high, increase size of voices_heap_blockaddr\n");
			apust.memory[offset] = 1023;
		}
		return;
	}
	if (offset == 0x20808 / 4) { // block address for scatter-gather heap that stores sampled audio to be played
		apust.voices_heap_blockaddr[apust.memory[0x20804 / 4]] = data;
		return;
	}
	if (offset == 0x202f8 / 4) { // voice number for parameters ?
		apust.voice_number = data;
		return;
	}
	if (offset == 0x202fc / 4) // 1 when accessing voice parameters 0 otherwise
		return;
	if (offset == 0x20304 / 4) { // format
		/*
		bits 28-31 sample format:
		0  8-bit pcm
		5  16-bit pcm
		10 adpcm ?
		14 24-bit pcm
		15 32-bit pcm
		bits 16-20 number of channels - 1:
		0  mono
		1  stereo
		*/
		return;
	}
	if (offset == 0x2037c / 4) { // value related to sample rate
		INT16 v0 = (INT16)(data >> 16); // upper 16 bits as a signed 16 bit value
		float vv = ((float)v0) / 4096.0f; // divide by 4096
		float vvv = powf(2, vv); // two to the vv
		int f = vvv*48000.0f; // sample rate
		apust.voices_frequency[apust.voice_number] = f;
		return;
	}
	if (offset == 0x203a0 / 4) // start offset of data in scatter-gather heap
		return;
	if (offset == 0x203a4 / 4) { // first sample to play
		apust.voices_position_start[apust.voice_number] = data * 1000;
		return;
	}
	if (offset == 0x203dc / 4) { // last sample to play
		apust.voices_position_end[apust.voice_number] = data * 1000;
		return;
	}
	if (offset == 0x2010c / 4) // voice processor 0 idle 1 not idle ?
		return;
	if (offset == 0x20124 / 4) { // voice number to activate ?
		v = apust.voice_number;
		apust.voices_active[v >> 6] |= ((UINT64)1 << (v & 63));
		apust.voices_position[v] = apust.voices_position_start[apust.voice_number];
		apust.voices_position_increment[apust.voice_number] = apust.voices_frequency[apust.voice_number];
		return;
	}
	if (offset == 0x20128 / 4) { // voice number to deactivate ?
		v = apust.voice_number;
		apust.voices_active[v >> 6] &= ~(1 << (v & 63));
		return;
	}
	if (offset == 0x20140 / 4) // voice number to ?
		return;
	if ((offset >= 0x20200 / 4) && (offset < 0x20280 / 4)) // headroom for each of the 32 mixbins
		return;
	if (offset == 0x20280 / 4) // hrtf headroom ?
		return;
}

READ32_MEMBER(xbox_base_state::audio_ac93_r)
{
	UINT32 ret = 0;

#ifdef LOG_AUDIO
	logerror("Audio_AC3: read from %08X mask %08X\n", 0xfec00000 + offset * 4, mem_mask);
#endif
	if (offset < 0x80 / 4)
	{
		ret = ac97st.mixer_regs[offset];
	}
	if ((offset >= 0x100 / 4) && (offset <= 0x138 / 4))
	{
		offset = offset - 0x100 / 4;
		if (offset == 0x18 / 4)
		{
			ac97st.controller_regs[offset] &= ~0x02000000; // REGRST: register reset
		}
		if (offset == 0x30 / 4)
		{
			ac97st.controller_regs[offset] |= 0x100; // PCRDY: primary codec ready
		}
		if (offset == 0x34 / 4)
		{
			ac97st.controller_regs[offset] &= ~1; // CAS: codec access semaphore
		}
		ret = ac97st.controller_regs[offset];
	}
	return ret;
}

WRITE32_MEMBER(xbox_base_state::audio_ac93_w)
{
#ifdef LOG_AUDIO
	logerror("Audio_AC3: write at %08X mask %08X value %08X\n", 0xfec00000 + offset * 4, mem_mask, data);
#endif
	if (offset < 0x80 / 4)
	{
		COMBINE_DATA(ac97st.mixer_regs + offset);
	}
	if ((offset >= 0x100 / 4) && (offset <= 0x138 / 4))
	{
		offset = offset - 0x100 / 4;
		COMBINE_DATA(ac97st.controller_regs + offset);
	}
}

TIMER_CALLBACK_MEMBER(xbox_base_state::audio_apu_timer)
{
	int cmd;
	int bb, b, v;
	UINT64 bv;
	UINT32 phys;

	cmd = apust.space->read_dword(apust.gpdsp_address + 0x800 + 0x10);
	if (cmd == 3)
		apust.space->write_dword(apust.gpdsp_address + 0x800 + 0x10, 0);
	/*else
	logerror("Audio_APU: unexpected value at address %d\n",apust.gpdsp_address+0x800+0x10);*/
	for (b = 0; b < 4; b++) {
		bv = 1;
		for (bb = 0; bb < 64; bb++) {
			if (apust.voices_active[b] & bv) {
				v = bb + (b << 6);
				apust.voices_position[v] += apust.voices_position_increment[v];
				while (apust.voices_position[v] >= apust.voices_position_end[v])
					apust.voices_position[v] = apust.voices_position_start[v] + apust.voices_position[v] - apust.voices_position_end[v] - 1000;
				phys = apust.voicedata_address + 0x80 * v;
				apust.space->write_dword(phys + 0x58, apust.voices_position[v] / 1000);
			}
			bv = bv << 1;
		}
	}
}

static UINT32 pcibridghostbridg_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
#ifdef LOG_PCI
	busdevice->logerror("  bus:0 function:%d register:%d mask:%08X\n",function,reg,mem_mask);
#endif
	if ((function == 3) && (reg == 0x6c))
		return 0x08800044;
	return 0;
}

static void pcibridghostbridg_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
#ifdef LOG_PCI
	busdevice->logerror("  bus:0 function:%d register:%d data:%08X mask:%08X\n", function, reg, data, mem_mask);
#endif
}

static UINT32 hubintisabridg_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
#ifdef LOG_PCI
	busdevice->logerror("  bus:0 function:%d register:%d mask:%08X\n",function,reg,mem_mask);
#endif
	if ((function == 0) && (reg == 8))
		return 0xb4; // 0:1:0 revision id must be at least 0xb4, otherwise usb will require a hub
	return 0;
}

static void hubintisabridg_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
#ifdef LOG_PCI
	busdevice->logerror("  bus:0 function:%d register:%d data:%08X mask:%08X\n", function, reg, data, mem_mask);
#endif
}

/*
 * dummy for non connected devices
 */

static UINT32 dummy_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
#ifdef LOG_PCI
	busdevice->logerror("  bus:0 function:%d register:%d mask:%08X\n",function,reg,mem_mask);
#endif
	return 0;
}

static void dummy_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
#ifdef LOG_PCI
	busdevice->logerror("  bus:0 function:%d register:%d data:%08X mask:%08X\n", function, reg, data, mem_mask);
#endif
}

READ32_MEMBER(xbox_base_state::dummy_r)
{
	return 0;
}

WRITE32_MEMBER(xbox_base_state::dummy_w)
{
}

/*
 * PIC & PIT
 */

WRITE_LINE_MEMBER(xbox_base_state::xbox_pic8259_1_set_int_line)
{
	m_maincpu->set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER(xbox_base_state::get_slave_ack)
{
	if (offset == 2) { // IRQ = 2
		return xbox_base_devs.pic8259_2->acknowledge();
	}
	return 0x00;
}

IRQ_CALLBACK_MEMBER(xbox_base_state::irq_callback)
{
	int r = 0;
	r = xbox_base_devs.pic8259_1->acknowledge();
	if (debug_irq_active)
		debug_generate_irq(debug_irq_number, false);
	return r;
}

WRITE_LINE_MEMBER(xbox_base_state::xbox_pit8254_out0_changed)
{
	if (xbox_base_devs.pic8259_1)
	{
		xbox_base_devs.pic8259_1->ir0_w(state);
	}
}

WRITE_LINE_MEMBER(xbox_base_state::xbox_pit8254_out2_changed)
{
	//xbox_speaker_set_input( state ? 1 : 0 );
}

/*
 * SMbus devices
 */

int smbus_callback_pic16lc(xbox_base_state &chs, int command, int rw, int data)
{
	return chs.smbus_pic16lc(command, rw, data);
}

int xbox_base_state::smbus_pic16lc(int command, int rw, int data)
{
	if (rw == 1) { // read
		if (command == 0) {
			if (pic16lc_buffer[0] == 'D')
				pic16lc_buffer[0] = 'X';
			else if (pic16lc_buffer[0] == 'X')
				pic16lc_buffer[0] = 'B';
			else if (pic16lc_buffer[0] == 'B')
				pic16lc_buffer[0] = 'D';
		}
		logerror("pic16lc: %d %d %d\n", command, rw, pic16lc_buffer[command]);
		return pic16lc_buffer[command];
	}
	else
		if (command == 0)
			pic16lc_buffer[0] = 'B';
		else
			pic16lc_buffer[command] = (UINT8)data;
	logerror("pic16lc: %d %d %d\n", command, rw, data);
	return 0;
}

int smbus_callback_cx25871(xbox_base_state &chs, int command, int rw, int data)
{
	return chs.smbus_cx25871(command, rw, data);
}

int xbox_base_state::smbus_cx25871(int command, int rw, int data)
{
	logerror("cx25871: %d %d %d\n", command, rw, data);
	return 0;
}

// let's try to fake the missing eeprom, make sure its ntsc
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

int smbus_callback_eeprom(xbox_base_state &chs, int command, int rw, int data)
{
	return chs.smbus_eeprom(command, rw, data);
}

int xbox_base_state::smbus_eeprom(int command, int rw, int data)
{
	if (command >= 112)
		return 0;
	if (rw == 1) { // if reading
		// hack to avoid hanging if eeprom contents are not correct
		// this would need dumping the serial eeprom on the xbox board
		if (command == 0) {
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

/*
 * SMbus controller
 */

void xbox_base_state::smbus_register_device(int address, int(*handler)(xbox_base_state &chs, int command, int rw, int data))
{
	if (address < 128)
		smbusst.devices[address] = handler;
}

READ32_MEMBER(xbox_base_state::smbus_r)
{
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.status << 0) & mem_mask);
	if ((offset == 1) && ((mem_mask == 0x00ff0000) || (mem_mask == 0xffff0000))) // 6 smbus data
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.data << 16) & mem_mask);
	return smbusst.words[offset];
}

WRITE32_MEMBER(xbox_base_state::smbus_w)
{
	COMBINE_DATA(smbusst.words);
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
	{
		if (!((smbusst.status ^ data) & 0x10)) // clearing interrupt
			xbox_base_devs.pic8259_2->ir3_w(0); // IRQ 11
		smbusst.status &= ~data;
	}
	if ((offset == 0) && (mem_mask == 0xff0000)) // 2 smbus control
	{
		data = data >> 16;
		smbusst.control = data;
		int cycletype = smbusst.control & 7;
		if (smbusst.control & 8) { // start
			if ((cycletype & 6) == 2)
			{
				if (smbusst.devices[smbusst.address])
					if (smbusst.rw == 0)
						smbusst.devices[smbusst.address](*this, smbusst.command, smbusst.rw, smbusst.data);
					else
						smbusst.data = smbusst.devices[smbusst.address](*this, smbusst.command, smbusst.rw, smbusst.data);
				else
					logerror("SMBUS: access to missing device at address %d\n", smbusst.address);
				smbusst.status |= 0x10;
				if (smbusst.control & 0x10)
				{
					xbox_base_devs.pic8259_2->ir3_w(1); // IRQ 11
				}
			}
		}
	}
	if ((offset == 1) && (mem_mask == 0xff)) // 4 smbus address
	{
		smbusst.address = data >> 1;
		smbusst.rw = data & 1;
	}
	if ((offset == 1) && ((mem_mask == 0x00ff0000) || (mem_mask == 0xffff0000))) // 6 smbus data
	{
		data = data >> 16;
		smbusst.data = data;
	}
	if ((offset == 2) && (mem_mask == 0xff)) // 8 smbus command
		smbusst.command = data;
	//if ((offset == 2) && (mem_mask == 0x00ff0000)) ;
}

READ32_MEMBER(xbox_base_state::smbus2_r)
{
	return 0;
}

WRITE32_MEMBER(xbox_base_state::smbus2_w)
{
}

/*
* Ethernet controller
*/

READ32_MEMBER(xbox_base_state::network_r)
{
	return 0;
}

WRITE32_MEMBER(xbox_base_state::network_w)
{
}

READ32_MEMBER(xbox_base_state::networkio_r)
{
	return 0;
}

WRITE32_MEMBER(xbox_base_state::networkio_w)
{
}

/*
 * SuperIO
 */

READ8_MEMBER(xbox_base_state::superio_read)
{
	if (superiost.configuration_mode == false)
		return 0;
	if (offset == 0) // index port 0x2e
		return superiost.index;
	if (offset == 1)
	{
		// data port 0x2f
		if (superiost.index < 0x30)
			return superiost.registers[0][superiost.index];
		return superiost.registers[superiost.selected][superiost.index];
	}
	return 0;
}

WRITE8_MEMBER(xbox_base_state::superio_write)
{
	if (superiost.configuration_mode == false)
	{
		// config port 0x2e
		if ((offset == 0) && (data == 0x55))
			superiost.configuration_mode = true;
		return;
	}
	if ((offset == 0) && (data == 0xaa))
	{
		// config port 0x2e
		superiost.configuration_mode = false;
		return;
	}
	if (offset == 0)
	{
		// index port 0x2e
		superiost.index = data;
	}
	if (offset == 1)
	{
		// data port 0x2f
		if (superiost.index < 0x30)
		{
			superiost.registers[0][superiost.index] = data;
			superiost.selected = superiost.registers[0][7];
		} else
		{
			superiost.registers[superiost.selected][superiost.index] = data;
			//if ((superiost.selected == 4) && (superiost.index == 0x30) && (data != 0))
			//  ; // add handlers 0x3f8- +7
		}
	}
}

READ8_MEMBER(xbox_base_state::superiors232_read)
{
	if (offset == 5)
		return 0x20;
	return 0;
}

WRITE8_MEMBER(xbox_base_state::superiors232_write)
{
	if (offset == 0)
	{
		printf("%c", data);
	}
}

READ32_MEMBER(xbox_base_state::ohci_usb_r)
{
#ifdef USB_HACK_ENABLED
	if (offset == 0) { /* hacks needed until usb (and jvs) is implemented */
		hack_usb();
	}
#endif
	return ohci_usb->read(space, offset, mem_mask);
}

WRITE32_MEMBER(xbox_base_state::ohci_usb_w)
{
#ifndef USB_HACK_ENABLED
	ohci_usb->write(space, offset, mem_mask);
#endif
}

READ32_MEMBER(xbox_base_state::ohci_usb2_r)
{
	return 0;
}

WRITE32_MEMBER(xbox_base_state::ohci_usb2_w)
{
}

ADDRESS_MAP_START(xbox_base_map, AS_PROGRAM, 32, xbox_base_state)
	AM_RANGE(0x00000000, 0x07ffffff) AM_RAM AM_SHARE("nv2a_share") // 128 megabytes
	AM_RANGE(0xf0000000, 0xf7ffffff) AM_RAM AM_SHARE("nv2a_share") // 3d accelerator wants this
	AM_RANGE(0xfd000000, 0xfdffffff) AM_RAM AM_READWRITE(geforce_r, geforce_w)
#ifdef USB_HACK_ENABLED
	AM_RANGE(0xfed00000, 0xfed003ff) AM_READWRITE(ohci_usb_r, ohci_usb_w)
#else
	AM_RANGE(0xfed00000, 0xfed00fff) AM_DEVREADWRITE("ohci_usb", ohci_usb_controller, read, write)
#endif
	AM_RANGE(0xfed08000, 0xfed08fff) AM_READWRITE(ohci_usb2_r, ohci_usb2_w)
	AM_RANGE(0xfe800000, 0xfe87ffff) AM_READWRITE(audio_apu_r, audio_apu_w)
	AM_RANGE(0xfec00000, 0xfec00fff) AM_READWRITE(audio_ac93_r, audio_ac93_w)
	AM_RANGE(0xfef00000, 0xfef003ff) AM_READWRITE(network_r, network_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(xbox_base_map_io, AS_IO, 32, xbox_base_state)
	AM_RANGE(0x0020, 0x0023) AM_DEVREADWRITE8("pic8259_1", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x002c, 0x002f) AM_READWRITE8(superio_read, superio_write, 0xffff0000)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8254", pit8254_device, read, write, 0xffffffff)
	AM_RANGE(0x00a0, 0x00a3) AM_DEVREADWRITE8("pic8259_2", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", bus_master_ide_controller_device, read_cs0, write_cs0)
	AM_RANGE(0x03f8, 0x03ff) AM_READWRITE8(superiors232_read, superiors232_write, 0xffffffff)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
	AM_RANGE(0x8000, 0x80ff) AM_READWRITE(dummy_r, dummy_w) // lpc bridge
	AM_RANGE(0xc000, 0xc00f) AM_READWRITE(smbus_r, smbus_w)
	AM_RANGE(0xc200, 0xc21f) AM_READWRITE(smbus2_r, smbus2_w)
	AM_RANGE(0xe000, 0xe007) AM_READWRITE(networkio_r, networkio_w)
	AM_RANGE(0xff60, 0xff6f) AM_DEVREADWRITE("ide", bus_master_ide_controller_device, bmdma_r, bmdma_w)
ADDRESS_MAP_END

void xbox_base_state::machine_start()
{
	ohci_game_controller_device *usb_device;

	nvidia_nv2a = std::make_unique<nv2a_renderer>(machine());
	memset(pic16lc_buffer, 0, sizeof(pic16lc_buffer));
	pic16lc_buffer[0] = 'B';
	pic16lc_buffer[4] = 0; // A/V connector, 0=scart 2=vga 4=svideo 7=none
	smbus_register_device(0x10, smbus_callback_pic16lc);
	smbus_register_device(0x45, smbus_callback_cx25871);
	smbus_register_device(0x54, smbus_callback_eeprom);
	xbox_base_devs.pic8259_1 = machine().device<pic8259_device>("pic8259_1");
	xbox_base_devs.pic8259_2 = machine().device<pic8259_device>("pic8259_2");
	xbox_base_devs.ide = machine().device<bus_master_ide_controller_device>("ide");
	memset(apust.memory, 0, sizeof(apust.memory));
	memset(apust.voices_heap_blockaddr, 0, sizeof(apust.voices_heap_blockaddr));
	memset(apust.voices_active, 0, sizeof(apust.voices_active));
	memset(apust.voices_position, 0, sizeof(apust.voices_position));
	memset(apust.voices_position_start, 0, sizeof(apust.voices_position_start));
	memset(apust.voices_position_end, 0, sizeof(apust.voices_position_end));
	memset(apust.voices_position_increment, 0, sizeof(apust.voices_position_increment));
	apust.space = &m_maincpu->space();
	apust.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(xbox_base_state::audio_apu_timer), this), (void *)"APU Timer");
	apust.timer->enable(false);
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("xbox", CMDFLAG_NONE, 0, 1, 4, std::bind(&xbox_base_state::xbox_debug_commands, this, _1, _2, _3));
	}
	// PIC challenge handshake data
	pic16lc_buffer[0x1c] = 0x0c;
	pic16lc_buffer[0x1d] = 0x0d;
	pic16lc_buffer[0x1e] = 0x0e;
	pic16lc_buffer[0x1f] = 0x0f;
	// usb
	ohci_usb = machine().device<ohci_usb_controller>("ohci_usb");
	usb_device = machine().device<ohci_game_controller_device>("ohci_gamepad");
	usb_device->initialize(machine(), ohci_usb);
	ohci_usb->usb_ohci_plug(3, usb_device); // connect to root hub port 3, chihiro needs to use 1 and 2
	// super-io
	memset(&superiost, 0, sizeof(superiost));
	superiost.configuration_mode = false;
	superiost.registers[0][0x26] = 0x2e; // Configuration port address byte 0
	// savestates
	save_item(NAME(debug_irq_active));
	save_item(NAME(debug_irq_number));
	save_item(NAME(smbusst.status));
	save_item(NAME(smbusst.control));
	save_item(NAME(smbusst.address));
	save_item(NAME(smbusst.data));
	save_item(NAME(smbusst.command));
	save_item(NAME(smbusst.rw));
	save_item(NAME(smbusst.words));
	save_item(NAME(pic16lc_buffer));
	nvidia_nv2a->set_interrupt_device(xbox_base_devs.pic8259_1);
	nvidia_nv2a->start(&m_maincpu->space());
	nvidia_nv2a->savestate_items();
}

MACHINE_CONFIG_START(xbox_base, xbox_base_state)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM3, 733333333) /* Wrong! family 6 model 8 stepping 10 */
	MCFG_CPU_PROGRAM_MAP(xbox_base_map)
	MCFG_CPU_IO_MAP(xbox_base_map_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(xbox_base_state, irq_callback)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, "PCI Bridge Device - Host Bridge", pcibridghostbridg_pci_r, pcibridghostbridg_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(1, "HUB Interface - ISA Bridge", hubintisabridg_pci_r, hubintisabridg_pci_w) // function 0 lpc function 1 smbus
	MCFG_PCI_BUS_LEGACY_DEVICE(2, "OHCI USB Controller 1", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(3, "OHCI USB Controller 2", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(4, "MCP Networking Adapter", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(5, "MCP APU", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(6, "AC`97 Audio Codec Interface", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(9, "IDE Controller", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(30, "AGP Host to PCI Bridge", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_ADD("agpbus", 1)
	MCFG_PCI_BUS_LEGACY_SIBLING("pcibus")
	MCFG_PCI_BUS_LEGACY_DEVICE(0, "NV2A GeForce 3MX Integrated GPU/Northbridge", geforce_pci_r, geforce_pci_w)
	MCFG_PIC8259_ADD("pic8259_1", WRITELINE(xbox_base_state, xbox_pic8259_1_set_int_line), VCC, READ8(xbox_base_state, get_slave_ack))
	MCFG_PIC8259_ADD("pic8259_2", DEVWRITELINE("pic8259_1", pic8259_device, ir2_w), GND, NOOP)

	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(1125000) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(xbox_base_state, xbox_pit8254_out0_changed))
	MCFG_PIT8253_CLK1(1125000) /* (unused) dram refresh */
	MCFG_PIT8253_CLK2(1125000) /* (unused) pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(xbox_base_state, xbox_pit8254_out2_changed))

	MCFG_DEVICE_ADD("ide", BUS_MASTER_IDE_CONTROLLER, 0)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir6_w))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE("maincpu", AS_PROGRAM)

	// next line is temporary
	MCFG_OHCI_USB_CONTROLLER_ADD("ohci_usb")
	MCFG_OHCI_USB_CONTROLLER_INTERRUPT_HANDLER(WRITELINE(xbox_base_state, xbox_ohci_usb_interrupt_changed))
	MCFG_DEVICE_ADD("ohci_gamepad", OHCI_GAME_CONTROLLER, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(xbox_base_state, screen_update_callback)
	MCFG_SCREEN_VBLANK_DRIVER(xbox_base_state, vblank_callback)
MACHINE_CONFIG_END
