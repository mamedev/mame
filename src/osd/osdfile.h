// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/// \file
/// \brief Core OS-dependent file interface
///
/// The prototypes in this file describe the interfaces that the MAME
/// core and various tools rely on to interact with the outside world.
/// They are broken out into several categories.

#ifndef MAME_OSD_OSDFILE_H
#define MAME_OSD_OSDFILE_H

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>


/***************************************************************************
    FILE I/O INTERFACES
***************************************************************************/

/* Make sure we have a path separator (default to /) */
#ifndef PATH_SEPARATOR
#if defined(_WIN32)
#define PATH_SEPARATOR          "\\"
#else
#define PATH_SEPARATOR          "/"
#endif
#endif

/// \defgroup openflags File open flags
/// \{

/// Open file for reading.
constexpr uint32_t OPEN_FLAG_READ           = 0x0001;

/// Open file for writing.
constexpr uint32_t OPEN_FLAG_WRITE          = 0x0002;

/// Create the file, or truncate it if it exists.
constexpr uint32_t OPEN_FLAG_CREATE         = 0x0004;

/// Create non-existent directories in the path.
constexpr uint32_t OPEN_FLAG_CREATE_PATHS   = 0x0008;

/// Do not decompress into memory on open.
constexpr uint32_t OPEN_FLAG_NO_PRELOAD     = 0x0010;

/// \}


/// \brief Interface to file-like resources
///
/// This interface is used to access file-like and stream-like
/// resources.  Examples include plain files, TCP socket, named pipes,
/// pseudo-terminals, and compressed archive members.
class osd_file
{
public:
	/// \brief Result of a file operation
	///
	/// Returned by most members of osd_file, and also used by other
	/// classes that access files or other file-like resources.
	enum class error
	{
		/// Operation completed successfully.
		NONE,

		/// Operation failed, but there is no more specific code to
		/// describe the failure.
		FAILURE,

		/// Operation failed due to an error allocating memory.
		OUT_OF_MEMORY,

		/// The requested file, path or resource was not found.
		NOT_FOUND,

		/// Current permissions do not allow the requested access.
		ACCESS_DENIED,

		/// Requested access is not permitted because the file or
		/// resource is currently open for exclusive access.
		ALREADY_OPEN,

		/// Request cannot be completed due to resource exhaustion
		/// (maximum number of open files or other objects has been
		/// reached).
		TOO_MANY_FILES,

		/// The request cannot be completed because invalid data was
		/// encountered (for example an inconsistent filesystem, or a
		/// corrupt archive file).
		INVALID_DATA,

		/// The requested access mode is invalid, or not appropriate for
		/// the file or resource.
		INVALID_ACCESS
	};

	/// \brief Smart pointer to a file handle
	typedef std::unique_ptr<osd_file> ptr;

	/// \brief Open a new file handle
	///
	/// This function is called by core_fopen and several other places
	/// in the core to access files. These functions will construct
	/// paths by concatenating various search paths held in the
	/// options.c options database with partial paths specified by the
	/// core.  The core assumes that the path separator is the first
	/// character of the string PATH_SEPARATOR, but does not interpret
	/// any path separators in the search paths, so if you use a
	/// different path separator in a search path, you may get a mixture
	/// of PATH_SEPARATORs (from the core) and alternate path separators
	/// (specified by users and placed into the options database).
	/// \param [in] path Path to the file to open.
	/// \param [in] openflags Combination of #OPEN_FLAG_READ,
	///   #OPEN_FLAG_WRITE, #OPEN_FLAG_CREATE and
	///   #OPEN_FLAG_CREATE_PATHS specifying the requested access mode
	///   and open behaviour.
	/// \param [out] file Receives the file handle if the operation
	///   succeeds.  Not valid if the operation fails.
	/// \param [out] filesize Receives the size of the opened file if
	///   the operation succeeded.  Not valid if the operation failed.
	///   Will be zero for stream-like objects (e.g. TCP sockets or
	///   named pipes).
	/// \return Result of the operation.
	static error open(std::string const &path, std::uint32_t openflags, ptr &file, std::uint64_t &filesize);

	/// \brief Create a new pseudo-terminal (PTY) pair
	///
	/// \param [out] file Receives the handle of the master side of the
	///   pseudo-terminal if the operation succeeds.  Not valid if the
	///   operation fails.
	/// \param [out] name Receives the name of the slave side of the
	///   pseudo-terminal if the operation succeeds.  Not valid if the
	///   operation fails.
	/// \return Result of the operation.
	static error openpty(ptr &file, std::string &name);

	/// \brief Close an open file
	virtual ~osd_file() { }

	/// \brief Read from an open file
	///
	/// Read data from an open file at specified offset.  Note that the
	/// seek and read are not guaranteed to be atomic, which may cause
	/// issues in multi-threaded applications.
	/// \param [out] buffer Pointer to memory that will receive the data
	///   read.
	/// \param [in] offset Byte offset within the file to read at,
	///   relative to the start of the file.  Ignored for stream-like
	///   objects (e.g. TCP sockets or named pipes).
	/// \param [in] length Number of bytes to read.  Fewer bytes may be
	///   read if the end of file is reached, or if no data is
	///   available.
	/// \param [out] actual Receives the number of bytes read if the
	///   operation succeeds.  Not valid if the operation fails.
	/// \return Result of the operation.
	virtual error read(void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) = 0;

	/// \brief Write to an open file
	///
	/// Write data to an open file at specified offset.  Note that the
	/// seek and write are not guaranteed to be atomic, which may cause
	/// issues in multi-threaded applications.
	/// \param [in] buffer Pointer to memory containing data to write.
	/// \param [in] offset Byte offset within the file to write at,
	///   relative to the start of the file.  Ignored for stream-like
	///   objects (e.g. TCP sockets or named pipes).
	/// \param [in] length Number of bytes to write.
	/// \param [out] actual Receives the number of bytes written if the
	///   operation succeeds.  Not valid if the operation fails.
	/// \return Result of the operation.
	virtual error write(void const *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) = 0;

	/// \brief Change the size of an open file
	///
	/// \param [in] offset Desired size of the file.
	/// \return Result of the operation.
	virtual error truncate(std::uint64_t offset) = 0;

	/// \brief Flush file buffers
	///
	/// This flushes any data cached by the application, but does not
	/// guarantee that all prior writes have reached persistent storage.
	/// \return Result of the operation.
	virtual error flush() = 0;

	/// \brief Delete a file
	///
	/// \param [in] filename Path to the file to delete.
	/// \return Result of the operation.
	static error remove(std::string const &filename);
};


/// \brief Describe geometry of physical drive
///
/// If the given path points to a physical drive, return the geometry of
/// that drive.
///
/// \param [in] filename Pointer to a path which might describe a
///   physical drive.
/// \param [out] cylinders Pointer to a uint32_t to receive the number of
///   cylinders of the physical drive.
/// \param [out] heads Pointer to a uint32_t to receive the number of
///   heads per cylinder of the physical drive.
/// \param [out] sectors Pointer to a uint32_t to receive the number of
///   sectors per cylinder of the physical drive.
/// \param [out] bps Pointer to a uint32_t to receive the number of
///   bytes per sector of the physical drive.
/// \return true if the filename points to a physical drive and if the
///   values pointed to by cylinders, heads, sectors, and bps are valid;
///   false in any other case
bool osd_get_physical_drive_geometry(const char *filename, uint32_t *cylinders, uint32_t *heads, uint32_t *sectors, uint32_t *bps);


/// \brief Is the given character legal for filenames?
///
/// \param [in] uchar The character to check.
/// \return Whether this character is legal in a filename.
bool osd_is_valid_filename_char(char32_t uchar);


/// \brief Is the given character legal for paths?
///
/// \param [in] uchar The character to check.
/// \return Whether this character is legal in a file path.
bool osd_is_valid_filepath_char(char32_t uchar);


/***************************************************************************
    DIRECTORY INTERFACES
***************************************************************************/

namespace osd
{
	// directory is an opaque type which represents an open directory
	class directory
	{
	public:
		typedef std::unique_ptr<directory> ptr;

		// osd::directory::entry contains basic information about a file when iterating through
		// a directory
		class entry
		{
		public:
			enum class entry_type
			{
				NONE,
				FILE,
				DIR,
				OTHER
			};

			const char *                            name;           // name of the entry
			entry_type                              type;           // type of the entry
			std::uint64_t                           size;           // size of the entry
			std::chrono::system_clock::time_point   last_modified;  // last modified time
		};

		/// \brief Open a directory for iteration.
		///
		/// \param [in] dirname Path to the directory in question.
		/// \return Upon success, a directory pointer which contains opaque
		///   data necessary to traverse the directory; on failure, nullptr.
		static ptr open(std::string const &dirname);

		/// \brief Close an open directory.
		virtual ~directory() { }

		/// \brief Return information about the next entry in the directory.
		///
		/// \return A constant pointer to an entry representing the current
		///   item in the directory, or nullptr, indicating that no more
		///   entries are present.
		virtual const entry *read() = 0;
	};
};


/// \brief Return a directory entry for a path.
///
/// \param [in] path The path in question.
/// \return An allocated pointer to an osd::directory::entry representing
///   info on the path; even if the file does not exist.
std::unique_ptr<osd::directory::entry> osd_stat(std::string const &path);


/***************************************************************************
    PATH INTERFACES
***************************************************************************/

/// \brief Returns whether the specified path is absolute.
///
/// \param [in] path The path in question.
/// \return true if the path is absolute, false otherwise.
bool osd_is_absolute_path(const std::string &path);


/// \brief Retrieves the full path.
/// \param [in] path The path in question.
/// \param [out] dst Reference to receive new path.
/// \return File error.
osd_file::error osd_get_full_path(std::string &dst, std::string const &path);


/// \brief Retrieves the volume name.
///
/// \param [in] idx Order number of volume.
/// \return Pointer to volume name.
const char *osd_get_volume_name(int idx);


#endif // MAME_OSD_OSDFILE_H
