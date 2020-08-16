/*
 * tfe.c - TFE ("The final ethernet") emulation.
 *
 * Written by
 *  Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>
 *  Christian Vogelgsang <chris@vogelgsang.org>
 * 
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */




#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef DOS_TFE
#include <pcap.h>
#endif

#include "../defc.h"
#include "tfesupp.h"
#include "protos_tfe.h"
#include "types.h"


/**/

/* warn illegal behaviour */
/* #define TFE_DEBUG_WARN_REG 1 */   /* warn about invalid register accesses */
/* #define TFE_DEBUG_WARN_RXTX 1 */  /* warn about invalid rx or tx conditions */

/** #define TFE_DEBUG_INIT 1 **/
/** #define TFE_DEBUG_LOAD 1 **/           /* enable to see tfe port reads */
/** #define TFE_DEBUG_STORE 1 **/          /* enable to see tfe port writes */
/** #define TFE_DEBUG_REGISTERS 1 **/      /* enable to see CS8900a register I/O */
/** #define TFE_DEBUG_IGNORE_RXEVENT 1 **/ /* enable to ignore RXEVENT in DEBUG_REGISTERS */
/** #define TFE_DEBUG_RXTX_STATE 1 **/     /* enable to see tranceiver state changes */
/** #define TFE_DEBUG_RXTX_DATA 1 **/      /* enable to see data in/out flow */
/** #define TFE_DEBUG_FRAMES 1 **/         /* enable to see arch frame send/recv */

/* ------------------------------------------------------------------------- */
/*    variables needed                                                       */

/*
 This variable is used when we need to postpone the initialization
 because tfe_init() is not yet called
*/
static int should_activate = 0;
static int init_tfe_flag = 0;
typedef signed int log_t;
#define LOG_ERR     ((log_t)-1)
#define LOG_DEFAULT ((log_t)-2)

static log_t tfe_log = LOG_ERR;


/* status which received packages to accept 
   This is used in tfe_should_accept().
*/
static byte  tfe_ia_mac[6] = { 0,0,0,0,0,0 };

/* remember the value of the hash mask */
static DWORD tfe_hash_mask[2];

/* reveiver setup */
static word16 tfe_recv_control     = 0; /* copy of CC_RXCTL (contains all bits below) */
static int  tfe_recv_broadcast   = 0; /* broadcast */
static int  tfe_recv_mac         = 0; /* individual address (IA) */
static int  tfe_recv_multicast   = 0; /* multicast if address passes the hash filter */
static int  tfe_recv_correct     = 0; /* accept correct frames */
static int  tfe_recv_promiscuous = 0; /* promiscuous mode */
static int  tfe_recv_hashfilter  = 0; /* accept if IA passes the hash filter */

/* Flag: Can we even use TFE, or is the hardware not available? */
static int tfe_cannot_use = 0;

/* Flag: Do we have the TFE enabled?  */
int tfe_enabled = 0;

/* Flag: Do we use the "original" memory map or the memory map of the RR-Net? */
int tfe_as_rr_net = 0;

static char *tfe_interface = NULL;

/* TFE registers */
/* these are the 8 16-bit-ports for "I/O space configuration"
   (see 4.10 on page 75 of cs8900a-4.pdf, the cs8900a data sheet)

   REMARK: The TFE operatoes the cs8900a in IO space configuration, as
   it generates I/OW and I/OR signals.
*/
#define TFE_COUNT_IO_REGISTER 0x10 /* we have 16 I/O register */

static byte *tfe = NULL;
/*
    RW: RXTXDATA   = DE00/DE01
    RW: RXTXDATA2  = DE02/DE03 (for 32-bit-operation)
    -W: TXCMD      = DE04/DE05 (TxCMD, Transmit Command)   mapped to PP + 0144 (Reg. 9, Sec. 4.4, page 46)
    -W: TXLENGTH   = DE06/DE07 (TxLenght, Transmit Length) mapped to PP + 0146 
    R-: INTSTQUEUE = DE08/DE09 (Interrupt Status Queue)    mapped to PP + 0120 (ISQ, Sec. 5.1, page 78)
    RW: PP_PTR     = DE0A/DE0B (PacketPage Pointer)        (see. page 75p: Read -011.---- ----.----)
    RW: PP_DATA0   = DE0C/DE0D (PacketPage Data (Port 0))  
    RW: PP_DATA1   = DE0E/DE0F (PacketPage Data (Port 1))  (for 32 bit only)
*/

#define TFE_ADDR_RXTXDATA   0x00 /* RW */
#define TFE_ADDR_RXTXDATA2  0x02 /* RW 32 bit only! */
#define TFE_ADDR_TXCMD      0x04 /* -W Maps to PP+0144 */
#define TFE_ADDR_TXLENGTH   0x06 /* -W Maps to PP+0146 */
#define TFE_ADDR_INTSTQUEUE 0x08 /* R- Interrupt status queue, maps to PP + 0120 */
#define TFE_ADDR_PP_PTR     0x0a /* RW PacketPage Pointer */
#define TFE_ADDR_PP_DATA    0x0c /* RW PacketPage Data, Port 0 */
#define TFE_ADDR_PP_DATA2   0x0e /* RW PacketPage Data, Port 1 - 32 bit only */    

/* Makros for reading and writing the visible TFE register: */
#define GET_TFE_8(  _xxx_ ) \
    ( assert(_xxx_<TFE_COUNT_IO_REGISTER), \
      tfe[_xxx_]                           \
    )

#define SET_TFE_8( _xxx_, _val_ ) \
    do { \
        assert(_xxx_<TFE_COUNT_IO_REGISTER); \
        tfe[_xxx_  ] = (_val_     ) & 0xff; \
    } while (0) 

#define GET_TFE_16(  _xxx_ ) \
    ( assert(_xxx_<TFE_COUNT_IO_REGISTER), \
      tfe[_xxx_] | (tfe[_xxx_+1] << 8)     \
    )

#define SET_TFE_16( _xxx_, _val_ ) \
    do { \
        assert(_xxx_<TFE_COUNT_IO_REGISTER); \
        tfe[_xxx_  ] = (_val_     ) & 0xff; \
        tfe[_xxx_+1] = (_val_ >> 8) & 0xff; \
    } while (0) 

/* The PacketPage register */
/* note: The locations 0 to MAX_PACKETPAGE_ARRAY-1 are handled in this array. */

#define MAX_PACKETPAGE_ARRAY 0x1000 /* 4 KB */

static byte *tfe_packetpage = NULL;

static word16 tfe_packetpage_ptr = 0;

/* Makros for reading and writing the PacketPage register: */

#define GET_PP_8(  _xxx_ ) \
    (assert(_xxx_<MAX_PACKETPAGE_ARRAY), \
     tfe_packetpage[_xxx_] \
    )

#define GET_PP_16(  _xxx_ ) \
    ( assert(_xxx_<MAX_PACKETPAGE_ARRAY),   \
      assert((_xxx_ & 1) == 0 ),            \
      ((word16)tfe_packetpage[_xxx_]        ) \
    | ((word16)tfe_packetpage[_xxx_+1] <<  8) \
    )

#define GET_PP_32(  _xxx_ ) \
    ( assert(_xxx_<MAX_PACKETPAGE_ARRAY),     \
      assert((_xxx_ & 3) == 0 ),              \
      (((long)tfe_packetpage[_xxx_  ])      ) \
    | (((long)tfe_packetpage[_xxx_+1]) <<  8) \
    | (((long)tfe_packetpage[_xxx_+2]) << 16) \
    | (((long)tfe_packetpage[_xxx_+3]) << 24) \
    )

#define SET_PP_8( _xxx_, _val_ ) \
    do { \
        assert(_xxx_<MAX_PACKETPAGE_ARRAY);           \
        tfe_packetpage[_xxx_  ] = (_val_    ) & 0xFF; \
    } while (0) 

#define SET_PP_16( _xxx_, _val_ ) \
    do { \
        assert(_xxx_<MAX_PACKETPAGE_ARRAY);           \
        assert((_xxx_ & 1) == 0 ),                    \
        tfe_packetpage[_xxx_  ] = (_val_    ) & 0xFF; \
        tfe_packetpage[_xxx_+1] = (_val_>> 8) & 0xFF; \
    } while (0) 

#define SET_PP_32( _xxx_, _val_ ) \
    do { \
        assert(_xxx_<MAX_PACKETPAGE_ARRAY);           \
        assert((_xxx_ & 3) == 0 ),                    \
        tfe_packetpage[_xxx_  ] = (_val_    ) & 0xFF; \
        tfe_packetpage[_xxx_+1] = (_val_>> 8) & 0xFF; \
        tfe_packetpage[_xxx_+2] = (_val_>>16) & 0xFF; \
        tfe_packetpage[_xxx_+3] = (_val_>>24) & 0xFF; \
    } while (0) 


/* The packetpage register: see p. 39f */
#define TFE_PP_ADDR_PRODUCTID       0x0000 /*   R- - 4.3., p. 41 */
#define TFE_PP_ADDR_IOBASE          0x0020 /* i RW - 4.3., p. 41 - 4.7., p. 72 */
#define TFE_PP_ADDR_INTNO           0x0022 /* i RW - 3.2., p. 17 - 4.3., p. 41 */
#define TFE_PP_ADDR_DMA_CHAN        0x0024 /* i RW - 3.2., p. 17 - 4.3., p. 41 */
#define TFE_PP_ADDR_DMA_SOF         0x0026 /* ? R- - 4.3., p. 41 - 5.4., p. 89 */
#define TFE_PP_ADDR_DMA_FC          0x0028 /* ? R- - 4.3., p. 41, "Receive DMA" */
#define TFE_PP_ADDR_RXDMA_BC        0x002a /* ? R- - 4.3., p. 41 - 5.4., p. 89 */
#define TFE_PP_ADDR_MEMBASE         0x002c /* i RW - 4.3., p. 41 - 4.9., p. 73 */
#define TFE_PP_ADDR_BPROM_BASE      0x0030 /* i RW - 3.6., p. 24 - 4.3., p. 41 */
#define TFE_PP_ADDR_BPROM_MASK      0x0034 /* i RW - 3.6., p. 24 - 4.3., p. 41 */
/* 0x0038 - 0x003F: reserved */
#define TFE_PP_ADDR_EEPROM_CMD      0x0040 /* i RW - 3.5., p. 23 - 4.3., p. 41 */
#define TFE_PP_ADDR_EEPROM_DATA     0x0042 /* i RW - 3.5., p. 23 - 4.3., p. 41 */
/* 0x0044 - 0x004F: reserved */
#define TFE_PP_ADDR_REC_FRAME_BC    0x0050 /*   RW - 4.3., p. 41 - 5.2.9., p. 86 */
/* 0x0052 - 0x00FF: reserved */
#define TFE_PP_ADDR_CONF_CTRL       0x0100 /* - RW - 4.4., p. 46; see below */
#define TFE_PP_ADDR_STATUS_EVENT    0x0120 /* - R- - 4.4., p. 46; see below */
/* 0x0140 - 0x0143: reserved */
#define TFE_PP_ADDR_TXCMD           0x0144 /* # -W - 4.5., p. 70 - 5.7., p. 98 */
#define TFE_PP_ADDR_TXLENGTH        0x0146 /* # -W - 4.5., p. 70 - 5.7., p. 98 */
/* 0x0148 - 0x014F: reserved */
#define TFE_PP_ADDR_LOG_ADDR_FILTER 0x0150 /*   RW - 4.6., p. 71 - 5.3., p. 86 */
#define TFE_PP_ADDR_MAC_ADDR        0x0158 /* # RW - 4.6., p. 71 - 5.3., p. 86 */
/* 0x015E - 0x03FF: reserved */
#define TFE_PP_ADDR_RXSTATUS        0x0400 /*   R- - 4.7., p. 72 - 5.2., p. 78 */
#define TFE_PP_ADDR_RXLENGTH        0x0402 /*   R- - 4.7., p. 72 - 5.2., p. 78 */
#define TFE_PP_ADDR_RX_FRAMELOC     0x0404 /*   R- - 4.7., p. 72 - 5.2., p. 78 */
/* here, the received frame is stored */
#define TFE_PP_ADDR_TX_FRAMELOC     0x0A00 /*   -W - 4.7., p. 72 - 5.7., p. 98 */
/* here, the frame to transmit is stored */
#define TFE_PP_ADDR_END             0x1000 /* memory to TFE_PP_ADDR_END-1 is used */


/* TFE_PP_ADDR_CONF_CTRL is subdivided: */
#define TFE_PP_ADDR_CC_RXCFG        0x0102 /* # RW - 4.4.6.,  p. 52 - 0003 */
#define TFE_PP_ADDR_CC_RXCTL        0x0104 /* # RW - 4.4.8.,  p. 54 - 0005 */
#define TFE_PP_ADDR_CC_TXCFG        0x0106 /*   RW - 4.4.9.,  p. 55 - 0007 */
#define TFE_PP_ADDR_CC_TXCMD        0x0108 /*   R- - 4.4.11., p. 57 - 0009 */
#define TFE_PP_ADDR_CC_BUFCFG       0x010A /*   RW - 4.4.12., p. 58 - 000B */
#define TFE_PP_ADDR_CC_LINECTL      0x0112 /* # RW - 4.4.16., p. 62 - 0013 */
#define TFE_PP_ADDR_CC_SELFCTL      0x0114 /*   RW - 4.4.18., p. 64 - 0015 */
#define TFE_PP_ADDR_CC_BUSCTL       0x0116 /*   RW - 4.4.20., p. 66 - 0017 */
#define TFE_PP_ADDR_CC_TESTCTL      0x0118 /*   RW - 4.4.22., p. 68 - 0019 */

/* TFE_PP_ADDR_STATUS_EVENT is subdivided: */
#define TFE_PP_ADDR_SE_ISQ          0x0120 /*   R- - 4.4.5.,  p. 51 - 0000 */
#define TFE_PP_ADDR_SE_RXEVENT      0x0124 /* # R- - 4.4.7.,  p. 53 - 0004 */
#define TFE_PP_ADDR_SE_TXEVENT      0x0128 /*   R- - 4.4.10., p. 56 - 0008 */
#define TFE_PP_ADDR_SE_BUFEVENT     0x012C /*   R- - 4.4.13., p. 59 - 000C */
#define TFE_PP_ADDR_SE_RXMISS       0x0130 /*   R- - 4.4.14., p. 60 - 0010 */
#define TFE_PP_ADDR_SE_TXCOL        0x0132 /*   R- - 4.4.15., p. 61 - 0012 */
#define TFE_PP_ADDR_SE_LINEST       0x0134 /*   R- - 4.4.17., p. 63 - 0014 */
#define TFE_PP_ADDR_SE_SELFST       0x0136 /*   R- - 4.4.19., p. 65 - 0016 */
#define TFE_PP_ADDR_SE_BUSST        0x0138 /* # R- - 4.4.21., p. 67 - 0018 */
#define TFE_PP_ADDR_SE_TDR          0x013C /*   R- - 4.4.23., p. 69 - 001C */


/* ------------------------------------------------------------------------- */
/*    more variables needed                                                  */

static word16 tx_buffer = TFE_PP_ADDR_TX_FRAMELOC;
static word16 rx_buffer = TFE_PP_ADDR_RXSTATUS;

static word16 tx_count  = 0;
static word16 rx_count  = 0;
static word16 tx_length = 0;
static word16 rx_length = 0;

#define TFE_TX_IDLE         0
#define TFE_TX_GOT_CMD      1
#define TFE_TX_GOT_LEN      2
#define TFE_TX_READ_BUSST   3

#define TFE_RX_IDLE         0
#define TFE_RX_GOT_FRAME    1

/* tranceiver state */
static int tx_state = TFE_TX_IDLE;
static int rx_state = TFE_RX_IDLE;
static int tx_enabled = 0;
static int rx_enabled = 0;

static int rxevent_read_mask = 3; /* set if L and/or H byte was read in RXEVENT? */


/* ------------------------------------------------------------------------- */
/*    some parameter definitions                                             */

#define MAX_TXLENGTH 1518
#define MIN_TXLENGTH 4

#define MAX_RXLENGTH 1518
#define MIN_RXLENGTH 64


/* ------------------------------------------------------------------------- */
/*    debugging functions                                                    */

#ifdef TFE_DEBUG_FRAMES

static int TfeDebugMaxFrameLengthToDump = 150;

static char *debug_outbuffer(const int length, const unsigned char * const buffer)
{
#define MAXLEN_DEBUG 1600

    int i;
    static char outbuffer[MAXLEN_DEBUG*4+1];
    char *p = outbuffer;

    assert( TfeDebugMaxFrameLengthToDump <= MAXLEN_DEBUG );
    
    *p = 0;

    for (i=0; i<TfeDebugMaxFrameLengthToDump; i++) {
        if (i>=length)
            break;
    
        sprintf( p, "%02X%c", buffer[i], ((i+1)%16==0)?'*':(((i+1)%8==0)?'-':' '));
        p+=3;
    }

    return outbuffer;
}

#endif

/* ------------------------------------------------------------------------- */
/*    initialization and deinitialization functions                          */

static void tfe_set_tx_status(int ready,int error)
{
    word16 old_status = GET_PP_16(TFE_PP_ADDR_SE_BUSST);
    
    /* mask out TxBidErr and Rdy4TxNOW */
    word16 new_status = old_status & ~0x180;
    if(ready)
        new_status |= 0x100; /* set Rdy4TxNOW */
    if(error)
        new_status |= 0x080; /* set TxBidErr */
    
    if(new_status!=old_status) {
        SET_PP_16(TFE_PP_ADDR_SE_BUSST,new_status);
#ifdef TFE_DEBUG_RXTX_STATE
        log_message(tfe_log,"TX: set status Rdy4TxNOW=%d TxBidErr=%d",
                    ready,error);
#endif
    }
}

static void tfe_set_receiver(int enabled)
{
    rx_enabled = enabled;
    rx_state = TFE_RX_IDLE;

    rxevent_read_mask = 3; /* was L or H byte read in RXEVENT? */
}

static void tfe_set_transmitter(int enabled)
{
    tx_enabled = enabled;
    tx_state = TFE_TX_IDLE;
    
    tfe_set_tx_status(0,0);
}

void tfe_reset(void)
{
    if (tfe_enabled && !should_activate)
    {
        int i;
        
        assert( tfe );
        assert( tfe_packetpage );

        tfe_arch_pre_reset();

        /* initialize visible IO register and PacketPage registers */
        memset( tfe, 0, TFE_COUNT_IO_REGISTER );
        memset( tfe_packetpage, 0, MAX_PACKETPAGE_ARRAY );

        /* according to page 19 unless stated otherwise */
        SET_PP_32(TFE_PP_ADDR_PRODUCTID,      0x0900630E ); /* p.41: 0E630009 for Rev. D; reversed order! */
		SET_PP_16(TFE_PP_ADDR_IOBASE,         0x0300);
        SET_PP_16(TFE_PP_ADDR_INTNO,          0x0004); /* xxxx xxxx xxxx x100b */
        SET_PP_16(TFE_PP_ADDR_DMA_CHAN,       0x0003); /* xxxx xxxx xxxx xx11b */

#if 0 /* not needed since all memory is initialized with 0 */
        SET_PP_16(TFE_PP_ADDR_DMA_SOF,        0x0000);
        SET_PP_16(TFE_PP_ADDR_DMA_FC,         0x0000); /* x000h */
        SET_PP_16(TFE_PP_ADDR_RXDMA_BC,       0x0000);
        SET_PP_32(TFE_PP_ADDR_MEMBASE,        0x0000); /* xxx0 0000h */
        SET_PP_32(TFE_PP_ADDR_BPROM_BASE,     0x00000000); /* xxx0 0000h */
        SET_PP_32(TFE_PP_ADDR_BPROM_MASK,     0x00000000); /* xxx0 0000h */

        SET_PP_16(TFE_PP_ADDR_SE_ISQ,         0x0000); /* p. 51 */
#endif

        /* according to descriptions of the registers, see definitions of 
        TFE_PP_ADDR_CC_... and TFE_PP_ADDR_SE_... above! */

        SET_PP_16(TFE_PP_ADDR_CC_RXCFG,       0x0003);
        SET_PP_16(TFE_PP_ADDR_CC_RXCTL,       0x0005);
        SET_PP_16(TFE_PP_ADDR_CC_TXCFG,       0x0007);
        SET_PP_16(TFE_PP_ADDR_CC_TXCMD,       0x0009);
        SET_PP_16(TFE_PP_ADDR_CC_BUFCFG,      0x000B);
        SET_PP_16(TFE_PP_ADDR_CC_LINECTL,     0x0013);
        SET_PP_16(TFE_PP_ADDR_CC_SELFCTL,     0x0015);
        SET_PP_16(TFE_PP_ADDR_CC_BUSCTL,      0x0017);
        SET_PP_16(TFE_PP_ADDR_CC_TESTCTL,     0x0019);

        SET_PP_16(TFE_PP_ADDR_SE_ISQ,         0x0000);
        SET_PP_16(TFE_PP_ADDR_SE_RXEVENT,     0x0004);
        SET_PP_16(TFE_PP_ADDR_SE_TXEVENT,     0x0008);
        SET_PP_16(TFE_PP_ADDR_SE_BUFEVENT,    0x000C);
        SET_PP_16(TFE_PP_ADDR_SE_RXMISS,      0x0010);
        SET_PP_16(TFE_PP_ADDR_SE_TXCOL,       0x0012);
        SET_PP_16(TFE_PP_ADDR_SE_LINEST,      0x0014);
        SET_PP_16(TFE_PP_ADDR_SE_SELFST,      0x0016);
        SET_PP_16(TFE_PP_ADDR_SE_BUSST,       0x0018);
        SET_PP_16(TFE_PP_ADDR_SE_TDR,         0x001C);
        
        SET_PP_16(TFE_PP_ADDR_TXCMD,          0x0009);

        /* 4.4.19 Self Status Register, p. 65
           Important: set INITD (Bit 7) to signal device is ready */
        SET_PP_16(TFE_PP_ADDR_SE_SELFST,      0x0896);

        tfe_recv_control = GET_PP_16(TFE_PP_ADDR_CC_RXCTL);
        
        /* spec: mac address is undefined after reset.
           real HW: keeps the last set address. */
        for(i=0;i<6;i++)
            SET_PP_8(TFE_PP_ADDR_MAC_ADDR+i,tfe_ia_mac[i]);
        
        /* reset state */
        tfe_set_transmitter(0);
        tfe_set_receiver(0);

        tfe_arch_post_reset();

        printf("CS8900a rev.D reset\n");
    }
}

#ifdef DOS_TFE
static void set_standard_tfe_interface(void)
{
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    dev = pcap_lookupdev(errbuf);
    util_string_set(&tfe_interface, dev);
}
#endif

static
int tfe_activate_i(void)
{
    assert( tfe == NULL );
    assert( tfe_packetpage == NULL );

#ifdef TFE_DEBUG
    log_message( tfe_log, "tfe_activate_i()." );
#endif

    /* allocate memory for visible IO register */
    tfe = (byte *)lib_malloc( TFE_COUNT_IO_REGISTER );
    if (tfe==NULL)
    {
#ifdef TFE_DEBUG_INIT
        log_message(tfe_log, "tfe_activate_i: Allocating tfe failed.");
#endif
        tfe_enabled = 0;
        return 0;
    }

    /* allocate memory for PacketPage register */
    tfe_packetpage = (byte *)lib_malloc( MAX_PACKETPAGE_ARRAY );
    if (tfe_packetpage==NULL)
    {
#ifdef TFE_DEBUG_INIT
        log_message(tfe_log, "tfe_activate: Allocating tfe_packetpage failed.");
#endif
        lib_free(tfe);
        tfe=NULL;
        tfe_enabled = 0;
        return 0;
    }

#ifdef TFE_DEBUG_INIT
    log_message(tfe_log, "tfe_activate: Allocated memory successfully.");
    log_message(tfe_log, "\ttfe at $%08X, tfe_packetpage at $%08X", tfe, tfe_packetpage );
#endif

#ifdef DOS_TFE
    set_standard_tfe_interface();
#endif

    if (!tfe_arch_activate(tfe_interface)) {
        lib_free(tfe_packetpage);
        lib_free(tfe);
        tfe=NULL;
        tfe_packetpage=NULL;
        tfe_enabled = 0;
        tfe_cannot_use = 1;
        return 0;
    }

    /* virtually reset the LAN chip */
    tfe_reset();

    return 0;
}

static
int tfe_deactivate_i(void)
{
#ifdef TFE_DEBUG
    log_message( tfe_log, "tfe_deactivate_i()." );
#endif

    assert(tfe && tfe_packetpage);

    tfe_arch_deactivate();

    lib_free(tfe);
    tfe = NULL;
    lib_free(tfe_packetpage);
    tfe_packetpage = NULL;
    return 0;
}

static
int tfe_activate(void) {
#ifdef TFE_DEBUG
    log_message( tfe_log, "tfe_activate()." );
#endif

    if (init_tfe_flag) {
        return tfe_activate_i();
    }
    else {
        should_activate = 1;
    }
    return 0;
}

static
int tfe_deactivate(void) {
#ifdef TFE_DEBUG
    log_message( tfe_log, "tfe_deactivate()." );
#endif

    if (should_activate)
        should_activate = 0;
    else {
        if (tfe_log != LOG_ERR)
            return tfe_deactivate_i();
    }

    return 0;
}

void tfe_init(void)
{
	tfe_enabled = 1;
	init_tfe_flag = 1;
	should_activate = 1;
    if (!tfe_arch_init()) {
        tfe_enabled = 0;
        tfe_cannot_use = 1;
    }

    if (should_activate) {
        should_activate = 0;
        if (tfe_activate() < 0) {
            tfe_enabled = 0;
            tfe_cannot_use = 1;
        }
    }
}

void tfe_shutdown(void)
{
    assert( (tfe && tfe_packetpage) || (!tfe && !tfe_packetpage));

    if (tfe)
        tfe_deactivate();

    if (tfe_interface != NULL)
        lib_free(tfe_interface);
}


/* ------------------------------------------------------------------------- */
/*    reading and writing TFE register functions                             */

/*
These registers are currently fully or partially supported:

TFE_PP_ADDR_CC_RXCFG        0x0102 * # RW - 4.4.6.,  p. 52 - 0003 *
TFE_PP_ADDR_CC_RXCTL        0x0104 * # RW - 4.4.8.,  p. 54 - 0005 *
TFE_PP_ADDR_CC_LINECTL      0x0112 * # RW - 4.4.16., p. 62 - 0013 *
TFE_PP_ADDR_SE_RXEVENT      0x0124 * # R- - 4.4.7.,  p. 53 - 0004 *
TFE_PP_ADDR_SE_BUSST        0x0138 * # R- - 4.4.21., p. 67 - 0018 *
TFE_PP_ADDR_TXCMD           0x0144 * # -W - 4.5., p. 70 - 5.7., p. 98 *
TFE_PP_ADDR_TXLENGTH        0x0146 * # -W - 4.5., p. 70 - 5.7., p. 98 *
TFE_PP_ADDR_MAC_ADDR        0x0158 * # RW - 4.6., p. 71 - 5.3., p. 86 *
                            0x015a
                            0x015c
*/


#ifdef TFE_DEBUG_FRAMES
    #define return( _x_ ) \
    { \
        int retval = _x_; \
        \
        log_message(tfe_log, "%s correct_mac=%u, broadcast=%u, multicast=%u, hashed=%u, hash_index=%u", (retval? "+++ ACCEPTED":"--- rejected"), *pcorrect_mac, *pbroadcast, *pmulticast, *phashed, *phash_index); \
        \
        return retval; \
    }

#endif
/*
 This is a helper for tfe_receive() to determine if the received frame should be accepted
 according to the settings.

 This function is even allowed to be called in tfearch.c from tfe_arch_receive() if 
 necessary, which is the reason why its prototype is included here in tfearch.h.
*/
int tfe_should_accept(unsigned char *buffer, int length, int *phashed, int *phash_index, 
                      int *pcorrect_mac, int *pbroadcast, int *pmulticast) 
{
    int hashreg; /* Hash Register (for hash computation) */

    assert(length>=6); /* we need at least 6 octets since the DA has this length */

    /* first of all, delete any status */
    *phashed      = 0;
    *phash_index  = 0;
    *pcorrect_mac = 0;
    *pbroadcast   = 0;
    *pmulticast   = 0;

#ifdef TFE_DEBUG_FRAMES
    log_message(tfe_log, "tfe_should_accept called with %02X:%02X:%02X:%02X:%02X:%02X, length=%4u and buffer %s", 
        tfe_ia_mac[0], tfe_ia_mac[1], tfe_ia_mac[2],
        tfe_ia_mac[3], tfe_ia_mac[4], tfe_ia_mac[5],
        length,
        debug_outbuffer(length, buffer)
        );
#endif


    if (   buffer[0]==tfe_ia_mac[0]
        && buffer[1]==tfe_ia_mac[1]
        && buffer[2]==tfe_ia_mac[2]
        && buffer[3]==tfe_ia_mac[3]
        && buffer[4]==tfe_ia_mac[4]
        && buffer[5]==tfe_ia_mac[5]
       ) {
        /* this is our individual address (IA) */

        *pcorrect_mac = 1;

        /* if we don't want "correct MAC", we might have the chance
         * that this address fits the hash index 
         */
        if (tfe_recv_mac || tfe_recv_promiscuous) 
            return(1);
    }

    if (   buffer[0]==0xFF
        && buffer[1]==0xFF
        && buffer[2]==0xFF
        && buffer[3]==0xFF
        && buffer[4]==0xFF
        && buffer[5]==0xFF
       ) {
        /* this is a broadcast address */
        *pbroadcast = 1;

        /* broadcasts cannot be accepted by the hash filter */
            return((tfe_recv_broadcast || tfe_recv_promiscuous) ? 1 : 0);
    }

    /* now check if DA passes the hash filter */
    hashreg = (~crc32_buf((char *)buffer,6) >> 26) & 0x3F;

    *phashed = (tfe_hash_mask[(hashreg>=32)?1:0] & (1 << (hashreg&0x1F))) ? 1 : 0;
    if (*phashed) {
        *phash_index = hashreg;

        if (buffer[0] & 0x80) {
            /* we have a multicast address */
            *pmulticast = 1;

            /* if the multicast address fits into the hash filter, 
             * the hashed bit has to be clear 
             */
            *phashed = 0;

            return((tfe_recv_multicast || tfe_recv_promiscuous) ? 1 : 0);
        }
        return((tfe_recv_hashfilter || tfe_recv_promiscuous) ? 1 : 0);
    }
       
    return(tfe_recv_promiscuous ? 1 : 0);
}

#ifdef TFE_DEBUG_FRAMES
    #undef return
#endif

static 
word16 tfe_receive(void)
{
    word16 ret_val = 0x0004;

    byte buffer[MAX_RXLENGTH];

    int  len;
    int  hashed;
    int  hash_index;
    int  rx_ok;
    int  correct_mac;
    int  broadcast;
    int  multicast = 0;
    int  crc_error;

    int  newframe;

    int  ready;

    do {
        len = MAX_RXLENGTH;

        ready = 1 ; /* assume we will find a good frame */

        newframe = tfe_arch_receive(
            buffer,       /* where to store a frame */
            &len,         /* length of received frame */
            &hashed,      /* set if the dest. address is accepted by the hash filter */
            &hash_index,  /* hash table index if hashed == TRUE */   
            &rx_ok,       /* set if good CRC and valid length */
            &correct_mac, /* set if dest. address is exactly our IA */
            &broadcast,   /* set if dest. address is a broadcast address */
            &crc_error    /* set if received frame had a CRC error */
            );

        assert((len&1) == 0); /* length has to be even! */

        if (newframe) {
            if (hashed || correct_mac || broadcast) {
                /* we already know the type of frame: Trust it! */
#ifdef TFE_DEBUG_FRAMES
                log_message( tfe_log, "+++ tfe_receive(): *** hashed=%u, correct_mac=%u, "
                    "broadcast=%u", hashed, correct_mac, broadcast);
#endif
            }
            else {
                /* determine ourself the type of frame */
                if (!tfe_should_accept(buffer, 
                    len, &hashed, &hash_index, &correct_mac, &broadcast, &multicast)) {

                    /* if we should not accept this frame, just do nothing
                     * now, look for another one */
                    ready = 0; /* try another frame */
                    continue;
                }
            }


            /* we did receive a frame, return that status */
            ret_val |= rx_ok     ? 0x0100 : 0;
            ret_val |= multicast ? 0x0200 : 0;

            if (!multicast) {
                ret_val |= hashed ? 0x0040 : 0;
            }

            if (hashed && rx_ok) {
                /* we have the 2nd, special format with hash index: */
                assert(hash_index < 64);
                ret_val |= hash_index << 9;
            }
            else {
                /* we have the regular format */
                ret_val |= correct_mac        ? 0x0400 : 0;
                ret_val |= broadcast          ? 0x0800 : 0;
                ret_val |= crc_error          ? 0x1000 : 0;
                ret_val |= (len<MIN_RXLENGTH) ? 0x2000 : 0;
                ret_val |= (len>MAX_RXLENGTH) ? 0x4000 : 0;
            }

            /* discard any octets that are beyond the MAX_RXLEN */
            if (len>MAX_RXLENGTH) {
                len = MAX_RXLENGTH;
            }

            if (rx_ok) {
                int i;

                /* set relevant parts of the PP area to correct values */
                SET_PP_16(TFE_PP_ADDR_RXLENGTH, len);

                for (i=0;i<len; i++) {
                    SET_PP_8(TFE_PP_ADDR_RX_FRAMELOC+i, buffer[i]);
                }

                /* set rx_buffer to where start reading *
                 * According to 4.10.9 (pp. 76-77), we start with RxStatus and RxLength!
                 */
                rx_buffer = TFE_PP_ADDR_RXSTATUS;
                rx_length = len;
                rx_count  = 0;
#ifdef TFE_DEBUG_WARN_RXTX
                if(rx_state!=TFE_RX_IDLE) {
                    log_message(tfe_log,"WARNING! New frame overwrites pending one!");
                }
#endif
                rx_state  = TFE_RX_GOT_FRAME;
#ifdef TFE_DEBUG_RXTX_STATE
                log_message(tfe_log,"RX: recvd frame (length=%04x,status=%04x)",
                            rx_length,ret_val);
#endif
            }
        }
    } while (!ready);

#ifdef TFE_DEBUG_FRAMES
    if (ret_val != 0x0004)
        log_message( tfe_log, "+++ tfe_receive(): ret_val=%04X", ret_val);
#endif

    return ret_val;
}

/* ------------------------------------------------------------------------- */
/* TX/RX buffer handling */

static void tfe_write_tx_buffer(byte value,int odd_address)
{
    /* write tx data only if valid buffer is ready */
    if(tx_state != TFE_TX_READ_BUSST) {
#ifdef TFE_DEBUG_WARN_RXTX
        log_message(tfe_log, "WARNING! Ignoring TX Write without correct Transmit Condition! (odd=%d,value=%02x)",
                    odd_address,value);
#endif
        /* ensure correct tx state (needed if transmit < 4 was started) */
        tfe_set_tx_status(0,0);
    } else {
#ifdef TFE_DEBUG_RXTX_STATE
        if(tx_count==0) {
            log_message(tfe_log,"TX: write frame (length=%04x)",tx_length);
        }
#endif

        /* always write LH, LH... to tx buffer */
        word16 addr = tx_buffer;
        if(odd_address) {
            addr++;
            tx_buffer += 2;
        }
        tx_count++;            
        SET_PP_8(addr, value);
        
#ifdef TFE_DEBUG_RXTX_DATA
        log_message(tfe_log, "TX: %04x/%04x: %02x (buffer=%04x,odd=%d)",
                    tx_count,tx_length,value,addr,odd_address);
#endif                

        /* full frame transmitted? */
        if(tx_count==tx_length) {
#ifdef TFE_DEBUG_FRAMES
            log_message(tfe_log, "tfe_arch_transmit() called with:                 "
                "length=%4u and buffer %s", tx_length,
                debug_outbuffer(tx_length, &tfe_packetpage[TFE_PP_ADDR_TX_FRAMELOC])
                );
#endif

            if(!tx_enabled) {
#ifdef TFE_DEBUG_WARN_RXTX
                log_message(tfe_log,"WARNING! Can't transmit frame (Transmitter is not enabled)!");
#endif
            } else {
                /* send frame */
                word16 txcmd = GET_PP_16(TFE_PP_ADDR_CC_TXCMD);
                tfe_arch_transmit(
                    txcmd & 0x0100 ? 1 : 0,   /* FORCE: Delete waiting frames in transmit buffer */
                    txcmd & 0x0200 ? 1 : 0,   /* ONECOLL: Terminate after just one collision */
                    txcmd & 0x1000 ? 1 : 0,   /* INHIBITCRC: Do not append CRC to the transmission */
                    txcmd & 0x2000 ? 1 : 0,   /* TXPADDIS: Disable padding to 60/64 octets */
                    tx_length,
                    &tfe_packetpage[TFE_PP_ADDR_TX_FRAMELOC]         
                    );
            }

            /* reset transmitter state */
            tx_state = TFE_TX_IDLE;
    
#ifdef TFE_DEBUG_RXTX_STATE
            log_message(tfe_log, "TX: sent  frame (length=%04x)",tx_length);
#endif

            /* reset tx status */
            tfe_set_tx_status(0,0);
        }
    }
}

static byte tfe_read_rx_buffer(int odd_address)
{
    if(rx_state != TFE_RX_GOT_FRAME) {
#ifdef TFE_DEBUG_WARN_RXTX
        log_message(tfe_log, "WARNING! RX Read without frame available! (odd=%d)",
                    odd_address);
#endif
        /* always reads zero on HW */
        return 0;
    } else {
        /*
         According to the CS8900 spec, the handling is the following:
         first read H, then L (RX_STATUS), then H, then L (RX_LENGTH).
         Inside the RX frame data, we always get L then H, until the end is reached.

                                     even    odd
         TFE_PP_ADDR_RXSTATUS:         -     proceed
         TFE_PP_ADDR_RXLENGTH:         -     proceed 
         TFE_PP_ADDR_RX_FRAMELOC:      -       -
         TFE_PP_ADDR_RX_FRAMELOC+2: proceed    -
         TFE_PP_ADDR_RX_FRAMELOC+4: proceed    -

         */
        word16 addr = odd_address ? 1:0; 
        byte value;
        /* read RXSTATUS or RX_LENGTH */
        if(rx_count<4) {
            addr  += rx_buffer;
            value  = GET_PP_8(addr);
            rx_count++;

            /* incr after RXSTATUS or RX_LENGTH even (L) read */
            if(!odd_address)
                rx_buffer += 2;
        } 
        /* read frame data */
        else {
            /* incr before frame read (but not in first word) */
            if((rx_count>=6) && (!odd_address))
                rx_buffer += 2;

            addr += rx_buffer;
            value = GET_PP_8(addr);
            rx_count++;
        }
        
#ifdef TFE_DEBUG_RXTX_DATA
        log_message(tfe_log,"RX: %04x/%04x: %02x (buffer=%04x,odd=%d)",
                    rx_count,rx_length+4,value,addr,odd_address);
#endif

        /* check frame end */
        if(rx_count>=rx_length+4) {
            /* reset receiver state to idle */
            rx_state = TFE_RX_IDLE;
#ifdef TFE_DEBUG_RXTX_STATE
            log_message(tfe_log,"RX: read  frame (length=%04x)",rx_length);
#endif
        }        
        return value;
    }
}

/* ------------------------------------------------------------------------- */
/* handle side-effects of read and write operations */

/*
 This is called *after* the relevant octets are written
*/
static
void tfe_sideeffects_write_pp(word16 ppaddress, int odd_address)
{
//    const char *on_off[2] = { "on","off" };
//#define on_off_str(x) ((x) ? on_off[0] : on_off[1])
    word16 content = GET_PP_16( ppaddress );

    assert((ppaddress & 1) == 0);

    switch (ppaddress)
    {
    case TFE_PP_ADDR_CC_RXCFG:
        /* Skip_1 Flag: remove current (partial) tx frame and restore state */
        if (content & 0x40) {
            
/*          tfe_arch_receive_remove_committed_frame(); */            
            
            /* restore tx state */ 
            if(tx_state!=TFE_TX_IDLE) {
                tx_state = TFE_TX_IDLE;
#ifdef TFE_DEBUG_RXTX_STATE
                log_message(tfe_log,"TX: skipping current frame");
#endif
            }

            /* reset transmitter */
            tfe_set_transmitter(tx_enabled);
                         
            /* this is an "act once" bit, thus restore it to zero. */
            content &= ~0x40; 
            SET_PP_16( ppaddress, content );
        }
        break;

    case TFE_PP_ADDR_CC_RXCTL:
        if(tfe_recv_control!=content) {
            tfe_recv_broadcast   = content & 0x0800; /* broadcast */
            tfe_recv_mac         = content & 0x0400; /* individual address (IA) */
            tfe_recv_multicast   = content & 0x0200; /* multicast if address passes the hash filter */
            tfe_recv_correct     = content & 0x0100; /* accept correct frames */
            tfe_recv_promiscuous = content & 0x0080; /* promiscuous mode */
            tfe_recv_hashfilter  = content & 0x0040; /* accept if IA passes the hash filter */
            tfe_recv_control     = content;
        
            /*log_message(tfe_log,"setup receiver: broadcast=%s mac=%s multicast=%s correct=%s promiscuous=%s hashfilter=%s",
                        on_off_str(tfe_recv_broadcast),
                        on_off_str(tfe_recv_mac),
                        on_off_str(tfe_recv_multicast),
                        on_off_str(tfe_recv_correct),
                        on_off_str(tfe_recv_promiscuous),
                        on_off_str(tfe_recv_hashfilter));*/

            tfe_arch_recv_ctl( tfe_recv_broadcast,
                               tfe_recv_mac,
                               tfe_recv_multicast,
                               tfe_recv_correct,
                               tfe_recv_promiscuous,
                               tfe_recv_hashfilter
                             );
        }
        break;

    case TFE_PP_ADDR_CC_LINECTL:
        {
            int enable_tx = (content & 0x0080) == 0x0080;
            int enable_rx = (content & 0x0040) == 0x0040;
    
            if((enable_tx!=tx_enabled)||(enable_rx!=rx_enabled)) {
                tfe_arch_line_ctl(enable_tx,enable_rx);
                tfe_set_transmitter(enable_tx);
                tfe_set_receiver(enable_rx);
            
                /*log_message(tfe_log,"line control: transmitter=%s receiver=%s",
                            on_off_str(enable_tx),
                            on_off_str(enable_rx));*/
            }
        }
        break;

    case TFE_PP_ADDR_CC_SELFCTL:
        {
            /* reset chip? */
            if((content & 0x40)==0x40) {
                tfe_reset();
            }
        }
        break;
        
    case TFE_PP_ADDR_TXCMD:
        {
            if(odd_address) {
                word16 txcommand = GET_PP_16(TFE_PP_ADDR_TXCMD);
                
                /* already transmitting? */
                if(tx_state == TFE_TX_READ_BUSST) {
#ifdef TFE_DEBUG_WARN_RXTX
                    log_message(tfe_log, "WARNING! Early abort of transmitted frame");
#endif
                }
                
                /* The transmit status command gets the last transmit command */
                SET_PP_16(TFE_PP_ADDR_CC_TXCMD, txcommand);
                
                /* set transmit state */
                tx_state = TFE_TX_GOT_CMD;
                tfe_set_tx_status(0,0);
                
#ifdef TFE_DEBUG_RXTX_STATE
                log_message(tfe_log, "TX: COMMAND accepted (%04x)",txcommand);
#endif
            }
        }
        break;

    case TFE_PP_ADDR_TXLENGTH:
        {
            if(odd_address && (tx_state == TFE_TX_GOT_CMD)) {
                word16 txlength = GET_PP_16(TFE_PP_ADDR_TXLENGTH);
                word16 txcommand = GET_PP_16(TFE_PP_ADDR_CC_TXCMD);
                
                if(txlength<4) {
                    /* frame to short */
#ifdef TFE_DEBUG_RXTX_STATE
                    log_message(tfe_log, "TX: LENGTH rejected - too short! (%04x)",txlength);
#endif
                    /* mask space available but do not commit */
                    tx_state = TFE_TX_IDLE;
                    tfe_set_tx_status(1,0);
                }
                else if (    (txlength>MAX_TXLENGTH)
                         || ((txlength>MAX_TXLENGTH-4) && (!(txcommand&0x1000)))
                        ) {
                    tx_state = TFE_TX_IDLE;
#ifdef TFE_DEBUG_RXTX_STATE
                    log_message(tfe_log, "TX: LENGTH rejected - too long! (%04x)",txlength);
#endif
                    /* txlength too big, mark an error */
                    tfe_set_tx_status(0,1);
                }
                else {
                    /* make sure we put the octets to transmit at the right place */
                    tx_buffer = TFE_PP_ADDR_TX_FRAMELOC;
                    tx_count  = 0;
                    tx_length = txlength;
                    tx_state  = TFE_TX_GOT_LEN;

#ifdef TFE_DEBUG_RXTX_STATE
                    log_message(tfe_log, "TX: LENGTH accepted (%04x)",txlength);
#endif
                    /* all right, signal that we're ready for the next frame */
                    tfe_set_tx_status(1,0);
                }
            }
        }
        break;

    case TFE_PP_ADDR_LOG_ADDR_FILTER:
    case TFE_PP_ADDR_LOG_ADDR_FILTER+2:
    case TFE_PP_ADDR_LOG_ADDR_FILTER+4:
    case TFE_PP_ADDR_LOG_ADDR_FILTER+6:
        {
            unsigned int pos = 8 * (ppaddress - TFE_PP_ADDR_LOG_ADDR_FILTER + odd_address);
            DWORD *p = (pos < 32) ? &tfe_hash_mask[0] : &tfe_hash_mask[1];

            *p &= ~(0xFF << pos); /* clear out relevant bits */
            *p |= GET_PP_8(ppaddress+odd_address) << pos;

            // tfe_arch_set_hashfilter(tfe_hash_mask);

#if 0
            if(odd_address && (ppaddress == TFE_PP_ADDR_LOG_ADDR_FILTER+6))
                log_message(tfe_log,"set hash filter: %02x:%02x:%02x:%02x:%02x:%02x",
                            tfe_hash_mask[0],
                            tfe_hash_mask[1],
                            tfe_hash_mask[2],
                            tfe_hash_mask[3],
                            tfe_hash_mask[4],
                            tfe_hash_mask[5]);
#endif
        }
        break;

    case TFE_PP_ADDR_MAC_ADDR:
    case TFE_PP_ADDR_MAC_ADDR+2:
    case TFE_PP_ADDR_MAC_ADDR+4:
        /* the MAC address has been changed */
        tfe_ia_mac[ppaddress-TFE_PP_ADDR_MAC_ADDR+odd_address] = 
            GET_PP_8(ppaddress+odd_address);
        tfe_arch_set_mac(tfe_ia_mac);
        
        if(odd_address && (ppaddress == TFE_PP_ADDR_MAC_ADDR+4))
            /*log_message(tfe_log,"set MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
                        tfe_ia_mac[0],tfe_ia_mac[1],tfe_ia_mac[2],
                        tfe_ia_mac[3],tfe_ia_mac[4],tfe_ia_mac[5]);*/
        break;
    }
#undef on_off_str
}

/*
 This is called *before* the relevant octets are read
*/
static
void tfe_sideeffects_read_pp(word16 ppaddress,int odd_address)
{
    switch (ppaddress)
    {
    case TFE_PP_ADDR_SE_RXEVENT:
        /* reading this before all octets of the frame are read
           performs an "implied skip" */
        {
            int access_mask = (odd_address) ? 1 : 2;
            
            /* update the status register only if the full word of the last
               status was read! unfortunately different access patterns are
               possible: either the status is read LH, LH, LH...
               or HL, HL, HL, or even L, L, L or H, H, H */
            if((access_mask & rxevent_read_mask)!=0) {
                /* receiver is not enabled */
                if(!rx_enabled) {
#ifdef TFE_DEBUG_WARN_RXTX
                    log_message(tfe_log,"WARNING! Can't receive any frame (Receiver is not enabled)!");
#endif
                } else {
                    /* perform frame reception */
                    word16 ret_val = tfe_receive();

                    /* RXSTATUS and RXEVENT are the same, except that RXSTATUS buffers
                       the old value while RXEVENT sets a new value whenever it is called
                    */
                    SET_PP_16(TFE_PP_ADDR_RXSTATUS,   ret_val); 
                    SET_PP_16(TFE_PP_ADDR_SE_RXEVENT, ret_val); 
                }
                
                /* reset read mask of (possible) other access */
                rxevent_read_mask = access_mask;
            } else {
                /* add access bit to mask */
                rxevent_read_mask |= access_mask;
            }
        }

        break;

    case TFE_PP_ADDR_SE_BUSST:
        if(odd_address) {
            /* read busst before transmit condition is fullfilled */
            if(tx_state == TFE_TX_GOT_LEN) {
                word16 bus_status = GET_PP_16(TFE_PP_ADDR_SE_BUSST);
                /* check Rdy4TXNow flag */
                if((bus_status & 0x100) == 0x100) {
                    tx_state = TFE_TX_READ_BUSST;
#ifdef TFE_DEBUG_RXTX_STATE
                    log_message(tfe_log, "TX: Ready4TXNow set! (%04x)",
                                bus_status);
#endif
                } else {
#ifdef TFE_DEBUG_RXTX_STATE
                    log_message(tfe_log, "TX: waiting for Ready4TXNow! (%04x)",
                                bus_status);
#endif                    
                }
            }
        }
        break;
    }
}

/* ------------------------------------------------------------------------- */
/* read/write from packet page register */

/* read a register from packet page */
static word16 tfe_read_register(word16 ppaddress)
{
  word16 value = GET_PP_16(ppaddress);
  
  /* --- check the register address --- */
  if(ppaddress<0x100) {
    /* reserved range reads 0x0300 on real HW */
    if((ppaddress>=0x0004)&&(ppaddress<0x0020)) {
      return 0x0300;
    }
  }
  
  /* --- read control register range --- */
  else if(ppaddress<0x120) {
    word16 regNum = ppaddress - 0x100;
    regNum &= ~1;
    regNum ++;
#ifdef TFE_DEBUG_REGISTERS
    log_message(tfe_log,
                "Read  Control Register %04x: %04x (reg=%02x)",
                ppaddress,value,regNum);
#endif
 
    /* reserved register? */
    if((regNum==0x01)||(regNum==0x11)||(regNum>0x19)) {
#ifdef TFE_DEBUG_WARN_REG
      log_message(tfe_log,
                  "WARNING! Read reserved Control Register %04x (reg=%02x)",
                  ppaddress,regNum);
#endif
      /* real HW returns 0x0300 in reserved register range */
      return 0x0300;
    }

    /* make sure interal address is always valid */
    assert((value&0x3f) == regNum); 
  }
  
  /* --- read status register range --- */
  else if(ppaddress<0x140) {
    word16 regNum = ppaddress - 0x120;
    regNum &= ~1;
#ifdef TFE_DEBUG_REGISTERS
#ifdef TFE_DEBUG_IGNORE_RXEVENT
    if(regNum!=4) // do not show RXEVENT
#endif
    log_message(tfe_log,
                "Read  Status  Register %04x: %04x (reg=%02x)",
                ppaddress,value,regNum);
#endif

    /* reserved register? */
    if((regNum==0x02)||(regNum==0x06)||(regNum==0x0a)||
       (regNum==0x0e)||(regNum==0x1a)||(regNum==0x1e)) {
#ifdef TFE_DEBUG_WARN_REG
      log_message(tfe_log,
                  "WARNING! Read reserved Status Register %04x (reg=%02x)",
                  ppaddress,regNum);
#endif
      /* real HW returns 0x0300 in reserved register range */
      return 0x0300;
    }

    /* make sure interal address is always valid */
    assert((value&0x3f) == regNum);
  }
  
  /* --- read transmit register range --- */
  else if(ppaddress<0x150) {
    if(ppaddress==0x144) {
      /* make sure interal address is always valid */
      assert((value&0x3f) == 0x09);
#ifdef TFE_DEBUG_REGISTERS
      log_message(tfe_log,
                  "Read  TX Cmd  Register %04x: %04x",
                  ppaddress,value);
#endif
    }
    else if(ppaddress==0x146) {
#ifdef TFE_DEBUG_REGISTERS
      log_message(tfe_log,
                  "Read  TX Len  Register %04x: %04x",
                  ppaddress,value);
#endif
    }
    /* reserved range */
    else {
#ifdef TFE_DEBUG_WARN_REG
      log_message(tfe_log,
                  "WARNING! Read reserved Initiate Transmit Register %04x",
                  ppaddress);
#endif
      /* real HW returns 0x0300 in reserved register range */
      return 0x0300;
    }
  }
  
  /* --- read address filter register range --- */
  else if(ppaddress<0x160) {
    /* reserved range */
    if(ppaddress>=0x15e) {
#ifdef TFE_DEBUG_WARN_REG
      log_message(tfe_log,
                  "WARNING! Read reserved Address Filter Register %04x",
                  ppaddress);
#endif
      /* real HW returns 0x0300 in reserved register range */
      return 0x0300;
    }
  }
 
  /* --- reserved range below 0x400 ---
     returns 0x300 on real HW 
  */
  else if(ppaddress<0x400) {
#ifdef TFE_DEBUG_WARN_REG
    log_message(tfe_log,
                "WARNING! Read reserved Register %04x",
                ppaddress);
#endif
    return 0x0300;
  }
  
  /* --- range from 0x400 .. 0x9ff --- RX Frame */
  else if(ppaddress<0xa00) {
#ifdef TFE_DEBUG_WARN_REG
    log_message(tfe_log,
                "WARNING! Read from RX Buffer Range %04x",
                ppaddress);
#endif
    return 0x0000;
  }
  
  /* --- range from 0xa00 .. 0xfff --- TX Frame */
  else {
#ifdef TFE_DEBUG_WARN_REG
    log_message(tfe_log,
                "WARNING! Read from TX Buffer Range %04x",
                ppaddress);
#endif
    return 0x0000;
  }
  
  /* actually read from pp memory */
  return value;
}

void tfe_write_register(word16 ppaddress,word16 value)
{
  /* --- write bus interface register range --- */
  if(ppaddress<0x100) {
    int ignore = 0;
    if(ppaddress<0x20) {
      ignore = 1;
    } else if((ppaddress>=0x26)&&(ppaddress<0x2c)) {
      ignore = 1;
    } else if(ppaddress==0x38) {
      ignore = 1;
    } else if(ppaddress>=0x44) {
      ignore = 1;
    }
    if(ignore) {
#ifdef TFE_DEBUG_WARN_REG
      log_message(tfe_log,
                  "WARNING! Ignoring write to read only/reserved Bus Interface Register %04x",
                  ppaddress);
#endif
      return;
    }
  }
  
  /* --- write to control register range --- */
  else if(ppaddress<0x120) {
    word16 regNum = ppaddress - 0x100;
    regNum &= ~1;
    regNum += 1;
    /* validate internal address */
    if((value&0x3f) != regNum) {
      /* fix internal address */
      value &= ~0x3f;
      value |= regNum;
    }
#ifdef TFE_DEBUG_REGISTERS
    log_message(tfe_log,
                "Write Control Register %04x: %04x (reg=%02x)",
                ppaddress,value,regNum);
#endif

    /* invalid register? -> ignore! */
    if((regNum==0x01)||(regNum==0x11)||(regNum>0x19)) {
#ifdef TFE_DEBUG_WARN_REG
      log_message(tfe_log,
                  "WARNING! Ignoring write to reserved Control Register %04x (reg=%02x)",
                  ppaddress,regNum);
  #endif
      return;
    }
  }
  
  /* --- write to status register range --- */
  else if(ppaddress<0x140) {
#ifdef TFE_DEBUG_WARN_REG
    log_message(tfe_log,
                "WARNING! Ignoring write to read-only Status Register %04x",
                ppaddress);
#endif
    return;
  }

  /* --- write to initiate transmit register range --- */
  else if(ppaddress<0x150) {
    /* check tx_cmd register */
    if(ppaddress==0x144) {
      /* validate internal address */
      if((value&0x3f) != 0x09) {
        /* fix internal address */
        value &= ~0x3f;
        value |= 0x09;
      }
      /* mask out reserved bits */
      value &= 0x33ff;
#ifdef TFE_DEBUG_REGISTERS
      log_message(tfe_log,
                  "Write TX Cmd  Register %04x: %04x",
                  ppaddress,value);
#endif
    }
    /* check tx_length register */
    else if(ppaddress==0x146) {
      /* HW always masks 0x0fff */
      value &= 0x0fff;
#ifdef TFE_DEBUG_REGISTERS
      log_message(tfe_log,
                  "Write TX Len  Register %04x: %04x",
                  ppaddress,value);
#endif
    }
    /* reserved range */
    else if((ppaddress<0x144)||(ppaddress>0x147)) {
#ifdef TFE_DEBUG_WARN_REG
      log_message(tfe_log,
                  "WARNING! Ignoring write to reserved Initiate Transmit Register %04x",
                  ppaddress);
#endif
      return;
    }
  }

  /* --- write to address filter register range --- */
  else if(ppaddress<0x160) {
    /* reserved range */
    if(ppaddress>=0x15e) {
#ifdef TFE_DEBUG_WARN_REG
      log_message(tfe_log,
                  "WARNING! Ingoring write to reserved Address Filter Register %04x",
                  ppaddress);
#endif
      return;
    }
  }

  /* --- ignore write outside --- */
  else if(ppaddress<0x400) {
#ifdef TFE_DEBUG_WARN_REG
    log_message(tfe_log,
                "WARNING! Ingoring write to reserved Register %04x",
                ppaddress);
#endif
    return;
  }
  else if(ppaddress<0xa00){
#ifdef TFE_DEBUG_WARN_REG
    log_message(tfe_log,
                "WARNING! Ignoring write to RX Buffer Range %04x",
                ppaddress);
#endif
    return;
  }
  else {
#ifdef TFE_DEBUG_WARN_REG
    log_message(tfe_log,
                "WARNIGN! Ignoring write to TX Buffer Range %04x",
                ppaddress);
#endif
    return;
  }

  /* actually set value */
  SET_PP_16(ppaddress, value);  
}

#define PP_PTR_AUTO_INCR_FLAG 0x8000 /* auto increment flag in package pointer */
#define PP_PTR_FLAG_MASK      0xf000 /* is always : x y 1 1 (with x=auto incr) */
#define PP_PTR_ADDR_MASK      0x0fff /* address portion of packet page pointer */

static void tfe_auto_incr_pp_ptr(void)
{
  /* perform auto increment of packet page pointer */
  if((tfe_packetpage_ptr & PP_PTR_AUTO_INCR_FLAG)==PP_PTR_AUTO_INCR_FLAG) {
    /* pointer is always increment by one on real HW */
    word16 ptr   = tfe_packetpage_ptr & PP_PTR_ADDR_MASK;
    word16 flags = tfe_packetpage_ptr & PP_PTR_FLAG_MASK;
    ptr++;
    tfe_packetpage_ptr = ptr | flags;
  }
}

/* ------------------------------------------------------------------------- */
/* read/write TFE registers from VICE */

#define LO_BYTE(x)      (byte)((x) & 0xff)
#define HI_BYTE(x)      (byte)(((x) >> 8) & 0xff)
#define LOHI_WORD(x,y)  ( (word16)(x) | ( ((word16)(y)) <<8 ) )

/* ----- read byte from I/O range in VICE ----- */
byte tfe_read(word16 io_address)
{
	byte retval,lo,hi;
    word16 word_value;
    word16 reg_base;

    assert( tfe );
    assert( tfe_packetpage );
    assert( io_address < 0x10);

    /*if (tfe_as_rr_net) {
        // rr status register is handled by rr cartidge 
        if (io_address < 0x02) {
            return 0;
        }
        io_address ^= 0x08;
    }*/
    /* register base addr */
    reg_base = io_address & ~1;

    /* RX register is special as it reads from RX buffer directly */
    if((reg_base==TFE_ADDR_RXTXDATA)||(reg_base==TFE_ADDR_RXTXDATA2)) {
        //io_source=IO_SOURCE_TFE_RR_NET;
        log_message(tfe_log, "reading RX Register.\n");
        return tfe_read_rx_buffer(io_address & 0x01);
    }
    
    /* read packet page pointer */
    if(reg_base==TFE_ADDR_PP_PTR) {
        word_value = tfe_packetpage_ptr;
    }
    /* read a register from packet page */
    else {
        word16 ppaddress = 0;

        /* determine read addr in packet page */
        switch (reg_base) {
            /* PP_DATA2 behaves like PP_DATA on real HW
               both show the contents at the page pointer */
        case TFE_ADDR_PP_DATA:
        case TFE_ADDR_PP_DATA2:
            /* mask and align address of packet pointer */
            ppaddress = tfe_packetpage_ptr & PP_PTR_ADDR_MASK;
            ppaddress &= ~1;
            /* if flags match then auto incr pointer */
            tfe_auto_incr_pp_ptr();
            break;
        case TFE_ADDR_INTSTQUEUE:
            ppaddress = TFE_PP_ADDR_SE_ISQ;
            break;
        case TFE_ADDR_TXCMD:
            ppaddress = TFE_PP_ADDR_TXCMD;
            break;
        case TFE_ADDR_TXLENGTH:
            ppaddress = TFE_PP_ADDR_TXLENGTH;
            break;
        default:
            /* invalid! */
            assert(0);
            break;
        }
            
        /* do side effects before access */
        tfe_sideeffects_read_pp(ppaddress,io_address&1);

        /* read register value */
        word_value = tfe_read_register(ppaddress);
        
#ifdef TFE_DEBUG_LOAD
        log_message(tfe_log, "reading PP Ptr: $%04X => $%04X.\n",ppaddress,word_value);
#endif
    }

    /* extract return value from word_value */
    lo = LO_BYTE(word_value);
    hi = HI_BYTE(word_value);
    if((io_address & 1) == 0) {
        /* low byte on even address */
        retval = lo;
    } else {
        /* high byte on odd address */
        retval = hi;
    }

#ifdef TFE_DEBUG_LOAD
    log_message(tfe_log, "read [$%02X] => $%02X.\n", io_address, retval);
#endif
    
    /* update _word_ value in register bank */
    tfe[reg_base]   = lo;
    tfe[reg_base+1] = hi;
    
    //io_source=IO_SOURCE_TFE_RR_NET;
    return retval;
}

/* ----- write byte to I/O range of VICE ----- */
void tfe_store(word16 io_address, byte var)
{
	word16 reg_base;
    word16 word_value;
    assert( tfe );
    assert( tfe_packetpage );
    assert( io_address < 0x10);

    /*if (tfe_as_rr_net) {
        // rr control register is handled by rr cartidge
        if (io_address < 0x02) {
            return;
        }
        io_address ^= 0x08;
    }*/
#ifdef TFE_DEBUG_STORE
    log_message(tfe_log, "store [$%02X] <= $%02X.\n", io_address, (int)var);
#endif

    /* register base addr */
    reg_base = io_address & ~1;

    /* TX Register is special as it writes to TX buffer directly */
    if((reg_base==TFE_ADDR_RXTXDATA)||(reg_base==TFE_ADDR_RXTXDATA2)) {
        tfe_write_tx_buffer(var,io_address & 1);
        return;
    }

    /* combine stored value with new written byte */
    if((io_address & 1) == 0) {
        /* overwrite low byte */
        word_value = LOHI_WORD(var,tfe[reg_base+1]);
    } else {
        /* overwrite high byte */
        word_value = LOHI_WORD(tfe[reg_base],var);
    }

    if(reg_base==TFE_ADDR_PP_PTR) {
        /* cv: we store the full package pointer in tfe_packetpage_ptr variable.
            this includes the mask area (0xf000) and the addr range (0x0fff).
            we ensure that the bits 0x3000 are always set (as in real HW).
            odd values of the pointer are valid and supported.
            only register read and write have to be mapped to word boundary. */
        word_value |= 0x3000;
        tfe_packetpage_ptr = word_value;
#ifdef TFE_DEBUG_STORE
        log_message(tfe_log, "set PP Ptr to $%04X.\n", tfe_packetpage_ptr);
#endif
    } else {
        /* write a register */

        /*! \TODO: Find a reasonable default */
        word16 ppaddress = TFE_PP_ADDR_PRODUCTID;

        /* now determine address of write in packet page */
        switch(reg_base) {
        case TFE_ADDR_PP_DATA:
        case TFE_ADDR_PP_DATA2:
            /* mask and align ppaddress from page pointer */
            ppaddress = tfe_packetpage_ptr & (MAX_PACKETPAGE_ARRAY-1);
            ppaddress &= ~1;
            /* auto increment pp ptr */
            tfe_auto_incr_pp_ptr();
            break;
        case TFE_ADDR_TXCMD:
            ppaddress = TFE_PP_ADDR_TXCMD;
            break;    
        case TFE_ADDR_TXLENGTH:
            ppaddress = TFE_PP_ADDR_TXLENGTH;
            break;
        case TFE_ADDR_INTSTQUEUE:
            ppaddress = TFE_PP_ADDR_SE_ISQ;
            break;
        case TFE_ADDR_PP_PTR:
            break;
        default:
            /* invalid */
            assert(0);
            break;
        }

#ifdef TFE_DEBUG_STORE
        log_message(tfe_log, "before writing to PP Ptr: $%04X <= $%04X.\n", 
                    ppaddress,word_value);
#endif

        /* perform the write */
        tfe_write_register(ppaddress,word_value);

        /* handle sideeffects */
        tfe_sideeffects_write_pp(ppaddress, io_address&1);

        /* update word value if it was changed in write register or by side effect */
        word_value = GET_PP_16(ppaddress);

#ifdef TFE_DEBUG_STORE
        log_message(tfe_log, "after  writing to PP Ptr: $%04X <= $%04X.\n", 
                    ppaddress,word_value);
#endif
    }

    /* update tfe registers */
    tfe[reg_base]   = LO_BYTE(word_value);
    tfe[reg_base+1] = HI_BYTE(word_value);
}

/* ------------------------------------------------------------------------- */
/*    resources support functions                                            */
#if 0
static int set_tfe_disabled(int val, void *param)
{
    /* dummy function since we don't want "disabled" to be stored on disk */
    return 0;
}

/*static int set_tfe_rr_net(int val, void *param)
{
    if (!tfe_cannot_use) {
        if (!val) {
            // TFE should not be used as rr net 
            if (tfe_as_rr_net) {
                tfe_as_rr_net = 0;

                // if adapter is already enabled then reset the LAN chip 
                if (tfe) {
                    tfe_reset();
                }
            }
            return 0;
        } else { 
            if (!tfe_as_rr_net) {
                tfe_as_rr_net = 1;
            
                // if adapter is already enabled then reset the LAN chip
                if (tfe) {
                    tfe_reset();
                }
            }
            return 0;
        }
    }

    return 0;
}*/

static int set_tfe_enabled(int val, void *param)
{
    if (!tfe_cannot_use) {
        if (!val) {
            /* TFE should be deactived */
            if (tfe_enabled) {
                tfe_enabled = 0;
                if (tfe_deactivate() < 0) {
                    return -1;
                }
            }
            return 0;
        } else {
            if (!tfe_enabled) {
                tfe_enabled = 1;
                if (tfe_activate() < 0) {
                    return -1;
                }
            }

            return 0;
        }

    }
    return 0;
}
#endif /* 0 */

int set_tfe_interface(const char *name)
{
    if (tfe_interface != NULL && name != NULL
        && strcmp(name, tfe_interface) == 0)
        return 0;

    util_string_set(&tfe_interface, name);

    if (tfe_enabled) {
        /* ethernet is enabled, make sure that the new name is
           taken account of 
         */
        if (tfe_deactivate() < 0) {
            return -1;
        }
        if (tfe_activate() < 0) {
            return -1;
        }

        /* virtually reset the LAN chip */
        if (tfe) {
            tfe_reset();
        }
    }
    return 0;
}


/*static const resource_string_t resources_string[] = {
    { "ETHERNET_INTERFACE", 
      ARCHDEP_ETHERNET_DEFAULT_DEVICE, RES_EVENT_NO, NULL,
      &tfe_interface, set_tfe_interface, NULL },
    { NULL }
};

static const resource_int_t resources_int[] = {
    { "ETHERNET_DISABLED", 0, RES_EVENT_NO, NULL,
      &tfe_cannot_use, set_tfe_disabled, NULL },
    { "ETHERNET_ACTIVE", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &tfe_enabled, set_tfe_enabled, NULL },
    { "ETHERNET_AS_RR", 0, RES_EVENT_NO, NULL,
      &tfe_as_rr_net, set_tfe_rr_net, NULL },
    { NULL }
};*/

/*int tfe_resources_init(void)
{
    if (resources_register_string(resources_string) < 0)
        return -1;

    return resources_register_int(resources_int);
}*/

/* ------------------------------------------------------------------------- */
/*    commandline support functions                                          */

/*static const cmdline_option_t cmdline_options[] =
{
    { "-tfe", SET_RESOURCE, 0,
      NULL, NULL, "ETHERNET_ACTIVE", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_ENABLE_TFE,
      NULL, NULL },
    { "+tfe", SET_RESOURCE, 0,
      NULL, NULL, "ETHERNET_ACTIVE", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_DISABLE_TFE,
      NULL, NULL },
    { "-tfeif", SET_RESOURCE, 1,
      NULL, NULL, "ETHERNET_INTERFACE", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NAME, IDCLS_TFE_INTERFACE,
      NULL, NULL },
    { "-tferrnet", SET_RESOURCE, 0,
      NULL, NULL, "ETHERNET_AS_RR", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_ENABLE_TFE_AS_RRNET,
      NULL, NULL },
    { "+tferrnet", SET_RESOURCE, 0,
      NULL, NULL, "ETHERNET_AS_RR", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_DISABLE_TFE_AS_RRNET,
      NULL, NULL },
    { NULL }
};

int tfe_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}*/


/* ------------------------------------------------------------------------- */
/*    snapshot support functions                                             */

#if 0

static char snap_module_name[] = "TFE1764";
#define SNAP_MAJOR 0
#define SNAP_MINOR 0

int tfe_read_snapshot_module(struct snapshot_s *s)
{
    /* @SRT TODO: not yet implemented */
    return -1;
}

int tfe_write_snapshot_module(struct snapshot_s *s)
{
    /* @SRT TODO: not yet implemented */
    return -1;
}

#endif /* #if 0 */

/* ------------------------------------------------------------------------- */
/*    functions for selecting and querying available NICs                    */

int tfe_enumadapter_open(void)
{
    if (!tfe_arch_enumadapter_open()) {
        tfe_cannot_use = 1;
        return 0;
    }
    return 1;
}

int tfe_enumadapter(char **ppname, char **ppdescription)
{
    return tfe_arch_enumadapter(ppname, ppdescription);
}

int tfe_enumadapter_close(void)
{
    return tfe_arch_enumadapter_close();
}

//#endif /* #ifdef HAVE_TFE */
