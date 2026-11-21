/*
 * The N-Gage QD's Bluetooth module as its driver sees it: CBUS device 5.
 *
 * Plain C with no QEMU in it, shared by the model (ngage_bt.c) and the
 * benches of the core's copy (rtl/board/bt_module.sv). What is here is
 * read out of the phone's driver, btldd.ldd ("bt_driver"), and the HCI
 * layer above it; the module itself is not documented anywhere we found.
 * docs/BLUETOOTH.md has the evidence for every line.
 *
 * Registers (16 bits each; the number is the CBUS register, the name the
 * driver's where it prints one):
 *
 *   0x00  data. Read: the next two bytes of the packet the module holds
 *         for the phone, first byte in bits 15:8. Write: the next two bytes
 *         of the phone's packet, the same way round.
 *   0x01  length in bytes of the packet the module holds (read only).
 *   0x06  read: status. Bits 4-8 are events, cleared by the read; the
 *         interrupt line pulses when one is set.
 *           bit 2  awake (always, here)
 *           bit 4  a packet for the phone is ready (registers 0x01, 0x00)
 *           bit 5  the module wants to sleep (never, here)
 *           bit 6  register 0x08 has an answer
 *           bit 7  the module has reset (register 0x09 says why)
 *           bit 8  the module woke up (never, here)
 *         write: bit 0 resets the module, bit 1 wakes it.
 *   0x08  "LPRF_MCU_Reg0". Write: 0x8000 | n asks for room for a packet of
 *         n bytes, 0xC000 | n says its n bytes have been written. Read: the
 *         answer, n in bits 13:0 with bits 15:14 = 00 for the room and 01
 *         for the packet.
 *   0x09  reset reason; bit 3 = the phone asked for it.
 *   0x0A  write 0x8000 after a reset: the module answers through register
 *         0x08 with the size of its buffer.
 *
 * Copyright (c) 2026 the NGage_MiSTer authors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef HW_ARM_NGAGE_BT_CHIP_H
#define HW_ARM_NGAGE_BT_CHIP_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define BT_REG_DATA     0x00
#define BT_REG_RXLEN    0x01
#define BT_REG_STATUS   0x06
#define BT_REG_REG0     0x08
#define BT_REG_REASON   0x09
#define BT_REG_START    0x0A

#define BT_ST_AWAKE     0x0004
#define BT_ST_RX        0x0010
#define BT_ST_ACK       0x0040
#define BT_ST_RESET     0x0080

#define BT_REASON_HOST  0x0008
#define BT_BUF_SIZE     520         /* the driver's largest write */
#define BT_RX_MAX       256         /* the longest event here: Read_Local_Name's */
#define BT_NAME_LEN     248
#define BT_NPAR         16

/* HCI status codes used here. */
#define BT_OK           0x00
#define BT_UNKNOWN_CMD  0x01
#define BT_DISALLOWED   0x0C
#define BT_BAD_PARAMS   0x12

/* What the module holds for the phone (rx_kind) and where a command's
 * return values come from (rep_src). */
enum { BT_RX_NONE, BT_RX_REPLY, BT_RX_INQ };
enum { BT_SRC_NONE, BT_SRC_FIX, BT_SRC_PAR, BT_SRC_NAME };

/*
 * The state is what the core's copy keeps, register for register, so that
 * the two can be compared after every access: the module does not buffer
 * packets. Of the phone's packet it keeps the bytes a command is decoded
 * from (tx[0], tx[2..8]; tx[] is whole here only for the log) and the
 * words of a new name; its own packet is made a word at a time, as the
 * phone reads it, from what the last command left (rep_*).
 */
typedef struct BtChip {
    bool in_reset;                  /* between the reset request and bt_chip_reset_done */
    uint16_t events;                /* status bits 4-8, cleared when read */
    uint16_t reg0, reason;
    /* the phone's packet */
    uint8_t tx[BT_BUF_SIZE];
    uint16_t tx_idx;                /* bytes written since room was asked for */
    uint16_t name_w[128];           /* words 2-126 of the last Change_Local_Name, as written */
    /* the controller */
    uint32_t par[BT_NPAR];          /* bt_par's values, first byte lowest */
    uint16_t par_set;               /* bit e: par[e] was written since the reset */
    uint16_t name_len;
    bool inq_run, inq_done;         /* an inquiry runs; it has ended and its event waits */
    /* the answer to the last command */
    bool rep_pend;                  /* it waits for the phone to take what is held */
    bool rep_cs;                    /* Command Status (else Command Complete) */
    uint8_t rep_status, rep_src, rep_idx;
    uint16_t rep_op, rep_n;
    /* the packet held for the phone */
    uint8_t rx_kind;
    uint16_t rx_len, rx_idx;
    /* for the owner (cleared by it) */
    bool irq;                       /* an event was raised: pulse the line */
    bool reset_req;                 /* call bt_chip_reset_done after the reset time */
    uint8_t inq_start;              /* nonzero: start the inquiry timer, this many 1.28 s */
    bool inq_stop;                  /* stop the inquiry timer */
    uint16_t tx_done;               /* nonzero: the phone's packet tx[0..tx_done) arrived */
    bool rx_new;                    /* a packet was put up (bt_rx_byte gives its bytes) */
} BtChip;

/*
 * The values a command reads back before the phone has written them: the
 * defaults of the Bluetooth 1.1 specification. {read OCF, bytes, default},
 * all in OGF 3; the write command is the next OCF.
 */
typedef struct BtPar { uint8_t ocf, n, def[4]; } BtPar;
static const BtPar bt_par[BT_NPAR] = {
    { 0x09, 1, { 0x00 } },                      /* PIN type: variable */
    { 0x15, 2, { 0xA0, 0x1F } },                /* connection accept timeout: 5.06 s */
    { 0x17, 2, { 0x00, 0x20 } },                /* page timeout: 5.12 s */
    { 0x19, 1, { 0x00 } },                      /* scan enable: none */
    { 0x1B, 4, { 0x00, 0x08, 0x12, 0x00 } },    /* page scan interval 1.28 s, window 11.25 ms */
    { 0x1D, 4, { 0x00, 0x08, 0x12, 0x00 } },    /* inquiry scan, the same */
    { 0x1F, 1, { 0x00 } },                      /* authentication: off */
    { 0x21, 1, { 0x00 } },                      /* encryption: off */
    { 0x23, 3, { 0x00, 0x00, 0x00 } },          /* class of device */
    { 0x25, 2, { 0x60, 0x00 } },                /* voice setting */
    { 0x29, 1, { 0x01 } },                      /* broadcast retransmissions */
    { 0x2B, 1, { 0x00 } },                      /* hold mode activity */
    { 0x2E, 1, { 0x00 } },                      /* SCO flow control: off */
    { 0x39, 4, { 0x01, 0x33, 0x8B, 0x9E } },    /* one inquiry access code, the general one */
    { 0x3B, 1, { 0x00 } },                      /* page scan period mode P0 */
    { 0x3D, 1, { 0x00 } },                      /* page scan mode: mandatory */
};
#define BT_PAR_IAC      13

/* Commands that complete at once with fixed values. */
typedef struct BtFix { uint16_t op; uint8_t n, ret[8]; } BtFix;
static const BtFix bt_fix[] = {
    { 0x0C0D, 4, { 0x00, 0x00, 0x00, 0x00 } },  /* Read_Stored_Link_Key: room for none, none read */
    { 0x0C11, 1, { 0x00 } },                    /* Write_Stored_Link_Key: none written */
    { 0x0C12, 2, { 0x00, 0x00 } },              /* Delete_Stored_Link_Key: none deleted */
    { 0x0C38, 1, { 0x01 } },                    /* Read_Number_Of_Supported_IAC */
    /* Read_Local_Version_Information: HCI 1.1, revision 0, LMP 1.1,
     * manufacturer 1 (Nokia Mobile Phones), subversion 0 */
    { 0x1001, 8, { 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00 } },
    /* Read_Local_Supported_Features: all of 1.1's */
    { 0x1003, 8, { 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* Read_Buffer_Size: ACL 339 bytes x 8, SCO 64 bytes x 8 */
    { 0x1005, 7, { 0x53, 0x01, 0x40, 0x08, 0x00, 0x08, 0x00 } },
    { 0x1007, 1, { 0x00 } },                    /* Read_Country_Code */
    { 0x1009, 6, { 0x00, 0x00, 0x00, 0x57, 0x60, 0x00 } },  /* Read_BD_ADDR: 00:60:57:00:00:00 */
    /* nothing to return: Set_Event_Mask, Set_Event_Filter, Create_New_Unit_Key,
     * Set_Host_Controller_To_Host_Flow_Control, Host_Buffer_Size */
    { 0x0C01, 0, { 0 } }, { 0x0C05, 0, { 0 } }, { 0x0C0B, 0, { 0 } },
    { 0x0C31, 0, { 0 } }, { 0x0C33, 0, { 0 } },
};

static inline void bt_chip_event(BtChip *c, uint16_t bit)
{
    c->events |= bit;
    c->irq = true;
}

static inline void bt_chip_init(BtChip *c)
{
    memset(c, 0, sizeof(*c));
}

/* The phone asked for a reset: everything is forgotten at once and the
 * module is deaf (reads 0, ignores writes) until the owner's reset time
 * has passed. */
static inline void bt_chip_reset(BtChip *c)
{
    bt_chip_init(c);
    c->in_reset = true;
    c->reset_req = true;
    c->inq_stop = true;
}

/* The module has come out of that reset. */
static inline void bt_chip_reset_done(BtChip *c)
{
    c->in_reset = false;
    c->reason = BT_REASON_HOST;
    bt_chip_event(c, BT_ST_RESET);
}

/* Byte i of the packet held for the phone. An event is 04 00, its code,
 * its parameter length, its parameters. */
static inline uint8_t bt_rx_byte(const BtChip *c, unsigned i)
{
    unsigned k = i - 8, e = c->rep_idx;

    if (c->rx_kind == BT_RX_INQ) {
        /* Inquiry Complete: one parameter, the status. hci.dll (0x5046BCE8)
         * takes no other length: with two bytes (a status and a count of
         * responses) it hands the stack "Invalid HCI Command Parameters". */
        static const uint8_t inq[5] = { 0x04, 0x00, 0x01, 0x01, 0x00 };
        return i < 5 ? inq[i] : 0;
    }
    if (c->rep_cs) {
        /* Command Status: status, one more command may be sent, opcode */
        const uint8_t cs[8] = { 0x04, 0x00, 0x0F, 0x04, c->rep_status, 0x01,
                                (uint8_t)c->rep_op, (uint8_t)(c->rep_op >> 8) };
        return i < 8 ? cs[i] : 0;
    }
    if (i < 8) {
        /* Command Complete: one more command may be sent, opcode, status */
        const uint8_t cc[8] = { 0x04, 0x00, 0x0E, (uint8_t)(4 + c->rep_n), 0x01,
                                (uint8_t)c->rep_op, (uint8_t)(c->rep_op >> 8), c->rep_status };
        return cc[i];
    }
    switch (c->rep_src) {
    case BT_SRC_FIX:
        return bt_fix[e].ret[k & 7];
    case BT_SRC_PAR:
        return (c->par_set >> e & 1) ? (uint8_t)(c->par[e] >> 8 * (k & 3)) : bt_par[e].def[k & 3];
    case BT_SRC_NAME:
        /* name byte k was byte k + 5 of the packet that set it */
        if (k < c->name_len) {
            uint16_t w = c->name_w[(k + 5) >> 1 & 127];
            return (k + 5) & 1 ? (uint8_t)w : (uint8_t)(w >> 8);
        }
        return 0;
    }
    return 0;
}

/* Put up the next packet for the phone, if it has taken the last one: a
 * command's answer first, then the end of an inquiry. */
static inline void bt_chip_pump(BtChip *c)
{
    if (c->rx_kind != BT_RX_NONE) {
        return;
    }
    if (c->rep_pend) {
        c->rep_pend = false;
        c->rx_kind = BT_RX_REPLY;
        c->rx_len = c->rep_cs ? 8 : 8 + c->rep_n;
    } else if (c->inq_done) {
        c->inq_done = false;
        c->rx_kind = BT_RX_INQ;
        c->rx_len = 5;
    } else {
        return;
    }
    c->rx_idx = 0;
    c->rx_new = true;
    bt_chip_event(c, BT_ST_RX);
}

/* The inquiry's time is up (the owner's timer). */
static inline void bt_chip_inquiry_done(BtChip *c)
{
    if (c->inq_run) {
        c->inq_run = false;
        c->inq_done = true;
        bt_chip_pump(c);
    }
}

static inline void bt_hci_packet(BtChip *c, unsigned len);

static inline uint16_t bt_chip_read(BtChip *c, unsigned reg)
{
    uint16_t v = 0;

    if (c->in_reset) {
        return 0;
    }
    switch (reg) {
    case BT_REG_DATA:
        if (c->rx_kind != BT_RX_NONE) {
            v = bt_rx_byte(c, c->rx_idx) << 8;
            if (c->rx_idx + 1 < c->rx_len) {
                v |= bt_rx_byte(c, c->rx_idx + 1);
            }
            c->rx_idx += 2;
            if (c->rx_idx >= c->rx_len) {
                c->rx_kind = BT_RX_NONE;    /* the phone has all of it */
                bt_chip_pump(c);
            }
        }
        break;
    case BT_REG_RXLEN:
        v = c->rx_kind != BT_RX_NONE ? c->rx_len : 0;
        break;
    case BT_REG_STATUS:
        v = c->events | BT_ST_AWAKE;
        c->events = 0;
        break;
    case BT_REG_REG0:
        v = c->reg0;
        break;
    case BT_REG_REASON:
        v = c->reason;
        break;
    }
    return v;
}

static inline void bt_chip_write(BtChip *c, unsigned reg, uint16_t v)
{
    unsigned n = v & 0x3FFF, got, w;

    if (reg == BT_REG_STATUS && (v & 1)) {
        bt_chip_reset(c);
        return;
    }
    if (c->in_reset) {
        return;
    }
    switch (reg) {
    case BT_REG_DATA:
        w = c->tx_idx >> 1;
        if (c->tx_idx < BT_BUF_SIZE) {
            c->tx[c->tx_idx++] = v >> 8;
            c->tx[c->tx_idx++] = v & 0xFF;
            if (w >= 2 && w <= 126 && c->tx[2] == 0x13 && c->tx[3] == 0x0C) {
                c->name_w[w] = v;           /* a Change_Local_Name's name, as it comes */
            }
        }
        break;
    case BT_REG_START:
        if (v & 0x8000) {
            c->reg0 = BT_BUF_SIZE;
            bt_chip_event(c, BT_ST_ACK);
        }
        break;
    case BT_REG_REG0:
        if ((v & 0xC000) == 0x8000) {           /* room for n bytes? */
            c->tx_idx = 0;
            c->reg0 = n;
            bt_chip_event(c, BT_ST_ACK);
        } else if ((v & 0xC000) == 0xC000) {    /* n bytes written */
            got = c->tx_idx;
            c->tx_idx = 0;
            c->reg0 = 0x4000 | n;
            bt_chip_event(c, BT_ST_ACK);
            if (n > BT_BUF_SIZE) {
                n = BT_BUF_SIZE;
            }
            if (got >= n) {                     /* all of it was written */
                c->tx_done = n;
                bt_hci_packet(c, n);
            }
        }
        break;
    }
}

/*
 * HCI: a Bluetooth 1.1 controller with nobody in range.
 *
 * The phone's packet is tx[0..len): 01 00, then the command (opcode, low
 * byte first; the parameter length; the parameters). One command is
 * answered at a time, as HCI's own flow control has it: a command that
 * comes before the last one's answer was read takes that answer's place.
 */
static inline void bt_hci_command(BtChip *c, unsigned op, uint32_t p, unsigned plen)
{
    unsigned ogf = op >> 10, ocf = op & 0x3FF;
    bool rep = true, cs = false;
    unsigned status = BT_OK, src = BT_SRC_NONE, idx = 0, n = 0;
    bool known = false;

    for (unsigned i = 0; i < sizeof(bt_fix) / sizeof(bt_fix[0]); i++) {
        if (bt_fix[i].op == op) {
            known = true;
            n = bt_fix[i].n;
            src = n ? BT_SRC_FIX : BT_SRC_NONE;
            idx = n ? i : 0;
        }
    }
    for (unsigned e = 0; e < BT_NPAR && ogf == 3 && !known; e++) {
        if (ocf == bt_par[e].ocf) {                     /* Read_... */
            known = true;
            src = BT_SRC_PAR;
            idx = e;
            n = bt_par[e].n;
        } else if (ocf == bt_par[e].ocf + 1u) {         /* Write_... */
            known = true;
            if (plen < bt_par[e].n || (e == BT_PAR_IAC && (p & 0xFF) != 1)) {
                status = BT_BAD_PARAMS;
            } else {
                c->par[e] = p;
                c->par_set |= 1u << e;
            }
        }
    }
    if (!known) {
        switch (op) {
        case 0x0401:                            /* Inquiry: LAP, length, responses */
            cs = true;
            if (plen < 5 || (p >> 24) < 1 || (p >> 24) > 0x30) {
                status = BT_BAD_PARAMS;
            } else if (c->inq_run) {
                status = BT_DISALLOWED;
            } else {
                c->inq_run = true;
                c->inq_start = p >> 24;
            }
            break;
        case 0x0402:                            /* Inquiry_Cancel */
            if (c->inq_run) {
                c->inq_run = false;
                c->inq_stop = true;
            } else {
                status = BT_DISALLOWED;
            }
            break;
        case 0x0C03:                            /* Reset */
            c->inq_run = c->inq_done = false;
            c->inq_stop = true;
            c->par_set = 0;
            c->name_len = 0;
            break;
        case 0x0C13:                            /* Change_Local_Name */
            c->name_len = plen < BT_NAME_LEN ? plen : BT_NAME_LEN;
            break;
        case 0x0C14:                            /* Read_Local_Name */
            src = BT_SRC_NAME;
            n = BT_NAME_LEN;
            break;
        case 0x0C35:                            /* Host_Number_Of_Completed_Packets: no event */
            rep = false;
            break;
        default:
            /* With no link and nobody to page, the link commands cannot
             * run; anything else is not a command of this controller. */
            cs = true;
            status = ogf == 1 || ogf == 2 ? BT_DISALLOWED : BT_UNKNOWN_CMD;
            break;
        }
    }
    if (rep) {
        c->rep_pend = true;
        c->rep_cs = cs;
        c->rep_status = status;
        c->rep_op = op;
        c->rep_src = src;
        c->rep_idx = idx;
        c->rep_n = n;
        if (c->rx_kind == BT_RX_REPLY) {
            c->rx_kind = BT_RX_NONE;            /* the answer nobody read */
        }
    }
}

static inline void bt_hci_packet(BtChip *c, unsigned len)
{
    unsigned plen;

    if (len < 5 || c->tx[0] != 0x01) {
        return;                                 /* not a command: there is no link for data */
    }
    plen = c->tx[4];
    if (plen > len - 5) {
        plen = len - 5;
    }
    bt_hci_command(c, c->tx[2] | c->tx[3] << 8,
                   c->tx[5] | c->tx[6] << 8 | c->tx[7] << 16 | (uint32_t)c->tx[8] << 24, plen);
    bt_chip_pump(c);
}

#endif
