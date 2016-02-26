// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl
#ifndef _AT_H_
#define _AT_H_

#include "emu.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/am9517a.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/isa/isa.h"
#include "sound/speaker.h"
#include "softlist.h"

class at_mb_device : public device_t
{
public:
	at_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_ADDRESS_MAP(map, 16);

	DECLARE_READ8_MEMBER(page8_r);
	DECLARE_WRITE8_MEMBER(page8_w);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(pit8254_out2_changed);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	DECLARE_READ8_MEMBER(dma8237_0_dack_r);
	DECLARE_READ8_MEMBER(dma8237_1_dack_r);
	DECLARE_READ8_MEMBER(dma8237_2_dack_r);
	DECLARE_READ8_MEMBER(dma8237_3_dack_r);
	DECLARE_READ8_MEMBER(dma8237_5_dack_r);
	DECLARE_READ8_MEMBER(dma8237_6_dack_r);
	DECLARE_READ8_MEMBER(dma8237_7_dack_r);
	DECLARE_WRITE8_MEMBER(dma8237_0_dack_w);
	DECLARE_WRITE8_MEMBER(dma8237_1_dack_w);
	DECLARE_WRITE8_MEMBER(dma8237_2_dack_w);
	DECLARE_WRITE8_MEMBER(dma8237_3_dack_w);
	DECLARE_WRITE8_MEMBER(dma8237_5_dack_w);
	DECLARE_WRITE8_MEMBER(dma8237_6_dack_w);
	DECLARE_WRITE8_MEMBER(dma8237_7_dack_w);
	DECLARE_WRITE_LINE_MEMBER(dma8237_out_eop);
	DECLARE_WRITE_LINE_MEMBER(dack0_w);
	DECLARE_WRITE_LINE_MEMBER(dack1_w);
	DECLARE_WRITE_LINE_MEMBER(dack2_w);
	DECLARE_WRITE_LINE_MEMBER(dack3_w);
	DECLARE_WRITE_LINE_MEMBER(dack4_w);
	DECLARE_WRITE_LINE_MEMBER(dack5_w);
	DECLARE_WRITE_LINE_MEMBER(dack6_w);
	DECLARE_WRITE_LINE_MEMBER(dack7_w);
	DECLARE_WRITE8_MEMBER(write_rtc);

	DECLARE_WRITE_LINE_MEMBER(shutdown);

	DECLARE_READ8_MEMBER(dma_read_byte);
	DECLARE_WRITE8_MEMBER(dma_write_byte);
	DECLARE_READ8_MEMBER(dma_read_word);
	DECLARE_WRITE8_MEMBER(dma_write_word);

	UINT32 a20_286(bool state);
protected:
	void device_start() override;
	void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
private:
	void set_dma_channel(int channel, int state);
	void speaker_set_spkrdata(UINT8 data);

	required_device<cpu_device> m_maincpu;
	required_device<isa16_device> m_isabus;
	required_device<pic8259_device> m_pic8259_slave;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pit8254_device> m_pit8254;
	required_device<speaker_sound_device> m_speaker;
	required_device<mc146818_device> m_mc146818;
	UINT8 m_at_spkrdata;
	UINT8 m_pit_out2;
	int m_dma_channel;
	bool m_cur_eop;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT16 m_dma_high_byte;
	UINT8 m_at_speaker;
	UINT8 m_channel_check;
	UINT8 m_nmi_enabled;
};

extern const device_type AT_MB;

MACHINE_CONFIG_EXTERN(at_softlists);

#endif
