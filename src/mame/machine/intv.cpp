// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
#include "emu.h"
#include "video/stic.h"
#include "video/tms9927.h"
#include "includes/intv.h"
#include "cpu/cp1610/cp1610.h"

// Dual Port Memory handlers

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
	return (m_intvkbd_dualport_ram[offset+0x200]&0x0300)>>8;
}

WRITE8_MEMBER( intv_state::intvkbd_dualport8_msb_w )
{
	unsigned int mask = m_intvkbd_dualport_ram[offset+0x200] & 0x00ff;
	m_intvkbd_dualport_ram[offset+0x200] = mask | ((data<<8)&0x0300);
}

// I/O for the Tape Drive
// (to be moved to a device)
struct tape_drive_state_type
{
	/* read state */
	int read_data;          /* 0x4000 */
	int ready;              /* 0x4001 */
	int leader_detect;      /* 0x4002 */
	int tape_missing;       /* 0x4003 */
	int playing;            /* 0x4004 */
	int no_data;            /* 0x4005 */

	/* write state */
	int motor_state;        /* 0x4020-0x4022 */
	int writing;            /* 0x4023 */
	int audio_b_mute;       /* 0x4024 */
	int audio_a_mute;       /* 0x4025 */
	int channel_select;     /* 0x4026 */
	int erase;              /* 0x4027 */
	int write_data;         /* 0x4040 */

	/* bit_counter */
	int bit_counter;
} tape_drive;

//static const char *const tape_motor_mode_desc[8] =
//{
//  "IDLE", "IDLE", "IDLE", "IDLE",
//  "EJECT", "PLAY/RECORD", "REWIND", "FF"
//};


READ8_MEMBER( intv_state::intvkbd_io_r )
{
	unsigned char rv = 0x00;

	switch (offset)
	{
		// These next 8 locations all map to bit7
		case 0x000:
			// "Data from Cassette"
			// Tape drive does the decoding to bits
			//rv = m_io_test->read() & 0x80;
			rv = tape_drive.read_data << 7;
			break;
		case 0x001:
			// "Watermark"
			// 0 = Drive Busy Executing Command?, 1 = Drive Ok?
			//rv = (m_io_test->read() & 0x40) << 1;
			rv = tape_drive.ready << 7;
			break;
		case 0x002:
			// "End of Tape"
			// 0 = Recordable surface, 1 = Leader Detect
			// (Leader is transparent, optical sensor)
			//rv = (m_io_test->read() & 0x20) << 2;
			rv = tape_drive.leader_detect << 7;
			//logerror("TAPE: Read %02x from 0x40%02x - Sense 2?\n",rv,offset);
			break;
		case 0x003:
			// "Cassette Present"
			// 0 = Tape Present, 1 = Tape Not Present
			//rv = (m_io_test->read() & 0x10) << 3;
			rv = tape_drive.tape_missing << 7;
			//logerror("TAPE: Read %02x from 0x40%02x - Tape Present\n",rv,offset);
			break;
		case 0x004:
			// "NOT Inter Record Gap (IRG)"
			// 0 = Not Playing/Recording?, 1 = Playing/Recording?
			//rv = (m_io_test->read() & 0x08) << 4;
			rv = tape_drive.playing << 7;
			//logerror("TAPE: Read %02x from 0x40%02x - Comp (339/1)\n",rv,offset);
			break;
		case 0x005:
			// "Dropout"
			// 0 = Data Detect, 1 = No Data
			//rv = (m_io_test->read() & 0x04) << 5;
			rv = tape_drive.no_data << 7;
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
		default:
			//logerror("Unknown read %02x from 0x40%02x\n",rv,offset);
			break;
	}
	return rv;
}

WRITE8_MEMBER( intv_state::intvkbd_io_w )
{
	switch (offset)
	{
		// Bits from offset $20 to $47 are all bit0, write only
		// These are all set to zero by system reset
		case 0x020:
			// "Tape Drive Control: Enable"
			tape_drive.motor_state &= 3;
			if (data & 1)
				tape_drive.motor_state |= 4;
			//logerror("TAPE: Motor Mode: %s\n",tape_motor_mode_desc[m_tape_motor_mode]);
			break;
		case 0x021:
			// "Tape Drive Control: Forward"
			tape_drive.motor_state &= 5;
			if (data & 1)
				tape_drive.motor_state |= 2;
			//logerror("TAPE: Motor Mode: %s\n",tape_motor_mode_desc[m_tape_motor_mode]);
			break;
		case 0x022:
			// "Tape Drive Control: Fast"
			tape_drive.motor_state &= 6;
			if (data & 1)
				tape_drive.motor_state |= 1;
			//logerror("TAPE: Motor Mode: %s\n",tape_motor_mode_desc[m_tape_motor_mode]);
			break;
		case 0x023:
			// "Tape Drive Control: Record"
			// 0=Read, 1=Write
			tape_drive.writing = (data & 1);
			break;
		case 0x024:
			// "Tape Drive Control: Mute 1"
			// 0=Enable Channel B Audio, 1=Mute
			tape_drive.audio_b_mute = (data & 1);
			break;
		case 0x025:
			// "Tape Drive Control: Mute 2"
			// 0=Enable Channel A Audio, 1=Mute
			tape_drive.audio_a_mute = (data & 1);
			break;
		case 0x026:
			// "Tape Drive Control: Mode"
			// If read mode:
			//  0=Read Channel B Data, 1 = Read Channel A Data
			// If write mode:
			//  0=Write Channel B data, 1 = Record Channel B Audio
			tape_drive.channel_select = (data & 1);
			break;
		case 0x027:
			// "Tape Drive Control: Erase"
			tape_drive.erase = (data & 1);
			break;
		case 0x040:
			// Data to Tape
			tape_drive.write_data = (data & 1);
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
		default:
			//logerror("%04X: Unknown write %02x to 0x40%02x\n",m_keyboard->pc(),data,offset);
			break;
	}
}

#if 0
static int max_bits = 0;
static unsigned char *tape_data;

void get_tape_bit(int position, int channel, int *data_present, int *data)
{
	int byte = (position >> 2)*2 + channel;
	int data_present_mask = 1 << ((3-(position % 4))*2 + 1);
	int data_mask = 1 << ((3-(position % 4))*2);

	//printf("%d\t0x%02x 0x%02x\n",byte,data_present_mask,data_mask);

	if (tape_data[byte] & data_present_mask)
		*data_present = 1;
	else
		*data_present = 0;

	if (tape_data[byte] & data_mask)
		*data = 1;
	else
		*data = 0;
}

void set_tape_bit(int position, int data)
{
	int byte = (position >> 2)*2 + 1;
	int data_present_mask = 1 << ((3-(position % 4))*2 + 1);
	int data_mask = 1 << ((3-(position % 4))*2);

	tape_data[byte] |= data_present_mask;
	if (data)
		tape_data[byte] |= data_mask;
	else
		tape_data[byte] &= (~data_mask);
}
#endif

#if defined(LATER)
int intvkbd_tape_init(int id)
{
	FILE *tapefile;
	int filesize;

	if (!(tapefile = image_fopen (IO_CASSETTE, id, OSD_FILETYPE_IMAGE, OSD_FOPEN_READ)))
	{
		return INIT_FAIL;
	}

	filesize = osd_fsize(tapefile);
	tape_data = (unsigned char *)malloc(filesize);
	osd_fread(tapefile, tape_data, filesize);

	osd_fclose(tapefile);

	max_bits = 2*filesize;

	tape_drive.tape_missing = 0;
	tape_drive.leader_detect = 0;
	tape_drive.ready = 1;

	tape_drive.bit_counter = 0;
	return INIT_PASS;
}

void intvkbd_tape_exit(int id)
{
	FILE *tapefile;
	int filesize;

	if (tape_data)
	{
		if (!(tapefile = image_fopen (IO_CASSETTE, id, OSD_FILETYPE_IMAGE, OSD_FOPEN_RW)))
		{
			filesize = osd_fsize(tapefile);
			osd_fwrite(tapefile, tape_data, filesize);
			osd_fclose(tapefile);

			free(tape_data);
			tape_data = 0;

			max_bits = 0;
			tape_drive.tape_missing = 1;
			tape_drive.bit_counter = 0;
		}
	}
}

void update_tape_drive(void)
{
	/* temp */

	if (tape_drive.writing)
	{
		if (tape_drive.channel_select == 0) /* data */
		{
			set_tape_bit(tape_drive.bit_counter,tape_drive.write_data);
		}
		else
		{
			/* recording audio - TBD */
		}
	}
	else
	{
		int channel;
		int data_present;
		int data;

		channel = tape_drive.channel_select ^ 1;

		get_tape_bit(tape_drive.bit_counter,channel,&data_present,&data);

		tape_drive.no_data = data_present ^ 1;
		tape_drive.read_data = data;

		/* temporary */
		tape_drive.playing = data_present ^ 1;
	}

	if (tape_drive.motor_state == 5) /* Playing */
	{
		tape_drive.bit_counter++;
		if (tape_drive.bit_counter >= max_bits)
			tape_drive.bit_counter = max_bits-1;
	}

	if (tape_drive.motor_state == 6) /* Rewinding */
	{
		tape_drive.bit_counter-=4;
		//tape_drive.bit_counter--;
		if (tape_drive.bit_counter < 0)
			tape_drive.bit_counter = 0;
	}
	if (tape_drive.motor_state == 7) /* FastFwd */
	{
		tape_drive.bit_counter+=2;
		//tape_drive.bit_counter++;
		if (tape_drive.bit_counter >= max_bits)
			tape_drive.bit_counter = max_bits-1;
	}

	if ((tape_drive.bit_counter == 0) || (tape_drive.bit_counter == max_bits-1))
		tape_drive.leader_detect = 1;
	else
		tape_drive.leader_detect = 0;
}
#endif

////////////

READ8_MEMBER( intv_state::intvkbd_periph_r )
{
	uint8_t value = 0;
	switch(offset) {
		case 0x06:
			if (m_printer_not_busy_enable)
				if (m_printer_not_busy)
					value |= 0x80;
			if (m_printer_no_paper)
				value |= 0x10;
			//logerror("PeriphRead:  0x%04x->0x%02x\n",offset,value);

			// After one query of busy,
			// next time the state is not_busy
			if (!m_printer_not_busy)
				m_printer_not_busy = true;

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
	switch(offset) {
		case 0x06:
			//logerror("PeriphWrite: 0x%04x->0x%02x\n",offset,data);
			if (data & 0x20)
				m_printer_not_busy_enable = true;
			else
				m_printer_not_busy_enable = false;
		break;
		case 0x07:
			//logerror("Printing: 0x%02x, %c\n",data,data);
			// For testing, print to stdout
			fputc(data, stdout);
			fflush(stdout);
			m_printer_not_busy = false;
		break;
		default:
			//logerror("PeriphWrite: 0x%04x->0x%02x\n",offset,data);
		break;
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
		return m_iocart1->read_rom(offset);
	else if (m_iocart2->exists())
		return m_iocart2->read_rom(offset);
	else
		return m_region_keyboard->as_u8(offset + 0xe000);
}


/* Set Reset and INTR/INTRM Vector */
void intv_state::machine_reset()
{
	m_maincpu->set_input_line_vector(CP1610_RESET, 0x1000); // CP1610

	/* These are actually the same vector, and INTR is unused */
	m_maincpu->set_input_line_vector(CP1610_INT_INTRM, 0x1004); // CP1610
	m_maincpu->set_input_line_vector(CP1610_INT_INTR,  0x1004); // CP1610

	/* Set initial PC */
	m_maincpu->set_state_int(cp1610_cpu_device::CP1610_R7, 0x1000);

	if (m_is_keybd)
	{
		m_printer_not_busy = true;          // printer state
		m_printer_no_paper = false;         // printer state
		m_printer_not_busy_enable = false;  // printer interface state
	}
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
			sprintf(str, "ROW%X", uint8_t(i));
			m_intv_keyboard[i] = ioport(str);
		}

		save_item(NAME(m_intvkbd_text_blanked));
		save_item(NAME(m_intvkbd_keyboard_col));
		save_item(NAME(m_tape_int_pending));
		save_item(NAME(m_tape_interrupts_enabled));
		save_item(NAME(m_tape_motor_mode));
	}

	if (m_cart && m_cart->exists())
	{
		// RAM
		switch (m_cart->get_type())
		{
			case INTV_RAM:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xd000, 0xd7ff, read16sm_delegate(FUNC(intv_cart_slot_device::read_ram),(intv_cart_slot_device*)m_cart), write16sm_delegate(FUNC(intv_cart_slot_device::write_ram),(intv_cart_slot_device*)m_cart));
				break;
			case INTV_GFACT:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x8800, 0x8fff, read16sm_delegate(FUNC(intv_cart_slot_device::read_ram),(intv_cart_slot_device*)m_cart), write16sm_delegate(FUNC(intv_cart_slot_device::write_ram),(intv_cart_slot_device*)m_cart));
				break;
			case INTV_VOICE:
				m_cart->late_subslot_setup();
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0080, 0x0081, read16sm_delegate(FUNC(intv_cart_slot_device::read_speech),(intv_cart_slot_device*)m_cart), write16sm_delegate(FUNC(intv_cart_slot_device::write_speech),(intv_cart_slot_device*)m_cart));

				// passthru for RAM-equipped carts
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x8800, 0x8fff, write16sm_delegate(FUNC(intv_cart_slot_device::write_88),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0xd000, 0xd7ff, write16sm_delegate(FUNC(intv_cart_slot_device::write_d0),(intv_cart_slot_device*)m_cart));
				break;
			case INTV_ECS:
				m_cart->late_subslot_setup();
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x00f0, 0x00ff, read16sm_delegate(FUNC(intv_cart_slot_device::read_ay),(intv_cart_slot_device*)m_cart), write16sm_delegate(FUNC(intv_cart_slot_device::write_ay),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x4000, 0x47ff, read16sm_delegate(FUNC(intv_cart_slot_device::read_ram),(intv_cart_slot_device*)m_cart), write16sm_delegate(FUNC(intv_cart_slot_device::write_ram),(intv_cart_slot_device*)m_cart));

				m_maincpu->space(AS_PROGRAM).install_write_handler(0x2000, 0x2fff, write16sm_delegate(FUNC(intv_cart_slot_device::write_rom20),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x7000, 0x7fff, write16sm_delegate(FUNC(intv_cart_slot_device::write_rom70),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0xe000, 0xefff, write16sm_delegate(FUNC(intv_cart_slot_device::write_rome0),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0xf000, 0xffff, write16sm_delegate(FUNC(intv_cart_slot_device::write_romf0),(intv_cart_slot_device*)m_cart));

				// passthru for Intellivoice expansion
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0080, 0x0081, read16sm_delegate(FUNC(intv_cart_slot_device::read_speech),(intv_cart_slot_device*)m_cart), write16sm_delegate(FUNC(intv_cart_slot_device::write_speech),(intv_cart_slot_device*)m_cart));

				// passthru for RAM-equipped carts
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x8800, 0x8fff, write16sm_delegate(FUNC(intv_cart_slot_device::write_88),(intv_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0xd000, 0xd7ff, write16sm_delegate(FUNC(intv_cart_slot_device::write_d0),(intv_cart_slot_device*)m_cart));
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

	for (int column = 0; column < stic_device::BACKTAB_WIDTH; column++)
		m_stic->write_to_btb(row, column,  m_ram16[column + row * stic_device::BACKTAB_WIDTH]);

	m_backtab_row += 1;
}

INTERRUPT_GEN_MEMBER(intv_state::intv_interrupt)
{
	int delay = m_stic->read_row_delay();
	m_maincpu->set_input_line(CP1610_INT_INTRM, ASSERT_LINE);
	m_sr1_int_pending = 1;
	m_bus_copy_mode = 1;
	m_backtab_row = 0;

	m_maincpu->adjust_icount(-(12*stic_device::ROW_BUSRQ+stic_device::FRAME_BUSRQ)); // Account for stic cycle stealing
	timer_set(m_maincpu->cycles_to_attotime(stic_device::VBLANK_END), TIMER_INTV_INTERRUPT_COMPLETE);
	for (int row = 0; row < stic_device::BACKTAB_HEIGHT; row++)
	{
		timer_set(m_maincpu->cycles_to_attotime(stic_device::FIRST_FETCH-stic_device::FRAME_BUSRQ+stic_device::CYCLES_PER_SCANLINE*stic_device::Y_SCALE*delay + (stic_device::CYCLES_PER_SCANLINE*stic_device::Y_SCALE*stic_device::CARD_HEIGHT - stic_device::ROW_BUSRQ)*row), TIMER_INTV_BTB_FILL);
	}

	if (delay == 0)
	{
		m_maincpu->adjust_icount(-stic_device::ROW_BUSRQ); // extra row fetch occurs if vertical delay == 0
	}

	m_stic->screenrefresh();
}

#if defined(LATER)

INTERRUPT_GEN( intvkbd_interrupt2 )
{
	static int tape_interrupt_divider = 0;

	tape_interrupt_divider++;
	tape_interrupt_divider = tape_interrupt_divider % 50;

	if (tape_interrupt_divider == 0)
	{
#if 0
			update_tape_drive();

		/* do sr1 interrupt plus possible tape interrupt */
		if (tape_interrupts_enabled)
			tape_int_pending = 1;
#endif
		sr1_int_pending = 1;
		cpu_set_irq_line(1, 0, PULSE_LINE);
	}
	else
	{
			update_tape_drive();

		/* do only possible tape interrupt */
		if (tape_interrupts_enabled)
		{
			tape_int_pending = 1;
			cpu_set_irq_line(1, 0, PULSE_LINE);
		}
	}

}
#endif
