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

#include "cxd8442q.h"

#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CXD8442Q, cxd8442q_device, "cxd8442q", "Sony CXD8442Q WSC-FIFOQ")

cxd8442q_device::cxd8442q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : device_t(mconfig, CXD8442Q, tag, owner, clock),
                                                                                                                    fifoChannels{FifoChannel(*this), FifoChannel(*this), FifoChannel(*this), FifoChannel(*this)}
{
}

void cxd8442q_device::map(address_map &map)
{
    // Each channel has the same structure
    // The devices are mapped at the platform level, so this device only needs to handle the DMA and assorted details
    for (int channel = 0; channel < FIFO_CH_TOTAL; ++channel)
    {
        int channel_base = FIFO_REGION_OFFSET * channel; // start of a channel's registers

        map(channel_base + 0x00, channel_base + 0x03).lrw32(NAME(([this, channel]() { return fifoChannels[channel].mask; })), NAME(([this, channel](uint32_t data) {
                                                                LOG("FIFO CH%d: Setting mask to 0x%x\n", channel, data);
                                                                fifoChannels[channel].mask = data;
                                                            })));

        map(channel_base + 0x04, channel_base + 0x07).lrw32(NAME(([this, channel]() { return fifoChannels[channel].address; })), NAME(([this, channel](uint32_t data) { 
            LOG("FIFO CH%d: Setting address to 0x%x\n", channel, data);
            fifoChannels[channel].address = data; })));

        map(channel_base + 0x0c, channel_base + 0x0f).lrw32(NAME(([this, channel]() { return fifoChannels[channel].dma_mode; })), NAME(([this, channel](uint32_t data) {
                                                                LOG("FIFO CH%d: Setting DMA mode to 0x%x\n", channel, data);
                                                                fifoChannels[channel].dma_mode = data;
                                                                if (data & FifoChannel::DMA_EN)
                                                                {
                                                                    fifoChannels[channel].reset_for_transaction();
                                                                    fifo_timer->adjust(attotime::zero, 0, attotime::from_usec(1)); // actual clock rate? this is a "guess" (read: I tried it and it happened to work)
                                                                }
                                                            })));

        map(channel_base + 0x30, channel_base + 0x33).lr32(NAME(([this, channel]() { return fifoChannels[channel].valid_count; })));

        map(channel_base + 0x34, channel_base + 0x37).lr32(NAME(([this, channel]() { return fifoChannels[channel].read_data_from_fifo(); })));
    }

    // TODO: experiment to see if this is readable/writeable from the real NWS5000X
    map(FIFO_RAM_OFFSET, FIFO_RAM_OFFSET + FIFO_MAX_RAM_SIZE - 1).lrw32(NAME(([this](offs_t offset) { return fifo_ram[offset]; })), NAME(([this](offs_t offset, uint32_t data) { fifo_ram[offset] = data; })));
}

void cxd8442q_device::device_add_mconfig(machine_config &config) {}

void cxd8442q_device::device_start()
{
    fifo_ram = std::make_unique<uint32_t[]>(FIFO_MAX_RAM_SIZE);
    save_pointer(NAME(fifo_ram), FIFO_MAX_RAM_SIZE);
    fifo_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cxd8442q_device::fifo_dma_execute), this));
}

void cxd8442q_device::device_reset()
{
    for (int channel = 0; channel < FIFO_CH_TOTAL; ++channel)
    {
        fifoChannels[channel].reset();
    }
}

TIMER_CALLBACK_MEMBER(cxd8442q_device::fifo_dma_execute)
{
    // TODO: mem->device
    bool dma_active = false;
    for (int channel = 0; channel < FIFO_CH_TOTAL; ++channel)
    {
        FifoChannel &thisChannel = fifoChannels[channel];

        // Skip this channel if we don't need to do anything
        if (!(thisChannel.dma_mode & FifoChannel::DMA_EN))
        {
            continue;
        }
        dma_active = true;

        // Check DRQ to see if the device is ready to give us data
        if (thisChannel.drq_r())
        {
            thisChannel.dma_cycle(); // TODO: error condition?
        }

        // Check if we have maxed out what the CPU configured as the FIFO size
        if (thisChannel.mask == thisChannel.valid_count)
        {
            // TODO: do we need to signal something to the CPU? Interrupt, etc?
            thisChannel.dma_mode = 0x0; // HACK to prevent infinite running of this thread
        }
    }

    // If no channels were doing anything, we can terminate this activity for now
    if (!dma_active)
    {
        fifo_timer->adjust(attotime::never);
    }
}

void cxd8442q_device::FifoChannel::dma_cycle()
{
    // Write byte to RAM
    // TODO: mem->dev direction
    // TODO: how to detect FIFO overrun? Is that even signaled to the CPU?
    if (dma_r_callback != nullptr && (dma_mode & DMA_EN) > 0)
    {
        // Grab our next chunk of data (might just be a byte, needs more investigation)
        fifo_device.fifo_ram[address + fifo_w_position] = dma_r_callback();
        ++valid_count;

        // Increment and check if we need to wrap around
        if (++fifo_w_position > mask)
        {
            fifo_w_position = 0;
        }
    }
}

uint32_t cxd8442q_device::FifoChannel::read_data_from_fifo()
{
    // read data out of RAM at the current read position (relative to the start address)
    uint32_t value = fifo_device.fifo_ram[address + fifo_r_position];

    // Increment and check if we need to wrap around
    if (++fifo_r_position > mask)
    {
        fifo_r_position = 0;
    }

    // TODO: detect when FIFO has no valid data? Or is that a silent failure on the system?

    // Based on testing with the 5000X FDC subsystem, the monitor ROM uses `lb` commands in the 4-byte region
    // to read out the value of the 8-bit floppy data register. So, this ensures that the right data shows up
    // regardless of byte offset. Will need more experimentation to see if the FIFO always acts like that
    // or if the FDC is wired to cause this behavior.
    return (value << 24) | (value << 16) | (value << 8) | value;
}
