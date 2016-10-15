// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    stream.h

    Code for implementing Imgtool streams

***************************************************************************/

#ifndef STREAM_H
#define STREAM_H

#include "imgterrs.h"
#include "corefile.h"

namespace imgtool
{
	class stream
	{
	public:
		typedef std::unique_ptr<stream> ptr;

		~stream();

		static imgtool::stream *open(const char *fname, int read_or_write);  /* similar params to mame_fopen */
		static imgtool::stream *open_write_stream(int filesize);
		static imgtool::stream *open_mem(void *buf, size_t sz);
		
		util::core_file *core_file();
		UINT32 read(void *buf, UINT32 sz);
		UINT32 write(const void *buf, UINT32 sz);
		UINT64 size() const;
		int seek(INT64 pos, int where);
		UINT64 tell();
		void *getptr();
		UINT32 putc(char c);
		UINT32 puts(const char *s);
		UINT32 printf(const char *fmt, ...) ATTR_PRINTF(2, 3);

		// transfers sz bytes from source to dest
		static UINT64 transfer(imgtool::stream &dest, imgtool::stream &source, UINT64 sz);
		static UINT64 transfer_all(imgtool::stream &dest, imgtool::stream &source);

		// fills sz bytes with b
		UINT64 fill(unsigned char b, UINT64 sz);

		// returns the CRC of a file
		int crc(unsigned long *result);
		static int file_crc(const char *fname, unsigned long *result);

		// returns whether a stream is read only or not
		int is_read_only();

	private:
		enum imgtype_t
		{
			IMG_FILE,
			IMG_MEM
		};

		imgtype_t       imgtype;
		bool            write_protect;
		const char      *name; // needed for clear
		std::uint64_t   position;
		std::uint64_t   filesize;

		util::core_file::ptr file;
		std::uint8_t *buffer;

		// ctors
		stream(bool wp);
		stream(bool wp, util::core_file::ptr &&f);
		stream(bool wp, std::size_t size);
		stream(bool wp, std::size_t size, void *buf);

		// private methods
		static stream *open_zip(const char *zipname, const char *subname, int read_or_write);
	};
}



#endif /* STREAM_H */
