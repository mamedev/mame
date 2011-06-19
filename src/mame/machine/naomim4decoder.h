#pragma once

#ifndef __NAOMIM4DECODER_H__
#define __NAOMIM4DECODER_H__

#include "emu.h"

// Decoder for M4-type NAOMI cart encryption

// In hardware, the decryption is managed by the XC3S50 Xilinx Spartan FPGA (IC2)
// and the annexed PIC16C621A PIC MCU (IC3).
// - The FPGA control the clock line of the security PIC.
// - The protocol between the FPGA and the MCU is nibble-based, though it hasn't been RE for now.
// - The decryption algorithm is clearly nibble-based too.

// The decryption algorithm itself implements a stream cipher built on top of a 16-bits block cipher.
// The underlying block-cipher is a SP-network of 2 rounds (both identical in structure). In every
// round, the substitution phase is done using 4 fixed 4-to-4 sboxes acting on every nibble. The permutation
// phase is indeed a nibble-based linear combination.
// With that block cipher, a stream cipher is constructed by feeding the output result of the 1st round
// of a certain 16-bits block as a whitening value for the next block. The cart dependent data used by
// the algorithm is comprised by a 16-bits "key" and a 16-bits IV (initialization vector) --though they
// will be merged in a only 32-bits number in the code--. The hardware auto-reset the feed value
// to the cart-based IV every 16 blocks (32 bytes); that reset is not address-based, but index-based.

class NaomiM4Decoder
{
public:
    NaomiM4Decoder(UINT32 cart_key);
    ~NaomiM4Decoder();

    void init();        // initialize the decryption of a new stream (set the IV)
    UINT16 decrypt(UINT16 ciphertext);      // decrypt the next 16-bits value in the stream

private:
    UINT16 one_round_core(UINT16 round_input);

    static const UINT8 k_sboxes[4][16];
    UINT16 *m_one_round;
    const UINT16 m_key;
    const UINT16 m_iv;
    UINT16 m_middle_value;  // used by the stream cipher as feeding to the next block
    UINT8 m_counter;
};

#endif // __NAOMIM4DECODER_H__
