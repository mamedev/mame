/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 *  Coding started in November 2000
 *  KW - Added Sawtooth waveforms  Feb2003
 *
 ***********************************************************************
 *
 * SEE DISCRETE.H for documentation on usage
 *
 ***********************************************************************
 *
 * Each sound primative DSS_xxxx or DST_xxxx has its own implementation
 * file. All discrete sound primatives MUST implement the following
 * API:
 *
 * dsX_NAME_step(inputs, context,float timestep)  - Perform time step
 *                                                  return output value
 * dsX_NAME_reset(context) - Reset to initial state
 *
 * Core software takes care of traversing the netlist in the correct
 * order
 *
 * DEVICE_START(discrete)      - Read Node list, initialise & reset
 * DEVICE_STOP(discrete)       - Shutdown discrete sound system
 * DEVICE_RESET(discrete)      - Put sound system back to time 0
 * discrete_stream_update() - This does the real update to the sim
 *
 ************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "inptport.h"
#include "wavwrite.h"
#include "discrete.h"


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define DISCRETE_DEBUGLOG			(0)


/*************************************
 *
 *  Profiling Nodes
 *
 *************************************/

#define DISCRETE_PROFILING			(0)



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void init_nodes(discrete_info *info, discrete_sound_block *block_list, const device_config *device);
static void find_input_nodes(discrete_info *info, discrete_sound_block *block_list);
static void setup_output_nodes(const device_config *device, discrete_info *info);
static void setup_disc_logs(discrete_info *info);
static node_description *discrete_find_node(const discrete_info *info, int node);
static DEVICE_RESET( discrete );



/*************************************
 *
 *  Debug logging
 *
 *************************************/

static void CLIB_DECL ATTR_PRINTF(2,3) discrete_log(const discrete_info *disc_info, const char *text, ...)
{
	if (DISCRETE_DEBUGLOG)
	{
		va_list arg;
		va_start(arg, text);

		if(disc_info->disclogfile)
		{
			vfprintf(disc_info->disclogfile, text, arg);
			fprintf(disc_info->disclogfile, "\n");
		}

		va_end(arg);
	}
}



/*************************************
 *
 *  Included simulation objects
 *
 *************************************/

#include "disc_wav.c"		/* Wave sources   - SINE/SQUARE/NOISE/etc */
#include "disc_mth.c"		/* Math Devices   - ADD/GAIN/etc */
#include "disc_inp.c"		/* Input Devices  - INPUT/CONST/etc */
#include "disc_flt.c"		/* Filter Devices - RCF/HPF/LPF */
#include "disc_dev.c"		/* Popular Devices - NE555/etc */



/*************************************
 *
 *  Master module list
 *
 *************************************/

static const discrete_module module_list[] =
{
	{ DSO_OUTPUT      ,"DSO_OUTPUT"      , 0 ,0                                      ,NULL                  ,NULL                 },
	{ DSO_CSVLOG      ,"DSO_CSVLOG"      , 0 ,0                                      ,NULL                  ,NULL                 },
	{ DSO_WAVELOG     ,"DSO_WAVELOG"     , 0 ,0                                      ,NULL                  ,NULL                 },
	{ DSO_IMPORT      ,"DSO_IMPORT"      , 0 ,0                                      ,NULL                  ,NULL                 },

	/* nop */
	{ DSS_NOP         ,"DSS_NOP"         , 0 ,0                                      ,NULL                  ,NULL                },

	/* from disc_inp.c */
	{ DSS_ADJUSTMENT  ,"DSS_ADJUSTMENT"  , 1 ,sizeof(struct dss_adjustment_context)  ,dss_adjustment_reset  ,dss_adjustment_step  },
	{ DSS_CONSTANT    ,"DSS_CONSTANT"    , 1 ,0                                      ,dss_constant_reset    ,NULL                 },
	{ DSS_INPUT_DATA  ,"DSS_INPUT_DATA"  , 1 ,sizeof(UINT8)                          ,dss_input_reset       ,NULL                 },
	{ DSS_INPUT_LOGIC ,"DSS_INPUT_LOGIC" , 1 ,sizeof(UINT8)                          ,dss_input_reset       ,NULL                 },
	{ DSS_INPUT_NOT   ,"DSS_INPUT_NOT"   , 1 ,sizeof(UINT8)                          ,dss_input_reset       ,NULL                 },
	{ DSS_INPUT_PULSE ,"DSS_INPUT_PULSE" , 1 ,sizeof(UINT8)                          ,dss_input_reset       ,dss_input_pulse_step },
	{ DSS_INPUT_STREAM,"DSS_INPUT_STREAM", 1 ,0                                      ,dss_input_stream_reset,dss_input_stream_step},

	/* from disc_wav.c */
	/* Generic modules */
	{ DSS_COUNTER     ,"DSS_COUNTER"     , 1 ,sizeof(struct dss_counter_context)     ,dss_counter_reset     ,dss_counter_step     },
	{ DSS_LFSR_NOISE  ,"DSS_LFSR_NOISE"  , 1 ,sizeof(struct dss_lfsr_context)        ,dss_lfsr_reset        ,dss_lfsr_step        },
	{ DSS_NOISE       ,"DSS_NOISE"       , 1 ,sizeof(struct dss_noise_context)       ,dss_noise_reset       ,dss_noise_step       },
	{ DSS_NOTE        ,"DSS_NOTE"        , 1 ,sizeof(struct dss_note_context)        ,dss_note_reset        ,dss_note_step        },
	{ DSS_SAWTOOTHWAVE,"DSS_SAWTOOTHWAVE", 1 ,sizeof(struct dss_sawtoothwave_context),dss_sawtoothwave_reset,dss_sawtoothwave_step},
	{ DSS_SINEWAVE    ,"DSS_SINEWAVE"    , 1 ,sizeof(struct dss_sinewave_context)    ,dss_sinewave_reset    ,dss_sinewave_step    },
	{ DSS_SQUAREWAVE  ,"DSS_SQUAREWAVE"  , 1 ,sizeof(struct dss_squarewave_context)  ,dss_squarewave_reset  ,dss_squarewave_step  },
	{ DSS_SQUAREWFIX  ,"DSS_SQUAREWFIX"  , 1 ,sizeof(struct dss_squarewfix_context)  ,dss_squarewfix_reset  ,dss_squarewfix_step  },
	{ DSS_SQUAREWAVE2 ,"DSS_SQUAREWAVE2" , 1 ,sizeof(struct dss_squarewave_context)  ,dss_squarewave2_reset ,dss_squarewave2_step },
	{ DSS_TRIANGLEWAVE,"DSS_TRIANGLEWAVE", 1 ,sizeof(struct dss_trianglewave_context),dss_trianglewave_reset,dss_trianglewave_step},
	/* Component specific modules */
	{ DSS_INVERTER_OSC ,"DSS_INVERTER_OSC" , 1 ,sizeof(struct dss_inverter_osc_context) ,dss_inverter_osc_reset ,dss_inverter_osc_step },
	{ DSS_OP_AMP_OSC  ,"DSS_OP_AMP_OSC"  , 1 ,sizeof(struct dss_op_amp_osc_context)  ,dss_op_amp_osc_reset  ,dss_op_amp_osc_step  },
	{ DSS_SCHMITT_OSC ,"DSS_SCHMITT_OSC" , 1 ,sizeof(struct dss_schmitt_osc_context) ,dss_schmitt_osc_reset ,dss_schmitt_osc_step },
	/* Not yet implemented */
	{ DSS_ADSR        ,"DSS_ADSR"        , 1 ,sizeof(struct dss_adsr_context)        ,dss_adsrenv_reset     ,dss_adsrenv_step     },

	/* from disc_mth.c */
	/* Generic modules */
	{ DST_ADDER       ,"DST_ADDER"       , 1 ,0                                      ,NULL                  ,dst_adder_step       },
	{ DST_CLAMP       ,"DST_CLAMP"       , 1 ,0                                      ,NULL                  ,dst_clamp_step       },
	{ DST_DIVIDE      ,"DST_DIVIDE"      , 1 ,0                                      ,NULL                  ,dst_divide_step      },
	{ DST_GAIN        ,"DST_GAIN"        , 1 ,0                                      ,NULL                  ,dst_gain_step        },
	{ DST_LOGIC_INV   ,"DST_LOGIC_INV"   , 1 ,0                                      ,NULL                  ,dst_logic_inv_step   },
	{ DST_GAIN        ,"DST_GAIN"        , 1 ,0                                      ,NULL                  ,dst_gain_step        },
	{ DST_BITS_DECODE ,"DST_BITS_DECODE" , 8 ,sizeof(struct dst_bits_decode_context) ,dst_bits_decode_reset ,dst_bits_decode_step },
	{ DST_LOGIC_AND   ,"DST_LOGIC_AND"   , 1 ,0                                      ,NULL                  ,dst_logic_and_step   },
	{ DST_LOGIC_NAND  ,"DST_LOGIC_NAND"  , 1 ,0                                      ,NULL                  ,dst_logic_nand_step  },
	{ DST_LOGIC_OR    ,"DST_LOGIC_OR"    , 1 ,0                                      ,NULL                  ,dst_logic_or_step    },
	{ DST_LOGIC_NOR   ,"DST_LOGIC_NOR"   , 1 ,0                                      ,NULL                  ,dst_logic_nor_step   },
	{ DST_LOGIC_XOR   ,"DST_LOGIC_XOR"   , 1 ,0                                      ,NULL                  ,dst_logic_xor_step   },
	{ DST_LOGIC_NXOR  ,"DST_LOGIC_NXOR"  , 1 ,0                                      ,NULL                  ,dst_logic_nxor_step  },
	{ DST_LOGIC_DFF   ,"DST_LOGIC_DFF"   , 1 ,sizeof(struct dst_flipflop_context)    ,dst_logic_ff_reset    ,dst_logic_dff_step   },
	{ DST_LOGIC_JKFF  ,"DST_LOGIC_JKFF"  , 1 ,sizeof(struct dst_flipflop_context)    ,dst_logic_ff_reset    ,dst_logic_jkff_step  },
	{ DST_LOOKUP_TABLE,"DST_LOOKUP_TABLE", 1 ,0                                      ,NULL                  ,dst_lookup_table_step},
	{ DST_MULTIPLEX   ,"DST_MULTIPLEX"   , 1 ,sizeof(struct dst_size_context)        ,dst_multiplex_reset   ,dst_multiplex_step   },
	{ DST_ONESHOT     ,"DST_ONESHOT"     , 1 ,sizeof(struct dst_oneshot_context)     ,dst_oneshot_reset     ,dst_oneshot_step     },
	{ DST_RAMP        ,"DST_RAMP"        , 1 ,sizeof(struct dss_ramp_context)        ,dst_ramp_reset        ,dst_ramp_step        },
	{ DST_SAMPHOLD    ,"DST_SAMPHOLD"    , 1 ,sizeof(struct dst_samphold_context)    ,dst_samphold_reset    ,dst_samphold_step    },
	{ DST_SWITCH      ,"DST_SWITCH"      , 1 ,0                                      ,NULL                  ,dst_switch_step      },
	{ DST_ASWITCH     ,"DST_ASWITCH"     , 1 ,0                                      ,NULL                  ,dst_aswitch_step     },
	{ DST_TRANSFORM   ,"DST_TRANSFORM"   , 1 ,0                                      ,NULL                  ,dst_transform_step   },
	/* Component specific */
	{ DST_COMP_ADDER  ,"DST_COMP_ADDER"  , 1 ,sizeof(struct dst_comp_adder_context)  ,dst_comp_adder_reset  ,dst_comp_adder_step  },
	{ DST_DAC_R1      ,"DST_DAC_R1"      , 1 ,sizeof(struct dst_dac_r1_context)      ,dst_dac_r1_reset      ,dst_dac_r1_step      },
	{ DST_DIODE_MIX   ,"DST_DIODE_MIX"   , 1 ,sizeof(struct dst_diode_mix__context)  ,dst_diode_mix_reset   ,dst_diode_mix_step   },
	{ DST_INTEGRATE   ,"DST_INTEGRATE"   , 1 ,sizeof(struct dst_integrate_context)   ,dst_integrate_reset   ,dst_integrate_step   },
	{ DST_MIXER       ,"DST_MIXER"       , 1 ,sizeof(struct dst_mixer_context)       ,dst_mixer_reset       ,dst_mixer_step       },
	{ DST_OP_AMP      ,"DST_OP_AMP"      , 1 ,sizeof(struct dst_op_amp_context)      ,dst_op_amp_reset      ,dst_op_amp_step      },
	{ DST_OP_AMP_1SHT ,"DST_OP_AMP_1SHT" , 1 ,sizeof(struct dst_op_amp_1sht_context) ,dst_op_amp_1sht_reset ,dst_op_amp_1sht_step },
	{ DST_TVCA_OP_AMP ,"DST_TVCA_OP_AMP" , 1 ,sizeof(struct dst_tvca_op_amp_context) ,dst_tvca_op_amp_reset ,dst_tvca_op_amp_step },
	{ DST_VCA         ,"DST_VCA"         , 1 ,0                                      ,NULL                  ,NULL                 },

	/* from disc_flt.c */
	/* Generic modules */
	{ DST_FILTER1     ,"DST_FILTER1"     , 1 ,sizeof(struct dss_filter1_context)     ,dst_filter1_reset     ,dst_filter1_step     },
	{ DST_FILTER2     ,"DST_FILTER2"     , 1 ,sizeof(struct dss_filter2_context)     ,dst_filter2_reset     ,dst_filter2_step     },
	/* Component specific modules */
	{ DST_SALLEN_KEY  ,"DST_SALLEN_KEY"  , 1 ,sizeof(struct dss_filter2_context)     ,dst_sallen_key_reset  ,dst_sallen_key_step  },
	{ DST_CRFILTER    ,"DST_CRFILTER"    , 1 ,sizeof(struct dst_rcfilter_context)    ,dst_crfilter_reset    ,dst_crfilter_step    },
	{ DST_OP_AMP_FILT ,"DST_OP_AMP_FILT" , 1 ,sizeof(struct dst_op_amp_filt_context) ,dst_op_amp_filt_reset ,dst_op_amp_filt_step },
	{ DST_RCDISC      ,"DST_RCDISC"      , 1 ,sizeof(struct dst_rcdisc_context)      ,dst_rcdisc_reset      ,dst_rcdisc_step      },
	{ DST_RCDISC2     ,"DST_RCDISC2"     , 1 ,sizeof(struct dst_rcdisc_context)      ,dst_rcdisc2_reset     ,dst_rcdisc2_step     },
	{ DST_RCDISC3     ,"DST_RCDISC3"     , 1 ,sizeof(struct dst_rcdisc_context)      ,dst_rcdisc3_reset     ,dst_rcdisc3_step     },
	{ DST_RCDISC4     ,"DST_RCDISC4"     , 1 ,sizeof(struct dst_rcdisc4_context)     ,dst_rcdisc4_reset     ,dst_rcdisc4_step     },
	{ DST_RCDISC5     ,"DST_RCDISC5"     , 1 ,sizeof(struct dst_rcdisc_context)      ,dst_rcdisc5_reset     ,dst_rcdisc5_step     },
	{ DST_RCINTEGRATE ,"DST_RCINTEGRATE" , 1 ,sizeof(struct dst_rcintegrate_context) ,dst_rcintegrate_reset ,dst_rcintegrate_step },
	{ DST_RCDISC_MOD  ,"DST_RCDISC_MOD"  , 1 ,sizeof(struct dst_rcdisc_mod_context)  ,dst_rcdisc_mod_reset  ,dst_rcdisc_mod_step  },
	{ DST_RCFILTER    ,"DST_RCFILTER"    , 1 ,sizeof(struct dst_rcfilter_context)    ,dst_rcfilter_reset    ,dst_rcfilter_step    },
	{ DST_RCFILTER_SW ,"DST_RCFILTER_SW" , 1 ,sizeof(struct dst_rcfilter_sw_context) ,dst_rcfilter_sw_reset ,dst_rcfilter_sw_step },
	/* For testing - seem to be buggered.  Use versions not ending in N. */
	{ DST_RCFILTERN   ,"DST_RCFILTERN"   , 1 ,sizeof(struct dss_filter1_context)     ,dst_rcfilterN_reset   ,dst_filter1_step     },
	{ DST_RCDISCN     ,"DST_RCDISCN"     , 1 ,sizeof(struct dss_filter1_context)     ,dst_rcdiscN_reset     ,dst_rcdiscN_step     },
	{ DST_RCDISC2N    ,"DST_RCDISC2N"    , 1 ,sizeof(struct dss_rcdisc2_context)     ,dst_rcdisc2N_reset    ,dst_rcdisc2N_step    },

	/* from disc_dev.c */
	/* generic modules */
	{ DST_CUSTOM      ,"DST_CUSTOM"      , 1 ,0                                      ,NULL                  ,NULL                 },
	/* Component specific modules */
	{ DSD_555_ASTBL   ,"DSD_555_ASTBL"   , 1 ,sizeof(struct dsd_555_astbl_context)   ,dsd_555_astbl_reset   ,dsd_555_astbl_step   },
	{ DSD_555_MSTBL   ,"DSD_555_MSTBL"   , 1 ,sizeof(struct dsd_555_mstbl_context)   ,dsd_555_mstbl_reset   ,dsd_555_mstbl_step   },
	{ DSD_555_CC      ,"DSD_555_CC"      , 1 ,sizeof(struct dsd_555_cc_context)      ,dsd_555_cc_reset      ,dsd_555_cc_step      },
	{ DSD_555_VCO1    ,"DSD_555_VCO1"    , 1 ,sizeof(struct dsd_555_vco1_context)    ,dsd_555_vco1_reset    ,dsd_555_vco1_step    },
	{ DSD_566         ,"DSD_566"         , 1 ,sizeof(struct dsd_566_context)         ,dsd_566_reset         ,dsd_566_step         },
	{ DSD_LS624       ,"DSD_LS624"       , 1 ,sizeof(struct dsd_ls624_context)       ,dsd_ls624_reset       ,dsd_ls624_step       },
	/* must be the last one */
	{ DSS_NULL        ,"DSS_NULL"        , 0 ,0                                      ,NULL                  ,NULL                 }
};



/*************************************
 *
 *  Find a given node
 *
 *************************************/

static node_description *discrete_find_node(const discrete_info *info, int node)
{
	if (node < NODE_START || node > NODE_END) return NULL;
	return info->indexed_node[NODE_INDEX(node)];
}

/*************************************
 *
 *  Build import list
 *
 *************************************/

static int discrete_build_list(discrete_info *info, discrete_sound_block *intf, discrete_sound_block *out, int offset)
{
	int node_count = 0;

	for (; intf[node_count].type != DSS_NULL; )
	{
		/* scan imported */
		if (intf[node_count].type == DSO_IMPORT)
		{
			offset = discrete_build_list(info, (discrete_sound_block *) intf[node_count].custom, out, offset);
		}
		else if (intf[node_count].type == DSO_REPLACE)
		{
			int i;

			node_count++;
			if (intf[node_count].type == DSS_NULL)
				fatalerror("discrete_build_list: DISCRETE_REPLACE at end of node_list");

			for (i = 0; i < offset; i++)
				if (out[i].type != NODE_SPECIAL )
					if (out[i].node == intf[node_count].node)
					{
						out[i] = intf[node_count];
						break;
					}

			if (i >= offset)
				fatalerror("discrete_build_list: DISCRETE_REPLACE did not found node %d", NODE_INDEX(intf[node_count].node));

		}
		else if (intf[node_count].type == DSO_DELETE)
		{
			int i,p,deleted;

			p = 0;
			deleted = 0;
			for (i = 0; i < offset; i++)
			{
				if ((out[i].node >= intf[node_count].input_node[0]) &&
						(out[i].node <= intf[node_count].input_node[1]))
				{
					discrete_log(info, "discrete_build_list() - DISCRETE_DELETE deleted NODE_%02d", NODE_INDEX(out[i].node) );
					deleted++;
				}
				else
				{
					out[p++] = out[i];
				}
			}
			offset -= deleted;
		}
		else
			out[offset++] = intf[node_count];

		node_count++;
	}
	out[offset] = intf[node_count];
	return offset;
}

/*************************************
 *
 *  Master discrete system start
 *
 *************************************/

static DEVICE_START( discrete )
{
	discrete_sound_block *intf;
	discrete_sound_block *intf_start = (discrete_sound_block *)device->static_config;
	discrete_info *info = get_safe_token(device);
	char name[32];

	info->device = device;

	/* If a clock is specified we will use it, otherwise run at the audio sample rate. */
	if (device->clock)
		info->sample_rate = device->clock;
	else
		info->sample_rate = device->machine->sample_rate;
	info->sample_time = 1.0 / info->sample_rate;
	info->neg_sample_time = - info->sample_time;

	info->total_samples = 0;

	/* create the logfile */
	sprintf(name, "discrete%s.log", device->tag);
	if (DISCRETE_DEBUGLOG && !info->disclogfile)
		info->disclogfile = fopen(name, "w");

	/* Build the final block list */
	intf = auto_alloc_array_clear(device->machine, discrete_sound_block, DISCRETE_MAX_NODES);
	discrete_build_list(info, intf_start, intf, 0);

	/* first pass through the nodes: sanity check, fill in the indexed_nodes, and make a total count */
	discrete_log(info, "discrete_start() - Doing node list sanity check");
	for (info->node_count = 0; intf[info->node_count].type != DSS_NULL; info->node_count++)
	{
		/* make sure we don't have too many nodes overall */
		if (info->node_count > DISCRETE_MAX_NODES)
			fatalerror("discrete_start() - Upper limit of %d nodes exceeded, have you terminated the interface block?", DISCRETE_MAX_NODES);

		/* make sure the node number is in range */
		if (intf[info->node_count].node < NODE_START || intf[info->node_count].node > NODE_END)
			fatalerror("discrete_start() - Invalid node number on node %02d descriptor", info->node_count);

		/* make sure the node type is valid */
		if (intf[info->node_count].type > DSO_OUTPUT)
			fatalerror("discrete_start() - Invalid function type on NODE_%02d", NODE_INDEX(intf[info->node_count].node) );

		/* make sure this is a main node */
		if (NODE_CHILD_NODE_NUM(intf[info->node_count].node) > 0)
			fatalerror("discrete_start() - Child node number on NODE_%02d", NODE_INDEX(intf[info->node_count].node) );
	}
	info->node_count++;
	discrete_log(info, "discrete_start() - Sanity check counted %d nodes", info->node_count);

	/* allocate memory for the array of actual nodes */
	info->node_list = auto_alloc_array_clear(device->machine, node_description, info->node_count);

	/* allocate memory for the node execution order array */
	info->running_order = auto_alloc_array_clear(device->machine, node_description *, info->node_count);

	/* allocate memory to hold pointers to nodes by index */
	info->indexed_node = auto_alloc_array_clear(device->machine, node_description *, DISCRETE_MAX_NODES);

	/* initialize the node data */
	init_nodes(info, intf, device);

	/* now go back and find pointers to all input nodes */
	find_input_nodes(info, intf);

	/* then set up the output nodes */
	setup_output_nodes(device, info);

	setup_disc_logs(info);
}



/*************************************
 *
 *  Master discrete system stop
 *
 *************************************/

static DEVICE_STOP( discrete )
{
	discrete_info *info = get_safe_token(device);
	int log_num;

	if (DISCRETE_PROFILING)
	{
		int nodenum;
		osd_ticks_t total = 0;
		osd_ticks_t tresh;

		printf("Total Samples: %d\n", info->total_samples);
		/* calculate total time */
		for (nodenum = 0; nodenum < info->node_count; nodenum++)
		{
			node_description *node = info->running_order[nodenum];

			/* Now step the node */
			total += node->run_time;
		}
		/* print statistics */
		tresh = total / info->node_count;
		for (nodenum = 0; nodenum < info->node_count; nodenum++)
		{
			node_description *node = info->running_order[nodenum];

			if (node->run_time > tresh)
				printf("%3d: %20s %8.2f %10d\n", NODE_INDEX(node->node), node->module.name, (float) node->run_time / (float) total * 100.0, ((int) node->run_time) / info->total_samples);
		}
	}

	/* close any csv files */
	for (log_num = 0; log_num < info->num_csvlogs; log_num++)
		if (info->disc_csv_file[log_num])
			fclose(info->disc_csv_file[log_num]);

	/* close any wave files */
	for (log_num = 0; log_num < info->num_wavelogs; log_num++)
		if (info->disc_wav_file[log_num])
			wav_close(info->disc_wav_file[log_num]);

	if (DISCRETE_DEBUGLOG)
	{
		/* close the debug log */
	    if (info->disclogfile)
	    	fclose(info->disclogfile);
		info->disclogfile = NULL;
	}
}



/*************************************
 *
 *  Master reset of all nodes
 *
 *************************************/

static DEVICE_RESET( discrete )
{
	discrete_info *info = get_safe_token(device);
	int nodenum;

	/* loop over all nodes */
	for (nodenum = 0; nodenum < info->node_count; nodenum++)
	{
		node_description *node = info->running_order[nodenum];

		node->output[0] = 0;

		/* if the node has a reset function, call it */
		if (node->module.reset)
			(*node->module.reset)(info, node);

		/* otherwise, just step it */
		else if (node->module.step)
			(*node->module.step)(info, node);
	}
}

/* This was a try which unfortunately did not provide
 * any improvement. Left here for reference
 */
#if 0
INLINE void bigselect(const device_config *device, node_description *node)
{
	switch (node->module.type)
	{
	case  DSS_ADJUSTMENT  :		dss_adjustment_step(device, node); break;
	case  DSS_INPUT_PULSE :		dss_input_pulse_step(device, node); break;
	case  DSS_INPUT_STREAM:		dss_input_stream_step(device, node); break;
	case  DSS_COUNTER     :		dss_counter_step(device, node); break;
	case  DSS_LFSR_NOISE  :		dss_lfsr_step(device, node); break;
	case  DSS_NOISE       :		dss_noise_step(device, node); break;
	case  DSS_NOTE        :		dss_note_step(device, node); break;
	case  DSS_SAWTOOTHWAVE:		dss_sawtoothwave_step(device, node); break;
	case  DSS_SINEWAVE    :		dss_sinewave_step(device, node); break;
	case  DSS_SQUAREWAVE  :		dss_squarewave_step(device, node); break;
	case  DSS_SQUAREWFIX  :		dss_squarewfix_step(device, node); break;
	case  DSS_SQUAREWAVE2 :		dss_squarewave2_step(device, node); break;
	case  DSS_TRIANGLEWAVE:		dss_trianglewave_step(device, node); break;
	case  DSS_INVERTER_OSC :	dss_inverter_osc_step(device, node); break;
	case  DSS_OP_AMP_OSC  :		dss_op_amp_osc_step(device, node); break;
	case  DSS_SCHMITT_OSC :		dss_schmitt_osc_step(device, node); break;
	case  DSS_ADSR        :		dss_adsrenv_step(device, node); break;
	case  DST_ADDER       :		dst_adder_step(device, node); break;
	case  DST_CLAMP       :		dst_clamp_step(device, node); break;
	case  DST_DIVIDE      :		dst_divide_step(device, node); break;
	case  DST_GAIN        :		dst_gain_step(device, node); break;
	case  DST_LOGIC_INV   :		dst_logic_inv_step(device, node); break;
	case  DST_LOGIC_AND   :		dst_logic_and_step(device, node); break;
	case  DST_LOGIC_NAND  :		dst_logic_nand_step(device, node); break;
	case  DST_LOGIC_OR    :		dst_logic_or_step(device, node); break;
	case  DST_LOGIC_NOR   :		dst_logic_nor_step(device, node); break;
	case  DST_LOGIC_XOR   :		dst_logic_xor_step(device, node); break;
	case  DST_LOGIC_NXOR  :		dst_logic_nxor_step(device, node); break;
	case  DST_LOGIC_DFF   :		dst_logic_dff_step(device, node); break;
	case  DST_LOGIC_JKFF  :		dst_logic_jkff_step(device, node); break;
	case  DST_LOOKUP_TABLE:		dst_lookup_table_step(device, node); break;
	case  DST_MULTIPLEX   :		dst_multiplex_step(device, node); break;
	case  DST_ONESHOT     :		dst_oneshot_step(device, node); break;
	case  DST_RAMP        :		dst_ramp_step(device, node); break;
	case  DST_SAMPHOLD    :		dst_samphold_step(device, node); break;
	case  DST_SWITCH      :		dst_switch_step(device, node); break;
	case  DST_ASWITCH     :		dst_aswitch_step(device, node); break;
	case  DST_TRANSFORM   :		dst_transform_step(device, node); break;
	case  DST_COMP_ADDER  :		dst_comp_adder_step(device, node); break;
	case  DST_DAC_R1      :		dst_dac_r1_step(device, node); break;
	case  DST_DIODE_MIX   :		dst_diode_mix_step(device, node); break;
	case  DST_INTEGRATE   :		dst_integrate_step(device, node); break;
	case  DST_MIXER       :		dst_mixer_step(device, node); break;
	case  DST_OP_AMP      :		dst_op_amp_step(device, node); break;
	case  DST_OP_AMP_1SHT :		dst_op_amp_1sht_step(device, node); break;
	case  DST_TVCA_OP_AMP :		dst_tvca_op_amp_step(device, node); break;
	case  DST_FILTER1     :		dst_filter1_step(device, node); break;
	case  DST_FILTER2     :		dst_filter2_step(device, node); break;
	case  DST_SALLEN_KEY  :		dst_sallen_key_step(device, node); break;
	case  DST_CRFILTER    :		dst_crfilter_step(device, node); break;
	case  DST_OP_AMP_FILT :		dst_op_amp_filt_step(device, node); break;
	case  DST_RCDISC      :		dst_rcdisc_step(device, node); break;
	case  DST_RCDISC2     :		dst_rcdisc2_step(device, node); break;
	case  DST_RCDISC3     :		dst_rcdisc3_step(device, node); break;
	case  DST_RCDISC4     :		dst_rcdisc4_step(device, node); break;
	case  DST_RCDISC5     :		dst_rcdisc5_step(device, node); break;
	case  DST_RCINTEGRATE :		dst_rcintegrate_step(device, node); break;
	case  DST_RCDISC_MOD  :		dst_rcdisc_mod_step(device, node); break;
	case  DST_RCFILTER    :		dst_rcfilter_step(device, node); break;
	case  DST_RCFILTER_SW :		dst_rcfilter_sw_step(device, node); break;
	case  DST_RCFILTERN   :		dst_filter1_step(device, node); break;
	case  DST_RCDISCN     :		dst_rcdiscN_step(device, node); break;
	case  DST_RCDISC2N    :		dst_rcdisc2N_step(device, node); break;
	case  DSD_555_ASTBL   :		dsd_555_astbl_step(device, node); break;
	case  DSD_555_MSTBL   :		dsd_555_mstbl_step(device, node); break;
	case  DSD_555_CC      :		dsd_555_cc_step(device, node); break;
	case  DSD_555_VCO1    :		dsd_555_vco1_step(device, node); break;
	case  DSD_566         :		dsd_566_step(device, node); break;
	case  DSD_LS624       :		dsd_ls624_step(device, node); break;
	}
}
#endif
/*************************************
 *
 *  Stream update functions
 *
 *************************************/

static STREAM_UPDATE( discrete_stream_update )
{
	discrete_info *info = (discrete_info *)param;
	int samplenum, nodenum, outputnum;
	double val;
	INT16 wave_data_l, wave_data_r;

	/* Setup any input streams */
	for (nodenum = 0; nodenum < info->discrete_input_streams; nodenum++)
	{
		info->input_stream_data[nodenum] = inputs[nodenum];
	}

	/* Now we must do samples iterations of the node list, one output for each step */
	for (samplenum = 0; samplenum < samples; samplenum++)
	{
		info->total_samples++;
		/* loop over all nodes */
		for (nodenum = 0; nodenum < info->node_count; nodenum++)
		{
			node_description *node = info->running_order[nodenum];

			/* Now step the node */
			if (DISCRETE_PROFILING)
				node->run_time -= osd_profiling_ticks();

			if (node->module.step)
				(*node->module.step)(info, node);
				//bigselect(info->device, node);

			if (DISCRETE_PROFILING)
				node->run_time += osd_profiling_ticks();

		}

		/* Add gain to the output and put into the buffers */
		/* Clipping will be handled by the main sound system */
		for (outputnum = 0; outputnum < info->discrete_outputs; outputnum++)
		{
			val = (*info->output_node[outputnum]->input[0]) * (*info->output_node[outputnum]->input[1]);
			outputs[outputnum][samplenum] = val;
		}

		/* Dump any csv logs */
		for (outputnum = 0; outputnum < info->num_csvlogs; outputnum++)
		{
			fprintf(info->disc_csv_file[outputnum], "%" I64FMT "d", ++info->sample_num);
			for (nodenum = 0; nodenum < info->csvlog_node[outputnum]->active_inputs; nodenum++)
			{
				fprintf(info->disc_csv_file[outputnum], ", %f", *info->csvlog_node[outputnum]->input[nodenum]);
			}
			fprintf(info->disc_csv_file[outputnum], "\n");
		}

		/* Dump any wave logs */
		for (outputnum = 0; outputnum < info->num_wavelogs; outputnum++)
		{
			/* get nodes to be logged and apply gain, then clip to 16 bit */
			val = (*info->wavelog_node[outputnum]->input[0]) * (*info->wavelog_node[outputnum]->input[1]);
			val = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
			wave_data_l = (INT16)val;
			if (info->wavelog_node[outputnum]->active_inputs == 2)
			{
				/* DISCRETE_WAVELOG1 */
				wav_add_data_16(info->disc_wav_file[outputnum], &wave_data_l, 1);
			}
			else
			{
				/* DISCRETE_WAVELOG2 */
				val = (*info->wavelog_node[outputnum]->input[2]) * (*info->wavelog_node[outputnum]->input[3]);
				val = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
				wave_data_r = (INT16)val;

				wav_add_data_16lr(info->disc_wav_file[outputnum], &wave_data_l, &wave_data_r, 1);
			}
		}

		/* advance input streams */
		for (nodenum = 0; nodenum < info->discrete_input_streams; nodenum++)
		{
			info->input_stream_data[nodenum]++;
		}
	}

}



/*************************************
 *
 *  First pass init of nodes
 *
 *************************************/

static void init_nodes(discrete_info *info, discrete_sound_block *block_list, const device_config *device)
{
	int nodenum;

	/* start with no outputs or input streams */
	info->discrete_outputs = 0;
	info->discrete_input_streams = 0;

	/* loop over all nodes */
	for (nodenum = 0; nodenum < info->node_count; nodenum++)
	{
		discrete_sound_block *block = &block_list[nodenum];
		node_description *node = &info->node_list[nodenum];
		int inputnum, modulenum;

		/* our running order just follows the order specified */
		info->running_order[nodenum] = node;

		/* keep track of special nodes */
		if (block->node == NODE_SPECIAL)
		{
			switch(block->type)
			{
				/* Output Node */
				case DSO_OUTPUT:
					if (info->discrete_outputs == DISCRETE_MAX_OUTPUTS)
						fatalerror("init_nodes() - There can not be more then %d output nodes", DISCRETE_MAX_OUTPUTS);
					info->output_node[info->discrete_outputs++] = node;
					break;

				/* CSVlog Node for debugging */
				case DSO_CSVLOG:
					if (info->num_csvlogs == DISCRETE_MAX_CSVLOGS)
						fatalerror("init_nodes() - There can not be more then %d discrete CSV logs.", DISCRETE_MAX_WAVELOGS);
					info->csvlog_node[info->num_csvlogs++] = node;
					break;

				/* Wavelog Node for debugging */
				case DSO_WAVELOG:
					if (info->num_wavelogs == DISCRETE_MAX_WAVELOGS)
						fatalerror("init_nodes() - There can not be more then %d discrete wave logs.", DISCRETE_MAX_WAVELOGS);
					info->wavelog_node[info->num_wavelogs++] = node;
					break;

				default:
					fatalerror("init_nodes() - Failed, trying to create unknown special discrete node.");
			}
		}

		/* otherwise, make sure we are not a duplicate, and put ourselves into the indexed list */
		else
		{
			if (info->indexed_node[NODE_INDEX(block->node)])
				fatalerror("init_nodes() - Duplicate entries for NODE_%02d", NODE_INDEX(block->node));
			info->indexed_node[NODE_INDEX(block->node)] = node;
		}

		/* find the requested module */
		for (modulenum = 0; module_list[modulenum].type != DSS_NULL; modulenum++)
			if (module_list[modulenum].type == block->type)
				break;
		if (module_list[modulenum].type != block->type)
			fatalerror("init_nodes() - Unable to find discrete module type %d for NODE_%02d", block->type, NODE_INDEX(block->node));

		/* static inits */
		node->node = block->node;
		node->module = module_list[modulenum];
		node->output[0] = 0.0;
		node->block = block;

		node->active_inputs = block->active_inputs;
		for (inputnum = 0; inputnum < DISCRETE_MAX_INPUTS; inputnum++)
		{
			node->input[inputnum] = &(block->initial[inputnum]);
		}

		node->context = NULL;
		node->name = block->name;
		node->custom = block->custom;

		/* setup module if custom */
		if (block->type == DST_CUSTOM)
		{
			const discrete_custom_info *custom = (const discrete_custom_info *)node->custom;
			node->module.reset = custom->reset;
			node->module.step = custom->step;
			node->module.contextsize = custom->contextsize;
			node->custom = custom->custom;
		}

		/* allocate memory if necessary */
		if (node->module.contextsize)
			node->context = auto_alloc_array_clear(device->machine, UINT8, node->module.contextsize);

		/* if we are an stream input node, track that */
		if (block->type == DSS_INPUT_STREAM)
		{
			if (info->discrete_input_streams == DISCRETE_MAX_OUTPUTS)
				fatalerror("init_nodes() - There can not be more then %d input stream nodes", DISCRETE_MAX_OUTPUTS);
			/* we will use the node's context pointer to point to the input stream data */
			//node->context = &info->input_stream_data[info->discrete_input_streams++];
			node->context = NULL;
			info->discrete_input_streams++;
		}

		/* and register save state */
		state_save_register_device_item_array(device, nodenum, node->output);
	}

	/* if no outputs, give an error */
	if (info->discrete_outputs == 0)
		fatalerror("init_nodes() - Couldn't find an output node");
}



/*************************************
 *
 *  Find and attach all input nodes
 *
 *************************************/

static void find_input_nodes(discrete_info *info, discrete_sound_block *block_list)
{
	int nodenum, inputnum;

	/* loop over all nodes */
	for (nodenum = 0; nodenum < info->node_count; nodenum++)
	{
		node_description *node = &info->node_list[nodenum];
		discrete_sound_block *block = &block_list[nodenum];

		/* loop over all active inputs */
		for (inputnum = 0; inputnum < node->active_inputs; inputnum++)
		{
			int inputnode = block->input_node[inputnum];

			/* if this input is node-based, find the node in the indexed list */
			if IS_VALUE_A_NODE(inputnode)
			{
				node_description *node_ref = info->indexed_node[NODE_INDEX(inputnode)];
				if (!node_ref)
					fatalerror("discrete_start - NODE_%02d referenced a non existent node NODE_%02d", NODE_INDEX(node->node), NODE_INDEX(inputnode));

				if (NODE_CHILD_NODE_NUM(inputnode) >= node_ref->module.num_output)
					fatalerror("discrete_start - NODE_%02d referenced non existent output %d on node NODE_%02d", NODE_INDEX(node->node), NODE_CHILD_NODE_NUM(inputnode), NODE_INDEX(inputnode));

				node->input[inputnum] = &(node_ref->output[NODE_CHILD_NODE_NUM(inputnode)]);	/* Link referenced node out to input */
				node->input_is_node |= 1 << inputnum;			/* Bit flag if input is node */
			}
			else
			{
				/* warn if trying to use a node for an input that can only be static */
				if IS_VALUE_A_NODE(block->initial[inputnum])
				{
					discrete_log(info, "Warning - discrete_start - NODE_%02d trying to use a node on static input %d",  NODE_INDEX(node->node), inputnum);
					/* also report it in the error log so it is not missed */
					logerror("Warning - discrete_start - NODE_%02d trying to use a node on static input %d",  NODE_INDEX(node->node), inputnum);
				}
			}
		}
	}
}



/*************************************
 *
 *  Set up the output nodes
 *
 *************************************/

static void setup_output_nodes(const device_config *device, discrete_info *info)
{
	/* initialize the stream(s) */
	info->discrete_stream = stream_create(device, info->discrete_input_streams, info->discrete_outputs, info->sample_rate, info, discrete_stream_update);
}



/*************************************
 *
 *  Set up the logs
 *
 *************************************/

static void setup_disc_logs(discrete_info *info)
{
	int log_num, node_num;
	char name[32];

	for (log_num = 0; log_num < info->num_csvlogs; log_num++)
	{
		sprintf(name, "discrete%s_%d.csv", info->device->tag, log_num);
		info->disc_csv_file[log_num] = fopen(name, "w");
		/* Output some header info */
		fprintf(info->disc_csv_file[log_num], "\"MAME Discrete System Node Log\"\n");
		fprintf(info->disc_csv_file[log_num], "\"Log Version\", 1.0\n");
		fprintf(info->disc_csv_file[log_num], "\"Sample Rate\", %d\n", info->sample_rate);
		fprintf(info->disc_csv_file[log_num], "\n");
		fprintf(info->disc_csv_file[log_num], "\"Sample\"");
		for (node_num = 0; node_num < info->csvlog_node[log_num]->active_inputs; node_num++)
		{
			fprintf(info->disc_csv_file[log_num], ", \"NODE_%2d\"", NODE_INDEX(info->csvlog_node[log_num]->block->input_node[node_num]));
		}
		fprintf(info->disc_csv_file[log_num], "\n");
	}

	for (log_num = 0; log_num < info->num_wavelogs; log_num++)
	{
		sprintf(name, "discrete%s_%d.wav", info->device->tag, log_num);
		info->disc_wav_file[log_num] = wav_open(name, info->sample_rate, info->wavelog_node[log_num]->active_inputs/2);
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( discrete )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(discrete_info);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( discrete );			break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( discrete );				break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( discrete );			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Discrete");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Analog");							break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.1");								break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

