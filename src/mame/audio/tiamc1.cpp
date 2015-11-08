// license:BSD-3-Clause
// copyright-holders:Eugene Sandulenko
/***************************************************************************

  TIA-MC1 sound hardware

  driver by Eugene Sandulenko
  special thanks to Shiru for his standalone emulator and documentation

            timer1       timer0
          |--------|   |--------|
          |  8253  |   |  8253  |   |---|
      in0 |        |   |        |   | 1 |   |---|
      ----+ g0  o0 +---+ g0  o0 +---+   |   | & |
      in1 |        |   |        |   |   o---+   |  out
      ----+ g1  o1 +---+ g1  o1 +---+   |   |   +-----
      in2 |        |   |        |   |---|   |   |
      ----+ g2  o2 +---+ g2  o2 +-----------+   |
      clk1|        |   |        |           |---|
      ----+ clk    | +-+ clk    |
          |        | | |        |
          |--------| | |--------|
      clk0           |
      ---------------+

  in0-in2 comes from port #da
  clk0    comes from 8224 and equals to 1777777Hz, i.e. processor clock
  clk1    comes from divider 16000000/4/16/16/2 = 7812Hz

  ********************

  TODO: use machine/pit8253.c

***************************************************************************/

#include "emu.h"
#include "includes/tiamc1.h"

#define CLOCK_DIVIDER 16
#define BUF_LEN 100000

#define T8253_CHAN0     0
#define T8253_CHAN1     1
#define T8253_CHAN2     2
#define T8253_CWORD     3


// device type definition
const device_type TIAMC1 = &device_creator<tiamc1_sound_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tiamc1_sound_device - constructor
//-------------------------------------------------

tiamc1_sound_device::tiamc1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TIAMC1, "TIA-MC1 Audio Custom", tag, owner, clock, "tiamc1_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_channel(NULL),
		m_timer1_divider(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tiamc1_sound_device::device_start()
{
	int i, j;

	timer8253_reset(&m_timer0);
	timer8253_reset(&m_timer1);

	m_channel = stream_alloc(0, 1, clock() / CLOCK_DIVIDER);

	m_timer1_divider = 0;

	for (i = 0; i < 2; i++)
	{
		struct timer8253struct *t = (i ? &m_timer1 : &m_timer0);

		for (j = 0; j < 3; j++)
		{
			save_item(NAME(t->channel[j].count), i * 3 + j);
			save_item(NAME(t->channel[j].cnval), i * 3 + j);
			save_item(NAME(t->channel[j].bcdMode), i * 3 + j);
			save_item(NAME(t->channel[j].cntMode), i * 3 + j);
			save_item(NAME(t->channel[j].valMode), i * 3 + j);
			save_item(NAME(t->channel[j].gate), i * 3 + j);
			save_item(NAME(t->channel[j].output), i * 3 + j);
			save_item(NAME(t->channel[j].loadCnt), i * 3 + j);
			save_item(NAME(t->channel[j].enable), i * 3 + j);
		}
	}

	save_item(NAME(m_timer1_divider));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tiamc1_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int count, o0, o1, o2, len, orval = 0;

	len = samples * CLOCK_DIVIDER;

	for (count = 0; count < len; count++)
	{
		m_timer1_divider++;
		if (m_timer1_divider == 228)
		{
			m_timer1_divider = 0;
			timer8253_tick(&m_timer1, 0);
			timer8253_tick(&m_timer1, 1);
			timer8253_tick(&m_timer1, 2);

			timer8253_set_gate(&m_timer0, 0, timer8253_get_output(&m_timer1, 0));
			timer8253_set_gate(&m_timer0, 1, timer8253_get_output(&m_timer1, 1));
			timer8253_set_gate(&m_timer0, 2, timer8253_get_output(&m_timer1, 2));
		}

		timer8253_tick(&m_timer0, 0);
		timer8253_tick(&m_timer0, 1);
		timer8253_tick(&m_timer0, 2);

		o0 = timer8253_get_output(&m_timer0, 0) ? 1 : 0;
		o1 = timer8253_get_output(&m_timer0, 1) ? 1 : 0;
		o2 = timer8253_get_output(&m_timer0, 2) ? 1 : 0;

		orval = (orval << 1) | (((o0 | o1) ^ 0xff) & o2);

		if ((count + 1) % CLOCK_DIVIDER == 0)
		{
			outputs[0][count / CLOCK_DIVIDER] = orval ? 0x2828 : 0;
			orval = 0;
		}
	}
}


void tiamc1_sound_device::timer8253_reset(struct timer8253struct *t)
{
	memset(t,0,sizeof(struct timer8253struct));
}


void tiamc1_sound_device::timer8253_tick(struct timer8253struct *t, int chn)
{
	if (t->channel[chn].enable && t->channel[chn].gate)
	{
		switch (t->channel[chn].cntMode)
		{
		case 0:
			t->channel[chn].count--;
			if (t->channel[chn].count == 0xffff)
				t->channel[chn].output = 1;
			break;

		case 3:
			t->channel[chn].count--;

			if (t->channel[chn].count < (t->channel[chn].cnval >> 1))
				t->channel[chn].output = 0;
			else
				t->channel[chn].output = 1;

			if (t->channel[chn].count == 0xffff)
				t->channel[chn].count = t->channel[chn].cnval;
			break;

		case 4:
			t->channel[chn].count--;
			if(t->channel[chn].count==0)
				t->channel[chn].output = 1;

			if(t->channel[chn].count == 0xffff)
			{
				t->channel[chn].enable = 0;
				t->channel[chn].output = 1;
			}
			break;
		}
	}
}



void tiamc1_sound_device::timer8253_wr(struct timer8253struct *t, int reg, UINT8 val)
{
	int chn;

	switch (reg)
	{
	case T8253_CWORD:
		chn = val >> 6;
		if (chn < 3)
		{
			t->channel[chn].bcdMode = (val & 1) ? 1 : 0;
			t->channel[chn].cntMode = (val >> 1) & 0x07;
			t->channel[chn].valMode = (val >> 4) & 0x03;

			switch (t->channel[chn].valMode)
			{
			case 1:
			case 2:
				t->channel[chn].loadCnt = 1;
				break;

			case 3:
				t->channel[chn].loadCnt = 2;
				break;

			default:
				osd_printf_debug("unhandled val mode %i\n", t->channel[chn].valMode);
			}

			switch (t->channel[chn].cntMode)
			{
			case 0:
				t->channel[chn].output = 0;
				t->channel[chn].enable = 0;
				break;

			case 3:
				t->channel[chn].output = 1;
				break;

			case 4:
				t->channel[chn].output = 1;
				t->channel[chn].enable = 0;
				break;

			default:
				osd_printf_debug("unhandled cnt mode %i\n", t->channel[chn].cntMode);
			}
		}
		break;

	default:
		chn = reg;

		switch (t->channel[chn].valMode)
		{
		case 1:
			t->channel[chn].cnval = (t->channel[chn].cnval & 0xff00) | val;
			break;
		case 2:
			t->channel[chn].cnval = (t->channel[chn].cnval & 0x00ff) | (val << 8);
			break;
		case 3:
			t->channel[chn].cnval = (t->channel[chn].cnval >> 8) | (val << 8);
			break;
		default:
			osd_printf_debug("unhandled val mode %i\n", t->channel[chn].valMode);
		}

		if (t->channel[chn].cntMode==0)
		{
			t->channel[chn].enable = 0;
		}

		t->channel[chn].loadCnt--;

		if (t->channel[chn].loadCnt == 0)
		{
			switch (t->channel[chn].valMode)
			{
			case 1:
			case 2:
				t->channel[chn].loadCnt = 1;
				break;

			case 3:
				t->channel[chn].loadCnt = 2;
				break;

			default:
				osd_printf_debug("unhandled val mode %i\n", t->channel[chn].valMode);
			}

			switch (t->channel[chn].cntMode)
			{
			case 3:
				t->channel[chn].count = t->channel[chn].cnval;
				t->channel[chn].enable = 1;
				break;

			case 0:
			case 4:
				t->channel[chn].count = t->channel[chn].cnval;
				t->channel[chn].enable = 1;
				break;

			default:
				osd_printf_debug("unhandled cnt mode %i\n", t->channel[chn].cntMode);
			}
		}
	}
}

void tiamc1_sound_device::timer8253_set_gate(struct timer8253struct *t, int chn, UINT8 gate)
{
	t->channel[chn].gate = gate;
}



char tiamc1_sound_device::timer8253_get_output(struct timer8253struct *t, int chn)
{
	return t->channel[chn].output;
}



WRITE8_MEMBER( tiamc1_sound_device::tiamc1_timer0_w )
{
	timer8253_wr(&m_timer0, offset, data);
}

WRITE8_MEMBER( tiamc1_sound_device::tiamc1_timer1_w )
{
	timer8253_wr(&m_timer1, offset, data);
}

WRITE8_MEMBER( tiamc1_sound_device::tiamc1_timer1_gate_w )
{
	timer8253_set_gate(&m_timer1, 0, (data & 1) ? 1 : 0);
	timer8253_set_gate(&m_timer1, 1, (data & 2) ? 1 : 0);
	timer8253_set_gate(&m_timer1, 2, (data & 4) ? 1 : 0);
}
