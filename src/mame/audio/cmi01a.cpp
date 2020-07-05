// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI-01A Channel Controller Card

***************************************************************************/

#include "emu.h"
#include "audio/cmi01a.h"

#define VERBOSE     (0)
#include "logmacro.h"

#define MASTER_OSCILLATOR       XTAL(34'291'712)


DEFINE_DEVICE_TYPE(CMI01A_CHANNEL_CARD, cmi01a_device, "cmi_01a", "Fairlight CMI-01A Channel Card")

cmi01a_device::cmi01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CMI01A_CHANNEL_CARD, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq_merger(*this, "cmi01a_irq")
	, m_pia(*this, "cmi01a_pia_%u", 0U)
	, m_ptm(*this, "cmi01a_ptm")
	, m_cmi02_pia(*this, "^cmi02_pia_%u", 1U)
	, m_stream(nullptr)
	, m_irq_cb(*this)
{
}

void cmi01a_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia[0], 0); // pia_cmi01a_1_config
	m_pia[0]->readcb1_handler().set(FUNC(cmi01a_device::tri_r));
	m_pia[0]->writepa_handler().set(FUNC(cmi01a_device::ws_dir_w));
	m_pia[0]->writepb_handler().set(FUNC(cmi01a_device::rp_w));
	m_pia[0]->ca2_handler().set(FUNC(cmi01a_device::pia_0_ca2_w));
	m_pia[0]->cb2_handler().set(FUNC(cmi01a_device::pia_0_cb2_w));
	m_pia[0]->irqa_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<0>));
	m_pia[0]->irqb_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia[1], 0); // pia_cmi01a_2_config
	m_pia[1]->readca1_handler().set(FUNC(cmi01a_device::zx_r));
	m_pia[1]->readca2_handler().set(FUNC(cmi01a_device::eosi_r));
	m_pia[1]->writepa_handler().set(FUNC(cmi01a_device::pia_1_a_w));
	m_pia[1]->writepb_handler().set(FUNC(cmi01a_device::pia_1_b_w));
	m_pia[1]->irqa_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<2>));
	m_pia[1]->irqb_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<3>));

	PTM6840(config, m_ptm, DERIVED_CLOCK(1, 1)); // ptm_cmi01a_config
	m_ptm->o1_callback().set(FUNC(cmi01a_device::ptm_o1));
	m_ptm->o2_callback().set(FUNC(cmi01a_device::ptm_o2));
	m_ptm->o3_callback().set(FUNC(cmi01a_device::ptm_o3));
	m_ptm->irq_callback().set(FUNC(cmi01a_device::ptm_irq));

	INPUT_MERGER_ANY_HIGH(config, m_irq_merger).output_handler().set(FUNC(cmi01a_device::cmi01a_irq));
}


void cmi01a_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	if ((m_status & CHANNEL_STATUS_RUN) && m_vol_latch)
	{
		int length = samples;
		int mask = (m_status & CHANNEL_STATUS_LOAD) ? 0x7fff : 0x7f;
		int addr = m_segment_cnt;

		uint8_t *wave_ptr = &m_wave_ram[m_segment_cnt & 0x3fff];
		stream_sample_t *buf = outputs[0];

		while (length--)
		{
			*buf++ = wave_ptr[addr++ & 0x3fff] << 6;
		}

		m_segment_cnt = (m_segment_cnt & ~mask) | addr;
	}
	else
	{
		memset(outputs[0], 0, samples);
	}
}

void cmi01a_device::device_resolve_objects()
{
	m_irq_cb.resolve_safe();
}

void cmi01a_device::device_start()
{
	m_wave_ram = std::make_unique<uint8_t[]>(0x4000);

	m_zx_timer = timer_alloc(TIMER_ZX);
	m_eosi_timer = timer_alloc(TIMER_EOSI);

	m_zx_timer->adjust(attotime::never);
	m_eosi_timer->adjust(attotime::never);

	m_stream = stream_alloc(0, 1, 44100);

	m_ptm->set_external_clocks(clock() / 8, clock() / 4, clock() / 4);
}

void cmi01a_device::device_reset()
{
	m_ptm->set_g1(1);
	m_ptm->set_g2(1);
	m_ptm->set_g3(1);

	m_segment_cnt = 0;
	m_new_addr = 0;
	m_env_dir_ctrl = 0;
	m_vol_latch = 0;
	m_flt_latch = 0;
	m_rp = 0;
	m_ws = 0;
	m_dir = 0;
	m_pia0_cb2_state = 1;
	m_zx_flag = 0;

	m_freq = 0.0;
	m_status = 0;

	m_ptm_o1 = 0;
	m_ptm_o2 = 0;
	m_ptm_o3 = 0;
	m_eclk = false;

	m_zx_timer->adjust(attotime::never);
	m_eosi_timer->adjust(attotime::never);
}

WRITE_LINE_MEMBER( cmi01a_device::pia_0_ca2_w )
{
	m_status &= ~CHANNEL_STATUS_LOAD;

	if (!state)
	{
		m_status |= CHANNEL_STATUS_LOAD;

		m_segment_cnt = 0x4000 | ((m_pia[0]->a_output() & 0x7f) << 7);
		m_new_addr = 1;
		m_pia[1]->cb1_w(1);
	}
}

void cmi01a_device::pia_1_a_w(uint8_t data)
{
// top two
}

void cmi01a_device::pia_1_b_w(uint8_t data)
{
}

void cmi01a_device::rp_w(uint8_t data)
{
	m_rp = data;
}

void cmi01a_device::ws_dir_w(uint8_t data)
{
	m_ws = data & 0x7f;
	m_dir = (data >> 7) & 1;
}

READ_LINE_MEMBER( cmi01a_device::tri_r )
{
	bool top_terminal_count = (m_dir == ENV_DIR_UP && m_rp == 0);
	bool bottom_terminal_count = (m_dir == ENV_DIR_DOWN && m_rp == 0xff);
	return (top_terminal_count || bottom_terminal_count) ? 0 : 1;
}

WRITE_LINE_MEMBER( cmi01a_device::cmi01a_irq )
{
	m_irq_cb(state ? ASSERT_LINE : CLEAR_LINE);
}

void cmi01a_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_ZX:
			zx_timer_cb();
			break;
		case TIMER_EOSI:
			eosi_timer_cb();
			break;
	}
}

void cmi01a_device::eosi_timer_cb()
{
	m_segment_cnt &= ~0x4000;

//	printf("End of sound\n");
}

void cmi01a_device::zx_timer_cb()
{
	// Toggle ZX
	m_zx_flag ^= 1;

	// Update ZX input to PIA 1
	m_pia[1]->ca1_w(m_zx_flag);

	// 74LS74 A12 (1) is clocked by /ZX, so a 1->0 transition of the ZX flag is a positive clock transition
	if (m_zx_flag == 0)
	{
		// Pulse /ZCINT if the O1 output of the PTM has changed
		if (m_ptm_o1 != m_zx_ff)
			m_pia[0]->ca1_w(0);

		m_zx_ff = m_ptm_o1;
		m_pia[0]->ca1_w(1);

		// Update ECLK
		bool eclk = (m_ptm_o2 && m_zx_ff) || (m_ptm_o3 && !m_zx_ff);
		set_eclk(eclk);
	}
}

void cmi01a_device::tick_ediv()
{
}

void cmi01a_device::set_eclk(bool eclk)
{
	bool old_eclk = m_eclk;
	m_eclk = eclk;

	if (old_eclk == m_eclk)
		return;

	tick_ediv();

	//	A	B	!(A && B)	!A || !B
	//	0	0	1			1
	//	0	1	1			1
	//	1	0	1			1
	//	1	1	0			0

	//const bool load = (m_status & CHANNEL_STATUS_LOAD);
	//const bool a = !load || !eclk;
	//const bool b =  load || m_ediv_out;

	//const bool div_clk = !a || !b;
}

void cmi01a_device::run_voice()
{
	int val_a = m_pia[1]->a_output();
	int pitch = ((val_a & 3) << 8)
				| m_pia[1]->b_output();
	int o_val = (val_a >> 2) & 0xf;

	LOG("CH%d running voice: PIA1 A output = %02x\n", m_channel, (uint8_t)val_a);
	LOG("CH%d running voice: Pitch = %04x\n", m_channel, (uint16_t)pitch);
	LOG("CH%d running voice: o_val = %x\n", m_channel, o_val);

	int m_tune = m_cmi02_pia[0]->b_output();
	LOG("CH%d running voice: Tuning = %02x\n", m_channel, (uint8_t)m_tune);
	double mfreq = (double)(0xf00 | m_tune) * ((MASTER_OSCILLATOR.dvalue() / 2.0) / 4096.0);
	LOG("CH%d running voice: mfreq = %f (%03x * %f)\n", m_channel, mfreq, 0xf00 | m_tune, (MASTER_OSCILLATOR.dvalue() / 2.0) / 4096.0);

	double cfreq = ((double)(0x800 | (pitch << 1)) * mfreq) / 4096.0;
	LOG("CH%d running voice: cfreq = %f (%04x * %f) / 4096.0\n", m_channel, cfreq, 0x800 | (pitch << 1), mfreq, cfreq);

	if (cfreq > MASTER_OSCILLATOR.dvalue())
	{
		LOG("CH%d Ignoring voice run due to excessive frequency\n");
		return;
	}

	LOG("CH%d Running voice\n", m_channel);
	/* Octave register enabled? */
	if (!BIT(o_val, 3))
		cfreq /= (double)(2 << ((7 ^ o_val) & 7));

	cfreq /= 16.0;

	m_freq = cfreq;

	LOG("CH%d running voice: Final freq: %f\n", m_channel, m_freq);

	m_stream->set_sample_rate(cfreq);

	// Set timers and things
	m_zx_flag = 0;
	attotime zx_period = attotime::from_ticks(64, cfreq);
	m_zx_timer->adjust(zx_period, 0, zx_period);

	if (m_status & CHANNEL_STATUS_LOAD)
	{
		int samples = 0x4000 - (m_segment_cnt & 0x3fff);
		LOG("CH%d voice is %04x samples long\n", m_channel, samples);
		m_eosi_timer->adjust(attotime::from_ticks(samples, cfreq));
	}
}

WRITE_LINE_MEMBER( cmi01a_device::pia_0_cb2_w )
{
	int old_state = m_pia0_cb2_state;
	m_pia0_cb2_state = state;
	LOG("CH%d PIA0 CB2: %d\n", m_channel, state);

	//streams_update();

	/* RUN */
	if (!old_state && m_pia0_cb2_state)
	{
		m_status |= CHANNEL_STATUS_RUN;

		/* Only reset address counter if /LOAD not asserted */
		if ((m_status & CHANNEL_STATUS_LOAD) == 0)
		{
			m_segment_cnt = 0x4000 | ((m_pia[0]->a_output() & 0x7f) << 7);
			m_new_addr = 1;
		}

		/* Clear ZX */
		m_pia[1]->ca1_w(0);

		/* Clear /ZCINT */
		m_pia[0]->ca1_w(1);

		m_ptm->set_g1(0);
		m_ptm->set_g2(0);
		m_ptm->set_g3(0);

		run_voice();
	}

	if (old_state && !m_pia0_cb2_state)
	{
		m_status &= ~CHANNEL_STATUS_RUN;

		/* Clear /EOSI */
		m_pia[1]->cb1_w(1);

		m_ptm->set_g1(1);
		m_ptm->set_g2(1);
		m_ptm->set_g3(1);

		m_zx_timer->adjust(attotime::never);
		m_eosi_timer->adjust(attotime::never);
		m_zx_ff = 0;
	}

}

void cmi01a_device::update_wave_addr(int inc)
{
	int old_cnt = m_segment_cnt;

	if (inc)
		++m_segment_cnt;

	/* Update end of sound interrupt flag */
	//m_pia[1]->cb1_w((m_segment_cnt & 0x4000) >> 14);

	/* TODO Update zero crossing flag */
	//m_pia[1]->ca1_w((m_segment_cnt & 0x40) >> 6);

	/* Clock a latch on a transition */
	if ((old_cnt & 0x40) && !(m_segment_cnt & 0x40))
	{
		//m_pia[1]->ca2_w(1);
		//m_pia[1]->ca2_w(0);
	}

	/* Zero crossing interrupt is a pulse */
}

WRITE_LINE_MEMBER( cmi01a_device::ptm_irq )
{
	m_irq_merger->in_w<4>(state);
}

WRITE_LINE_MEMBER( cmi01a_device::ptm_o1 )
{
	m_ptm_o1 = state;
	// TODO: Update ECLK
}

WRITE_LINE_MEMBER( cmi01a_device::ptm_o2 )
{
	m_ptm_o2 = state;
	// TODO: Update ECLK
}

WRITE_LINE_MEMBER( cmi01a_device::ptm_o3 )
{
	m_ptm_o3 = state;
	// TODO: Update ECLK
}

READ_LINE_MEMBER( cmi01a_device::eosi_r )
{
	return BIT(m_segment_cnt, 14);
}

READ_LINE_MEMBER( cmi01a_device::zx_r )
{
	return (m_segment_cnt & 0x40) >> 6;
}

void cmi01a_device::write(offs_t offset, uint8_t data)
{
	LOG("%s: channel card %d write: %02x = %02x\n", machine().describe_context(), m_channel, offset, data);

	switch (offset)
	{
		case 0x0:
			if (m_new_addr)
				m_new_addr = 0;

			m_wave_ram[m_segment_cnt & 0x3fff] = data;
			update_wave_addr(1);
			break;

		case 0x3:
			m_env_dir_ctrl = ENV_DIR_DOWN;
			break;

		case 0x4:
			m_env_dir_ctrl = ENV_DIR_UP;
			break;

		case 0x5:
			m_vol_latch = data;
			break;

		case 0x6:
			m_flt_latch = data;
			break;

		case 0x8: case 0x9: case 0xa: case 0xb:
			LOG("CH%d PIA0 Write: %d = %02x\n", m_channel, offset & 3, data);
			m_pia[0]->write(offset & 3, data);
			break;

		case 0xc: case 0xd: case 0xe: case 0xf:
			LOG("CH%d PIA1 Write: %d = %02x\n", m_channel, (BIT(offset, 0) << 1) | BIT(offset, 1), data);
			m_pia[1]->write((BIT(offset, 0) << 1) | BIT(offset, 1), data);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		{
			/* PTM addressing is a little funky */
			int a0 = offset & 1;
			int a1 = (m_ptm_o1 && BIT(offset, 3)) || (!BIT(offset, 3) && BIT(offset, 2));
			int a2 = BIT(offset, 1);

			if ((offset == 5 || offset == 7) && (data < 0x30))
				data = 0xff;

			LOG("CH%d PTM Write: %d = %02x\n", m_channel, (a2 << 2) | (a1 << 1) | a0, data);
			m_ptm->write((a2 << 2) | (a1 << 1) | a0, data);
			break;
		}

		default:
			LOG("%s: Unknown channel card write to E0%02X = %02X\n", machine().describe_context(), offset, data);
			break;
	}
}

uint8_t cmi01a_device::read(offs_t offset)
{
	if (machine().side_effects_disabled())
		return 0;

	uint8_t data = 0;

	switch (offset)
	{
		case 0x0:
			if (m_new_addr)
			{
				m_new_addr = 0;
				break;
			}
			data = m_wave_ram[m_segment_cnt & 0x3fff];
			update_wave_addr(1);
			break;

		case 0x3:
			m_env_dir_ctrl = ENV_DIR_DOWN;
			break;

		case 0x4:
			m_env_dir_ctrl = ENV_DIR_UP;
			break;

		case 0x5:
			data = 0xff;
			break;

		case 0x8: case 0x9: case 0xa: case 0xb:
			data = m_pia[0]->read(offset & 3);
			LOG("CH%d PIA0 Read: %d = %02x\n", m_channel, offset & 3, data);
			break;

		case 0xc: case 0xd: case 0xe: case 0xf:
			data = m_pia[1]->read((BIT(offset, 0) << 1) | BIT(offset, 1));
			LOG("CH%d PIA1 Read: %d = %02x\n", m_channel, (BIT(offset, 0) << 1) | BIT(offset, 1), data);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		{
			int a0 = offset & 1;
			int a1 = (m_ptm_o1 && BIT(offset, 3)) || (!BIT(offset, 3) && BIT(offset, 2));
			int a2 = BIT(offset, 1);

			data = m_ptm->read((a2 << 2) | (a1 << 1) | a0);

			LOG("CH%d PTM Read: %d = %02x\n", m_channel, (a2 << 2) | (a1 << 1) | a0, data);
			break;
		}

		default:
			LOG("%s: Unknown channel card %d read from E0%02X\n", machine().describe_context(), m_channel, offset);
			break;
	}

	LOG("%s: channel card %d read: %02x = %02x\n", machine().describe_context(), m_channel, offset, data);

	return data;
}
