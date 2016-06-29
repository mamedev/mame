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
#include "debug/debugcpu.h"
#include "includes/chihiro.h"
#include "includes/xbox.h"

#define LOG_PCI
//#define LOG_AUDIO
//#define LOG_OHCI
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
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
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
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &buffer))
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
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
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
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
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
		if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
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
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
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
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
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
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
	{
		machine().debugger().console().printf("Address is unmapped.\n");
		return;
	}

	UINT32 kthrd = space.read_dword_unaligned(address);
	machine().debugger().console().printf("Current thread is %08X\n", kthrd);

	address = (offs_t)kthrd + 0x1c;
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
		return;

	UINT32 topstack = space.read_dword_unaligned(address);
	machine().debugger().console().printf("Current thread stack top is %08X\n", topstack);

	address = (offs_t)kthrd + 0x28;
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
		return;

	UINT32 tlsdata = space.read_dword_unaligned(address);
	if (tlsdata == 0)
		address = (offs_t)topstack - 0x210 - 8;
	else
		address = (offs_t)tlsdata - 8;
	if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &address))
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
			if (!machine().debugger().cpu().translate(space, TRANSLATE_READ_DEBUG, &addr))
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
 * ohci usb controller
 */

#ifdef LOG_OHCI
static const char *const usbregnames[] = {
	"HcRevision",
	"HcControl",
	"HcCommandStatus",
	"HcInterruptStatus",
	"HcInterruptEnable",
	"HcInterruptDisable",
	"HcHCCA",
	"HcPeriodCurrentED",
	"HcControlHeadED",
	"HcControlCurrentED",
	"HcBulkHeadED",
	"HcBulkCurrentED",
	"HcDoneHead",
	"HcFmInterval",
	"HcFmRemaining",
	"HcFmNumber",
	"HcPeriodicStart",
	"HcLSThreshold",
	"HcRhDescriptorA",
	"HcRhDescriptorB",
	"HcRhStatus",
	"HcRhPortStatus[1]"
};
#endif

const device_type OHCI_USB_CONTROLLER = &device_creator<ohci_usb_controller>;

ohci_usb_controller::ohci_usb_controller(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, OHCI_USB_CONTROLLER, "OHCI USB CONTROLLER", tag, owner, clock, "ohciusb", __FILE__),
	m_interrupt_handler(*this)
{
	memset(&ohcist, 0, sizeof(ohcist));
}

void ohci_usb_controller::device_start()
{
	m_maincpu = machine().device<cpu_device>("maincpu");
	m_interrupt_handler.resolve_safe();
	ohcist.hc_regs[HcRevision] = 0x10;
	ohcist.hc_regs[HcFmInterval] = 0x2edf;
	ohcist.hc_regs[HcLSThreshold] = 0x628;
	ohcist.hc_regs[HcRhDescriptorA] = 4;
	ohcist.hc_regs[HcControl] = UsbReset << 6;
	ohcist.state = UsbReset;
	ohcist.interruptbulkratio = 1;
	ohcist.writebackdonehadcounter = 7;
	for (int n = 0; n <= 4; n++)
		ohcist.ports[n].address = -1;
	for (int n = 0; n < 256; n++)
		ohcist.address[n].port = -1;
	ohcist.space = &(m_maincpu->space());
	ohcist.timer = timer_alloc(0);
	ohcist.timer->enable(false);
}

void ohci_usb_controller::device_reset()
{
}

READ32_MEMBER(ohci_usb_controller::read)
{
	UINT32 ret;

#ifdef LOG_OHCI
	if (offset >= 0x54 / 4)
		logerror("usb controller 0 register HcRhPortStatus[%d] read\n", (offset - 0x54 / 4) + 1);
	else
		logerror("usb controller 0 register %s read\n", usbregnames[offset]);
#endif
	ret = ohcist.hc_regs[offset];
	return ret;
}

WRITE32_MEMBER(ohci_usb_controller::write)
{
	UINT32 old = ohcist.hc_regs[offset];

#ifdef LOG_OHCI
	if (offset >= 0x54 / 4)
		logerror("usb controller 0 register HcRhPortStatus[%d] write %08X\n", (offset - 0x54 / 4) + 1, data);
	else
		logerror("usb controller 0 register %s write %08X\n", usbregnames[offset], data);
#endif
	if (offset == HcRhStatus) {
		if (data & CRWE)
			ohcist.hc_regs[HcRhStatus] &= ~DRWE;
		if (data & OCIC)
			ohcist.hc_regs[HcRhStatus] &= ~OCI;
		if (data & LPSC)
			ohcist.hc_regs[HcRhStatus] &= ~LPS;
		return;
	}
	if (offset == HcControl) {
		int hcfs;

		hcfs = (data >> 6) & 3; // HostControllerFunctionalState
		if (hcfs == UsbOperational) {
			ohcist.timer->enable();
			ohcist.timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
			ohcist.writebackdonehadcounter = 7;
			// need to load the FrameRemaining field of HcFmRemaining with the value of the FrameInterval field in HcFmInterval
		}
		else
			ohcist.timer->enable(false);
		ohcist.interruptbulkratio = (data & 3) + 1;
		if ((hcfs != UsbReset) && (ohcist.state == UsbReset))
		{
			ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
			usb_ohci_interrupts();
		}
		ohcist.state = hcfs;
	}
	if (offset == HcCommandStatus) {
		if (data & 1) // HostControllerReset
			ohcist.hc_regs[HcControl] |= 3 << 6;
		ohcist.hc_regs[HcCommandStatus] |= data;
		return;
	}
	if (offset == HcInterruptStatus) {
		ohcist.hc_regs[HcInterruptStatus] &= ~data;
		usb_ohci_interrupts();
		return;
	}
	if (offset == HcInterruptEnable) {
		ohcist.hc_regs[HcInterruptEnable] |= data;
		usb_ohci_interrupts();
		return;
	}
	if (offset == HcInterruptDisable) {
		ohcist.hc_regs[HcInterruptEnable] &= ~data;
		usb_ohci_interrupts();
		return;
	}
	if (offset >= HcRhPortStatus1) {
		int port = offset - HcRhPortStatus1 + 1; // port 0 not used
													// bit 0  R:CurrentConnectStatus           W:ClearPortEnable: 1 clears PortEnableStatus
		if (data & CCS) {
			ohcist.hc_regs[offset] &= ~PES;
			ohcist.address[ohcist.ports[port].address].port = -1;
		}
		// bit 1  R:PortEnableStatus               W:SetPortEnable: 1 sets PortEnableStatus
		if (data & PES) {
			ohcist.hc_regs[offset] |= PES;
			// the port is enabled, so the device connected to it can communicate on the bus
			ohcist.address[ohcist.ports[port].address].function = ohcist.ports[port].function;
			ohcist.address[ohcist.ports[port].address].port = port;
		}
		// bit 2  R:PortSuspendStatus              W:SetPortSuspend: 1 sets PortSuspendStatus
		if (data & PSS) {
			ohcist.hc_regs[offset] |= PSS;
		}
		// bit 3  R:PortOverCurrentIndicator       W:ClearSuspendStatus: 1 clears PortSuspendStatus
		if (data & POCI) {
			ohcist.hc_regs[offset] &= ~PSS;
		}
		// bit 4  R: PortResetStatus               W:SetPortReset: 1 sets PortResetStatus
		if (data & PRS) {
			ohcist.hc_regs[offset] |= PRS;
			if (ohcist.ports[port].address >= 0)
				ohcist.address[ohcist.ports[port].address].port = -1;
			ohcist.ports[port].address = 0;
			if (ohcist.hc_regs[offset] & PES)
			{
				ohcist.address[0].function = ohcist.ports[port].function;
				ohcist.address[0].port = port;
			}
			ohcist.ports[port].function->execute_reset();
			// after 10ms set PortResetStatusChange and clear PortResetStatus and set PortEnableStatus
			ohcist.ports[port].delay = 10;
		}
		// bit 8  R:PortPowerStatus                W:SetPortPower: 1 sets PortPowerStatus
		if (data & PPS) {
			ohcist.hc_regs[offset] |= PPS;
		}
		// bit 9  R:LowSpeedDeviceAttached         W:ClearPortPower: 1 clears PortPowerStatus
		if (data & LSDA) {
			ohcist.hc_regs[offset] &= ~PPS;
		}
		// bit 16 R:ConnectStatusChange            W: 1 clears ConnectStatusChange
		if (data & CSC) {
			ohcist.hc_regs[offset] &= ~CSC;
		}
		// bit 17 R:PortEnableStatusChange         W: 1 clears PortEnableStatusChange
		if (data & PESC) {
			ohcist.hc_regs[offset] &= ~PESC;
		}
		// bit 18 R:PortSuspendStatusChange        W: 1 clears PortSuspendStatusChange
		if (data & PSSC) {
			ohcist.hc_regs[offset] &= ~PSSC;
		}
		// bit 19 R:PortOverCurrentIndicatorChange W: 1 clears PortOverCurrentIndicatorChange
		if (data & POCIC) {
			ohcist.hc_regs[offset] &= ~POCIC;
		}
		// bit 20 R:PortResetStatusChange          W: 1 clears PortResetStatusChange
		if (data & PRSC) {
			ohcist.hc_regs[offset] &= ~PRSC;
		}
		if (ohcist.hc_regs[offset] != old)
			ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
		usb_ohci_interrupts();
		return;
	}
	ohcist.hc_regs[offset] = data;
}

void ohci_usb_controller::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	UINT32 hcca;
	UINT32 plh;
	int changed = 0;
	int list = 1;
	bool cont = false;
	bool retire = false;
	int pid, remain, mps, done;

	hcca = ohcist.hc_regs[HcHCCA];
	if (ohcist.state == UsbOperational) {
		// increment frame number
		ohcist.framenumber = (ohcist.framenumber + 1) & 0xffff;
		ohcist.space->write_dword(hcca + 0x80, ohcist.framenumber);
		ohcist.hc_regs[HcFmNumber] = ohcist.framenumber;
	}
	// port reset delay
	for (int p = 1; p <= 4; p++) {
		if (ohcist.ports[p].delay > 0) {
			ohcist.ports[p].delay--;
			if (ohcist.ports[p].delay == 0) {
				ohcist.hc_regs[HcRhPortStatus1 + p - 1] = (ohcist.hc_regs[HcRhPortStatus1 + p - 1] & ~PRS) | PRSC | PES;
				ohcist.address[ohcist.ports[p].address].function = ohcist.ports[p].function;
				ohcist.address[ohcist.ports[p].address].port = p;
				changed = 1;
			}
		}
	}
	if (ohcist.state == UsbOperational) {
		while (list >= 0)
		{
			// select list, do transfer
			if (list == 0) {
				if (ohcist.hc_regs[HcControl] & PLE) {
					// periodic list
					plh = ohcist.space->read_dword(hcca + (ohcist.framenumber & 0x1f) * 4);
					cont = true;
					while (cont == true) {
						if (plh != 0) {
							usb_ohci_read_endpoint_descriptor(plh);
							// if this an isochronous endpoint and isochronous list not enabled, stop list processing
							if (((ohcist.hc_regs[HcControl] & IE) == 0) && (ohcist.endpoint_descriptor.f == 1))
								cont = false;
						}
						else
							cont = false;
						if (cont == false)
							break;
						// service endpoint descriptor
						// only if it is not halted and not to be skipped
						if (!(ohcist.endpoint_descriptor.h | ohcist.endpoint_descriptor.k)) {
							// compare the Endpoint Descriptor TailPointer and NextTransferDescriptor fields.
							if (ohcist.endpoint_descriptor.headp != ohcist.endpoint_descriptor.tailp) {
								UINT32 a, b;
								int R = 0;

								// service transfer descriptor
								if (ohcist.endpoint_descriptor.f != 1) {
									usb_ohci_read_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									// get pid
									if (ohcist.endpoint_descriptor.d == 1)
										pid = OutPid; // out
									else if (ohcist.endpoint_descriptor.d == 2)
										pid = InPid; // in
									else {
										pid = ohcist.transfer_descriptor.dp; // 0 setup 1 out 2 in
									}
									a = ohcist.transfer_descriptor.be;
									b = ohcist.transfer_descriptor.cbp;
								}
								else {
									usb_ohci_read_isochronous_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									// get pid
									if (ohcist.endpoint_descriptor.d == 1)
										pid = OutPid; // out
									else if (ohcist.endpoint_descriptor.d == 2)
										pid = InPid; // in
									else
										pid = InPid; // in
									R = (int)ohcist.framenumber - (int)ohcist.isochronous_transfer_descriptor.sf;
									//if ((R < 0) || (R > (int)ohcist.isochronous_transfer_descriptor.fc))
									//  ; // greater than fc should be an error
									if (R == (int)ohcist.isochronous_transfer_descriptor.fc)
										a = ohcist.isochronous_transfer_descriptor.be;
									else {
										a = ohcist.isochronous_transfer_descriptor.offset[R + 1] - 1;
										if (a & (1 << 12))
											a = (ohcist.isochronous_transfer_descriptor.be & 0xfffff000) | (a & 0xfff);
										else
											a = ohcist.isochronous_transfer_descriptor.bp0 | (a & 0xfff);
									}
									b = ohcist.isochronous_transfer_descriptor.offset[R];
									if (b & (1 << 12))
										b = (ohcist.isochronous_transfer_descriptor.be & 0xfffff000) | (b & 0xfff);
									else
										b = ohcist.isochronous_transfer_descriptor.bp0 | (b & 0xfff);
								}
								if ((a ^ b) & 0xfffff000)
									remain = ((a | 0x1000) & 0x1fff) - (b & 0xfff) + 1;
								else
									remain = a - b + 1;
								mps = ohcist.endpoint_descriptor.mps;
								if (remain < mps)
									mps = remain;
								// if sending ...
								if (pid != InPid) {
									// ... get mps bytes
									for (int c = 0; c < remain; c++) {
										ohcist.buffer[c] = ohcist.space->read_byte(b);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// should check for time available
								// execute transaction
								done = ohcist.address[ohcist.endpoint_descriptor.fa].function->execute_transfer(ohcist.endpoint_descriptor.en, pid, ohcist.buffer, mps);
								// if receiving ...
								if (pid == InPid) {
									// ... store done bytes
									for (int c = 0; c < done; c++) {
										ohcist.space->write_byte(b, ohcist.buffer[c]);
										b++;
										if ((b & 0xfff) == 0)
											b = a & 0xfffff000;
									}
								}
								if (ohcist.endpoint_descriptor.f != 1) {
									// status writeback (CompletionCode field, DataToggleControl field, CurrentBufferPointer field, ErrorCount field)
									ohcist.transfer_descriptor.cc = NoError;
									ohcist.transfer_descriptor.t = (ohcist.transfer_descriptor.t ^ 1) | 2;
									// if all data is transferred (or there was no data to transfer) cbp must be 0, otherwise it must be updated
									if (done == remain)
										b = 0;
									ohcist.transfer_descriptor.cbp = b;
									ohcist.transfer_descriptor.ec = 0;
									retire = false;
									if ((done == mps) && (done == remain)) {
										retire = true;
									}
									if ((done != mps) && (done <= remain))
										retire = true;
									if (done == 0)
										retire = true;
									if (retire == true) {
										// retire transfer descriptor
										a = ohcist.endpoint_descriptor.headp;
										ohcist.endpoint_descriptor.headp = ohcist.transfer_descriptor.nexttd;
										ohcist.transfer_descriptor.nexttd = ohcist.hc_regs[HcDoneHead];
										ohcist.hc_regs[HcDoneHead] = a;
										ohcist.endpoint_descriptor.c = ohcist.transfer_descriptor.t & 1;
										if (ohcist.transfer_descriptor.di != 7) {
											if (ohcist.transfer_descriptor.di < ohcist.writebackdonehadcounter)
												ohcist.writebackdonehadcounter = ohcist.transfer_descriptor.di;
										}
										usb_ohci_writeback_transfer_descriptor(a);
										usb_ohci_writeback_endpoint_descriptor(plh);
									}
									else {
										usb_ohci_writeback_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									}
								}
								else
								{
									// status writeback
									ohcist.isochronous_transfer_descriptor.cc = NoError;
									if (done == remain)
										b = 0;
									ohcist.isochronous_transfer_descriptor.offset[R] = b;
									retire = false;
									if ((done == mps) && (done == remain)) {
										retire = true;
									}
									if ((done != mps) && (done <= remain))
										retire = true;
									if (done == 0)
										retire = true;
									if (retire == true) {
										// retire transfer descriptor
									}
									else {
										usb_ohci_writeback_isochronous_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									}
								}
							}
						}
						// go to next endpoint
						if (ohcist.endpoint_descriptor.nexted != 0)
						{
							plh = ohcist.endpoint_descriptor.nexted;
						}
						else
							cont = false;
					}
				}
				list = -1;
			}
			if (list == 1) {
				// control list
				// check if control list active
				if (ohcist.hc_regs[HcControl] & CLE) {
					cont = true;
					while (cont == true) {
						// if current endpoint descriptor is not 0 use it, otherwise ...
						if (ohcist.hc_regs[HcControlCurrentED] == 0) {
							// ... check the filled bit ...
							if (ohcist.hc_regs[HcCommandStatus] & (1 << 1)) {
								// ... if 1 start processing from the head of the list
								ohcist.hc_regs[HcControlCurrentED] = ohcist.hc_regs[HcControlHeadED];
								// clear CLF (ControlListFilled)
								ohcist.hc_regs[HcCommandStatus] &= ~(1 << 1);
								// but if the list is empty, go to the next list
								if (ohcist.hc_regs[HcControlCurrentED] == 0)
									cont = false;
								else
									cont = true;
							}
							else
								cont = false;
						}
						else
							cont = true;
						if (cont == false)
							break;
						// service endpoint descriptor
						usb_ohci_read_endpoint_descriptor(ohcist.hc_regs[HcControlCurrentED]);
						// only if it is not halted and not to be skipped
						if (!(ohcist.endpoint_descriptor.h | ohcist.endpoint_descriptor.k)) {
							// compare the Endpoint Descriptor TailPointer and NextTransferDescriptor fields.
							if (ohcist.endpoint_descriptor.headp != ohcist.endpoint_descriptor.tailp) {
								UINT32 a, b;
								// set CLF (ControlListFilled)
								ohcist.hc_regs[HcCommandStatus] |= (1 << 1);
								// service transfer descriptor
								usb_ohci_read_transfer_descriptor(ohcist.endpoint_descriptor.headp);
								// get pid
								if (ohcist.endpoint_descriptor.d == 1)
									pid = OutPid; // out
								else if (ohcist.endpoint_descriptor.d == 2)
									pid = InPid; // in
								else {
									pid = ohcist.transfer_descriptor.dp; // 0 setup 1 out 2 in
								}
								// determine how much data to transfer
								// setup pid must be 8 bytes
								a = ohcist.transfer_descriptor.be & 0xfff;
								b = ohcist.transfer_descriptor.cbp & 0xfff;
								if ((ohcist.transfer_descriptor.be ^ ohcist.transfer_descriptor.cbp) & 0xfffff000)
									a |= 0x1000;
								remain = a - b + 1;
								mps = ohcist.endpoint_descriptor.mps;
								if ((pid == InPid) || (pid == OutPid)) {
									if (remain < mps)
										mps = remain;
								}
								if (ohcist.transfer_descriptor.cbp == 0) {
									remain = 0;
									mps = 0;
								}
								b = ohcist.transfer_descriptor.cbp;
								// if sending ...
								if (pid != InPid) {
									// ... get mps bytes
									for (int c = 0; c < remain; c++) {
										ohcist.buffer[c] = ohcist.space->read_byte(b);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// should check for time available
								// execute transaction
								done = ohcist.address[ohcist.endpoint_descriptor.fa].function->execute_transfer(ohcist.endpoint_descriptor.en, pid, ohcist.buffer, mps);
								// if receiving ...
								if (pid == InPid) {
									// ... store done bytes
									for (int c = 0; c < done; c++) {
										ohcist.space->write_byte(b, ohcist.buffer[c]);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// status writeback (CompletionCode field, DataToggleControl field, CurrentBufferPointer field, ErrorCount field)
								ohcist.transfer_descriptor.cc = NoError;
								ohcist.transfer_descriptor.t = (ohcist.transfer_descriptor.t ^ 1) | 2;
								// if all data is transferred (or there was no data to transfer) cbp must be 0, otherwise it must be updated
								if ((done == remain) || (pid == SetupPid))
									b = 0;
								ohcist.transfer_descriptor.cbp = b;
								ohcist.transfer_descriptor.ec = 0;
								retire = false;
								if ((done == mps) && (done == remain)) {
									retire = true;
								}
								if ((done != mps) && (done <= remain))
									retire = true;
								if (done == 0)
									retire = true;
								if (retire == true) {
									// retire transfer descriptor
									a = ohcist.endpoint_descriptor.headp;
									ohcist.endpoint_descriptor.headp = ohcist.transfer_descriptor.nexttd;
									ohcist.transfer_descriptor.nexttd = ohcist.hc_regs[HcDoneHead];
									ohcist.hc_regs[HcDoneHead] = a;
									ohcist.endpoint_descriptor.c = ohcist.transfer_descriptor.t & 1;
									if (ohcist.transfer_descriptor.di != 7) {
										if (ohcist.transfer_descriptor.di < ohcist.writebackdonehadcounter)
											ohcist.writebackdonehadcounter = ohcist.transfer_descriptor.di;
									}
									usb_ohci_writeback_transfer_descriptor(a);
									usb_ohci_writeback_endpoint_descriptor(ohcist.hc_regs[HcControlCurrentED]);
								}
								else {
									usb_ohci_writeback_transfer_descriptor(ohcist.endpoint_descriptor.headp);
								}
							}
							else {
								// no transfer descriptors for this endpoint, so go to next endpoint
								ohcist.hc_regs[HcControlCurrentED] = ohcist.endpoint_descriptor.nexted;
							}
						}
						else {
							// not enabled, so go to next endpoint
							ohcist.hc_regs[HcControlCurrentED] = ohcist.endpoint_descriptor.nexted;
						}
						// one bulk every n control transfers
						ohcist.interruptbulkratio--;
						if (ohcist.interruptbulkratio <= 0) {
							ohcist.interruptbulkratio = (ohcist.hc_regs[HcControl] & 3) + 1; // ControlBulkServiceRatio
							cont = false;
						}
					}
				}
				list = 2;
			}
			if (list == 2) {
				// bulk list
				// check if bulk list active
				if (ohcist.hc_regs[HcControl] & BLE) {
					// if current endpoint descriptor is not 0 use it, otherwise ...
					if (ohcist.hc_regs[HcBulkCurrentED] == 0) {
						// ... check the filled bit ...
						if (ohcist.hc_regs[HcCommandStatus] & (1 << 2)) {
							// ... if 1 start processing from the head of the list
							ohcist.hc_regs[HcBulkCurrentED] = ohcist.hc_regs[HcBulkHeadED];
							// clear BLF (BulkListFilled)
							ohcist.hc_regs[HcCommandStatus] &= ~(1 << 2);
							// but if the list is empty, go to the next list
							if (ohcist.hc_regs[HcBulkCurrentED] == 0)
								cont = false;
							else
								cont = true;
						}
						else
							cont = false;
					}
					else
						cont = true;
					if (cont == true) {
						// service endpoint descriptor
						usb_ohci_read_endpoint_descriptor(ohcist.hc_regs[HcBulkCurrentED]);
						// only if it is not halted and not to be skipped
						if (!(ohcist.endpoint_descriptor.h | ohcist.endpoint_descriptor.k)) {
							// compare the Endpoint Descriptor TailPointer and NextTransferDescriptor fields.
							if (ohcist.endpoint_descriptor.headp != ohcist.endpoint_descriptor.tailp) {
								UINT32 a, b;
								// set BLF (BulkListFilled)
								ohcist.hc_regs[HcCommandStatus] |= (1 << 2);
								// service transfer descriptor
								usb_ohci_read_transfer_descriptor(ohcist.endpoint_descriptor.headp);
								// get pid
								if (ohcist.endpoint_descriptor.d == 1)
									pid = OutPid; // out
								else if (ohcist.endpoint_descriptor.d == 2)
									pid = InPid; // in
								else {
									pid = ohcist.transfer_descriptor.dp; // 0 setup 1 out 2 in
								}
								// determine how much data to transfer
								a = ohcist.transfer_descriptor.be & 0xfff;
								b = ohcist.transfer_descriptor.cbp & 0xfff;
								if ((ohcist.transfer_descriptor.be ^ ohcist.transfer_descriptor.cbp) & 0xfffff000)
									a |= 0x1000;
								remain = a - b + 1;
								mps = ohcist.endpoint_descriptor.mps;
								if (remain < mps)
									mps = remain;
								b = ohcist.transfer_descriptor.cbp;
								// if sending ...
								if (pid != InPid) {
									// ... get mps bytes
									for (int c = 0; c < remain; c++) {
										ohcist.buffer[c] = ohcist.space->read_byte(b);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// should check for time available
								// execute transaction
								done = ohcist.address[ohcist.endpoint_descriptor.fa].function->execute_transfer(ohcist.endpoint_descriptor.en, pid, ohcist.buffer, mps);
								// if receiving ...
								if (pid == InPid) {
									// ... store done bytes
									for (int c = 0; c < done; c++) {
										ohcist.space->write_byte(b, ohcist.buffer[c]);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// status writeback (CompletionCode field, DataToggleControl field, CurrentBufferPointer field, ErrorCount field)
								ohcist.transfer_descriptor.cc = NoError;
								ohcist.transfer_descriptor.t = (ohcist.transfer_descriptor.t ^ 1) | 2;
								// if all data is transferred (or there was no data to transfer) cbp must be 0, otherwise it must be updated
								if (done == remain)
									b = 0;
								ohcist.transfer_descriptor.cbp = b;
								ohcist.transfer_descriptor.ec = 0;
								retire = false;
								if ((done == mps) && (done == remain)) {
									retire = true;
								}
								if ((done != mps) && (done <= remain))
									retire = true;
								if (done == 0)
									retire = true;
								if (retire == true) {
									// retire transfer descriptor
									a = ohcist.endpoint_descriptor.headp;
									ohcist.endpoint_descriptor.headp = ohcist.transfer_descriptor.nexttd;
									ohcist.transfer_descriptor.nexttd = ohcist.hc_regs[HcDoneHead];
									ohcist.hc_regs[HcDoneHead] = a;
									ohcist.endpoint_descriptor.c = ohcist.transfer_descriptor.t & 1;
									if (ohcist.transfer_descriptor.di != 7) {
										if (ohcist.transfer_descriptor.di < ohcist.writebackdonehadcounter)
											ohcist.writebackdonehadcounter = ohcist.transfer_descriptor.di;
									}
									usb_ohci_writeback_transfer_descriptor(a);
									usb_ohci_writeback_endpoint_descriptor(ohcist.hc_regs[HcBulkCurrentED]);
								}
								else {
									usb_ohci_writeback_transfer_descriptor(ohcist.endpoint_descriptor.headp);
								}
							}
							else {
								// no transfer descriptors for this endpoint, so go to next endpoint
								ohcist.hc_regs[HcBulkCurrentED] = ohcist.endpoint_descriptor.nexted;
							}
						}
						else {
							// not enabled, so go to next endpoint
							ohcist.hc_regs[HcBulkCurrentED] = ohcist.endpoint_descriptor.nexted;
						}
					}
					// go to the next list
					if ((ohcist.hc_regs[HcCommandStatus] & (1 << 1)) && (ohcist.hc_regs[HcControl] & CLE))
						list = 1; // go to control list if enabled and filled
					else if ((ohcist.hc_regs[HcCommandStatus] & (1 << 2)) && (ohcist.hc_regs[HcControl] & BLE))
						list = 2; // otherwise stay in bulk list if enabled and filled
					else
						list = 0; // if no control or bulk lists, go to periodic list
				}
			}
		}
		if (ohcist.framenumber == 0)
			ohcist.hc_regs[HcInterruptStatus] |= FrameNumberOverflow;
		ohcist.hc_regs[HcInterruptStatus] |= StartofFrame;
		if ((ohcist.writebackdonehadcounter != 0) && (ohcist.writebackdonehadcounter != 7))
			ohcist.writebackdonehadcounter--;
		if ((ohcist.writebackdonehadcounter == 0) && ((ohcist.hc_regs[HcInterruptStatus] & WritebackDoneHead) == 0)) {
			UINT32 b = 0;

			if ((ohcist.hc_regs[HcInterruptStatus] & ohcist.hc_regs[HcInterruptEnable]) != WritebackDoneHead)
				b = 1;
			ohcist.hc_regs[HcInterruptStatus] |= WritebackDoneHead;
			ohcist.space->write_dword(hcca + 0x84, ohcist.hc_regs[HcDoneHead] | b);
			ohcist.hc_regs[HcDoneHead] = 0;
			ohcist.writebackdonehadcounter = 7;
		}
	}
	if (changed != 0) {
		ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
	}
	usb_ohci_interrupts();
}

void ohci_usb_controller::usb_ohci_plug(int port, ohci_function_device *function)
{
	if ((port > 0) && (port <= 4)) {
		ohcist.ports[port].function = function;
		ohcist.ports[port].address = -1;
		ohcist.hc_regs[HcRhPortStatus1 + port - 1] = CCS | CSC;
		if (ohcist.state != UsbReset)
		{
			ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
			usb_ohci_interrupts();
		}
	}
}

void ohci_usb_controller::usb_ohci_interrupts()
{
	if (((ohcist.hc_regs[HcInterruptStatus] & ohcist.hc_regs[HcInterruptEnable]) != 0) && ((ohcist.hc_regs[HcInterruptEnable] & MasterInterruptEnable) != 0))
	{
		//pic8259_1->ir1_w(1);
		m_interrupt_handler(1);
	} else
	{
		//pic8259_1->ir1_w(0);
		m_interrupt_handler(0);
	}
}

void ohci_usb_controller::usb_ohci_read_endpoint_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.space->read_dword(address);
	ohcist.endpoint_descriptor.word0 = w;
	ohcist.endpoint_descriptor.fa = w & 0x7f;
	ohcist.endpoint_descriptor.en = (w >> 7) & 15;
	ohcist.endpoint_descriptor.d = (w >> 11) & 3;
	ohcist.endpoint_descriptor.s = (w >> 13) & 1;
	ohcist.endpoint_descriptor.k = (w >> 14) & 1;
	ohcist.endpoint_descriptor.f = (w >> 15) & 1;
	ohcist.endpoint_descriptor.mps = (w >> 16) & 0x7ff;
	ohcist.endpoint_descriptor.tailp = ohcist.space->read_dword(address + 4);
	w = ohcist.space->read_dword(address + 8);
	ohcist.endpoint_descriptor.headp = w & 0xfffffffc;
	ohcist.endpoint_descriptor.h = w & 1;
	ohcist.endpoint_descriptor.c = (w >> 1) & 1;
	ohcist.endpoint_descriptor.nexted = ohcist.space->read_dword(address + 12);
}

void ohci_usb_controller::usb_ohci_writeback_endpoint_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.endpoint_descriptor.word0 & 0xf8000000;
	w = w | (ohcist.endpoint_descriptor.mps << 16) | (ohcist.endpoint_descriptor.f << 15) | (ohcist.endpoint_descriptor.k << 14) | (ohcist.endpoint_descriptor.s << 13) | (ohcist.endpoint_descriptor.d << 11) | (ohcist.endpoint_descriptor.en << 7) | ohcist.endpoint_descriptor.fa;
	ohcist.space->write_dword(address, w);
	w = ohcist.endpoint_descriptor.headp | (ohcist.endpoint_descriptor.c << 1) | ohcist.endpoint_descriptor.h;
	ohcist.space->write_dword(address + 8, w);
}

void ohci_usb_controller::usb_ohci_read_transfer_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.space->read_dword(address);
	ohcist.transfer_descriptor.word0 = w;
	ohcist.transfer_descriptor.cc = (w >> 28) & 15;
	ohcist.transfer_descriptor.ec = (w >> 26) & 3;
	ohcist.transfer_descriptor.t = (w >> 24) & 3;
	ohcist.transfer_descriptor.di = (w >> 21) & 7;
	ohcist.transfer_descriptor.dp = (w >> 19) & 3;
	ohcist.transfer_descriptor.r = (w >> 18) & 1;
	ohcist.transfer_descriptor.cbp = ohcist.space->read_dword(address + 4);
	ohcist.transfer_descriptor.nexttd = ohcist.space->read_dword(address + 8);
	ohcist.transfer_descriptor.be = ohcist.space->read_dword(address + 12);
}

void ohci_usb_controller::usb_ohci_writeback_transfer_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.transfer_descriptor.word0 & 0x0003ffff;
	w = w | (ohcist.transfer_descriptor.cc << 28) | (ohcist.transfer_descriptor.ec << 26) | (ohcist.transfer_descriptor.t << 24) | (ohcist.transfer_descriptor.di << 21) | (ohcist.transfer_descriptor.dp << 19) | (ohcist.transfer_descriptor.r << 18);
	ohcist.space->write_dword(address, w);
	ohcist.space->write_dword(address + 4, ohcist.transfer_descriptor.cbp);
	ohcist.space->write_dword(address + 8, ohcist.transfer_descriptor.nexttd);
}

void ohci_usb_controller::usb_ohci_read_isochronous_transfer_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.space->read_dword(address);
	ohcist.isochronous_transfer_descriptor.word0 = w;
	ohcist.isochronous_transfer_descriptor.cc = (w >> 28) & 15;
	ohcist.isochronous_transfer_descriptor.fc = (w >> 24) & 7;
	ohcist.isochronous_transfer_descriptor.di = (w >> 21) & 7;
	ohcist.isochronous_transfer_descriptor.sf = w & 0xffff;
	w = ohcist.space->read_dword(address + 4);
	ohcist.isochronous_transfer_descriptor.word1 = w;
	ohcist.isochronous_transfer_descriptor.bp0 = w & 0xfffff000;
	ohcist.isochronous_transfer_descriptor.nexttd = ohcist.space->read_dword(address + 8);
	ohcist.isochronous_transfer_descriptor.be = ohcist.space->read_dword(address + 12);
	w = ohcist.space->read_dword(address + 16);
	ohcist.isochronous_transfer_descriptor.offset[0] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[1] = (w >> 16) & 0xffff;
	w = ohcist.space->read_dword(address + 20);
	ohcist.isochronous_transfer_descriptor.offset[2] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[3] = (w >> 16) & 0xffff;
	w = ohcist.space->read_dword(address + 24);
	ohcist.isochronous_transfer_descriptor.offset[4] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[5] = (w >> 16) & 0xffff;
	w = ohcist.space->read_dword(address + 28);
	ohcist.isochronous_transfer_descriptor.offset[6] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[7] = (w >> 16) & 0xffff;
}

void ohci_usb_controller::usb_ohci_writeback_isochronous_transfer_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.isochronous_transfer_descriptor.word0 & 0x1f0000;
	w = w | (ohcist.isochronous_transfer_descriptor.cc << 28) | (ohcist.isochronous_transfer_descriptor.fc << 24) | (ohcist.isochronous_transfer_descriptor.di << 21) | ohcist.isochronous_transfer_descriptor.sf;
	ohcist.space->write_dword(address, w);
	w = ohcist.isochronous_transfer_descriptor.word1 & 0xfff;
	w = w | ohcist.isochronous_transfer_descriptor.bp0;
	ohcist.space->write_dword(address + 4, w);
	ohcist.space->write_dword(address + 8, ohcist.isochronous_transfer_descriptor.nexttd);
	ohcist.space->write_dword(address + 12, ohcist.isochronous_transfer_descriptor.be);
	w = (ohcist.isochronous_transfer_descriptor.offset[1] << 16) | ohcist.isochronous_transfer_descriptor.offset[0];
	ohcist.space->write_dword(address + 16, w);
	w = (ohcist.isochronous_transfer_descriptor.offset[3] << 16) | ohcist.isochronous_transfer_descriptor.offset[2];
	ohcist.space->write_dword(address + 20, w);
	w = (ohcist.isochronous_transfer_descriptor.offset[5] << 16) | ohcist.isochronous_transfer_descriptor.offset[4];
	ohcist.space->write_dword(address + 24, w);
	w = (ohcist.isochronous_transfer_descriptor.offset[7] << 16) | ohcist.isochronous_transfer_descriptor.offset[6];
	ohcist.space->write_dword(address + 28, w);
}

void ohci_usb_controller::usb_ohci_device_address_changed(int old_address, int new_address)
{
	ohcist.address[new_address].function = ohcist.address[old_address].function;
	ohcist.address[new_address].port = ohcist.address[old_address].port;
	ohcist.address[old_address].port = -1;
}

/*
* ohci device base class
*/

ohci_function_device::ohci_function_device()
{
}

void ohci_function_device::initialize(running_machine &machine, ohci_usb_controller *usb_bus_manager)
{
	busmanager = usb_bus_manager;
	state = DefaultState;
	descriptors = auto_alloc_array(machine, UINT8, 1024);
	descriptors_pos = 0;
	address = 0;
	newaddress = 0;
	for (int e = 0; e < 256;e++) {
		endpoints[e].type = -1;
		endpoints[e].controldirection = 0;
		endpoints[e].controltype = 0;
		endpoints[e].controlrecipient = 0;
		endpoints[e].remain = 0;
		endpoints[e].position = nullptr;
	}
	endpoints[0].type = ControlEndpoint;
	wantstatuscallback = false;
	settingaddress = false;
	configurationvalue = 0;
	selected_configuration = nullptr;
	latest_configuration = nullptr;
	latest_alternate = nullptr;
}

void ohci_function_device::add_device_descriptor(const USBStandardDeviceDescriptor &descriptor)
{
	UINT8 *p = descriptors + descriptors_pos;

	p[0] = descriptor.bLength;
	p[1] = descriptor.bDescriptorType;
	p[2] = descriptor.bcdUSB & 255;
	p[3] = descriptor.bcdUSB >> 8;
	p[4] = descriptor.bDeviceClass;
	p[5] = descriptor.bDeviceSubClass;
	p[6] = descriptor.bDeviceProtocol;
	p[7] = descriptor.bMaxPacketSize0;
	p[8] = descriptor.idVendor & 255;
	p[9] = descriptor.idVendor >> 8;
	p[10] = descriptor.idProduct & 255;
	p[11] = descriptor.idProduct >> 8;
	p[12] = descriptor.bcdDevice & 255;
	p[13] = descriptor.bcdDevice >> 8;
	p[14] = descriptor.iManufacturer;
	p[15] = descriptor.iProduct;
	p[16] = descriptor.iSerialNumber;
	p[17] = descriptor.bNumConfigurations;
	descriptors_pos += descriptor.bLength;
	memcpy(&device_descriptor, &descriptor, sizeof(USBStandardDeviceDescriptor));
}

void ohci_function_device::add_configuration_descriptor(const USBStandardConfigurationDescriptor &descriptor)
{
	usb_device_configuration *c = new usb_device_configuration;
	UINT8 *p = descriptors + descriptors_pos;

	p[0] = descriptor.bLength;
	p[1] = descriptor.bDescriptorType;
	p[2] = descriptor.wTotalLength & 255;
	p[3] = descriptor.wTotalLength >> 8;
	p[4] = descriptor.bNumInterfaces;
	p[5] = descriptor.bConfigurationValue;
	p[6] = descriptor.iConfiguration;
	p[7] = descriptor.bmAttributes;
	p[8] = descriptor.MaxPower;
	c->position = p;
	c->size = descriptor.bLength;
	descriptors_pos += descriptor.bLength;
	memcpy(&c->configuration_descriptor, &descriptor, sizeof(USBStandardConfigurationDescriptor));
	configurations.push_front(c);
	latest_configuration = c;
	latest_alternate = nullptr;
}

void ohci_function_device::add_interface_descriptor(const USBStandardInterfaceDescriptor &descriptor)
{
	usb_device_interface *ii;
	usb_device_interface_alternate *aa;
	UINT8 *p = descriptors + descriptors_pos;

	if (latest_configuration == nullptr)
		return;
	p[0] = descriptor.bLength;
	p[1] = descriptor.bDescriptorType;
	p[2] = descriptor.bInterfaceNumber;
	p[3] = descriptor.bAlternateSetting;
	p[4] = descriptor.bNumEndpoints;
	p[5] = descriptor.bInterfaceClass;
	p[6] = descriptor.bInterfaceSubClass;
	p[7] = descriptor.bInterfaceProtocol;
	p[8] = descriptor.iInterface;
	descriptors_pos += descriptor.bLength;
	latest_configuration->size += descriptor.bLength;
	for (auto i = latest_configuration->interfaces.begin(); i != latest_configuration->interfaces.end(); ++i)
	{
		if ((*i)->alternate_settings.front()->interface_descriptor.bInterfaceNumber == descriptor.bInterfaceNumber)
		{
			(*i)->size += descriptor.bLength;
			latest_configuration->interfaces.front()->size += descriptor.bLength;
			aa = new usb_device_interface_alternate;
			memcpy(&aa->interface_descriptor, &descriptor, sizeof(USBStandardInterfaceDescriptor));
			aa->position = p;
			aa->size = descriptor.bLength;
			(*i)->alternate_settings.push_front(aa);
			latest_alternate = aa;
			return;
		}
	}
	ii = new usb_device_interface;
	aa = new usb_device_interface_alternate;
	memcpy(&aa->interface_descriptor, &descriptor, sizeof(USBStandardInterfaceDescriptor));
	aa->position = p;
	aa->size = descriptor.bLength;
	ii->position = p;
	ii->size = descriptor.bLength;
	ii->selected_alternate = -1;
	ii->alternate_settings.push_front(aa);
	latest_alternate = aa;
	latest_configuration->interfaces.push_front(ii);
}

void ohci_function_device::add_endpoint_descriptor(const USBStandardEndpointDescriptor &descriptor)
{
	UINT8 *p = descriptors + descriptors_pos;

	if (latest_alternate == nullptr)
		return;
	p[0] = descriptor.bLength;
	p[1] = descriptor.bDescriptorType;
	p[2] = descriptor.bEndpointAddress;
	p[3] = descriptor.bmAttributes;
	p[4] = descriptor.wMaxPacketSize & 255;
	p[5] = descriptor.wMaxPacketSize >> 8;
	p[6] = descriptor.bInterval;
	descriptors_pos += descriptor.bLength;
	latest_alternate->endpoint_descriptors.push_front(descriptor);
	latest_alternate->size += descriptor.bLength;
	latest_configuration->interfaces.front()->size += descriptor.bLength;
	latest_configuration->size += descriptor.bLength;
}

void ohci_function_device::add_string_descriptor(const UINT8 *descriptor)
{
	usb_device_string *ss;
	int len = descriptor[0];
	UINT8 *p = descriptors + descriptors_pos;


	ss = new usb_device_string;
	memcpy(p, descriptor, len);
	descriptors_pos += len;
	ss->size = len;
	ss->position = p;
	device_strings.push_front(ss);
	//latest_configuration->size += len;
}

void ohci_function_device::select_configuration(int index)
{
	configurationvalue = index;
	for (auto c = configurations.begin(); c != configurations.end(); ++c)
	{
		if ((*c)->configuration_descriptor.bConfigurationValue == index)
		{
			selected_configuration = *c;
			// by default, activate alternate setting 0 in each interface
			for (auto i = (*c)->interfaces.begin(); i != (*c)->interfaces.end(); ++i)
			{
				(*i)->selected_alternate = 0;
				for (auto a = (*i)->alternate_settings.begin(); a != (*i)->alternate_settings.end(); ++a)
				{
					if ((*a)->interface_descriptor.bAlternateSetting == 0)
					{
						// activate the endpoints in interface i alternate setting 0
						for (auto e = (*a)->endpoint_descriptors.begin(); e != (*a)->endpoint_descriptors.end(); ++e)
						{
							endpoints[e->bEndpointAddress].type = e->bmAttributes & 3;
							endpoints[e->bEndpointAddress].remain = 0;
						}
						break;
					}
				}
			}
			break;
		}
	}
}

void ohci_function_device::select_alternate(int interfacei, int index)
{
	// among all the interfaces in the currently selected configuration, consider interface interfacei
	for (auto i = selected_configuration->interfaces.begin(); i != selected_configuration->interfaces.end(); ++i)
	{
		// deactivate the endpoints in the currently selected alternate setting for interface interfacei
		for (auto a = (*i)->alternate_settings.begin(); a != (*i)->alternate_settings.end(); ++a)
		{
			if (((*a)->interface_descriptor.bInterfaceNumber == interfacei) && ((*a)->interface_descriptor.bAlternateSetting == (*i)->selected_alternate))
			{
				for (auto e = (*a)->endpoint_descriptors.begin(); e != (*a)->endpoint_descriptors.end(); ++e)
				{
					endpoints[e->bEndpointAddress].type = -1;
				}
				break;
			}
		}
		// activate the endpoints in the newly selected alternate setting
		for (auto a = (*i)->alternate_settings.begin(); a != (*i)->alternate_settings.end(); ++a)
		{
			if (((*a)->interface_descriptor.bInterfaceNumber == interfacei) && ((*a)->interface_descriptor.bAlternateSetting == index))
			{
				(*i)->selected_alternate = index;
				for (auto e = (*a)->endpoint_descriptors.begin(); e != (*a)->endpoint_descriptors.end(); ++e)
				{
					endpoints[e->bEndpointAddress].type = e->bmAttributes & 3;
					endpoints[e->bEndpointAddress].remain = 0;
				}
				break;
			}
		}
	}
}

int ohci_function_device::find_alternate(int interfacei)
{
	// find the active alternate setting for interface inteerfacei
	for (auto i = selected_configuration->interfaces.begin(); i != selected_configuration->interfaces.end(); ++i)
	{
		for (auto a = (*i)->alternate_settings.begin(); a != (*i)->alternate_settings.end(); ++a)
		{
			if ((*a)->interface_descriptor.bInterfaceNumber == interfacei)
			{
				return (*i)->selected_alternate;
			}
		}
	}
	return 0;
}

UINT8 *ohci_function_device::position_device_descriptor(int &size)
{
	size = descriptors_pos; // descriptors[0];
	return descriptors;
}

UINT8 *ohci_function_device::position_configuration_descriptor(int index, int &size)
{
	for (auto c = configurations.begin(); c != configurations.end(); ++c)
	{
		if ((*c)->configuration_descriptor.bConfigurationValue == (index + 1))
		{
			size = (*c)->size;
			return (*c)->position;
		}
	}
	size = 0;
	return nullptr;
}

UINT8 *ohci_function_device::position_string_descriptor(int index, int &size)
{
	int i = 0;

	for (auto s = device_strings.begin(); s != device_strings.end(); ++s)
	{
		if (index == i)
		{
			size = (*s)->size;
			return (*s)->position;
		}
		i++;
	}
	size = 0;
	return nullptr;
}

void ohci_function_device::execute_reset()
{
	address = 0;
	newaddress = 0;
}

int ohci_function_device::execute_transfer(int endpoint, int pid, UINT8 *buffer, int size)
{
	int descriptortype, descriptorindex;

	if (pid == SetupPid) {
		USBSetupPacket *p=(USBSetupPacket *)buffer;
		// control transfers are done in 3 stages: first the setup stage, then an optional data stage, then a status stage
		// so there are 3 cases:
		// 1- control transfer with a data stage where the host sends data to the device
		//    in this case the sequence of pids transferred is control pid, data out pid, data in pid
		// 2- control transfer with a data stage where the host receives data from the device
		//    in this case the sequence of pids transferred is control pid, data in pid, data out pid
		// 3- control transfer without a data stage
		//    in this case the sequence of pids transferred is control pid, data in pid
		// define direction 0:host->device 1:device->host
		// direction == 1 -> IN data stage and OUT status stage
		// direction == 0 -> OUT data stage and IN status stage
		// data stage not present -> IN status stage
		endpoints[endpoint].controldirection = (p->bmRequestType & 128) >> 7;
		endpoints[endpoint].controltype = (p->bmRequestType & 0x60) >> 5;
		endpoints[endpoint].controlrecipient = p->bmRequestType & 0x1f;
		wantstatuscallback = false;
		if (endpoint == 0) {
			endpoints[endpoint].position = nullptr;
			// number of byte to transfer in data stage (0 no data stage)
			endpoints[endpoint].remain = p->wLength;
			// if standard device request
			if ((endpoints[endpoint].controltype == StandardType) && (endpoints[endpoint].controlrecipient == DeviceRecipient)) {
				switch (p->bRequest) {
				case GET_STATUS:
					return handle_get_status_request(endpoint, p);
					break;
				case CLEAR_FEATURE:
					return handle_clear_feature_request(endpoint, p);
					break;
				case SET_FEATURE:
					return handle_set_feature_request(endpoint, p);
					break;
				case SET_ADDRESS:
					newaddress = p->wValue;
					settingaddress = true;
					break;
				case GET_DESCRIPTOR:
					descriptortype = p->wValue >> 8;
					descriptorindex = p->wValue & 255;
					if (descriptortype == DEVICE) { // device descriptor
						endpoints[endpoint].position = position_device_descriptor(endpoints[endpoint].remain);
					}
					else if (descriptortype == CONFIGURATION) { // configuration descriptor
						endpoints[endpoint].position = position_configuration_descriptor(descriptorindex, endpoints[endpoint].remain);
					}
					else if (descriptortype == STRING) { // string descriptor
						//p->wIndex; language id
						endpoints[endpoint].position = position_string_descriptor(descriptorindex, endpoints[endpoint].remain);
					}
					else
						endpoints[endpoint].remain = 0;
					if (endpoints[endpoint].remain > p->wLength)
						endpoints[endpoint].remain = p->wLength;
					break;
				case SET_CONFIGURATION:
					if (p->wValue == 0)
						state = AddressState;
					else {
						select_configuration(p->wValue);
						state = ConfiguredState;
					}
					break;
				case SET_INTERFACE:
					select_alternate(p->wIndex, p->wValue);
					break;
				case SET_DESCRIPTOR:
					return handle_set_descriptor_request(endpoint, p);
					break;
				case GET_CONFIGURATION:
					endpoints[endpoint].buffer[0] = (UINT8)configurationvalue;
					endpoints[endpoint].position = endpoints[endpoint].buffer;
					endpoints[endpoint].remain = 1;
					if (p->wLength == 0)
						endpoints[endpoint].remain = 0;
					break;
				case GET_INTERFACE:
					endpoints[endpoint].buffer[0] = (UINT8)find_alternate(p->wIndex);
					endpoints[endpoint].position = endpoints[endpoint].buffer;
					endpoints[endpoint].remain = 1;
					if (p->wLength == 0)
						endpoints[endpoint].remain = 0;
					break;
				case SYNCH_FRAME:
					return handle_synch_frame_request(endpoint, p);
				default:
					return handle_nonstandard_request(endpoint, p);
					break;
				}
			}
			else
				return handle_nonstandard_request(endpoint, p);
			size = 0;
		}
		else
			return handle_nonstandard_request(endpoint, p);
	}
	else if (pid == InPid) {
		if (endpoints[endpoint].type == ControlEndpoint) { //if (endpoint == 0) {
			// if no data has been transferred (except for the setup stage)
			// and the lenght of this IN transaction is 0
			// assume this is the status stage
			if ((endpoints[endpoint].remain == 0) && (size == 0)) {
				if ((endpoint == 0) && (settingaddress == true))
				{
					// set of address is active at end of status stage
					busmanager->usb_ohci_device_address_changed(address, newaddress);
					address = newaddress;
					settingaddress = false;
					state = AddressState;
				}
				if (wantstatuscallback == true)
					handle_status_stage(endpoint);
				wantstatuscallback = false;
				return 0;
			}
			// case ==1, give data
			// case ==0, nothing
			// if device->host, since InPid then this is data stage
			if (endpoints[endpoint].controldirection == DeviceToHost) {
				// data stage
				if (size > endpoints[endpoint].remain)
					size = endpoints[endpoint].remain;
				if (endpoints[endpoint].position != nullptr)
					memcpy(buffer, endpoints[endpoint].position, size);
				endpoints[endpoint].position = endpoints[endpoint].position + size;
				endpoints[endpoint].remain = endpoints[endpoint].remain - size;
			}
			else {
				if (wantstatuscallback == true)
					handle_status_stage(endpoint);
				wantstatuscallback = false;
			}
		}
		else if (endpoints[endpoint].type == BulkEndpoint)
			return handle_bulk_pid(endpoint, pid, buffer, size);
		else if (endpoints[endpoint].type == InterruptEndpoint)
			return handle_interrupt_pid(endpoint, pid, buffer, size);
		else if (endpoints[endpoint].type == IsochronousEndpoint)
			return handle_isochronous_pid(endpoint, pid, buffer, size);
		else
			return -1;
	}
	else if (pid == OutPid) {
		if (endpoints[endpoint].type == ControlEndpoint) {
			// case ==1, nothing
			// case ==0, give data
			// if host->device, since OutPid then this is data stage
			if (endpoints[endpoint].controldirection == HostToDevice) {
				// data stage
				if (size > endpoints[endpoint].remain)
					size = endpoints[endpoint].remain;
				if (endpoints[endpoint].position != nullptr)
					memcpy(endpoints[endpoint].position, buffer, size);
				endpoints[endpoint].position = endpoints[endpoint].position + size;
				endpoints[endpoint].remain = endpoints[endpoint].remain - size;
			}
			else {
				if (wantstatuscallback == true)
					handle_status_stage(endpoint);
				wantstatuscallback = false;
			}
		}
		else if (endpoints[endpoint].type == BulkEndpoint)
			return handle_bulk_pid(endpoint, pid, buffer, size);
		else if (endpoints[endpoint].type == InterruptEndpoint)
			return handle_interrupt_pid(endpoint, pid, buffer, size);
		else if (endpoints[endpoint].type == IsochronousEndpoint)
			return handle_isochronous_pid(endpoint, pid, buffer, size);
		else
			return -1;
	}
	return size;
}

INPUT_PORTS_START(xbox_controller)
	PORT_START("ThumbstickLh") // left analog thumbstick horizontal movement
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("ThumbstickLh") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L)
	PORT_START("ThumbstickLv") // left analog thumbstick vertical movement
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("ThumbstickLv") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_K) PORT_CODE_INC(KEYCODE_I)

	PORT_START("ThumbstickRh") // right analog thumbstick horizontal movement
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("ThumbstickRh") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD)
	PORT_START("ThumbstickRv") // right analog thumbstick vertical movement
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("ThumbstickRv") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_2_PAD) PORT_CODE_INC(KEYCODE_8_PAD)

	PORT_START("DPad") // pressure sensitive directional pad
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("DPad Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("DPad Down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_NAME("DPad Left")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_NAME("DPad Right")

	PORT_START("TriggerL") // analog trigger
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("TriggerL") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_1_PAD) PORT_CODE_INC(KEYCODE_7_PAD)

	PORT_START("TriggerR") // analog trigger
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("TriggerR") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_3_PAD) PORT_CODE_INC(KEYCODE_9_PAD)

	PORT_START("Buttons") // digital buttons
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Start") // Start button
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Back") // Back button

	PORT_START("AGreen") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("A-Green") PORT_SENSITIVITY(100) PORT_KEYDELTA(32)  PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_A) PORT_CODE_INC(KEYCODE_Q)

	PORT_START("BRed") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("B-Red") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_S) PORT_CODE_INC(KEYCODE_W)

	PORT_START("XBlue") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("X-Blue") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_D) PORT_CODE_INC(KEYCODE_E)

	PORT_START("YYellow") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("Y-Yellow") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_F) PORT_CODE_INC(KEYCODE_R)

	PORT_START("Black") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("Black") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_G) PORT_CODE_INC(KEYCODE_T)

	PORT_START("White") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("White") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_H) PORT_CODE_INC(KEYCODE_Y)
INPUT_PORTS_END

const USBStandardDeviceDescriptor ohci_game_controller_device::devdesc = { 18,1,0x110,0x00,0x00,0x00,64,0x45e,0x202,0x100,0,0,0,1 };
const USBStandardConfigurationDescriptor ohci_game_controller_device::condesc = { 9,2,0x20,1,1,0,0x80,50 };
const USBStandardInterfaceDescriptor ohci_game_controller_device::intdesc = { 9,4,0,0,2,0x58,0x42,0,0 };
const USBStandardEndpointDescriptor ohci_game_controller_device::enddesc82 = { 7,5,0x82,3,0x20,4 };
const USBStandardEndpointDescriptor ohci_game_controller_device::enddesc02 = { 7,5,0x02,3,0x20,4 };

const device_type OHCI_GAME_CONTROLLER = &device_creator<ohci_game_controller_device>;

ohci_game_controller_device::ohci_game_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, OHCI_GAME_CONTROLLER, "OHCI Game Controller", tag, owner, clock, "ohci_gc", __FILE__),
	ohci_function_device(),
	m_ThumbstickLh(*this, "ThumbstickLh"),
	m_ThumbstickLv(*this, "ThumbstickLv"),
	m_ThumbstickRh(*this, "ThumbstickRh"),
	m_ThumbstickRv(*this, "ThumbstickRv"),
	m_DPad(*this, "DPad"),
	m_TriggerL(*this, "TriggerL"),
	m_TriggerR(*this, "TriggerR"),
	m_Buttons(*this, "Buttons"),
	m_AGreen(*this, "AGreen"),
	m_BRed(*this, "BRed"),
	m_XBlue(*this, "XBlue"),
	m_YYellow(*this, "YYellow"),
	m_Black(*this, "Black"),
	m_White(*this, "White")
{
}

void ohci_game_controller_device::initialize(running_machine &machine, ohci_usb_controller *usb_bus_manager)
{
	ohci_function_device::initialize(machine, usb_bus_manager);
	add_device_descriptor(devdesc);
	add_configuration_descriptor(condesc);
	add_interface_descriptor(intdesc);
	add_endpoint_descriptor(enddesc82);
	add_endpoint_descriptor(enddesc02);
}

int ohci_game_controller_device::handle_nonstandard_request(int endpoint, USBSetupPacket *setup)
{
	//                                    >=8  ==42  !=0  !=0  1,3       2<20 <=20
	static const UINT8 reportinfo[16] = { 0x10,0x42 ,0x32,0x43,1   ,0x65,0x14,0x20,0x98,0xa9,0xba,0xcb,0xdc,0xed,0xfe };

	if (endpoint != 0)
		return -1;
	if ((endpoints[endpoint].controltype == VendorType) && (endpoints[endpoint].controlrecipient == InterfaceRecipient))
	{
		if ((setup->bRequest == GET_DESCRIPTOR) && (setup->wValue == 0x4200))
		{
			endpoints[endpoint].position = (UINT8 *)reportinfo;
			endpoints[endpoint].remain = 16;
			return 0;
		}
	}
	if ((endpoints[endpoint].controltype == ClassType) && (endpoints[endpoint].controlrecipient == InterfaceRecipient))
	{
		if ((setup->bRequest == 1) && (setup->wValue == 0x0100))
		{
			endpoints[endpoint].position = endpoints[endpoint].buffer;
			endpoints[endpoint].remain = setup->wLength;
			for (int n = 0; n < setup->wLength; n++)
				endpoints[endpoint].buffer[n] = 0x10 ^ n;
			endpoints[endpoint].buffer[2] = 0;
			return 0;
		}
	}
	if ((endpoints[endpoint].controltype == VendorType) && (endpoints[endpoint].controlrecipient == InterfaceRecipient))
	{
		if ((setup->bRequest == 1) && (setup->wValue == 0x0200))
		{
			endpoints[endpoint].position = endpoints[endpoint].buffer;
			endpoints[endpoint].remain = setup->wLength;
			for (int n = 0; n < setup->wLength; n++)
				endpoints[endpoint].buffer[n] = 0x20 ^ n;
			return 0;
		}
	}
	if ((endpoints[endpoint].controltype == VendorType) && (endpoints[endpoint].controlrecipient == InterfaceRecipient))
	{
		if ((setup->bRequest == 1) && (setup->wValue == 0x0100))
		{
			endpoints[endpoint].position = endpoints[endpoint].buffer;
			endpoints[endpoint].remain = setup->wLength;
			for (int n = 0; n < setup->wLength; n++)
				endpoints[endpoint].buffer[n] = 0x30 ^ n;
			return 0;
		}
	}
	return -1;
}

int ohci_game_controller_device::handle_interrupt_pid(int endpoint, int pid, UINT8 *buffer, int size)
{
	if ((endpoint == 2) && (pid == InPid)) {
		int v;

		buffer[0] = 0;
		buffer[1] = 20;
		v = m_DPad->read();
		v = v | (m_Buttons->read() << 4);
		buffer[2] = (UINT8)v;
		buffer[3] = 0;
		buffer[4] = m_AGreen->read();
		buffer[5] = m_BRed->read();
		buffer[6] = m_XBlue->read();
		buffer[7] = m_YYellow->read();
		buffer[8] = m_Black->read();
		buffer[9] = m_White->read();
		buffer[10] = m_TriggerL->read();
		buffer[11] = m_TriggerR->read();
		v = m_ThumbstickLh->read();
		v = (v - 128) * 256;
		buffer[12] = (UINT16)v & 255;
		buffer[13] = (UINT16)v >> 8;
		v = m_ThumbstickLv->read();
		v = (v - 128) * 256;
		buffer[14] = (UINT16)v & 255;
		buffer[15] = (UINT16)v >> 8;
		v = m_ThumbstickRh->read();
		v = (v - 128) * 256;
		buffer[16] = (UINT16)v & 255;
		buffer[17] = (UINT16)v >> 8;
		v = m_ThumbstickRv->read();
		v = (v - 128) * 256;
		buffer[18] = (UINT16)v & 255;
		buffer[19] = (UINT16)v >> 8;
		return size;
	}
	return -1;
}

void ohci_game_controller_device::device_start()
{
}

ioport_constructor ohci_game_controller_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xbox_controller);
}

WRITE_LINE_MEMBER(xbox_base_state::xbox_ohci_usb_interrupt_changed)
{
	xbox_base_devs.pic8259_1->ir1_w(state);
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
