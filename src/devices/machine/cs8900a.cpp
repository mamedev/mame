// license:GPL-2.0+
// copyright-holders:Spiro Trikaliotis, Rhett Aultman

/*************************************************************************

    CS8900A ethernet controller implementation

    by Rhett Aultman <roadriverrail@gmail.com>
    ported to MAME from VICE Project (https://sourceforge.net/p/vice-emu/)
    VICE CS8900 code by Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>

 **************************************************************************/

#include "emu.h"
#include "cs8900a.h"

#include <cstring>

DEFINE_DEVICE_TYPE(CS8900A, cs8900a_device, "cs8900a", "CS8900A Crystal LAN 10Base-T Ethernet MAC")

/* warn illegal behaviour */
#define CS8900_DEBUG_WARN_REG (1U << 1)     /* warn about invalid register accesses */
#define CS8900_DEBUG_WARN_RXTX (1U << 2)    /* warn about invalid rx or tx conditions */

#define CS8900_DEBUG (1U << 3)              /* enable to see port reads */
#define CS8900_DEBUG_INIT (1U << 4)
#define CS8900_DEBUG_LOAD (1U << 5)         /* enable to see port reads */
#define CS8900_DEBUG_STORE (1U << 6)        /* enable to see port writes */
#define CS8900_DEBUG_REGISTERS (1U << 7)    /* enable to see CS8900a register I/O */
#define CS8900_DEBUG_RXTX_STATE (1U << 8)   /* enable to see tranceiver state changes */
#define CS8900_DEBUG_RXTX_DATA (1U << 9)    /* enable to see data in/out flow */
#define CS8900_DEBUG_FRAMES (1U << 10)      /* enable to see arch frame send/recv */

/** #define CS8900_DEBUG_IGNORE_RXEVENT 1 **/ /* enable to ignore RXEVENT in DEBUG_REGISTERS */
#define VERBOSE 0

#include "logmacro.h"

#define MAX_FRAME_QUEUE_ENTRIES 4096

/* CS8900 registers */

/*  these are the 8 16-bit-ports for "I/O space configuration"
    (see 4.10 on page 75 of cs8900a-4.pdf, the cs8900a data sheet)

    REMARK: The code operatoes the cs8900a in IO space configuration, as
    it generates I/OW and I/OR signals.
 */

/*
    RW: RXTXDATA    = DE00/DE01
    RW: RXTXDATA2   = DE02/DE03 (for 32-bit-operation)
    -W: TXCMD       = DE04/DE05 (TxCMD, Transmit Command)   mapped to PP + 0144 (Reg. 9, Sec. 4.4, page 46)
    -W: TXLENGTH    = DE06/DE07 (TxLength, Transmit Length) mapped to PP + 0146
    R-: INTSTQUEUE  = DE08/DE09 (Interrupt Status Queue)    mapped to PP + 0120 (ISQ, Sec. 5.1, page 78)
    RW: PP_PTR      = DE0A/DE0B (PacketPage Pointer)        (see. page 75p: Read -011.---- ----.----)
    RW: PP_DATA0    = DE0C/DE0D (PacketPage Data (Port 0))
    RW: PP_DATA1    = DE0E/DE0F (PacketPage Data (Port 1))  (for 32 bit only)
 */

enum io_space_conf_regs_e : u8 {
	CS8900_ADDR_RXTXDATA    = 0x00, /* RW */
	CS8900_ADDR_RXTXDATA2   = 0x02, /* RW 32 bit only! */
	CS8900_ADDR_TXCMD       = 0x04, /* -W Maps to PP+0144 */
	CS8900_ADDR_TXLENGTH    = 0x06, /* -W Maps to PP+0146 */
	CS8900_ADDR_INTSTQUEUE  = 0x08, /* R- Interrupt status queue, maps to PP + 0120 */
	CS8900_ADDR_PP_PTR      = 0x0a, /* RW PacketPage Pointer */
	CS8900_ADDR_PP_DATA     = 0x0c, /* RW PacketPage Data, Port 0 */
	CS8900_ADDR_PP_DATA2    = 0x0e  /* RW PacketPage Data, Port 1 - 32 bit only */
};

/* Makros for reading and writing the visible register: */
#define GET_CS8900_8(_xxx_)                     \
	(assert(_xxx_ < CS8900_COUNT_IO_REGISTER),  \
		cs8900_ioregs[_xxx_]                    \
	)

#define SET_CS8900_8(_xxx_, _val_)                  \
	do {                                            \
		assert(_xxx_ < CS8900_COUNT_IO_REGISTER);   \
		cs8900_ioregs[_xxx_] = (_val_) & 0xff;      \
	} while (0)

#define GET_CS8900_16(_xxx_)                                    \
	(assert(_xxx_ < CS8900_COUNT_IO_REGISTER),                  \
		cs8900_ioregs[_xxx_] | (cs8900_ioregs[_xxx_ + 1] << 8)  \
	)

#define SET_CS8900_16(_xxx_, _val_)                     \
	do {                                                \
		assert(_xxx_ < CS8900_COUNT_IO_REGISTER);       \
		cs8900_ioregs[_xxx_] = (_val_) & 0xff;          \
		cs8900_ioregs[_xxx_ + 1] = (_val_ >> 8) & 0xff; \
	} while (0)

/* The PacketPage register */
/* note: The locations 0 to MAX_PACKETPAGE_ARRAY-1 are handled in this array. */

/* Macros for reading and writing the PacketPage register: */

#define GET_PP_8(_xxx_)                     \
	(assert(_xxx_ < MAX_PACKETPAGE_ARRAY),  \
		cs8900_packetpage[_xxx_]            \
	)

#define GET_PP_16(_xxx_)                                \
	(assert(_xxx_ < MAX_PACKETPAGE_ARRAY),              \
		assert((_xxx_ & 1) == 0),                       \
			((u16)cs8900_packetpage[_xxx_])             \
			|((u16)cs8900_packetpage[_xxx_ + 1] <<  8)  \
	)

#define GET_PP_32(_xxx_)                                    \
	(assert(_xxx_ < MAX_PACKETPAGE_ARRAY),                  \
		assert((_xxx_ & 3) == 0),                           \
			(((u32)cs8900_packetpage[_xxx_]))               \
			|(((u32)cs8900_packetpage[_xxx_ + 1]) << 8)     \
			|(((u32)cs8900_packetpage[_xxx_ + 2]) << 16)    \
			|(((u32)cs8900_packetpage[_xxx_ + 3]) << 24)    \
	)

#define SET_PP_8(_xxx_, _val_)                      \
	do {                                            \
		assert(_xxx_ < MAX_PACKETPAGE_ARRAY);       \
		cs8900_packetpage[_xxx_] = (_val_) & 0xFF;  \
	} while (0)

#define SET_PP_16(_xxx_, _val_)                             \
	do {                                                    \
		assert(_xxx_ < MAX_PACKETPAGE_ARRAY);               \
		assert((_xxx_ & 1) == 0),                           \
			cs8900_packetpage[_xxx_] = (_val_) & 0xFF;      \
		cs8900_packetpage[_xxx_ + 1] = (_val_ >> 8) & 0xFF; \
	} while (0)

#define SET_PP_32(_xxx_, _val_)                                 \
	do {                                                        \
		assert(_xxx_ < MAX_PACKETPAGE_ARRAY);                   \
		assert((_xxx_ & 3) == 0),                               \
			cs8900_packetpage[_xxx_] = (_val_) & 0xFF;          \
		cs8900_packetpage[_xxx_ + 1] = (_val_ >> 8) & 0xFF;     \
		cs8900_packetpage[_xxx_ + 2] = (_val_ >> 16) & 0xFF;    \
		cs8900_packetpage[_xxx_ + 3] = (_val_ >> 24) & 0xFF;    \
	} while (0)

enum packetpage_regs_e : u16 {
	/* The packetpage register: see p. 39f */
	CS8900_PP_ADDR_PRODUCTID    = 0x0000,       /*   R- - 4.3., p. 41 */
	CS8900_PP_ADDR_IOBASE       = 0x0020,       /* i RW - 4.3., p. 41 - 4.7., p. 72 */
	CS8900_PP_ADDR_INTNO        = 0x0022,       /* i RW - 3.2., p. 17 - 4.3., p. 41 */
	CS8900_PP_ADDR_DMA_CHAN     = 0x0024,       /* i RW - 3.2., p. 17 - 4.3., p. 41 */
	CS8900_PP_ADDR_DMA_SOF      = 0x0026,       /* ? R- - 4.3., p. 41 - 5.4., p. 89 */
	CS8900_PP_ADDR_DMA_FC       = 0x0028,       /* ? R- - 4.3., p. 41, "Receive DMA" */
	CS8900_PP_ADDR_RXDMA_BC     = 0x002a,       /* ? R- - 4.3., p. 41 - 5.4., p. 89 */
	CS8900_PP_ADDR_MEMBASE      = 0x002c,       /* i RW - 4.3., p. 41 - 4.9., p. 73 */
	CS8900_PP_ADDR_BPROM_BASE   = 0x0030,       /* i RW - 3.6., p. 24 - 4.3., p. 41 */
	CS8900_PP_ADDR_BPROM_MASK   = 0x0034,       /* i RW - 3.6., p. 24 - 4.3., p. 41 */
	/* 0x0038 - 0x003F: reserved */
	CS8900_PP_ADDR_EEPROM_CMD   = 0x0040,       /* i RW - 3.5., p. 23 - 4.3., p. 41 */
	CS8900_PP_ADDR_EEPROM_DATA  = 0x0042,       /* i RW - 3.5., p. 23 - 4.3., p. 41 */
	/* 0x0044 - 0x004F: reserved */
	CS8900_PP_ADDR_REC_FRAME_BC = 0x0050,       /*   RW - 4.3., p. 41 - 5.2.9., p. 86 */
	/* 0x0052 - 0x00FF: reserved */
	CS8900_PP_ADDR_CONF_CTRL    = 0x0100,       /* - RW - 4.4., p. 46; see below */
	CS8900_PP_ADDR_STATUS_EVENT = 0x0120,       /* - R- - 4.4., p. 46; see below */
	/* 0x0140 - 0x0143: reserved */
	CS8900_PP_ADDR_TXCMD        = 0x0144,       /* # -W - 4.5., p. 70 - 5.7., p. 98 */
	CS8900_PP_ADDR_TXLENGTH     = 0x0146,       /* # -W - 4.5., p. 70 - 5.7., p. 98 */
	/* 0x0148 - 0x014F: reserved */
	CS8900_PP_ADDR_LOG_ADDR_FILTER  = 0x0150,   /*   RW - 4.6., p. 71 - 5.3., p. 86 */
	CS8900_PP_ADDR_MAC_ADDR         = 0x0158,   /* # RW - 4.6., p. 71 - 5.3., p. 86 */
	/* 0x015E - 0x03FF: reserved */
	CS8900_PP_ADDR_RXSTATUS     = 0x0400,       /*   R- - 4.7., p. 72 - 5.2., p. 78 */
	CS8900_PP_ADDR_RXLENGTH     = 0x0402,       /*   R- - 4.7., p. 72 - 5.2., p. 78 */
	CS8900_PP_ADDR_RX_FRAMELOC  = 0x0404,       /*   R- - 4.7., p. 72 - 5.2., p. 78 */
	/* here, the received frame is stored */
	CS8900_PP_ADDR_TX_FRAMELOC  = 0x0A00,       /*   -W - 4.7., p. 72 - 5.7., p. 98 */
	/* here, the frame to transmit is stored */
	CS8900_PP_ADDR_END          = 0x1000,       /* memory to CS8900_PP_ADDR_END-1 is used */
	/* CS8900_PP_ADDR_CONF_CTRL is subdivided: */
	CS8900_PP_ADDR_CC_RXCFG     = 0x0102,       /* # RW - 4.4.6.,   p. 52 - 0003 */
	CS8900_PP_ADDR_CC_RXCTL     = 0x0104,       /* # RW - 4.4.8.,   p. 54 - 0005 */
	CS8900_PP_ADDR_CC_TXCFG     = 0x0106,       /*   RW - 4.4.9.,   p. 55 - 0007 */
	CS8900_PP_ADDR_CC_TXCMD     = 0x0108,       /*   R- - 4.4.11., p. 57 - 0009 */
	CS8900_PP_ADDR_CC_BUFCFG    = 0x010A,       /*   RW - 4.4.12., p. 58 - 000B */
	CS8900_PP_ADDR_CC_LINECTL   = 0x0112,       /* # RW - 4.4.16., p. 62 - 0013 */
	CS8900_PP_ADDR_CC_SELFCTL   = 0x0114,       /*   RW - 4.4.18., p. 64 - 0015 */
	CS8900_PP_ADDR_CC_BUSCTL    = 0x0116,       /*   RW - 4.4.20., p. 66 - 0017 */
	CS8900_PP_ADDR_CC_TESTCTL   = 0x0118,       /*   RW - 4.4.22., p. 68 - 0019 */
	/* CS8900_PP_ADDR_STATUS_EVENT is subdivided: */
	CS8900_PP_ADDR_SE_ISQ       = 0x0120,       /*   R- - 4.4.5.,   p. 51 - 0000 */
	CS8900_PP_ADDR_SE_RXEVENT   = 0x0124,       /* # R- - 4.4.7.,   p. 53 - 0004 */
	CS8900_PP_ADDR_SE_TXEVENT   = 0x0128,       /*   R- - 4.4.10., p. 56 - 0008 */
	CS8900_PP_ADDR_SE_BUFEVENT  = 0x012C,       /*   R- - 4.4.13., p. 59 - 000C */
	CS8900_PP_ADDR_SE_RXMISS    = 0x0130,       /*   R- - 4.4.14., p. 60 - 0010 */
	CS8900_PP_ADDR_SE_TXCOL     = 0x0132,       /*   R- - 4.4.15., p. 61 - 0012 */
	CS8900_PP_ADDR_SE_LINEST    = 0x0134,       /*   R- - 4.4.17., p. 63 - 0014 */
	CS8900_PP_ADDR_SE_SELFST    = 0x0136,       /*   R- - 4.4.19., p. 65 - 0016 */
	CS8900_PP_ADDR_SE_BUSST     = 0x0138,       /* # R- - 4.4.21., p. 67 - 0018 */
	CS8900_PP_ADDR_SE_TDR       = 0x013C        /*   R- - 4.4.23., p. 69 - 001C */
};

enum tx_rx_min_max_e : u16 {
	MAX_TXLENGTH = 1518,
	MIN_TXLENGTH = 4,
	MAX_RXLENGTH = 1518,
	MIN_RXLENGTH = 64
};

enum cs8900_tx_state_e : u8 {
	CS8900_TX_IDLE          = 0,
	CS8900_TX_GOT_CMD       = 1,
	CS8900_TX_GOT_LEN       = 2,
	CS8900_TX_READ_BUSST    = 3
};

enum cs8900_rx_state_e : u8 {
	CS8900_RX_IDLE      = 0,
	CS8900_RX_GOT_FRAME = 1
};

enum pp_ptr_masks_e : u16 {
	PP_PTR_AUTO_INCR_FLAG   = 0x8000, /* auto increment flag in package pointer */
	PP_PTR_FLAG_MASK        = 0xf000, /* is always : x y 1 1 (with x=auto incr) */
	PP_PTR_ADDR_MASK        = 0x0fff /* address portion of packet page pointer */
};

#define LO_BYTE(x) (u8)((x)& 0xff)
#define HI_BYTE(x) (u8)(((x) >> 8)& 0xff)
#define LOHI_WORD(x, y) ((u16)(x) | (((u16)(y)) << 8))

// ------------------- END #defines ---------------------

// ---- debug logging support ----
std::string cs8900a_device::debug_outbuffer(const int length, const unsigned char *const buffer)
{
	static constexpr u32 MAXLEN_DEBUG = 1600;
	static constexpr u32 TFE_DEBUG_MAX_FRAME_DUMP = 150;
	std::ostringstream outbuffer;

	static_assert(TFE_DEBUG_MAX_FRAME_DUMP <= MAXLEN_DEBUG, "Max frame dump must not be larger than max debug length");

	for (int i = 0; (i < TFE_DEBUG_MAX_FRAME_DUMP) && (i < length); i++)
		util::stream_format(outbuffer, "%02X%c", buffer[i], ((i + 1) % 16 == 0) ? '*' : (((i + 1) % 8 == 0) ? '-' : ' '));

	return outbuffer.str();
}

void cs8900a_device::cs8900_set_tx_status(int ready, int error)
{
	u16 old_status = GET_PP_16(CS8900_PP_ADDR_SE_BUSST);

	/* mask out TxBidErr and Rdy4TxNOW */
	u16 new_status = old_status & ~0x180;

	if (ready)
		new_status |= 0x100;    /* set Rdy4TxNOW */

	if (error)
		new_status |= 0x080;    /* set TxBidErr */

	if (new_status != old_status)
	{
		SET_PP_16(CS8900_PP_ADDR_SE_BUSST, new_status);
		LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: set status Rdy4TxNOW=%d TxBidErr=%d",
					ready, error);
	}
}

void cs8900a_device::cs8900_set_receiver(int enabled)
{
	rx_enabled = enabled;
	rx_state = CS8900_RX_IDLE;

	rxevent_read_mask = 3; /* was L or H byte read in RXEVENT? */
}

void cs8900a_device::cs8900_set_transmitter(int enabled)
{
	tx_enabled = enabled;
	tx_state = CS8900_TX_IDLE;

	cs8900_set_tx_status(0, 0);
}

void cs8900a_device::device_reset()
{
	// Flush the inbound packet queue
	std::queue<std::vector<u8> > empty_queue;
	std::swap(m_frame_queue, empty_queue);

	/* initialize visible IO register and PacketPage registers */
	memset(cs8900_ioregs, 0, CS8900_COUNT_IO_REGISTER);
	memset(cs8900_packetpage, 0, MAX_PACKETPAGE_ARRAY);

	/* according to page 19 unless stated otherwise */
	SET_PP_32(CS8900_PP_ADDR_PRODUCTID, 0x0900630E); /* p.41: 0E630009 for Rev. D; reversed order! */
	SET_PP_16(CS8900_PP_ADDR_IOBASE, 0x0300);
	SET_PP_16(CS8900_PP_ADDR_INTNO, 0x0004); /* xxxx xxxx xxxx x100b */
	SET_PP_16(CS8900_PP_ADDR_DMA_CHAN, 0x0003); /* xxxx xxxx xxxx xx11b */

	/* according to descriptions of the registers, see definitions of
	     CS8900_PP_ADDR_CC_... and CS8900_PP_ADDR_SE_... above! */

	SET_PP_16(CS8900_PP_ADDR_CC_RXCFG,      0x0003);
	SET_PP_16(CS8900_PP_ADDR_CC_RXCTL,      0x0005);
	SET_PP_16(CS8900_PP_ADDR_CC_TXCFG,      0x0007);
	SET_PP_16(CS8900_PP_ADDR_CC_TXCMD,      0x0009);
	SET_PP_16(CS8900_PP_ADDR_CC_BUFCFG,     0x000B);
	SET_PP_16(CS8900_PP_ADDR_CC_LINECTL,    0x0013);
	SET_PP_16(CS8900_PP_ADDR_CC_SELFCTL,    0x0015);
	SET_PP_16(CS8900_PP_ADDR_CC_BUSCTL,     0x0017);
	SET_PP_16(CS8900_PP_ADDR_CC_TESTCTL,    0x0019);

	SET_PP_16(CS8900_PP_ADDR_SE_ISQ,        0x0000);
	SET_PP_16(CS8900_PP_ADDR_SE_RXEVENT,    0x0004);
	SET_PP_16(CS8900_PP_ADDR_SE_TXEVENT,    0x0008);
	SET_PP_16(CS8900_PP_ADDR_SE_BUFEVENT,   0x000C);
	SET_PP_16(CS8900_PP_ADDR_SE_RXMISS,     0x0010);
	SET_PP_16(CS8900_PP_ADDR_SE_TXCOL,      0x0012);
	SET_PP_16(CS8900_PP_ADDR_SE_LINEST,     0x0014);
	SET_PP_16(CS8900_PP_ADDR_SE_SELFST,     0x0016);
	SET_PP_16(CS8900_PP_ADDR_SE_BUSST,      0x0018);
	SET_PP_16(CS8900_PP_ADDR_SE_TDR,        0x001C);

	SET_PP_16(CS8900_PP_ADDR_TXCMD,         0x0009);

	/* 4.4.19 Self Status Register, p. 65
	     Important: set INITD (Bit 7) to signal device is ready */
	SET_PP_16(CS8900_PP_ADDR_SE_SELFST,     0x0896);

	cs8900_recv_control = GET_PP_16(CS8900_PP_ADDR_CC_RXCTL);

	/* spec: mac address is undefined after reset.
	     real HW: keeps the last set address. */
	for (int i = 0; i < 6; i++)
		SET_PP_8(CS8900_PP_ADDR_MAC_ADDR + i, cs8900_ia_mac[i]);

	/* reset state */
	cs8900_set_transmitter(0);
	cs8900_set_receiver(0);
}

void cs8900a_device::device_start()
{
	LOGMASKED(CS8900_DEBUG, "device_start().\n");
	LOGMASKED(CS8900_DEBUG_INIT, "\tcs8900_ioregs at $%08X, cs8900_packetpage at $%08X", cs8900_ioregs, cs8900_packetpage);

	/* virtually reset the LAN chip */
	device_reset();

	save_item(NAME(cs8900_ia_mac));
	save_item(NAME(cs8900_hash_mask));
	save_item(NAME(cs8900_ioregs));
	save_item(NAME(cs8900_packetpage));
	save_item(NAME(cs8900_packetpage_ptr));
	save_item(NAME(cs8900_recv_control));
	save_item(NAME(cs8900_recv_broadcast));
	save_item(NAME(cs8900_recv_mac));
	save_item(NAME(cs8900_recv_multicast));
	save_item(NAME(cs8900_recv_correct));
	save_item(NAME(cs8900_recv_promiscuous));
	save_item(NAME(cs8900_recv_hashfilter));
	save_item(NAME(tx_buffer));
	save_item(NAME(rx_buffer));
	save_item(NAME(tx_count));
	save_item(NAME(rx_count));
	save_item(NAME(tx_length));
	save_item(NAME(rx_length));
	save_item(NAME(tx_state));
	save_item(NAME(rx_state));
	save_item(NAME(tx_enabled));
	save_item(NAME(rx_enabled));
	save_item(NAME(rxevent_read_mask));
}

cs8900a_device::cs8900a_device(const machine_config& mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_network_interface(mconfig, *this, 10)
	, cs8900_ia_mac{0, 0, 0, 0, 0, 0}
	, cs8900_packetpage_ptr(0)
	, cs8900_recv_control(0)            /* copy of CC_RXCTL (contains all bits below) */
	, cs8900_recv_broadcast(false)      /* broadcast */
	, cs8900_recv_mac(false)            /* individual address (IA) */
	, cs8900_recv_multicast(false)      /* multicast if address passes the hash filter */
	, cs8900_recv_correct(false)        /* accept correct frames */
	, cs8900_recv_promiscuous(false)    /* promiscuous mode */
	, cs8900_recv_hashfilter(false)     /* accept if IA passes the hash filter */
	, tx_buffer(CS8900_PP_ADDR_TX_FRAMELOC)
	, rx_buffer(CS8900_PP_ADDR_RXSTATUS)
	, tx_count(0)
	, rx_count(0)
	, tx_length(0)
	, rx_length(0)
	, tx_state(CS8900_TX_IDLE)
	, rx_state(CS8900_RX_IDLE)
	, tx_enabled(0)
	, rx_enabled(0)
	, rxevent_read_mask(3) /* set if L and/or H u8 was read in RXEVENT? */
{}

cs8900a_device::cs8900a_device(machine_config const& mconfig, char const *tag, device_t *owner, u32 clock)
	: cs8900a_device(mconfig, CS8900A, tag, owner, clock)
{}

/*
    This is a helper for cs8900_receive() to determine if the received frame should be accepted
    according to the settings.
 */
bool cs8900a_device::cs8900_should_accept(unsigned char *buffer, int length, bool *phashed, int *phash_index,
											bool *pcorrect_mac, bool *pbroadcast, bool *pmulticast)
{
	int hashreg; /* Hash Register (for hash computation) */

	assert(length >= 6); /* we need at least 6 octets since the DA has this length */

	/* first of all, delete any status */
	*phashed = false;
	*phash_index = 0;
	*pcorrect_mac = false;
	*pbroadcast = false;
	*pmulticast = false;

	LOGMASKED(CS8900_DEBUG_FRAMES, "cs8900_should_accept called with %02X:%02X:%02X:%02X:%02X:%02X, length=%4u",
				cs8900_ia_mac[0], cs8900_ia_mac[1], cs8900_ia_mac[2],
				cs8900_ia_mac[3], cs8900_ia_mac[4], cs8900_ia_mac[5],
				length,
				debug_outbuffer(length, buffer)
				);

	if ((buffer[0] == cs8900_ia_mac[0])
			&& (buffer[1] == cs8900_ia_mac[1])
			&& (buffer[2] == cs8900_ia_mac[2])
			&& (buffer[3] == cs8900_ia_mac[3])
			&& (buffer[4] == cs8900_ia_mac[4])
			&& (buffer[5] == cs8900_ia_mac[5])
			)
	{
		/* this is our individual address (IA) */
		*pcorrect_mac = true;

		/* if we don't want "correct MAC", we might have the chance
		 * that this address fits the hash index
		 */
		if (cs8900_recv_mac || cs8900_recv_promiscuous)
			return true;
	}

	if ((buffer[0] == 0xFF)
			&& (buffer[1] == 0xFF)
			&& (buffer[2] == 0xFF)
			&& (buffer[3] == 0xFF)
			&& (buffer[4] == 0xFF)
			&& (buffer[5] == 0xFF)
			)
	{
		/* this is a broadcast address */
		*pbroadcast = true;

		/* broadcasts cannot be accepted by the hash filter */
		return (cs8900_recv_broadcast || cs8900_recv_promiscuous);
	}

	/* now check if DA passes the hash filter */
	hashreg = (~util::crc32_creator::simple((char *)buffer, 6) >> 26) & 0x3F;

	*phashed = (cs8900_hash_mask[(hashreg >= 32) ? 1 : 0] & (1 << (hashreg & 0x1F)));

	if (*phashed)
	{
		*phash_index = hashreg;

		if (buffer[0] & 0x80)
		{
			/* we have a multicast address */
			*pmulticast = true;

			/* if the multicast address fits into the hash filter,
			 * the hashed bit has to be clear
			 */
			*phashed = false;

			return (cs8900_recv_multicast || cs8900_recv_promiscuous);
		}
		return (cs8900_recv_hashfilter || cs8900_recv_promiscuous);
	}

	return cs8900_recv_promiscuous;
}

u16 cs8900a_device::cs8900_receive()
{
	u16 ret_val = 0x0004;

	u8 buffer[MAX_RXLENGTH];

	int len;
	bool hashed = false;
	int hash_index  = 0;
	bool rx_ok = false;
	bool correct_mac = false;
	bool broadcast = false;
	bool multicast = false;
	bool crc_error = false;

	bool newframe = false;

	bool ready = false;

	do {
		len = MAX_RXLENGTH;

		ready = true; /* assume we will find a good frame */
		std::vector<u8> frame;

		if (!m_frame_queue.empty())
		{
			std::vector<u8> frame = m_frame_queue.front();
			m_frame_queue.pop();
			len = frame.size();
			std::memcpy(buffer, frame.data(), len);
			newframe = true;
			rx_ok = true;
		}
		else
		{
			newframe = false;
		}

		assert((len & 1) == 0); /* length has to be even! */

		if (newframe)
		{
			if (hashed || correct_mac || broadcast)
			{
				/* we already know the type of frame: Trust it! */
				LOGMASKED(CS8900_DEBUG_FRAMES, "+++ cs8900_receive(): *** hashed=%u, correct_mac=%u, "
							"broadcast=%u", hashed, correct_mac, broadcast);
			}
			else
			{
				/* determine ourself the type of frame */
				if (!cs8900_should_accept(buffer,
						len, &hashed, &hash_index, &correct_mac, &broadcast, &multicast))
				{
					/* if we should not accept this frame, just do nothing
					 * now, look for another one */
					ready = false; /* try another frame */
					continue;
				}
			}

			/* we did receive a frame, return that status */
			ret_val |= rx_ok ? 0x0100 : 0;
			ret_val |= multicast ? 0x0200 : 0;

			if (!multicast)
			{
				ret_val |= hashed ? 0x0040 : 0;
			}

			if (hashed && rx_ok)
			{
				/* we have the 2nd, special format with hash index: */
				assert(hash_index < 64);
				ret_val |= hash_index << 9;
			}
			else
			{
				/* we have the regular format */
				ret_val |= correct_mac ? 0x0400 : 0;
				ret_val |= broadcast ? 0x0800 : 0;
				ret_val |= crc_error ? 0x1000 : 0;
				ret_val |= (len < MIN_RXLENGTH) ? 0x2000 : 0;
				ret_val |= (len > MAX_RXLENGTH) ? 0x4000 : 0;
			}

			/* discard any octets that are beyond the MAX_RXLEN */
			if (len > MAX_RXLENGTH)
			{
				len = MAX_RXLENGTH;
			}

			if (rx_ok)
			{
				int i;

				/* set relevant parts of the PP area to correct values */
				SET_PP_16(CS8900_PP_ADDR_RXLENGTH, len);

				for (i = 0; i < len; i++)
				{
					SET_PP_8(CS8900_PP_ADDR_RX_FRAMELOC + i, buffer[i]);
				}

				/* set rx_buffer to where start reading *
				 * According to 4.10.9 (pp. 76-77), we start with RxStatus and RxLength!
				 */
				rx_buffer = CS8900_PP_ADDR_RXSTATUS;
				rx_length = len;
				rx_count = 0;

				if (rx_state != CS8900_RX_IDLE)
				{
					LOGMASKED(CS8900_DEBUG_WARN_RXTX, "WARNING! New frame overwrites pending one!");
				}
				rx_state = CS8900_RX_GOT_FRAME;
				LOGMASKED(CS8900_DEBUG_RXTX_STATE, "RX: recvd frame (length=%04x,status=%04x)",
							rx_length, ret_val);
			}
		}
	} while (!ready);

	if (ret_val != 0x0004)
		LOGMASKED(CS8900_DEBUG_FRAMES, "+++ cs8900_receive(): ret_val=%04X", ret_val);

	return ret_val;
}

/* ------------------------------------------------------------------------- */
/*      reading and writing IO register functions                                                           */

/*
    These registers are currently fully or partially supported:

    CS8900_PP_ADDR_CC_RXCFG             0x0102 * # RW - 4.4.6.,  p. 52 - 0003 *
    CS8900_PP_ADDR_CC_RXCTL             0x0104 * # RW - 4.4.8.,  p. 54 - 0005 *
    CS8900_PP_ADDR_CC_LINECTL           0x0112 * # RW - 4.4.16., p. 62 - 0013 *
    CS8900_PP_ADDR_SE_RXEVENT           0x0124 * # R- - 4.4.7.,  p. 53 - 0004 *
    CS8900_PP_ADDR_SE_BUSST             0x0138 * # R- - 4.4.21., p. 67 - 0018 *
    CS8900_PP_ADDR_TXCMD                0x0144 * # -W - 4.5., p. 70 - 5.7., p. 98 *
    CS8900_PP_ADDR_TXLENGTH             0x0146 * # -W - 4.5., p. 70 - 5.7., p. 98 *
    CS8900_PP_ADDR_MAC_ADDR             0x0158 * # RW - 4.6., p. 71 - 5.3., p. 86 *
                                        0x015a
                                        0x015c
 */

/* ------------------------------------------------------------------------- */
/* TX/RX buffer handling */

void cs8900a_device::cs8900_write_tx_buffer(u8 value, int odd_address)
{
	/* write tx data only if valid buffer is ready */
	if (tx_state != CS8900_TX_READ_BUSST)
	{
		LOGMASKED(CS8900_DEBUG_WARN_RXTX, "WARNING! Ignoring TX Write without correct Transmit Condition! (odd=%d,value=%02x)",
					odd_address, value);

		/* ensure correct tx state (needed if transmit < 4 was started) */
		cs8900_set_tx_status(0, 0);
	}
	else
	{
		if (tx_count == 0)
		{
			LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: write frame (length=%04x)", tx_length);
		}

		/* always write LH, LH... to tx buffer */
		u16 addr = tx_buffer;

		if (odd_address)
		{
			addr++;
			tx_buffer += 2;
		}
		tx_count++;
		SET_PP_8(addr, value);

		LOGMASKED(CS8900_DEBUG_RXTX_DATA, "TX: %04x/%04x: %02x (buffer=%04x,odd=%d)",
					tx_count, tx_length, value, addr, odd_address);

		/* full frame transmitted? */
		if (tx_count == tx_length)
		{
			LOGMASKED(CS8900_DEBUG_FRAMES, "cs8900_arch_transmit() called with:\t\t\t\t\t\t\t\t\t"
						"length=%4u and buffer %s", tx_length,
						debug_outbuffer(tx_length, &cs8900_packetpage[CS8900_PP_ADDR_TX_FRAMELOC])
						);

			if (!tx_enabled)
			{
				LOGMASKED(CS8900_DEBUG_WARN_RXTX, "WARNING! Can't transmit frame (Transmitter is not enabled)!");
			}
			else
			{
				/* send frame */
				LOGMASKED(CS8900_DEBUG, "SENDING from buf %p len %d\n", &cs8900_packetpage[CS8900_PP_ADDR_TX_FRAMELOC], tx_length);
				send(&cs8900_packetpage[CS8900_PP_ADDR_TX_FRAMELOC], tx_length);
			}

			/* reset transmitter state */
			tx_state = CS8900_TX_IDLE;

			LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: sent frame (length=%04x)", tx_length);

			/* reset tx status */
			cs8900_set_tx_status(0, 0);
		}
	}
}

u8 cs8900a_device::cs8900_read_rx_buffer(int odd_address)
{
	if (rx_state != CS8900_RX_GOT_FRAME)
	{
		LOGMASKED(CS8900_DEBUG_WARN_RXTX, "WARNING! RX Read without frame available! (odd=%d)",
					odd_address);

		/* always reads zero on HW */
		return 0;
	}
	else
	{
		/*
		    According to the CS8900 spec, the handling is the following:
		    first read H, then L (RX_STATUS), then H, then L (RX_LENGTH).
		    Inside the RX frame data, we always get L then H, until the end is reached.

		                                                even    odd
		    CS8900_PP_ADDR_RXSTATUS:                    -       proceed
		    CS8900_PP_ADDR_RXLENGTH:                    -       proceed
		    CS8900_PP_ADDR_RX_FRAMELOC:                 -       -
		    CS8900_PP_ADDR_RX_FRAMELOC+2:               proceed -
		    CS8900_PP_ADDR_RX_FRAMELOC+4:               proceed -
		*/
		u16 addr = odd_address ? 1 : 0;
		u8  value;

		/* read RXSTATUS or RX_LENGTH */
		if (rx_count < 4)
		{
			addr += rx_buffer;
			value = GET_PP_8(addr);
			rx_count++;

			/* incr after RXSTATUS or RX_LENGTH even (L) read */
			if (!odd_address)
				rx_buffer += 2;
		}
		/* read frame data */
		else
		{
			/* incr before frame read (but not in first word) */
			if ((rx_count >= 6) && (!odd_address))
				rx_buffer += 2;

			addr += rx_buffer;
			value = GET_PP_8(addr);
			rx_count++;
		}

		LOGMASKED(CS8900_DEBUG_RXTX_DATA, "RX: %04x/%04x: %02x (buffer=%04x,odd=%d)",
					rx_count, rx_length + 4, value, addr, odd_address);

		/* check frame end */
		if (rx_count >= rx_length + 4)
		{
			/* reset receiver state to idle */
			rx_state = CS8900_RX_IDLE;
			LOGMASKED(CS8900_DEBUG_RXTX_STATE, "RX: read frame (length=%04x)", rx_length);
		}
		return value;
	}
}

/* ------------------------------------------------------------------------- */
/* handle side-effects of read and write operations */

/*
     This is called *after* the relevant octets are written
 */
void cs8900a_device::cs8900_sideeffects_write_pp(u16 ppaddress, int odd_address)
{
	u16 content = GET_PP_16(ppaddress);

	assert((ppaddress & 1) == 0);

	switch (ppaddress)
	{
	case CS8900_PP_ADDR_CC_RXCFG:

		/* Skip_1 Flag: remove current (partial) tx frame and restore state */
		if (content & 0x40)
		{
			/* restore tx state */
			if (tx_state != CS8900_TX_IDLE)
			{
				tx_state = CS8900_TX_IDLE;
				LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: skipping current frame");
			}

			/* reset transmitter */
			cs8900_set_transmitter(tx_enabled);

			/* this is an "act once" bit, thus restore it to zero. */
			content &= ~0x40;
			SET_PP_16(ppaddress, content);
		}
		break;

	case CS8900_PP_ADDR_CC_RXCTL:

		if (cs8900_recv_control != content)
		{
			cs8900_recv_broadcast = content & 0x0800; /* broadcast */
			cs8900_recv_mac = content & 0x0400; /* individual address (IA) */
			cs8900_recv_multicast = content & 0x0200; /* multicast if address passes the hash filter */
			cs8900_recv_correct = content & 0x0100; /* accept correct frames */
			cs8900_recv_promiscuous = content & 0x0080; /* promiscuous mode */
			cs8900_recv_hashfilter = content & 0x0040; /* accept if IA passes the hash filter */
			cs8900_recv_control = content;
		}
		break;

	case CS8900_PP_ADDR_CC_LINECTL:
	{
		int enable_tx = (content & 0x0080) == 0x0080;
		int enable_rx = (content & 0x0040) == 0x0040;

		if ((enable_tx != tx_enabled) || (enable_rx != rx_enabled))
		{
			cs8900_set_transmitter(enable_tx);
			cs8900_set_receiver(enable_rx);
		}
		break;
	}

	case CS8900_PP_ADDR_CC_SELFCTL:
	{
		/* reset chip? */
		if ((content & 0x40) == 0x40)
		{
			device_reset();
		}
		break;
	}

	case CS8900_PP_ADDR_TXCMD:
	{
		if (odd_address)
		{
			u16 txcommand = GET_PP_16(CS8900_PP_ADDR_TXCMD);

			/* already transmitting? */
			if (tx_state == CS8900_TX_READ_BUSST)
			{
				LOGMASKED(CS8900_DEBUG_WARN_RXTX, "WARNING! Early abort of transmitted frame");
			}

			/* The transmit status command gets the last transmit command */
			SET_PP_16(CS8900_PP_ADDR_CC_TXCMD, txcommand);

			/* set transmit state */
			tx_state = CS8900_TX_GOT_CMD;
			cs8900_set_tx_status(0, 0);

			LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: COMMAND accepted (%04x)", txcommand);
		}
		break;
	}

	case CS8900_PP_ADDR_TXLENGTH:
	{
		if (odd_address && (tx_state == CS8900_TX_GOT_CMD))
		{
			u16 txlength = GET_PP_16(CS8900_PP_ADDR_TXLENGTH);
			u16 txcommand = GET_PP_16(CS8900_PP_ADDR_CC_TXCMD);

			if (txlength < 4)
			{
				/* frame to short */
				LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: LENGTH rejected - too short! (%04x)", txlength);

				/* mask space available but do not commit */
				tx_state = CS8900_TX_IDLE;
				cs8900_set_tx_status(1, 0);
			}
			else if ((txlength > MAX_TXLENGTH)
						|| ((txlength > MAX_TXLENGTH - 4) && (!(txcommand & 0x1000)))
					)
			{
				tx_state = CS8900_TX_IDLE;
				LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: LENGTH rejected - too long! (%04x)", txlength);

				/* txlength too big, mark an error */
				cs8900_set_tx_status(0, 1);
			}
			else
			{
				/* make sure we put the octets to transmit at the right place */
				tx_buffer = CS8900_PP_ADDR_TX_FRAMELOC;
				tx_count = 0;
				tx_length = txlength;
				tx_state = CS8900_TX_GOT_LEN;

				LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: LENGTH accepted (%04x)", txlength);

				/* all right, signal that we're ready for the next frame */
				cs8900_set_tx_status(1, 0);
			}
		}
		break;
	}

	case CS8900_PP_ADDR_LOG_ADDR_FILTER:
	case CS8900_PP_ADDR_LOG_ADDR_FILTER + 2:
	case CS8900_PP_ADDR_LOG_ADDR_FILTER + 4:
	case CS8900_PP_ADDR_LOG_ADDR_FILTER + 6:
	{
		unsigned int pos = 8 * (ppaddress - CS8900_PP_ADDR_LOG_ADDR_FILTER + odd_address);
		u32 *p = (pos < 32) ? &cs8900_hash_mask[0] : &cs8900_hash_mask[1];

		*p &= ~(0xFF << pos); /* clear out relevant bits */
		*p |= GET_PP_8(ppaddress + odd_address) << pos;
		break;
	}

	case CS8900_PP_ADDR_MAC_ADDR:
	case CS8900_PP_ADDR_MAC_ADDR + 2:
	case CS8900_PP_ADDR_MAC_ADDR + 4:

		/* the MAC address has been changed */
		cs8900_ia_mac[ppaddress - CS8900_PP_ADDR_MAC_ADDR + odd_address] = GET_PP_8(ppaddress + odd_address);
		set_mac(cs8900_ia_mac);

		if (odd_address && (ppaddress == CS8900_PP_ADDR_MAC_ADDR + 4))
			LOGMASKED(CS8900_DEBUG, "set MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
						cs8900_ia_mac[0], cs8900_ia_mac[1], cs8900_ia_mac[2],
						cs8900_ia_mac[3], cs8900_ia_mac[4], cs8900_ia_mac[5]);
		break;
	}
}

/*
     This is called *before* the relevant octets are read
 */
void cs8900a_device::cs8900_sideeffects_read_pp(u16 ppaddress, int odd_address)
{
	switch (ppaddress)
	{
	case CS8900_PP_ADDR_SE_RXEVENT:

		/* reading this before all octets of the frame are read performs an "implied skip" */
	{
		int access_mask = (odd_address) ? 1 : 2;

		/*  update the status register only if the full word of the last
		    status was read! unfortunately different access patterns are
		    possible: either the status is read LH, LH, LH...
		    or HL, HL, HL, or even L, L, L or H, H, H */
		if ((access_mask & rxevent_read_mask) != 0)
		{
			/* receiver is not enabled */
			if (!rx_enabled)
			{
				LOGMASKED(CS8900_DEBUG_WARN_RXTX, "WARNING! Can't receive any frame (Receiver is not enabled)!");
			}
			else
			{
				/* perform frame reception */
				u16 ret_val = cs8900_receive();

				/*  RXSTATUS and RXEVENT are the same, except that RXSTATUS buffers
				    the old value while RXEVENT sets a new value whenever it is called
				 */
				SET_PP_16(CS8900_PP_ADDR_RXSTATUS, ret_val);
				SET_PP_16(CS8900_PP_ADDR_SE_RXEVENT, ret_val);
			}

			/* reset read mask of (possible) other access */
			rxevent_read_mask = access_mask;
		}
		else
		{
			/* add access bit to mask */
			rxevent_read_mask |= access_mask;
		}
		break;
	}

	case CS8900_PP_ADDR_SE_BUSST:

		if (odd_address)
		{
			/* read busst before transmit condition is fullfilled */
			if (tx_state == CS8900_TX_GOT_LEN)
			{
				u16 bus_status = GET_PP_16(CS8900_PP_ADDR_SE_BUSST);

				/* check Rdy4TXNow flag */
				if ((bus_status & 0x100) == 0x100)
				{
					tx_state = CS8900_TX_READ_BUSST;
					LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: Ready4TXNow set! (%04x)",
								bus_status);
				}
				else
				{
					LOGMASKED(CS8900_DEBUG_RXTX_STATE, "TX: waiting for Ready4TXNow! (%04x)",
								bus_status);
				}
			}
		}
		break;
	}
}

/* ------------------------------------------------------------------------- */
/* read/write from packet page register */

/* read a register from packet page */
u16 cs8900a_device::cs8900_read_register(u16 ppaddress)
{
	u16 value = GET_PP_16(ppaddress);

	/* --- check the register address --- */
	if (ppaddress < 0x100)
	{
		/* reserved range reads 0x0300 on real HW */
		if ((ppaddress >= 0x0004) && (ppaddress < 0x0020))
		{
			return 0x0300;
		}
	}
	/* --- read control register range --- */
	else if (ppaddress < 0x120)
	{
		u16 regNum = ppaddress - 0x100;
		regNum &= ~1;
		regNum++;
		LOGMASKED(CS8900_DEBUG_REGISTERS, "Read Control Register %04x: %04x (reg=%02x)", ppaddress, value, regNum);
		/* reserved register? */
		if ((regNum == 0x01) || (regNum == 0x11) || (regNum > 0x19))
		{
			LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Read reserved Control Register %04x (reg=%02x)", ppaddress, regNum);
			/* real HW returns 0x0300 in reserved register range */
			return 0x0300;
		}
		/* make sure interal address is always valid */
		assert((value & 0x3f) == regNum);
	}
	/* --- read status register range --- */
	else if (ppaddress < 0x140)
	{
		u16 regNum = ppaddress - 0x120;
		regNum &= ~1;
#ifdef CS8900_DEBUG_IGNORE_RXEVENT
		if (regNum != 4) // do not show RXEVENT
#endif // ifdef CS8900_DEBUG_IGNORE_RXEVENT
		LOGMASKED(CS8900_DEBUG_REGISTERS, "Read  Status  Register %04x: %04x (reg=%02x)", ppaddress, value, regNum);
		/* reserved register? */
		if ((regNum == 0x02) || (regNum == 0x06) || (regNum == 0x0a) ||
				(regNum == 0x0e) || (regNum == 0x1a) || (regNum == 0x1e))
		{
			LOGMASKED(CS8900_DEBUG_WARN_REG,
						"WARNING! Read reserved Status Register %04x (reg=%02x)", ppaddress, regNum);
			/* real HW returns 0x0300 in reserved register range */
			return 0x0300;
		}
		/* make sure interal address is always valid */
		assert((value & 0x3f) == regNum);
	}
	/* --- read transmit register range --- */
	else if (ppaddress < 0x150)
	{
		if (ppaddress == 0x144)
		{
			/* make sure interal address is always valid */
			assert((value & 0x3f) == 0x09);
			LOGMASKED(CS8900_DEBUG_REGISTERS, "Read  TX Cmd  Register %04x: %04x", ppaddress, value);
		}
		else if (ppaddress == 0x146)
		{
			LOGMASKED(CS8900_DEBUG_REGISTERS, "Read  TX Len  Register %04x: %04x", ppaddress, value);
		}
		/* reserved range */
		else
		{
			LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Read reserved Initiate Transmit Register %04x", ppaddress);

			/* real HW returns 0x0300 in reserved register range */
			return 0x0300;
		}
	}
	/* --- read address filter register range --- */
	else if (ppaddress < 0x160)
	{
		/* reserved range */
		if (ppaddress >= 0x15e)
		{
			LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Read reserved Address Filter Register %04x", ppaddress);

			/* real HW returns 0x0300 in reserved register range */
			return 0x0300;
		}
	}
	/* --- reserved range below 0x400 (returns 0x300 on real HW) */
	else if (ppaddress < 0x400)
	{
		LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Read reserved Register %04x", ppaddress);
		return 0x0300;
	}
	/* --- range from 0x400 .. 0x9ff --- RX Frame */
	else if (ppaddress < 0xa00)
	{
		LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Read from RX Buffer Range %04x", ppaddress);
		return 0x0000;
	}
	/* --- range from 0xa00 .. 0xfff --- TX Frame */
	else
	{
		LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Read from TX Buffer Range %04x", ppaddress);
		return 0x0000;
	}
	/* actually read from pp memory */
	return value;
}

void cs8900a_device::cs8900_write_register(u16 ppaddress, u16 value)
{
	/* --- write bus interface register range --- */
	if (ppaddress < 0x100)
	{
		int ignore = 0;

		if (ppaddress < 0x20)
		{
			ignore = 1;
		}
		else if ((ppaddress >= 0x26) && (ppaddress < 0x2c))
		{
			ignore = 1;
		}
		else if (ppaddress == 0x38)
		{
			ignore = 1;
		}
		else if (ppaddress >= 0x44)
		{
			ignore = 1;
		}

		if (ignore)
		{
			LOGMASKED(CS8900_DEBUG_WARN_REG,
								"WARNING! Ignoring write to read only/reserved Bus Interface Register %04x",
								ppaddress);
			return;
		}
	}
	/* --- write to control register range --- */
	else if (ppaddress < 0x120)
	{
		u16 regNum = ppaddress - 0x100;
		regNum &= ~1;
		regNum += 1;

		/* validate internal address */
		if ((value & 0x3f) != regNum)
		{
			/* fix internal address */
			value &= ~0x3f;
			value |= regNum;
		}
		LOGMASKED(CS8900_DEBUG_REGISTERS, "Write Control Register %04x: %04x (reg=%02x)", ppaddress, value, regNum);

		/* invalid register? -> ignore! */
		if ((regNum == 0x01) || (regNum == 0x11) || (regNum > 0x19))
		{
			LOGMASKED(CS8900_DEBUG_WARN_REG,
								"WARNING! Ignoring write to reserved Control Register %04x (reg=%02x)",
								ppaddress, regNum);
			return;
		}
	}
	/* --- write to status register range --- */
	else if (ppaddress < 0x140)
	{
		LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Ignoring write to read-only Status Register %04x", ppaddress);
		return;
	}
	/* --- write to initiate transmit register range --- */
	else if (ppaddress < 0x150)
	{
		/* check tx_cmd register */
		if (ppaddress == 0x144)
		{
			/* validate internal address */
			if ((value & 0x3f) != 0x09)
			{
				/* fix internal address */
				value &= ~0x3f;
				value |= 0x09;
			}

			/* mask out reserved bits */
			value &= 0x33ff;
			LOGMASKED(CS8900_DEBUG_REGISTERS, "Write TX Cmd  Register %04x: %04x", ppaddress, value);
		}
		/* check tx_length register */
		else if (ppaddress == 0x146)
		{
			/* HW always masks 0x0fff */
			value &= 0x0fff;
			LOGMASKED(CS8900_DEBUG_REGISTERS, "Write TX Len  Register %04x: %04x", ppaddress, value);
		}
		/* reserved range */
		else if ((ppaddress < 0x144) || (ppaddress > 0x147))
		{
			LOGMASKED(CS8900_DEBUG_WARN_REG,
								"WARNING! Ignoring write to reserved Initiate Transmit Register %04x",
								ppaddress);
			return;
		}
	}
	/* --- write to address filter register range --- */
	else if (ppaddress < 0x160)
	{
		/* reserved range */
		if (ppaddress >= 0x15e)
		{
			LOGMASKED(CS8900_DEBUG_WARN_REG,
								"WARNING! Ingoring write to reserved Address Filter Register %04x",
								ppaddress);
			return;
		}
	}
	/* --- ignore write outside --- */
	else if (ppaddress < 0x400)
	{
		LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Ingoring write to reserved Register %04x", ppaddress);
		return;
	}
	else if (ppaddress < 0xa00)
	{
		LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Ignoring write to RX Buffer Range %04x", ppaddress);
		return;
	}
	else
	{
		LOGMASKED(CS8900_DEBUG_WARN_REG, "WARNING! Ignoring write to TX Buffer Range %04x", ppaddress);
		return;
	}
	/* actually set value */
	SET_PP_16(ppaddress, value);
}

void cs8900a_device::cs8900_auto_incr_pp_ptr()
{
	/* perform auto increment of packet page pointer */
	if ((cs8900_packetpage_ptr & PP_PTR_AUTO_INCR_FLAG) == PP_PTR_AUTO_INCR_FLAG)
	{
		/* pointer is always increment by one on real HW */
		u16 ptr = cs8900_packetpage_ptr & PP_PTR_ADDR_MASK;
		u16 flags = cs8900_packetpage_ptr & PP_PTR_FLAG_MASK;
		ptr++;
		cs8900_packetpage_ptr = ptr | flags;
	}
}

/* ----- read byte from I/O range ----- */
u8 cs8900a_device::cs8900_read(u16 io_address)
{
	u8 retval, lo, hi;
	u16 word_value;
	u16 reg_base;

	assert(cs8900_ioregs);
	assert(cs8900_packetpage);
	assert(io_address < 0x10);

	/* register base addr */
	reg_base = io_address & ~1;

	/* RX register is special as it reads from RX buffer directly */
	if ((reg_base == CS8900_ADDR_RXTXDATA) || (reg_base == CS8900_ADDR_RXTXDATA2))
	{
		LOGMASKED(CS8900_DEBUG_LOAD, "reading RX Register.\n");
		return cs8900_read_rx_buffer(io_address & 0x01);
	}

	/* read packet page pointer */
	if (reg_base == CS8900_ADDR_PP_PTR)
	{
		word_value = cs8900_packetpage_ptr;
	}
	/* read a register from packet page */
	else
	{
		u16 ppaddress = 0;

		/* determine read addr in packet page */
		switch (reg_base)
		{
		/* PP_DATA2 behaves like PP_DATA on real HW both show the contents at the page pointer */
		case CS8900_ADDR_PP_DATA:
		case CS8900_ADDR_PP_DATA2:

			/* mask and align address of packet pointer */
			ppaddress = cs8900_packetpage_ptr & PP_PTR_ADDR_MASK;
			ppaddress &= ~1;

			/* if flags match then auto incr pointer */
			if (!machine().side_effects_disabled())
			{
				cs8900_auto_incr_pp_ptr();
			}
			break;

		case CS8900_ADDR_INTSTQUEUE:
			ppaddress = CS8900_PP_ADDR_SE_ISQ;
			break;

		case CS8900_ADDR_TXCMD:
			ppaddress = CS8900_PP_ADDR_TXCMD;
			break;

		case CS8900_ADDR_TXLENGTH:
			ppaddress = CS8900_PP_ADDR_TXLENGTH;
			break;

		default:
			/* invalid! */
			assert(0);
			break;
		}

		/* do side effects before access */
		if (!machine().side_effects_disabled())
		{
			cs8900_sideeffects_read_pp(ppaddress, io_address & 1);
		}

		/* read register value */
		word_value = cs8900_read_register(ppaddress);

		LOGMASKED(CS8900_DEBUG_LOAD, "reading PP Ptr: $%04X => $%04X.\n", ppaddress, word_value);
	}

	/* extract return value from word_value */
	lo = LO_BYTE(word_value);
	hi = HI_BYTE(word_value);

	if ((io_address & 1) == 0)
	{
		/* low byte on even address */
		retval = lo;
	}
	else
	{
		/* high byte on odd address */
		retval = hi;
	}

	LOGMASKED(CS8900_DEBUG_LOAD, "read [$%02X] => $%02X.\n", io_address, retval);

	/* update _word_ value in register bank */
	cs8900_ioregs[reg_base] = lo;
	cs8900_ioregs[reg_base + 1] = hi;

	return retval;
}

/* ----- write byte to I/O range of VICE ----- */
void cs8900a_device::cs8900_store(u16 io_address, u8 var)
{
	u16 reg_base;
	u16 word_value;

	assert(cs8900_ioregs);
	assert(cs8900_packetpage);
	assert(io_address < 0x10);

	LOGMASKED(CS8900_DEBUG_STORE, "store [$%02X] <= $%02X.\n", io_address, (int)var);

	/* register base addr */
	reg_base = io_address & ~1;

	/* TX Register is special as it writes to TX buffer directly */
	if ((reg_base == CS8900_ADDR_RXTXDATA) || (reg_base == CS8900_ADDR_RXTXDATA2))
	{
		cs8900_write_tx_buffer(var, io_address & 1);
		return;
	}

	/* combine stored value with new written byte */
	if ((io_address & 1) == 0)
	{
		/* overwrite low byte */
		word_value = LOHI_WORD(var, cs8900_ioregs[reg_base + 1]);
	}
	else
	{
		/* overwrite high byte */
		word_value = LOHI_WORD(cs8900_ioregs[reg_base], var);
	}

	if (reg_base == CS8900_ADDR_PP_PTR)
	{
		/*  cv: we store the full package pointer in cs8900_packetpage_ptr variable.
		    this includes the mask area (0xf000) and the addr range (0x0fff).
		    we ensure that the bits 0x3000 are always set (as in real HW).
		    odd values of the pointer are valid and supported.
		    only register read and write have to be mapped to word boundary. */
		word_value |= 0x3000;
		cs8900_packetpage_ptr = word_value;
		LOGMASKED(CS8900_DEBUG_STORE, "set PP Ptr to $%04X.\n", cs8900_packetpage_ptr);
	}
	else
	{
		/* write a register */

		/*! \TODO: Find a reasonable default */
		u16 ppaddress = CS8900_PP_ADDR_PRODUCTID;

		/* now determine address of write in packet page */
		switch (reg_base)
		{
		case CS8900_ADDR_PP_DATA:
		case CS8900_ADDR_PP_DATA2:

			/* mask and align ppaddress from page pointer */
			ppaddress = cs8900_packetpage_ptr & (MAX_PACKETPAGE_ARRAY - 1);
			ppaddress &= ~1;

			/* auto increment pp ptr */
			cs8900_auto_incr_pp_ptr();
			break;

		case CS8900_ADDR_TXCMD:
			ppaddress = CS8900_PP_ADDR_TXCMD;
			break;

		case CS8900_ADDR_TXLENGTH:
			ppaddress = CS8900_PP_ADDR_TXLENGTH;
			break;

		case CS8900_ADDR_INTSTQUEUE:
			ppaddress = CS8900_PP_ADDR_SE_ISQ;
			break;

		case CS8900_ADDR_PP_PTR:
			break;

		default:
			/* invalid */
			assert(0);
			break;
		}

		LOGMASKED(CS8900_DEBUG_STORE, "before writing to PP Ptr: $%04X <= $%04X.\n",
					ppaddress, word_value);

		/* perform the write */
		cs8900_write_register(ppaddress, word_value);

		/* handle sideeffects */
		cs8900_sideeffects_write_pp(ppaddress, io_address & 1);

		/* update word value if it was changed in write register or by side effect */
		word_value = GET_PP_16(ppaddress);

		LOGMASKED(CS8900_DEBUG_STORE, "after writing to PP Ptr: $%04X <= $%04X.\n",
					ppaddress, word_value);
	}

	/* update IO registers */
	cs8900_ioregs[reg_base] = LO_BYTE(word_value);
	cs8900_ioregs[reg_base + 1] = HI_BYTE(word_value);
}

u8 cs8900a_device::read(u16 address)
{
	return cs8900_read(address);
}

void cs8900a_device::write(u16 address, u8 data)
{
	return cs8900_store(address, data);
}

int cs8900a_device::recv_start_cb(u8 *buf, int length)
{
	LOGMASKED(CS8900_DEBUG, "recv_start_cb(), %p len %d\n", buf, length);

	// Make an extra call to cs8900_should_accept() on the receive
	// callback to reduce the number of packets queueing up that
	// will just get rejected anyway.
	bool phashed = false;
	int phash_index = 0;
	bool pcorrect_mac = false;
	bool pbroadcast = false;
	bool pmulticast = false;

	if (cs8900_should_accept(buf, length, &phashed, &phash_index, &pcorrect_mac, &pbroadcast, &pmulticast))
	{
		std::vector<u8> frame;
		frame.assign(buf, buf + length);
		m_frame_queue.push(frame);

		// In order to ensure the frame queue doesn't grow without bound, evict from the front
		// when too many packets pile up.  This could happen due to a DOS or to the card not being
		// in use and stray packets enter.

		if (m_frame_queue.size() > MAX_FRAME_QUEUE_ENTRIES)
		{
			m_frame_queue.pop();
		}
	}
	return length;
}
