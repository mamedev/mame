// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/
#include "cdrom.h"

#pragma once

#ifndef __SEGACDBLOCKDEV_H__
#define __SEGACDBLOCKDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEGACDBLOCK_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SEGACDBLOCK, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> segacdblock_device

class segacdblock_device : public device_t,
						   public device_memory_interface
{
public:
	// construction/destruction
	segacdblock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

	DECLARE_READ16_MEMBER( cr0_r );
	DECLARE_READ16_MEMBER( cr1_r );
	DECLARE_READ16_MEMBER( cr2_r );
	DECLARE_READ16_MEMBER( cr3_r );

	DECLARE_WRITE16_MEMBER( cr0_w );
	DECLARE_WRITE16_MEMBER( cr1_w );
	DECLARE_WRITE16_MEMBER( cr2_w );
	DECLARE_WRITE16_MEMBER( cr3_w );

	DECLARE_READ16_MEMBER( hirq_r );
	DECLARE_WRITE16_MEMBER( hirq_w );

	DECLARE_READ16_MEMBER( hirq_mask_r );
	DECLARE_WRITE16_MEMBER( hirq_mask_w );

	DECLARE_READ16_MEMBER( datatrns_r );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual machine_config_constructor device_mconfig_additions() const;

	address_space_config        m_space_config;
	address_space *m_space;

private:
	static const device_timer_id PERI_TIMER = 0;
	static const device_timer_id CMD_TIMER = 1;
	static const device_timer_id CD_TIMER = 2;

	UINT16 m_cr[4];
	UINT16 m_dr[4];
	UINT16 m_hirq_mask;
	UINT16 m_hirq;
	UINT32 m_fad;
	UINT16 m_cd_state;
	UINT8 m_cmd_issued;
	bool m_sh1_inited;
	bool m_isDiscInTray;
	bool m_TOCPhase;
	bool m_TransferActive;
	UINT32 m_DiscLeadOut;

	cdrom_file *cdrom;

	enum transT
	{
		CDDMA_STOPPED,
		CDDMA_INPROGRESS
		/*XFERTYPE_TOC,
		XFERTYPE_FILEINFO_1,
		XFERTYPE_FILEINFO_254,
		XFERTYPE_SUBQ,
		XFERTYPE_SUBRW*/
	};
	
	enum sourceT
	{
		SOURCE_NONE,
		SOURCE_TOC,
		SOURCE_DATA,
		SOURCE_AUDIO
	};

	transT xfertype;
	sourceT sourcetype;
	UINT32 m_dma_src,m_dma_size;

	void dma_setup();

	UINT8 tocbuf[102*4]; /**< @todo Subchannel Q of lead-in */
	UINT8 *m_DMABuffer;
	void TOCRetrieve();

	void sh1_writes_registers(UINT16 r1, UINT16 r2, UINT16 r3, UINT16 r4);
	void cd_standard_return(bool isPeri);

	// 0x00
	void cd_cmd_status();
	void cd_cmd_get_hw_info();
	void cd_cmd_get_toc();
	void cd_cmd_get_session_info(UINT8 param);
	void cd_cmd_init(UINT8 init_flags);
	void cd_cmd_end_transfer();

	// 0x40
	void cd_cmd_reset_selector();
	
	// 0x60
	void cd_cmd_set_sector_length();
	void cd_cmd_get_copy_error();

	// 0x70
	void cd_cmd_change_dir();
	void cd_cmd_abort();

	// 0xe0
	void cd_cmd_auth_device(UINT16 AuthType);
	void cd_cmd_device_auth_status(UINT16 AuthType);
	void SH2SendsCommand();
	void SH1CommandExecute();

	void set_flag(UINT16 which);
	void clear_flag(UINT16 which);
	//emu_timer *m_sh1_timer;
	emu_timer *m_peri_timer;
	emu_timer *m_cmd_timer;
	emu_timer *m_cd_timer;
	UINT16 read_cd_state();
	void write_cd_state(UINT16 which);
	void write_fad();
};


// device type definition
extern const device_type SEGACDBLOCK;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
