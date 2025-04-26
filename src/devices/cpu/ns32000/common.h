// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_NS32000_COMMON_H
#define MAME_CPU_NS32000_COMMON_H

#pragma once

namespace ns32000
{
	enum st_mask : unsigned
	{
		ST_ICI =  0x0, // bus idle (CPU busy)
		ST_ICW =  0x1, // bus idle (CPU wait)
		ST_ISE =  0x3, // bus idle (slave execution)
		ST_IAM =  0x4, // interrupt acknowledge, master
		ST_IAC =  0x5, // interrupt acknowledge, cascaded
		ST_EIM =  0x6, // end of interrupt, master
		ST_EIC =  0x7, // end of interrupt, cascaded
		ST_SIF =  0x8, // sequential instruction fetch
		ST_NIF =  0x9, // non-sequential instruction fetch
		ST_ODT =  0xa, // operand data transfer
		ST_RMW =  0xb, // read RMW operand
		ST_EAR =  0xc, // effective address read
		ST_SOP =  0xd, // slave operand
		ST_SST =  0xe, // slave status
		ST_SID =  0xf, // slave ID

		ST_PT1 =  0xd, // access pte1 by MMU (32532)
		ST_PT2 =  0xe, // access pte2 by MMU (32532)
		ST_SLV = 0x10, // slave access ST4 (32532)
	};
}

class ns32000_slave_interface : public device_interface
{
public:
	auto out_spc() { return m_out_spc.bind(); }

	ns32000_slave_interface(machine_config const &mconfig, device_t &device)
		: ns32000_slave_interface(mconfig, device, "ns32000_slave")
	{
	}

	enum slave_idbyte : u8
	{
		FORMAT_9  = 0x3e, // fpu
		FORMAT_11 = 0xbe, // fpu
		FORMAT_12 = 0xfe, // fpu
		FORMAT_14 = 0x1e, // mmu
	};

	enum slave_status : u16
	{
		SLAVE_Q  = 0x0001, // quit (1=error)
		SLAVE_L  = 0x0004,
		SLAVE_F  = 0x0020,
		SLAVE_Z  = 0x0040,
		SLAVE_N  = 0x0080,
		SLAVE_TS = 0x8000, // trap status (1=UND, 0=FPU)

		SLAVE_OK = 0,
	};

	virtual void state_add(device_state_interface &parent, int &index) = 0;

protected:
	ns32000_slave_interface(machine_config const &mconfig, device_t &device, char const *type)
		: device_interface(device, type)
		, m_out_spc(*this)
	{
	}

	devcb_write_line m_out_spc;
};

class ns32000_fpu_interface : public ns32000_slave_interface
{
protected:
	ns32000_fpu_interface(machine_config const &mconfig, device_t &device)
		: ns32000_slave_interface(mconfig, device)
	{
	}
};

class ns32000_mmu_interface : public ns32000_slave_interface
{
public:
	enum translate_result : unsigned { COMPLETE, CANCEL, ABORT };
	virtual translate_result translate(address_space &space, unsigned st, u32 &address, bool user, bool write, bool flag = false, bool suppress = false) = 0;

protected:
	ns32000_mmu_interface(machine_config const &mconfig, device_t &device)
		: ns32000_slave_interface(mconfig, device)
	{
	}
};

class ns32000_slow_slave_interface : public device_interface
{
public:
	virtual u16 slow_status(int *icount = nullptr) = 0;
	virtual u16 slow_read() = 0;
	virtual void slow_write(u16 data) = 0;

protected:
	ns32000_slow_slave_interface(machine_config const &mconfig, device_t &device)
		: device_interface(device, "ns32000_slave_slow")
	{
	}
};

class ns32000_fast_slave_interface : public device_interface
{
public:
	virtual u32 fast_status(int *icount = nullptr) = 0;
	virtual u32 fast_read() = 0;
	virtual void fast_write(u32 data) = 0;

protected:
	ns32000_fast_slave_interface(machine_config const &mconfig, device_t &device)
		: device_interface(device, "ns32000_slave_fast")
	{
	}
};

#endif // MAME_CPU_NS32000_COMMON_H
