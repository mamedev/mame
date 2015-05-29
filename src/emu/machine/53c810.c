// license:BSD-3-Clause
// copyright-holders:smf
/* LSI Logic LSI53C810A PCI to SCSI I/O Processor */

#include "emu.h"
#include "53c810.h"
#include "bus/scsi/scsihle.h"

#define DMA_MAX_ICOUNT  512     /* Maximum number of DMA Scripts opcodes to run */
#define DASM_OPCODES 0

UINT32 lsi53c810_device::FETCH()
{
	UINT32 r = m_fetch_cb(dsp);
	dsp += 4;
	return r;
}

void lsi53c810_device::dmaop_invalid()
{
	fatalerror("LSI53C810: Invalid SCRIPTS DMA opcode %08X at %08X\n", dcmd, dsp);
}

void lsi53c810_device::dmaop_move_memory()
{
	UINT32 src = FETCH();
	UINT32 dst = FETCH();
	int count;

	count = dcmd & 0xffffff;
	if (!m_dma_cb.isnull())
		m_dma_cb(src, dst, count, 1);
}

void lsi53c810_device::dmaop_interrupt()
{
	if(dcmd & 0x100000) {
		fatalerror("LSI53C810: INTFLY opcode not implemented\n");
	}
	dsps = FETCH();

	istat |= 0x1;   /* DMA interrupt pending */
	dstat |= 0x4;   /* SIR (SCRIPTS Interrupt Instruction Received) */

	if (!m_irq_cb.isnull())
		m_irq_cb(1);

	dma_icount = 0;
	halted = 1;
}

void lsi53c810_device::dmaop_block_move()
{
	UINT32 address;
	UINT32 count;
	INT32 dsps;

	address = FETCH();
	count = dcmd & 0x00ffffff;

	// normal indirect
	if (dcmd & 0x20000000)
		address = m_fetch_cb(address);

	// table indirect
	if (dcmd & 0x10000000)
	{
		dsps = (INT32)address&0xffffff;
		// sign extend
		if (dsps & 0x00800000)
		{
			dsps |= 0xff000000;
		}
		logerror("table offset: %x, DSA = %x\n", dsps, dsa);
		dsps += dsa;

		logerror("Loading from table at %x\n", dsps);
		count = m_fetch_cb(dsps);
		address = m_fetch_cb(dsps + 4);
	}

	logerror("block move: address %x count %x phase %x\n", address, count, (dcmd>>24)&7);

	if (scntl0 & 0x01)
	{
		/* target mode */
		fatalerror("LSI53C810: dmaop_block_move not implemented in target mode\n");
	}
	else
	{
		/* initiator mode */
		logerror("53c810: block_move not actually implemented\n");
	}
}

void lsi53c810_device::dmaop_select()
{
//  UINT32 operand;

//  operand = FETCH();

	if (scntl0 & 0x01)
	{
		/* target mode */
		logerror("LSI53C810: reselect ID #%d\n", (dcmd >> 16) & 0x07);
	}
	else
	{
		select((dcmd>>16)&7);

		/* initiator mode */
		logerror("53c810: SELECT: our ID %d, target ID %d\n", scid&7, (dcmd>>16)&7);

		sstat1 &= ~0x07;    // clear current bus phase
		if (dcmd & 0x01000000)  // select with ATN
		{
			osd_printf_debug("53c810: want select with ATN, setting message phase\n");
			sstat1 |= 0x7;  // ATN means we want message in phase
		}
	}
}

void lsi53c810_device::dmaop_wait_disconnect()
{
//  UINT32 operand;

//  operand = FETCH();

	if (scntl0 & 0x01)
	{
		/* target mode */
		fatalerror("LSI53C810: dmaop_wait_disconnect not implemented in target mode\n");
	}
	else
	{
		/* initiator mode */
		fatalerror("LSI53C810: dmaop_wait_disconnect not implemented\n");
	}
}

void lsi53c810_device::dmaop_wait_reselect()
{
	//  UINT32 operand;

//  operand = FETCH();

	if (scntl0 & 0x01)
	{
		/* target mode */
		fatalerror("LSI53C810: dmaop_wait_reselect not implemented in target mode\n");
	}
	else
	{
		/* initiator mode */
		fatalerror("LSI53C810: dmaop_wait_reselect not implemented\n");
	}
}

void lsi53c810_device::dmaop_set()
{
//  UINT32 operand;

//  operand = FETCH();

	/* initiator mode */
	if (dcmd & 0x8)
	{
		// set ATN in SOCL
		socl |= 0x08;
	}
	if (dcmd & 0x40)
	{
		// set ACK in SOCL
		socl |= 0x40;
	}
	if (dcmd & 0x200)
	{
		// set target mode
		scntl0 |= 0x01;
	}
	if (dcmd & 0x400)
	{
		// set carry in ALU
		carry = 1;
	}
}

void lsi53c810_device::dmaop_clear()
{
//  UINT32 operand;

//  operand = FETCH();

	/* initiator mode */
	if (dcmd & 0x8)
	{
		//  clear ATN in SOCL
		socl &= ~0x08;
	}
	if (dcmd & 0x40)
	{
		// clear ACK in SOCL
		socl &= ~0x40;
	}
	if (dcmd & 0x200)
	{
		// clear target mode
		scntl0 &= ~0x01;
	}
	if (dcmd & 0x400)
	{
		// clear carry in ALU
		carry = 0;
	}
}

void lsi53c810_device::dmaop_move_from_sfbr()
{
	fatalerror("LSI53C810: dmaop_move_from_sfbr not implemented in target mode\n");
}

void lsi53c810_device::dmaop_move_to_sfbr()
{
	fatalerror("LSI53C810: dmaop_move_to_sfbr not implemented\n");
}

void lsi53c810_device::dmaop_read_modify_write()
{
	fatalerror("LSI53C810: dmaop_read_modify_write not implemented\n");
}

int lsi53c810_device::scripts_compute_branch()
{
	int dtest, ptest, wanted, passed;

//        |jump if true
// 878b0000   ||compare data
// 1000 0111 1000 1011 0000 0000 0000 0000
//   |   |rel   ||wait valid phase
//   |      |compare phase
//   |desired phase: message in

	if (dcmd & 0x00200000)
	{
		fatalerror("LSI53C810: jump with carry test not implemented\n");
	}

	if (dcmd & 0x00100000)
	{
		fatalerror("LSI53C810: jump with interrupt on the fly not implemented\n");
	}

	// set desired result to take jump
	wanted = (dcmd & 0x00080000) ? 1 : 0;
	// default to passing the tests in case they're disabled
	dtest = ptest = wanted;

	// phase test?
	if (dcmd & 0x00020000)
	{
		logerror("53c810: phase test.  current: %x.  target: %x\n", sstat1 & 7, (dcmd>>24)&7);

		// do the phases match?
		if (((dcmd>>24)&7) == (sstat1 & 7))
		{
			ptest = 1;
		}
	else
		{
			ptest = 0;
		}
	}

	// data test?
	if (dcmd & 0x00040000)
	{
		logerror("53c810: data test.  target: %x [not yet implemented]\n", dcmd&0xff);
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

UINT32 lsi53c810_device::scripts_get_jump_dest()
{
	INT32 dsps;
	UINT32 dest;

	dsps = FETCH();

	/* relative or absolute addressing? */
	if (dcmd & 0x00800000)
	{
		// sign-extend the 24-bit value
		if (dsps & 0x00800000)
		{
			dsps |= 0xff000000;
		}

		logerror("dsps = %x, dsp = %x\n", dsps, dsp);
		dsps += dsp;
	}

	dest = (UINT32)dsps;

	logerror("cur DSP %x, dest %x\n", dsp, dest);

	return dest;
}

void lsi53c810_device::dmaop_jump()
{
	if (scripts_compute_branch())
	{
		dsp = scripts_get_jump_dest();
	}
	else
	{
		FETCH();    // skip operand to continue on
	}
}

void lsi53c810_device::dmaop_call()
{
	if (scripts_compute_branch())
	{
		// save return address
		temp = dsp;

		// and go
		dsp = scripts_get_jump_dest();
	}
	else
	{
		FETCH();    // skip operand to continue on
	}
}

void lsi53c810_device::dmaop_return()
{
	// is this correct?  return only happens if the condition is true?
	if (scripts_compute_branch())
	{
		// restore return address
		dsp = temp;
	}
	else
	{
		FETCH();    // skip operand to continue on
	}
}

void lsi53c810_device::dmaop_store()
{
	fatalerror("LSI53C810: dmaop_store not implemented\n");
}

void lsi53c810_device::dmaop_load()
{
	fatalerror("LSI53C810: dmaop_load not implemented\n");
}



void lsi53c810_device::dma_exec()
{
	dma_icount = DMA_MAX_ICOUNT;

	while(dma_icount > 0)
	{
		int op;

		if (DASM_OPCODES)
		{
			char buf[256];
			lsi53c810_dasm(buf, dsp);
			logerror("0x%08X: %s\n", dsp, buf);
		}

		dcmd = FETCH();

		op = (dcmd >> 24) & 0xff;
		dma_opcode[op]();

		dma_icount--;
	}
}

UINT8 lsi53c810_device::lsi53c810_reg_r( int offset )
{
//  logerror("53c810: read reg %d:0x%x (PC=%x)\n", offset, offset, space.device().safe_pc());
	switch(offset)
	{
		case 0x00:      /* SCNTL0 */
			return scntl0;
		case 0x01:      /* SCNTL1 */
			return scntl1;
		case 0x02:      /* SCNTL2 */
			return scntl2;
		case 0x03:      /* SCNTL3 */
			return scntl3;
		case 0x04:      /* SCID */
			return scid;
		case 0x05:      /* SXFER */
			return sxfer;
		case 0x09:      /* SOCL */
			return socl;
		case 0x0c:      /* DSTAT */
			istat &= ~1;
			return dstat;
		case 0x0d:      /* SSTAT0 */
			return sstat0;
		case 0x0e:      /* SSTAT1 */
			return sstat1;
		case 0x0f:      /* SSTAT2 */
			return sstat2;
		case 0x10:      /* DSA [7-0] */
			return dsa & 0xff;
		case 0x11:      /* DSA [15-8] */
			return (dsa >> 8) & 0xff;
		case 0x12:      /* DSA [23-16] */
			return (dsa >> 16) & 0xff;
		case 0x13:      /* DSA [31-24] */
			return (dsa >> 24) & 0xff;
		case 0x14:      /* ISTAT */
			// clear the interrupt on service
			if (!m_irq_cb.isnull())
				m_irq_cb(0);

			return istat;
		case 0x2c:      /* DSP [7-0] */
			return dsp & 0xff;
		case 0x2d:      /* DSP [15-8] */
			return (dsp >> 8) & 0xff;
		case 0x2e:      /* DSP [23-16] */
			return (dsp >> 16) & 0xff;
		case 0x2f:      /* DSP [31-24] */
			return (dsp >> 24) & 0xff;
		case 0x34:      /* SCRATCH A */
		case 0x35:
		case 0x36:
		case 0x37:
			return scratch_a[offset % 4];
		case 0x39:      /* DIEN */
			return dien;
		case 0x3b:      /* DCNTL */
			return dcntl;
		case 0x40:      /* SIEN0 */
			return sien0;
		case 0x41:      /* SIEN1 */
			return sien1;
		case 0x48:      /* STIME0 */
			return stime0;
		case 0x4a:      /* RESPID */
			return respid;
		case 0x4d:      /* STEST1 */
			return stest1;
		case 0x5c:      /* SCRATCH B */
		case 0x5d:
		case 0x5e:
		case 0x5f:
			return scratch_b[offset % 4];

		default:
			fatalerror("LSI53C810: reg_r: Unknown reg %02X\n", offset);
	}

	// never executed
	//return 0;
}

void lsi53c810_device::lsi53c810_reg_w(int offset, UINT8 data)
{
//  logerror("53c810: %02x to reg %d:0x%x (PC=%x)\n", data, offset, offset, space.device().safe_pc());
	switch(offset)
	{
		case 0x00:      /* SCNTL0 */
			scntl0 = data;
			break;
		case 0x01:      /* SCNTL1 */
			scntl1 = data;
			break;
		case 0x02:      /* SCNTL2 */
			scntl2 = data;
			break;
		case 0x03:      /* SCNTL3 */
			scntl3 = data;
			break;
		case 0x04:      /* SCID */
			scid = data;
			break;
		case 0x05:      /* SXFER */
			sxfer = data;
			break;
		case 0x09:      /* SOCL */
			socl = data;
			break;
		case 0x0d:      /* SSTAT0 */
			sstat0 = data;
			break;
		case 0x0e:      /* SSTAT1 */
			sstat1 = data;
			break;
		case 0x0f:      /* SSTAT2 */
			sstat2 = data;
			break;
		case 0x10:      /* DSA [7-0] */
			dsa &= 0xffffff00;
			dsa |= data;
			break;
		case 0x11:      /* DSA [15-8] */
			dsa &= 0xffff00ff;
			dsa |= data << 8;
			break;
		case 0x12:      /* DSA [23-16] */
			dsa &= 0xff00ffff;
			dsa |= data << 16;
			break;
		case 0x13:      /* DSA [31-24] */
			dsa &= 0x00ffffff;
			dsa |= data << 24;
			break;
		case 0x14:      /* ISTAT */
			istat = data;
			break;
		case 0x2c:      /* DSP [7-0] */
			dsp &= 0xffffff00;
			dsp |= data;
			break;
		case 0x2d:      /* DSP [15-8] */
			dsp &= 0xffff00ff;
			dsp |= data << 8;
			break;
		case 0x2e:      /* DSP [23-16] */
			dsp &= 0xff00ffff;
			dsp |= data << 16;
			break;
		case 0x2f:      /* DSP [31-24] */
			dsp &= 0x00ffffff;
			dsp |= data << 24;
			halted = 0;
			if((dmode & 0x1) == 0 && !halted) {
				dma_exec();
			}
			break;
		case 0x34:      /* SCRATCH A */
		case 0x35:
		case 0x36:
		case 0x37:
			scratch_a[offset % 4] = data;
			break;
		case 0x38:      /* DMODE */
			dmode = data;
			break;
		case 0x39:      /* DIEN */
			dien = data;
			break;
		case 0x3b:      /* DCNTL */
			dcntl = data;

			if(dcntl & 0x14 && !halted)     /* single-step & start DMA */
			{
				int op;
				dcmd = FETCH();
				op = (dcmd >> 24) & 0xff;
				dma_opcode[op]();

				istat |= 0x3;   /* DMA interrupt pending */
				dstat |= 0x8;   /* SSI (Single Step Interrupt) */
				if (!m_irq_cb.isnull())
					m_irq_cb(1);
			}
			else if(dcntl & 0x04 && !halted)    /* manual start DMA */
			{
				dma_exec();
			}
			break;
		case 0x40:      /* SIEN0 */
			sien0 = data;
			break;
		case 0x41:      /* SIEN1 */
			sien1 = data;
			break;
		case 0x48:      /* STIME0 */
			stime0 = data;
			break;
		case 0x4a:      /* RESPID */
			respid = data;
			break;
		case 0x4d:      /* STEST1 */
			stest1 = data;
			break;
		case 0x5c:      /* SCRATCH B */
		case 0x5d:
		case 0x5e:
		case 0x5f:
			scratch_b[offset % 4] = data;
			break;

		default:
			fatalerror("LSI53C810: reg_w: Unknown reg %02X, %02X\n", offset, data);
	}
}

void lsi53c810_device::add_opcode(UINT8 op, UINT8 mask, opcode_handler_delegate handler)
{
	for (int i = 0; i < 256; i++)
	{
		if ((i & mask) == op)
		{
			dma_opcode[i] = handler;
		}
	}
}

lsi53c810_device::lsi53c810_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: legacy_scsi_host_adapter(mconfig, LSI53C810, "53C810 SCSI", tag, owner, clock, "lsi53c810", __FILE__)
{
}

void lsi53c810_device::device_start()
{
	legacy_scsi_host_adapter::device_start();

	m_irq_cb.bind_relative_to(*owner());
	m_dma_cb.bind_relative_to(*owner());
	m_fetch_cb.bind_relative_to(*owner());

	for (int i = 0; i < 256; i++)
	{
		dma_opcode[i] = opcode_handler_delegate(FUNC(lsi53c810_device::dmaop_invalid), this);
	}

	add_opcode(0x00, 0xc0, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_block_move ), this));
	add_opcode(0x40, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_select ), this));
	add_opcode(0x48, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_wait_disconnect ), this));
	add_opcode(0x50, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_wait_reselect ), this));
	add_opcode(0x58, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_set ), this));
	add_opcode(0x60, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_clear ), this));
	add_opcode(0x68, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_move_from_sfbr ), this));
	add_opcode(0x70, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_move_to_sfbr ), this));
	add_opcode(0x78, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_read_modify_write ), this));
	add_opcode(0x80, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_jump ), this));
	add_opcode(0x88, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_call ), this));
	add_opcode(0x90, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_return ), this));
	add_opcode(0x98, 0xf8, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_interrupt ), this));
	add_opcode(0xc0, 0xfe, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_move_memory ), this));
	add_opcode(0xe0, 0xed, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_store ), this));
	add_opcode(0xe1, 0xed, opcode_handler_delegate(FUNC( lsi53c810_device::dmaop_load ), this));
}

/*************************************
 *
 *  Disassembler
 *
 *************************************/

UINT32 lsi53c810_device::lsi53c810_dasm_fetch(UINT32 pc)
{
	return m_fetch_cb(pc);
}

unsigned lsi53c810_device::lsi53c810_dasm(char *buf, UINT32 pc)
{
	unsigned result = 0;
	const char *op_mnemonic = NULL;
	UINT32 op = lsi53c810_dasm_fetch(pc);
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
		dest = lsi53c810_dasm_fetch(pc + 4);

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

		for (i = 0; i < ARRAY_LENGTH(flags); i++)
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

		dest = lsi53c810_dasm_fetch(pc + 4);

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
				fatalerror("unknown op 0x%08X\n", op);
		}
		result = 8;
	}
	else if ((op & 0xE0000000) == 0x00000000)
	{
		/* MOVE FROM */
		dest = lsi53c810_dasm_fetch(pc + 4);

		buf += sprintf(buf, "MOVE FROM 0x%08X, WHEN %s",
			dest, phases[(op >> 24) & 0x07]);

		result = 8;
	}
	else if ((op & 0xE0000000) == 0x20000000)
	{
		/* MOVE PTR */
		dest = lsi53c810_dasm_fetch(pc + 4);

		buf += sprintf(buf, "MOVE 0x%08X, PTR 0x%08X, WHEN %s",
			(op & 0x00FFFFFF), dest, phases[(op >> 24) & 0x07]);

		result = 8;
	}
	else
	{
		fatalerror("unknown op 0x%08X\n", op);
	}
	return result;
}

const device_type LSI53C810 = &device_creator<lsi53c810_device>;
