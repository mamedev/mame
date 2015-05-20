// license:BSD-3-Clause
// copyright-holders:Dan Boris
/***************************************************************************

    Atari I, Robot hardware

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "includes/irobot.h"

/* Note:
 * There's probably something wrong with the way the Mathbox gets started.
 * Try compiling with IR_TIMING=1, run with logging on and take a look at
 * the resulting logilfe.
 * The mathbox is started in short intervals (<10 scanlines) without (!)
 * checking its idle status.
 * It also seems that the mathbox in this emulation would have to cope with
 * approx. 5000 instructions per scanline [look at the number of instructions
 * and the number of scanlines to the next mathbox start]. This seems a bit
 * too high.
 */

#define DISASSEMBLE_MB_ROM      0       /* generate a disassembly of the mathbox ROMs */

#define IR_CPU_STATE() \
	logerror(\
			"%s, scanline: %d\n", machine().describe_context(), m_screen->vpos())


READ8_MEMBER(irobot_state::irobot_sharedmem_r)
{
	if (m_outx == 3)
		return m_mbRAM[BYTE_XOR_BE(offset)];

	if (m_outx == 2)
		return m_combase[BYTE_XOR_BE(offset & 0xFFF)];

	if (m_outx == 0)
		return m_mbROM[((m_mpage & 1) << 13) + BYTE_XOR_BE(offset)];

	if (m_outx == 1)
		return m_mbROM[0x4000 + ((m_mpage & 3) << 13) + BYTE_XOR_BE(offset)];

	return 0xFF;
}

/* Comment out the mbRAM =, comRAM2 = or comRAM1 = and it will start working */
WRITE8_MEMBER(irobot_state::irobot_sharedmem_w)
{
	if (m_outx == 3)
		m_mbRAM[BYTE_XOR_BE(offset)] = data;

	if (m_outx == 2)
		m_combase[BYTE_XOR_BE(offset & 0xFFF)] = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(irobot_state::irobot_irvg_done_callback)
{
	logerror("vg done. ");
	m_irvg_running = 0;
}

WRITE8_MEMBER(irobot_state::irobot_statwr_w)
{
	logerror("write %2x ", data);
	IR_CPU_STATE();

	m_combase = m_comRAM[data >> 7];
	m_combase_mb = m_comRAM[(data >> 7) ^ 1];
	m_bufsel = data & 0x02;
	if (((data & 0x01) == 0x01) && (m_vg_clear == 0))
		irobot_poly_clear();

	m_vg_clear = data & 0x01;

	if ((data & 0x04) && !(m_statwr & 0x04))
	{
		irobot_run_video();
#if IR_TIMING
		if (m_irvg_running == 0)
			logerror("vg start ");
		else
			logerror("vg start [busy!] ");
		IR_CPU_STATE();
		m_irvg_timer->adjust(attotime::from_msec(10));
#endif
		m_irvg_running=1;
	}
	if ((data & 0x10) && !(m_statwr & 0x10))
		irmb_run();
	m_statwr = data;
}

WRITE8_MEMBER(irobot_state::irobot_out0_w)
{
	UINT8 *RAM = memregion("maincpu")->base();

	m_out0 = data;
	switch (data & 0x60)
	{
		case 0:
			membank("bank2")->set_base(&RAM[0x1C000]);
			break;
		case 0x20:
			membank("bank2")->set_base(&RAM[0x1C800]);
			break;
		case 0x40:
			membank("bank2")->set_base(&RAM[0x1D000]);
			break;
	}
	m_outx = (data & 0x18) >> 3;
	m_mpage = (data & 0x06) >> 1;
	m_alphamap = (data & 0x80);
}

WRITE8_MEMBER(irobot_state::irobot_rom_banksel_w)
{
	UINT8 *RAM = memregion("maincpu")->base();

	switch ((data & 0x0E) >> 1)
	{
		case 0:
			membank("bank1")->set_base(&RAM[0x10000]);
			break;
		case 1:
			membank("bank1")->set_base(&RAM[0x12000]);
			break;
		case 2:
			membank("bank1")->set_base(&RAM[0x14000]);
			break;
		case 3:
			membank("bank1")->set_base(&RAM[0x16000]);
			break;
		case 4:
			membank("bank1")->set_base(&RAM[0x18000]);
			break;
		case 5:
			membank("bank1")->set_base(&RAM[0x1A000]);
			break;
	}
	set_led_status(machine(), 0,data & 0x10);
	set_led_status(machine(), 1,data & 0x20);
}

TIMER_CALLBACK_MEMBER(irobot_state::scanline_callback)
{
	int scanline = param;

	if (scanline == 0) m_irvg_vblank=0;
	if (scanline == 224) m_irvg_vblank=1;
	logerror("SCANLINE CALLBACK %d\n",scanline);
	/* set the IRQ line state based on the 32V line state */
	m_maincpu->set_input_line(M6809_IRQ_LINE, (scanline & 32) ? ASSERT_LINE : CLEAR_LINE);

	/* set a callback for the next 32-scanline increment */
	scanline += 32;
	if (scanline >= 256) scanline = 0;
	machine().scheduler().timer_set(m_screen->time_until_pos(scanline), timer_expired_delegate(FUNC(irobot_state::scanline_callback),this), scanline);
}

void irobot_state::machine_reset()
{
	UINT8 *MB = memregion("mathbox")->base();

	/* initialize the memory regions */
	m_mbROM     = MB + 0x00000;
	m_mbRAM     = MB + 0x0c000;
	m_comRAM[0] = MB + 0x0e000;
	m_comRAM[1] = MB + 0x0f000;

	m_irvg_vblank=0;
	m_irvg_running = 0;
	m_irvg_timer = machine().device<timer_device>("irvg_timer");
	m_irmb_running = 0;
	m_irmb_timer = machine().device<timer_device>("irmb_timer");

	/* set an initial timer to go off on scanline 0 */
	machine().scheduler().timer_set(m_screen->time_until_pos(0), timer_expired_delegate(FUNC(irobot_state::scanline_callback),this));

	irobot_rom_banksel_w(m_maincpu->space(AS_PROGRAM),0,0);
	irobot_out0_w(m_maincpu->space(AS_PROGRAM),0,0);
	m_combase = m_comRAM[0];
	m_combase_mb = m_comRAM[1];
	m_outx = 0;
}

WRITE8_MEMBER(irobot_state::irobot_control_w)
{
	m_control_num = offset & 0x03;
}

READ8_MEMBER(irobot_state::irobot_control_r)
{
	if (m_control_num == 0)
		return ioport("AN0")->read();
	else if (m_control_num == 1)
		return ioport("AN1")->read();
	return 0;

}

/*  we allow irmb_running and irvg_running to appear running before clearing
    them to simulate the mathbox and vector generator running in real time */
READ8_MEMBER(irobot_state::irobot_status_r)
{
	int d=0;

	logerror("status read. ");
	IR_CPU_STATE();

	if (!m_irmb_running) d |= 0x20;
	if (m_irvg_running) d |= 0x40;

	//        d = (m_irmb_running * 0x20) | (m_irvg_running * 0x40);
	if (m_irvg_vblank) d = d | 0x80;
#if IR_TIMING
	/* flags are cleared by callbacks */
#else
	m_irmb_running=0;
	m_irvg_running=0;
#endif
	return d;
}


/***********************************************************************

    I-Robot Mathbox

    Based on 4 2901 chips slice processors connected to form a 16-bit ALU

    Microcode roms:
    6N: bits 0..3: Address of ALU A register
    5P: bits 0..3: Address of ALU B register
    6M: bits 0..3: ALU Function bits 5..8
    7N: bits 0..3: ALU Function bits 1..4
    8N: bits 0,1: Memory write timing
        bit 2: Hardware multiply mode
        bit 3: ALU Function bit 0
    6P: bits 0,1: Direct addressing bits 0,1
        bits 2,3: Jump address bits 0,1
    8M: bits 0..3: Jump address bits 6..9
    9N: bits 0..3: Jump address bits 2..5
    8P: bits 0..3: Memory address bits 2..5
    9M: bit 0: Shift control
        bits 1..3: Jump type
            0 = No Jump
            1 = On carry
            2 = On zero
            3 = On positive
            4 = On negative
            5 = Unconditional
            6 = Jump to Subroutine
            7 = Return from Subroutine
    7M: Bit 0: Mathbox memory enable
        Bit 1: Latch data to address bus
        Bit 2: Carry in select
        Bit 3: Carry in value
        (if 2,3 = 11 then mathbox is done)
    9P: Bit 0: Hardware divide enable
        Bits 1,2: Memory select
        Bit 3: Memory R/W
    7P: Bits 0,1: Direct addressing bits 6,7
        Bits 2,3: Unused

***********************************************************************/

#define FL_MULT 0x01
#define FL_shift 0x02
#define FL_MBMEMDEC 0x04
#define FL_ADDEN 0x08
#define FL_DPSEL 0x10
#define FL_carry 0x20
#define FL_DIV 0x40
#define FL_MBRW 0x80


#if DISASSEMBLE_MB_ROM
static void disassemble_instruction(irmb_ops *op);
#endif


UINT32 irobot_state::irmb_din(const irmb_ops *curop)
{
	UINT32 d = 0;

	if (!(curop->flags & FL_MBMEMDEC) && (curop->flags & FL_MBRW))
	{
		UINT32 ad = curop->diradd | (m_irmb_latch & curop->latchmask);

		if (curop->diren || (m_irmb_latch & 0x6000) == 0)
			d = ((UINT16 *)m_mbRAM)[ad & 0xfff];             /* MB RAM read */
		else if (m_irmb_latch & 0x4000)
			d = ((UINT16 *)m_mbROM)[ad + 0x2000];                /* MB ROM read, CEMATH = 1 */
		else
			d = ((UINT16 *)m_mbROM)[ad & 0x1fff];                /* MB ROM read, CEMATH = 0 */
	}
	return d;
}


void irobot_state::irmb_dout(const irmb_ops *curop, UINT32 d)
{
	/* Write to video com ram */
	if (curop->ramsel == 3)
		((UINT16 *)m_combase_mb)[m_irmb_latch & 0x7ff] = d;

	/* Write to mathox ram */
	if (!(curop->flags & FL_MBMEMDEC))
	{
		UINT32 ad = curop->diradd | (m_irmb_latch & curop->latchmask);

		if (curop->diren || (m_irmb_latch & 0x6000) == 0)
			((UINT16 *)m_mbRAM)[ad & 0xfff] = d;             /* MB RAM write */
	}
}


/* Convert microcode roms to a more usable form */
void irobot_state::load_oproms()
{
	UINT8 *MB = memregion("proms")->base() + 0x20;
	int i;

	/* allocate RAM */
	m_mbops = auto_alloc_array(machine(), irmb_ops, 1024);

	for (i = 0; i < 1024; i++)
	{
		int nxtadd, func, ramsel, diradd, latchmask, dirmask, time;

		m_mbops[i].areg = &m_irmb_regs[MB[0x0000 + i] & 0x0F];
		m_mbops[i].breg = &m_irmb_regs[MB[0x0400 + i] & 0x0F];
		func = (MB[0x0800 + i] & 0x0F) << 5;
		func |= ((MB[0x0C00 +i] & 0x0F) << 1);
		func |= (MB[0x1000 + i] & 0x08) >> 3;
		time = MB[0x1000 + i] & 0x03;
		m_mbops[i].flags = (MB[0x1000 + i] & 0x04) >> 2;
		nxtadd = (MB[0x1400 + i] & 0x0C) >> 2;
		diradd = MB[0x1400 + i] & 0x03;
		nxtadd |= ((MB[0x1800 + i] & 0x0F) << 6);
		nxtadd |= ((MB[0x1C00 + i] & 0x0F) << 2);
		diradd |= (MB[0x2000 + i] & 0x0F) << 2;
		func |= (MB[0x2400 + i] & 0x0E) << 9;
		m_mbops[i].flags |= (MB[0x2400 + i] & 0x01) << 1;
		m_mbops[i].flags |= (MB[0x2800 + i] & 0x0F) << 2;
		m_mbops[i].flags |= ((MB[0x2C00 + i] & 0x01) << 6);
		m_mbops[i].flags |= (MB[0x2C00 + i] & 0x08) << 4;
		ramsel = (MB[0x2C00 + i] & 0x06) >> 1;
		diradd |= (MB[0x3000 + i] & 0x03) << 6;

		if (m_mbops[i].flags & FL_shift) func |= 0x200;

		m_mbops[i].func = func;
		m_mbops[i].nxtop = &m_mbops[nxtadd];

		/* determine the number of 12MHz cycles for this operation */
		if (time == 3)
			m_mbops[i].cycles = 2;
		else
			m_mbops[i].cycles = 3 + time;

		/* precompute the hardcoded address bits and the mask to be used on the latch value */
		if (ramsel == 0)
		{
			dirmask = 0x00FC;
			latchmask = 0x3000;
		}
		else
		{
			dirmask = 0x0000;
			latchmask = 0x3FFC;
		}
		if (ramsel & 2)
			latchmask |= 0x0003;
		else
			dirmask |= 0x0003;

		m_mbops[i].ramsel = ramsel;
		m_mbops[i].diradd = diradd & dirmask;
		m_mbops[i].latchmask = latchmask;
		m_mbops[i].diren = (ramsel == 0);

#if DISASSEMBLE_MB_ROM
		disassemble_instruction(&m_mbops[i]);
#endif
	}
}


/* Init mathbox (only called once) */
DRIVER_INIT_MEMBER(irobot_state,irobot)
{
	int i;
	for (i = 0; i < 16; i++)
	{
		m_irmb_stack[i] = &m_mbops[0];
		m_irmb_regs[i] = 0;
	}
	m_irmb_latch=0;
	load_oproms();
}

TIMER_DEVICE_CALLBACK_MEMBER(irobot_state::irobot_irmb_done_callback)
{
	logerror("mb done. ");
	m_irmb_running = 0;
	m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}


#define COMPUTE_CI \
	CI = 0;\
	if (curop->flags & FL_DPSEL)\
		CI = cflag;\
	else\
	{\
		if (curop->flags & FL_carry)\
			CI = 1;\
		if (!(prevop->flags & FL_DIV) && !nflag)\
			CI = 1;\
	}

#define ADD(r,s) \
	COMPUTE_CI;\
	result = r + s + CI;\
	cflag = (result >> 16) & 1;\
	vflag = (((r & 0x7fff) + (s & 0x7fff) + CI) >> 15) ^ cflag

#define SUBR(r,s) \
	COMPUTE_CI;\
	result = (r ^ 0xFFFF) + s + CI;         /*S - R + CI - 1*/ \
	cflag = (result >> 16) & 1;\
	vflag = (((s & 0x7fff) + ((r ^ 0xffff) & 0x7fff) + CI) >> 15) ^ cflag

#define SUB(r,s) \
	COMPUTE_CI;\
	result = r + (s ^ 0xFFFF) + CI;      /*R - S + CI - 1*/ \
	cflag = (result >> 16) & 1;\
	vflag = (((r & 0x7fff) + ((s ^ 0xffff) & 0x7fff) + CI) >> 15) ^ cflag

#define OR(r,s) \
	result = r | s;\
	vflag = cflag = 0

#define AND(r,s) \
	result = r & s;\
	vflag = cflag = 0

#define IAND(r,s) \
	result = (r ^ 0xFFFF) & s;\
	vflag = cflag = 0

#define XOR(r,s) \
	result = r ^ s;\
	vflag = cflag = 0

#define IXOR(r,s) \
	result = (r ^ s) ^ 0xFFFF;\
	vflag = cflag = 0


#define DEST0 \
	Q = Y = zresult

#define DEST1 \
	Y = zresult

#define DEST2 \
	Y = *curop->areg;\
	*curop->breg = zresult

#define DEST3 \
	*curop->breg = zresult;\
	Y = zresult

#define DEST4_NOSHIFT \
	*curop->breg = (zresult >> 1) | ((curop->flags & 0x20) << 10);\
	Q = (Q >> 1) | ((curop->flags & 0x20) << 10);\
	Y = zresult

#define DEST4_SHIFT \
	*curop->breg = (zresult >> 1) | ((nflag ^ vflag) << 15);\
	Q = (Q >> 1) | ((zresult & 0x01) << 15);\
	Y = zresult

#define DEST5_NOSHIFT \
	*curop->breg = (zresult >> 1) | ((curop->flags & 0x20) << 10);\
	Y = zresult

#define DEST5_SHIFT \
	*curop->breg = (zresult >> 1) | ((nflag ^ vflag) << 15);\
	Y = zresult

#define DEST6_NOSHIFT \
	*curop->breg = zresult << 1;\
	Q = ((Q << 1) & 0xffff) | (nflag ^ 1);\
	Y = zresult

#define DEST6_SHIFT \
	*curop->breg = (zresult << 1) | ((Q & 0x8000) >> 15);\
	Q = (Q << 1) & 0xffff;\
	Y = zresult

#define DEST7_NOSHIFT \
	*curop->breg = zresult << 1;\
	Y = zresult

#define DEST7_SHIFT \
	*curop->breg = (zresult << 1) | ((Q & 0x8000) >> 15);\
	Y = zresult


#define JUMP0   curop++;
#define JUMP1   if (cflag) curop = curop->nxtop; else curop++;
#define JUMP2   if (!zresult) curop = curop->nxtop; else curop++;
#define JUMP3   if (!nflag) curop = curop->nxtop; else curop++;
#define JUMP4   if (nflag) curop = curop->nxtop; else curop++;
#define JUMP5   curop = curop->nxtop;
#define JUMP6   m_irmb_stack[SP] = curop + 1; SP = (SP + 1) & 15; curop = curop->nxtop;
#define JUMP7   SP = (SP - 1) & 15; curop = m_irmb_stack[SP];


/* Run mathbox */
void irobot_state::irmb_run()
{
	const irmb_ops *prevop = &m_mbops[0];
	const irmb_ops *curop = &m_mbops[0];

	UINT32 Q = 0;
	UINT32 Y = 0;
	UINT32 nflag = 0;
	UINT32 vflag = 0;
	UINT32 cflag = 0;
	UINT32 zresult = 1;
	UINT32 CI = 0;
	UINT32 SP = 0;
	UINT32 icount = 0;

	g_profiler.start(PROFILER_USER1);

	while ((prevop->flags & (FL_DPSEL | FL_carry)) != (FL_DPSEL | FL_carry))
	{
		UINT32 result;
		UINT32 fu;
		UINT32 tmp;

		icount += curop->cycles;

		/* Get function code */
		fu = curop->func;

		/* Modify function for MULT */
		if (!(prevop->flags & FL_MULT) || (Q & 1))
			fu = fu ^ 0x02;
		else
			fu = fu | 0x02;

		/* Modify function for DIV */
		if ((prevop->flags & FL_DIV) || nflag)
			fu = fu ^ 0x08;
		else
			fu = fu | 0x08;

		/* Do source and operation */
		switch (fu & 0x03f)
		{
			case 0x00:  ADD(*curop->areg, Q);                               break;
			case 0x01:  ADD(*curop->areg, *curop->breg);                    break;
			case 0x02:  ADD(0, Q);                                          break;
			case 0x03:  ADD(0, *curop->breg);                               break;
			case 0x04:  ADD(0, *curop->areg);                               break;
			case 0x05:  tmp = irmb_din(curop); ADD(tmp, *curop->areg);       break;
			case 0x06:  tmp = irmb_din(curop); ADD(tmp, Q);                  break;
			case 0x07:  tmp = irmb_din(curop); ADD(tmp, 0);                  break;
			case 0x08:  SUBR(*curop->areg, Q);                              break;
			case 0x09:  SUBR(*curop->areg, *curop->breg);                   break;
			case 0x0a:  SUBR(0, Q);                                         break;
			case 0x0b:  SUBR(0, *curop->breg);                              break;
			case 0x0c:  SUBR(0, *curop->areg);                              break;
			case 0x0d:  tmp = irmb_din(curop); SUBR(tmp, *curop->areg);      break;
			case 0x0e:  tmp = irmb_din(curop); SUBR(tmp, Q);             break;
			case 0x0f:  tmp = irmb_din(curop); SUBR(tmp, 0);             break;
			case 0x10:  SUB(*curop->areg, Q);                               break;
			case 0x11:  SUB(*curop->areg, *curop->breg);                    break;
			case 0x12:  SUB(0, Q);                                          break;
			case 0x13:  SUB(0, *curop->breg);                               break;
			case 0x14:  SUB(0, *curop->areg);                               break;
			case 0x15:  tmp = irmb_din(curop); SUB(tmp, *curop->areg);       break;
			case 0x16:  tmp = irmb_din(curop); SUB(tmp, Q);                  break;
			case 0x17:  tmp = irmb_din(curop); SUB(tmp, 0);                  break;
			case 0x18:  OR(*curop->areg, Q);                                break;
			case 0x19:  OR(*curop->areg, *curop->breg);                     break;
			case 0x1a:  OR(0, Q);                                           break;
			case 0x1b:  OR(0, *curop->breg);                                break;
			case 0x1c:  OR(0, *curop->areg);                                break;
			case 0x1d:  OR(irmb_din(curop), *curop->areg);                   break;
			case 0x1e:  OR(irmb_din(curop), Q);                              break;
			case 0x1f:  OR(irmb_din(curop), 0);                              break;
			case 0x20:  AND(*curop->areg, Q);                               break;
			case 0x21:  AND(*curop->areg, *curop->breg);                    break;
			case 0x22:  AND(0, Q);                                          break;
			case 0x23:  AND(0, *curop->breg);                               break;
			case 0x24:  AND(0, *curop->areg);                               break;
			case 0x25:  AND(irmb_din(curop), *curop->areg);                  break;
			case 0x26:  AND(irmb_din(curop), Q);                         break;
			case 0x27:  AND(irmb_din(curop), 0);                         break;
			case 0x28:  IAND(*curop->areg, Q);                              break;
			case 0x29:  IAND(*curop->areg, *curop->breg);                   break;
			case 0x2a:  IAND(0, Q);                                         break;
			case 0x2b:  IAND(0, *curop->breg);                              break;
			case 0x2c:  IAND(0, *curop->areg);                              break;
			case 0x2d:  IAND(irmb_din(curop), *curop->areg);             break;
			case 0x2e:  IAND(irmb_din(curop), Q);                            break;
			case 0x2f:  IAND(irmb_din(curop), 0);                            break;
			case 0x30:  XOR(*curop->areg, Q);                               break;
			case 0x31:  XOR(*curop->areg, *curop->breg);                    break;
			case 0x32:  XOR(0, Q);                                          break;
			case 0x33:  XOR(0, *curop->breg);                               break;
			case 0x34:  XOR(0, *curop->areg);                               break;
			case 0x35:  XOR(irmb_din(curop), *curop->areg);                  break;
			case 0x36:  XOR(irmb_din(curop), Q);                         break;
			case 0x37:  XOR(irmb_din(curop), 0);                         break;
			case 0x38:  IXOR(*curop->areg, Q);                              break;
			case 0x39:  IXOR(*curop->areg, *curop->breg);                   break;
			case 0x3a:  IXOR(0, Q);                                         break;
			case 0x3b:  IXOR(0, *curop->breg);                              break;
			case 0x3c:  IXOR(0, *curop->areg);                              break;
			case 0x3d:  IXOR(irmb_din(curop), *curop->areg);             break;
			case 0x3e:  IXOR(irmb_din(curop), Q);                            break;
default:    case 0x3f:  IXOR(irmb_din(curop), 0);                            break;
		}

		/* Evaluate flags */
		zresult = result & 0xFFFF;
		nflag = zresult >> 15;

		prevop = curop;

		/* Do destination and jump */
		switch (fu >> 6)
		{
			case 0x00:
			case 0x08:  DEST0;          JUMP0;  break;
			case 0x01:
			case 0x09:  DEST1;          JUMP0;  break;
			case 0x02:
			case 0x0a:  DEST2;          JUMP0;  break;
			case 0x03:
			case 0x0b:  DEST3;          JUMP0;  break;
			case 0x04:  DEST4_NOSHIFT;  JUMP0;  break;
			case 0x05:  DEST5_NOSHIFT;  JUMP0;  break;
			case 0x06:  DEST6_NOSHIFT;  JUMP0;  break;
			case 0x07:  DEST7_NOSHIFT;  JUMP0;  break;
			case 0x0c:  DEST4_SHIFT;    JUMP0;  break;
			case 0x0d:  DEST5_SHIFT;    JUMP0;  break;
			case 0x0e:  DEST6_SHIFT;    JUMP0;  break;
			case 0x0f:  DEST7_SHIFT;    JUMP0;  break;

			case 0x10:
			case 0x18:  DEST0;          JUMP1;  break;
			case 0x11:
			case 0x19:  DEST1;          JUMP1;  break;
			case 0x12:
			case 0x1a:  DEST2;          JUMP1;  break;
			case 0x13:
			case 0x1b:  DEST3;          JUMP1;  break;
			case 0x14:  DEST4_NOSHIFT;  JUMP1;  break;
			case 0x15:  DEST5_NOSHIFT;  JUMP1;  break;
			case 0x16:  DEST6_NOSHIFT;  JUMP1;  break;
			case 0x17:  DEST7_NOSHIFT;  JUMP1;  break;
			case 0x1c:  DEST4_SHIFT;    JUMP1;  break;
			case 0x1d:  DEST5_SHIFT;    JUMP1;  break;
			case 0x1e:  DEST6_SHIFT;    JUMP1;  break;
			case 0x1f:  DEST7_SHIFT;    JUMP1;  break;

			case 0x20:
			case 0x28:  DEST0;          JUMP2;  break;
			case 0x21:
			case 0x29:  DEST1;          JUMP2;  break;
			case 0x22:
			case 0x2a:  DEST2;          JUMP2;  break;
			case 0x23:
			case 0x2b:  DEST3;          JUMP2;  break;
			case 0x24:  DEST4_NOSHIFT;  JUMP2;  break;
			case 0x25:  DEST5_NOSHIFT;  JUMP2;  break;
			case 0x26:  DEST6_NOSHIFT;  JUMP2;  break;
			case 0x27:  DEST7_NOSHIFT;  JUMP2;  break;
			case 0x2c:  DEST4_SHIFT;    JUMP2;  break;
			case 0x2d:  DEST5_SHIFT;    JUMP2;  break;
			case 0x2e:  DEST6_SHIFT;    JUMP2;  break;
			case 0x2f:  DEST7_SHIFT;    JUMP2;  break;

			case 0x30:
			case 0x38:  DEST0;          JUMP3;  break;
			case 0x31:
			case 0x39:  DEST1;          JUMP3;  break;
			case 0x32:
			case 0x3a:  DEST2;          JUMP3;  break;
			case 0x33:
			case 0x3b:  DEST3;          JUMP3;  break;
			case 0x34:  DEST4_NOSHIFT;  JUMP3;  break;
			case 0x35:  DEST5_NOSHIFT;  JUMP3;  break;
			case 0x36:  DEST6_NOSHIFT;  JUMP3;  break;
			case 0x37:  DEST7_NOSHIFT;  JUMP3;  break;
			case 0x3c:  DEST4_SHIFT;    JUMP3;  break;
			case 0x3d:  DEST5_SHIFT;    JUMP3;  break;
			case 0x3e:  DEST6_SHIFT;    JUMP3;  break;
			case 0x3f:  DEST7_SHIFT;    JUMP3;  break;

			case 0x40:
			case 0x48:  DEST0;          JUMP4;  break;
			case 0x41:
			case 0x49:  DEST1;          JUMP4;  break;
			case 0x42:
			case 0x4a:  DEST2;          JUMP4;  break;
			case 0x43:
			case 0x4b:  DEST3;          JUMP4;  break;
			case 0x44:  DEST4_NOSHIFT;  JUMP4;  break;
			case 0x45:  DEST5_NOSHIFT;  JUMP4;  break;
			case 0x46:  DEST6_NOSHIFT;  JUMP4;  break;
			case 0x47:  DEST7_NOSHIFT;  JUMP4;  break;
			case 0x4c:  DEST4_SHIFT;    JUMP4;  break;
			case 0x4d:  DEST5_SHIFT;    JUMP4;  break;
			case 0x4e:  DEST6_SHIFT;    JUMP4;  break;
			case 0x4f:  DEST7_SHIFT;    JUMP4;  break;

			case 0x50:
			case 0x58:  DEST0;          JUMP5;  break;
			case 0x51:
			case 0x59:  DEST1;          JUMP5;  break;
			case 0x52:
			case 0x5a:  DEST2;          JUMP5;  break;
			case 0x53:
			case 0x5b:  DEST3;          JUMP5;  break;
			case 0x54:  DEST4_NOSHIFT;  JUMP5;  break;
			case 0x55:  DEST5_NOSHIFT;  JUMP5;  break;
			case 0x56:  DEST6_NOSHIFT;  JUMP5;  break;
			case 0x57:  DEST7_NOSHIFT;  JUMP5;  break;
			case 0x5c:  DEST4_SHIFT;    JUMP5;  break;
			case 0x5d:  DEST5_SHIFT;    JUMP5;  break;
			case 0x5e:  DEST6_SHIFT;    JUMP5;  break;
			case 0x5f:  DEST7_SHIFT;    JUMP5;  break;

			case 0x60:
			case 0x68:  DEST0;          JUMP6;  break;
			case 0x61:
			case 0x69:  DEST1;          JUMP6;  break;
			case 0x62:
			case 0x6a:  DEST2;          JUMP6;  break;
			case 0x63:
			case 0x6b:  DEST3;          JUMP6;  break;
			case 0x64:  DEST4_NOSHIFT;  JUMP6;  break;
			case 0x65:  DEST5_NOSHIFT;  JUMP6;  break;
			case 0x66:  DEST6_NOSHIFT;  JUMP6;  break;
			case 0x67:  DEST7_NOSHIFT;  JUMP6;  break;
			case 0x6c:  DEST4_SHIFT;    JUMP6;  break;
			case 0x6d:  DEST5_SHIFT;    JUMP6;  break;
			case 0x6e:  DEST6_SHIFT;    JUMP6;  break;
			case 0x6f:  DEST7_SHIFT;    JUMP6;  break;

			case 0x70:
			case 0x78:  DEST0;          JUMP7;  break;
			case 0x71:
			case 0x79:  DEST1;          JUMP7;  break;
			case 0x72:
			case 0x7a:  DEST2;          JUMP7;  break;
			case 0x73:
			case 0x7b:  DEST3;          JUMP7;  break;
			case 0x74:  DEST4_NOSHIFT;  JUMP7;  break;
			case 0x75:  DEST5_NOSHIFT;  JUMP7;  break;
			case 0x76:  DEST6_NOSHIFT;  JUMP7;  break;
			case 0x77:  DEST7_NOSHIFT;  JUMP7;  break;
			case 0x7c:  DEST4_SHIFT;    JUMP7;  break;
			case 0x7d:  DEST5_SHIFT;    JUMP7;  break;
			case 0x7e:  DEST6_SHIFT;    JUMP7;  break;
			case 0x7f:  DEST7_SHIFT;    JUMP7;  break;
		}

		/* Do write */
		if (!(prevop->flags & FL_MBRW))
			irmb_dout(prevop, Y);

		/* ADDEN */
		if (!(prevop->flags & FL_ADDEN))
		{
			if (prevop->flags & FL_MBRW)
				m_irmb_latch = irmb_din(prevop);
			else
				m_irmb_latch = Y;
		}
	}
	g_profiler.stop();

	logerror("%d instructions for Mathbox \n", icount);


#if IR_TIMING
	if (m_irmb_running == 0)
	{
		m_irmb_timer->adjust(attotime::from_hz(12000000) * icount);
		logerror("mb start ");
		IR_CPU_STATE();
	}
	else
	{
		logerror("mb start [busy!] ");
		IR_CPU_STATE();
		m_irmb_timer->adjust(attotime::from_hz(200) * icount);
	}
#else
	m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
#endif
	m_irmb_running=1;
}




#if DISASSEMBLE_MB_ROM
static void disassemble_instruction(irmb_ops *op)
{
	int lp;

	if (i==0)
		logerror(" Address  a b func stor: Q :Y, R, S RDCSAESM da m rs\n");
	logerror("%04X    : ",i);
	logerror("%X ",op->areg);
	logerror("%X ",op->breg);

	lp=(op->func & 0x38)>>3;
	if ((lp&1)==0)
		lp|=1;
	else if((op->flags & FL_DIV) != 0)
		lp&=6;
	else
		logerror("*");

	switch (lp)
	{
		case 0:
			logerror("ADD  ");
			break;
		case 1:
			logerror("SUBR ");
			break;
		case 2:
			logerror("SUB  ");
			break;
		case 3:
			logerror("OR   ");
			break;
		case 4:
			logerror("AND  ");
			break;
		case 5:
			logerror("AND  ");
			break;
		case 6:
			logerror("XOR  ");
			break;
		case 7:
			logerror("XNOR ");
			break;
	}

	switch ((op->func & 0x1c0)>>6)
	{
		case 0:
			logerror("  - : Q :F,");
			break;
		case 1:
			logerror("  - : - :F,");
			break;
		case 2:
			logerror("  R%x: - :A,",op->breg);
			break;
		case 3:
			logerror("  R%x: - :F,",op->breg);
			break;
		case 4:
			logerror(">>R%x:>>Q:F,",op->breg);
			break;
		case 5:
			logerror(">>R%x: - :F,",op->breg);
			break;
		case 6:
			logerror("<<R%x:<<Q:F,",op->breg);
			break;
		case 7:
			logerror("<<R%x: - :F,",op->breg);
			break;
	}

	lp=(op->func & 0x7);
	if ((lp&2)==0)
		lp|=2;
	else if((op->flags & FL_MULT) == 0)
		lp&=5;
	else
		logerror("*");

	switch (lp)
	{
		case 0:
			logerror("R%x, Q ",op->areg);
			break;
		case 1:
			logerror("R%x,R%x ",op->areg,op->breg);
			break;
		case 2:
			logerror("00, Q ");
			break;
		case 3:
			logerror("00,R%x ",op->breg);
			break;
		case 4:
			logerror("00,R%x ",op->areg);
			break;
		case 5:
			logerror(" D,R%x ",op->areg);
			break;
		case 6:
			logerror(" D, Q ");
			break;
		case 7:
			logerror(" D,00 ");
			break;
	}

	for (lp=0;lp<8;lp++)
		if (op->flags & (0x80>>lp))
			logerror("1");
		else
			logerror("0");

	logerror(" %02X ",op->diradd);
	logerror("%X\n",op->ramsel);
	if (op->jtype)
	{
		logerror("              ");
		switch (op->jtype)
		{
			case 1:
				logerror("BO ");
				break;
			case 2:
				logerror("BZ ");
				break;
			case 3:
				logerror("BH ");
				break;
			case 4:
				logerror("BL ");
				break;
			case 5:
				logerror("B  ");
				break;
			case 6:
				logerror("Cl ");
				break;
			case 7:
				logerror("Return\n\n");
				break;
		}
		if (op->jtype != 7) logerror("  %04X    \n",op->nxtadd);
		if (op->jtype == 5) logerror("\n");
	}
}
#endif
