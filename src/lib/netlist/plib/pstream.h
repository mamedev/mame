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
#include "palloc.h"
#include "pfmtlog.h"

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
	virtual ~pstream()
	{
	}

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
	virtual ~pistream() {}

	bool eof() const { return ((flags() & FLAG_EOF) != 0); }

	/* this digests linux & dos/windows text files */

	bool readline(pstring &line);

	bool readbyte(char &b)
	{
		return (read(&b, 1) == 1);
	}

	pos_type read(void *buf, const unsigned n)
	{
		return vread(buf, n);
	}

protected:
	/* read up to n bytes from stream */
	virtual pos_type vread(void *buf, const pos_type n) = 0;

private:
	pstringbuffer m_linebuf;
};

// -----------------------------------------------------------------------------
// postream: output stream
// -----------------------------------------------------------------------------

class postream : public pstream
{
	P_PREVENT_COPYING(postream)
public:

	explicit postream(unsigned flags) : pstream(flags) {}
	virtual ~postream() {}

	/* this digests linux & dos/windows text files */

	void writeline(const pstring &line)
	{
		write(line);
		write(10);
	}

	void write(const pstring &text)
	{
		write(text.cstr(), text.blen());
	}

	void write(const char c)
	{
		write(&c, 1);
	}

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
	virtual ~postringstream() { }

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
};

// -----------------------------------------------------------------------------
// pstdout: write to stdout
// -----------------------------------------------------------------------------

class pstdout : public pofilestream
{
	P_PREVENT_COPYING(pstdout)
public:
	pstdout();
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

	pistringstream(const pstring &str) : pimemstream(str.cstr(), str.len()), m_str(str) { }

private:
	/* only needed for a reference till destruction */
	pstring m_str;
};

// -----------------------------------------------------------------------------
// pstream_fmt_writer_t: writer on top of ostream
// -----------------------------------------------------------------------------

class pstream_fmt_writer_t : public plib::pfmt_writer_t<>
{
	P_PREVENT_COPYING(pstream_fmt_writer_t)
public:

	explicit pstream_fmt_writer_t(postream &strm) : m_strm(strm) {}
	virtual ~pstream_fmt_writer_t() { }

protected:
	virtual void vdowrite(const pstring &ls) const override
	{
		m_strm.write(ls);
	}

private:
	postream &m_strm;
};

}

#endif /* PSTREAM_H_ */
