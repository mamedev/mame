// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#pragma once

#ifndef __SOUTHBRIDGE_H__
#define __SOUTHBRIDGE_H__

#include "emu.h"

#include "machine/ins8250.h"
#include "machine/ds128x.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"

#include "machine/ataintf.h"
#include "machine/at_keybc.h"

#include "imagedev/harddriv.h"
#include "pci.h"

#include "sound/dac.h"
#include "sound/speaker.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"

#include "machine/pc_lpt.h"
#include "bus/pc_kbd/pc_kbdc.h"

#include "machine/am9517a.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> southbridge_device

class southbridge_device :
		public device_t
{
public:
		// construction/destruction
		southbridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;

protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
public:

		required_device<cpu_device> m_maincpu;
		required_device<pic8259_device> m_pic8259_master;
		required_device<pic8259_device> m_pic8259_slave;
		required_device<am9517a_device> m_dma8237_1;
		required_device<am9517a_device> m_dma8237_2;
		required_device<pit8254_device> m_pit8254;
		required_device<at_keyboard_controller_device> m_keybc;
		required_device<isa16_device> m_isabus;
		required_device<speaker_sound_device> m_speaker;
		required_device<ds12885_device> m_ds12885;
		required_device<pc_kbdc_device> m_pc_kbdc;
		required_device<bus_master_ide_controller_device> m_ide;
		required_device<bus_master_ide_controller_device> m_ide2;
		DECLARE_READ8_MEMBER(at_page8_r);
		DECLARE_WRITE8_MEMBER(at_page8_w);
		DECLARE_READ8_MEMBER(at_portb_r);
		DECLARE_WRITE8_MEMBER(at_portb_w);
		DECLARE_READ8_MEMBER(get_slave_ack);
		DECLARE_WRITE_LINE_MEMBER(at_pit8254_out0_changed);
		DECLARE_WRITE_LINE_MEMBER(at_pit8254_out1_changed);
		DECLARE_WRITE_LINE_MEMBER(at_pit8254_out2_changed);
		DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
		DECLARE_READ8_MEMBER(pc_dma8237_0_dack_r);
		DECLARE_READ8_MEMBER(pc_dma8237_1_dack_r);
		DECLARE_READ8_MEMBER(pc_dma8237_2_dack_r);
		DECLARE_READ8_MEMBER(pc_dma8237_3_dack_r);
		DECLARE_READ8_MEMBER(pc_dma8237_5_dack_r);
		DECLARE_READ8_MEMBER(pc_dma8237_6_dack_r);
		DECLARE_READ8_MEMBER(pc_dma8237_7_dack_r);
		DECLARE_WRITE8_MEMBER(pc_dma8237_0_dack_w);
		DECLARE_WRITE8_MEMBER(pc_dma8237_1_dack_w);
		DECLARE_WRITE8_MEMBER(pc_dma8237_2_dack_w);
		DECLARE_WRITE8_MEMBER(pc_dma8237_3_dack_w);
		DECLARE_WRITE8_MEMBER(pc_dma8237_5_dack_w);
		DECLARE_WRITE8_MEMBER(pc_dma8237_6_dack_w);
		DECLARE_WRITE8_MEMBER(pc_dma8237_7_dack_w);
		DECLARE_WRITE_LINE_MEMBER(at_dma8237_out_eop);
		DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
		DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
		DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
		DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
		DECLARE_WRITE_LINE_MEMBER(pc_dack4_w);
		DECLARE_WRITE_LINE_MEMBER(pc_dack5_w);
		DECLARE_WRITE_LINE_MEMBER(pc_dack6_w);
		DECLARE_WRITE_LINE_MEMBER(pc_dack7_w);
		DECLARE_READ8_MEMBER(ide_read_cs1_r);
		DECLARE_WRITE8_MEMBER(ide_write_cs1_w);
		DECLARE_READ8_MEMBER(ide2_read_cs1_r);
		DECLARE_WRITE8_MEMBER(ide2_write_cs1_w);
		DECLARE_READ8_MEMBER(at_dma8237_2_r);
		DECLARE_WRITE8_MEMBER(at_dma8237_2_w);
		DECLARE_READ8_MEMBER(at_keybc_r);
		DECLARE_WRITE8_MEMBER(at_keybc_w);
		DECLARE_WRITE8_MEMBER(write_rtc);
		DECLARE_READ8_MEMBER(pc_dma_read_byte);
		DECLARE_WRITE8_MEMBER(pc_dma_write_byte);
		DECLARE_READ8_MEMBER(pc_dma_read_word);
		DECLARE_WRITE8_MEMBER(pc_dma_write_word);
protected:
		UINT8 m_at_spkrdata;
		UINT8 m_pit_out2;
		int m_dma_channel;
		bool m_cur_eop;
		UINT8 m_dma_offset[2][4];
		UINT8 m_at_pages[0x10];
		UINT16 m_dma_high_byte;
		UINT8 m_at_speaker;
		bool m_refresh;
		void at_speaker_set_spkrdata(UINT8 data);

		UINT8 m_channel_check;
		UINT8 m_nmi_enabled;

		void pc_select_dma_channel(int channel, bool state);
};

#endif  /* __SOUTHBRIDGE_H__ */
