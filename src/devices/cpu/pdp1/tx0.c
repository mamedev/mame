// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    TX-0 emulator

    Two variants:
    * initial model 64kWord RAM
    * later model 8kWord RAM

    Raphael Nabet 2004
*/

#include "emu.h"
#include "debugger.h"
#include "tx0.h"

#define LOG 0
#define LOG_EXTRA 0


#define READ_TX0_18BIT(A) ((signed)m_program->read_dword((A)<<2))
#define WRITE_TX0_18BIT(A,V) (m_program->write_dword((A)<<2,(V)))


#define io_handler_rim 3

#define PC      m_pc
#define IR      m_ir
#define MBR     m_mbr
#define MAR     m_mar
#define AC      m_ac
#define LR      m_lr
#define XR      m_xr
#define PF      m_pf

#define ADDRESS_MASK_64KW   0177777
#define ADDRESS_MASK_8KW    0017777

#define INCREMENT_PC_64KW   (PC = (PC+1) & ADDRESS_MASK_64KW)
#define INCREMENT_PC_8KW    (PC = (PC+1) & ADDRESS_MASK_8KW)


const device_type TX0_8KW  = &device_creator<tx0_8kw_device>;
const device_type TX0_64KW = &device_creator<tx0_64kw_device>;


tx0_device::tx0_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int addr_bits, int address_mask, int ir_mask)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 32, addr_bits , -2)
	, m_address_mask(address_mask)
	, m_ir_mask(ir_mask)
	, m_cpy_handler(*this)
	, m_r1l_handler(*this)
	, m_dis_handler(*this)
	, m_r3l_handler(*this)
	, m_prt_handler(*this)
	, m_rsv_handler(*this)
	, m_p6h_handler(*this)
	, m_p7h_handler(*this)
	, m_sel_handler(*this)
	, m_io_reset_callback(*this)
{
	m_is_octal = true;
}

tx0_8kw_device::tx0_8kw_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tx0_device(mconfig, TX0_8KW, "TX-0 8KW", tag, owner, clock, "tx0_8w_cpu", __FILE__, 13, ADDRESS_MASK_8KW, 037)
{
}


tx0_64kw_device::tx0_64kw_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tx0_device(mconfig, TX0_64KW, "TX-0 64KW", tag, owner, clock, "tx0_64kw_cpu", __FILE__, 16, ADDRESS_MASK_64KW, 03)
{
}


int tx0_device::tx0_read(offs_t address)
{
	if ((address >= 16) || (m_gbl_cm_sel) || ((m_cm_sel >> address) & 1))
		/* core memory (CM) */
		return READ_TX0_18BIT(address);
	else if ((m_lr_sel >> address) & 1)
		/* live register (LR) */
		return LR;

	/* toggle switch storage (TSS) */
	return m_tss[address];
}

void tx0_device::tx0_write(offs_t address, int data)
{
	if ((address >= 16) || (m_gbl_cm_sel) || ((m_cm_sel >> address) & 1))
		/* core memory (CM) */
		WRITE_TX0_18BIT(address, data);
	else if ((m_lr_sel >> address) & 1)
		/* live register (LR) */
		LR = data;
	else
		/* toggle switch storage (TSS) */
		/* TSS is read-only */
	{
		/* nothing */
	}
}


void tx0_device::device_start()
{
	m_mbr = 0;
	m_ac = 0;
	m_mar = 0;
	m_lr = 0;
	m_xr = 0;
	m_pf = 0;
	m_tbr = 0;
	m_tac = 0;
	for ( int i = 0; i < 16; i++ )
	{
		m_tss[i] = 0;
	}
	m_cm_sel = 0;
	m_lr_sel = 0;
	m_gbl_cm_sel = 0;
	m_stop_cyc0 = 0;
	m_stop_cyc1 = 0;
	m_cycle = 0;
	m_pc = 0;
	m_ir = 0;
	m_run = 0;
	m_rim = 0;
	m_ioh = 0;
	m_ios = 0;

	// Resolve callbacks
	m_cpy_handler.resolve();
	m_r1l_handler.resolve();
	m_dis_handler.resolve();
	m_r3l_handler.resolve();
	m_prt_handler.resolve();
	m_rsv_handler.resolve();
	m_p6h_handler.resolve();
	m_p7h_handler.resolve();
	m_sel_handler.resolve();
	m_io_reset_callback.resolve();

	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_mbr));
	save_item(NAME(m_ac));
	save_item(NAME(m_mar));
	save_item(NAME(m_pc));
	save_item(NAME(m_ir));
	save_item(NAME(m_lr));
	save_item(NAME(m_xr));
	save_item(NAME(m_pf));
	save_item(NAME(m_tbr));
	save_item(NAME(m_tac));
	save_item(NAME(m_tss));
	save_item(NAME(m_cm_sel));
	save_item(NAME(m_lr_sel));
	save_item(NAME(m_gbl_cm_sel));
	save_item(NAME(m_stop_cyc0));
	save_item(NAME(m_stop_cyc1));
	save_item(NAME(m_run));
	save_item(NAME(m_rim));
	save_item(NAME(m_cycle));
	save_item(NAME(m_ioh));
	save_item(NAME(m_ios));
	save_item(NAME(m_rim_step));

	// Register state for debugger
	state_add( TX0_PC,         "PC",       m_pc         ).mask(m_address_mask).formatstr("0%06O");
	state_add( TX0_IR,         "IR",       m_ir         ).mask(m_ir_mask)     .formatstr("0%02O");
	state_add( TX0_MBR,        "MBR",      m_mbr        ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_MAR,        "MAR",      m_mar        ).mask(m_address_mask).formatstr("0%06O");
	state_add( TX0_AC,         "AC",       m_ac         ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_LR,         "LR",       m_lr         ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_XR,         "XR",       m_xr         ).mask(0037777)       .formatstr("0%05O");
	state_add( TX0_PF,         "PF",       m_pf         ).mask(077)           .formatstr("0%02O");
	state_add( TX0_TBR,        "TBR",      m_tbr        ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TAC,        "TAC",      m_tac        ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS00,      "TSS00",    m_tss[000]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS01,      "TSS01",    m_tss[001]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS02,      "TSS02",    m_tss[002]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS03,      "TSS03",    m_tss[003]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS04,      "TSS04",    m_tss[004]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS05,      "TSS05",    m_tss[005]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS06,      "TSS06",    m_tss[006]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS07,      "TSS07",    m_tss[007]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS10,      "TSS10",    m_tss[010]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS11,      "TSS11",    m_tss[011]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS12,      "TSS12",    m_tss[012]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS13,      "TSS13",    m_tss[013]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS14,      "TSS14",    m_tss[014]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS15,      "TSS15",    m_tss[015]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS16,      "TSS16",    m_tss[016]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_TSS17,      "TSS17",    m_tss[017]   ).mask(0777777)       .formatstr("0%06O");
	state_add( TX0_CM_SEL,     "CMSEL",    m_cm_sel     ).mask(0177777)       .formatstr("0%06O");
	state_add( TX0_LR_SEL,     "LRSEL",    m_lr_sel     ).mask(0177777)       .formatstr("0%06O");
	state_add( TX0_GBL_CM_SEL, "GBLCMSEL", m_gbl_cm_sel ).mask(1)             .formatstr("%1X");
	state_add( TX0_STOP_CYC0,  "STOPCYC0", m_stop_cyc0  ).mask(1)             .formatstr("%1X");
	state_add( TX0_STOP_CYC1,  "STOPCYC1", m_stop_cyc1  ).mask(1)             .formatstr("%1X");
	state_add( TX0_RUN,        "RUN",      m_run        ).mask(1)             .formatstr("%1X");
	state_add( TX0_RIM,        "RIM",      m_rim        ).mask(1)             .formatstr("%1X");
	state_add( TX0_CYCLE,      "CYCLE",    m_cycle      )                     .formatstr("%1X");
	state_add( TX0_IOH,        "IOH",      m_ioh        )                     .formatstr("%1X");
	state_add( TX0_IOS,        "IOS",      m_ios        ).mask(1)             .formatstr("%1X");

	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("0%06O").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_ir).noshow();

	m_icountptr = &m_icount;
}


void tx0_device::device_reset()
{
	/* reset CPU flip-flops */
	pulse_reset();

	m_gbl_cm_sel = 1;   /* HACK */
}


void tx0_device::call_io_handler(int io_handler)
{
	/* data will be transferred to AC */
	switch (io_handler)
	{
		case 0: m_cpy_handler(ASSERT_LINE); break;
		case 1: m_r1l_handler(ASSERT_LINE); break;
		case 2: m_dis_handler(ASSERT_LINE); break;
		case 3: m_r3l_handler(ASSERT_LINE); break;
		case 4: m_prt_handler(ASSERT_LINE); break;
		case 5: m_rsv_handler(ASSERT_LINE); break;
		case 6: m_p6h_handler(ASSERT_LINE); break;
		case 7: m_p7h_handler(ASSERT_LINE); break;
	}
}


/* execute instructions on this CPU until icount expires */
void tx0_64kw_device::execute_run()
{
	do
	{
		debugger_instruction_hook(this, PC);


		if (m_ioh && m_ios)
		{
			m_ioh = 0;
		}


		if ((! m_run) && (! m_rim))
			m_icount = 0;   /* if processor is stopped, just burn cycles */
		else if (m_rim)
		{
			switch (m_rim_step)
			{
			case 0:
				/* read first word as instruction */
				AC = 0;
				call_io_handler(io_handler_rim);
				m_rim_step = 1;
				m_ios = 0;
				break;

			case 1:
				if (! m_ios)
				{   /* transfer incomplete: wait some more */
					m_icount = 0;
				}
				else
				{   /* data transfer complete */
					m_ios = 0;

					MBR = AC;
					IR = MBR >> 16;     /* basic opcode */
					if ((IR == 2) || (IR == 1))     /* trn or add instruction? */
					{
						PC = MBR & ADDRESS_MASK_64KW;
						m_rim = 0;  /* exit read-in mode */
						m_run = (IR == 2) ? 1 : 0;  /* stop if add instruction */
						m_rim_step = 0;
					}
					else if ((IR == 0) || (IR == 3))    /* sto or opr instruction? */
					{
						MAR = MBR & ADDRESS_MASK_64KW;
						m_rim_step = 2;
					}
				}
				break;

			case 2:
				/* read second word as data */
				AC = 0;
				call_io_handler(io_handler_rim);
				m_rim_step = 3;
				m_ios = 0;
				break;

			case 3:
				if (! m_ios)
				{   /* transfer incomplete: wait some more */
					m_icount = 0;
				}
				else
				{   /* data transfer complete */
					m_ios = 0;

					tx0_write(MAR, MBR = AC);

					m_rim_step = 0;
				}
				break;
			}
		}
		else
		{
			if (m_cycle == 0)
			{   /* fetch new instruction */
				MBR = tx0_read(MAR = PC);
				INCREMENT_PC_64KW;
				IR = MBR >> 16;     /* basic opcode */
				MAR = MBR & ADDRESS_MASK_64KW;
			}

			if (! m_ioh)
			{
				if ((m_stop_cyc0 && (m_cycle == 0))
					|| (m_stop_cyc1 && (m_cycle == 1)))
					m_run = 0;

				execute_instruction_64kw();
			}

			m_icount --;
		}
	}
	while (m_icount > 0);
}

/* execute instructions on this CPU until icount expires */
void tx0_8kw_device::execute_run()
{
	do
	{
		debugger_instruction_hook(this, PC);


		if (m_ioh && m_ios)
		{
			m_ioh = 0;
		}


		if ((! m_run) && (! m_rim))
			m_icount = 0;   /* if processor is stopped, just burn cycles */
		else if (m_rim)
		{
			switch (m_rim_step)
			{
			case 0:
				/* read first word as instruction */
				AC = 0;
				call_io_handler(io_handler_rim);
				m_rim_step = 1;
				m_ios = 0;
				break;

			case 1:
				if (! m_ios)
				{   /* transfer incomplete: wait some more */
					m_icount = 0;
				}
				else
				{   /* data transfer complete */
					m_ios = 0;

					MBR = AC;
					IR = MBR >> 13;     /* basic opcode */
					if ((IR == 16) || (IR == 8))    /* trn or add instruction? */
					{
						PC = MBR & ADDRESS_MASK_8KW;
						m_rim = 0;  /* exit read-in mode */
						m_run = (IR == 16) ? 1 : 0; /* stop if add instruction */
						m_rim_step = 0;
					}
					else if ((IR == 0) || (IR == 24))   /* sto or opr instruction? */
					{
						MAR = MBR & ADDRESS_MASK_8KW;
						m_rim_step = 2;
					}
				}
				break;

			case 2:
				/* read second word as data */
				AC = 0;
				call_io_handler(io_handler_rim);
				m_rim_step = 3;
				m_ios = 0;
				break;

			case 3:
				if (! m_ios)
				{   /* transfer incomplete: wait some more */
					m_icount = 0;
				}
				else
				{   /* data transfer complete */
					m_ios = 0;

					tx0_write(MAR, MBR = AC);

					m_rim_step = 0;
				}
				break;
			}
		}
		else
		{
			if (m_cycle == 0)
			{   /* fetch new instruction */
				MBR = tx0_read(MAR = PC);
				INCREMENT_PC_8KW;
				IR = MBR >> 13;     /* basic opcode */
				MAR = MBR & ADDRESS_MASK_8KW;
			}

			if (! m_ioh)
			{
				if ((m_stop_cyc0 && (m_cycle == 0))
					|| (m_stop_cyc1 && (m_cycle == 1)))
					m_run = 0;

				execute_instruction_8kw();
			}

			m_icount -= 1;
		}
	}
	while (m_icount > 0);
}


/* execute one instruction */
void tx0_64kw_device::execute_instruction_64kw()
{
	if (! m_cycle)
	{
		m_cycle = 1;    /* most frequent case */
		switch (IR)
		{
		case 0:         /* STOre */
		case 1:         /* ADD */
			break;

		case 2:     /* TRansfer on Negative */
			if (AC & 0400000)
			{
				PC = MAR & ADDRESS_MASK_64KW;
				m_cycle = 0;    /* instruction only takes one cycle if branch
                                    is taken */
			}
			break;

		case 3:     /* OPeRate */
			if (MAR & 0100000)
				/* (0.8) CLL = Clear the left nine digital positions of the AC */
				AC &= 0000777;

			if (MAR & 0040000)
				/* (0.8) CLR = Clear the right nine digital positions of the AC */
				AC &= 0777000;

			if (((MAR & 0030000) >> 12) == 1)
				/* (0.8) IOS In-Out Stop = Stop machine so that an In-Out command
				    (specified by digits 6 7 8 of MAR) may be executed */
				m_ioh = 1;

			if (((MAR & 0007000) >> 9) != 0)
			{
				/* ((MAR & 0007000) >> 9) is device ID */
				/* 7: */
				/* (0.8) P7H = Punch holes 1-6 in flexo tape specified by AC
				    digital positions 2, 5, 8, 11, 14, and 17.  Also punches a 7th
				    hole on tape. */
				/* 6: */
				/* (0.8) P6H = Same as P7H but no seventh hole */
				/* 4: */
				/* (0.8) PNT = Print one flexowriter character specified by AC
				    digits 2, 5, 8, 11, 14, and 17. */
				/* 1: */
				/* (0.8) R1C = Read one line of flexo tape so that tape positions
				    1, 2, 3, 4, 5, and 6 will be put in the AC digital positions 0,
				    3, 6, 9, 12 and 15. */
				/* 3: */
				/* (0.8) R3C = Read one line of flexo tape into AC digits 0, 3, 6,
				    9, 12 and 15.  Then cycle the AC one digital position; read the
				    next line on tape into AC digits 0, 3, 6, 9, 12 and 15, cycle
				    the AC right one digital position and read the third and last
				    line into AC digits 0, 3, 6, 9, 12 and 15.  (This command is
				    equal to a triple CYR-R1C.) */
				/* 2: */
				/* (0.8) DIS = Intensify a point on the scope with x and y
				    coordinates where x is specified by AC digits 0-8 with digit 0
				    being used as the sign and y is specified by AC digits 9-17
				    with digit 9 being used as the sign for y.  The complement
				    system is in effect when the signs are negative. */
				/* (5 is undefined) */
				int index = (MAR & 0007000) >> 9;

				call_io_handler(index);
				m_ioh = 1;
			}
			break;
		}
	}
	else
	{
		m_cycle = 0;    /* always true */
		switch (IR)
		{
		case 0:         /* STOre */
			tx0_write(MAR, (MBR = AC));
			break;

		case 1:         /* ADD */
			MBR = tx0_read(MAR);

			AC = AC + MBR;
			AC = (AC + (AC >> 18)) & 0777777;   /* propagate carry around */

			if (AC == 0777777)      /* check for -0 */
				AC = 0;
			break;

		case 2:     /* TRansfer on Negative */
			break;

		case 3:     /* OPeRate */
			if ((MAR & 0000104) == 0000100)
				/* (1.1) PEN = Read the light pen flip-flops 1 and 2 into AC(0) and
				    AC(1). */
				/*...*/{ }

			if ((MAR & 0000104) == 0000004)
				/* (1.1) TAC = Insert a one in each digital position of the AC
				    wherever there is a one in the corresponding digital position
				    of the TAC. */
				/*...*/ { }

			if (MAR & 0000040)
				/* (1.2) COM = Complement every digit in the accumulator */
				AC ^= 0777777;

			if ((MAR & 0000003) == 1)
				/* (1.2) AMB = Store the contents of the AC in the MBR. */
				MBR = AC;

			if ((MAR & 0000003) == 3)
				/* (1.2) TBR = Store the contents of the TBR in the MBR. */
				/*...*/ { }

			if ((MAR & 0000003) == 2)
				/* (1.3) LMB = Store the contents of the LR in the MBR. */
				MBR = LR;
			break;

			if (((MAR & 0000600) >> 7) == 1)
				/* (1.3) MLR = Store the contents of the MBR (memory buffer
				    register) in the live reg. */
				LR = MBR;

			if (((MAR & 0000600) >> 7) == 2)
				/* (1.4) SHR = Shift the AC right one place, i.e. multiply the AC
				    by 2^-1 */
				AC >>= 1;

			if (((MAR & 0000600) >> 7) == 3)
				/* (1.4) CYR = Cycle the AC right one digital position (AC(17) will
				    become AC(0)) */
				AC = (AC >> 1) | ((AC & 1) << 17);

			if (MAR & 0000020)
				/* (1.4) PAD = Partial add AC to MBR, that is, for every digital
				    position of the MBR that contains a one, complement the digit
				    in the corresponding digital position of the AC.  This is also
				    called a half add. */
				AC ^= MBR;

			if (MAR & 0000010)
			{   /* (1.7) CRY = Partial add the 18 digits of the AC to the
                    corresponding 18 digits of the carry.

                    To determine what the 18 digits of the carry are, use the
                    following rule:

                    "Grouping the AC and MBR digits into pairs and proceeding from
                    right to left, assign the carry digit of the next pair to a one
                    if in the present pair MBR = 1 and AC = 0 or if in the present
                    pair AC = 1 and carry 1.

                    (Note: the 0th digit pair determines the 17th pair's carry
                    digit)" */
				AC ^= MBR;

				AC = AC + MBR;
				AC = (AC + (AC >> 18)) & 0777777;   /* propagate carry around */

				if (AC == 0777777)      /* check for -0 */
					AC = 0;
			}

			if (((MAR & 0030000) >> 12) == 3)
				/* (1.8) Hlt = Halt the computer */
				m_run = 0;

			break;
		}
	}
}

void tx0_device::indexed_address_eval()
{
	MAR = MAR + XR;
	MAR = (MAR + (MAR >> 14)) & 0037777;    /* propagate carry around */
	//if (MAR == 0037777)       /* check for -0 */
	//  MAR = 0;
	if (MAR & 0020000)          /* fix negative (right???) */
		MAR = (MAR + 1) & 0017777;
}

/* execute one instruction */
void tx0_8kw_device::execute_instruction_8kw()
{
	if (! m_cycle)
	{
		m_cycle = 1;    /* most frequent case */
		switch (IR)
		{
		case 0:     /* STOre */
		case 1:     /* STore indeXed */
		case 2:     /* Store indeX in Address */
		case 3:     /* ADd One */
		case 4:     /* Store LR */
		case 5:     /* Store Lr indeXed */
		case 6:     /* STore Zero */
		case 8:     /* ADD */
		case 9:     /* ADd indeXed */
		case 10:    /* LoaD indeX */
		case 11:    /* AUgment indeX */
		case 12:    /* Load LR */
		case 13:    /* Load Lr indeXed */
		case 14:    /* LoaD Ac */
		case 15:    /* Load Ac indeXed */
			break;

		case 16:    /* TRansfer on Negative */
			if (AC & 0400000)
			{
				PC = MAR & 0017777;
				m_cycle = 0;    /* instruction only takes one cycle if branch
                                    is taken */
			}
			break;

		case 17:    /* Transfer on ZEro */
			if ((AC == 0000000) || (AC == 0777777))
			{
				PC = MAR & 0017777;
				m_cycle = 0;    /* instruction only takes one cycle if branch
                                    is taken */
			}
			break;

		case 18:    /* Transfer and Set indeX */
			XR = PC;
			PC = MAR & 0017777;
			m_cycle = 0;    /* instruction only takes one cycle if branch
                                is taken */
			break;

		case 19:    /* Transfer and IndeX */
			if ((XR != 0000000) && (XR != 0037777))
			{
				if (XR & 0020000)
					XR ++;
				else
					XR--;
				PC = MAR & 0017777;
				m_cycle = 0;    /* instruction only takes one cycle if branch
                                    is taken */
			}
			break;

		case 21:    /* TRansfer indeXed */
			indexed_address_eval();
		case 20:    /* TRAnsfer */
			PC = MAR & 0017777;
			m_cycle = 0;    /* instruction only takes one cycle if branch
                                is taken */
			break;

		case 22:    /* Transfer on external LeVel */
			/*if (...)
			{
			    PC = MAR & 0017777;
			    m_cycle = 0;*/  /* instruction only takes one cycle if branch
                                    is taken */
			/*}*/
			break;

		case 24:    /* OPeRate */
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			if (((IR & 001) == 00) && ((MAR & 017000) == 004000))
			{   /* Select class instruction */
				if (IR & 004)
					/* (0.8???) CLA = CLear Ac */
					AC = 0;

				/* (IOS???) SEL = SELect */
				m_sel_handler(ASSERT_LINE);
			}
			else
			{   /* Normal operate class instruction */
				if (((IR & 001) == 01) && ((MAR & 017000) == 011000))
					/* (0.6) CLL = CLear Left 9 bits of ac */
					AC &= 0000777;

				if (((IR & 001) == 01) && ((MAR & 017000) == 012000))
					/* (0.6) CLR = CLear Right 9 bits of ac */
					AC &= 0777000;

				if (IR & 002)
					/* (0.7) AMB = transfer Ac to MBr */
					MBR = AC;

				if (IR & 004)
					/* (0.8) CLA = CLear Ac */
					AC = 0;

				if (((IR & 001) == 01) && ((MAR & 010000) == 000000))
				{   /* (IOS) In-Out group commands */
					/* ((MAR & 0007000) >> 9) is device ID */
					/* 0: */
					/* (***) CPY = CoPY synchronizes transmission of information
					    between in-out equipment and computer. */
					/* 1: */
					/* (IOS) R1L = Read 1 Line of tape from PETR into AC bits 0, 3,
					    6, 9, 12, 15, with CYR before read (inclusive or) */
					/* 3: */
					/* (IOS) R3L = Read 3 Lines of tape from PETR into AC bits 0,
					    3, 6, 9, 12, 15, with CYR before each read (inclusive or) */
					/* 2: */
					/* (IOS) DIS = DISplay a point on scope (AC bits 0-8 specify x
					    coordinate, AC bits 9-17 specify y coordinate). The
					    coordinate (0, 0) is usually at the lower left hand corner
					    of the scope.  A console switch is available to relocate
					    (0,0) to the center. */
					/* 6: */
					/* (IOS) P6H = Punch one 6-bit line of flexo tape (without 7th
					    hole) from ac bits 2, 5, 8, 11, 14, 17.  Note: lines
					    without 7th hole are ignored by PETR. */
					/* 7: */
						/* (IOS) P7H = same as P6H, but with 7th hole */
					/* 4: */
					/* (IOS) PRT = Print one six bit flexo character from AC bits
					    2, 5, 8, 11, 14, 17. */
					/* (5 is undefined) */
					int index = (MAR & 0007000) >> 9;

					call_io_handler(index);
					m_ioh = 1;
				}

				if (((IR & 001) == 00) && ((MAR & 010000) == 010000))
				{   /* (IOS) EX0 through EX7 = operate user's EXternal equipment. */
					switch ((MAR & 0007000) >> 9)
					{
					/* ... */
					}
				}
			}
			break;
		}
	}
	else
	{
		if (((IR != 2) && (IR != 3)) || (m_cycle == 2))
			m_cycle = 0;
		else
			m_cycle = 2;    /* SXA and ADO have an extra cycle 2 */
		switch (IR)
		{
		case 1:     /* STore indeXed */
			indexed_address_eval();
		case 0:     /* STOre */
			tx0_write(MAR, (MBR = AC));
			break;

		case 2:     /* Store indeX in Address */
			if (m_cycle)
			{   /* cycle 1 */
				MBR = tx0_read(MAR);
				MBR = (MBR & 0760000) | (XR & 0017777);
			}
			else
			{   /* cycle 2 */
				tx0_write(MAR, MBR);
			}
			break;

		case 3:     /* ADd One */
			if (m_cycle)
			{   /* cycle 1 */
				AC = tx0_read(MAR) + 1;

				#if 0
					AC = (AC + (AC >> 18)) & 0777777;   /* propagate carry around */
					if (AC == 0777777)      /* check for -0 (right???) */
						AC = 0;
				#else
					if (AC >= 0777777)
						AC = (AC + 1) & 0777777;
				#endif
			}
			else
			{   /* cycle 2 */
				tx0_write(MAR, (MBR = AC));
			}
			break;

		case 5:     /* Store Lr indeXed */
			indexed_address_eval();
		case 4:     /* Store LR */
			tx0_write(MAR, (MBR = LR));
			break;

		case 6:     /* STore Zero */
			tx0_write(MAR, (MBR = 0));
			break;

		case 9:     /* ADd indeXed */
			indexed_address_eval();
		case 8:     /* ADD */
			MBR = tx0_read(MAR);

			AC = AC + MBR;
			AC = (AC + (AC >> 18)) & 0777777;   /* propagate carry around */

			if (AC == 0777777)      /* check for -0 */
				AC = 0;
			break;

		case 10:    /* LoaD indeX */
			MBR = tx0_read(MAR);
			XR = (MBR & 0017777) | ((MBR >> 4) & 0020000);
			break;

		case 11:    /* AUgment indeX */
			MBR = tx0_read(MAR);

			XR = XR + ((MBR & 0017777) | ((MBR >> 4) & 0020000));
			XR = (XR + (XR >> 14)) & 0037777;   /* propagate carry around */

			//if (XR == 0037777)        /* check for -0 */
			//  XR = 0;
			break;

		case 13:    /* Load Lr indeXed */
			indexed_address_eval();
		case 12:    /* Load LR */
			LR = MBR = tx0_read(MAR);
			break;

		case 15:    /* Load Ac indeXed */
			indexed_address_eval();
		case 14:    /* LoaD Ac */
			AC = MBR = tx0_read(MAR);
			break;

		case 16:    /* TRansfer on Negative */
		case 17:    /* Transfer on ZEro */
		case 18:    /* Transfer and Set indeX */
		case 19:    /* Transfer and IndeX */
		case 20:    /* TRAnsfer */
		case 21:    /* TRansfer indeXed */
		case 22:    /* Transfer on external LeVel */
			break;

		case 24:    /* OPeRate */
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			if (((IR & 001) == 00) && ((MAR & 017000) == 004000))
			{   /* Select class instruction */
			}
			else
			{   /* Normal operate class instruction */
				if (((IR & 001) == 00) && ((MAR & 017000) == 003000))
				{   /* (1.1) PEN = set ac bit 0 from light PEN ff, and ac bit 1 from
                        light gun ff.  (ffs contain one if pen or gun saw displayed
                        point.)  Then clear both light pen and light gun ffs */
					/*AC = (AC & 0177777) |?...;*/
					/*... = 0;*/
				}

				if (((IR & 001) == 00) && ((MAR & 017000) == 001000))
					/* (1.1) TAC = transfer TAC into ac (inclusive or) */
					AC |= m_tac;

				if (((IR & 001) == 00) && ((MAR & 017000) == 002000))
					/* (1.2) TBR = transfer TBR into mbr (inclusive or) */
					MBR |= m_tbr;

				if (((IR & 001) == 00) && ((MAR & 017000) == 006000))
					/* (1.2) RPF = Read Program Flag register into mbr (inclusive or) */
					MBR |= PF << 8;

				if (MAR & 0000040)
					/* (1.2) COM = COMplement ac */
					AC ^= 0777777;

				if ((! (MAR & 0000400)) && (MAR & 0000100))
				{   /* (1.2) XMB = Transfer XR contents to MBR */
					MBR = XR;
					if (XR & 0020000)
						MBR |= 0740000;
				}

				if (MAR & 0000004)
				{
					switch (MAR & 0000003)
					{
					case 0000003:   /* (1.2) And LR and MBR */
						MBR &= LR;
						break;

					case 0000001:   /* (1.3) Or LR into MBR */
						MBR |= LR;
						break;

					default:
						if (LOG)
							logerror("unrecognized instruction");
						break;
					}
				}

				if (((! (MAR & 0000400)) && (MAR & 0000200)) && ((! (MAR & 0000004)) && (MAR & 0000002)))
				{   /* LMB and MBL used simultaneously interchange LR and MBR */
					int tmp = MBR;
					MBR = LR;
					LR = tmp;
				}
				else if ((! (MAR & 0000400)) && (MAR & 0000200))
					/* (1.4) MBL = Transfer MBR contents to LR */
					LR = MBR;
				else if ((! (MAR & 0000004)) && (MAR & 0000002))
					/* (1.4) LMB = Store the contents of the LR in the MBR. */
					MBR = LR;

				if (MAR & 0000020)
					/* (1.5) PAD = Partial ADd mbr to ac */
					AC ^= MBR;

				if (MAR & 0000400)
				{
					switch (MAR & 0000300)
					{
					case 0000000:   /* (1.6) CYR = CYcle ac contents Right one binary
                                        position (AC(17) -> AC(0)) */
						AC = (AC >> 1) | ((AC & 1) << 17);
						break;

					case 0000200:   /* (1.6) CYcle ac contents Right one binary
                                        position (AC(0) unchanged) */
						AC = (AC >> 1) | (AC & 0400000);
						break;

					default:
						if (LOG)
							logerror("unrecognized instruction");
						break;
					}
				}

				if (((IR & 001) == 00) && ((MAR & 017000) == 007000))
					/* (1.6) SPF = Set Program Flag register from mbr */
					PF = (MBR >> 8) & 077;

				if (MAR & 0000010)
				{   /* (1.7?) CRY = Partial ADd the 18 digits of the AC to the
                        corresponding 18 digits of the carry. */
					AC ^= MBR;

					AC = AC + MBR;
					AC = (AC + (AC >> 18)) & 0777777;   /* propagate carry around */

					if (AC == 0777777)      /* check for -0 */
						AC = 0;
				}

				if ((! (MAR & 0000004)) && (MAR & 0000001))
					/* (1.8) MBX = Transfer MBR contents to XR */
					XR = (MBR & 0017777) | ((MBR >> 4) & 0020000);

				if (((IR & 001) == 01) && ((MAR & 017000) == 010000))
					/* (1.8) HLT = HaLT the computer and sound chime */
					m_run = 0;
			}
			break;

		default:        /* Illegal */
			/* ... */
			break;
		}
	}
}

/*
    Simulate a pulse on reset line:
    reset most registers and flip-flops, and initialize a few emulator state
    variables.
*/
void tx0_device::pulse_reset()
{
	/* processor registers */
	PC = 0;         /* ??? */
	IR = 0;         /* ??? */
	/*MBR = 0;*/    /* ??? */
	/*MAR = 0;*/    /* ??? */
	/*AC = 0;*/     /* ??? */
	/*LR = 0;*/     /* ??? */

	/* processor state flip-flops */
	m_run = 0;      /* ??? */
	m_rim = 0;      /* ??? */
	m_ioh = 0;      /* ??? */
	m_ios = 0;      /* ??? */

	m_rim_step = 0;

	/* now, we kindly ask IO devices to reset, too */
	m_io_reset_callback(ASSERT_LINE);
}

void tx0_device::io_complete()
{
	m_ios = 1;
}


offs_t tx0_8kw_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tx0_8kw );
	return CPU_DISASSEMBLE_NAME(tx0_8kw)(this, buffer, pc, oprom, opram, options);
}


offs_t tx0_64kw_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tx0_64kw );
	return CPU_DISASSEMBLE_NAME(tx0_64kw)(this, buffer, pc, oprom, opram, options);
}
