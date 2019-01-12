// license:GPL-2.0+
// copyright-holders:Couriersud
#include <cstring>
#include "../plib/pstring.h"
#include "../plib/plists.h"
#include "../plib/pstream.h"
#include "../plib/pmain.h"
#include "../plib/ppmf.h"
#include "../nl_setup.h"

class nlwav_app : public plib::app
{
public:
	nlwav_app() :
		plib::app(),
		opt_inp(*this,  "i", "input",       "-",      "input file"),
		opt_out(*this,  "o", "output",      "-",      "output file"),
		opt_amp(*this,  "a", "amp",    10000.0,      "amplification after mean correction"),
		opt_rate(*this, "r", "rate",   48000,        "sample rate of output file"),
		opt_verb(*this, "v", "verbose",              "be verbose - this produces lots of output"),
		opt_quiet(*this,"q", "quiet",                "be quiet - no warnings"),
		opt_version(*this,  "",  "version",          "display version and exit"),
		opt_help(*this, "h", "help",                 "display help and exit")
	{}
	plib::option_str    opt_inp;
	plib::option_str    opt_out;
	plib::option_num<double> opt_amp;
	plib::option_num<long>   opt_rate;
	plib::option_bool   opt_verb;
	plib::option_bool   opt_quiet;
	plib::option_bool   opt_version;
	plib::option_bool   opt_help;

	int execute();
	pstring usage();

	plib::pstdin pin_strm;
private:
	void convert1(long sample_rate);
	void convert(long sample_rate);
};


/* From: https://ffmpeg.org/pipermail/ffmpeg-devel/2007-October/038122.html
 * The most compatible way to make a wav header for unknown length is to put
 * 0xffffffff in the header. 0 as the RIFF length and 0 as the data chunk length
 * is a common agreement in serious recording applications while
 * still recording the file. So a playback application can determine that the
 * given file is still being recorded. As soon as the recording application
 * finishes the ongoing recording, it writes the correct values for RIFF lenth
 * and data chunk length to the file.
 */
/* http://de.wikipedia.org/wiki/RIFF_WAVE */

class wav_t
{
public:
	wav_t(plib::postream &strm, unsigned sr) : m_f(strm)
	{
		initialize(sr);
		write(m_fh);
		write(m_fmt);
		write(m_data);
	}
	~wav_t()
	{
		if (m_f.seekable())
		{
			m_fh.filelen = m_data.len + sizeof(m_data) + sizeof(m_fh) + sizeof(m_fmt) - 8;
			m_f.seekp(0);
			write(m_fh);
			write(m_fmt);

			//data.len = fmt.block_align * n;
			write(m_data);
		}
	}

	unsigned channels() { return m_fmt.channels; }
	unsigned sample_rate() { return m_fmt.sample_rate; }

	template <typename T>
	void write(const T &val)
	{
		m_f.write(reinterpret_cast<const char *>(&val), sizeof(T));
	}

	void write_sample(int sample)
	{
		m_data.len += m_fmt.block_align;
		int16_t ps = static_cast<int16_t>(sample); /* 16 bit sample, FIXME: Endianess? */
		write(ps);
	}

private:
	struct riff_chunk_t
	{
		uint8_t    group_id[4];
		uint32_t   filelen;
		uint8_t    rifftype[4];
	};

	struct riff_format_t
	{
		uint8_t             signature[4];
		uint32_t            fmt_length;
		uint16_t            format_tag;
		uint16_t            channels;
		uint32_t            sample_rate;
		uint32_t            bytes_per_second;
		uint16_t            block_align;
		uint16_t            bits_sample;
	};

	struct riff_data_t
	{
		uint8_t     signature[4];
		uint32_t    len;
		// data follows
	};

	void initialize(unsigned sr)
	{
		std::memcpy(m_fh.group_id, "RIFF", 4);
		m_fh.filelen = 0x0; // Fixme
		std::memcpy(m_fh.rifftype, "WAVE", 4);

		std::memcpy(m_fmt.signature, "fmt ", 4);
		m_fmt.fmt_length = 16;
		m_fmt.format_tag = 0x0001; //PCM
		m_fmt.channels = 1;
		m_fmt.sample_rate = sr;
		m_fmt.bits_sample = 16;
		m_fmt.block_align = m_fmt.channels * ((m_fmt.bits_sample + 7) / 8);
		m_fmt.bytes_per_second = m_fmt.sample_rate * m_fmt.block_align;

		std::memcpy(m_data.signature, "data", 4);
		//m_data.len = m_fmt.bytes_per_second * 2 * 0;
		/* force "play" to play and warn about eof instead of being silent */
		m_data.len = (m_f.seekable() ? 0 : 0xffffffff);

	}

	riff_chunk_t m_fh;
	riff_format_t m_fmt;
	riff_data_t m_data;

	plib::postream &m_f;

};

class log_processor
{
public:
	typedef plib::pmfp<void, double, double> callback_type;
	log_processor(plib::pistream &is, callback_type cb) : m_is(is), m_cb(cb) { }

	void process()
	{
		plib::putf8_reader reader(&m_is);
		pstring line;

		while(reader.readline(line))
		{
			double t = 0.0; double v = 0.0;
			sscanf(line.c_str(), "%lf %lf", &t, &v);
			m_cb(t, v);
		}
	}

private:
	plib::pistream &m_is;
	callback_type m_cb;
};

struct aggregator
{
	typedef plib::pmfp<void, double, double> callback_type;

	aggregator(double quantum, callback_type cb)
	: m_quantum(quantum)
	, m_cb(cb)
	, ct(0.0)
	, lt(0.0)
	, outsam(0.0)
	, cursam(0.0)
	{ }
	void process(double time, double val)
	{
		while (time >= ct)
		{
			outsam += (ct - lt) * cursam;
			outsam = outsam / m_quantum;
			m_cb(ct, outsam);
			outsam = 0.0;
			lt = ct;
			ct += m_quantum;
		}
		outsam += (time-lt)*cursam;
		lt = time;
		cursam = val;
	}

private:
	double m_quantum;
	callback_type m_cb;
	double ct;
	double lt;
	double outsam;
	double cursam;
};

class wavwriter
{
public:
	wavwriter(plib::postream &fo, unsigned sample_rate, double ampa)
	: mean(0.0)
	, means(0.0)
	, maxsam(-1e9)
	, minsam(1e9)
	, n(0)
	, m_fo(fo)
	, amp(ampa)
	, m_wo(m_fo, sample_rate)
	{ }

	void process(double time, double outsam)
	{
		means += outsam;
		maxsam = std::max(maxsam, outsam);
		minsam = std::min(minsam, outsam);
		n++;
		//mean = means / (double) n;
		mean += 5.0 / static_cast<double>(m_wo.sample_rate()) * (outsam - mean);

		outsam = (outsam - mean) * amp;
		outsam = std::max(-32000.0, outsam);
		outsam = std::min(32000.0, outsam);
		m_wo.write_sample(static_cast<int>(outsam));
	}

	double mean;
	double means;
	double maxsam;
	double minsam;
	std::size_t n;

private:
	plib::postream &m_fo;
	double amp;
	wav_t m_wo;
};

void nlwav_app::convert(long sample_rate)
{
	plib::postream *fo = (opt_out() == "-" ? &pout_strm : plib::palloc<plib::pofilestream>(opt_out()));
	plib::pistream *fin = (opt_inp() == "-" ? &pin_strm : plib::palloc<plib::pifilestream>(opt_inp()));
	plib::putf8_reader reader(fin);
	wav_t *wo = plib::palloc<wav_t>(*fo, static_cast<unsigned>(sample_rate));

	double dt = 1.0 / static_cast<double>(wo->sample_rate());
	double ct = dt;
	//double mean = 2.4;
	double amp = opt_amp();
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

	while(reader.readline(line))
	{
#if 1
		double t = 0.0; double v = 0.0;
		sscanf(line.c_str(), "%lf %lf", &t, &v);
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
				mean += 5.0 / static_cast<double>(wo->sample_rate()) * (outsam - mean);
			}
			outsam = (outsam - mean) * amp;
			outsam = std::max(-32000.0, outsam);
			outsam = std::min(32000.0, outsam);
			wo->write_sample(static_cast<int>(outsam));
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
	plib::pfree(wo);
	if (opt_inp() != "-")
		plib::pfree(fin);
	if (opt_out() != "-")
		plib::pfree(fo);

	if (!opt_quiet())
	{
		perr("Mean (low freq filter): {}\n", mean);
		perr("Mean (static):          {}\n", means / static_cast<double>(n));
		perr("Amp + {}\n", 32000.0 / (maxsam- mean));
		perr("Amp - {}\n", -32000.0 / (minsam- mean));
	}
}

void nlwav_app::convert1(long sample_rate)
{
	plib::postream *fo = (opt_out() == "-" ? &pout_strm : plib::palloc<plib::pofilestream>(opt_out()));
	plib::pistream *fin = (opt_inp() == "-" ? &pin_strm : plib::palloc<plib::pifilestream>(opt_inp()));

	double dt = 1.0 / static_cast<double>(sample_rate);

	wavwriter *wo = plib::palloc<wavwriter>(*fo, static_cast<unsigned>(sample_rate), opt_amp());
	aggregator ag(dt, aggregator::callback_type(&wavwriter::process, wo));
	log_processor lp(*fin, log_processor::callback_type(&aggregator::process, &ag));

	lp.process();

	if (!opt_quiet())
	{
		perr("Mean (low freq filter): {}\n", wo->mean);
		perr("Mean (static):          {}\n", wo->means / static_cast<double>(wo->n));
		perr("Amp + {}\n", 32000.0 / (wo->maxsam - wo->mean));
		perr("Amp - {}\n", -32000.0 / (wo->minsam - wo->mean));
	}

	plib::pfree(wo);
	if (opt_inp() != "-")
		plib::pfree(fin);
	if (opt_out() != "-")
		plib::pfree(fo);

}

pstring nlwav_app::usage()
{
	return help("Convert netlist log files into wav files.\n",
			"nltool [options]");
}


int nlwav_app::execute()
{
	if (opt_help())
	{
		pout(usage());
		return 0;
	}

	if (opt_version())
	{
		pout(
			"nlwav (netlist) 0.1\n"
			"Copyright (C) 2019 Couriersud\n"
			"License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n\n"
			"Written by Couriersud.\n");
		return 0;
	}

	if ((1))
		convert1(opt_rate());
	else
		convert(opt_rate());

	return 0;
}

PMAIN(nlwav_app)

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
