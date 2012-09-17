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

    ---
    note, if game says 'read meters' simply press CTRL then 'B'
    I think we're stuck in some king of free play test mode so Start 1 will spin and give you credit

*/



#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/bfm_sc45.h"
#include "sound/ymz280b.h"
#include "machine/68681.h"
#include "bfm_sc4.lh"
#include "video/awpvid.h"
//DMD01
#include "video/bfm_dm01.h"
#include "cpu/m6809/m6809.h"
#include "sc4_dmd.lh"


UINT8 read_input_matrix(running_machine &machine, int row)
{

	static const char *const portnames[16] = { "IN-0", "IN-1", "IN-2", "IN-3", "IN-4", "IN-5", "IN-6", "IN-7", "IN-8", "IN-9", "IN-A", "IN-B" };
	UINT8 value;

	if (row<4)
	{
		value = (machine.root_device().ioport(portnames[row])->read_safe(0x00) & 0x1f) + ((machine.root_device().ioport(portnames[row+8])->read_safe(0x00) & 0x07) << 5);
	}
	else
	{
		value = (machine.root_device().ioport(portnames[row])->read_safe(0x00) & 0x1f) + ((machine.root_device().ioport(portnames[row+4])->read_safe(0x00) & 0x18) << 2);
	}

	return value;
}

READ16_MEMBER(sc4_state::sc4_cs1_r)
{
	int pc = space.device().safe_pc();

	if (offset<0x100000/2)
	{
		// allow some sets to boot, should probably return this data on Mbus once we figure out what it is
		if ((pc == m_chk41addr) && (offset == m_chk41addr>>1))
		{
			UINT32 r_A0 = space.device().state().state_int(M68K_A0);
			UINT32 r_A1 = space.device().state().state_int(M68K_A1);
			UINT32 r_D1 = space.device().state().state_int(M68K_D1);

			if (r_D1 == 0x7)
			{
				bool valid = true;
				for (int i=0;i<8;i++)
				{
					UINT8 code = space.read_byte(r_A0+i);
					if (code != 0xff) // assume our mbus code just returns 0xff for now..
						valid = false;
				}

				if (valid && m_dochk41)
				{
					m_dochk41 = false;
					// the value is actually random.. probably based on other reads
					// making this a comparison?
					printf("Ident code? ");
					for (int i=0;i<8;i++)
					{
						UINT8 code = space.read_byte(r_A1+i);
						printf("%02x",code);
						space.write_byte(r_A0+i, code);
					}
					printf("\n");
				}
			}
		}


		return m_cpuregion[offset];
	}
	else
		logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d\n", pc, offset*2, mem_mask, 1);

	return 0x0000;
}

READ16_MEMBER(sc4_state::sc4_mem_r)
{
	int pc = space.device().safe_pc();
	int cs = m68307_get_cs(m_maincpu, offset * 2);
	int base = 0, end = 0, base2 = 0, end2 = 0;
//  if (!(debugger_access())) printf("cs is %d\n", cs);
	UINT16 retvalue;


	switch ( cs )
	{
		case 1:
			return sc4_cs1_r(space,offset,mem_mask);

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
				logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d\n", pc, offset*2, mem_mask, cs);
				int addr = (offset<<1);

				if (addr < 0x0080)
				{
					UINT16 retvalue = 0x0000;
					if (mem_mask&0xff00)
					{
						logerror("mem_mask&0xff00 unhandled\n");
					}

					if (mem_mask&0x00ff)
					{
						retvalue = read_input_matrix(machine(), (addr & 0x00f0)>>4);
					}
					return retvalue;
				}
				else
				{
					switch (addr)
					{
						case 0x0240:
							retvalue = 0x00ff;

							if (mem_mask&0xff00)
							{
								retvalue |= (sec.read_data_line() << (6+8));
								retvalue |= 0xbf00; // coin?
								//printf("%08x maincpu read access offset %08x mem_mask %04x cs %d (LAMPS etc.)\n", pc, offset*2, mem_mask, cs);
							}
							return retvalue;

						case 0x02e0:
							return 0x0080;//space.machine().rand();;

						case 0x1000:
							return 0x0000;//space.machine().rand();;

						case 0x1010:
							return 0x0000;//space.machine().rand();;

						case 0x1020:
							return 0x0000;//space.machine().rand();;

						case 0x1030:
							return 0x0000;//space.machine().rand();;

						case 0x1040: // door switch, test switch etc.
							return 0x0000;//space.machine().rand();;

						case 0x1244:
							return ymz280b_r(m_ymz,space,0);

						case 0x1246:
							return ymz280b_r(m_ymz,space,1);

						default:
							logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d (LAMPS etc.)\n", pc, offset*2, mem_mask, cs);
					}
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
				return duart68681_r(m_duart,space,offset);
			}
			else
			{
				logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d\n", pc, offset*2, mem_mask, cs);
				return 0x0000;
			}

		case 4:
			logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d\n", pc, offset*2, mem_mask, cs);
			return 0x0000;//0xffff;

		default:
			logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d (invalid?)\n", pc, offset*2, mem_mask, cs);

	}

	return 0x0000;
}

static WRITE8_HANDLER( bfm_sc4_reel4_w );

WRITE8_MEMBER(sc4_state::mux_output_w)
{
	int i;
	int off = offset<<3;

	for (i=0; i<8; i++)
		output_set_lamp_value(off+i, ((data & (1 << i)) != 0));


	output_set_indexed_value("matrix", off+i, ((data & (1 << i)) != 0));
}

WRITE8_MEMBER(sc4_state::mux_output2_w)
{
	int i;
	int off = offset<<3;

	// some games use this as a matrix port (luckb etc.)
	for (i=0; i<8; i++)
	{
		output_set_indexed_value("matrix", off+i, ((data & (1 << i)) != 0));
	}

	// others drive 7-segs with it..  so rendering it there as well in our debug layouts

	// todo: reorder properly!
	UINT8 bf7segdata = BITSWAP8(data,7,6,5,4,3,2,1,0);
	output_set_digit_value(offset, bf7segdata);
}

WRITE16_MEMBER(sc4_state::sc4_mem_w)
{
	int pc = space.device().safe_pc();
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
				int addr = (offset<<1);

				if (addr < 0x0200)
				{

					if (mem_mask&0xff00)
					{
						logerror("lamp write mem_mask&0xff00 unhandled\n");
					}

					if (mem_mask&0x00ff)
					{	// lamps
						mux_output_w(space, (addr & 0x01f0)>>4, data);
					}

				}
				else if ((addr >= 0x1000) && (addr < 0x1200))
				{
					if (mem_mask&0xff00)
					{
						logerror("lamp write mem_mask&0xff00 unhandled\n");
					}

					if (mem_mask&0x00ff)
					{	// lamps
						mux_output2_w(space, (addr & 0x01f0)>>4, data);
					}
				}
				else
				{
					switch (addr)
					{
						case 0x0330:
							logerror("%08x meter write %04x\n",pc, data);
							//m_meterstatus = (m_meterstatus&0xc0) | (data & 0x3f);
							sec.write_clock_line(~data&0x20);
							break;

						case 0x1248:
							ymz280b_w(m_ymz,space,0, data & 0xff);
							break;

						case 0x124a:
							ymz280b_w(m_ymz,space,1, data & 0xff);
							break;

						case 0x1330:
							bfm_sc4_reel4_w(space,0,data&0xf);
							//m_meterstatus = (m_meterstatus&0x3f) | ((data & 0x30) << 2);
							sec.write_data_line(~data&0x10);
							break;

						default:
							logerror("%08x maincpu write access offset %08x data %04x mem_mask %04x cs %d (LAMPS etc.)\n", pc, offset*2, data, mem_mask, cs);
					}
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
				duart68681_w(m_duart,space,offset,data&0x00ff);
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
	AM_RANGE(0x0000000, 0x0fffff) AM_READ(sc4_cs1_r) // technically we should be going through the cs handler, but this is always set to ROM, and assuming that is a lot faster
	AM_RANGE(0x0000000, 0xffffff) AM_READWRITE(sc4_mem_r, sc4_mem_w)
ADDRESS_MAP_END




READ32_MEMBER(sc4_adder4_state::adder4_mem_r)
{
	int pc = space.device().safe_pc();
	int cs = m68340_get_cs(m_adder4cpu, offset * 4);

	switch ( cs )
	{
		case 1:
			return m_adder4cpuregion[offset];

		case 2:
			offset &=0x3fff;
			return m_adder4ram[offset];

		default:
			logerror("%08x adder4cpu read access offset %08x mem_mask %08x cs %d\n", pc, offset*4, mem_mask, cs);

	}

	return 0x0000;
}

WRITE32_MEMBER(sc4_adder4_state::adder4_mem_w)
{
	int pc = space.device().safe_pc();
	int cs = m68340_get_cs(m_adder4cpu, offset * 4);

	switch ( cs )
	{
		default:
			logerror("%08x adder4cpu write access offset %08x data %08x mem_mask %08x cs %d\n", pc, offset*4, data, mem_mask, cs);

		case 2:
			offset &=0x3fff;
			COMBINE_DATA(&m_adder4ram[offset]);
			break;


	}

}

static ADDRESS_MAP_START( sc4_adder4_map, AS_PROGRAM, 32, sc4_adder4_state )
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(adder4_mem_r, adder4_mem_w)
ADDRESS_MAP_END








void bfm_sc4_reset_serial_vfd(running_machine &machine)
{
	sc4_state *state = machine.driver_data<sc4_state>();

	state->m_vfd0->reset();
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
				//Should move to the internal serial process when DM01 is device-ified
//                  m_vfd0->shift_data(!data);
					state->vfd_ser_value <<= 1;
					if (data) state->vfd_ser_value |= 1;

					state->vfd_ser_count++;
					if ( state->vfd_ser_count == 8 )
					{
						state->vfd_ser_count = 0;
						if (machine.device("matrix"))
						{
							BFM_dm01_writedata(machine,state->vfd_ser_value);
						}
						else
						{
							state->m_vfd0->write_char(state->vfd_ser_value);
						}
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


void bfm_sc4_68307_porta_w(address_space &space, bool dedicated, UINT8 data, UINT8 line_mask)
{
	sc4_state *state = space.machine().driver_data<sc4_state>();

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
	sc4_state *state = space.machine().driver_data<sc4_state>();

	state->m_reel3_latch = data;

	if ( stepper_update(2, data&0x0f ) ) state->m_reel_changed |= 0x04;

	if ( stepper_optic_state(2) ) state->m_optic_pattern |=  0x04;
	else                          state->m_optic_pattern &= ~0x04;

	awp_draw_reel(2);
}

static WRITE8_HANDLER( bfm_sc4_reel4_w )
{
	sc4_state *state = space.machine().driver_data<sc4_state>();

	state->m_reel4_latch = data;

	if ( stepper_update(3, data&0x0f ) ) state->m_reel_changed |= 0x08;

	if ( stepper_optic_state(3) ) state->m_optic_pattern |=  0x08;
	else                          state->m_optic_pattern &= ~0x08;

	awp_draw_reel(3);
}

void bfm_sc4_68307_portb_w(address_space &space, bool dedicated, UINT16 data, UINT16 line_mask)
{
//  if (dedicated == false)
	{
		int pc = space.device().safe_pc();
		//m68ki_cpu_core *m68k = m68k_get_safe_token(&space.device());
		// serial output to the VFD at least..
		logerror("%08x bfm_sc4_68307_portb_w %04x %04x\n", pc, data, line_mask);

		bfm_sc4_write_serial_vfd(space.machine(), (data & 0x4000)?1:0, (data & 0x1000)?1:0, !(data & 0x2000)?1:0);

		bfm_sc4_reel3_w(space, 0, (data&0x0f00)>>8);
	}

}
UINT8 bfm_sc4_68307_porta_r(address_space &space, bool dedicated, UINT8 line_mask)
{
	int pc = space.device().safe_pc();
	logerror("%08x bfm_sc4_68307_porta_r\n", pc);
	return space.machine().rand();
}

UINT16 bfm_sc4_68307_portb_r(address_space &space, bool dedicated, UINT16 line_mask)
{
	if (dedicated==false)
	{
		return 0x0000;
	}
	else
	{
		// generating certain interrupts expecteds the bit 0x8000 to be set here
		// but it's set ot dedicated i/o, not general purpose, source?
		return 0x8040;
	}
}

MACHINE_RESET_MEMBER(sc4_state,sc4)
{

	int pattern =0, i;

	for ( i = 0; i < m_reels; i++)
	{
		stepper_reset_position(i);
		if ( stepper_optic_state(i) ) pattern |= 1<<i;
	}

	m_dochk41 = true;

	m_optic_pattern = pattern;
	sec.reset();
}

static NVRAM_HANDLER( bfm_sc4 )
{
	sc4_state *state = machine.driver_data<sc4_state>();
	if ( read_or_write )
	{	// writing
		file->write(state->m_mainram,0x10000);
	}
	else
	{ // reading
		if ( file )
		{
			file->read(state->m_mainram,0x10000);
		}
	}
}



MACHINE_START_MEMBER(sc4_state,sc4)
{
	m_cpuregion = (UINT16*)memregion( "maincpu" )->base();
	m_mainram = (UINT16*)auto_alloc_array_clear(machine(), UINT16, 0x10000);
	m_duart = machine().device("duart68681");
	m_ymz = machine().device("ymz");
	m68307_set_port_callbacks(machine().device("maincpu"),
		bfm_sc4_68307_porta_r,
		bfm_sc4_68307_porta_w,
		bfm_sc4_68307_portb_r,
		bfm_sc4_68307_portb_w );
	m68307_set_duart68681(machine().device("maincpu"),machine().device("m68307_68681"));

	int reels = 6;
	m_reels=reels;

	for ( int n = 0; n < reels; n++ )
	{
		 if (m_reel_setup[n]) stepper_config(machine(), n, m_reel_setup[n]);
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



void bfm_sc4_duart_irq_handler(device_t *device, int state, UINT8 vector)
{
	// triggers after reel tests on luckb, at the start on dnd...
	// not sure this is right, causes some games to crash
	logerror("bfm_sc4_duart_irq_handler\n");
	if (state == ASSERT_LINE)
	{
		m68307_licr2_interrupt((legacy_cpu_device*)device->machine().device("maincpu"));
	}
};

void bfm_sc4_duart_tx(device_t *device, int channel, UINT8 data)
{
	logerror("bfm_sc4_duart_tx\n");
};



UINT8 bfm_sc4_duart_input_r(device_t *device)
{
	sc4_state *state = device->machine().driver_data<sc4_state>();
//  printf("bfm_sc4_duart_input_r\n");
	return state->m_optic_pattern;
}

void bfm_sc4_duart_output_w(device_t *device, UINT8 data)
{
//  logerror("bfm_sc4_duart_output_w\n");
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
	bfm_sc4_duart_output_w,
	// TODO: What are the actual frequencies?
	XTAL_16MHz/2/8,		/* IP2/RxCB clock */
	XTAL_16MHz/2/16,	/* IP3/TxCA clock */
	XTAL_16MHz/2/16,	/* IP4/RxCA clock */
	XTAL_16MHz/2/8,		/* IP5/TxCB clock */
};



void m68307_duart_irq_handler(device_t *device, int state, UINT8 vector)
{
	logerror("m68307_duart_irq_handler\n");
	if (state == ASSERT_LINE)
	{
		m68307_serial_interrupt((legacy_cpu_device*)device->machine().device("maincpu"), vector);
	}
};

void m68307_duart_tx(device_t *device, int channel, UINT8 data)
{
	if (channel==0)
	{
		logerror("m68307_duart_tx %02x\n",data);
	}
	else
	{
		printf("(illegal channel 1) m68307_duart_tx %02x\n",data);
	}
};

UINT8 m68307_duart_input_r(device_t *device)
{
	logerror("m68307_duart_input_r\n");
	return 0x00;
}

void m68307_duart_output_w(device_t *device, UINT8 data)
{
	logerror("m68307_duart_output_w %02x\n", data);
}



static const duart68681_config m68307_duart68681_config =
{
	m68307_duart_irq_handler,
	m68307_duart_tx,
	m68307_duart_input_r,
	m68307_duart_output_w
};



MACHINE_CONFIG_START( sc4, sc4_state )
	MCFG_CPU_ADD("maincpu", M68307, 16000000)	 // 68307! (EC000 core)
	MCFG_CPU_PROGRAM_MAP(sc4_map)

	// internal duart of the 68307... paired in machine start
	MCFG_DUART68681_ADD("m68307_68681", 16000000/4, m68307_duart68681_config) // ?? Mhz

	MCFG_MACHINE_START_OVERRIDE(sc4_state, sc4 )
	MCFG_MACHINE_RESET_OVERRIDE(sc4_state, sc4 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NVRAM_HANDLER(bfm_sc4)

	MCFG_DUART68681_ADD("duart68681", 16000000/4, bfm_sc4_duart68681_config) // ?? Mhz

	MCFG_BFMBDA_ADD("vfd0",0)

	MCFG_DEFAULT_LAYOUT(layout_bfm_sc4)

	MCFG_SOUND_ADD("ymz", YMZ280B, 16000000) // ?? Mhz
	MCFG_SOUND_CONFIG(ymz280b_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



MACHINE_START_MEMBER(sc4_adder4_state,adder4)
{
	m_adder4cpuregion = (UINT32*)memregion( "adder4" )->base();
	m_adder4ram = (UINT32*)auto_alloc_array_clear(machine(), UINT32, 0x10000);
	MACHINE_START_CALL_MEMBER(sc4);
}

MACHINE_CONFIG_DERIVED_CLASS( sc4_adder4, sc4, sc4_adder4_state )
	MCFG_CPU_ADD("adder4", M68340, 25175000)	 // 68340 (CPU32 core)
	MCFG_CPU_PROGRAM_MAP(sc4_adder4_map)

	MCFG_MACHINE_START_OVERRIDE(sc4_adder4_state, adder4 )
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED_CLASS( sc4dmd, sc4, sc4_state )
	/* video hardware */

	MCFG_DEFAULT_LAYOUT(layout_sc4_dmd)
	MCFG_CPU_ADD("matrix", M6809, 2000000 )				/* matrix board 6809 CPU at 2 Mhz ?? I don't know the exact freq.*/
	MCFG_CPU_PROGRAM_MAP(bfm_dm01_memmap)
	MCFG_CPU_PERIODIC_INT(bfm_dm01_vbl, 1500 )			/* generate 1500 NMI's per second ?? what is the exact freq?? */

	MCFG_MACHINE_START_OVERRIDE(sc4_state, sc4 )
MACHINE_CONFIG_END

INPUT_PORTS_START( sc4_base )
	PORT_START("IN-0")
	PORT_DIPNAME( 0x01, 0x00, "IN-0:0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN-0:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN-0:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN-0:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN-0:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_START("IN-1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 )

	PORT_START("IN-2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON9 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start 1 / P1 Button 1") // also gives you 0.50 credits at the moment, I guess because we're in freeplay?


	PORT_START("IN-3")
	PORT_DIPNAME( 0x01, 0x00, "IN-3:0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN-3:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN-3:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN-3:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN-3:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )


	PORT_START("IN-4")
	PORT_DIPNAME( 0x01, 0x00, "IN-4:0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN-4:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN-4:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN-4:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN-4:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )


	PORT_START("IN-5")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED ) // Jackpot Key, filled in later
	PORT_DIPNAME( 0x10, 0x00, "IN-5:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_START("IN-6")
	PORT_DIPNAME( 0x01, 0x00, "IN-6:0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN-6:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN-6:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN-6:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN-6:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_START("IN-7")
	PORT_DIPNAME( 0x01, 0x00, "IN-7:0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN-7:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN-7:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN-7:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN-7:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_START("IN-8")
	PORT_DIPNAME( 0x01, 0x00, "IN-8:0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN-8:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN-8:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN-8:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN-8:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )


	PORT_START("IN-9")
	PORT_DIPNAME( 0x01, 0x00, "IN-9:0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN-9:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN-9:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN-9:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN-9:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )


	PORT_START("IN-a")
	PORT_DIPNAME( 0x01, 0x00, "IN-a:0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN-a:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN-a:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN-a:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN-a:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )


	PORT_START("IN-b")
	PORT_DIPNAME( 0x01, 0x00, "IN-b:0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN-b:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN-b:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN-b:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN-b:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
INPUT_PORTS_END




