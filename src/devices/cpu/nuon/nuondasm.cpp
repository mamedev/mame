// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Nuon disassembler

#include "emu.h"
#include "nuondasm.h"

/*
  Nuon instructions are a variant on the VLIW concept, where the
  instruction tells every execution unit what to do.  It has five
  execution units: mul, ecu, rcu, mem and alu.  The rcu can also do in
  parallel to its specific instruction a decrement of the rc0 register
  and one of the rc1 one, making the marketing say they execute seven
  instructions in parallel.

  Instructions are built out of packets, which is a sub-instruction
  for a specific execution unit.  Each packet except the 16-bits alu
  ones also have a bit to tell whether another packet follows for the
  same instruction.  16-bits alu packets are always at the end when
  present, the order or the packets seems otherwise free.  There are
  complicated instruction alignment, size and resource sharing
  constraints the disassembler does not care about.

  A packet is a series of one to four 16-bits big endian values.  The
  top bits of the first 16-bits word help at classifying the kind of
  packet, in a very huffman-ish way. 00 indicates a 16-bit alu
  packet. 01 and 11 a 16-bits indicates a non-alu packet, with a
  following packet (01) or not (11).

  1001 indicates a 32-bits instruction, with the following two bits
  classifying the instruction.  00 is ecu, 01 is mem, 10 is alu and 11
  is mul.  There are no 32-bits rcu packets.  1010 and 1011 indicates
  the second word of a 32-bits instruction with (1010) or without
  (1011) a following packet.

  10000 indicates "special", not associated to an execution unit,
  e.g. nop and breakpoint.

  10001 finally indicates a 32-bits prefix modifying a 16/32 bits
  instruction that follows by providing 27 extra bits to the
  instruction decode, usually turning small constants into 32-bits
  constants.  There can be only one per full-instruction, even if the
  disassembler doesn't care.

  Lots of registers are memory mapped, and specific rom/ram is at
  specific unchangeable addresses.  The instructions sometimes have
  base offsets hardcoded in their interpretation to save bits.  The
  offsets that may be encountered in instructions are (in hex):
  - 20000000: data rom (replaced by ram in Aries 3)
  - 20100000: data ram
  - 20200000: instruction rom (undumped, looks minimal, e.g. copies something like 128 bytes from f0000000 to 20300000 then runs it)
  - 20300000: instruction ram
  - 20400000: data tag ram
  - 20480000: instruction tag ram
  - 20500000: control register space

  Notations:
  - rn, rm, ro: global register with n, m or o in [0,31]
  - vn, vm, vo: global vector register with n, m or o in [0,7]
  - xwv/NNus << MM : value of NN bits build from the bits x, w and v, signed (s) or unsigned (u), (optionally) left-shifted by MM bits
  - rx: specific register depending on the n value. rx:n=0, ry:n=1, ru:n=2, rv:n=3
  - (xy): registers (uv) for u=0 or (xy) for u=1
  - f: follow-up bit, last packet if f=1, more packets follow if f=0
  - a: if 1 add dec rc0 to the rcu packet
  - b: if 1 add dec rc1 to the rcu packet
  - cc: 5-bits condition code, see cc method for the list
  - cc1: 3-bits condition code, bottom two bits of the 5-bit code are zero
  - svs: encoded shift, 16:s=0, 24:s=1, 32:s=2, 30:s=3

  0000 00mm mmmn nnnn                                                                    [alu] add rm, rn
  0000 01vv vvvn nnnn                                                                    [alu] add #v/5u, rn
  0000 11mm m00n nn00                                                                    [alu] add_sv vm, vn
  0000 11mm m00n nn01                                                                    [alu] sub_sv vm, vn
  0000 11nn nnn0 0010                                                                    [alu] neg rn
  0000 11nn nnn0 0011                                                                    [alu] abs rn
  0001 00mm mmmn nnnn                                                                    [alu] sub rm, rn
  0001 01vv vvvn nnnn                                                                    [alu] sub #v/5u, rn
  0001 10vv vvvn nnnn                                                                    [alu] eor #v/5s, rn
  0001 11mm mmmn nnnn                                                                    [alu] cmp rm, rn
  0010 00vv vvvn nnnn                                                                    [alu] cmp #v/5u, rn
  0010 01mm mmmn nnnn                                                                    [alu] and rm, rn
  0010 10mm mmmn nnnn                                                                    [alu] or rm, rn
  0010 11mm mmmn nnnn                                                                    [alu] eor rm, rn
  0011 00nn nnnd dddd                                                                    [alu] asl #(32 - (d/5u) & 31, rn
  0011 01nn nnnd dddd                                                                    [alu] asr #d/5u, rn
  0011 10nn nnnd dddd                                                                    [alu] lsr #d/5u, rn
  0011 11nn nnnd dddd                                                                    [alu] btst #d/5u, rn

  f100 00nn nnnm mmmm                                                                    [mul] mul rm, rn, >>acshift, rn

  f100 10mm mmmn nnnn                                                                    [mem] ld_s (rm), rn
  f100 11mm mmmn nnnn                                                                    [mem] st_s rn, (rm)
  f101 00mm mmmn nnnn                                                                    [mem] ld_s 20500000 + m/5u << 4, rn
  f101 01mm mmmn nnnn                                                                    [mem] st_s rn, 20500000 + m/5u << 4
  f101 10mm mmmn nnnn                                                                    [mem] mv_s rn, rm
  f101 11mm mmmv vvvv                                                                    [mem] mv_s #v/5u, rm
  f110 00nn n00m mm00                                                                    [mem] mv_v vm, vn
  f110 0000 001n nn00                                                                    [mem] push vn
  f110 0001 001n nn00                                                                    [mem] push vn, rz
  f110 0010 001n nnnn                                                                    [mem] push rn, cc, rzi1, rz
  f110 0011 001n nnnn                                                                    [mem] push rn, cc, rzi1, rz
  f110 0000 010n nn00                                                                    [mem] pop vn
  f110 0001 010n nn00                                                                    [mem] pop vn, rz
  f110 0010 010n nnnn                                                                    [mem] pop rn, cc, rzi1, rz
  f110 0011 010n nnnn                                                                    [mem] pop rn, cc, rzi1, rz

  f110 10cc crrr rrrr                                                                    [ecu] bra cc1, pc + r/7s << 1
  f110 11rr rrrr rrrr                                                                    [ecu] bra pc + r/10s<< 1
  f111 0000 0000 0001                                                                    [ecu] halt
  f111 00cc ccc1 0000                                                                    [ecu] rts cc
  f111 00cc ccc1 0001                                                                    [ecu] rts cc, nop
  f111 00cc ccc1 0010                                                                    [ecu] rti cc, (rzi1)
  f111 00cc ccc1 0011                                                                    [ecu] rti cc, (rzi1), nop
  f111 00cc ccc1 0100                                                                    [ecu] rti cc, (rzi2)
  f111 00cc ccc1 0101                                                                    [ecu] rti cc, (rzi2), nop

  f111 01mm mmmn n0ab                                                                    [rcu] addr rm, rx
  f111 01vv vvvn n1ab                                                                    [rcu] addr #v/5s << 16, rx
  f111 10mm mmmn n0ab                                                                    [rcu] mvr rm, rx
  f111 1000 000n n1ab                                                                    [rcu] modulo rx
  f111 1001 0000 01ab                                                                    [rcu] dec_only
  f111 1010 000n n1ab                                                                    [rcu] range rx

  1000 000f 0000 0000                                                                    [???] nop/pad
  1000 001f 0000 0000                                                                    [???] breakpoint

  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 0000 01ww wwwn nnnn                        [alu] add #xw/32s, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 0001 01ww wwwn nnnn                        [alu] sub #xw/32s, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 0001 10ww wwwn nnnn                        [alu] eor #xw/32s, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 0010 00ww wwwn nnnn                        [alu] cmp #xw/32s, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . f101 11nn nnnw wwww                        [mem] mv_s #xw/32s, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx x0CC . f110 10cc cvvv vvvv                        [ecu] bra cC, pc + #xv/31s << 1      (asm only wants 28 bits?)
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx x1CC . f110 10cc cvvv vvvv                        [ecu] bra cC, pc + #xv/31s << 1, nop (asm only wants 28 bits?)
  1000 1xxx xxxx xxxx . vvvv vvvv vvvv vvvv . f111 01ww wwwn n1ab                        [rcu] addr #xwv/32s, rx
  1000 1xxx xxxx xxxx . vvvv vvvv vvvv vvvv . f111 10ww wwwn n0ab                        [rcu] mvr #xwv/32s, rx

  1000 1xxx xxxx xxxx . xxxx xxx0 0000 0000 . 1001 00cc cccv vvvv . 101f 0010 wwww wwww  [ecu] jmp cc, #xwv/31u << 1
  1000 1xxx xxxx xxxx . xxxx xxx0 0000 0000 . 1001 00cc cccv vvvv . 101f 0100 wwww wwww  [ecu] jsr cc, #xwv/31u << 1
  1000 1xxx xxxx xxxx . xxxx xxx0 0000 0000 . 1001 00cc cccv vvvv . 101f 1010 wwww wwww  [ecu] jmp cc, #xwv/31u << 1, nop
  1000 1xxx xxxx xxxx . xxxx xxx0 0000 0000 . 1001 00cc cccv vvvv . 101f 1100 wwww wwww  [ecu] jsr cc, #xwv/31u << 1, nop
  1000 1xxx xxxx xxxx . xxxx xxxx xxx0 0qmm . 1001 01oo ooov vvvv . 101f 111p pppw wwww  [mem] st_s #xwv/32u, 20000000 + qpom/12u << 2
  1000 1xxx xxxx xxxx . xxxx xxxx xxx0 1qmm . 1001 01oo ooov vvvv . 101f 111p pppw wwww  [mem] st_s #xwv/32u, 20100000 + qpom/12u << 2
  1000 1xxx xxxx xxxx . xxxx xxxx xxx1 0qmm . 1001 01oo ooov vvvv . 101f 111p pppw wwww  [mem] st_s #xwv/32u, 20500000 + qpom/12u << 2
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10ww wwwm mmmm . 101f 0000 001n nnnn  [alu] add #xw/32s, rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10ww wwwm mmmm . 101f 0001 001n nnnn  [alu] add #xw/32s, rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10mm mmmw wwww . 101f 0001 100n nnnn  [alu] add rm, #xw/32s, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10nn nnnv vvvv . 101f 0010 1000 0000  [alu] cmp rn, #xv/32s
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10ww wwwm mmmm . 101f 0011 001n nnnn  [alu] and #xw/32s, rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10nn nnnm mmmm . 101f 0011 011w wwww  [alu] and #xw/32s, >>rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10ww wwwn nnnn . 101f 0100 0010 0000  [alu] ftst #xw/32s, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10nn nnnm mmmm . 101f 0100 011w wwww  [alu] ftst #xw/32s, >>rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10ww wwwm mmmm . 101f 0101 001n nnnn  [alu] or #xw/32s, rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10nn nnnm mmmm . 101f 0101 011w wwww  [alu] or #xw/32s, >>rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10ww wwwm mmmm . 101f 0110 001n nnnn  [alu] eor #xw/32s, rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10nn nnnm mmmm . 101f 0110 011w wwww  [alu] eor #xw/32s, >>rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10ww wwwm mmmm . 101f 1100 001n nnnn  [alu] addwc #xw/32s, rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10ww wwwm mmmm . 101f 1101 001n nnnn  [alu] subwc #xw/32s, rm, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10mm mmmw wwww . 101f 1101 100n nnnn  [alu] subwc rm, #xw/32s, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10vv vvvn nnnn . 101f 1110 0010 0000  [alu] cmpwc #xv/32s, rn
  1000 1xxx xxxx xxxx . xxxx xxxx xxxx xxxx . 1001 10nn nnnv vvvv . 101f 1110 1000 0000  [alu] cmpwc rn, #xv/32s

  1001 00cc cccv vvvv . 101f 000w wwww wwww                                              [ecu] bra cc, pc + wv/14s << 1
  1001 00cc cccv vvvv . 101f 0010 0www wwww                                              [ecu] jmp cc, 20200000 + wv/12u << 1
  1001 00cc cccv vvvv . 101f 0011 0www wwww                                              [ecu] jmp cc, 20300000 + wv/12u << 1
  1001 00cc cccv vvvv . 101f 0100 0www wwww                                              [ecu] jsr cc, 20200000 + wv/12u << 1
  1001 00cc cccv vvvv . 101f 0101 0www wwww                                              [ecu] jsr cc, 20300000 + wv/12u << 1
  1001 00cc cccn nnnn . 101f 0110 8000 0000                                              [ecu] jmp cc, (rn)
  1001 00cc cccn nnnn . 101f 0111 0000 0000                                              [ecu] jsr cc, (rn)
  1001 00cc cccv vvvv . 101f 100w wwww wwww                                              [ecu] bra cc, pc + wv/14s << 1, nop
  1001 00cc cccv vvvv . 101f 1010 0www wwww                                              [ecu] jmp cc, 20200000 + wv/12u << 1, nop
  1001 00cc cccv vvvv . 101f 1011 0www wwww                                              [ecu] jmp cc, 20300000 + wv/12u << 1, nop
  1001 00cc cccv vvvv . 101f 1100 0www wwww                                              [ecu] jsr cc, 20200000 + wv/12u << 1, nop
  1001 00cc cccv vvvv . 101f 1101 0www wwww                                              [ecu] jsr cc, 20300000 + wv/12u << 1, nop
  1001 00cc cccn nnnn . 101f 1110 8000 0000                                              [ecu] jmp cc, (rn)
  1001 00cc cccn nnnn . 101f 1111 0000 0000                                              [ecu] jsr cc, (rn), nop

  1001 01mm mmmn nnnn . 101f 0000 00oo oooo                                              [mem] ld_b 20000000 + om/11u, rn
  1001 01mm mmmn nnnn . 101f 0000 01oo oooo                                              [mem] ld_b 20100000 + om/11u, rn
  1001 01mm mmmn nnnn . 101f 0000 10oo oooo                                              [mem] ld_b 20500000 + om/11u, rn
  1001 01mm mmmn nnnn . 101f 0001 00oo oooo                                              [mem] ld_w 20000000 + om/11u << 1, rn
  1001 01mm mmmn nnnn . 101f 0001 01oo oooo                                              [mem] ld_w 20100000 + om/11u << 1, rn
  1001 01mm mmmn nnnn . 101f 0001 10oo oooo                                              [mem] ld_w 20500000 + om/11u << 1, rn
  1001 01mm mmmn nnnn . 101f 0010 00oo oooo                                              [mem] ld_s 20000000 + om/11u << 2, rn
  1001 01mm mmmn nnnn . 101f 0010 01oo oooo                                              [mem] ld_s 20100000 + om/11u << 2, rn
  1001 01mm mmmn nnnn . 101f 0010 10oo oooo                                              [mem] ld_s 20500000 + om/11u << 2, rn
  1001 01mm mmmn nn00 . 101f 0011 00oo oooo                                              [mem] ld_sv 20000000 + om/11u << 3, vn
  1001 01mm mmmn nn00 . 101f 0011 01oo oooo                                              [mem] ld_sv 20100000 + om/11u << 3, vn
  1001 01mm mmmn nn00 . 101f 0011 10oo oooo                                              [mem] ld_sv 20500000 + om/11u << 3, vn
  1001 01mm mmmn nn01 . 101f 0011 00oo oooo                                              [mem] ld_v 20000000 + om/11u << 4, vn
  1001 01mm mmmn nn01 . 101f 0011 01oo oooo                                              [mem] ld_v 20100000 + om/11u << 4, vn
  1001 01mm mmmn nn01 . 101f 0011 10oo oooo                                              [mem] ld_v 20500000 + om/11u << 4, vn
  1001 01mm mmmn nn10 . 101f 0011 00oo oooo                                              [mem] ld_p 20000000 + om/11u << 1, vn
  1001 01mm mmmn nn10 . 101f 0011 01oo oooo                                              [mem] ld_p 20100000 + om/11u << 1, vn
  1001 01mm mmmn nn10 . 101f 0011 10oo oooo                                              [mem] ld_p 20500000 + om/11u << 1, vn
  1001 01mm mmmn nn11 . 101f 0011 00oo oooo                                              [mem] ld_pz 20000000 + om/11u << 1, vn
  1001 01mm mmmn nn11 . 101f 0011 01oo oooo                                              [mem] ld_pz 20100000 + om/11u << 1, vn
  1001 01mm mmmn nn11 . 101f 0011 10oo oooo                                              [mem] ld_pz 20500000 + om/11u << 1, vn
  1001 0100 000n nnnn . 101f 0100 0000 0000                                              [mem] ld_b (rm), rn
  1001 01mm mmmn nnnn . 101f 0100 0000 001u                                              [mem] ld_b (xy), rn
  1001 0100 000n nnnn . 101f 0100 0000 0100                                              [mem] ld_w (rm), rn
  1001 01mm mmmn nnnn . 101f 0100 0000 011u                                              [mem] ld_w (xy), rn
  1001 0100 000n nnnn . 101f 0100 0000 101u                                              [mem] ld_s (xy), rn
  1001 0100 000n nn00 . 101f 0100 0000 1100                                              [mem] ld_sv (rm), rn
  1001 01mm mmmn nn00 . 101f 0100 0000 111u                                              [mem] ld_sv (xy), rn
  1001 0100 000n nn00 . 101f 0100 0001 0000                                              [mem] ld_v (rm), rn
  1001 01mm mmmn nn00 . 101f 0100 0001 001u                                              [mem] ld_v (xy), rn
  1001 0100 000n nn00 . 101f 0100 0001 0100                                              [mem] ld_p (rm), rn
  1001 01mm mmmn nn00 . 101f 0100 0001 011u                                              [mem] ld_p (xy), rn
  1001 0100 000n nn00 . 101f 0100 0001 1000                                              [mem] ld_pz (rm), rn
  1001 01mm mmmn nn00 . 101f 0100 0001 101u                                              [mem] ld_pz (xy), rn
  1001 01mm mmmn nn00 . 101f 0101 0000 0000                                              [mem] mirror rm, rn
  1001 01nn nnnv vvvv . 101f 0110 0www wwww                                              [mem] mv_s #wv/12s, rn
  1001 01mm mmmn nnnn . 101f 1010 00oo oooo                                              [mem] st_s rn, 20000000 + om/11u << 2
  1001 01mm mmmn nnnn . 101f 1010 01oo oooo                                              [mem] st_s rn, 20100000 + om/11u << 2
  1001 01mm mmmn nnnn . 101f 1010 10oo oooo                                              [mem] st_s rn, 20500000 + om/11u << 2
  1001 01mm mmmm nn00 . 101f 1011 00oo oooo                                              [mem] st_sv vn, 20000000 + om/11u << 3
  1001 01mm mmmm nn00 . 101f 1011 01oo oooo                                              [mem] st_sv vn, 20100000 + om/11u << 3
  1001 01mm mmmm nn00 . 101f 1011 10oo oooo                                              [mem] st_sv vn, 20500000 + om/11u << 3
  1001 01mm mmmm nn01 . 101f 1011 00oo oooo                                              [mem] st_v vn, 20000000 + om/11u << 4
  1001 01mm mmmm nn01 . 101f 1011 01oo oooo                                              [mem] st_v vn, 20100000 + om/11u << 4
  1001 01mm mmmm nn01 . 101f 1011 10oo oooo                                              [mem] st_v vn, 20500000 + om/11u << 4
  1001 01mm mmmm nn10 . 101f 1011 00oo oooo                                              [mem] st_p vn, 20000000 + om/11u << 1
  1001 01mm mmmm nn10 . 101f 1011 01oo oooo                                              [mem] st_p vn, 20100000 + om/11u << 1
  1001 01mm mmmm nn10 . 101f 1011 10oo oooo                                              [mem] st_p vn, 20500000 + om/11u << 1
  1001 01mm mmmm nn11 . 101f 1011 00oo oooo                                              [mem] st_pz vn, 20000000 + om/11u << 1
  1001 01mm mmmm nn11 . 101f 1011 01oo oooo                                              [mem] st_pz vn, 20100000 + om/11u << 1
  1001 01mm mmmm nn11 . 101f 1011 10oo oooo                                              [mem] st_pz vn, 20500000 + om/11u << 1
  1001 0100 000n nnnn . 101f 1100 0000 101u                                              [mem] st_s rn, (xy)
  1001 01nn nnnm mm00 . 101f 1100 0000 1100                                              [mem] st_sv vm, (rn)
  1001 0100 000n nn00 . 101f 1100 0000 111u                                              [mem] st_sv vn, (xy)
  1001 01nn nnnm mm00 . 101f 1100 0001 0000                                              [mem] st_v vm, (rn)
  1001 0100 000n nn00 . 101f 1100 0001 001u                                              [mem] st_v vn, (xy)
  1001 01nn nnnm mm00 . 101f 1100 0001 0100                                              [mem] st_p vm, (rn)
  1001 0100 000n nn00 . 101f 1100 0001 011u                                              [mem] st_p vn, (xy)
  1001 01nn nnnm mm00 . 101f 1100 0001 1000                                              [mem] st_pz vm, (rn)
  1001 0100 000n nn00 . 101f 1100 0001 101u                                              [mem] st_pz vn, (xy)
  1001 01mm mmmv vvvv . 101f 111o ooow wwww                                              [mem] st_s #wv/10u, 20500000 + om/9 << 4

  1001 10oo ooom mmmm . 101f 0000 000n nnnn                                              [alu] add ro, rm, rn
  1001 10vv vvvm mmmm . 101f 0000 001n nnnn                                              [alu] add #v/5u, rm, rn
  1001 10vv vvvn nnnn . 101f 0000 010w wwww                                              [alu] add #wv/10u, rn
  1001 10vv vvvn nnnn . 101f 0000 011d dddd                                              [alu] add #v/5u, >>#d/5s, rn
  1001 10mm mmmn nnnn . 101f 0000 101d dddd                                              [alu] add rm, >>#d/5s, rn
  1001 10oo o00m mm00 . 101f 0000 110n nn00                                              [alu] add_sv vo, vm, vn
  1001 10oo o00m mm00 . 101f 0000 111n nn00                                              [alu] add_p vo, vm, vn
  1001 10oo ooom mmmm . 101f 0001 000n nnnn                                              [alu] sub ro, rm, rn
  1001 10vv vvvm mmmm . 101f 0001 001n nnnn                                              [alu] sub #v/5u, rm, rn
  1001 10vv vvvn nnnn . 101f 0001 010w wwww                                              [alu] sub #wv/10u, rn
  1001 10vv vvvn nnnn . 101f 0001 011d dddd                                              [alu] sub #v/5u, >>#d/5s, rn
  1001 10vv vvvm mmmm . 101f 0001 100n nnnn                                              [alu] sub rm, #v/5u, rn
  1001 10mm mmmn nnnn . 101f 0001 101d dddd                                              [alu] sub rm, >>#d/5s, rn
  1001 10oo o00m mm00 . 101f 0001 110n nn00                                              [alu] sub_sv vo, vm, vn
  1001 10oo o00m mm00 . 101f 0001 111n nn00                                              [alu] sub_p vo, vm, vn
  1001 10vv vvvn nnnn . 101f 0010 010w wwww                                              [alu] cmp #wv/10u, rn
  1001 10vv vvvn nnnn . 101f 0010 011d dddd                                              [alu] cmp #v/5u, >>#d/5s, rn
  1001 10nn nnnv vvvv . 101f 0010 1000 0000                                              [alu] cmp rn, #v/5u
  1001 10mm mmmn nnnn . 101f 0010 101d dddd                                              [alu] cmp rm, >>#d/5s, rn
  1001 10oo ooom mmmm . 101f 0011 000n nnnn                                              [alu] and ro, rm, rn
  1001 10vv vvvm mmmm . 101f 0011 001n nnnn                                              [alu] and #v/5s, rm, rn
  1001 10vv vvvn nnnn . 101f 0011 010d dddd                                              [alu] and #v/5s, <>#d/5s, rn
  1001 10nn nnnm mmmm . 101f 0011 011v vvvv                                              [alu] and #v/5s, >>rm, rn
  1001 10oo ooon nnnn . 101f 0011 100d dddd                                              [alu] and ro, >>#d/5s, rn
  1001 10oo ooom mmmm . 101f 0011 101n nnnn                                              [alu] and ro, >>rm, rn
  1001 10oo ooom mmmm . 101f 0011 110n nnnn                                              [alu] and ro, <>rm, rn
  1001 10mm mmmn nnnn . 101f 0100 0000 0000                                              [alu] ftst rm, rn
  1001 10vv vvvn nnnn . 101f 0100 0010 0000                                              [alu] ftst #v/5s, rn
  1001 10vv vvvn nnnn . 101f 0100 010d dddd                                              [alu] ftst #v/5s, <>#d/5s, rn
  1001 10nn nnnm mmmm . 101f 0100 011v vvvv                                              [alu] ftst #v/5s, >>rm, rn
  1001 10oo ooon nnnn . 101f 0100 100d dddd                                              [alu] ftst ro, >>#d/5s, rn
  1001 10oo ooom mmmm . 101f 0100 101n nnnn                                              [alu] ftst ro, >>rm, rn
  1001 10oo ooom mmmm . 101f 0100 110n nnnn                                              [alu] ftst ro, <>rm, rn
  1001 10oo ooom mmmm . 101f 0101 000n nnnn                                              [alu] or ro, rm, rn
  1001 10oo ooom mmmm . 101f 0101 001n nnnn                                              [alu] or #v/5s, rm, rn
  1001 10vv vvvn nnnn . 101f 0101 010d dddd                                              [alu] or #v/5s, <>#d/5s, rn
  1001 10nn nnnm mmmm . 101f 0101 011v vvvv                                              [alu] or #v/5s, >>rm, rn
  1001 10oo ooon nnnn . 101f 0101 100d dddd                                              [alu] or ro, >>#d/5s, rn
  1001 10oo ooom mmmm . 101f 0101 101n nnnn                                              [alu] or ro, >>rm, rn
  1001 10oo ooom mmmm . 101f 0101 110n nnnn                                              [alu] or ro, <>rm, rn
  1001 10oo ooom mmmm . 101f 0110 000n nnnn                                              [alu] eor ro, rm, rn
  1001 10oo ooom mmmm . 101f 0110 001n nnnn                                              [alu] eor #v/5s, rm, rn
  1001 10vv vvvn nnnn . 101f 0110 010d dddd                                              [alu] eor #v/5s, <>#d/5s, rn
  1001 10nn nnnm mmmm . 101f 0110 011v vvvv                                              [alu] eor #v/5s, >>rm, rn
  1001 10oo ooon nnnn . 101f 0110 100d dddd                                              [alu] eor ro, >>#d/5s, rn
  1001 10oo ooom mmmm . 101f 0110 101n nnnn                                              [alu] eor ro, >>rm, rn
  1001 10oo ooom mmmm . 101f 0110 110n nnnn                                              [alu] eor ro, <>rm, rn
  1001 10mm mmmo oooo . 101f 0111 000n nnnn                                              [alu] as >>ro, rm, rn
  1001 10mm mmmn nnnn . 101f 0111 001d dddd                                              [alu] asl #(32 - (d/5u) & 31, rm, rn
  1001 10mm mmmn nnnn . 101f 0111 010d dddd                                              [alu] asr #d/5u, rm, rn
  1001 10mm mmmo oooo . 101f 0111 011n nnnn                                              [alu] ls >>ro, rm, rn
  1001 10mm mmmn nnnn . 101f 0111 100d dddd                                              [alu] lsr #d/5u, rm, rn
  1001 10mm mmmo oooo . 101f 0111 101n nnnn                                              [alu] rot ro, rm, rn
  1001 10mm mmmn nnnn . 101f 0111 110d dddd                                              [alu] rot #d/5u, rm, rn
  1001 10nn nnnm mmmm . 101f 1000 000v vvvv                                              [alu] bits #v/5u, >>rm, rn
  1001 10nn nnnd dddd . 101f 1000 001v vvvv                                              [alu] bits #v/5u, >>#d/5u, rn
  1001 10oo ooom mmmm . 101f 1000 010n nnnn                                              [alu] butt ro, rm, rn
  1001 10mm mmmn nnnn . 101f 1000 011v vvvv                                              [alu] sat #v/5u + 1, rm, rn
  1001 10nn nnnm mmmm . 101f 1001 1000 0000                                              [alu] msb rm, rn
  1001 10oo ooom mmmm . 101f 1100 000n nnnn                                              [alu] addwc ro, rm, rn
  1001 10vv vvvm mmmm . 101f 1100 001n nnnn                                              [alu] addwc #v/5u, rm, rn
  1001 10vv vvvn nnnn . 101f 1100 010w wwww                                              [alu] addwc #wv/10u, rn
  1001 10vv vvvn nnnn . 101f 1100 011d dddd                                              [alu] addwc #v/5u, >>#d/5s, rn
  1001 10mm mmmn nnnn . 101f 1100 101d dddd                                              [alu] addwc rm, >>#d/5s, rn
  1001 10oo ooom mmmm . 101f 1101 000n nnnn                                              [alu] subwc ro, rm, rn
  1001 10vv vvvm mmmm . 101f 1101 001n nnnn                                              [alu] subwc #v/5u, rm, rn
  1001 10vv vvvn nnnn . 101f 1101 010w wwww                                              [alu] subwc #wv/10u, rn
  1001 10vv vvvn nnnn . 101f 1101 011d dddd                                              [alu] subwc #v/5u, >>#d/5s, rn
  1001 10mm mmmv vvvv . 101f 1101 100n nnnn                                              [alu] subwc rm, #v/5u, rn
  1001 10mm mmmn nnnn . 101f 1101 101d dddd                                              [alu] subwc rm, >>#d/5s, rn
  1001 10mm mmmn nnnn . 101f 1110 0000 0000                                              [alu] cmpwc rm, rn
  1001 10vv vvvn nnnn . 101f 1110 010w wwww                                              [alu] cmpwc #wv/10u, rn
  1001 10vv vvvn nnnn . 101f 1110 011d dddd                                              [alu] cmpwc #v/5u, >>#d/5s, rn
  1001 10nn nnnv vvvv . 101f 1110 1000 0000                                              [alu] cmpwc rm, #v/5u
  1001 10mm mmmn nnnn . 101f 1110 101d dddd                                              [alu] cmpwc rm, >>#d/5s, rn

  1001 11mm mmmo oooo . 101f 0000 000n nnnn                                              [mul] mul ro, rm, >>acshift, rn
  1001 11nn nnnm mmmm . 101f 0000 1ddd dddd                                              [mul] mul rm, rn, >>#d/7s, rn
  1001 11nn nnnm mmmm . 101f 0001 000o oooo                                              [mul] mul rm, rn, >>ro, rn
  1001 11mm mmmv vvvv . 101f 0001 100n nnnn                                              [mul] mul #v/5u, rm, >>acshift, rn
  1001 11nn nnnv vvvv . 101f 0010 0ddd dddd                                              [mul] mul #v/5u, rn, >>#d/7s, rn
  1001 11nn nnnv vvvv . 101f 0010 100m mmmm                                              [mul] mul #v/5u, rn, >>rm, rn
  1001 11oo ooom mm00 . 101f 0100 000n nn00                                              [mul] mul_sv ro, vm, >>svshift, vn
  1001 11oo ooom mm00 . 101f 0100 1ssn nn00                                              [mul] mul_sv ro, vm, >>svs, vn
  1001 11mm m000 0000 . 101f 0101 000n nn00                                              [mul] mul_sv ru, vm, >>svshift, vn
  1001 11mm m000 0000 . 101f 0101 1ssn nn00                                              [mul] mul_sv ru, vm, >>svs, vn
  1001 11mm m000 0000 . 101f 0110 000n nn00                                              [mul] mul_sv rv, vm, >>svshift, vn
  1001 11mm m000 0000 . 101f 0110 1ssn nn00                                              [mul] mul_sv rv, vm, >>svs, vn
  1001 11mm m00o oo00 . 101f 0111 000n nn00                                              [mul] mul_sv vo, vm, >>svshift, vn
  1001 11mm m00o oo00 . 101f 0111 1ssn nn00                                              [mul] mul_sv vo, vm, >>svs, vn
  1001 11oo ooom mm00 . 101f 1000 000n nn00                                              [mul] mul_p ro, vm, >>svshift, vn
  1001 11oo ooom mm00 . 101f 1000 1ssn nn00                                              [mul] mul_p ro, vm, >>svs, vn
  1001 11mm m000 0000 . 101f 1001 000n nn00                                              [mul] mul_p ru, vm, >>svshift, vn
  1001 11mm m000 0000 . 101f 1001 1ssn nn00                                              [mul] mul_p ru, vm, >>svs, vn
  1001 11mm m000 0000 . 101f 1010 000n nn00                                              [mul] mul_p rv, vm, >>svshift, vn
  1001 11mm m000 0000 . 101f 1010 1ssn nn00                                              [mul] mul_p rv, vm, >>svs, vn
  1001 11mm m00o oo00 . 101f 1011 000n nn00                                              [mul] mul_p vo, vm, >>svshift, vn
  1001 11mm m00o oo00 . 101f 1011 1ssn nn00                                              [mul] mul_p vo, vm, >>svs, vn
  1001 11oo ooom mm00 . 101f 1100 000n nnnn                                              [mul] dotp ro, vm, >>svshift, rn
  1001 11oo ooom mm00 . 101f 1100 1ssn nnnn                                              [mul] dotp ro, vm, >>#svs, rn
  1001 11oo o00m mm00 . 101f 1101 000n nnnn                                              [mul] dotp vo, vm, >>svshift, rn
  1001 11oo o00m mm00 . 101f 1101 1ssn nnnn                                              [mul] dotp vo, vm, >>#svs, rn
  1001 11mm mmmo oooo . 101f 1110 100n nnnn                                              [mul] addm ro, rm, rn
  1001 11mm mmmo oooo . 101f 1111 000n nnnn                                              [mul] subm ro, rm, rn
*/

const nuon_disassembler::reginfo nuon_disassembler::reginfos[] = {
	{ 0x20500000, "mpectl"          },
	{ 0x20500010, "excepsrc"        },
	{ 0x20500020, "excepclr"        },
	{ 0x20500030, "excephalten"     },
	{ 0x20500040, "cc"              },
	{ 0x20500050, "pcfetch"         },
	{ 0x20500060, "pcroute"         },
	{ 0x20500070, "pcexec"          },
	{ 0x20500080, "rz"              },
	{ 0x20500090, "rzi1"            },
	{ 0x205000a0, "rzi2"            },
	{ 0x205000b0, "intvec1"         },
	{ 0x205000c0, "intvec1"         },
	{ 0x205000d0, "intsrc"          },
	{ 0x205000e0, "intclr"          },
	{ 0x205000f0, "intctl"          },
	{ 0x20500100, "inten1"          },
	{ 0x20500110, "inten1set"       },
	{ 0x20500120, "inten1clr"       },
	{ 0x20500130, "inten1sel"       },
	{ 0x205001e0, "rc0"             },
	{ 0x205001f0, "rc1"             },
	{ 0x20500200, "rx"              },
	{ 0x20500210, "ry"              },
	{ 0x20500220, "xyrange"         },
	{ 0x20500230, "xybase"          },
	{ 0x20500240, "xyctl"           },
	{ 0x20500250, "ru"              },
	{ 0x20500260, "rv"              },
	{ 0x20500270, "uvrange"         },
	{ 0x20500280, "uvbase"          },
	{ 0x20500290, "uvctl"           },
	{ 0x205002a0, "linpixctl"       },
	{ 0x205002b0, "clutbase"        },
	{ 0x205002c0, "svshift"         },
	{ 0x205002d0, "acshift"         },
	{ 0x205002e0, "sp"              },
	{ 0x205002f0, "dabreak"         },
	{ 0x20500300, "r0"              },
	{ 0x20500310, "r1"              },
	{ 0x20500320, "r2"              },
	{ 0x20500330, "r3"              },
	{ 0x20500340, "r4"              },
	{ 0x20500350, "r5"              },
	{ 0x20500360, "r6"              },
	{ 0x20500370, "r7"              },
	{ 0x20500380, "r8"              },
	{ 0x20500390, "r9"              },
	{ 0x205003a0, "r10"             },
	{ 0x205003b0, "r11"             },
	{ 0x205003c0, "r12"             },
	{ 0x205003d0, "r13"             },
	{ 0x205003e0, "r14"             },
	{ 0x205003f0, "r15"             },
	{ 0x20500400, "r16"             },
	{ 0x20500410, "r17"             },
	{ 0x20500420, "r18"             },
	{ 0x20500430, "r19"             },
	{ 0x20500440, "r20"             },
	{ 0x20500450, "r21"             },
	{ 0x20500460, "r22"             },
	{ 0x20500470, "r23"             },
	{ 0x20500480, "r24"             },
	{ 0x20500490, "r25"             },
	{ 0x205004a0, "r26"             },
	{ 0x205004b0, "r27"             },
	{ 0x205004c0, "r28"             },
	{ 0x205004d0, "r29"             },
	{ 0x205004e0, "r30"             },
	{ 0x205004f0, "r31"             },
	{ 0x20500500, "odmactl"         },
	{ 0x20500510, "odmacptr"        },
	{ 0x20500600, "mdmactl"         },
	{ 0x20500610, "mdmacptr"        },
	{ 0x205007e0, "comminfo"        },
	{ 0x205007f0, "commctl"         },
	{ 0x20500800, "commxmit"        },
	{ 0x20500810, "commrecv"        },
	{ 0x20500ff0, "configa"         },
	{ 0x20500ff4, "configb"         },
	{ 0x20500ff8, "dcachectl"       },
	{ 0x20500ffc, "icachectl"       },
	{ 0x20501100, "vdmactla"        },
	{ 0x20501110, "vdmactlb"        },
	{ 0x20501120, "vdmaptra"        },
	{ 0x20501130, "vdmaptrb"        },

	// MPE 2 only
	{ 0x20501200, "vldcmd"          },
	{ 0x20501210, "vldmode"         },
	{ 0x20501220, "vldmblock"       },
	{ 0x20501230, "vldbp"           },
	{ 0x20501240, "vldbits"         },
	{ 0x20501250, "vldctrl"         },
	{ 0x20501260, "vldresetiqvalid" },
	{ 0x20501300, "vldstatus"       },
	{ 0x20501310, "vlddata"         },
	{ 0x20501320, "vldzeros"        },
	{ 0x20501330, "vlddebug1"       },
	{ 0x20501340, "vlddebug3"       },
	{ 0x20501340, "vlddebug3"       },
};

u32 nuon_disassembler::opcode_alignment() const
{
	return 2;
}

offs_t nuon_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	std::string res;
	offs_t bpc = pc;
	bool cont = true;
	for(int count = 0; count < 5 && cont; count++) {
		if(!res.empty())
			res = res + " ; ";
		std::string packet = parse_packet(opcodes, pc, bpc, cont);
		res += packet;
	}
	stream << res;
	return pc - bpc;
}

s32 nuon_disassembler::s2i(u32 val, int bits) const
{
	if(bits != 32 && (val & (1 << (bits-1))))
		val |= 0xffffffff << bits;
	return val;
}

std::string nuon_disassembler::s2x(s32 val, int bits) const
{
	if(bits != 32 && (val & (1 << (bits-1))))
		val |= 0xffffffff << bits;

	int digits = (((bits - 1) | 3) + 1) >> 2;
	if(val < -10)
		return util::string_format("-0x%0*x", digits, u32(-val));
	else if(val < 10)
		return util::string_format("%d", val);
	else
		return util::string_format("0x%0*x", digits, val);
}

std::string nuon_disassembler::u2x(u32 val, int bits) const
{
	if(val < 10)
		return util::string_format("%d", val);
	int digits = (((bits - 1) | 3) + 1) >> 2;
	return util::string_format("0x%0*x", digits, val);
}

std::string nuon_disassembler::cc(u16 val, bool comma) const
{
	static const char *const conds[32] = {
		"ne", "c0eq", "c1eq", "cc", "eq", "cs", "vc", "vs",
		"lt", "mvc", "mvs", "hi", "le", "ls", "pl", "mi",
		"gt", "t", "modmi", "modpl", "ge", "modge", "modlt", "?17",
		"c0ne", "?19", "?1a", "cf0lo", "c1ne", "cf0hi", "cf1lo", "cf1hi"
	};

	if(val == 0x11)
		return ""; // t
	if(comma)
		return std::string(conds[val]) + ", ";
	else
		return ' ' + std::string(conds[val]);
}

std::string nuon_disassembler::rx(u16 sel) const
{
	static const char *const regs[4] = { "rx", "ry", "ru", "rv" };
	return regs[sel &3];
}

std::string nuon_disassembler::dec(u16 opc, std::string r) const
{
	if(opc & 2) {
		if(!r.empty())
			r += " ; ";
		r += "dec rc0";
	}
	if(opc & 1) {
		if(!r.empty())
			r += " ; ";
		r += "dec rc1";
	}
	return r;
}

std::string nuon_disassembler::xy(u16 sel) const
{
	static const char *const regs[2] = { "(uv)", "(xy)" };
	return regs[sel];
}

u32 nuon_disassembler::svs(u16 sel) const
{
	static const u32 shifts[4] = { 16, 24, 32, 30 };
	return shifts[sel];
}

bool nuon_disassembler::m(u16 val, u16 mask, u16 test) const
{
	return (val & mask) == test;
}

u32 nuon_disassembler::b(u16 val, int start, int count, int target)
{
	u32 v1 = (val & (((1 << count) - 1) << start));
	if(target < start)
		return v1 >> (start - target);
	else if(target > start)
		return v1 << (target - start);
	else
		return v1;
}

std::string nuon_disassembler::reg(u32 adr) const
{
	const unsigned int reg_count = sizeof(reginfos) / sizeof(reginfo);
	const unsigned int last_index = reg_count - 1;
	const unsigned int fill1  = last_index | (last_index >> 1);
	const unsigned int fill2  = fill1 | (fill1 >> 2);
	const unsigned int fill4  = fill2 | (fill2 >> 4);
	const unsigned int fill8  = fill4 | (fill4 >> 8);
	const unsigned int fill16 = fill8 | (fill8 >> 16);
	const unsigned int ppow2  = fill16 - (fill16 >> 1);
	unsigned int slot = ppow2;
	unsigned int step = ppow2;
	while(step) {
		if(slot > last_index)
			slot = slot ^ (step | (step >> 1));
		else {
			u32 radr = reginfos[slot].adr;
			if(adr == radr)
				return reginfos[slot].name;
			if(adr > radr)
				slot = slot | (step >> 1);
			else
				slot = slot ^ (step | (step >> 1));
		}
		step = step >> 1;
	}

	if(reginfos[slot].adr == adr)
		return reginfos[slot].name;

	return util::string_format("0x%08x", adr);
}

std::string nuon_disassembler::parse_packet(const data_buffer &opcodes, offs_t &pc, offs_t bpc, bool &cont)
{
	u16 opc1 = opcodes.r16(pc);

	if(m(opc1, 0xc000, 0x0000)) {
		// 16-bits alu
		cont = false;
		pc += 2;
		if(m(opc1, 0xfc00, 0x0000)) return util::string_format("add r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x0400)) return util::string_format("add #%s, r%d", u2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc63, 0x0c00)) return util::string_format("add_sv v%d, v%d", b(opc1, 7, 3, 0), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc63, 0x0c01)) return util::string_format("sub_sv v%d, v%d", b(opc1, 7, 3, 0), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc1f, 0x0c02)) return util::string_format("neg r%d", b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc1f, 0x0c03)) return util::string_format("abs r%d", b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x1000)) return util::string_format("sub r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x1400)) return util::string_format("sub #%s, r%d", u2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x1800)) return util::string_format("eor #%s, r%d", s2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x1c00)) return util::string_format("cmp r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x2000)) return util::string_format("cmp #%s, r%d", u2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x2400)) return util::string_format("and r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x2800)) return util::string_format("or r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x2c00)) return util::string_format("eor r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x3000)) return util::string_format("asl #%d, r%d", (32 - b(opc1, 0, 5, 0)) & 31, b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x3400)) return util::string_format("asr #%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x3800)) return util::string_format("asr #%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x3c00)) return util::string_format("btst #%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));

		return util::string_format("?%04x", opc1);
	}

	if(m(opc1, 0x4000, 0x4000)) {
		// 16-bits normal non-alu
		cont = !(opc1 & 0x8000);
		pc += 2;

		// 16-bits mul
		if(m(opc1, 0x7c00, 0x4000)) return util::string_format("mul r%d, r%d, >>acshift, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0), b(opc1, 5, 5, 0));

		// 16-bits mem
		if(m(opc1, 0x7c00, 0x4800)) return util::string_format("ld_s (r%d), r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0x7c00, 0x4c00)) return util::string_format("st_s r%d, (r%d)", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0x7c00, 0x5000)) return util::string_format("ld_s %s, r%d", reg(0x20500000 | b(opc1, 5, 5, 4)), b(opc1, 0, 5, 0));
		if(m(opc1, 0x7c00, 0x5400)) return util::string_format("st_s r%d, %s", b(opc1, 0, 5, 0), reg(0x20500000 | b(opc1, 5, 5, 4)));
		if(m(opc1, 0x7c00, 0x5800)) return util::string_format("mv_s r%d, (r%d)", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0x7c00, 0x5c00)) return util::string_format("mv_s #%s, (r%d)", u2x(b(opc1, 0, 5, 0), 5), b(opc1, 5, 5, 0));
		if(m(opc1, 0x7c63, 0x6000)) return util::string_format("mv_v v%d, v%d", b(opc1, 2, 3, 0), b(opc1, 7, 3, 0));
		if(m(opc1, 0x7fe3, 0x6020)) return util::string_format("push v%d", b(opc1, 2, 3, 0));
		if(m(opc1, 0x7fe3, 0x6120)) return util::string_format("push v%d. rz", b(opc1, 2, 3, 0));
		if(m(opc1, 0x7fe0, 0x6220)) return util::string_format("push r%d, cc, rzi1, rz", b(opc1, 0, 5, 0));
		if(m(opc1, 0x7fe0, 0x6320)) return util::string_format("push r%d, cc, rzi2, rz", b(opc1, 0, 5, 0));
		if(m(opc1, 0x7fe3, 0x6040)) return util::string_format("pop v%d", b(opc1, 2, 3, 0));
		if(m(opc1, 0x7fe3, 0x6140)) return util::string_format("pop v%d. rz", b(opc1, 2, 3, 0));
		if(m(opc1, 0x7fe0, 0x6240)) return util::string_format("pop r%d, cc, rzi1, rz", b(opc1, 0, 5, 0));
		if(m(opc1, 0x7fe0, 0x6340)) return util::string_format("pop r%d, cc, rzi2, rz", b(opc1, 0, 5, 0));

		// 16-bits ecu
		if(m(opc1, 0x7c00, 0x6800)) return util::string_format("bra %s%s", cc(b(opc1, 7, 3, 2), true), reg(bpc + s2i(b(opc1, 0, 7, 1), 8)));
		if(m(opc1, 0x7c00, 0x6c00)) return util::string_format("bra %s%s", cc(b(opc1, 7, 3, 2), true), reg(bpc + s2i(b(opc1, 0, 7, 1), 8)));
		if(m(opc1, 0x7fff, 0x7001)) return util::string_format("halt");
		if(m(opc1, 0x7c1f, 0x7010)) return util::string_format("rts%s", cc(b(opc1, 5, 5, 0), false));
		if(m(opc1, 0x7c1f, 0x7011)) return util::string_format("rts %snop", cc(b(opc1, 5, 5, 0), true));
		if(m(opc1, 0x7c1f, 0x7012)) return util::string_format("rti %s(rzi1)", cc(b(opc1, 5, 5, 0), true));
		if(m(opc1, 0x7c1f, 0x7013)) return util::string_format("rti %s(rzi1), nop", cc(b(opc1, 5, 5, 0), true));
		if(m(opc1, 0x7c1f, 0x7014)) return util::string_format("rti %s(rzi2)", cc(b(opc1, 5, 5, 0), true));
		if(m(opc1, 0x7c1f, 0x7015)) return util::string_format("rti %s(rzi2), nop", cc(b(opc1, 5, 5, 0), true));

		// 16-bits rcu
		if(m(opc1, 0x7c04, 0x7400)) return dec(opc1, util::string_format("addr r%d, %s", b(opc1, 5, 5, 0), rx(b(opc1, 3, 2, 0))));
		if(m(opc1, 0x7c04, 0x7404)) return dec(opc1, util::string_format("addr %s << 16, %s", u2x(b(opc1, 5, 5, 0), 5), rx(b(opc1, 3, 2, 0))));
		if(m(opc1, 0x7c04, 0x7800)) return dec(opc1, util::string_format("mvr r%d, %s", b(opc1, 5, 5, 0), rx(b(opc1, 3, 2, 0))));
		if(m(opc1, 0x7fe4, 0x7804)) return dec(opc1, util::string_format("modulo %s", rx(b(opc1, 3, 2, 0))));
		if(m(opc1, 0x7ffc, 0x7904)) return dec(opc1);
		if(m(opc1, 0x7fe4, 0x7a04)) return dec(opc1, util::string_format("range %s", rx(b(opc1, 3, 2, 0))));

		return util::string_format("?%04x", opc1);
	}

	if(m(opc1, 0xf800, 0x8000)) {
		// 16-bits special
		pc += 2;
		cont = m(opc1, 0x0100, 0x0000);
		if(m(opc1, 0xfeff, 0x8000)) return util::string_format("nop");
		if(m(opc1, 0xfeff, 0x8200)) return util::string_format("breakpoint");

		return util::string_format("?%04x", opc1);
	}

	if(m(opc1, 0xf000, 0x9000)) {
		// 32-bits instruction
		u16 opc2 = opcodes.r16(pc+2);
		cont = m(opc2, 0xf000, 0xa000);
		pc += 4;

		// 32-bit ecu instructions
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xee00, 0xa000)) return util::string_format("bra %s%s", cc(b(opc1, 5, 5, 0), true), reg(bpc + s2i(b(opc2, 0, 9, 6) | b(opc1, 0, 5, 1), 15)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xef80, 0xa200)) return util::string_format("jmp %s%s", cc(b(opc1, 5, 5, 0), true), reg(0x20200000 | b(opc2, 0, 7, 6) | b(opc1, 0, 5, 1)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xef80, 0xa300)) return util::string_format("jmp %s%s", cc(b(opc1, 5, 5, 0), true), reg(0x20300000 | b(opc2, 0, 7, 6) | b(opc1, 0, 5, 1)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xef80, 0xa400)) return util::string_format("jsr %s%s", cc(b(opc1, 5, 5, 0), true), reg(0x20200000 | b(opc2, 0, 7, 6) | b(opc1, 0, 5, 1)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xef80, 0xa500)) return util::string_format("jsr %s%s", cc(b(opc1, 5, 5, 0), true), reg(0x20300000 | b(opc2, 0, 7, 6) | b(opc1, 0, 5, 1)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xefff, 0xa680)) return util::string_format("jmp %s(r%d)", cc(b(opc1, 5, 5, 0), true), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xefff, 0xa700)) return util::string_format("jsr %s(r%d)", cc(b(opc1, 5, 5, 0), true), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xee00, 0xa800)) return util::string_format("bra %s%s, nop", cc(b(opc1, 5, 5, 0), true), reg(bpc + s2i(b(opc2, 0, 9, 6) | b(opc1, 0, 5, 1), 15)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xef80, 0xaa00)) return util::string_format("jmp %s%s, nop", cc(b(opc1, 5, 5, 0), true), reg(0x20200000 | b(opc2, 0, 7, 6) | b(opc1, 0, 5, 1)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xef80, 0xab00)) return util::string_format("jmp %s%s, nop", cc(b(opc1, 5, 5, 0), true), reg(0x20300000 | b(opc2, 0, 7, 6) | b(opc1, 0, 5, 1)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xef80, 0xac00)) return util::string_format("jsr %s%s, nop", cc(b(opc1, 5, 5, 0), true), reg(0x20200000 | b(opc2, 0, 7, 6) | b(opc1, 0, 5, 1)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xef80, 0xad00)) return util::string_format("jsr %s%s, nop", cc(b(opc1, 5, 5, 0), true), reg(0x20300000 | b(opc2, 0, 7, 6) | b(opc1, 0, 5, 1)));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xefff, 0xae80)) return util::string_format("jmp %s(r%d), nop", cc(b(opc1, 5, 5, 0), true), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9000) && m(opc2, 0xefff, 0xaf00)) return util::string_format("jsr %s(r%d), nop", cc(b(opc1, 5, 5, 0), true), b(opc1, 0, 5, 0));

		// 32-bit mem instructions
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xa000)) return util::string_format("ld_b %s, r%d", reg(0x20000000 | b(opc2, 0, 6, 5) | b(opc1, 5, 5, 0)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xa040)) return util::string_format("ld_b %s, r%d", reg(0x20100000 | b(opc2, 0, 6, 5) | b(opc1, 5, 5, 0)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xa080)) return util::string_format("ld_b %s, r%d", reg(0x20500000 | b(opc2, 0, 6, 5) | b(opc1, 5, 5, 0)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xa100)) return util::string_format("ld_w %s, r%d", reg(0x20000000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xa140)) return util::string_format("ld_w %s, r%d", reg(0x20100000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xa180)) return util::string_format("ld_w %s, r%d", reg(0x20500000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xa200)) return util::string_format("ld_s %s, r%d", reg(0x20000000 | b(opc2, 0, 6, 7) | b(opc1, 5, 5, 2)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xa240)) return util::string_format("ld_s %s, r%d", reg(0x20100000 | b(opc2, 0, 6, 7) | b(opc1, 5, 5, 2)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xa280)) return util::string_format("ld_s %s, r%d", reg(0x20500000 | b(opc2, 0, 6, 7) | b(opc1, 5, 5, 2)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefc0, 0xa300)) return util::string_format("ld_sv %s, v%d", reg(0x20000000 | b(opc2, 0, 6, 8) | b(opc1, 5, 5, 3)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefc0, 0xa340)) return util::string_format("ld_sv %s, v%d", reg(0x20100000 | b(opc2, 0, 6, 8) | b(opc1, 5, 5, 3)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefc0, 0xa380)) return util::string_format("ld_sv %s, v%d", reg(0x20500000 | b(opc2, 0, 6, 8) | b(opc1, 5, 5, 3)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9401) && m(opc2, 0xefc0, 0xa300)) return util::string_format("ld_v %s, v%d", reg(0x20000000 | b(opc2, 0, 6, 9) | b(opc1, 5, 5, 4)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9401) && m(opc2, 0xefc0, 0xa340)) return util::string_format("ld_v %s, v%d", reg(0x20100000 | b(opc2, 0, 6, 9) | b(opc1, 5, 5, 4)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9401) && m(opc2, 0xefc0, 0xa380)) return util::string_format("ld_v %s, v%d", reg(0x20500000 | b(opc2, 0, 6, 9) | b(opc1, 5, 5, 4)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9402) && m(opc2, 0xefc0, 0xa300)) return util::string_format("ld_p %s, v%d", reg(0x20000000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9402) && m(opc2, 0xefc0, 0xa340)) return util::string_format("ld_p %s, v%d", reg(0x20100000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9402) && m(opc2, 0xefc0, 0xa380)) return util::string_format("ld_p %s, v%d", reg(0x20500000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9403) && m(opc2, 0xefc0, 0xa300)) return util::string_format("ld_pz %s, v%d", reg(0x20000000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9403) && m(opc2, 0xefc0, 0xa340)) return util::string_format("ld_pz %s, v%d", reg(0x20100000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9403) && m(opc2, 0xefc0, 0xa380)) return util::string_format("ld_pz %s, v%d", reg(0x20500000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefff, 0xa400)) return util::string_format("ld_b r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xffe0, 0x9400) && m(opc2, 0xeffe, 0xa402)) return util::string_format("ld_b %s, r%d", xy(b(opc2, 0, 1, 0)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefff, 0xa404)) return util::string_format("ld_w r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xffe0, 0x9400) && m(opc2, 0xeffe, 0xa406)) return util::string_format("ld_w %s, r%d", xy(b(opc2, 0, 1, 0)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xffe0, 0x9400) && m(opc2, 0xeffe, 0xa40a)) return util::string_format("ld_s %s, r%d", xy(b(opc2, 0, 1, 0)), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefff, 0xa40c)) return util::string_format("ld_sv r%d, %d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0));
		if(m(opc1, 0xffe3, 0x9400) && m(opc2, 0xeffe, 0xa40e)) return util::string_format("ld_sv %s, v%d", xy(b(opc2, 0, 1, 0)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefff, 0xa410)) return util::string_format("ld_v r%d, %d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0));
		if(m(opc1, 0xffe3, 0x9400) && m(opc2, 0xeffe, 0xa412)) return util::string_format("ld_v %s, v%d", xy(b(opc2, 0, 1, 0)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefff, 0xa414)) return util::string_format("ld_p r%d, %d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0));
		if(m(opc1, 0xffe3, 0x9400) && m(opc2, 0xeffe, 0xa416)) return util::string_format("ld_p %s, v%d", xy(b(opc2, 0, 1, 0)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefff, 0xa418)) return util::string_format("ld_pz r%d, %d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0));
		if(m(opc1, 0xffe3, 0x9400) && m(opc2, 0xeffe, 0xa41a)) return util::string_format("ld_pz %s, v%d", xy(b(opc2, 0, 1, 0)), b(opc1, 2, 3, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefff, 0xa500)) return util::string_format("mirror r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xef80, 0xa600)) return util::string_format("mv_s #%s, %s", s2x(b(opc2, 0, 7, 5) | b(opc1, 0, 5, 0), 12), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xaa00)) return util::string_format("st_s r%d, %s", b(opc1, 0, 5, 0), reg(0x20000000 | b(opc2, 0, 6, 7) | b(opc1, 5, 5, 2)));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xaa40)) return util::string_format("st_s r%d, %s", b(opc1, 0, 5, 0), reg(0x20100000 | b(opc2, 0, 6, 7) | b(opc1, 5, 5, 2)));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xefc0, 0xaa80)) return util::string_format("st_s r%d, %s", b(opc1, 0, 5, 0), reg(0x20500000 | b(opc2, 0, 6, 7) | b(opc1, 5, 5, 2)));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefc0, 0xab00)) return util::string_format("st_sv v%d, %s", b(opc1, 2, 3, 0), reg(0x20000000 | b(opc2, 0, 6, 8) | b(opc1, 5, 5, 3)));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefc0, 0xab40)) return util::string_format("st_sv v%d, %s", b(opc1, 2, 3, 0), reg(0x20100000 | b(opc2, 0, 6, 8) | b(opc1, 5, 5, 3)));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefc0, 0xab80)) return util::string_format("st_sv v%d, %s", b(opc1, 2, 3, 0), reg(0x20500000 | b(opc2, 0, 6, 8) | b(opc1, 5, 5, 3)));
		if(m(opc1, 0xfc03, 0x9401) && m(opc2, 0xefc0, 0xab00)) return util::string_format("st_v v%d, %s", b(opc1, 2, 3, 0), reg(0x20000000 | b(opc2, 0, 6, 9) | b(opc1, 5, 5, 4)));
		if(m(opc1, 0xfc03, 0x9401) && m(opc2, 0xefc0, 0xab40)) return util::string_format("st_v v%d, %s", b(opc1, 2, 3, 0), reg(0x20100000 | b(opc2, 0, 6, 9) | b(opc1, 5, 5, 4)));
		if(m(opc1, 0xfc03, 0x9401) && m(opc2, 0xefc0, 0xab80)) return util::string_format("st_v v%d, %s", b(opc1, 2, 3, 0), reg(0x20500000 | b(opc2, 0, 6, 9) | b(opc1, 5, 5, 4)));
		if(m(opc1, 0xfc03, 0x9402) && m(opc2, 0xefc0, 0xab00)) return util::string_format("st_p v%d, %s", b(opc1, 2, 3, 0), reg(0x20000000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)));
		if(m(opc1, 0xfc03, 0x9402) && m(opc2, 0xefc0, 0xab40)) return util::string_format("st_p v%d, %s", b(opc1, 2, 3, 0), reg(0x20100000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)));
		if(m(opc1, 0xfc03, 0x9402) && m(opc2, 0xefc0, 0xab80)) return util::string_format("st_p v%d, %s", b(opc1, 2, 3, 0), reg(0x20500000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)));
		if(m(opc1, 0xfc03, 0x9403) && m(opc2, 0xefc0, 0xab00)) return util::string_format("st_pz v%d, %s", b(opc1, 2, 3, 0), reg(0x20000000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)));
		if(m(opc1, 0xfc03, 0x9403) && m(opc2, 0xefc0, 0xab40)) return util::string_format("st_pz v%d, %s", b(opc1, 2, 3, 0), reg(0x20100000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)));
		if(m(opc1, 0xfc03, 0x9403) && m(opc2, 0xefc0, 0xab80)) return util::string_format("st_pz v%d, %s", b(opc1, 2, 3, 0), reg(0x20500000 | b(opc2, 0, 6, 6) | b(opc1, 5, 5, 1)));
		if(m(opc1, 0xffe0, 0x9400) && m(opc2, 0xeffe, 0xac0a)) return util::string_format("st_s r%d, %s", b(opc1, 0, 5, 0), xy(b(opc2, 0, 1, 0)));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefff, 0xac0c)) return util::string_format("st_sv v%d, (r%d)", b(opc1, 2, 3, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xffe3, 0x9400) && m(opc2, 0xeffe, 0xac0e)) return util::string_format("st_sv v%d, %s", b(opc1, 2, 3, 0), xy(b(opc2, 0, 1, 0)));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefff, 0xac10)) return util::string_format("st_v v%d, (r%d)", b(opc1, 2, 3, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xffe3, 0x9400) && m(opc2, 0xeffe, 0xac12)) return util::string_format("st_v v%d, %s", b(opc1, 2, 3, 0), xy(b(opc2, 0, 1, 0)));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefff, 0xac14)) return util::string_format("st_p v%d, (r%d)", b(opc1, 2, 3, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xffe3, 0x9400) && m(opc2, 0xeffe, 0xac16)) return util::string_format("st_p v%d, %s", b(opc1, 2, 3, 0), xy(b(opc2, 0, 1, 0)));
		if(m(opc1, 0xfc03, 0x9400) && m(opc2, 0xefff, 0xac18)) return util::string_format("st_pz v%d, (r%d)", b(opc1, 2, 3, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xffe3, 0x9400) && m(opc2, 0xeffe, 0xac1a)) return util::string_format("st_pz v%d, %s", b(opc1, 2, 3, 0), xy(b(opc2, 0, 1, 0)));
		if(m(opc1, 0xfc00, 0x9400) && m(opc2, 0xee00, 0xae00)) return util::string_format("st_s #%s, %s", u2x(b(opc2, 0, 5, 5) | b(opc1, 0, 5, 0), 10), reg(0x20500000 | b(opc2, 5, 4, 9) | b(opc1, 5, 5, 4)));

		// 32-bit alu instructions
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa000)) return util::string_format("add r%d, r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa020)) return util::string_format("add #%s, r%d, r%d", u2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa040)) return util::string_format("add #%s, r%d", u2x(b(opc2, 0, 5, 5) | b(opc1, 5, 5, 0), 10), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa060)) return util::string_format("add #%s, >>#%d, r%d", u2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa0a0)) return util::string_format("add r%d, >>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc63, 0x9800) && m(opc2, 0xefe3, 0xa0c0)) return util::string_format("add_sv v%d, v%d, v%d", b(opc1, 7, 3, 0), b(opc1, 2, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc63, 0x9800) && m(opc2, 0xefe3, 0xa0e0)) return util::string_format("add_p v%d, v%d, v%d", b(opc1, 7, 3, 0), b(opc1, 2, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa100)) return util::string_format("sub r%d, r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa120)) return util::string_format("sub #%s, r%d, r%d", u2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa140)) return util::string_format("sub #%s, r%d", u2x(b(opc2, 0, 5, 5) | b(opc1, 5, 5, 0), 10), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa160)) return util::string_format("sub #%s, >>#%d, r%d", u2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa180)) return util::string_format("sub r%d, #%s, r%d", b(opc1, 5, 5, 0), u2x(b(opc1, 0, 5, 0), 5), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa1a0)) return util::string_format("sub r%d, >>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc63, 0x9800) && m(opc2, 0xefe3, 0xa1c0)) return util::string_format("sub_sv v%d, v%d, v%d", b(opc1, 7, 3, 0), b(opc1, 2, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc63, 0x9800) && m(opc2, 0xefe3, 0xa1e0)) return util::string_format("sub_p v%d, v%d, v%d", b(opc1, 7, 3, 0), b(opc1, 2, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa240)) return util::string_format("cmp #%s, r%d", u2x(b(opc2, 0, 5, 5) | b(opc1, 5, 5, 0), 10), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa260)) return util::string_format("cmp #%s, >>#%d, r%d", u2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefff, 0xa280)) return util::string_format("cmp r%d, #%s", b(opc1, 5, 5, 0), u2x(b(opc1, 0, 5, 0), 5));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa2a0)) return util::string_format("cmp r%d, >>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa300)) return util::string_format("and r%d, r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa320)) return util::string_format("and #%s, r%d, r%d", u2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa340)) return util::string_format("and #%s, <>#%d, r%d", u2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa360)) return util::string_format("and #%s, >>r%d, r%d", u2x(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa380)) return util::string_format("and r%d, >>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa3a0)) return util::string_format("and r%d, >>r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa3c0)) return util::string_format("and r%d, <>r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa400)) return util::string_format("ftst r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa420)) return util::string_format("ftst #%s, r%d", s2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa440)) return util::string_format("ftst #%s, <>#%d, r%d", s2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa460)) return util::string_format("ftst #%s, >>r%d, r%d", s2x(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa480)) return util::string_format("ftst r%d, <>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa4a0)) return util::string_format("ftst r%d, >>r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa4c0)) return util::string_format("ftst r%d, <>r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa500)) return util::string_format("or r%d, r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa520)) return util::string_format("or #%s, r%d, r%d", s2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa540)) return util::string_format("or #%s, <>#%d, r%d", s2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa560)) return util::string_format("or #%s, >>r%d, r%d", s2x(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa580)) return util::string_format("or r%d, <>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa5a0)) return util::string_format("or r%d, >>r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa5c0)) return util::string_format("or r%d, <>r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa600)) return util::string_format("eor r%d, r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa620)) return util::string_format("eor #%s, r%d, r%d", s2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa640)) return util::string_format("eor #%s, <>#%d, r%d", s2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa660)) return util::string_format("eor #%s, >>r%d, r%d", s2x(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa680)) return util::string_format("eor r%d, <>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa6a0)) return util::string_format("eor r%d, >>r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa6c0)) return util::string_format("eor r%d, <>r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa700)) return util::string_format("as >>r%d, r%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa720)) return util::string_format("asl #%s, r%d, r%d", u2x((32-b(opc2, 0, 5, 0)) & 31, 5), b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa740)) return util::string_format("asr #%s, r%d, r%d", u2x(b(opc2, 0, 5, 0), 5), b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa760)) return util::string_format("ls >>r%d, r%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa780)) return util::string_format("lsr #%s, r%d, r%d", u2x(b(opc2, 0, 5, 0), 5), b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa7a0)) return util::string_format("rot r%d, r%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa7c0)) return util::string_format("rot #%s, r%d, r%d", u2x(b(opc2, 0, 5, 0), 5), b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa800)) return util::string_format("bits #%s, >>r%d, r%d", u2x(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa820)) return util::string_format("bits #%s, >>#%d, r%d", u2x(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa840)) return util::string_format("butt r%d, r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xa860)) return util::string_format("sat #%d, r%d, r%d", 1+b(opc2, 0, 5, 0), b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefff, 0xa980)) return util::string_format("msb r%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xac00)) return util::string_format("addwc r%d, r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xac20)) return util::string_format("addwc #%s, r%d, r%d", u2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xac40)) return util::string_format("addwc #%s, r%d", u2x(b(opc2, 0, 5, 5) | b(opc1, 5, 5, 0), 10), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xac60)) return util::string_format("addwc #%s, >>#%d, r%d", u2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xaca0)) return util::string_format("addwc r%d, >>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xad00)) return util::string_format("subwc r%d, r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xad20)) return util::string_format("subwc #%s, r%d, r%d", u2x(b(opc1, 5, 5, 0), 5), b(opc1, 0, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xad40)) return util::string_format("subwc #%s, r%d", u2x(b(opc2, 0, 5, 5) | b(opc1, 5, 5, 0), 10), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xad60)) return util::string_format("subwc #%s, >>#%d, r%d", u2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xad80)) return util::string_format("subwc r%d, #%s, r%d", b(opc1, 5, 5, 0), u2x(b(opc1, 0, 5, 0), 5), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xada0)) return util::string_format("subwc r%d, >>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefff, 0xae00)) return util::string_format("cmpwc r%d, r%d", b(opc1, 5, 5, 0), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xae40)) return util::string_format("cmpwc #%s, r%d", u2x(b(opc2, 0, 5, 5) | b(opc1, 5, 5, 0), 10), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xae60)) return util::string_format("cmpwc #%s, >>#%d, r%d", u2x(b(opc1, 5, 5, 0), 5), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefff, 0xae80)) return util::string_format("cmpwc r%d, #%s", b(opc1, 5, 5, 0), u2x(b(opc1, 0, 5, 0), 5));
		if(m(opc1, 0xfc00, 0x9800) && m(opc2, 0xefe0, 0xaea0)) return util::string_format("cmpwc r%d, >>#%d, r%d", b(opc1, 5, 5, 0), s2i(b(opc2, 0, 5, 0), 5), b(opc1, 0, 5, 0));

		// 32-bits mul instructions
		if(m(opc1, 0xfc00, 0x9c00) && m(opc2, 0xefe0, 0xa000)) return util::string_format("mul r%d, r%d, >>acshift, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9c00) && m(opc2, 0xef80, 0xa080)) return util::string_format("mul r%d, r%d, >>%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0), s2i(b(opc2, 0, 7, 0), 7), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9c00) && m(opc2, 0xefe0, 0xa100)) return util::string_format("mul r%d, r%d, >>acshift, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0), b(opc2, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9c00) && m(opc2, 0xefe0, 0xa180)) return util::string_format("mul #%s, r%d, >>acshift, r%d", u2x(b(opc1, 0, 5, 0), 5), b(opc1, 5, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9c00) && m(opc2, 0xef80, 0xa200)) return util::string_format("mul #%s, r%d, >>%d, r%d", u2x(b(opc1, 0, 5, 0), 5), b(opc1, 5, 5, 0), s2i(b(opc2, 0, 7, 0), 7), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc00, 0x9c00) && m(opc2, 0xefe0, 0xa280)) return util::string_format("mul #%s, r%d, >>acshift, r%d", u2x(b(opc1, 0, 5, 0), 5), b(opc1, 5, 5, 0), b(opc2, 0, 5, 0), b(opc1, 5, 5, 0));
		if(m(opc1, 0xfc03, 0x9c00) && m(opc2, 0xefe3, 0xa400)) return util::string_format("mul_sv r%d, v%d, >>svshift, v%d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9c00) && m(opc2, 0xef83, 0xa480)) return util::string_format("mul_sv r%d, v%d, >>#%d, v%d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc7f, 0x9c00) && m(opc2, 0xefe3, 0xa500)) return util::string_format("mul_sv ru v%d, >>svshift, v%d", b(opc1, 7, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc7f, 0x9c00) && m(opc2, 0xef83, 0xa580)) return util::string_format("mul_sv ru, v%d, >>#%d, v%d", b(opc1, 7, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc7f, 0x9c00) && m(opc2, 0xefe3, 0xa600)) return util::string_format("mul_sv rv v%d, >>svshift, v%d", b(opc1, 7, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc7f, 0x9c00) && m(opc2, 0xef83, 0xa680)) return util::string_format("mul_sv rv, v%d, >>#%d, v%d", b(opc1, 7, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc63, 0x9c00) && m(opc2, 0xefe3, 0xa700)) return util::string_format("mul_sv v%d, v%d, >>svshift, v%d", b(opc1, 2, 3, 0), b(opc1, 7, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc63, 0x9c00) && m(opc2, 0xef83, 0xa780)) return util::string_format("mul_sv v%d, v%d, >>#%d, v%d", b(opc1, 2, 3, 0), b(opc1, 7, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9c00) && m(opc2, 0xefe3, 0xa800)) return util::string_format("mul_p r%d, v%d, >>svshift, v%d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9c00) && m(opc2, 0xef83, 0xa880)) return util::string_format("mul_p r%d, v%d, >>#%d, v%d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc7f, 0x9c00) && m(opc2, 0xefe3, 0xa900)) return util::string_format("mul_p ru v%d, >>svshift, v%d", b(opc1, 7, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc7f, 0x9c00) && m(opc2, 0xef83, 0xa980)) return util::string_format("mul_p ru, v%d, >>#%d, v%d", b(opc1, 7, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc7f, 0x9c00) && m(opc2, 0xefe3, 0xaa00)) return util::string_format("mul_p rv v%d, >>svshift, v%d", b(opc1, 7, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc7f, 0x9c00) && m(opc2, 0xef83, 0xaa80)) return util::string_format("mul_p rv, v%d, >>#%d, v%d", b(opc1, 7, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc63, 0x9c00) && m(opc2, 0xefe3, 0xab00)) return util::string_format("mul_p v%d, v%d, >>svshift, v%d", b(opc1, 2, 3, 0), b(opc1, 7, 3, 0), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc63, 0x9c00) && m(opc2, 0xef83, 0xab80)) return util::string_format("mul_p v%d, v%d, >>#%d, v%d", b(opc1, 2, 3, 0), b(opc1, 7, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 2, 3, 0));
		if(m(opc1, 0xfc03, 0x9c00) && m(opc2, 0xefe0, 0xac00)) return util::string_format("dotp r%d, v%d, >>svshift, r%d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc03, 0x9c00) && m(opc2, 0xef80, 0xac80)) return util::string_format("dotp r%d, v%d, >>#%d, r%d", b(opc1, 5, 5, 0), b(opc1, 2, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc03, 0x9c00) && m(opc2, 0xefe0, 0xad00)) return util::string_format("dotp v%d, v%d, >>svshift, r%d", b(opc1, 7, 3, 0), b(opc1, 2, 3, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc03, 0x9c00) && m(opc2, 0xef80, 0xad80)) return util::string_format("dotp v%d, v%d, >>#%d, r%d", b(opc1, 7, 3, 0), b(opc1, 2, 3, 0), svs(b(opc2, 5, 2, 0)), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9c00) && m(opc2, 0xefe0, 0xae80)) return util::string_format("addm r%d, r%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0), b(opc2, 0, 5, 0));
		if(m(opc1, 0xfc00, 0x9c00) && m(opc2, 0xefe0, 0xaf00)) return util::string_format("subm r%d, r%d, r%d", b(opc1, 0, 5, 0), b(opc1, 5, 5, 0), b(opc2, 0, 5, 0));

		return util::string_format("?%04x %04x", opc1, opc2);
	}

	if(m(opc1, 0xf800, 0x8800)) {
		// 32-bit prefix
		u16 opc2 = opcodes.r16(pc+2);
		u16 opc3 = opcodes.r16(pc+4);

		if(m(opc3, 0xc000, 0x0000)) {
			// 32+16-bits alu
			cont = false;
			pc += 6;
			if(m(opc3, 0xfc00, 0x0400))
				return util::string_format("add #%s, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 5, 5, 0), 32),
										   b(opc1, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x1400))
				return util::string_format("sub #%s, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 5, 5, 0), 32),
										   b(opc1, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x1800))
				return util::string_format("eor #%s, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 5, 5, 0), 32),
										   b(opc1, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x2000))
				return util::string_format("cmp #%s, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 5, 5, 0), 32),
										   b(opc1, 0, 5, 0));

			return util::string_format("?%04x %04x %04x", opc1, opc2, opc3);
		}

		if(m(opc3, 0x4000, 0x4000)) {
			// 32+16-bits normal non-alu
			cont = !(opc3 & 0x8000);
			pc += 6;

			// 32+16-bits mem
			if(m(opc3, 0x7c00, 0x5c00))
				return util::string_format("mv_s #%s, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc3, 5, 5, 0));
			// 32+16-bits ecu
			if(m(opc3, 0x7c00, 0x6800) && m(opc2, 0x0004, 0x0000))
				return util::string_format("bra %s%s",
										   cc(b(opc1, 7, 3, 2) | b(opc2, 0, 2, 0), true),
										   reg(bpc + s2i(b(opc1, 0, 11, 21) | b(opc2, 3, 13, 8) | b(opc3, 5, 7, 1), 32)));
			if(m(opc3, 0x7c00, 0x6800) && m(opc2, 0x0004, 0x0004))
				return util::string_format("bra %s%s, nop",
										   cc(b(opc1, 7, 3, 2) | b(opc2, 0, 2, 0), true),
										   reg(bpc + s2i(b(opc1, 0, 11, 21) | b(opc2, 3, 13, 8) | b(opc3, 5, 7, 1), 32)));

			// 32+16-bits rcu
			if(m(opc3, 0x7c40, 0x7404))
				return dec(opc3, util::string_format("addr #%s, %s",
													 s2x(b(opc1, 0, 11, 21) | b(opc2, 5, 5, 16) | b(opc2, 0, 16, 0), 32),
													 rx(b(opc3, 3, 2, 0))));
			if(m(opc3, 0x7c40, 0x7800))
				return dec(opc3, util::string_format("mvr #%s, %s",
													 s2x(b(opc1, 0, 11, 21) | b(opc2, 5, 5, 16) | b(opc2, 0, 16, 0), 32),
													 rx(b(opc3, 3, 2, 0))));

			return util::string_format("?%04x %04x %04x", opc1, opc2, opc3);
		}

		if(m(opc3, 0xf000, 0x9000)) {
			// 32+32-bit instructions

			u16 opc4 = opcodes.r16(pc+6);
			cont = m(opc4, 0xf000, 0xa000);
			pc += 8;

			// 32+32-bit ecu instructions
			if(m(opc3, 0xfc00, 0x9000) && m(opc4, 0xef00, 0xa200) && m(opc2, 0x01ff, 0x0000))
				return util::string_format("jmp %s%s",
										   cc(b(opc3, 5, 5, 0), true),
										   u2x(b(opc1, 0, 11, 21) | b(opc2, 9, 7, 14) | b(opc4, 0, 8, 6) | b(opc3, 0, 5, 1), 32));
			if(m(opc3, 0xfc00, 0x9000) && m(opc4, 0xef00, 0xa400) && m(opc2, 0x01ff, 0x0000))
				return util::string_format("jsr %s%s",
										   cc(b(opc3, 5, 5, 0), true),
										   u2x(b(opc1, 0, 11, 21) | b(opc2, 9, 7, 14) | b(opc4, 0, 8, 6) | b(opc3, 0, 5, 1), 32));
			if(m(opc3, 0xfc00, 0x9000) && m(opc4, 0xef00, 0xaa00) && m(opc2, 0x01ff, 0x0000))
				return util::string_format("jmp %s%s, nop",
										   cc(b(opc3, 5, 5, 0), true),
										   u2x(b(opc1, 0, 11, 21) | b(opc2, 9, 7, 14) | b(opc4, 0, 8, 6) | b(opc3, 0, 5, 1), 32));
			if(m(opc3, 0xfc00, 0x9000) && m(opc4, 0xef00, 0xac00) && m(opc2, 0x01ff, 0x0000))
				return util::string_format("jsr %s%s, nop",
										   cc(b(opc3, 5, 5, 0), true),
										   u2x(b(opc1, 0, 11, 21) | b(opc2, 9, 7, 14) | b(opc4, 0, 8, 6) | b(opc3, 0, 5, 1), 32));

			// 32+32-bit mem instructions
			if(m(opc3, 0xfc00, 0x9400) && m(opc4, 0xee00, 0xae00) && m(opc2, 0x0018, 0x0000))
				return util::string_format("st_s #%s, %s",
										   u2x(b(opc1, 0, 11, 21) | b(opc2, 5, 11, 10) | b(opc4, 5, 5, 5) | b(opc3, 0, 5, 0), 32),
										   reg(0x20000000 | b(opc2, 2, 1, 13) | b(opc4, 5, 4, 9) | b(opc3, 5, 5, 4) | b(opc2, 0, 2, 2)));
			if(m(opc3, 0xfc00, 0x9400) && m(opc4, 0xee00, 0xae00) && m(opc2, 0x0018, 0x0008))
				return util::string_format("st_s #%s, %s",
										   u2x(b(opc1, 0, 11, 21) | b(opc2, 5, 11, 10) | b(opc4, 5, 5, 5) | b(opc3, 0, 5, 0), 32),
										   reg(0x20100000 | b(opc2, 2, 1, 13) | b(opc4, 5, 4, 9) | b(opc3, 5, 5, 4) | b(opc2, 0, 2, 2)));
			if(m(opc3, 0xfc00, 0x9400) && m(opc4, 0xee00, 0xae00) && m(opc2, 0x0018, 0x0010))
				return util::string_format("st_s #%s, %s",
										   u2x(b(opc1, 0, 11, 21) | b(opc2, 5, 11, 10) | b(opc4, 5, 5, 5) | b(opc3, 0, 5, 0), 32),
										   reg(0x20500000 | b(opc2, 2, 1, 13) | b(opc4, 5, 4, 9) | b(opc3, 5, 5, 4) | b(opc2, 0, 2, 2)));

			// 32+32-bit alu instructions
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa020))
				return util::string_format("add #%s, r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 5, 5, 0), 32),
										   b(opc3, 0, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa120))
				return util::string_format("sub #%s, r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 5, 5, 0), 32),
										   b(opc3, 0, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa180))
				return util::string_format("sub r%d, #%s, r%d",
										   b(opc3, 5, 5, 0),
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefff, 0xa280))
				return util::string_format("cmp r%d, #%s",
										   b(opc3, 5, 5, 0),
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa320))
				return util::string_format("and #%s, r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc3, 5, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa360))
				return util::string_format("and #%s, >>r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc3, 5, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa420))
				return util::string_format("ftst #%s, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc3, 5, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa460))
				return util::string_format("ftst #%s, >>r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc3, 5, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa520))
				return util::string_format("or #%s, r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc3, 5, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa560))
				return util::string_format("or #%s, >>r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc3, 5, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa620))
				return util::string_format("eor #%s, r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc3, 5, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xa660))
				return util::string_format("eor #%s, >>r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc3, 5, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xac20))
				return util::string_format("addwc #%s, r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 5, 5, 0), 32),
										   b(opc3, 0, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xad20))
				return util::string_format("subwc #%s, r%d, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 5, 5, 0), 32),
										   b(opc3, 0, 5, 0), b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefe0, 0xad80))
				return util::string_format("subwc r%d, #%s, r%d",
										   b(opc3, 5, 5, 0),
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32),
										   b(opc4, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefff, 0xae20))
				return util::string_format("cmpwc #%s, r%d",
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 5, 5, 0), 32),
										   b(opc3, 0, 5, 0));
			if(m(opc3, 0xfc00, 0x9800) && m(opc4, 0xefff, 0xae80))
				return util::string_format("cmpwc r%d, #%s",
										   b(opc3, 5, 5, 0),
										   s2x(b(opc1, 0, 11, 21) | b(opc2, 0, 16, 5) | b(opc3, 0, 5, 0), 32));
			cont = false;
			return util::string_format("?%04x %04x %04x %04x", opc1, opc2, opc3, opc4);
		}

		cont = false;
		pc += 6;
		return util::string_format("?%04x %04x %04x", opc1, opc2, opc3);
	}

	pc += 2;
	cont = false;
	return util::string_format("?%04x", opc1);
}

