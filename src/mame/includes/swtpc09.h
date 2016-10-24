// license:BSD-3-Clause
// copyright-holders:Robert Justice
/***************************************************************************
    swtpc09 include file
    Robert Justice ,2009-2014

****************************************************************************/

#ifndef swtpc09_H_
#define swtpc09_H_

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/generic.h"
#include "machine/wd_fdc.h"
#include "imagedev/flopdrv.h"
#include "machine/6840ptm.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/6522via.h"
#include "machine/terminal.h"
#include "imagedev/harddriv.h"
#include "machine/idectrl.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"



class swtpc09_state : public driver_device
{
public:
	swtpc09_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pia(*this, "pia"),
	m_ptm(*this, "ptm"),
	m_acia(*this, "acia"),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_floppy2(*this, "fdc:2"),
	m_floppy3(*this, "fdc:3"),
	m_via(*this, "via"),
	m_piaide(*this, "piaide"),
	m_harddisk(*this, "harddisk"),
	m_ide(*this, "ide")
	{ }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<ptm6840_device> m_ptm;
	required_device<acia6850_device> m_acia;
	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	optional_device<via6522_device> m_via;
	optional_device<pia6821_device> m_piaide;
	optional_device<device_t> m_harddisk;
	optional_device<ide_controller_device> m_ide;

	uint8_t pia0_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pia0_ca1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia0_irq_a(int state);

	void ptm_o1_callback(int state);
	void ptm_o3_callback(int state);
	void ptm_irq(int state);

	void acia_interrupt(int state);
	void write_acia_clock(int state);

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	uint8_t dmf3_via_read_porta(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dmf3_via_read_portb(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dmf3_via_write_porta(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dmf3_via_irq(int state);

	uint8_t piaide_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t piaide_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void piaide_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void piaide_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t dmf2_dma_address_reg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dmf2_dma_address_reg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dmf2_control_reg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dmf2_control_reg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t dmf3_dma_address_reg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dmf3_dma_address_reg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dmf3_control_reg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dmf3_control_reg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void dc4_control_reg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void dat_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_swtpc09();
	void init_swtpc09i();
	void init_swtpc09u();
	void init_swtpc09d3();

	void swtpc09_fdc_dma_transfer();
	void swtpc09_irq_handler(uint8_t peripheral, uint8_t state);

	uint8_t m_term_data;               // terminal keyboard value
	uint8_t m_pia_counter;             // this is the counter on pia porta
	uint8_t m_fdc_dma_address_reg;     // dmf2 or dmf3 dma extended address reg
	uint8_t m_system_type;             // flag to indicate hw and rom combination
	uint8_t m_fdc_status;              // for floppy controller
	uint8_t m_via_ca1_input;           // dmf3 fdc interupt is connected here
	uint8_t m_dmf3_via_porta;
	uint8_t m_piaide_porta;
	uint8_t m_piaide_portb;
	uint8_t m_active_interrupt;
	uint8_t m_interrupt;


	// TODO: move this in proper device

	/* channel_data structure holds info about each 6844 DMA channel */
	typedef struct m6844_channel_data
	{
		int active;
		int address;
		int counter;
		uint8_t control;
		int start_address;
		int start_counter;
	} m6844_channel_data;

	/* 6844 description */
	m6844_channel_data m_m6844_channel[4];
	uint8_t m_m6844_priority;
	uint8_t m_m6844_interrupt;
	uint8_t m_m6844_chain;
	uint8_t m6844_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m6844_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

};

#endif /* swtpc09_H_ */
