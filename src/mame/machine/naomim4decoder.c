#include "naomim4decoder.h"

const UINT8 NaomiM4Decoder::k_sboxes[4][16] = {
    {13,14,1,11,7,9,10,0,15,6,4,5,8,2,12,3},
    {12,3,14,6,7,15,2,13,1,4,11,0,9,10,8,5},
    {6,12,0,10,1,5,14,9,7,2,15,13,4,11,3,8},
    {9,12,8,7,10,4,0,15,1,11,14,2,13,5,6,3}
};

NaomiM4Decoder::NaomiM4Decoder(UINT32 cart_key)
: m_key(cart_key & 0xffff)
, m_iv(cart_key >> 16)   // initialization vector
{
    m_one_round = global_alloc_array(UINT16,0x10000);

    // populate the lookup table for one of the internal rounds of the cipher
    for (UINT32 i=0; i<0x10000; ++i)
    {
        m_one_round[i] = one_round_core(i);
    }
}

NaomiM4Decoder::~NaomiM4Decoder()
{
    global_free(m_one_round);
}

void NaomiM4Decoder::init()
{
    m_counter = 0;
}

UINT16 NaomiM4Decoder::decrypt(UINT16 ciphertext)          // decrypt the next 16-bits value in the stream
{
    if (0 == (m_counter++ & 0xf))  // the iv is recovered every 16 values (32 bytes)
        m_middle_value = m_iv;

    UINT16 output_whitening = m_key ^ m_middle_value;

    m_middle_value = m_one_round[ciphertext ^ m_middle_value];

    return m_one_round[m_middle_value ^ m_key] ^ output_whitening;
}

UINT16 NaomiM4Decoder::one_round_core(UINT16 round_input)
{
    UINT8 input_nibble[4];
    UINT8 output_nibble[4];
    UINT8 aux_nibble;
    UINT8 nibble_idx;
    UINT8 i;
    UINT16 result;

    for (nibble_idx = 0; nibble_idx < 4; ++nibble_idx, round_input >>= 4)
    {
        input_nibble[nibble_idx] = round_input & 0xf;
        output_nibble[nibble_idx] = 0;
    }

    aux_nibble = input_nibble[3];
    for (nibble_idx = 0; nibble_idx < 4; ++nibble_idx)  // 4 s-boxes per round
    {
        aux_nibble ^= k_sboxes[nibble_idx][input_nibble[nibble_idx]];
        for (i = 0; i < 4; ++i)  // diffusion of the bits
            output_nibble[(nibble_idx - i) & 3] |= aux_nibble & (1 << i);
    }

    for (nibble_idx = 0, result = 0; nibble_idx < 4; ++nibble_idx)
        result |= (output_nibble[nibble_idx] << (4 * nibble_idx));

    return result;
}

