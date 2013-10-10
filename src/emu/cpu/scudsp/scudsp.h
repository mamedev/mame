/*****************************************************************************
 *
 * SCUDSP CPU core
 *
 * skeleton for now ...
 *
 *****************************************************************************/

#pragma once

#ifndef __SCUDSP_H__
#define __SCUDSP_H__

enum
{
	SCUDSP_RA=1,
	SCUDSP_CT0, SCUDSP_CT1, SCUDSP_CT2, SCUDSP_CT3,
	SCUDSP_PC, SCUDSP_FLAGS
};

#define SCUDSP_RESET        INPUT_LINE_RESET    /* Non-Maskable */


class scudsp_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	scudsp_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

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
//	virtual DECLARE_ADDRESS_MAP(map, 32) = 0;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 7; }
	virtual UINT32 execute_input_lines() const { return 0; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA) ? &m_data_config : NULL ); }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	UINT8	m_pc;   /* registers */
	UINT32	m_flags;  /* flags */
	UINT8   m_ra;
	UINT8   m_ct0,m_ct1,m_ct2,m_ct3;                        /*Index for RAM*/      /*6-bits */
	int     m_reset_state;
	address_space *m_program;
	address_space *m_data;
	int m_icount;

	UINT32 scudsp_get_source_mem_value(UINT8 mode);
	void scudsp_set_dest_mem_reg( UINT32 mode, UINT32 value );
	void scudsp_illegal();
};


extern const device_type SCUDSP;


CPU_DISASSEMBLE( scudsp );

#endif /* __SCUDSP_H__ */
