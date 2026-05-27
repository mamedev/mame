// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Castool command line front end

    27/03/2009 Initial version by Miodrag Milanovic

***************************************************************************/

#include "formats/a26_cas.h"
#include "formats/ace_tap.h"
#include "formats/adam_cas.h"
#include "formats/apf_apt.h"
#include "formats/atom_tap.h"
#include "formats/cbm_tap.h"
#include "formats/cgen_cas.h"
#include "formats/coco_cas.h"
#include "formats/csw_cas.h"
#include "formats/fm7_cas.h"
#include "formats/fmsx_cas.h"
#include "formats/gtp_cas.h"
#include "formats/hect_tap.h"
#include "formats/kc_cas.h"
#include "formats/kim1_cas.h"
#include "formats/lviv_lvt.h"
#include "formats/mz_cas.h"
#include "formats/orao_cas.h"
#include "formats/oric_tap.h"
#include "formats/p2000t_cas.h"
#include "formats/p6001_cas.h"
#include "formats/phc25_cas.h"
#include "formats/pmd_cas.h"
#include "formats/primoptp.h"
#include "formats/rk_cas.h"
#include "formats/sc3000_bit.h"
#include "formats/sol_cas.h"
#include "formats/sorc_cas.h"
#include "formats/sord_cas.h"
#include "formats/spc1000_cas.h"
#include "formats/svi_cas.h"
#include "formats/thom_cas.h"
#include "formats/trs_cas.h"
#include "formats/tvc_cas.h"
#include "formats/tzx_cas.h"
#include "formats/uef_cas.h"
#include "formats/vg5k_cas.h"
#include "formats/vt_cas.h"
#include "formats/x07_cas.h"
#include "formats/x1_tap.h"
#include "formats/zx81_p.h"

#include "corestr.h"
#include "ioprocs.h"
#include "osdcomm.h"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>


struct SupportedCassetteFormats
{
	const char *name;
	const cassette_image::Format * const *formats;
	const char *desc;
};

const struct SupportedCassetteFormats formats[] = {
	{"a26", a26_cassette_formats               ,"Atari 2600 SuperCharger"},
	{"apf", apf_cassette_formats               ,"APF Imagination Machine"},
	{"atom", atom_cassette_formats             ,"Acorn Atom"},
	{"bbc", bbc_cassette_formats               ,"Acorn BBC & Electron"},
	{"cbm", cbm_cassette_formats               ,"Commodore 8-bit series"},
	{"cdt", cdt_cassette_formats               ,"Amstrad CPC"},
	{"cgenie", cgenie_cassette_formats         ,"EACA Colour Genie"},
	{"coco", coco_cassette_formats             ,"Tandy Radio Shack Color Computer"},
	{"csw", csw_cassette_formats               ,"Compressed Square Wave"},
	{"ddp", coleco_adam_cassette_formats       ,"Coleco ADAM"},
	{"fm7", fm7_cassette_formats               ,"Fujitsu FM-7"},
	{"fmsx", fmsx_cassette_formats             ,"MSX"},
	{"gtp", gtp_cassette_formats               ,"Elektronika inzenjering Galaksija"},
	{"hector", hector_cassette_formats         ,"Micronique Hector & Interact Family Computer"},
	{"jupiter", ace_cassette_formats           ,"Jupiter Cantab Jupiter Ace"},
	{"kc85", kc_cassette_formats               ,"VEB Mikroelektronik KC 85"},
	{"kim1", kim1_cassette_formats             ,"MOS KIM-1"},
	{"lviv", lviv_lvt_format                   ,"PK-01 Lviv"},
	{"mo5", mo5_cassette_formats               ,"Thomson MO-series"},
	{"mz", mz700_cassette_formats              ,"Sharp MZ-700"},
	{"orao", orao_cassette_formats             ,"PEL Varazdin Orao"},
	{"oric", oric_cassette_formats             ,"Tangerine Oric"},
	{"p2000t", p2000t_cassette_formats         ,"Philips P2000T"},
	{"pc6001", pc6001_cassette_formats         ,"NEC PC-6001"},
	{"phc25", phc25_cassette_formats           ,"Sanyo PHC-25"},
	{"pmd85", pmd85_cassette_formats           ,"Tesla PMD-85"},
	{"primo", primo_ptp_format                 ,"Microkey Primo"},
	{"rku", rku_cassette_formats               ,"UT-88"},
	{"rk8", rk8_cassette_formats               ,"Mikro-80"},
	{"rks", rks_cassette_formats               ,"Specialist"},
	{"rko", rko_cassette_formats               ,"Orion"},
	{"rkr", rkr_cassette_formats               ,"Radio-86RK"},
	{"rka", rka_cassette_formats               ,"Zavod BRA Apogee BK-01"},
	{"rkm", rkm_cassette_formats               ,"Mikrosha"},
	{"rkp", rkp_cassette_formats               ,"SAM SKB VM Partner-01.01"},
	{"sc3000", sc3000_cassette_formats         ,"Sega SC-3000"},
	{"sol20", sol20_cassette_formats           ,"PTC SOL-20"},
	{"sorcerer", sorcerer_cassette_formats     ,"Exidy Sorcerer"},
	{"sordm5", sordm5_cassette_formats         ,"Sord M5"},
	{"spc1000", spc1000_cassette_formats       ,"Samsung SPC-1000"},
	{"svi", svi_cassette_formats               ,"Spectravideo SVI-318 & SVI-328"},
	{"to7", to7_cassette_formats               ,"Thomson TO-series"},
	{"trs80l2", trs80l2_cassette_formats       ,"TRS-80 Level 2"},
	{"tvc64", tvc64_cassette_formats           ,"Videoton TVC 64"},
	{"tzx", tzx_cassette_formats               ,"Sinclair ZX Spectrum"},
	{"vg5k", vg5k_cassette_formats             ,"Philips VG 5000"},
	{"vtech1", vtech1_cassette_formats         ,"Video Technology Laser 110-310"},
	{"vtech2", vtech2_cassette_formats         ,"Video Technology Laser 350-700"},
	{"x07", x07_cassette_formats               ,"Canon X-07"},
	{"x1", x1_cassette_formats                 ,"Sharp X1"},
	{"zx80_o", zx80_o_format                   ,"Sinclair ZX80"},
	{"zx81_p", zx81_p_format                   ,"Sinclair ZX81"},



	{nullptr,nullptr,nullptr}
};


static std::string get_extension(const char *name)
{
	const char *s;
	s = name;
	if (s != nullptr)
		s = strrchr(s, '.');
	return s ? std::string(s+1) : "";
}

static void display_usage(const char *argv0)
{
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "       %s convert <format> <inputfile> <outputfile.wav>\n", argv0);
}

static void display_formats(void)
{
	int i,j;
	fprintf(stderr, "Supported formats:\n\n");
	for (i = 0; formats[i].name; i++) {
		fprintf(stderr, "%10s - %s\n",formats[i].name,formats[i].desc);
		for (j = 1; formats[i].formats[j]; j++) {
			fprintf(stderr, "%15s %s\n","",formats[i].formats[j]->extensions);
		}
	}
}

int CLIB_DECL main(int argc, char *argv[])
{
	int i;
	int found =0;
	const cassette_image::Format * const *selected_formats = nullptr;
	cassette_image::ptr cassette;

	if (argc > 1)
	{
		if (!core_stricmp("convert", argv[1]))
		{
			// convert command
			if (argc!=5) {
				fprintf(stderr, "Wrong parameter number.\n\n");
				display_usage(argv[0]);
				return -1;
			} else {
				for (i = 0; formats[i].name; i++) {
					if (core_stricmp(formats[i].name,argv[2])==0) {
						selected_formats = formats[i].formats;
						found = 1;
					}
				}
				if (found==0) {
					fprintf(stderr, "Wrong format name.\n\n");
					display_usage(argv[0]);
					fprintf(stderr, "\n");
					display_formats();
					return -1;
				}

				FILE *f = fopen(argv[3], "rb");
				if (!f) {
					fprintf(stderr, "File %s not found.\n",argv[3]);
					return -1;
				}

				auto io = util::stdio_read_write(f, 0x00);
				f = nullptr;
				if (!io) {
					fprintf(stderr, "Out of memory.\n");
					return -1;
				}

				if (cassette_image::open_choices(std::move(io), get_extension(argv[3]), selected_formats, cassette_image::FLAG_READONLY, cassette) != cassette_image::error::SUCCESS)  {
					fprintf(stderr, "Invalid format of input file.\n");
					return -1;
				}

				cassette->dump(argv[4]);
				cassette.reset();
				goto theend;
			}
		}
	}

	/* Usage */
	fprintf(stderr, "castool - Generic cassette manipulation tool for use with MAME\n\n");
	display_usage(argv[0]);
	fprintf(stderr, "\n");
	display_formats();
	fprintf(stderr, "\nExample usage:\n");
	fprintf(stderr, "        %s convert tzx game.tzx game.wav\n\n", argv[0]);

theend :
	return 0;
}
