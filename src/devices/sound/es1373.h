// license:BSD-3-Clause
// copyright-holders:Ted Green
// Creative Labs Ensonic AudioPCI97 ES1373

#pragma once

#ifndef ES1373_H
#define ES1373_H

#include "machine/pci.h"

// No interrupts
#define MCFG_ES1373_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ES1373, 0x12741371, 0x04, 0x040100, 0x12741371)

#define MCFG_ES1373_IRQ_ADD(_cpu_tag, _irq_num) \
	downcast<es1373_device *>(device)->set_irq_info(_cpu_tag, _irq_num);

/* Ensonic ES1373 registers 0x00-0x3f */
#define ES_INT_CS_CTRL          (0x00/4)
#define ES_INT_CS_STATUS        (0x04/4)
#define ES_UART_DATA            (0x08/4)
#define ES_UART_STATUS          (0x09/4)
#define ES_UART_CTRL            (0x09/4)
#define ES_UART_RSVD            (0x0A/4)
#define ES_MEM_PAGE             (0x0C/4)
#define ES_SRC_IF               (0x10/4)
#define ES_CODEC                (0x14/4)
#define ES_LEGACY               (0x18/4)
#define ES_CHAN_CTRL            (0x1C/4)
#define ES_SERIAL_CTRL          (0x20/4)
#define ES_DAC1_CNT             (0x24/4)
#define ES_DAC2_CNT             (0x28/4)
#define ES_ADC_CNT              (0x2C/4)
#define ES_HOST_IF0             (0x30/4)
#define ES_HOST_IF1             (0x34/4)
#define ES_HOST_IF2             (0x38/4)
#define ES_HOST_IF3             (0x3C/4)

// Interrupt/Chip Select Control Register (ES_INT_CS_CTRL) bits
#define ICCTRL_ADC_STOP_MASK   0x00002000
#define ICCTRL_DAC1_EN_MASK    0x00000040
#define ICCTRL_DAC2_EN_MASK    0x00000020
#define ICCTRL_ADC_EN_MASK     0x00000010
#define ICCTRL_UART_EN_MASK    0x00000008
#define ICCTRL_JYSTK_EN_MASK   0x00000004

// Interrupt/Chip Select Status Register (ES_INT_CS_STATUS) bits
#define ICSTATUS_INTR_MASK        0x80000000
#define ICSTATUS_DAC1_INT_MASK    0x00000004
#define ICSTATUS_DAC2_INT_MASK    0x00000002
#define ICSTATUS_ADC_INT_MASK     0x00000001

// Serial Interface Control Register (ES_SERIAL_CTRL) bits
#define SCTRL_P2_END_MASK     0x00380000
#define SCTRL_P2_START_MASK   0x00070000
#define SCTRL_R1_LOOP_MASK    0x00008000
#define SCTRL_P2_LOOP_MASK    0x00004000
#define SCTRL_P1_LOOP_MASK    0x00002000
#define SCTRL_P2_PAUSE_MASK   0x00001000
#define SCTRL_P1_PAUSE_MASK   0x00000800
#define SCTRL_R1_INT_EN_MASK  0x00000400
#define SCTRL_P2_INT_EN_MASK  0x00000200
#define SCTRL_P1_INT_EN_MASK  0x00000100
#define SCTRL_P1_RELOAD_MASK  0x00000080
#define SCTRL_P2_STOP_MASK    0x00000040
#define SCTRL_R1_S_MASK       0x00000030
#define SCTRL_P2_S_MASK       0x0000000C
#define SCTRL_P1_S_MASK       0x00000003

#define SCTRL_8BIT_MONO             0x0
#define SCTRL_8BIT_STEREO           0x1
#define SCTRL_16BIT_MONO            0x2
#define SCTRL_16BIT_STEREO      0x3

#define ES_PCI_READ 0
#define ES_PCI_WRITE 1

struct chan_info {
	int number;
	bool enable;
	bool int_en;
	bool loop_en;
	bool initialized;
	UINT8  format;       // Format of channel
	UINT32 buf_wptr;     // Address to sample cache memory
	UINT32 buf_rptr;     // Address to sample cache memory
	UINT16 buf_count;    // Number of samples that have been played
	UINT16 buf_size;     // Number of samples minus one to play
	UINT32 pci_addr;     // PCI Addresss for system memory accesses
	UINT16 pci_count;    // Number of 32 bits transfered
	UINT16 pci_size;     // Total number of words (32 bits) minus one in system memory
};

class es1373_device : public pci_device, public device_sound_interface
{
public:
	es1373_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space) override;

	void set_irq_info(const char *tag, const int irq_num);

	DECLARE_READ32_MEMBER (reg_r);
	DECLARE_WRITE32_MEMBER(reg_w);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// Sound stream
	sound_stream *m_stream;

protected:
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	FILE *m_eslog;

private:
	UINT32 m_tempCount;
	emu_timer *m_timer;
	address_space *m_memory_space;
	const char *m_cpu_tag;
	cpu_device *m_cpu;
	int m_irq_num;
	DECLARE_ADDRESS_MAP(map, 32);
	UINT16 m_ac97_regs[0x80];
	UINT32 m_es_regs[0x10];
	UINT32 m_sound_cache[0x40];
	UINT16 m_src_ram[0x80];
	chan_info m_dac1;
	chan_info m_dac2;
	chan_info m_adc;
	void transfer_pci_audio(chan_info& chan, int type);
	UINT32 calc_size(const UINT8 &format);
	void send_audio_out(chan_info& chan, UINT32 intr_mask, stream_sample_t *outL, stream_sample_t *outR, int samples);

};

extern const device_type ES1373;

#endif
