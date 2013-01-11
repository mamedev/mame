#pragma once

#ifndef __ISA_IDE_CD_H__
#define __ISA_IDE_CD_H__

#include "emu.h"
#include "machine/isa.h"
#include "imagedev/chd_cd.h"
#include "machine/cr589.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
#define ATAPI_CYCLES_PER_SECTOR (5000)  // plenty of time to allow DMA setup etc.  BIOS requires this be at least 2000, individual games may vary.

#define ATAPI_ERRFEAT_ABRT 0x04

#define ATAPI_STAT_BSY     0x80
#define ATAPI_STAT_DRDY    0x40
#define ATAPI_STAT_DMARDDF 0x20
#define ATAPI_STAT_SERVDSC 0x10
#define ATAPI_STAT_DRQ     0x08
#define ATAPI_STAT_CORR    0x04
#define ATAPI_STAT_CHECK   0x01

#define ATAPI_INTREASON_COMMAND 0x01
#define ATAPI_INTREASON_IO      0x02
#define ATAPI_INTREASON_RELEASE 0x04

#define ATAPI_REG_DATA      0
#define ATAPI_REG_ERRFEAT   1
#define ATAPI_REG_INTREASON 2
#define ATAPI_REG_SAMTAG    3
#define ATAPI_REG_COUNTLOW  4
#define ATAPI_REG_COUNTHIGH 5
#define ATAPI_REG_DRIVESEL  6
#define ATAPI_REG_CMDSTATUS 7
#define ATAPI_REG_MAX 16

#define ATAPI_DATA_SIZE ( 64 * 1024 )

#define MAX_TRANSFER_SIZE ( 63488 )

// ======================> isa16_ide_cd_device

class isa16_ide_cd_device :
		public device_t,
		public device_isa16_card_interface
{
public:
		// construction/destruction
		isa16_ide_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual ioport_constructor device_input_ports() const;

		bool is_primary() { return m_is_primary; }


		DECLARE_READ16_MEMBER(atapi_r);
		DECLARE_WRITE16_MEMBER(atapi_w);
		DECLARE_READ16_MEMBER(atapi_status_r);
		DECLARE_WRITE16_MEMBER(atapi_cmd_w);
protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();
		virtual void device_config_complete() { m_shortname = "isa_ide_cd"; }
private:
		// internal state
		bool m_is_primary;

		// CDROM
		scsicd_device *m_inserted_cdrom;

		int m_atapi_data_ptr;
		int m_atapi_data_len;
		int m_atapi_xferlen;
		int m_atapi_cdata_wait;
		int m_atapi_xfermod;
		/* memory */
		UINT8 m_atapi_regs[ATAPI_REG_MAX];
		UINT8 m_atapi_data[ATAPI_DATA_SIZE];

		int m_cur_drive;
};


// device type definition
extern const device_type ISA16_IDE_CD;

#endif  /* __ISA_IDE_CD_H__ */
