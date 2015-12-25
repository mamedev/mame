// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Hitachi HD61700 cpu core emulation.
    by Sandro Ronco

    This CPU core is based on documentations works done by:
    - Piotr Piatek ( http://www.pisi.com.pl/piotr433/pb1000he.htm )
    - BLUE ( http://www.geocities.jp/hd61700lab/ )

    The HD61700 has 0x1800 bytes of internal ROM accessed as 16bit with address
    shift by -1 (mapped from 0x0000 to 0x0c00), the external memory is accessed
    with a 8bit data bus, this core emulate all memory as 16bit access with
    address shifted by -1 and in the memory above 0x0c00 only the lower 8bit
    are used.

    TODO:
    - dasm don't decode some mnemonics
    - CPU fast/slow mode
    - probably other minor things

******************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "hd61700.h"

// internal ROM
#define INT_ROM                 0x0c00

// 5 bit registers
#define REG_SX                  m_regsir[0]
#define REG_SY                  m_regsir[1]
#define REG_SZ                  m_regsir[2]
#define READ_SREG(a)            (m_regsir[((a)>>5) & 0x03] & 0x1f)
#define WRITE_SREG(a,d)         (m_regsir[((a)>>5) & 0x03] = (d) & 0x1f)

// 8 bit registers
#define REG_PE                  m_reg8bit[0]
#define REG_PD                  m_reg8bit[1]
#define REG_IB                  m_reg8bit[2]
#define REG_UA                  m_reg8bit[3]
#define REG_IA                  m_reg8bit[4]
#define REG_IE                  m_reg8bit[5]
#define REG_TM                  m_reg8bit[7]
#define READ_REG8(a)            (m_reg8bit[(a) & 0x07])
#define WRITE_REG8(a,d)         (m_reg8bit[(a) & 0x07] = d)

// 16 bit registers
#define REG_IX                  m_reg16bit[0]
#define REG_IY                  m_reg16bit[1]
#define REG_IZ                  m_reg16bit[2]
#define REG_US                  m_reg16bit[3]
#define REG_SS                  m_reg16bit[4]
#define REG_KY                  m_reg16bit[5]

// main registers
#define READ_REG(a)             (m_regmain[(a) & 0x1f])
#define WRITE_REG(a,d)          (m_regmain[(a) & 0x1f] = d)
#define COPY_REG(d,s)           (m_regmain[(d) & 0x1f] = m_regmain[(s) & 0x1f])
#define REG_GET16(r)            (((m_regmain[((r)) & 0x1f]<<0)) | (m_regmain[((r) + 1) & 0x1f]<<8))
#define REG_PUT16(r,d)          do{(m_regmain[(r) & 0x1f] = (((d)>>0) & 0xff)); (m_regmain[((r)+1)&0x1f]=(((d)>>8)&0xff));}while(0)

// opcode
#define GET_REG_IDX(a,b)        (((a<<2) & 0x04) | ((b>>5) & 0x03))
#define RESTORE_REG(o,r,pr)     r = (o&0x02) ? r : pr
#define COND_WRITE_REG(o,a,d)   if (o&0x08) WRITE_REG(a,d)
#define GET_IM3(d)              (((d>>5)&0x07) + 1)

// flags
#define SET_FLAG_C              m_flags |= FLAG_C
#define CLEAR_FLAG_Z            m_flags |= FLAG_Z
#define CLEAR_FLAG_LZ           m_flags |= FLAG_LZ
#define CLEAR_FLAG_UZ           m_flags |= FLAG_UZ
#define CLEAR_FLAGS             m_flags &= ~(FLAG_Z | FLAG_C | FLAG_LZ | FLAG_UZ)

#define CHECK_FLAG_Z(d)         if((d) != 0) CLEAR_FLAG_Z
#define CHECK_FLAG_C(d,l)       if (d > l) SET_FLAG_C
#define CHECK_FLAGB_LZ(d)       if(((d) & 0x0f) != 0) CLEAR_FLAG_LZ
#define CHECK_FLAGB_UZ(d)       if(((d) & 0xf0) != 0) CLEAR_FLAG_UZ
#define CHECK_FLAGW_LZ(d)       if(((d) & 0x0f00) != 0) CLEAR_FLAG_LZ
#define CHECK_FLAGW_UZ(d)       if(((d) & 0xf000) != 0) CLEAR_FLAG_UZ
#define CHECK_FLAGB_UZ_LZ(d)    do{CHECK_FLAGB_LZ(d); CHECK_FLAGB_UZ(d);}while(0)
#define CHECK_FLAGW_UZ_LZ(d)    do{CHECK_FLAGW_LZ(d); CHECK_FLAGW_UZ(d);}while(0)

//CPU state
#define CPU_FAST                0x01
#define CPU_SLP                 0x02

/* HD61700 IRQ vector */
static const UINT16 irq_vector[] = {0x0032, 0x0042, 0x0052, 0x0062, 0x0072};

//**************************************************************************
//  HD61700 DEVICE
//**************************************************************************

const device_type HD61700 = &device_creator<hd61700_cpu_device>;

//-------------------------------------------------
//  hd61700_cpu_device - constructor
//-------------------------------------------------

hd61700_cpu_device::hd61700_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, HD61700, "HD61700", tag, owner, clock, "hd61700", __FILE__),
		m_program_config("program", ENDIANNESS_BIG, 16, 18, -1),
		m_ppc(0x0000),
		m_curpc(0x0000),
		m_pc(0),
		m_flags(0),
		m_lcd_ctrl_cb(*this),
		m_lcd_read_cb(*this),
		m_lcd_write_cb(*this),
		m_kb_read_cb(*this),
		m_kb_write_cb(*this),
		m_port_read_cb(*this),
		m_port_write_cb(*this)
{
	// ...
}


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void hd61700_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	m_sec_timer = timer_alloc(SEC_TIMER);
	m_sec_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	m_lcd_ctrl_cb.resolve_safe();
	m_lcd_read_cb.resolve_safe(0xff);
	m_lcd_write_cb.resolve_safe();
	m_kb_read_cb.resolve_safe(0xff);
	m_kb_write_cb.resolve_safe();
	m_port_read_cb.resolve_safe(0xff);
	m_port_write_cb.resolve_safe();

	// save state
	save_item(NAME(m_ppc));
	save_item(NAME(m_curpc));
	save_item(NAME(m_pc));
	save_item(NAME(m_flags));
	save_item(NAME(m_fetch_addr));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_state));
	save_item(NAME(prev_ua));
	save_item(NAME(m_regsir));
	save_item(NAME(m_reg8bit));
	save_item(NAME(m_reg16bit));
	save_item(NAME(m_regmain));
	save_item(NAME(m_lines_status));

	memset(m_regsir, 0, sizeof(m_regsir));
	memset(m_reg8bit, 0, sizeof(m_reg8bit));
	memset(m_reg16bit, 0, sizeof(m_reg16bit));
	memset(m_regmain, 0, sizeof(m_regmain));

	// register state for debugger
	state_add(HD61700_PC, "pc",   m_pc).callimport().callexport().formatstr("%04X");
	state_add(HD61700_F,  "f", m_flags).callimport().callexport().formatstr("%02X");
	state_add(HD61700_SX, "SX", REG_SX).callimport().callexport().formatstr("%02X");
	state_add(HD61700_SY, "SY", REG_SY).callimport().callexport().formatstr("%02X");
	state_add(HD61700_SZ, "SZ", REG_SZ).callimport().callexport().formatstr("%02X");

	state_add(HD61700_PE, "pe", REG_PE).callimport().callexport().formatstr("%02X");
	state_add(HD61700_PD, "pd", REG_PD).callimport().callexport().formatstr("%02X");
	state_add(HD61700_IB, "ib", REG_IB).callimport().callexport().formatstr("%02X");
	state_add(HD61700_UA, "ua", REG_UA).callimport().callexport().formatstr("%02X");
	state_add(HD61700_IA, "ia", REG_IA).callimport().callexport().formatstr("%02X");
	state_add(HD61700_IE, "ie", REG_IE).callimport().callexport().formatstr("%02X");
	state_add(HD61700_TM, "tm", REG_TM).callimport().callexport().formatstr("%02X");

	state_add(HD61700_IX, "ix", REG_IX).callimport().callexport().formatstr("%04X");
	state_add(HD61700_IY, "iy", REG_IY).callimport().callexport().formatstr("%04X");
	state_add(HD61700_IZ, "iz", REG_IZ).callimport().callexport().formatstr("%04X");
	state_add(HD61700_US, "us", REG_US).callimport().callexport().formatstr("%04X");
	state_add(HD61700_SS, "ss", REG_SS).callimport().callexport().formatstr("%04X");
	state_add(HD61700_KY, "ky", REG_KY).callimport().callexport().formatstr("%04X");

	for (int ireg=0; ireg<32; ireg++)
	{
		std::string tmpstr;
		state_add(HD61700_MAINREG + ireg, strformat(tmpstr, "R%d", ireg).c_str(), m_regmain[ireg]).callimport().callexport().formatstr("%02X");
	}

	state_add(STATE_GENPC, "curpc", m_curpc).callimport().callexport().formatstr("%8s").noshow();
	state_add(STATE_GENPCBASE, "curpcbase", m_ppc).callimport().callexport().formatstr("%8s").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_flags).mask(0xff).formatstr("%8s").noshow();

	// set our instruction counter
	m_icountptr = &m_icount;
}


//-------------------------------------------------
//  device_reset - reset up the device
//-------------------------------------------------

void hd61700_cpu_device::device_reset()
{
	m_ppc = 0x0000;
	m_curpc = 0x0000;
	set_pc(0x0000);
	m_flags = FLAG_SW;
	m_state = 0;
	m_irq_status = 0;
	prev_ua = 0;

	memset(m_regsir, 0, sizeof(m_regsir));
	memset(m_reg8bit, 0, sizeof(m_reg8bit));
	memset(m_reg16bit, 0, sizeof(m_reg16bit));
	memset(m_regmain, 0, sizeof(m_regmain));

	for (auto & elem : m_lines_status)
		elem = CLEAR_LINE;
}



//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void hd61700_cpu_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case SEC_TIMER:
			REG_TM++;
			if ((REG_TM&0x3f) == 60)
			{
				REG_TM = (REG_TM & 0xc0) + 0x40;

				if (((REG_IE>>3) & (1<<HD61700_TIMER_INT)))
				{
					REG_IB |= (1<<HD61700_TIMER_INT);

					if (REG_IB & 0x20)
					{
						m_state &= ~CPU_SLP;
						m_flags |= FLAG_APO;
					}
				}
			}
			break;
	}
}



//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void hd61700_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case HD61700_PC:
			set_pc(m_pc);
			break;
	}
}

//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void hd61700_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c",
				m_flags & FLAG_Z   ? '.' : 'Z',
				m_flags & FLAG_C   ? 'C' : '.',
				m_flags & FLAG_LZ  ? '.' : 'L',
				m_flags & FLAG_UZ  ? '.' : 'U',
				m_flags & FLAG_SW  ? 'S' : '.',
				m_flags & FLAG_APO ? 'A' : '.'
			);
			break;
	}
}



//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t hd61700_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( hd61700 );
	return CPU_DISASSEMBLE_NAME(hd61700)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  check_irqs - check if need interrupts
//-------------------------------------------------

bool hd61700_cpu_device::check_irqs(void)
{
	for (int i=4; i>=0; i--)
	{
		if (REG_IB & (1<<i) && !(m_irq_status & (1<<i)))
		{
			m_irq_status |= (1<<i);
			push(REG_SS, (m_pc>>8)&0xff);
			push(REG_SS, (m_pc>>0)&0xff);

			set_pc(irq_vector[i]);
			m_icount -= 12;
			return true;
		}
	}

	return false;
}


//-------------------------------------------------
//  execute - execute for the provided number of
//  cycles
//-------------------------------------------------

void hd61700_cpu_device::execute_run()
{
	do
	{
		debugger_instruction_hook(this, m_curpc);

		// verify that CPU is not in sleep
		if (m_state & CPU_SLP)
		{
			m_icount -= 6;
		}
		else
		{
			UINT8 op;

			check_irqs();

			m_ppc = m_curpc;

			// instruction fetch
			op = read_op();

			// execute the instruction
			switch ( op )
			{
				case 0x00:  //adc
				case 0x01:  //sbc
				case 0x08:  //ad
				case 0x09:  //sb
					{
						UINT8 arg = read_op();
						UINT8 src = READ_REG(get_sir_im8(arg));
						UINT16 res = READ_REG(arg) + ((op&1) ? -src : +src);

						COND_WRITE_REG(op, arg, res & 0xff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x02:  //ld
					{
						UINT8 arg = read_op();
						COPY_REG(arg, get_sir_im8(arg));

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x04:  //anc
				case 0x05:  //nac
				case 0x06:  //orc
				case 0x07:  //xrc
				case 0x0c:  //an
				case 0x0d:  //na
				case 0x0e:  //or
				case 0x0f:  //xr
					{
						UINT8 arg = read_op();

						UINT8 res = make_logic(op, READ_REG(arg),  READ_REG(get_sir_im8(arg)));

						COND_WRITE_REG(op, arg, res);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);

						//na(c) and or(c) always set C flag
						if ((op&3) == 1 || (op&3) == 2)
							SET_FLAG_C;

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x0a:  //adb
				case 0x0b:  //sbb
					{
						UINT8 arg = read_op();
						UINT16 res;

						if (op & 0x01)
							res = make_bcd_sub(READ_REG(arg), READ_REG(get_sir_im8(arg)));
						else
							res = make_bcd_add(READ_REG(arg), READ_REG(get_sir_im8(arg)));

						WRITE_REG(arg, res & 0xff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x10:  //st
					{
						UINT8 arg = read_op();
						UINT8 src = get_sir_im8(arg);
						UINT16 offset = REG_GET16(src);

						mem_writebyte(REG_UA>>4, offset, READ_REG(arg));

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0x11:  //ld
					{
						UINT8 arg = read_op();
						UINT8 src = get_sir_im8(arg);
						UINT16 offset = REG_GET16(src);

						WRITE_REG(arg, mem_readbyte(REG_UA>>4, offset));

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0x12:  //stl
					{
						UINT8 arg = read_op();
						m_lcd_write_cb((offs_t)0, READ_REG(arg));

						check_optional_jr(arg);
						m_icount -= 11;
					}
					break;

				case 0x13:  //ldl
					{
						UINT8 arg = read_op();
						UINT8 res = m_lcd_read_cb(0);

						WRITE_REG(arg, res);

						check_optional_jr(arg);
						m_icount -= 11;
					}
					break;

				case 0x14:  //ppo/pfl
					{
						UINT8 arg = read_op();

						if (arg & 0x40)
						{
							m_flags = (m_flags & 0x0f) | (READ_REG(arg) & 0xf0);
						}
						else
						{
							m_lcd_ctrl_cb((offs_t)0, READ_REG(arg));
						}

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x15:  //psr
					{
						UINT8 arg = read_op();
						WRITE_SREG(arg, READ_REG(arg));

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x16:  //pst
				case 0x17:  //pst
					{
						UINT8 arg = read_op();
						UINT8 src = READ_REG(arg);
						UINT8 idx = GET_REG_IDX(op, arg);

						switch (idx)
						{
							case 0:     //PE
							case 1:     //PD
								WRITE_REG8(idx, src);
								m_port_write_cb((offs_t)0, REG_PD & REG_PE);
								break;
							case 2:     //IB
								REG_IB = (REG_IB & 0x1f) | (src & 0xe0);
								break;
							case 3:     //UA
								WRITE_REG8(idx, src);
								break;
							case 4:     //IA
								m_kb_write_cb((offs_t)0, src);
								WRITE_REG8(idx, src);
								break;
							case 5:     //IE
								REG_IB &= (((src>>3)&0x1f) | 0xe0);
								m_irq_status &= ((src>>3)&0x1f);
								WRITE_REG8(idx, src);
								break;
							case 6:     //TM
							case 7:     //TM
								// read-only
								break;
						}

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x18:
					{
						UINT8 arg = read_op();
						UINT8 op1 = (arg>>5) & 0x03;
						switch (op1)
						{
							case 0x00:  //rod
							case 0x02:  //bid
								{
									UINT8 src = READ_REG(arg);
									UINT8 res = (src>>1)&0x7f;

									if (!(op1&0x02))
										res = res | ((m_flags&FLAG_C) !=0 )<<7;

									WRITE_REG(arg, res);

									CLEAR_FLAGS;
									CHECK_FLAG_Z(res & 0xff);
									CHECK_FLAGB_UZ_LZ(res);
									if (src&0x01) SET_FLAG_C;
								}
								break;

							case 0x01:  //rou
							case 0x03:  //biu
								{
									UINT8 src = READ_REG(arg);
									UINT8 res = (src<<1)&0xfe;

									if (!(op1&0x02))
										res = res | ((m_flags&FLAG_C) !=0 )<<0;

									WRITE_REG(arg, res);

									CLEAR_FLAGS;
									CHECK_FLAG_Z(res & 0xff);
									CHECK_FLAGB_UZ_LZ(res);
									if (src&0x80) SET_FLAG_C;
								}
								break;
						}

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x1a:
					{
						UINT8 arg = read_op();
						UINT8 op1 = (arg>>5) & 0x03;
						switch (op1)
						{
							case 0x00:  //did
							case 0x01:  //diu
								{
									UINT8 res;

									if (op1&0x01)
										res = (READ_REG(arg)<<4)&0xf0;
									else
										res = (READ_REG(arg)>>4)&0x0f;

									WRITE_REG(arg, res);

									CLEAR_FLAGS;
									CHECK_FLAG_Z(res & 0xff);
									CHECK_FLAGB_UZ_LZ(res);
								}
								break;
							case 0x02:  //byd
							case 0x03:  //byu
								{
									WRITE_REG(arg, 0);
									CLEAR_FLAGS;
									break;
								}
						}

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x1b:  //cmp/inv
					{
						UINT8 arg = read_op();
						UINT8 res = ~(READ_REG(arg));
						if (!(arg & 0x40)) res++;

						WRITE_REG(arg, res);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						if (res || (arg & 0x40)) SET_FLAG_C;

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x1c:  //gpo/gfl
					{
						UINT8 arg = read_op();
						UINT8 src = 0xff;

						if (arg&0x40)
						{
							src = m_flags;
						}
						else
						{
							src = m_port_read_cb(0);
							src&=(~REG_PE);
						}

						WRITE_REG(arg, src);

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x1d:  //gsr
					{
						UINT8 arg = read_op();
						WRITE_REG(arg, READ_SREG(arg));
						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x1e:  //gst
				case 0x1f:  //gst
					{
						UINT8 arg = read_op();
						UINT8 idx = GET_REG_IDX(op, arg);
						WRITE_REG(arg, READ_REG8(idx));
						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x20:  //st
				case 0x22:  //sti
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IX;
						REG_IX += get_sign_mreg(arg);

						mem_writebyte(REG_UA>>4, REG_IX++, READ_REG(arg));

						RESTORE_REG(op, REG_IX, prev_ir);

						m_icount -= 8;
					}
					break;

				case 0x21:  //st
				case 0x23:  //sti
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IZ;
						REG_IZ += get_sign_mreg(arg);

						mem_writebyte(REG_UA>>6, REG_IZ++, READ_REG(arg));

						RESTORE_REG(op, REG_IZ, prev_ir);

						m_icount -= 8;
					}
					break;

				case 0x24:  //std
					{
						UINT8 arg = read_op();
						REG_IX += get_sign_mreg(arg);

						mem_writebyte(REG_UA>>4, REG_IX, READ_REG(arg));
						m_icount -= 6;
					}
					break;

				case 0x25:  //std
					{
						UINT8 arg = read_op();
						REG_IZ += get_sign_mreg(arg);

						mem_writebyte(REG_UA>>6, REG_IZ, READ_REG(arg));
						m_icount -= 6;
					}
					break;

				case 0x26:  //phs
					{
						UINT8 arg = read_op();

						push(REG_SS, READ_REG(arg));
						m_icount -= 9;
					}
					break;
				case 0x27:  //phu
					{
						UINT8 arg = read_op();

						push(REG_US, READ_REG(arg));
						m_icount -= 9;
					}
					break;

				case 0x28:  //ld
				case 0x2a:  //ldi
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IX;
						REG_IX += get_sign_mreg(arg);

						WRITE_REG(arg, mem_readbyte(REG_UA>>4, REG_IX++));

						RESTORE_REG(op, REG_IX, prev_ir);

						m_icount -= 8;
					}
					break;

				case 0x29:  //ld
				case 0x2b:  //ldi
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IZ;
						REG_IZ += get_sign_mreg(arg);

						WRITE_REG(arg, mem_readbyte(REG_UA>>6, REG_IZ++));

						RESTORE_REG(op, REG_IZ, prev_ir);

						m_icount -= 8;
					}
					break;

				case 0x2c:  //ldd
					{
						UINT8 arg = read_op();
						REG_IX += get_sign_mreg(arg);

						WRITE_REG(arg, mem_readbyte(REG_UA>>4, REG_IX));

						m_icount -= 6;
					}
					break;

				case 0x2d:  //ldd
					{
						UINT8 arg = read_op();
						REG_IZ += get_sign_mreg(arg);

						WRITE_REG(arg, mem_readbyte(REG_UA>>6, REG_IZ));

						m_icount -= 6;
					}
					break;

				case 0x2e:  //pps
					{
						UINT8 arg = read_op();
						WRITE_REG(arg, pop(REG_SS));

						m_icount -= 11;
					}
					break;
				case 0x2f:  //ppu
					{
						UINT8 arg = read_op();
						WRITE_REG(arg, pop(REG_US));

						m_icount -= 11;
					}
					break;

				case 0x30:  //jp z
				case 0x31:  //jp nc
				case 0x32:  //jp lz
				case 0x33:  //jp uz
				case 0x34:  //jp nz
				case 0x35:  //jp c
				case 0x36:  //jp nlz
				case 0x37:  //unconditional jp
					{
						UINT8 lsb = read_op();
						if (m_pc < INT_ROM) read_op();
						UINT8 msb = read_op();

						if (check_cond(op))
							set_pc((msb<<8) | lsb);
						m_icount -= 3;
					}
					break;

				case 0x38:  //adc
				case 0x3a:  //sbc
				case 0x3c:  //ad
				case 0x3e:  //sb
					{
						UINT8 arg = read_op();
						UINT16 offset = REG_IX + get_sign_mreg(arg);
						UINT8 src = mem_readbyte(REG_UA>>4, offset);

						UINT16 res = src + ((op & 2) ? -READ_REG(arg) : +READ_REG(arg)) ;

						if ((op & 4))
							mem_writebyte(REG_UA>>4, offset, res & 0xff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						m_icount -= 9;
					}
					break;

				case 0x39:  //adc
				case 0x3b:  //sbc
				case 0x3d:  //ad
				case 0x3f:  //sb
					{
						UINT8 arg = read_op();
						UINT16 offset = REG_IZ + get_sign_mreg(arg);
						UINT8 src = mem_readbyte(REG_UA>>6, offset);

						UINT16 res = src + ((op & 2) ? -READ_REG(arg) : +READ_REG(arg)) ;

						if ((op & 4))
							mem_writebyte(REG_UA>>6, offset, res & 0xff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						m_icount -= 9;
					}
					break;

				case 0x40:  //adc
				case 0x41:  //sbc
				case 0x48:  //ad
				case 0x49:  //sb
					{
						UINT8 arg = read_op();
						UINT8 src = read_op();
						UINT16 res = READ_REG(arg) + ((op&1) ? -src : +src);

						COND_WRITE_REG(op, arg, res & 0xff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x42:  //ld
					{
						UINT8 arg = read_op();
						UINT8 src = read_op() ;

						WRITE_REG(arg, src);

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x44:  //anc
				case 0x45:  //nac
				case 0x46:  //orc
				case 0x47:  //xrc
				case 0x4c:  //an
				case 0x4d:  //na
				case 0x4e:  //or
				case 0x4f:  //xr
					{
						UINT8 arg = read_op();
						UINT8 src = read_op();

						UINT8 res = make_logic(op, READ_REG(arg), src);

						COND_WRITE_REG(op, arg, res);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);

						//na(c) and or(c) always set C flag
						if ((op&3) == 1 || (op&3) == 2)
							SET_FLAG_C;

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x4a:  //adb
				case 0x4b:  //sbb
					{
						UINT8 arg = read_op();
						UINT8 src = read_op();
						UINT16 res;

						if (op & 0x01)
							res = make_bcd_sub(READ_REG(arg), src);
						else
							res = make_bcd_add(READ_REG(arg), src);

						WRITE_REG(arg, res & 0xff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						check_optional_jr(arg);
						m_icount -= 3;
					}
					break;

				case 0x50:  //st
					{
						UINT8 arg = read_op();
						UINT8 src = read_op();
						UINT16 offset = REG_GET16(READ_SREG(arg));

						mem_writebyte(REG_UA>>4, offset, src);
						m_icount -= 8;
					}
					break;

				case 0x51:  //ld
					{
						UINT8 arg = read_op();
						UINT8 src = read_op();

						WRITE_REG(arg, src);
						m_icount -= 8;
					}
					break;

				case 0x52:  //stl
					{
						UINT8 arg = read_op();
						m_lcd_write_cb((offs_t)0, arg);

						m_icount -= 12;
					}
					break;

				case 0x54:  //ppo/pfl
					{
						UINT8 arg = read_op();
						UINT8 src = read_op();

						if (arg & 0x40)
						{
							m_flags = (m_flags & 0x0f) | (src & 0xf0);
						}
						else
						{
							m_lcd_ctrl_cb((offs_t)0, src);
						}

						m_icount -= 3;
					}
					break;

				case 0x55:  //psr
					{
						UINT8 arg = read_op();
						WRITE_SREG(arg, arg);

						m_icount -= 3;
					}
					break;

				case 0x56:  //pst
				case 0x57:  //pst
					{
						UINT8 arg = read_op();
						UINT8 src = read_op();
						UINT8 idx = GET_REG_IDX(op, arg);

						switch (idx)
						{
							case 0:     //PE
							case 1:     //PD
								WRITE_REG8(idx, src);
								m_port_write_cb((offs_t)0, REG_PD & REG_PE);
								break;
							case 2:     //IB
								REG_IB = (REG_IB & 0x1f) | (src & 0xe0);
								break;
							case 3:     //UA
								WRITE_REG8(idx, src);
								break;
							case 4:     //IA
								m_kb_write_cb((offs_t)0, src);
								WRITE_REG8(idx, src);
								break;
							case 5:     //IE
								REG_IB &= (((src>>3)&0x1f) | 0xe0);
								m_irq_status &= ((src>>3)&0x1f);
								WRITE_REG8(idx, src);
								break;
							case 6:     //TM
							case 7:     //TM
								// read-only
								break;
						}

						m_icount -= 3;
					}
					break;

				case 0x58:  //bups
				case 0x59:  //bdns
					{
						UINT8 arg = read_op();
						UINT8 tmp;
						UINT16 res;

						for(;;)
						{
							tmp = mem_readbyte(REG_UA>>4, REG_IX);
							mem_writebyte(REG_UA>>6, REG_IZ, tmp);

							res = tmp - arg;
							if (REG_IX == REG_IY || !res)
								break;

							REG_IX += ((op&1) ? -1 : +1);
							REG_IZ += ((op&1) ? -1 : +1);
							m_icount -= 6;
						}

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						m_icount -= 9;
					}
					break;

				case 0x5c:  //sup
				case 0x5d:  //sdn
					{
						UINT8 arg = read_op();
						UINT16 res;

						for(;;)
						{
							res = mem_readbyte(REG_UA>>4, REG_IX) - arg;

							if (REG_IX == REG_IY || !res)
								break;

							REG_IX += ((op&1) ? -1 : +1);
							m_icount -= 6;
						}

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						m_icount -= 9;
					}
					break;

				case 0x60:  //st
				case 0x62:  //sti
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IX;
						REG_IX += get_sign_im8(arg);

						mem_writebyte(REG_UA>>4, REG_IX++, READ_REG(arg));

						RESTORE_REG(op, REG_IX, prev_ir);

						m_icount -= 8;
					}
					break;

				case 0x61:  //st
				case 0x63:  //sti
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IZ;
						REG_IZ += get_sign_im8(arg);

						mem_writebyte(REG_UA>>6, REG_IZ++, READ_REG(arg));

						RESTORE_REG(op, REG_IZ, prev_ir);
						m_icount -= 8;
					}
					break;

				case 0x64:  //std
					{
						UINT8 arg = read_op();
						REG_IX += get_sign_im8(arg);

						mem_writebyte(REG_UA>>4, REG_IX, READ_REG(arg));
						m_icount -= 6;
					}
					break;

				case 0x65:  //std
					{
						UINT8 arg = read_op();
						REG_IZ += get_sign_im8(arg);

						mem_writebyte(REG_UA>>6, REG_IZ, READ_REG(arg));
						m_icount -= 6;
					}
					break;

				case 0x68:  //ld
				case 0x6a:  //ldi
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IX;
						REG_IX += get_sign_im8(arg);

						WRITE_REG(arg, mem_readbyte(REG_UA>>4, REG_IX++));

						RESTORE_REG(op, REG_IX, prev_ir);
						m_icount -= 8;
					}
					break;

				case 0x69:  //ld
				case 0x6b:  //ldi
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IZ;
						REG_IZ += get_sign_im8(arg);

						WRITE_REG(arg, mem_readbyte(REG_UA>>6, REG_IZ++));

						RESTORE_REG(op, REG_IZ, prev_ir);
						m_icount -= 8;
					}
					break;

				case 0x6c:  //ldd
					{
						UINT8 arg = read_op();
						REG_IX += get_sign_im8(arg);

						WRITE_REG(arg, mem_readbyte(REG_UA>>4, REG_IX));
						m_icount -= 6;
					}
					break;

				case 0x6d:  //ldd
					{
						UINT8 arg = read_op();
						REG_IZ += get_sign_im8(arg);

						WRITE_REG(arg, mem_readbyte(REG_UA>>6, REG_IZ));
						m_icount -= 6;
					}
					break;

				case 0x70:  //cal z
				case 0x71:  //cal nc
				case 0x72:  //cal lz
				case 0x73:  //cal uz
				case 0x74:  //cal nz
				case 0x75:  //cal c
				case 0x76:  //cal nlz
				case 0x77:  //unconditional cal
					{
						UINT8 lsb = read_op();
						if (m_pc < INT_ROM) read_op();
						UINT8 msb = read_op();

						if (check_cond(op))
						{
							m_pc--;
							push(REG_SS, (m_pc>>8)&0xff);
							push(REG_SS, (m_pc>>0)&0xff);

							set_pc((msb<<8) | lsb);
							m_icount -= 6;
						}
						m_icount -= 3;
					}
					break;

				case 0x78:  //adc
				case 0x7a:  //sbc
				case 0x7c:  //ad
				case 0x7e:  //sb
					{
						UINT8 arg = read_op();
						UINT16 offset = REG_IX + get_sign_im8(arg);
						UINT8 src = mem_readbyte(REG_UA>>4, offset);

						UINT16 res = src + ((op & 2) ? -READ_REG(arg) : +READ_REG(arg)) ;

						if ((op & 4))
							mem_writebyte(REG_UA>>4, offset, res & 0xff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						m_icount -= 9;
					}
					break;

				case 0x79:  //adc
				case 0x7b:  //sbc
				case 0x7d:  //ad
				case 0x7f:  //sb
					{
						UINT8 arg = read_op();
						UINT16 offset = REG_IZ + get_sign_im8(arg);
						UINT8 src = mem_readbyte(REG_UA>>6, offset);

						UINT16 res = src + ((op & 2) ? -READ_REG(arg) : +READ_REG(arg)) ;

						if ((op & 4))
							mem_writebyte(REG_UA>>6, offset, res & 0xff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						m_icount -= 9;
					}
					break;

				case 0x80:  //adcw
				case 0x81:  //sbcw
				case 0x88:  //adw
				case 0x89:  //sbw
					{
						UINT8 arg = read_op();
						UINT8 src = get_sir_im8(arg);
						UINT32 res = REG_GET16(arg) + ((op & 0x01) ? -REG_GET16(src) : +REG_GET16(src));

						if (op & 0x08)  REG_PUT16(arg, res&0xffff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xffff);
						CHECK_FLAGW_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xffff);

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0x82:  //ldw
					{
						UINT8 arg = read_op();
						UINT8 src = get_sir_im8(arg);

						COPY_REG(arg, src);
						COPY_REG(arg+1, src+1);

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0x84:  //ancw
				case 0x85:  //nacw
				case 0x86:  //orcw
				case 0x87:  //xrcw
				case 0x8c:  //anw
				case 0x8d:  //naw
				case 0x8e:  //orw
				case 0x8f:  //xrw
					{
						UINT8 arg = read_op();
						UINT8 src = get_sir_im8(arg);

						UINT8 res0 = make_logic(op, READ_REG(arg), READ_REG(src));
						COND_WRITE_REG(op, arg, res0);

						UINT8 res1 = make_logic(op, READ_REG(arg+1), READ_REG(src+1));
						COND_WRITE_REG(op, arg+1, res1);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res0 || res1);
						CHECK_FLAGB_UZ_LZ(res1);

						//na(c) and or(c) always set C flag
						if ((op&3) == 1 || (op&3) == 2)
							SET_FLAG_C;

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0x8a:  //adbw
				case 0x8b:  //sbbw
					{
						UINT8 arg = read_op();
						UINT8 src = get_sir_im8(arg);
						UINT16 res0, res1;

						if (op & 0x01)
							res0 = make_bcd_sub(READ_REG(arg), READ_REG(src));
						else
							res0 = make_bcd_add(READ_REG(arg), READ_REG(src));

						WRITE_REG(arg, res0 & 0xff);

						res1 = (res0>0xff) ? 1 : 0 ;

						if (op & 0x01)
							res1 = make_bcd_sub(READ_REG(arg+1), READ_REG(src+1) + res1);
						else
							res1 = make_bcd_add(READ_REG(arg+1), READ_REG(src+1) + res1);

						WRITE_REG(arg+1, res1 & 0xff);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res0 || res1);
						CHECK_FLAGB_UZ_LZ(res1);
						CHECK_FLAG_C(res1, 0xff);

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0x90:  //stw
					{
						UINT8 arg = read_op();
						UINT8 src = get_sir_im8(arg);
						UINT16 offset = REG_GET16(src);

						mem_writebyte(REG_UA>>4, offset+0, READ_REG(arg+0));
						mem_writebyte(REG_UA>>4, offset+1, READ_REG(arg+1));

						check_optional_jr(arg);
						m_icount -= 11;
					}
					break;

				case 0x91:  //ldw
					{
						UINT8 arg = read_op();
						UINT8 src = get_sir_im8(arg);
						UINT16 offset = REG_GET16(src);

						WRITE_REG(arg+0, mem_readbyte(REG_UA>>4, offset+0));
						WRITE_REG(arg+1, mem_readbyte(REG_UA>>4, offset+1));

						check_optional_jr(arg);
						m_icount -= 11;
					}
					break;

				case 0x92:  //stlw
					{
						UINT8 arg = read_op();

						m_lcd_write_cb((offs_t)0, READ_REG(arg));
						m_lcd_write_cb((offs_t)0, READ_REG(arg+1));

						check_optional_jr(arg);
						m_icount -= 19;
					}
					break;

				case 0x93:  //ldcw
					{
						UINT8 arg = read_op();
						UINT8 reg0, reg1;

						reg0 = m_lcd_read_cb(0);
						reg1 = m_lcd_read_cb(0);

						WRITE_REG(arg+0, reg0);
						WRITE_REG(arg+1, reg1);

						check_optional_jr(arg);
						m_icount -= 19;
					}
					break;

				case 0x96:  //pre
				case 0x97:  //pre
					{
						UINT8 arg = read_op();
						UINT8 idx = GET_REG_IDX(op, arg);

						if (idx < 5)
							m_reg16bit[idx] = REG_GET16(arg);

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0x98:  //rodw
					{
						UINT8 arg = read_op();
						UINT8 op1 = (arg>>5) & 0x03;
						switch (op1)
						{
							case 0x00:  //rodw
							case 0x02:  //bidw
								{
									UINT16 src = REG_GET16(arg-1);
									UINT16 res = (src>>1)&0x7fff;

									if (!(op1&0x02))
										res = res | ((m_flags&FLAG_C) !=0 )<<15;

									REG_PUT16(arg-1, res);

									CLEAR_FLAGS;
									CHECK_FLAG_Z(res & 0xffff);
									CHECK_FLAGB_UZ_LZ(res);
									if (src&0x01) SET_FLAG_C;
								}
								break;

							case 0x01:  //rouw
							case 0x03:  //biuw
								{
									UINT16 src = REG_GET16(arg);
									UINT16 res = (src<<1)&0xfffe;

									if (!(op1&0x02))
										res = res | ((m_flags&FLAG_C) !=0 )<<0;

									REG_PUT16(arg, res);

									CLEAR_FLAGS;
									CHECK_FLAG_Z(res & 0xffff);
									CHECK_FLAGW_UZ_LZ(res);
									if (src&0x8000) SET_FLAG_C;
								}
								break;
						}

						check_optional_jr(arg);
						m_icount -= 11;
					}
					break;

				case 0x9a:
					{
						UINT8 arg = read_op();
						UINT8 op1 = (arg>>5) & 0x03;
						switch (op1)
						{
							case 0x00:  //didw
								{
									UINT16 src = (REG_GET16(arg-1)>>4)&0x0fff;
									REG_PUT16(arg-1, src);

									CLEAR_FLAGS;
									CHECK_FLAG_Z(src & 0xffff);
									CHECK_FLAGB_UZ_LZ(src);
								}
								break;

							case 0x01:  //diuw
								{
									UINT16 src = (REG_GET16(arg)<<4)&0xfff0;
									REG_PUT16(arg, src);

									CLEAR_FLAGS;
									CHECK_FLAG_Z(src & 0xffff);
									CHECK_FLAGW_UZ_LZ(src);
								}
								break;

							case 0x02:  //bydw
								{
									UINT8 src = READ_REG(arg);

									WRITE_REG(arg, 0);
									WRITE_REG(arg-1, src);

									CLEAR_FLAGS;
									CHECK_FLAG_Z(src & 0xff);
									CHECK_FLAGB_UZ_LZ(src);
								}
								break;

							case 0x03:  //byuw
								{
									UINT8 src = READ_REG(arg);

									WRITE_REG(arg, 0);
									WRITE_REG(arg+1, src);

									CLEAR_FLAGS;
									CHECK_FLAG_Z(src & 0xff);
									CHECK_FLAGB_UZ_LZ(src);
								}
								break;
						}

						check_optional_jr(arg);
						m_icount -= 11;
					}
					break;

				case 0x9b:  //cmpw/invw
					{
						UINT8 arg = read_op();
						UINT16 res = ~(REG_GET16(arg));
						if (!(arg & 0x40)) res++;

						REG_PUT16(arg, res);

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xffff);
						CHECK_FLAGW_UZ_LZ(res);

						if (res || (arg & 0x40)) SET_FLAG_C;

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0x9c:  //gpow/gflw
					{
						UINT8 arg = read_op();
						UINT8 reg0, reg1;

						if (arg&0x40)
						{
							reg0 = reg1 = m_flags;
						}
						else
						{
							reg0 = m_port_read_cb(0);
							reg1 = m_port_read_cb(0);

							reg0&=(~REG_PE);
							reg1&=(~REG_PE);

						}

						WRITE_REG(arg+0, reg0);
						WRITE_REG(arg+1, reg1);

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0x9e:  //gre
				case 0x9f:  //gre
					{
						UINT8 arg = read_op();
						UINT8 idx = GET_REG_IDX(op, arg);
						UINT16 src;

						if (idx >= 5)
						{
							UINT16 port = m_kb_read_cb(0);
							src = (REG_KY & 0x0f00) | (port & 0xf0ff);
						}
						else
							src = m_reg16bit[idx];

						REG_PUT16(arg, src);

						check_optional_jr(arg);
						m_icount -= 8;
					}
					break;

				case 0xa0:  //stw
				case 0xa2:  //stiw
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IX;
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						REG_IX += ((arg & 0x80) ? -ir_inc : +ir_inc);

						mem_writebyte(REG_UA>>4, REG_IX++, READ_REG(arg+0));
						mem_writebyte(REG_UA>>4, REG_IX++, READ_REG(arg+1));

						RESTORE_REG(op, REG_IX, prev_ir);
						m_icount -= 11;
					}
					break;

				case 0xa1:  //stw
				case 0xa3:  //stiw
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IZ;
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						REG_IZ += ((arg & 0x80) ? -ir_inc : +ir_inc);

						mem_writebyte(REG_UA>>6, REG_IZ++, READ_REG(arg+0));
						mem_writebyte(REG_UA>>6, REG_IZ++, READ_REG(arg+1));

						RESTORE_REG(op, REG_IZ, prev_ir);
						m_icount -= 11;
					}
					break;

				case 0xa4:  //stdw
					{
						UINT8 arg = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						REG_IX += ((arg & 0x80) ? -ir_inc : +ir_inc);

						mem_writebyte(REG_UA>>4, REG_IX--, READ_REG(arg-0));
						mem_writebyte(REG_UA>>4, REG_IX, READ_REG(arg-1));

						m_icount -= 9;
					}
					break;
				case 0xa5:  //stdw
					{
						UINT8 arg = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						REG_IZ += ((arg & 0x80) ? -ir_inc : +ir_inc);

						mem_writebyte(REG_UA>>6, REG_IZ--, READ_REG(arg-0));
						mem_writebyte(REG_UA>>6, REG_IZ, READ_REG(arg-1));

						m_icount -= 9;
					}
					break;

				case 0xa6:  //phsw
					{
						UINT8 arg = read_op();
						push(REG_SS, READ_REG(arg));
						push(REG_SS, READ_REG(arg-1));

						m_icount -= 12;
					}
					break;
				case 0xa7:  //phuw
					{
						UINT8 arg = read_op();
						push(REG_US, READ_REG(arg));
						push(REG_US, READ_REG(arg-1));

						m_icount -= 12;
					}
					break;

				case 0xa8:  //ldw
				case 0xaa:  //ldiw
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IX;
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						REG_IX += ((arg & 0x80) ? -ir_inc : +ir_inc);

						WRITE_REG(arg+0, mem_readbyte(REG_UA>>4, REG_IX++));
						WRITE_REG(arg+1, mem_readbyte(REG_UA>>4, REG_IX++));

						RESTORE_REG(op, REG_IX, prev_ir);

						m_icount -= 11;
					}
					break;

				case 0xa9:  //ldw
				case 0xab:  //ldiw
					{
						UINT8 arg = read_op();
						UINT16 prev_ir = REG_IZ;
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						REG_IZ += ((arg & 0x80) ? -ir_inc : +ir_inc);

						WRITE_REG(arg+0, mem_readbyte(REG_UA>>6, REG_IZ++));
						WRITE_REG(arg+1, mem_readbyte(REG_UA>>6, REG_IZ++));

						RESTORE_REG(op, REG_IZ, prev_ir);

						m_icount -= 11;
					}
					break;

				case 0xac:  //lddw
					{
						UINT8 arg = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						REG_IX += ((arg & 0x80) ? -ir_inc : +ir_inc);

						WRITE_REG(arg-0, mem_readbyte(REG_UA>>4, REG_IX--));
						WRITE_REG(arg-1, mem_readbyte(REG_UA>>4, REG_IX));

						m_icount -= 9;
					}
					break;

				case 0xad:  //lddw
					{
						UINT8 arg = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						REG_IZ += ((arg & 0x80) ? -ir_inc : +ir_inc);

						WRITE_REG(arg-0, mem_readbyte(REG_UA>>6, REG_IZ--));
						WRITE_REG(arg-1, mem_readbyte(REG_UA>>6, REG_IZ));

						m_icount -= 9;
					}
					break;

				case 0xae:  //ppsw
				case 0xaf:  //ppuw
					{
						UINT8 arg = read_op();

						WRITE_REG(arg, pop((op&1) ? REG_US : REG_SS));
						WRITE_REG(arg+1, pop((op&1) ? REG_US : REG_SS));

						m_icount -= 14;
					}
					break;

				case 0xb0:  //jr z
				case 0xb1:  //jr nc
				case 0xb2:  //jr lz
				case 0xb3:  //jr uz
				case 0xb4:  //jr nz
				case 0xb5:  //jr c
				case 0xb6:  //jr nlz
				case 0xb7:  //unconditional jr
					{
						UINT8 arg = read_op();
						UINT32 new_pc = m_pc-1 + get_im_7(arg);

						if (check_cond(op))
							set_pc(new_pc);
						m_icount -= 3;
					}
					break;

				case 0xb8:  //adcw
				case 0xbc:  //adw
					{
						UINT8 arg = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						UINT16 offset = REG_IX + ((arg & 0x80) ? -ir_inc : +ir_inc);

						UINT16 src0 = mem_readbyte(REG_UA>>4, offset) + READ_REG(arg);
						UINT16 src1 = mem_readbyte(REG_UA>>4, offset+1) + READ_REG(arg+1) + ((src0>0xff) ? 1 : 0);

						if (op&0x04)
						{
							mem_writebyte(REG_UA>>4, offset+0, src0 & 0xff);
							mem_writebyte(REG_UA>>4, offset+1, src1 & 0xff);
						}

						CLEAR_FLAGS;
						CHECK_FLAGB_UZ_LZ(src1);
						CHECK_FLAG_Z(src0 || src1);
						CHECK_FLAG_C(src1, 0xff);

						m_icount -= 15;
					}
					break;

				case 0xba:  //sbcw
				case 0xbe:  //sbw
					{
						UINT8 arg = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						UINT16 offset = REG_IX + ((arg & 0x80) ? -ir_inc : +ir_inc);

						UINT16 src0 = mem_readbyte(REG_UA>>4, offset) - READ_REG(arg);
						UINT16 src1 = mem_readbyte(REG_UA>>4, offset+1) - READ_REG(arg+1) - ((src0>0xff) ? 1 : 0);

						if (op&0x04)
						{
							mem_writebyte(REG_UA>>4, offset+0, src0 & 0xff);
							mem_writebyte(REG_UA>>4, offset+1, src1 & 0xff);
						}

						CLEAR_FLAGS;
						CHECK_FLAGB_UZ_LZ(src1);
						CHECK_FLAG_Z(src0 || src1);
						CHECK_FLAG_C(src1, 0xff);

						m_icount -= 15;
					}
					break;

				case 0xb9:  //adcw
				case 0xbd:  //adw
					{
						UINT8 arg = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						UINT16 offset = REG_IZ + ((arg & 0x80) ? -ir_inc : +ir_inc);

						UINT16 src0 = mem_readbyte(REG_UA>>6, offset) + READ_REG(arg);
						UINT16 src1 = mem_readbyte(REG_UA>>6, offset+1) + READ_REG(arg+1) + ((src0>0xff) ? 1 : 0);

						if (op&0x04)
						{
							mem_writebyte(REG_UA>>6, offset+0, src0 & 0xff);
							mem_writebyte(REG_UA>>6, offset+1, src1 & 0xff);
						}

						CLEAR_FLAGS;
						CHECK_FLAGB_UZ_LZ(src1);
						CHECK_FLAG_Z(src0 || src1);
						CHECK_FLAG_C(src1, 0xff);

						m_icount -= 15;
					}
					break;

				case 0xbb:  //sbcw
				case 0xbf:  //sbw
					{
						UINT8 arg = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg));

						UINT16 offset = REG_IZ + ((arg & 0x80) ? -ir_inc : +ir_inc);

						UINT16 src0 = mem_readbyte(REG_UA>>6, offset) - READ_REG(arg);
						UINT16 src1 = mem_readbyte(REG_UA>>6, offset+1) - READ_REG(arg+1) - ((src0>0xff) ? 1 : 0);

						if (op&0x04)
						{
							mem_writebyte(REG_UA>>6, offset+0, src0 & 0xff);
							mem_writebyte(REG_UA>>6, offset+1, src1 & 0xff);
						}

						CLEAR_FLAGS;
						CHECK_FLAGB_UZ_LZ(src1);
						CHECK_FLAG_Z(src0 || src1);
						CHECK_FLAG_C(src1, 0xff);

						m_icount -= 15;
					}
					break;

				case 0xc0:  //adbcm
				case 0xc1:  //sbbcm
				case 0xc8:  //adbm
				case 0xc9:  //sbbm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 dst = arg;
						UINT8 src = get_sir_im8(arg, arg1);
						UINT8 c, f;
						UINT16 res = 0;

						c = f = 0;
						for (int n=GET_IM3(arg1); n>0; n--)
						{
							if (op & 0x01)
								res = make_bcd_sub(READ_REG(dst), READ_REG(src) + c);
							else
								res = make_bcd_add(READ_REG(dst), READ_REG(src) + c);

							c = (res > 0xff) ? 1 : 0;

							COND_WRITE_REG(op, dst, res&0xff);

							f |= (res&0xff);
							dst++; src++;

							m_icount -= 5;
						}

						CLEAR_FLAGS;
						CHECK_FLAG_Z(f);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						check_optional_jr(arg);
						m_icount -= 2;
					}
					break;

				case 0xc2:  //ldm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 dst = arg;
						UINT8 src = get_sir_im8(arg, arg1);

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							COPY_REG(dst++, src++);
							m_icount -= 5;
						}

						check_optional_jr(arg);
						m_icount -= 2;
					}
					break;

				case 0xc4:  //ancm
				case 0xc5:  //nacm
				case 0xc6:  //orcm
				case 0xc7:  //xrcm
				case 0xcc:  //anm
				case 0xcd:  //nam
				case 0xce:  //orm
				case 0xcf:  //xrm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 dst = arg;
						UINT8 src = get_sir_im8(arg, arg1);
						UINT8 res = 0, f = 0;

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							res = make_logic(op, READ_REG(dst), READ_REG(src));

							COND_WRITE_REG(op, dst, res);

							f |= res;

							dst++; src++;

							m_icount -= 3;
						}

						CLEAR_FLAGS;
						CHECK_FLAG_Z(f);
						CHECK_FLAGB_UZ_LZ(res);

						//na(c) and or(c) always set C flag
						if ((op&3) == 1 || (op&3) == 2)
							SET_FLAG_C;

						check_optional_jr(arg);
					}
					break;

				case 0xca:  //adbm
				case 0xcb:  //sbbm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 dst = arg;
						UINT16 res = 0;
						UINT8 src, f;
						src = arg1 & 0x1f;
						f = 0;

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							if (op & 0x01)
								res = make_bcd_sub(READ_REG(dst), src);
							else
								res = make_bcd_add(READ_REG(dst), src);

							src = (res > 0xff) ? 1 : 0;

							COND_WRITE_REG(op, dst, res&0xff);

							f |= (res&0xff);
							dst++;

							m_icount -= 5;
						}

						CLEAR_FLAGS;
						CHECK_FLAG_Z(f);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						check_optional_jr(arg);
						m_icount -= 2;
					}
					break;

				case 0xd0:  //stw
					{
						UINT8 arg = read_op();
						UINT16 offset = REG_GET16(READ_SREG(arg));

						mem_writebyte(REG_UA>>4, offset+0, read_op());
						mem_writebyte(REG_UA>>4, offset+1, read_op());

						m_icount -= 11;
					}
					break;

				case 0xd1:  //ldw
					{
						UINT8 arg = read_op();
						UINT8 reg0 = read_op();
						UINT8 reg1 = read_op();

						WRITE_REG(arg+0, reg0);
						WRITE_REG(arg+1, reg1);
						m_icount -= 11;
					}
					break;

				case 0xd2:  //stlm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							m_lcd_write_cb((offs_t)0, READ_REG(arg));

							arg++;
							m_icount -= 8;
						}

						m_icount -= 3;
					}
					break;

				case 0xd3:  //ldlm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 src;

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							src = m_lcd_read_cb(0);

							WRITE_REG(arg, src++);

							m_icount -= 8;
						}

						m_icount -= 3;
					}
					break;

				case 0xd6:  //pre
				case 0xd7:  //pre
					{
						UINT8 arg = read_op();
						UINT8 lsb = read_op();
						UINT8 msb = read_op();
						UINT8 idx = GET_REG_IDX(op, arg);

						if (idx < 5)
							m_reg16bit[idx] = (msb<<8) | lsb;

						m_icount -= 8;
					}
					break;

				case 0xd8:  //bup
				case 0xd9:  //bdn
					{
						UINT8 src;
						for(;;)
						{
							src = mem_readbyte(REG_UA>>4, REG_IX);
							mem_writebyte(REG_UA>>6, REG_IZ, src);

							if (REG_IX == REG_IY)
								break;

							REG_IX += ((op&1) ? -1 : +1);
							REG_IZ += ((op&1) ? -1 : +1);
							m_icount -= 6;
						}

						m_icount -= 9;
					}
					break;

				case 0xda:
					{
						UINT8 arg = read_op();
						UINT8 op1 = (arg>>5) & 0x03;
						switch (op1)
						{
							case 0x00:  //didm
								{
									UINT8 arg1 = read_op();
									UINT8 r1 = 0, r2 = 0;

									for (int n=GET_IM3(arg1); n>0; n--)
									{
										r2 = r1;
										r1 = READ_REG(arg);
										r2 = ((r1>>4)&0x0f) | ((r2<<4)&0xf0);
										WRITE_REG(arg--, r2);
										m_icount -= 5;
									}

									CLEAR_FLAGS;
									CHECK_FLAGB_UZ_LZ(r2);
									CHECK_FLAG_Z(r2);
								}
								break;

							case 0x01:  //dium
								{
									UINT8 arg1 = read_op();
									UINT8 r1 = 0, r2 = 0;

									for (int n=GET_IM3(arg1); n>0; n--)
									{
										r2 = r1;
										r1 = READ_REG(arg);
										r2 = ((r1<<4)&0xf0) | ((r2>>4)&0x0f);
										WRITE_REG(arg++, r2);
										m_icount -= 5;
									}

									CLEAR_FLAGS;
									CHECK_FLAGB_UZ_LZ(r2);
									CHECK_FLAG_Z(r2);
								}
								break;

							case 0x02:  //bydm
								{
									UINT8 arg1 = read_op();
									UINT8 r1 = 0, r2 = 0, f = 0;

									for (int n=GET_IM3(arg1); n>0; n--)
									{
										r2 = r1;
										r1 = READ_REG(arg);
										WRITE_REG(arg--, r2);
										f |= r2;
										m_icount -= 5;
									}

									CLEAR_FLAGS;
									CHECK_FLAGB_UZ_LZ(r2);
									CHECK_FLAG_Z(f);
								}
								break;

							case 0x03:  //byum
								{
									UINT8 arg1 = read_op();
									UINT8 r1 = 0, r2 = 0, f = 0;

									for (int n=GET_IM3(arg1); n>0; n--)
									{
										r2 = r1;
										r1 = READ_REG(arg);
										WRITE_REG(arg++, r2);
										f |= r2;
										m_icount -= 5;
									}

									CLEAR_FLAGS;
									CHECK_FLAGB_UZ_LZ(r2);
									CHECK_FLAG_Z(f);
								}
								break;
						}
					}
					break;

				case 0xdb:  //cmpm/invm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 r1 = 0, r2 = 0, f = 0;

						r2 = (arg&0x40) ? 0 : 1;

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							r1 = r2 + (~READ_REG(arg));
							WRITE_REG(arg++, r1);
							if (r1) r2 = 0;

							f |= r1;

							m_icount -= 5;
						}

						CLEAR_FLAGS;
						CHECK_FLAG_Z(f);
						CHECK_FLAGB_UZ_LZ(r1);
						if (f != 0 || (arg & 0x40))
							SET_FLAG_C;
					}
					break;

				case 0xdc:  //sup
				case 0xdd:  //sdn
					{
						UINT8 arg = read_op();
						UINT16 res;

						for(;;)
						{
							res = mem_readbyte(REG_UA>>4, REG_IX) - READ_REG(arg);

							if (REG_IX == REG_IY || !res)
								break;

							REG_IX += ((op&1) ? -1 : +1);
							m_icount -= 6;
						}

						CLEAR_FLAGS;
						CHECK_FLAG_Z(res & 0xff);
						CHECK_FLAGB_UZ_LZ(res);
						CHECK_FLAG_C(res, 0xff);

						m_icount -= 9;
					}
					break;

				case 0xde:  //jp
					{
						UINT8 arg = read_op();
						set_pc(REG_GET16(arg));

						m_icount -= 5;
					}
					break;

				case 0xdf:  //jp
					{
						UINT8 arg = read_op();
						UINT16 offset = REG_GET16(arg);
						UINT8 lsb = mem_readbyte(REG_UA>>4, offset+0);
						UINT8 msb = mem_readbyte(REG_UA>>4, offset+1);

						set_pc((msb<<8) | lsb);

						m_icount -= 5;
					}
					break;

				case 0xe0:  //stm
				case 0xe2:  //stim
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT16 prev_ir = REG_IX;
						UINT8 ir_inc = READ_REG(get_sir_im8(arg, arg1));

						REG_IX += ((arg & 0x80) ? -ir_inc : +ir_inc);

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							mem_writebyte(REG_UA>>4, REG_IX++, READ_REG(arg++));
							m_icount -= 3;
						}

						RESTORE_REG(op, REG_IX, prev_ir);

						m_icount -= 5;
					}
					break;

				case 0xe1:  //stm
				case 0xe3:  //stim
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT16 prev_ir = REG_IZ;
						UINT8 ir_inc = READ_REG(get_sir_im8(arg, arg1));

						REG_IZ += ((arg & 0x80) ? -ir_inc : +ir_inc);

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							mem_writebyte(REG_UA>>6, REG_IZ++, READ_REG(arg++));
							m_icount -= 3;
						}

						RESTORE_REG(op, REG_IZ, prev_ir);

						m_icount -= 5;
					}
					break;

				case 0xe4:  //stdm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg, arg1));

						REG_IX += ((arg & 0x80) ? -ir_inc : +ir_inc);

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							mem_writebyte(REG_UA>>4, REG_IX--, READ_REG(arg--));
							m_icount -= 3;
						}

						REG_IX++;//todo

						m_icount -= 3;
					}
					break;

				case 0xe5:  //stdm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg, arg1));

						REG_IZ += ((arg & 0x80) ? -ir_inc : +ir_inc);

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							mem_writebyte(REG_UA>>6, REG_IZ--, READ_REG(arg--));
							m_icount -= 3;
						}

						REG_IZ++;

						m_icount -= 3;
					}
					break;

				case 0xe6:  //phsm
				case 0xe7:  //phum
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							push((op&1) ? REG_US : REG_SS, READ_REG(arg--));

							m_icount -= 3;
						}

						m_icount -= 3;
					}
					break;

				case 0xe8:  //ldm
				case 0xea:  //ldim
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT16 prev_ir = REG_IX;
						UINT8 ir_inc = READ_REG(get_sir_im8(arg, arg1));

						REG_IX += ((arg & 0x80) ? -ir_inc : +ir_inc);

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							WRITE_REG(arg++, mem_readbyte(REG_UA>>4, REG_IX++));
							m_icount -= 3;
						}

						RESTORE_REG(op, REG_IX, prev_ir);

						m_icount -= 5;
					}
					break;

				case 0xe9:  //ldm
				case 0xeb:  //ldim
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT16 prev_ir = REG_IZ;
						UINT8 ir_inc = READ_REG(get_sir_im8(arg, arg1));

						REG_IZ += ((arg & 0x80) ? -ir_inc : +ir_inc);

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							WRITE_REG(arg++, mem_readbyte(REG_UA>>6, REG_IZ++));
							m_icount -= 3;
						}

						RESTORE_REG(op, REG_IZ, prev_ir);

						m_icount -= 5;
					}
					break;

				case 0xec:  //lddm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg, arg1));

						REG_IX += ((arg & 0x80) ? -ir_inc : +ir_inc);

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							WRITE_REG(arg--, mem_readbyte(REG_UA>>4, REG_IX--));
							m_icount -= 3;
						}

						REG_IX++;

						m_icount -= 3;
					}
					break;

				case 0xed:  //lddm
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();
						UINT8 ir_inc = READ_REG(get_sir_im8(arg, arg1));

						REG_IZ += ((arg & 0x80) ? -ir_inc : +ir_inc);

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							WRITE_REG(arg--, mem_readbyte(REG_UA>>6, REG_IZ--));
							m_icount -= 3;
						}

						REG_IZ++;

						m_icount -= 3;
					}
					break;

				case 0xee:  //ppsm
				case 0xef:  //ppum
					{
						UINT8 arg = read_op();
						UINT8 arg1 = read_op();

						for (int n=GET_IM3(arg1); n>0; n--)
						{
							WRITE_REG(arg++, pop((op&1) ? REG_US : REG_SS));
							m_icount -= 3;
						}

						m_icount -= 5;
					}
					break;

				case 0xf0:  //rtn z
				case 0xf1:  //rtn nc
				case 0xf2:  //rtn lz
				case 0xf3:  //rtn uz
				case 0xf4:  //rtn nz
				case 0xf5:  //rtn c
				case 0xf6:  //rtn nlz
				case 0xf7:  //unconditional rtn
					{
						if (check_cond(op))
						{
							UINT8 lsb = pop(REG_SS) ;
							UINT8 msb = pop(REG_SS);

							set_pc((((msb<<8) | (lsb<<0)) + 1));
						}
						m_icount -= 3;
					}
					break;

				case 0xf8:  //nop
						m_icount -= 3;
					break;

				case 0xf9:  //clt
						REG_TM = 0;
						m_icount -= 3;
					break;

				case 0xfa:  //fst
						m_state |= CPU_FAST;
						m_icount -= 3;
					break;

				case 0xfb:  //slw
						m_state &= ~CPU_FAST;
						m_icount -= 3;
					break;

				case 0xfd:  //rtni
					{
						UINT8 lsb = pop(REG_SS);
						UINT8 msb = pop(REG_SS);

						set_pc((msb<<8) | (lsb<<0));

						m_icount -= 5;
					}
					//fall through

				case 0xfc:  //cani
					{
						for (UINT8 arg=0x10; arg>0; arg>>=1)
						{
							if (REG_IB & arg)
							{
								REG_IB &= (~arg);
								m_irq_status &= (~arg);
								break;
							}
						}

						m_icount -= 3;
					}
					break;

				case 0xfe:  //off
					{
						set_pc(0);
						REG_UA = REG_IA = 0;
						REG_IX = REG_IY = REG_IZ = 0;
						REG_PE = 0;
						REG_IE = (REG_IE&0x0c) | ((REG_IB>>1) & 0x10);
						REG_IB &= 0xe3;
						if (m_flags & FLAG_SW)
							m_flags |= FLAG_APO;
						else
							m_flags &= ~FLAG_APO;

						m_state |= CPU_SLP;

						m_irq_status = 0;
						m_lcd_ctrl_cb((offs_t)0, 0);
						m_kb_write_cb((offs_t)0, 0);
						m_icount -= 3;
					}
					break;

				case 0xff:  //trp
					{
						m_pc--;
						push(REG_SS, (m_pc>>8)&0xff);
						push(REG_SS, (m_pc>>0)&0xff);
						set_pc(0x0022);
						m_icount -= 9;
					}
					break;

				case 0x03:
				case 0x19:
				case 0x43:
				case 0x53:
				case 0x5a:
				case 0x5b:
				case 0x5e:
				case 0x5f:
				case 0x66:
				case 0x67:
				case 0x6e:
				case 0x6f:
				case 0x83:
				case 0x94:
				case 0x95:
				case 0x99:
				case 0x9d:
				case 0xc3:
				case 0xd4:
				case 0xd5:
					logerror( "%06x: illegal instruction %02x encountered\n", m_pc, op );
					break;

				default:
					logerror( "%06x: unimplemented instruction %02x encountered\n", m_pc, op );
					break;
			}
		}

		//if is in the internal ROM align the pc
		if ((m_fetch_addr&1) && m_pc < INT_ROM)
			set_pc((m_fetch_addr+1)>>1);

		m_icount -= 3;
	} while (m_icount > 0);
}


//-------------------------------------------------
//  execute_set_input
//-------------------------------------------------

void hd61700_cpu_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		case INPUT_LINE_RESET:
			REG_UA = 0;
			REG_IA = 0;
			REG_IX = 0;
			REG_IY = 0;
			REG_IZ = 0;
			REG_PE = 0;
			REG_IE = 0;
			REG_IB = 0;
			m_state &= ~(CPU_SLP | CPU_FAST);
			set_pc(0x0000);
			break;
		case HD61700_KEY_INT:   //level sensitive line
			if (((REG_IE>>3) & (1<<inputnum)) && state != CLEAR_LINE)
				REG_IB |= (1<<inputnum);
			break;

		case HD61700_INT1:  //edge sensitive line
			if (((REG_IE>>3) & (1<<inputnum)) && (m_lines_status[inputnum] != state))
				REG_IB |= (1<<inputnum);

			if (m_lines_status[inputnum] == CLEAR_LINE && state != CLEAR_LINE)
				REG_IE = (REG_IE & 0xfd) | 0x02;    //rising edge
			else if (m_lines_status[inputnum] != CLEAR_LINE && state == CLEAR_LINE)
				REG_IE = (REG_IE & 0xfd) | 0x00;    //falling edge

			REG_KY = (REG_KY & 0xf7ff) | ((state != CLEAR_LINE)<<11);
			break;
		case HD61700_INT2:  //level sensitive line
			if (((REG_IE>>3) & (1<<inputnum)) && state != CLEAR_LINE)
				REG_IB |= (1<<inputnum);

			REG_IE = (REG_IE & 0xfe) | (state != CLEAR_LINE);
			REG_KY = (REG_KY & 0xfbff) | ((state != CLEAR_LINE)<<10);
			break;
		case HD61700_ON_INT:    //level sensitive line
			if ((REG_IE>>3) & (1<<inputnum) && state != CLEAR_LINE)
				REG_IB |= (1<<inputnum);

			REG_KY = (REG_KY & 0xfdff) | ((state != CLEAR_LINE)<<9);
			break;

		case HD61700_SW:    //level sensitive line
			if (state != CLEAR_LINE)
			{
				if ((m_state & CPU_SLP) || m_lines_status[HD61700_SW] == CLEAR_LINE)
				{
					m_state &= ~(CPU_SLP | CPU_FAST);
					m_flags &= ~FLAG_APO;
					set_pc(0x0001);
				}
			}

			m_flags |= ((state != CLEAR_LINE)<<3);
			break;
	}

	m_lines_status[inputnum] = state;
}


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void hd61700_cpu_device::set_pc(INT32 new_pc)
{
	m_curpc = (m_curpc & 0x30000) | new_pc;
	m_pc = new_pc & 0xffff;
	m_fetch_addr = new_pc<<1;
}

inline UINT8 hd61700_cpu_device::read_op()
{
	UINT16 data = 0;
	UINT32 addr18 = make_18bit_addr((m_irq_status) ? 0 : prev_ua, m_pc);

	if (m_pc <= INT_ROM)
	{
		data = m_program->read_word(addr18<<1);

		if (!(m_fetch_addr&1))
			data = (data>>8) ;
	}
	else
	{
		if (m_fetch_addr&1)
			data = m_program->read_word((addr18+1)<<1);
		else
			data = m_program->read_word((addr18+0)<<1);
	}

	m_fetch_addr += ((m_pc > INT_ROM) ? 2 : 1);

	m_pc = m_fetch_addr>>1;

	m_curpc =  make_18bit_addr((m_irq_status) ? 0 : prev_ua, m_pc);

	prev_ua = REG_UA;

	return (data&0xff);
}

inline UINT8 hd61700_cpu_device::mem_readbyte(UINT8 segment, UINT16 offset)
{
	return m_program->read_word(make_18bit_addr(segment, offset)<<1) & 0xff;
}

inline void hd61700_cpu_device::mem_writebyte(UINT8 segment, UINT16 offset, UINT8 data)
{
	m_program->write_word(make_18bit_addr(segment, offset)<<1, data);
}

inline UINT32 hd61700_cpu_device::make_18bit_addr(UINT8 segment, UINT16 offset)
{
	if (offset >= ((REG_IB>>6) & 0x03) * 0x4000)
		return (UINT32)((offset | ((segment&0x03)<<16)) & 0x3ffff);
	else
		return offset;
}

inline void hd61700_cpu_device::push(UINT16 &offset, UINT8 data)
{
	offset--;
	mem_writebyte(REG_UA>>2, offset, data);
}

inline UINT8 hd61700_cpu_device::pop(UINT16 &offset)
{
	UINT8 data = mem_readbyte(REG_UA>>2, offset);
	offset++;
	return data;
}

inline int hd61700_cpu_device::check_cond(UINT32 op)
{
	switch ( op & 0x07 )
	{
	case 0x00:  // Z set
		if ( !(m_flags & FLAG_Z) )
			return 1;
		break;

	case 0x01:  // NC set
		if ( !(m_flags & FLAG_C) )
			return 1;
		break;

	case 0x02:  // LZ set
		if ( !(m_flags & FLAG_LZ) )
			return 1;
		break;

	case 0x03:  // UZ set
		if ( !(m_flags & FLAG_UZ) )
			return 1;
		break;

	case 0x04:  // NZ set
		if ( m_flags & FLAG_Z )
			return 1;
		break;

	case 0x05:  // C set
		if ( m_flags & FLAG_C )
			return 1;
		break;

	case 0x06:  // NLZ clear
		if ( m_flags & FLAG_LZ )
			return 1;
		break;

	case 0x07:  // unconditional
		return 1;
	}

	return 0;
}

inline UINT8 hd61700_cpu_device::make_logic(UINT8 type, UINT8 d1, UINT8 d2)
{
	switch (type&3)
	{
		case 0: //and
			return d1 & d2;
		case 1: //nand
			return ~(d1 & d2);
		case 2: //or
			return d1 | d2;
		case 3: //xor
			return d1 ^ d2;
		default:
			return 0x00;
	}
}

inline void hd61700_cpu_device::check_optional_jr(UINT8 arg)
{
	if (arg & 0x80)
	{
		if (m_pc < INT_ROM && !(m_fetch_addr&1)) read_op();

		UINT8 arg1 = read_op();

		UINT32 new_pc = m_pc + get_im_7(arg1) - 1;

		set_pc(new_pc);
		m_icount -= 3;
	}
}

inline UINT8 hd61700_cpu_device::get_sir_im8(UINT8 arg)
{
	if (((arg>>5) & 0x03) == 0x03)
	{
		UINT8 arg1 = read_op();

		return arg1&0x1f;
	}
	else
	{
		return READ_SREG(arg);
	}
}

inline UINT8 hd61700_cpu_device::get_sir_im8(UINT8 arg, UINT8 arg1)
{
	if (((arg>>5) & 0x03) == 0x03)
	{
		return arg1&0x1f;
	}
	else
	{
		return READ_SREG(arg);
	}
}

inline int hd61700_cpu_device::get_sign_mreg(UINT8 arg)
{
	int res = READ_REG(get_sir_im8(arg));

	if (arg & 0x80) res = -res;

	return res;
}

inline int hd61700_cpu_device::get_sign_im8(UINT8 arg)
{
	int res = read_op();

	if (arg & 0x80) res = -res;

	return res;
}

inline int hd61700_cpu_device::get_im_7(UINT8 data)
{
	if (data&0x80)
		return 0x80 - data;
	else
		return data;
}

inline UINT16 hd61700_cpu_device::make_bcd_sub(UINT8 arg1, UINT8 arg2)
{
	UINT32 ret = (arg1&0x0f) - (arg2&0x0f);
	UINT8 carry;

	if (ret > 0x09)
	{
		ret = (ret - 0x06) & 0x0f;
		carry = 1;
	}
	else
		carry = 0;

	ret += ((arg1&0xf0) - (arg2&0xf0) - (carry<<4));

	if (ret > 0x9f)
	{
		ret = (ret - 0x60) & 0x0ff;
		carry = 1;
	}
	else
		carry = 0;

	ret -= (carry<<8);

	return ret;
}

inline UINT16 hd61700_cpu_device::make_bcd_add(UINT8 arg1, UINT8 arg2)
{
	UINT32 ret = (arg1&0x0f) + (arg2&0x0f);
	UINT8 carry;

	if (ret > 0x09)
	{
		ret = (ret + 0x06) & 0x0f;
		carry = 1;
	}
	else
		carry = 0;

	ret += ((arg1&0xf0) + (arg2&0xf0) + (carry<<4));

	if (ret > 0x9f)
	{
		ret = (ret + 0x60) & 0x0ff;
		carry = 1;
	}
	else
		carry = 0;

	ret += (carry<<8);

	return ret;
}
