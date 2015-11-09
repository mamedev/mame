// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert

#include "includes/gamecom.h"



static const int gamecom_timer_limit[8] = { 2, 1024, 2048, 4096, 8192, 16384, 32768, 65536 };

TIMER_CALLBACK_MEMBER(gamecom_state::gamecom_clock_timer_callback)
{
	UINT8 val = m_p_ram[SM8521_CLKT] + 1;
	m_p_ram[SM8521_CLKT] = ( m_p_ram[SM8521_CLKT] & 0xC0 ) | (val & 0x3f);
	m_maincpu->set_input_line(sm8500_cpu_device::CK_INT, ASSERT_LINE );
}

void gamecom_state::machine_reset()
{
	UINT8 *rom = m_region_kernel->base();
	m_bank1->set_base(rom);
	m_bank2->set_base(rom);
	m_bank3->set_base(rom);
	m_bank4->set_base(rom);

	m_cart_ptr = NULL;
	m_lch_reg = 0x07;
	m_lcv_reg = 0x27;
	m_lcdc_reg = 0xb0;

	std::string region_tag;
	m_cart1_rom = memregion(region_tag.assign(m_cart1->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	m_cart2_rom = memregion(region_tag.assign(m_cart2->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
}

void gamecom_state::gamecom_set_mmu(UINT8 mmu, UINT8 data)
{
	if (data < 0x20)
	{
		/* select internal ROM bank */
		switch (mmu)
		{
			case 1: m_bank1->set_base(m_region_kernel->base() + (data << 13)); break;
			case 2: m_bank2->set_base(m_region_kernel->base() + (data << 13)); break;
			case 3: m_bank3->set_base(m_region_kernel->base() + (data << 13)); break;
			case 4: m_bank4->set_base(m_region_kernel->base() + (data << 13)); break;
		}
	}
	else
	{
		/* select cartridge bank */
		if (m_cart_ptr)
		{
			switch (mmu)
			{
				case 1: m_bank1->set_base(m_cart_ptr + (data << 13)); break;
				case 2: m_bank2->set_base(m_cart_ptr + (data << 13)); break;
				case 3: m_bank3->set_base(m_cart_ptr + (data << 13)); break;
				case 4: m_bank4->set_base(m_cart_ptr + (data << 13)); break;
			}
		}
	}
}

void gamecom_state::handle_stylus_press( int column )
{
	UINT16 data = m_io_grid[column]->read();
	if (data)
	{
		UINT16 stylus_y = data ^ 0x3ff;
		m_p_ram[SM8521_P0] = stylus_y;
		m_p_ram[SM8521_P1] = ( m_p_ram[SM8521_P1] & 0xFC ) | ( stylus_y >> 8 );
	}
	else
	{
		m_p_ram[SM8521_P0] = 0xFF;
		m_p_ram[SM8521_P1] |= 3;
	}
}

void gamecom_state::handle_input_press(UINT16 mux_data)
{
	switch( mux_data )
	{
		case 0xFFFB:
			/* column #0 */
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
		case 0xFFF7:    /* column #1 */
			handle_stylus_press(1);
			break;
		case 0xFFEF:    /* column #2 */
			handle_stylus_press(2);
			break;
		case 0xFFDF:    /* column #3 */
			handle_stylus_press(3);
			break;
		case 0xFFBF:    /* column #4 */
			handle_stylus_press(4);
			break;
		case 0xFF7F:    /* column #5 */
			handle_stylus_press(5);
			break;
		case 0xFEFF:    /* column #6 */
			handle_stylus_press(6);
			break;
		case 0xFDFF:    /* column #7 */
			handle_stylus_press(7);
			break;
		case 0xFBFF:    /* column #8 */
			handle_stylus_press(8);
			break;
		case 0xF7FF:    /* column #9 */
			handle_stylus_press(9);
			break;
		case 0xEFFF:    /* column #10 */
			handle_stylus_press(10);
			break;
		case 0xDFFF:    /* column #11 */
			handle_stylus_press(11);
			break;
		case 0xBFFF:    /* column #12 */
			handle_stylus_press(12);
			break;
		case 0x7FFF:    /* keys #1 */
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
			m_p_ram[SM8521_P0] = m_io_in0->read();
			m_p_ram[SM8521_P1] = (m_p_ram[SM8521_P1] & 0xFC) | ( m_io_in1->read() & 3 );
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
			m_p_ram[SM8521_P0] = (m_p_ram[SM8521_P0] & 0xFC) | ( m_io_in2->read() & 3 );
			m_p_ram[SM8521_P1] = 0xFF;
			break;
	}
}

WRITE8_MEMBER( gamecom_state::gamecom_pio_w )
{
	offset += 0x14;
	m_p_ram[offset] = data;
	switch (offset)
	{
		case SM8521_P1:
		case SM8521_P2:
			handle_input_press(m_p_ram[SM8521_P1] | (m_p_ram[SM8521_P2] << 8));
			break;
		case SM8521_P3:
				/* P3 bit7 clear, bit6 set -> enable cartridge port #0? */
				/* P3 bit6 clear, bit7 set -> enable cartridge port #1? */
				switch (data & 0xc0)
				{
				case 0x40: m_cart_ptr = m_cart1_rom->base(); break;
				case 0x80: m_cart_ptr = m_cart2_rom->base(); break;
				default:   m_cart_ptr = NULL;       break;
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
// ToDo: Read from vblank bit
//  if(SM8521_LCV == offset + 0x20)
//      popmessage("Read from vblank bit, TODO");

	return m_p_ram[offset + 0x20];
}

/* TODO: preliminary, proper formula not yet understood (and manual doesn't help much either) */
void gamecom_state::recompute_lcd_params()
{
	int vblank_period,hblank_period;
	int H_timing,V_timing;
	int pixel_clock;
	attoseconds_t refresh;

	if(m_lch_reg != 7)
		popmessage("LCH = %02x!",m_lch_reg);

	if((m_lcdc_reg & 0xf) != 0)
		popmessage("LCDC = %02x!",m_lcdc_reg);

	if(m_lcv_reg != 0x27)
		popmessage("LCV = %02x!",m_lcv_reg);

	H_timing = ((m_lch_reg & 0x1f) + 1) * 200/4;
	V_timing = (m_lcv_reg & 0x1f);
	pixel_clock = (XTAL_11_0592MHz / 2); // TODO: divisor actually settable

	rectangle visarea(0, 200-1, 0, 160-1); // TODO: check settings

	vblank_period = (V_timing + 160);
	hblank_period = (H_timing + 200);

	refresh  = HZ_TO_ATTOSECONDS(pixel_clock) * (hblank_period) * vblank_period;
	machine().first_screen()->configure((hblank_period), (vblank_period), visarea, refresh );
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
		m_timer[0].state_limit = gamecom_timer_limit[data & 0x07] >> 1;
		m_timer[0].state_count = 0;
		m_p_ram[SM8521_TM0D] = 0;
		break;
	case SM8521_TM1D:
		m_timer[1].check_value = data;
		return;
	case SM8521_TM1C:
		m_timer[1].enabled = data & 0x80;
		m_timer[1].state_limit = gamecom_timer_limit[data & 0x07] >> 1;
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

	case SM8521_LCDC:
		m_lcdc_reg = data;
		recompute_lcd_params();
		break;

	case SM8521_LCH:
		/*
		--x- ---- Horizontal DOT size (160 / 200)
		---x xxxx H-Timing bits
		*/
		m_lch_reg = data;
		recompute_lcd_params();
		break;

	case SM8521_LCV:
		/*
		x--- ---- V-blank bit (R)
		-xx- ---- V-line size bits (100 / 160 / 200 / undef)
		---x xxxx V-Blank width bits
		*/
		m_lcv_reg = data;
		recompute_lcd_params();
		break;

	/* Sound */
	case SM8521_SGC:
		/*
		x--- ---- enable sound output
		---- x--- enable DAC
		---- -x-- enable SG2
		---- --x- enable SG1
		---- ---x enable SG0
		*/
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
		if((m_sound.sgc & 0x88) == 0x88)
			m_dac->write_unsigned8(data);
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
WRITE8_MEMBER( gamecom_state::gamecom_handle_dma )
{
	UINT8 dmc = m_p_ram[SM8521_DMC];
	m_dma.overwrite_mode = dmc & 0x01;
	m_dma.transfer_mode = dmc & 0x06;
	m_dma.decrement_x = dmc & 0x08;
	m_dma.decrement_y = dmc & 0x10;
	m_dma.enabled = dmc & 0x80;
	if ( !m_dma.enabled )
	{
		return;
	}

	m_dma.width_x = m_p_ram[SM8521_DMDX];
	m_dma.width_x_count = 0;
	m_dma.width_y = m_p_ram[SM8521_DMDY];
	m_dma.width_y_count = 0;
	m_dma.source_x = m_p_ram[SM8521_DMX1];
	m_dma.source_x_current = m_dma.source_x;
	m_dma.source_y = m_p_ram[SM8521_DMY1];
	m_dma.source_width = ( m_p_ram[SM8521_LCH] & 0x20 ) ? 50 : 40;
	m_dma.dest_x = m_p_ram[SM8521_DMX2];
	m_dma.dest_x_current = m_dma.dest_x;
	m_dma.dest_y = m_p_ram[SM8521_DMY2];
	m_dma.dest_width = ( m_p_ram[SM8521_LCH] & 0x20 ) ? 50 : 40;
	m_dma.palette[0] = m_p_ram[SM8521_DMPL] & 0x03;
	m_dma.palette[1] = ( m_p_ram[SM8521_DMPL] >> 2 ) & 3;
	m_dma.palette[2] = ( m_p_ram[SM8521_DMPL] >> 4 ) & 3;
	m_dma.palette[3] = m_p_ram[SM8521_DMPL] >> 6;
	m_dma.source_mask = 0x1FFF;
	m_dma.dest_mask = 0x1FFF;
//  logerror("DMA: width %Xx%X, source (%X,%X), dest (%X,%X), transfer_mode %X, banks %X \n", m_dma.width_x, m_dma.width_y, m_dma.source_x, m_dma.source_y, m_dma.dest_x, m_dma.dest_y, m_dma.transfer_mode, m_p_ram[SM8521_DMVP] );
//  logerror( "   Palette: %d, %d, %d, %d\n", m_dma.palette[0], m_dma.palette[1], m_dma.palette[2], m_dma.palette[3] );
	switch( m_dma.transfer_mode )
	{
	case 0x00:
		/* VRAM->VRAM */
		m_dma.source_bank = &m_p_videoram[(m_p_ram[SM8521_DMVP] & 0x01) ? 0x2000 : 0x0000];
		m_dma.dest_bank = &m_p_videoram[(m_p_ram[SM8521_DMVP] & 0x02) ? 0x2000 : 0x0000];
		break;
	case 0x02:
		/* ROM->VRAM */
//      logerror( "DMA DMBR = %X\n", m_p_ram[SM8521_DMBR] );
		m_dma.source_width = 64;
		m_dma.source_mask = 0x3FFF;
		if (m_p_ram[SM8521_DMBR] < 16)
			m_dma.source_bank = m_region_kernel->base() + (m_p_ram[SM8521_DMBR] << 14);
		else
		if (m_cart_ptr)
			m_dma.source_bank = m_cart_ptr + (m_p_ram[SM8521_DMBR] << 14);

		m_dma.dest_bank = &m_p_videoram[(m_p_ram[SM8521_DMVP] & 0x02) ? 0x2000 : 0x0000];
		break;
	case 0x04:
		/* Extend RAM->VRAM */
		m_dma.source_width = 64;
		m_dma.source_bank = &m_p_nvram[0x0000];
		m_dma.dest_bank = &m_p_videoram[(m_p_ram[SM8521_DMVP] & 0x02) ? 0x2000 : 0x0000];
		break;
	case 0x06:
		/* VRAM->Extend RAM */
		m_dma.source_bank = &m_p_videoram[(m_p_ram[SM8521_DMVP] & 0x01) ? 0x2000 : 0x0000];
		m_dma.dest_width = 64;
		m_dma.dest_bank = &m_p_nvram[0x0000];
		break;
	}
	m_dma.source_current = m_dma.source_width * m_dma.source_y;
	m_dma.source_current += m_dma.source_x >> 2;
	m_dma.dest_current = m_dma.dest_width * m_dma.dest_y;
	m_dma.dest_current += m_dma.dest_x >> 2;
	m_dma.source_line = m_dma.source_current;
	m_dma.dest_line = m_dma.dest_current;
	m_dma.state_count = 0;

	unsigned y_count, x_count;

	for( y_count = 0; y_count <= m_dma.width_y; y_count++ )
	{
		for( x_count = 0; x_count <= m_dma.width_x; x_count++ )
		{
			UINT16 src_addr = m_dma.source_current & m_dma.source_mask;
			UINT16 dest_addr = m_dma.dest_current & m_dma.dest_mask;
			UINT8 dest_adj = (3 - (m_dma.dest_x_current & 3)) << 1;
			UINT8 src_adj = (3 - (m_dma.source_x_current & 3)) << 1;

			/* handle DMA for 1 pixel */
			// Get new pixel
			UINT8 source_pixel = (m_dma.source_bank[src_addr] >> src_adj) & 3;

			// If overwrite mode, write new pixel
			if ( m_dma.overwrite_mode || source_pixel)
			{
				// Get 4 pixels and remove the one about to be replaced
				UINT8 other_pixels = m_dma.dest_bank[dest_addr] & ~(3 << dest_adj);
				// Get palette of new pixel and place into the hole
				m_dma.dest_bank[dest_addr] = other_pixels | (m_dma.palette[ source_pixel ] << dest_adj);
			}

			/* Advance a pixel */
			if ( m_dma.decrement_x )
			{
				m_dma.source_x_current--;
				if ( ( m_dma.source_x_current & 0x03 ) == 0x03 )
					m_dma.source_current--;
			}
			else
			{
				m_dma.source_x_current++;
				if ( ( m_dma.source_x_current & 0x03 ) == 0x00 )
					m_dma.source_current++;
			}
			m_dma.dest_x_current++;
			if ( ( m_dma.dest_x_current & 0x03 ) == 0x00 )
				m_dma.dest_current++;
		}

		/* Advance a line */
		m_dma.source_x_current = m_dma.source_x;
		m_dma.dest_x_current = m_dma.dest_x;
		if ( m_dma.decrement_y )
			m_dma.source_line -= m_dma.source_width;
		else
			m_dma.source_line += m_dma.source_width;
		m_dma.source_current = m_dma.source_line;
		m_dma.dest_line += m_dma.dest_width;
		m_dma.dest_current = m_dma.dest_line;
	}
	m_dma.enabled = 0;
	m_maincpu->set_input_line(sm8500_cpu_device::DMA_INT, ASSERT_LINE );
}

WRITE8_MEMBER( gamecom_state::gamecom_update_timers )
{
	if ( m_timer[0].enabled )
	{
		m_timer[0].state_count += data;
		while ( m_timer[0].state_count >= m_timer[0].state_limit )
		{
			m_timer[0].state_count -= m_timer[0].state_limit;
			m_p_ram[SM8521_TM0D]++;
			if ( m_p_ram[SM8521_TM0D] >= m_timer[0].check_value )
			{
				m_p_ram[SM8521_TM0D] = 0;
				m_maincpu->set_input_line(sm8500_cpu_device::TIM0_INT, ASSERT_LINE );
			}
		}
	}
	if ( m_timer[1].enabled )
	{
		m_timer[1].state_count += data;
		while ( m_timer[1].state_count >= m_timer[1].state_limit )
		{
			m_timer[1].state_count -= m_timer[1].state_limit;
			m_p_ram[SM8521_TM1D]++;
			if ( m_p_ram[SM8521_TM1D] >= m_timer[1].check_value )
			{
				m_p_ram[SM8521_TM1D] = 0;
				m_maincpu->set_input_line(sm8500_cpu_device::TIM1_INT, ASSERT_LINE );
			}
		}
	}
}

DRIVER_INIT_MEMBER(gamecom_state,gamecom)
{
	m_clock_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gamecom_state::gamecom_clock_timer_callback),this));
	m_p_ram = m_region_maincpu->base(); // required here because pio_w gets called before machine_reset
}

int gamecom_state::common_load(device_image_interface &image, generic_slot_device *slot)
{
	UINT32 size = slot->common_get_size("rom");
	UINT32 load_offset = 0;

	if (size != 0x008000 && size != 0x040000 && size != 0x080000
			&& size != 0x100000 && size != 0x1c0000 && size != 0x200000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	if (size == 0x1c0000)
		load_offset = 0x40000;

	// in order to simplify banked access from the driver, we always allocate 0x200000,
	slot->rom_alloc(0x200000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	// we load what we have
	slot->common_load_rom(slot->get_rom_base() + load_offset, size, "rom");
	// and then we mirror the content, instead of masking out larger accesses
	UINT8 *crt = slot->get_rom_base();
	if (size < 0x010000) { memcpy(crt + 0x008000, crt, 0x008000); } /* ->64KB */
	if (size < 0x020000) { memcpy(crt + 0x010000, crt, 0x010000); } /* ->128KB */
	if (size < 0x040000) { memcpy(crt + 0x020000, crt, 0x020000); } /* ->256KB */
	if (size < 0x080000) { memcpy(crt + 0x040000, crt, 0x040000); } /* ->512KB */
	if (size < 0x100000) { memcpy(crt + 0x080000, crt, 0x080000); } /* ->1MB */
	if (size < 0x1c0000) { memcpy(crt + 0x100000, crt, 0x100000); } /* -> >=1.8MB */

	return IMAGE_INIT_PASS;
}

DEVICE_IMAGE_LOAD_MEMBER( gamecom_state, gamecom_cart1 )
{
	return common_load(image, m_cart1);
}

DEVICE_IMAGE_LOAD_MEMBER( gamecom_state, gamecom_cart2 )
{
	return common_load(image, m_cart2);
}
