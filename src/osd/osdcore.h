// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/// \file
/// \brief Core OS-dependent code interface
///
/// The prototypes in this file describe the interfaces that the MAME
/// core and various tools rely on to interact with the outside world.
/// They are broken out into several categories.
#ifndef MAME_OSD_OSDCORE_H
#define MAME_OSD_OSDCORE_H

#pragma once


#include "osdcomm.h"

#include "strformat.h"

#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>


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
#define OPEN_FLAG_READ          0x0001

/// Open file for writing.
#define OPEN_FLAG_WRITE         0x0002

/// Create the file, or truncate it if it exists.
#define OPEN_FLAG_CREATE        0x0004

/// Create non-existent directories in the path.
#define OPEN_FLAG_CREATE_PATHS  0x0008

/// Do not decompress into memory on open.
#define OPEN_FLAG_NO_PRELOAD    0x0010

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


/// \brief Get environment variable value
///
/// \param [in] name Name of the environment variable as a
///   NUL-terminated string.
/// \return Pointer to environment variable value as a NUL-terminated
///   string if found, or nullptr if not found.
const char *osd_getenv(const char *name);


/// \brief Get current process ID
///
/// \return The process ID of the current process.
int osd_getpid();


/*-----------------------------------------------------------------------------
    osd_get_physical_drive_geometry: if the given path points to a physical
        drive, return the geometry of that drive

    Parameters:

        filename - pointer to a path which might describe a physical drive

        cylinders - pointer to a uint32_t to receive the number of cylinders
            of the physical drive

        heads - pointer to a uint32_t to receive the number of heads per
            cylinder of the physical drive

        sectors - pointer to a uint32_t to receive the number of sectors per
            cylinder of the physical drive

        bps - pointer to a uint32_t to receive the number of bytes per sector
            of the physical drive

    Return value:

        true if the filename points to a physical drive and if the values
        pointed to by cylinders, heads, sectors, and bps are valid; false in
        any other case
-----------------------------------------------------------------------------*/
bool osd_get_physical_drive_geometry(const char *filename, uint32_t *cylinders, uint32_t *heads, uint32_t *sectors, uint32_t *bps);


/*-----------------------------------------------------------------------------
    osd_uchar_from_osdchar: convert the given character or sequence of
        characters from the OS-default encoding to a Unicode character

    Parameters:

        uchar - pointer to a uint32_t to receive the resulting unicode
            character

        osdchar - pointer to one or more chars that are in the OS-default
            encoding

        count - number of characters provided in the OS-default encoding

    Return value:

        The number of characters required to form a Unicode character.
-----------------------------------------------------------------------------*/
int osd_uchar_from_osdchar(char32_t *uchar, const char *osdchar, size_t count);


/*-----------------------------------------------------------------------------
    osd_is_valid_filename_char: is the given character legal for filenames?

    Parameters:

        uchar - the character to check

    Return value:

        Whether this character is legal in a filename
-----------------------------------------------------------------------------*/

bool osd_is_valid_filename_char(char32_t uchar);


/*-----------------------------------------------------------------------------
    osd_is_valid_filepath_char: is the given character legal for paths?

    Parameters:

        uchar - the character to check

    Return value:

        Whether this character is legal in a file path
-----------------------------------------------------------------------------*/

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

		// -----------------------------------------------------------------------------
		// osd::directory::open: open a directory for iteration
		//
		// Parameters:
		//
		// dirname - path to the directory in question
		//
		// Return value:
		//
		// upon success, this function should return an directory pointer
		// which contains opaque data necessary to traverse the directory; on
		// failure, this function should return nullptr
		// -----------------------------------------------------------------------------
		static ptr open(std::string const &dirname);

		// -----------------------------------------------------------------------------
		// osd::directory::~directory: close an open directory
		// -----------------------------------------------------------------------------
		virtual ~directory() { }

		// -----------------------------------------------------------------------------
		// osd::directory::read: return information about the next entry in the directory
		//
		// Return value:
		//
		// a constant pointer to an entry representing the current item
		// in the directory, or nullptr, indicating that no more entries are
		// present
		// -----------------------------------------------------------------------------
		virtual const entry *read() = 0;
	};
};


/*-----------------------------------------------------------------------------
    osd_is_absolute_path: returns whether the specified path is absolute

    Parameters:

        path - the path in question

    Return value:

        non-zero if the path is absolute, zero otherwise
-----------------------------------------------------------------------------*/
bool osd_is_absolute_path(const std::string &path);



/***************************************************************************
    TIMING INTERFACES
***************************************************************************/

/* a osd_ticks_t is a 64-bit unsigned integer that is used as a core type in timing interfaces */
typedef uint64_t osd_ticks_t;


/*-----------------------------------------------------------------------------
    osd_ticks: return the current running tick counter

    Parameters:

        None

    Return value:

        an osd_ticks_t value which represents the current tick counter

    Notes:

        The resolution of this counter should be 1ms or better for accurate
        performance. It is also important that the source of this timer be
        accurate. It is ok if this call is not ultra-fast, since it is
        primarily used for once/frame synchronization.
-----------------------------------------------------------------------------*/
osd_ticks_t osd_ticks();


/*-----------------------------------------------------------------------------
    osd_ticks_per_second: return the number of ticks per second

    Parameters:

        None

    Return value:

        an osd_ticks_t value which represents the number of ticks per
        second
-----------------------------------------------------------------------------*/
osd_ticks_t osd_ticks_per_second();


/*-----------------------------------------------------------------------------
    osd_sleep: sleep for the specified time interval

    Parameters:

        duration - an osd_ticks_t value that specifies how long we should
            sleep

    Return value:

        None

    Notes:

        The OSD layer should try to sleep for as close to the specified
        duration as possible, or less. This is used as a mechanism to
        "give back" unneeded time to other programs running in the system.
        On a simple, non multitasking system, this routine can be a no-op.
        If there is significant volatility in the amount of time that the
        sleep occurs for, the OSD layer should strive to sleep for less time
        than specified rather than sleeping too long.
-----------------------------------------------------------------------------*/
void osd_sleep(osd_ticks_t duration);

/***************************************************************************
    WORK ITEM INTERFACES
***************************************************************************/

/* this is the maximum number of supported threads for a single work queue */
/* threadid values are expected to range from 0..WORK_MAX_THREADS-1 */
#define WORK_MAX_THREADS            16

/* these flags can be set when creating a queue to give hints to the code about
   how to configure the queue */
#define WORK_QUEUE_FLAG_IO          0x0001
#define WORK_QUEUE_FLAG_MULTI       0x0002
#define WORK_QUEUE_FLAG_HIGH_FREQ   0x0004

/* these flags can be set when queueing a work item to indicate how to handle
   its deconstruction */
#define WORK_ITEM_FLAG_AUTO_RELEASE 0x0001

/* osd_work_queue is an opaque type which represents a queue of work items */
struct osd_work_queue;

/* osd_work_item is an opaque type which represents a single work item */
struct osd_work_item;

/* osd_work_callback is a callback function that does work */
typedef void *(*osd_work_callback)(void *param, int threadid);


/*-----------------------------------------------------------------------------
    osd_work_queue_alloc: create a new work queue

    Parameters:

        flags - one or more of the WORK_QUEUE_FLAG_* values ORed together:

            WORK_QUEUE_FLAG_IO - indicates that the work queue will do some
                I/O; this may be a useful hint so that threads are created
                even on single-processor systems since I/O can often be
                overlapped with other work

            WORK_QUEUE_FLAG_MULTI - indicates that the work queue should
                take advantage of as many processors as it can; items queued
                here are assumed to be fully independent or shared

            WORK_QUEUE_FLAG_HIGH_FREQ - indicates that items are expected
                to be queued at high frequency and acted upon quickly; in
                general, this implies doing some spin-waiting internally
                before falling back to OS-specific synchronization

    Return value:

        A pointer to an allocated osd_work_queue object.

    Notes:

        A work queue abstracts the notion of how potentially threaded work
        can be performed. If no threading support is available, it is a
        simple matter to execute the work items as they are queued.
-----------------------------------------------------------------------------*/
osd_work_queue *osd_work_queue_alloc(int flags);


/*-----------------------------------------------------------------------------
    osd_work_queue_items: return the number of pending items in the queue

    Parameters:

        queue - pointer to an osd_work_queue that was previously created via
            osd_work_queue_alloc

    Return value:

        The number of incomplete items remaining in the queue.
-----------------------------------------------------------------------------*/
int osd_work_queue_items(osd_work_queue *queue);


/*-----------------------------------------------------------------------------
    osd_work_queue_wait: wait for the queue to be empty

    Parameters:

        queue - pointer to an osd_work_queue that was previously created via
            osd_work_queue_alloc

        timeout - a timeout value in osd_ticks_per_second()

    Return value:

        true if the queue is empty; false if the wait timed out before the
        queue was emptied.
-----------------------------------------------------------------------------*/
bool osd_work_queue_wait(osd_work_queue *queue, osd_ticks_t timeout);


/*-----------------------------------------------------------------------------
    osd_work_queue_free: free a work queue, waiting for all items to complete

    Parameters:

        queue - pointer to an osd_work_queue that was previously created via
            osd_work_queue_alloc

    Return value:

        None.
-----------------------------------------------------------------------------*/
void osd_work_queue_free(osd_work_queue *queue);


/*-----------------------------------------------------------------------------
    osd_work_item_queue_multiple: queue a set of work items

    Parameters:

        queue - pointer to an osd_work_queue that was previously created via
            osd_work_queue_alloc

        callback - pointer to a function that will do the work

        numitems - number of work items to queue

        param - a void * parameter that can be used to pass data to the
            function

        paramstep - the number of bytes to increment param by for each item
            queued; for example, if you have an array of work_unit objects,
            you can point param to the base of the array and set paramstep to
            sizeof(work_unit)

        flags - one or more of the WORK_ITEM_FLAG_* values ORed together:

            WORK_ITEM_FLAG_AUTO_RELEASE - indicates that the work item
                should be automatically freed when it is complete

    Return value:

        A pointer to the final allocated osd_work_item in the list.

    Notes:

        On single-threaded systems, this function may actually execute the
        work item immediately before returning.
-----------------------------------------------------------------------------*/
osd_work_item *osd_work_item_queue_multiple(osd_work_queue *queue, osd_work_callback callback, int32_t numitems, void *parambase, int32_t paramstep, uint32_t flags);


/* inline helper to queue a single work item using the same interface */
static inline osd_work_item *osd_work_item_queue(osd_work_queue *queue, osd_work_callback callback, void *param, uint32_t flags)
{
	return osd_work_item_queue_multiple(queue, callback, 1, param, 0, flags);
}


/*-----------------------------------------------------------------------------
    osd_work_item_wait: wait for a work item to complete

    Parameters:

        item - pointer to an osd_work_item that was previously returned from
            osd_work_item_queue

        timeout - a timeout value in osd_ticks_per_second()

    Return value:

        true if the item completed; false if the wait timed out before the
        item completed.
-----------------------------------------------------------------------------*/
bool osd_work_item_wait(osd_work_item *item, osd_ticks_t timeout);


/*-----------------------------------------------------------------------------
    osd_work_item_result: get the result of a work item

    Parameters:

        item - pointer to an osd_work_item that was previously returned from
            osd_work_item_queue

    Return value:

        A void * that represents the work item's result.
-----------------------------------------------------------------------------*/
void *osd_work_item_result(osd_work_item *item);


/*-----------------------------------------------------------------------------
    osd_work_item_release: release the memory allocated to a work item

    Parameters:

        item - pointer to an osd_work_item that was previously returned from
            osd_work_item_queue

    Return value:

        None.

    Notes:

        The osd_work_item exists until explicitly released, even if it has
        long since completed. It is the queuer's responsibility to release
        any work items it has queued.
-----------------------------------------------------------------------------*/
void osd_work_item_release(osd_work_item *item);



/***************************************************************************
    MISCELLANEOUS INTERFACES
***************************************************************************/

/// \brief Allocate memory that can contain executable code
///
/// Allocated memory must be both writable and executable.  Allocated
/// memory must be freed by calling #osd_free_executable passing the
/// same size.
/// \param [in] size Number of bytes to allocate.
/// \return Pointer to allocated memory, or nullptr if allocation
///   failed.
/// \sa osd_free_executable
void *osd_alloc_executable(size_t size);


/// \brief Free memory allocated by osd_alloc_executable
///
/// \param [in] ptr Pointer returned by #osd_alloc_executable.
/// \param [in] size Number of bytes originally requested.  Must match
///   the value passed to #osd_alloc_executable.
/// \sa osd_alloc_executable
void osd_free_executable(void *ptr, size_t size);


/// \brief Break into host debugger if attached
///
/// This function is called when a fatal error occurs.  If a debugger is
/// attached, it should break and display the specified message.
/// \param [in] message Message to output to the debugger as a
///   NUL-terminated string.
void osd_break_into_debugger(const char *message);


/// \brief Get clipboard text
///
/// Gets current clipboard content as UTF-8 text.  Returns an empty
/// string if the clipboard contents cannot be converted to plain text.
/// \return Clipboard contents or an empty string.
std::string osd_get_clipboard_text();


/***************************************************************************
    DIRECTORY INTERFACES
***************************************************************************/

/*-----------------------------------------------------------------------------
    osd_stat: return a directory entry for a path

    Parameters:

        path - path in question

    Return value:

        an allocated pointer to an osd::directory::entry representing
        info on the path; even if the file does not exist.

-----------------------------------------------------------------------------*/
std::unique_ptr<osd::directory::entry> osd_stat(std::string const &path);

/***************************************************************************
    PATH INTERFACES
***************************************************************************/

/*-----------------------------------------------------------------------------
    osd_get_full_path: retrieves the full path

    Parameters:

        path - the path in question
        dst - reference to receive new path

    Return value:

        file error

-----------------------------------------------------------------------------*/
osd_file::error osd_get_full_path(std::string &dst, std::string const &path);


/***************************************************************************
    MIDI I/O INTERFACES
***************************************************************************/

class osd_midi_device
{
public:
	virtual ~osd_midi_device() { }
	// free result with osd_close_midi_channel()
	virtual bool open_input(const char *devname) = 0;
	// free result with osd_close_midi_channel()
	virtual bool open_output(const char *devname) = 0;
	virtual void close() = 0;
	virtual bool poll() = 0;
	virtual int read(uint8_t *pOut) = 0;
	virtual void write(uint8_t data) = 0;
};

//FIXME: really needed here?
void osd_list_network_adapters();


/***************************************************************************
    UNCATEGORIZED INTERFACES
***************************************************************************/

/*-----------------------------------------------------------------------------
    osd_get_volume_name: retrieves the volume name

    Parameters:

        idx - order number of volume

    Return value:

        pointer to volume name

-----------------------------------------------------------------------------*/
const char *osd_get_volume_name(int idx);

/*-----------------------------------------------------------------------------
    osd_subst_env: substitute environment variables with values

    Parameters:

        dst - result pointer
        src - source string

-----------------------------------------------------------------------------*/
void osd_subst_env(std::string &dst, std::string const &src);

class osd_gpu
{
public:
	osd_gpu() { }
	virtual ~osd_gpu() { }

	typedef uint64_t handle_t;

	class vertex_decl
	{
	public:
		enum attr_type : uint32_t
		{
			FLOAT32,
			FLOAT16,
			UINT32,
			UINT16,
			UINT8,

			MAX_TYPES
		};

		static constexpr size_t TYPE_SIZES[MAX_TYPES] = { 4, 2, 4, 2, 1 };

		static constexpr uint32_t MAX_COLORS = 2;
		static constexpr uint32_t MAX_TEXCOORDS = 8;

		enum attr_usage : uint32_t
		{
			POSITION,
			COLOR,
			TEXCOORD = COLOR + MAX_COLORS,
			NORMAL = TEXCOORD + MAX_TEXCOORDS,
			BINORMAL,
			TANGENT,

			MAX_ATTRS
		};

		class attr_entry
		{
		public:
			attr_entry() : m_usage(POSITION), m_type(FLOAT32), m_count(3), m_size(12) { }
			attr_entry(attr_usage usage, attr_type type, size_t count) : m_usage(usage), m_type(type), m_count(count), m_size(TYPE_SIZES[type] * count) { }

			attr_usage usage() const { return m_usage; }
			attr_type type() const { return m_type; }
			size_t count() const { return m_count; }
			size_t size() const { return m_size; }

		private:
			attr_usage m_usage;
			attr_type m_type;
			size_t m_count;
			size_t m_size;
		};

		vertex_decl()
			: m_entry_count(0)
			, m_size(0)
		{
		}

		vertex_decl & add_attr(attr_usage usage, attr_type type, size_t count)
		{
			m_entries[m_entry_count] = attr_entry(usage, type, count);
			m_size += m_entries[m_entry_count].size();
			m_entry_count++;
			return *this;
		}

		size_t entry_count() const { return m_entry_count; }
		size_t size() const { return m_size; }
		const attr_entry &entry(const uint32_t index) const { return m_entries[index]; }

	protected:
		attr_entry m_entries[MAX_ATTRS];
		size_t m_entry_count;
		size_t m_size;
	};

	class vertex_buffer_interface
	{
	public:
		vertex_buffer_interface(vertex_decl &decl, uint32_t flags)
			: m_decl(decl)
			, m_flags(flags)
		{
		}
		virtual ~vertex_buffer_interface() {}

		const vertex_decl &decl() const { return m_decl; }
		uint32_t flags() const { return m_flags; }
		handle_t handle() { return m_handle; }

		virtual size_t count() const = 0;
		virtual size_t size() const = 0;
		virtual void upload() = 0;

	protected:
		const vertex_decl &m_decl;
		const uint32_t m_flags;
		handle_t m_handle;
	};

	class static_vertex_buffer_interface : public vertex_buffer_interface
	{
	public:
		enum vertex_buffer_flags : uint32_t
		{
			RETAIN_ON_CPU = 0x00000001
		};

		static_vertex_buffer_interface(vertex_decl &decl, size_t count, uint32_t flags)
			: vertex_buffer_interface(decl, flags)
			, m_count(count)
			, m_size(decl.size() * count)
		{
		}

		virtual ~static_vertex_buffer_interface()
		{
			if (m_data)
				delete [] m_data;
		}

		size_t count() const override { return m_count; }
		size_t size() const override { return m_size; }

		void set_data(void *data)
		{
			allocate_if_needed();
			memcpy(m_data, data, m_size);
		}

	protected:
		void allocate_if_needed()
		{
			if ((m_flags & RETAIN_ON_CPU) != 0 && m_data == nullptr)
				m_data = new uint8_t[m_size];
		}

		const size_t m_count;
		const size_t m_size;
		uint8_t *m_data;
	};

	virtual void bind_buffer(vertex_buffer_interface *vb) = 0;
	virtual void unbind_buffer(vertex_buffer_interface *vb) = 0;
};


/// \defgroup osd_printf Diagnostic output functions
/// \{

// output channels
enum osd_output_channel
{
	OSD_OUTPUT_CHANNEL_ERROR,
	OSD_OUTPUT_CHANNEL_WARNING,
	OSD_OUTPUT_CHANNEL_INFO,
	OSD_OUTPUT_CHANNEL_DEBUG,
	OSD_OUTPUT_CHANNEL_VERBOSE,
	OSD_OUTPUT_CHANNEL_LOG,
	OSD_OUTPUT_CHANNEL_COUNT
};

class osd_output
{
public:
	osd_output() { }
	virtual ~osd_output() { }

	virtual void output_callback(osd_output_channel channel, util::format_argument_pack<std::ostream> const &args) = 0;

	static void push(osd_output *delegate);
	static void pop(osd_output *delegate);

protected:

	void chain_output(osd_output_channel channel, util::format_argument_pack<std::ostream> const &args) const
	{
		if (m_chain)
			m_chain->output_callback(channel, args);
	}

private:
	osd_output *m_chain = nullptr;
};

void osd_vprintf_error(util::format_argument_pack<std::ostream> const &args);
void osd_vprintf_warning(util::format_argument_pack<std::ostream> const &args);
void osd_vprintf_info(util::format_argument_pack<std::ostream> const &args);
void osd_vprintf_verbose(util::format_argument_pack<std::ostream> const &args);
void osd_vprintf_debug(util::format_argument_pack<std::ostream> const &args);

/// \brief Print error message
///
/// By default, error messages are sent to standard error.  The relaxed
/// format rules used by util::string_format apply.
/// \param [in] fmt Message format string.
/// \param [in] args Optional message format arguments.
/// \sa util::string_format
template <typename Format, typename... Params> void osd_printf_error(Format &&fmt, Params &&...args)
{
	return osd_vprintf_error(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
}

/// \brief Print warning message
///
/// By default, warning messages are sent to standard error.  The
/// relaxed format rules used by util::string_format apply.
/// \param [in] fmt Message format string.
/// \param [in] args Optional message format arguments.
/// \sa util::string_format
template <typename Format, typename... Params> void osd_printf_warning(Format &&fmt, Params &&...args)
{
	return osd_vprintf_warning(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
}

/// \brief Print informational message
///
/// By default, informational messages are sent to standard output.
/// The relaxed format rules used by util::string_format apply.
/// \param [in] fmt Message format string.
/// \param [in] args Optional message format arguments.
/// \sa util::string_format
template <typename Format, typename... Params> void osd_printf_info(Format &&fmt, Params &&...args)
{
	return osd_vprintf_info(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
}

/// \brief Print verbose diagnostic message
///
/// Verbose diagnostic messages are disabled by default.  If enabled,
/// they are sent to standard output by default.  The relaxed format
/// rules used by util::string_format apply.  Note that the format
/// string and arguments will always be evaluated, even if verbose
/// diagnostic messages are disabled.
/// \param [in] fmt Message format string.
/// \param [in] args Optional message format arguments.
/// \sa util::string_format
template <typename Format, typename... Params> void osd_printf_verbose(Format &&fmt, Params &&...args)
{
	return osd_vprintf_verbose(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
}

/// \brief Print debug message
///
/// By default, debug messages are sent to standard output for debug
/// builds only.  The relaxed format rules used by util::string_format
/// apply.  Note that the format string and arguments will always be
/// evaluated, even if debug messages are disabled.
/// \param [in] fmt Message format string.
/// \param [in] args Optional message format arguments.
/// \sa util::string_format
template <typename Format, typename... Params> void osd_printf_debug(Format &&fmt, Params &&...args)
{
	return osd_vprintf_debug(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
}

/// \}


// returns command line arguments as an std::vector<std::string> in UTF-8
std::vector<std::string> osd_get_command_line(int argc, char *argv[]);

/* discourage the use of printf directly */
/* sadly, can't do this because of the ATTR_PRINTF under GCC */
/*
#undef printf
#define printf !MUST_USE_osd_printf_*_CALLS_WITHIN_THE_CORE!
*/

// specifies "aggressive focus" - should MAME capture input for any windows co-habiting a MAME window?
void osd_set_aggressive_input_focus(bool aggressive_focus);

#endif // MAME_OSD_OSDCORE_H
