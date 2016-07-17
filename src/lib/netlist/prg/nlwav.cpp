// license:GPL-2.0+
// copyright-holders:Couriersud
#include <plib/poptions.h>
#include <cstdio>
#include <cstring>
#include "plib/pstring.h"
#include "plib/plists.h"
#include "plib/pstream.h"
#include "nl_setup.h"

class nlwav_options_t : public plib::options
{
public:
	nlwav_options_t() :
		plib::options(),
		opt_inp(*this,  "i", "input",       "",      "input file"),
		opt_out(*this,  "o", "output",      "",      "output file"),
		opt_amp(*this,  "a", "amp",    10000.0,      "amplification after mean correction"),
		opt_verb(*this, "v", "verbose",              "be verbose - this produces lots of output"),
		opt_quiet(*this,"q", "quiet",                "be quiet - no warnings"),
		opt_version(*this,  "",  "version",          "display version and exit"),
		opt_help(*this, "h", "help",                 "display help and exit")
	{}
	plib::option_str    opt_inp;
	plib::option_str    opt_out;
	plib::option_double opt_amp;
	plib::option_bool   opt_verb;
	plib::option_bool   opt_quiet;
	plib::option_bool   opt_version;
	plib::option_bool   opt_help;
};

plib::pstdout pout_strm;
plib::pstderr perr_strm;

plib::pstream_fmt_writer_t pout(pout_strm);
plib::pstream_fmt_writer_t perr(perr_strm);

/* http://de.wikipedia.org/wiki/RIFF_WAVE */
class wav_t
{
public:
	wav_t(plib::postream &strm, unsigned sr) : m_f(strm)
	{
		initialize(sr);
		m_f.write(&m_fh, sizeof(m_fh));
		m_f.write(&m_fmt, sizeof(m_fmt));
		m_f.write(&m_data, sizeof(m_data));
	}
	~wav_t()
	{
		m_f.seek(0);
		m_f.write(&m_fh, sizeof(m_fh));
		m_f.write(&m_fmt, sizeof(m_fmt));

		//data.len = fmt.block_align * n;
		m_f.write(&m_data, sizeof(m_data));
	}

	unsigned channels() { return m_fmt.channels; }
	unsigned sample_rate() { return m_fmt.sample_rate; }

	void write_sample(int sample)
	{
		m_data.len += m_fmt.block_align;
		short ps = sample; /* 16 bit sample, FIXME: Endianess? */
		m_f.write(&ps, sizeof(ps));
	}

private:
	struct riff_chunk_t
	{
		char        group_id[4];
		unsigned    filelen;
		char        rifftype[4];
	};

	struct riff_format_t
	{
		char        signature[4];
		unsigned    fmt_length;
		short       format_tag;
		short       channels;
		unsigned    sample_rate;
		unsigned    bytes_per_second;
		short       block_align;
		short       bits_sample;
	};

	struct riff_data_t
	{
		char        signature[4];
		unsigned    len;
		// data follows
	};

	void initialize(unsigned sr)
	{
		std::strncpy(m_fh.group_id, "RIFF", 4);
		m_fh.filelen = 0; // Fixme
		std::strncpy(m_fh.rifftype, "WAVE", 4);

		std::strncpy(m_fmt.signature, "fmt ", 4);
		m_fmt.fmt_length = 16;
		m_fmt.format_tag = 0x0001; //PCM
		m_fmt.channels = 1;
		m_fmt.sample_rate = sr;
		m_fmt.bits_sample = 16;
		m_fmt.block_align = m_fmt.channels * ((m_fmt.bits_sample + 7) / 8);
		m_fmt.bytes_per_second = m_fmt.sample_rate * m_fmt.block_align;

		std::strncpy(m_data.signature, "data", 4);
		m_data.len = m_fmt.bytes_per_second * 2 * 0;

	}

	riff_chunk_t m_fh;
	riff_format_t m_fmt;
	riff_data_t m_data;

	plib::postream &m_f;

};

static void convert(nlwav_options_t &opts)
{
	plib::pofilestream fo(opts.opt_out());
	wav_t wo(fo, 48000);

	plib::pifilestream fin(opts.opt_inp());

	double dt = 1.0 / (double) wo.sample_rate();
	double ct = dt;
	//double mean = 2.4;
	double amp = opts.opt_amp();
	double mean = 0.0;
	double means = 0.0;
	double cursam = 0.0;
	double outsam = 0.0;
	double lt = 0.0;
	double maxsam = -1e9;
	double minsam = 1e9;
	int n = 0;
	//short sample = 0;
	pstring line;

	while(fin.readline(line))
	{
#if 1
		double t = 0.0; double v = 0.0;
		sscanf(line.cstr(), "%lf %lf", &t, &v);
		while (t >= ct)
		{
			outsam += (ct - lt) * cursam;
			outsam = outsam / dt;
			if (t>0.0)
			{
				means += outsam;
				maxsam = std::max(maxsam, outsam);
				minsam = std::min(minsam, outsam);
				n++;
				//mean = means / (double) n;
				mean += 5.0 / (double) wo.sample_rate() * (outsam - mean);
			}
			outsam = (outsam - mean) * amp;
			outsam = std::max(-32000.0, outsam);
			outsam = std::min(32000.0, outsam);
			wo.write_sample((int) outsam);
			outsam = 0.0;
			lt = ct;
			ct += dt;
		}
		outsam += (t-lt)*cursam;
		lt = t;
		cursam = v;
#else
		float t = 0.0; float v = 0.0;
		fscanf(FIN, "%f %f", &t, &v);
		while (ct <= t)
		{
			wo.write_sample(sample);
			n++;
			ct += dt;
		}
		means += v;
		mean = means / (double) n;
		v = v - mean;
		v = v * amp;
		if (v>32000.0)
			v = 32000.0;
		else if (v<-32000.0)
			v = -32000.0;
		sample = v;
		//printf("%f %f\n", t, v);
#endif
	}
	pout("Mean (low freq filter): {}\n", mean);
	pout("Mean (static):          {}\n", means / (double) n);
	pout("Amp + {}\n", 32000.0 / (maxsam- mean));
	pout("Amp - {}\n", -32000.0 / (minsam- mean));
}

static void usage(plib::pstream_fmt_writer_t &fw, nlwav_options_t &opts)
{
	fw("{}\n", opts.help("Convert netlist log files into wav files.\n",
			"nltool [options]").cstr());
}


int main(int argc, char *argv[])
{
	nlwav_options_t opts;
	int ret;

	if ((ret = opts.parse(argc, argv)) != argc)
	{
		perr("Error parsing {}\n", argv[ret]);
		usage(perr, opts);
		return 1;
	}

	if (opts.opt_help())
	{
		usage(pout, opts);
		return 0;
	}

	if (opts.opt_version())
	{
		pout(
			"nlwav (netlist) 0.1\n"
			"Copyright (C) 2016 Couriersud\n"
			"License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n\n"
			"Written by Couriersud.\n");
		return 0;
	}

	convert(opts);

	return 0;
}

/*
Der Daten-Abschnitt enth??lt die Abtastwerte:
Offset  L??nge  Inhalt  Beschreibung
36 (0x24)   4   'data'  Header-Signatur
40 (0x28)   4   <length>    L??nge des Datenblocks, max. <Dateigr????e>?????????44

0 (0x00)    char    4   'RIFF'
4 (0x04)    unsigned    4   <Dateigr????e>?????????8
8 (0x08)    char    4   'WAVE'

Der fmt-Abschnitt (24 Byte) beschreibt das Format der einzelnen Abtastwerte:
Offset  L??nge  Inhalt  Beschreibung
12 (0x0C)   4   'fmt '  Header-Signatur (folgendes Leerzeichen beachten)
16 (0x10)   4   <fmt length>    L??nge des restlichen fmt-Headers (16 Bytes)
20 (0x14)   2   <format tag>    Datenformat der Abtastwerte (siehe separate Tabelle weiter unten)
22 (0x16)   2   <channels>  Anzahl der Kan??le: 1 = mono, 2 = stereo; mittlerweile sind auch mehr als 2 Kan??le (z. B. f??r Raumklang) m??glich.[2]
24 (0x18)   4   <sample rate>   Samples pro Sekunde je Kanal (z. B. 44100)
28 (0x1C)   4   <bytes/second>  Abtastrate????????Frame-Gr????e
32 (0x20)   2   <block align>   Frame-Gr????e = <Anzahl der Kan??le>????????((<Bits/Sample (eines Kanals)>???+???7)???/???8)   (Division ohne Rest)
34 (0x22)   2   <bits/sample>   Anzahl der Datenbits pro Samplewert je Kanal (z. B. 12)
*/
