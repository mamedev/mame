// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari vector hardware

***************************************************************************/

#ifndef MAME_MACHINE_ATARI_VG_H
#define MAME_MACHINE_ATARI_VG_H

#pragma once

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_ATARIVGEAROM_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ATARIVGEAROM, 0)


#define EAROM_SIZE  0x40

// ======================> atari_vg_earom_device

class atari_vg_earom_device :   public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	atari_vg_earom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( ctrl_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	int m_old_ctrl;
	int m_state;
	int m_in_offset;
	int m_in_data;
	int m_out_data;
	char m_rom[EAROM_SIZE];
};

// device type definition
DECLARE_DEVICE_TYPE(ATARIVGEAROM, atari_vg_earom_device)

#endif // MAME_MACHINE_ATARI_VG_H
