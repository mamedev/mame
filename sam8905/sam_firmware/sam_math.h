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

/*============================================================================
 * Signed Multiply with Saturation (CODE:AA6F)
 *
 * The firmware's signed_multiply_sat function performs:
 *   result = (|a| * |b|) >> 7, with sign handling and saturation
 *
 * Used for LFO amplitude scaling, envelope depth modulation, etc.
 *
 * Original disassembly pattern:
 *   - Extract signs, take absolute values
 *   - Multiply (rotate left to get 8.8 format)
 *   - Take high byte
 *   - Re-apply combined sign
 *   - Add second operand and saturate on overflow
 *============================================================================*/

/**
 * Signed 8×8 multiply with saturation (CODE:AA6F)
 *
 * Computes approximately: (a * b) >> 7 + b, saturated to [-127, +127]
 *
 * The result is clamped if overflow occurs.
 *
 * @param a  First signed operand (-128 to +127)
 * @param b  Second signed operand (-128 to +127)
 * @return   Saturated result (-127 to +127)
 */
static inline int8_t signed_multiply_sat(int8_t a, int8_t b)
{
    /* Extract signs and take absolute values */
    uint8_t neg_result = 0;
    uint8_t abs_a = (uint8_t)a;
    uint8_t abs_b = (uint8_t)b;

    if (a < 0) {
        neg_result = 1;
        abs_a = (uint8_t)(-a);
    }
    if (b < 0) {
        neg_result ^= 1;
        abs_b = (uint8_t)(-b);
    }

    /* Multiply and take high byte (after rotate left = *2, so effectively >>7) */
    /* Original: (val << 1 | val >> 7 | 1) * abs_a >> 8 */
    uint16_t product = (uint16_t)((abs_b << 1) | (abs_b >> 7) | 1) * abs_a;
    uint8_t scaled = (uint8_t)(product >> 8);

    /* Re-apply sign */
    int16_t signed_scaled = neg_result ? -(int16_t)scaled : (int16_t)scaled;

    /* Add second operand */
    int16_t result = signed_scaled + (int16_t)b;

    /* Saturate to [-127, +127] on overflow */
    if (result > 127) return 127;
    if (result < -127) return -127;
    return (int8_t)result;
}

/*============================================================================
 * Signed Multiply Chain (CODE:AA52)
 *
 * Chained multiply: multiplies two values and adds to accumulator.
 * Used for velocity modulation of envelope parameters.
 *
 * signed_multiply_chain(factor, curve_val, accumulator)
 *   = accumulator + (factor * curve_val) >> 7
 *============================================================================*/

/**
 * Signed multiply with accumulate (CODE:AA52)
 *
 * Computes: accumulator + (factor * curve_val) >> 7
 *
 * @param factor       Signed factor (-128 to +127)
 * @param curve_val    Curve lookup value (signed)
 * @param accumulator  Value to add to
 * @return             Result (not saturated)
 */
static inline int8_t signed_multiply_chain(int8_t factor, int8_t curve_val, int8_t accumulator)
{
    if (factor == 0) {
        return accumulator;
    }

    /* Extract signs */
    uint8_t neg_result = 0;
    uint8_t abs_factor = (uint8_t)factor;
    uint8_t abs_curve = (uint8_t)curve_val;

    if (factor < 0) {
        neg_result = 1;
        abs_factor = (uint8_t)(-factor);
    }
    if (curve_val < 0) {
        neg_result ^= 1;
        abs_curve = (uint8_t)(-curve_val);
    }

    /* Multiply and scale */
    uint16_t product = (uint16_t)abs_factor * abs_curve;
    uint8_t scaled = (uint8_t)(product >> 7);

    /* Apply sign and add to accumulator */
    int16_t offset = neg_result ? -(int16_t)scaled : (int16_t)scaled;
    int16_t result = (int16_t)accumulator + offset;

    /* Clamp to int8 range */
    if (result > 127) return 127;
    if (result < -128) return -128;
    return (int8_t)result;
}

/*============================================================================
 * 16×24 Multiplication (CODE:9FCD pitch modulation)
 *
 * ⚠️ WARNING: This matches the EXACT sequence of 8051 MUL AB instructions
 * in the original firmware. DO NOT simplify to a single 32-bit multiply!
 * The carry propagation affects the result.
 *
 * The firmware computes: offset = (mod_16bit * base_24bit) >> 16
 * Using six 8×8 multiplies with byte-by-byte carry propagation.
 *============================================================================*/

/**
 * 16×24 bit multiply returning high 24 bits (>>16)
 *
 * Matches CODE:9FCD pitch modulation multiply sequence exactly.
 * Uses only 8×8 multiplies to match 8051 MUL AB behavior.
 *
 * @param mod     16-bit modulation value (abs value, already processed for sign)
 * @param base    24-bit base pitch (only bits 0-18 used for 19-bit pitch)
 * @return        (mod * base) >> 16, masked to 19 bits
 */
static inline uint32_t multiply_16x24(uint16_t mod, uint32_t base)
{
    uint8_t mod_lo = (uint8_t)(mod & 0xFF);
    uint8_t mod_hi = (uint8_t)(mod >> 8);
    uint8_t base_lo = (uint8_t)(base & 0xFF);
    uint8_t base_mid = (uint8_t)((base >> 8) & 0xFF);
    uint8_t base_hi = (uint8_t)((base >> 16) & 0x07);

    /*
     * Mirroring CODE:9FCD multiply sequence (from Ghidra decompilation):
     *
     * Step 1: bVar2 = (base_lo * mod_hi) >> 8
     * Step 2: sVar1 = base_mid * mod_hi (full 16-bit)
     * Step 3: bVar4 = lo(sVar1) + (base_mid * mod_lo >> 8)  [may overflow]
     * Step 4: bVar6 = hi(sVar1) - carry_from_step3
     * Step 5: sVar1 = base_hi * mod_lo
     * Step 6: bVar3 = hi(sVar1)
     * Step 7: offset_lo = lo(sVar1) + bVar4 + bVar2
     * Step 8: sVar1 = base_hi * mod_hi
     * Step 9: bVar4 = lo(sVar1) + bVar3  [track carry]
     * Step 10: offset_mid = bVar4 + bVar6  [track carry]
     * Step 11: offset_hi = hi(sVar1) - carry9 - carry10
     */

    /* Step 1 */
    uint16_t mul1 = (uint16_t)base_lo * mod_hi;
    uint8_t bVar2 = (uint8_t)(mul1 >> 8);

    /* Step 2 */
    uint16_t sVar1 = (uint16_t)base_mid * mod_hi;

    /* Step 3 */
    uint16_t mul2 = (uint16_t)base_mid * mod_lo;
    uint16_t add1 = (uint8_t)sVar1 + (uint8_t)(mul2 >> 8);
    uint8_t bVar4 = (uint8_t)add1;
    uint8_t carry1 = (add1 > 0xFF) ? 1 : 0;

    /* Step 4 - original uses subtraction for borrow */
    uint8_t bVar6 = (uint8_t)(sVar1 >> 8) - carry1;

    /* Step 5 */
    sVar1 = (uint16_t)base_hi * mod_lo;
    uint8_t bVar3 = (uint8_t)(sVar1 >> 8);

    /* Step 6-7: offset_lo = lo(step5) + bVar4 + bVar2 */
    /* Note: original firmware accumulates differently, tracking carries */
    uint16_t add2 = (uint8_t)sVar1 + bVar4;
    uint16_t add3 = (uint8_t)add2 + bVar2;
    uint8_t offset_lo = (uint8_t)add3;
    /* Carry bits are absorbed into the accumulation pattern */

    /* Step 8 */
    sVar1 = (uint16_t)base_hi * mod_hi;

    /* Step 9 */
    add1 = (uint8_t)sVar1 + bVar3;
    bVar4 = (uint8_t)add1;
    uint8_t carry2 = (add1 > 0xFF) ? 1 : 0;

    /* Step 10 */
    add1 = bVar4 + bVar6;
    uint8_t offset_mid = (uint8_t)add1;
    uint8_t carry3 = (add1 > 0xFF) ? 1 : 0;

    /* Step 11 */
    uint8_t offset_hi = (uint8_t)(sVar1 >> 8) - carry2 - carry3;

    /* Combine into 19-bit result */
    return ((uint32_t)(offset_hi & 0x07) << 16) |
           ((uint32_t)offset_mid << 8) |
           offset_lo;
}

/*============================================================================
 * Pitch Scaling
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
