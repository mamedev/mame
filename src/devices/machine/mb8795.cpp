// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "mb8795.h"

const device_type MB8795 = &device_creator<mb8795_device>;

DEVICE_ADDRESS_MAP_START(map, 8, mb8795_device)
	AM_RANGE(0x0, 0x0) AM_READWRITE(txstat_r, txstat_w)
	AM_RANGE(0x1, 0x1) AM_READWRITE(txmask_r, txmask_w)
	AM_RANGE(0x2, 0x2) AM_READWRITE(rxstat_r, rxstat_w)
	AM_RANGE(0x3, 0x3) AM_READWRITE(rxmask_r, rxmask_w)
	AM_RANGE(0x4, 0x4) AM_READWRITE(txmode_r, txmode_w)
	AM_RANGE(0x5, 0x5) AM_READWRITE(rxmode_r, rxmode_w)
	AM_RANGE(0x6, 0x6) AM_WRITE(reset_w)
	AM_RANGE(0x7, 0x7) AM_READ(tdc_lsb_r)
	AM_RANGE(0x8, 0xf) AM_READWRITE(mac_r, mac_w) // Mapping limitation, real is up to 0xd
ADDRESS_MAP_END

mb8795_device::mb8795_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MB8795, "Fujitsu MB8795", tag, owner, clock, "mb8795", __FILE__),
	device_network_interface(mconfig, *this, 10), txstat(0), txmask(0), rxstat(0), rxmask(0), txmode(0), rxmode(0), txlen(0), rxlen(0), txcount(0), drq_tx(false), 
	drq_rx(false), irq_tx(false), irq_rx(false), timer_tx(nullptr), timer_rx(nullptr),
	irq_tx_cb(*this),
	irq_rx_cb(*this),
	drq_tx_cb(*this),
	drq_rx_cb(*this)
{
}

void mb8795_device::check_irq()
{
	bool old_irq_tx = irq_tx;
	bool old_irq_rx = irq_rx;
	irq_tx = txstat & txmask;
	irq_rx = rxstat & rxmask;
	if(irq_tx != old_irq_tx && !irq_tx_cb.isnull())
		irq_tx_cb(irq_tx);
	if(irq_rx != old_irq_rx && !irq_rx_cb.isnull())
		irq_rx_cb(irq_rx);
}

void mb8795_device::device_start()
{
	irq_tx_cb.resolve();
	irq_rx_cb.resolve();
	drq_tx_cb.resolve();
	drq_rx_cb.resolve();

	memset(mac, 0, 6);
	timer_tx = timer_alloc(TIMER_TX);
	timer_rx = timer_alloc(TIMER_RX);
}

void mb8795_device::device_reset()
{
	txstat = EN_TXS_READY;
	txmask = 0x00;
	rxstat = 0x00;
	rxmask = 0x00;
	txmode = 0x00;
	rxmode = 0x00;

	drq_tx = drq_rx = false;
	irq_tx = irq_rx = false;

	txlen = rxlen = txcount = 0;

	set_promisc(true);

	start_send();
}

void mb8795_device::recv_cb(UINT8 *buf, int len)
{
	memcpy(rxbuf, buf, len);
	rxlen = len;
	receive();
}

READ8_MEMBER(mb8795_device::txstat_r)
{
	//  logerror("%s: txstat_r %02x (%08x)\n", tag(), txstat, space.device().safe_pc());
	return txstat;
}

WRITE8_MEMBER(mb8795_device::txstat_w)
{
	txstat = txstat & (0xf0 | ~data);
	check_irq();
	logerror("%s: txstat_w %02x (%08x)\n", tag(), txstat, space.device().safe_pc());
}

READ8_MEMBER(mb8795_device::txmask_r)
{
	logerror("%s: txmask_r %02x (%08x)\n", tag(), txmask, space.device().safe_pc());
	return txmask;
}

WRITE8_MEMBER(mb8795_device::txmask_w)
{
	txmask = data & 0xaf;
	check_irq();
	logerror("%s: txmask_w %02x (%08x)\n", tag(), txmask, space.device().safe_pc());
}

READ8_MEMBER(mb8795_device::rxstat_r)
{
	logerror("%s: rxstat_r %02x (%08x)\n", tag(), rxstat, space.device().safe_pc());
	return rxstat;
}

WRITE8_MEMBER(mb8795_device::rxstat_w)
{
	rxstat = rxstat & (0x70 | ~data);
	check_irq();
	logerror("%s: rxstat_w %02x (%08x)\n", tag(), rxstat, space.device().safe_pc());
}

READ8_MEMBER(mb8795_device::rxmask_r)
{
	logerror("%s: rxmask_r %02x (%08x)\n", tag(), rxmask, space.device().safe_pc());
	return rxmask;
}

WRITE8_MEMBER(mb8795_device::rxmask_w)
{
	rxmask = data & 0x9f;
	check_irq();
	logerror("%s: rxmask_w %02x (%08x)\n", tag(), rxmask, space.device().safe_pc());
}

READ8_MEMBER(mb8795_device::txmode_r)
{
	logerror("%s: txmode_r %02x (%08x)\n", tag(), txmode, space.device().safe_pc());
	return txmode;
}

WRITE8_MEMBER(mb8795_device::txmode_w)
{
	txmode = data;
	logerror("%s: txmode_w %02x (%08x)\n", tag(), txmode, space.device().safe_pc());
}

READ8_MEMBER(mb8795_device::rxmode_r)
{
	logerror("%s: rxmode_r %02x (%08x)\n", tag(), rxmode, space.device().safe_pc());
	return rxmode;
}

WRITE8_MEMBER(mb8795_device::rxmode_w)
{
	rxmode = data;
	logerror("%s: rxmode_w %02x (%08x)\n", tag(), rxmode, space.device().safe_pc());
}

WRITE8_MEMBER(mb8795_device::reset_w)
{
	if(data & EN_RST_RESET)
		device_reset();
}

READ8_MEMBER(mb8795_device::tdc_lsb_r)
{
	logerror("%s: tdc_lsb_r %02x (%08x)\n", tag(), txcount & 0xff, space.device().safe_pc());
	return txcount;
}

READ8_MEMBER(mb8795_device::mac_r)
{
	if(offset < 6)
		return mac[offset];
	if(offset == 7) {
		logerror("%s: tdc_msb_r %02x (%08x)\n", tag(), txcount >> 8, space.device().safe_pc());
		return (txcount >> 8) & 0x3f;
	}
	return 0;
}

WRITE8_MEMBER(mb8795_device::mac_w)
{
	if(offset < 6) {
		mac[offset] = data;
		set_mac((const char *)mac);
	}
}

void mb8795_device::start_send()
{
	timer_tx->adjust(attotime::zero);
}

void mb8795_device::tx_dma_w(UINT8 data, bool eof)
{
	txbuf[txlen++] = data;
	if(txstat & EN_TXS_READY) {
		txstat &= ~EN_TXS_READY;
		check_irq();
	}

	drq_tx = false;
	if(!drq_tx_cb.isnull())
		drq_tx_cb(drq_tx);

	if(eof) {
		logerror("%s: send packet, dest=%02x.%02x.%02x.%02x.%02x.%02x len=%04x loopback=%s\n", tag(),
					txbuf[0], txbuf[1], txbuf[2], txbuf[3], txbuf[4], txbuf[5],
					txlen,
					txmode & EN_TMD_LB_DISABLE ? "off" : "on");

		if(txlen > 1500)
			txlen = 1500; // Weird packet send on loopback test in the next

		if(!(txmode & EN_TMD_LB_DISABLE)) {
			memcpy(rxbuf, txbuf, txlen);
			rxlen = txlen;
			receive();
		}
		send(txbuf, txlen);
		txlen = 0;
		txstat |= EN_TXS_READY;
		txcount++;
		start_send();
	} else
		timer_tx->adjust(attotime::from_nsec(800));
}

void mb8795_device::rx_dma_r(UINT8 &data, bool &eof)
{
	drq_rx = false;
	if(!drq_rx_cb.isnull())
		drq_rx_cb(drq_rx);

	if(rxlen) {
		data = rxbuf[0];
		rxlen--;
		memmove(rxbuf, rxbuf+1, rxlen);
	} else
		data = 0;

	if(rxlen) {
		timer_rx->adjust(attotime::from_nsec(800));
		eof = false;
	} else
		eof = true;
}

void mb8795_device::receive()
{
	bool keep = false;
	switch(rxmode & EN_RMD_WHATRECV) {
	case EN_RMD_RECV_NONE:
		keep = false;
		break;
	case EN_RMD_RECV_NORMAL:
		keep = recv_is_broadcast() || recv_is_me() || recv_is_local_multicast();
		break;
	case EN_RMD_RECV_MULTI:
		keep = recv_is_broadcast() || recv_is_me() || recv_is_multicast();
		break;
	case EN_RMD_RECV_PROMISC:
		keep = true;
		break;
	}
	logerror("%s: received packet for %02x.%02x.%02x.%02x.%02x.%02x len=%04x, mode=%d -> %s\n", tag(),
			rxbuf[0], rxbuf[1], rxbuf[2], rxbuf[3], rxbuf[4], rxbuf[5],
			rxlen, rxmode & 3, keep ? "kept" : "dropped");
	if(!keep)
		rxlen = 0;
	else {
		// Minimal ethernet packet size
		if(rxlen < 64) {
			memset(rxbuf+rxlen, 0, 64-rxlen);
			rxlen = 64;
		}
		// Checksum?  In any case, it's there
		memset(rxbuf+rxlen, 0, 4);
		rxlen += 4;

		rxstat |= EN_RXS_OK;
		check_irq();
		timer_rx->adjust(attotime::zero);
	}
}

bool mb8795_device::recv_is_broadcast()
{
	return
		rxbuf[0] == 0xff &&
		rxbuf[1] == 0xff &&
		rxbuf[2] == 0xff &&
		rxbuf[3] == 0xff &&
		rxbuf[4] == 0xff &&
		rxbuf[5] == 0xff;
}

bool mb8795_device::recv_is_me()
{
	return
		rxbuf[0] == mac[0] &&
		rxbuf[1] == mac[1] &&
		rxbuf[2] == mac[2] &&
		rxbuf[3] == mac[3] &&
		rxbuf[4] == mac[4] &&
		((rxmode & EN_RMD_ADDRSIZE) || rxbuf[5] == mac[5]);
}

bool mb8795_device::recv_is_local_multicast()
{
	return
		(rxbuf[0] & 0x01) &&
		(rxbuf[0] & 0xfe) == mac[0] &&
		rxbuf[1] == mac[1] &&
		rxbuf[2] == mac[2];
}

bool mb8795_device::recv_is_multicast()
{
	return rxbuf[0] & 0x01;
}

void mb8795_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(id == TIMER_TX) {
		drq_tx = true;
		if(!drq_tx_cb.isnull())
			drq_tx_cb(drq_tx);
	}

	if(id == TIMER_RX && rxlen) {
		drq_rx = true;
		if(!drq_rx_cb.isnull())
			drq_rx_cb(drq_rx);
	}
}
