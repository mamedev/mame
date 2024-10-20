// license:BSD-3-Clause
// copyright-holders:Brice Onken,Tsubai Masanari,Olivier Galibert
// thanks-to:Patrick Mackinlay

/*
 * HP 1TV3-0302 SPIFI3-SE SCSI controller
 *
 * Datasheets for this seem to be impossible to find - the only avaliable implementation to reference that I have
 * found is the Sony NEWS APBus NetBSD driver. Hopefully a datasheet will turn up eventually.
 * Based on internet research, it seems some HP PA-RISC systems also used the SPIFI3, including the E55.
 *
 * Because this driver was developed to work with NetBSD, NEWS-OS, and the NWS-5000 monitor ROM, only
 * the features and flows that Sony used are implemented. Emulating non-Sony designs using this chip will likely
 * require similar RE work to determine the exact SPIFI features used and add support for them into this driver.
 * In its current state, this driver is unlikely to work out of the box with any other machines.
 *
 * Register definitions were derived from the NetBSD source code, copyright (c) 2000 Tsubai Masanari.
 * SCSI state machine code was derived from the MAME NCR53C90 driver, copyright (c) Olivier Galibert
 *
 * References:
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifireg.h
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 * - https://github.com/mamedev/mame/blob/master/src/devices/machine/ncr53c90.cpp
 */

#ifndef MAME_MACHINE_SPIFI3_H
#define MAME_MACHINE_SPIFI3_H

#pragma once

#include "machine/nscsi_bus.h"

class spifi3_device
	: public nscsi_device,
	  public nscsi_slot_card_interface
{
public:
	spifi3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	void map(address_map &map) ATTR_COLD;

	auto irq_handler_cb() { return m_irq_handler.bind(); }
	auto drq_handler_cb() { return m_drq_handler.bind(); }

	uint8_t dma_r();
	void dma_w(uint8_t val);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void scsi_ctrl_changed() override;

private:
	static constexpr int FIFO_SIZE = 8;

	enum scsi_mode : uint8_t
	{
		MODE_D, // Disconnected
		MODE_T, // Target
		MODE_I  // Initiator
	};

	enum scsi_data_target : uint8_t
	{
		COMMAND_BUFFER,
		FIFO
	};

	enum dma_direction : uint8_t
	{
		DMA_NONE,
		DMA_IN,
		DMA_OUT
	};

	struct spifi_cmd_entry
	{
		// NetBSD has these mapped as uint32_t to align the accesses and such
		// in reality, these are all 8-bit values that are mapped, in typical NWS-5000 series
		// fashion, to be 32-bit word aligned.
		// the same probably applies to the register file.
		uint8_t cdb[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		uint8_t quecode = 0;
		uint8_t quetag = 0;
		uint8_t idmsg = 0;
		uint8_t status = 0;
	};

	struct register_file
	{
		uint32_t spstat = 0;
		uint32_t cmlen = 0;
		uint32_t cmdpage = 0;
		// count_hi, count_mid, count_low

		uint32_t svptr_hi = 0;
		uint32_t svptr_mid = 0;
		uint32_t svptr_low = 0;

		uint32_t intr = 0;
		uint32_t imask = 0;
		uint32_t prctrl = 0;

		uint32_t prstat = 0;
		uint32_t init_status = 0;
		uint32_t fifoctrl = 0;
		uint32_t fifodata = 0;

		uint32_t config = 0;
		uint32_t data_xfer = 0;
		uint32_t autocmd = 0;
		uint32_t autostat = 0;

		uint32_t resel = 0;
		uint32_t select = 0;
		// prcmd, which is used to trigger commands
		uint32_t auxctrl = 0;

		uint32_t autodata = 0;
		uint32_t loopctrl = 0;
		uint32_t loopdata = 0;
		uint32_t identify = 0;

		uint32_t complete = 0;
		uint32_t scsi_status = 0x1; // Must be 0x1 for SPIFI to be recognized at boot
		uint32_t data = 0;
		uint32_t icond = 0;

		uint32_t fastwide = 0;
		uint32_t exctrl = 0;
		uint32_t exstat = 0;
		uint32_t test = 0;

		uint32_t quematch = 0;
		uint32_t quecode = 0;
		uint32_t quetag = 0;
		uint32_t quepage = 0;

		spifi_cmd_entry cmbuf[8];
	} spifi_reg;

	template<typename T>
	class spifi_queue
	{
	public:
		uint32_t head;
		uint32_t tail;
		uint32_t size;
		T fifo[FIFO_SIZE];

		spifi_queue() { clear_queue(); }

		uint32_t get_size() { return size; }

		bool empty() { return size == 0; }

		bool full() { return size == FIFO_SIZE; }

		void clear_queue()
		{
			head = 0;
			tail = 0;
			size = 0;

			for (int i = 0; i < FIFO_SIZE; ++i)
			{
				fifo[i] = 0;
			}
		}

		void push(T value)
		{
			if(size == FIFO_SIZE)
			{
				fatalerror("spifi3: FIFO overflow!");
			}

			fifo[tail] = value;
			tail = (tail + 1) % FIFO_SIZE;
			++size;
		}

		T pop()
		{
			if (size == 0)
			{
				fatalerror("spifi3: FIFO underflow!");
			}

			const T removed_value = fifo[head];
			head = (head + 1) % FIFO_SIZE;
			--size;
			return removed_value;
		}
	};

	// State tracking variables
	dma_direction dma_dir;
	scsi_mode mode;
	scsi_data_target xfr_data_source;
	uint32_t state;
	uint32_t xfr_phase;
	uint32_t command_pos;
	bool irq = false;
	bool drq = false;
	uint32_t tcounter;
	uint8_t sync_period = 5; // TODO: appropriate value for SPIFI
	uint8_t clock_conv = 2; // TODO: appropriate value for SPIFI
	uint32_t bus_id;
	spifi_queue<uint8_t> m_even_fifo;
	spifi_queue<uint8_t> m_odd_fifo;
	emu_timer *tm;

	// I/O ports
	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;

	// State-related methods
	TIMER_CALLBACK_MEMBER(tick);
	void step(bool timeout);
	void check_irq();
	void check_drq();
	void reset_disconnect();
	void send_byte(scsi_data_target data_source);
	void recv_byte();
	void function_complete();
	void function_bus_complete();
	void bus_complete();
	void dma_set(dma_direction dir);
	void decrement_tcounter(uint32_t count = 1);
	bool transfer_count_zero();
	void delay(uint32_t cycles);
	void delay_cycles(uint32_t cycles);
	void arbitrate();
	void clear_fifo();
	void auto_phase_transfer(uint32_t new_phase);
	void start_autodata(uint32_t data_phase);
	void start_autostat();
	void start_automsg(uint32_t msg_phase);
	void start_autocmd();
	dma_direction dma_setting(uint32_t target_id);

	// Register processing methods
	uint32_t get_target_id();
	bool autodata_active(uint32_t target_id);
	bool autodata_in(uint32_t target_id);
	bool autodata_out(uint32_t target_id);
	void autostat_done(uint32_t target_id);
	bool autostat_active(uint32_t target_id);
	bool automsg_active();
	bool autocmd_active();

	// Register accessors
	uint32_t spstat_r();
	uint32_t cmlen_r();
	void cmlen_w(uint32_t data);
	uint32_t cmdpage_r();
	void cmdpage_w(uint32_t data);
	uint32_t count_r(offs_t offset);
	void count_w(offs_t offset, uint32_t data);
	uint32_t intr_r();
	void intr_w(uint32_t data);
	uint32_t imask_r();
	void imask_w(uint32_t data);
	uint32_t prstat_r();
	uint32_t init_status_r();
	uint32_t fifoctrl_r();
	void fifoctrl_w(uint32_t data);
	uint32_t data_xfer_r();
	void data_xfer_w(uint32_t data);
	uint32_t autocmd_r();
	void autocmd_w(uint32_t data);
	uint32_t autostat_r();
	void autostat_w(uint32_t data);
	uint32_t select_r();
	void select_w(uint32_t data);
	void prcmd_w(uint32_t data);
	uint32_t auxctrl_r();
	void auxctrl_w(uint32_t data);
	uint32_t autodata_r();
	void autodata_w(uint32_t data);
	uint32_t identify_r();
	void identify_w(uint32_t data);
	uint32_t scsi_status_r();
	void scsi_status_w(uint32_t data);
	uint32_t icond_r();
	void icond_w(uint32_t data);
	uint32_t fastwide_r();
	void fastwide_w(uint32_t data);
	uint32_t exctrl_r();
	void exctrl_w(uint32_t data);

	// Command buffer accessors
	uint8_t cmd_buf_r(offs_t offset);
	void cmd_buf_w(offs_t offset, uint8_t data);

	// Data helpers
	bool dma_command(dma_direction current_direction) const
	{
		return current_direction != DMA_NONE;
	}
};

DECLARE_DEVICE_TYPE(SPIFI3, spifi3_device)

#endif // MAME_MACHINE_SPIFI3_H
