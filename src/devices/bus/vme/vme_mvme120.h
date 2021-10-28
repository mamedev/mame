// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
#ifndef MAME_BUS_VME_VME_MVME120_H
#define MAME_BUS_VME_VME_MVME120_H

#pragma once

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/vme/vme.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/mc68901.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

//#define VERBOSE 0
#define VERBOSE (LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define MVME120_MASTER_CLOCK 	20_MHz_XTAL
#define MVME122_MASTER_CLOCK 	25_MHz_XTAL

#define MVME120_CPU_CLOCK		( MVME120_MASTER_CLOCK / 2 )
#define MVME122_CPU_CLOCK		( MVME122_MASTER_CLOCK / 2 )

DECLARE_DEVICE_TYPE(VME_MVME120,   vme_mvme120_card_device)
DECLARE_DEVICE_TYPE(VME_MVME121,   vme_mvme121_card_device)
DECLARE_DEVICE_TYPE(VME_MVME122,   vme_mvme122_card_device)
DECLARE_DEVICE_TYPE(VME_MVME123,   vme_mvme123_card_device)

//**************************************************************************
//  Base Device declaration
//**************************************************************************
class vme_mvme120_device :  public device_t, public device_vme_card_interface
{
public:
	/* Board types */
	enum mvme120_board_t {
		mvme120_board,
		mvme121_board,
		mvme122_board,
		mvme123_board
	};
		
	vme_mvme120_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, mvme120_board_t board_id);

	// Switch and jumper handlers
	DECLARE_INPUT_CHANGED_MEMBER(s3_autoboot);
	DECLARE_INPUT_CHANGED_MEMBER(s3_baudrate);
	
protected:
	void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	
	uint16_t bootvect_r(offs_t offset);
	void bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	
	virtual void device_start () override;
	virtual void device_reset () override;
	
	void mvme120_mem(address_map &map);
	
	// add the rest of the devices here...
	required_device<cpu_device> m_maincpu;
	required_device<mc68901_device> m_mfp;
	required_device<rs232_port_device> m_rs232;
	
	required_ioport m_input_s3;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint16_t  *m_sysrom;
	uint16_t  m_sysram[2];
	uint8_t	  m_boot_memory_cycles;

	// "VME120 Control Register"
	uint8_t		m_ctrlreg;
	uint8_t 	ctrlreg_r(offs_t offset);
	void 		ctrlreg_w(offs_t offset, uint8_t data);
	
	// VMEbus dummy
	uint16_t vme_a24_r();
	void vme_a24_w(uint16_t data);
	uint16_t vme_a16_r();
	void vme_a16_w(uint16_t data);
	
	// Add the devices' registers and callbacks here...
	DECLARE_WRITE_LINE_MEMBER(watchdog_reset);	
	DECLARE_WRITE_LINE_MEMBER(mfp_interrupt);
	
	const mvme120_board_t  m_board_id;
};


//**************************************************************************
//  Board Device declarations
//**************************************************************************

class vme_mvme120_card_device : public vme_mvme120_device
{
public :
	vme_mvme120_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mvme120_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme120_device(mconfig, type, tag, owner, clock, mvme120_board)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};

class vme_mvme121_card_device : public vme_mvme120_device
{
public :
	vme_mvme121_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mvme121_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme120_device(mconfig, type, tag, owner, clock, mvme121_board)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};


class vme_mvme122_card_device : public vme_mvme120_device
{
public :
	vme_mvme122_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mvme122_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme120_device(mconfig, type, tag, owner, clock, mvme122_board)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};


class vme_mvme123_card_device : public vme_mvme120_device
{
public :
	vme_mvme123_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mvme123_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme120_device(mconfig, type, tag, owner, clock, mvme123_board)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};


#endif // MAME_BUS_VME_VME_FCCPU20_H
