// license:BSD-3-Clause
// copyright-holders:Andreas Naive
/****************************************************************************
Namco System 10 decryption emulation

The decryption used by both System10 PCB types act on 16-bit words and is
designed to operate in a serial way: once the decryption is triggered, every
word is XORed with a mask calculated over data taken from the previous words
(both encrypted and decrypted). Type-1 (MEM-M) calculate the mask using the previous
cipherword and plainword, plus the 8 lowest bits of the word address. Type-2 (MEM-N)
boards dropped the use of the word address (maybe due to the serial protocol
used by the K9F2808U0B?), but expanded the number of previous cipherwords and
plainwords used up to three.

The only known type-1 game (mrdrilr2) has the encrypted data contained in them
[0x62000,0x380000] region of the first ROM of the game (1A).

In type-2 PCBs, the encrypted data is always contained in the first ROM of the
game (8E), and it's always stored spanning an integer number of NAND blocks
(the K9F2808U0B is organized in blocks of 16 KiB, each containing 32 pages of
0x200 bytes). Usually the first part of the encrypted data is stored at about the end
of the ROM, with all the blocks in that area processed in reverse order (first the
one nearest the end, then the second nearest, etc); the second part goes immediately
after it from a logic perspective, but it's, usually, physically located at the area
starting at 0x28000 in the ROM. However, in at least a couple of games, there are
out-of-order blocks (details below). Games, after
some bootup code has been executed, will copy the encrypted content from
the NANDs to RAM, moment at which the decryption is triggered. Physical locations
of the encrypted programs in the first NAND, together with the RAM region where
they are loaded, are summarized in the following table ( ' indicating processing
in reverse order of the constituting blocks) :

game        data regions                           RAM address
--------    ----------------------------------     -----------
chocovdr    [fdc000,1000000)' + [28000,1dc000)     80010000
gamshara    [fdc000,1000000)' + [28000,144000)     80010000
gjspace     [fd4000,ff8000)'  + [28000,80000)      80010000
            + [fd0000,fd4000) + [80000,200000)
knpuzzle    [fc8000,fcc000)'  + [28000,40c000)     80030000
            + [fc4000,fc8000) + [40c000,458000)
konotako    [fdc000,1000000)' + [28000,b4000)      80010000
mrdrilrg    [fd4000,fd8000)'  + [28000,3dc000)     80030000
nflclsfb    [fdc000,1000000)' + [28000,204000)     80010000
panikuru    [fdc000,fe0000)'  + [28000,ac000)      80030000
startrgn    [fdc000,1000000)' + [28000,b4000)      80010000

Both knpuzzle & gjspace present a NAND block which is out of order with respect
to the normal layout; besides, that block is physically located immediately before
the end-of-ROM region, in what maybe is an attempt to hinder the
recognition/reconstruction of the encrypted data.

Most games do a single decryption run, so the process is only initialized once;
however, at least four of them (mrdrilr2, gamshara, mrdrilrg & panikuru) do
reinitialize the internal state of the decryption several times. As of 2016-09-16,
only mrdrilr2 (type-1) and gamshara (type-2) show signs
of doing it by writing to the triggering register; how the others two are triggering the
reinitializations is still unclear. mrdrilr2 does a reinitialization every time the address
hits a 0x80000-bytes multiple); gamshara does a reinitialization every 5 NAND blocks
(16 times in total); mrdrilrg does the second one after 0x38000 bytes and then subsequent
ones every 32 blocks (8 times in total); panikuru does one every 2 blocks up to a total
of 16 times.

The calculation of the XOR masks operate in this way: most bits are
calculated by using linear equations over GF(2) taking as input data the bits from
previously processed words; however, one nonlinear calculation is performed
per word processed, and that calculation typically affect just one bit. The only
known exceptions are mrdrilr2 (type-1), where the counter is used in nonlinear terms
(but, like in type-2 ones, just one nonlinear term involving previous words is present),
and mrdrilrg (type-2), where the same nonlinear terms are
affecting two of them). Till now, all the formulae for type-2 games seem to depend only on the
previous 3 words, and the first mask after a (re-)initialization is always zero, so
chances are that the mask bits are calculated one word in advance, having access to the
current encrypted and decrypted words plus two further words in each sequence, maybe stored
in 32 bits registers. All the nonlinear terms reverse-engineered till now are of the form
A x B, where A and B are linear formulae; thus, as everything else in the schema involves
only linear relations, those nonlinear terms are probably caused by an Y-combinator taking
the resuls of two such linear relations as input, and deciding between both branches based
on another linear formula.

The bits affected by the nonlinear calculations in type-2 games are given below:
chocovdr  -> #10
gamshara  -> #2
gjspace   -> none
gunbalina -> #11
knpuzzle  -> #1
konotako  -> #15
mrdrilrg  -> #0 & #4
nflclsfb  -> #2
panikuru  -> #2
ptblank3  -> #11
startrgn  -> #4

Overall, the values used as linear masks, those from the INIT_SBOX and
the values and bit order used at initialization time are not expected to
be exactly the ones used by the hardware; given the many degrees of freedom
caused by the nature of the scheme, the whole set of values should
be considered as a representative of a class of equivalence of functionally
equivalent datasets, nothing else.


TO-DO:
* If further dumps support the theory of the calculations of type-2 games just depending
on 3 previous words, change the implementation accordingly to reflect that.

Observing the linear equations of type-2 games, there is a keen difference between bits using
just a bunch of previous bits, and others using much more bits from more words;
simplifying the latter ones could be handy, and probably closer to what the
hardware is doing. Two possible simplifications could be:
A) The linear relations are creating lots of identities involving the bits
from the sequence; they could be exploited to simplify the equations (but
only when the implementation be stable, to avoid duplicating work).
B) It's possible that some of those calculations are being stored and then
used as another input bits for subsequent masks. Determining that (supposed)
bits and factoring out them would simplify the expressions, in case they
really exist.
*****************************************************************************/

#include "emu.h"
#include "ns10crypt.h"

#include <utility>


DEFINE_DEVICE_TYPE(MRDRILR2_DECRYPTER, mrdrilr2_decrypter_device, "mrdrilr2_decrypter", "Mr Driller 2 decrypter")

DEFINE_DEVICE_TYPE(NS10_TYPE2_DECRYPTER, ns10_type2_decrypter_device, "ns10_type2_decrypter", "Namco System 10 Type 2 decrypter")
DEFINE_DEVICE_TYPE(NS10_TYPE2_DECRYPTER_NONLINEAR, ns10_type2_decrypter_nonlinear_device, "ns10_type2_decrypter_nonlinear", "Namco System 10 Type 2 decrypter (non-linear bit lookup table)")

// base class

ns10_decrypter_device::ns10_decrypter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
{
}

void ns10_decrypter_device::device_start()
{
	save_item(NAME(m_active));
}

void ns10_decrypter_device::device_reset()
{
	m_active = false;
}

void ns10_decrypter_device::activate(int iv)
{
	init(iv);
	m_active = true;
}

void ns10_decrypter_device::deactivate()
{
	m_active = false;
}

ns10_decrypter_device::~ns10_decrypter_device()
{
}

// type-1 decrypter

constexpr int UNKNOWN{16};
constexpr int U{UNKNOWN};
// this could perfectly be part of the per-game logic but, with only one known type-1 game, we cannot say anything definitive
const int ns10_type1_decrypter_device::INIT_SBOX[16]{U, U, U, 0, 4, 9, U, U, U, 8, U, 1, U, 9, U, 5};

ns10_type1_decrypter_device::ns10_type1_decrypter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ns10_decrypter_device(mconfig, type, tag, owner, clock)
{
}

void ns10_type1_decrypter_device::device_start()
{
	ns10_decrypter_device::device_start();

	save_item(NAME(m_mask));
	save_item(NAME(m_counter));
}

void ns10_type1_decrypter_device::device_reset()
{
	ns10_decrypter_device::device_reset();

	m_mask = 0;
	m_counter = 0;
}

uint16_t ns10_type1_decrypter_device::decrypt(uint16_t cipherword)
{
	uint16_t plainword = m_mask ^ bitswap<16>(cipherword, 9, 13, 15, 7, 14, 8, 6, 10, 11, 12, 3, 5, 0, 1, 4, 2);

	uint16_t nbs =
		((BIT(m_counter, 4)) << 15) ^
		((BIT(cipherword, 2) ^ BIT(cipherword, 5)) << 14) ^
		((BIT(cipherword, 0)) << 13) ^
		(((BIT(cipherword, 4) | BIT(cipherword, 5))) << 12) ^ // this is the only nonlinear term not involving the counter
		((BIT(m_counter, 0)) << 11) ^
		((BIT(cipherword, 6)) << 10) ^
		(((BIT(cipherword, 4) & BIT(m_counter, 1))) << 8) ^
		((BIT(m_counter, 3)) << 6) ^
		(((BIT(cipherword, 3) | BIT(m_counter, 7))) << 5) ^
		((BIT(cipherword, 2) ^ BIT(m_counter, 3)) << 4) ^
		((BIT(m_counter, 2)) << 3) ^
		(((BIT(cipherword, 7) & BIT(m_counter, 7))) << 2) ^
		((BIT(m_counter, 5)) << 1) ^
		(((BIT(cipherword, 7) | BIT(m_counter, 1))) << 0);
	m_mask = nbs ^ bitswap<16>(cipherword, 6, 11, 3, 1, 13, 5, 15, 10, 2, 9, 8, 4, 0, 12, 7, 14) ^ bitswap<16>(plainword, 9, 7, 5, 2, 14, 4, 13, 8, 0, 15, 10, 1, 3, 6, 12, 11) ^ 0xecbe;
	++m_counter;

	return plainword;
}

void ns10_type1_decrypter_device::init(int iv)
{
	m_mask = INIT_SBOX[iv];
	m_counter = 0;
}

mrdrilr2_decrypter_device::mrdrilr2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ns10_type1_decrypter_device(mconfig, MRDRILR2_DECRYPTER, tag, owner, clock)
{
}

// type-2 decrypter

// this could perfectly be part of the per-game logic; by now, only gamshara seems to use it, so we keep it global
const int ns10_type2_decrypter_device::INIT_SBOX[16]{0, 12, 13, 6, 2, 4, 9, 8, 11, 1, 7, 15, 10, 5, 14, 3};

ns10_type2_decrypter_device::ns10_type2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ns10_decrypter_device(mconfig, NS10_TYPE2_DECRYPTER, tag, owner, clock)
{
}

ns10_type2_decrypter_device::ns10_type2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, ns10_crypto_logic &&logic)
	: ns10_decrypter_device(mconfig, NS10_TYPE2_DECRYPTER, tag, owner, clock)
	, m_logic(std::move(logic))
{
	m_logic_initialized = true;
}

uint16_t ns10_type2_decrypter_device::decrypt(uint16_t cipherword)
{
	uint16_t const plainword = cipherword ^ m_mask;

	m_previous_cipherwords <<= 16;
	m_previous_cipherwords ^= cipherword;
	m_previous_plainwords <<= 16;
	m_previous_plainwords ^= plainword;

	m_mask = 0;
	for (int j = 15; j >= 0; --j)
	{
		m_mask <<= 1;
		m_mask ^= gf2_reduce(m_logic.eMask[j] & m_previous_cipherwords);
		m_mask ^= gf2_reduce(m_logic.dMask[j] & m_previous_plainwords);
	}
	m_mask ^= m_logic.xMask;
	m_mask ^= m_logic.nonlinear_calculation(m_previous_cipherwords, m_previous_plainwords);

	return plainword;
}

void ns10_type2_decrypter_device::init(int iv)
{
	if (m_logic.iv_calculation)
		m_previous_cipherwords = m_logic.iv_calculation(iv);
	else
		m_previous_cipherwords = bitswap(INIT_SBOX[iv], 3, 16, 16, 2, 1, 16, 16, 0, 16, 16, 16, 16, 16, 16, 16, 16);
	m_previous_plainwords = 0;
	m_mask = 0;
}

void ns10_type2_decrypter_device::device_start()
{
	ns10_decrypter_device::device_start();

	// If the logic isn't initialized then this will just fail, this is a programmer error
	if (!m_logic_initialized)
		fatalerror("ns10_type2_decrypter_device: Required logic data for decrypter device not initialized");

	save_item(NAME(m_mask));
	save_item(NAME(m_previous_cipherwords));
	save_item(NAME(m_previous_plainwords));
}

void ns10_type2_decrypter_device::device_reset()
{
	ns10_decrypter_device::device_reset();

	m_mask = 0;
	m_previous_cipherwords = 0;
	m_previous_plainwords = 0;
}

// type-2 decrypter with a table for the non-linear bits
const int ns10_type2_decrypter_nonlinear_device::INIT_SBOX[16]{0, 12, 13, 6, 2, 4, 9, 8, 11, 1, 7, 15, 10, 5, 14, 3};

ns10_type2_decrypter_nonlinear_device::ns10_type2_decrypter_nonlinear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ns10_decrypter_device(mconfig, NS10_TYPE2_DECRYPTER_NONLINEAR, tag, owner, clock)
	, m_nonlinear_region(*this, "nonlinear_table")
{
}

ns10_type2_decrypter_nonlinear_device::ns10_type2_decrypter_nonlinear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, ns10_crypto_logic &&logic)
	: ns10_decrypter_device(mconfig, NS10_TYPE2_DECRYPTER_NONLINEAR, tag, owner, clock)
	, m_nonlinear_region(*this, "nonlinear_table")
	, m_logic(std::move(logic))
{
	m_logic_initialized = true;
}

uint16_t ns10_type2_decrypter_nonlinear_device::decrypt(uint16_t cipherword)
{
	uint16_t const plainword = cipherword ^ m_mask;

	m_previous_cipherwords <<= 16;
	m_previous_cipherwords ^= cipherword;
	m_previous_plainwords <<= 16;
	m_previous_plainwords ^= plainword;

	m_mask = 0;
	for (int j = 15; j >= 0; --j)
	{
		m_mask <<= 1;
		m_mask ^= gf2_reduce(m_logic.eMask[j] & m_previous_cipherwords);
		m_mask ^= gf2_reduce(m_logic.dMask[j] & m_previous_plainwords);
	}
	m_mask ^= m_logic.xMask;

	uint8_t nonlinear_bit = BIT(m_nonlinear_region->base()[m_nonlinear_count / 8], 7 - (m_nonlinear_count % 8));
	m_nonlinear_count++;
	if (m_nonlinear_count >= m_nonlinear_region->bytes() * 8)
		m_nonlinear_count = 0;
	m_mask ^= m_logic.nonlinear_calculation(nonlinear_bit);

	return plainword;
}

void ns10_type2_decrypter_nonlinear_device::init(int iv)
{
	if (m_logic.iv_calculation)
		m_previous_cipherwords = m_logic.iv_calculation(iv);
	else
		m_previous_cipherwords = bitswap(INIT_SBOX[iv], 3, 16, 16, 2, 1, 16, 16, 0, 16, 16, 16, 16, 16, 16, 16, 16);
	m_previous_plainwords = 0;
	m_mask = 0;
}

void ns10_type2_decrypter_nonlinear_device::device_start()
{
	ns10_decrypter_device::device_start();

	// If the logic isn't initialized then this will just fail, this is a programmer error
	if (!m_logic_initialized)
		fatalerror("ns10_type2_decrypter_nonlinear_device: Required logic data for decrypter device not initialized");

	save_item(NAME(m_mask));
	save_item(NAME(m_previous_cipherwords));
	save_item(NAME(m_previous_plainwords));
	save_item(NAME(m_nonlinear_count));
}

void ns10_type2_decrypter_nonlinear_device::device_reset()
{
	ns10_decrypter_device::device_reset();

	m_mask = 0;
	m_previous_cipherwords = 0;
	m_previous_plainwords = 0;
	m_nonlinear_count = 0;
}
