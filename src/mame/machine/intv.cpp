// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
#include "emu.h"
#include "video/stic.h"
#include "video/tms9927.h"
#include "includes/intv.h"
#include "cpu/cp1610/cp1610.h"



WRITE16_MEMBER( intv_state::intvkbd_dualport16_w )
{
	unsigned char *RAM;

	COMBINE_DATA(&m_intvkbd_dualport_ram[offset]);

	/* copy the LSB over to the 6502 OP RAM, in case they are opcodes */
	RAM  = m_region_keyboard->base();
	RAM[offset] = (uint8_t) (data >> 0);
}

READ8_MEMBER( intv_state::intvkbd_dualport8_lsb_r )
{
	return (uint8_t) (m_intvkbd_dualport_ram[offset] >> 0);
}

WRITE8_MEMBER( intv_state::intvkbd_dualport8_lsb_w )
{
	unsigned char *RAM;

	m_intvkbd_dualport_ram[offset] &= ~0x00FF;
	m_intvkbd_dualport_ram[offset] |= ((uint16_t) data) << 0;

	/* copy over to the 6502 OP RAM, in case they are opcodes */
	RAM = m_region_keyboard->base();
	RAM[offset] = data;
}

READ8_MEMBER( intv_state::intvkbd_dualport8_msb_r )
{
	unsigned char rv;

	if (offset < 0x100)
	{
		switch (offset)
		{
			// These next 8 locations all map to bit7
			case 0x000:
				// "Data from Cassette"
				// Tape drive does the decoding to bits
				rv = m_io_test->read() & 0x80;
				break;
			case 0x001:
				// "Watermark"
				// 0 = Drive Busy Executing Command?, 1 = Drive Ok?
				rv = (m_io_test->read() & 0x40) << 1;
				break;
			case 0x002:
				// "End of Tape"
				// 0 = Recordable surface, 1 = Leader Detect
				// (Leader is transparent, optical sensor)
				rv = (m_io_test->read() & 0x20) << 2;
				//logerror("TAPE: Read %02x from 0x40%02x - Sense 2?\n",rv,offset);
				break;
			case 0x003:
				// "Cassette Present"
				// 0 = Tape Present, 1 = Tape Not Present
				rv = (m_io_test->read() & 0x10) << 3;
				//logerror("TAPE: Read %02x from 0x40%02x - Tape Present\n",rv,offset);
				break;
			case 0x004:
				// "NOT Inter Record Gap (IRG)"
				// 0 = Not Playing/Recording?, 1 = Playing/Recording?
				rv = (m_io_test->read() & 0x08) << 4;
				//logerror("TAPE: Read %02x from 0x40%02x - Comp (339/1)\n",rv,offset);
				break;
			case 0x005:
				// "Dropout"
				// 0 = Data Detect, 1 = No Data
				rv = (m_io_test->read() & 0x04) << 5;
				//logerror("TAPE: Read %02x from 0x40%02x - Clocked Comp (339/13)\n",rv,offset);
				break;
			case 0x006:
				// "NOT Clock Interrupt"
				if (m_sr1_int_pending)
					rv = 0x00;
				else
					rv = 0x80;
				//logerror("TAPE: Read %02x from 0x40%02x - SR1 Int Pending\n",rv,offset);
				break;
			case 0x007:
				// "NOT Tape Interrupt"
				if (m_tape_int_pending)
					rv = 0x00;
				else
					rv = 0x80;
				//logerror("TAPE: Read %02x from 0x40%02x - Tape? Int Pending\n",rv,offset);
				break;
			case 0x060:
				// "Read Keyboard"
				rv = 0xff;
				if (m_intvkbd_keyboard_col < 10)
					rv = m_intv_keyboard[m_intvkbd_keyboard_col]->read();
				break;
			case 0x80:
				// "Clear Tape Interrupt"
				rv = 0x00;
				//logerror("TAPE: Read %02x from 0x40%02x, clear tape int pending\n",rv,offset);
				m_tape_int_pending = 0;
				break;
			case 0xa0:
				// "Clear Clock Interrupt"
				rv = 0x00;
				//logerror("TAPE: Read %02x from 0x40%02x, clear SR1 int pending\n",rv,offset);
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
				rv = m_crtc->read(space, offset-0xc0);
				break;
			default:
				rv = (m_intvkbd_dualport_ram[offset]&0x0300)>>8;
				//logerror("Unknown read %02x from 0x40%02x\n",rv,offset);
				break;
		}
		return rv;
	}
	else
		return (m_intvkbd_dualport_ram[offset]&0x0300)>>8;
}

static bool not_busy = 1;	// printer state
static bool not_paper = 0; // printer state - we always have paper
static bool not_busy_enable = 0;  // interface state?

READ8_MEMBER( intv_state::intvkbd_periph_r )
{
	uint8_t value = 0;
	switch(offset) {
		case 0x06:
			if (not_busy_enable)
				if (not_busy)
					value |= 0x80;
			if (not_paper)
				value |= 0x10;
			//logerror("PeriphRead:  0x%04x->0x%02x\n",offset,value);
			
			// After one query of busy, 
			// next time the state is not_busy
			if (not_busy == 0) 
				not_busy = 1;
				
			return value;
		break;
		case 0x07:
		
		default:
			//logerror("PeriphRead:  0x%04x->0x%02x\n",offset,0xff);
			return 0xff;
		break;
	}
}

WRITE8_MEMBER( intv_state::intvkbd_periph_w )
{
	static FILE *fp = fopen("printer.txt","wb");
	
	switch(offset) {
		case 0x06:
			//logerror("PeriphWrite: 0x%04x->0x%02x\n",offset,data);
			if (data & 0x20)
				not_busy_enable = 1;
			else
				not_busy_enable = 0;
		break;
		case 0x07:
			//logerror("Printing: 0x%02x, %c\n",data,data);
			fputc(data, fp);
			fflush(fp);
			not_busy = 0;
		break;
		default:
			//logerror("PeriphWrite: 0x%04x->0x%02x\n",offset,data);
		break;
	}
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
			// Bits from offset $20 to $47 are all bit0, write only
			// These are all set to zero by system reset
			case 0x020:
				// "Tape Drive Control: Enable"
				m_tape_motor_mode &= 3;
				if (data & 1)
					m_tape_motor_mode |= 4;
				//logerror("TAPE: Motor Mode: %s\n",tape_motor_mode_desc[m_tape_motor_mode]);
				break;
			case 0x021:
				// "Tape Drive Control: Forward"
				m_tape_motor_mode &= 5;
				if (data & 1)
					m_tape_motor_mode |= 2;
				//logerror("TAPE: Motor Mode: %s\n",tape_motor_mode_desc[m_tape_motor_mode]);
				break;
			case 0x022:
				// "Tape Drive Control: Fast"
				m_tape_motor_mode &= 6;
				if (data & 1)
					m_tape_motor_mode |= 1;
				//logerror("TAPE: Motor Mode: %s\n",tape_motor_mode_desc[m_tape_motor_mode]);
				break;
			case 0x023:
				// "Tape Drive Control: Record"
				// 0=Read, 1=Write
				break;
			case 0x024:
				// "Tape Drive Control: Mute 1"
				// 0=Enable Channel B Audio, 1=Mute
				break;
			case 0x025:
				// "Tape Drive Control: Mute 2"
				// 0=Enable Channel A Audio, 1=Mute	
				break;
			case 0x026:
				// "Tape Drive Control: Mode"
				// If read mode:
				//	0=Read Channel B Data, 1 = Read Channel A Data
				// If write mode:
				//  0=Write Channel B data, 1 = Record Channel B Audio	
				break;
			case 0x027:
				// "Tape Drive Control: Erase"
				m_tape_unknown_write[offset - 0x23] = (data & 1);
				break;
			case 0x040:
				// Data to Tape
				m_tape_unknown_write[5] = (data & 1);
				break;
			case 0x041:
				// "Tape Interrupt Enable"
				//if (data & 1)
					//logerror("TAPE: Tape Interrupts Enabled\n");
				//else
					//logerror("TAPE: Tape Interrupts Disabled\n");
				m_tape_interrupts_enabled = (data & 1);
				break;
			case 0x042:
				// "NOT External Interrupt Enable"
				//if (data & 1)
					//logerror("TAPE: Cart Bus Interrupts Disabled\n");
				//else
					//logerror("TAPE: Cart Bus Interrupts Enabled\n");
				break;
			case 0x043:
				// "NOT Blank Screen"
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
				// "Clear Tape Interrupt"
				//logerror("TAPE: Write to 0x40%02x, clear tape int pending\n",offset);
				m_tape_int_pending = 0;
				break;
			case 0xa0:
				// "Clear Clock Interrupt"
				//logerror("TAPE: Write to 0x40%02x, clear SR1 int pending\n",offset);
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
				m_crtc->write(space, offset-0xc0, data);
				break;
			default:
				//logerror("%04X: Unknown write %02x to 0x40%02x\n",space.device().safe_pc(),data,offset);
				break;
		}
	}
	else
	{
		mask = m_intvkbd_dualport_ram[offset] & 0x00ff;
		m_intvkbd_dualport_ram[offset] = mask | ((data<<8)&0x0300);
	}
}


READ16_MEMBER( intv_state::intv_stic_r )
{
	if (m_bus_copy_mode || !m_stic->read_stic_handshake())
		return m_stic->read(space, offset, mem_mask);
	else
		return offset;
}

WRITE16_MEMBER( intv_state::intv_stic_w )
{
	if (m_bus_copy_mode || !m_stic->read_stic_handshake())
		m_stic->write(space, offset, data, mem_mask);
}


READ16_MEMBER( intv_state::intv_gram_r )
{
	//logerror("read: %d = GRAM(%d)\n",state->m_gram[offset],offset);
	if (m_bus_copy_mode || !m_stic->read_stic_handshake())
		return m_stic->gram_read(space, offset, mem_mask);
	else
		return offset;
}

WRITE16_MEMBER( intv_state::intv_gram_w )
{
	if (m_bus_copy_mode || !m_stic->read_stic_handshake())
		m_stic->gram_write(space, offset, data, mem_mask);
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

READ16_MEMBER( intv_state::intv_ram16_r )
{
	//logerror("%x = ram16_r(%x)\n",state->m_ram16[offset],offset);
	return (int)m_ram16[offset];
}

WRITE16_MEMBER( intv_state::intv_ram16_w )
{
	//logerror("%g: WRITING TO GRAM offset = %d\n",machine.time(),offset);
	//logerror("ram16_w(%x) = %x\n",offset,data);
	m_ram16[offset] = data & 0xffff;
}

READ8_MEMBER( intv_state::intvkb_iocart_r )
{
	if (m_iocart1->exists())
		return m_iocart1->read_rom(space, offset, mem_mask);
	else if (m_iocart2->exists())
		return m_iocart2->read_rom(space, offset, mem_mask);
	else
		return m_region_keyboard->as_u8(offset + 0xe000);
}


/* Set Reset and INTR/INTRM Vector */
void intv_state::machine_reset()
{
	m_maincpu->set_input_line_vector(CP1610_RESET, 0x1000);

	/* These are actually the same vector, and INTR is unused */
	m_maincpu->set_input_line_vector(CP1610_INT_INTRM, 0x1004);
	m_maincpu->set_input_line_vector(CP1610_INT_INTR,  0x1004);

	/* Set initial PC */
	m_maincpu->set_state_int(CP1610_R7, 0x1000);
}

void intv_state::machine_start()
{
	save_item(NAME(m_bus_copy_mode));
	save_item(NAME(m_backtab_row));
	save_item(NAME(m_ram16));
	save_item(NAME(m_sr1_int_pending));
	save_item(NAME(m_ram8));

	// intvkbd
	if (m_is_keybd)
	{
		for (int i = 0; i < 10; i++)
		{
			char str[5];
			sprintf(str, "ROW%i", i);
			m_intv_keyboard[i] = ioport(str);
		}

		save_item(NAME(m_intvkbd_text_blanked));
		save_item(NAME(m_intvkbd_keyboard_col));
		save_item(NAME(m_tape_int_pending));
		save_item(NAME(m_tape_interrupts_enabled));
		save_item(NAME(m_tape_unknown_write));
		save_item(NAME(m_tape_motor_mode));
	}

	if (m_cart && m_cart->exists())
	{
		// RAM
		switch (m_cart->get_type())
		{
			case INTV_RAM:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xd000, 0xd7ff, read16_delegate(FUNC(intv_cart_slot_device::read_ram),(intv_cart_slot_device*)m_cart), write16_delegate(FUNC(intv_cart_slot_device::write_ram),(intv_cart_slot_device*)m_cart));
				break;
			case INTV_GFACT:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x8800, 0x8fff, read16_delegate(FUNC(intv_cart_slot_device::read_ram),(intv_cart_slot_device*)m_cart), write16_delegate(FUNC(intv_cart_slot_device::write_ram),(intv_cart_slot_device*)m_cart));
				break;
			case INTV_VOICE:
				m_cart->late_subslot_setup();
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0080, 0x0081, read16_delegate(FUNC(intv_cart_slot_device::read_speech),(intv_cart_slot_device*)m_cart), write16_delegate(FUNC(intv_cart_slot_device::write_speech),(intv_cart_slot_device*)m_cart));

				// passthru for RAM-equipped carts
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x8800, 0x8fff, write16_delegate(FUNC(intv_cart_slot_device::write_88),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0xd000, 0xd7ff, write16_delegate(FUNC(intv_cart_slot_device::write_d0),(intv_cart_slot_device*)m_cart));
				break;
			case INTV_ECS:
				m_cart->late_subslot_setup();
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x00f0, 0x00ff, read16_delegate(FUNC(intv_cart_slot_device::read_ay),(intv_cart_slot_device*)m_cart), write16_delegate(FUNC(intv_cart_slot_device::write_ay),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x4000, 0x47ff, read16_delegate(FUNC(intv_cart_slot_device::read_ram),(intv_cart_slot_device*)m_cart), write16_delegate(FUNC(intv_cart_slot_device::write_ram),(intv_cart_slot_device*)m_cart));

				m_maincpu->space(AS_PROGRAM).install_write_handler(0x2000, 0x2fff, write16_delegate(FUNC(intv_cart_slot_device::write_rom20),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x7000, 0x7fff, write16_delegate(FUNC(intv_cart_slot_device::write_rom70),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0xe000, 0xefff, write16_delegate(FUNC(intv_cart_slot_device::write_rome0),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0xf000, 0xffff, write16_delegate(FUNC(intv_cart_slot_device::write_romf0),(intv_cart_slot_device*)m_cart));

				// passthru for Intellivoice expansion
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0080, 0x0081, read16_delegate(FUNC(intv_cart_slot_device::read_speech),(intv_cart_slot_device*)m_cart), write16_delegate(FUNC(intv_cart_slot_device::write_speech),(intv_cart_slot_device*)m_cart));

				// passthru for RAM-equipped carts
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x8800, 0x8fff, write16_delegate(FUNC(intv_cart_slot_device::write_88),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0xd000, 0xd7ff, write16_delegate(FUNC(intv_cart_slot_device::write_d0),(intv_cart_slot_device*)m_cart));
				break;
		}

		m_cart->save_ram();
	}
}


TIMER_CALLBACK_MEMBER(intv_state::intv_interrupt_complete)
{
	m_maincpu->set_input_line(CP1610_INT_INTRM, CLEAR_LINE);
	m_bus_copy_mode = 0;
}

TIMER_CALLBACK_MEMBER(intv_state::intv_btb_fill)
{
	uint8_t row = m_backtab_row;
	//m_maincpu->adjust_icount(-STIC_ROW_FETCH);

	for (int column = 0; column < STIC_BACKTAB_WIDTH; column++)
		m_stic->write_to_btb(row, column,  m_ram16[column + row * STIC_BACKTAB_WIDTH]);

	m_backtab_row += 1;
}

INTERRUPT_GEN_MEMBER(intv_state::intv_interrupt)
{
	int delay = m_stic->read_row_delay();
	m_maincpu->set_input_line(CP1610_INT_INTRM, ASSERT_LINE);
	m_sr1_int_pending = 1;
	m_bus_copy_mode = 1;
	m_backtab_row = 0;

	m_maincpu->adjust_icount(-(12*STIC_ROW_BUSRQ+STIC_FRAME_BUSRQ)); // Account for stic cycle stealing
	timer_set(m_maincpu->cycles_to_attotime(STIC_VBLANK_END), TIMER_INTV_INTERRUPT_COMPLETE);
	for (int row = 0; row < STIC_BACKTAB_HEIGHT; row++)
	{
		timer_set(m_maincpu->cycles_to_attotime(STIC_FIRST_FETCH-STIC_FRAME_BUSRQ+STIC_CYCLES_PER_SCANLINE*STIC_Y_SCALE*delay + (STIC_CYCLES_PER_SCANLINE*STIC_Y_SCALE*STIC_CARD_HEIGHT - STIC_ROW_BUSRQ)*row), TIMER_INTV_BTB_FILL);
	}

	if (delay == 0)
	{
		m_maincpu->adjust_icount(-STIC_ROW_BUSRQ); // extra row fetch occurs if vertical delay == 0
	}

	m_stic->screenrefresh();
}
