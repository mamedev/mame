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
	const input_port_config *port;
	INT32		lastpval;
	INT32		pmin;
	double		pscale;
	double		min;
	double		scale;
};

INLINE discrete_info *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_DISCRETE);
	return (discrete_info *)device->token;
}

READ8_DEVICE_HANDLER(discrete_sound_r)
{
	discrete_info    *info = get_safe_token(device);
	node_description *node = discrete_find_node(info, offset);

	UINT8 data = 0;

	/* Read the node input value if allowed */
	if (node)
	{
		UINT8 *node_data = (UINT8 *)node->context;

		/* Bring the system up to now */
		stream_update(info->discrete_stream);

		if ((node->module.type >= DSS_INPUT_DATA) && (node->module.type <= DSS_INPUT_PULSE))
		{
			data = *node_data;
		}
	}
	else
		discrete_log(info, "discrete_sound_r read from non-existent NODE_%02d\n", offset-NODE_00);

    return data;
}

#if 0
static STREAM_BACKGROUND_CALLBACK( disc_cb )
{
	node_description *node = (node_description *) ptr;

	node->output[0] = param * DSS_INPUT__GAIN + DSS_INPUT__OFFSET;
}
#endif

WRITE8_DEVICE_HANDLER(discrete_sound_w)
{
	discrete_info    *info = get_safe_token(device);
	node_description *node = discrete_find_node(info, offset);

	/* Update the node input value if it's a proper input node */
	if (node)
	{
		UINT8 *node_data = (UINT8 *)node->context;
		UINT8 last_data  = *node_data;
		UINT8 new_data    = 0;

		switch (node->module.type)
		{
			case DSS_INPUT_DATA:
				new_data = data;
				break;
			case DSS_INPUT_LOGIC:
			case DSS_INPUT_PULSE:
				new_data = data ? 1 : 0;
				break;
			case DSS_INPUT_NOT:
				new_data = data ? 0 : 1;
				break;
		}

		if (last_data != new_data)
		{
#if 1
			/* Bring the system up to now */
			stream_update(info->discrete_stream);

			*node_data = new_data;

			/* Update the node output here so we don't have to do it each step */
			node->output[0] = *node_data * DSS_INPUT__GAIN + DSS_INPUT__OFFSET;
#else
			*node_data = new_data;
			/* Bring the system up to now */
			stream_update_background(info->discrete_stream, disc_cb, (void *) node, *node_data);

#endif
		}
	}
	else
	{
		discrete_log(info, "discrete_sound_w write to non-existent NODE_%02d\n", offset-NODE_00);
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
#define DSS_ADJUSTMENT__MIN		(*(node->input[0]))
#define DSS_ADJUSTMENT__MAX		(*(node->input[1]))
#define DSS_ADJUSTMENT__LOG		(*(node->input[2]))
#define DSS_ADJUSTMENT__PORT	(*(node->input[3]))
#define DSS_ADJUSTMENT__PMIN	(*(node->input[4]))
#define DSS_ADJUSTMENT__PMAX	(*(node->input[5]))

static DISCRETE_STEP(dss_adjustment)
{
	struct dss_adjustment_context *context = (struct dss_adjustment_context *)node->context;

	INT32  rawportval = input_port_read_direct(context->port);

	/* only recompute if the value changed from last time */
	if (rawportval != context->lastpval)
	{
		double portval   = (double)(rawportval - context->pmin) * context->pscale;
		double scaledval = portval * context->scale + context->min;

		context->lastpval = rawportval;
		if (DSS_ADJUSTMENT__LOG == 0)
			node->output[0] = scaledval;
		else
			node->output[0] = pow(10, scaledval);
	}
}

static DISCRETE_RESET(dss_adjustment)
{
	struct dss_adjustment_context *context = (struct dss_adjustment_context *)node->context;

	double min, max;

	if (node->custom)
	{
		context->port = input_port_by_tag(disc_info->device->machine->portconfig, (const char *)node->custom);
		if (context->port == NULL)
			fatalerror("DISCRETE_ADJUSTMENT_TAG - NODE_%d has invalid tag", node->node-NODE_00);
	}
	else
		context->port = input_port_by_index(disc_info->device->machine->portconfig, DSS_ADJUSTMENT__PORT);

	context->lastpval = 0x7fffffff;
	context->pmin     = DSS_ADJUSTMENT__PMIN;
	context->pscale   = 1.0 / (double)(DSS_ADJUSTMENT__PMAX - DSS_ADJUSTMENT__PMIN);

	/* linear scale */
	if (DSS_ADJUSTMENT__LOG == 0)
	{
		context->min   = DSS_ADJUSTMENT__MIN;
		context->scale = DSS_ADJUSTMENT__MAX - DSS_ADJUSTMENT__MIN;
	}

	/* logarithmic scale */
	else
	{
		/* force minimum and maximum to be > 0 */
		min = (DSS_ADJUSTMENT__MIN > 0) ? DSS_ADJUSTMENT__MIN : 1;
		max = (DSS_ADJUSTMENT__MAX > 0) ? DSS_ADJUSTMENT__MAX : 1;
		context->min   = log10(min);
		context->scale = log10(max) - log10(min);
	}

	DISCRETE_STEP_CALL(dss_adjustment);
}


/************************************************************************
 *
 * DSS_CONSTANT - This is a constant.
 *
 * input[0]    - Constant value
 *
 ************************************************************************/
#define DSS_CONSTANT__INIT	(*(node->input[0]))

static DISCRETE_RESET(dss_constant)
{
	node->output[0]= DSS_CONSTANT__INIT;
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
static DISCRETE_RESET(dss_input)
{
	UINT8 *node_data = (UINT8 *)node->context;

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
	node->output[0] = *node_data * DSS_INPUT__GAIN + DSS_INPUT__OFFSET;
}

static DISCRETE_STEP(dss_input_pulse)
{
	UINT8 *node_data = (UINT8 *)node->context;

	/* Set a valid output */
	node->output[0] = *node_data;
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

static DISCRETE_STEP(dss_input_stream)
{
	/* the context pointer is set to point to the current input stream data in discrete_stream_update */
	stream_sample_t **ptr = (stream_sample_t **)node->context;
	stream_sample_t *data = *ptr;

	node->output[0] = data ? (*data) * DSS_INPUT_STREAM__GAIN + DSS_INPUT_STREAM__OFFSET : 0;
}

static DISCRETE_RESET(dss_input_stream)
{
	int istream = DSS_INPUT_STREAM__STREAM;
	/* we will use the node's context pointer to point to the input stream data */
	assert(istream < disc_info->discrete_input_streams);
	node->context = (discrete_info *) &disc_info->input_stream_data[istream];
}
