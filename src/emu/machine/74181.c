/*
 * 74181
 *
 * 4-bit arithmetic Logic Unit
 *
 */

#include "driver.h"
#include "74181.h"



#define TTL74181_MAX_CHIPS 		(2)
#define TTL74181_INPUT_TOTAL	(14)
#define TTL74181_OUTPUT_TOTAL	(8)



typedef struct _TTL74181_state TTL74181_state;
struct _TTL74181_state
{
	UINT8 inputs[TTL74181_INPUT_TOTAL];
	UINT8 outputs[TTL74181_OUTPUT_TOTAL];
	UINT8 dirty;
};

static TTL74181_state chips[TTL74181_MAX_CHIPS];


void TTL74181_config(int which, void *intf)
{
	TTL74181_state *c;

	assert_always(mame_get_phase(Machine) == MAME_PHASE_INIT, "Can only call at init time!");
	assert_always(intf == 0, "Interface must be NULL");
	assert_always((which >= 0) && (which < TTL74181_MAX_CHIPS), "Exceeded maximum number of 74181 chips");

	c = &chips[which];

	c->dirty = 1;

	state_save_register_item_array("TTL74181", which, c->inputs);
	state_save_register_item_array("TTL74181", which, c->outputs);
	state_save_register_item      ("TTL74181", which, c->dirty);
}


void TTL74181_reset(int which)
{
	/* nothing to do */
}


static void TTL74181_update(int which)
{
	TTL74181_state *c = &chips[which];

	UINT8 a0 =  c->inputs[TTL74181_INPUT_A0];
	UINT8 a1 =  c->inputs[TTL74181_INPUT_A1];
	UINT8 a2 =  c->inputs[TTL74181_INPUT_A2];
	UINT8 a3 =  c->inputs[TTL74181_INPUT_A3];

	UINT8 b0 =  c->inputs[TTL74181_INPUT_B0];
	UINT8 b1 =  c->inputs[TTL74181_INPUT_B1];
	UINT8 b2 =  c->inputs[TTL74181_INPUT_B2];
	UINT8 b3 =  c->inputs[TTL74181_INPUT_B3];

	UINT8 s0 =  c->inputs[TTL74181_INPUT_S0];
	UINT8 s1 =  c->inputs[TTL74181_INPUT_S1];
	UINT8 s2 =  c->inputs[TTL74181_INPUT_S2];
	UINT8 s3 =  c->inputs[TTL74181_INPUT_S3];

	UINT8 cp =  c->inputs[TTL74181_INPUT_C];
	UINT8 mp = !c->inputs[TTL74181_INPUT_M];

	UINT8 ap0 = !(a0 | (b0 & s0) | (s1 & !b0));
	UINT8 bp0 = !((!b0 & s2 & a0) | (a0 & b0 & s3));
	UINT8 ap1 = !(a1 | (b1 & s0) | (s1 & !b1));
	UINT8 bp1 = !((!b1 & s2 & a1) | (a1 & b1 & s3));
	UINT8 ap2 = !(a2 | (b2 & s0) | (s1 & !b2));
	UINT8 bp2 = !((!b2 & s2 & a2) | (a2 & b2 & s3));
	UINT8 ap3 = !(a3 | (b3 & s0) | (s1 & !b3));
	UINT8 bp3 = !((!b3 & s2 & a3) | (a3 & b3 & s3));

	UINT8 fp0 = !(cp & mp) ^ (!ap0 & bp0);
	UINT8 fp1 = (!((mp & ap0) | (mp & bp0 & cp))) ^ (!ap1 & bp1);
	UINT8 fp2 = (!((mp & ap1) | (mp & ap0 & bp1) | (mp & cp & bp0 & bp1))) ^ (!ap2 & bp2);
	UINT8 fp3 = (!((mp & ap2) | (mp & ap1 & bp2) | (mp & ap0 & bp1 & bp2) | (mp & cp & bp0 & bp1 & bp2))) ^ (!ap3 & bp3);

	UINT8 aeqb = fp0 & fp1 & fp2 & fp3;
	UINT8 pp = !(bp0 & bp1 & bp2 & bp3);
	UINT8 gp = !((ap0 & bp1 & bp2 & bp3) | (ap1 & bp2 & bp3) | (ap2 & bp3) | ap3);
	UINT8 cn4 = (!(cp & bp0 & bp1 & bp2 & bp3)) | gp;

	c->outputs[TTL74181_OUTPUT_F0]   = fp0;
	c->outputs[TTL74181_OUTPUT_F1]   = fp1;
	c->outputs[TTL74181_OUTPUT_F2]   = fp2;
	c->outputs[TTL74181_OUTPUT_F3]   = fp3;
	c->outputs[TTL74181_OUTPUT_AEQB] = aeqb;
	c->outputs[TTL74181_OUTPUT_P]    = pp;
	c->outputs[TTL74181_OUTPUT_G]    = gp;
	c->outputs[TTL74181_OUTPUT_CN4]  = cn4;
}


void TTL74181_write(int which, int startline, int lines, UINT8 data)
{
	int line;
	TTL74181_state *c;

	assert_always((which >= 0) && (which < TTL74181_MAX_CHIPS), "Chip index out of range");

	c = &chips[which];

	assert_always(c != NULL, "Invalid index - chip has not been configured");
	assert_always(lines >= 1, "Must set at least one line");
	assert_always(lines <= 4, "Can't set more than 4 lines at once");
	assert_always((startline + lines) <= TTL74181_INPUT_TOTAL, "Input line index out of range");

	for (line = 0; line < lines; line++)
	{
		UINT8 input = (data >> line) & 0x01;

		if (c->inputs[startline + line] != input)
		{
			c->inputs[startline + line] = input;

			c->dirty = 1;
		}
	}
}


UINT8 TTL74181_read(int which, int startline, int lines)
{
	int line;
	UINT8 data;
	TTL74181_state *c;

	assert_always((which >= 0) && (which < TTL74181_MAX_CHIPS), "Chip index out of range");

	c = &chips[which];

	assert_always(c != NULL, "Invalid index - chip has not been configured");
	assert_always(lines >= 1, "Must read at least one line");
	assert_always(lines <= 4, "Can't read more than 4 lines at once");
	assert_always((startline + lines) <= TTL74181_OUTPUT_TOTAL, "Output line index out of range");

	if (c->dirty)
	{
		TTL74181_update(which);

		c->dirty = 0;
	}


	data = 0;

	for (line = 0; line < lines; line++)
	{
		data = data | (c->outputs[startline + line] << line);
	}

	return data;
}
