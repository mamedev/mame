// license:BSD-3-Clause
// copyright-holders:Curt Coder

#include "p2000t_cas.h"

#include "coretmpl.h" // BIT

#include <ostream>


// This code will reproduce the timing of a P2000 mini cassette tape.
constexpr double P2000_CLOCK_PERIOD = 0.000084;
constexpr double P2000_BOT_GAP      = 1;
constexpr double P2000_BOB_GAP      = 0.515;
constexpr double P2000_MARK_GAP     = 0.085;
constexpr double P2000_END_GAP      = 0.155;
constexpr double P2000_EOT_GAP      = 1.8;
constexpr int P2000_HIGH            = 0x7FFFFFFF;
constexpr int P2000_LOW             = -1 * 0x7FFFFFFF;


#define CHR(x)                                 \
	err = (x);                                 \
	if (err != cassette_image::error::SUCCESS) \
		return err;

/*
Here's a description on how the P2000t stores data on tape.

## Tape Format

Each track (or side) of a cassette is divided into 40 block of information. A
file may comprise between 1 and 40 blocks, depending on its length.

## Blocks, Gaps, Marks and Records

At the start of the tape is an area of clear (erased) tape, the BOT (beginning
of tape) gap; to read pas this gap takes approximately 1 second. After this, the
first block starts, followed directly by the second, third and so on. After the
last block on the track comes the EOT (end of tape) gap; if all 40 blocks on the
tracks are used, this area of erased tape has a length equivalent to 1.8 seconds
of reading time.

 | BOT | BLOCK 1 | BLOCK 2 | BLOCK ... | EOT |

### BLOCK

Each tape block is made up of five sections of tape:

| START GAP | MARK | MARK GAP | DATA RECORD | END GAP |

- Start gap: A section of erased tape separating the start of one block from the
             end of the previous block. This takes approx. 515ms to read over.
- Mark: Four bytes of recorded information (described below)
- Mark Gap: A section of erased tape separating the mark from the data record;
            length about 85ms.
- Data Record: 1056 bytes of recorded data (described below)
- End Gap: A section of erased tape of around 155 ms.

### MARK

The mark is made up of four bytes with the following bit patterns:

  - preamble syncronization pattern (0xAA)
  - 0000 0000 (0x00)
  - 0000 0000 (0x00)
  - postamble syncronization pattern (0xAA)

The function of the synchronisation byte is to make sure the RDC clock is set
properly.

### DATA RECORD

The data record contains the information which has been written onto the tape.
It compromises five sections:

| sync byte | header | data | check sum | post sync |

- sync byte: Preamble synchronisation pattern 0xAA
- header: 32 bytes which specify the contents and type of the data section. (See below)
- data section: 1024 bytes of information.
- checksum: 2 bytes that contain checksum
- post sync: 0xAA

### HEADER

See the struct declaration below for details on what is contained.

### DATA

The data section consists of 1024 bytes written in serial form, least
significant byte first.

### Checksum

The checksum is only calculated for the header and data section. The algorithm
is not documented, an implementation can be found below.
*/

// Specifies the type of data stored in the tape block
enum P2000_File_Type : uint8_t
{
	Basic          = 'B',
	Program        = 'P',
	Viewdata       = 'V',
	WordProcessing = 'W',
	Other          = 'O',
};

// Represents the internal code for data, differrent codes are used in different countries
// only relevant when P2000_File_Type is Program.
enum P2000_Data_Type : uint8_t
{
	German        = 'D',
	Swedish       = 'S',
	Dutch_English = 'U',
};


// This is the 32 byte header definition used by the P2000, it is mainly
// here for documentation.
struct P2000T_Header
{
	// Starting address in ram where data should go. This address is supplied by
	// the application program when calling the monitor cassette routine to write
	// the data on cassette in the first place.
	uint16_t data_transfer_address;
	// Total # of bytes which make up the file (can be spread over many blocks).
	// The monitor uses this to determine how many blocks to read.
	uint16_t file_length;
	// # bytes in this record that are actually used. For example if this is 256
	// only 256 bytes will be loaded in ram
	uint16_t data_section_length;
	// The eight character file name identifies the file to which the record
	// belongs; it will be the same in all records making up the file. Each record
	// except the first is considered an extension.
	char file_name[8];
	// Addition file extension.
	char ext[3];
	// This file type specifies the type of data stored.
	P2000_File_Type file_type;
	// Code and region information.
	P2000_Data_Type data_code;
	// Start address where program should start. (if type = Program)
	uint16_t start_addr;
	// Address in ram where the program should load (if type = Program)
	uint16_t load_addr;
	// Unused.
	char reserved[8];
	// Record number (i.e. which block)
	uint8_t rec_nr;
};

std::ostream &operator<<(std::ostream &os, P2000T_Header const &hdr)
{
	return os << "File: " << std::string(hdr.file_name, 8) << '.'
			  << std::string(hdr.ext, 3) << "  " << hdr.file_length;
}

static cassette_image::error p2000t_cas_identify(cassette_image *cass, cassette_image::Options *opts)
{
	opts->bits_per_sample  = 32;
	opts->channels         = 1;
	opts->sample_frequency = 44100;
	return cassette_image::error::SUCCESS;
}

uint16_t rotr16a(uint16_t x, uint16_t n)
{
	return (x >> n) | (x << (16 - n));
}

void update_chksum(uint16_t *de, bool bit)
{
	// Reverse engineered from monitor.rom
	// code is at: [0x07ac, 0x07c5]
	uint8_t e = *de & 0xff;
	uint8_t d = (*de >> 8) & 0xff;
	e         = e ^ (bit ? 1 : 0);
	if (e & 0x01)
	{
		e = e ^ 2;
		d = d ^ 0x40;
	}
	else
	{
		d = d ^ 0x00;
	}
	*de = rotr16a((d << 8) | e, 1);
}

/*
    A transition on a clock boundary from low to high is a 1.
    A transition on a clock boundary from high to low is a 0
    An intermediate transition halfway between the clock boundary
    can occur when there are consecutive 0s or 1s. See the example
    below where the clock is marked by a |


            1    0    1    1    0    0
    RDA:  _|----|____|--__|----|__--|__--
    RDC:  _|-___|-___|-___|-___|-___|-___
            ^                     ^
            |-- clock signal      |-- intermediate transition.

    This signal can be written by a simple algorithm where the first bit
    is always false (transition to low, half clock).  Now only one bit is needed
    to determine what the next partial clock should look like.

    This works because we are always guaranteed that a block starts with 0xAA,
    and hence will ALWAYS find a signal like this on tape: _-- (low, high, high)
    after a gap. This is guaranteed when the tape is moving forward as well as
    backwards.
*/
cassette_image::error p2000t_put_bit(cassette_image *cass, double *time_index, bool bit)
{
	const int channel         = 0;
	cassette_image::error err = cassette_image::error::SUCCESS;
	CHR(cass->put_sample(channel, *time_index, P2000_CLOCK_PERIOD, bit ? P2000_HIGH : P2000_LOW));
	*time_index += P2000_CLOCK_PERIOD;

	CHR(cass->put_sample(channel, *time_index, P2000_CLOCK_PERIOD, bit ? P2000_LOW : P2000_HIGH));
	*time_index += P2000_CLOCK_PERIOD;
	return err;
}

// Store byte of data, updating the checksum
cassette_image::error p2000t_put_byte(cassette_image *cass, double *time_index, uint16_t *chksum, uint8_t byte)
{
	cassette_image::error err = cassette_image::error::SUCCESS;
	for (int i = 0; i < 8 && err == cassette_image::error::SUCCESS; i++)
	{
		update_chksum(chksum, util::BIT(byte, i));
		CHR(p2000t_put_bit(cass, time_index, util::BIT(byte, i)));
	}
	return err;
}

// Store a sequence of bytes, updating the checksum
cassette_image::error p2000t_put_bytes(cassette_image *cass, double *time_index, uint16_t *chksum, const uint8_t *bytes, const uint16_t cByte)
{
	cassette_image::error err = cassette_image::error::SUCCESS;
	for (int i = 0; i < cByte && err == cassette_image::error::SUCCESS; i++)
	{
		CHR(p2000t_put_byte(cass, time_index, chksum, bytes[i]));
	}
	return err;
}

// Insert time seconds of silence.
cassette_image::error p2000t_silence(cassette_image *cassette,
double *time_index,
double time)
{
	auto err = cassette->put_sample(0, *time_index, time, 0);
	*time_index += time;
	return err;
}

static cassette_image::error p2000t_cas_load(cassette_image *cassette)
{
	cassette_image::error err = cassette_image::error::SUCCESS;
	uint64_t image_size       = cassette->image_size();
	constexpr int CAS_BLOCK   = 1280;

	/*
	    The cas format is pretty simple. it consists of a sequence of blocks,
	    where a block consists of the following:

	    [0-256]   P2000 memory address 0x6000 - 0x6100
	    ....   Nonsense (keyboard status etc.)
	    0x30   P200T_Header
	    0x50
	    ...    Nonsense..
	    [0-1024] Data block

	    This means that one block gets stored in 1280 bytes.
	*/
	if (image_size % CAS_BLOCK != 0)
	{
		return cassette_image::error::INVALID_IMAGE;
	}

	uint8_t block[CAS_BLOCK];
	constexpr uint8_t BLOCK_MARK[4] = { 0xAA, 0x00, 0x00, 0xAA };
	auto blocks                     = image_size / CAS_BLOCK;
	double time_idx                 = 0;

	// Beginning of tape marker
	CHR(p2000t_silence(cassette, &time_idx, P2000_BOT_GAP));
	for (int i = 0; i < blocks; i++)
	{
		uint16_t crc = 0, unused = 0;
		cassette->image_read(&block, CAS_BLOCK * i, CAS_BLOCK);

		// Insert sync header.. 0xAA, 0x00, 0x00, 0xAA
		CHR(p2000t_silence(cassette, &time_idx, P2000_BOB_GAP));
		CHR(p2000t_put_bytes(cassette, &time_idx, &unused, BLOCK_MARK, std::size(BLOCK_MARK)));
		CHR(p2000t_silence(cassette, &time_idx, P2000_MARK_GAP));

		// Insert data block
		CHR(p2000t_put_byte(cassette, &time_idx, &unused, 0xAA));
		CHR(p2000t_put_bytes(cassette, &time_idx, &crc, block + 0x30, 32));
		CHR(p2000t_put_bytes(cassette, &time_idx, &crc, block + 256, 1024));
		CHR(p2000t_put_bytes(cassette, &time_idx, &unused, ( uint8_t * )&crc, 2));
		CHR(p2000t_put_byte(cassette, &time_idx, &unused, 0xAA));

		// Block finished.
		CHR(p2000t_silence(cassette, &time_idx, P2000_END_GAP));
	}

	// End of tape marker
	return p2000t_silence(cassette, &time_idx, P2000_EOT_GAP);
}

static const cassette_image::Format p2000t_cas = {
	"cas", p2000t_cas_identify, p2000t_cas_load, nullptr /* no save */
};

CASSETTE_FORMATLIST_START(p2000t_cassette_formats)
CASSETTE_FORMAT(p2000t_cas)
CASSETTE_FORMATLIST_END
