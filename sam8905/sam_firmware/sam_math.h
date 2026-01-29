#pragma once

/**
 * SAM8905 Controller Firmware - Math Utilities
 *
 * Fixed-point math functions optimized for 8-bit architectures.
 * These avoid direct 16-bit or 32-bit multiplication which may not
 * be available or efficient on all targets.
 *
 * DESIGN PRINCIPLE:
 * -----------------
 * All multiplication in this firmware uses these utilities. This allows:
 * 1. Easy replacement with assembly for specific targets (8051, Z80, etc.)
 * 2. Consistent handling of overflow and rounding
 * 3. Clear documentation of fixed-point formats
 *
 * On 8051, multiplication is 8×8→16 (MUL AB). Larger multiplies must be
 * built from multiple 8×8 operations. These functions encapsulate that.
 *
 * NAMING CONVENTION:
 * ------------------
 * mul_u8_u8()      - unsigned 8×8 → 16-bit result
 * mul_u8_u16_hi()  - unsigned 8×16 → high 16 bits of 24-bit result
 * mul_s8_u16_hi()  - signed 8 × unsigned 16 → signed high 16 bits
 */

#include <stdint.h>

/*============================================================================
 * 8×8 Multiplication (native on 8051)
 *============================================================================*/

/**
 * Unsigned 8×8 → 16-bit multiplication
 *
 * On 8051: MUL AB (single instruction)
 *
 * @param a  First operand (0-255)
 * @param b  Second operand (0-255)
 * @return   a × b (0-65025)
 */
static inline uint16_t mul_u8_u8(uint8_t a, uint8_t b)
{
    return (uint16_t)a * b;
}

/*============================================================================
 * 8×16 Multiplication (two 8×8 ops on 8051)
 *============================================================================*/

/**
 * Unsigned 8×16 → 24-bit multiplication, return high 16 bits
 *
 * Computes (a × b) >> 8, with rounding based on bit 7 of low byte.
 *
 * On 8051, this is implemented as:
 *   product_lo = a × b_lo           (MUL AB)
 *   product_hi = a × b_hi           (MUL AB)
 *   result = (product_lo >> 8) + product_hi + round_bit
 *
 * The round_bit is bit 7 of product_lo (improves accuracy).
 *
 * @param a    8-bit multiplier (0-255)
 * @param b    16-bit multiplicand
 * @return     (a × b) >> 8, rounded
 */
static inline uint16_t mul_u8_u16_hi(uint8_t a, uint16_t b)
{
    uint8_t b_lo = (uint8_t)(b & 0xFF);
    uint8_t b_hi = (uint8_t)(b >> 8);

    uint16_t product_lo = mul_u8_u8(a, b_lo);
    uint16_t product_hi = mul_u8_u8(a, b_hi);

    /* Round bit from product_lo[7] */
    uint8_t round_bit = (product_lo >> 7) & 1;

    /* Combine: result = product_hi + (product_lo >> 8) + round */
    uint16_t result = product_hi + (product_lo >> 8) + round_bit;

    return result;
}

/**
 * Unsigned 8×16 → full 24-bit multiplication
 *
 * Returns all 24 bits of the product (no shift or rounding).
 *
 * @param a    8-bit multiplier (0-255)
 * @param b    16-bit multiplicand
 * @return     a × b (full 24-bit result in 32-bit container)
 */
static inline uint32_t mul_u8_u16_full(uint8_t a, uint16_t b)
{
    uint8_t b_lo = (uint8_t)(b & 0xFF);
    uint8_t b_hi = (uint8_t)(b >> 8);

    uint16_t product_lo = mul_u8_u8(a, b_lo);
    uint16_t product_hi = mul_u8_u8(a, b_hi);

    /* Full result = product_hi << 8 + product_lo */
    return ((uint32_t)product_hi << 8) + product_lo;
}

/*============================================================================
 * Signed 8×16 Multiplication
 *============================================================================*/

/**
 * Signed 8 × unsigned 16 → signed 16-bit result (high bits)
 *
 * Computes (a × b) >> 8 where a is signed and b is unsigned.
 * Used for pitch bend and other signed offset calculations.
 *
 * @param a    Signed 8-bit multiplier (-128 to +127)
 * @param b    Unsigned 16-bit multiplicand
 * @return     (a × b) >> 8, signed
 */
static inline int16_t mul_s8_u16_hi(int8_t a, uint16_t b)
{
    if (a >= 0) {
        return (int16_t)mul_u8_u16_hi((uint8_t)a, b);
    } else {
        /* Negate, multiply, negate result */
        uint8_t abs_a = (uint8_t)(-a);
        uint16_t magnitude = mul_u8_u16_hi(abs_a, b);
        return -(int16_t)magnitude;
    }
}

/*============================================================================
 * 16×16 Multiplication (four 8×8 ops on 8051)
 *============================================================================*/

/**
 * Unsigned 16×16 → 32-bit multiplication
 *
 * On 8051, this requires four MUL AB operations:
 *   a_lo × b_lo, a_lo × b_hi, a_hi × b_lo, a_hi × b_hi
 *
 * Use sparingly - expensive on 8-bit targets.
 *
 * @param a    16-bit multiplier
 * @param b    16-bit multiplicand
 * @return     a × b (full 32-bit result)
 */
static inline uint32_t mul_u16_u16(uint16_t a, uint16_t b)
{
    uint8_t a_lo = (uint8_t)(a & 0xFF);
    uint8_t a_hi = (uint8_t)(a >> 8);
    uint8_t b_lo = (uint8_t)(b & 0xFF);
    uint8_t b_hi = (uint8_t)(b >> 8);

    uint16_t p0 = mul_u8_u8(a_lo, b_lo);  /* bits 0-15 */
    uint16_t p1 = mul_u8_u8(a_lo, b_hi);  /* bits 8-23 */
    uint16_t p2 = mul_u8_u8(a_hi, b_lo);  /* bits 8-23 */
    uint16_t p3 = mul_u8_u8(a_hi, b_hi);  /* bits 16-31 */

    /* Combine with proper alignment */
    uint32_t result = p0;
    result += (uint32_t)p1 << 8;
    result += (uint32_t)p2 << 8;
    result += (uint32_t)p3 << 16;

    return result;
}

/**
 * Unsigned 16×16 → high 16 bits (>>16)
 *
 * Computes (a × b) >> 16. Useful for fixed-point scaling.
 *
 * @param a    16-bit multiplier
 * @param b    16-bit multiplicand
 * @return     (a × b) >> 16
 */
static inline uint16_t mul_u16_u16_hi(uint16_t a, uint16_t b)
{
    return (uint16_t)(mul_u16_u16(a, b) >> 16);
}

/*============================================================================
 * Division Utilities
 *============================================================================*/

/**
 * Unsigned 16÷8 → 8-bit quotient with remainder
 *
 * On 8051: DIV AB (single instruction, but only 8÷8)
 * For 16÷8, we need a software routine.
 *
 * @param dividend  16-bit dividend
 * @param divisor   8-bit divisor (must be non-zero)
 * @param remainder Optional pointer to receive remainder
 * @return          dividend / divisor (truncated to 8 bits if overflow)
 */
static inline uint8_t div_u16_u8(uint16_t dividend, uint8_t divisor, uint8_t *remainder)
{
    if (divisor == 0) {
        if (remainder) *remainder = 0;
        return 0xFF;  /* Overflow/error */
    }

    uint16_t q = dividend / divisor;
    if (remainder) {
        *remainder = (uint8_t)(dividend % divisor);
    }

    /* Clamp to 8 bits */
    return (q > 255) ? 255 : (uint8_t)q;
}

/*============================================================================
 * Fixed-Point Utilities
 *============================================================================*/

/**
 * Scale a 24-bit value by a signed 8-bit factor
 *
 * Computes: result = base + (factor × (base >> 3)) >> 8
 *
 * This is the pitch table scaling formula:
 *   result ≈ base × (1 + factor/2048)
 *
 * Range: factor=+127 → +6.2%, factor=-128 → -6.2%
 *
 * @param base_lo   Low byte of 24-bit base value
 * @param base_mid  Mid byte of 24-bit base value
 * @param base_hi   High byte of 24-bit base value (only bits 0-2 used)
 * @param factor    Signed scaling factor (-128 to +127)
 * @param result_lo Pointer to receive result low byte
 * @param result_mid Pointer to receive result mid byte
 * @param result_hi Pointer to receive result high byte
 */
static inline void scale_pitch_24bit(
    uint8_t base_lo, uint8_t base_mid, uint8_t base_hi,
    int8_t factor,
    uint8_t *result_lo, uint8_t *result_mid, uint8_t *result_hi)
{
    /* Extract 16-bit multiplier from 24-bit base (base >> 3) */
    uint8_t mul_lo = (base_lo >> 3) | (base_mid << 5);
    uint8_t mul_hi = (base_mid >> 3) | (base_hi << 5) | (base_hi >> 3);
    uint16_t multiplier = ((uint16_t)mul_hi << 8) | mul_lo;

    /* Compute signed offset */
    int16_t offset = mul_s8_u16_hi(factor, multiplier);

    /* Add to 24-bit base */
    int32_t result = ((int32_t)base_hi << 16) | ((int32_t)base_mid << 8) | base_lo;
    result += offset;

    /* Store result, masking high byte to 3 bits */
    *result_lo = (uint8_t)(result & 0xFF);
    *result_mid = (uint8_t)((result >> 8) & 0xFF);
    *result_hi = (uint8_t)((result >> 16) & 0x07);
}
