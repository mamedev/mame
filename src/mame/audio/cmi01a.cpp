// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI-01A Channel Controller Card

***************************************************************************/

#include "emu.h"
#include "audio/cmi01a.h"
#include "machine/input_merger.h"

#define MASTER_OSCILLATOR       XTAL(34'291'712)


DEFINE_DEVICE_TYPE(CMI01A_CHANNEL_CARD, cmi01a_device, "cmi_01a", "Fairlight CMI-01A Channel Card")

cmi01a_device::cmi01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CMI01A_CHANNEL_CARD, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
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
	m_pia[0]->irqa_handler().set("cmi01a_irq", FUNC(input_merger_device::in_w<0>));
	m_pia[0]->irqb_handler().set("cmi01a_irq", FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia[1], 0); // pia_cmi01a_2_config
	m_pia[1]->readca1_handler().set(FUNC(cmi01a_device::zx_r));
	m_pia[1]->readca2_handler().set(FUNC(cmi01a_device::eosi_r));
	m_pia[1]->writepa_handler().set(FUNC(cmi01a_device::pia_1_a_w));
	m_pia[1]->writepb_handler().set(FUNC(cmi01a_device::pia_1_b_w));
	m_pia[1]->irqa_handler().set("cmi01a_irq", FUNC(input_merger_device::in_w<2>));
	m_pia[1]->irqb_handler().set("cmi01a_irq", FUNC(input_merger_device::in_w<3>));

	PTM6840(config, m_ptm, 2000000); // ptm_cmi01a_config
	m_ptm->set_external_clocks(250000, 500000, 500000);
	m_ptm->o1_callback().set(FUNC(cmi01a_device::ptm_o1));
	m_ptm->irq_callback().set("cmi01a_irq", FUNC(input_merger_device::in_w<4>));

	INPUT_MERGER_ANY_HIGH(config, "cmi01a_irq").output_handler().set(FUNC(cmi01a_device::cmi01a_irq));
}


void cmi01a_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	if (m_active && m_vol_latch)
	{
		int length = samples;
		int seg_addr = m_segment_cnt & 0x7f;
		uint8_t *wave_ptr = &m_wave_ram[m_segment_cnt & 0x3fff];
		stream_sample_t *buf = outputs[0];

		while (length--)
		{
			*buf++ = wave_ptr[seg_addr];
			seg_addr = (seg_addr + 1) & 0x7f;
		}

		m_segment_cnt = (m_segment_cnt & ~0x7f) | seg_addr;
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
	m_zx_timer->adjust(attotime::never);

	m_stream = stream_alloc(0, 1, 44100);
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

	m_freq = 0.0;
	m_active = false;

	m_ptm_o1 = 0;
}

WRITE_LINE_MEMBER( cmi01a_device::pia_0_ca2_w )
{
	// upate_stream()
	if (!state)
	{
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
	return (top_terminal_count || bottom_terminal_count) ? 1 : 0;
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
	}
}

void cmi01a_device::zx_timer_cb()
{
	/* Set ZX */
	if (m_zx_flag == 0)
		m_pia[1]->ca1_w(1);
	else
		m_pia[1]->ca1_w(0);

	m_zx_flag ^= 1;

	if (m_zx_flag == 0)
	{
		/* Low to high transition - clock flip flop */
		int op = m_ptm_o1;

		/* Set /ZCINT */
		if (op != m_zx_ff)
			m_pia[0]->ca1_w(0);

		m_zx_ff = op;
		m_pia[0]->ca1_w(1);
	}
}

void cmi01a_device::run_voice()
{
	int val_a = m_pia[1]->a_output();
	int pitch = ((val_a & 3) << 8) | m_pia[1]->b_output();
	int o_val = (val_a >> 2) & 0xf;

	int m_tune = m_cmi02_pia[0]->b_output();
	double mfreq = (double)(0xf00 | m_tune) * (MASTER_OSCILLATOR / 2.0 / 4096.0).dvalue();

	double cfreq = ((double)(0x800 | (pitch << 1))* mfreq) / 4096.0;

//  if (cfreq > 0.0)
	{
		/* Octave register enabled? */
		if (!(o_val & 0x8))
			cfreq /= 2 << ((7 ^ o_val) & 7);

		cfreq /= 16.0f;

		m_freq = cfreq;

		m_stream->set_sample_rate(cfreq);

		// Set timers and things?
		attotime zx_period = attotime::from_ticks(64, cfreq);
		m_zx_timer->adjust(zx_period, 0, zx_period);

		m_active = true;
	}
}

WRITE_LINE_MEMBER( cmi01a_device::pia_0_cb2_w )
{
	//streams_update();

	/* RUN */
	if (state)
	{
		m_segment_cnt = 0x4000 | ((m_pia[0]->a_output() & 0x7f) << 7);
		m_new_addr = 1;

		/* Clear /EOSI */
//      pia6821_cb1_w(card->pia[1], 0, 1);

		/* Clear ZX */
		m_pia[1]->ca1_w(0);

		/* Clear /ZCINT */
		m_pia[0]->ca1_w(1);

		m_ptm->set_g1(0);
		m_ptm->set_g2(0);
		m_ptm->set_g3(0);

		run_voice();
	}
	else
	{
		/* Clear /EOSI */
		m_pia[1]->cb1_w(1);

		m_ptm->set_g1(1);
		m_ptm->set_g2(1);
		m_ptm->set_g3(1);

		//printf("Stop %d\n", m_channel);

		m_zx_timer->adjust(attotime::never);
		m_active = false;
		m_zx_flag = 0;  // TEST
		m_zx_ff = 0;
	}

}

void cmi01a_device::update_wave_addr(int inc)
{
	int old_cnt = m_segment_cnt;

	if (inc)
		++m_segment_cnt;

	/* Update end of sound interrupt flag */
	m_pia[1]->cb1_w((m_segment_cnt & 0x4000) >> 14);

	/* TODO Update zero crossing flag */
	m_pia[1]->ca1_w((m_segment_cnt & 0x40) >> 6);

	/* Clock a latch on a transition */
	if ((old_cnt & 0x40) && !(m_segment_cnt & 0x40))
	{
		// TODO: ECLK
		m_pia[1]->ca2_w(1);
		m_pia[1]->ca2_w(0);
	}

	/* Zero crossing interrupt is a pulse */
}

WRITE_LINE_MEMBER( cmi01a_device::ptm_o1 )
{
	m_ptm_o1 = state;
}

READ_LINE_MEMBER( cmi01a_device::eosi_r )
{
	return (m_segment_cnt & 0x4000) >> 14;
}

READ_LINE_MEMBER( cmi01a_device::zx_r )
{
	return m_segment_cnt & 0x40;
}

void cmi01a_device::write(offs_t offset, uint8_t data)
{
	//printf("C%d W: %02x = %02x\n", m_channel, offset, data);

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
			m_pia[0]->write(offset & 3, data);
			break;

		case 0xc: case 0xd: case 0xe: case 0xf:
			m_pia[1]->write((BIT(offset, 0) << 1) | BIT(offset, 1), data);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		{
			/* PTM addressing is a little funky */
			int a0 = offset & 1;
			int a1 = (m_ptm_o1 && BIT(offset, 3)) || (!BIT(offset, 3) && BIT(offset, 2));
			int a2 = BIT(offset, 1);

			//printf("CH%d PTM W: [%x] = %02x\n", m_channel, (a2 << 2) | (a1 << 1) | a0, data);
			m_ptm->write((a2 << 2) | (a1 << 1) | a0, data);
			break;
		}

		default:
			logerror("Unknown channel card write to E0%02X = %02X\n", offset, data);
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
			break;

		case 0xc: case 0xd: case 0xe: case 0xf:
			data = m_pia[1]->read((BIT(offset, 0) << 1) | BIT(offset, 1));
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		{
			int a0 = offset & 1;
			int a1 = (m_ptm_o1 && BIT(offset, 3)) || (!BIT(offset, 3) && BIT(offset, 2));
			int a2 = BIT(offset, 1);

			data = m_ptm->read((a2 << 2) | (a1 << 1) | a0);

			//printf("CH%d PTM R: [%x] %02x\n", m_channel, (a2 << 2) | (a1 << 1) | a0, data);
			break;
		}

		default:
			logerror("Unknown channel card read from E0%02X\n", offset);
			break;
	}

	//printf("C%d R: %02x = %02x\n", m_channel, offset, data);

	return data;
}
