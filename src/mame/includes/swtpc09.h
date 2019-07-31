// license:BSD-3-Clause
// copyright-holders:Robert Justice,68bit
/***************************************************************************
    swtpc09 include file
    Robert Justice ,2009-2014

****************************************************************************/

#ifndef MAME_INCLUDES_SWTPC09_H
#define MAME_INCLUDES_SWTPC09_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "video/generic.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "machine/6840ptm.h"
#include "machine/6821pia.h"
#include "machine/6522via.h"
#include "imagedev/harddriv.h"
#include "machine/bankdev.h"
#include "machine/mc14411.h"


class swtpc09_state : public driver_device
{
public:
	swtpc09_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_brg(*this, "brg")
		, m_pia(*this, "pia")
		, m_ptm(*this, "ptm")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_floppy2(*this, "fdc:2")
		, m_floppy3(*this, "fdc:3")
		, m_via(*this, "via")
		, m_dat(*this, "dat")
		, m_bankdev(*this, "bankdev")
		, m_maincpu_clock(*this, "maincpu_clock")
		, m_fdc_clock(*this, "fdc_clock")
		, m_baud_rate_high(*this, "baud_rate_high")
		, m_floppy_expected_density(*this, "floppy_expected_density")
		, m_floppy_expected_sectors(*this, "floppy_expected_sectors")
		, m_floppy_track_zero_expected_sectors(*this, "floppy_track_zero_expected_sectors")
		, m_sbug_double_density(*this, "sbug_double_density")
		, m_piaide_flex_boot_cd00(*this, "piaide_flex_boot_cd00")
	{ }

	void swtpc09_base(machine_config &config);
	void swtpc09i(machine_config &config);
	void swtpc09d3(machine_config &config);
	void swtpc09u(machine_config &config);
	void swtpc09(machine_config &config);

	void init_swtpc09();
	void init_swtpc09i();
	void init_swtpc09u();
	void init_swtpc09d3();

private:
	DECLARE_FLOPPY_FORMATS(floppy_flex_formats);
	DECLARE_FLOPPY_FORMATS(floppy_uniflex_formats);

	DECLARE_READ8_MEMBER(pia0_a_r);
	DECLARE_READ8_MEMBER(pia0_ca1_r);
	DECLARE_WRITE_LINE_MEMBER( pia0_irq_a );

	DECLARE_WRITE_LINE_MEMBER(io_irq_w);

	DECLARE_WRITE_LINE_MEMBER( ptm_o1_callback );
	DECLARE_WRITE_LINE_MEMBER( ptm_o3_callback );
	DECLARE_WRITE_LINE_MEMBER( ptm_irq );

	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_sso_w );

	DECLARE_READ8_MEMBER( dmaf3_via_read_porta );
	DECLARE_READ8_MEMBER( dmaf3_via_read_portb );
	DECLARE_WRITE8_MEMBER( dmaf3_via_write_porta );
	DECLARE_WRITE_LINE_MEMBER( dmaf3_via_irq );

	TIMER_CALLBACK_MEMBER(floppy_motor_callback);

	DECLARE_READ8_MEMBER( dmaf2_fdc_r );
	DECLARE_WRITE8_MEMBER( dmaf2_fdc_w );
	DECLARE_READ8_MEMBER ( dmaf2_dma_address_reg_r );
	DECLARE_WRITE8_MEMBER ( dmaf2_dma_address_reg_w );
	DECLARE_READ8_MEMBER ( dmaf2_control_reg_r );
	DECLARE_WRITE8_MEMBER ( dmaf2_control_reg_w );

	DECLARE_READ8_MEMBER( dmaf3_fdc_r );
	DECLARE_WRITE8_MEMBER( dmaf3_fdc_w );
	DECLARE_READ8_MEMBER ( dmaf3_dma_address_reg_r );
	DECLARE_WRITE8_MEMBER ( dmaf3_dma_address_reg_w );
	DECLARE_READ8_MEMBER ( dmaf3_control_reg_r );
	DECLARE_WRITE8_MEMBER ( dmaf3_control_reg_w );

	DECLARE_READ8_MEMBER( dmaf3_wd_r );
	DECLARE_WRITE8_MEMBER( dmaf3_wd_w );

	DECLARE_WRITE8_MEMBER(dat_w);
	DECLARE_READ8_MEMBER(main_r);
	DECLARE_WRITE8_MEMBER(main_w);

	DECLARE_READ8_MEMBER ( m6844_r );
	DECLARE_WRITE8_MEMBER ( m6844_w );

	DECLARE_READ8_MEMBER ( unmapped_r );
	DECLARE_WRITE8_MEMBER ( unmapped_w );

	void flex_dc5_piaide_mem(address_map &map);
	void flex_dmaf2_mem(address_map &map);
	void mp09_mem(address_map &map);
	void uniflex_dmaf2_mem(address_map &map);
	void uniflex_dmaf3_mem(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void swtpc09_fdc_dma_transfer();
	void swtpc09_irq_handler(uint8_t peripheral, uint8_t state);

	void floppy_motor_trigger();

	void validate_floppy_side(uint8_t cmd);
	uint8_t validate_fdc_sector_size(uint8_t cmd);
	uint8_t validate_fdc_dden(uint8_t dden);

	offs_t dat_translate(offs_t offset) const;

	required_device<mc6809_device> m_maincpu;
	required_device<mc14411_device> m_brg;
	required_device<pia6821_device> m_pia;
	required_device<ptm6840_device> m_ptm;

	optional_device<fd1797_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	optional_device<floppy_connector> m_floppy2;
	optional_device<floppy_connector> m_floppy3;

	optional_device<via6522_device> m_via;
	required_shared_ptr<uint8_t> m_dat;
	required_device<address_map_bank_device> m_bankdev;
	required_ioport m_maincpu_clock;
	required_ioport m_fdc_clock;
	required_ioport m_baud_rate_high;
	required_ioport m_floppy_expected_density;
	required_ioport m_floppy_expected_sectors;
	required_ioport m_floppy_track_zero_expected_sectors;
	required_ioport m_sbug_double_density;
	required_ioport m_piaide_flex_boot_cd00;

	uint8_t m_pia_counter;             // this is the counter on pia porta

	uint8_t m_fdc_dma_address_reg;     // dmaf2 or dmaf3 dma extended address reg
	uint8_t m_system_type;             // flag to indicate hw and rom combination
	uint8_t m_fdc_status;              // for floppy controller
	int m_floppy_motor_on;
	emu_timer *m_floppy_motor_timer;
	floppy_image_device *m_fdc_floppy; // Current selected floppy.
	uint8_t m_fdc_side;                // Current floppy side.
	uint8_t m_via_ca1_input;           // dmaf3 fdc interrupt is connected here
	uint8_t m_dmaf3_via_porta;
	uint8_t m_active_interrupt;
	uint8_t m_interrupt;

	address_space *m_banked_space;

	// TODO: move this in proper device

	/* channel_data structure holds info about each 6844 DMA channel */
	struct m6844_channel_data
	{
		int active;
		int address;
		int counter;
		uint8_t control;
		int start_address;
		int start_counter;
	};

	/* 6844 description */
	m6844_channel_data m_m6844_channel[4];
	uint8_t m_m6844_priority;
	uint8_t m_m6844_interrupt;
	uint8_t m_m6844_chain;
};

#endif // MAME_INCLUDES_SWTPC09_H
