/***************************************************************************

    National Semiconductor ADC0831 / ADC0832 / ADC0834 / ADC0838

    8-Bit serial I/O A/D Converters with Muliplexer Options


    2009-06 Converted to be a device

***************************************************************************/

#include "driver.h"
#include "adc083x.h"

#define VERBOSE_LEVEL (0)

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ... )
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[ 32768 ];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		logerror("%s: %s", cpuexec_describe_context(machine), buf);
	}
}

/***************************************************************************
    PARAMETERS
***************************************************************************/

enum
{
	STATE_IDLE,
	STATE_WAIT_FOR_START,
	STATE_SHIFT_MUX,
	STATE_MUX_SETTLE,
	STATE_OUTPUT_MSB_FIRST,
	STATE_WAIT_FOR_SE,
	STATE_OUTPUT_LSB_FIRST,
	STATE_FINISHED
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _adc0831_state adc0831_state;
struct _adc0831_state
{
	adc083x_input_convert_func input_callback_r;

	INT32 cs;
	INT32 clk;
	INT32 di;
	INT32 se;
	INT32 sars;
	INT32 _do;
	INT32 sgl;
	INT32 odd;
	INT32 sel1;
	INT32 sel0;
	INT32 state;
	INT32 bit;
	INT32 output;
	INT32 mux_bits;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE adc0831_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert((device->type == ADC0831) || (device->type == ADC0832) || (device->type == ADC0834) || (device->type == ADC0838));
	return (adc0831_state *)device->token;
}

INLINE const adc0831_interface *get_interface(const device_config *device)
{
	assert(device != NULL);
	assert((device->type == ADC0831) || (device->type == ADC0832) || (device->type == ADC0834) || (device->type == ADC0838));
	return (const adc0831_interface *) device->static_config;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    adc083x_cs_write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( adc083x_cs_write )
{
	adc0831_state *adc083x = get_safe_token(device);

	if (adc083x->cs != data)
	{
		verboselog(device->machine, 2, "adc083x_cs_write( %s, %d )\n", device->tag, data);
	}

	if (adc083x->cs == 0 && data != 0)
	{
		adc083x->state = STATE_IDLE;
		if (device->type == ADC0834 || device->type == ADC0838)
		{
			adc083x->sars = 1;
		}
		adc083x->_do = 1;
	}
	if (adc083x->cs != 0 && data == 0)
	{
		if (device->type == ADC0831)
		{
			adc083x->state = STATE_MUX_SETTLE;
		}
		else
		{
			adc083x->state = STATE_WAIT_FOR_START;
		}

		if (device->type == ADC0834 || device->type == ADC0838)
		{
			adc083x->sars = 1;
		}
		adc083x->_do = 1;
	}

	adc083x->cs = data;
}

/*-------------------------------------------------
    adc083x_conversion
-------------------------------------------------*/

static UINT8 adc083x_conversion( const device_config *device )
{
	adc0831_state *adc083x = get_safe_token(device);
	int result;
	int positive_channel = ADC083X_AGND;
	int negative_channel = ADC083X_AGND;
	double positive = 0;
	double negative = 0;
	double vref = adc083x->input_callback_r(device, ADC083X_VREF);
	double gnd = adc083x->input_callback_r(device, ADC083X_VREF);

	if (device->type == ADC0831)
	{
		positive_channel = ADC083X_CH0;
		negative_channel = ADC083X_CH1;
	}

	else if (device->type == ADC0832)
	{
		positive_channel = ADC083X_CH0 + adc083x->odd;
		if (adc083x->sgl == 0)
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_AGND;
		}
	}
	else if (device->type == ADC0834)
	{
		positive_channel = ADC083X_CH0 + adc083x->odd + (adc083x->sel1 * 2);
		if (adc083x->sgl == 0)
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_AGND;
		}
		}
	else if (device->type == ADC0838)
	{
		positive_channel = ADC083X_CH0 + adc083x->odd + (adc083x->sel0 * 2) + (adc083x->sel1 * 4);
		if (adc083x->sgl == 0)
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_COM;
		}
	}

	if (positive_channel != ADC083X_AGND)
	{
		positive = adc083x->input_callback_r(device, positive_channel) - gnd;
	}
	if (negative_channel != ADC083X_AGND)
	{
		negative = adc083x->input_callback_r(device, negative_channel) - gnd;
	}

	result = (int)(((positive - negative) * 255) / vref);
	logerror("%d", result);
	if (result < 0)
	{
		result = 0;
	}
	else if (result > 255)
	{
		result = 255;
	}

	return result;
}

/*-------------------------------------------------
    adc083x_clk_write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( adc083x_clk_write )
{
	adc0831_state *adc083x = get_safe_token(device);

	if (adc083x->clk != data)
	{
		verboselog(device->machine, 2, "adc083x_clk_write( %s, %d )\n", device->tag, data);
	}

	if (adc083x->cs == 0)
	{
		if (adc083x->clk == 0 && data != 0)
		{
			switch (adc083x->state)
			{
			case STATE_WAIT_FOR_START:
				if (adc083x->di != 0)
				{
					verboselog(device->machine, 1, "adc083x %s got start bit\n", device->tag);
					adc083x->state = STATE_SHIFT_MUX;
					adc083x->sars = 0;
					adc083x->sgl = 0;
					adc083x->odd = 0;
					adc083x->sel1 = 0;
					adc083x->sel0 = 0;
					adc083x->bit = 0;
				}
				else
				{
					verboselog(device->machine, 1, "adc083x %s not start bit\n", device->tag);
				}
				break;
			case STATE_SHIFT_MUX:
				switch (adc083x->bit)
				{
				case 0:
					if (adc083x->di != 0)
					{
						adc083x->sgl = 1;
					}
					verboselog(device->machine, 1, "adc083x %s sgl <- %d\n", device->tag, adc083x->sgl);
					break;
				case 1:
					if (adc083x->di != 0)
					{
						adc083x->odd = 1;
					}
					verboselog(device->machine, 1, "adc083x %s odd <- %d\n", device->tag, adc083x->odd);
					break;
				case 2:
					if (adc083x->di != 0)
					{
						adc083x->sel1 = 1;
					}
					verboselog(device->machine, 1, "adc083x %s sel1 <- %d\n", device->tag, adc083x->sel1);
					break;
				case 3:
					if (adc083x->di != 0)
					{
						adc083x->sel0 = 1;
					}
					verboselog(device->machine, 1, "adc083x %s sel0 <- %d\n", device->tag, adc083x->sel0);
					break;
				}
				adc083x->bit++;
				if (adc083x->bit == adc083x->mux_bits)
				{
					adc083x->state = STATE_MUX_SETTLE;
				}
				break;
			case STATE_WAIT_FOR_SE:
				adc083x->sars = 0;
				if (device->type == ADC0838 && adc083x->se != 0)
				{
					verboselog(device->machine, 1, "adc083x %s not se\n", device->tag);
				}
				else
				{
					verboselog(device->machine, 1, "adc083x %s got se\n", device->tag);
					adc083x->state = STATE_OUTPUT_LSB_FIRST;
					adc083x->bit = 1;
				}
				break;
			}
		}
		if (adc083x->clk != 0 && data == 0)
		{
			switch (adc083x->state)
			{
			case STATE_MUX_SETTLE:
				verboselog(device->machine, 1, "adc083x %s mux settle\n", device->tag);
				adc083x->output = adc083x_conversion(device);
				adc083x->state = STATE_OUTPUT_MSB_FIRST;
				adc083x->bit = 7;
				if (device->type == ADC0834 || device->type == ADC0838)
				{
					adc083x->sars = 1;
				}
				adc083x->_do = 0;
				break;
			case STATE_OUTPUT_MSB_FIRST:
				adc083x->_do = (adc083x->output >> adc083x->bit) & 1;
				verboselog(device->machine, 1, "adc083x %s msb %d -> %d\n", device->tag, adc083x->bit, adc083x->_do);
				adc083x->bit--;
				if (adc083x->bit < 0)
				{
					if (device->type == ADC0831)
					{
						adc083x->state = STATE_FINISHED;
					}
					else
					{
						adc083x->state = STATE_WAIT_FOR_SE;
					}
				}
				break;
			case STATE_OUTPUT_LSB_FIRST:
				adc083x->_do = (adc083x->output >> adc083x->bit) & 1;
				verboselog(device->machine, 1, "adc083x %s lsb %d -> %d\n", device->tag, adc083x->bit, adc083x->_do);
				adc083x->bit++;
				if (adc083x->bit == 8)
				{
					adc083x->state = STATE_FINISHED;
				}
				break;
			case STATE_FINISHED:
				adc083x->state = STATE_IDLE;
				adc083x->_do = 0;
				break;
			}
		}
	}

	adc083x->clk = data;
}

/*-------------------------------------------------
    adc083x_di_write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( adc083x_di_write )
{
	adc0831_state *adc083x = get_safe_token(device);

	if (adc083x->di != data)
	{
		verboselog(device->machine, 2, "adc083x_di_write( %s, %d )\n", device->tag, data);
	}

	adc083x->di = data;
}

/*-------------------------------------------------
    adc083x_se_write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( adc083x_se_write )
{
	adc0831_state *adc083x = get_safe_token(device);

	if (adc083x->se != data)
	{
		verboselog(device->machine, 2, "adc083x_se_write( %s, %d )\n", device->tag, data);
	}

	adc083x->se = data;
}

/*-------------------------------------------------
    adc083x_sars_read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( adc083x_sars_read )
{
	adc0831_state *adc083x = get_safe_token(device);

	verboselog(device->machine, 1, "adc083x_sars_read( %s ) %d\n", device->tag, adc083x->sars);
	return adc083x->sars;
}

/*-------------------------------------------------
    adc083x_do_read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( adc083x_do_read )
{
	adc0831_state *adc083x = get_safe_token(device);

	verboselog(device->machine, 1, "adc083x_sars_read( %s ) %d\n", device->tag, adc083x->_do);
	return adc083x->_do;
}


/*-------------------------------------------------
    DEVICE_START( adc083x )
-------------------------------------------------*/

static DEVICE_START( adc0831 )
{
	adc0831_state *adc083x = get_safe_token(device);
	const adc0831_interface *intf = get_interface(device);

	if (device->type == ADC0831)
	{
		adc083x->sars = 1;
		adc083x->mux_bits = 0;
	}
	else if (device->type == ADC0832)
	{
			adc083x->sars = 1;
			adc083x->mux_bits = 2;
	}
	else if (device->type == ADC0834)
	{
			adc083x->sars = 0;
			adc083x->mux_bits = 3;
	}
	else if (device->type == ADC0838)
	{
			adc083x->sars = 0;
			adc083x->mux_bits = 4;
	}

	/* resolve callbacks */
	adc083x->input_callback_r = intf->input_callback_r;

	/* register for state saving */
	state_save_register_device_item(device, 0, adc083x->cs);
	state_save_register_device_item(device, 0, adc083x->clk);
	state_save_register_device_item(device, 0, adc083x->di);
	state_save_register_device_item(device, 0, adc083x->se);
	state_save_register_device_item(device, 0, adc083x->sars);
	state_save_register_device_item(device, 0, adc083x->_do);
	state_save_register_device_item(device, 0, adc083x->sgl);
	state_save_register_device_item(device, 0, adc083x->odd);
	state_save_register_device_item(device, 0, adc083x->sel1);
	state_save_register_device_item(device, 0, adc083x->sel0);
	state_save_register_device_item(device, 0, adc083x->state);
	state_save_register_device_item(device, 0, adc083x->bit);
	state_save_register_device_item(device, 0, adc083x->output);
	state_save_register_device_item(device, 0, adc083x->mux_bits);
}


/*-------------------------------------------------
    DEVICE_RESET( adc083x )
-------------------------------------------------*/

static DEVICE_RESET( adc0831 )
{
	adc0831_state *adc083x = get_safe_token(device);

	adc083x->cs = 0;
	adc083x->clk = 0;
	adc083x->di = 0;
	adc083x->se = 0;
	adc083x->_do = 1;
	adc083x->sgl = 0;
	adc083x->odd = 0;
	adc083x->sel1 = 0;
	adc083x->sel0 = 0;
	adc083x->state = STATE_IDLE;
	adc083x->bit = 0;
	adc083x->output = 0;
}

/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##adc0831##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"A/D Converters 0831"
#define DEVTEMPLATE_FAMILY		"National Semiconductor A/D Converters 083x"
#define DEVTEMPLATE_CLASS		DEVICE_CLASS_PERIPHERAL
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##adc0832##s
#define DEVTEMPLATE_DERIVED_FEATURES	0
#define DEVTEMPLATE_DERIVED_NAME		"A/D Converters 0832"
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##adc0834##s
#define DEVTEMPLATE_DERIVED_FEATURES	0
#define DEVTEMPLATE_DERIVED_NAME		"A/D Converters 0834"
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##adc0838##s
#define DEVTEMPLATE_DERIVED_FEATURES	0
#define DEVTEMPLATE_DERIVED_NAME		"A/D Converters 0838"
#include "devtempl.h"
