// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony CXD8442Q WSC-FIFOQ APbus FIFO and DMA controller
 *
 * The FIFOQ is an APbus DMA controller designed for interfacing some of the simpler and lower speed peripherals
 * to the APbus while providing DMA capabilities. Each FIFO chip can support up to 4 devices.
 *
 * The NWS-5000X uses two of these chips to drive the following peripherals:
 *  - Floppy disk controller
 *  - Sound
 *  - ??? (more to come)
 *
 * TODO:
 *  - Hardware-accurate behavior of the FIFO - this is a best guess.
 *  - Actual clock rate
 */

#ifndef MAME_MACHINE_CXD8442Q_H
#define MAME_MACHINE_CXD8442Q_H

#pragma once

#include "emu.h"
#include "device.h"
#include "devfind.h"
#include "mconfig.h"

class cxd8442q_device : public device_t
{
public:
    cxd8442q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    void map(address_map &map);

    // FIFO channels
    enum FifoChannelNumber
    {
        CH0 = 0,
        CH1 = 1,
        CH2 = 2,
        CH3 = 4
    };

    static const int FIFO_CH_TOTAL = 4;
    static const int FIFO_MAX_RAM_SIZE = 524288;   // supposedly the max FIFO ram size
    static const int FIFO_REGION_OFFSET = 0x10000; // offset from one region to the next
    static const int FIFO_RAM_OFFSET = 0x80000;    // offset from the channel 0 control register

    // set a channel's DRQ line
    template <FifoChannelNumber channel_number>
    void drq_w(int state) { fifoChannels[channel_number].drq_w(state); };
    template <FifoChannelNumber channel_number>
    void bind_dma_r(std::function<uint32_t(void)> dma_r) { fifoChannels[channel_number].set_dma_r_callback(dma_r); };
    template <FifoChannelNumber channel_number>
    void bind_dma_w(std::function<void(uint32_t)> dma_w) { fifoChannels[channel_number].set_dma_w_callback(dma_w); };

protected:
    // FIFO RAM (shared, probably)
    std::unique_ptr<uint32_t[]> fifo_ram;

    emu_timer *fifo_timer; // TODO: figure out the real clock rate for this
    TIMER_CALLBACK_MEMBER(fifo_dma_execute);

    // device_t overrides
    virtual void device_start() override;
    virtual void device_reset() override;
    virtual void device_add_mconfig(machine_config &config) override;

    // Class that represents a single FIFO channel. Each instance of the FIFOQ chip has 4 of these.
    class FifoChannel
    {
    public:
        FifoChannel(cxd8442q_device &fifo_device) : fifo_device(fifo_device) {}

        static const int DMA_EN = 0x1;
        // Warning: this is an incomplete definition
        uint32_t mask = 0;
        uint32_t address = 0;  // Not 100% sure, but I think this is the offset into the Ch0+0x80000 region
        uint32_t dma_mode = 0; // This register enables/disables DMA execution (and maybe direction?)
        uint32_t valid_count = 0;

        void reset()
        {
            mask = 0;
            address = 0;
            dma_mode = 0;
            drq = false;
            reset_for_transaction();
        }

        void reset_for_transaction()
        {
            valid_count = 0;
            fifo_w_position = 0;
            fifo_r_position = 0;
        }

        void dma_cycle();

        void set_dma_r_callback(std::function<uint32_t(void)> dma_r)
        {
            dma_r_callback = dma_r;
        }

        void set_dma_w_callback(std::function<void(uint32_t)> dma_w)
        {
            dma_w_callback = dma_w;
        }

        uint32_t read_data_from_fifo();

        void drq_w(int state)
        {
            drq = state != 0;
        }

        bool drq_r()
        {
            return drq;
        }

    private:
        // reference to parent device
        cxd8442q_device &fifo_device;

        // state tracking
        bool drq = false;

        // data pointers (I think within [address, address + mask])
        uint32_t fifo_w_position = 0;
        uint32_t fifo_r_position = 0;

        // Callback pointers
        std::function<uint32_t(void)> dma_r_callback;
        std::function<void(uint32_t)> dma_w_callback;

        // TODO: IRQ
    };

    FifoChannel fifoChannels[FIFO_CH_TOTAL];
};

DECLARE_DEVICE_TYPE(CXD8442Q, cxd8442q_device)

#endif // MAME_MACHINE_CXD8442Q_H
