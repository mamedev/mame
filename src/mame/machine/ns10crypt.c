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
however, at least three of them (gamshara, mrdrilrg & panikuru) do reinitialize the
internal state of the decrypted several times. As of 2015-08-19, only gamshara shows signs
of doing it by writing to the triggering register; how the others two are triggering the
reinitializations is still unclear. gamshara does a reinitialization every 5 NAND blocks
(16 times in total); mrdrilrg does the second one after 0x38000 bytes and then subsequent
ones every 32 blocks (8 times in total); panikuru does one every 2 blocks up to a total
of 16 times.

The calculation of the XOR masks seem to operate this way: most bits are
calculated by using linear equations over GF(2) taking as input data the bits from
previously processed words; however, one nonlinear calculation is performed
per word processed, and that calculation typically affect just one bit (the only
known exception is mrdrilrg, where the same nonlinear terms are
affecting two of them). Till now, all the formulae seem to depend only on the
previous 3 words, and the first mask after a (re-)initialization is always zero, so
chances are the mask bits are calculated one word in advance, having access to the
current encrypted and decrypted words plus two further words in each sequence, maybe stored
in 32 bits registers. All the nonlinear terms reverse-engineered till now are of the form
A x B, where A and B are linear formulae; thus, as everything else in the schema involves
only linear relations, those nonlinear terms are probably caused by an Y-combinator taking
the resuls of two such linear relations as input, and deciding between both branches based
on another linear formula.

The bits affected by the nonlinear calculations are given below:
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

Overall, the values used as linear masks, those from the initSbox and
the values and bit order used at initialization time are not expected to
be exactly the ones used by the hardware; given the many degrees of freedom
caused by the nature of the scheme, the whole set of values should
be considered as a representative of a class of equivalence of functionally
equivalent datasets, nothing else.


TO-DO:
* If further dumps support the theory of the calculations just depending on 3 previous words,
change the implementation accordingly to reflect that.
* Research how type-1 encryption is related to this.

Observing the linear equations, there is a keen difference between bits using
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

const device_type CHOCOVDR_DECRYPTER = &device_creator<chocovdr_decrypter_device>;
const device_type GAMSHARA_DECRYPTER = &device_creator<gamshara_decrypter_device>;
const device_type  GJSPACE_DECRYPTER = &device_creator<gjspace_decrypter_device>;
const device_type KNPUZZLE_DECRYPTER = &device_creator<knpuzzle_decrypter_device>;
const device_type KONOTAKO_DECRYPTER = &device_creator<konotako_decrypter_device>;
const device_type NFLCLSFB_DECRYPTER = &device_creator<nflclsfb_decrypter_device>;
const device_type STARTRGN_DECRYPTER = &device_creator<startrgn_decrypter_device>;

// this could perfectly be part of the per-game logic; by now, only gamshara seems to use it, so we keep it global
const int ns10_decrypter_device::initSbox[16] = {0,12,13,6,2,4,9,8,11,1,7,15,10,5,14,3};

ns10_decrypter_device::ns10_decrypter_device(device_type type, const ns10_crypto_logic &logic, const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, "Namco System 10 Decrypter", tag, owner, clock, "ns10_crypto", __FILE__)
	, _active(false)
	, _logic(logic)
{
}

void ns10_decrypter_device::activate(int iv)
{
	init(iv);
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
		_mask ^= _reducer->gf2_reduce(_logic.eMask[j] & _previous_cipherwords);
		_mask ^= _reducer->gf2_reduce(_logic.dMask[j] & _previous_plainwords);
	}
	_mask ^= _logic.xMask;
	_mask ^= _logic.nonlinear_calculation(_previous_cipherwords, _previous_plainwords, *_reducer);

	return plainword;
}

void ns10_decrypter_device::device_start()
{
	_active = false;
	_reducer = auto_alloc(machine(), gf2_reducer());
}

void ns10_decrypter_device::init(int iv)
{
	// by now, only gamshara requires non-trivial initialization code; data
	// should be moved to the per-game logic in case any other game do it differently
	_previous_cipherwords = BITSWAP16(initSbox[iv],3,16,16,2,1,16,16,0,16,16,16,16,16,16,16,16);
	_previous_plainwords  = 0;
	_mask                 = 0;
}

gf2_reducer::gf2_reducer()
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

int gf2_reducer::gf2_reduce(UINT64 num)const
{
	return
		_gf2Reduction[num & 0xffff]         ^
		_gf2Reduction[(num >> 16) & 0xffff] ^
		_gf2Reduction[(num >> 32) & 0xffff] ^
		_gf2Reduction[num >> 48];
}


// game-specific logic

// static UINT16 mrdrilrg_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords, const gf2_reducer& reducer)
// {
	// UINT64 previous_masks = previous_cipherwords ^ previous_plainwords;
	// return (reducer.gf2_reduce(0x00000a00a305c826ull & previous_masks) & reducer.gf2_reduce(0x0000011800020000ull & previous_masks)) * 0x0011;
// }

// static UINT16 panikuru_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords, const gf2_reducer& reducer)
// {
	// return ((reducer.gf2_reduce(0x0000000088300281ull & previous_cipherwords) ^ reducer.gf2_reduce(0x0000000004600281ull & previous_plainwords))
			// & (reducer.gf2_reduce(0x0000a13140090000ull & previous_cipherwords) ^ reducer.gf2_reduce(0x0000806240090000ull & previous_plainwords))) << 2;
// }

static UINT16 chocovdr_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords, const gf2_reducer& reducer)
{
	UINT64 previous_masks = previous_cipherwords ^ previous_plainwords;
	return ((previous_masks >> 9) & (reducer.gf2_reduce(0x0000000010065810ull & previous_cipherwords) ^ reducer.gf2_reduce(0x0000000021005810ull & previous_plainwords)) & 1) << 10;
}

static const ns10_decrypter_device::ns10_crypto_logic chocovdr_crypto_logic = {
	{
		0x00005239351ec1daull, 0x0000000000008090ull, 0x0000000048264808ull, 0x0000000000004820ull,
		0x0000000000000500ull, 0x0000000058ff5a54ull, 0x00000000d8220208ull, 0x00005239351e91d3ull,
		0x000000009a1dfaffull, 0x0000000090040001ull, 0x0000000000000100ull, 0x0000000000001408ull,
		0x0000000032efd3f1ull, 0x00000000000000d0ull, 0x0000000032efd2d7ull, 0x0000000000000840ull,
	}, {
		0x00002000410485daull, 0x0000000000008081ull, 0x0000000008044088ull, 0x0000000000004802ull,
		0x0000000000000500ull, 0x00000000430cda54ull, 0x0000000010000028ull, 0x00002000410491dbull,
		0x000000001100fafeull, 0x0000000018040001ull, 0x0000000000000010ull, 0x0000000000000508ull,
		0x000000006800d3f5ull, 0x0000000000000058ull, 0x000000006800d2d5ull, 0x0000000000001840ull,
	},
	0x5b22,
	chocovdr_nonlinear_calc
};

static UINT16 gamshara_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords, const gf2_reducer&)
{
	UINT64 previous_masks = previous_cipherwords ^ previous_plainwords;
	return ((previous_masks >> 7) & (previous_masks >> 13) & 1) << 2;
}

static const ns10_decrypter_device::ns10_crypto_logic gamshara_crypto_logic = {
	{
		0x0000000000000028ull, 0x0000cae83f389fd9ull, 0x0000000000001000ull, 0x0000000042823402ull,
		0x0000cae8736a0592ull, 0x0000cae8736a8596ull, 0x000000008b4095b9ull, 0x0000000000002100ull,
		0x0000000004018228ull, 0x0000000000000042ull, 0x0000000000000818ull, 0x0000000000004010ull,
		0x000000008b4099f1ull, 0x00000000044bce08ull, 0x00000000000000c1ull, 0x0000000042823002ull,
	}, {
		0x0000000000000028ull, 0x00000904c2048dd9ull, 0x0000000000008000ull, 0x0000000054021002ull,
		0x00000904e0078592ull, 0x00000904e00785b2ull, 0x00000000440097f9ull, 0x0000000000002104ull,
		0x0000000029018308ull, 0x0000000000000042ull, 0x0000000000000850ull, 0x0000000000004012ull,
		0x000000004400d1f1ull, 0x000000006001ce08ull, 0x00000000000000c8ull, 0x0000000054023002ull,
	},
	0x25ab,
	gamshara_nonlinear_calc
};

static UINT16 gjspace_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords, const gf2_reducer&)
{
	return 0;
}

static const ns10_decrypter_device::ns10_crypto_logic gjspace_crypto_logic = {
	{
		0x0000000000000240ull, 0x0000d617eb0f1ab1ull, 0x00000000451111c0ull, 0x00000000013b1f44ull,
		0x0000aab0b356abceull, 0x00007ca76b89602aull, 0x0000000000001800ull, 0x00000000031d1303ull,
		0x0000000000000801ull, 0x0000000030111160ull, 0x0000000001ab3978ull, 0x00000000c131b160ull,
		0x0000000000001110ull, 0x0000000000008002ull, 0x00000000e1113540ull, 0x0000d617fdce8bfcull,
	}, {
		0x0000000000008240ull, 0x000000002f301ab1ull, 0x00000000050011c0ull, 0x00000000412817c4ull,
		0x00000004c338abc6ull, 0x000000046108602aull, 0x0000000000005800ull, 0x00000000c3081347ull,
		0x0000000000000801ull, 0x0000000061001160ull, 0x0000000061183978ull, 0x00000000e520b142ull,
		0x0000000000001101ull, 0x000000000000a002ull, 0x0000000029001740ull, 0x00000000a4309bfcull,
	},
	0x2e7f,
	gjspace_nonlinear_calc
};

static UINT16 knpuzzle_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords, const gf2_reducer& reducer)
{
	UINT64 previous_masks = previous_cipherwords ^ previous_plainwords;
	return ((previous_masks >> 0x13) & (reducer.gf2_reduce(0x0000000014001290ull & previous_cipherwords) ^ reducer.gf2_reduce(0x0000000000021290ull & previous_plainwords)) & 1) << 1;
}

static const ns10_decrypter_device::ns10_crypto_logic knpuzzle_crypto_logic = {
	{
		0x00000000c0a4208cull, 0x00000000204100a8ull, 0x000000000c0306a0ull, 0x000000000819e944ull,
		0x0000000000001400ull, 0x0000000000000061ull, 0x000000000141401cull, 0x0000000000000020ull,
		0x0000000001418010ull, 0x00008d6a1eb690cfull, 0x00008d6a4d3b90ceull, 0x0000000000004201ull,
		0x00000000012c00a2ull, 0x000000000c0304a4ull, 0x0000000000000500ull, 0x0000000000000980ull,
	}, {
		0x000000002a22608cull, 0x00000000002300a8ull, 0x0000000000390ea0ull, 0x000000000100a9c4ull,
		0x0000000000001400ull, 0x0000000000000041ull, 0x0000000003014014ull, 0x0000000000000022ull,
		0x0000000003010110ull, 0x00000800031a80cfull, 0x00000800003398deull, 0x0000000000004200ull,
		0x00000000012a04a2ull, 0x00000000003984a4ull, 0x0000000000000700ull, 0x0000000000000882ull,
	},
	0x01e2,
	knpuzzle_nonlinear_calc
};

static UINT16 konotako_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords, const gf2_reducer&)
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

static UINT16 nflclsfb_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords, const gf2_reducer& reducer)
{
	UINT64 previous_masks = previous_cipherwords ^ previous_plainwords;
	return ((previous_masks >> 1) & (reducer.gf2_reduce(0x0000000040de8fb3ull & previous_cipherwords) ^ reducer.gf2_reduce(0x0000000088008fb3ull & previous_plainwords)) & 1) << 2;
}

static const ns10_decrypter_device::ns10_crypto_logic nflclsfb_crypto_logic = {
	{
		0x000034886e281880ull, 0x0000000012c5e7baull, 0x0000000000000200ull, 0x000000002900002aull,
		0x00000000000004c0ull, 0x0000000012c5e6baull, 0x00000000e0df8bbbull, 0x000000002011532aull,
		0x0000000000009040ull, 0x0000000000006004ull, 0x000000000000a001ull, 0x000034886e2818e1ull,
		0x0000000000004404ull, 0x0000000000004200ull, 0x0000000000009100ull, 0x0000000020115712ull,
	}, {
		0x00000e00060819c0ull, 0x000000000e08e7baull, 0x0000000000000800ull, 0x000000000100002aull,
		0x00000000000010c0ull, 0x000000000e08cebaull, 0x0000000088018bbbull, 0x000000008c005302ull,
		0x000000000000c040ull, 0x0000000000006010ull, 0x0000000000000001ull, 0x00000e00060818e3ull,
		0x0000000000000404ull, 0x0000000000004201ull, 0x0000000000001100ull, 0x000000008c0057b2ull,
	},
	0xbe32,
	nflclsfb_nonlinear_calc
};

static UINT16 startrgn_nonlinear_calc(UINT64 previous_cipherwords, UINT64 previous_plainwords, const gf2_reducer&)
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

chocovdr_decrypter_device::chocovdr_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ns10_decrypter_device(CHOCOVDR_DECRYPTER, chocovdr_crypto_logic, mconfig, tag, owner, clock)
{
}

gamshara_decrypter_device::gamshara_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ns10_decrypter_device(GAMSHARA_DECRYPTER, gamshara_crypto_logic, mconfig, tag, owner, clock)
{
}

gjspace_decrypter_device::gjspace_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ns10_decrypter_device(GJSPACE_DECRYPTER, gjspace_crypto_logic, mconfig, tag, owner, clock)
{
}

knpuzzle_decrypter_device::knpuzzle_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ns10_decrypter_device(KNPUZZLE_DECRYPTER, knpuzzle_crypto_logic, mconfig, tag, owner, clock)
{
}

konotako_decrypter_device::konotako_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ns10_decrypter_device(KONOTAKO_DECRYPTER, konotako_crypto_logic, mconfig, tag, owner, clock)
{
}

nflclsfb_decrypter_device::nflclsfb_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ns10_decrypter_device(NFLCLSFB_DECRYPTER, nflclsfb_crypto_logic, mconfig, tag, owner, clock)
{
}

startrgn_decrypter_device::startrgn_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ns10_decrypter_device(STARTRGN_DECRYPTER, startrgn_crypto_logic, mconfig, tag, owner, clock)
{
}
