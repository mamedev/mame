// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    network.cpp

    Network output interface.

*******************************************************************c********/

#include "output_module.h"
#include "modules/osdmodule.h"
#include <uv.h>

#include <mutex>
#include <thread>

class output_network : public osd_module, public output_module
{
public:
	output_network()
	: osd_module(OSD_OUTPUT_PROVIDER, "network"), output_module(), m_loop(nullptr)
	{
	}
	virtual ~output_network() {
	}

	virtual int init(const osd_options &options) override { 
		m_loop = new uv_loop_t;
		int err = uv_loop_init(m_loop);
		if (err) {
			return 1;
		}
		
		m_working_thread = std::thread([](output_network* self) { self->process_output(); }, this);
		return 0; 
	}
	virtual void exit() override { 
		m_working_thread.join(); 
		uv_loop_close(m_loop);
		delete m_loop;
	}

	// output_module

	virtual void notify(const char *outname, INT32 value) override { }

	// implementation
	void process_output()
	{
		uv_run(m_loop, UV_RUN_DEFAULT);
	}
	
private:
	std::thread m_working_thread;
	uv_loop_t* m_loop;
};

MODULE_DEFINITION(OUTPUT_NETWORK, output_network)
