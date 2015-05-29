// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"
#include "cpu/sc61860/sc61860.h"

#include "includes/pocketc.h"
#include "includes/pc1401.h"
#include "machine/ram.h"

/* C-CE while reset, program will not be destroyed! */

/* error codes
1 syntax error
2 calculation error
3 illegal function argument
4 too large a line number
5 next without for
  return without gosub
6 memory overflow
7 print using error
8 i/o device error
9 other errors*/



WRITE8_MEMBER(pc1401_state::pc1401_outa)
{
	m_outa = data;
}

WRITE8_MEMBER(pc1401_state::pc1401_outb)
{
	m_outb = data;
}

WRITE8_MEMBER(pc1401_state::pc1401_outc)
{
	//logerror("%g outc %.2x\n", machine.time().as_double(), data);
	m_portc = data;
}

READ8_MEMBER(pc1401_state::pc1401_ina)
{
	int data = m_outa;

	if (m_outb & 0x01)
		data |= ioport("KEY0")->read();

	if (m_outb & 0x02)
		data |= ioport("KEY1")->read();

	if (m_outb & 0x04)
		data |= ioport("KEY2")->read();

	if (m_outb & 0x08)
		data |= ioport("KEY3")->read();

	if (m_outb & 0x10)
		data |= ioport("KEY4")->read();

	if (m_outb & 0x20)
	{
		data |= ioport("KEY5")->read();

		/* At Power Up we fake a 'C-CE' pressure */
		if (m_power)
			data |= 0x01;
	}

	if (m_outa & 0x01)
		data |= ioport("KEY6")->read();

	if (m_outa & 0x02)
		data |= ioport("KEY7")->read();

	if (m_outa & 0x04)
		data |= ioport("KEY8")->read();

	if (m_outa & 0x08)
		data |= ioport("KEY9")->read();

	if (m_outa & 0x10)
		data |= ioport("KEY10")->read();

	if (m_outa & 0x20)
		data |= ioport("KEY11")->read();

	if (m_outa & 0x40)
		data |= ioport("KEY12")->read();

	return data;
}

READ8_MEMBER(pc1401_state::pc1401_inb)
{
	int data=m_outb;

	if (ioport("EXTRA")->read() & 0x04)
		data |= 0x01;

	return data;
}

READ_LINE_MEMBER(pc1401_state::pc1401_brk)
{
	return (ioport("EXTRA")->read() & 0x01);
}

READ_LINE_MEMBER(pc1401_state::pc1401_reset)
{
	return (ioport("EXTRA")->read() & 0x02);
}

void pc1401_state::machine_start()
{
	UINT8 *ram = memregion("maincpu")->base() + 0x2000;
	UINT8 *cpu = m_maincpu->internal_ram();

	machine().device<nvram_device>("cpu_nvram")->set_base(cpu, 96);
	machine().device<nvram_device>("ram_nvram")->set_base(ram, 0x2800);
}

void pc1401_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_POWER_UP:
		m_power = 0;
		break;
	default:
		assert_always(FALSE, "Unknown id in pc1401_state::device_timer");
	}
}

DRIVER_INIT_MEMBER(pc1401_state,pc1401)
{
	int i;
	UINT8 *gfx=memregion("gfx1")->base();
#if 0
	static const char sucker[]={
		/* this routine dump the memory (start 0)
		   in an endless loop,
		   the pc side must be started before this
		   its here to allow verification of the decimal data
		   in mame disassembler
		*/
#if 1
		18,4,/*lip xl */
		2,0,/*lia 0 startaddress low */
		219,/*exam */
		18,5,/*lip xh */
		2,0,/*lia 0 startaddress high */
		219,/*exam */
/*400f x: */
		/* dump internal rom */
		18,5,/*lip 4 */
		89,/*ldm */
		218,/*exab */
		18,4,/*lip 5 */
		89,/*ldm */
		4,/*ix for increasing x */
		0,0,/*lii,0 */
		18,20,/*lip 20 */
		53, /* */
		18,20,/* lip 20 */
		219,/*exam */
#else
		18,4,/*lip xl */
		2,255,/*lia 0 */
		219,/*exam */
		18,5,/*lip xh */
		2,255,/*lia 0 */
		219,/*exam */
/*400f x: */
		/* dump external memory */
		4, /*ix */
		87,/*                ldd */
#endif
		218,/*exab */



		0,4,/*lii 4 */

		/*a: */
		218,/*                exab */
		90,/*                 sl */
		218,/*                exab */
		18,94,/*            lip 94 */
		96,252,/*                 anma 252 */
		2,2, /*lia 2 */
		196,/*                adcm */
		95,/*                 outf */
		/*b:  */
		204,/*inb */
		102,128,/*tsia 0x80 */
#if 0
		41,4,/*            jnzm b */
#else
		/* input not working reliable! */
		/* so handshake removed, PC side must run with disabled */
		/* interrupt to not lose data */
		78,20, /*wait 20 */
#endif

		218,/*                exab */
		90,/*                 sl */
		218,/*                exab */
		18,94,/*            lip 94 */
		96,252,/*anma 252 */
		2,0,/*                lia 0 */
		196,/*adcm */
		95,/*                 outf */
		/*c:  */
		204,/*inb */
		102,128,/*tsia 0x80 */
#if 0
		57,4,/*            jzm c */
#else
		78,20, /*wait 20 */
#endif

		65,/*deci */
		41,34,/*jnzm a */

		41,41,/*jnzm x: */

		55,/*               rtn */
	};

	for (i=0; i<sizeof(sucker);i++) pc1401_mem[0x4000+i]=sucker[i];
	logerror("%d %d\n",i, 0x4000+i);
#endif

	for (i=0; i<128; i++)
		gfx[i]=i;

	m_power = 1;
	timer_set(attotime::from_seconds(1), TIMER_POWER_UP);
}
