// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS40 MCU family cores

*/

#ifndef _HMCS40_H_
#define _HMCS40_H_

#include "emu.h"


// I/O ports setup

// max 8 4-bit R ports
#define MCFG_HMCS40_READ_R_CB(R, _devcb) \
	hmcs40_cpu_device::set_read_r##R##_callback(*device, DEVCB_##_devcb);
#define MCFG_HMCS40_WRITE_R_CB(R, _devcb) \
	hmcs40_cpu_device::set_write_r##R##_callback(*device, DEVCB_##_devcb);

// 16-bit discrete
#define MCFG_HMCS40_READ_D_CB(_devcb) \
	hmcs40_cpu_device::set_read_d_callback(*device, DEVCB_##_devcb);
#define MCFG_HMCS40_WRITE_D_CB(_devcb) \
	hmcs40_cpu_device::set_write_d_callback(*device, DEVCB_##_devcb);



class hmcs40_cpu_device : public cpu_device
{
public:
	// construction/destruction
	hmcs40_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int family, bool is_cmos, int stack_levels, int pcwidth, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_LITTLE, 16, prgwidth, -1, program)
		, m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data)
		, m_pcwidth(pcwidth)
		, m_prgwidth(prgwidth)
		, m_datawidth(datawidth)
		, m_family(family)
		, m_is_cmos(is_cmos)
		, m_stack_levels(stack_levels)
		, m_read_r0(*this), m_read_r1(*this), m_read_r2(*this), m_read_r3(*this), m_read_r4(*this), m_read_r5(*this), m_read_r6(*this), m_read_r7(*this)
		, m_write_r0(*this), m_write_r1(*this), m_write_r2(*this), m_write_r3(*this), m_write_r4(*this), m_write_r5(*this), m_write_r6(*this), m_write_r7(*this)
		, m_read_d(*this)
		, m_write_d(*this)
	{ }

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_r0_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_read_r0.set_callback(object); }
	template<class _Object> static devcb_base &set_read_r1_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_read_r1.set_callback(object); }
	template<class _Object> static devcb_base &set_read_r2_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_read_r2.set_callback(object); }
	template<class _Object> static devcb_base &set_read_r3_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_read_r3.set_callback(object); }
	template<class _Object> static devcb_base &set_read_r4_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_read_r4.set_callback(object); }
	template<class _Object> static devcb_base &set_read_r5_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_read_r5.set_callback(object); }
	template<class _Object> static devcb_base &set_read_r6_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_read_r6.set_callback(object); }
	template<class _Object> static devcb_base &set_read_r7_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_read_r7.set_callback(object); }

	template<class _Object> static devcb_base &set_write_r0_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_write_r0.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r1_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_write_r1.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r2_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_write_r2.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r3_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_write_r3.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r4_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_write_r4.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r5_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_write_r5.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r6_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_write_r6.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r7_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_write_r7.set_callback(object); }

	template<class _Object> static devcb_base &set_read_d_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_read_d.set_callback(object); }
	template<class _Object> static devcb_base &set_write_d_callback(device_t &device, _Object object) { return downcast<hmcs40_cpu_device &>(device).m_write_d.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 2; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return(spacenum == AS_PROGRAM) ? &m_program_config :((spacenum == AS_DATA) ? &m_data_config : NULL); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	void state_string_export(const device_state_entry &entry, astring &string);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_pcwidth;      // Program Counter bit-width
	int m_prgwidth;
	int m_datawidth;
	int m_pcmask;
	int m_prgmask;
	int m_datamask;
	int m_family;       // MCU family (42-47)
	bool m_is_cmos;
	int m_stack_levels; // number of callstack levels
	UINT16 m_stack[4];  // max 4
	UINT16 m_op;        // current opcode
	UINT16 m_prev_op;
	int m_icount;
	
	UINT16 m_pc;        // Program Counter
	UINT16 m_prev_pc;
	UINT8 m_page;       // LPU prepared page
	UINT8 m_a;          // 4-bit Accumulator
	UINT8 m_b;          // 4-bit B register
	UINT8 m_x;          // 1/3/4-bit X register
	UINT8 m_spx;        // 1/3/4-bit SPX register
	UINT8 m_y;          // 4-bit Y register
	UINT8 m_spy;        // 4-bit SPY register
	UINT8 m_s;          // Status F/F
	UINT8 m_c;          // Carry F/F
	UINT8 m_r[8];       // R outputs state
	UINT16 m_d;         // D pins state

	// i/o handlers
	devcb_read8 m_read_r0, m_read_r1, m_read_r2, m_read_r3, m_read_r4, m_read_r5, m_read_r6, m_read_r7;
	devcb_write8 m_write_r0, m_write_r1, m_write_r2, m_write_r3, m_write_r4, m_write_r5, m_write_r6, m_write_r7;
	devcb_read16 m_read_d;
	devcb_write16 m_write_d;

	// misc internal helpers
	void increment_pc();
	
	UINT8 ram_r();
	void ram_w(UINT8 data);
	void pop_stack();
	void push_stack();

	virtual UINT8 read_r(int index);
	virtual void write_r(int index, UINT8 data);
	virtual int read_d(int index);
	virtual void write_d(int index, int state);

	// opcode handlers
	void op_illegal();

	void op_lab();
	void op_lba();
	void op_lay();
	void op_laspx();
	void op_laspy();
	void op_xamr();

	void op_lxa();
	void op_lya();
	void op_lxi();
	void op_lyi();
	void op_iy();
	void op_dy();
	void op_ayy();
	void op_syy();
	void op_xsp();

	void op_lam();
	void op_lbm();
	void op_xma();
	void op_xmb();
	void op_lmaiy();
	void op_lmady();

	void op_lmiiy();
	void op_lai();
	void op_lbi();

	void op_ai();
	void op_ib();
	void op_db();
	void op_amc();
	void op_smc();
	void op_am();
	void op_daa();
	void op_das();
	void op_nega();
	void op_comb();
	void op_sec();
	void op_rec();
	void op_tc();
	void op_rotl();
	void op_rotr();
	void op_or();

	void op_mnei();
	void op_ynei();
	void op_anem();
	void op_bnem();
	void op_alei();
	void op_alem();
	void op_blem();

	void op_sem();
	void op_rem();
	void op_tm();

	void op_br();
	void op_cal();
	void op_lpu();
	void op_tbr();
	void op_rtn();

	void op_seie();
	void op_seif0();
	void op_seif1();
	void op_setf();
	void op_secf();
	void op_reie();
	void op_reif0();
	void op_reif1();
	void op_retf();
	void op_recf();
	void op_ti0();
	void op_ti1();
	void op_tif0();
	void op_tif1();
	void op_ttf();
	void op_lti();
	void op_lta();
	void op_lat();
	void op_rtni();

	void op_sed();
	void op_red();
	void op_td();
	void op_sedd();
	void op_redd();
	void op_lar();
	void op_lbr();
	void op_lra();
	void op_lrb();
	void op_p();
};


class hmcs43_cpu_device : public hmcs40_cpu_device
{
public:
	hmcs43_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_cmos, const char *shortname);

protected:
	// overrides
	virtual UINT8 read_r(int index);
	virtual void write_r(int index, UINT8 data);
	virtual int read_d(int index);
};

class hd38750_device : public hmcs43_cpu_device
{
public:
	hd38750_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class hd38755_device : public hmcs43_cpu_device
{
public:
	hd38755_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class hd44750_device : public hmcs43_cpu_device
{
public:
	hd44750_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class hd44758_device : public hmcs43_cpu_device
{
public:
	hd44758_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class hmcs44_cpu_device : public hmcs40_cpu_device
{
public:
	hmcs44_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_cmos, const char *shortname);

protected:
	// overrides
	virtual UINT8 read_r(int index);
	virtual void write_r(int index, UINT8 data);
};

class hd38800_device : public hmcs44_cpu_device
{
public:
	hd38800_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class hd38805_device : public hmcs44_cpu_device
{
public:
	hd38805_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class hd44801_device : public hmcs44_cpu_device
{
public:
	hd44801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class hd44808_device : public hmcs44_cpu_device
{
public:
	hd44808_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class hmcs45_cpu_device : public hmcs40_cpu_device
{
public:
	hmcs45_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_cmos, const char *shortname);

protected:
	// overrides
	virtual UINT8 read_r(int index);
	virtual void write_r(int index, UINT8 data);
};

class hd38820_device : public hmcs45_cpu_device
{
public:
	hd38820_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class hd38825_device : public hmcs45_cpu_device
{
public:
	hd38825_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class hd44820_device : public hmcs45_cpu_device
{
public:
	hd44820_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class hd44828_device : public hmcs45_cpu_device
{
public:
	hd44828_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



extern const device_type HD38750;
extern const device_type HD38755;
extern const device_type HD44750;
extern const device_type HD44758;

extern const device_type HD38800;
extern const device_type HD38805;
extern const device_type HD44801;
extern const device_type HD44808;

extern const device_type HD38820;
extern const device_type HD38825;
extern const device_type HD44820;
extern const device_type HD44828;


#endif /* _HMCS40_H_ */
