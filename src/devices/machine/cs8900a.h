// license:GPL-2.0+
// copyright-holders:Spiro Trikaliotis, Rhett Aultman

/*************************************************************************

    CS8900A ethernet controller implementation

    by Rhett Aultman <roadriverrail@gmail.com>
    ported to MAME from VICE Project (https://sourceforge.net/p/vice-emu/)
    VICE CS8900 code by Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>

**************************************************************************/

#ifndef MAME_MACHINE_CS8900A_H
#define MAME_MACHINE_CS8900A_H

#pragma once

#include "dinetwork.h"

#include <queue>

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class cs8900a_device : public device_t, public device_network_interface {
public:

	cs8900a_device(machine_config const& mconfig, char const *tag, device_t *owner, u32 clock);
	u8 read(u16 address);
	void write(u16 address, u8 data);

protected:

	cs8900a_device(const machine_config& mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_network_interface overrides
	virtual int recv_start_cb(u8 *buf, int length) override;

private:

	static constexpr u8 CS8900_COUNT_IO_REGISTER = 0x10; /* we have 16 I/O register */
	static constexpr u16 MAX_PACKETPAGE_ARRAY = 0x1000; /* 4 KB */

	u8 cs8900_ia_mac[6];

	u32 cs8900_hash_mask[2];

	// CS8900A IO Registers
	u8 cs8900_ioregs[CS8900_COUNT_IO_REGISTER];

	// CS8900A PacketPage
	u8 cs8900_packetpage[MAX_PACKETPAGE_ARRAY];
	u16 cs8900_packetpage_ptr;

	/* reveiver setup */
	u16 cs8900_recv_control;        /* copy of CC_RXCTL (contains all bits below) */
	bool cs8900_recv_broadcast;     /* broadcast */
	bool cs8900_recv_mac;           /* individual address (IA) */
	bool cs8900_recv_multicast;     /* multicast if address passes the hash filter */
	bool cs8900_recv_correct;       /* accept correct frames */
	bool cs8900_recv_promiscuous;   /* promiscuous mode */
	bool cs8900_recv_hashfilter;    /* accept if IA passes the hash filter */

	u16 tx_buffer;
	u16 rx_buffer;

	u16 tx_count;
	u16 rx_count;
	u16 tx_length;
	u16 rx_length;

	/* tranceiver state */
	int tx_state;
	int rx_state;
	int tx_enabled;
	int rx_enabled;

	int rxevent_read_mask; /* set if L and/or H u8 was read in RXEVENT? */

	// The frame queue
	std::queue<std::vector<u8> >m_frame_queue;

	std::string debug_outbuffer(const int length, const unsigned char *const buffer);
	void cs8900_set_tx_status(int ready, int error);
	void cs8900_set_receiver(int enabled);
	void cs8900_set_transmitter(int enabled);
	bool cs8900_should_accept(unsigned char *buffer, int length, bool *phashed, int *phash_index, bool *pcorrect_mac, bool *pbroadcast, bool *pmulticast);
	u16 cs8900_receive(void);
	void cs8900_write_tx_buffer(u8  value, int odd_address);
	u8 cs8900_read_rx_buffer(int odd_address);
	void cs8900_sideeffects_write_pp(u16 ppaddress, int odd_address);
	void cs8900_sideeffects_read_pp(u16 ppaddress, int odd_address);
	u16 cs8900_read_register(u16 ppaddress);
	void cs8900_write_register(u16 ppaddress, u16 value);
	void cs8900_auto_incr_pp_ptr();
	u8 cs8900_read(u16 io_address);
	void cs8900_store(u16 io_address, u8 var);
};

DECLARE_DEVICE_TYPE(CS8900A, cs8900a_device)

#endif // MAME_MACHINE_CS8900A_H
