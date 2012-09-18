#include "emu.h"
#include "video/stic.h"
#include "includes/intv.h"
#include "cpu/cp1610/cp1610.h"
#include "hashfile.h"

#define INTELLIVOICE_MASK	0x02
#define ECS_MASK			0x01



WRITE16_MEMBER( intv_state::intvkbd_dualport16_w )
{
	unsigned char *RAM;

	COMBINE_DATA(&m_intvkbd_dualport_ram[offset]);

	/* copy the LSB over to the 6502 OP RAM, in case they are opcodes */
	RAM	 = memregion("keyboard")->base();
	RAM[offset] = (UINT8) (data >> 0);
}

READ8_MEMBER( intv_state::intvkbd_dualport8_lsb_r )
{
	return (UINT8) (m_intvkbd_dualport_ram[offset] >> 0);
}

WRITE8_MEMBER( intv_state::intvkbd_dualport8_lsb_w )
{
	unsigned char *RAM;

	m_intvkbd_dualport_ram[offset] &= ~0x00FF;
	m_intvkbd_dualport_ram[offset] |= ((UINT16) data) << 0;

	/* copy over to the 6502 OP RAM, in case they are opcodes */
	RAM	 = memregion("keyboard")->base();
	RAM[offset] = data;
}



READ8_MEMBER( intv_state::intvkbd_dualport8_msb_r )
{
	unsigned char rv;

	if (offset < 0x100)
	{
		switch (offset)
		{
			case 0x000:
				rv = ioport("TEST")->read() & 0x80;
				logerror("TAPE: Read %02x from 0x40%02x - XOR Data?\n",rv,offset);
				break;
			case 0x001:
				rv = (ioport("TEST")->read() & 0x40) << 1;
				logerror("TAPE: Read %02x from 0x40%02x - Sense 1?\n",rv,offset);
				break;
			case 0x002:
				rv = (ioport("TEST")->read() & 0x20) << 2;
				logerror("TAPE: Read %02x from 0x40%02x - Sense 2?\n",rv,offset);
				break;
			case 0x003:
				rv = (ioport("TEST")->read() & 0x10) << 3;
				logerror("TAPE: Read %02x from 0x40%02x - Tape Present\n",rv,offset);
				break;
			case 0x004:
				rv = (ioport("TEST")->read() & 0x08) << 4;
				logerror("TAPE: Read %02x from 0x40%02x - Comp (339/1)\n",rv,offset);
				break;
			case 0x005:
				rv = (ioport("TEST")->read() & 0x04) << 5;
				logerror("TAPE: Read %02x from 0x40%02x - Clocked Comp (339/13)\n",rv,offset);
				break;
			case 0x006:
				if (m_sr1_int_pending)
					rv = 0x00;
				else
					rv = 0x80;
				logerror("TAPE: Read %02x from 0x40%02x - SR1 Int Pending\n",rv,offset);
				break;
			case 0x007:
				if (m_tape_int_pending)
					rv = 0x00;
				else
					rv = 0x80;
				logerror("TAPE: Read %02x from 0x40%02x - Tape? Int Pending\n",rv,offset);
				break;
			case 0x060:	/* Keyboard Read */
				rv = 0xff;
				if (m_intvkbd_keyboard_col == 0)
					rv = ioport("ROW0")->read();
				if (m_intvkbd_keyboard_col == 1)
					rv = ioport("ROW1")->read();
				if (m_intvkbd_keyboard_col == 2)
					rv = ioport("ROW2")->read();
				if (m_intvkbd_keyboard_col == 3)
					rv = ioport("ROW3")->read();
				if (m_intvkbd_keyboard_col == 4)
					rv = ioport("ROW4")->read();
				if (m_intvkbd_keyboard_col == 5)
					rv = ioport("ROW5")->read();
				if (m_intvkbd_keyboard_col == 6)
					rv = ioport("ROW6")->read();
				if (m_intvkbd_keyboard_col == 7)
					rv = ioport("ROW7")->read();
				if (m_intvkbd_keyboard_col == 8)
					rv = ioport("ROW8")->read();
				if (m_intvkbd_keyboard_col == 9)
					rv = ioport("ROW9")->read();
				break;
			case 0x80:
				rv = 0x00;
				logerror("TAPE: Read %02x from 0x40%02x, clear tape int pending\n",rv,offset);
				m_tape_int_pending = 0;
				break;
			case 0xa0:
				rv = 0x00;
				logerror("TAPE: Read %02x from 0x40%02x, clear SR1 int pending\n",rv,offset);
				m_sr1_int_pending = 0;
				break;
			case 0xc0:
			case 0xc1:
			case 0xc2:
			case 0xc3:
			case 0xc4:
			case 0xc5:
			case 0xc6:
			case 0xc7:
			case 0xc8:
			case 0xc9:
			case 0xca:
			case 0xcb:
			case 0xcc:
			case 0xcd:
			case 0xce:
			case 0xcf:
				/* TMS9927 regs */
				rv = intvkbd_tms9927_r(space, offset-0xc0);
				break;
			default:
				rv = (m_intvkbd_dualport_ram[offset]&0x0300)>>8;
				logerror("Unknown read %02x from 0x40%02x\n",rv,offset);
				break;
		}
		return rv;
	}
	else
		return (m_intvkbd_dualport_ram[offset]&0x0300)>>8;
}

static const char *const tape_motor_mode_desc[8] =
{
	"IDLE", "IDLE", "IDLE", "IDLE",
	"EJECT", "PLAY/RECORD", "REWIND", "FF"
};

WRITE8_MEMBER( intv_state::intvkbd_dualport8_msb_w )
{
	unsigned int mask;

	if (offset < 0x100)
	{
		switch (offset)
		{
			case 0x020:
				m_tape_motor_mode &= 3;
				if (data & 1)
					m_tape_motor_mode |= 4;
				logerror("TAPE: Motor Mode: %s\n",tape_motor_mode_desc[m_tape_motor_mode]);
				break;
			case 0x021:
				m_tape_motor_mode &= 5;
				if (data & 1)
					m_tape_motor_mode |= 2;
				logerror("TAPE: Motor Mode: %s\n",tape_motor_mode_desc[m_tape_motor_mode]);
				break;
			case 0x022:
				m_tape_motor_mode &= 6;
				if (data & 1)
					m_tape_motor_mode |= 1;
				logerror("TAPE: Motor Mode: %s\n",tape_motor_mode_desc[m_tape_motor_mode]);
				break;
			case 0x023:
			case 0x024:
			case 0x025:
			case 0x026:
			case 0x027:
				m_tape_unknown_write[offset - 0x23] = (data & 1);
				break;
			case 0x040:
				m_tape_unknown_write[5] = (data & 1);
				break;
			case 0x041:
				if (data & 1)
					logerror("TAPE: Tape Interrupts Enabled\n");
				else
					logerror("TAPE: Tape Interrupts Disabled\n");
				m_tape_interrupts_enabled = (data & 1);
				break;
			case 0x042:
				if (data & 1)
					logerror("TAPE: Cart Bus Interrupts Disabled\n");
				else
					logerror("TAPE: Cart Bus Interrupts Enabled\n");
				break;
			case 0x043:
				if (data & 0x01)
					m_intvkbd_text_blanked = 0;
				else
					m_intvkbd_text_blanked = 1;
				break;
			case 0x044:
				m_intvkbd_keyboard_col &= 0x0e;
				m_intvkbd_keyboard_col |= (data&0x01);
				break;
			case 0x045:
				m_intvkbd_keyboard_col &= 0x0d;
				m_intvkbd_keyboard_col |= ((data&0x01)<<1);
				break;
			case 0x046:
				m_intvkbd_keyboard_col &= 0x0b;
				m_intvkbd_keyboard_col |= ((data&0x01)<<2);
				break;
			case 0x047:
				m_intvkbd_keyboard_col &= 0x07;
				m_intvkbd_keyboard_col |= ((data&0x01)<<3);
				break;
			case 0x80:
				logerror("TAPE: Write to 0x40%02x, clear tape int pending\n",offset);
				m_tape_int_pending = 0;
				break;
			case 0xa0:
				logerror("TAPE: Write to 0x40%02x, clear SR1 int pending\n",offset);
				m_sr1_int_pending = 0;
				break;
			case 0xc0:
			case 0xc1:
			case 0xc2:
			case 0xc3:
			case 0xc4:
			case 0xc5:
			case 0xc6:
			case 0xc7:
			case 0xc8:
			case 0xc9:
			case 0xca:
			case 0xcb:
			case 0xcc:
			case 0xcd:
			case 0xce:
			case 0xcf:
				/* TMS9927 regs */
				intvkbd_tms9927_w(space, offset-0xc0, data);
				break;
			default:
				logerror("%04X: Unknown write %02x to 0x40%02x\n",space.device().safe_pc(),data,offset);
				break;
		}
	}
	else
	{
		mask = m_intvkbd_dualport_ram[offset] & 0x00ff;
		m_intvkbd_dualport_ram[offset] = mask | ((data<<8)&0x0300);
	}
}

READ16_MEMBER( intv_state::intv_gram_r )
{
	//logerror("read: %d = GRAM(%d)\n",state->m_gram[offset],offset);
	if (m_bus_copy_mode || !m_stic_handshake)
	{
	return (int)m_gram[offset];
	}
	else {return offset;}

}

WRITE16_MEMBER( intv_state::intv_gram_w )
{
	data &= 0xFF;
	if(m_bus_copy_mode || !m_stic_handshake)
	{
		m_gram[offset] = data;
		m_gramdirtybytes[offset] = 1;
	m_gramdirty = 1;
   }
}


READ16_MEMBER( intv_state::intv_ram8_r )
{
	//logerror("%x = ram8_r(%x)\n",state->m_ram8[offset],offset);
	return (int)m_ram8[offset];
}

WRITE16_MEMBER( intv_state::intv_ram8_w )
{
	//logerror("ram8_w(%x) = %x\n",offset,data);
	m_ram8[offset] = data&0xff;
}

READ16_MEMBER( intv_state::intv_ecs_ram8_r )
{
	return (int)m_ecs_ram8[offset];
}

WRITE16_MEMBER( intv_state::intv_ecs_ram8_w )
{
	m_ecs_ram8[offset] = data&0xff;
}

READ16_MEMBER( intv_state::intv_cart_ram8_r )
{
	return (int)m_cart_ram8[offset];
}
WRITE16_MEMBER( intv_state::intv_cart_ram8_w )
{
	m_cart_ram8[offset] = data&0xff;
}

READ16_MEMBER( intv_state::intv_ram16_r )
{
	//logerror("%x = ram16_r(%x)\n",state->m_ram16[offset],offset);
	return (int)m_ram16[offset];
}

WRITE16_MEMBER( intv_state::intv_ram16_w )
{
	//logerror("%g: WRITING TO GRAM offset = %d\n",machine.time(),offset);
	//logerror("ram16_w(%x) = %x\n",offset,data);
	m_ram16[offset] = data&0xffff;
}

// ECS and Wsmlb bank switching register handlers
WRITE16_MEMBER( intv_state::ecs_bank1_page_select )
{
	if (offset == 0xFFF)
	{
		if (data == 0x2A50)
		{
			membank("bank1")->set_base(machine().root_device().memregion("maincpu")->base() + (0x2000 << 1));
		}
		else if (data == 0x2A51)
		{
			membank("bank1")->set_base(machine().root_device().memregion("ecs_rom")->base() + (0x2000 << 1));
		}
	}
}

WRITE16_MEMBER( intv_state::ecs_bank2_page_select )
{
	if (offset == 0xFFF)
	{
		if (data == 0x7A50)
		{
			membank("bank2")->set_base(machine().root_device().memregion("ecs_rom")->base() + (0x7000 << 1)); // ECS ROM at 0x7000 is on page 1
		}
		else if (data == 0x7A51 )
		{
			membank("bank2")->set_base(machine().root_device().memregion("maincpu")->base() + (0x7000 << 1));
		}
	}
}

WRITE16_MEMBER( intv_state::ecs_bank3_page_select )
{
	if (offset == 0xFFF)
	{
		if (data == 0xEA50)
		{
			membank("bank3")->set_base(machine().root_device().memregion("maincpu")->base() + (0xE000 << 1));
		}
		else if (data == 0xEA51)
		{
			membank("bank3")->set_base(machine().root_device().memregion("ecs_rom")->base() + (0xE000 << 1));
		}
	}
}

WRITE16_MEMBER( intv_state::wsmlb_bank_page_select )
{
		logerror("offset %x data %x\n",offset,data);
	if (offset == 0xFFF)
	{
		if (data == 0xFA50)
		{
			membank("bank4")->set_base(machine().root_device().memregion("maincpu")->base() + (0xF000 << 1));
		}
		else if (data == 0xFA51)
		{
			membank("bank4")->set_base(machine().root_device().memregion("ecs_rom")->base() + (0xF000 << 1));
		}
	}
}

static int intv_load_rom_file(device_image_interface &image)
{
    int i,j;

	UINT8 temp;
	UINT8 num_segments;
	UINT8 start_seg;
	UINT8 end_seg;

	UINT32 current_address;
	UINT32 end_address;

	UINT8 high_byte;
	UINT8 low_byte;

	UINT8 *memory = image.device().machine().root_device().memregion("maincpu")->base();
	intv_state *state = image.device().machine().driver_data<intv_state>();
	address_space *program = image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);
	const char *filetype = image.filetype();

	/* if it is in .rom format, we enter here */
	if (!mame_stricmp (filetype, "rom"))
	{
		image.fread( &temp, 1);			/* header */
		if (temp != 0xa8)
		{
			return IMAGE_INIT_FAIL;
		}

		image.fread( &num_segments, 1);

		image.fread( &temp, 1);
		if (temp != (num_segments ^ 0xff))
		{
			return IMAGE_INIT_FAIL;
		}

		for (i = 0; i < num_segments; i++)
		{
			image.fread( &start_seg, 1);
			current_address = start_seg * 0x100;

			image.fread( &end_seg, 1);
			end_address = end_seg * 0x100 + 0xff;

			while (current_address <= end_address)
			{
				image.fread( &low_byte, 1);
				memory[(current_address << 1) + 1] = low_byte;
				image.fread( &high_byte, 1);
				memory[current_address << 1] = high_byte;
				current_address++;
			}

			/* Here we should calculate and compare the CRC16... */
			image.fread( &temp, 1);
			image.fread( &temp, 1);
		}

		/* Access tables and fine address restriction tables are not supported ATM */
		for (i = 0; i < (16 + 32 + 2); i++)
		{
			image.fread( &temp, 1);
		}
		return IMAGE_INIT_PASS;
	}
	/* otherwise, we load it as a .bin file, using extrainfo from intv.hsi in place of .cfg */
	else
	{
		/* This code is a blatant hack, due to impossibility to load a separate .cfg file in MESS. */
		/* It shall be eventually replaced by the .xml loading */

		/* extrainfo format */
		// 1. mapper number (to deal with bankswitch). no bankswitch is mapper 0 (most games).
		// 2.->5. current images have at most 4 chunks of data. we store here block size and location to load
		//  (value & 0xf0) >> 4 is the location / 0x1000
		//  (value & 0x0f) is the size / 0x800
		// 6. some images have a ram chunk. as above we store location and size in 8 bits
		// 7. extra = 1 ECS, 2 Intellivoice
		int start, size;
		int mapper, rom[5], ram, extra;
		const char *extrainfo = hashfile_extrainfo(image);
		if (!extrainfo)
		{
			/* If no extrainfo, we assume a single 0x2000 chunk at 0x5000 */
			for (i = 0; i < 0x2000; i++ )
			{
				image.fread( &low_byte, 1);
				memory[((0x5000 + i) << 1) + 1] = low_byte;
				image.fread( &high_byte, 1);
				memory[(0x5000 + i) << 1] = high_byte;
			}
		}
		else
		{
			sscanf(extrainfo,"%d %d %d %d %d %d %d", &mapper, &rom[0], &rom[1], &rom[2],
																&rom[3], &ram, &extra);

//          logerror("extrainfo: %d %d %d %d %d %d %d \n", mapper, rom[0], rom[1], rom[2],
//                                                              rom[3], ram, extra);

			if (mapper)
			{
				logerror("Bankswitch not yet implemented! \n");
			}

			if (ram)
			{
				start = (( ram & 0xf0 ) >> 4) * 0x1000;
				size = ( ram & 0x0f ) * 0x800;

				program->install_readwrite_handler(start, start + size,
					read16_delegate( FUNC( intv_state::intv_cart_ram8_r ), state),
					write16_delegate( FUNC( intv_state::intv_cart_ram8_w ), state));
			}
			/* For now intellivoice always active
            if (extra & INTELLIVOICE_MASK)
            {
                // tbd
            }
         */

			if (extra & ECS_MASK)
			{
				logerror("Requires ECS Module\n");
			}

			for (j = 0; j < 4; j++)
			{
				start = (( rom[j] & 0xf0 ) >> 4) * 0x1000;
				size = ( rom[j] & 0x0f ) * 0x800;

				/* some cart has to be loaded to 0x4800, but none goes to 0x4000. Hence, we use */
				/* 0x04 << 4 in extrainfo (to reduce the stored values) and fix the value here. */
				if (start == 0x4000) start += 0x800;

//              logerror("step %d: %d %d \n", j, start / 0x1000, size / 0x1000);

				for (i = 0; i < size; i++ )
				{
					image.fread( &low_byte, 1);
					memory[((start + i) << 1) + 1] = low_byte;
					image.fread( &high_byte, 1);
					memory[(start + i) << 1] = high_byte;
				}
			}
		}

		return IMAGE_INIT_PASS;
	}
}

DEVICE_IMAGE_LOAD( intv_cart )
{

	if (image.software_entry() == NULL)
		return intv_load_rom_file(image);
	else
	{
		UINT16 offset[] = {0x4800, 0x5000, 0x6000, 0x7000, 0x9000, 0xa000, 0xc000, 0xd000, 0xf000};
		const char* region_name[] = {"4800", "5000", "6000", "7000", "9000", "A000", "C000", "D000", "F000"};
		UINT8 *memory = image.device().machine().root_device().memregion("maincpu")->base();
		intv_state *state = image.device().machine().driver_data<intv_state>();
		address_space *program = image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);

		UINT32 size=0;
		UINT16 address = 0;
		UINT8 *region;
		for(int i = 0; i < 9; i++)
		{
			address = offset[i];
			size = image.get_software_region_length(region_name[i]);
			if (size)
			{
				region = image.get_software_region(region_name[i]);
				for (int j = 0; j < (size>>1); j++)
				{
					memory[((address + j) << 1) + 1] = region[2*j];
					memory[(address + j) << 1] = region[2*j+1];
				}
			}
		}
		// deal with wsmlb paged rom

		UINT8 *ecs_rom_region = image.device().machine().root_device().memregion("ecs_rom")->base();
		size = image.get_software_region_length("F000_bank1");
		if (size && ecs_rom_region) // only load if ecs is plugged in (should probably be done a different way)
		{
			region = image.get_software_region("F000_bank1");
			for (int j = 0; j < (size>>1); j++)
			{
				ecs_rom_region[((address + j) << 1) + 1] = region[2*j];
				ecs_rom_region[(address + j) << 1] = region[2*j+1];
			}
		}

		// Cartridge 8bit ram support
		size = image.get_software_region_length("D000_RAM8");
		if (size)
		{
			program->install_readwrite_handler(0xD000, 0xD000 + size,
				read16_delegate( FUNC( intv_state::intv_cart_ram8_r ), state),
				write16_delegate( FUNC( intv_state::intv_cart_ram8_w ), state));
		}

		size = image.get_software_region_length("8800_RAM8");
		if (size)
		{
			program->install_readwrite_handler(0x8800, 0x8800 + size,
				read16_delegate( FUNC( intv_state::intv_cart_ram8_r ), state),
				write16_delegate( FUNC( intv_state::intv_cart_ram8_w ), state));
		}
		return IMAGE_INIT_PASS;
	}
}

#ifdef UNUSED_FUNCTION
DRIVER_INIT_MEMBER(intv_state,intv)
{
}
#endif

/* Set Reset and INTR/INTRM Vector */
void intv_state::machine_reset()
{
	machine().device("maincpu")->execute().set_input_line_vector(CP1610_RESET, 0x1000);

	/* These are actually the same vector, and INTR is unused */
	machine().device("maincpu")->execute().set_input_line_vector(CP1610_INT_INTRM, 0x1004);
	machine().device("maincpu")->execute().set_input_line_vector(CP1610_INT_INTR,  0x1004);

	/* Set initial PC */
	machine().device("maincpu")->state().set_state_int(CP1610_R7, 0x1000);

	return;
}

MACHINE_RESET_MEMBER(intv_state,intvecs)
{
	membank("bank1")->set_base(machine().root_device().memregion("maincpu")->base() + (0x2000 << 1));
	membank("bank2")->set_base(machine().root_device().memregion("ecs_rom")->base() + (0x7000 << 1));
	membank("bank3")->set_base(machine().root_device().memregion("maincpu")->base() + (0xE000 << 1));
	membank("bank4")->set_base(machine().root_device().memregion("maincpu")->base() + (0xF000 << 1));

	machine().device("maincpu")->execute().set_input_line_vector(CP1610_RESET, 0x1000);

	/* These are actually the same vector, and INTR is unused */
	machine().device("maincpu")->execute().set_input_line_vector(CP1610_INT_INTRM, 0x1004);
	machine().device("maincpu")->execute().set_input_line_vector(CP1610_INT_INTR,  0x1004);

	/* Set initial PC */
	machine().device("maincpu")->state().set_state_int(CP1610_R7, 0x1000);

	return;
}


static TIMER_CALLBACK(intv_interrupt_complete)
{
	intv_state *state = machine.driver_data<intv_state>();
	machine.device("maincpu")->execute().set_input_line(CP1610_INT_INTRM, CLEAR_LINE);
	state->m_bus_copy_mode = 0;
}

static TIMER_CALLBACK(intv_btb_fill)
{
	intv_state *state = machine.driver_data<intv_state>();
	UINT8 column;
	UINT8 row = state->m_backtab_row;
	//machine.device("maincpu")->execute().adjust_icount(-STIC_ROW_FETCH);
	for(column=0; column < STIC_BACKTAB_WIDTH; column++)
	{
		state->m_backtab_buffer[row][column] = state->m_ram16[column + row * STIC_BACKTAB_WIDTH];
	}

	state->m_backtab_row += 1;
}

INTERRUPT_GEN_MEMBER(intv_state::intv_interrupt)
{
	machine().device("maincpu")->execute().set_input_line(CP1610_INT_INTRM, ASSERT_LINE);
	m_sr1_int_pending = 1;
	m_bus_copy_mode = 1;
	m_backtab_row = 0;
	UINT8 row;
	machine().device("maincpu")->execute().adjust_icount(-(12*STIC_ROW_BUSRQ+STIC_FRAME_BUSRQ)); // Account for stic cycle stealing
	machine().scheduler().timer_set(machine().device<cpu_device>("maincpu")
		->cycles_to_attotime(STIC_VBLANK_END), FUNC(intv_interrupt_complete));
	for (row=0; row < STIC_BACKTAB_HEIGHT; row++)
	{
		machine().scheduler().timer_set(machine().device<cpu_device>("maincpu")
			->cycles_to_attotime(STIC_FIRST_FETCH-STIC_FRAME_BUSRQ+STIC_CYCLES_PER_SCANLINE*STIC_Y_SCALE*m_row_delay + (STIC_CYCLES_PER_SCANLINE*STIC_Y_SCALE*STIC_CARD_HEIGHT - STIC_ROW_BUSRQ)*row), FUNC(intv_btb_fill));
	}

	if (m_row_delay == 0)
	{
		machine().device("maincpu")->execute().adjust_icount(-STIC_ROW_BUSRQ); // extra row fetch occurs if vertical delay == 0
	}

	intv_stic_screenrefresh(machine());
}

/* hand 0 == left, 1 == right, 2 == ECS hand controller 1, 3 == ECS hand controller 2 */
UINT8 intv_control_r(address_space &space, int hand)
{
	static const char* const keypad_name[] = { "KEYPAD1", "KEYPAD2", "KEYPAD3", "KEYPAD4" };
	static const UINT8 keypad_table[] =
	{
		0xFF, 0x3F, 0x9F, 0x5F, 0xD7, 0xB7, 0x77, 0xDB,
		0xBB, 0x7B, 0xDD, 0xBD, 0x7D, 0xDE, 0xBE, 0x7E
	};

	static const char* const disc_name[] = { "DISC1", "DISC2", "DISC3", "DISC4" };
	static const UINT8 disc_table[] =
	{
		0xF3, 0xE3, 0xE7, 0xF7, 0xF6, 0xE6, 0xEE, 0xFE,
		0xFC, 0xEC, 0xED, 0xFD, 0xF9, 0xE9, 0xEB, 0xFB
	};

	static const char* const discx_name[] = { "DISCX1", "DISCX2", "DISCX3", "DISCX4" };
	static const char* const discy_name[] = { "DISCY1", "DISCY2", "DISKY3", "DISCY4" };
	static const UINT8 discyx_table[5][5] =
	{
		{ 0xE3, 0xF3, 0xFB, 0xEB, 0xE9 },
		{ 0xE7, 0xE3, 0xFB, 0xE9, 0xF9 },
		{ 0xF7, 0xF7, 0xFF, 0xFD, 0xFD },
		{ 0xF6, 0xE6, 0xFE, 0xEC, 0xED },
		{ 0xE6, 0xEE, 0xFE, 0xFC, 0xEC }
	};

	int x, y;
	UINT8 rv = 0xFF;

	/* keypad */
	x = space.machine().root_device().ioport(keypad_name[hand])->read();
	for (y = 0; y < 16; y++)
	{
		if (x & (1 << y))
		{
			rv &= keypad_table[y];
		}
	}

	switch ((space.machine().root_device().ioport("OPTIONS")->read() >> hand) & 1)
	{
		case 0: /* disc == digital */
		default:

			x = space.machine().root_device().ioport(disc_name[hand])->read();
			for (y = 0; y < 16; y++)
			{
				if (x & (1 << y))
				{
					rv &= disc_table[y];
				}
			}
			break;

		case 1: /* disc == _fake_ analog */

			x = space.machine().root_device().ioport(discx_name[hand])->read();
			y = space.machine().root_device().ioport(discy_name[hand])->read();
			rv &= discyx_table[y / 32][x / 32];
	}

	return rv;
}

READ8_MEMBER( intv_state::intv_left_control_r )
{
	return intv_control_r(space, 0);
}

READ8_MEMBER( intv_state::intv_right_control_r )
{
	return intv_control_r(space, 1);
}

READ8_MEMBER( intv_state::intv_ecs_porta_r )
{
	if (ioport("ECS_CNTRLSEL")->read() == 0)
		return intv_control_r(space, 2);
	else
		return 0xff; // not sure what to return here, maybe it should be last output?
}

READ8_MEMBER( intv_state::intv_ecs_portb_r )
{
	switch (ioport("ECS_CNTRLSEL")->read())
	{
		case 0x00: // hand controller
		{
			return intv_control_r(space, 3);
		}
		case 0x01: // synthesizer keyboard
		{
			static const char* const ecs_synth_rows[] = { "ECS_SYNTH_ROW0", "ECS_SYNTH_ROW1", "ECS_SYNTH_ROW2", "ECS_SYNTH_ROW3", "ECS_SYNTH_ROW4", "ECS_SYNTH_ROW5", "ECS_SYNTH_ROW6" };
			UINT8 rv = 0xFF;
			// return correct result if more than one bit of 0xFE is set
			for (int i=0; i<7; i++)
			{
			if (m_ecs_psg_porta & (1<<i))
				rv &= ioport(ecs_synth_rows[i])->read();
			}
			return rv;
		}
		case 0x02: // ecs keyboard
		{
			static const char* const ecs_keyboard_rows[] = { "ECS_ROW0", "ECS_ROW1", "ECS_ROW2", "ECS_ROW3", "ECS_ROW4", "ECS_ROW5", "ECS_ROW6" };
			UINT8 rv = 0xFF;
			// return correct result if more than one bit of 0xFE is set
			for (int i=0; i<7; i++)
			{
			if (m_ecs_psg_porta & (1<<i))
				rv &= ioport(ecs_keyboard_rows[i])->read();
			}
			return rv;
		}
		default:
			return 0xff;
	}
}

WRITE8_MEMBER( intv_state::intv_ecs_porta_w )
{
	m_ecs_psg_porta = (~data) & 0xFF;
}

/* Intellivision console + keyboard component */

DEVICE_IMAGE_LOAD( intvkbd_cart )
{
	if (strcmp(image.device().tag(),":cart1") == 0) /* Legacy cartridge slot */
	{
		/* First, initialize these as empty so that the intellivision
         * will think that the playcable is not attached */
		UINT8 *memory = image.device().machine().root_device().memregion("maincpu")->base();

		/* assume playcable is absent */
		memory[0x4800 << 1] = 0xff;
		memory[(0x4800 << 1) + 1] = 0xff;

		intv_load_rom_file(image);
	}

	if (strcmp(image.device().tag(),":cart2") == 0) /* Keyboard component cartridge slot */
	{
		UINT8 *memory = image.device().machine().root_device().memregion("keyboard")->base();

		/* Assume an 8K cart, like BASIC */
		image.fread( &memory[0xe000], 0x2000);
	}

	return IMAGE_INIT_PASS;

}
