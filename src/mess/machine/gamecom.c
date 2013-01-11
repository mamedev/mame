
#include "includes/gamecom.h"



static const int gamecom_timer_limit[8] = { 2, 1024, 2048, 4096, 8192, 16384, 32768, 65536 };

TIMER_CALLBACK_MEMBER(gamecom_state::gamecom_clock_timer_callback)
{
	UINT8 * RAM = machine().root_device().memregion("maincpu")->base();
	UINT8 val = ( ( RAM[SM8521_CLKT] & 0x3F ) + 1 ) & 0x3F;
	RAM[SM8521_CLKT] = ( RAM[SM8521_CLKT] & 0xC0 ) | val;
	machine().device("maincpu")->execute().set_input_line(CK_INT, ASSERT_LINE );
}

void gamecom_state::machine_reset()
{
	UINT8 *rom = memregion("kernel")->base();
	membank( "bank1" )->set_base( rom );
	membank( "bank2" )->set_base( rom );
	membank( "bank3" )->set_base( rom );
	membank( "bank4" )->set_base( rom );

	m_cartridge = NULL;
}

void gamecom_state::gamecom_set_mmu(UINT8 mmu, UINT8 data )
{
	char bank[8];
	sprintf(bank,"bank%d",mmu);
	if (data < 0x20)
	{
		/* select internal ROM bank */
		membank( bank )->set_base( memregion("kernel")->base() + (data << 13) );
	}
	else
	{
		/* select cartridge bank */
		if ( m_cartridge )
			membank( bank )->set_base( m_cartridge + ( data << 13 ) );
	}
}

void gamecom_state::handle_stylus_press( int column )
{
	static const UINT16 row_data[17] = { 0x3FE, 0x3FD, 0x3FB, 0x3F7, 0x3EF, 0x3DF, 0x3BF, 0x37F, 0x2FF, 0x1FF };

	if ( column == 0 )
	{
		if ( !BIT( ioport("IN2")->read(), 2) )
		{
			m_stylus_x = ioport("STYX")->read() >> 4;
			m_stylus_y = ioport("STYY")->read() >> 4;
		}
		else
		{
			m_stylus_x = 16;
			m_stylus_y = 16;
		}
	}

	if ( m_stylus_x == column )
	{
		m_p_ram[SM8521_P0] = row_data[m_stylus_y];
		m_p_ram[SM8521_P1] = ( m_p_ram[SM8521_P1] & 0xFC ) | ( ( row_data[m_stylus_y] >> 8 ) & 3 );
	}
	else
	{
		m_p_ram[SM8521_P0] = 0xFF;
		m_p_ram[SM8521_P1] = ( m_p_ram[SM8521_P1] & 0xFC ) | 3;
	}
}

WRITE8_MEMBER( gamecom_state::gamecom_pio_w )
{
	offset += 0x14;
	m_p_ram[offset] = data;
	switch( offset )
	{
	case SM8521_P2:     switch( ( m_p_ram[SM8521_P1] << 8 ) | data )
				{
				case 0xFBFF:    /* column #0 */
						/* P0 bit 0 cleared => 01 */
						/* P0 bit 1 cleared => 0E */
						/* P0 bit 2 cleared => 1B */
						/* P0 bit 3 cleared => etc */
						/* P0 bit 4 cleared => */
						/* P0 bit 5 cleared => */
						/* P0 bit 6 cleared => */
						/* P0 bit 7 cleared => */
						/* P1 bit 0 cleared => */
						/* P1 bit 1 cleared => */
					handle_stylus_press(0);
					break;
				case 0xF7FF:    /* column #1 */
					handle_stylus_press(1);
					break;
				case 0xEFFF:    /* column #2 */
					handle_stylus_press(2);
					break;
				case 0xDFFF:    /* column #3 */
					handle_stylus_press(3);
					break;
				case 0xBFFF:    /* column #4 */
					handle_stylus_press(4);
					break;
				case 0x7FFF:    /* column #5 */
					handle_stylus_press(5);
					break;
				case 0xFFFE:    /* column #6 */
					handle_stylus_press(6);
					break;
				case 0xFFFD:    /* column #7 */
					handle_stylus_press(7);
					break;
				case 0xFFFB:    /* column #8 */
					handle_stylus_press(8);
					break;
				case 0xFFF7:    /* column #9 */
					handle_stylus_press(9);
					break;
				case 0xFFEF:    /* column #10 */
					handle_stylus_press(10);
					break;
				case 0xFFDF:    /* column #11 */
					handle_stylus_press(11);
					break;
				case 0xFFBF:    /* column #12 */
					handle_stylus_press(12);
					break;
				case 0xFF7F:    /* keys #1 */
						/* P0 bit 0 cleared => 83 (up) */
						/* P0 bit 1 cleared => 84 (down) */
						/* P0 bit 2 cleared => 85 (left) */
						/* P0 bit 3 cleared => 86 (right) */
						/* P0 bit 4 cleared => 87 (menu) */
						/* P0 bit 5 cleared => 8A (pause) */
						/* P0 bit 6 cleared => 89 (sound) */
						/* P0 bit 7 cleared => 8B (button A) */
						/* P1 bit 0 cleared => 8C (button B) */
						/* P1 bit 1 cleared => 8D (button C) */
					m_p_ram[SM8521_P0] = ioport("IN0")->read();
					m_p_ram[SM8521_P1] = (m_p_ram[SM8521_P1] & 0xFC) | ( ioport("IN1")->read() & 3 );
					break;
				case 0xFFFF:    /* keys #2 */
						/* P0 bit 0 cleared => 88 (power) */
						/* P0 bit 1 cleared => 8E (button D) */
						/* P0 bit 2 cleared => A0 */
						/* P0 bit 3 cleared => A0 */
						/* P0 bit 4 cleared => A0 */
						/* P0 bit 5 cleared => A0 */
						/* P0 bit 6 cleared => A0 */
						/* P0 bit 7 cleared => A0 */
						/* P1 bit 0 cleared => A0 */
						/* P1 bit 1 cleared => A0 */
					m_p_ram[SM8521_P0] = (m_p_ram[SM8521_P0] & 0xFC) | ( ioport("IN2")->read() & 3 );
					m_p_ram[SM8521_P1] = 0xFF;
					break;
				}
				return;
	case SM8521_P3:
				/* P3 bit7 clear, bit6 set -> enable cartridge port #0? */
				/* P3 bit6 clear, bit7 set -> enable cartridge port #1? */
				switch( data & 0xc0 )
				{
				case 0x40: m_cartridge = m_cartridge1; break;
				case 0x80: m_cartridge = m_cartridge2; break;
				default:   m_cartridge = NULL;       break;
				}
				return;
	}
}

READ8_MEMBER( gamecom_state::gamecom_pio_r )
{
	return m_p_ram[offset + 0x14];
}

READ8_MEMBER( gamecom_state::gamecom_internal_r )
{
	return m_p_ram[offset + 0x20];
}

WRITE8_MEMBER( gamecom_state::gamecom_internal_w )
{
	offset += 0x20;
	switch( offset )
	{
	case SM8521_MMU0:   /* disable bootstrap ROM? most likely not written to on game.com */
		logerror( "Write to MMU0\n" );
		break;
	case SM8521_MMU1:
		gamecom_set_mmu(1, data);
		break;
	case SM8521_MMU2:
		gamecom_set_mmu(2, data);
		break;
	case SM8521_MMU3:
		gamecom_set_mmu(3, data);
		break;
	case SM8521_MMU4:
		gamecom_set_mmu(4, data);
		break;

	/* Video hardware and DMA */
	case SM8521_DMBR:
		data &= 0x7f;
		break;
	case SM8521_TM0D:
		m_timer[0].check_value = data;
		return;
	case SM8521_TM0C:
		m_timer[0].enabled = data & 0x80;
		m_timer[0].state_limit = gamecom_timer_limit[data & 0x07];
		m_timer[0].state_count = 0;
		m_p_ram[SM8521_TM0D] = 0;
		break;
	case SM8521_TM1D:
		m_timer[1].check_value = data;
		return;
	case SM8521_TM1C:
		m_timer[1].enabled = data & 0x80;
		m_timer[1].state_limit = gamecom_timer_limit[data & 0x07];
		m_timer[1].state_count = 0;
		m_p_ram[SM8521_TM1D] = 0;
		break;
	case SM8521_CLKT:   /* bit 6-7 */
		if ( data & 0x80 )
		{
			/* timer run */
			if ( data & 0x40 )
			{
				/* timer resolution 1 minute */
				m_clock_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(60));
			}
			else
			{
				/* TImer resolution 1 second */
				m_clock_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
			}
		}
		else
		{
			/* disable timer reset */
			m_clock_timer->enable( 0 );
			data &= 0xC0;
		}
		break;

	/* Sound */
	case SM8521_SGC:
		m_sound.sgc = data;
		break;
	case SM8521_SG0L:
		m_sound.sg0l = data;
		break;
	case SM8521_SG1L:
		m_sound.sg1l = data;
		break;
	case SM8521_SG0TH:
		m_sound.sg0t = ( m_sound.sg0t & 0xFF ) | ( data << 8 );
		break;
	case SM8521_SG0TL:
		m_sound.sg0t = ( m_sound.sg0t & 0xFF00 ) | data;
		break;
	case SM8521_SG1TH:
		m_sound.sg1t = ( m_sound.sg1t & 0xFF ) | ( data << 8 );
		break;
	case SM8521_SG1TL:
		m_sound.sg1t = ( m_sound.sg1t & 0xFF00 ) | data;
		break;
	case SM8521_SG2L:
		m_sound.sg2l = data;
		break;
	case SM8521_SG2TH:
		m_sound.sg2t = ( m_sound.sg2t & 0xFF ) | ( data << 8 );
		break;
	case SM8521_SG2TL:
		m_sound.sg2t = ( m_sound.sg2t & 0xFF00 ) | data;
		break;
	case SM8521_SGDA:
		m_sound.sgda = data;
		break;

	case SM8521_SG0W0:
	case SM8521_SG0W1:
	case SM8521_SG0W2:
	case SM8521_SG0W3:
	case SM8521_SG0W4:
	case SM8521_SG0W5:
	case SM8521_SG0W6:
	case SM8521_SG0W7:
	case SM8521_SG0W8:
	case SM8521_SG0W9:
	case SM8521_SG0W10:
	case SM8521_SG0W11:
	case SM8521_SG0W12:
	case SM8521_SG0W13:
	case SM8521_SG0W14:
	case SM8521_SG0W15:
		m_sound.sg0w[offset - SM8521_SG0W0] = data;
		break;
	case SM8521_SG1W0:
	case SM8521_SG1W1:
	case SM8521_SG1W2:
	case SM8521_SG1W3:
	case SM8521_SG1W4:
	case SM8521_SG1W5:
	case SM8521_SG1W6:
	case SM8521_SG1W7:
	case SM8521_SG1W8:
	case SM8521_SG1W9:
	case SM8521_SG1W10:
	case SM8521_SG1W11:
	case SM8521_SG1W12:
	case SM8521_SG1W13:
	case SM8521_SG1W14:
	case SM8521_SG1W15:
		m_sound.sg1w[offset - SM8521_SG1W0] = data;
		break;

	/* Reserved addresses */
	case SM8521_18: case SM8521_1B:
	case SM8521_29: case SM8521_2A: case SM8521_2F:
	case SM8521_33: case SM8521_3E: case SM8521_3F:
	case SM8521_41: case SM8521_43: case SM8521_45: case SM8521_4B:
	case SM8521_4F:
	case SM8521_55: case SM8521_56: case SM8521_57: case SM8521_58:
	case SM8521_59: case SM8521_5A: case SM8521_5B: case SM8521_5C:
	case SM8521_5D:
		logerror( "%X: Write to reserved address (0x%02X). Value written: 0x%02X\n", m_maincpu->pc(), offset, data );
		break;
	}
	m_p_ram[offset] = data;
}


/* The manual is not conclusive as to which bit of the DMVP register (offset 0x3D) determines
   which page for source or destination is used */
/* For now the increment/decrement-x and increment/decrement-y parts are NOT supported.
   Their usage is also not explained properly in the manuals. Guess we'll have to wait
   for them to show up in some rom images...
 */
void gamecom_handle_dma( device_t *device, int cycles )
{
	gamecom_state *state = device->machine().driver_data<gamecom_state>();
	UINT8 * RAM = state->memregion("maincpu")->base();
	UINT8 data = RAM[SM8521_DMC];
	state->m_dma.overwrite_mode = data & 0x01;
	state->m_dma.transfer_mode = data & 0x06;
	state->m_dma.decrement_x = data & 0x08;
	state->m_dma.decrement_y = data & 0x10;
	state->m_dma.enabled = data & 0x80;
	if ( !state->m_dma.enabled ) return;


	if ( state->m_dma.decrement_x || state->m_dma.decrement_y )
		logerror( "TODO: Decrement-x and decrement-y are not supported yet\n" );

	state->m_dma.width_x = RAM[SM8521_DMDX];
	state->m_dma.width_x_count = 0;
	state->m_dma.width_y = RAM[SM8521_DMDY];
	state->m_dma.width_y_count = 0;
	state->m_dma.source_x = RAM[SM8521_DMX1];
	state->m_dma.source_x_current = state->m_dma.source_x;
	state->m_dma.source_y = RAM[SM8521_DMY1];
	state->m_dma.source_width = ( RAM[SM8521_LCH] & 0x20 ) ? 50 : 40;
	state->m_dma.dest_x = RAM[SM8521_DMX2];
	state->m_dma.dest_x_current = state->m_dma.dest_x;
	state->m_dma.dest_y = RAM[SM8521_DMY2];
	state->m_dma.dest_width = ( RAM[SM8521_LCH] & 0x20 ) ? 50 : 40;
	state->m_dma.palette[0] = RAM[SM8521_DMPL] & 0x03;
	state->m_dma.palette[1] = ( RAM[SM8521_DMPL] >> 2 ) & 3;
	state->m_dma.palette[2] = ( RAM[SM8521_DMPL] >> 4 ) & 3;
	state->m_dma.palette[3] = RAM[SM8521_DMPL] >> 6;
	state->m_dma.source_mask = 0x1FFF;
	state->m_dma.dest_mask = 0x1FFF;
//  logerror("DMA: width %Xx%X, source (%X,%X), dest (%X,%X), transfer_mode %X, banks %X \n", state->m_dma.width_x, state->m_dma.width_y, state->m_dma.source_x, state->m_dma.source_y, state->m_dma.dest_x, state->m_dma.dest_y, state->m_dma.transfer_mode, RAM[SM8521_DMVP] );
//  logerror( "   Palette: %d, %d, %d, %d\n", state->m_dma.palette[0], state->m_dma.palette[1], state->m_dma.palette[2], state->m_dma.palette[3] );
	switch( state->m_dma.transfer_mode )
	{
	case 0x00:
		/* VRAM->VRAM */
		state->m_dma.source_bank = &state->m_p_videoram[(RAM[SM8521_DMVP] & 0x01) ? 0x2000 : 0x0000];
		state->m_dma.dest_bank = &state->m_p_videoram[(RAM[SM8521_DMVP] & 0x02) ? 0x2000 : 0x0000];
		break;
	case 0x02:
		/* ROM->VRAM */
//      logerror( "DMA DMBR = %X\n", RAM[SM8521_DMBR] );
		state->m_dma.source_width = 64;
		state->m_dma.source_mask = 0x3FFF;
		if ( RAM[SM8521_DMBR] < 16 )
			state->m_dma.source_bank = state->memregion("kernel")->base() + (RAM[SM8521_DMBR] << 14);
		else
		if (state->m_cartridge)
			state->m_dma.source_bank = state->m_cartridge + (RAM[SM8521_DMBR] << 14);

		state->m_dma.dest_bank = &state->m_p_videoram[(RAM[SM8521_DMVP] & 0x02) ? 0x2000 : 0x0000];
		break;
	case 0x04:
		/* Extend RAM->VRAM */
		state->m_dma.source_width = 64;
		state->m_dma.source_bank = &state->m_p_nvram[0x0000];
		state->m_dma.dest_bank = &state->m_p_videoram[(RAM[SM8521_DMVP] & 0x02) ? 0x2000 : 0x0000];
		break;
	case 0x06:
		/* VRAM->Extend RAM */
		state->m_dma.source_bank = &state->m_p_videoram[(RAM[SM8521_DMVP] & 0x01) ? 0x2000 : 0x0000];
		state->m_dma.dest_width = 64;
		state->m_dma.dest_bank = &state->m_p_nvram[0x0000];
		break;
	}
	state->m_dma.source_current = state->m_dma.source_width * state->m_dma.source_y;
	state->m_dma.source_current += state->m_dma.source_x >> 2;
	state->m_dma.dest_current = state->m_dma.dest_width * state->m_dma.dest_y;
	state->m_dma.dest_current += state->m_dma.dest_x >> 2;
	state->m_dma.source_line = state->m_dma.source_current;
	state->m_dma.dest_line = state->m_dma.dest_current;
	state->m_dma.state_count = 0;

	unsigned y_count, x_count;

	for( y_count = 0; y_count <= state->m_dma.width_y; y_count++ )
	{
		for( x_count = 0; x_count <= state->m_dma.width_x; x_count++ )
		{
			int source_pixel = 0;
			int dest_pixel = 0;
			int src_addr = state->m_dma.source_current & state->m_dma.source_mask;
			int dest_addr = state->m_dma.dest_current & state->m_dma.dest_mask;
			/* handle DMA for 1 pixel */
			/* Read pixel data */
			switch ( state->m_dma.source_x_current & 0x03 )
			{
			case 0x00: source_pixel = state->m_dma.source_bank[src_addr] >> 6; break;
			case 0x01: source_pixel = ( state->m_dma.source_bank[src_addr] >> 4 ) & 3; break;
			case 0x02: source_pixel = ( state->m_dma.source_bank[src_addr] >> 2 ) & 3; break;
			case 0x03: source_pixel = state->m_dma.source_bank[src_addr] & 3;      break;
			}

			if ( !state->m_dma.overwrite_mode && source_pixel == 0 )
			{
				switch ( state->m_dma.dest_x_current & 0x03 )
				{
				case 0x00: dest_pixel = state->m_dma.dest_bank[dest_addr] >> 6; break;
				case 0x01: dest_pixel = ( state->m_dma.dest_bank[dest_addr] >> 4 ) & 3; break;
				case 0x02: dest_pixel = ( state->m_dma.dest_bank[dest_addr] >> 2 ) & 3; break;
				case 0x03: dest_pixel = state->m_dma.dest_bank[dest_addr] & 3;      break;
				}
				source_pixel = dest_pixel;
			}

			/* Translate pixel data using DMA palette. */
			/* Not sure if this should be done before the compound stuff - WP */
			source_pixel = state->m_dma.palette[ source_pixel ];
			/* Write pixel data */
			switch( state->m_dma.dest_x_current & 0x03 )
			{
			case 0x00:
				state->m_dma.dest_bank[dest_addr] = ( state->m_dma.dest_bank[dest_addr] & 0x3F ) | ( source_pixel << 6 );
				break;
			case 0x01:
				state->m_dma.dest_bank[dest_addr] = ( state->m_dma.dest_bank[dest_addr] & 0xCF ) | ( source_pixel << 4 );
				break;
			case 0x02:
				state->m_dma.dest_bank[dest_addr] = ( state->m_dma.dest_bank[dest_addr] & 0xF3 ) | ( source_pixel << 2 );
				break;
			case 0x03:
				state->m_dma.dest_bank[dest_addr] = ( state->m_dma.dest_bank[dest_addr] & 0xFC ) | source_pixel;
				break;
			}

			/* Advance a pixel */
			if ( state->m_dma.decrement_x )
			{
				state->m_dma.source_x_current--;
				if ( ( state->m_dma.source_x_current & 0x03 ) == 0x03 )
					state->m_dma.source_current--;
			}
			else
			{
				state->m_dma.source_x_current++;
				if ( ( state->m_dma.source_x_current & 0x03 ) == 0x00 )
					state->m_dma.source_current++;
			}
			state->m_dma.dest_x_current++;
			if ( ( state->m_dma.dest_x_current & 0x03 ) == 0x00 )
				state->m_dma.dest_current++;
		}

		/* Advance a line */
		state->m_dma.source_x_current = state->m_dma.source_x;
		state->m_dma.dest_x_current = state->m_dma.dest_x;
		state->m_dma.source_line += state->m_dma.source_width;
		state->m_dma.source_current = state->m_dma.source_line;
		state->m_dma.dest_line += state->m_dma.dest_width;
		state->m_dma.dest_current = state->m_dma.dest_line;
	}
	state->m_dma.enabled = 0;
	device->machine().device("maincpu")->execute().set_input_line(DMA_INT, ASSERT_LINE );
}

void gamecom_update_timers( device_t *device, int cycles )
{
	gamecom_state *state = device->machine().driver_data<gamecom_state>();
	UINT8 * RAM = state->memregion("maincpu")->base();
	if ( state->m_timer[0].enabled )
	{
		state->m_timer[0].state_count += cycles;
		while ( state->m_timer[0].state_count >= state->m_timer[0].state_limit )
		{
			state->m_timer[0].state_count -= state->m_timer[0].state_limit;
			RAM[SM8521_TM0D]++;
			if ( RAM[SM8521_TM0D] >= state->m_timer[0].check_value )
			{
				RAM[SM8521_TM0D] = 0;
				device->machine().device("maincpu")->execute().set_input_line(TIM0_INT, ASSERT_LINE );
			}
		}
	}
	if ( state->m_timer[1].enabled )
	{
		state->m_timer[1].state_count += cycles;
		while ( state->m_timer[1].state_count >= state->m_timer[1].state_limit )
		{
			state->m_timer[1].state_count -= state->m_timer[1].state_limit;
			RAM[SM8521_TM1D]++;
			if ( RAM[SM8521_TM1D] >= state->m_timer[1].check_value )
			{
				RAM[SM8521_TM1D] = 0;
				device->machine().device("maincpu")->execute().set_input_line(TIM1_INT, ASSERT_LINE );
			}
		}
	}
}

DRIVER_INIT_MEMBER(gamecom_state,gamecom)
{
	m_clock_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gamecom_state::gamecom_clock_timer_callback),this));
	m_p_ram = memregion("maincpu")->base(); // required here because pio_w gets called before machine_reset
}

DEVICE_IMAGE_LOAD( gamecom_cart1 )
{
	gamecom_state *state = image.device().machine().driver_data<gamecom_state>();
	UINT32 filesize;
	UINT32 load_offset = 0;

	state->m_cartridge1 = state->memregion("cart1")->base();

	if (image.software_entry() == NULL)
		filesize = image.length();
	else
		filesize = image.get_software_region_length("rom");

	switch(filesize)
	{
		case 0x008000: load_offset = 0;        break;  /* 32 KB */
		case 0x040000: load_offset = 0;        break;  /* 256KB */
		case 0x080000: load_offset = 0;        break;  /* 512KB */
		case 0x100000: load_offset = 0;        break;  /* 1  MB */
		case 0x1c0000: load_offset = 0x040000; break;  /* 1.8MB */
		case 0x200000: load_offset = 0;        break;  /* 2  MB */
		default:                                       /* otherwise */
			logerror("Error loading cartridge: Invalid file size 0x%X\n", filesize);
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unhandled cart size");
			return IMAGE_INIT_FAIL;
	}

	if (image.software_entry() == NULL)
	{
		if (image.fread( state->m_cartridge1 + load_offset, filesize) != filesize)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to load all of the cart");
			return IMAGE_INIT_FAIL;
		}
	}
	else
		memcpy(state->m_cartridge1 + load_offset, image.get_software_region("rom"), filesize);

	if (filesize < 0x010000) { memcpy(state->m_cartridge1 + 0x008000, state->m_cartridge1, 0x008000); } /* ->64KB */
	if (filesize < 0x020000) { memcpy(state->m_cartridge1 + 0x010000, state->m_cartridge1, 0x010000); } /* ->128KB */
	if (filesize < 0x040000) { memcpy(state->m_cartridge1 + 0x020000, state->m_cartridge1, 0x020000); } /* ->256KB */
	if (filesize < 0x080000) { memcpy(state->m_cartridge1 + 0x040000, state->m_cartridge1, 0x040000); } /* ->512KB */
	if (filesize < 0x100000) { memcpy(state->m_cartridge1 + 0x080000, state->m_cartridge1, 0x080000); } /* ->1MB */
	if (filesize < 0x1c0000) { memcpy(state->m_cartridge1 + 0x100000, state->m_cartridge1, 0x100000); } /* -> >=1.8MB */
	return IMAGE_INIT_PASS;
}

DEVICE_IMAGE_LOAD( gamecom_cart2 )
{
	gamecom_state *state = image.device().machine().driver_data<gamecom_state>();
	UINT32 filesize;
	UINT32 load_offset = 0;

	state->m_cartridge2 = state->memregion("cart2")->base();

//  if (image.software_entry() == NULL)
		filesize = image.length();
//  else
//      filesize = image.get_software_region_length("rom");

	switch(filesize)
	{
		case 0x008000: load_offset = 0;        break;  /* 32 KB */
		case 0x040000: load_offset = 0;        break;  /* 256KB */
		case 0x080000: load_offset = 0;        break;  /* 512KB */
		case 0x100000: load_offset = 0;        break;  /* 1  MB */
		case 0x1c0000: load_offset = 0x040000; break;  /* 1.8MB */
		case 0x200000: load_offset = 0;        break;  /* 2  MB */
		default:                                       /* otherwise */
			logerror("Error loading cartridge: Invalid file size 0x%X\n", filesize);
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unhandled cart size");
			return IMAGE_INIT_FAIL;
	}

//  if (image.software_entry() == NULL)
	{
		if (image.fread( state->m_cartridge2 + load_offset, filesize) != filesize)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to load all of the cart");
			return IMAGE_INIT_FAIL;
		}
	}
//  else
//      memcpy(state->m_cartridge2 + load_offset, image.get_software_region("rom"), filesize);

	if (filesize < 0x010000) { memcpy(state->m_cartridge2 + 0x008000, state->m_cartridge2, 0x008000); } /* ->64KB */
	if (filesize < 0x020000) { memcpy(state->m_cartridge2 + 0x010000, state->m_cartridge2, 0x010000); } /* ->128KB */
	if (filesize < 0x040000) { memcpy(state->m_cartridge2 + 0x020000, state->m_cartridge2, 0x020000); } /* ->256KB */
	if (filesize < 0x080000) { memcpy(state->m_cartridge2 + 0x040000, state->m_cartridge2, 0x040000); } /* ->512KB */
	if (filesize < 0x100000) { memcpy(state->m_cartridge2 + 0x080000, state->m_cartridge2, 0x080000); } /* ->1MB */
	if (filesize < 0x1c0000) { memcpy(state->m_cartridge2 + 0x100000, state->m_cartridge2, 0x100000); } /* -> >=1.8MB */
	return IMAGE_INIT_PASS;
}
