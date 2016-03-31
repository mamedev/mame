// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    vecstream.h

    streams with vector storage

    These types are useful if you want a persistent buffer for formatted
    text and you need to use it like a character array or character
    pointer, as you get read-only access to it without copying.  The
    storage is always guaranteed to be contiguous.  Writing to the
    stream may invalidate pointers to storage.

***************************************************************************/

#ifndef __MAME_UTIL_VECSTREAM_H__
#define __MAME_UTIL_VECSTREAM_H__

#include <algorithm>
#include <cassert>
#include <ios>
#include <istream>
#include <ostream>
#include <memory>
#include <ostream>
#include <streambuf>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace util {
template <typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT> >
class basic_vectorbuf : public std::basic_streambuf<CharT, Traits>
{
public:
	typedef typename std::basic_streambuf<CharT, Traits>::char_type char_type;
	typedef typename std::basic_streambuf<CharT, Traits>::int_type  int_type;
	typedef typename std::basic_streambuf<CharT, Traits>::pos_type  pos_type;
	typedef typename std::basic_streambuf<CharT, Traits>::off_type  off_type;
	typedef Allocator                                               allocator_type;
	typedef std::vector<char_type, Allocator>                       vector_type;

	basic_vectorbuf(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) : std::basic_streambuf<CharT, Traits>(), m_mode(mode), m_storage(), m_threshold(nullptr)
	{
		setup();
	}

	basic_vectorbuf(vector_type const &content, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) : std::basic_streambuf<CharT, Traits>(), m_mode(mode), m_storage(content), m_threshold(nullptr)
	{
		setup();
	}

	basic_vectorbuf(vector_type &&content, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) : std::basic_streambuf<CharT, Traits>(), m_mode(mode), m_storage(std::move(content)), m_threshold(nullptr)
	{
		setup();
	}

	basic_vectorbuf(basic_vectorbuf const &that) : std::basic_streambuf<CharT, Traits>(that), m_mode(that.m_mode), m_storage(that.m_storage), m_threshold(nullptr)
	{
		adjust();
	}

	basic_vectorbuf(basic_vectorbuf &&that) : std::basic_streambuf<CharT, Traits>(that), m_mode(that.m_mode), m_storage(std::move(that.m_storage)), m_threshold(that.m_threshold)
	{
		that.clear();
	}

	vector_type const &vec() const
	{
		if (m_mode & std::ios_base::out)
		{
			if (this->pptr() > m_threshold) m_threshold = this->pptr();
			auto const base(this->pbase());
			auto const end(m_threshold - base);
			if (m_storage.size() > std::make_unsigned_t<decltype(end)>(end))
			{
				m_storage.resize(std::make_unsigned_t<decltype(end)>(end));
				assert(&m_storage[0] == base);
				auto const put_offset(this->pptr() - base);
				const_cast<basic_vectorbuf *>(this)->setp(base, base + put_offset);
				const_cast<basic_vectorbuf *>(this)->pbump(put_offset);
			}
		}
		return m_storage;
	}

	void vec(const vector_type &content)
	{
		m_storage = content;
		setup();
	}

	void vec(vector_type &&content)
	{
		m_storage = std::move(content);
		setup();
	}

	void clear()
	{
		m_storage.clear();
		setup();
	}

	void swap(basic_vectorbuf &that)
	{
		using std::swap;
		std::basic_streambuf<CharT, Traits>::swap(that);
		swap(m_mode, that.m_mode);
		swap(m_storage, that.m_storage);
		swap(m_threshold, that.m_threshold);
	}

	void reserve(typename vector_type::size_type size)
	{
		if ((m_mode & std::ios_base::out) && (m_storage.capacity() < size))
		{
			m_storage.reserve(size);
			adjust();
		}
	}

	basic_vectorbuf &operator=(basic_vectorbuf const &that)
	{
		std::basic_streambuf<CharT, Traits>::operator=(that);
		m_mode = that.m_mode;
		m_storage = that.m_storage;
		m_threshold = that.m_threshold;
		adjust();
		return *this;
	}

	basic_vectorbuf &operator=(basic_vectorbuf &&that)
	{
		std::basic_streambuf<CharT, Traits>::operator=(that);
		m_mode = that.m_mode;
		m_storage = std::move(that.m_storage);
		m_threshold = that.m_threshold;
		that.clear();
		return *this;
	}

protected:
	pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override
	{
		bool const in(which & std::ios_base::in);
		bool const out(which & std::ios_base::out);
		if ((!in && !out) ||
			(in && out && (std::ios_base::cur == dir)) ||
			(in && !(m_mode & std::ios_base::in)) ||
			(out && !(m_mode & std::ios_base::out)))
		{
			return pos_type(off_type(-1));
		}
		maximise_egptr();
		off_type const end((m_mode & std::ios_base::out) ? off_type(m_threshold - this->pbase()) : off_type(m_storage.size()));
		switch (dir)
		{
		case std::ios_base::beg:
			break;
		case std::ios_base::end:
			off += end;
			break;
		case std::ios_base::cur:
			off += off_type(in ? (this->gptr() - this->eback()) : (this->pptr() - this->pbase()));
			break;
		default:
			return pos_type(off_type(-1));
		}
		if ((off_type(0) > off) || ((m_mode & std::ios_base::app) && out && (end != off))) return pos_type(off_type(-1));
		if ((out ? off_type(this->epptr() - this->pbase()) : end) < off) return pos_type(off_type(-1));
		if (out)
		{
			this->setp(this->pbase(), this->epptr());
			this->pbump(off);
			if (m_threshold < this->pptr()) m_threshold = this->pptr();
			if (m_mode & std::ios_base::in)
			{
				if (in) this->setg(this->eback(), this->eback() + off, m_threshold);
				else if (this->egptr() < m_threshold) this->setg(this->eback(), this->gptr(), m_threshold);
			}
		}
		else if (in)
		{
			this->setg(this->eback(), this->eback() + off, this->egptr());
		}
		return pos_type(off);
	}

	virtual pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in |std:: ios_base::out) override
	{
		return seekoff(off_type(pos), std::ios_base::beg, which);
	}

	virtual int_type underflow() override
	{
		if (!this->gptr()) return Traits::eof();
		maximise_egptr();
		return (this->gptr() < this->egptr()) ? Traits::to_int_type(*this->gptr()) : Traits::eof();
	}

	virtual int_type overflow(int_type ch = Traits::eof()) override
	{
		if (!(m_mode & std::ios_base::out)) return Traits::eof();
		if (Traits::eq_int_type(ch, Traits::eof())) return Traits::not_eof(ch);
		auto const put_offset(this->pptr() - this->pbase() + 1);
		auto const threshold_offset((std::max)(m_threshold - this->pbase(), put_offset));
		m_storage.push_back(Traits::to_char_type(ch));
		m_storage.resize(m_storage.capacity());
		auto const base(&m_storage[0]);
		this->setp(base, base + m_storage.size());
		m_threshold = base + threshold_offset;
		if (m_mode & std::ios_base::in) this->setg(base, base + (this->gptr() - this->eback()), m_threshold);
		this->pbump(int(put_offset));
		return ch;
	}

	virtual int_type pbackfail(int_type ch = Traits::eof()) override
	{
		if (this->gptr() != this->eback())
		{
			if (Traits::eq_int_type(ch, Traits::eof()))
			{
				this->gbump(-1);
				return Traits::not_eof(ch);
			}
			else if (Traits::eq(Traits::to_char_type(ch), this->gptr()[-1]))
			{
				this->gbump(-1);
				return ch;
			}
			else if (m_mode & std::ios_base::out)
			{
				this->gbump(-1);
				*this->gptr() = Traits::to_char_type(ch);
				return ch;
			}
		}
		return Traits::eof();
	}

private:
	void setup()
	{
		if (m_mode & std::ios_base::out)
		{
			auto const end(m_storage.size());
			m_storage.resize(m_storage.capacity());
			if (m_storage.empty())
			{
				m_threshold = nullptr;
				this->setg(nullptr, nullptr, nullptr);
				this->setp(nullptr, nullptr);
			}
			else
			{
				auto const base(&m_storage[0]);
				m_threshold = base + end;
				this->setp(base, base + m_storage.size());
				if (m_mode & std::ios_base::in) this->setg(base, base, m_threshold);
			}
			if (m_mode & (std::ios_base::app | std::ios_base::ate)) this->pbump(int(unsigned(end)));
		}
		else if (m_storage.empty())
		{
			this->setg(nullptr, nullptr, nullptr);
		}
		else if (m_mode & std::ios_base::in)
		{
			auto const base(&m_storage[0]);
			this->setg(base, base, base + m_storage.size());
		}
	}

	void adjust()
	{
		auto const put_offset(this->pptr() - this->pbase());
		auto const get_offset(this->gptr() - this->eback());
		setup();
		if (m_mode & std::ios_base::out)
		{
			this->pbump(int(put_offset));
			m_threshold = this->pptr();
			if (m_mode & std::ios_base::in)
			{
				auto const base(&m_storage[0]);
				this->setg(base, base + get_offset, m_threshold);
			}
		}
		else if (m_mode & std::ios_base::in)
		{
			this->gbump(int(get_offset));
		}
	}

	void maximise_egptr()
	{
		if (m_mode & std::ios_base::out)
		{
			if (m_threshold < this->pptr()) m_threshold = this->pptr();
			if ((m_mode & std::ios_base::in) && (this->egptr() < m_threshold)) this->setg(this->eback(), this->gptr(), m_threshold);
		}
	}

	std::ios_base::openmode m_mode;
	mutable vector_type     m_storage;
	mutable CharT           *m_threshold;
};

template <typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT> >
class basic_ivectorstream : public std::basic_istream<CharT, Traits>
{
public:
	typedef typename basic_vectorbuf<CharT, Traits, Allocator>::vector_type vector_type;

	basic_ivectorstream(std::ios_base::openmode mode = std::ios_base::in) : std::basic_istream<CharT, Traits>(&m_rdbuf), m_rdbuf(mode) { }
	basic_ivectorstream(vector_type const &content, std::ios_base::openmode mode = std::ios_base::in) : std::basic_istream<CharT, Traits>(&m_rdbuf), m_rdbuf(content, mode) { }
	basic_ivectorstream(vector_type &&content, std::ios_base::openmode mode = std::ios_base::in) : std::basic_istream<CharT, Traits>(&m_rdbuf), m_rdbuf(std::move(content), mode) { }

	basic_vectorbuf<CharT, Traits, Allocator> *rdbuf() const { return static_cast<basic_vectorbuf<CharT, Traits, Allocator> *>(std::basic_istream<CharT, Traits>::rdbuf()); }
	vector_type const &vec() const { return rdbuf()->vec(); }
	void vec(const vector_type &content) { rdbuf()->vec(content); }
	void vec(vector_type &&content) { rdbuf()->vec(std::move(content)); }

	void swap(basic_ivectorstream &that) { std::basic_istream<CharT, Traits>::swap(that); rdbuf()->swap(*that.rdbuf()); }

private:
	basic_vectorbuf<CharT, Traits, Allocator> m_rdbuf;
};

template <typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT> >
class basic_ovectorstream : public std::basic_ostream<CharT, Traits>
{
public:
	typedef typename basic_vectorbuf<CharT, Traits, Allocator>::vector_type vector_type;

	basic_ovectorstream(std::ios_base::openmode mode = std::ios_base::out) : std::basic_ostream<CharT, Traits>(&m_rdbuf), m_rdbuf(mode) { }
	basic_ovectorstream(vector_type const &content, std::ios_base::openmode mode = std::ios_base::out) : std::basic_ostream<CharT, Traits>(&m_rdbuf), m_rdbuf(content, mode) { }
	basic_ovectorstream(vector_type &&content, std::ios_base::openmode mode = std::ios_base::out) : std::basic_ostream<CharT, Traits>(&m_rdbuf), m_rdbuf(std::move(content), mode) { }

	basic_vectorbuf<CharT, Traits, Allocator> *rdbuf() const { return static_cast<basic_vectorbuf<CharT, Traits, Allocator> *>(std::basic_ostream<CharT, Traits>::rdbuf()); }

	vector_type const &vec() const { return rdbuf()->vec(); }
	void vec(const vector_type &content) { rdbuf()->vec(content); }
	void vec(vector_type &&content) { rdbuf()->vec(std::move(content)); }
	basic_ovectorstream &reserve(typename vector_type::size_type size) { rdbuf()->reserve(size); return *this; }

	void swap(basic_ovectorstream &that) { std::basic_ostream<CharT, Traits>::swap(that); rdbuf()->swap(*that.rdbuf()); }

private:
	basic_vectorbuf<CharT, Traits, Allocator> m_rdbuf;
};

template <typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT> >
class basic_vectorstream : public std::basic_iostream<CharT, Traits>
{
public:
	typedef typename basic_vectorbuf<CharT, Traits, Allocator>::vector_type vector_type;

	basic_vectorstream(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) : std::basic_iostream<CharT, Traits>(&m_rdbuf), m_rdbuf(mode) { }
	basic_vectorstream(vector_type const &content, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) : std::basic_iostream<CharT, Traits>(&m_rdbuf), m_rdbuf(content, mode) { }
	basic_vectorstream(vector_type &&content, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) : std::basic_iostream<CharT, Traits>(&m_rdbuf), m_rdbuf(std::move(content), mode) { }

	basic_vectorbuf<CharT, Traits, Allocator> *rdbuf() const { return static_cast<basic_vectorbuf<CharT, Traits, Allocator> *>(std::basic_iostream<CharT, Traits>::rdbuf()); }

	vector_type const &vec() const { return rdbuf()->vec(); }
	void vec(const vector_type &content) { rdbuf()->vec(content); }
	void vec(vector_type &&content) { rdbuf()->vec(std::move(content)); }
	basic_vectorstream &reserve(typename vector_type::size_type size) { rdbuf()->reserve(size); return *this; }

	void swap(basic_vectorstream &that) { std::basic_iostream<CharT, Traits>::swap(that); rdbuf()->swap(*that.rdbuf()); }

private:
	basic_vectorbuf<CharT, Traits, Allocator> m_rdbuf;
};

typedef basic_ivectorstream<char>       ivectorstream;
typedef basic_ivectorstream<wchar_t>    wivectorstream;
typedef basic_ovectorstream<char>       ovectorstream;
typedef basic_ovectorstream<wchar_t>    wovectorstream;
typedef basic_vectorstream<char>        vectorstream;
typedef basic_vectorstream<wchar_t>     wvectorstream;

template <typename CharT, typename Traits, typename Allocator>
void swap(basic_vectorbuf<CharT, Traits, Allocator> &a, basic_vectorbuf<CharT, Traits, Allocator> &b) { a.swap(b); }

template <typename CharT, typename Traits, typename Allocator>
void swap(basic_ivectorstream<CharT, Traits, Allocator> &a, basic_ivectorstream<CharT, Traits, Allocator> &b) { a.swap(b); }
template <typename CharT, typename Traits, typename Allocator>
void swap(basic_ovectorstream<CharT, Traits, Allocator> &a, basic_ovectorstream<CharT, Traits, Allocator> &b) { a.swap(b); }
template <typename CharT, typename Traits, typename Allocator>
void swap(basic_vectorstream<CharT, Traits, Allocator> &a, basic_vectorstream<CharT, Traits, Allocator> &b) { a.swap(b); }

} // namespace util

#endif // __MAME_UTIL_VECSTREAM_H__
