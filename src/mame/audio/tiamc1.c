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
#include "streams.h"
#include "includes/tiamc1.h"

#define CLOCK_DIVIDER 16
#define BUF_LEN 100000

static sound_stream *channel;
static int timer1_divider;

struct timer8253chan {
	UINT16 count;
	UINT16 cnval;
	UINT8 bcdMode;
	UINT8 cntMode;
	UINT8 valMode;
	UINT8 gate;
	UINT8 output;
	UINT8 loadCnt;
	UINT8 enable;
};

struct timer8253struct {
	struct timer8253chan channel[3];
};

static struct timer8253struct timer0;
static struct timer8253struct timer1;


#define T8253_CHAN0		0
#define T8253_CHAN1		1
#define T8253_CHAN2		2
#define T8253_CWORD		3


static void timer8253_reset(struct timer8253struct *t) {
	memset(t,0,sizeof(struct timer8253struct));
}


static void timer8253_tick(struct timer8253struct *t,int chn) {
	if (t->channel[chn].enable && t->channel[chn].gate) {
		switch (t->channel[chn].cntMode) {
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

			if(t->channel[chn].count == 0xffff) {
				t->channel[chn].enable = 0;
				t->channel[chn].output = 1;
			}
			break;
		}
	}
}



static void timer8253_wr(struct timer8253struct *t, int reg, UINT8 val)
{
	int chn;

	switch (reg) {
	case T8253_CWORD:
		chn = val >> 6;
		if (chn < 3) {
			t->channel[chn].bcdMode = (val & 1) ? 1 : 0;
			t->channel[chn].cntMode = (val >> 1) & 0x07;
			t->channel[chn].valMode = (val >> 4) & 0x03;

			switch (t->channel[chn].valMode) {
			case 1:
			case 2:
				t->channel[chn].loadCnt = 1;
				break;

			case 3:
				t->channel[chn].loadCnt = 2;
				break;

			default:
				mame_printf_debug("unhandled val mode %i\n", t->channel[chn].valMode);
			}

			switch (t->channel[chn].cntMode) {
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
				mame_printf_debug("unhandled cnt mode %i\n", t->channel[chn].cntMode);
			}
		}
		break;

	default:
		chn = reg;

		switch (t->channel[chn].valMode) {
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
			mame_printf_debug("unhandled val mode %i\n", t->channel[chn].valMode);
		}

		if (t->channel[chn].cntMode==0) {
			t->channel[chn].enable = 0;
		}

		t->channel[chn].loadCnt--;

		if (t->channel[chn].loadCnt == 0) {
			switch (t->channel[chn].valMode) {
			case 1:
			case 2:
				t->channel[chn].loadCnt = 1;
				break;

			case 3:
				t->channel[chn].loadCnt = 2;
				break;

			default:
				mame_printf_debug("unhandled val mode %i\n", t->channel[chn].valMode);
			}

			switch (t->channel[chn].cntMode) {
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
				mame_printf_debug("unhandled cnt mode %i\n", t->channel[chn].cntMode);
			}
		}
	}
}

static void timer8253_set_gate(struct timer8253struct *t, int chn, UINT8 gate)
{
	t->channel[chn].gate = gate;
}



static char timer8253_get_output(struct timer8253struct *t, int chn)
{
	return t->channel[chn].output;
}


WRITE8_HANDLER( tiamc1_timer0_w )
{
	timer8253_wr(&timer0, offset, data);
}

WRITE8_HANDLER( tiamc1_timer1_w )
{
	timer8253_wr(&timer1, offset, data);
}

WRITE8_HANDLER( tiamc1_timer1_gate_w )
{
	timer8253_set_gate(&timer1, 0, (data & 1) ? 1 : 0);
	timer8253_set_gate(&timer1, 1, (data & 2) ? 1 : 0);
	timer8253_set_gate(&timer1, 2, (data & 4) ? 1 : 0);
}


static STREAM_UPDATE( tiamc1_sound_update )
{
	int count, o0, o1, o2, len, orval = 0;

	len = samples * CLOCK_DIVIDER;

	for (count = 0; count < len; count++) {
		timer1_divider++;
		if (timer1_divider == 228) {
			timer1_divider = 0;
			timer8253_tick(&timer1, 0);
			timer8253_tick(&timer1, 1);
			timer8253_tick(&timer1, 2);

			timer8253_set_gate(&timer0, 0, timer8253_get_output(&timer1, 0));
			timer8253_set_gate(&timer0, 1, timer8253_get_output(&timer1, 1));
			timer8253_set_gate(&timer0, 2, timer8253_get_output(&timer1, 2));
		}

		timer8253_tick(&timer0, 0);
		timer8253_tick(&timer0, 1);
		timer8253_tick(&timer0, 2);

		o0 = timer8253_get_output(&timer0, 0) ? 1 : 0;
		o1 = timer8253_get_output(&timer0, 1) ? 1 : 0;
		o2 = timer8253_get_output(&timer0, 2) ? 1 : 0;

		orval = (orval << 1) | (((o0 | o1) ^ 0xff) & o2);

		if ((count + 1) % CLOCK_DIVIDER == 0) {
			outputs[0][count / CLOCK_DIVIDER] = orval ? 0x2828 : 0;
			orval = 0;
		}
	}
}

static DEVICE_START( tiamc1_sound )
{
	running_machine *machine = device->machine;
	int i, j;

	timer8253_reset(&timer0);
	timer8253_reset(&timer1);

	channel = stream_create(device, 0, 1, device->clock / CLOCK_DIVIDER, 0, tiamc1_sound_update);

	timer1_divider = 0;

	for (i = 0; i < 2; i++) {
		struct timer8253struct *t = (i ? &timer1 : &timer0);

		for (j = 0; j < 3; j++) {
			state_save_register_item(machine, "channel", NULL, i * 3 + j, t->channel[j].count);
			state_save_register_item(machine, "channel", NULL, i * 3 + j, t->channel[j].cnval);
			state_save_register_item(machine, "channel", NULL, i * 3 + j, t->channel[j].bcdMode);
			state_save_register_item(machine, "channel", NULL, i * 3 + j, t->channel[j].cntMode);
			state_save_register_item(machine, "channel", NULL, i * 3 + j, t->channel[j].valMode);
			state_save_register_item(machine, "channel", NULL, i * 3 + j, t->channel[j].gate);
			state_save_register_item(machine, "channel", NULL, i * 3 + j, t->channel[j].output);
			state_save_register_item(machine, "channel", NULL, i * 3 + j, t->channel[j].loadCnt);
			state_save_register_item(machine, "channel", NULL, i * 3 + j, t->channel[j].enable);
		}
	}

	state_save_register_global(machine, timer1_divider);
}


DEVICE_GET_INFO( tiamc1_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(tiamc1_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "TIA-MC1 Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


