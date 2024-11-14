// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_MB8795_H
#define MAME_MACHINE_MB8795_H

#include "dinetwork.h"

class mb8795_device :   public device_t,
						public device_network_interface
{
public:
	mb8795_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto tx_irq() { return irq_tx_cb.bind(); }
	auto rx_irq() { return irq_rx_cb.bind(); }
	auto tx_drq() { return drq_tx_cb.bind(); }
	auto rx_drq() { return drq_rx_cb.bind(); }

	void tx_dma_w(uint8_t data, bool eof);
	void rx_dma_r(uint8_t &data, bool &eof);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void recv_cb(uint8_t *buf, int len) override;

	TIMER_CALLBACK_MEMBER(tx_update);
	TIMER_CALLBACK_MEMBER(rx_update);

private:
	enum { TIMER_TX, TIMER_RX };

	// Lifted from netbsd
	enum {
		EN_TXS_READY        = 0x80, /* ready for packet */
		EN_TXS_BUSY         = 0x40, /* receive carrier detect */
		EN_TXS_TXRECV       = 0x20, /* transmission received */
		EN_TXS_SHORTED      = 0x10, /* possible coax short */
		EN_TXS_UNDERFLOW    = 0x08, /* underflow on xmit */
		EN_TXS_COLLERR      = 0x04, /* collision detected */
		EN_TXS_COLLERR16    = 0x02, /* 16th collision error */
		EN_TXS_PARERR       = 0x01, /* parity error in tx data */

		EN_RXS_OK           = 0x80, /* packet received ok */
		EN_RXS_RESET        = 0x10, /* reset packet received */
		EN_RXS_SHORT        = 0x08, /* < minimum length */
		EN_RXS_ALIGNERR     = 0x04, /* alignment error */
		EN_RXS_CRCERR       = 0x02, /* CRC error */
		EN_RXS_OVERFLOW     = 0x01, /* receiver FIFO overflow */

		EN_TMD_COLLMASK     = 0xf0, /* collision count */
		EN_TMD_COLLSHIFT    =    4,
		EN_TMD_PARIGNORE    = 0x08, /* ignore parity */
		EN_TMD_TURBO1       = 0x04,
		EN_TMD_LB_DISABLE   = 0x02, /* loop back disabled */
		EN_TMD_DISCONTENT   = 0x01, /* disable contention (rx carrier) */

		EN_RMD_TEST         = 0x80, /* must be zero */
		EN_RMD_ADDRSIZE     = 0x10, /* reduces NODE match to 5 chars */
		EN_RMD_SHORTENABLE  = 0x08, /* "rx packets >= 10 bytes" - <? */
		EN_RMD_RESETENABLE  = 0x04, /* detect "reset" ethernet frames */
		EN_RMD_WHATRECV     = 0x03, /* controls what packets are received */
		EN_RMD_RECV_PROMISC = 0x03, /* all packets */
		EN_RMD_RECV_MULTI   = 0x02, /* accept broad/multicasts */
		EN_RMD_RECV_NORMAL  = 0x01, /* accept broad/limited multicasts */
		EN_RMD_RECV_NONE    = 0x00, /* accept no packets */

		EN_RST_RESET        = 0x80 /* reset interface */
	};

	uint8_t mac[6];
	uint8_t txbuf[2000], rxbuf[2000];
	uint8_t txstat, txmask, rxstat, rxmask, txmode, rxmode;
	uint16_t txlen, rxlen, txcount;
	bool drq_tx, drq_rx, irq_tx, irq_rx;
	emu_timer *timer_tx, *timer_rx;

	devcb_write_line irq_tx_cb, irq_rx_cb, drq_tx_cb, drq_rx_cb;

	void check_irq();
	void start_send();
	void receive();
	bool recv_is_broadcast();
	bool recv_is_me();
	bool recv_is_multicast();
	bool recv_is_local_multicast();

	uint8_t txstat_r();
	void txstat_w(uint8_t data);
	uint8_t txmask_r();
	void txmask_w(uint8_t data);
	uint8_t rxstat_r();
	void rxstat_w(uint8_t data);
	uint8_t rxmask_r();
	void rxmask_w(uint8_t data);
	uint8_t txmode_r();
	void txmode_w(uint8_t data);
	uint8_t rxmode_r();
	void rxmode_w(uint8_t data);
	void reset_w(uint8_t data);
	uint8_t tdc_lsb_r();
	uint8_t mac_r(offs_t offset);
	void mac_w(offs_t offset, uint8_t data);
};

DECLARE_DEVICE_TYPE(MB8795, mb8795_device)

#endif // MAME_MACHINE_MB8795_H
