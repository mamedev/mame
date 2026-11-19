// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD parallel cable (RAMLink <-> CMD HD) emulation

**********************************************************************

                   _PCLK       1
                     GND       2
                   _PEXT       3
                     PD1       4
                     PD4       5
                     GND       6
                   _PRDY       7
                     PD0       8
                     PD7       9
                   _PATN      10
                     PD5      11
                     PD3      12
                     PD6      13
                     PD2      14

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_CMDPAR_H
#define MAME_BUS_CBMIEC_CMDPAR_H

#pragma once

#include <vector>



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_cmd_parallel_interface

class device_cmd_parallel_interface : public device_interface
{
public:
	enum : u8
	{
		PRDY = 0x01,
		PCLK = 0x02,
		PATN = 0x04,
		PEXT = 0x08
	};

	virtual ~device_cmd_parallel_interface();

protected:
	device_cmd_parallel_interface(const machine_config &mconfig, device_t &device);

	// device_interface implementation
	virtual void interface_pre_start() override ATTR_COLD;
	virtual void interface_post_start() override ATTR_COLD;

	// open collector outputs, 1 = released
	void cmdpar_data_w(u8 data);
	void cmdpar_lines_w(u8 lines);

	// wired-AND cable state
	u8 cmdpar_data_r() const;
	u8 cmdpar_lines_r() const;

	// call before a CPU accesses the port; returns false after setting up a retry of the access
	bool cmdpar_sample_ready();

	virtual void cmdpar_lines_changed(u8 lines, u8 old);

private:
	std::vector<device_cmd_parallel_interface *> m_peers;
	std::vector<device_execute_interface *> m_execs;

	u8 m_data_out;
	u8 m_lines_out;
};


#endif // MAME_BUS_CBMIEC_CMDPAR_H
