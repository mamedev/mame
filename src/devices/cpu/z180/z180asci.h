// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    z180asci.h

*********************************************************************/

#ifndef MAME_CPU_Z180_Z180ASCI_H
#define MAME_CPU_Z180_Z180ASCI_H

#pragma once

#include "machine/clock.h"

//**************************************************************************
//  z180asci_channel_base
//**************************************************************************

class z180asci_channel_base : public device_t
{
public:
	// inline configuration
	auto txa_handler() { return m_txa_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }
	auto cka_handler() { return m_cka_handler.bind(); }

	uint8_t cntla_r();
	uint8_t cntlb_r();
	uint8_t stat_r();
	uint8_t tdr_r();
	uint8_t rdr_r();
	uint8_t asext_r();
	uint8_t astcl_r();
	uint8_t astch_r();
	void cntla_w(uint8_t data);
	void cntlb_w(uint8_t data);
	virtual void stat_w(uint8_t data) = 0;
	void tdr_w(uint8_t data);
	void rdr_w(uint8_t data);
	virtual void asext_w(uint8_t data) = 0;
	void astcl_w(uint8_t data);
	void astch_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( rxa_wr );
	DECLARE_WRITE_LINE_MEMBER( cts_wr ) { m_cts = state; }
	DECLARE_WRITE_LINE_MEMBER( dcd_wr ) { m_dcd = state; }
	DECLARE_WRITE_LINE_MEMBER( cka_wr );

	virtual void state_add(device_state_interface &parent) = 0;

	int check_interrupt() { return m_irq; }
	void clear_interrupt() { m_irq = 0; }
protected:
	z180asci_channel_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const int id, const bool ext);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_resolve_objects() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_clock_changed() override;

	enum serial_state
	{
		STATE_START,
		STATE_DATA,
		STATE_STOP
	};


	void output_txa(int txa);
	void output_rts(int rts);

	required_device<clock_device> m_brg;

	devcb_write_line m_txa_handler;
	devcb_write_line m_rts_handler;
	devcb_write_line m_cka_handler;

	uint8_t   m_asci_cntla;                  // ASCI control register A
	uint8_t   m_asci_cntlb;                  // ASCI control register B
	uint8_t   m_asci_stat;                   // ASCI status register
	uint8_t   m_asci_tdr;                    // ASCI transmit data register
	uint8_t   m_asci_rdr;                    // ASCI receive data register
	uint8_t   m_asci_ext;                    // ASCI extension control register
	PAIR16    m_asci_tc;                     // ASCI time constant

	uint8_t   m_tsr;
	uint8_t   m_rsr;
	uint8_t   m_data_fifo[4];
	uint8_t   m_status_fifo[4];
	int		  m_fifo_head;
	int		  m_fifo_tail;

	int		  m_cts;
	int		  m_dcd;
	int       m_irq;
	int		  m_txa;
	int 	  m_rxa;
	int		  m_rts;

	uint32_t  m_divisor;

	int m_clock_state;
	int m_tx_state;
	int m_tx_counter;
	int m_rx_state;
	int m_rx_counter;

	const int  m_id;
	const bool m_ext;
};

//**************************************************************************
//  z180asci_channel_0
//**************************************************************************

class z180asci_channel_0 : public z180asci_channel_base
{
public:
	// construction/destruction
	z180asci_channel_0(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void stat_w(uint8_t data) override;
	void asext_w(uint8_t data) override;
	void state_add(device_state_interface &parent) override;
protected:
	z180asci_channel_0(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool ext);
	// device-level overrides
	virtual void device_reset() override;
};

//**************************************************************************
//  z180asci_channel_1
//**************************************************************************

class z180asci_channel_1 : public z180asci_channel_base
{
public:
	// construction/destruction
	z180asci_channel_1(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void stat_w(uint8_t data) override;
	void asext_w(uint8_t data) override;
	void state_add(device_state_interface &parent) override;
protected:
	z180asci_channel_1(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool ext);
	// device-level overrides
	virtual void device_reset() override;
};

//**************************************************************************
//  z180asci_ext_channel_0
//**************************************************************************

class z180asci_ext_channel_0 : public z180asci_channel_0
{
public:
	// construction/destruction
	z180asci_ext_channel_0(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

//**************************************************************************
//  z180asci_ext_channel_1
//**************************************************************************

class z180asci_ext_channel_1 : public z180asci_channel_1
{
public:
	// construction/destruction
	z180asci_ext_channel_1(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(Z180ASCI_CHANNEL_0, z180asci_channel_0)
DECLARE_DEVICE_TYPE(Z180ASCI_CHANNEL_1, z180asci_channel_1)

DECLARE_DEVICE_TYPE(Z180ASCI_EXT_CHANNEL_0, z180asci_ext_channel_0)
DECLARE_DEVICE_TYPE(Z180ASCI_EXT_CHANNEL_1, z180asci_ext_channel_1)

#endif // MAME_CPU_Z180_Z180ASCI_H
