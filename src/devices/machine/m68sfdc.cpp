// license:BSD-3-Clause
// copyright-holders:68bit
//
// Motorola M68SFDC floppy disk controller
//
// References:
//
// "M68SFDC2(D) EXORdisk II Floppy disk controller module - Users's guide.",
// Motorola, June 1978.
//
// "AN-764: A floppy disk controller using the MC6852 SSDA and other M6800
// microprocessor family parts", Motorola 1976.

#include "emu.h"
#include "m68sfdc.h"

m68sfdc_device::m68sfdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M68SFDC, tag, owner, clock),
	m_pia(*this, "pia"),
	m_ssda(*this, "ssda"),
	m_timer_head_load(nullptr),
	m_timer_timeout(nullptr),
	m_irq_handler(*this),
	m_nmi_handler(*this),
	m_select2_mode(*this, "SELECT2_MODE"),
	m_select3_mode(*this, "SELECT3_MODE"),
	m_disk_sides(*this, "DISK_SIDES"),
	m_write_protect_mode(*this, "WRITE_PROTECT_MODE"),
	m_stepper_mode(*this, "STEPPER_MODE")
{
}

INPUT_PORTS_START(m68sfdc)

	PORT_START("SELECT2_MODE")
	PORT_CONFNAME(0x01, 0x00, "Select 2 line mode")
	PORT_CONFSETTING(0, "Not connected")
	PORT_CONFSETTING(1, "Selects drives 2 and 3")

	PORT_START("SELECT3_MODE")
	PORT_CONFNAME(0x01, 0x00, "Select 3 line mode")
	PORT_CONFSETTING(0, "Not connected")
	PORT_CONFSETTING(1, "Selects drive head")

	PORT_START("DISK_SIDES")
	PORT_CONFNAME(0x20, 0x20, "Disk sides switch")
	PORT_CONFSETTING(0x20, "Single sided")
	PORT_CONFSETTING(0x00, "Double sided")

	PORT_START("WRITE_PROTECT_MODE")
	PORT_CONFNAME(0x1, 0x1, "Write-enabled line mode")
	PORT_CONFSETTING(0x0, "Active low write protect")
	PORT_CONFSETTING(0x1, "Active low write enabled")

	PORT_START("STEPPER_MODE")
	PORT_CONFNAME(0x1, 0x0, "Stepper control lines mode")
	PORT_CONFSETTING(0x0, "Conventional")
	PORT_CONFSETTING(0x1, "'Step' line steps in, 'direction' line steps out")

INPUT_PORTS_END

ioport_constructor m68sfdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(m68sfdc);
}

void m68sfdc_device::device_resolve_objects()
{
}

void m68sfdc_device::device_start()
{
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();

	m_timer_head_load = timer_alloc(TM_HEAD_LOAD);
	m_timer_timeout = timer_alloc(TM_TIMEOUT);
	save_item(NAME(m_select_0));
	save_item(NAME(m_select_1));
	save_item(NAME(m_select_2));
	save_item(NAME(m_select_3));
	save_item(NAME(m_step));
	save_item(NAME(m_direction));
	save_item(NAME(m_head_load1));
	save_item(NAME(m_head_load2));
	save_item(NAME(m_head_load));
	save_item(NAME(m_crc));
	save_item(NAME(m_last_crc));
	save_item(NAME(m_pia_cb2));
	save_item(NAME(m_reset));
	save_item(NAME(m_enable_drive_write));
	save_item(NAME(m_enable_read));
	save_item(NAME(m_shift_crc));
	save_item(NAME(m_shift_crc_count));
	save_item(NAME(m_tuf_count));
	save_item(NAME(m_ssda_reg));

	m_floppy = nullptr;

	t_gen = timer_alloc(TM_GEN);
}

void m68sfdc_device::device_reset()
{
	m_select_0 = 0;
	m_select_1 = 0;
	m_select_2 = 0;
	m_select_3 = 0;
	m_step = 1;
	m_direction = 0;
	m_head_load1 = 0;
	m_head_load2 = 0;
	m_head_load = 0;
	m_crc = 0;
	m_last_crc = 0;
	m_pia_cb2 = 0;
	m_reset = 1;
	m_enable_drive_write = 0;
	m_enable_read = 0;
	m_shift_crc = 0;
	m_shift_crc_count = 0;
	m_tuf_count = 0;

	m_irq_handler(false);
	m_nmi_handler(false);
}

void m68sfdc_device::set_floppies_4(floppy_connector *f0, floppy_connector *f1, floppy_connector *f2, floppy_connector *f3)
{
	m_floppy0 = f0;
	m_floppy1 = f1;
	m_floppy2 = f2;
	m_floppy3 = f3;

	if (m_floppy0)
	{
		m_floppy = m_floppy0->get_device();
	}
}

WRITE_LINE_MEMBER(m68sfdc_device::handle_irq)
{
	m_irq_handler(state);
}

WRITE_LINE_MEMBER(m68sfdc_device::handle_nmi)
{
	m_nmi_handler(state);
}

void m68sfdc_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TM_HEAD_LOAD:
	{
		live_sync();
		m_head_load2 = 0;
		u8 head_load = m_head_load1 && m_head_load2;
		if (head_load != m_head_load)
		{
			// TODO sound?
			m_head_load = head_load;
		}
		break;
	}
	case TM_TIMEOUT:
	{
		live_sync();
		m_pia->ca1_w(0);
		break;
	}
	case TM_GEN:
		live_sync();
		live_run();
		break;
	default:
		throw emu_fatalerror("Unknown id in m68sfdc_device::device_timer");
	}
}



uint8_t m68sfdc_device::flip_bits(uint8_t data)
{
	data = (data & 0b11110000) >> 4 | (data & 0b00001111) << 4;
	data = (data & 0b11001100) >> 2 | (data & 0b00110011) << 2;
	data = (data & 0b10101010) >> 1 | (data & 0b01010101) << 1;
	return data;
}


u8 m68sfdc_device::read(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		live_sync();
		// Triggers the 0.8 second head-load timer.
		m_timer_head_load->reset(attotime::from_msec(800));
	}

	if (offset > 3)
	{
		u8 data = m_ssda->read(offset - 4);
		// The data bits are connected in reverse.
		data = (data & 0b11110000) >> 4 | (data & 0b00001111) << 4;
		data = (data & 0b11001100) >> 2 | (data & 0b00110011) << 2;
		data = (data & 0b10101010) >> 1 | (data & 0b01010101) << 1;
		return data;

	}

	// The 6821 address lines are swapped.
	offset = ((offset & 1) << 1) | (offset >> 1);
	return m_pia->read(offset);
}

#define C1_RX_RS 0x01
#define C1_AC_MASK 0xc0
#define C1_AC_C2 0x00
#define C2_PC_MASK 0x03
#define C2_PC1 0x01


void m68sfdc_device::write(offs_t offset, u8 data)
{
	live_sync();

	// Triggers the 0.8 second head-load timer.
	m_head_load2 = 1;
	m_timer_head_load->reset(attotime::from_msec(800));

	if (offset > 3)
	{
		// Address line A1 is not decoded for the SSDA
		offset = (offset - 4) & 0x0001;

		// The data bits are connected in reverse.
		data = (data & 0b11110000) >> 4 | (data & 0b00001111) << 4;
		data = (data & 0b11001100) >> 2 | (data & 0b00110011) << 2;
		data = (data & 0b10101010) >> 1 | (data & 0b01010101) << 1;
		m_ssda->write(offset, data);

		// Maintain shadow copies of the 6852 register writes.
		if (offset == 0)
			m_ssda_reg[0] = data;
		else
			m_ssda_reg[(m_ssda_reg[0] >> 6) + 1] = data;

		if (offset == 1 && (m_ssda_reg[0] & C1_AC_MASK) == C1_AC_C2 &&
			(data & C2_PC_MASK) == C2_PC1 && m_enable_read)
		{
			// This a write to the 6852 CR2 register which enables
			// the SM output (PC2 = 0, PC1 = 1), while the read
			// logic is enabled. At this point all is setup to
			// search for a sync code.
			if (m_reset == 0 && m_enable_read)
			{
				live_start(SYNC1);
			}
		}

		if (offset == 0 && m_enable_read && (data & C1_RX_RS) != 0)
		{
			live_abort();
		}

		return;
	}

	// The 6821 address lines are swapped.
	offset = ((offset & 1) << 1) | (offset >> 1);
	m_pia->write(offset, data);
}


uint8_t m68sfdc_device::pia_pa_r()
{
	int ready = 1;
	int track0 = 1;
	if (m_floppy)
	{
		ready = m_floppy->ready_r();
		track0 = m_floppy->trk00_r();
	}

	// While this is not connected in the schematic, the MDOS 3 format
	// command probes this input to determine if a disk is to be formatted
	// singled sided (1) or double sided (0), and it is assumed to be a
	// later revision.
	int sides = m_disk_sides->read();

	return (track0 ? 0 : 0x80) | (ready << 6) | sides;
}

void m68sfdc_device::update_floppy_selection()
{
	floppy_image_device *floppy = nullptr;
	u8 select2_mode = m_select2_mode->read();

	if (select2_mode == 0 || m_select_2 == 0)
	{
		if (!m_select_1 && m_select_0)
			floppy = m_floppy0->get_device();
		else if (m_select_1 && !m_select_0)
			floppy = m_floppy1->get_device();
	}
	else
	{
		if (!m_select_1 && m_select_0)
			floppy = m_floppy2->get_device();
		else if (m_select_1 && !m_select_0)
			floppy = m_floppy3->get_device();
	}

	if (floppy != m_floppy)
	{
		if (m_floppy)
		{
			m_floppy->mon_w(1); // Active low
			m_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
		}
		m_floppy = floppy;
		if (m_floppy)
		{
			// Assume the motors are always on?
			m_floppy->mon_w(0); // Active low
			if (m_stepper_mode->read())
			{
				m_floppy->dir_w(0);
				m_floppy->stp_w(0);
			}
			else
			{
				m_floppy->dir_w(m_direction);
				m_floppy->stp_w(m_step);
			}
			m_floppy->ss_w(m_select3_mode->read() ? m_select_3 : 0);
			m_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&m68sfdc_device::fdc_index_callback, this));
		}
	}
}

void m68sfdc_device::pia_pa_w(u8 data)
{
	// Select 0 and select 1 are used for drive selection. When 0x02 these
	// select drive 0 or 2, and when 0x01 select drive 1 or 3. These are
	// used in conjuction with select 2 to decode four drives.
	m_select_0 = !BIT(data, 0);
	m_select_1 = !BIT(data, 1);
	// u8 m_gt_trk43 = !BIT(data, 2);
	u8 direction = !BIT(data, 3);
	m_head_load1 = !BIT(data, 4);

	if (m_floppy)
	{
		if (m_stepper_mode->read())
		{
			if (m_direction == 0 && direction == 1)
			{
				m_floppy->dir_w(0);
				m_floppy->stp_w(1);
				m_floppy->stp_w(0);
			}
		}
		else
		{
			m_floppy->dir_w(m_direction);
			m_floppy->stp_w(m_step);
		}

		m_floppy->ss_w(m_select3_mode->read() ? m_select_3 : 0);
	}
	m_direction = direction;

	update_floppy_selection();

	u8 head_load = m_head_load1 && m_head_load2;
	if (head_load != m_head_load)
	{
		// TODO sound?
		m_head_load = head_load;
	}
}

void m68sfdc_device::pia_ca2_w(int state)
{
	if (m_floppy)
	{
		if (m_stepper_mode->read())
		{
			if (m_step == 1 && state == 0)
			{
				m_floppy->dir_w(1);
				m_floppy->stp_w(1);
				m_floppy->stp_w(0);
			}
		}
		else
		{
			m_floppy->dir_w(m_direction);
			m_floppy->stp_w(state);
		}
	}
	m_step = state;
}

uint8_t m68sfdc_device::pia_pb_r()
{
	int wpt = m_floppy ? m_floppy->wpt_r() : 1;

	if (m_write_protect_mode->read())
		wpt = !wpt;

	return (wpt << 4) | (m_crc << 7);
}

void m68sfdc_device::pia_pb_w(u8 data)
{
	u8 reset = BIT(data, 0);
	u8 enable_drive_write = !BIT(data, 1);
	m_enable_read = BIT(data, 2);
	u8 shift_crc = BIT(data, 3);
	// Select 2 is used for drive selection in MDOS, expanding the
	// capability from 2 to 4 drives. A port value of 1 selects drives 0
	// and 1, and a port value of 0 selects drives 2 and 3.
	m_select_2 = !BIT(data, 5);
	// Select 3 is used for head selection in MDOS 3. A port value of 1
	// selects head 0, and a port value of 0 selects head 1.
	m_select_3 = !BIT(data, 6);

	int reset_edge = m_reset == 0 && reset == 1;
	int disable_write_edge = m_enable_drive_write == 1 && enable_drive_write == 0;
	int enable_write_edge = m_enable_drive_write == 0 && enable_drive_write == 1;
	int shift_crc_edge = m_shift_crc == 0 && shift_crc == 1;

	m_reset = reset;
	m_enable_drive_write = enable_drive_write;
	m_shift_crc = shift_crc;

	if (m_floppy)
		m_floppy->ss_w(m_select3_mode->read() ? m_select_3 : 0);

	update_floppy_selection();

	if (shift_crc_edge)
		m_shift_crc_count = 2;

	if (reset_edge)
		m_shift_crc_count = 0;

	// When reset goes high the read circuit switches to using a 500kHz
	// clock to search for the sync byte. It also resets the CRC
	// calculation. A reset may occur during a write, in a format
	// operation, so don't idle if still writing.
	if ((reset_edge && !enable_drive_write) || disable_write_edge)
	{
		// End of read or write operations.
		// typically m_enable_read will be low here too.
		live_abort();
	}

	if (enable_write_edge && m_floppy && !(m_select_0 && m_select_1))
	{
		// Start of write operations, even if the logic is in reset.
		m_tuf_count = 0;
		live_start(WRITE);
	}
}

int m68sfdc_device::pia_cb1_r()
{
	// Index pulse, active high at CB1.
	if (m_floppy)
	{
		int index = m_floppy->idx_r() ? 0 : 1;
		return index;
	}
	return 0;
}

void m68sfdc_device::pia_cb2_w(int state)
{
	if (m_pia_cb2 == 1 && state == 0)
	{
		// Trigger the timeout timer on a high to low transition of CB2
		m_pia->ca1_w(1);
		m_timer_timeout->reset(attotime::from_msec(800));
	}
	m_pia_cb2 = state;
}

void m68sfdc_device::fdc_index_callback(floppy_image_device *floppy, int state)
{
	live_sync();
	m_pia->cb1_w(state ? 0 : 1);
	live_run();
}


void m68sfdc_device::live_start(int state)
{
	cur_live.tm = machine().time();
	cur_live.state = state;
	cur_live.next_state = -1;
	cur_live.shift_reg = 0;
	cur_live.crc = 0xffff;
	cur_live.bit_counter = 0;
	cur_live.data_separator_phase = false;
	cur_live.data_reg = 0;

	pll_reset(cur_live.tm);
	checkpoint_live = cur_live;
	pll_save_checkpoint();

	live_run();
}

void m68sfdc_device::checkpoint()
{
	pll_commit(m_floppy, cur_live.tm);
	checkpoint_live = cur_live;
	pll_save_checkpoint();
}

void m68sfdc_device::rollback()
{
	cur_live = checkpoint_live;
	pll_retrieve_checkpoint();
}


void m68sfdc_device::pll_reset(const attotime &when)
{
	cur_pll.reset(when);
	// 500kHz
	cur_pll.set_clock(attotime::from_nsec(2000));
}

void m68sfdc_device::live_delay(int state)
{
	cur_live.next_state = state;
	t_gen->adjust(cur_live.tm - machine().time());
}

void m68sfdc_device::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
			pll_commit(m_floppy, cur_live.tm);
		} else {
			pll_commit(m_floppy, cur_live.tm);
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				pll_stop_writing(m_floppy, cur_live.tm);
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void m68sfdc_device::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	pll_stop_writing(m_floppy, cur_live.tm);
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}

bool m68sfdc_device::read_one_bit(const attotime &limit)
{
	int bit = pll_get_next_bit(cur_live.tm, m_floppy, limit);
	if(bit < 0)
		return true;
	cur_live.shift_reg = (cur_live.shift_reg << 1) | bit;
	cur_live.bit_counter++;
	if(cur_live.data_separator_phase) {
		cur_live.data_reg = (cur_live.data_reg << 1) | bit;
		if((cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			cur_live.crc = (cur_live.crc << 1) ^ 0x1021;
		else
			cur_live.crc = cur_live.crc << 1;
	}
	cur_live.data_separator_phase = !cur_live.data_separator_phase;
	return false;
}

bool m68sfdc_device::write_one_bit(const attotime &limit)
{
	bool bit = cur_live.shift_reg & 0x8000;
	if(pll_write_next_bit(bit, cur_live.tm, m_floppy, limit))
		return true;
	if(cur_live.bit_counter & 1) {
		if((cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			cur_live.crc = (cur_live.crc << 1) ^ 0x1021;
		else
			cur_live.crc = cur_live.crc << 1;
	}
	cur_live.shift_reg = cur_live.shift_reg << 1;
	cur_live.bit_counter--;
	return false;
}

void m68sfdc_device::live_write_fm(uint8_t fm)
{
	uint16_t raw = 0xaaaa;
	for(int i=0; i<8; i++)
		if(fm & (0x80 >> i))
			raw |= 0x4000 >> (2*i);
	cur_live.data_reg = fm;
	cur_live.shift_reg = raw;
}

void m68sfdc_device::live_run(attotime limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	if(limit == attotime::never) {
		if(m_floppy)
			limit = m_floppy->time_next_index();
		if(limit == attotime::never) {
			// Happens when there's no disk or if the wd is not
			// connected to a drive, hence no index pulse. Force a
			// sync from time to time in that case, so that the main
			// cpu timeout isn't too painful.  Avoids looping into
			// infinity looking for data too.

			limit = machine().time() + attotime::from_msec(1);
			t_gen->adjust(attotime::from_msec(1));
		}
	}

	for(;;) {
		switch(cur_live.state) {
		case SYNC1: {
			if(read_one_bit(limit))
				return;

			// The SSDA performs the sync code search, and the code
			// will have been loaded into the SSDA sync code
			// register. This is emulated here, and the code loaded
			// from a copy of SSDA register writes.
			int sync = flip_bits(m_ssda_reg[3]);

			// The SSDA searches for only the 8-bit 0xf5 code, and
			// the CPU loads and checks the subsequent code. The
			// 0xaa prefix check is an emulator hack for now to
			// improve detection reliability.
			if ((cur_live.shift_reg & 0xff) == sync &&
				(cur_live.shift_reg >> 8) == 0xaa)
			{
				// Initialize the CRC. The hardware has an 8
				// bit shift register to delay the bit stream
				// so that it can reset the CRC on this sync
				// event and then feed it the delayed sync
				// code.
				cur_live.crc = 0xffff;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				for (int i = 6; i >= 0; i-=2)
				{
					int bit = BIT(cur_live.shift_reg, i);
					if((cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
						cur_live.crc = (cur_live.crc << 1) ^ 0x1021;
					else
						cur_live.crc = cur_live.crc << 1;
				}
				live_delay(SYNC_BYTE1);
				return;
			}

			break;
		}
		case SYNC_BYTE1:
			m_ssda->receive_byte(flip_bits(cur_live.shift_reg & 0xff));
			cur_live.state = SYNC2;
			checkpoint();
			break;

		case SYNC2: {
			if(read_one_bit(limit))
				return;

			if(cur_live.bit_counter == 8)
			{
				live_delay(SYNC_BYTE2);
				return;
			}

			break;
		}
		case SYNC_BYTE2:
			m_ssda->receive_byte(flip_bits(cur_live.shift_reg & 0xff));
			cur_live.bit_counter = 0;
			cur_live.state = READ;
			checkpoint();
			break;

		case READ: {
			if(read_one_bit(limit))
				return;

			if(cur_live.bit_counter & 15)
				break;

			live_delay(READ_BYTE);
			return;
		}

		case READ_BYTE:
			m_ssda->receive_byte(flip_bits(cur_live.data_reg));
			cur_live.state = READ;

			// The data to the CRC generator is delayed 8 bits behind
			// the SSDA data input delaying the CRC line.
			m_crc = m_last_crc;
			m_last_crc = cur_live.crc != 0;

			// Unfortunately the emulated system can at times read
			// the CRC line early, the timing needs work, so as a
			// workaround for now the CRC line is asserted early at
			// expected CRC end positions: address marks, and 128
			// and 256 byte data sectors.
			if (cur_live.bit_counter == (4 + 2) * 16 ||
				cur_live.bit_counter == (128 + 2) * 16 ||
				cur_live.bit_counter == (256 + 2) * 16)
			{
				m_crc = m_last_crc;
			}

			checkpoint();
			break;

		case WRITE:
		{
			int tuf;
			u8 data = flip_bits(m_ssda->get_tx_byte(&tuf));

			if (tuf)
			{
				m_tuf_count = 3;
			}
			else if (m_tuf_count > 0)
			{
				if (m_tuf_count == 2)
				{
					// Start of the sync code,
					// initialize the CRC.
					cur_live.crc = 0xffff;
				}

			}

			if (m_tuf_count > 0)
			{
				// Data clocked at 500kHz
				cur_live.shift_reg = data << 8;
				cur_live.bit_counter = 8;
				m_tuf_count--;
			}
			else
			{
				// Data clocked at 250kHz

				// If the 'shift crc' line has been asserted
				// then write the CRC code rather than the SSDA
				// data, and for two bytes.
				if (m_shift_crc_count > 0)
				{
					// Two CRC bytes
					data = cur_live.crc >> 8;
					m_shift_crc_count--;
				}

				live_write_fm(data);
				cur_live.bit_counter = 16;
			}

			cur_live.state = WRITE_BITS;
			checkpoint();
			break;
		}

		case WRITE_BITS:
			if(write_one_bit(limit))
				return;
			if(cur_live.bit_counter == 0) {
				live_delay(WRITE);
				return;
			}
			break;

		default:
			logerror("%s: Unknown live state %d\n", cur_live.tm.to_string(), cur_live.state);
			return;
		}
	}
}

void m68sfdc_device::pll_commit(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.commit(floppy, tm);
}

void m68sfdc_device::pll_stop_writing(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.stop_writing(floppy, tm);
}

void m68sfdc_device::pll_save_checkpoint()
{
	checkpoint_pll = cur_pll;
}

void m68sfdc_device::pll_retrieve_checkpoint()
{
	cur_pll = checkpoint_pll;
}

int m68sfdc_device::pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.get_next_bit(tm, m_floppy, limit);
}

bool m68sfdc_device::pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.write_next_bit(bit, tm, m_floppy, limit);
}

void m68sfdc_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia, 0);
	m_pia->readpa_handler().set(FUNC(m68sfdc_device::pia_pa_r));
	m_pia->writepa_handler().set(FUNC(m68sfdc_device::pia_pa_w));
	m_pia->ca1_w(0);
	m_pia->ca2_handler().set(FUNC(m68sfdc_device::pia_ca2_w));
	m_pia->readpb_handler().set(FUNC(m68sfdc_device::pia_pb_r));
	m_pia->writepb_handler().set(FUNC(m68sfdc_device::pia_pb_w));
	m_pia->readcb1_handler().set(FUNC(m68sfdc_device::pia_cb1_r));
	m_pia->cb2_handler().set(FUNC(m68sfdc_device::pia_cb2_w));
	m_pia->irqa_handler().set(FUNC(m68sfdc_device::handle_nmi));
	m_pia->irqb_handler().set(FUNC(m68sfdc_device::handle_irq));

	MC6852(config, m_ssda, 0);
}

DEFINE_DEVICE_TYPE(M68SFDC, m68sfdc_device, "m68sfdc", "M68SFDC")
