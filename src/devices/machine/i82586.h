// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_I82586_H
#define MAME_MACHINE_I82586_H

#pragma once

#include "dinetwork.h"

class i82586_base_device :
	public device_t,
	public device_memory_interface,
	public device_network_interface
{
public:
	enum scb_mask
	{
		T       = 0x00000008, // throttle timers loaded
		RUS     = 0x000000f0, // ru status
		CUS     = 0x00000700, // cu status
		RNR     = 0x00001000, // ru left ready state
		CNA     = 0x00002000, // cu left active state
		FR      = 0x00004000, // ru finished receiving a frame
		CX      = 0x00008000, // cu finished executing a command with its interrupt bit set
		RUC     = 0x00700000, // ru command
		RESET   = 0x00800000, // reset chip (same as hardware reset)
		CUC     = 0x07000000, // cu command
		ACK_RNR = 0x10000000, // acknowledge ru became became not ready
		ACK_CNA = 0x20000000, // acknowledge cu became not active
		ACK_FR  = 0x40000000, // acknowledge ru received a frame
		ACK_CX  = 0x80000000  // acknowledge cu completed an action command
	};

	enum scb_cu_command
	{
		CUC_NOP        = 0x00000000, // no operation
		CUC_START      = 0x01000000, // start execution of first command on cbl
		CUC_RESUME     = 0x02000000, // resume execution of cu
		CUC_SUSPEND    = 0x03000000, // suspend execution after current command
		CUC_ABORT      = 0x04000000, // abort current command immediately
		CUC_THROTTLE_D = 0x05000000, // load throttle timers after next terminal count
		CUC_THROTTLE_I = 0x06000000  // load and restart throttle timers immediately
	};
	enum cu_state : u8
	{
		CU_IDLE      = 0x0, // not executing, not associated with command, initial state
		CU_SUSPENDED = 0x1, // not executing, associated with command
		CU_ACTIVE    = 0x2  // executing, associated with command
	};

	enum scb_ru_command
	{
		RUC_NOP     = 0x00000000, // no operation
		RUC_START   = 0x00100000, // start reception of frames
		RUC_RESUME  = 0x00200000, // resume frame reception
		RUC_SUSPEND = 0x00300000, // suspend frame reception
		RUC_ABORT   = 0x00400000  // abort receiver operation immediately
	};
	enum ru_state : u8
	{
		RU_IDLE      = 0x0, // no resources, discarding frames, initial state
		RU_SUSPENDED = 0x1, // has resources, discarding frames
		RU_NR        = 0x2, // no resources, discarding frames, accumulating statistics
		RU_READY     = 0x4, // has resources, storing frames
		RU_NR_RFD    = 0xa, // no receive frame descriptor resources (not 82586 mode)
		RU_NR_RBD    = 0xc  // no receive block descriptor resources (not 82586 mode)
	};

	enum cu_cb_cs_mask
	{
		CB_MAXCOL = 0x0000000f, // number of collisions
		CB_S5     = 0x00000020, // transmission unsuccessful due to maximum collision retries
		CB_S6     = 0x00000040, // heart beat indicator
		CB_S7     = 0x00000080, // transmission deferred due to traffic on link
		CB_S8     = 0x00000100, // transmission unsuccessful due to dma underrun
		CB_S9     = 0x00000200, // transmission unsuccessful due to loss of cts
		CB_S10    = 0x00000400, // no carrier sense during transmission
		CB_A      = 0x00001000, // command aborted
		CB_OK     = 0x00002000, // error free completion
		CB_B      = 0x00004000, // busy executing command
		CB_C      = 0x00008000, // command completed
		CB_CMD    = 0x00070000, // command operation code
		CB_NC     = 0x00100000, // no crc insertion (82596 only)
		CB_SF     = 0x00080000, // simplified mode (82596 only)
		CB_I      = 0x20000000, // interrupt after completion
		CB_S      = 0x40000000, // suspend after completion
		CB_EL     = 0x80000000  // end of command list
	};
	enum cu_cb_command
	{
		CB_NOP       = 0x00000000, // no operation
		CB_IASETUP   = 0x00010000, // individual address setup
		CB_CONFIGURE = 0x00020000, // configure
		CB_MCSETUP   = 0x00030000, // multicast setup
		CB_TRANSMIT  = 0x00040000, // transmit
		CB_TDREFLECT = 0x00050000, // time domain reflectometer
		CB_DUMP      = 0x00060000, // dump
		CB_DIAGNOSE  = 0x00070000  // diagnose
	};
	enum cu_transmit_count_mask
	{
		TB_COUNT = 0x3fff,
		TB_EOF = 0x8000
	};
	enum cu_tdreflect_mask
	{
		TDR_TIME     = 0x07ff, // time until echo
		TDR_ET_SRT   = 0x1000, // short circuit
		TDR_ET_OPN   = 0x2000, // no termination
		TDR_XCVR_PRB = 0x4000, // transceiver problem
		TDR_LNK_OK   = 0x8000  // no link problem
	};

	enum ru_rfd_cs_mask
	{
		RFD_S_COLLISION = 0x00000001, // receive collision
		RFD_S_MULTICAST = 0x00000002, // multicast address
		RFD_S_TRUNCATED = 0x00000020, // frame truncated

		RFD_S_EOP       = 0x00000040, // no eop
		RFD_S_SHORT     = 0x00000080, // frame too short
		RFD_S_OVERRUN   = 0x00000100, // dma overrun
		RFD_S_BUFFER    = 0x00000200, // no buffer
		RFD_S_ALIGN     = 0x00000400, // alignment error
		RFD_S_CRC       = 0x00000800, // crc error
		RFD_S_LENGTH    = 0x00001000, // length error

		RFD_ERROR_82586 = 0x00000fc0, // 82586 error bits
		RFD_ERROR       = 0x00001fe1, // everything except multicast

		RFD_OK          = 0x00002000, // frame received successfully
		RFD_B           = 0x00004000, // busy receiving frame
		RFD_C           = 0x00008000, // frame reception completed
		RFD_SF          = 0x00080000, // simplified/flexible mode
		RFD_S           = 0x40000000, // suspend after receive
		RFD_EL          = 0x80000000  // end of receive frame list
	};
	enum ru_receive_count_mask
	{
		RB_COUNT = 0x3fff, // actual count
		RB_F     = 0x4000, // buffer used
		RB_EOF   = 0x8000  // end of frame
	};
	enum ru_receive_size_mask
	{
		RB_SIZE = 0x3fff, // receive buffer size
		RB_P    = 0x4000, // rbd prefetched (82596 only)
		RB_EL   = 0x8000  // end of rbd list
	};

	static const u32 SCP_ADDRESS = 0x00fffff4; // the default value of the system configuration pointer
	static const u32 TBD_EMPTY   = 0x0000ffff; // FIXME: datasheet says this field should be "all 1's", but InterPro sets only lower 16 bits (in linear mode)
	static const u32 RBD_EMPTY   = 0x0000ffff;
	static const int MAX_FRAME_SIZE = 65536;   // real device effectively has no limit, emulation relies on index wrapping

	static const u32 FCS_RESIDUE = 0xdebb20e3; // the residue after computing the fcs over a complete frame (including fcs)

	// callback configuration
	auto out_irq_cb() { return m_out_irq.bind(); }

	void ca(int state);
	void reset_w(int state);

protected:
	i82586_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endian, u8 datawidth, u8 addrwidth);

	// standard device_* overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	// device_network_interface overrides
	virtual void send_complete_cb(int result) override;
	virtual int recv_start_cb(u8 *buf, int length) override;
	virtual void recv_complete_cb(int result) override;

	// setup and scb
	virtual void initialise() = 0;
	virtual void process_scb();
	virtual void update_scb();

	// command unit
	virtual TIMER_CALLBACK_MEMBER(cu_execute);
	virtual void cu_complete(const u16 status);
	virtual bool cu_iasetup() = 0;
	virtual bool cu_configure() = 0;
	virtual bool cu_mcsetup() = 0;
	virtual bool cu_transmit(u32 command) = 0;
	virtual bool cu_tdreflect() = 0;
	virtual bool cu_dump() = 0;

	// receive unit
	int recv_start(u8 *buf, int length);
	virtual bool address_filter(u8 *mac);
	virtual u16 ru_execute(u8 *buf, int length) = 0;
	virtual void ru_complete(const u16 status) = 0;

	// helpers
	void set_irq(bool irq);
	virtual u32 address(u32 base, int offset, int address) = 0;
	virtual u32 address(u32 base, int offset, int address, u16 empty) = 0;
	static u32 compute_crc(u8 *buf, int length, bool crc32);
	static u64 address_hash(u8 *buf, int length);
	int fetch_bytes(u8 *buf, u32 src, int length);
	int store_bytes(u32 dst, u8 *buf, int length);
	void dump_bytes(u8 *buf, int length);

	// device_* members
	const address_space_config m_space_config;
	address_space *m_space;

	devcb_write_line m_out_irq;
	emu_timer *m_cu_timer;

	// interrupt state
	bool m_cx;          // command executed (with interrupt)
	bool m_fr;          // frame received
	bool m_cna;         // command unit became inactive
	bool m_rnr;         // receive unit became not ready
	bool m_initialised;
	bool m_reset;
	bool m_irq;
	int m_irq_assert;   // configurable interrupt polarity

	// receive/command unit state
	cu_state m_cu_state; // command unit state
	ru_state m_ru_state; // receive unit state
	u32 m_scp_address;   // system configuration pointer
	u32 m_scb_base;      // system control block base (segmented modes)
	u32 m_scb_address;   // system control block address (linear mode)
	u32 m_scb_cs;        // current scb command and status
	u32 m_cba;           // current command block address
	u32 m_rfd;           // current receive frame descriptor address

	u64 m_mac_multi; // multicast address hash table

	// configure parameters
	enum lb_mode
	{
		LOOPBACK_NONE     = 0x00,
		LOOPBACK_INTERNAL = 0x40,
		LOOPBACK_EXTERNAL = 0x80,
		LOOPBACK_BOTH     = 0xc0  // internal loopback for 82586, external loopback for 82596
	};
	virtual u8 cfg_get(int offset) const = 0;
	virtual void cfg_set(int offset, u8 data) = 0;

	bool cfg_save_bad_frames()   const { return BIT(cfg_get(2), 7); }
	u8 cfg_address_length()      const { return (cfg_get(3) & 0x7) % 7; } // "NOTE: 7 is interpreted as 0"
	bool cfg_no_src_add_ins()    const { return BIT(cfg_get(3), 3); }
	lb_mode cfg_loopback_mode()  const { return (lb_mode)(cfg_get(3) & LOOPBACK_BOTH); }
	bool cfg_promiscuous_mode()  const { return BIT(cfg_get(8), 0); }
	bool cfg_broadcast_disable() const { return BIT(cfg_get(8), 1); }
	bool cfg_no_crc_insertion()  const { return BIT(cfg_get(8), 4); }
	bool cfg_crc16()             const { return BIT(cfg_get(8), 5); }
	u8 cfg_min_frame_length()    const { return cfg_get(10); }
};

class i82586_device : public i82586_base_device
{
public:
	i82586_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static const int CFG_SIZE = 12;
	static const int DUMP_SIZE = 170;

protected:
	// standard device_* overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// setup and scb
	virtual void initialise() override;

	// command unit
	virtual bool cu_iasetup() override;
	virtual bool cu_configure() override;
	virtual bool cu_mcsetup() override;
	virtual bool cu_transmit(u32 command) override;
	virtual bool cu_tdreflect() override;
	virtual bool cu_dump() override;

	// receive unit
	virtual bool address_filter(u8 *mac) override;
	virtual u16 ru_execute(u8 *buf, int length) override;
	virtual void ru_complete(const u16 status) override;

	// helpers
	virtual u32 address(u32 base, int offset, int address) override { return m_scb_base + m_space->read_word(base + offset); }
	virtual u32 address(u32 base, int offset, int address, u16 empty) override;

private:
	// configure parameters
	virtual u8 cfg_get(int offset) const override { return m_cfg_bytes[offset]; }
	virtual void cfg_set(int offset, u8 data) override { m_cfg_bytes[offset] = data; }

	u8 m_cfg_bytes[CFG_SIZE];
	u16 m_rbd_offset; // next available receive buffer descriptor
};

class i82596_device : public i82586_base_device
{
public:
	enum sysbus_mask
	{
		SYSBUS_M    = 0x06,
		SYSBUS_TRG  = 0x08,
		SYSBUS_LOCK = 0x10,
		SYSBUS_INT  = 0x20,
		SYSBUS_BE   = 0x80
	};
	enum sysbus_mode
	{
		MODE_82586       = 0x0,
		MODE_32SEGMENTED = 0x2,
		MODE_LINEAR      = 0x4
	};

	static const int CFG_SIZE = 14;
	static const int DUMP_SIZE = 304;

	void port(u32 data); // cpu access interface

protected:
	i82596_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endian, u8 datawidth);

	// standard device_* overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// setup and scb
	virtual void initialise() override;

	// command unit
	virtual bool cu_iasetup() override;
	virtual bool cu_configure() override;
	virtual bool cu_mcsetup() override;
	virtual bool cu_transmit(u32 command) override;
	virtual bool cu_tdreflect() override;
	virtual bool cu_dump() override;

	// receive unit
	virtual bool address_filter(u8 *mac) override;
	virtual u16 ru_execute(u8 *buf, int length) override;
	virtual void ru_complete(const u16 status) override;

	// helpers
	virtual u32 address(u32 base, int offset, int address) override { return (mode() == MODE_LINEAR) ? m_space->read_dword(base + address) : m_scb_base + m_space->read_word(base + offset); }
	virtual u32 address(u32 base, int offset, int address, u16 empty) override;

private:
	// configure parameters
	virtual u8 cfg_get(int offset) const override { return m_cfg_bytes[offset]; }
	virtual void cfg_set(int offset, u8 data) override { m_cfg_bytes[offset] = data; }

	bool cfg_crc_in_memory() const { return BIT(cfg_get(11), 2); }
	bool cfg_mc_all()        const { return BIT(cfg_get(11), 5); }
	bool cfg_multi_ia()      const { return BIT(cfg_get(13), 6); }

	// initialisation, configuration and sysbus
	inline sysbus_mode mode() const { return (sysbus_mode)(m_sysbus & SYSBUS_M); }

	u8 m_cfg_bytes[CFG_SIZE];
	u8 m_sysbus;

	u32 m_rbd_address;  // next available receive buffer descriptor
	u64 m_mac_multi_ia; // multi-ia address hash table
};

class i82596_le16_device : public i82596_device
{
public:
	i82596_le16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i82596_be16_device : public i82596_device
{
public:
	i82596_be16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i82596_le32_device : public i82596_device
{
public:
	i82596_le32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i82596_be32_device : public i82596_device
{
public:
	i82596_be32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(I82586, i82586_device)
DECLARE_DEVICE_TYPE(I82596_LE16, i82596_le16_device)
DECLARE_DEVICE_TYPE(I82596_BE16, i82596_be16_device)
DECLARE_DEVICE_TYPE(I82596_LE32, i82596_le32_device)
DECLARE_DEVICE_TYPE(I82596_BE32, i82596_be32_device)

#endif // MAME_MACHINE_I82586_H
