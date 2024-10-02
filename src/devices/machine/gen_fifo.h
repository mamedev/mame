// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_MACHINE_GEN_FIFO_H
#define MAME_MACHINE_GEN_FIFO_H


/* Generic fifo device with flow control
 *
 *
 * To use it:
 * - Create the device, put push/pop in the appropriate memory maps of
 *   the source (push) and destination (pop) devices
 * - Connect the empty/full callbacks in the MCFG if needed
 * - Call setup at init with the size and the callbacks
 * - Enjoy
 *
 * The design is such that destination devices must be able to be
 * halted and to retrigger a fifo read when asked to, while the source
 * devices only need to be haltable.  There is some leeway on the
 * source devices, e.g. no values are ever lost, so one can rudely
 * overflow the fifo when the source is not a simple executable
 * device.
 *
 * The callbacks:
 * - on_fifo_empty_pre_sync:
 *     Called on a pop with an empty fifo.  Must ask the destination
 *     to try again (e.g. ->stall() or equivalent).  The pop itself
 *     will then return zero.  Triggers a machine-wide sync, because
 *     the source device may be behind and could push data in the
 *     remaining of its timeslice.
 *
 * - on_fifo_empty_post_sync:
 *     Called after the sync consecutive to a pop with an empty fifo.
 *     It means that even with the source device synced the fifo is
 *     still empty.  The destination device should be halted.
 *
 * - on_fifo_unempty:
 *     Called when the fifo is filled again after a call to
 *     on_fifo_empty_post_sync.  The destination device should be
 *     restarted, the pop will succeed this time.
 *
 * - on_fifo_full_post_sync:
 *     When the source pushes in a full fifo, the extra value is
 *     stored and a machine sync is triggered to give a chance to the
 *     destination device to pop some of the fifo data in its
 *     remaining timeslice.  If after the sync the fifo is still full,
 *     that callback is triggered.  The source device should be halted.
 *
 * - on_fifo_unfull:
 *     Called when the fifo again has free space after a call to
 *     on_fifo_full_post_sync.  The source device should be restarted.
 *
 * - on_push:
 *     Called when a new value was just pushed.  That callback is
 *     usually not needed, but it can be useful when the destination
 *     is not an executable device but something hardcoded
 *     (rasterizer, etc).
 *
 * - on_pop:
 *     Called when a new value was just popped.  That callback is
 *     usually not needed, but it can be useful when the source is not
 *     an executable device but something hardcoded.
 *
 * Note: setup can be called multiple times, each call overrides the
 * previous and clears the fifo.
 *
 * Note: the fifo element type T must be copyable if one wants to use
 * peek().  It must trivially copyable and of size 1, 2, 4 or 8 bytes
 * to use the memory-map accessors.  Otherwise only movability is
 * required.
 */


template<typename T> class generic_fifo_device_base : public device_t {
public:
	/* The general setup.  Call be called multiple times, clears the fifo. */
	void setup(size_t size,
			   std::function<void ()> on_fifo_empty_pre_sync,
			   std::function<void ()> on_fifo_empty_post_sync,
			   std::function<void ()> on_fifo_unempty,
			   std::function<void ()> on_fifo_full_post_sync,
			   std::function<void ()> on_fifo_unfull,
			   std::function<void ()> on_push,
			   std::function<void ()> on_pop) {
		m_on_fifo_empty_pre_sync = on_fifo_empty_pre_sync;
		m_on_fifo_empty_post_sync = on_fifo_empty_post_sync;
		m_on_fifo_unempty = on_fifo_unempty;
		m_on_fifo_full_post_sync = on_fifo_full_post_sync;
		m_on_fifo_unfull = on_fifo_unfull;
		m_on_push = on_push;
		m_on_pop = on_pop;

		clear();
		m_size = size;
	}

	/* Generic push/pop */
	T pop();
	void push(T value);

	/* Indicates whether the fifo is empty or full.  Note that a pop
	   on a full fifo does not ensure it will become non-full, there
	   may be extra values stored.  Also, an empty fifo can fill up
	   from extra values later after a sync. */

	bool is_empty() const { return m_values.empty(); }
	bool is_full() const { return m_values.size() >= m_size; }

	/* Empty the fifo. */
	void clear();

	/* Callbacks signalling empty (true)/nonempty (false) and full (true)/nonfull (false) */
	auto empty_cb() { return m_empty_cb.bind(); }
	auto full_cb() { return m_full_cb.bind(); }

	/* Get the fifo current size - Note that real hardware usually
	   can't do that.  May be bigger that the fifo size if some extra
	   values are stored. */
	size_t size() const { return m_values.size() + m_extra_values.size(); }

	/* Peek at a value in the fifo at an offset, 0 being the next
	   value to be popped, 1 the one following, etc - Note that real
	   hardware usually can't do that.  Returns 0 or equivalent when
	   the offset is over the current fifo size. */
	template <typename U=T> std::enable_if_t<(std::is_copy_constructible<T>::value && std::is_same<T, U>::value), T> peek(offs_t offset) const {
		if(offset < m_values.size())
			return m_values[offset];
		offset -= m_values.size();
		if(offset < m_extra_values.size())
			return m_extra_values[offset];
		return T();
	}

protected:
	generic_fifo_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual ~generic_fifo_device_base() = default;

	/* These used to build memory accessors that can be put in a
	   memory map.  They do the appropriate bit-identical type
	   conversions if you say make a fifo of floats and want to access
	   in with a 32-bits handler, which deals with 32-bits unsigned
	   integers.  Don't be afraid by the apparently costly memcpy, the
	   compiler sees through it and removes it, and that's the only
	   standard-sanctioned way to do that.

	   They need to be upcalled in the deriving class from real
	   accessors though.
	*/

	template <typename U> std::enable_if_t<(std::is_trivially_copyable<T>::value && sizeof(T) == sizeof(U)), void> write_gen(U data) {
		T t;
		memcpy(&t, &data, sizeof(data));
		push(std::move(t));
	}

	template <typename U> std::enable_if_t<(std::is_trivially_copyable<T>::value && sizeof(T) == sizeof(U)), U> read_gen() {
		T t(pop());
		U data;
		memcpy(&data, &t, sizeof(data));
		return data;
	}


private:
	// Timer IDs for sync on empty and full
	enum timer_ids { T_EMPTY, T_FULL };

	// Configured callbacks

	devcb_write_line m_empty_cb;
	devcb_write_line m_full_cb;

	std::function<void ()> m_on_fifo_empty_pre_sync;
	std::function<void ()> m_on_fifo_empty_post_sync;
	std::function<void ()> m_on_fifo_unempty;
	std::function<void ()> m_on_fifo_full_post_sync;
	std::function<void ()> m_on_fifo_unfull;
	std::function<void ()> m_on_push;
	std::function<void ()> m_on_pop;

	// The values are stored into two vectors for simplicity.
	// m_values may become a rotating buffer when everything else
	// works.  m_extra_values should probably stay a vector, but could
	// become a list.

	std::vector<T> m_values;
	std::vector<T> m_extra_values;

	// The synchronization timers
	emu_timer *m_sync_empty, *m_sync_full;

	// Configured size of the fifo
	size_t m_size;

	// Notes whether the halting callbacks were triggered
	bool m_empty_triggered, m_full_triggered;

	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_empty);
	TIMER_CALLBACK_MEMBER(sync_full);
};

class generic_fifo_u32_device : public generic_fifo_device_base<u32>
{
public:
	generic_fifo_u32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~generic_fifo_u32_device() = default;

	u32 read() { return read_gen<u32>(); }
	void write(u32 data) { write_gen(data); }
};

DECLARE_DEVICE_TYPE(GENERIC_FIFO_U32, generic_fifo_u32_device)

#endif
