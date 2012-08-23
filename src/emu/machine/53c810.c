/* LSI Logic LSI53C810A PCI to SCSI I/O Processor */

#include "emu.h"
#include "53c810.h"
#include "machine/scsidev.h"

#define DMA_MAX_ICOUNT	512		/* Maximum number of DMA Scripts opcodes to run */
#define DASM_OPCODES 0

static scsidev_device *devices[8];	/* SCSI IDs 0-7 */
static const struct LSI53C810interface *intf;
static UINT8 last_id;

static struct {
	UINT8 scntl0;
	UINT8 scntl1;
	UINT8 scntl2;
	UINT8 scntl3;
	UINT8 scid;
	UINT8 sxfer;
	UINT8 socl;
	UINT8 istat;
	UINT8 dstat;
	UINT8 sstat0;
	UINT8 sstat1;
	UINT8 sstat2;
	UINT8 dien;
	UINT8 dcntl;
	UINT8 dmode;
	UINT32 temp;
	UINT32 dsa;
	UINT32 dsp;
	UINT32 dsps;
	UINT32 dcmd;
	UINT8 sien0;
	UINT8 sien1;
	UINT8 stime0;
	UINT8 respid;
	UINT8 stest1;
	UINT8 scratch_a[4];
	UINT8 scratch_b[4];
	int dma_icount;
	int halted;
	int carry;
	UINT32 (* fetch)(UINT32 dsp);
	void (* irq_callback)(running_machine &machine);
	void (* dma_callback)(UINT32, UINT32, int, int);
} lsi810;

typedef void (*opcode_handler)(running_machine &machine);
#define OPCODE_HANDLER(name) void name(running_machine &machine)
static opcode_handler dma_opcode[256];

INLINE UINT32 FETCH(running_machine &machine)
{
	UINT32 r = intf->fetch(machine, lsi810.dsp);
	lsi810.dsp += 4;
	return r;
}

#ifdef UNUSED_FUNCTION
static UINT32 sign_extend24(UINT32 val)
{
	if (val & 0x00800000)
		val |= 0xFF000000;
	else
		val &= ~0xFF000000;
	return val;
}
#endif

static OPCODE_HANDLER( dmaop_invalid )
{
	fatalerror("LSI53C810: Invalid SCRIPTS DMA opcode %08X at %08X", lsi810.dcmd, lsi810.dsp);
}

static OPCODE_HANDLER( dmaop_move_memory )
{
	UINT32 src = FETCH(machine);
	UINT32 dst = FETCH(machine);
	int count;

	count = lsi810.dcmd & 0xffffff;
	if(intf->dma_callback != NULL) {
		intf->dma_callback(machine, src, dst, count, 1);
	}
}

static OPCODE_HANDLER( dmaop_interrupt )
{
	if(lsi810.dcmd & 0x100000) {
		fatalerror("LSI53C810: INTFLY opcode not implemented");
	}
	lsi810.dsps = FETCH(machine);

	lsi810.istat |= 0x1;	/* DMA interrupt pending */
	lsi810.dstat |= 0x4;	/* SIR (SCRIPTS Interrupt Instruction Received) */

	if(intf->irq_callback != NULL) {
		intf->irq_callback(machine, 1);
	}
	lsi810.dma_icount = 0;
	lsi810.halted = 1;
}

static OPCODE_HANDLER( dmaop_block_move )
{
	UINT32 address;
	UINT32 count;
	INT32 dsps;

	address = FETCH(machine);
	count = lsi810.dcmd & 0x00ffffff;

	// normal indirect
	if (lsi810.dcmd & 0x20000000)
		address = intf->fetch(machine, address);

	// table indirect
	if (lsi810.dcmd & 0x10000000)
	{
		dsps = (INT32)address&0xffffff;
		// sign extend
		if (dsps & 0x00800000)
		{
			dsps |= 0xff000000;
		}
		logerror("table offset: %x, DSA = %x\n", dsps, lsi810.dsa);
		dsps += lsi810.dsa;

		logerror("Loading from table at %x\n", dsps);
		count = lsi810.fetch(dsps);
		address = lsi810.fetch(dsps+4);
	}

	logerror("block move: address %x count %x phase %x\n", address, count, (lsi810.dcmd>>24)&7);

	if (lsi810.scntl0 & 0x01)
	{
		/* target mode */
		fatalerror("LSI53C810: dmaop_block_move not implemented in target mode");
	}
	else
	{
		/* initiator mode */
		logerror("53c810: block_move not actually implemented\n");
	}
}

static OPCODE_HANDLER( dmaop_select )
{
//  UINT32 operand;

//  operand = FETCH(machine);

	if (lsi810.scntl0 & 0x01)
	{
		/* target mode */
		logerror("LSI53C810: reselect ID #%d\n", (lsi810.dcmd >> 16) & 0x07);
	}
	else
	{
		/* initiator mode */
		logerror("53c810: SELECT: our ID %d, target ID %d\n", lsi810.scid&7, (lsi810.dcmd>>16)&7);

		lsi810.sstat1 &= ~0x07;	// clear current bus phase
		if (lsi810.dcmd & 0x01000000)	// select with ATN
		{
			mame_printf_debug("53c810: want select with ATN, setting message phase\n");
			lsi810.sstat1 |= 0x7;	// ATN means we want message in phase
		}
	}
}

static OPCODE_HANDLER( dmaop_wait_disconnect )
{
//  UINT32 operand;

//  operand = FETCH(machine);

	if (lsi810.scntl0 & 0x01)
	{
		/* target mode */
		fatalerror("LSI53C810: dmaop_wait_disconnect not implemented in target mode");
	}
	else
	{
		/* initiator mode */
		fatalerror("LSI53C810: dmaop_wait_disconnect not implemented");
	}
}

static OPCODE_HANDLER( dmaop_wait_reselect )
{
  //  UINT32 operand;

//  operand = FETCH(machine);

	if (lsi810.scntl0 & 0x01)
	{
		/* target mode */
		fatalerror("LSI53C810: dmaop_wait_reselect not implemented in target mode");
	}
	else
	{
		/* initiator mode */
		fatalerror("LSI53C810: dmaop_wait_reselect not implemented");
	}
}

static OPCODE_HANDLER( dmaop_set )
{
//  UINT32 operand;

//  operand = FETCH(machine);

	/* initiator mode */
	if (lsi810.dcmd & 0x8)
	{
		// set ATN in SOCL
		lsi810.socl |= 0x08;
	}
	if (lsi810.dcmd & 0x40)
	{
		// set ACK in SOCL
		lsi810.socl |= 0x40;
	}
	if (lsi810.dcmd & 0x200)
	{
		// set target mode
		lsi810.scntl0 |= 0x01;
	}
	if (lsi810.dcmd & 0x400)
	{
		// set carry in ALU
		lsi810.carry = 1;
	}
}

static OPCODE_HANDLER( dmaop_clear )
{
//  UINT32 operand;

//  operand = FETCH(machine);

	/* initiator mode */
	if (lsi810.dcmd & 0x8)
	{
		//  clear ATN in SOCL
		lsi810.socl &= ~0x08;
	}
	if (lsi810.dcmd & 0x40)
	{
		// clear ACK in SOCL
		lsi810.socl &= ~0x40;
	}
	if (lsi810.dcmd & 0x200)
	{
		// clear target mode
		lsi810.scntl0 &= ~0x01;
	}
	if (lsi810.dcmd & 0x400)
	{
		// clear carry in ALU
		lsi810.carry = 0;
	}
}

static OPCODE_HANDLER( dmaop_move_from_sfbr )
{
	fatalerror("LSI53C810: dmaop_move_from_sfbr not implemented in target mode");
}

static OPCODE_HANDLER( dmaop_move_to_sfbr )
{
	fatalerror("LSI53C810: dmaop_move_to_sfbr not implemented");
}

static OPCODE_HANDLER( dmaop_read_modify_write )
{
	fatalerror("LSI53C810: dmaop_read_modify_write not implemented");
}

static int scripts_compute_branch(void)
{
	int dtest, ptest, wanted, passed;

//        |jump if true
// 878b0000   ||compare data
// 1000 0111 1000 1011 0000 0000 0000 0000
//   |   |rel   ||wait valid phase
//   |      |compare phase
//   |desired phase: message in

	if (lsi810.dcmd & 0x00200000)
	{
		fatalerror("LSI53C810: jump with carry test not implemented");
	}

	if (lsi810.dcmd & 0x00100000)
	{
		fatalerror("LSI53C810: jump with interrupt on the fly not implemented");
	}

	// set desired result to take jump
	wanted = (lsi810.dcmd & 0x00080000) ? 1 : 0;
	// default to passing the tests in case they're disabled
	dtest = ptest = wanted;

	// phase test?
	if (lsi810.dcmd & 0x00020000)
	{
		logerror("53c810: phase test.  current: %x.  target: %x\n", lsi810.sstat1 & 7, (lsi810.dcmd>>24)&7);

		// do the phases match?
		if (((lsi810.dcmd>>24)&7) == (lsi810.sstat1 & 7))
		{
			ptest = 1;
		}
	else
		{
			ptest = 0;
		}
	}

	// data test?
	if (lsi810.dcmd & 0x00040000)
	{
		logerror("53c810: data test.  target: %x [not yet implemented]\n", lsi810.dcmd&0xff);
	}

	// if all conditions go, take the jump
	passed = 0;
	if ((ptest == dtest) && (dtest == wanted))
	{
		passed = 1;
	}

	logerror("53c810: phase test %d  data test %d  wanted %d => pass %d\n", ptest, dtest, wanted, passed);

	return passed;
}

static UINT32 scripts_get_jump_dest(running_machine &machine)
{
	INT32 dsps;
	UINT32 dest;

	dsps = FETCH(machine);

	/* relative or absolute addressing? */
	if (lsi810.dcmd & 0x00800000)
	{
		// sign-extend the 24-bit value
		if (dsps & 0x00800000)
		{
			dsps |= 0xff000000;
		}

		logerror("dsps = %x, dsp = %x\n", dsps, lsi810.dsp);
		dsps += lsi810.dsp;
	}

	dest = (UINT32)dsps;

	logerror("cur DSP %x, dest %x\n", lsi810.dsp, dest);

	return dest;
}

static OPCODE_HANDLER( dmaop_jump )
{
	if (scripts_compute_branch())
	{
		lsi810.dsp = scripts_get_jump_dest(machine);
	}
	else
	{
		FETCH(machine);	// skip operand to continue on
	}
}

static OPCODE_HANDLER( dmaop_call )
{
	if (scripts_compute_branch())
	{
		// save return address
		lsi810.temp = lsi810.dsp;

		// and go
		lsi810.dsp = scripts_get_jump_dest(machine);
	}
	else
	{
		FETCH(machine);	// skip operand to continue on
	}
}

static OPCODE_HANDLER( dmaop_return )
{
	// is this correct?  return only happens if the condition is true?
	if (scripts_compute_branch())
	{
		// restore return address
		lsi810.dsp = lsi810.temp;
	}
	else
	{
		FETCH(machine);	// skip operand to continue on
	}
}

static OPCODE_HANDLER( dmaop_store )
{
	fatalerror("LSI53C810: dmaop_store not implemented");
}

static OPCODE_HANDLER( dmaop_load )
{
	fatalerror("LSI53C810: dmaop_load not implemented");
}



static void dma_exec(running_machine &machine)
{
	lsi810.dma_icount = DMA_MAX_ICOUNT;

	while(lsi810.dma_icount > 0)
	{
		int op;

		if (DASM_OPCODES)
		{
			char buf[256];
			lsi53c810_dasm(machine, buf, lsi810.dsp);
			logerror("0x%08X: %s\n", lsi810.dsp, buf);
		}

		lsi810.dcmd = FETCH(machine);

		op = (lsi810.dcmd >> 24) & 0xff;
		dma_opcode[op](machine);

		lsi810.dma_icount--;
	}
}

READ8_HANDLER( lsi53c810_reg_r )
{
	logerror("53c810: read reg %d:0x%x (PC=%x)\n", offset, offset, cpu_get_pc(&space->device()));
	switch(offset)
	{
		case 0x00:		/* SCNTL0 */
			return lsi810.scntl0;
		case 0x01:		/* SCNTL1 */
			return lsi810.scntl1;
		case 0x02:		/* SCNTL2 */
			return lsi810.scntl2;
		case 0x03:		/* SCNTL3 */
			return lsi810.scntl3;
		case 0x04:		/* SCID */
			return lsi810.scid;
		case 0x05:		/* SXFER */
			return lsi810.sxfer;
		case 0x09:		/* SOCL */
			return lsi810.socl;
		case 0x0c:		/* DSTAT */
			return lsi810.dstat;
		case 0x0d:		/* SSTAT0 */
			return lsi810.sstat0;
		case 0x0e:		/* SSTAT1 */
			return lsi810.sstat1;
		case 0x0f:		/* SSTAT2 */
			return lsi810.sstat2;
		case 0x10:		/* DSA [7-0] */
			return lsi810.dsa & 0xff;
		case 0x11:		/* DSA [15-8] */
			return (lsi810.dsa >> 8) & 0xff;
		case 0x12:		/* DSA [23-16] */
			return (lsi810.dsa >> 16) & 0xff;
		case 0x13:		/* DSA [31-24] */
			return (lsi810.dsa >> 24) & 0xff;
		case 0x14:		/* ISTAT */
			// clear the interrupt on service
			if(intf->irq_callback != NULL)
			{
				intf->irq_callback(space->machine(), 0);
			}

			return lsi810.istat;
		case 0x2c:		/* DSP [7-0] */
			return lsi810.dsp & 0xff;
		case 0x2d:		/* DSP [15-8] */
			return (lsi810.dsp >> 8) & 0xff;
		case 0x2e:		/* DSP [23-16] */
			return (lsi810.dsp >> 16) & 0xff;
		case 0x2f:		/* DSP [31-24] */
			return (lsi810.dsp >> 24) & 0xff;
		case 0x34:		/* SCRATCH A */
		case 0x35:
		case 0x36:
		case 0x37:
			return lsi810.scratch_a[offset % 4];
		case 0x39:		/* DIEN */
			return lsi810.dien;
		case 0x3b:		/* DCNTL */
			return lsi810.dcntl;
		case 0x40:		/* SIEN0 */
			return lsi810.sien0;
		case 0x41:		/* SIEN1 */
			return lsi810.sien1;
		case 0x48:		/* STIME0 */
			return lsi810.stime0;
		case 0x4a:		/* RESPID */
			return lsi810.respid;
		case 0x4d:		/* STEST1 */
			return lsi810.stest1;
		case 0x5c:		/* SCRATCH B */
		case 0x5d:
		case 0x5e:
		case 0x5f:
			return lsi810.scratch_b[offset % 4];

		default:
			fatalerror("LSI53C810: reg_r: Unknown reg %02X", offset);
	}

	return 0;
}

WRITE8_HANDLER( lsi53c810_reg_w )
{
	logerror("53c810: %02x to reg %d:0x%x (PC=%x)\n", data, offset, offset, cpu_get_pc(&space->device()));
	switch(offset)
	{
		case 0x00:		/* SCNTL0 */
			lsi810.scntl0 = data;
			break;
		case 0x01:		/* SCNTL1 */
			lsi810.scntl1 = data;
			break;
		case 0x02:		/* SCNTL2 */
			lsi810.scntl2 = data;
			break;
		case 0x03:		/* SCNTL3 */
			lsi810.scntl3 = data;
			break;
		case 0x04:		/* SCID */
			lsi810.scid = data;
			break;
		case 0x05:		/* SXFER */
			lsi810.sxfer = data;
			break;
		case 0x09:		/* SOCL */
			lsi810.socl = data;
			break;
		case 0x0d:		/* SSTAT0 */
			lsi810.sstat0 = data;
			break;
		case 0x0e:		/* SSTAT1 */
			lsi810.sstat1 = data;
			break;
		case 0x0f:		/* SSTAT2 */
			lsi810.sstat2 = data;
			break;
		case 0x10:		/* DSA [7-0] */
			lsi810.dsa &= 0xffffff00;
			lsi810.dsa |= data;
			break;
		case 0x11:		/* DSA [15-8] */
			lsi810.dsa &= 0xffff00ff;
			lsi810.dsa |= data << 8;
			break;
		case 0x12:		/* DSA [23-16] */
			lsi810.dsa &= 0xff00ffff;
			lsi810.dsa |= data << 16;
			break;
		case 0x13:		/* DSA [31-24] */
			lsi810.dsa &= 0x00ffffff;
			lsi810.dsa |= data << 24;
			break;
		case 0x14:		/* ISTAT */
			lsi810.istat = data;
			break;
		case 0x2c:		/* DSP [7-0] */
			lsi810.dsp &= 0xffffff00;
			lsi810.dsp |= data;
			break;
		case 0x2d:		/* DSP [15-8] */
			lsi810.dsp &= 0xffff00ff;
			lsi810.dsp |= data << 8;
			break;
		case 0x2e:		/* DSP [23-16] */
			lsi810.dsp &= 0xff00ffff;
			lsi810.dsp |= data << 16;
			break;
		case 0x2f:		/* DSP [31-24] */
			lsi810.dsp &= 0x00ffffff;
			lsi810.dsp |= data << 24;
			lsi810.halted = 0;
			if((lsi810.dmode & 0x1) == 0 && !lsi810.halted) {
				dma_exec(space->machine());
			}
			break;
		case 0x34:		/* SCRATCH A */
		case 0x35:
		case 0x36:
		case 0x37:
			lsi810.scratch_a[offset % 4] = data;
			break;
		case 0x38:		/* DMODE */
			lsi810.dmode = data;
			break;
		case 0x39:		/* DIEN */
			lsi810.dien = data;
			break;
		case 0x3b:		/* DCNTL */
			lsi810.dcntl = data;

			if(lsi810.dcntl & 0x14 && !lsi810.halted)		/* single-step & start DMA */
			{
				int op;
				lsi810.dcmd = FETCH(space->machine());
				op = (lsi810.dcmd >> 24) & 0xff;
				dma_opcode[op](space->machine());

				lsi810.istat |= 0x3;	/* DMA interrupt pending */
				lsi810.dstat |= 0x8;	/* SSI (Single Step Interrupt) */
				if(intf->irq_callback != NULL) {
					intf->irq_callback(space->machine(), 1);
				}
			}
			else if(lsi810.dcntl & 0x04 && !lsi810.halted)	/* manual start DMA */
			{
				dma_exec(space->machine());
			}
			break;
		case 0x40:		/* SIEN0 */
			lsi810.sien0 = data;
			break;
		case 0x41:		/* SIEN1 */
			lsi810.sien1 = data;
			break;
		case 0x48:		/* STIME0 */
			lsi810.stime0 = data;
			break;
		case 0x4a:		/* RESPID */
			lsi810.respid = data;
			break;
		case 0x4d:		/* STEST1 */
			lsi810.stest1 = data;
			break;
		case 0x5c:		/* SCRATCH B */
		case 0x5d:
		case 0x5e:
		case 0x5f:
			lsi810.scratch_b[offset % 4] = data;
			break;

		default:
			fatalerror("LSI53C810: reg_w: Unknown reg %02X, %02X", offset, data);
	}
}

static void add_opcode(UINT8 op, UINT8 mask, opcode_handler handler)
{
	int i;
	for(i=0; i < 256; i++) {
		if((i & mask) == op) {
			dma_opcode[i] = handler;
		}
	}
}

void lsi53c810_init(running_machine &machine, const struct LSI53C810interface *interface)
{
	int i;

	// save interface pointer for later
	intf = interface;

	memset(&lsi810, 0, sizeof(lsi810));
	for(i = 0; i < 256; i++)
	{
		dma_opcode[i] = dmaop_invalid;
	}

	add_opcode(0x00, 0xc0, dmaop_block_move);
	add_opcode(0x40, 0xf8, dmaop_select);
	add_opcode(0x48, 0xf8, dmaop_wait_disconnect);
	add_opcode(0x50, 0xf8, dmaop_wait_reselect);
	add_opcode(0x58, 0xf8, dmaop_set);
	add_opcode(0x60, 0xf8, dmaop_clear);
	add_opcode(0x68, 0xf8, dmaop_move_from_sfbr);
	add_opcode(0x70, 0xf8, dmaop_move_to_sfbr);
	add_opcode(0x78, 0xf8, dmaop_read_modify_write);
	add_opcode(0x80, 0xf8, dmaop_jump);
	add_opcode(0x88, 0xf8, dmaop_call);
	add_opcode(0x90, 0xf8, dmaop_return);
	add_opcode(0x98, 0xf8, dmaop_interrupt);
	add_opcode(0xc0, 0xfe, dmaop_move_memory);
	add_opcode(0xe0, 0xed, dmaop_store);
	add_opcode(0xe1, 0xed, dmaop_load);

	memset(devices, 0, sizeof(devices));

	// try to open the devices
	for (i = 0; i < interface->scsidevs->devs_present; i++)
	{
		scsidev_device *device = machine.device<scsidev_device>( interface->scsidevs->devices[i].tag );
		devices[device->GetDeviceID()] = device;
	}
}

void lsi53c810_read_data(int bytes, UINT8 *pData)
{
	if (devices[last_id])
	{
		devices[last_id]->ReadData( pData, bytes);
	}
	else
	{
		logerror("lsi53c810: read unknown device SCSI ID %d\n", last_id);
	}
}

void lsi53c810_write_data(int bytes, UINT8 *pData)
{
	if (devices[last_id])
	{
		devices[last_id]->WriteData( pData, bytes );
	}
	else
	{
		logerror("lsi53c810: write to unknown device SCSI ID %d\n", last_id);
	}
}

/*************************************
 *
 *  Disassembler
 *
 *************************************/

static UINT32 lsi53c810_dasm_fetch(running_machine &machine, UINT32 pc)
{
	return intf->fetch(machine, pc);
}

unsigned lsi53c810_dasm(running_machine &machine, char *buf, UINT32 pc)
{
	unsigned result = 0;
	const char *op_mnemonic = NULL;
	UINT32 op = lsi53c810_dasm_fetch(machine, pc);
	UINT32 dest;
	int i;

	static const char *const phases[] =
	{
		"DATA_OUT", "DATA_IN", "CMD", "STATUS",
		"RESERVED_OUT??", "RESERVED_IN??", "MSG_OUT", "MSG_IN"
	};

	if ((op & 0xF8000000) == 0x40000000)
	{
		/* SELECT */
		dest = lsi53c810_dasm_fetch(machine, pc + 4);

		buf += sprintf(buf, "SELECT%s %d, 0x%08X",
			(op & 0x01000000) ? " ATN" : "",
			(op >> 16) & 0x07,
			dest);

		result = 8;
	}
	else if (((op & 0xF8000000) == 0x58000000)
		| ((op & 0xF8000000) == 0x60000000))
	{
		static const struct
		{
			UINT32 flag;
			const char *text;
		} flags[] =
		{
			{ 0x00000008, "ATN" },
			{ 0x00000040, "ACK" },
			{ 0x00000200, "TARGET" },
			{ 0x00000400, "CARRY" }
		};
		int need_cojunction = FALSE;

		/* SET/CLEAR */
		switch(op & 0xF8000000)
		{
			case 0x58000000: op_mnemonic = "SET"; break;
			case 0x60000000: op_mnemonic = "CLEAR"; break;
		}

		buf += sprintf(buf, "%s ", op_mnemonic);
		need_cojunction = FALSE;

		for (i = 0; i < sizeof(flags) / sizeof(flags[0]); i++)
		{
			if (op & flags[i].flag)
			{
				if (need_cojunction)
					buf += sprintf(buf, " AND ");
				else
					need_cojunction = TRUE;
				buf += sprintf(buf, "%s", flags[i].text);
			}
		}
	}
	else if (((op & 0xF8000000) == 0x80000000)
		| ((op & 0xF8000000) == 0x88000000)
		| ((op & 0xF8000000) == 0x98000000))
	{
		/* JUMP/CALL/INT */
		switch(op & 0xF8000000)
		{
			case 0x80000000: op_mnemonic = "JUMP"; break;
			case 0x88000000: op_mnemonic = "CALL"; break;
			case 0x98000000: op_mnemonic = "INT"; break;
		}

		dest = lsi53c810_dasm_fetch(machine, pc + 4);

		if (op & 0x00800000)
		{
			/* relative */
			if (dest & 0x00800000)
				dest |= 0xFF000000;
			else
				dest &= 0x00FFFFFF;
			dest = (pc + 8) + dest;
			buf += sprintf(buf, "%s REL(0x%08X)", op_mnemonic, dest);
		}
		else
		{
			/* absolute */
			buf += sprintf(buf, "%s 0x%08X", op_mnemonic, dest);
		}

		switch(op & 0x000B0000)
		{
			case 0x00000000:
				buf += sprintf(buf, ", NOT??");
				break;

			case 0x00080000:
				break;

			case 0x00020000:
			case 0x00030000:
			case 0x000A0000:
			case 0x000B0000:
				buf += sprintf(buf, ", %s%s %s",
					(op & 0x00010000) ? "WHEN" : "IF",
					(op & 0x00080000) ? "" : " NOT",
					phases[(op >> 24) & 0x07]);
				break;

			default:
				fatalerror("unknown op 0x%08X", op);
				break;
		}
		result = 8;
	}
	else if ((op & 0xE0000000) == 0x00000000)
	{
		/* MOVE FROM */
		dest = lsi53c810_dasm_fetch(machine, pc + 4);

		buf += sprintf(buf, "MOVE FROM 0x%08X, WHEN %s",
			dest, phases[(op >> 24) & 0x07]);

		result = 8;
	}
	else if ((op & 0xE0000000) == 0x20000000)
	{
		/* MOVE PTR */
		dest = lsi53c810_dasm_fetch(machine, pc + 4);

		buf += sprintf(buf, "MOVE 0x%08X, PTR 0x%08X, WHEN %s",
			(op & 0x00FFFFFF), dest, phases[(op >> 24) & 0x07]);

		result = 8;
	}
	else
	{
		fatalerror("unknown op 0x%08X", op);
	}
	return result;
}

