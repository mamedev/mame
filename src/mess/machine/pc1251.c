#include "emu.h"
#include "cpu/sc61860/sc61860.h"

#include "includes/pocketc.h"
#include "includes/pc1251.h"

/* C-CE while reset, program will not be destroyed! */



void pc1251_outa(device_t *device, int data)
{
	pc1251_state *state = device->machine().driver_data<pc1251_state>();
	state->m_outa = data;
}

void pc1251_outb(device_t *device, int data)
{
	pc1251_state *state = device->machine().driver_data<pc1251_state>();
	state->m_outb = data;
}

void pc1251_outc(device_t *device, int data)
{
}

int pc1251_ina(device_t *device)
{
	pc1251_state *state = device->machine().driver_data<pc1251_state>();
	int data = state->m_outa;
	running_machine &machine = device->machine();

	if (state->m_outb & 0x01)
	{
		data |= machine.root_device().ioport("KEY0")->read();

		/* At Power Up we fake a 'CL' pressure */
		if (state->m_power)
			data |= 0x02;		// problem with the deg lcd
	}

	if (state->m_outb & 0x02)
		data |= machine.root_device().ioport("KEY1")->read();

	if (state->m_outb & 0x04)
		data |= machine.root_device().ioport("KEY2")->read();

	if (state->m_outa & 0x01)
		data |= machine.root_device().ioport("KEY3")->read();

	if (state->m_outa & 0x02)
		data |= machine.root_device().ioport("KEY4")->read();

	if (state->m_outa & 0x04)
		data |= machine.root_device().ioport("KEY5")->read();

	if (state->m_outa & 0x08)
		data |= machine.root_device().ioport("KEY6")->read();

	if (state->m_outa & 0x10)
		data |= machine.root_device().ioport("KEY7")->read();

	if (state->m_outa & 0x20)
		data |= machine.root_device().ioport("KEY8")->read();

	if (state->m_outa & 0x40)
		data |= machine.root_device().ioport("KEY9")->read();

	return data;
}

int pc1251_inb(device_t *device)
{
	pc1251_state *state = device->machine().driver_data<pc1251_state>();
	int data = state->m_outb;

	if (state->m_outb & 0x08)
		data |= (state->ioport("MODE")->read() & 0x07);

	return data;
}

int pc1251_brk(device_t *device)
{
	return (device->machine().root_device().ioport("EXTRA")->read() & 0x01);
}

int pc1251_reset(device_t *device)
{
	return (device->machine().root_device().ioport("EXTRA")->read() & 0x02);
}

MACHINE_START( pc1251 )
{
	device_t *main_cpu = machine.device("maincpu");
	UINT8 *ram = machine.root_device().memregion("maincpu")->base() + 0x8000;
	UINT8 *cpu = sc61860_internal_ram(main_cpu);

	machine.device<nvram_device>("cpu_nvram")->set_base(cpu, 96);
	machine.device<nvram_device>("ram_nvram")->set_base(ram, 0x4800);
}

TIMER_CALLBACK_MEMBER(pc1251_state::pc1251_power_up)
{
	m_power = 0;
}

DRIVER_INIT_MEMBER(pc1251_state,pc1251)
{
	int i;
	UINT8 *gfx = memregion("gfx1")->base();
	for (i=0; i<128; i++) gfx[i]=i;

	m_power = 1;
	machine().scheduler().timer_set(attotime::from_seconds(1), timer_expired_delegate(FUNC(pc1251_state::pc1251_power_up),this));
}

