// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Mariusz Wojcieszek
/*****************************************************************************
 *
 * Sega SCUDSP emulator
 *
 *****************************************************************************/

#ifndef MAME_CPU_SCUDSP_SCUDSP_H
#define MAME_CPU_SCUDSP_SCUDSP_H

#pragma once

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


#define SCUDSP_RESET        INPUT_LINE_RESET    /* Non-Maskable */

class scudsp_cpu_device : public cpu_device
{
public:
	// construction/destruction
	scudsp_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto in_dma_callback() { return m_in_dma_cb.bind(); }
	auto out_dma_callback() { return m_out_dma_cb.bind(); }

	/* port 0 */
	uint32_t program_control_r();
	void program_control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	/* port 1 */
	void program_w(uint32_t data);
	/* port 2 */
	void ram_address_control_w(uint32_t data);
	/* port 3 */
	uint32_t ram_address_r();
	void ram_address_w(uint32_t data);

	void data_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 7; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	devcb_write_line     m_out_irq_cb;
	devcb_read16         m_in_dma_cb;
	devcb_write16        m_out_dma_cb;

private:
	union SCUDSPREG32 {
		int32_t  si;
		uint32_t ui;
	};

	union SCUDSPREG16 {
		int16_t  si;
		uint16_t ui;
	};

	address_space_config m_program_config;
	address_space_config m_data_config;

	uint8_t   m_pc;   /* registers */
	uint32_t  m_flags;  /* flags */
	uint8_t   m_ra;
	uint8_t   m_ct0,m_ct1,m_ct2,m_ct3;
	uint8_t   m_delay;                                   /* Delay */
	uint8_t   m_top;                                     /*Jump Command memory*/
	uint16_t  m_lop;                                    /*Counter Register*/   /*12-bits*/
	SCUDSPREG32 m_rx;                                /*X-Bus register*/
	int64_t   m_mul;                                     /*Multiplier register*//*48-bits*/
	SCUDSPREG32 m_ry;                                /*Y-Bus register*/
	int64_t   m_alu;                                    /*ALU register*/       /*48-bits*/
	SCUDSPREG16 m_ph;                                /*ALU high register*/
	SCUDSPREG32 m_pl;                                /*ALU low register*/
	SCUDSPREG16 m_ach;                               /*ALU external high register*/
	SCUDSPREG32 m_acl;                               /*ALU external low register*/
	uint32_t  m_ra0,m_wa0;                                /*DSP DMA registers*/
	struct{
		uint32_t src, dst;
		uint16_t add;
		uint16_t size, update, ex, dir, count;
	}m_dma;
	address_space *m_program;
	address_space *m_data;
	int m_icount;
	uint8_t m_update_mul;

	uint32_t scudsp_get_source_mem_reg_value( uint32_t mode );
	uint32_t scudsp_get_source_mem_value(uint8_t mode);
	void scudsp_set_dest_mem_reg( uint32_t mode, uint32_t value );
	void scudsp_set_dest_mem_reg_2( uint32_t mode, uint32_t value );
	uint32_t scudsp_compute_condition( uint32_t condition );
	uint32_t scudsp_get_mem_source_dma( uint32_t memcode, uint32_t counter );
	void scudsp_set_dest_dma_mem( uint32_t memcode, uint32_t value, uint32_t counter );

	void scudsp_illegal(uint32_t opcode);
	void scudsp_operation(uint32_t opcode);
	void scudsp_move_immediate(uint32_t opcode);
	void scudsp_dma(uint32_t opcode);
	void scudsp_jump(uint32_t opcode);
	void scudsp_loop(uint32_t opcode);
	void scudsp_end(uint32_t opcode);
	void scudsp_exec_dma();
};


DECLARE_DEVICE_TYPE(SCUDSP, scudsp_cpu_device)

#endif // MAME_CPU_SCUDSP_SCUDSP_H
