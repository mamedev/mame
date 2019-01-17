// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstream.h
 */

#ifndef PSTREAM_H_
#define PSTREAM_H_

#include "pconfig.h"
#include "pstring.h"
#include "pfmtlog.h"
#include "pexception.h"
#include "palloc.h"

#define USE_CSTREAM (0)

#include <vector>
#include <type_traits>

#if USE_CSTREAM
#include <fstream>
//#include <strstream>
#include <sstream>
#endif


namespace plib {

#if USE_CSTREAM
typedef std::ostream postream;
typedef std::ofstream pofilestream;
typedef std::ostringstream postringstream;
typedef std::ostringstream pomemstream;

#endif

// -----------------------------------------------------------------------------
// pstream: things common to all streams
// -----------------------------------------------------------------------------

class pstream : nocopyassign
{
public:

	using pos_type = std::size_t;
	using size_type = std::size_t;

	static constexpr pos_type SEEK_EOF = static_cast<pos_type>(-1);

	bool seekable() const { return ((m_flags & FLAG_SEEKABLE) != 0); }

	void seekp(const pos_type n)
	{
		vseek(n);
	}

	pos_type tellp() const
	{
		return vtell();
	}

protected:
	explicit pstream(const unsigned flags) : m_flags(flags)
	{
	}
	pstream(pstream &&src) : m_flags(src.m_flags)
	{
	}
	~pstream();

	virtual void vseek(const pos_type n) = 0;
	virtual pos_type vtell() const = 0;

	static constexpr unsigned FLAG_EOF = 0x01;
	static constexpr unsigned FLAG_SEEKABLE = 0x04;

	void set_flag(const unsigned flag)
	{
		m_flags |= flag;
	}
	void clear_flag(const unsigned flag)
	{
		m_flags &= ~flag;
	}
	unsigned flags() const { return m_flags; }
private:

	unsigned m_flags;
};

// -----------------------------------------------------------------------------
// pistream: input stream
// -----------------------------------------------------------------------------

class pistream : public pstream
{
public:

	virtual ~pistream();

	bool eof() const { return ((flags() & FLAG_EOF) != 0); }

	pos_type read(void *buf, const pos_type n)
	{
		return vread(buf, n);
	}

protected:
	explicit pistream(const unsigned flags) : pstream(flags) {}
	explicit pistream(pistream &&src) : pstream(std::move(src)) {}
	/* read up to n bytes from stream */
	virtual size_type vread(void *buf, const size_type n) = 0;

};

// -----------------------------------------------------------------------------
// postream: output stream
// -----------------------------------------------------------------------------

#if !USE_CSTREAM
class postream : public pstream
{
public:

	virtual ~postream();

	void write(const char *buf, const size_type n)
	{
		vwrite(buf, n);
	}

protected:
	explicit postream(unsigned flags) : pstream(flags) {}
	explicit postream(postream &&src) : pstream(std::move(src)) {}
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, const size_type n) = 0;

private:
};

// -----------------------------------------------------------------------------
// pomemstream: output string stream
// -----------------------------------------------------------------------------

class pomemstream : public postream
{
public:

	pomemstream();

	pomemstream(pomemstream &&src)
	: postream(std::move(src))
	, m_pos(src.m_pos)
	, m_capacity(src.m_capacity)
	, m_size(src.m_size)
	, m_mem(src.m_mem)
	{
		src.m_mem = nullptr;
	}

	virtual ~pomemstream() override;

	char *memory() const { return m_mem; }
	pos_type size() const { return m_size; }

protected:
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, const pos_type) override;
	virtual void vseek(const pos_type n) override;
	virtual pos_type vtell() const override;

private:
	pos_type m_pos;
	pos_type m_capacity;
	pos_type m_size;
	char *m_mem;
};

class postringstream : public postream
{
public:

	postringstream() : postream(0) { }
	postringstream(postringstream &&src)
	: postream(std::move(src))
	, m_buf(src.m_buf)
	{ src.m_buf = ""; }

	virtual ~postringstream() override;

	const pstring &str() { return m_buf; }

protected:
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, const pos_type n) override
	{
		m_buf += pstring(static_cast<const char *>(buf), n);
	}
	virtual void vseek(const pos_type n) override { }
	virtual pos_type vtell() const override { return m_buf.size(); }

private:
	pstring m_buf;
};

// -----------------------------------------------------------------------------
// pofilestream: file output stream
// -----------------------------------------------------------------------------

class pofilestream : public postream
{
public:

	pofilestream(const pstring &fname);
	pofilestream(pofilestream &&src)
	: postream(std::move(src))
	, m_file(src.m_file)
	, m_pos(src.m_pos)
	, m_actually_close(src.m_actually_close)
	, m_filename(src.m_filename)
	{
		src.m_file = nullptr;
		src.m_actually_close = false;
	}

	virtual ~pofilestream() override;

protected:
	pofilestream(void *file, const pstring &name, const bool do_close);
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, const pos_type n) override;
	virtual void vseek(const pos_type n) override;
	virtual pos_type vtell() const override;

private:
	void *m_file;
	pos_type m_pos;
	bool m_actually_close;
	pstring m_filename;

	void init();
};

// -----------------------------------------------------------------------------
// pstderr: write to stderr
// -----------------------------------------------------------------------------
#endif

class pstderr : public pofilestream
{
public:
	pstderr();
	virtual ~pstderr();
};

// -----------------------------------------------------------------------------
// pstdout: write to stdout
// -----------------------------------------------------------------------------

class pstdout : public pofilestream
{
public:
	pstdout();
	virtual ~pstdout();
};

// -----------------------------------------------------------------------------
// pifilestream: file input stream
// -----------------------------------------------------------------------------

class pifilestream : public pistream
{
public:

	pifilestream(const pstring &fname);
	virtual ~pifilestream() override;

	pifilestream(pifilestream &&src)
	: pistream(std::move(src))
	, m_file(src.m_file)
	, m_pos(src.m_pos)
	, m_actually_close(src.m_actually_close)
	, m_filename(src.m_filename)
	{
		src.m_actually_close = false;
		src.m_file = nullptr;
	}

protected:
	pifilestream(void *file, const pstring &name, const bool do_close);

	/* read up to n bytes from stream */
	virtual pos_type vread(void *buf, const pos_type n) override;
	virtual void vseek(const pos_type n) override;
	virtual pos_type vtell() const override;

private:
	void *m_file;
	pos_type m_pos;
	bool m_actually_close;
	pstring m_filename;

	void init();
};

// -----------------------------------------------------------------------------
// pstdin: reads from stdin
// -----------------------------------------------------------------------------

class pstdin : public pifilestream
{
public:

	pstdin();
	virtual ~pstdin() override;
};

// -----------------------------------------------------------------------------
// pimemstream: input memory stream
// -----------------------------------------------------------------------------

class pimemstream : public pistream
{
public:

	pimemstream(const void *mem, const pos_type len);
	pimemstream();

	pimemstream(pimemstream &&src)
	: pistream(std::move(src))
	, m_pos(src.m_pos)
	, m_len(src.m_len)
	, m_mem(src.m_mem)
	{
		src.m_mem = nullptr;
	}

	explicit pimemstream(const pomemstream &ostrm);

	virtual ~pimemstream() override;

	pos_type size() const { return m_len; }
protected:

	void set_mem(const void *mem, const pos_type len)
	{
		m_mem = static_cast<const char *>(mem);
		m_len = len;
	}

	/* read up to n bytes from stream */
	virtual pos_type vread(void *buf, const pos_type n) override;
	virtual void vseek(const pos_type n) override;
	virtual pos_type vtell() const override;

private:
	pos_type m_pos;
	pos_type m_len;
	const char *m_mem;
};

// -----------------------------------------------------------------------------
// pistringstream: input string stream
// -----------------------------------------------------------------------------

class pistringstream : public pimemstream
{
public:
	pistringstream(const pstring &str)
	: pimemstream()
	, m_str(str)
	{
		set_mem(m_str.c_str(), std::strlen(m_str.c_str()));
	}
	pistringstream(pistringstream &&src)
	: pimemstream(std::move(src)), m_str(src.m_str)
	{
		set_mem(m_str.c_str(), std::strlen(m_str.c_str()));
	}
	virtual ~pistringstream() override;

private:
	/* only needed for a reference till destruction */
	const pstring m_str;
};

// -----------------------------------------------------------------------------
// putf8reader_t: reader on top of istream
// -----------------------------------------------------------------------------

/* this digests linux & dos/windows text files */

class putf8_reader : plib::nocopyassign
{
public:
	putf8_reader(pistream *strm) : m_strm(strm), m_stream_owned(false) {}
	virtual ~putf8_reader()
	{
		if (m_stream_owned && m_strm != nullptr)
			pfree(m_strm);
	}

	putf8_reader(putf8_reader &&src) : m_strm(src.m_strm), m_stream_owned(src.m_stream_owned)
	{
		src.m_strm = nullptr;
		src.m_stream_owned = false;
	}

	putf8_reader(pimemstream &&strm)
	: m_strm(palloc<pimemstream>(std::move(strm))),
	  m_stream_owned(true)
	{}

	putf8_reader(pifilestream &&strm)
	: m_strm(palloc<pifilestream>(std::move(strm))),
	  m_stream_owned(true)
	{}

	putf8_reader(pistringstream &&strm)
	: m_strm(palloc<pistringstream>(std::move(strm))),
	  m_stream_owned(true)
	{}


	bool eof() const { return m_strm->eof(); }
	bool readline(pstring &line);

	bool readbyte1(char &b)
	{
		return (m_strm->read(&b, 1) == 1);
	}

	bool readcode(putf8string::traits_type::code_t &c)
	{
		char b[4];
		if (m_strm->read(&b[0], 1) != 1)
			return false;
		const std::size_t l = putf8string::traits_type::codelen(b);
		for (std::size_t i = 1; i < l; i++)
			if (m_strm->read(&b[i], 1) != 1)
				return false;
		c = putf8string::traits_type::code(b);
		return true;
	}

private:
	pistream *m_strm;
	bool m_stream_owned;
	putf8string m_linebuf;
};

// -----------------------------------------------------------------------------
// putf8writer_t: writer on top of ostream
// -----------------------------------------------------------------------------

class putf8_writer : plib::nocopyassign
{
public:
	explicit putf8_writer(postream *strm) : m_strm(strm) {}

	putf8_writer(putf8_writer &&src) : m_strm(src.m_strm) {}
	virtual ~putf8_writer() {}

	void writeline(const pstring &line) const
	{
		write(line);
		write(10);
	}

	void write(const pstring &text) const
	{
		putf8string conv_utf8(text);
		m_strm->write(conv_utf8.c_str(), conv_utf8.mem_t_size());
	}

	void write(const pstring::value_type c) const
	{
		pstring t = pstring("") + c;
		write(t);
	}

private:
	postream *m_strm;
};

class putf8_fmt_writer : public pfmt_writer_t<putf8_fmt_writer>, public putf8_writer
{
public:

	explicit putf8_fmt_writer(postream *strm)
	: pfmt_writer_t()
	, putf8_writer(strm)
	{
	}

	virtual ~putf8_fmt_writer() override;

//protected:
	void vdowrite(const pstring &ls) const;

private:
};

// -----------------------------------------------------------------------------
// pbinary_writer_t: writer on top of ostream
// -----------------------------------------------------------------------------

class pbinary_writer : plib::nocopyassign
{
public:
	explicit pbinary_writer(postream &strm) : m_strm(strm) {}
	pbinary_writer(pbinary_writer &&src) : m_strm(src.m_strm) {}
	virtual ~pbinary_writer() {}

	template <typename T>
	void write(const T &val)
	{
		m_strm.write(reinterpret_cast<const char *>(&val), sizeof(T));
	}

	void write(const pstring &s)
	{
		const char *sm = s.c_str();
		const std::size_t sl = std::strlen(sm);
		write(sl);
		m_strm.write(sm, sl);
	}

	template <typename T>
	void write(const std::vector<T> &val)
	{
		std::size_t sz = val.size();
		write(sz);
		m_strm.write(val.data(), sizeof(T) * sz);
	}

private:
	postream &m_strm;
};

class pbinary_reader : plib::nocopyassign
{
public:
	explicit pbinary_reader(pistream &strm) : m_strm(strm) {}
	pbinary_reader(pbinary_reader &&src) : m_strm(src.m_strm) { }
	virtual ~pbinary_reader() {}

	template <typename T>
	void read(T &val)
	{
		m_strm.read(&val, sizeof(T));
	}

	void read( pstring &s)
	{
		std::size_t sz = 0;
		read(sz);
		plib::string_info<pstring>::mem_t *buf = new plib::string_info<pstring>::mem_t[sz+1];
		m_strm.read(buf, sz);
		buf[sz] = 0;
		s = pstring(buf);
		delete [] buf;
	}

	template <typename T>
	void read(std::vector<T> &val)
	{
		std::size_t sz = 0;
		read(sz);
		val.resize(sz);
		m_strm.read(val.data(), sizeof(T) * sz);
	}

private:
	pistream &m_strm;
};

inline void copystream(postream &dest, pistream &src)
{
	char buf[1024];
	pstream::pos_type r;
	while ((r=src.read(buf, 1024)) > 0)
		dest.write(buf, r);
}


}

#endif /* PSTREAM_H_ */
