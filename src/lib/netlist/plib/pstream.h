// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstream.h
 */

#ifndef PSTREAM_H_
#define PSTREAM_H_

#include <cstdarg>
#include <cstddef>

#include "pconfig.h"
#include "pstring.h"
#include "pfmtlog.h"
#include "pexception.h"

namespace plib {
// -----------------------------------------------------------------------------
// pstream: things common to all streams
// -----------------------------------------------------------------------------

class pstream
{
	P_PREVENT_COPYING(pstream)
public:

	using pos_type = std::size_t;

	static constexpr pos_type SEEK_EOF = static_cast<pos_type>(-1);

	explicit pstream(const unsigned flags) : m_flags(flags)
	{
	}
	virtual ~pstream();

	bool seekable() const { return ((m_flags & FLAG_SEEKABLE) != 0); }

	void seek(const pos_type n)
	{
		return vseek(n);
	}

	pos_type tell()
	{
		return vtell();
	}

protected:
	virtual void vseek(const pos_type n) = 0;
	virtual pos_type vtell() = 0;

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
	P_PREVENT_COPYING(pistream)
public:

	explicit pistream(const unsigned flags) : pstream(flags) {}
	virtual ~pistream();

	bool eof() const { return ((flags() & FLAG_EOF) != 0); }

	pos_type read(void *buf, const unsigned n)
	{
		return vread(buf, n);
	}

protected:
	/* read up to n bytes from stream */
	virtual pos_type vread(void *buf, const pos_type n) = 0;

};

// -----------------------------------------------------------------------------
// postream: output stream
// -----------------------------------------------------------------------------

class postream : public pstream
{
	P_PREVENT_COPYING(postream)
public:

	explicit postream(unsigned flags) : pstream(flags) {}
	virtual ~postream();

	void write(const void *buf, const pos_type n)
	{
		vwrite(buf, n);
	}

	void write(pistream &strm);

protected:
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, const pos_type n) = 0;

private:
};

// -----------------------------------------------------------------------------
// pomemstream: output string stream
// -----------------------------------------------------------------------------

class pomemstream : public postream
{
	P_PREVENT_COPYING(pomemstream)
public:

	pomemstream();
	virtual ~pomemstream();

	char *memory() const { return m_mem; }
	pos_type size() const { return m_size; }

protected:
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, const pos_type) override;
	virtual void vseek(const pos_type n) override;
	virtual pos_type vtell() override;

private:
	pos_type m_pos;
	pos_type m_capacity;
	pos_type m_size;
	char *m_mem;
};

class postringstream : public postream
{
	P_PREVENT_COPYING(postringstream )

public:

	postringstream() : postream(0) { }
	virtual ~postringstream();

	const pstringbuffer &str() { return m_buf; }

protected:
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, const pos_type n) override
	{
		m_buf.cat(buf, n);
	}
	virtual void vseek(const pos_type n) override { }
	virtual pos_type vtell() override { return m_buf.len(); }

private:
	pstringbuffer m_buf;
};

// -----------------------------------------------------------------------------
// pofilestream: file output stream
// -----------------------------------------------------------------------------

class pofilestream : public postream
{
	P_PREVENT_COPYING(pofilestream)
public:

	explicit pofilestream(const pstring &fname);
	virtual ~pofilestream();

protected:
	pofilestream(void *file, const pstring name, const bool do_close);
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, const pos_type n) override;
	virtual void vseek(const pos_type n) override;
	virtual pos_type vtell() override;

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

class pstderr : public pofilestream
{
	P_PREVENT_COPYING(pstderr)
public:
	pstderr();
	virtual ~pstderr();
};

// -----------------------------------------------------------------------------
// pstdout: write to stdout
// -----------------------------------------------------------------------------

class pstdout : public pofilestream
{
	P_PREVENT_COPYING(pstdout)
public:
	pstdout();
	virtual ~pstdout();
};

// -----------------------------------------------------------------------------
// pifilestream: file input stream
// -----------------------------------------------------------------------------

class pifilestream : public pistream
{
	P_PREVENT_COPYING(pifilestream)
public:

	explicit pifilestream(const pstring &fname);
	virtual ~pifilestream();

protected:
	pifilestream(void *file, const pstring name, const bool do_close);

	/* read up to n bytes from stream */
	virtual pos_type vread(void *buf, const pos_type n) override;
	virtual void vseek(const pos_type n) override;
	virtual pos_type vtell() override;

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
	P_PREVENT_COPYING(pstdin)
public:

	pstdin();
	virtual ~pstdin();
};

// -----------------------------------------------------------------------------
// pimemstream: input memory stream
// -----------------------------------------------------------------------------

class pimemstream : public pistream
{
	P_PREVENT_COPYING(pimemstream)
public:

	pimemstream(const void *mem, const pos_type len);
	explicit pimemstream(const pomemstream &ostrm);
	virtual ~pimemstream();

	pos_type size() const { return m_len; }
protected:
	/* read up to n bytes from stream */
	virtual pos_type vread(void *buf, const pos_type n) override;
	virtual void vseek(const pos_type n) override;
	virtual pos_type vtell() override;

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
	P_PREVENT_COPYING(pistringstream)
public:
	pistringstream(const pstring &str) : pimemstream(str.c_str(), str.len()), m_str(str) { }
	virtual ~pistringstream();

private:
	/* only needed for a reference till destruction */
	pstring m_str;
};

// -----------------------------------------------------------------------------
// putf8reader_t: reader on top of istream
// -----------------------------------------------------------------------------

/* this digests linux & dos/windows text files */

class putf8_reader
{
	P_PREVENT_COPYING(putf8_reader)
public:
	explicit putf8_reader(pistream &strm) : m_strm(strm) {}
	virtual ~putf8_reader() {}

	bool eof() const { return m_strm.eof(); }
	bool readline(pstring &line);

	bool readbyte1(char &b)
	{
		return (m_strm.read(&b, 1) == 1);
	}

	bool readcode(pstring::code_t &c)
	{
		char b[4];
		if (m_strm.read(&b[0], 1) != 1)
			return false;
		const unsigned l = pstring::traits::codelen(b);
		for (unsigned i = 1; i < l; i++)
			if (m_strm.read(&b[i], 1) != 1)
				return false;
		c = pstring::traits::code(b);
		return true;
	}

private:
	pistream &m_strm;
	pstringbuffer m_linebuf;
};

// -----------------------------------------------------------------------------
// putf8writer_t: writer on top of ostream
// -----------------------------------------------------------------------------

class putf8_writer
{
	P_PREVENT_COPYING(putf8_writer)
public:
	explicit putf8_writer(postream &strm) : m_strm(strm) {}
	virtual ~putf8_writer() {}

	void writeline(const pstring &line) const
	{
		write(line);
		write(10);
	}

	void write(const pstring &text) const
	{
		m_strm.write(text.c_str(), text.blen());
	}

	void write(const pstring::code_t c) const
	{
		write(pstring(c));
	}

private:
	postream &m_strm;
};

class putf8_fmt_writer : public pfmt_writer_t<>, public putf8_writer
{
	P_PREVENT_COPYING(putf8_fmt_writer)
public:

	explicit putf8_fmt_writer(postream &strm);
	virtual ~putf8_fmt_writer();

protected:
	virtual void vdowrite(const pstring &ls) const override;

private:
};

}

#endif /* PSTREAM_H_ */
