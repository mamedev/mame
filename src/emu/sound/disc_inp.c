/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 ***********************************************************************
 *
 * DSS_ADJUSTMENT        - UI Mapped adjustable input
 * DSS_CONSTANT          - Node based constant - Do we need this ???
 * DSS_INPUT_x           - Input devices
 * DSS_INPUT_STREAM      - Connects external streams to the discrete system
 *
 ************************************************************************/

#define DSS_INPUT__GAIN		(*(node->input[0]))
#define DSS_INPUT__OFFSET	(*(node->input[1]))
#define DSS_INPUT__INIT		(*(node->input[2]))


struct dss_adjustment_context
{
	INT32		port;
	INT32		lastpval;
	INT32		pmin;
	double		pscale;
	double		min;
	double		scale;
};


READ8_HANDLER(discrete_sound_r)
{
	discrete_info *info = sndti_token(SOUND_DISCRETE, 0);
	node_description *node = discrete_find_node(info, offset);
	UINT8 data = 0;

	/* Read the node input value if allowed */
	if (node)
	{
		UINT8 *node_data = node->context;

		if ((node->module.type >= DSS_INPUT_DATA) && (node->module.type <= DSS_INPUT_PULSE))
		{
			data = *node_data;
		}
	}
	else
		discrete_log("discrete_sound_r read from non-existent NODE_%02d\n", offset-NODE_00);

    return data;
}

WRITE8_HANDLER(discrete_sound_w)
{
	discrete_info *info = sndti_token(SOUND_DISCRETE, 0);
	node_description *node = discrete_find_node(info, offset);

	/* Update the node input value if it's a proper input node */
	if (node)
	{
		UINT8 *node_data = node->context;
		UINT8 last_data = *node_data;

		switch (node->module.type)
		{
			case DSS_INPUT_DATA:
				*node_data = data;
				break;
			case DSS_INPUT_LOGIC:
			case DSS_INPUT_PULSE:
				*node_data = data ? 1 : 0;
				break;
			case DSS_INPUT_NOT:
				*node_data = data ? 0 : 1;
				break;
		}

		/* Bring the system up to now */
		if (last_data != *node_data)
			stream_update(info->discrete_stream);
	}
	else
	{
		discrete_log("discrete_sound_w write to non-existent NODE_%02d\n", offset-NODE_00);
	}
}


/************************************************************************
 *
 * DSS_ADJUSTMENT - UI Adjustable constant node to emulate trimmers
 *
 * input[0]    - Enable
 * input[1]    - Minimum value
 * input[2]    - Maximum value
 * input[3]    - Log/Linear 0=Linear !0=Log
 * input[4]    - Input Port number
 * input[5]    -
 * input[6]    -
 *
 ************************************************************************/
#define DSS_ADJUSTMENT__ENABLE	(*(node->input[0]))
#define DSS_ADJUSTMENT__MIN		(*(node->input[1]))
#define DSS_ADJUSTMENT__MAX		(*(node->input[2]))
#define DSS_ADJUSTMENT__LOG		(*(node->input[3]))
#define DSS_ADJUSTMENT__PORT	(*(node->input[4]))
#define DSS_ADJUSTMENT__PMIN	(*(node->input[5]))
#define DSS_ADJUSTMENT__PMAX	(*(node->input[6]))

void dss_adjustment_step(node_description *node)
{
	if (DSS_ADJUSTMENT__ENABLE)
	{
		struct dss_adjustment_context *context = node->context;
		INT32 rawportval = readinputport(context->port);

		/* only recompute if the value changed from last time */
		if (rawportval != context->lastpval)
		{
			double portval = (double)(rawportval - context->pmin) * context->pscale;
			double scaledval = portval * context->scale + context->min;

			context->lastpval = rawportval;
			if (DSS_ADJUSTMENT__LOG == 0)
				node->output = scaledval;
			else
				node->output = pow(10, scaledval);
		}
	}
	else
	{
		node->output = 0;
	}
}

void dss_adjustment_reset(node_description *node)
{
	struct dss_adjustment_context *context = node->context;
	double min, max;

	if (node->custom)
	{
		context->port = port_tag_to_index(node->custom);
		if (context->port == -1)
			fatalerror("DISCRETE_ADJUSTMENT_TAG - NODE_%d has invalid tag", node->node-NODE_00);
	}
	else
		context->port = DSS_ADJUSTMENT__PORT;

	context->lastpval = 0x7fffffff;
	context->pmin = DSS_ADJUSTMENT__PMIN;
	context->pscale = 1.0 / (double)(DSS_ADJUSTMENT__PMAX - DSS_ADJUSTMENT__PMIN);

	/* linear scale */
	if (DSS_ADJUSTMENT__LOG == 0)
	{
		context->min = DSS_ADJUSTMENT__MIN;
		context->scale = DSS_ADJUSTMENT__MAX - DSS_ADJUSTMENT__MIN;
	}

	/* logarithmic scale */
	else
	{
		/* force minimum and maximum to be > 0 */
		min = (DSS_ADJUSTMENT__MIN > 0) ? DSS_ADJUSTMENT__MIN : 1;
		max = (DSS_ADJUSTMENT__MAX > 0) ? DSS_ADJUSTMENT__MAX : 1;
		context->min = log10(min);
		context->scale = log10(max) - log10(min);
	}

	dss_adjustment_step(node);
}


/************************************************************************
 *
 * DSS_CONSTANT - This is a constant.
 *
 * input[0]    - Constant value
 *
 ************************************************************************/
#define DSS_CONSTANT__INIT	(*(node->input[0]))

void dss_constant_step(node_description *node)
{
	node->output= DSS_CONSTANT__INIT;
}


/************************************************************************
 *
 * DSS_INPUT_x    - Receives input from discrete_sound_w
 *
 * input[0]    - Gain value
 * input[1]    - Offset value
 * input[2]    - Starting Position
 * input[3]    - Current data value
 *
 ************************************************************************/
void dss_input_step(node_description *node)
{
	UINT8 *node_data = node->context;

	node->output = *node_data * DSS_INPUT__GAIN + DSS_INPUT__OFFSET;
}

void dss_input_reset(node_description *node)
{
	UINT8 *node_data = node->context;

	switch (node->module.type)
	{
		case DSS_INPUT_DATA:
			*node_data = DSS_INPUT__INIT;
			break;
		case DSS_INPUT_LOGIC:
		case DSS_INPUT_PULSE:
			*node_data = (DSS_INPUT__INIT == 0) ? 0 : 1;
			break;
		case DSS_INPUT_NOT:
			*node_data = (DSS_INPUT__INIT == 0) ? 1 : 0;
			break;
	}
	dss_input_step(node);
}

void dss_input_pulse_step(node_description *node)
{
	UINT8 *node_data = node->context;

	/* Set a valid output */
	node->output = *node_data;
	/* Reset the input to default for the next cycle */
	/* node order is now important */
	*node_data = DSS_INPUT__INIT;
}


/************************************************************************
 *
 * DSS_INPUT_STREAM    - Receives input from a routed stream
 *
 * input[0]    - Input stream number
 * input[1]    - Gain value
 * input[2]    - Offset value
 *
 ************************************************************************/
#define DSS_INPUT_STREAM__STREAM	(*(node->input[0]))
#define DSS_INPUT_STREAM__GAIN		(*(node->input[1]))
#define DSS_INPUT_STREAM__OFFSET	(*(node->input[2]))

void dss_input_stream_step(node_description *node)
{
	// the context pointer is set to point to the current input stream data in discrete_stream_update
	stream_sample_t *data = node->context;

	node->output = data ? *data++ * DSS_INPUT_STREAM__GAIN + DSS_INPUT_STREAM__OFFSET : 0;
}
