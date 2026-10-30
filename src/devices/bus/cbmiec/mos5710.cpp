// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS5710 Custom Floppy Controller and Gate Array

**********************************************************************/

/*

Address Decode

   A 15 14 13 12 10  4  3
RAM   0  0  0  0  x  x  x
VIA1  0  0  0  1  0  x  x
VIA2  0  0  0  1  1  x  x
FDC   0  0  1  0  x  0  0
CIA   0  1  0  0  x  0  x
FDC2  0  1  0  0  0  1  x
RAM   0  1  1  x  x  x  x


Registers

2000  FDC Status Register
2001  FDC Track Register
2002  FDC Sector Register
2003  FDC Data Register
2004  FDC Control for 2001, 2002
2005  FDC Control/Counter for 2001, 2002

400C  CIA Serial Data Register
400D  CIA Interrupt Control Register
400D  CIA Control Register A

4010  FDC2 used in 2002, 2004
4011  FDC2 used in 2002
4012  FDC2 used in 2001
4013  FDC2 used in 2001
4014  FDC2 used in 2002
4015  FDC2 used in 2002
4016  FDC2 used in 2003
4017  FDC2

*/

#include "emu.h"
#include "mos5710.h"

DEFINE_DEVICE_TYPE(MOS5710, mos5710_device, "mos5710", "MOS5710 Custom Floppy Controller and Gate Array")

mos5710_device::mos5710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MOS5710, tag, owner, clock),
	m_write_irq(*this),
	m_write_sp(*this),
	m_write_cnt(*this),
	m_floppy(nullptr)
{
}

void mos5710_device::device_start()
{
	m_live_timer = timer_alloc(FUNC(mos5710_device::live_tick), this);
	m_sp_timer = timer_alloc(FUNC(mos5710_device::sp_tick), this);

	std::fill(std::begin(m_fdc2), std::end(m_fdc2), 0);
	m_tm = attotime::zero;
	m_irq = false;
	m_sp_in = 1;
	m_cnt_in = 1;
	m_sp_out = 1;
	m_cnt_out = 1;

	save_item(NAME(m_pll.ctime));
	save_item(NAME(m_pll.period));
	save_item(NAME(m_pll.min_period));
	save_item(NAME(m_pll.max_period));
	save_item(NAME(m_pll.period_adjust_base));
	save_item(NAME(m_pll.phase_adjust));
	save_item(NAME(m_pll.freq_hist));
	save_item(NAME(m_fdc2));
	save_item(NAME(m_ctrl0));
	save_item(NAME(m_ctrl1));
	save_item(NAME(m_sync_clock));
	save_item(NAME(m_sync_data));
	save_item(NAME(m_reg5));
	save_item(NAME(m_tm));
	save_item(NAME(m_writing));
	save_item(NAME(m_reading));
	save_item(NAME(m_synced));
	save_item(NAME(m_shift));
	save_item(NAME(m_bits));
	save_item(NAME(m_crc));
	save_item(NAME(m_rdata));
	save_item(NAME(m_rbr));
	save_item(NAME(m_rsync));
	save_item(NAME(m_crc_error));
	save_item(NAME(m_crc_checked));
	save_item(NAME(m_wdata));
	save_item(NAME(m_wbr));
	save_item(NAME(m_context));
	save_item(NAME(m_sdr));
	save_item(NAME(m_sdr_full));
	save_item(NAME(m_sp_shift));
	save_item(NAME(m_sp_bits));
	save_item(NAME(m_sp_shifting));
	save_item(NAME(m_icr));
	save_item(NAME(m_imr));
	save_item(NAME(m_cra));
	save_item(NAME(m_sp_in));
	save_item(NAME(m_cnt_in));
	save_item(NAME(m_sp_out));
	save_item(NAME(m_cnt_out));
	save_item(NAME(m_irq));
}

void mos5710_device::device_reset()
{
	if (m_writing && m_floppy)
		m_pll.stop_writing(m_floppy, machine().time());

	m_live_timer->adjust(attotime::never);
	m_pll.set_clock(attotime::from_hz(16_MHz_XTAL / 32));

	m_ctrl0 = 0;
	m_ctrl1 = 0;
	m_sync_clock = 0;
	m_sync_data = 0;
	m_reg5 = 0;
	m_tm = machine().time();
	m_writing = false;
	m_reading = false;
	m_synced = false;
	m_shift = 0;
	m_bits = 0;
	m_crc = 0xffff;
	m_rdata = 0;
	m_rbr = false;
	m_rsync = false;
	m_crc_error = false;
	m_crc_checked = true;
	m_wdata = 0;
	m_wbr = false;
	m_context = false;

	m_sdr = 0;
	m_sdr_full = false;
	m_sp_shift = 0;
	m_sp_bits = 0;
	m_icr = 0;
	m_imr = 0;
	m_cra = 0;
	sp_stop();
	set_sp_out(1);
	set_cnt_out(1);
	update_irq();
}

void mos5710_device::device_clock_changed()
{
	if (m_sp_shifting)
	{
		attotime period = clocks_to_attotime(7);
		m_sp_timer->adjust(period, 0, period);
	}
}

void mos5710_device::set_floppy(floppy_image_device *floppy)
{
	if (floppy == m_floppy)
		return;

	live_sync();

	if (m_writing && m_floppy)
		m_pll.stop_writing(m_floppy, m_tm);

	m_floppy = floppy;

	if (m_writing && m_floppy)
		m_pll.start_writing(m_tm, m_floppy);
}


//**************************************************************************
//  SERIAL PORT
//**************************************************************************

void mos5710_device::update_irq()
{
	bool irq = (m_icr & m_imr & 0x1f) != 0;

	if (irq != m_irq)
	{
		m_irq = irq;
		m_write_irq(irq ? ASSERT_LINE : CLEAR_LINE);
	}
}

void mos5710_device::set_sp_out(int state)
{
	if (m_sp_out != state)
	{
		m_sp_out = state;
		m_write_sp(state);
	}
}

void mos5710_device::set_cnt_out(int state)
{
	if (m_cnt_out != state)
	{
		m_cnt_out = state;
		m_write_cnt(state);
	}
}

void mos5710_device::sp_start()
{
	m_sp_shift = m_sdr;
	m_sdr_full = false;
	m_sp_bits = 0;
	m_sp_shifting = true;

	attotime period = clocks_to_attotime(7);
	m_sp_timer->adjust(period, 0, period);
}

void mos5710_device::sp_stop()
{
	m_sp_shifting = false;
	m_sp_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(mos5710_device::sp_tick)
{
	if (m_cnt_out)
	{
		set_cnt_out(0);
		set_sp_out(BIT(m_sp_shift, 7));
		m_sp_shift <<= 1;
	}
	else
	{
		set_cnt_out(1);

		if (++m_sp_bits == 8)
		{
			m_icr |= ICR_SP;
			update_irq();

			if (m_sdr_full)
			{
				m_sp_shift = m_sdr;
				m_sdr_full = false;
				m_sp_bits = 0;
			}
			else
			{
				sp_stop();
			}
		}
	}
}

void mos5710_device::sp_w(int state)
{
	m_sp_in = state;
}

void mos5710_device::cnt_w(int state)
{
	if (!BIT(m_cra, 6) && !m_cnt_in && state)
	{
		m_sp_shift = (m_sp_shift << 1) | m_sp_in;

		if (++m_sp_bits == 8)
		{
			m_sdr = m_sp_shift;
			m_sp_bits = 0;
			m_icr |= ICR_SP;
			update_irq();
		}
	}

	m_cnt_in = state;
}

uint8_t mos5710_device::cia_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset & 0x0f)
	{
	case 0x0c:
		data = m_sdr;
		break;

	case 0x0d:
		data = m_icr | (m_irq ? 0x80 : 0);

		if (!machine().side_effects_disabled())
		{
			m_icr = 0;
			update_irq();
		}
		break;

	case 0x0e:
		data = m_cra;
		break;
	}

	return data;
}

void mos5710_device::cia_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
	case 0x0c:
		m_sdr = data;

		if (BIT(m_cra, 6))
		{
			if (m_sp_shifting)
				m_sdr_full = true;
			else
				sp_start();
		}
		break;

	case 0x0d:
		if (BIT(data, 7))
			m_imr |= data & 0x1f;
		else
			m_imr &= ~data & 0x1f;

		update_irq();
		break;

	case 0x0e:
		if (BIT(m_cra ^ data, 6))
		{
			sp_stop();
			m_sdr_full = false;
			m_sp_bits = 0;
			set_sp_out(1);
			set_cnt_out(1);
		}

		m_cra = data;
		break;
	}
}


//**************************************************************************
//  FLOPPY CONTROLLER
//**************************************************************************

uint16_t mos5710_device::sync_pattern() const
{
	uint16_t raw = 0;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_sync_clock, i))
			raw |= 2 << (i * 2);

		if (BIT(m_sync_data, i))
			raw |= 1 << (i * 2);
	}

	return raw;
}

uint16_t mos5710_device::crc_byte(uint16_t crc, uint8_t data)
{
	for (int i = 0; i < 8; i++)
	{
		if ((crc ^ (data << 8)) & 0x8000)
			crc = (crc << 1) ^ 0x1021;
		else
			crc <<= 1;

		data <<= 1;
	}

	return crc;
}

void mos5710_device::live_start_read()
{
	m_reading = true;
	m_synced = false;
	m_crc_error = false;

	if (!m_writing)
	{
		m_tm = machine().time();
		m_pll.set_clock(attotime::from_hz(16_MHz_XTAL / 32));
		m_pll.read_reset(m_tm);
		m_shift = 0;
		m_bits = 0;
	}
}

void mos5710_device::live_start_write()
{
	m_tm = machine().time();
	m_pll.set_clock(attotime::from_hz(16_MHz_XTAL / 32));
	m_pll.reset(m_tm);
	m_writing = true;
	m_synced = false;
	m_bits = 0;
	m_context = false;

	if (m_floppy)
		m_pll.start_writing(m_tm, m_floppy);
}

void mos5710_device::live_stop_write()
{
	if (m_floppy)
		m_pll.stop_writing(m_floppy, m_tm);

	m_writing = false;
	m_synced = false;
	m_shift = 0;
	m_bits = 0;
	m_pll.set_clock(attotime::from_hz(16_MHz_XTAL / 32));
	m_pll.read_reset(m_tm);
}

void mos5710_device::write_load_byte()
{
	uint8_t data;
	bool mark = false;

	if (m_ctrl0 & CTRL0_DATA)
	{
		data = m_wdata;
		mark = m_ctrl0 & CTRL0_MARK;

		if (m_ctrl0 & CTRL0_CRC)
			m_crc = crc_byte(m_crc, data);
	}
	else
	{
		data = m_crc >> 8;
		m_crc <<= 8;
	}

	m_wbr = true;

	uint16_t raw = 0;
	bool context = m_context;

	for (int i = 7; i >= 0; i--)
	{
		bool bit = BIT(data, i);

		if (!bit && !context)
			raw |= 2 << (i * 2);

		if (bit)
			raw |= 1 << (i * 2);

		context = bit;
	}

	if (mark)
		raw &= ~(2 << ((m_ctrl1 & CTRL1_CLOCK_MASK) * 2));

	m_context = context;
	m_shift = raw;
	m_bits = 16;
}

void mos5710_device::read_deliver(uint8_t data, bool sync)
{
	if ((m_ctrl1 & CTRL1_CRC_CHECK) && !m_crc_checked)
	{
		m_crc_error = m_crc != 0;
		m_crc_checked = true;
	}

	m_crc = crc_byte(m_crc, data);
	m_rdata = data;
	m_rbr = true;
	m_rsync = sync;
}

void mos5710_device::live_run(const attotime &limit)
{
	for (;;)
	{
		if (m_writing)
		{
			if (!m_bits)
				write_load_byte();

			if (m_pll.write_next_bit(BIT(m_shift, 15), m_tm, m_floppy, limit))
				return;

			m_shift <<= 1;
			m_bits--;
		}
		else if (m_reading)
		{
			int bit = m_pll.get_next_bit(m_tm, m_floppy, limit);

			if (bit < 0)
				return;

			m_shift = (m_shift << 1) | bit;

			if (!m_synced)
			{
				if (m_shift == sync_pattern())
				{
					m_synced = true;
					m_bits = 0;
					m_crc = 0xffff;
					read_deliver(m_sync_data, true);
				}
			}
			else if (++m_bits == 16)
			{
				uint8_t data = 0;

				for (int i = 0; i < 8; i++)
					if (BIT(m_shift, i * 2))
						data |= 1 << i;

				m_bits = 0;
				read_deliver(data, m_shift == sync_pattern());
			}
		}
		else
		{
			return;
		}
	}
}

void mos5710_device::live_sync()
{
	attotime now = machine().time();

	if (m_writing || m_reading)
	{
		if (m_tm < now)
			live_run(now);

		if (m_writing && m_floppy)
			m_pll.commit(m_floppy, m_tm);
	}
}

void mos5710_device::live_schedule()
{
	if (m_writing)
		m_live_timer->adjust(m_pll.period * 16);
	else
		m_live_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(mos5710_device::live_tick)
{
	live_sync();
	live_schedule();
}

uint8_t mos5710_device::fdc_r(offs_t offset)
{
	uint8_t data = 0xff;

	live_sync();

	switch (offset & 0x07)
	{
	case 0:
		data = (m_wbr ? 0x80 : 0) | ((m_floppy && m_floppy->idx_r()) ? 0 : 0x40) | m_ctrl0;
		break;

	case 1:
		data = (m_rbr ? 0x80 : 0) | (m_rsync ? 0x40 : 0) | (m_crc_error ? 0x20 : 0) | (m_ctrl1 & 0x1f);
		break;

	case 2:
		data = m_rdata;

		if (!machine().side_effects_disabled())
			m_rbr = false;
		break;

	case 3:
		data = m_sync_clock;
		break;

	case 4:
		data = m_sync_data;
		break;

	case 5:
		data = m_reg5;
		break;
	}

	return data;
}

void mos5710_device::fdc_w(offs_t offset, uint8_t data)
{
	live_sync();

	switch (offset & 0x07)
	{
	case 0:
	{
		uint8_t old = m_ctrl0;
		m_ctrl0 = data & 0x3f;

		if (!(old & CTRL0_CRC) && (m_ctrl0 & CTRL0_CRC))
			m_crc = 0xffff;

		if (!(old & CTRL0_WG) && (m_ctrl0 & CTRL0_WG))
		{
			live_start_write();
			live_run(machine().time());
		}
		else if ((old & CTRL0_WG) && !(m_ctrl0 & CTRL0_WG))
		{
			live_stop_write();
		}
		break;
	}

	case 1:
	{
		uint8_t old = m_ctrl1;
		m_ctrl1 = data;

		if (!(old & CTRL1_READ) && (data & CTRL1_READ))
			live_start_read();
		else if (!(data & CTRL1_READ))
			m_reading = false;

		if (!(old & CTRL1_CRC_CHECK) && (data & CTRL1_CRC_CHECK))
		{
			m_crc_error = false;
			m_crc_checked = false;
		}
		break;
	}

	case 2:
		m_wdata = data;
		m_wbr = false;
		break;

	case 3:
		m_sync_clock = data;
		break;

	case 4:
		m_sync_data = data;
		break;

	case 5:
		m_reg5 = data;
		break;
	}

	live_schedule();
}

uint8_t mos5710_device::fdc2_r(offs_t offset)
{
	return m_fdc2[offset & 0x07];
}

void mos5710_device::fdc2_w(offs_t offset, uint8_t data)
{
	m_fdc2[offset & 0x07] = data;
}
