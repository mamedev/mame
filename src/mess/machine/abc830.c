/*

    Luxor ABC 830

    Type                    Size        Tracks  Sides   Sectors/track   Sectors     Speed           Drives

    ABC-830     Floppy      160 KB      40      1       16              640         250 Kbps        MPI 51, BASF 6106
                Floppy      80 KB       40      1       8               320         250 Kbps        Scandia Metric FD2
    ABC-832     Floppy      640 KB      80      2       16              2560        250 Kbps        Micropolis 1015, Micropolis 1115, BASF 6118
    ABC-834     Floppy      640 KB      80      2       16              2560        250 Kbps        Teac FD 55 F
    ABC-838     Floppy      1 MB        77      2       25              3978        500 Kbps        BASF 6104, BASF 6115
    ABC-850     Floppy      640 KB      80      2       16              2560        250 Kbps        TEAC FD 55 F, BASF 6136
                HDD         10 MB       320     4       32              40960       5 Mbps          Rodime 202, BASF 6186
    ABC-852     Floppy      640 KB      80      2       16              2560        250 Kbps        TEAC FD 55 F, BASF 6136
                HDD         20 MB       615     4       32              78720       5 Mbps          NEC 5126
                Streamer    45 MB       9                                           90 Kbps         Archive 5945-C (tape: DC300XL 450ft)
    ABC-856     Floppy      640 KB      80      2       16              2560        250 Kbps        TEAC FD 55 F
                HDD         64 MB       1024    8       32              262144      5 Mbps          Micropolis 1325
                Streamer    45 MB       9                                           90 Kbps         Archive 5945-C (tape: DC300XL 450ft)

*/

#include "abc830.h"



//**************************************************************************
//  FLOPPY CONFIGURATIONS
//**************************************************************************

//-------------------------------------------------
//  floppy_interface abc830_floppy_interface
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( abc830 )
	// NOTE: Real ABC 830 (160KB) disks use a 7:1 sector interleave.
	// Specify INTERLEAVE([7]) below if you prefer the physical layout.
	LEGACY_FLOPPY_OPTION(abc830, "dsk", "Luxor ABC 830", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

const floppy_interface abc830_floppy_interface =
{
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    FLOPPY_STANDARD_5_25_SSDD_40,
    LEGACY_FLOPPY_OPTIONS_NAME(abc830),
    "abc830",
	NULL
};


//-------------------------------------------------
//  floppy_interface abc832_floppy_interface
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( abc832 )
	LEGACY_FLOPPY_OPTION(abc832, "dsk", "Luxor ABC 832/834", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

const floppy_interface abc832_floppy_interface =
{
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    FLOPPY_STANDARD_5_25_DSDD,
    LEGACY_FLOPPY_OPTIONS_NAME(abc832),
    "abc832",
	NULL
};


//-------------------------------------------------
//  floppy_interface abc838_floppy_interface
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( abc838 )
	LEGACY_FLOPPY_OPTION(abc838, "dsk", "Luxor ABC 838", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([77])
		SECTORS([26])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

const floppy_interface abc838_floppy_interface =
{
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    FLOPPY_STANDARD_8_DSDD,
    LEGACY_FLOPPY_OPTIONS_NAME(abc838),
    "abc838",
	NULL
};


//-------------------------------------------------
//  floppy_interface fd2_floppy_interface
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( fd2 )
	// NOTE: FD2 cannot be used with the Luxor controller card,
	// it has a proprietary one. This is just for reference.
	LEGACY_FLOPPY_OPTION(fd2, "dsk", "Scandia Metric FD2", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([8])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

const floppy_interface fd2_floppy_interface =
{
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    FLOPPY_STANDARD_5_25_SSSD,
    LEGACY_FLOPPY_OPTIONS_NAME(fd2),
    "floppy_5_25",
	NULL
};



