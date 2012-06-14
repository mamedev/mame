/*
    TMS9980A.

    See tms9980a.c and tms9900.c for documentation
*/

#ifndef __TMS9980A_H__
#define __TMS9980A_H__

#include "emu.h"
#include "debugger.h"
#include "tms9900.h"

#define MCFG_TMS9980A_ADD(_tag, _device, _clock, _prgmap, _iomap, _config)		\
	MCFG_DEVICE_ADD(_tag, _device, _clock )		\
	MCFG_DEVICE_PROGRAM_MAP(_prgmap)			\
	MCFG_DEVICE_IO_MAP(_iomap)					\
	MCFG_DEVICE_CONFIG(_config)

#define TMS9980A_CONFIG(name) \
	const tms9900_config(name) =


class tms9980a_device : public tms99xx_device
{
public:
	tms9980a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	void		mem_read(void);
	void		mem_write(void);
	void		acquire_instruction(void);

	UINT16		read_workspace_register_debug(int reg);
	void		write_workspace_register_debug(int reg, UINT16 data);

	UINT32		execute_min_cycles() const;
	UINT32		execute_max_cycles() const;
	UINT32		execute_input_lines() const;
	void		execute_set_input(int irqline, int state);

	UINT32		disasm_min_opcode_bytes() const;
	UINT32		disasm_max_opcode_bytes() const;
	offs_t		disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	address_space_config	m_program_config80;
	address_space_config	m_io_config80;

	int 		get_intlevel(int state);
};

// device type definition
extern const device_type TMS9980A;

#endif /* __TMS9995_H__ */
