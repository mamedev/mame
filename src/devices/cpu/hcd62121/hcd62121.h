// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __HCD62121_H__
#define __HCD62121_H__




#define MCFG_HCD62121_KOL_CB(_devcb) devcb = &hcd62121_cpu_device::set_kol_callback(*device, DEVCB_##_devcb);
#define MCFG_HCD62121_KOH_CB(_devcb) devcb = &hcd62121_cpu_device::set_koh_callback(*device, DEVCB_##_devcb);
#define MCFG_HCD62121_PORT_CB(_devcb) devcb = &hcd62121_cpu_device::set_port_callback(*device, DEVCB_##_devcb);
#define MCFG_HCD62121_OPT_CB(_devcb) devcb = &hcd62121_cpu_device::set_opt_callback(*device, DEVCB_##_devcb);
#define MCFG_HCD62121_KI_CB(_devcb) devcb = &hcd62121_cpu_device::set_ki_callback(*device, DEVCB_##_devcb);
#define MCFG_HCD62121_IN0_CB(_devcb) devcb = &hcd62121_cpu_device::set_in0_callback(*device, DEVCB_##_devcb);


class hcd62121_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	hcd62121_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_kol_callback(device_t &device, _Object object) { return downcast<hcd62121_cpu_device &>(device).m_kol_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_koh_callback(device_t &device, _Object object) { return downcast<hcd62121_cpu_device &>(device).m_koh_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_port_callback(device_t &device, _Object object) { return downcast<hcd62121_cpu_device &>(device).m_port_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_opt_callback(device_t &device, _Object object) { return downcast<hcd62121_cpu_device &>(device).m_opt_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_ki_callback(device_t &device, _Object object) { return downcast<hcd62121_cpu_device &>(device).m_ki_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in0_callback(device_t &device, _Object object) { return downcast<hcd62121_cpu_device &>(device).m_in0_cb.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const override { return 4; }
	virtual u32 execute_max_cycles() const override { return 48; }
	virtual u32 execute_input_lines() const override { return 2; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual u32 disasm_min_opcode_bytes() const override { return 1; }
	virtual u32 disasm_max_opcode_bytes() const override { return 18; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const u8 *oprom, const u8 *opram, u32 options) override;

private:
	u8 read_op();
	u8 datasize(u8 op);
	void read_reg(int size, u8 op1);
	void write_reg(int size, u8 op1);
	void read_regreg(int size, u8 op1, u8 op2, bool op_is_logical);
	void write_regreg(int size, u8 op1, u8 op2);
	void read_iregreg(int size, u8 op1, u8 op2);
	void write_iregreg(int size, u8 op1, u8 op2);
	void write_iregreg2(int size, u8 op1, u8 op2);
	bool check_cond(u8 op);
	void set_zero_flag(bool is_zero);
	void set_carry_flag(bool is_carry);
	void set_zl_flag(bool is_zl);
	void set_zh_flag(bool is_zh);
	void set_cl_flag(bool is_cl);
	void op_msk(int size);
	void op_imsk(int size);
	void op_and(int size);
	void op_or(int size);
	void op_xor(int size);
	void op_add(int size);
	void op_addb(int size);
	void op_sub(int size);
	void op_pushw(u16 source);
	u16 op_popw();

	address_space_config m_program_config;

	u32 m_prev_pc;
	u16 m_sp;
	u16 m_ip;
	u8 m_dsize;
	u8 m_cseg;
	u8 m_dseg;
	u8 m_sseg;
	u8 m_f;
	u16 m_lar;
	u8 m_reg[0x80];

	// OPT7 - OPT0 output pins (pins 65-72)
	u8 m_opt;

	// PORT7 - PORT0 I/O pins (pins 73-80)
	u8 m_port;

	u8 m_temp1[0x10];
	u8 m_temp2[0x10];
	u32 m_rtemp;

	address_space *m_program;

	int m_icount;

	devcb_write8 m_kol_cb;
	devcb_write8 m_koh_cb;
	devcb_write8 m_port_cb;
	devcb_write8 m_opt_cb;
	devcb_read8 m_ki_cb;
	devcb_read8 m_in0_cb;
};


extern const device_type HCD62121;


#endif /* __HCD62121_H__ */
