/*

    Scorpion 4 Hardware Platform (c)1996 Bell Fruit Manufacturing

    Skeleton Driver

    THIS DRIVER IS NOT WORKING

    -----------------

    Scorpion 4:::

    Main CPU is a MC68307FG16, present on Motherboard

    Configuration is SC4 motherboard + game card

    The game card contains the program roms, sound rom and YMZ280B

    Adder 4 video board adds an additional card with a MC68340PV25E (25.175Mhz)

    -------------------------------

    This file contains the hardware emulation, for the supported sets
    see bfm_sc4.c

*/



#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/bfm_sc45.h"
#include "sound/ymz280b.h"
#include "machine/68681.h"
#include "bfm_sc4.lh"
#include "machine/bfm_bd1.h"
#include "video/awpvid.h"
#include "machine/steppers.h" // stepper motor



class sc4_state : public driver_device
{
public:
	sc4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{
	}

	UINT16* m_cpuregion;
	UINT16* m_mainram;
	// devices
	device_t* m_duart;
	device_t* m_ymz;
	required_device<cpu_device> m_maincpu;

	// serial vfd
	int vfd_enabled;
	bool vfd_old_clock;

	UINT8 vfd_ser_value;
	int vfd_ser_count;

	int m_reel_changed;
	int m_reels;
	int m_reel12_latch;
	int m_reel3_latch;
	int m_reel4_latch;
	int m_reel56_latch;
	int m_optic_pattern;

	DECLARE_READ16_MEMBER(sc4_mem_r);
	DECLARE_WRITE16_MEMBER(sc4_mem_w);
};

class sc4_adder4_state : public sc4_state
{
public:
	sc4_adder4_state(const machine_config &mconfig, device_type type, const char *tag)
		: sc4_state(mconfig, type, tag),
		  m_adder4cpu(*this, "adder4")
	{ }

	// devices
	required_device<cpu_device> m_adder4cpu;
};

READ16_MEMBER(sc4_state::sc4_mem_r)
{
	int pc = cpu_get_pc(&space.device());
	int cs = m68307_get_cs(m_maincpu, offset * 2);
	int base = 0, end = 0, base2 = 0, end2 = 0;
//  if (!(debugger_access())) printf("cs is %d\n", cs);

	switch ( cs )
	{
		case 1:
			if (offset<0x100000/2)
				return m_cpuregion[offset];
			else
				logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d\n", pc, offset*2, mem_mask, cs);
			break;

		case 2:
			base = 0x800000/2;
			end = base + 0x10000 / 2;

			base2 = 0x810000/2;
			end2 = base2 + 0x10000 / 2;


			if ((offset>=base) && (offset<end))
			{
				offset-=base;
				return(m_mainram[offset]);
			}
			else if ((offset>=base2) && (offset<end2))
			{
				offset-=base2;

				switch (offset << 1)
				{
					case 0x0030:
						return 0x0000;

					case 0x0050:
						return 0x0000;

					case 0x0060:
						return 0x0000;

					case 0x0240:
						return 0xffff;

					case 0x02e0:
						return 0x0000;

					case 0x1000:
						return 0x0000;

					case 0x1010:
						return 0x0000;

					case 0x1020:
						return 0x0000;

					case 0x1040:
						return 0x0000;

					case 0x1244:
						return ymz280b_r(m_ymz,0);

					case 0x1246:
						return ymz280b_r(m_ymz,1);

					default:
						logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d (LAMPS etc.)\n", pc, offset*2, mem_mask, cs);
				}
			}
			else
			{
				logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d\n", pc, offset*2, mem_mask, cs);
			}
			break;

		case 3:
			base = 0xc00000/2;
			end = base + 0x20 / 2;

			if ((offset>=base) && (offset<end))
			{
				offset-=base;
				return duart68681_r(m_duart,offset);
			}
			else
			{
				logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d\n", pc, offset*2, mem_mask, cs);
				return 0x0000;
			}

			break;

		case 4:
			logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d\n", pc, offset*2, mem_mask, cs);
			return 0x0000;//0xffff;
			break;

		default:
			logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d (invalid?)\n", pc, offset*2, mem_mask, cs);

	}

	return 0x0000;
}

static WRITE8_HANDLER( bfm_sc4_reel4_w );


WRITE16_MEMBER(sc4_state::sc4_mem_w)
{
	int pc = cpu_get_pc(&space.device());
	int cs = m68307_get_cs(m_maincpu, offset * 2);
	int base = 0, end = 0, base2 = 0, end2 = 0;

	switch ( cs )
	{
		case 1:
			if (offset<0x100000/2)
				logerror("%08x maincpu write access offset %08x data %04x mem_mask %04x cs %d (ROM WRITE?!)\n", pc, offset*2, data, mem_mask, cs);
			else
				logerror("%08x maincpu write access offset %08x data %04x mem_mask %04x cs %d\n", pc, offset*2, data, mem_mask, cs);

			break;

		case 2:
			base = 0x800000/2;
			end = base + 0x10000 / 2;
			base2 = 0x810000/2;
			end2 = base2 + 0x10000 / 2;

			if ((offset>=base) && (offset<end))
			{
				offset-=base;
				COMBINE_DATA(&m_mainram[offset]);
			}
			else if ((offset>=base2) && (offset<end2))
			{
				offset-=base2;

				switch (offset << 1)
				{
					case 0x1248:
						ymz280b_w(m_ymz,0, data & 0xff);
						break;

					case 0x124a:
						ymz280b_w(m_ymz,1, data & 0xff);
						break;

					case 0x1330:
						bfm_sc4_reel4_w(&space,0,data&0xf);
						break;

					default:
						logerror("%08x maincpu write access offset %08x data %04x mem_mask %04x cs %d (LAMPS etc.)\n", pc, offset*2, data, mem_mask, cs);
				}
			}
			else
			{
				logerror("%08x maincpu write access offset %08x data %04x mem_mask %04x cs %d\n", pc, offset*2, data, mem_mask, cs);
			}
			break;

		case 3:
			base = 0xc00000/2;
			end = base + 0x20 / 2;

			if ((offset>=base) && (offset<end))
			{
				offset-=base;
				duart68681_w(m_duart,offset,data&0x00ff);
			}
			else
			{
				logerror("%08x maincpu write access offset %08x data %04x mem_mask %04x cs %d\n", pc, offset*2, data, mem_mask, cs);
			}

			break;

		case 4:
			logerror("%08x maincpu write access offset %08x data %04x mem_mask %04x cs %d\n", pc, offset*2, data, mem_mask, cs);
			break;

		default:
			logerror("%08x maincpu write access offset %08x data %04x mem_mask %04x cs %d (invalid?)\n", pc, offset*2, data, mem_mask, cs);
	}
}

static ADDRESS_MAP_START( sc4_map, AS_PROGRAM, 16, sc4_state )
	AM_RANGE(0x0000000, 0xffffff) AM_READWRITE(sc4_mem_r, sc4_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sc4_adder4_map, AS_PROGRAM, 32, sc4_adder4_state )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM
ADDRESS_MAP_END








void bfm_sc4_reset_serial_vfd(running_machine &machine)
{
	sc4_state *state = machine.driver_data<sc4_state>();

	BFM_BD1_reset(0);
	BFM_BD1_draw(0);
	state->vfd_old_clock = false;
}

void bfm_sc4_write_serial_vfd(running_machine &machine, bool cs, bool clock, bool data)
{
	sc4_state *state = machine.driver_data<sc4_state>();

	// if we're turned on
	if ( cs )
	{
		if ( !state->vfd_enabled )
		{
			bfm_sc4_reset_serial_vfd(machine);
			state->vfd_old_clock = clock;
			state->vfd_enabled = true;
		}
		else
		{
			// if the clock line changes
			if ( clock != state->vfd_old_clock )
			{
				if ( !clock )
				{
					state->vfd_ser_value <<= 1;
					if (data) state->vfd_ser_value |= 1;

					state->vfd_ser_count++;
					if ( state->vfd_ser_count == 8 )
					{
						state->vfd_ser_count = 0;
						BFM_BD1_newdata(0, state->vfd_ser_value);
						BFM_BD1_draw(0);
					}
				}
				state->vfd_old_clock = clock;
			}
		}
	}
	else
	{
		state->vfd_enabled = false;
	}
}


static WRITE8_HANDLER( bfm_sc4_68307_porta_w )
{
	sc4_state *state = space->machine().driver_data<sc4_state>();

	state->m_reel12_latch = data;

	if ( stepper_update(0, data&0x0f   ) ) state->m_reel_changed |= 0x01;
	if ( stepper_update(1, (data>>4))&0x0f ) state->m_reel_changed |= 0x02;

	if ( stepper_optic_state(0) ) state->m_optic_pattern |=  0x01;
	else                          state->m_optic_pattern &= ~0x01;
	if ( stepper_optic_state(1) ) state->m_optic_pattern |=  0x02;
	else                          state->m_optic_pattern &= ~0x02;

	awp_draw_reel(0);
	awp_draw_reel(1);
}

static WRITE8_HANDLER( bfm_sc4_reel3_w )
{
	sc4_state *state = space->machine().driver_data<sc4_state>();

	state->m_reel3_latch = data;

	if ( stepper_update(2, data&0x0f ) ) state->m_reel_changed |= 0x04;

	if ( stepper_optic_state(2) ) state->m_optic_pattern |=  0x04;
	else                          state->m_optic_pattern &= ~0x04;

	awp_draw_reel(2);
}

static WRITE8_HANDLER( bfm_sc4_reel4_w )
{
	sc4_state *state = space->machine().driver_data<sc4_state>();

	state->m_reel4_latch = data;

	if ( stepper_update(3, data&0x0f ) ) state->m_reel_changed |= 0x08;

	if ( stepper_optic_state(3) ) state->m_optic_pattern |=  0x08;
	else                          state->m_optic_pattern &= ~0x08;

	awp_draw_reel(3);
}


static WRITE16_HANDLER( bfm_sc4_68307_portb_w )
{
	int pc = cpu_get_pc(&space->device());
	//m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	// serial output to the VFD at least..
	logerror("%08x bfm_sc4_68307_portb_w %04x %04x\n", pc, data, mem_mask);

	bfm_sc4_write_serial_vfd(space->machine(), (data & 0x4000)?1:0, (data & 0x1000)?1:0, !(data & 0x2000)?1:0);

	bfm_sc4_reel3_w(space, 0, (data&0x0f00)>>8);

}

static READ8_HANDLER( bfm_sc4_68307_porta_r )
{
	int pc = cpu_get_pc(&space->device());
	logerror("%08x bfm_sc4_68307_porta_r\n", pc);
	return space->machine().rand();
}

static READ16_HANDLER( bfm_sc4_68307_portb_r )
{
	int pc = cpu_get_pc(&space->device());
	logerror("%08x bfm_sc4_68307_portb_r %04x\n", pc, mem_mask);
	return 0x0000;//0xffff;//space->machine().rand();
}

static MACHINE_RESET( sc4 )
{
	sc4_state *state = machine.driver_data<sc4_state>();

	int pattern =0, i;

	for ( i = 0; i < state->m_reels; i++)
	{
		stepper_reset_position(i);
		if ( stepper_optic_state(i) ) pattern |= 1<<i;
	}

	state->m_optic_pattern = pattern;
}

static MACHINE_START( sc4 )
{
	sc4_state *state = machine.driver_data<sc4_state>();
	state->m_cpuregion = (UINT16*)machine.region( "maincpu" )->base();
	state->m_mainram = (UINT16*)auto_alloc_array_clear(machine, UINT16, 0x100000);
	state->m_duart = machine.device("duart68681");
	state->m_ymz = machine.device("ymz");
	m68307_set_port_callbacks(machine.device("maincpu"),
		bfm_sc4_68307_porta_r,
		bfm_sc4_68307_porta_w,
		bfm_sc4_68307_portb_r,
		bfm_sc4_68307_portb_w );
	BFM_BD1_init(0);

	int reels = 6;
	state->m_reels=reels;

	// todo, make reel configs a per game structure
	for ( int n = 0; n < reels; n++ )
	{
		if (n!=3) stepper_config(machine, n, &starpoint_interface_48step);
		else  stepper_config(machine, n, &starpoint_interface_200step_reel); // luckb
	}
	if (reels)
	{
		awp_reel_setup();
	}
}


static void bfm_sc4_irqhandler(device_t *device, int state)
{
	logerror("YMZ280 is generating an interrupt. State=%08x\n",state);
}

static const ymz280b_interface ymz280b_config =
{
	bfm_sc4_irqhandler
};



void bfm_sc4_duart_irq_handler(device_t *device, UINT8 vector)
{
	logerror("bfm_sc4_duart_irq_handler\n");
};

void bfm_sc4_duart_tx(device_t *device, int channel, UINT8 data)
{
	logerror("bfm_sc4_duart_irq_handler\n");
};



UINT8 bfm_sc4_duart_input_r(device_t *device)
{
	sc4_state *state = device->machine().driver_data<sc4_state>();
//	printf("bfm_sc4_duart_input_r\n");
	return state->m_optic_pattern;
}

void bfm_sc4_duart_output_w(device_t *device, UINT8 data)
{
//	logerror("bfm_sc4_duart_output_w\n");
	sc4_state *state = device->machine().driver_data<sc4_state>();

	state->m_reel56_latch = data;

	if ( stepper_update(4, data&0x0f   ) ) state->m_reel_changed |= 0x10;
	if ( stepper_update(5, (data>>4)&0x0f) ) state->m_reel_changed |= 0x20;

	if ( stepper_optic_state(4) ) state->m_optic_pattern |=  0x10;
	else                          state->m_optic_pattern &= ~0x10;
	if ( stepper_optic_state(5) ) state->m_optic_pattern |=  0x20;
	else                          state->m_optic_pattern &= ~0x20;

	awp_draw_reel(4);
	awp_draw_reel(5);
}


static const duart68681_config bfm_sc4_duart68681_config =
{
	bfm_sc4_duart_irq_handler,
	bfm_sc4_duart_tx,
	bfm_sc4_duart_input_r,
	bfm_sc4_duart_output_w
};

// generate some fake interrupts for force things to go a bit further
// until we have the peripheral hookups working..
static INTERRUPT_GEN( sc4_fake_int_check )
{
	int which_int = 0;
	static int count = 0;

	count++;

	if (count>30000)
	{
		which_int = device->machine().rand() % 5;

		/*
        if ( device->machine().input().code_pressed_once(KEYCODE_Q) ) which_int = 1;
        if ( device->machine().input().code_pressed_once(KEYCODE_W) ) which_int = 2;
        if ( device->machine().input().code_pressed_once(KEYCODE_E) ) which_int = 3;
        if ( device->machine().input().code_pressed_once(KEYCODE_R) ) which_int = 4;
        if ( device->machine().input().code_pressed_once(KEYCODE_T) ) which_int = 5;
        */

		//if (which_int==1) m68307_timer0_interrupt((legacy_cpu_device*)device->machine().device("maincpu"));
		//if (which_int==2) m68307_timer1_interrupt((legacy_cpu_device*)device->machine().device("maincpu"));
		if (which_int==3) m68307_serial_interrupt((legacy_cpu_device*)device->machine().device("maincpu"));
		//if (which_int==4) m68307_mbus_interrupt((legacy_cpu_device*)device->machine().device("maincpu"));
	//  if (which_int==5) m68307_licr2_interrupt((legacy_cpu_device*)device->machine().device("maincpu"));
	}
}


MACHINE_CONFIG_START( sc4, sc4_state )
	MCFG_CPU_ADD("maincpu", M68307, 16000000)	 // 68307! (EC000 core)
	MCFG_CPU_PROGRAM_MAP(sc4_map)
	MCFG_CPU_PERIODIC_INT(sc4_fake_int_check,1000)

	MCFG_MACHINE_START( sc4 )
	MCFG_MACHINE_RESET( sc4 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DUART68681_ADD("duart68681", 16000000/4, bfm_sc4_duart68681_config) // ?? Mhz


	MCFG_DEFAULT_LAYOUT(layout_awpvid14)

	MCFG_SOUND_ADD("ymz", YMZ280B, 16000000) // ?? Mhz
	MCFG_SOUND_CONFIG(ymz280b_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED_CLASS( sc4_adder4, sc4, sc4_adder4_state )
	MCFG_CPU_ADD("adder4", M68340, 25175000)	 // 68340 (CPU32 core)
	MCFG_CPU_PROGRAM_MAP(sc4_adder4_map)
MACHINE_CONFIG_END


INPUT_PORTS_START( sc4 )
INPUT_PORTS_END




