// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

/*

    am9517a.c is a more complete implementation of this, the uPD71071 appears to be a clone of it

    NEC uPD71071 DMA Controller
    Used on the Fujitsu FM-Towns

    Register description:

    0x00:   Initialise (Write-only)
            - bit 0: Reset
            - bit 1: 16-bit data bus

    0x01:   Channel Register
            On read:
            - bits 0-3: Selected channel
            - bit 4: Only base registers may be read or written
            On write:
            - bits 0-1: Select channel for programming count, address, and mode registers
            - bit 2: Only base registers can be read or written to

    0x02:
    0x03:   Count Register (16-bit)
            DMA Transfer counter

    0x04:
    0x05:
    0x06:
    0x07:   Address Register (32-bit)
            Self-explanatory, I hope. :)
            NOTE: Datasheet clearly shows this as 24-bit, with register 7 unused.
            But the FM-Towns definitely uses reg 7 as bits 24-31.
            The documentation on the V53A manual doesn't show these bits either, maybe it's
            an external connection on the FMT? might be worth checking overflow behavior etc.

    0x08:
    0x09:   Device Control register (16-bit)
            bit 0: Enable memory-to-memory (MTM) transfers
            bit 1: Enable fixed address for channel 0 only (MTM only)
            bit 2: Disable DMA operation (stops HLDRQ signal to the CPU)
            bit 3: Use compressed timing
            bit 4: Rotational Priority
            bit 5: Extended Writing
            bit 6: DMARQ active level (1=active low)
            bit 7: DMAAK active level (1=active high)
            bit 8: Bus mode (0=bus release, 1=bus hold)
            bit 9: Wait Enable during Verify

    0x0a:   Mode Control register
            bit 0: Transfer size (1=16-bit, 0=8-bit,  16-bit data bus size only)
            bit 2-3: Transfer direction (ignored for MTM transfers)
                        00 = Verify
                        01 = I/O to memory
                        10 = memory to I/O
                        11 = invalid
            bit 4: Enable auto-initialise
            bit 5: Address direction (0=increment, 1=decrement, affects only current Address reg)
            bit 6-7: Transfer mode (ignored for MTM transfers)
                        00 = Demand
                        01 = Single
                        10 = Block
                        11 = Cascade

    0x0b:   Status register
            bit 0-3: Terminal count (per channel)
            bit 4-7: DMA request present (external hardware DMA only)

    0x0c:
    0x0d:   Temporary register (16-bit, read-only)
            Stores the last data transferred in an MTM transfer

    0x0e:   Request register
            bit 0-3: Software DMA request (1=set)
            bit 0 only in MTM transfers

    0x0f:   Mask register
            bit 0-3: DMARQ mask
            bits 1 and 0 only in MTM transfers

    Note, the uPD71071 compatible mode of the V53 CPU differs from a real uPD71071 in the following ways



*/

#include "emu.h"
#include "machine/upd71071.h"


const device_type UPD71071 = &device_creator<upd71071_device>;

upd71071_device::upd71071_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, UPD71071, "NEC uPD71071", tag, owner, clock, "upd71071", __FILE__),
				m_upd_clock(0),
				m_out_hreq_cb(*this),
				m_out_eop_cb(*this),
				m_dma_read_0_cb(*this),
				m_dma_read_1_cb(*this),
				m_dma_read_2_cb(*this),
				m_dma_read_3_cb(*this),
				m_dma_write_0_cb(*this),
				m_dma_write_1_cb(*this),
				m_dma_write_2_cb(*this),
				m_dma_write_3_cb(*this),
				m_out_dack_0_cb(*this),
				m_out_dack_1_cb(*this),
				m_out_dack_2_cb(*this),
				m_out_dack_3_cb(*this),
				m_cpu(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd71071_device::device_start()
{
	m_out_hreq_cb.resolve_safe();
	m_out_eop_cb.resolve_safe();
	m_dma_read_0_cb.resolve_safe(0);
	m_dma_read_1_cb.resolve_safe(0);
	m_dma_read_2_cb.resolve_safe(0);
	m_dma_read_3_cb.resolve_safe(0);
	m_dma_write_0_cb.resolve_safe();
	m_dma_write_1_cb.resolve_safe();
	m_dma_write_2_cb.resolve_safe();
	m_dma_write_3_cb.resolve_safe();
	m_out_dack_0_cb.resolve_safe();
	m_out_dack_1_cb.resolve_safe();
	m_out_dack_2_cb.resolve_safe();
	m_out_dack_3_cb.resolve_safe();
	for (auto & elem : m_timer)
	{
		elem = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(upd71071_device::dma_transfer_timer),this));
	}
	m_selected_channel = 0;

	m_reg.device_control = 0;
	m_reg.mask = 0x0f;  // mask all channels
	for (int x = 0; x < 4; x++)
		m_reg.mode_control[x] = 0;

	save_item(NAME(m_reg.initialise));
	save_item(NAME(m_reg.channel));
	save_item(NAME(m_reg.count_current));
	save_item(NAME(m_reg.count_base));
	save_item(NAME(m_reg.address_current));
	save_item(NAME(m_reg.address_base));
	save_item(NAME(m_reg.device_control));
	save_item(NAME(m_reg.mode_control));
	save_item(NAME(m_reg.status));
	save_item(NAME(m_reg.temp_l));
	save_item(NAME(m_reg.temp_h));
	save_item(NAME(m_reg.request));
	save_item(NAME(m_reg.mask));

	save_item(NAME(m_selected_channel));
	save_item(NAME(m_buswidth));
	save_item(NAME(m_dmarq));
	save_item(NAME(m_base));
	save_item(NAME(m_hreq));
	save_item(NAME(m_eop));
}



TIMER_CALLBACK_MEMBER(upd71071_device::dma_transfer_timer)
{
	// single byte or word transfer
	int channel = param;
	UINT16 data = 0;  // data to transfer

	switch (m_reg.mode_control[channel] & 0x0c)
	{
		case 0x00:  // Verify
			break;
		case 0x04:  // I/O -> memory
			switch (channel)
			{
				case 0:
					if (!m_dma_read_0_cb.isnull())
						data = m_dma_read_0_cb(0);
					break;
				case 1:
					if (!m_dma_read_1_cb.isnull())
						data = m_dma_read_1_cb(0);
					break;
				case 2:
					if (!m_dma_read_2_cb.isnull())
						data = m_dma_read_2_cb(0);
					break;
				case 3:
					if (!m_dma_read_3_cb.isnull())
						data = m_dma_read_3_cb(0);
					break;
			}

			if (m_cpu)
			{
				address_space& space = m_cpu->space(AS_PROGRAM);
				space.write_byte(m_reg.address_current[channel], data & 0xff);
			}
			else
			{
				printf("upd71071_device: dma_transfer_timer - write to memory, no dest space %02x\n", data & 0xff);
			}

			if (m_reg.mode_control[channel] & 0x20)  // Address direction
				m_reg.address_current[channel]--;
			else
				m_reg.address_current[channel]++;
			m_reg.count_current[channel]--;
			if(m_reg.count_current[channel] == 0xffff)
			{
				if (m_reg.mode_control[channel] & 0x10)  // auto-initialise
				{
					m_reg.address_current[channel] = m_reg.address_base[channel];
					m_reg.count_current[channel] = m_reg.count_base[channel];
				}
				// TODO: send terminal count
				set_eop(ASSERT_LINE);
			}
			break;
		case 0x08:  // memory -> I/O
			if (m_cpu)
			{
				address_space& space = m_cpu->space(AS_PROGRAM);
				data = space.read_byte(m_reg.address_current[channel]);
			}
			else
			{
				printf("upd71071_device: dma_transfer_timer - read from memory, no src space\n");
				data = 0x00;
			}

			switch (channel)
			{
				case 0:
					if (!m_dma_write_0_cb.isnull())
						m_dma_write_0_cb((offs_t)0, data);
					break;
				case 1:
					if (!m_dma_write_1_cb.isnull())
						m_dma_write_1_cb((offs_t)0, data);
					break;
				case 2:
					if (!m_dma_write_2_cb.isnull())
						m_dma_write_2_cb((offs_t)0, data);
					break;
				case 3:
					if (!m_dma_write_3_cb.isnull())
						m_dma_write_3_cb((offs_t)0, data);
					break;
			}
			if (m_reg.mode_control[channel] & 0x20)  // Address direction
				m_reg.address_current[channel]--;
			else
				m_reg.address_current[channel]++;
			m_reg.count_current[channel]--;
			if(m_reg.count_current[channel] == 0xffff)
			{
				if (m_reg.mode_control[channel] & 0x10)  // auto-initialise
				{
					m_reg.address_current[channel] = m_reg.address_base[channel];
					m_reg.count_current[channel] = m_reg.count_base[channel];
				}
				// TODO: send terminal count
				set_eop(ASSERT_LINE);
			}
			break;
		case 0x0c:  // Invalid
			break;
	}
}

void upd71071_device::soft_reset()
{
	// Does not change base/current address, count, or buswidth
	m_selected_channel = 0;
	m_base = 0;
	for (int x = 0; x < 4; x++)
		m_reg.mode_control[x] = 0;
	m_reg.device_control = 0;
	m_reg.temp_h = 0;
	m_reg.temp_l = 0;
	m_reg.mask = 0x0f;  // mask all channels
	m_reg.status &= ~0x0f;  // clears bits 0-3 only
	m_reg.request = 0;
}

int upd71071_device::dmarq(int state, int channel)
{
	if (state != 0)
	{
		if (m_reg.device_control & 0x0004)
			return 2;

		if (m_reg.mask & (1 << channel))  // is channel masked?
			return 1;

		m_dmarq[channel] = 1;  // DMARQ line is set
		m_reg.status |= (0x10 << channel);

		// start transfer
		switch (m_reg.mode_control[channel] & 0xc0)
		{
			case 0x00:  // Demand
				// TODO
				set_eop(CLEAR_LINE);
				m_timer[channel]->adjust(attotime::from_hz(m_upd_clock), channel);
				break;
			case 0x40:  // Single
				m_timer[channel]->adjust(attotime::from_hz(m_upd_clock), channel);
				break;
			case 0x80:  // Block
				// TODO
				break;
			case 0xc0:  // Cascade
				// TODO
				break;

		}
	}
	else
	{
		m_dmarq[channel] = 0;  // clear DMARQ line
		m_reg.status &= ~(0x10 << channel);
		m_reg.status |= (0x01 << channel);  // END or TC
	}
	return 0;
}

READ8_MEMBER(upd71071_device::read)
{
	UINT8 ret = 0;

	logerror("DMA: read from register %02x\n",offset);
	switch(offset)
	{
		case 0x01:  // Channel
			ret = (1 << m_selected_channel);
			if (m_base != 0)
				ret |= 0x10;
			break;
		case 0x02:  // Count (low)
			if (m_base != 0)
				ret = m_reg.count_base[m_selected_channel] & 0xff;
			else
				ret = m_reg.count_current[m_selected_channel] & 0xff;
			break;
		case 0x03:  // Count (high)
			if (m_base != 0)
				ret = (m_reg.count_base[m_selected_channel] >> 8) & 0xff;
			else
				ret = (m_reg.count_current[m_selected_channel] >> 8) & 0xff;
			break;
		case 0x04:  // Address (low)
			if (m_base != 0)
				ret = m_reg.address_base[m_selected_channel] & 0xff;
			else
				ret = m_reg.address_current[m_selected_channel] & 0xff;
			break;
		case 0x05:  // Address (mid)
			if (m_base != 0)
				ret = (m_reg.address_base[m_selected_channel] >> 8) & 0xff;
			else
				ret = (m_reg.address_current[m_selected_channel] >> 8) & 0xff;
			break;
		case 0x06:  // Address (high)
			if (m_base != 0)
				ret = (m_reg.address_base[m_selected_channel] >> 16) & 0xff;
			else
				ret = (m_reg.address_current[m_selected_channel] >> 16) & 0xff;
			break;
		case 0x07:  // Address (highest)
			if (m_base != 0)
				ret = (m_reg.address_base[m_selected_channel] >> 24) & 0xff;
			else
				ret = (m_reg.address_current[m_selected_channel] >> 24) & 0xff;
			break;
		case 0x08:  // Device control (low)
			ret = m_reg.device_control & 0xff;
			break;
		case 0x09:  // Device control (high)
			ret = (m_reg.device_control >> 8) & 0xff;
			break;
		case 0x0a:  // Mode control
			ret = m_reg.mode_control[m_selected_channel];
			break;
		case 0x0b:  // Status
			ret = m_reg.status;
			m_reg.status &= ~0x0f;  // resets END/TC?
			break;
		case 0x0c:  // Temporary (low)
			ret = m_reg.temp_h;
			break;
		case 0x0d:  // Temporary (high)
			ret = m_reg.temp_l;
			break;
		case 0x0e:  // Request
			ret = m_reg.request;
			break;
		case 0x0f:  // Mask
			ret = m_reg.mask;
			break;
	}
	return ret;
}

WRITE8_MEMBER(upd71071_device::write)
{
	switch (offset)
	{
		case 0x00:  // Initialise
			// TODO: reset (bit 0)
			m_buswidth = data & 0x02;
			if (data & 0x01)
				soft_reset();
			logerror("DMA: Initialise [%02x]\n",data);
			break;
		case 0x01:  // Channel
			m_selected_channel = data & 0x03;
			m_base = data & 0x04;
			logerror("DMA: Channel selected [%02x]\n",data);
			break;
		case 0x02:  // Count (low)
			m_reg.count_base[m_selected_channel] =
				(m_reg.count_base[m_selected_channel] & 0xff00) | data;
			if (m_base == 0)
				m_reg.count_current[m_selected_channel] =
					(m_reg.count_current[m_selected_channel] & 0xff00) | data;
			logerror("DMA: Channel %i Counter set [%04x]\n",m_selected_channel,m_reg.count_base[m_selected_channel]);
			break;
		case 0x03:  // Count (high)
			m_reg.count_base[m_selected_channel] =
				(m_reg.count_base[m_selected_channel] & 0x00ff) | (data << 8);
			if (m_base == 0)
				m_reg.count_current[m_selected_channel] =
					(m_reg.count_current[m_selected_channel] & 0x00ff) | (data << 8);
			logerror("DMA: Channel %i Counter set [%04x]\n",m_selected_channel,m_reg.count_base[m_selected_channel]);
			break;
		case 0x04:  // Address (low)
			m_reg.address_base[m_selected_channel] =
				(m_reg.address_base[m_selected_channel] & 0xffffff00) | data;
			if (m_base == 0)
				m_reg.address_current[m_selected_channel] =
					(m_reg.address_current[m_selected_channel] & 0xffffff00) | data;
			logerror("DMA: Channel %i Address set [%08x]\n",m_selected_channel,m_reg.address_base[m_selected_channel]);
			break;
		case 0x05:  // Address (mid)
			m_reg.address_base[m_selected_channel] =
				(m_reg.address_base[m_selected_channel] & 0xffff00ff) | (data << 8);
			if (m_base == 0)
				m_reg.address_current[m_selected_channel] =
					(m_reg.address_current[m_selected_channel] & 0xffff00ff) | (data << 8);
			logerror("DMA: Channel %i Address set [%08x]\n",m_selected_channel,m_reg.address_base[m_selected_channel]);
			break;
		case 0x06:  // Address (high)
			m_reg.address_base[m_selected_channel] =
				(m_reg.address_base[m_selected_channel] & 0xff00ffff) | (data << 16);
			if (m_base == 0)
				m_reg.address_current[m_selected_channel] =
					(m_reg.address_current[m_selected_channel] & 0xff00ffff) | (data << 16);
			logerror("DMA: Channel %i Address set [%08x]\n",m_selected_channel,m_reg.address_base[m_selected_channel]);
			break;
		case 0x07:  // Address (highest)
			m_reg.address_base[m_selected_channel] =
				(m_reg.address_base[m_selected_channel] & 0x00ffffff) | (data << 24);
			if (m_base == 0)
				m_reg.address_current[m_selected_channel] =
					(m_reg.address_current[m_selected_channel] & 0x00ffffff) | (data << 24);
			logerror("DMA: Channel %i Address set [%08x]\n",m_selected_channel,m_reg.address_base[m_selected_channel]);
			break;
		case 0x08:  // Device control (low)
			m_reg.device_control = (m_reg.device_control & 0xff00) | data;
			logerror("DMA: Device control set [%04x]\n",m_reg.device_control);
			break;
		case 0x09:  // Device control (high)
			m_reg.device_control = (m_reg.device_control & 0x00ff) | (data << 8);
			logerror("DMA: Device control set [%04x]\n",m_reg.device_control);
			break;
		case 0x0a:  // Mode control
			m_reg.mode_control[m_selected_channel] = data;
			logerror("DMA: Channel %i Mode control set [%02x]\n",m_selected_channel,m_reg.mode_control[m_selected_channel]);
			break;
		case 0x0e:  // Request
			m_reg.request = data;
			logerror("DMA: Request set [%02x]\n",data);
			break;
		case 0x0f:  // Mask
			m_reg.mask = data;
			logerror("DMA: Mask set [%02x]\n",data);
			break;
	}
}

WRITE_LINE_MEMBER(upd71071_device::set_hreq)
{
	if (m_hreq != state)
	{
		m_out_hreq_cb(state);
		m_hreq = state;
	}
}

WRITE_LINE_MEMBER(upd71071_device::set_eop)
{
	if (m_eop != state)
	{
		m_out_eop_cb(state);
		m_eop = state;
	}
}
