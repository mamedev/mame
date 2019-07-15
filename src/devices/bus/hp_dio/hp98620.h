// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#ifndef MAME_BUS_HPDIO_98620_H
#define MAME_BUS_HPDIO_98620_H

#pragma once

#include "hp_dio.h"


namespace bus {
	namespace hp_dio {

class dio16_98620_device :
		public device_t,
		public device_dio32_card_interface
{
public:
	// construction/destruction
	dio16_98620_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dio16_98620_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	DECLARE_READ16_MEMBER(dma_r);
	DECLARE_WRITE16_MEMBER(dma_w);

	DECLARE_WRITE_LINE_MEMBER(irq_w);

private:

	static constexpr int REG0_RESET_ARM_INT = 0x00;
	static constexpr int REG0_ADDRESS_HIGH = 0x00;
	static constexpr int REG0_ADDRESS_LOW = 0x02;
	static constexpr int REG0_TRANSFER_COUNT = 0x04;
	static constexpr int REG0_CONTROL_ARM = 0x06;
	static constexpr int REG0_STATUS = 0x06;

	static constexpr int REG1_RESET_ARM_INT = 0x08;
	static constexpr int REG1_ADDRESS_HIGH = 0x08;
	static constexpr int REG1_ADDRESS_LOW = 0x0a;
	static constexpr int REG1_TRANSFER_COUNT = 0x0c;
	static constexpr int REG1_CONTROL_ARM = 0x0e;
	static constexpr int REG1_STATUS = 0x0e;

	static constexpr uint8_t REG_CONTROL_IE = 1 << 0;
	static constexpr uint8_t REG_CONTROL_WORD = 1 << 1;
	static constexpr uint8_t REG_CONTROL_OUT = 1 << 2;
	static constexpr uint8_t REG_CONTROL_PRI = 1 << 3;
	static constexpr uint8_t REG_CONTROL_INT_MASK = 0x7;
	static constexpr int REG_CONTROL_INT_SHIFT = 4;


	static constexpr uint8_t REG_STATUS_ARMED = 1 << 0;
	static constexpr uint8_t REG_STATUS_INT = 1 << 1;

	/* general control registers */
	static constexpr int REG_1TQ4_ID_LOW = 0x10;
	static constexpr int REG_1TQ4_ID_HIGH = 0x12;
	static constexpr int REG_GENERAL_CONTROL = 0x14;
	static constexpr int REG_GENERAL_CONTROL_RESET0 = 1 << 4;
	static constexpr int REG_GENERAL_CONTROL_RESET1 = 1 << 5;

	/* channel specific registers */
	static constexpr int REG0_1TQ4_ADDRESS_HIGH = 0x100;
	static constexpr int REG0_1TQ4_ADDRESS_LOW = 0x102;
	static constexpr int REG0_1TQ4_TRANSFER_COUNT_HIGH = 0x104;
	static constexpr int REG0_1TQ4_TRANSFER_COUNT_LOW = 0x106;

	static constexpr int REG0_1TQ4_CONTROL = 0x108;
	static constexpr int REG0_1TQ4_STATUS = 0x10a;

	static constexpr uint16_t REG_1TQ4_CONTROL_LWORD = 1 << 8;
	static constexpr uint16_t REG_1TQ4_CONTROL_START = 1 << 15;

	static constexpr uint16_t REG_1TQ4_STATUS_ARMED = 1 << 0;
	static constexpr uint16_t REG_1TQ4_STATUS_INT = 1 << 1;

	/* registers */
	static constexpr int REG1_1TQ4_ADDRESS_HIGH = 0x200;
	static constexpr int REG1_1TQ4_ADDRESS_LOW = 0x202;
	static constexpr int REG1_1TQ4_TRANSFER_COUNT_HIGH = 0x204;
	static constexpr int REG1_1TQ4_TRANSFER_COUNT_LOW = 0x206;

	static constexpr int REG1_1TQ4_CONTROL = 0x208;
	static constexpr int REG1_1TQ4_STATUS = 0x20a;

	WRITE_LINE_MEMBER(dmar0_in) override;
	WRITE_LINE_MEMBER(dmar1_in) override;

	void dma_transfer(int channel);
	void update_irq();
	void update_ctrl(const int channel, const uint16_t data, const bool is_1tq4);
	uint16_t get_ctrl(const int channel);

	bool     m_installed_io;
	uint8_t  m_control;
	uint8_t  m_data;
	bool m_irq_state;

	struct dma_regs {
		uint32_t address;
		uint32_t tc;
		uint32_t control;
		/* control register */
		int irq_level;
		int tsz;
		int subcount;

		bool irq;
		bool ie;
		bool armed;

		bool dma_out;
		bool dma_pri; // TODO
		bool lword;
		bool word;
	} m_regs[2];

	bool dmar[2];
};

} } // namespace bus::hp_dio

DECLARE_DEVICE_TYPE_NS(HPDIO_98620, bus::hp_dio, dio16_98620_device)

#endif // MAME_BUS_HPDIO_98620_H
