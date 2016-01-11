// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef __SCMP_H__
#define __SCMP_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	SCMP_PC, SCMP_P1, SCMP_P2, SCMP_P3, SCMP_AC, SCMP_ER, SCMP_SR,
	SCMP_GENPC = STATE_GENPC,
	SCMP_GENSP = STATE_GENSP,
	SCMP_GENPCBASE = STATE_GENPCBASE
};


#define MCFG_SCMP_CONFIG(_flag_out_devcb, _sout_devcb, _sin_devcb, _sensea_devcb, _senseb_devcb, _halt_devcb) \
	scmp_device::set_flag_out_cb(*device, DEVCB_##_flag_out_devcb); \
	scmp_device::set_sout_cb(*device, DEVCB_##_sout_devcb); \
	scmp_device::set_sin_cb(*device, DEVCB_##_sin_devcb); \
	scmp_device::set_sensea_cb(*device, DEVCB_##_sensea_devcb); \
	scmp_device::set_senseb_cb(*device, DEVCB_##_senseb_devcb); \
	scmp_device::set_halt_cb(*device, DEVCB_##_halt_devcb);


class scmp_device : public cpu_device
{
public:
	// construction/destruction
	scmp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	scmp_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// static configuration helpers
	template<class _Object> static devcb_base &set_flag_out_cb(device_t &device, _Object object) { return downcast<scmp_device &>(device).m_flag_out_func.set_callback(object); }
	template<class _Object> static devcb_base &set_sout_cb(device_t &device, _Object object) { return downcast<scmp_device &>(device).m_sout_func.set_callback(object); }
	template<class _Object> static devcb_base &set_sin_cb(device_t &device, _Object object) { return downcast<scmp_device &>(device).m_sin_func.set_callback(object); }
	template<class _Object> static devcb_base &set_sensea_cb(device_t &device, _Object object) { return downcast<scmp_device &>(device).m_sensea_func.set_callback(object); }
	template<class _Object> static devcb_base &set_senseb_cb(device_t &device, _Object object) { return downcast<scmp_device &>(device).m_senseb_func.set_callback(object); }
	template<class _Object> static devcb_base &set_halt_cb(device_t &device, _Object object) { return downcast<scmp_device &>(device).m_halt_func.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 5; }
	virtual UINT32 execute_max_cycles() const override { return 131593; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	PAIR    m_PC;
	PAIR    m_P1;
	PAIR    m_P2;
	PAIR    m_P3;
	UINT8   m_AC;
	UINT8   m_ER;
	UINT8   m_SR;

	address_space *m_program;
	direct_read_data *m_direct;
	int                 m_icount;

	devcb_write8       m_flag_out_func;
	devcb_write_line   m_sout_func;
	devcb_read_line    m_sin_func;
	devcb_read_line    m_sensea_func;
	devcb_read_line    m_senseb_func;
	devcb_write_line   m_halt_func;

	inline UINT16 ADD12(UINT16 addr, INT8 val);
	inline UINT8 ROP();
	inline UINT8 ARG();
	inline UINT8 RM(UINT32 a);
	inline void WM(UINT32 a, UINT8 v);
	inline void illegal(UINT8 opcode);
	inline PAIR *GET_PTR_REG(int num);
	inline void BIN_ADD(UINT8 val);
	inline void DEC_ADD(UINT8 val);
	inline UINT16 GET_ADDR(UINT8 code);
	void execute_one(int opcode);
	void take_interrupt();

};


class ins8060_device : public scmp_device
{
public:
	// construction/destruction
	ins8060_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 2 - 1) / 2; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 2); }
};


extern const device_type SCMP;
extern const device_type INS8060;


#endif
