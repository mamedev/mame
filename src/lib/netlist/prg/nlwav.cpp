// license:BSD-3-Clause
// copyright-holders:Couriersud
#include "plib/pstring.h"
#include "plib/pmain.h"
#include "plib/ppmf.h"
#include "plib/pstream.h"
#include "plib/pstrutil.h"

#include <cstdio>

// FIXME: These are either defined in other parts or have more complex consequences
// spell-checker: words nlconst, rpad, pfmt, pexception, pbinary, dynlib, pstring, psplit, putf, pmfp
//
// Specific technical terms
// spell-checker: words vcda, vcdd

// FIXME: see below - this belongs somewhere else!
#ifdef _WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

// From: https://ffmpeg.org/pipermail/ffmpeg-devel/2007-October/038122.html
// The most compatible way to make a wav header for unknown length is to put
// 0xffffffff in the header. 0 as the RIFF length and 0 as the data chunk length
// is a common agreement in serious recording applications while
// still recording the file. So a playback application can determine that the
// given file is still being recorded. As soon as the recording application
// finishes the ongoing recording, it writes the correct values for RIFF length
// and data chunk length to the file.
//
// http://de.wikipedia.org/wiki/RIFF_WAVE
//

using arena = plib::aligned_arena<>;

class wav_t
{
public:

	enum format
	{
		s16,
		s32,
		f32
	};

	wav_t(std::ostream &strm, bool is_seekable, format fmt, std::size_t sr, std::size_t channels)
	: m_f(strm)
	, m_stream_is_seekable(is_seekable)
	, m_format(fmt)
	// force "play" to play and warn about eof instead of being silent
	, m_fmt(static_cast<std::uint16_t>(channels), static_cast<std::uint32_t>(sr), fmt)
	, m_data(is_seekable ? 0 : 0xffffffff)
	{

		write(m_fh);
		write(m_fmt);
		write(m_data);
	}

	PCOPYASSIGNMOVE(wav_t, delete)

	~wav_t()
	{
		if (m_stream_is_seekable)
		{
			m_fh.file_len = m_data.len + sizeof(m_data) + sizeof(m_fh) + sizeof(m_fmt) - 8;
			m_f.seekp(0);
			write(m_fh);
			write(m_fmt);

			//data.len = fmt.block_align * n;
			write(m_data);
		}
	}

	std::size_t channels() const { return m_fmt.channels; }
	std::size_t sample_rate() const { return m_fmt.sample_rate; }

	template <typename T>
	void write(const T &val)
	{
		plib::ostream_write(m_f, &val, 1);
	}

	template <typename T>
	void write_sample_int(double sample)
	{
		constexpr auto type_max(static_cast<double>(plib::numeric_limits<T>::max()));
		constexpr auto type_min(static_cast<double>(plib::numeric_limits<T>::min()));

		sample *= type_max;
		sample = std::max(type_min, sample);
		sample = std::min(type_max, sample);
		const T dest(static_cast<T>(sample));
		write(dest);
	}

	// expects normalized samples between -1.0 to 1.0 for s16 and s32
	void write_samples(double *sample)
	{
		m_data.len += m_fmt.block_align;
		for (std::size_t i = 0; i < channels(); i++)
		{
			switch (m_format)
			{
				case s16:
					write_sample_int<int16_t>(sample[i]);
					break;
				case s32:
					write_sample_int<int32_t>(sample[i]);
					break;
				case f32:
					const auto df32(static_cast<float>(sample[i]));
					write(df32);
					break;
			}
		}
	}

private:
	struct riff_chunk_t
	{
		std::array<uint8_t, 4> group_id = {{'R','I','F','F'}};
		uint32_t               file_len  = 0;
		std::array<uint8_t, 4> riff_type = {{'W','A','V','E'}};
	};

	struct riff_format_t
	{
		riff_format_t(uint16_t num_channels, uint32_t a_sample_rate, format fm)
		{
			switch (fm)
			{
				case s16:
					format_tag = 0x0001; // PCM
					bits_sample  = 16;
					break;
				case s32:
					format_tag = 0x0001; // PCM
					bits_sample  = 32;
					break;
				case f32:
					format_tag = 0x0003; // FLOAT
					bits_sample  = 32;
					break;
			}
			channels = num_channels;
			sample_rate = a_sample_rate;
			block_align = channels * ((bits_sample + 7) / 8);
			bytes_per_second = sample_rate * block_align;
		}
		std::array<uint8_t, 4> signature = {{'f','m','t',' '}};
		uint32_t            fmt_length   = 16;
		uint16_t            format_tag;
		uint16_t            channels;
		uint32_t            sample_rate;
		uint32_t            bytes_per_second;
		uint16_t            block_align;
		uint16_t            bits_sample;
	};

	struct riff_data_t
	{
		explicit riff_data_t(uint32_t alen) : len(alen) {}
		std::array<uint8_t, 4> signature = {{'d','a','t','a'}};
		uint32_t    len;
		// data follows
	};

	std::ostream &m_f;
	bool m_stream_is_seekable;
	format m_format;

	riff_chunk_t m_fh;
	riff_format_t m_fmt;
	riff_data_t m_data;

};

class log_processor
{
public:
	using callback_type = plib::pmfp<void (std::size_t, double, double)>;

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

	bool read_more(std::vector<plib::putf8_reader> &r)
	{
		bool success = false;
		for (std::size_t i = 0; i< r.size(); i++)
		{
			if (m_e[i].need_more)
			{
				putf8string line;
				m_e[i].eof = !r[i].read_line(line);
				if (!m_e[i].eof)
				{
					// sscanf is very fast ...
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
					if (2 != std::sscanf(line.c_str(), "%lf %lf", &m_e[i].t, &m_e[i].v))
						fprintf(stderr, "arg: <%s>\n", line.c_str());
					m_e[i].need_more = false;
				}
			}
			success |= !m_e[i].eof;
		}
		return success;
	}

	void process(std::vector<std::unique_ptr<std::istream>> &is)
	{
		std::vector<plib::putf8_reader> readers;
		for (auto &i : is)
		{
			plib::putf8_reader r(std::move(i));
			readers.push_back(std::move(r));
		}

		pstring line;
		bool more = read_more(readers);

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
			more = read_more(readers);
		}
	}


private:
	callback_type m_cb;
	std::vector<elem> m_e;
};

struct aggregator
{
	using callback_type = plib::pmfp<void (std::size_t, double, double)>;

	aggregator(std::size_t channels, double quantum, callback_type cb)
	: m_channels(channels)
	, m_quantum(quantum)
	, m_cb(cb)
	, ct(0.0)
	, lt(0.0)
	, output_samples(channels, 0.0)
	, current_samples(channels, 0.0)
	{ }
	void process(std::size_t chan, double time, double val)
	{
		while (time >= ct + m_quantum)
		{
			ct += m_quantum;
			for (std::size_t i=0; i< m_channels; i++)
			{
				output_samples[i] += (ct - lt) * current_samples[i];
				output_samples[i] = output_samples[i] / m_quantum;
				m_cb(i, ct, output_samples[i]);
				output_samples[i] = 0.0;
			}
			lt = ct;
		}
		for (std::size_t i=0; i< m_channels; i++)
			output_samples[i] += (time-lt)*current_samples[i];
		lt = time;
		current_samples[chan] = val;
	}

private:
	std::size_t m_channels;
	double m_quantum;
	callback_type m_cb;
	double ct;
	double lt;
	std::vector<double> output_samples;
	std::vector<double> current_samples;
};

struct filter_hp
{
	using callback_type = plib::pmfp<void (std::size_t, double, double)>;

	filter_hp(double freq, bool boost, std::size_t channels, callback_type cb)
	: m_cb(cb)
	, m_hp_omega(plib::constants<double>::two() * plib::constants<double>::pi() * freq)
	, m_boost(boost)
	, m_lt(channels, 0.0)
	, m_in(channels, 0.0)
	, m_cap(channels, 0.0)
	{ }
	void process(std::size_t chan, double time, double val)
	{
		// based on CR filter
		auto dt(time - m_lt[chan]);

		double omega = ((m_boost && (time < 1.0/m_hp_omega)) ? 1e12 : m_hp_omega);
		auto m(1.0 - plib::exp(-dt * omega));
		m_cap[chan] += m * (m_in[chan] - m_cap[chan]);
		// out = in - vcap
		m_cb(chan, time, m_in[chan] - m_cap[chan]);

		m_in[chan] = val;
		m_lt[chan] = time;
	}

private:
	callback_type m_cb;
	double m_hp_omega;
	bool m_boost;
	std::vector<double> m_lt;
	std::vector<double> m_in;
	std::vector<double> m_cap;
};

struct filter_lp
{
	using callback_type = plib::pmfp<void (std::size_t, double, double)>;

	filter_lp(double freq, std::size_t channels, callback_type cb)
	: m_cb(cb)
	, m_lp_omega(plib::constants<double>::two() * plib::constants<double>::pi() * freq)
	, m_lt(channels, 0.0)
	, m_in(channels, 0.0) // lp filter
	, m_cap(channels, 0.0) // hp filter
	{ }
	void process(std::size_t chan, double time, double val)
	{
		// based on RC filter
		auto dt(time - m_lt[chan]);

		auto m(1.0 - plib::exp(-dt * m_lp_omega));

		m_cap[chan] += m * (m_in[chan] - m_cap[chan]);
		// out = vcap
		m_cb(chan, time, m_cap[chan]);

		m_in[chan] = val;
		m_lt[chan] = time;
	}

private:
	callback_type m_cb;
	double m_lp_omega;
	std::vector<double> m_lt;
	std::vector<double> m_in;
	std::vector<double> m_cap;
};

class wav_writer
{
public:
	wav_writer(std::ostream &fo, bool is_seekable, wav_t::format fmt,
		std::size_t channels, std::size_t sample_rate, double amplification)
	: max_samples(channels, -1e9)
	, min_samples(channels, 1e9)
	, m_n(channels, 0)
	, m_samples(channels, 0)
	, m_last_time(0)
	, m_fo(fo)
	, m_amp(amplification <= 0.0 ? 1.0e6 : amplification)
	, m_auto(amplification <= 0.0)
	, m_wo(m_fo, is_seekable, fmt, sample_rate, channels)
	{ }

	void process(std::size_t chan, double time, double sample)
	{
		if (time > m_last_time)
			m_wo.write_samples(m_samples.data());
		m_last_time = time;
		max_samples[chan] = std::max(max_samples[chan], sample);
		min_samples[chan] = std::min(min_samples[chan], sample);
		m_n[chan]++;

		auto val(sample * m_amp);
		if (m_auto && plib::abs(val) > 1.0)
		{
			do
			{
				m_amp /= 2.0;
				val = sample * m_amp;
			} while (plib::abs(val) > 1.0);
			// FIXME: log this in state and provide on verbose output
			//printf("dynamic amplification adjusted to %f at %f\n", m_amp, time);
		}
		m_samples[chan] = val;
	}

	std::vector<double> max_samples;
	std::vector<double> min_samples;
	std::vector<std::size_t> m_n;
	std::vector<double> m_samples;
	double m_last_time;

private:

	std::ostream &m_fo;
	double m_amp;
	bool m_auto;
	wav_t m_wo;
};

class vcd_writer
{
public:

	enum format_e
	{
		DIGITAL,
		ANALOG
	};

	vcd_writer(std::ostream &fo, const std::vector<pstring> &channels,
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
				write("$var real 64 " + m_ids[i++] + " " + ch + " $end\n");
			else if (format == DIGITAL)
				write("$var wire 1 " + m_ids[i++] + " " + ch + " $end\n");
		}
		write("$enddefinitions $end\n");
		if (format == ANALOG)
		{
			write("$dumpvars\n");
			//r0.0 N1X1
			for (i = 0; i < channels.size(); i++)
				write("r0.0 " + m_ids[i] + "\n");
			write("$end\n");
		}

	}

	void process(std::size_t chan, double time, double sample)
	{
		if (time > m_last_time)
		{
			write("#" + plib::to_string(static_cast<std::int64_t>(m_last_time * 1e9)) + " ");
			write(m_buf + "\n");
			m_buf = "";
			m_last_time = time;
		}
		if (m_format == ANALOG)
			m_buf += "r" + plib::to_string(sample)+ " " + m_ids[chan] + " ";
		else
		{
			if (sample >= m_high_level)
				m_buf += "1" + m_ids[chan] + " ";
			else if (sample <= m_low_level)
				m_buf += "0" + m_ids[chan] + " ";
		}
	}

private:
	void write(const pstring &line)
	{
		const putf8string u8line(line);
		m_fo.write(u8line.c_str(), static_cast<std::streamsize>(plib::strlen(u8line.c_str())));
	}

	std::size_t m_channels;
	double m_last_time;

	std::ostream &m_fo;
	std::vector<pstring> m_ids;
	pstring m_buf;
	double m_high_level;
	double m_low_level;
	format_e m_format;
};

class tab_writer
{
public:

	enum format_e
	{
		DIGITAL,
		ANALOG
	};

	tab_writer(std::ostream &fo, const std::vector<pstring> &channels,
		double start, double inc, std::size_t samples)
	: m_last_time(0)
	, m_next_time(start)
	, m_fo(fo)
	, m_inc(inc)
	, m_samples(samples)
	, m_buf(channels.size())
	, m_n(0)
	{
	}

	void process(std::size_t chan, double time, double sample)
	{
		if (time > m_last_time)
		{
			if (m_n < m_samples)
			{
				while (m_next_time < time && m_n < m_samples)
				{
					pstring o;
					for (auto &e : m_buf)
					{
						o += pstring(",") + plib::to_string(e); // FIXME: locale!!
					}
					write(o.substr(1) + "\n");
					m_n++;
					m_next_time += m_inc;
				}
			}
			m_last_time = time;
		}
		m_buf[chan] = sample;
	}

private:
	void write(const pstring &line)
	{
		const putf8string u8line(line);
		m_fo.write(u8line.c_str(), static_cast<std::streamsize>(plib::strlen(u8line.c_str())));
	}

	double m_last_time;
	double m_next_time;

	std::ostream &m_fo;
	std::vector<pstring> m_ids;
	double m_inc;
	std::size_t m_samples;
	std::vector<double> m_buf;
	std::size_t m_n;
};

class nlwav_app : public plib::app
{
public:
	nlwav_app() :
		plib::app(),
		opt_fmt(*this,  "f", "format",      0,       std::vector<pstring>({"wav16s","wav32s","wav32f","vcda","vcdd", "tab"}),
			"output format. Available options are wav16s|wav32s|wav32f|vcda|vcdd|tab.\n"
			" wav16s  : multichannel wav output 16 bit signed\n"
			" wav32s  : multichannel wav output 32 bit signed\n"
			" wav32f  : multichannel wav output 32 bit float\n"
			" vcda : analog VCD output\n"
			" vcdd : digital VCD output\n"
			" tab  : sampled output\n"
			" Digital signals are created using the --high and --low options"
			),
		opt_out(*this,  "o", "output",      "-",     "output file"),
		opt_grp1(*this, "wav options", "These options apply to wav output only"),
		opt_rate(*this, "r", "rate",   48000,        "sample rate of output file"),
		opt_amp(*this,  "a", "amp",    10000.0,      "amplification after mean correction"),
		opt_lowpass(*this,  "", "lowpass",    20000.0,      "lowpass filter frequency.\nDefault {1:.0} Hz."),
		opt_highpass(*this,  "", "highpass",    20.0, "highpass filter frequency.\nDefault is {1:.0} Hz."),
		opt_hp_boost(*this,  "", "hp-boost",           "enable highpass boost to filter out initial click."),
		opt_grp2(*this, "vcdd options", "These options apply to vcdd output only"),
		opt_high(*this, "u", "high",   2.0,          "minimum input for high level"),
		opt_low(*this,  "l", "low",   1.0,           "maximum input for low level"),
		opt_grp3(*this, "tab options", "These options apply to sampled output only"),
		opt_start(*this, "s", "start",   0.0,        "time when sampling starts"),
		opt_inc(*this, "i", "increment", 0.001,      "time between samples"),
		opt_samples(*this, "n", "samples",   1000000,"number of samples"),
		opt_grp4(*this, "General options", "These options always apply"),
		opt_verb(*this, "v", "verbose",              "be verbose - this produces lots of output"),
		opt_quiet(*this,"q", "quiet",                "be quiet - no warnings"),
		opt_args(*this,                              "input file(s)"),
		opt_version(*this,  "",  "version",          "display version and exit"),
		opt_help(*this, "h", "help",                 "display help and exit"),
		opt_ex1(*this, "./nlwav -f vcdd -o x.vcd log_V*",
			"convert all files starting with \"log_V\" into a digital vcd file"),
		opt_ex2(*this, "./nlwav -f wav16s -o x.wav log_V*",
			"convert all files starting with \"log_V\" into a multichannel wav file (16bit, signed)"),
		opt_ex3(*this, "./nlwav -f tab -o x.tab -s 0.0000005 -i 0.000001 -n 256 log_BLUE.log",
			"convert file log_BLUE.log to sampled output. First sample at 500ns "
			"followed by 255 samples every micro-second.")
	{}

	int execute() override;
	pstring usage() override;

private:
	void convert_wav(std::ostream &output, wav_t::format fmt);
	void convert_vcd(std::ostream &output, vcd_writer::format_e format);
	void convert_tab(std::ostream &output);
	void convert(const pstring &output_file);

	plib::option_str_limit<unsigned> opt_fmt;
	plib::option_str    opt_out;
	plib::option_group opt_grp1;
	plib::option_num<std::size_t>   opt_rate;
	plib::option_num<double> opt_amp;
	plib::option_num<double> opt_lowpass;
	plib::option_num<double> opt_highpass;
	plib::option_bool opt_hp_boost;
	plib::option_group opt_grp2;
	plib::option_num<double> opt_high;
	plib::option_num<double> opt_low;
	plib::option_group opt_grp3;
	plib::option_num<double> opt_start;
	plib::option_num<double> opt_inc;
	plib::option_num<std::size_t> opt_samples;

	plib::option_group opt_grp4;
	plib::option_bool   opt_verb;
	plib::option_bool   opt_quiet;
	plib::option_args   opt_args;
	plib::option_bool   opt_version;
	plib::option_bool   opt_help;
	plib::option_example   opt_ex1;
	plib::option_example   opt_ex2;
	plib::option_example   opt_ex3;
	std::vector<std::unique_ptr<std::istream>> m_in_streams;
};

void nlwav_app::convert_wav(std::ostream &output, wav_t::format fmt)
{

	double dt = plib::reciprocal(static_cast<double>(opt_rate()));
	auto num_channels = m_in_streams.size();

	auto wo = plib::make_unique<wav_writer, arena>(output, opt_out() != "-", fmt, num_channels, opt_rate(), opt_amp());
	auto ago = plib::make_unique<aggregator, arena>(num_channels, dt, aggregator::callback_type(&wav_writer::process, wo.get()));
	auto fgo_hp = plib::make_unique<filter_hp, arena>(opt_highpass(), opt_hp_boost(), num_channels, filter_hp::callback_type(&aggregator::process, ago.get()));
	auto fgo_lp = plib::make_unique<filter_lp, arena>(opt_lowpass(), num_channels, filter_lp::callback_type(&filter_hp::process, fgo_hp.get()));

	auto top_cb = log_processor::callback_type(&filter_lp::process, fgo_lp.get());

	log_processor lp(num_channels, top_cb);

	lp.process(m_in_streams);

	if (!opt_quiet())
	{
#if 0
		std_err("Mean (low freq filter): {}\n", wo->mean);
		std_err("Mean (static):          {}\n", wo->means / static_cast<double>(wo->m_n));
		std_err("Amp + {}\n", 32000.0 / (wo->max_samples - wo->mean));
		std_err("Amp - {}\n", -32000.0 / (wo->min_samples - wo->mean));
#endif
	}
}

void nlwav_app::convert_vcd(std::ostream &output, vcd_writer::format_e format)
{

	arena::unique_ptr<vcd_writer> wo = plib::make_unique<vcd_writer, arena>(output, opt_args(),
		format, opt_high(), opt_low());
	log_processor::callback_type agcb = log_processor::callback_type(&vcd_writer::process, wo.get());

	log_processor lp(m_in_streams.size(), agcb);

	lp.process(m_in_streams);

	if (!opt_quiet())
	{
#if 0
		std_err("Mean (low freq filter): {}\n", wo->mean);
		std_err("Mean (static):          {}\n", wo->means / static_cast<double>(wo->m_n));
		std_err("Amp + {}\n", 32000.0 / (wo->max_samples - wo->mean));
		std_err("Amp - {}\n", -32000.0 / (wo->min_samples - wo->mean));
#endif
	}
}

void nlwav_app::convert_tab(std::ostream &output)
{

	auto wo = plib::make_unique<tab_writer, arena>(output, opt_args(),
		opt_start(), opt_inc(), opt_samples());
	log_processor::callback_type agcb = log_processor::callback_type(&tab_writer::process, wo.get());

	log_processor lp(m_in_streams.size(), agcb);

	lp.process(m_in_streams);

}


pstring nlwav_app::usage()
{
	return help("Convert netlist log files into wav files.\n",
			"nlwav [OPTION] ... [FILE] ...");
}

template <typename F>
static void open_ostream_and_exec(const pstring &fname, bool binary, F func)
{
	if (fname != "-")
	{
		// FIXME: binary depends on format!
		plib::ofstream output_stream(plib::filesystem::u8path(fname),
			binary ? (std::ios::out | std::ios::binary) : std::ios::out);
		if (output_stream.fail())
			throw plib::file_open_e(fname);
		output_stream.imbue(std::locale::classic());
		func(output_stream);
	}
	else
	{
		std::cout.imbue(std::locale::classic());
		// FIXME: switch to binary on windows
#ifdef _WIN32
		_setmode(_fileno(stdout), _O_BINARY);
#endif
		func(std::cout);
	}
}

void nlwav_app::convert(const pstring &output_file)
{
	switch (opt_fmt())
	{
		case 0:
			open_ostream_and_exec(output_file, true, [this](std::ostream &output) { convert_wav(output, wav_t::s16); });
			break;
		case 1:
			open_ostream_and_exec(output_file, true, [this](std::ostream &output) { convert_wav(output, wav_t::s32); });
			break;
		case 2:
			open_ostream_and_exec(output_file, true, [this](std::ostream &output) { convert_wav(output, wav_t::f32); });
			break;
		case 3:
			open_ostream_and_exec(output_file, false, [this](std::ostream &output) { convert_vcd(output, vcd_writer::ANALOG); });
			break;
		case 4:
			open_ostream_and_exec(output_file, false, [this](std::ostream &output) { convert_vcd(output, vcd_writer::DIGITAL); });
			break;
		case 5:
			open_ostream_and_exec(output_file, false, [this](std::ostream &output) { convert_tab(output); });
			break;
		default:
			// tease compiler - can't happen
			break;
	}
}

int nlwav_app::execute()
{
	if (opt_help())
	{
		std_out(usage());
		return 0;
	}

	if (opt_version())
	{
		std_out(
			"nlwav (netlist) 0.1\n"
			"Copyright (C) 2021 Couriersud\n"
			"License BSD-3-Clause\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n\n"
			"Written by Couriersud.\n");
		return 0;
	}

	try
	{
		for (const auto &oi: opt_args())
		{
			std::unique_ptr<std::istream> fin;
			if (oi == "-")
			{
				auto temp(std::make_unique<std::stringstream>());
				plib::copy_stream(*temp, std::cin);
				fin = std::move(temp);
			}
			else
			{
				fin = std::make_unique<plib::ifstream>(plib::filesystem::u8path(oi), std::ios::in);
				if (fin->fail())
					throw plib::file_open_e(oi);
			}
			fin->imbue(std::locale::classic());
			m_in_streams.push_back(std::move(fin));
		}

		convert(opt_out());
	}
	catch (plib::pexception &e)
	{
		std_err("Exception caught: {}\n", e.text());
		return 1;
	}
	return 0;
}

PMAIN(nlwav_app)

// spell-checker:disable
//
// Der Daten-Abschnitt enth??lt die Abtastwerte:
// Offset  L??nge  Inhalt  Beschreibung
// 36 (0x24)   4   'data'  Header-Signatur
// 40 (0x28)   4   <length>    L??nge des Datenblocks, max. <Dateigr????e>?????????44
//
// 0 (0x00)    char    4   'RIFF'
// 4 (0x04)    unsigned    4   <Dateigr????e>?????????8
// 8 (0x08)    char    4   'WAVE'
//
// Der fmt-Abschnitt (24 Byte) beschreibt das Format der einzelnen Abtastwerte:
// Offset  L??nge  Inhalt  Beschreibung
// 12 (0x0C)   4   'fmt '  Header-Signatur (folgendes Leerzeichen beachten)
// 16 (0x10)   4   <fmt length>    L??nge des restlichen fmt-Headers (16 Bytes)
// 20 (0x14)   2   <format tag>    Datenformat der Abtastwerte (siehe separate Tabelle weiter unten)
// 22 (0x16)   2   <channels>  Anzahl der Kan??le: 1 = mono, 2 = stereo; mittlerweile sind auch mehr als 2 Kan??le (z. B. f??r Raumklang) m??glich.[2]
// 24 (0x18)   4   <sample rate>   Samples pro Sekunde je Kanal (z. B. 44100)
// 28 (0x1C)   4   <bytes/second>  Abtastrate????????Frame-Gr????e
// 32 (0x20)   2   <block align>   Frame-Gr????e = <Anzahl der Kan??le>????????((<Bits/Sample (eines Kanals)>???+???7)???/???8)   (Division ohne Rest)
// 34 (0x22)   2   <bits/sample>   Anzahl der Datenbits pro Samplewert je Kanal (z. B. 12)
//
// spell-checker:enable
