// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Mariusz Wojcieszek
/*****************************************************************************
 *
 * Sega SCUDSP emulator
 *
 *****************************************************************************/

#pragma once

#ifndef __SCUDSP_H__
#define __SCUDSP_H__

enum
{
	SCUDSP_PC=1,
	SCUDSP_FLAGS,
	SCUDSP_DELAY,
	SCUDSP_TOP,
	SCUDSP_LOP,
	SCUDSP_RX,
	SCUDSP_MUL,
	SCUDSP_RY,
	SCUDSP_ALU,
	SCUDSP_PH,
	SCUDSP_PL,
	SCUDSP_ACH,
	SCUDSP_ACL,
	SCUDSP_RA0,
	SCUDSP_WA0,
	SCUDSP_RA,
	SCUDSP_CT0,
	SCUDSP_CT1,
	SCUDSP_CT2,
	SCUDSP_CT3
};


#define MCFG_SCUDSP_OUT_IRQ_CB(_devcb) \
	devcb = &scudsp_cpu_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_SCUDSP_IN_DMA_CB(_devcb) \
	devcb = &scudsp_cpu_device::set_in_dma_callback(*device, DEVCB_##_devcb);

#define MCFG_SCUDSP_OUT_DMA_CB(_devcb) \
	devcb = &scudsp_cpu_device::set_out_dma_callback(*device, DEVCB_##_devcb);


#define SCUDSP_RESET        INPUT_LINE_RESET    /* Non-Maskable */

union SCUDSPREG32 {
	INT32  si;
	UINT32 ui;
};

union SCUDSPREG16 {
	INT16  si;
	UINT16 ui;
};

class scudsp_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	scudsp_cpu_device(const machine_config &mconfig, std::string _tag, device_t *_owner, UINT32 _clock);

	template<class _Object> static devcb_base &set_out_irq_callback(device_t &device, _Object object) { return downcast<scudsp_cpu_device &>(device).m_out_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_dma_callback(device_t &device, _Object object) { return downcast<scudsp_cpu_device &>(device).m_in_dma_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dma_callback(device_t &device, _Object object) { return downcast<scudsp_cpu_device &>(device).m_out_dma_cb.set_callback(object); }

	/* port 0 */
	DECLARE_READ32_MEMBER( program_control_r );
	DECLARE_WRITE32_MEMBER( program_control_w );
	/* port 1 */
	DECLARE_WRITE32_MEMBER( program_w );
	/* port 2 */
	DECLARE_WRITE32_MEMBER( ram_address_control_w );
	/* port 3 */
	DECLARE_READ32_MEMBER( ram_address_r );
	DECLARE_WRITE32_MEMBER( ram_address_w );
//  virtual DECLARE_ADDRESS_MAP(map, 32) = 0;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 7; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr ); }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	devcb_write_line     m_out_irq_cb;
	devcb_read16         m_in_dma_cb;
	devcb_write16        m_out_dma_cb;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	UINT8   m_pc;   /* registers */
	UINT32  m_flags;  /* flags */
	UINT8   m_ra;
	UINT8   m_ct0,m_ct1,m_ct2,m_ct3;
	UINT8   m_delay;                                   /* Delay */
	UINT8   m_top;                                     /*Jump Command memory*/
	UINT16  m_lop;                                    /*Counter Register*/   /*12-bits*/
	SCUDSPREG32 m_rx;                                /*X-Bus register*/
	INT64   m_mul;                                     /*Multiplier register*//*48-bits*/
	SCUDSPREG32 m_ry;                                /*Y-Bus register*/
	INT64   m_alu;                                    /*ALU register*/       /*48-bits*/
	SCUDSPREG16 m_ph;                                /*ALU high register*/
	SCUDSPREG32 m_pl;                                /*ALU low register*/
	SCUDSPREG16 m_ach;                               /*ALU external high register*/
	SCUDSPREG32 m_acl;                               /*ALU external low register*/
	UINT32  m_ra0,m_wa0;                                /*DSP DMA registers*/
	struct{
		UINT32 src, dst;
		UINT16 add;
		UINT16 size, update, ex, dir, count;
	}m_dma;
	address_space *m_program;
	address_space *m_data;
	int m_icount;
	UINT8 m_update_mul;

	UINT32 scudsp_get_source_mem_reg_value( UINT32 mode );
	UINT32 scudsp_get_source_mem_value(UINT8 mode);
	void scudsp_set_dest_mem_reg( UINT32 mode, UINT32 value );
	void scudsp_set_dest_mem_reg_2( UINT32 mode, UINT32 value );
	UINT32 scudsp_compute_condition( UINT32 condition );
	UINT32 scudsp_get_mem_source_dma( UINT32 memcode, UINT32 counter );
	void scudsp_set_dest_dma_mem( UINT32 memcode, UINT32 value, UINT32 counter );

	void scudsp_illegal(UINT32 opcode);
	void scudsp_operation(UINT32 opcode);
	void scudsp_move_immediate(UINT32 opcode);
	void scudsp_dma(UINT32 opcode);
	void scudsp_jump(UINT32 opcode);
	void scudsp_loop(UINT32 opcode);
	void scudsp_end(UINT32 opcode);
	void scudsp_exec_dma();
};


extern const device_type SCUDSP;


CPU_DISASSEMBLE( scudsp );

#endif /* __SCUDSP_H__ */
