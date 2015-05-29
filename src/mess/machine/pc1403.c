// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"
#include "cpu/sc61860/sc61860.h"

#include "includes/pocketc.h"
#include "includes/pc1403.h"
#include "machine/ram.h"

/* C-CE while reset, program will not be destroyed! */




/*
   port 2:
     bits 0,1: external rom a14,a15 lines
   port 3:
     bits 0..6 keyboard output select matrix line
*/

WRITE8_MEMBER(pc1403_state::pc1403_asic_write)
{
	m_asic[offset>>9]=data;
	switch( (offset>>9) ){
		case 0/*0x3800*/:
				// output
				logerror ("asic write %.4x %.2x\n",offset, data);
				break;
		case 1/*0x3a00*/:
				logerror ("asic write %.4x %.2x\n",offset, data);
				break;
		case 2/*0x3c00*/:
				membank("bank1")->set_base(memregion("user1")->base()+((data&7)<<14));
				logerror ("asic write %.4x %.2x\n",offset, data);
				break;
		case 3/*0x3e00*/: break;
	}
}

READ8_MEMBER(pc1403_state::pc1403_asic_read)
{
	UINT8 data=m_asic[offset>>9];
	switch( (offset>>9) ){
		case 0: case 1: case 2:
			logerror ("asic read %.4x %.2x\n",offset, data);
			break;
	}
	return data;
}

WRITE8_MEMBER(pc1403_state::pc1403_outa)
{
	m_outa=data;
}

READ8_MEMBER(pc1403_state::pc1403_ina)
{
	UINT8 data=m_outa;

	if (m_asic[3] & 0x01)
		data |= ioport("KEY0")->read();

	if (m_asic[3] & 0x02)
		data |= ioport("KEY1")->read();

	if (m_asic[3] & 0x04)
		data |= ioport("KEY2")->read();

	if (m_asic[3] & 0x08)
		data |= ioport("KEY3")->read();

	if (m_asic[3] & 0x10)
		data |= ioport("KEY4")->read();

	if (m_asic[3] & 0x20)
		data |= ioport("KEY5")->read();

	if (m_asic[3] & 0x40)
		data |= ioport("KEY6")->read();

	if (m_outa & 0x01)
	{
		data |= ioport("KEY7")->read();

		/* At Power Up we fake a 'C-CE' pressure */
		if (m_power)
			data |= 0x02;
	}

	if (m_outa & 0x02)
		data |= ioport("KEY8")->read();

	if (m_outa & 0x04)
		data |= ioport("KEY9")->read();

	if (m_outa & 0x08)
		data |= ioport("KEY10")->read();

	if (m_outa & 0x10)
		data |= ioport("KEY11")->read();

	if (m_outa & 0x20)
		data |= ioport("KEY12")->read();

	if (m_outa & 0x40)
		data |= ioport("KEY13")->read();

	return data;
}

#if 0
READ8_MEMBER(pc1403_state::pc1403_inb)
{
	int data = m_outb;

	if (ioport("KEY13")->read())
		data |= 1;

	return data;
}
#endif

WRITE8_MEMBER(pc1403_state::pc1403_outc)
{
	m_portc = data;
//    logerror("%g pc %.4x outc %.2x\n", device->machine().time().as_double(), device->m_maincpu->safe_pc(), data);
}


READ_LINE_MEMBER(pc1403_state::pc1403_brk)
{
	return (ioport("EXTRA")->read() & 0x01);
}

READ_LINE_MEMBER(pc1403_state::pc1403_reset)
{
	return (ioport("EXTRA")->read() & 0x02);
}

void pc1403_state::machine_start()
{
	UINT8 *ram = memregion("maincpu")->base() + 0x8000;
	UINT8 *cpu = m_maincpu->internal_ram();

	machine().device<nvram_device>("cpu_nvram")->set_base(cpu, 96);
	machine().device<nvram_device>("ram_nvram")->set_base(ram, 0x8000);
}

void pc1403_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_POWER_UP:
		m_power=0;
		break;
	default:
		assert_always(FALSE, "Unknown id in pc1403_state::device_timer");
	}
}

DRIVER_INIT_MEMBER(pc1403_state,pc1403)
{
	int i;
	UINT8 *gfx=memregion("gfx1")->base();

	for (i=0; i<128; i++) gfx[i]=i;

	m_power = 1;
	timer_set(attotime::from_seconds(1), TIMER_POWER_UP);

	membank("bank1")->set_base(memregion("user1")->base());
}
