// license:GPL-2.0+
// copyright-holders:Couriersud
#include "plib/pstring.h"
#include "netlist/nl_setup.h"
#include "plib/plists.h"
#include "plib/pmain.h"
#include "plib/ppmf.h"
#include "plib/pstream.h"

#include <cstdio>

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
	// XXNOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
	wav_t(plib::postream &strm, bool is_seekable, std::size_t sr, std::size_t channels)
	: m_f(strm)
	, m_stream_is_seekable(is_seekable)
	/* force "play" to play and warn about eof instead of being silent */
	, m_fmt(static_cast<std::uint16_t>(channels), static_cast<std::uint32_t>(sr))
	, m_data(is_seekable ? 0 : 0xffffffff)
	{

		write(m_fh);
		write(m_fmt);
		write(m_data);
	}

	COPYASSIGNMOVE(wav_t, delete)

	~wav_t()
	{
		if (m_stream_is_seekable)
		{
			m_fh.filelen = m_data.len + sizeof(m_data) + sizeof(m_fh) + sizeof(m_fmt) - 8;
			m_f.seekp(0);
			write(m_fh);
			write(m_fmt);

			//data.len = fmt.block_align * n;
			write(m_data);
		}
	}

	std::size_t channels() { return m_fmt.channels; }
	std::size_t sample_rate() { return m_fmt.sample_rate; }

	template <typename T>
	void write(const T &val)
	{
		m_f.write(reinterpret_cast<const plib::postream::char_type *>(&val), sizeof(T));
	}

	void write_sample(int *sample)
	{
		m_data.len += m_fmt.block_align;
		for (std::size_t i = 0; i < channels(); i++)
		{
			auto ps = static_cast<int16_t>(sample[i]); /* 16 bit sample, FIXME: Endianess? */
			write(ps);
		}
	}

private:
	struct riff_chunk_t
	{
		uint8_t    group_id[4]   = {'R','I','F','F'};
		uint32_t   filelen       = 0;
		uint8_t    rifftype[4]   = {'W','A','V','E'};
	};

	struct riff_format_t
	{
		riff_format_t(uint16_t achannels, uint32_t asample_rate)
		{
			channels = achannels;
			sample_rate = asample_rate;
			block_align = channels * ((bits_sample + 7) / 8);
			bytes_per_second = sample_rate * block_align;
		}
		uint8_t             signature[4] = {'f','m','t',' '};
		uint32_t            fmt_length   = 16;
		uint16_t            format_tag   = 0x0001; // PCM
		uint16_t            channels;
		uint32_t            sample_rate;
		uint32_t            bytes_per_second;
		uint16_t            block_align;
		uint16_t            bits_sample  = 16;
	};

	struct riff_data_t
	{
		riff_data_t(uint32_t alen) : len(alen) {}
		uint8_t     signature[4] = {'d','a','t','a'};
		uint32_t    len;
		// data follows
	};

	plib::postream &m_f;
	bool m_stream_is_seekable;

	riff_chunk_t m_fh;
	riff_format_t m_fmt;
	riff_data_t m_data;

};

class log_processor
{
public:
	using callback_type = plib::pmfp<void, std::size_t, double, double>;

	struct elem
	{
		elem() : t(0), v(0), eof(false), need_more(true) { }
		double t;
		double v;
		bool eof;
		bool need_more;
	};

	log_processor(std::size_t channels, callback_type &cb)
	: m_cb(cb)
	, m_e(channels)
	{ }

	bool readmore(std::vector<plib::putf8_reader> &r)
	{
		bool success = false;
		for (std::size_t i = 0; i< r.size(); i++)
		{
			if (m_e[i].need_more)
			{
				pstring line;
				m_e[i].eof = !r[i].readline(line);
				if (!m_e[i].eof)
				{
					// sscanf is very fast ...
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
					std::sscanf(line.c_str(), "%lf %lf", &m_e[i].t, &m_e[i].v);
					m_e[i].need_more = false;
				}
			}
			success |= !m_e[i].eof;
		}
		return success;
	}

	void process(std::vector<plib::unique_ptr<plib::pistream>> &is)
	{
		std::vector<plib::putf8_reader> readers;
		for (auto &i : is)
		{
			plib::putf8_reader r(std::move(i));
			readers.push_back(std::move(r));
		}

		pstring line;
		bool more = readmore(readers);

		while (more)
		{
			double mint = 1e200;
			std::size_t mini = 0;
			for (std::size_t i = 0; i<readers.size(); i++)
				if (!m_e[i].need_more)
				{
					if (m_e[i].t < mint)
					{
						mint = m_e[i].t;
						mini = i;
					}
				}

			m_e[mini].need_more = true;
			m_cb(mini, mint, m_e[mini].v);
			more = readmore(readers);
		}
	}

private:
	callback_type m_cb;
	std::vector<elem> m_e;
};

struct aggregator
{
	using callback_type = plib::pmfp<void, std::size_t, double, double>;

	aggregator(std::size_t channels, double quantum, callback_type cb)
	: m_channels(channels)
	, m_quantum(quantum)
	, m_cb(cb)
	, ct(0.0)
	, lt(0.0)
	, outsam(channels, 0.0)
	, cursam(channels, 0.0)
	{ }
	void process(std::size_t chan, double time, double val)
	{
		while (time >= ct + m_quantum)
		{
			for (std::size_t i=0; i< m_channels; i++)
			{
				outsam[i] += (ct - lt) * cursam[i];
				outsam[i] = outsam[i] / m_quantum;
				m_cb(i, ct, outsam[i]);
				outsam[i] = 0.0;
			}
			lt = ct;
			ct += m_quantum;
		}
		for (std::size_t i=0; i< m_channels; i++)
			outsam[i] += (time-lt)*cursam[i];
		lt = time;
		cursam[chan] = val;
	}

private:
	std::size_t m_channels;
	double m_quantum;
	callback_type m_cb;
	double ct;
	double lt;
	std::vector<double> outsam;
	std::vector<double> cursam;
};

class wavwriter
{
public:
	wavwriter(plib::postream &fo, bool is_seekable, std::size_t channels, std::size_t sample_rate, double ampa)
	: mean(channels, 0.0)
	, means(channels, 0.0)
	, maxsam(channels, -1e9)
	, minsam(channels, 1e9)
	, m_n(channels, 0)
	, m_samples(channels, 0)
	, m_last_time(0)
	, m_fo(fo)
	, m_amp(ampa)
	, m_wo(m_fo, is_seekable, sample_rate, channels)
	{ }

	void process(std::size_t chan, double time, double outsam)
	{
		if (time > m_last_time)
			m_wo.write_sample(m_samples.data());
		m_last_time = time;
		means[chan] += outsam;
		maxsam[chan] = std::max(maxsam[chan], outsam);
		minsam[chan] = std::min(minsam[chan], outsam);
		m_n[chan]++;
		//mean = means / (double) m_n;
		mean[chan] += 5.0 / static_cast<double>(m_wo.sample_rate()) * (outsam - mean[chan]);

		outsam = (outsam - mean[chan]) * m_amp;
		outsam = std::max(-32000.0, outsam);
		outsam = std::min(32000.0, outsam);
		m_samples[chan] = static_cast<int>(outsam);
	}

	std::vector<double> mean;
	std::vector<double> means;
	std::vector<double> maxsam;
	std::vector<double> minsam;
	std::vector<std::size_t> m_n;
	std::vector<int> m_samples;
	double m_last_time;

private:

	plib::postream &m_fo;
	double m_amp;
	wav_t m_wo;
};

class vcdwriter
{
public:

	enum format_e
	{
		DIGITAL,
		ANALOG
	};

	vcdwriter(plib::postream &fo, const std::vector<pstring> &channels,
		format_e format, double high_level = 2.0, double low_level = 1.0)
	: m_channels(channels.size())
	, m_last_time(0)
	, m_fo(fo)
	, m_high_level(high_level)
	, m_low_level(low_level)
	, m_format(format)
	{
		for (pstring::value_type c = 64; c < 64+26; c++)
			m_ids.emplace_back(pstring(1, c));
		write("$date Sat Jan 19 14:14:17 2019\n");
		write("$end\n");
		write("$version Netlist nlwav 0.1\n");
		write("$end\n");
		write("$timescale 1 ns\n");
		write("$end\n");
		std::size_t i = 0;
		for (const auto &ch : channels)
		{
			//      $var real 64 N1X1 N1X1 $end
			if (format == ANALOG)
				write(pstring("$var real 64 ") + m_ids[i++] + " " + ch + " $end\n");
			else if (format == DIGITAL)
				write(pstring("$var wire 1 ") + m_ids[i++] + " " + ch + " $end\n");
		}
		write("$enddefinitions $end\n");
		if (format == ANALOG)
		{
			write("$dumpvars\n");
			//r0.0 N1X1
			for (i = 0; i < channels.size(); i++)
				write(pstring("r0.0 ") + m_ids[i] + "\n");
			write("$end\n");
		}

	}

	void process(std::size_t chan, double time, double outsam)
	{
		if (time > m_last_time)
		{
			write(pstring("#") + plib::to_string(static_cast<std::int64_t>(m_last_time * 1e9)) + " ");
			write(m_buf + "\n");
			m_buf = "";
			m_last_time = time;
		}
		if (m_format == ANALOG)
			m_buf += "r" + plib::to_string(outsam)+ " " + m_ids[chan] + " ";
		else
		{
			if (outsam >= m_high_level)
				m_buf += pstring("1") + m_ids[chan] + " ";
			else if (outsam <= m_low_level)
				m_buf += pstring("0") + m_ids[chan] + " ";
		}
	}

private:
	void write(const pstring &line)
	{
		m_fo.write(line.c_str(), static_cast<std::streamsize>(plib::strlen(line.c_str())));
	}

	std::size_t m_channels;
	double m_last_time;

	plib::postream &m_fo;
	std::vector<pstring> m_ids;
	pstring m_buf;
	double m_high_level;
	double m_low_level;
	format_e m_format;
};

class nlwav_app : public plib::app
{
public:
	nlwav_app() :
		plib::app(),
		opt_fmt(*this,  "f", "format",      0,       std::vector<pstring>({"wav","vcda","vcdd"}),
			"output format. Available options are wav|vcda|vcdd."
			" wav  : multichannel wav output"
			" vcda : analog VCD output"
			" vcdd : digital VCD output"
			" Digital signals are created using the --high and --low options"
			),
		opt_out(*this,  "o", "output",      "-",     "output file"),
		opt_rate(*this, "r", "rate",   48000,        "sample rate of output file"),
		opt_amp(*this,  "a", "amp",    10000.0,      "amplification after mean correction (wav only)"),
		opt_high(*this, "u", "high",   2.0,          "minimum input for high level (vcdd only)"),
		opt_low(*this,  "l", "low",   1.0,           "maximum input for low level (vcdd only)"),
		opt_verb(*this, "v", "verbose",              "be verbose - this produces lots of output"),
		opt_quiet(*this,"q", "quiet",                "be quiet - no warnings"),
		opt_args(*this,                              "input file(s)"),
		opt_version(*this,  "",  "version",          "display version and exit"),
		opt_help(*this, "h", "help",                 "display help and exit"),
		opt_ex1(*this, "./nlwav -f vcdd -o x.vcd log_V*",
			"convert all files starting with \"log_V\" into a digital vcd file"),
		opt_ex2(*this, "./nlwav -f wav -o x.wav log_V*",
			"convert all files starting with \"log_V\" into a multichannel wav file"),
		m_outstrm(nullptr)
	{}

	int execute() override;
	pstring usage() override;

private:
	void convert_wav();
	void convert_vcd(vcdwriter::format_e format);

	plib::option_str_limit<unsigned> opt_fmt;
	plib::option_str    opt_out;
	plib::option_num<std::size_t>   opt_rate;
	plib::option_num<double> opt_amp;
	plib::option_num<double> opt_high;
	plib::option_num<double> opt_low;
	plib::option_bool   opt_verb;
	plib::option_bool   opt_quiet;
	plib::option_args   opt_args;
	plib::option_bool   opt_version;
	plib::option_bool   opt_help;
	plib::option_example   opt_ex1;
	plib::option_example   opt_ex2;
#if !USE_CSTREAM
	plib::pstdin pin_strm;
#endif
	std::vector<plib::unique_ptr<plib::pistream>> m_instrms;
	plib::postream *m_outstrm;
};

void nlwav_app::convert_wav()
{

	double dt = 1.0 / static_cast<double>(opt_rate());

	plib::unique_ptr<wavwriter> wo = plib::make_unique<wavwriter>(*m_outstrm, opt_out() != "-", m_instrms.size(), opt_rate(), opt_amp());
	plib::unique_ptr<aggregator> ago = plib::make_unique<aggregator>(m_instrms.size(), dt, aggregator::callback_type(&wavwriter::process, wo.get()));
	aggregator::callback_type agcb = log_processor::callback_type(&aggregator::process, ago.get());

	log_processor lp(m_instrms.size(), agcb);

	lp.process(m_instrms);

	if (!opt_quiet())
	{
#if 0
		perr("Mean (low freq filter): {}\n", wo->mean);
		perr("Mean (static):          {}\n", wo->means / static_cast<double>(wo->m_n));
		perr("Amp + {}\n", 32000.0 / (wo->maxsam - wo->mean));
		perr("Amp - {}\n", -32000.0 / (wo->minsam - wo->mean));
#endif
	}
}

void nlwav_app::convert_vcd(vcdwriter::format_e format)
{

	plib::unique_ptr<vcdwriter> wo = plib::make_unique<vcdwriter>(*m_outstrm, opt_args(),
		format, opt_high(), opt_low());
	log_processor::callback_type agcb = log_processor::callback_type(&vcdwriter::process, wo.get());

	log_processor lp(m_instrms.size(), agcb);

	lp.process(m_instrms);

	if (!opt_quiet())
	{
#if 0
		perr("Mean (low freq filter): {}\n", wo->mean);
		perr("Mean (static):          {}\n", wo->means / static_cast<double>(wo->m_n));
		perr("Amp + {}\n", 32000.0 / (wo->maxsam - wo->mean));
		perr("Amp - {}\n", -32000.0 / (wo->minsam - wo->mean));
#endif
	}
}

pstring nlwav_app::usage()
{
	return help("Convert netlist log files into wav files.\n",
			"nlwav [OPTION] ... [FILE] ...");
}


int nlwav_app::execute()
{
	for (auto &i : opt_args())
		pout(pstring("Hello : ") + i + "\n");
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

#if !USE_CSTREAM
	m_outstrm = (opt_out() == "-" ? &pout_strm : plib::pnew<plib::pofilestream>(opt_out()));
#else
	m_outstrm = (opt_out() == "-" ? &std::cout : plib::pnew<plib::pofilestream>(opt_out()));
#endif

	for (auto &oi: opt_args())
	{
#if !USE_CSTREAM
		plib::unique_ptr<plib::pistream> fin = (oi == "-" ?
			  plib::make_unique<plib::pstdin>()
			: plib::make_unique<plib::pifilestream>(oi));
#else
		//FIXME:
		plib::unique_ptr<plib::pistream> fin = plib::make_unique<plib::pifilestream>(oi);
#endif
		m_instrms.push_back(std::move(fin));
	}

	switch (opt_fmt())
	{
		case 0:
			convert_wav(); break;
		case 1:
			convert_vcd(vcdwriter::ANALOG); break;
		case 2:
			convert_vcd(vcdwriter::DIGITAL); break;
		default:
			// tease compiler - can't happen
			break;
	}

	if (opt_out() != "-")
		plib::pdelete(m_outstrm);

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
