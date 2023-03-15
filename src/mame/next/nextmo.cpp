// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "nextmo.h"

DEFINE_DEVICE_TYPE(NEXTMO, nextmo_device, "nextmo", "NeXT Magneto-optical drive")

void nextmo_device::map(address_map &map)
{
	map(0x04, 0x04).rw(FUNC(nextmo_device::r4_r), FUNC(nextmo_device::r4_w));
	map(0x05, 0x05).rw(FUNC(nextmo_device::r5_r), FUNC(nextmo_device::r5_w));
	map(0x06, 0x06).rw(FUNC(nextmo_device::r6_r), FUNC(nextmo_device::r6_w));
	map(0x07, 0x07).rw(FUNC(nextmo_device::r7_r), FUNC(nextmo_device::r7_w));
	map(0x08, 0x08).rw(FUNC(nextmo_device::r8_r), FUNC(nextmo_device::r8_w));
	map(0x09, 0x09).rw(FUNC(nextmo_device::r9_r), FUNC(nextmo_device::r9_w));
	map(0x0a, 0x0a).rw(FUNC(nextmo_device::ra_r), FUNC(nextmo_device::ra_w));
	map(0x0b, 0x0b).rw(FUNC(nextmo_device::rb_r), FUNC(nextmo_device::rb_w));
	map(0x10, 0x17).rw(FUNC(nextmo_device::r10_r), FUNC(nextmo_device::r10_w));
}

nextmo_device::nextmo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEXTMO, tag, owner, clock),
	r4(0),
	irq_cb(*this),
	drq_cb(*this)
{
}

void nextmo_device::device_start()
{
	irq_cb.resolve_safe();
	drq_cb.resolve_safe();
}

void nextmo_device::device_reset()
{
}

uint8_t nextmo_device::r4_r()
{
	logerror("nextmo: r4_r %02x %s\n", r4, machine().describe_context());
	return r4;
}

void nextmo_device::r4_w(uint8_t data)
{
	if(r4 & 1)
		device_reset();
	r4 = (r4 & (~data & 0xfc)) | (data & 3);
	logerror("nextmo: r4_w %02x %s\n", r4, machine().describe_context());
}

uint8_t nextmo_device::r5_r()
{
	logerror("nextmo: r5_r %02x %s\n", r5, machine().describe_context());
	return r5;
}

void nextmo_device::r5_w(uint8_t data)
{
	r5 = data;
	logerror("nextmo: r5_w %02x %s\n", r5, machine().describe_context());
}

uint8_t nextmo_device::r6_r()
{
	logerror("nextmo: r6_r %02x %s\n", r6, machine().describe_context());
	return r6;
}

void nextmo_device::r6_w(uint8_t data)
{
	r6 = data;
	logerror("nextmo: r6_w %02x %s\n", r6, machine().describe_context());
}

uint8_t nextmo_device::r7_r()
{
	logerror("nextmo: r7_r %02x %s\n", r7, machine().describe_context());
	return r7;
}

void nextmo_device::r7_w(uint8_t data)
{
	r7 = data;
	logerror("nextmo: r7_w %02x %s\n", r7, machine().describe_context());
	if(r7 & 0xc0) {
		logerror("nextmo: start dma %02x %02x\n", r6, r7);
		sector_pos = 0;
		if(!drq_cb.isnull())
			drq_cb(true);
	}
}

uint8_t nextmo_device::dma_r()
{
	uint8_t r = 0;
	if(r7 & 0x80)
		r = sector[sector_pos++];
	check_dma_end();
	return r;
}

void nextmo_device::dma_w(uint8_t data)
{
	if(r7 & 0x40)
		sector[sector_pos++] = data;
	check_dma_end();
}

void nextmo_device::check_dma_end()
{
	int limit;
	if(r7 & 0x40)
		limit = r6 & 0x20 ? 0x510 : 0x400;
	else
		limit = r6 & 0x20 ? 0x400 : 0x510;
	if(sector_pos == limit) {
		if(!drq_cb.isnull())
			drq_cb(false);
		if(r7 & 0x40) {
			if(r6 & 0x20)
				check_ecc();
			else
				compute_ecc();
		}
		r7 &= 0x3f;
		r4 |= 0x08;
	}
}

uint8_t nextmo_device::r8_r()
{
	logerror("nextmo: r8_r %s\n", machine().describe_context());
	return 0x00;
}

void nextmo_device::r8_w(uint8_t data)
{
	logerror("nextmo: r8_w %02x %s\n", data, machine().describe_context());
}

uint8_t nextmo_device::r9_r()
{
	logerror("nextmo: r9_r %s\n", machine().describe_context());
	return 0x00;
}

void nextmo_device::r9_w(uint8_t data)
{
	logerror("nextmo: r9_w %02x %s\n", data, machine().describe_context());
}

uint8_t nextmo_device::ra_r()
{
	logerror("nextmo: ra_r %s\n", machine().describe_context());
	return 0x00;
}

void nextmo_device::ra_w(uint8_t data)
{
	logerror("nextmo: ra_w %02x %s\n", data, machine().describe_context());
}

uint8_t nextmo_device::rb_r()
{
	logerror("nextmo: rb_r %s\n", machine().describe_context());
	return 0x24;
}

void nextmo_device::rb_w(uint8_t data)
{
	logerror("nextmo: rb_w %02x %s\n", data, machine().describe_context());
}

uint8_t nextmo_device::r10_r(offs_t offset)
{
	logerror("nextmo: r10_r %d %s\n", offset, machine().describe_context());
	return 0x00;
}

void nextmo_device::r10_w(offs_t offset, uint8_t data)
{
	logerror("nextmo: r10_w %d, %02x %s\n", offset, data, machine().describe_context());
}

void nextmo_device::check_ecc()
{
	logerror("nextmo: check_ecc\n");
	for(int i=0; i<0x400; i++)
		sector[i] = i;
}

void nextmo_device::compute_ecc()
{
	logerror("nextmo: compute_ecc\n");
	memset(sector+0x400, 0, 0x110);
}
