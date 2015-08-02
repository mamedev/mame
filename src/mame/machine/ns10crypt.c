// license:BSD-3
// copyright-holders:Andreas Naive
/****************************************************************************
Namco System 10 decryption emulation

(As of 2015-08, this file is still pretty much a WIP; changes are expected as
out knowledge progress.)

The decryption used by type-2 System10 PCBs (MEM-N) acts on 16-bit words and is
designed to operate in a serial way: once the decryption is triggered, every
word is XORed with a mask calculated over data taken from the previous words 
(both encrypted and decrypted). Type-1 PCBs seem to use a similar
scheme, probably involving the word address too and a bitswap, but his relation
to what is described here needs further investigation.

In type-2 PCBs, the encrypted data is always contained in the first ROM of the
game (8E), and it's always stored spanning an integer number of NAND blocks
(the K9F2808U0B is organized in blocks of 16 KiB, each containing 32 pages of
0x200 bytes). The first part of the encrypted data is stored at about the end
of the ROM, and apparently all the blocks in that area are processed in
reverse order (first the one nearest the end, then the second nearest, etc);
the second part goes inmediately after it from a logic perspective, but it's
physically located at the area starting at 0x28000 in the ROM. Games, after
some bootup code has been executed, will copy the encrypted content from
the ROM to RAM, moment at which the decryption is triggered.

Most games do a single decryption run, so the process is only initialized once;
however, at least one game (gamshara) does write to the triggering registers
more than once, effectively resetting the internal state of the decrypter
several times. (gamshara do it every 5 NAND blocks).

The calculation of the XOR masks seem to operate this way: most bits are
calculated by using linear equations over GF(2) taking as input data the bits from
previously processed words; however, one nonlinear calculation is performed
per word processed, and that calculation can affect several bits from the
mask (but, apparently, the same nonlinear terms affect all of them),
though in most cases only one bit is involved. Till now, most of the linear
relations seem to depend only on the previous 3 words, but there are some
bits from those showing nonlinear behaviour which seem to use farther words;
this is still being investigated, and the implementation is probable to
change to reflect new findings.

The bits affected by the nonlinear calculations are given below:
chocovdr  -> #10
gamshara  -> #2
gjspace   -> a subset of {#3, #4, #5, #10, #11}, maybe all of them
gunbalina -> #11
knpuzzle  -> #1
konotako  -> #15
mrdrilrg  -> #0 & #4
nflclsfb  -> #2
panikuru  -> #2
ptblank3  -> #11
startrgn  -> #4


TO-DO:
* Research the nonlinear calculations in most of the games.
* Determine how many previous words the hardware is really using, and change
the implementation accordingly.

Observing the linear equations, there is a keen difference between bits using
just a bunch of previous bits, and others using much more bits from more words;
simplyfing the latter ones could be handy, and probably closer to what the
hardware is doing. Two possible simplyfications could be:
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

const device_type KONOTAKO_DECRYPTER = &device_creator<konotako_decrypter_device>;
const device_type STARTRGN_DECRYPTER = &device_creator<startrgn_decrypter_device>;

ns10_decrypter_device::ns10_decrypter_device(device_type type, const ns10_crypto_logic &logic, const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, "Namco System 10 Decrypter", tag, owner, clock, "ns10_crypto", __FILE__)
	, _active(false)
	, _logic(logic)
{
}

void ns10_decrypter_device::activate()
{
	init();
	_active = true;
}

void ns10_decrypter_device::deactivate()
{
	_active = false;
}

bool ns10_decrypter_device::is_active()const
{
	return _active;
}

UINT16 ns10_decrypter_device::decrypt(UINT16 cipherword)
{
	UINT16 plainword = cipherword ^ _mask;

	_previous_cipherwords <<= 16;
	_previous_cipherwords  ^= cipherword;
	_previous_plainwords  <<= 16;
	_previous_plainwords   ^= plainword;

	_mask = 0;
	for (int j = 15; j >= 0; --j)
	{
		_mask <<= 1;
		_mask ^= gf2_reduce(_logic.eMask[j] & _previous_cipherwords);
		_mask ^= gf2_reduce(_logic.dMask[j] & _previous_plainwords);
	}
	_mask ^= _logic.xMask;
	_mask ^= _logic.nonlinear_calculation(_previous_cipherwords, _previous_plainwords);

	return plainword;
}

void ns10_decrypter_device::device_start()
{
	int reduction;

	// create a look-up table of GF2 reductions of 16-bits words
	for (int i = 0; i < 0x10000; ++i)
	{
		reduction = 0;
		for (int j = 0; j < 16; ++j)
			reduction ^= BIT(i, j);

		_gf2Reduction[i] = reduction;
	}
}

void ns10_decrypter_device::init()
{
	_previous_cipherwords = 0;
	_previous_plainwords  = 0;
	_mask                 = 0;
}

int ns10_decrypter_device::gf2_reduce(UINT64 num)
{
	return
		_gf2Reduction[num & 0xffff]         ^
		_gf2Reduction[(num >> 16) & 0xffff] ^
		_gf2Reduction[(num >> 32) & 0xffff] ^
		_gf2Reduction[num >> 48];
}


// game-specific logic

static UINT16 konotako_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords)
{
	UINT64 previous_masks = previous_cipherwords ^ previous_plainwords;
	return ((previous_masks >> 7) & (previous_masks >> 15) & 1) << 15;
}

static const ns10_decrypter_device::ns10_crypto_logic konotako_crypto_logic = {
	{
		0x000000000000004cull, 0x00000000d39e3d3dull, 0x0000000000001110ull, 0x0000000000002200ull,
		0x000000003680c008ull, 0x0000000000000281ull, 0x0000000000005002ull, 0x00002a7371895a47ull,
		0x0000000000000003ull, 0x00002a7371897a4eull, 0x00002a73aea17a41ull, 0x00002a73fd895a4full,
		0x000000005328200aull, 0x0000000000000010ull, 0x0000000000000040ull, 0x0000000000000200ull,
	}, {
		0x000000000000008cull, 0x0000000053003d25ull, 0x0000000000001120ull, 0x0000000000002200ull,
		0x0000000037004008ull, 0x0000000000000282ull, 0x0000000000006002ull, 0x0000060035005a47ull,
		0x0000000000000003ull, 0x0000060035001a4eull, 0x0000060025007a41ull, 0x00000600b5005a2full,
		0x000000009000200bull, 0x0000000000000310ull, 0x0000000000001840ull, 0x0000000000000400ull,
	},
	0x0748,
	konotako_nonlinear_calc
};

static UINT16 startrgn_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords)
{
	UINT64 previous_masks = previous_cipherwords ^ previous_plainwords;
	return ((previous_masks >> 12) & (previous_masks >> 14) & 1) << 4;
}

static const ns10_decrypter_device::ns10_crypto_logic startrgn_crypto_logic = {
	{
		0x00003e4bfe92c6a9ull, 0x000000000000010cull, 0x00003e4b7bd6c4aaull, 0x0000b1a904b8fab8ull,
		0x0000000000000080ull, 0x0000000000008c00ull, 0x0000b1a9b2f0b4cdull, 0x000000006c100828ull,
		0x000000006c100838ull, 0x0000b1a9d3913fcdull, 0x000000006161aa00ull, 0x0000000000006040ull,
		0x0000000000000420ull, 0x0000000000001801ull, 0x00003e4b7bd6deabull, 0x0000000000000105ull,
	}, {
		0x000012021f00c6a8ull, 0x0000000000000008ull, 0x000012020b1046aaull, 0x000012001502fea8ull,
		0x0000000000002000ull, 0x0000000000008800ull, 0x000012001e02b4cdull, 0x000000002c0008aaull,
		0x000000002c00083aull, 0x000012003f027ecdull, 0x0000000021008a00ull, 0x0000000000002040ull,
		0x0000000000000428ull, 0x0000000000001001ull, 0x000012020b10ceabull, 0x0000000000000144ull,
	},
	0x8c46,
	startrgn_nonlinear_calc
};

// game-specific devices

konotako_decrypter_device::konotako_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ns10_decrypter_device(KONOTAKO_DECRYPTER, konotako_crypto_logic, mconfig, tag, owner, clock)
{
}

startrgn_decrypter_device::startrgn_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ns10_decrypter_device(STARTRGN_DECRYPTER, startrgn_crypto_logic, mconfig, tag, owner, clock)
{
}
