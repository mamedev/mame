// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pc_xporter.h

    Implementation of the Applied Engineering PC Transporter card

*********************************************************************/

#pragma once

#include "emu.h"
#include "a2bus.h"
#include "cpu/nec/nec.h"
#include "bus/pc_kbd/keyboards.h"
#include "machine/ins8250.h"
#include "machine/i8255.h"
#include "machine/am9517a.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "sound/speaker.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_pcxporter_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_pcxporter_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_pcxporter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_READ16_MEMBER(pc_bios_r);

	required_device<v30_device> m_v30;
	required_device<pic8259_device>  m_pic8259;
	required_device<am9517a_device>  m_dma8237;
	required_device<pit8253_device>  m_pit8253;
	required_device<speaker_sound_device>  m_speaker;
	required_device<isa8_device>  m_isabus;
	optional_device<pc_kbdc_device>  m_pc_kbdc;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;
	virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_c800(address_space &space, UINT16 offset) override;
	virtual void write_c800(address_space &space, UINT16 offset, UINT8 data) override;

	UINT8   m_u73_q2;
	UINT8   m_out1;
	int m_dma_channel;
	UINT8 m_dma_offset[4];
	UINT8 m_pc_spkrdata;
	UINT8 m_pit_out2;
	bool m_cur_eop;

	UINT8 m_nmi_enabled;

	// interface to the keyboard
	DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );
	DECLARE_WRITE_LINE_MEMBER( keyboard_data_w );

	DECLARE_WRITE_LINE_MEMBER( pc_pit8253_out1_changed );
	DECLARE_WRITE_LINE_MEMBER( pc_pit8253_out2_changed );

	DECLARE_WRITE_LINE_MEMBER( pc_dma_hrq_changed );
	DECLARE_WRITE_LINE_MEMBER( pc_dma8237_out_eop );
	DECLARE_READ8_MEMBER( pc_dma_read_byte );
	DECLARE_WRITE8_MEMBER( pc_dma_write_byte );
	DECLARE_READ8_MEMBER( pc_dma8237_1_dack_r );
	DECLARE_READ8_MEMBER( pc_dma8237_2_dack_r );
	DECLARE_READ8_MEMBER( pc_dma8237_3_dack_r );
	DECLARE_WRITE8_MEMBER( pc_dma8237_1_dack_w );
	DECLARE_WRITE8_MEMBER( pc_dma8237_2_dack_w );
	DECLARE_WRITE8_MEMBER( pc_dma8237_3_dack_w );
	DECLARE_WRITE8_MEMBER( pc_dma8237_0_dack_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack0_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack1_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack2_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack3_w );
	DECLARE_READ8_MEMBER( kbd_6502_r );
	DECLARE_WRITE8_MEMBER( kbd_6502_w );

	DECLARE_WRITE_LINE_MEMBER( pc_speaker_set_spkrdata );

	DECLARE_WRITE8_MEMBER(pc_page_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8 m_ram[768*1024];
	UINT8 m_c800_ram[0x400];
	UINT8 m_regs[0x400];
	UINT32 m_offset;
	address_space *m_pcmem_space, *m_pcio_space;
	bool m_reset_during_halt;

	UINT8 m_6845_reg;

	void pc_select_dma_channel(int channel, bool state);
};

// device type definition
extern const device_type A2BUS_PCXPORTER;
