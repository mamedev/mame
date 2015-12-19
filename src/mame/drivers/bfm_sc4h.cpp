// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    Scorpion 4 Hardware Platform (c)1996 Bell Fruit Manufacturing

    preliminary driver

    -----------------

    Scorpion 4:::

    Main CPU is a MC68307FG16, present on Motherboard

    Configuration is SC4 motherboard + game card

    The game card contains the program roms, sound rom and YMZ280B

    Adder 4 video board adds an additional card with a MC68340PV25E (25.175Mhz)

    -------------------------------

    This file contains the hardware emulation, for the supported sets
    see bfm_sc4.c

    The hopper(s) are not currently emulated, many of the games can
    be operated in 'Door Open' mode granting you free credits.

    Most games will show a RAM error on first boot due, after that they
    will initialize their NVRAM.

    If 'Read Meters' is shown press the 'Cancel' button (this moves around
    per game, so where it maps might not be obvious)  Doing this will allow
    the games to run in Door Open mode, pressing 'Start' (also moves around)
    will allow you to test the game.  Not all games have this feature.

    Pressing the service key ('Green Button') often allows test mode to be
    entered, some games have more comprehensive tests than others.

    Various (poorly programmed) sets require specific Jackpot 'keys' etc. to
    boot and won't even warn you if they're invalid, others allow you to
    set options if keys are not present. (again the buttons to do so move
    between games)

    Many games have missing sound roms, incorrect sound roms, or badly
    dumped sound roms.  We also have several dumps where only sound roms
    are present.

    Many of the titles here were also released on the SC5 platform.

*/



#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68307.h"
#include "machine/68340.h"
#include "includes/bfm_sc45.h"
#include "video/awpvid.h"
//DMD01
#include "cpu/m6809/m6809.h"



UINT8 sc4_state::read_input_matrix(int row)
{
	ioport_port* portnames[16] = { m_io1, m_io2, m_io3, m_io4, m_io5, m_io6, m_io7, m_io8, m_io9, m_io10, m_io11, m_io12 };
	UINT8 value;

	if (row<4)
		value = (read_safe(portnames[row], 0x00) & 0x1f) + ((read_safe(portnames[row+8], 0x00) & 0x07) << 5);
	else
		value = (read_safe(portnames[row], 0x00) & 0x1f) + ((read_safe(portnames[row+4], 0x00) & 0x18) << 2);

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


		return m_cpuregion->u16(offset);
	}
	else
		logerror("%08x maincpu read access offset %08x mem_mask %04x cs %d\n", pc, offset*2, mem_mask, 1);

	return 0x0000;
}

READ16_MEMBER(sc4_state::sc4_mem_r)
{
	int pc = space.device().safe_pc();
	int cs = m_maincpu->get_cs(offset * 2);
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
						retvalue = read_input_matrix((addr & 0x00f0)>>4);
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
								retvalue |= ioport("IN-COIN")->read() << 8; // coin?
								//printf("%08x maincpu read access offset %08x mem_mask %04x cs %d (LAMPS etc.)\n", pc, offset*2, mem_mask, cs);
							}
							return retvalue;

						case 0x02e0:
							return 0x0080; // status of something?

						case 0x1000:
							return ioport("IN-16")->read();

						case 0x1010:
							return ioport("IN-17")->read();

						case 0x1020:
							return ioport("IN-18")->read();

						case 0x1030:
							return ioport("IN-19")->read();

						case 0x1040: // door switch, test switch etc.
							return ioport("IN-20")->read();

						case 0x1244:
							return m_ymz->read(space,0);

						case 0x1246:
							return m_ymz->read(space,1);

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
				return m_duart->read(space,offset);
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

WRITE8_MEMBER(bfm_sc45_state::mux_output_w)
{
	int i;
	int off = offset<<3;

	for (i=0; i<8; i++)
		output_set_lamp_value(off+i, ((data & (1 << i)) != 0));


	output_set_indexed_value("matrix", off+i, ((data & (1 << i)) != 0));
}

WRITE8_MEMBER(bfm_sc45_state::mux_output2_w)
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
	int cs = m_maincpu->get_cs(offset * 2);
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
					{   // lamps
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
					{   // lamps
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
							m_ymz->write(space,0, data & 0xff);
							break;

						case 0x124a:
							m_ymz->write(space,1, data & 0xff);
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
				m_duart->write(space,offset,data&0x00ff);
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








void bfm_sc45_state::bfm_sc4_reset_serial_vfd()
{
	m_vfd0->reset();
	vfd_old_clock = false;
}

void bfm_sc45_state::bfm_sc45_write_serial_vfd(bool cs, bool clock, bool data)
{
	// if we're turned on
	if ( cs )
	{
		if ( !vfd_enabled )
		{
			bfm_sc4_reset_serial_vfd();
			vfd_old_clock = clock;
			vfd_enabled = true;
		}
		else
		{
			// if the clock line changes
			if ( clock != vfd_old_clock )
			{
				if ( !clock )
				{
				//Should move to the internal serial process when DM01 is device-ified
//                  m_vfd0->shift_data(!data);
					vfd_ser_value <<= 1;
					if (data) vfd_ser_value |= 1;

					vfd_ser_count++;
					if ( vfd_ser_count == 8 )
					{
						vfd_ser_count = 0;
						if (machine().device("matrix"))
						{
							m_dm01->writedata(vfd_ser_value);
						}
						else
						{
							m_vfd0->write_char(vfd_ser_value);
						}
					}
				}
				vfd_old_clock = clock;
			}
		}
	}
	else
	{
		vfd_enabled = false;
	}
}


void sc4_state::bfm_sc4_68307_porta_w(address_space &space, bool dedicated, UINT8 data, UINT8 line_mask)
{
	m_reel12_latch = data;

	if(m_reel1)
	{
		m_reel1->update( data    &0x0f);
		awp_draw_reel("reel1", m_reel1);
	}

	if (m_reel2)
	{
		m_reel2->update((data>>4)&0x0f);
		awp_draw_reel("reel2", m_reel2);
	}
}

WRITE8_MEMBER( sc4_state::bfm_sc4_reel3_w )
{
	m_reel3_latch = data;

	if(m_reel3)
	{
		m_reel3->update( data    &0x0f);
		awp_draw_reel("reel3", m_reel3);
	}
}

WRITE8_MEMBER( sc4_state::bfm_sc4_reel4_w )
{
	m_reel4_latch = data;

	if(m_reel4)
	{
		m_reel4->update( data    &0x0f);
		awp_draw_reel("reel4", m_reel4);
	}
}

void sc4_state::bfm_sc4_68307_portb_w(address_space &space, bool dedicated, UINT16 data, UINT16 line_mask)
{
//  if (dedicated == false)
	{
		int pc = space.device().safe_pc();
		//_m68ki_cpu_core *m68k = m68k_get_safe_token(&space.device());
		// serial output to the VFD at least..
		logerror("%08x bfm_sc4_68307_portb_w %04x %04x\n", pc, data, line_mask);

		bfm_sc45_write_serial_vfd((data & 0x4000)?1:0, (data & 0x1000)?1:0, !(data & 0x2000)?1:0);

		bfm_sc4_reel3_w(space, 0, (data&0x0f00)>>8, 0xff);
	}

}
UINT8 sc4_state::bfm_sc4_68307_porta_r(address_space &space, bool dedicated, UINT8 line_mask)
{
	int pc = space.device().safe_pc();
	logerror("%08x bfm_sc4_68307_porta_r\n", pc);
	return 0xbb;// machine().rand();
}

UINT16 sc4_state::bfm_sc4_68307_portb_r(address_space &space, bool dedicated, UINT16 line_mask)
{
	if (dedicated==false)
	{
		return 0x0000;
	}
	else
	{
		// generating certain interrupts expects the bit 0x8000 to be set here
		// but it's set to dedicated i/o, not general purpose, source?
		return 0x8040;
	}
}

MACHINE_RESET_MEMBER(sc4_state,sc4)
{
	m_dochk41 = true;

	sec.reset();
}


MACHINE_START_MEMBER(sc4_state,sc4)
{
	m_nvram->set_base(m_mainram, sizeof(m_mainram));


	m_maincpu->set_port_callbacks(
		m68307_porta_read_delegate(FUNC(sc4_state::bfm_sc4_68307_porta_r),this),
		m68307_porta_write_delegate(FUNC(sc4_state::bfm_sc4_68307_porta_w),this),
		m68307_portb_read_delegate(FUNC(sc4_state::bfm_sc4_68307_portb_r),this),
		m68307_portb_write_delegate(FUNC(sc4_state::bfm_sc4_68307_portb_w),this) );

}


WRITE_LINE_MEMBER(sc4_state::bfm_sc4_duart_irq_handler)
{
	// triggers after reel tests on luckb, at the start on dnd...
	// not sure this is right, causes some games to crash
	logerror("bfm_sc4_duart_irq_handler\n");
	if (state == ASSERT_LINE)
	{
		m_maincpu->licr2_interrupt();
	}
}

WRITE_LINE_MEMBER(sc4_state::bfm_sc4_duart_txa)
{
	logerror("bfm_sc4_duart_tx\n");
}



READ8_MEMBER(sc4_state::bfm_sc4_duart_input_r)
{
	//  printf("bfm_sc4_duart_input_r\n");
	return m_optic_pattern;
}

WRITE8_MEMBER(sc4_state::bfm_sc4_duart_output_w)
{
//  logerror("bfm_sc4_duart_output_w\n");
	m_reel56_latch = data;

	if(m_reel5)
	{
		m_reel5->update( data    &0x0f);
		awp_draw_reel("reel5", m_reel5);
	}

	if (m_reel6)
	{
		m_reel6->update((data>>4)&0x0f);
		awp_draw_reel("reel6", m_reel6);
	}
}


WRITE_LINE_MEMBER(sc4_state::m68307_duart_txa)
{
	logerror("m68307_duart_tx %02x\n",state);
}

READ8_MEMBER(sc4_state::m68307_duart_input_r)
{
	logerror("m68307_duart_input_r\n");
	return 0x00;
}

WRITE8_MEMBER(sc4_state::m68307_duart_output_w)
{
	logerror("m68307_duart_output_w %02x\n", data);
}

/* default dmd */
WRITE_LINE_MEMBER(sc4_state::bfmdm01_busy)
{
	// Must tie back to inputs somehow!
}

MACHINE_CONFIG_FRAGMENT( sc4_common )
	MCFG_CPU_ADD("maincpu", M68307, 16000000)    // 68307! (EC000 core)
	MCFG_CPU_PROGRAM_MAP(sc4_map)
	MCFG_MC68307_SERIAL_A_TX_CALLBACK(WRITELINE(sc4_state, m68307_duart_txa))
	MCFG_MC68307_SERIAL_INPORT_CALLBACK(READ8(sc4_state, m68307_duart_input_r))
	MCFG_MC68307_SERIAL_OUTPORT_CALLBACK(WRITE8(sc4_state, m68307_duart_output_w))

	MCFG_MACHINE_START_OVERRIDE(sc4_state, sc4 )
	MCFG_MACHINE_RESET_OVERRIDE(sc4_state, sc4 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_MC68681_ADD("duart68681", 16000000/4) // ?? Mhz
	MCFG_MC68681_SET_EXTERNAL_CLOCKS(XTAL_16MHz/2/8, XTAL_16MHz/2/16, XTAL_16MHz/2/16, XTAL_16MHz/2/8)
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(sc4_state, bfm_sc4_duart_irq_handler))
	MCFG_MC68681_A_TX_CALLBACK(WRITELINE(sc4_state, bfm_sc4_duart_txa))
	MCFG_MC68681_INPORT_CALLBACK(READ8(sc4_state, bfm_sc4_duart_input_r))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(sc4_state, bfm_sc4_duart_output_w))

	MCFG_BFMBDA_ADD("vfd0",0)

//  MCFG_DEFAULT_LAYOUT(layout_bfm_sc4)

	MCFG_SOUND_ADD("ymz", YMZ280B, 16000000) // ?? Mhz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

//Standard 6 reels all connected
MACHINE_CONFIG_START( sc4, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))
MACHINE_CONFIG_END

//Standard 3 reels
MACHINE_CONFIG_START( sc4_3reel, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))

MACHINE_CONFIG_END

//Standard 4 reels
MACHINE_CONFIG_START( sc4_4reel, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
MACHINE_CONFIG_END

//4 reels, with the last connected to RL4 not RL3
MACHINE_CONFIG_START( sc4_4reel_alt, sc4_state )

	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))

	MCFG_STARPOINT_RM20_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
MACHINE_CONFIG_END


//Standard 5 reels
MACHINE_CONFIG_START( sc4_5reel, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
MACHINE_CONFIG_END

//5 reels, with RL4 skipped
MACHINE_CONFIG_START( sc4_5reel_alt, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))

	MCFG_STARPOINT_RM20_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))

MACHINE_CONFIG_END

//6 reels, last 200 steps
MACHINE_CONFIG_START( sc4_200_std, sc4_state )

	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))
MACHINE_CONFIG_END

//6 reels, last 200 steps
MACHINE_CONFIG_START( sc4_200_alt, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))
MACHINE_CONFIG_END

//6 reels, RL4 200 steps
MACHINE_CONFIG_START( sc4_200_alta, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))
MACHINE_CONFIG_END

//6 reels, 3 48 step, 3 200 step
MACHINE_CONFIG_START( sc4_200_altb, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_200STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))
MACHINE_CONFIG_END

//5 reels, last one 200 steps
MACHINE_CONFIG_START( sc4_200_5r, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
MACHINE_CONFIG_END



//5 reels, last one 200 steps, RL4 skipped
MACHINE_CONFIG_START( sc4_200_5ra, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))

	MCFG_STARPOINT_RM20_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))
MACHINE_CONFIG_END

//5 reels, last one 200 steps, RL5 skipped
MACHINE_CONFIG_START( sc4_200_5rb, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))

	MCFG_STARPOINT_200STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))
MACHINE_CONFIG_END

//5 reels, RL5 200 steps, RL4 skipped
MACHINE_CONFIG_START( sc4_200_5rc, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))

	MCFG_STARPOINT_200STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))
MACHINE_CONFIG_END

//4 reels, last one 200 steps
MACHINE_CONFIG_START( sc4_200_4r, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
MACHINE_CONFIG_END

//4 reels, last one 200 steps, RL4 skipped
MACHINE_CONFIG_START( sc4_200_4ra, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))

	MCFG_STARPOINT_200STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
MACHINE_CONFIG_END


//4 reels, last one 200 steps, RL4,5 skipped
MACHINE_CONFIG_START( sc4_200_4rb, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))

	MCFG_STARPOINT_200STEP_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel6_optic_cb))
MACHINE_CONFIG_END

MACHINE_CONFIG_START( sc4_4reel_200, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_200STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
MACHINE_CONFIG_END

MACHINE_CONFIG_START( sc4_3reel_200, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_200STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
MACHINE_CONFIG_END

MACHINE_CONFIG_START( sc4_3reel_200_48, sc4_state )

	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_STARPOINT_200STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_200STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
MACHINE_CONFIG_END

MACHINE_CONFIG_START( sc4_no_reels, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)
MACHINE_CONFIG_END

MACHINE_START_MEMBER(sc4_adder4_state,adder4)
{
	m_adder4cpuregion = (UINT32*)memregion( "adder4" )->base();
	m_adder4ram = make_unique_clear<UINT32[]>(0x10000);
	MACHINE_START_CALL_MEMBER(sc4);
}

MACHINE_CONFIG_START( sc4_adder4, sc4_adder4_state )
	MCFG_FRAGMENT_ADD(sc4_common)

	MCFG_CPU_ADD("adder4", M68340, 25175000)     // 68340 (CPU32 core)
	MCFG_CPU_PROGRAM_MAP(sc4_adder4_map)

	MCFG_MACHINE_START_OVERRIDE(sc4_adder4_state, adder4 )
MACHINE_CONFIG_END

MACHINE_CONFIG_START( sc4dmd, sc4_state )
	MCFG_FRAGMENT_ADD(sc4_common)
	/* video hardware */

	//MCFG_DEFAULT_LAYOUT(layout_sc4_dmd)
	MCFG_DEVICE_ADD("dm01", BF_DM01, 0)
	MCFG_BF_DM01_BUSY_CB(WRITELINE(sc4_state, bfmdm01_busy))
	MCFG_CPU_ADD("matrix", M6809, 2000000 )             /* matrix board 6809 CPU at 2 Mhz ?? I don't know the exact freq.*/
	MCFG_CPU_PROGRAM_MAP(bfm_dm01_memmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(sc4_state, nmi_line_assert, 1500 )          /* generate 1500 NMI's per second ?? what is the exact freq?? */

	MCFG_MACHINE_START_OVERRIDE(sc4_state, sc4 )

	MCFG_STARPOINT_RM20_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel1_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel2_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel3_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel4_optic_cb))
	MCFG_STARPOINT_RM20_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(sc4_state, reel5_optic_cb))
MACHINE_CONFIG_END

INPUT_PORTS_START( sc4_raw ) // completley unmapped, but named inputs for all the ports, used for testing.
	PORT_START("IN-0")
	PORT_DIPNAME( 0x01, 0x00, "IN 0-0 (STRB 0 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 0-1 (STRB 0 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 0-2 (STRB 0 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 0-3 (STRB 0 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 0-4 (STRB 0 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-1")
	PORT_DIPNAME( 0x01, 0x00, "IN 1-0 (STRB 1 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 1-1 (STRB 1 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 1-2 (STRB 1 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 1-3 (STRB 1 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 1-4 (STRB 1 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-2")
	PORT_DIPNAME( 0x01, 0x00, "IN 2-0 (STRB 2 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 2-1 (STRB 2 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 2-2 (STRB 2 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 2-3 (STRB 2 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 2-4 (STRB 2 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-3")
	PORT_DIPNAME( 0x01, 0x00, "IN 3-0 (STRB 3 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 3-1 (STRB 3 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 3-2 (STRB 3 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 3-3 (STRB 3 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 3-4 (STRB 3 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-4")
	PORT_DIPNAME( 0x01, 0x00, "IN 4-0 (STRB 4 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 4-1 (STRB 4 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 4-2 (STRB 4 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 4-3 (STRB 4 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 4-4 (STRB 4 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )


	PORT_START("IN-5")
	PORT_DIPNAME( 0x01, 0x00, "IN 5-0 (STRB 5 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 5-1 (STRB 5 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 5-2 (STRB 5 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 5-3 (STRB 5 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 5-4 (STRB 5 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-6")
	PORT_DIPNAME( 0x01, 0x00, "IN 6-0 (STRB 6 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 6-1 (STRB 6 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 6-2 (STRB 6 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 6-3 (STRB 6 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 6-4 (STRB 6 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-7")
	PORT_DIPNAME( 0x01, 0x00, "IN 7-0 (STRB 7 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 7-1 (STRB 7 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 7-2 (STRB 7 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 7-3 (STRB 7 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 7-4 (STRB 7 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-8")
	PORT_DIPNAME( 0x01, 0x00, "IN 8-0 (STRB 8 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 8-1 (STRB 8 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 8-2 (STRB 8 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 8-3 (STRB 8 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 8-4 (STRB 8 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )


	PORT_START("IN-9")
	PORT_DIPNAME( 0x01, 0x00, "IN 9-0 (STRB 9 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 9-1 (STRB 9 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 9-2 (STRB 9 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 9-3 (STRB 9 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 9-4 (STRB 9 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-10")
	PORT_DIPNAME( 0x01, 0x00, "IN 10-0 (STRB 10 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 10-1 (STRB 10 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 10-2 (STRB 10 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 10-3 (STRB 10 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 10-4 (STRB 10 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )


	PORT_START("IN-11")
	PORT_DIPNAME( 0x01, 0x00, "IN 11-0 (STRB 11 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 11-1 (STRB 11 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 11-2 (STRB 11 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 11-3 (STRB 11 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 11-4 (STRB 11 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	// where are 11,12,13,14,15 ?

	PORT_START("IN-16")
	PORT_DIPNAME( 0x01, 0x00, "IN 16-0 (STRB 16 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 16-1 (STRB 16 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 16-2 (STRB 16 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 16-3 (STRB 16 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 16-4 (STRB 16 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-17")
	PORT_DIPNAME( 0x01, 0x00, "IN 17-0 (STRB 17 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 17-1 (STRB 17 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 17-2 (STRB 17 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 17-3 (STRB 17 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 17-4 (STRB 17 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-18")
	PORT_DIPNAME( 0x01, 0x00, "IN 18-0 (STRB 18 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 18-1 (STRB 18 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 18-2 (STRB 18 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 18-3 (STRB 18 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 18-4 (STRB 18 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-19")
	PORT_DIPNAME( 0x01, 0x00, "IN 19-0 (STRB 19 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 19-1 (STRB 19 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 19-2 (STRB 19 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 19-3 (STRB 19 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 19-4 (STRB 19 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-20")
	PORT_DIPNAME( 0x01, 0x00, "IN 20-0 (STRB 20 Data 0)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 20-1 (STRB 20 Data 1)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 20-2 (STRB 20 Data 2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 20-3 (STRB 20 Data 3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 20-4 (STRB 20 Data 4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT(           0xffe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN-COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // 1 PND
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // 50p
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) // 20p
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) // 10p
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 ) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 ) // 5p
INPUT_PORTS_END


INPUT_PORTS_START( sc4_base ) // just some fairly generic defaults we map to games where there isn't a specific mapping yet
	PORT_INCLUDE ( sc4_raw )
	PORT_MODIFY("IN-1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_1_0 ) PORT_NAME("IN1-0")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_1_1 ) PORT_NAME("IN1-1")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_1_2 ) PORT_NAME("IN1-2")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_1_3 ) PORT_NAME("IN1-3")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_1_4 ) PORT_NAME("IN1-4")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_1_5 ) PORT_NAME("IN1-5")

	PORT_MODIFY("IN-2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_2_0 ) PORT_NAME("IN2-0")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_2_1 ) PORT_NAME("IN2-1")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_2_2 ) PORT_NAME("IN2-2")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_2_3) PORT_NAME("IN2-3")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_2_4) PORT_NAME("IN2-4")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_2_5) PORT_NAME("IN2-5")

	PORT_MODIFY("IN-3")
	PORT_DIPNAME( 0x04, 0x00, "IN 3-2 (STK 4  3.2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 3-3 (STK 2  3.3)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IN 3-4 (STK 3  3.4)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_MODIFY("IN-5")
	PORT_DIPNAME( 0x01, 0x00, "IN 5-0 (PRIZ4 5.0)" ) // (PRIZ4 5.0)  // Jackpot key
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 5-1 (PRIZ35.1)" ) // (PRI31 5.1)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 5-2 (PRIZ2 5.2)" ) // (PRIZ2 5.2)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 5-3 (PRIZ1 5.3)" ) // (PRIZ1 5.3)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )

	PORT_MODIFY("IN-6")
	PORT_DIPNAME( 0x01, 0x00, "IN 6-0 (PERC1 6.0)" ) // (PERC1 6.0)  // %age key
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "IN 6-1 (PERC2 6.1)" ) // (PERC2 6.1)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "IN 6-2 (PERC3 6.2)" ) // (PERC3 6.2)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "IN 6-3 (PERC4 6.3)" ) // (PERC4 6.3)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )

	PORT_MODIFY("IN-7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_7_0) PORT_NAME("IN7-0")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_7_1) PORT_NAME("IN7-1")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_7_2) PORT_NAME("IN7-2")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_7_3) PORT_NAME("IN7-3")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_7_4) PORT_NAME("IN7-4")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_7_5) PORT_NAME("IN7-5")

	PORT_MODIFY("IN-8")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_8_0) PORT_NAME("IN8-0")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_8_1) PORT_NAME("IN8-1")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_8_2) PORT_NAME("IN8-2")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_8_3) PORT_NAME("IN8-3")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_8_4) PORT_NAME("IN8-4")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_8_5) PORT_NAME("IN8-5")

	PORT_MODIFY("IN-9")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_9_0) PORT_NAME("IN9-0")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_9_1) PORT_NAME("IN9-1")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_9_2) PORT_NAME("IN9-2")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_9_3) PORT_NAME("IN9-3")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_9_4) PORT_NAME("IN9-4")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_9_5) PORT_NAME("IN9-5")

	PORT_MODIFY("IN-10")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_10_0) PORT_NAME("IN10-0")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_10_1) PORT_NAME("IN10-1")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_10_2) PORT_NAME("IN10-2")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_10_3) PORT_NAME("IN10-3")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_10_4) PORT_NAME("IN10-4")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_10_5) PORT_NAME("IN10-5")

	PORT_MODIFY("IN-20")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_20_0) PORT_NAME("Green Button")
	PORT_DIPNAME( 0x0002, 0x0000, "Door Lock" ) // DORLOK20.1
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Service Door" ) // SERDOR20.2
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "Cashbox Door" ) // CSHDOR20.3
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Hopper DMP" ) // HOPDMP20.4
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
INPUT_PORTS_END
