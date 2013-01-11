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



void pc1401_outa(device_t *device, int data)
{
	pc1401_state *state = device->machine().driver_data<pc1401_state>();
	state->m_outa = data;
}

void pc1401_outb(device_t *device, int data)
{
	pc1401_state *state = device->machine().driver_data<pc1401_state>();
	state->m_outb = data;
}

void pc1401_outc(device_t *device, int data)
{
	pc1401_state *state = device->machine().driver_data<pc1401_state>();
	//logerror("%g outc %.2x\n", machine.time().as_double(), data);
	state->m_portc = data;
}

int pc1401_ina(device_t *device)
{
	pc1401_state *state = device->machine().driver_data<pc1401_state>();
	int data = state->m_outa;

	if (state->m_outb & 0x01)
		data |= device->machine().root_device().ioport("KEY0")->read();

	if (state->m_outb & 0x02)
		data |= device->machine().root_device().ioport("KEY1")->read();

	if (state->m_outb & 0x04)
		data |= device->machine().root_device().ioport("KEY2")->read();

	if (state->m_outb & 0x08)
		data |= device->machine().root_device().ioport("KEY3")->read();

	if (state->m_outb & 0x10)
		data |= device->machine().root_device().ioport("KEY4")->read();

	if (state->m_outb & 0x20)
	{
		data |= state->ioport("KEY5")->read();

		/* At Power Up we fake a 'C-CE' pressure */
		if (state->m_power)
			data |= 0x01;
	}

	if (state->m_outa & 0x01)
		data |= device->machine().root_device().ioport("KEY6")->read();

	if (state->m_outa & 0x02)
		data |= device->machine().root_device().ioport("KEY7")->read();

	if (state->m_outa & 0x04)
		data |= device->machine().root_device().ioport("KEY8")->read();

	if (state->m_outa & 0x08)
		data |= device->machine().root_device().ioport("KEY9")->read();

	if (state->m_outa & 0x10)
		data |= device->machine().root_device().ioport("KEY10")->read();

	if (state->m_outa & 0x20)
		data |= device->machine().root_device().ioport("KEY11")->read();

	if (state->m_outa & 0x40)
		data |= device->machine().root_device().ioport("KEY12")->read();

	return data;
}

int pc1401_inb(device_t *device)
{
	pc1401_state *state = device->machine().driver_data<pc1401_state>();
	int data=state->m_outb;

	if (state->ioport("EXTRA")->read() & 0x04)
		data |= 0x01;

	return data;
}

int pc1401_brk(device_t *device)
{
	return (device->machine().root_device().ioport("EXTRA")->read() & 0x01);
}

int pc1401_reset(device_t *device)
{
	return (device->machine().root_device().ioport("EXTRA")->read() & 0x02);
}

MACHINE_START( pc1401 )
{
	device_t *main_cpu = machine.device("maincpu");
	UINT8 *ram = machine.root_device().memregion("maincpu")->base() + 0x2000;
	UINT8 *cpu = sc61860_internal_ram(main_cpu);

	machine.device<nvram_device>("cpu_nvram")->set_base(cpu, 96);
	machine.device<nvram_device>("ram_nvram")->set_base(ram, 0x2800);
}

TIMER_CALLBACK_MEMBER(pc1401_state::pc1401_power_up)
{
	m_power = 0;
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
	machine().scheduler().timer_set(attotime::from_seconds(1), timer_expired_delegate(FUNC(pc1401_state::pc1401_power_up),this));
}
