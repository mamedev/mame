// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud,Derrick Renaud,Frank Palazzolo
/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by K.Wilkins (mame@esplexo.co.uk)
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
 * Each sound primitive DSS_xxxx or DST_xxxx has its own implementation
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
 * device_start                - Read Node list, initialise & reset
 * device_stop                 - Shutdown discrete sound system
 * device_reset                - Put sound system back to time 0
 * discrete_stream_update()    - This does the real update to the sim
 *
 ************************************************************************/

#include "emu.h"
#include "discrete.h"

#include "wavwrite.h"

#include <atomic>
#include <cstdarg>
#include <iostream>


// device type definition
DEFINE_DEVICE_TYPE(DISCRETE, discrete_sound_device, "discrete", "Discrete Sound")

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

static constexpr int MAX_SAMPLES_PER_TASK_SLICE = 960 / 4;

/*************************************
 *
 *  Debugging
 *
 *************************************/

#define DISCRETE_DEBUGLOG           (0)

/*************************************
 *
 *  Use tasks ?
 *
 *************************************/

#define USE_DISCRETE_TASKS          (1)

/*************************************
 *
 *  Internal classes
 *
 *************************************/

struct output_buffer
{
	output_buffer() : source(nullptr), ptr(nullptr)
	{
	}

	// required for use in vector (std::atomic<T> has deleted copy constructor)
	output_buffer(output_buffer &&that) : node_buf(std::move(that.node_buf)), source(that.source), ptr(that.ptr.load(std::memory_order_relaxed)), node_num(that.node_num)
	{
	}

	std::unique_ptr<double []>  node_buf;
	const double *              source;
	std::atomic<double *>       ptr;
	int                         node_num;
};

struct input_buffer
{
	const double *              ptr;                // pointer into linked_outbuf.nodebuf
	output_buffer *             linked_outbuf;      // what output are we connected to?
	double                      buffer;             // input[] will point here
};

class discrete_task
{
public:
	discrete_task(discrete_device &pdev) : m_device(pdev), m_threadid(-1), m_samples(0)
	{
		// FIXME: the code expects to be able to take pointers to members of elements of this vector before it's filled
		source_list.reserve(16);
	}

	void check(discrete_task &dest_task);
	void prepare_for_queue(int samples);

	static void *task_callback(void *param, int threadid);

	//const linked_list_entry *list;
	discrete_device::node_step_list_t        step_list;

	int task_group = 0;

private:
	void step_nodes();
	bool process();

	bool lock_threadid(int32_t threadid)
	{
		int expected = -1;
		return m_threadid.compare_exchange_strong(expected, threadid, std::memory_order_acquire, std::memory_order_relaxed);
	}

	void unlock()
	{
		m_threadid.store(-1, std::memory_order_release);
	}

	/* list of source nodes */
	std::vector<input_buffer> source_list;      /* discrete_source_node */

	std::vector<output_buffer>  m_buffers;
	discrete_device &           m_device;

	std::atomic<int32_t>        m_threadid;
	int                         m_samples;
};


/*************************************
 *
 *  Included simulation objects
 *
 *************************************/

#include "disc_sys.hxx"       /* discrete core modules and support functions */
#include "disc_wav.hxx"       /* Wave sources   - SINE/SQUARE/NOISE/etc */
#include "disc_mth.hxx"       /* Math Devices   - ADD/GAIN/etc */
#include "disc_inp.hxx"       /* Input Devices  - INPUT/CONST/etc */
#include "disc_flt.hxx"       /* Filter Devices - RCF/HPF/LPF */
#include "disc_dev.hxx"       /* Popular Devices - NE555/etc */

/*************************************
 *
 *  INLINEs
 *
 *************************************/



/*************************************
 *
 *  Task implementation
 *
 *************************************/

inline void discrete_task::step_nodes()
{
	for (input_buffer &sn : source_list)
		sn.buffer = *sn.ptr++;

	if (EXPECTED(!m_device.profiling()))
	{
		// Now step the nodes
		for (discrete_step_interface *entry : step_list)
			entry->step();
	}
	else
	{
		osd_ticks_t last = get_profile_ticks();

		for (discrete_step_interface *node : step_list)
		{
			node->run_time -= last;
			node->step();
			last = get_profile_ticks();
			node->run_time += last;
		}
	}

	// buffer the outputs
	for (output_buffer &outbuf : m_buffers)
		*outbuf.ptr.load(std::memory_order_relaxed) = *outbuf.source;
	std::atomic_thread_fence(std::memory_order_release);
	for (output_buffer &outbuf : m_buffers)
		outbuf.ptr.store(outbuf.ptr.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
}

void *discrete_task::task_callback(void *param, int threadid)
{
	const auto &list = *reinterpret_cast<const discrete_sound_device::task_list_t *>(param);
	while (true)
	{
		for (const auto &task : list)
		{
			// try to lock
			if (task->lock_threadid(threadid))
			{
				if (!task->process())
					return nullptr;
				task->unlock();
			}
		}
	}

	return nullptr;
}

inline bool discrete_task::process()
{
	int samples = std::min(m_samples, MAX_SAMPLES_PER_TASK_SLICE);

	// check dependencies
	for (input_buffer &sn : source_list)
	{
		const int avail = sn.linked_outbuf->ptr.load(std::memory_order_relaxed) - sn.ptr;
		if (avail < 0)
			throw emu_fatalerror("discrete_task::process: available samples are negative");
		if (avail < samples)
			samples = avail;
	}
	std::atomic_thread_fence(std::memory_order_acquire);

	m_samples -= samples;
	if (m_samples < 0)
		throw emu_fatalerror("discrete_task::process: m_samples got negative");
	while (samples > 0)
	{
		// step
		step_nodes();
		samples--;
	}
	if (m_samples == 0)
	{
		// return and keep the task locked so it is not picked up by other worker threads
		return false;
	}
	return true;
}

void discrete_task::prepare_for_queue(int samples)
{
	m_threadid.store(-1, std::memory_order_relaxed); // unlock the thread
	m_samples = samples;

	// set up task buffers
	for (output_buffer &ob : m_buffers)
		ob.ptr.store(ob.node_buf.get(), std::memory_order_relaxed);

	// initialize sources
	for (input_buffer &sn : source_list)
		sn.ptr = sn.linked_outbuf->node_buf.get();
}

void discrete_task::check(discrete_task &dest_task)
{
	// FIXME: this function takes addresses of elements of a vector that has items added later
	// 16 is enough for the systems in MAME, but the code should be fixed properly
	m_buffers.reserve(16);

	/* Determine, which nodes in the task are referenced by nodes in dest_task
	 * and add them to the list of nodes to be buffered for further processing
	 */
	for (discrete_step_interface *node_entry : step_list)
	{
		discrete_base_node *task_node = node_entry->self;

		for (discrete_step_interface *step_entry : dest_task.step_list)
		{
			discrete_base_node *dest_node = step_entry->self;

			/* loop over all active inputs */
			for (int inputnum = 0; inputnum < dest_node->active_inputs(); inputnum++)
			{
				int inputnode_num = dest_node->input_node(inputnum);
				if IS_VALUE_A_NODE(inputnode_num)
				{
					/* FIXME: sub nodes ! */
					if (NODE_DEFAULT_NODE(task_node->block_node()) == NODE_DEFAULT_NODE(inputnode_num))
					{
						int found = -1;
						output_buffer *pbuf = nullptr;

						for (int i = 0; i < m_buffers.size(); i++)
//                          if (m_buffers[i].node->block_node() == inputnode_num)
							if (m_buffers[i].node_num == inputnode_num)
							{
								found = i;
								pbuf = &m_buffers[i];
								break;
							}

						if (found < 0)
						{
							output_buffer buf;

							buf.node_buf = std::make_unique<double []>((task_node->sample_rate() + sound_manager::STREAMS_UPDATE_FREQUENCY) / sound_manager::STREAMS_UPDATE_FREQUENCY);
							buf.ptr.store(buf.node_buf.get(), std::memory_order_relaxed);
							buf.source = dest_node->m_input[inputnum];
							buf.node_num = inputnode_num;
							//buf.node = device->discrete_find_node(inputnode);
							m_buffers.emplace_back(std::move(buf));
							pbuf = &m_buffers.back();
						}
						m_device.discrete_log("dso_task_start - buffering %d(%d) in task %p group %d referenced by %d group %d", NODE_INDEX(inputnode_num), NODE_CHILD_NODE_NUM(inputnode_num), this, task_group, dest_node->index(), dest_task.task_group);

						/* register into source list */
						dest_task.source_list.push_back(input_buffer{ nullptr, pbuf, 0.0 });
						// FIXME: taking address of element of vector before it's filled
						dest_node->m_input[inputnum] = &dest_task.source_list.back().buffer;
					}
				}
			}
		}
	}
}

/*************************************
 *
 *  Base node implementation
 *
 *************************************/

discrete_base_node::discrete_base_node() :
	m_device(nullptr),
	m_block(nullptr),
	m_active_inputs(0),
	m_custom(nullptr),
	m_input_is_node(0),
	m_step_intf(nullptr),
	m_input_intf(nullptr),
	m_output_intf(nullptr)
{
	std::fill(std::begin(m_output), std::end(m_output), 0.0);
}


discrete_base_node::~discrete_base_node()
{
	/* currently noting */
}

void discrete_base_node::init(discrete_device *pdev, const discrete_block *xblock)
{
	m_device = pdev;
	m_block = xblock;

	m_custom = m_block->custom;
	m_active_inputs = m_block->active_inputs;

	m_step_intf = dynamic_cast<discrete_step_interface *>(this);
	m_input_intf = dynamic_cast<discrete_input_interface *>(this);
	m_output_intf = dynamic_cast<discrete_sound_output_interface *>(this);

	if (m_step_intf)
	{
		m_step_intf->run_time = 0;
		m_step_intf->self = this;
	}
}

void discrete_base_node::save_state()
{
	if (m_block->node != NODE_SPECIAL)
		m_device->save_item(NAME(m_output), m_block->node);
}

discrete_base_node *discrete_device::discrete_find_node(int node)
{
	if (node < NODE_START || node > NODE_END) return nullptr;
	return m_indexed_node[NODE_INDEX(node)];
}

void discrete_base_node::resolve_input_nodes()
{
	int inputnum;

	/* loop over all active inputs */
	for (inputnum = 0; inputnum < m_active_inputs; inputnum++)
	{
		int inputnode = m_block->input_node[inputnum];

		/* if this input is node-based, find the node in the indexed list */
		if IS_VALUE_A_NODE(inputnode)
		{
			//discrete_base_node *node_ref = m_device->m_indexed_node[NODE_INDEX(inputnode)];
			discrete_base_node *node_ref = m_device->discrete_find_node(inputnode);
			if (!node_ref)
				fatalerror("discrete_start - NODE_%02d referenced a non existent node NODE_%02d\n", index(), NODE_INDEX(inputnode));

			if ((NODE_CHILD_NODE_NUM(inputnode) >= node_ref->max_output()) /*&& (node_ref->module_type() != DST_CUSTOM)*/)
				fatalerror("discrete_start - NODE_%02d referenced non existent output %d on node NODE_%02d\n", index(), NODE_CHILD_NODE_NUM(inputnode), NODE_INDEX(inputnode));

			m_input[inputnum] = &(node_ref->m_output[NODE_CHILD_NODE_NUM(inputnode)]);  /* Link referenced node out to input */
			m_input_is_node |= 1 << inputnum;           /* Bit flag if input is node */
		}
		else
		{
			/* warn if trying to use a node for an input that can only be static */
			if IS_VALUE_A_NODE(m_block->initial[inputnum])
			{
				m_device->discrete_log("Warning - discrete_start - NODE_%02d trying to use a node on static input %d",  index(), inputnum);
				/* also report it in the error log so it is not missed */
				m_device->logerror("Warning - discrete_start - NODE_%02d trying to use a node on static input %d",  index(), inputnum);
			}
			else
			{
				m_input[inputnum] = &(m_block->initial[inputnum]);
			}
		}
	}
	for (inputnum = m_active_inputs; inputnum < DISCRETE_MAX_INPUTS; inputnum++)
	{
		/* FIXME: Check that no nodes follow ! */
		m_input[inputnum] = &(m_block->initial[inputnum]);
	}
}

const double *discrete_device::node_output_ptr(int onode)
{
	const discrete_base_node *node;
	node = discrete_find_node(onode);

	if (node != nullptr)
	{
		return &(node->m_output[NODE_CHILD_NODE_NUM(onode)]);
	}
	else
		return nullptr;
}

/*************************************
 *
 *  Device implementation
 *
 *************************************/


//-------------------------------------------------
//  discrete_log: Debug logging
//-------------------------------------------------

void CLIB_DECL discrete_device::discrete_log(const char *text, ...) const
{
	if (DISCRETE_DEBUGLOG)
	{
		va_list arg;
		va_start(arg, text);

		if(m_disclogfile)
		{
			vfprintf(m_disclogfile, text, arg);
			fprintf(m_disclogfile, "\n");
			fflush(m_disclogfile);
		}

		va_end(arg);
	}
}

//-------------------------------------------------
//  discrete_build_list: Build import list
//-------------------------------------------------

void discrete_device::discrete_build_list(const discrete_block *intf, sound_block_list_t &block_list)
{
	int node_count = 0;

	for (; intf[node_count].type != DSS_NULL; )
	{
		/* scan imported */
		if (intf[node_count].type == DSO_IMPORT)
		{
			discrete_log("discrete_build_list() - DISCRETE_IMPORT @ NODE_%02d", NODE_INDEX(intf[node_count].node) );
			discrete_build_list((discrete_block *) intf[node_count].custom, block_list);
		}
		else if (intf[node_count].type == DSO_REPLACE)
		{
			bool found = false;
			node_count++;
			if (intf[node_count].type == DSS_NULL)
				fatalerror("discrete_build_list: DISCRETE_REPLACE at end of node_list\n");

			for (int i=0; i < block_list.size(); i++)
			{
				const discrete_block *block = block_list[i];

				if (block->type != NODE_SPECIAL )
					if (block->node == intf[node_count].node)
					{
						block_list[i] = &intf[node_count];
						discrete_log("discrete_build_list() - DISCRETE_REPLACE @ NODE_%02d", NODE_INDEX(intf[node_count].node) );
						found = true;
						break;
					}
			}

			if (!found)
				fatalerror("discrete_build_list: DISCRETE_REPLACE did not found node %d\n", NODE_INDEX(intf[node_count].node));

		}
		else if (intf[node_count].type == DSO_DELETE)
		{
			std::vector<int> deletethem;

			for (int i=0; i<block_list.size(); i++)
			{
				const discrete_block *block = block_list[i];

				if ((block->node >= intf[node_count].input_node[0]) &&
						(block->node <= intf[node_count].input_node[1]))
				{
					discrete_log("discrete_build_list() - DISCRETE_DELETE deleted NODE_%02d", NODE_INDEX(block->node) );
					deletethem.push_back(i);
				}
			}
			for (int i : deletethem)
				block_list.erase(block_list.begin() + i); // FIXME: how is this supposed to work if there's more than one item to remove?  indices are shifted back on each removal
		}
		else
		{
			discrete_log("discrete_build_list() - adding node %d\n", node_count);
			block_list.push_back(&intf[node_count]);
		}

		node_count++;
	}
}

//-------------------------------------------------
// discrete_sanity_check: Sanity check list
//-------------------------------------------------

void discrete_device::discrete_sanity_check(const sound_block_list_t &block_list)
{
	int node_count = 0;

	discrete_log("discrete_start() - Doing node list sanity check");
	for (int i=0; i < block_list.size(); i++)
	{
		const discrete_block *block = block_list[i];

		/* make sure we don't have too many nodes overall */
		if (node_count > DISCRETE_MAX_NODES)
			fatalerror("discrete_start() - Upper limit of %d nodes exceeded, have you terminated the interface block?\n", DISCRETE_MAX_NODES);

		/* make sure the node number is in range */
		if (block->node < NODE_START || block->node > NODE_END)
			fatalerror("discrete_start() - Invalid node number on node %02d descriptor\n", block->node);

		/* make sure the node type is valid */
		if (block->type > DSO_OUTPUT)
			fatalerror("discrete_start() - Invalid function type on NODE_%02d\n", NODE_INDEX(block->node) );

		/* make sure this is a main node */
		if (NODE_CHILD_NODE_NUM(block->node) > 0)
			fatalerror("discrete_start() - Child node number on NODE_%02d\n", NODE_INDEX(block->node) );

		node_count++;
	}
	discrete_log("discrete_start() - Sanity check counted %d nodes", node_count);

}

//-------------------------------------------------
// discrete_sanity_check: Sanity check list
//-------------------------------------------------

/*************************************
 *
 *  Master discrete system start
 *
 *************************************/


/*************************************
 *
 *  Master discrete system stop
 *
 *************************************/

static uint64_t list_run_time(const discrete_device::node_list_t &list)
{
	uint64_t total = 0;

	for (const auto &node : list)
	{
		discrete_step_interface *step;
		if (node->interface(step))
			total += step->run_time;
	}
	return total;
}

static uint64_t step_list_run_time(const discrete_device::node_step_list_t &list)
{
	uint64_t total = 0;

	for (discrete_step_interface *node : list)
	{
		total += node->run_time;
	}
	return total;
}

void discrete_device::display_profiling()
{
	int count;
	uint64_t total;
	uint64_t tresh;

	/* calculate total time */
	total = list_run_time(m_node_list);
	count = m_node_list.size();
	/* print statistics */
	osd_printf_info("Total Samples  : %16d\n", m_total_samples);
	tresh = total / count;
	osd_printf_info("Threshold (mean): %16d\n", tresh / m_total_samples);
	for (const auto &node : m_node_list)
	{
		discrete_step_interface *step;
		if (node->interface(step))
			if (step->run_time > tresh)
				osd_printf_info("%3d: %20s %8.2f %10.2f\n", node->index(), node->module_name(), double(step->run_time) / double(total) * 100.0, double(step->run_time) / double(m_total_samples));
	}

	/* Task information */
	for (const auto &task : task_list)
	{
		double tt = step_list_run_time(task->step_list);

		osd_printf_info("Task(%d): %8.2f %15.2f\n", task->task_group, tt / double(total) * 100.0, tt / double(m_total_samples));
	}

	osd_printf_info("Average samples/double->update: %8.2f\n", double(m_total_samples) / double(m_total_stream_updates));
}


/*************************************
 *
 *  First pass init of nodes
 *
 *************************************/


void discrete_device::init_nodes(const sound_block_list_t &block_list)
{
	discrete_task *task = nullptr;
	/* list tail pointers */
	bool has_tasks = false;

	/* check whether we have tasks ... */
	if (USE_DISCRETE_TASKS)
	{
		for (int i = 0; !has_tasks && (i < block_list.size()); i++)
		{
			if (block_list[i]->type == DSO_TASK_START)
				has_tasks = true;
		}
	}

	if (!has_tasks)
	{
		/* make sure we have one simple task
		 * No need to create a node since there are no dependencies.
		 */
		task_list.push_back(std::make_unique<discrete_task>(*this));
		task = task_list.back().get();
	}

	/* loop over all nodes */
	for (int i = 0; i < block_list.size(); i++)
	{
		const discrete_block &block = *block_list[i];

		// add to node list
		m_node_list.push_back(block.factory(*this, block));
		discrete_base_node &node = *m_node_list.back();

		if (block.node == NODE_SPECIAL)
		{
			// keep track of special nodes
			switch (block.type)
			{
				/* Output Node */
				case DSO_OUTPUT:
					/* nothing -> handled later */
					break;

				/* CSVlog Node for debugging */
				case DSO_CSVLOG:
					break;

				/* Wavelog Node for debugging */
				case DSO_WAVLOG:
					break;

				/* Task processing */
				case DSO_TASK_START:
					if (USE_DISCRETE_TASKS)
					{
						if (task != nullptr)
							fatalerror("init_nodes() - Nested DISCRETE_START_TASK.\n");
						task_list.push_back(std::make_unique<discrete_task>(*this));
						task = task_list.back().get();
						task->task_group = block.initial[0];
						if (task->task_group < 0 || task->task_group >= DISCRETE_MAX_TASK_GROUPS)
							fatalerror("discrete_dso_task: illegal task_group %d\n", task->task_group);
						//logerror("task group %d\n", task->task_group);
					}
					break;

				case DSO_TASK_END:
					if (USE_DISCRETE_TASKS)
					{
						if (task == nullptr)
							fatalerror("init_nodes() - NO DISCRETE_START_TASK.\n");
					}
					break;

				default:
					fatalerror("init_nodes() - Failed, trying to create unknown special discrete node.\n");
			}
		}
		else
		{
			// otherwise, make sure we are not a duplicate, and put ourselves into the indexed list
			if (m_indexed_node[NODE_INDEX(block.node)])
				fatalerror("init_nodes() - Duplicate entries for NODE_%02d\n", NODE_INDEX(block.node));
			m_indexed_node[NODE_INDEX(block.node)] = &node;
		}

		// our running order just follows the order specified
		// does the node step?
		discrete_step_interface *step;
		if (node.interface(step))
		{
			/* do we belong to a task? */
			if (task == nullptr)
				fatalerror("init_nodes() - found node outside of task: %s\n", node.module_name());
			else
				task->step_list.push_back(step);
		}

		if (USE_DISCRETE_TASKS &&  block.type == DSO_TASK_END)
		{
			task = nullptr;
		}

		/* and register save state */
		node.save_state();
	}

	if (!has_tasks)
	{
	}
}


/*************************************
 *
 *  node_description implementation
 *
 *************************************/


int discrete_device::same_module_index(const discrete_base_node &node)
{
	int index = 0;

	for (const auto &n : m_node_list)
	{
		if (n.get() == &node)
			return index;
		if (n->module_type() == node.module_type())
			index++;
	}
	return -1;
}


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  discrete_device - constructor
//-------------------------------------------------

discrete_device::discrete_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		m_intf(nullptr),
		m_sample_rate(0),
		m_sample_time(0),
		m_neg_sample_time(0),
		m_indexed_node(nullptr),
		m_disclogfile(nullptr),
		m_queue(nullptr),
		m_profiling(0),
		m_total_samples(0),
		m_total_stream_updates(0)
{
}

discrete_sound_device::discrete_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: discrete_device(mconfig, DISCRETE, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr)
{
}

discrete_device::~discrete_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void discrete_device::device_start()
{
	// create the stream
	//m_stream = stream_alloc(0, 2, 22257);

	const discrete_block *intf_start = m_intf;

	/* If a clock is specified we will use it, otherwise run at the audio sample rate. */
	if (this->clock())
		m_sample_rate = this->clock();
	else
		m_sample_rate = this->machine().sample_rate();
	m_sample_time = 1.0 / m_sample_rate;
	m_neg_sample_time = - m_sample_time;

	m_total_samples = 0;
	m_total_stream_updates = 0;

	/* create the logfile */
	if (DISCRETE_DEBUGLOG)
		m_disclogfile = fopen(util::string_format("discrete%s.log", this->tag()).c_str(), "w");

	/* enable profiling */
	m_profiling = 0;
	if (osd_getenv("DISCRETE_PROFILING"))
		m_profiling = atoi(osd_getenv("DISCRETE_PROFILING"));

	/* Build the final block list */
	sound_block_list_t block_list;
	discrete_build_list(intf_start, block_list);

	/* first pass through the nodes: sanity check, fill in the indexed_nodes, and make a total count */
	discrete_sanity_check(block_list);

	/* Start with empty lists */
	m_node_list.clear();

	/* allocate memory to hold pointers to nodes by index */
	m_indexed_node = make_unique_clear<discrete_base_node * []>(DISCRETE_MAX_NODES);

	/* initialize the node data */
	init_nodes(block_list);

	/* now go back and find pointers to all input nodes */
	for (const auto &node : m_node_list)
	{
		node->resolve_input_nodes();
	}

	/* allocate a queue */
	m_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI | WORK_QUEUE_FLAG_HIGH_FREQ);

	/* Process nodes which have a start func */
	for (const auto &node : m_node_list)
	{
		node->start();
	}

	/* Now set up tasks */
	for (const auto &task : task_list)
	{
		for (const auto &dest_task : task_list)
		{
			if (task->task_group > dest_task->task_group)
				dest_task->check(*task);
		}
	}
}

void discrete_device::device_stop()
{
	if (m_queue)
	{
		osd_work_queue_free(m_queue);
	}

	if (m_profiling)
	{
		display_profiling();
	}

	/* Process nodes which have a stop func */

	for (const auto &node : m_node_list)
	{
		node->stop();
	}

	if (DISCRETE_DEBUGLOG)
	{
		/* close the debug log */
		if (m_disclogfile)
			fclose(m_disclogfile);
		m_disclogfile = nullptr;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void discrete_sound_device::device_start()
{
	m_input_stream_list.clear();
	m_output_list.clear();

	/* call the parent */
	discrete_device::device_start();

	/* look for input stream nodes */
	for (const auto &node : m_node_list)
	{
		/* if we are an stream input node, track that */
		discrete_dss_input_stream_node *input_stream = dynamic_cast<discrete_dss_input_stream_node *>(node.get());
		if (input_stream != nullptr)
		{
			m_input_stream_list.push_back(input_stream);
		}
		/* if this is an output interface, add it the output list */
		discrete_sound_output_interface *out;
		if (node->interface(out))
			m_output_list.push_back(out);
	}

	/* if no outputs, give an error */
	if (m_output_list.empty())
		fatalerror("init_nodes() - Couldn't find an output node\n");

	/* initialize the stream(s) */
	m_stream = stream_alloc(m_input_stream_list.size(), m_output_list.size(), m_sample_rate);

	/* Finalize stream_input_nodes */
	for (discrete_dss_input_stream_node *node : m_input_stream_list)
	{
		node->stream_start();
	}


}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void discrete_device::device_reset()
{
	update_to_current_time();

	/* loop over all nodes */
	for (const auto &node : m_node_list)
	{
		/* Fimxe : node_level */
		node->m_output[0] = 0;

		node->reset();
	}
}

void discrete_sound_device::device_reset()
{
	discrete_device::device_reset();
}

//-------------------------------------------------
//  discrete_device_process - process a number of
//  samples.
//
//  input / output buffers are s32
//  to not to have to convert the buffers.
//  a "discrete cpu" device will pass nullptr here
//-------------------------------------------------

void discrete_device::process(int samples)
{
	if (samples == 0)
		return;

	// Set up tasks
	for (const auto &task : task_list)
		task->prepare_for_queue(samples);
	std::atomic_thread_fence(std::memory_order_release);

	for (const auto &task : task_list)
	{
		// Fire a work item for each task
		(void)task;
		osd_work_item_queue(m_queue, discrete_task::task_callback, (void *)&task_list, WORK_ITEM_FLAG_AUTO_RELEASE);
	}
	osd_work_queue_wait(m_queue, osd_ticks_per_second() * 10);

	if (m_profiling)
	{
		m_total_samples += samples;
		m_total_stream_updates++;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void discrete_sound_device::sound_stream_update(sound_stream &stream)
{
	int outputnum = 0;

	/* Setup any output streams */
	for (discrete_sound_output_interface *node : m_output_list)
	{
		node->set_output_ptr(stream, outputnum);
		outputnum++;
	}

	/* Setup any input streams */
	for (discrete_dss_input_stream_node *node : m_input_stream_list)
	{
		node->m_inview_sample = 0;
	}

	/* just process it */
	process(stream.samples());
}

//-------------------------------------------------
//  read - read from the chip's registers and internal RAM
//-------------------------------------------------

uint8_t discrete_device::read(offs_t offset)
{
	const discrete_base_node *node = discrete_find_node(offset);

	uint8_t data;

	/* Read the node input value if allowed */
	if (node)
	{
		/* Bring the system up to now */
		update_to_current_time();

		data = (uint8_t) node->m_output[NODE_CHILD_NODE_NUM(offset)];
	}
	else
		fatalerror("discrete_sound_r read from non-existent NODE_%02d\n", offset-NODE_00);

	return data;
}

//-------------------------------------------------
//  write - write to the chip's registers and internal RAM
//-------------------------------------------------

void discrete_device::write(offs_t offset, uint8_t data)
{
	const discrete_base_node *node = discrete_find_node(offset);

	/* Update the node input value if it's a proper input node */
	if (node)
	{
		discrete_input_interface *intf;
		if (node->interface(intf))
				intf->input_write(0, data);
		else
			discrete_log("discrete_sound_w write to non-input NODE_%02d\n", offset-NODE_00);
	}
	else
	{
		discrete_log("discrete_sound_w write to non-existent NODE_%02d\n", offset-NODE_00);
	}
}
