/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *  (c) D.Renaud 2003-2004
 *
 ************************************************************************
 *
 * DSO_OUTPUT            - Output node
 * DSO_TASK              - Task node
 *
 * Task and list routines
 * 
 ************************************************************************/

static DISCRETE_STEP( dso_task )
{
	discrete_task_context *ctx =  (discrete_task_context *) node->context;
	int i;
	
	for (i = 0; i < ctx->numbuffered; i++)
		*(ctx->ptr[i]++) = DISCRETE_INPUT(i);
}

static DISCRETE_RESET( dso_task )
{
	/* nothing to do - just avoid being stepped */
}

static DISCRETE_STEP( dso_output )
{
	stream_sample_t **output = (stream_sample_t **) &node->context;
	double val;

	/* Add gain to the output and put into the buffers */
	/* Clipping will be handled by the main sound system */
	val = DISCRETE_INPUT(0) * DISCRETE_INPUT(1);
	**output = val;
	(*output)++;
}

static DISCRETE_RESET( dso_output )
{
	/* nothing to do - just avoid being stepped */
}

