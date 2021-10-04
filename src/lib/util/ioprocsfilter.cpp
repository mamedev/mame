// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocs.h

    I/O filters

***************************************************************************/

#include "ioprocsfilter.h"

#include "ioprocs.h"
#include "ioprocsfill.h"

#include <zlib.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <limits>
#include <system_error>
#include <type_traits>
#include <utility>


namespace util {

namespace {

// helper class for holding zlib data

class zlib_data
{
protected:
	zlib_data(std::size_t buffer_size) noexcept :
		m_buffer_size(std::min<std::common_type_t<uInt, std::size_t> >(std::numeric_limits<uInt>::max(), buffer_size))
	{
		assert(buffer_size);
	}

	z_stream &get_z_stream() noexcept { return m_z_stream; }
	z_stream const &get_z_stream() const noexcept { return m_z_stream; }
	Bytef *get_buffer() noexcept { return m_buffer.get(); }
	Bytef const *get_buffer() const noexcept { return m_buffer.get(); }
	std::size_t get_buffer_size() const noexcept { return m_buffer_size; }

	bool input_empty() const noexcept { return !m_z_stream.avail_in; }
	bool output_available() const noexcept { return bool(m_z_stream.avail_out); }

	void initialize_z_stream() noexcept
	{
		m_z_stream.zalloc = Z_NULL;
		m_z_stream.zfree = Z_NULL;
		m_z_stream.opaque = Z_NULL;
	}

	std::error_condition allocate_buffer() noexcept
	{
		if (!m_buffer)
			m_buffer.reset(new (std::nothrow) Bytef [m_buffer_size]);
		return m_buffer ? std::error_condition() : std::errc::not_enough_memory;
	}

	void release_buffer() noexcept
	{
		m_buffer.reset();
	}

	static std::error_condition convert_z_error(int err) noexcept
	{
		switch (err)
		{
		case Z_OK:
		case Z_STREAM_END:
			return std::error_condition();
		case Z_NEED_DICT:
			return std::errc::invalid_argument;
		case Z_ERRNO:
			return std::error_condition(errno, std::generic_category());
		case Z_STREAM_ERROR:
			return std::errc::invalid_argument;
		case Z_DATA_ERROR:
			return std::errc::invalid_argument; // TODO: revisit this error code
		case Z_MEM_ERROR:
			return std::errc::not_enough_memory;
		case Z_BUF_ERROR:
			return std::errc::operation_would_block; // TODO: revisit this error code (should be handled internally)
		case Z_VERSION_ERROR:
			return std::errc::invalid_argument; // TODO: revisit this error code (library ABI mismatch)
		default:
			return std::errc::io_error; // TODO: better default error code
		}
	}

private:
	z_stream m_z_stream;
	std::unique_ptr<Bytef []> m_buffer;
	std::size_t const m_buffer_size;
};


// helper class for decompressing deflated data

class inflate_data : private zlib_data
{
protected:
	inflate_data(std::size_t buffer_size) noexcept : zlib_data(buffer_size)
	{
	}

	~inflate_data()
	{
		if (m_inflate_initialized)
			inflateEnd(&get_z_stream());
	}

	using zlib_data::input_empty;

	bool stream_initialized() const noexcept
	{
		return m_inflate_initialized;
	}

	std::error_condition initialize_z_stream() noexcept
	{
		if (m_inflate_initialized)
			return std::errc::invalid_argument;
		std::error_condition err = allocate_buffer();
		if (err)
			return err;
		zlib_data::initialize_z_stream();
		int const zerr = inflateInit(&get_z_stream());
		if (Z_OK == zerr)
			m_inflate_initialized = true;
		return convert_z_error(zerr);
	}

	std::error_condition close_z_stream() noexcept
	{
		if (!m_inflate_initialized)
			return std::errc::invalid_argument;
		int const zerr = inflateEnd(&get_z_stream());
		m_inflate_initialized = false;
		return convert_z_error(zerr);
	}

	std::error_condition decompress_some(bool &stream_end) noexcept
	{
		assert(m_inflate_initialized);
		int const zerr = inflate(&get_z_stream(), Z_SYNC_FLUSH);
		if (!get_z_stream().avail_in)
			reset_input();
		stream_end = Z_STREAM_END == zerr;
		return convert_z_error(zerr);
	}

	std::size_t input_available() const noexcept
	{
		return std::size_t(get_z_stream().avail_in);
	}

	void reset_input() noexcept
	{
		get_z_stream().next_in = get_buffer();
		get_z_stream().avail_in = 0U;
	}

	std::pair<void *, std::size_t> get_unfilled_input() noexcept
	{
		assert(get_buffer());
		Bytef *const base = get_z_stream().next_in + get_z_stream().avail_in;
		return std::make_pair(base, get_buffer() + get_buffer_size() - base);
	}

	void add_input(std::size_t length) noexcept
	{
		assert(get_buffer());
		get_z_stream().avail_in += length;
		assert((get_buffer() + get_buffer_size()) >= (get_z_stream().next_in + get_z_stream().avail_in));
	}

	std::size_t output_produced() const noexcept
	{
		return m_output_max - get_z_stream().avail_out;
	}

	void set_output(void *buffer, std::size_t length) noexcept
	{
		m_output_max = std::min<std::common_type_t<uInt, std::size_t> >(std::numeric_limits<uInt>::max(), length);
		get_z_stream().next_out = reinterpret_cast<Bytef *>(buffer);
		get_z_stream().avail_out = uInt(m_output_max);
	}

private:
	std::error_condition allocate_buffer() noexcept
	{
		std::error_condition err;
		if (!get_buffer())
		{
			err = zlib_data::allocate_buffer();
			reset_input();
		}
		return err;
	}

	bool m_inflate_initialized = false;
	std::size_t m_output_max = 0U;
};


// helper class for deflating data

class deflate_data : private zlib_data
{
protected:
	deflate_data(int level, std::size_t buffer_size) noexcept : zlib_data(buffer_size), m_level(level)
	{
	}

	~deflate_data()
	{
		if (m_deflate_initialized)
			deflateEnd(&get_z_stream());
	}

	using zlib_data::input_empty;
	using zlib_data::output_available;

	bool stream_initialized() const noexcept
	{
		return m_deflate_initialized;
	}

	bool compression_finished() const noexcept
	{
		return m_compression_finished;
	}

	std::error_condition initialize_z_stream() noexcept
	{
		if (m_deflate_initialized)
			return std::errc::invalid_argument;
		std::error_condition err = allocate_buffer();
		if (err)
			return err;
		zlib_data::initialize_z_stream();
		int const zerr = deflateInit(&get_z_stream(), m_level);
		reset_output();
		if (Z_OK == zerr)
			m_deflate_initialized = true;
		return convert_z_error(zerr);
	}

	std::error_condition close_z_stream() noexcept
	{
		if (!m_deflate_initialized)
			return std::errc::invalid_argument;
		m_deflate_initialized = m_compression_finished = false;
		return convert_z_error(deflateEnd(&get_z_stream()));
	}

	std::error_condition reset_z_stream() noexcept
	{
		if (!m_deflate_initialized)
			return std::errc::invalid_argument;
		assert(get_buffer());
		reset_output();
		m_compression_finished = false;
		return convert_z_error(deflateReset(&get_z_stream()));
	}

	std::error_condition compress_some() noexcept
	{
		assert(m_deflate_initialized);
		return convert_z_error(deflate(&get_z_stream(), Z_NO_FLUSH));
	}

	std::error_condition finish_compression() noexcept
	{
		assert(m_deflate_initialized);
		int const zerr = deflate(&get_z_stream(), Z_FINISH);
		if (Z_STREAM_END == zerr)
			m_compression_finished = true;
		return convert_z_error(zerr);
	}

	std::size_t input_used() const noexcept
	{
		return m_input_max - get_z_stream().avail_in;
	}

	void set_input(void const *data, std::size_t length) noexcept
	{
		m_input_max = std::min<std::common_type_t<uInt, std::size_t> >(std::numeric_limits<uInt>::max(), length);
		get_z_stream().next_in = const_cast<Bytef z_const *>(reinterpret_cast<Bytef const *>(data));
		get_z_stream().avail_in = uInt(m_input_max);
	}

	std::pair<void const *, std::size_t> get_output() const noexcept
	{
		assert(get_buffer());
		return std::make_pair(
				&get_buffer()[m_output_used],
				get_buffer_size() - get_z_stream().avail_out - m_output_used);
	}

	void consume_output(std::size_t length) noexcept
	{
		m_output_used += length;
		assert(m_output_used <= (get_buffer_size() - get_z_stream().avail_out));
		if (m_output_used == (get_buffer_size() - get_z_stream().avail_out))
			reset_output();
	}

private:
	void reset_output() noexcept
	{
		assert(get_buffer());
		m_output_used = 0U;
		get_z_stream().next_out = get_buffer();
		get_z_stream().avail_out = uInt(get_buffer_size());
	}

	int const m_level;
	bool m_deflate_initialized = false;
	bool m_compression_finished = false;
	std::size_t m_output_used = 0U;
	std::size_t m_input_max = 0U;
};


// helper for holding an object and deleting it (or not) as necessary

template <typename T>
class filter_base
{
public:
	filter_base(std::unique_ptr<T> &&object) noexcept : m_object(object.release()), m_owned(true)
	{
		assert(m_object);
	}

	filter_base(T &object) noexcept : m_object(&object), m_owned(false)
	{
	}

protected:
	~filter_base()
	{
		if (m_owned)
			delete m_object;
	}

	T &object() noexcept
	{
		return *m_object;
	}

private:
	T *const m_object;
	bool const m_owned;
};


// helper for forwarding to a read stream

template <typename T>
class read_stream_proxy : public virtual read_stream, public T
{
public:
	using T::T;

	virtual std::error_condition read(void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		return this->object().read(buffer, length, actual);
	}
};


// helper for forwarding to a write stream

template <typename T>
class write_stream_proxy : public virtual write_stream, public T
{
public:
	using T::T;

	virtual std::error_condition finalize() noexcept override
	{
		return this->object().finalize();
	}

	virtual std::error_condition flush() noexcept override
	{
		return this->object().flush();
	}

	virtual std::error_condition write(void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		return this->object().write(buffer, length, actual);
	}
};


// helper for forwarding to random-access storage

template <typename T>
class random_access_proxy : public virtual random_access, public T
{
public:
	using T::T;

	virtual std::error_condition seek(std::int64_t offset, int whence) noexcept override
	{
		return this->object().seek(offset, whence);
	}

	virtual std::error_condition tell(std::uint64_t &result) noexcept override
	{
		return this->object().tell(result);
	}

	virtual std::error_condition length(std::uint64_t &result) noexcept override
	{
		return this->object().length(result);
	}
};


// helper for forwarding to random-access read storage

template <typename T>
class random_read_proxy : public virtual random_read, public read_stream_proxy<T>
{
public:
	using read_stream_proxy<T>::read_stream_proxy;

	virtual std::error_condition read_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		return this->object().read_at(offset, buffer, length, actual);
	}
};


// helper for forwarding to random-access write storage

template <typename T>
class random_write_proxy : public virtual random_write, public write_stream_proxy<T>
{
public:
	using write_stream_proxy<T>::write_stream_proxy;

	virtual std::error_condition write_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		return this->object().write_at(offset, buffer, length, actual);
	}
};


// helper for forwarding to random-access read/write storage

template <typename T>
class random_read_write_proxy : public random_read_write, protected random_write_proxy<random_read_proxy<T> >
{
public:
	using random_write_proxy<random_read_proxy<T> >::random_write_proxy;
};


// filter for decompressing deflated data

template <typename Stream>
class zlib_read_filter : public read_stream, protected filter_base<Stream>, protected inflate_data
{
public:
	zlib_read_filter(std::unique_ptr<Stream> &&stream, std::size_t read_chunk) noexcept :
		filter_base<Stream>(std::move(stream)),
		inflate_data(read_chunk)
	{
	}

	zlib_read_filter(Stream &stream, std::size_t read_chunk) noexcept :
		filter_base<Stream>(stream),
		inflate_data(read_chunk)
	{
	}

	virtual std::error_condition read(void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::error_condition err;
		actual = 0U;

		if (!stream_initialized())
			err = initialize_z_stream();

		if (!err && (length > actual))
		{
			bool short_input = false;
			do
			{
				if (input_empty())
				{
					auto const space = get_unfilled_input();
					std::size_t filled;
					err = this->object().read(space.first, space.second, filled);
					add_input(filled);
					short_input = space.second > filled;
				}

				if (!err && !input_empty())
				{
					set_output(reinterpret_cast<std::uint8_t *>(buffer) + actual, length - actual);
					bool stream_end;
					err = decompress_some(stream_end);
					actual += output_produced();

					if (stream_end)
					{
						assert(!err);
						if constexpr (std::is_base_of_v<random_read, Stream>)
						{
							if (!input_empty())
							{
								std::int64_t const overshoot = std::uint64_t(input_available());
								reset_input();
								return this->object().seek(-overshoot, SEEK_CUR);
							}
						}
						else
						{
							return std::error_condition();
						}
					}
				}
			}
			while (!err && (length > actual) && !short_input);
		}

		return err;
	}
};


// filter for deflating data

class zlib_write_filter : public write_stream, protected filter_base<write_stream>, protected deflate_data
{
public:
	zlib_write_filter(write_stream::ptr &&stream, int level, std::size_t buffer_size) noexcept :
		filter_base<write_stream>(std::move(stream)),
		deflate_data(level, buffer_size)
	{
	}

	zlib_write_filter(write_stream &stream, int level, std::size_t buffer_size) noexcept :
		filter_base<write_stream>(stream),
		deflate_data(level, buffer_size)
	{
	}

	~zlib_write_filter()
	{
		finalize();
	}

	virtual std::error_condition finalize() noexcept override
	{
		if (!stream_initialized())
			return std::error_condition();

		do
		{
			if (!compression_finished() && output_available())
			{
				std::error_condition err = finish_compression();
				if (err)
					return err;
			}

			while ((compression_finished() && get_output().second) || !output_available())
			{
				std::error_condition err = write_some();
				if (err)
					return err;
			}
		}
		while (!compression_finished());

		return close_z_stream();
	}

	virtual std::error_condition flush() noexcept override
	{
		if (stream_initialized())
		{
			auto const output = get_output();
			if (output.second)
			{
				std::size_t written;
				std::error_condition err = object().write(output.first, output.second, written);
				consume_output(written);
				if (err)
				{
					object().flush();
					return err;
				}
			}
		}
		return object().flush();
	}

	virtual std::error_condition write(void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::error_condition err;
		actual = 0U;

		if (!stream_initialized())
		{
			err = initialize_z_stream();
		}
		else if (compression_finished())
		{
			while (!err && get_output().second)
				err = write_some();
			if (!err)
				err = reset_z_stream();
		}

		while (!err && (length > actual))
		{
			set_input(reinterpret_cast<std::uint8_t const *>(buffer) + actual, length - actual);
			do
			{
				if (output_available())
					err = compress_some();

				while (!err && !output_available())
					err = write_some();
			}
			while (!err && !input_empty());
			actual += input_used();
		}

		return err;
	}

private:
	std::error_condition write_some() noexcept
	{
		auto const output = get_output();
		std::size_t written;
		std::error_condition err = object().write(output.first, output.second, written);
		consume_output(written);
		return err;
	}
};

} // anonymous namespace


// creating filters that fill unread space

read_stream::ptr read_stream_fill(read_stream::ptr &&stream, std::uint8_t filler) noexcept
{
	std::unique_ptr<read_stream_fill_wrapper<read_stream_proxy<filter_base<read_stream> > > > result;
	if (stream)
		result.reset(new (std::nothrow) decltype(result)::element_type(std::move(stream)));
	if (result)
		result->set_filler(filler);
	return result;
}

random_read::ptr random_read_fill(random_read::ptr &&stream, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_read_fill_wrapper<random_read_proxy<random_access_proxy<filter_base<random_read> > > > > result;
	if (stream)
		result.reset(new (std::nothrow) decltype(result)::element_type(std::move(stream)));
	if (result)
		result->set_filler(filler);
	return result;
}

read_stream::ptr read_stream_fill(read_stream &stream, std::uint8_t filler) noexcept
{
	std::unique_ptr<read_stream_fill_wrapper<read_stream_proxy<filter_base<read_stream> > > > result;
	result.reset(new (std::nothrow) decltype(result)::element_type(stream));
	if (result)
		result->set_filler(filler);
	return result;
}

random_read::ptr random_read_fill(random_read &stream, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_read_fill_wrapper<random_read_proxy<random_access_proxy<filter_base<random_read> > > > > result;
	result.reset(new (std::nothrow) decltype(result)::element_type(stream));
	if (result)
		result->set_filler(filler);
	return result;
}


// creating filters that fill unwritten space

random_write::ptr random_write_fill(random_write::ptr &&stream, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_write_fill_wrapper<random_write_proxy<random_access_proxy<filter_base<random_write> > > > > result;
	if (stream)
		result.reset(new (std::nothrow) decltype(result)::element_type(std::move(stream)));
	if (result)
		result->set_filler(filler);
	return result;
}

random_write::ptr random_write_fill(random_write &stream, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_write_fill_wrapper<random_write_proxy<random_access_proxy<filter_base<random_write> > > > > result;
	result.reset(new (std::nothrow) decltype(result)::element_type(stream));
	if (result)
		result->set_filler(filler);
	return result;
}


// creating filters that fill unread/unwritten space

random_read_write::ptr random_read_write_fill(random_read_write::ptr &&stream, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_read_write_fill_wrapper<random_read_write_proxy<random_access_proxy<filter_base<random_read_write> > > > > result;
	if (stream)
		result.reset(new (std::nothrow) decltype(result)::element_type(std::move(stream)));
	if (result)
		result->set_filler(filler);
	return result;
}

random_read_write::ptr random_read_write_fill(random_read_write &stream, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_read_write_fill_wrapper<random_read_write_proxy<random_access_proxy<filter_base<random_read_write> > > > > result;
	result.reset(new (std::nothrow) decltype(result)::element_type(stream));
	if (result)
		result->set_filler(filler);
	return result;
}


// creating decompressing filters

read_stream::ptr zlib_read(read_stream::ptr &&stream, std::size_t read_chunk) noexcept
{
	read_stream::ptr result;
	if (stream)
		result.reset(new (std::nothrow) zlib_read_filter<read_stream>(std::move(stream), read_chunk));
	return result;
}

read_stream::ptr zlib_read(random_read::ptr &&stream, std::size_t read_chunk) noexcept
{
	read_stream::ptr result;
	if (stream)
		result.reset(new (std::nothrow) zlib_read_filter<random_read>(std::move(stream), read_chunk));
	return result;
}

read_stream::ptr zlib_read(read_stream &stream, std::size_t read_chunk) noexcept
{
	return read_stream::ptr(new (std::nothrow) zlib_read_filter<read_stream>(stream, read_chunk));
}

read_stream::ptr zlib_read(random_read &stream, std::size_t read_chunk) noexcept
{
	return read_stream::ptr(new (std::nothrow) zlib_read_filter<random_read>(stream, read_chunk));
}


// creating compressing filters

write_stream::ptr zlib_write(write_stream::ptr &&stream, int level, std::size_t buffer_size) noexcept
{
	write_stream::ptr result;
	if (stream)
		result.reset(new (std::nothrow) zlib_write_filter(std::move(stream), level, buffer_size));
	return result;
}

write_stream::ptr zlib_write(write_stream &stream, int level, std::size_t buffer_size) noexcept
{
	return write_stream::ptr(new (std::nothrow) zlib_write_filter(stream, level, buffer_size));
}

} // namespace util
