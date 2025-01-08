// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Custom Thomson 3.5"/5.25"/QDD diskette controller gate array used in CD 90-351, TO8 and TO9+


#include "emu.h"
#include "thmfc1.h"

DEFINE_DEVICE_TYPE(THMFC1, thmfc1_device, "thmfc1", "SGS-Thomson THM-FC-1 Diskette Controller") // SGS logo used on silkscreen

thmfc1_device::thmfc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, THMFC1, tag, owner, clock),
	m_floppy(*this, "%u", 0U)
{
}

void thmfc1_device::map(address_map &map)
{
	map(0, 0).rw(FUNC(thmfc1_device::stat0_r), FUNC(thmfc1_device::cmd0_w));
	map(1, 1).rw(FUNC(thmfc1_device::stat1_r), FUNC(thmfc1_device::cmd1_w));
	map(2, 2).w(FUNC(thmfc1_device::cmd2_w));
	map(3, 3).rw(FUNC(thmfc1_device::rdata_r), FUNC(thmfc1_device::wdata_w));
	map(4, 4).w(FUNC(thmfc1_device::wclk_w));
	map(5, 5).w(FUNC(thmfc1_device::wsect_w));
	map(6, 6).w(FUNC(thmfc1_device::wtrck_w));
	map(7, 7).w(FUNC(thmfc1_device::wcell_w));
}

void thmfc1_device::device_start()
{
	m_timer_motoroff = timer_alloc(FUNC(thmfc1_device::motor_off), this);

	save_item(NAME(m_cmd0));
	save_item(NAME(m_cmd1));
	save_item(NAME(m_cmd2));
	save_item(NAME(m_stat0));
	save_item(NAME(m_data));
	save_item(NAME(m_clk));
	save_item(NAME(m_sect));
	save_item(NAME(m_trck));
	save_item(NAME(m_cell));
	save_item(NAME(m_last_sync));
	save_item(NAME(m_window_start));
	save_item(NAME(m_shift_reg));
	save_item(NAME(m_crc));
	save_item(NAME(m_bit_counter));
	save_item(NAME(m_data_reg));
	save_item(NAME(m_data_separator_phase));
}

void thmfc1_device::device_reset()
{
	m_cmd0 = 0;
	m_cmd1 = 0;
	m_cmd2 = 0;
	m_stat0 = S0_FREE;
	m_data = 0;
	m_clk = 0;
	m_sect = 0;
	m_trck = 0;
	m_cell = 0;
	m_last_sync = 0;
	m_window_start = 0;
	m_shift_reg = 0;
	m_crc = 0;
	m_bit_counter = 0;
	m_data_reg = 0;
	m_data_separator_phase = false;
	m_state = S_IDLE;
	m_cur_floppy = nullptr;
}

TIMER_CALLBACK_MEMBER(thmfc1_device::motor_off)
{
	logerror("motor off\n");
	if(m_cur_floppy)
		m_cur_floppy->mon_w(1);
}

void thmfc1_device::device_post_load()
{
	if(m_cmd2 & C2_DRS0)
		m_cur_floppy = m_floppy[0]->get_device();
	else if(m_cmd2 & C2_DRS1)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;
}

void thmfc1_device::cmd0_w(u8 data)
{
	sync();

	static const char *const mode[4] = { "reset", "wsect", "rhead", "rsect" };
	m_cmd0 = data;
	logerror("cmd0_w %02x, code=%s, ensyn=%d nomck=%d wgc=%d mode=%s\n", m_cmd0,
			 m_cmd0 & C0_FM ? "fm" : "mfm",
			 m_cmd0 & C0_ENSYN ? 1 : 0,
			 m_cmd0 & C0_NOMCK ? 1 : 0,
			 m_cmd0 & C0_WGC ? 1 : 0,
			 mode[m_cmd0 & 3]);

	if(m_stat0 & S0_FREE) {
		switch(m_cmd0 & 3) {
		case 0:
			break;
		case 1:
			logerror("wsect\n");
			exit(0);
		case 2:
			logerror("rhead\n");
			exit(0);
		case 3:
			logerror("read_sector start h=%d t=%2d s=%2d sz=%d\n",
					 m_cmd1 & C1_SIDE ? 1 : 0,
					 m_trck,
					 m_sect,
					 128 << ((m_cmd1 >> 5) & 3));
			m_state = S_WAIT_HEADER_SYNC;
			m_stat0 &= ~S0_FREE;
			m_window_start = m_last_sync;
			break;
		}
	}
}

void thmfc1_device::cmd1_w(u8 data)
{
	sync();

	m_cmd1 = data;
	logerror("cmd1_w %02x, sector=(size=%d, side=%d) precomp=%d sync_only_when_ready=%s\n", m_cmd1,
			 128 << ((m_cmd1 >> 5) & 3),
			 m_cmd1 & C1_SIDE ? 1 : 0,
			 (m_cmd1 >> 1) & 7,
			 m_cmd1 & C1_DSYRD ? "on" : "off");
}

void thmfc1_device::cmd2_w(u8 data)
{
	sync();

	u8 prev = m_cmd2;

	m_cmd2 = data;
	logerror("cmd2_w %02x, side=%d dir=%d step=%d motor=%s sel=%c%c\n", m_cmd2,
			 m_cmd2 & C2_SISELB ? 1 : 0,
			 m_cmd2 & C2_DIRECB ? 1 : 0,
			 m_cmd2 & C2_STEP ? 1 : 0,
			 m_cmd2 & C2_MTON ? "on" : "off",
			 m_cmd2 & C2_DRS1 ? 'b' : '-',
			 m_cmd2 & C2_DRS0 ? 'a' : '-');

	if(m_cmd2 & C2_DRS0)
		m_cur_floppy = m_floppy[0]->get_device();
	else if(m_cmd2 & C2_DRS1)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	if(m_cur_floppy) {
		if((prev & C2_MTON) && !(m_cmd2 & C2_MTON))
			m_timer_motoroff->adjust(attotime::from_seconds(2));
		if(m_cmd2 & C2_MTON) {
			m_cur_floppy->mon_w(0);
			m_timer_motoroff->adjust(attotime::never);
		}
		m_cur_floppy->ss_w(m_cmd2 & C2_SISELB ? 0 : 1);
		m_cur_floppy->dir_w(m_cmd2 & C2_DIRECB ? 0 : 1);
		m_cur_floppy->stp_w(m_cmd2 & C2_STEP ? 0 : 1);
	}
}

void thmfc1_device::wdata_w(u8 data)
{
	m_data = data;
	m_stat0 &= ~(S0_BYTE | S0_DREQ);
	logerror("wdata_w %02x\n", data);
}

void thmfc1_device::wclk_w(u8 data)
{
	sync();

	m_clk = data;
	logerror("wclk_w %02x\n", data);
}

void thmfc1_device::wsect_w(u8 data)
{
	sync();

	m_sect = data;
	logerror("wsect_w %02x\n", data);
}

void thmfc1_device::wtrck_w(u8 data)
{
	sync();

	m_trck = data;
	logerror("wtrck_w %02x\n", data);
}

void thmfc1_device::wcell_w(u8 data)
{
	sync();

	m_cell = data;
	logerror("wcell_w %02x\n", data);
}

u8 thmfc1_device::stat0_r()
{
	if(!machine().side_effects_disabled()) {
		sync();
#if 0
		static int ps = -1;
		if(m_stat0 != ps)
			logerror("stat0_r %02x -%s%s%s%s%s%s\n", m_stat0,
					 m_stat0 & S0_BYTE  ? " byte" : "",
					 m_stat0 & S0_END   ? " end" : "",
					 m_stat0 & S0_FREE  ? " free" : "",
					 m_stat0 & S0_CRCER ? " crcer" : "",
					 m_stat0 & S0_DREQ  ? " dreq" : "",
					 m_stat0 & S0_SYNC  ? " sync" : "");
		ps = m_stat0;
#endif
	}
	return m_stat0;
}

u8 thmfc1_device::stat1_r()
{
	u8 res = 0;
	if(m_cur_floppy) {
		if(m_cur_floppy->idx_r())
			res |= S1_INDX;
		if(!m_cur_floppy->dskchg_r())
			res |= S1_DKCH;
		if(!m_cur_floppy->mon_r())
			res |= S1_MTON;
		if(!m_cur_floppy->trk00_r())
			res |= S1_TRK0;
		if(!m_cur_floppy->wpt_r())
			res |= S1_WPRT;
		if(!m_cur_floppy->ready_r())
			res |= S1_RDY;
	}

	if(!machine().side_effects_disabled())
		logerror("stat1_r %02x -%s%s%s%s%s%s\n", res,
				 res & S1_INDX ? " index" : "",
				 res & S1_DKCH ? " dskchg" : "",
				 res & S1_MTON ? " mton" : "",
				 res & S1_TRK0 ? " trk0" : "",
				 res & S1_WPRT ? " wprt" : "",
				 res & S1_RDY ? " ready" : "");
	return res;
}

u8 thmfc1_device::rdata_r()
{
	if(!machine().side_effects_disabled()) {
		logerror("rdata_r %02X (stat0=%02X)\n", m_data, m_stat0);
		m_stat0 &= ~(S0_BYTE | S0_DREQ);
	}
	return m_data;
}

u64 thmfc1_device::time_to_cycles(const attotime &tm) const
{
	return tm.as_ticks(clock());
}

attotime thmfc1_device::cycles_to_time(u64 cycles) const
{
	return attotime::from_ticks(cycles, clock());
}

bool thmfc1_device::read_one_bit(u64 limit, u64 &next_flux_change)
{
	while(next_flux_change <= m_last_sync) {
		attotime flux = m_cur_floppy ? m_cur_floppy->get_next_transition(cycles_to_time(m_last_sync+1)) : attotime::never;
		next_flux_change = flux.is_never() ? u64(-1) : time_to_cycles(flux);
	}

	u64 window_end = m_window_start + (m_cell & 0x7f);
	if(window_end > limit)
		return true;

	int bit = next_flux_change < window_end;
	if(bit && (m_cmd0 & C0_NOMCK))
		m_window_start = next_flux_change + ((m_cell & 0x7f) >> 1);
	else
		m_window_start = window_end;

	m_last_sync = window_end;

	m_shift_reg = (m_shift_reg << 1) | bit;
	m_bit_counter++;
	if(m_data_separator_phase) {
		m_data_reg = (m_data_reg << 1) | bit;
		if((m_crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			m_crc = (m_crc << 1) ^ 0x1021;
		else
			m_crc = m_crc << 1;
	}

	m_data_separator_phase = !m_data_separator_phase;
	return false;
}

u8 thmfc1_device::clk_bits() const
{
	return
		(m_shift_reg & 0x8000 ? 0x80 : 0x00) |
		(m_shift_reg & 0x2000 ? 0x40 : 0x00) |
		(m_shift_reg & 0x0800 ? 0x20 : 0x00) |
		(m_shift_reg & 0x0200 ? 0x10 : 0x00) |
		(m_shift_reg & 0x0080 ? 0x08 : 0x00) |
		(m_shift_reg & 0x0020 ? 0x04 : 0x00) |
		(m_shift_reg & 0x0008 ? 0x02 : 0x00) |
		(m_shift_reg & 0x0002 ? 0x01 : 0x00);
}


void thmfc1_device::sync()
{
	u64 next_sync = machine().time().as_ticks(clock());
	u64 next_flux_change = 0;
	while(m_last_sync < next_sync)
		switch(m_state) {
		case S_IDLE:
			m_last_sync = next_sync;
			break;

		case S_WAIT_HEADER_SYNC: {
			if(read_one_bit(next_sync, next_flux_change))
				return;
			if(m_shift_reg == 0xaaaa) {
				m_crc = 0xffff;
				m_data_separator_phase = false;
			}
			if(m_data_reg == m_data && clk_bits() == m_clk) {
				m_bit_counter = 0;
				m_state = S_VERIFY_HEADER;
			}
			break;
		}

		case S_VERIFY_HEADER: {
			if(read_one_bit(next_sync, next_flux_change))
				return;
			if(m_bit_counter & 0xf)
				break;
			bool valid = true;
			switch(m_bit_counter >> 4) {
			case 1:
			case 2:
				valid = m_data_reg == m_data && clk_bits() == m_clk;
				break;
			case 3:
				valid = m_data_reg == 0xfe;
				break;
			case 4:
				valid = m_data_reg == m_trck;
				break;
			case 5:
				// C1_SIDE seems to be ignored here, at least for floppy disks
				valid = (m_data_reg & 1) == (m_cmd2 & C2_SISELB ? 0 : 1);
				break;
			case 6:
				valid = m_data_reg == m_sect;
				break;
			case 7:
				valid = (m_data_reg & 3) == ((m_cmd1 >> 5) & 3);
				break;
				// 8 skipped
			case 9:
				valid = m_crc == 0;
				m_bit_counter = 0;
				m_state = S_SKIP_GAP;
				break;
			}
			if(!valid)
				m_state = S_WAIT_HEADER_SYNC;
			break;
		}

		case S_SKIP_GAP:
			if(read_one_bit(next_sync, next_flux_change))
				return;
			if(m_bit_counter == 27 << 4) {
				m_bit_counter = 0;
				m_state = S_WAIT_SECTOR_SYNC;
			}
			break;


		case S_WAIT_SECTOR_SYNC: {
			if(read_one_bit(next_sync, next_flux_change))
				return;
			if(m_shift_reg == 0xaaaa) {
				m_crc = 0xffff;
				m_data_separator_phase = false;
			}
			if(m_data_reg == m_data && clk_bits() == m_clk) {
				m_bit_counter = 0;
				m_data = m_data_reg;
				m_stat0 |= S0_DREQ;
				m_state = S_READ_SECTOR;
			}
			if(m_bit_counter == 42 << 4)
				m_state = S_WAIT_HEADER_SYNC;
			break;
		}

		case S_READ_SECTOR:
			if(read_one_bit(next_sync, next_flux_change))
				return;
			if(m_bit_counter != 16)
				break;
			if(m_stat0 & (S0_BYTE|S0_DREQ)) {
				logerror("read_sector end\n");
				if(m_crc)
					m_stat0 |= S0_CRCER;
				m_stat0 &= ~S0_BYTE;
				m_stat0 |= S0_FREE;
				m_cmd0 &= ~3;
				m_state = S_IDLE;
				break;
			}
			m_stat0 |= S0_BYTE;
			m_data = m_data_reg;
			m_bit_counter = 0;
			break;
		}
}
