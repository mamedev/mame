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
 * dsX_NAME_step(inputs, context, float timestep)  - Perform time step
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

#include "emu.h"
#include "streams.h"
#include "wavwrite.h"
#include "discrete.h"


/*************************************
 *
 *  Performance
 *
 *************************************/

/*
 * Normally, the discrete core processes 960 samples per update.
 * With the various buffers involved, this on a Core2 is not as
 * performant as processing 240 samples 4 times.
 * The setting most probably depends on CPU and which modules are
 * run and how many tasks are defined.
 *
 * Values < 32 exhibit poor performance (too much overhead) while
 * Values > 500 have a slightly worse performace (too much cache misses?).
 */

#define MAX_SAMPLES_PER_TASK_SLICE	(240)

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

static void init_nodes(discrete_info *info, const linked_list_entry *block_list, device_t *device);
static node_description *discrete_find_node(const discrete_info *info, int node);
//static DEVICE_RESET( discrete );
//static STREAM_UPDATE( discrete_stream_update );
static STREAM_UPDATE( buffer_stream_update );

static int profiling = 0;

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
			fflush(disc_info->disclogfile);
		}

		va_end(arg);
	}
}

/*************************************
 *
 *  INLINEs
 *
 *************************************/

INLINE void linked_list_tail_add(const discrete_info *info, linked_list_entry ***list_tail_ptr, const void *ptr)
{
	**list_tail_ptr = auto_alloc(info->device->machine, linked_list_entry);
	(**list_tail_ptr)->ptr = ptr;
	(**list_tail_ptr)->next = NULL;
	*list_tail_ptr = &((**list_tail_ptr)->next);
}

INLINE int linked_list_count(const linked_list_entry *list)
{
	int cnt = 0;
	const linked_list_entry *entry;

	for (entry = list; entry != NULL; entry = entry->next)
		cnt++;

	return cnt;
}

INLINE void linked_list_add(const discrete_info *info, linked_list_entry **list, const void *ptr)
{
	linked_list_entry *entry;

	if (*list == NULL)
	{
		*list = auto_alloc(info->device->machine, linked_list_entry);
		(*list)->ptr = ptr;
		(*list)->next = NULL;
	}
	else
	{
		for (entry = *list; entry != NULL && entry->next != NULL; entry = entry->next)
			;
		entry->next = auto_alloc(info->device->machine, linked_list_entry);
		entry->next->ptr = ptr;
		entry->next->next = NULL;
	}
}

/*************************************
 *
 *  Included simulation objects
 *
 *************************************/

#include "disc_sys.c"		/* discrete core modules and support functions */
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
	{ DSO_OUTPUT      ,"DSO_OUTPUT"      , 0 ,0                                      ,dso_output_reset      ,dso_output_step      ,NULL                  ,NULL                 },
	{ DSO_CSVLOG      ,"DSO_CSVLOG"      , 0 ,sizeof(struct dso_csvlog_context)      ,NULL                  ,dso_csvlog_step      ,dso_csvlog_start      ,dso_csvlog_stop      },
	{ DSO_WAVELOG     ,"DSO_WAVELOG"     , 0 ,sizeof(struct dso_wavlog_context)      ,NULL                  ,dso_wavlog_step      ,dso_wavlog_start      ,dso_wavlog_stop     },
	{ DSO_IMPORT      ,"DSO_IMPORT"      , 0 ,0                                      ,NULL                  ,NULL                 ,NULL                  ,NULL                 },

	/* parallel modules */
	{ DSO_TASK_START  ,"DSO_TASK_START"  , 0 ,0                                      ,dso_task_reset        ,dso_task_start_step  ,dso_task_start_start  ,NULL                 },
	{ DSO_TASK_END    ,"DSO_TASK_END"    , 0 ,0                                      ,dso_task_reset        ,dso_task_end_step    ,NULL                  ,NULL                 },
	{ DSO_TASK_SYNC   ,"DSO_TASK_SYNC"   , 0 ,0                                      ,NULL                  ,NULL                 ,NULL                  ,NULL                 },

	/* nop */
	{ DSS_NOP         ,"DSS_NOP"         , 0 ,0                                      ,NULL                  ,NULL                 ,NULL                  ,NULL                 },

	/* from disc_inp.c */
	{ DSS_ADJUSTMENT  ,"DSS_ADJUSTMENT"  , 1 ,sizeof(struct dss_adjustment_context)  ,dss_adjustment_reset  ,dss_adjustment_step  ,NULL                  ,NULL                 },
	{ DSS_CONSTANT    ,"DSS_CONSTANT"    , 1 ,0                                      ,dss_constant_reset    ,NULL                 ,NULL                  ,NULL                 },
	{ DSS_INPUT_DATA  ,"DSS_INPUT_DATA"  , 1 ,sizeof(struct dss_input_context)       ,dss_input_reset       ,NULL                 ,NULL                  ,NULL                 },
	{ DSS_INPUT_LOGIC ,"DSS_INPUT_LOGIC" , 1 ,sizeof(struct dss_input_context)       ,dss_input_reset       ,NULL                 ,NULL                  ,NULL                 },
	{ DSS_INPUT_NOT   ,"DSS_INPUT_NOT"   , 1 ,sizeof(struct dss_input_context)       ,dss_input_reset       ,NULL                 ,NULL                  ,NULL                 },
	{ DSS_INPUT_PULSE ,"DSS_INPUT_PULSE" , 1 ,sizeof(struct dss_input_context)       ,dss_input_reset       ,dss_input_pulse_step ,NULL                  ,NULL                 },
	{ DSS_INPUT_STREAM,"DSS_INPUT_STREAM", 1 ,sizeof(struct dss_input_context)       ,dss_input_stream_reset,dss_input_stream_step,dss_input_stream_start,NULL                 },
	{ DSS_INPUT_BUFFER,"DSS_INPUT_BUFFER", 1 ,sizeof(struct dss_input_context)       ,dss_input_stream_reset,dss_input_stream_step,dss_input_stream_start,NULL                 },

	/* from disc_wav.c */
	/* Generic modules */
	{ DSS_COUNTER     ,"DSS_COUNTER"     , 1 ,sizeof(struct dss_counter_context)     ,dss_counter_reset     ,dss_counter_step     ,NULL                  ,NULL                 },
	{ DSS_LFSR_NOISE  ,"DSS_LFSR_NOISE"  , 2 ,sizeof(struct dss_lfsr_context)        ,dss_lfsr_reset        ,dss_lfsr_step        ,NULL                  ,NULL                 },
	{ DSS_NOISE       ,"DSS_NOISE"       , 1 ,sizeof(struct dss_noise_context)       ,dss_noise_reset       ,dss_noise_step       ,NULL                  ,NULL                 },
	{ DSS_NOTE        ,"DSS_NOTE"        , 1 ,sizeof(struct dss_note_context)        ,dss_note_reset        ,dss_note_step        ,NULL                  ,NULL                 },
	{ DSS_SAWTOOTHWAVE,"DSS_SAWTOOTHWAVE", 1 ,sizeof(struct dss_sawtoothwave_context),dss_sawtoothwave_reset,dss_sawtoothwave_step,NULL                  ,NULL                 },
	{ DSS_SINEWAVE    ,"DSS_SINEWAVE"    , 1 ,sizeof(struct dss_sinewave_context)    ,dss_sinewave_reset    ,dss_sinewave_step    ,NULL                  ,NULL                 },
	{ DSS_SQUAREWAVE  ,"DSS_SQUAREWAVE"  , 1 ,sizeof(struct dss_squarewave_context)  ,dss_squarewave_reset  ,dss_squarewave_step  ,NULL                  ,NULL                 },
	{ DSS_SQUAREWFIX  ,"DSS_SQUAREWFIX"  , 1 ,sizeof(struct dss_squarewfix_context)  ,dss_squarewfix_reset  ,dss_squarewfix_step  ,NULL                  ,NULL                 },
	{ DSS_SQUAREWAVE2 ,"DSS_SQUAREWAVE2" , 1 ,sizeof(struct dss_squarewave_context)  ,dss_squarewave2_reset ,dss_squarewave2_step ,NULL                  ,NULL                 },
	{ DSS_TRIANGLEWAVE,"DSS_TRIANGLEWAVE", 1 ,sizeof(struct dss_trianglewave_context),dss_trianglewave_reset,dss_trianglewave_step,NULL                  ,NULL                 },
	/* Component specific modules */
	{ DSS_INVERTER_OSC ,"DSS_INVERTER_OSC" , 1 ,sizeof(struct dss_inverter_osc_context) ,dss_inverter_osc_reset ,dss_inverter_osc_step ,NULL                  ,NULL                 },
	{ DSS_OP_AMP_OSC  ,"DSS_OP_AMP_OSC"  , 1 ,sizeof(struct dss_op_amp_osc_context)  ,dss_op_amp_osc_reset  ,dss_op_amp_osc_step  ,NULL                  ,NULL                 },
	{ DSS_SCHMITT_OSC ,"DSS_SCHMITT_OSC" , 1 ,sizeof(struct dss_schmitt_osc_context) ,dss_schmitt_osc_reset ,dss_schmitt_osc_step ,NULL                  ,NULL                 },
	/* Not yet implemented */
	{ DSS_ADSR        ,"DSS_ADSR"        , 1 ,sizeof(struct dss_adsr_context)        ,dss_adsrenv_reset     ,dss_adsrenv_step     ,NULL                  ,NULL                 },

	/* from disc_mth.c */
	/* Generic modules */
	{ DST_ADDER       ,"DST_ADDER"       , 1 ,0                                      ,NULL                  ,dst_adder_step       ,NULL                  ,NULL                 },
	{ DST_CLAMP       ,"DST_CLAMP"       , 1 ,0                                      ,NULL                  ,dst_clamp_step       ,NULL                  ,NULL                 },
	{ DST_DIVIDE      ,"DST_DIVIDE"      , 1 ,0                                      ,NULL                  ,dst_divide_step      ,NULL                  ,NULL                 },
	{ DST_GAIN        ,"DST_GAIN"        , 1 ,0                                      ,NULL                  ,dst_gain_step        ,NULL                  ,NULL                 },
	{ DST_LOGIC_INV   ,"DST_LOGIC_INV"   , 1 ,0                                      ,NULL                  ,dst_logic_inv_step   ,NULL                  ,NULL                 },
	{ DST_BITS_DECODE ,"DST_BITS_DECODE" , 8 ,sizeof(struct dst_bits_decode_context) ,dst_bits_decode_reset ,dst_bits_decode_step ,NULL                  ,NULL                 },
	{ DST_LOGIC_AND   ,"DST_LOGIC_AND"   , 1 ,0                                      ,NULL                  ,dst_logic_and_step   ,NULL                  ,NULL                 },
	{ DST_LOGIC_NAND  ,"DST_LOGIC_NAND"  , 1 ,0                                      ,NULL                  ,dst_logic_nand_step  ,NULL                  ,NULL                 },
	{ DST_LOGIC_OR    ,"DST_LOGIC_OR"    , 1 ,0                                      ,NULL                  ,dst_logic_or_step    ,NULL                  ,NULL                 },
	{ DST_LOGIC_NOR   ,"DST_LOGIC_NOR"   , 1 ,0                                      ,NULL                  ,dst_logic_nor_step   ,NULL                  ,NULL                 },
	{ DST_LOGIC_XOR   ,"DST_LOGIC_XOR"   , 1 ,0                                      ,NULL                  ,dst_logic_xor_step   ,NULL                  ,NULL                 },
	{ DST_LOGIC_NXOR  ,"DST_LOGIC_NXOR"  , 1 ,0                                      ,NULL                  ,dst_logic_nxor_step  ,NULL                  ,NULL                 },
	{ DST_LOGIC_DFF   ,"DST_LOGIC_DFF"   , 1 ,sizeof(struct dst_flipflop_context)    ,dst_logic_ff_reset    ,dst_logic_dff_step   ,NULL                  ,NULL                 },
	{ DST_LOGIC_JKFF  ,"DST_LOGIC_JKFF"  , 1 ,sizeof(struct dst_flipflop_context)    ,dst_logic_ff_reset    ,dst_logic_jkff_step  ,NULL                  ,NULL                 },
	{ DST_LOGIC_SHIFT ,"DST_LOGIC_SHIFT" , 1 ,sizeof(struct dst_shift_context)       ,dst_logic_shift_reset ,dst_logic_shift_step ,NULL                  ,NULL                 },
	{ DST_LOOKUP_TABLE,"DST_LOOKUP_TABLE", 1 ,0                                      ,NULL                  ,dst_lookup_table_step,NULL                  ,NULL                 },
	{ DST_MULTIPLEX   ,"DST_MULTIPLEX"   , 1 ,sizeof(struct dst_size_context)        ,dst_multiplex_reset   ,dst_multiplex_step   ,NULL                  ,NULL                 },
	{ DST_ONESHOT     ,"DST_ONESHOT"     , 1 ,sizeof(struct dst_oneshot_context)     ,dst_oneshot_reset     ,dst_oneshot_step     ,NULL                  ,NULL                 },
	{ DST_RAMP        ,"DST_RAMP"        , 1 ,sizeof(struct dst_ramp_context)        ,dst_ramp_reset        ,dst_ramp_step        ,NULL                  ,NULL                 },
	{ DST_SAMPHOLD    ,"DST_SAMPHOLD"    , 1 ,sizeof(struct dst_samphold_context)    ,dst_samphold_reset    ,dst_samphold_step    ,NULL                  ,NULL                 },
	{ DST_SWITCH      ,"DST_SWITCH"      , 1 ,0                                      ,NULL                  ,dst_switch_step      ,NULL                  ,NULL                 },
	{ DST_ASWITCH     ,"DST_ASWITCH"     , 1 ,0                                      ,NULL                  ,dst_aswitch_step     ,NULL                  ,NULL                 },
	{ DST_TRANSFORM   ,"DST_TRANSFORM"   , 1 ,0                                      ,NULL                  ,dst_transform_step   ,NULL                  ,NULL                 },
	/* Component specific */
	{ DST_COMP_ADDER  ,"DST_COMP_ADDER"  , 1 ,sizeof(struct dst_comp_adder_context)  ,dst_comp_adder_reset  ,dst_comp_adder_step  ,NULL                  ,NULL                 },
	{ DST_DAC_R1      ,"DST_DAC_R1"      , 1 ,sizeof(struct dst_dac_r1_context)      ,dst_dac_r1_reset      ,dst_dac_r1_step      ,NULL                  ,NULL                 },
	{ DST_DIODE_MIX   ,"DST_DIODE_MIX"   , 1 ,sizeof(struct dst_diode_mix_context)   ,dst_diode_mix_reset   ,dst_diode_mix_step   ,NULL                  ,NULL                 },
	{ DST_INTEGRATE   ,"DST_INTEGRATE"   , 1 ,sizeof(struct dst_integrate_context)   ,dst_integrate_reset   ,dst_integrate_step   ,NULL                  ,NULL                 },
	{ DST_MIXER       ,"DST_MIXER"       , 1 ,sizeof(struct dst_mixer_context)       ,dst_mixer_reset       ,dst_mixer_step       ,NULL                  ,NULL                 },
	{ DST_OP_AMP      ,"DST_OP_AMP"      , 1 ,sizeof(struct dst_op_amp_context)      ,dst_op_amp_reset      ,dst_op_amp_step      ,NULL                  ,NULL                 },
	{ DST_OP_AMP_1SHT ,"DST_OP_AMP_1SHT" , 1 ,sizeof(struct dst_op_amp_1sht_context) ,dst_op_amp_1sht_reset ,dst_op_amp_1sht_step ,NULL                  ,NULL                 },
	{ DST_TVCA_OP_AMP ,"DST_TVCA_OP_AMP" , 1 ,sizeof(struct dst_tvca_op_amp_context) ,dst_tvca_op_amp_reset ,dst_tvca_op_amp_step ,NULL                  ,NULL                 },
	{ DST_VCA         ,"DST_VCA"         , 1 ,0                                      ,NULL                  ,NULL                 ,NULL                  ,NULL                 },
	{ DST_XTIME_BUFFER,"DST_XTIME_BUFFER", 1 ,0                                      ,NULL                  ,dst_xtime_buffer_step,NULL                  ,NULL                 },
	{ DST_XTIME_AND   ,"DST_XTIME_AND"   , 1 ,0                                      ,NULL                  ,dst_xtime_and_step   ,NULL                  ,NULL                 },
	{ DST_XTIME_OR    ,"DST_XTIME_OR"    , 1 ,0                                      ,NULL                  ,dst_xtime_or_step    ,NULL                  ,NULL                 },
	{ DST_XTIME_XOR   ,"DST_XTIME_XOR"   , 1 ,0                                      ,NULL                  ,dst_xtime_xor_step   ,NULL                  ,NULL                 },

	/* from disc_flt.c */
	/* Generic modules */
	{ DST_FILTER1     ,"DST_FILTER1"     , 1 ,sizeof(struct dst_filter1_context)     ,dst_filter1_reset     ,dst_filter1_step     ,NULL                  ,NULL                 },
	{ DST_FILTER2     ,"DST_FILTER2"     , 1 ,sizeof(struct dst_filter2_context)     ,dst_filter2_reset     ,dst_filter2_step     ,NULL                  ,NULL                 },
	/* Component specific modules */
	{ DST_SALLEN_KEY  ,"DST_SALLEN_KEY"  , 1 ,sizeof(struct dst_filter2_context)     ,dst_sallen_key_reset  ,dst_sallen_key_step  ,NULL                  ,NULL                 },
	{ DST_CRFILTER    ,"DST_CRFILTER"    , 1 ,sizeof(struct dst_rcfilter_context)    ,dst_crfilter_reset    ,dst_crfilter_step    ,NULL                  ,NULL                 },
	{ DST_OP_AMP_FILT ,"DST_OP_AMP_FILT" , 1 ,sizeof(struct dst_op_amp_filt_context) ,dst_op_amp_filt_reset ,dst_op_amp_filt_step ,NULL                  ,NULL                 },
	{ DST_RC_CIRCUIT_1,"DST_RC_CIRCUIT_1", 1 ,sizeof(struct dst_rc_circuit_1_context),dst_rc_circuit_1_reset,dst_rc_circuit_1_step,NULL                  ,NULL                 },
	{ DST_RCDISC      ,"DST_RCDISC"      , 1 ,sizeof(struct dst_rcdisc_context)      ,dst_rcdisc_reset      ,dst_rcdisc_step      ,NULL                  ,NULL                 },
	{ DST_RCDISC2     ,"DST_RCDISC2"     , 1 ,sizeof(struct dst_rcdisc_context)      ,dst_rcdisc2_reset     ,dst_rcdisc2_step     ,NULL                  ,NULL                 },
	{ DST_RCDISC3     ,"DST_RCDISC3"     , 1 ,sizeof(struct dst_rcdisc_context)      ,dst_rcdisc3_reset     ,dst_rcdisc3_step     ,NULL                  ,NULL                 },
	{ DST_RCDISC4     ,"DST_RCDISC4"     , 1 ,sizeof(struct dst_rcdisc4_context)     ,dst_rcdisc4_reset     ,dst_rcdisc4_step     ,NULL                  ,NULL                 },
	{ DST_RCDISC5     ,"DST_RCDISC5"     , 1 ,sizeof(struct dst_rcdisc_context)      ,dst_rcdisc5_reset     ,dst_rcdisc5_step     ,NULL                  ,NULL                 },
	{ DST_RCINTEGRATE ,"DST_RCINTEGRATE" , 1 ,sizeof(struct dst_rcintegrate_context) ,dst_rcintegrate_reset ,dst_rcintegrate_step ,NULL                  ,NULL                 },
	{ DST_RCDISC_MOD  ,"DST_RCDISC_MOD"  , 1 ,sizeof(struct dst_rcdisc_mod_context)  ,dst_rcdisc_mod_reset  ,dst_rcdisc_mod_step  ,NULL                  ,NULL                 },
	{ DST_RCFILTER    ,"DST_RCFILTER"    , 1 ,sizeof(struct dst_rcfilter_context)    ,dst_rcfilter_reset    ,dst_rcfilter_step    ,NULL                  ,NULL                 },
	{ DST_RCFILTER_SW ,"DST_RCFILTER_SW" , 1 ,sizeof(struct dst_rcfilter_sw_context) ,dst_rcfilter_sw_reset ,dst_rcfilter_sw_step ,NULL                  ,NULL                 },
	/* For testing - seem to be buggered.  Use versions not ending in N. */
	{ DST_RCFILTERN   ,"DST_RCFILTERN"   , 1 ,sizeof(struct dst_filter1_context)     ,dst_rcfilterN_reset   ,dst_filter1_step     ,NULL                  ,NULL                 },
	{ DST_RCDISCN     ,"DST_RCDISCN"     , 1 ,sizeof(struct dst_filter1_context)     ,dst_rcdiscN_reset     ,dst_rcdiscN_step     ,NULL                  ,NULL                 },
	{ DST_RCDISC2N    ,"DST_RCDISC2N"    , 1 ,sizeof(struct dst_rcdisc2_context)     ,dst_rcdisc2N_reset    ,dst_rcdisc2N_step    ,NULL                  ,NULL                 },

	/* from disc_dev.c */
	/* generic modules */
	{ DST_CUSTOM      ,"DST_CUSTOM"      , 8 ,0                                      ,NULL                  ,NULL                 ,NULL                  ,NULL                 },
	/* Component specific modules */
	{ DSD_555_ASTBL   ,"DSD_555_ASTBL"   , 1 ,sizeof(struct dsd_555_astbl_context)   ,dsd_555_astbl_reset   ,dsd_555_astbl_step   ,NULL                  ,NULL                 },
	{ DSD_555_MSTBL   ,"DSD_555_MSTBL"   , 1 ,sizeof(struct dsd_555_mstbl_context)   ,dsd_555_mstbl_reset   ,dsd_555_mstbl_step   ,NULL                  ,NULL                 },
	{ DSD_555_CC      ,"DSD_555_CC"      , 1 ,sizeof(struct dsd_555_cc_context)      ,dsd_555_cc_reset      ,dsd_555_cc_step      ,NULL                  ,NULL                 },
	{ DSD_555_VCO1    ,"DSD_555_VCO1"    , 1 ,sizeof(struct dsd_555_vco1_context)    ,dsd_555_vco1_reset    ,dsd_555_vco1_step    ,NULL                  ,NULL                 },
	{ DSD_566         ,"DSD_566"         , 1 ,sizeof(struct dsd_566_context)         ,dsd_566_reset         ,dsd_566_step         ,NULL                  ,NULL                 },
	{ DSD_LS624       ,"DSD_LS624"       , 1 ,sizeof(struct dsd_ls624_context)       ,dsd_ls624_reset       ,dsd_ls624_step       ,NULL                  ,NULL                 },
	/* must be the last one */
	{ DSS_NULL        ,"DSS_NULL"        , 0 ,0                                      ,NULL                  ,NULL                 ,NULL                  ,NULL                 }
};

INLINE void step_nodes_in_list(node_list_t &list)
{

	if (EXPECTED(!profiling))
	{
		for_each(node_description *, entry, &list)
		{
			/* Now step the node */
			(*entry.item()->step)(entry.item());
		}
	}
	else
	{
		osd_ticks_t last = get_profile_ticks();

		for_each(node_description *, entry, &list)
		{
			node_description *node = entry.item();

			node->run_time -= last;
			(*node->step)(node);
			last = get_profile_ticks();
			node->run_time += last;
		}
	}
}

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

static void discrete_build_list(discrete_info *info, const discrete_sound_block *intf, linked_list_entry ***current)
{
	int node_count = 0;

	for (; intf[node_count].type != DSS_NULL; )
	{
		/* scan imported */
		if (intf[node_count].type == DSO_IMPORT)
		{
			discrete_log(info, "discrete_build_list() - DISCRETE_IMPORT @ NODE_%02d", NODE_INDEX(intf[node_count].node) );
			discrete_build_list(info, (discrete_sound_block *) intf[node_count].custom, current);
		}
		else if (intf[node_count].type == DSO_REPLACE)
		{
			linked_list_entry *entry;

			node_count++;
			if (intf[node_count].type == DSS_NULL)
				fatalerror("discrete_build_list: DISCRETE_REPLACE at end of node_list");

			for (entry = info->block_list; entry != NULL; entry = entry->next)
			{
				discrete_sound_block *block = (discrete_sound_block *) entry->ptr;

				if (block->type != NODE_SPECIAL )
					if (block->node == intf[node_count].node)
					{
						entry->ptr = (void *) &intf[node_count];
						discrete_log(info, "discrete_build_list() - DISCRETE_REPLACE @ NODE_%02d", NODE_INDEX(intf[node_count].node) );
						break;
					}
			}

			if (entry == NULL)
				fatalerror("discrete_build_list: DISCRETE_REPLACE did not found node %d", NODE_INDEX(intf[node_count].node));

		}
		else if (intf[node_count].type == DSO_DELETE)
		{
			linked_list_entry *entry, *last;

			last = NULL;
			for (entry = info->block_list; entry != NULL; last = entry, entry = entry->next)
			{
				discrete_sound_block *block = (discrete_sound_block *) entry->ptr;

				if ((block->node >= intf[node_count].input_node[0]) &&
						(block->node <= intf[node_count].input_node[1]))
				{
					discrete_log(info, "discrete_build_list() - DISCRETE_DELETE deleted NODE_%02d", NODE_INDEX(block->node) );
					if (last != NULL)
						last->next = entry->next;
					else
						info->block_list = entry->next;
				}
			}
		}
		else
		{
			discrete_log(info, "discrete_build_list() - adding node %d (*current %p)\n", node_count, *current);
			linked_list_tail_add(info, current, &intf[node_count]);
		}

		node_count++;
	}
}

/*************************************
 *
 *  Sanity check list
 *
 *************************************/

static void discrete_sanity_check(const discrete_info *info)
{
	const linked_list_entry *entry;
	int node_count = 0;

	discrete_log(info, "discrete_start() - Doing node list sanity check");
	for (entry = info->block_list; entry != NULL; entry = entry->next)
	{
		discrete_sound_block *block = (discrete_sound_block *) entry->ptr;

		/* make sure we don't have too many nodes overall */
		if (node_count > DISCRETE_MAX_NODES)
			fatalerror("discrete_start() - Upper limit of %d nodes exceeded, have you terminated the interface block?", DISCRETE_MAX_NODES);

		/* make sure the node number is in range */
		if (block->node < NODE_START || block->node > NODE_END)
			fatalerror("discrete_start() - Invalid node number on node %02d descriptor", block->node);

		/* make sure the node type is valid */
		if (block->type > DSO_OUTPUT)
			fatalerror("discrete_start() - Invalid function type on NODE_%02d", NODE_INDEX(block->node) );

		/* make sure this is a main node */
		if (NODE_CHILD_NODE_NUM(block->node) > 0)
			fatalerror("discrete_start() - Child node number on NODE_%02d", NODE_INDEX(block->node) );

		node_count++;
	}
	discrete_log(info, "discrete_start() - Sanity check counted %d nodes", node_count);

}

/*************************************
 *
 *  Master discrete system start
 *
 *************************************/
#if 0
static DEVICE_START( discrete )
{
	linked_list_entry **intf;
	const discrete_sound_block *intf_start = (discrete_sound_block *)device->baseconfig().static_config();
	discrete_info *info = get_safe_token(device);
	char name[32];

	info->device = device;

	/* If a clock is specified we will use it, otherwise run at the audio sample rate. */
	if (device->clock())
		info->sample_rate = device->clock();
	else
		info->sample_rate = device->machine->sample_rate;
	info->sample_time = 1.0 / info->sample_rate;
	info->neg_sample_time = - info->sample_time;

	info->total_samples = 0;
	info->total_stream_updates = 0;

	/* create the logfile */
	sprintf(name, "discrete%s.log", device->tag());
	if (DISCRETE_DEBUGLOG)
		info->disclogfile = fopen(name, "w");

	/* enable profiling */
	if (getenv("DISCRETE_PROFILING"))
		profiling = atoi(getenv("DISCRETE_PROFILING"));

	/* Build the final block list */
	info->block_list = NULL;
	intf = &info->block_list;
	discrete_build_list(info, intf_start, &intf);

	/* first pass through the nodes: sanity check, fill in the indexed_nodes, and make a total count */
	discrete_sanity_check(info);

	/* Start with empty lists */
	info->node_list.reset();
	info->output_list.reset();
	info->input_list.reset();

	/* allocate memory to hold pointers to nodes by index */
	info->indexed_node = auto_alloc_array_clear(device->machine, node_description *, DISCRETE_MAX_NODES);

	/* initialize the node data */
	init_nodes(info, info->block_list, device);

	/* now go back and find pointers to all input nodes */
	for_each(node_description *, node, &info->node_list)
	{
		node.item()->find_input_nodes();
	}

	/* initialize the stream(s) */
	info->discrete_stream = stream_create(device,info->input_list.count(), info->output_list.count(), info->sample_rate, info, discrete_stream_update);

	/* allocate a queue */

	info->queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI | WORK_QUEUE_FLAG_HIGH_FREQ);

	/* Process nodes which have a start func */

	for_each(node_description *, node, &info->node_list)
	{
		if (node.item()->module->start)
			(*node.item()->module->start)(node.item());
	}

}
#endif


/*************************************
 *
 *  Master discrete system stop
 *
 *************************************/

static UINT64 list_run_time(const node_list_t &list)
{
	UINT64 total = 0;

	for_each(node_description *, node, &list)
	{
		total += node.item()->run_time;
	}
	return total;
}

static void display_profiling(const discrete_info *info)
{
	int count;
	UINT64 total;
	UINT64 tresh;
	double tt;

	/* calculate total time */
	total = list_run_time(info->node_list);
	count = info->node_list.count();
	/* print statistics */
	printf("Total Samples  : %16" I64FMT "d\n", info->total_samples);
	tresh = total / count;
	printf("Threshold (mean): %16" I64FMT "d\n", tresh / info->total_samples );
	for_each(node_description *, node, &info->node_list)
	{
		if (node.item()->run_time > tresh)
			printf("%3d: %20s %8.2f %10.2f\n", node.item()->index(), node.item()->module->name, (float) node.item()->run_time / (float) total * 100.0, ((float) node.item()->run_time) / (float) info->total_samples);
	}

	/* Task information */
	for_each(discrete_task *, task, &info->task_list)
	{
		tt =  list_run_time(task.item()->list);

		printf("Task(%d): %8.2f %15.2f\n", task.item()->task_group, tt / (double) total * 100.0, tt / (double) info->total_samples);
	}

	printf("Average samples/stream_update: %8.2f\n", (double) info->total_samples / (double) info->total_stream_updates);
}

#if 0
static DEVICE_STOP( discrete )
{
	discrete_info *info = get_safe_token(device);

	osd_work_queue_free(info->queue);

	if (profiling)
	{
		display_profiling(info);
	}

	/* Process nodes which have a stop func */

	for_each(node_description *, node, &info->node_list)
	{
		if (node.item()->module->stop)
			(*node.item()->module->stop)(node.item());
	}

	if (DISCRETE_DEBUGLOG)
	{
		/* close the debug log */
	    if (info->disclogfile)
	    	fclose(info->disclogfile);
		info->disclogfile = NULL;
	}
}
#endif


/*************************************
 *
 *  Master reset of all nodes
 *
 *************************************/
#if 0
static DEVICE_RESET( discrete )
{
	const discrete_info *info = get_safe_token(device);

	/* loop over all nodes */
	for_each (node_description *, node, &info->node_list)
	{
		node.item()->output[0] = 0;

		/* if the node has a reset function, call it */
		if (node.item()->module->reset)
			(*node.item()->module->reset)(node.item());

		/* otherwise, just step it */
		else if (node.item()->step)
			(*node.item()->step)(node.item());
	}
}
#endif

/*************************************
 *
 *  Stream update functions
 *
 *************************************/

static void *task_callback(void *param, int threadid)
{
	task_list_t *list = (task_list_t *) param;
	int samples;

	do
	{
		for_each(discrete_task *, task, list)
		{
			/* try to lock */
			if (task.item()->lock_threadid(threadid))
			{
				samples = MIN(task.item()->samples, MAX_SAMPLES_PER_TASK_SLICE);

				/* check dependencies */
				for_each(discrete_source_node *, sn, &task.item()->source_list)
				{
					int avail;

					avail = sn.item()->task->ptr[sn.item()->output_node] - sn.item()->ptr;
					assert_always(avail >= 0, "task_callback: available samples are negative");
					if (avail < samples)
						samples = avail;
				}

				task.item()->samples -= samples;
				assert_always(task.item()->samples >=0, "task_callback: task_samples got negative");
				while (samples > 0)
				{
					/* step */
					step_nodes_in_list(task.item()->list);
					samples--;
				}
				if (task.item()->samples == 0)
				{
					/* return and keep the task locked so it is not picked up by other worker threads */
					return NULL;
				}
				task.item()->unlock();
			}
		}
	} while (1);

	return NULL;
}


static STREAM_UPDATE( buffer_stream_update )
{
	const node_description *node = (node_description *) param;
	const struct dss_input_context *context = (struct dss_input_context *)node->context;
	stream_sample_t *ptr = outputs[0];
	int data = context->data;
	int samplenum = samples;

	while (samplenum-- > 0)
	  *(ptr++) = data;
}

#if 0
static STREAM_UPDATE( discrete_stream_update )
{
	discrete_info *info = (discrete_info *)param;
	int outputnum;
	//, task_group;

	if (samples == 0)
		return;

	/* Setup any output streams */
	outputnum = 0;
	for_each(node_description *, node, &info->output_list)
	{
		node.item()->context = (void *) outputs[outputnum];
		outputnum++;
	}

	/* Setup any input streams */
	for_each(node_description *, node, &info->input_list)
	{
		struct dss_input_context *context = (struct dss_input_context *) node.item()->context;
		context->ptr = (stream_sample_t *) inputs[context->stream_in_number];
	}

	/* Setup tasks */
	for_each(discrete_task *, task, &info->task_list)
	{
		int					i;

		task.item()->samples = samples;
		/* unlock the thread */
		task.item()->unlock();

		/* set up task buffers */
		for (i = 0; i < task.item()->numbuffered; i++)
			task.item()->ptr[i] = task.item()->node_buf[i];

		/* initialize sources */
		for_each(discrete_source_node *, sn, &task.item()->source_list)
		{
			sn.item()->ptr = sn.item()->task->node_buf[sn.item()->output_node];
		}
	}

	for_each(discrete_task *, task, &info->task_list)
	{
		/* Fire a work item for each task */
		osd_work_item_queue(info->queue, task_callback, (void *) &info->task_list, WORK_ITEM_FLAG_AUTO_RELEASE);
	}
	osd_work_queue_wait(info->queue, osd_ticks_per_second()*10);

	if (profiling)
	{
		info->total_samples += samples;
		info->total_stream_updates++;
	}

}
#endif


/*************************************
 *
 *  First pass init of nodes
 *
 *************************************/


static void init_nodes(discrete_info *info, const linked_list_entry *block_list, device_t *device)
{
	const linked_list_entry	*entry;
	discrete_task *task = NULL;
	/* list tail pointers */
	int					has_tasks = 0;

	/* check whether we have tasks ... */
	for (entry = block_list; entry != NULL; entry = entry->next)
	{
		const discrete_sound_block *block = (discrete_sound_block *) entry->ptr;
		if (block->type == DSO_TASK_START)
			has_tasks = 1;
	}

	if (!has_tasks)
	{
		/* make sure we have one simple task
         * No need to create a node since there are no dependencies.
         */
		task = auto_alloc_clear(info->device->machine, discrete_task);
		info->task_list.add_tail(task);
	}

	/* loop over all nodes */
	for (entry = block_list; entry != NULL; entry = entry->next)
	{
		const discrete_sound_block *block = (discrete_sound_block *) entry->ptr;
		int modulenum;

		/* find the requested module */
		for (modulenum = 0; module_list[modulenum].type != DSS_NULL; modulenum++)
			if (module_list[modulenum].type == block->type)
				break;
		if (module_list[modulenum].type != block->type)
			fatalerror("init_nodes() - Unable to find discrete module type %d for NODE_%02d", block->type, NODE_INDEX(block->node));

		node_description *node = auto_alloc_clear(info->device->machine, node_description(info, &module_list[modulenum], block));

		/* keep track of special nodes */
		if (block->node == NODE_SPECIAL)
		{
			switch(block->type)
			{
				/* Output Node */
				case DSO_OUTPUT:
					info->output_list.add_tail(node);
					break;

				/* CSVlog Node for debugging */
				case DSO_CSVLOG:
					break;

				/* Wavelog Node for debugging */
				case DSO_WAVELOG:
					break;

				/* Task processing */
				case DSO_TASK_START:
					if (task != NULL)
						fatalerror("init_nodes() - Nested DISCRETE_START_TASK.");
					task = auto_alloc_clear(info->device->machine, discrete_task);
					node->context = task;
					break;

				case DSO_TASK_END:
					if (task == NULL)
						fatalerror("init_nodes() - NO DISCRETE_START_TASK.");
					task->numbuffered = 0;
					task->task_group = -1; /* will be set later */
					task->source_list.reset();
					info->task_list.add_tail(task);
					node->context = task;
					//task = NULL;
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

		/* if we are an stream input node, track that */
		if (block->type == DSS_INPUT_STREAM)
		{
			info->input_list.add_tail(node);
		}
		else if (block->type == DSS_INPUT_BUFFER)
		{
			info->input_list.add_tail(node);
		}

		/* add to node list */
		info->node_list.add_tail(node);

		/* our running order just follows the order specified */
		/* does the node step ? */
		if (node->step != NULL)
		{
			/* do we belong to a task? */
			if (task == NULL)
				fatalerror("init_nodes() - found node outside of task.");
			else
				task->list.add_tail(node);
		}

		if (block->type == DSO_TASK_END)
		{
			task = NULL;
		}

		/* and register save state */
		node->save_state(device);
	}

	if (!has_tasks)
	{
	}

	/* if no outputs, give an error */
	if (info->output_list.count() == 0)
		fatalerror("init_nodes() - Couldn't find an output node");
}


/*************************************
 *
 *  node_description implementation
 *
 *************************************/

node_description::node_description(const discrete_info * xinfo, const discrete_module *xmodule, const discrete_sound_block *xblock) :
	context(NULL),
	module(xmodule),
	info(xinfo),
	run_time(0),
	m_block(xblock)
{
	output[0] = 0.0;
	m_custom = m_block->custom;
	m_active_inputs = m_block->active_inputs;

	/* setup module if custom */
	if (m_block->type == DST_CUSTOM)
	{
		const discrete_custom_info *custom = (const discrete_custom_info *)m_custom;
		module = &custom->module;
		m_custom = custom->custom;
	}

	/* copy initial / default step function */
	step = module->step;

	/* allocate memory if necessary */
	if (module->contextsize)
		context =  global_alloc_array(UINT8, module->contextsize);
}

node_description::~node_description(void)
{
	if (module->contextsize)
		global_free(context);
}

int node_description::index(void)
{
	return NODE_INDEX(m_block->node);
}

void node_description::save_state(device_t *device)
{
	if (m_block->node != NODE_SPECIAL)
		state_save_register_device_item_array(device, m_block->node, output);
}

void node_description::find_input_nodes(void)
{
	int inputnum;

	/* loop over all active inputs */
	for (inputnum = 0; inputnum < m_active_inputs; inputnum++)
	{
		int inputnode = m_block->input_node[inputnum];

		/* if this input is node-based, find the node in the indexed list */
		if IS_VALUE_A_NODE(inputnode)
		{
			const node_description *node_ref = info->indexed_node[NODE_INDEX(inputnode)];
			if (!node_ref)
				fatalerror("discrete_start - NODE_%02d referenced a non existent node NODE_%02d", index(), NODE_INDEX(inputnode));

			if ((NODE_CHILD_NODE_NUM(inputnode) >= node_ref->module->num_output) && (node_ref->module->type != DST_CUSTOM))
				fatalerror("discrete_start - NODE_%02d referenced non existent output %d on node NODE_%02d", index(), NODE_CHILD_NODE_NUM(inputnode), NODE_INDEX(inputnode));

			input[inputnum] = &(node_ref->output[NODE_CHILD_NODE_NUM(inputnode)]);	/* Link referenced node out to input */
			m_input_is_node |= 1 << inputnum;			/* Bit flag if input is node */
		}
		else
		{
			/* warn if trying to use a node for an input that can only be static */
			if IS_VALUE_A_NODE(m_block->initial[inputnum])
			{
				discrete_log(info, "Warning - discrete_start - NODE_%02d trying to use a node on static input %d",  index(), inputnum);
				/* also report it in the error log so it is not missed */
				logerror("Warning - discrete_start - NODE_%02d trying to use a node on static input %d",  index(), inputnum);
			}
			else
			{
				input[inputnum] = &(m_block->initial[inputnum]);
				//node->input[inputnum] = getDoublePtr(node->block->initial[inputnum]);
			}
		}
	}
	for (inputnum = m_active_inputs; inputnum < DISCRETE_MAX_INPUTS; inputnum++)
	{
		/* FIXME: Check that no nodes follow ! */
		input[inputnum] = &(m_block->initial[inputnum]);
	}
}

int node_description::same_module_index(const node_list_t &list)
{
	int index = 0;

	for_each(node_description *, n, &list)
	{
		if (n.item() == this)
			return index;
		if (n.item()->module->type == this->module->type)
			index++;
	}
	return -1;
}

#if 0
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
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Analog");							break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.1");								break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(DISCRETE, discrete);
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type DISCRETE = discrete_device_config::static_alloc_device_config;

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

#if 0
//-------------------------------------------------
//  static_set_type - configuration helper to set
//  the chip type
//-------------------------------------------------

void discrete_device_config::static_set_type(device_config *device, int type)
{
	discrete_device_config *asc = downcast<discrete_device_config *>(device);
	asc->m_type = type;
}

//-------------------------------------------------
//  static_set_type - configuration helper to set
//  the IRQ callback
//-------------------------------------------------


void discrete_device_config::static_set_irqf(device_config *device, void (*irqf)(device_t *device, int state))
{
	discrete_device_config *asc = downcast<discrete_device_config *>(device);
	asc->m_irq_func = irqf;
}
#endif

//-------------------------------------------------
//  discrete_device_config - constructor
//-------------------------------------------------

discrete_device_config::discrete_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "DISCRETE", tag, owner, clock),
	  device_config_sound_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *discrete_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(discrete_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *discrete_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, discrete_device(machine, *this));
}


//-------------------------------------------------
//  discrete_device - constructor
//-------------------------------------------------

discrete_device::discrete_device(running_machine &_machine, const discrete_device_config &config)
	: device_t(_machine, config),
	  device_sound_interface(_machine, config, *this),
	  m_config(config)
{
	//memset(&m_info, 0, sizeof(m_info));

}

discrete_device::~discrete_device(void)
{
	discrete_info *info = &m_info;

	osd_work_queue_free(info->queue);

	if (profiling)
	{
		display_profiling(info);
	}

	/* Process nodes which have a stop func */

	for_each(node_description *, node, &info->node_list)
	{
		if (node.item()->module->stop)
			(*node.item()->module->stop)(node.item());
	}

	if (DISCRETE_DEBUGLOG)
	{
		/* close the debug log */
	    if (info->disclogfile)
	    	fclose(info->disclogfile);
		info->disclogfile = NULL;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void discrete_device::device_start()
{
	// create the stream
	//m_stream = stream_create(this, 0, 2, 22257, this, static_stream_generate);

	linked_list_entry **intf;
	const discrete_sound_block *intf_start = (discrete_sound_block *) baseconfig().static_config();
	discrete_info *info = &m_info;
	char name[32];

	info->device = this;

	/* If a clock is specified we will use it, otherwise run at the audio sample rate. */
	if (this->clock())
		info->sample_rate = this->clock();
	else
		info->sample_rate = this->machine->sample_rate;
	info->sample_time = 1.0 / info->sample_rate;
	info->neg_sample_time = - info->sample_time;

	info->total_samples = 0;
	info->total_stream_updates = 0;

	/* create the logfile */
	sprintf(name, "discrete%s.log", this->tag());
	if (DISCRETE_DEBUGLOG)
		info->disclogfile = fopen(name, "w");

	/* enable profiling */
	if (getenv("DISCRETE_PROFILING"))
		profiling = atoi(getenv("DISCRETE_PROFILING"));

	/* Build the final block list */
	info->block_list = NULL;
	intf = &info->block_list;
	discrete_build_list(info, intf_start, &intf);

	/* first pass through the nodes: sanity check, fill in the indexed_nodes, and make a total count */
	discrete_sanity_check(info);

	/* Start with empty lists */
	info->node_list.reset();
	info->output_list.reset();
	info->input_list.reset();

	/* allocate memory to hold pointers to nodes by index */
	info->indexed_node = auto_alloc_array_clear(this->machine, node_description *, DISCRETE_MAX_NODES);

	/* initialize the node data */
	init_nodes(info, info->block_list, this);

	/* now go back and find pointers to all input nodes */
	for_each(node_description *, node, &info->node_list)
	{
		node.item()->find_input_nodes();
	}

	/* initialize the stream(s) */
	info->discrete_stream = stream_create(this,info->input_list.count(), info->output_list.count(), info->sample_rate, this, static_stream_generate);

	/* allocate a queue */

	info->queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI | WORK_QUEUE_FLAG_HIGH_FREQ);

	/* Process nodes which have a start func */

	for_each(node_description *, node, &info->node_list)
	{
		if (node.item()->module->start)
			(*node.item()->module->start)(node.item());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void discrete_device::device_reset()
{

	const discrete_info *info = &m_info;
	stream_update(info->discrete_stream);

	/* loop over all nodes */
	for_each (node_description *, node, &info->node_list)
	{
		node.item()->output[0] = 0;

		/* if the node has a reset function, call it */
		if (node.item()->module->reset)
			(*node.item()->module->reset)(node.item());

		/* otherwise, just step it */
		else if (node.item()->step)
			(*node.item()->step)(node.item());
	}
}

//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------

STREAM_UPDATE( discrete_device::static_stream_generate )
{
	reinterpret_cast<discrete_device *>(param)->stream_generate(inputs, outputs, samples);
}

void discrete_device::stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	discrete_info *info = &m_info;
	int outputnum;
	//, task_group;

	if (samples == 0)
		return;
	/* Setup any output streams */
	outputnum = 0;
	for_each(node_description *, node, &info->output_list)
	{
		node.item()->context = (void *) outputs[outputnum];
		outputnum++;
	}

	/* Setup any input streams */
	for_each(node_description *, node, &info->input_list)
	{
		struct dss_input_context *context = (struct dss_input_context *) node.item()->context;
		context->ptr = (stream_sample_t *) inputs[context->stream_in_number];
	}

	/* Setup tasks */
	for_each(discrete_task *, task, &info->task_list)
	{
		int					i;

		task.item()->samples = samples;
		/* unlock the thread */
		task.item()->unlock();

		/* set up task buffers */
		for (i = 0; i < task.item()->numbuffered; i++)
			task.item()->ptr[i] = task.item()->node_buf[i];

		/* initialize sources */
		for_each(discrete_source_node *, sn, &task.item()->source_list)
		{
			sn.item()->ptr = sn.item()->task->node_buf[sn.item()->output_node];
		}
	}

	for_each(discrete_task *, task, &info->task_list)
	{
		/* Fire a work item for each task */
		osd_work_item_queue(info->queue, task_callback, (void *) &info->task_list, WORK_ITEM_FLAG_AUTO_RELEASE);
	}
	osd_work_queue_wait(info->queue, osd_ticks_per_second()*10);

	if (profiling)
	{
		info->total_samples += samples;
		info->total_stream_updates++;
	}
}

//-------------------------------------------------
//  read - read from the chip's registers and internal RAM
//-------------------------------------------------

READ8_MEMBER( discrete_device::read )
{
	discrete_info    *info = &m_info;
	node_description *node = discrete_find_node(info, offset);

	UINT8 data = 0;

	/* Read the node input value if allowed */
	if (node)
	{
		/* Bring the system up to now */
		stream_update(info->discrete_stream);

		data = (UINT8) node->output[NODE_CHILD_NODE_NUM(offset)];
	}
	else
		fatalerror("discrete_sound_r read from non-existent NODE_%02d\n", offset-NODE_00);

    return data;
}

//-------------------------------------------------
//  write - write to the chip's registers and internal RAM
//-------------------------------------------------

WRITE8_MEMBER( discrete_device::write )
{
	discrete_info    *info = &m_info;
	node_description *node = discrete_find_node(info, offset);

	/* Update the node input value if it's a proper input node */
	if (node)
	{
		struct dss_input_context *context = (struct dss_input_context *)node->context;
		UINT8 new_data    = 0;

		switch (node->module->type)
		{
			case DSS_INPUT_DATA:
			case DSS_INPUT_BUFFER:
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

		if (context->data != new_data)
		{
			if (context->is_buffered)
			{
				/* Bring the system up to now */
				stream_update(context->buffer_stream);

				context->data = new_data;
			}
			else
			{
				/* Bring the system up to now */
				stream_update(info->discrete_stream);

				context->data = new_data;

				/* Update the node output here so we don't have to do it each step */
				node->output[0] = new_data * context->gain + context->offset;
			}
		}
	}
	else
	{
		discrete_log(info, "discrete_sound_w write to non-existent NODE_%02d\n", offset-NODE_00);
	}
}
