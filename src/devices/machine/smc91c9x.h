// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles, Ted Green

**************************************************************************/

#ifndef MAME_MACHINE_SMC91C9X_H
#define MAME_MACHINE_SMC91C9X_H

#pragma once

#include "dinetwork.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class smc91c9x_device : public device_t,public device_network_interface
{
public:
	auto irq_handler() { return m_irq_handler.bind(); }

	u16 read(offs_t offset, u16 mem_mask = ~0);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

	void set_link_connected(bool connected) { m_link_unconnected = !connected; }

protected:
	enum class dev_type {
		SMC91C94,
		SMC91C96
	};

	smc91c9x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, dev_type device_type);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_network_interface overrides
	virtual void send_complete_cb(int result) override;
	virtual int recv_start_cb(u8 *buf, int length) override;
	virtual void recv_complete_cb(int result) override;

	void dump_bytes(u8 *buf, int length);
	int address_filter(u8 *buf);
	int receive(u8 *buf, int length);

	TIMER_CALLBACK_MEMBER(tx_poll);

	const dev_type m_device_type;
	unsigned m_num_ebuf;

private:
	// Ethernet registers - bank 0
	enum bank0_addr : u8 {
		B0_TCR        = (0 * 8 + 0),
		B0_EPH_STATUS = (0 * 8 + 1),
		B0_RCR        = (0 * 8 + 2),
		B0_COUNTER    = (0 * 8 + 3),
		B0_MIR        = (0 * 8 + 4),
		B0_MCR        = (0 * 8 + 5),
		B0_BANK       = (0 * 8 + 7)
	};

	// Ethernet registers - bank 1
	enum bank1_addr : u8 {
		B1_CONFIG       = (1 * 8 + 0),
		B1_BASE         = (1 * 8 + 1),
		B1_IA0_1        = (1 * 8 + 2),
		B1_IA2_3        = (1 * 8 + 3),
		B1_IA4_5        = (1 * 8 + 4),
		B1_GENERAL_PURP = (1 * 8 + 5),
		B1_CONTROL      = (1 * 8 + 6)
	};

	// Ethernet registers - bank 2
	enum bank2_addr : u8 {
		B2_MMU_COMMAND  = (2 * 8 + 0),
		B2_PNR_ARR      = (2 * 8 + 1),
		B2_FIFO_PORTS   = (2 * 8 + 2),
		B2_POINTER      = (2 * 8 + 3),
		B2_DATA_0       = (2 * 8 + 4),
		B2_DATA_1       = (2 * 8 + 5),
		B2_INTERRUPT    = (2 * 8 + 6)
	};

	// Ethernet registers - bank 3
	enum bank3_addr : u8 {
		B3_MT0_1    = (3 * 8 + 0),
		B3_MT2_3    = (3 * 8 + 1),
		B3_MT4_5    = (3 * 8 + 2),
		B3_MT6_7    = (3 * 8 + 3),
		B3_MGMT     = (3 * 8 + 4),
		B3_REVISION = (3 * 8 + 5),
		B3_ERCV     = (3 * 8 + 6)
	};

	// Ethernet MMU commands
	enum mmu_cmd : u8 {
		ECMD_NOP                        = 0,
		ECMD_ALLOCATE                   = 2,
		ECMD_RESET_MMU                  = 4,
		ECMD_REMOVE_TOPFRAME_RX         = 6,
		ECMD_REMOVE_TOPFRAME_TX         = 7,
		ECMD_REMOVE_RELEASE_TOPFRAME_RX = 8,
		ECMD_RELEASE_PACKET             = 10,
		ECMD_ENQUEUE_PACKET             = 12,
		ECMD_RESET_FIFOS                = 14
	};

	// Ethernet interrupt bits
	enum eint_def : u8 {
		EINT_RCV            = 0x01,
		EINT_TX             = 0x02,
		EINT_TX_EMPTY       = 0x04,
		EINT_ALLOC          = 0x08,
		EINT_RX_OVRN        = 0x10,
		EINT_EPH            = 0x20,
		EINT_ERCV           = 0x40,    // 91c92 only
		EINT_TX_IDLE        = 0x80     // 91c94 only
	};

	// Address filter return codes
	enum addr_filter_def : int {
		ADDR_NOMATCH   = 0,
		ADDR_UNICAST   = 1,
		ADDR_BROADCAST = 2,
		ADDR_MULTICAST = 3
	};

	// Rx/Tx control bits
	enum control_mask : u8 {
		EBUF_RX_ALWAYS = 0x40,   // Always set on receive buffer control byte
		EBUF_ODD       = 0x20,   // Odd number of data payload bytes
		EBUF_CRC       = 0x10       // Tx add CRC
	};

	// Receive buffer status
	enum rx_status_mask : u16 {
		ALGNERR       = 0x8000,
		BRODCAST      = 0x4000,
		BADCRC        = 0x2000,
		ODDFRM        = 0x1000,
		TOOLNG        = 0x0800, // Received fram is longer than 1518 bytes on cable
		TOOSHORT      = 0x0400, // Received fram is shorter than 64 bytes on cable
		HASHVALUE     = 0x007e,
		MULTCAST      = 0x0001
	};

	// EPH Status bits
	enum eph_mask : u16 {
		LINK_OK       = 0x4000, // State of link integrity test
		CTR_ROL       = 0x1000, // Counter roll Over
		EXC_DEF       = 0x0800, // Excessive deferral
		LOST_CARR     = 0x0400, // Lost carrier sense
		LATCOL        = 0x0200, // Late collisions detected
		WAKEUP        = 0x0100, // Magic packet received
		TX_DEFER      = 0x0080, // Transmit deferred
		LTX_BRD       = 0x0040, // Last transmit frame was a broadcast
		SQET          = 0x0020, // Signal Quality Error Test
		E16COL        = 0x0010, // 16 collisions reached
		LTX_MULT      = 0x0008, // Last transmit frame was a multicast
		MULCOL        = 0x0004, // Multiple collisions detected
		SNGLCOL       = 0x0002, // Single collision detected
		TX_SUC        = 0x0001  // Last transmit frame was successful
	};

	// CTR register bits
	enum ctr_mask : u16 {
		RCV_BAD       = 0x4000, // Receive bad CRC packets
		PWRDN         = 0x2000, // Power down ethernet
		WAKEUP_EN     = 0x1000, // Enable magic packet wakeup
		AUTO_RELEASE  = 0x0800, // Release transmit packets on good transmission
		LE_ENABLE     = 0x0080, // Link Error enable
		CR_ENABLE     = 0x0040, // Counter Roll over enable
		TE_ENABLE     = 0x0020, // Transmit Error enable
		EEPROM_SEL    = 0x0004, // EEPROM address
		RELOAD        = 0x0002, // Reload config from EEPROM
		STORE         = 0x0001  // Store config to EEPROM
	};

	// Transmit Control Register bits
	enum tcr_mask : u16 {
		FDSE        = 0x8000,
		EPH_LOOP    = 0x2000,
		STP_SQET    = 0x1000,
		FDUPLX      = 0x0800,
		MON_CSN     = 0x0400,
		NOCRC       = 0x0100,
		PAD_EN      = 0x0080,
		FORCOL      = 0x0004,
		LOOP        = 0x0002,
		TXENA       = 0x0001
	};

	// Receive Control Register bits
	enum rcr_mask : u16 {
		SOFT_RST    = 0x8000,
		FILT_CAR    = 0x4000,
		STRIP_CRC   = 0x0200,
		RXEN        = 0x0100,
		ALMUL       = 0x0004,
		PRMS        = 0x0002,
		RX_ABORT    = 0x0001
	};

	// Pointer Register bits
	enum pointer_mask : u16 {
		RCV         = 0x8000,
		AUTO_INCR   = 0x4000,
		READ        = 0x2000,
		PTR         = 0x07ff
	};

	static constexpr u32 FCS_RESIDUE = 0xdebb20e3;
	static constexpr unsigned ETHER_BUFFER_SIZE = 256 * 6;
	static const u8 ETH_BROADCAST[];

	// mmu

	// The bits in these vectors indicate a packet has been allocated
	u32 m_alloc_rx, m_alloc_tx;

	// Requests a packet allocation and returns true
	// and sets the packet number if successful
	bool alloc_req(const int tx, int &packet_num);
	// Releases an allocation
	void alloc_release(const int packet_num);
	// Resets the MMU
	void mmu_reset();

	// internal state
	devcb_write_line m_irq_handler;

	// link unconnected
	bool m_link_unconnected;

	/* raw register data and masks */
	uint16_t          m_reg[64];
	uint16_t          m_regmask[64];

	/* IRQ information */
	uint8_t           m_irq_state;

	// Main memory
	std::unique_ptr<u8[]> m_buffer;

	/* counters */
	uint32_t          m_sent;
	uint32_t          m_recd;

	emu_timer* m_tx_poll;

	int m_tx_active;
	int m_rx_active;
	int m_tx_retry_count;
	u8 m_rx_hash;
	int m_loopback_result;

	void update_ethernet_irq();
	void update_stats();

	void process_command(uint16_t data);
	void reset_tx_fifos();

	// TODO: Make circular fifo a separate device
	// Simple circular FIFO, power of 2 size, no over/under run checking
	static constexpr unsigned FIFO_SIZE = 1 << 5;

	// FIFO for allocated (queued) transmit packets
	u8 m_queued_tx[FIFO_SIZE];
	int m_queued_tx_h, m_queued_tx_t;
	void reset_queued_tx() { m_queued_tx_t = m_queued_tx_h = 0; }
	void push_queued_tx(const u8 &data) { m_queued_tx[m_queued_tx_h++] = data; m_queued_tx_h &= FIFO_SIZE - 1; }
	u8 pop_queued_tx() { u8 val = m_queued_tx[m_queued_tx_t++]; m_queued_tx_t &= FIFO_SIZE - 1; return val; }
	bool empty_queued_tx() const { return m_queued_tx_h == m_queued_tx_t; }
	u8 curr_queued_tx() const { return m_queued_tx[m_queued_tx_t]; }

	// FIFO for completed transmit packets
	u8 m_completed_tx[FIFO_SIZE];
	int m_completed_tx_h, m_completed_tx_t;
	void reset_completed_tx() { m_completed_tx_t = m_completed_tx_h = 0; }
	void push_completed_tx(const u8 &data) { m_completed_tx[m_completed_tx_h++] = data; m_completed_tx_h &= FIFO_SIZE - 1; }
	u8 pop_completed_tx() { u8 val = m_completed_tx[m_completed_tx_t++]; m_completed_tx_t &= FIFO_SIZE - 1; return val; }
	bool empty_completed_tx() const { return m_completed_tx_h == m_completed_tx_t; }
	u8 curr_completed_tx() const { return m_completed_tx[m_completed_tx_t]; }

	// FIFO for completed receive packets
	u8 m_completed_rx[FIFO_SIZE];
	int m_completed_rx_h, m_completed_rx_t;
	void reset_completed_rx() { m_completed_rx_t = m_completed_rx_h = 0; }
	void push_completed_rx(const u8 &data) { m_completed_rx[m_completed_rx_h++] = data; m_completed_rx_h &= FIFO_SIZE - 1; }
	u8 pop_completed_rx() { u8 val = m_completed_rx[m_completed_rx_t++]; m_completed_rx_t &= FIFO_SIZE - 1; return val; }
	bool empty_completed_rx() const { return m_completed_rx_h == m_completed_rx_t; }
	u8 curr_completed_rx() const { return m_completed_rx[m_completed_rx_t]; }

};


class smc91c94_device : public smc91c9x_device
{
public:
	smc91c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class smc91c96_device : public smc91c9x_device
{
public:
	smc91c96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(SMC91C94, smc91c94_device)
DECLARE_DEVICE_TYPE(SMC91C96, smc91c96_device)


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#endif // MAME_MACHINE_SMC91C9X_H
