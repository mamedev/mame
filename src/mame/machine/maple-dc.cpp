// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "maple-dc.h"
#include "mie.h"

DEFINE_DEVICE_TYPE(MAPLE_DC, maple_dc_device, "maple_dc", "Dreamcast Maple Bus")

void maple_dc_device::amap(address_map &map)
{
	map(0x04, 0x07).rw(FUNC(maple_dc_device::sb_mdstar_r), FUNC(maple_dc_device::sb_mdstar_w));
	map(0x10, 0x13).rw(FUNC(maple_dc_device::sb_mdtsel_r), FUNC(maple_dc_device::sb_mdtsel_w));
	map(0x14, 0x17).rw(FUNC(maple_dc_device::sb_mden_r), FUNC(maple_dc_device::sb_mden_w));
	map(0x18, 0x1b).rw(FUNC(maple_dc_device::sb_mdst_r), FUNC(maple_dc_device::sb_mdst_w));
	map(0x80, 0x83).rw(FUNC(maple_dc_device::sb_msys_r), FUNC(maple_dc_device::sb_msys_w));
	map(0x8c, 0x8f).w(FUNC(maple_dc_device::sb_mdapro_w));
}

maple_dc_device::maple_dc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MAPLE_DC, tag, owner, clock),
	cpu(*this, finder_base::DUMMY_TAG),
	irq_cb(*this)
{
	// Do not move that in device_start or there will be a race
	// condition with the maple devices call to register_port.
	memset(devices, 0, sizeof(devices));
}

void maple_dc_device::register_port(int port, maple_device *device)
{
	if(devices[port])
		fatalerror("maple_dc_device: duplicate registration on port %d\n", port);

	devices[port] = device;
}

void maple_dc_device::device_start()
{
	logerror("maple_dc_device started\n");
	timer = timer_alloc(0);
	irq_cb.resolve_safe();

	mdstar = 0;

	save_item(NAME(mdstar));
	save_item(NAME(mden));
	save_item(NAME(mdst));
	save_item(NAME(msys));
	save_item(NAME(mdtsel));
	save_item(NAME(dma_state));
	save_item(NAME(dma_adr));
	save_item(NAME(dma_port));
	save_item(NAME(dma_dest));
	save_item(NAME(dma_endflag));
}

void maple_dc_device::device_reset()
{
	mden = 0;
	mdst = 0;
	msys = 0;
	mdtsel = 0;
	dma_state = DMA_IDLE;
	dma_adr = 0;
	dma_port = 0;
	dma_dest = 0;
	dma_endflag = false;
}

void maple_dc_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	timer.adjust(attotime::never);

	switch(dma_state) {
	case DMA_WAIT_REPLY:
		dma_state = DMA_TIMEOUT;
		dma_step();
		break;

	case DMA_WAIT_NOP:
		dma_state = DMA_SEND;
		dma_step();
		break;

	case DMA_DONE:
		dma_state = DMA_IDLE;
		mdst = 0;
		irq_cb(DMA_MAPLE_IRQ);
		break;

	default:
		logerror("MAPLE: Unexpected timer callback trigger\n");
		break;
	}
}

void maple_dc_device::dma_step()
{
	for(;;) {
		switch(dma_state) {
		case DMA_SEND: {
			sh4_ddt_dma ddtdata;
			uint32_t header[2];
			uint32_t data[512];
			ddtdata.source    = dma_adr; // source address
			ddtdata.length    = 2;       // words to transfer
			ddtdata.size      = 4;       // bytes per word
			ddtdata.buffer    = header;  // destination buffer
			ddtdata.direction = 0;       // 0 source to buffer, 1 buffer to source
			ddtdata.channel   = 0;
			ddtdata.mode      = -1;      // copy from/to buffer
			cpu->sh4_dma_ddt(&ddtdata);
			dma_adr += 8;

			dma_endflag    = header[0] & 0x80000000;
			dma_port       = (header[0] >> 16) & 3;
			uint32_t pattern = (header[0] >> 8) & 7;
			uint32_t length  = (header[0] & 255) + 1;
			dma_dest       = header[1];

			ddtdata.source    = dma_adr; // source address
			ddtdata.length    = length;  // words to transfer
			ddtdata.size      = 4;       // bytes per word
			ddtdata.buffer    = data;    // destination buffer
			ddtdata.direction = 0;       // 0 source to buffer, 1 buffer to source
			ddtdata.channel   = 0;
			ddtdata.mode      = -1;      // copy from/to buffer
			cpu->sh4_dma_ddt(&ddtdata);
			dma_adr += length*4;

			switch(pattern) {
			case 0: // start
				if(devices[dma_port])
					devices[dma_port]->maple_w(data, length);
				else {
					// Avoid spending time on that specific timeout
					dma_state = DMA_TIMEOUT;
					break;
				}
				dma_state = DMA_WAIT_REPLY;

				// the MIE seems too slow for the correct timeout
				// it's rather strange though
				//              timer->adjust(attotime::from_nsec(40000 + 20*(msys>>16))); // The 40us represent the sending and reception time
				timer->adjust(attotime::from_msec(5));
				break;
			case 2: // sdckb occupy permission (light gun protocol)
				logerror("MAPLE: sdckb occupy permission\n");
				break;
			case 3: // reset
				if(devices[dma_port])
					devices[dma_port]->maple_reset();
				break;
			case 4: // sdckb occupy cancel
				logerror("MAPLE: sdckb occupy cancel\n");
				break;
			case 7: // nop
				logerror("MAPLE: nop\n");
				dma_state = DMA_WAIT_NOP;
				break;
			}
			if(dma_state == DMA_SEND && dma_endflag)
				dma_state = DMA_DONE;
			break;
		}

		case DMA_WAIT_REPLY:
			return;

		case DMA_WAIT_NOP:
			return;

		case DMA_TIMEOUT: {
			sh4_ddt_dma ddtdata;
			uint32_t data = 0xffffffff;
			ddtdata.destination = dma_dest; // destination address
			ddtdata.length      = 1;        // words to transfer
			ddtdata.size        = 4;        // bytes per word
			ddtdata.buffer      = &data;    // destination buffer
			ddtdata.direction   = 1;        // 0 source to buffer, 1 buffer to source
			ddtdata.channel     = 0;
			ddtdata.mode        = -1;       // copy from/to buffer
			cpu->sh4_dma_ddt(&ddtdata);
			dma_state = dma_endflag ? DMA_DONE : DMA_SEND;
			break;
		}

		case DMA_GOT_REPLY: {
			timer->adjust(attotime::never);

			sh4_ddt_dma ddtdata;
			uint32_t data[512];
			uint32_t length = 0;
			bool partial = false;
			if(devices[dma_port])
				devices[dma_port]->maple_r(data, length, partial);
			else
				fatalerror("MAPLE: reading from unconnected device on port %d\n", dma_port);

			if(length) {
				ddtdata.destination = dma_dest; // destination address
				ddtdata.length      = length;   // words to transfer
				ddtdata.size        = 4;        // bytes per word
				ddtdata.buffer      = data;     // destination buffer
				ddtdata.direction   = 1;        // 0 source to buffer, 1 buffer to source
				ddtdata.channel     = 0;
				ddtdata.mode        = -1;       // copy from/to buffer
				cpu->sh4_dma_ddt(&ddtdata);
				dma_dest += length*4;
			}

			if(partial)
				dma_state = DMA_WAIT_REPLY;
			else
				dma_state = dma_endflag ? DMA_DONE : DMA_SEND;
			break;
		}
		case DMA_DONE:
			timer->adjust(attotime::from_usec(200));
			return;
		}
	}
}

void maple_dc_device::end_of_reply()
{
	if(dma_state == DMA_WAIT_REPLY) {
		dma_state = DMA_GOT_REPLY;
		dma_step();
	} else
		logerror("MAPLE: Unexpected end of reply\n");
}

void maple_dc_device::maple_hw_trigger()
{
	if(mdtsel & 1) // HW trigger
	{
		dma_adr = mdstar;
		dma_state = DMA_SEND;
		dma_step();
	}
}

uint32_t maple_dc_device::sb_mdstar_r()
{
	return mdstar;
}

void maple_dc_device::sb_mdstar_w(uint32_t data)
{
	mdstar = data & ~31;
}

uint32_t maple_dc_device::sb_mden_r()
{
	return mden;
}

void maple_dc_device::sb_mden_w(uint32_t data)
{
	mden = data & 1;
}

uint32_t maple_dc_device::sb_mdtsel_r()
{
	return mdtsel;
}

void maple_dc_device::sb_mdtsel_w(uint32_t data)
{
	mdtsel = data & 1;
}

uint32_t maple_dc_device::sb_mdst_r()
{
	return dma_state != DMA_IDLE ? 1 : 0;
}

void maple_dc_device::sb_mdst_w(uint32_t data)
{
	uint32_t old = mdst;
	mdst = data & 1;

	if(!old && data && (mden & 1) && mdtsel == 0) {
		dma_adr = mdstar;
		dma_state = DMA_SEND;
		dma_step();
	}
}

uint32_t maple_dc_device::sb_msys_r()
{
	return msys;
}

void maple_dc_device::sb_msys_w(uint32_t data)
{
	msys = data;
}

void maple_dc_device::sb_mdapro_w(uint32_t data)
{
}
