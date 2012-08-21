/***************************************************************************

    main.c

    Castool command line front end

    27/03/2009 Initial version by Miodrag Milanovic

***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "corestr.h"

#include "formats/a26_cas.h"
#include "formats/apf_apt.h"
#include "formats/cbm_tap.h"
#include "formats/cgen_cas.h"
#include "formats/coco_cas.h"
#include "formats/csw_cas.h"
#include "formats/fmsx_cas.h"
#include "formats/gtp_cas.h"
#include "formats/hect_tap.h"
#include "formats/ace_tap.h"
#include "formats/adam_cas.h"
#include "formats/kc_cas.h"
#include "formats/kim1_cas.h"
#include "formats/lviv_lvt.h"
#include "formats/mz_cas.h"
#include "formats/orao_cas.h"
#include "formats/oric_tap.h"
#include "formats/pmd_cas.h"
#include "formats/primoptp.h"
#include "formats/rk_cas.h"
#include "formats/sord_cas.h"
#include "formats/svi_cas.h"
#include "formats/thom_cas.h"
#include "formats/trs_cas.h"
#include "formats/tvc_cas.h"
#include "formats/tzx_cas.h"
#include "formats/uef_cas.h"
#include "formats/vg5k_cas.h"
#include "formats/vt_cas.h"
#include "formats/x07_cas.h"
#include "formats/zx81_p.h"

struct SupportedCassetteFormats
{
	const char *name;
	const struct CassetteFormat * const *formats;
	const char *desc;
};

const struct SupportedCassetteFormats formats[] = {
	{"a26", a26_cassette_formats               ,"Atari 2600"},
	{"ddp", coleco_adam_cassette_formats       ,"Coleco Adam"},
	{"apf", apf_cassette_formats               ,"APF Imagination Machine"},
	{"cbm", cbm_cassette_formats               ,"Commodore"},
	{"cgenie", cgenie_cassette_formats         ,"Colour Genie"},
	{"coco", coco_cassette_formats             ,"TRS-80 Radio Shack Color Computer Family"},
	{"csw", csw_cassette_formats               ,"Compressed Square Wave"},
	{"bbc", bbc_cassette_formats               ,"BBC"},
	{"fmxs", fmsx_cassette_formats             ,"MSX"},
	{"gtp", gtp_cassette_formats               ,"Galaksija"},
	{"hector", hector_cassette_formats         ,"Hector - k7 : classical, FOR : forth cassette "},
	{"jupiter", ace_cassette_formats           ,"Jupiter"},
	{"kc85", kc_cassette_formats               ,"VEB Mikroelektronik KC 85"},
	{"kim1", kim1_cassette_formats             ,"KIM-1"},
	{"lviv", lviv_lvt_format                   ,"Lviv"},
	{"mz", mz700_cassette_formats              ,"Sharp MZ"},
	{"orao", orao_cassette_formats             ,"Orao"},
	{"oric", oric_cassette_formats             ,"Oric"},
	{"pmd85", pmd85_cassette_formats           ,"PMD-85"},
	{"primo", primo_ptp_format                 ,"Primo"},
	{"rku", rku_cassette_formats               ,"UT-88"},
	{"rk8", rk8_cassette_formats               ,"Mikro-80"},
	{"rks", rks_cassette_formats               ,"Specialist"},
	{"rko", rko_cassette_formats               ,"Orion"},
	{"rkr", rkr_cassette_formats               ,"Radio-86RK"},
	{"rka", rka_cassette_formats               ,"Apogee"},
	{"rkm", rkm_cassette_formats               ,"Mikrosha"},
	{"rkp", rkp_cassette_formats               ,"Partner"},
	{"sordm5", sordm5_cassette_formats         ,"Sord M5"},
	{"svi", svi_cassette_formats               ,"SVI"},
	{"to7", to7_cassette_formats               ,"Thomson TO"},
	{"mo5", mo5_cassette_formats               ,"Thomson MO"},
	{"trs80l2", trs80l2_cassette_formats       ,"TRS-80 Level 2"},
	{"tvc64", tvc64_cassette_formats           ,"Videoton TVC 64"},
	{"tzx", tzx_cassette_formats               ,"ZX Spectrum"},
	{"cdt", cdt_cassette_formats               ,"Amstrad CPC"},
	{"uef", uef_cassette_formats               ,"Acorn Electron"},
	{"vg5k", vg5k_cassette_formats             ,"VG 5000 k7"},
	{"vtech1", vtech1_cassette_formats         ,"Video Technology Laser 110-310"},
	{"vtech2", vtech2_cassette_formats         ,"Video Technology Laser 350-700"},
	{"x07", x07_cassette_formats               ,"Canon X-07"},
	{"zx81_p", zx81_p_format                   ,"Sinclair ZX81"},
	{"zx80_o", zx80_o_format                   ,"Sinclair ZX80"},
	{NULL,NULL,NULL}
};


static const char *get_extension(const char *name)
{
	const char *s;
	s = name;
	if (s != NULL)
		s = strrchr(s, '.');
	return s ? s+1 : NULL;
}

static void display_usage(void)
{
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "		castool.exe convert <format> <inputfile> <outputfile.wav>\n");
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
	const struct CassetteFormat * const *selected_formats = NULL;
	cassette_image *cassette;
	FILE *f;

	if (argc > 1)
	{
		if (!core_stricmp("convert", argv[1]))
		{
			// convert command
			if (argc!=5) {
				fprintf(stderr, "Wrong parameter number.\n\n");
				display_usage();
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
					display_usage();
					fprintf(stderr, "\n");
					display_formats();
					return -1;
				}

				f = fopen(argv[3], "rb");
				if (!f) {
					fprintf(stderr, "File %s not found.\n",argv[3]);
					return -1;
				}

				if (cassette_open_choices(f, &stdio_ioprocs, get_extension(argv[3]), selected_formats, CASSETTE_FLAG_READONLY, &cassette))	{
					fprintf(stderr, "Invalid format of input file.\n");
					fclose(f);
					return -1;
				}

				cassette_dump(cassette,argv[4]);
				cassette_close(cassette);
				fclose(f);
				goto theend;
			}
		}
	}

	/* Usage */
	fprintf(stderr, "castool - Generic cassette manipulation tool for use with MESS\n\n");
	display_usage();
	fprintf(stderr, "\n");
	display_formats();
	fprintf(stderr, "\nExample usage:\n");
	fprintf(stderr, "        castool.exe convert tzx game.tzx game.wav\n\n");

theend :
	return 0;
}
