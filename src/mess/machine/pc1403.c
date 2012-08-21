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

WRITE8_HANDLER(pc1403_asic_write)
{
	pc1403_state *state = space->machine().driver_data<pc1403_state>();
    state->m_asic[offset>>9]=data;
    switch( (offset>>9) ){
    case 0/*0x3800*/:
	// output
	logerror ("asic write %.4x %.2x\n",offset, data);
	break;
    case 1/*0x3a00*/:
	logerror ("asic write %.4x %.2x\n",offset, data);
	break;
    case 2/*0x3c00*/:
	state->membank("bank1")->set_base(state->memregion("user1")->base()+((data&7)<<14));
	logerror ("asic write %.4x %.2x\n",offset, data);
	break;
    case 3/*0x3e00*/: break;
    }
}

READ8_HANDLER(pc1403_asic_read)
{
	pc1403_state *state = space->machine().driver_data<pc1403_state>();
    UINT8 data=state->m_asic[offset>>9];
    switch( (offset>>9) ){
    case 0: case 1: case 2:
	logerror ("asic read %.4x %.2x\n",offset, data);
	break;
    }
    return data;
}

void pc1403_outa(device_t *device, int data)
{
	pc1403_state *state = device->machine().driver_data<pc1403_state>();
    state->m_outa=data;
}

int pc1403_ina(device_t *device)
{
	pc1403_state *state = device->machine().driver_data<pc1403_state>();
    UINT8 data=state->m_outa;

    if (state->m_asic[3] & 0x01)
		data |= device->machine().root_device().ioport("KEY0")->read();

    if (state->m_asic[3] & 0x02)
		data |= device->machine().root_device().ioport("KEY1")->read();

    if (state->m_asic[3] & 0x04)
		data |= device->machine().root_device().ioport("KEY2")->read();

    if (state->m_asic[3] & 0x08)
		data |= device->machine().root_device().ioport("KEY3")->read();

    if (state->m_asic[3] & 0x10)
		data |= device->machine().root_device().ioport("KEY4")->read();

    if (state->m_asic[3] & 0x20)
		data |= device->machine().root_device().ioport("KEY5")->read();

    if (state->m_asic[3] & 0x40)
		data |= device->machine().root_device().ioport("KEY6")->read();

    if (state->m_outa & 0x01)
	{
		data |= state->ioport("KEY7")->read();

		/* At Power Up we fake a 'C-CE' pressure */
		if (state->m_power)
			data |= 0x02;
	}

    if (state->m_outa & 0x02)
		data |= device->machine().root_device().ioport("KEY8")->read();

    if (state->m_outa & 0x04)
		data |= device->machine().root_device().ioport("KEY9")->read();

    if (state->m_outa & 0x08)
		data |= device->machine().root_device().ioport("KEY10")->read();

    if (state->m_outa & 0x10)
		data |= device->machine().root_device().ioport("KEY11")->read();

    if (state->m_outa & 0x20)
		data |= device->machine().root_device().ioport("KEY12")->read();

    if (state->m_outa & 0x40)
		data |= device->machine().root_device().ioport("KEY13")->read();

    return data;
}

#if 0
int pc1403_inb(void)
{
	pc1403_state *state = machine.driver_data<pc140_state>();
	int data = state->m_outb;

	if (machine.root_device().ioport("KEY13")->read())
		data |= 1;

	return data;
}
#endif

void pc1403_outc(device_t *device, int data)
{
	pc1403_state *state = device->machine().driver_data<pc1403_state>();
    state->m_portc = data;
//    logerror("%g pc %.4x outc %.2x\n", device->machine().time().as_double(), cpu_get_pc(device->machine().device("maincpu")), data);
}


int pc1403_brk(device_t *device)
{
	return (device->machine().root_device().ioport("EXTRA")->read() & 0x01);
}

int pc1403_reset(device_t *device)
{
	return (device->machine().root_device().ioport("EXTRA")->read() & 0x02);
}

MACHINE_START( pc1403 )
{
	device_t *main_cpu = machine.device("maincpu");
	UINT8 *ram = machine.root_device().memregion("maincpu")->base() + 0x8000;
	UINT8 *cpu = sc61860_internal_ram(main_cpu);

	machine.device<nvram_device>("cpu_nvram")->set_base(cpu, 96);
	machine.device<nvram_device>("ram_nvram")->set_base(ram, 0x8000);
}

static TIMER_CALLBACK(pc1403_power_up)
{
	pc1403_state *state = machine.driver_data<pc1403_state>();
	state->m_power=0;
}

DRIVER_INIT_MEMBER(pc1403_state,pc1403)
{
	int i;
	UINT8 *gfx=machine().root_device().memregion("gfx1")->base();

	for (i=0; i<128; i++) gfx[i]=i;

	m_power = 1;
	machine().scheduler().timer_set(attotime::from_seconds(1), FUNC(pc1403_power_up));

	membank("bank1")->set_base(memregion("user1")->base());
}
