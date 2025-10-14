// license:BSD-3-Clause
// copyright-holders:Ville Linde, Aaron Giles
/***************************************************************************

    poly.h

    Polygon helper routines.

****************************************************************************

    Pixel model:

    (0.0,0.0)       (1.0,0.0)       (2.0,0.0)       (3.0,0.0)
        +---------------+---------------+---------------+
        |               |               |               |
        |               |               |               |
        |   (0.5,0.5)   |   (1.5,0.5)   |   (2.5,0.5)   |
        |       *       |       *       |       *       |
        |               |               |               |
        |               |               |               |
    (0.0,1.0)       (1.0,1.0)       (2.0,1.0)       (3.0,1.0)
        +---------------+---------------+---------------+
        |               |               |               |
        |               |               |               |
        |   (0.5,1.5)   |   (1.5,1.5)   |   (2.5,1.5)   |
        |       *       |       *       |       *       |
        |               |               |               |
        |               |               |               |
        |               |               |               |
        +---------------+---------------+---------------+
    (0.0,2.0)       (1.0,2.0)       (2.0,2.0)       (3.0,2.0)

***************************************************************************/

#ifndef MAME_VIDEO_POLY_H
#define MAME_VIDEO_POLY_H

#pragma once

#include <climits>
#include <atomic>


#define KEEP_POLY_STATISTICS 0
#define TRACK_POLY_WAITS 0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

static constexpr u8 POLY_FLAG_NO_WORK_QUEUE       = 0x01;
static constexpr u8 POLY_FLAG_NO_CLIPPING         = 0x02;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// base class for poly_array
class poly_array_base
{
public:
	// construction
	poly_array_base() { }

	// destruction
	virtual ~poly_array_base() { }

	// reset
	virtual void reset() = 0;
};


// class for managing an array of items
template<class ArrayType, int TrackingCount>
class poly_array : public poly_array_base
{
public:
	// this is really architecture-specific, but 64 is a reasonable
	// value for most modern x64/ARM architectures
	static constexpr size_t CACHE_LINE_SHIFT = 6;
	static constexpr size_t CACHE_LINE_SIZE = 1 << CACHE_LINE_SHIFT;
	static constexpr uintptr_t CACHE_LINE_MASK = ~uintptr_t(0) << CACHE_LINE_SHIFT;

	// size of an item, rounded up to the cache line size
	static constexpr size_t ITEM_SIZE = ((sizeof(ArrayType) + CACHE_LINE_SIZE - 1) / CACHE_LINE_SIZE) * CACHE_LINE_SIZE;

	// items are allocated in a 64k chunks
	static constexpr size_t CHUNK_GRANULARITY = 65536;

	// number of items in a chunk
	static constexpr u32 ITEMS_PER_CHUNK = CHUNK_GRANULARITY / ITEM_SIZE;

	// construction
	poly_array() :
		m_base(nullptr),
		m_next(0),
		m_max(0),
		m_allocated(0)
	{
		for (int index = 0; index < TrackingCount; index++)
			m_last[index] = nullptr;

		// allocate one chunk to start with
		realloc(ITEMS_PER_CHUNK);
	}

	// destruction
	virtual ~poly_array() { m_base = nullptr; }

	// getters
	u32 count() const { return m_next; }
	u32 max() const { return m_max; }
	size_t itemsize() const { return ITEM_SIZE; }
	u32 allocated() const { return m_allocated; }

	// return an item by index
	ArrayType &byindex(u32 index)
	{
		assert(index < m_next);
		if (index < m_allocated)
			return *item_ptr(index);
		assert(m_chain);
		return m_chain->byindex(index - m_allocated);
	}

	// return a contiguous chunk of items
	ArrayType *contiguous(u32 index, u32 count, u32 &chunk)
	{
		assert(index < m_next);
		assert(index + count <= m_next);
		if (index < m_allocated)
		{
			chunk = std::min(count, m_allocated - index);
			return item_ptr(index);
		}
		assert(m_chain);
		return m_chain->contiguous(index - m_allocated, count, chunk);
	}

	// compute the index
	int indexof(ArrayType &item) const
	{
		u32 result = (reinterpret_cast<u8 *>(&item) - m_base) / ITEM_SIZE;
		if (result < m_allocated)
			return result;
		assert(m_chain);
		return m_allocated + m_chain->indexof(item);
	}

	// operations
	virtual void reset() override
	{
		m_next = 0;

		// if we didn't have a chain, just repopulate
		if (!m_chain)
			repopulate();
		else
		{
			// otherwise, reallocate and get rid of the chain
			realloc(m_max);
			m_chain.reset();
		}
	}

	// allocate a return a new item
	ArrayType &next(int tracking_index = 0)
	{
		// track the maximum
		if (m_next > m_max)
			m_max = m_next;

		// fast case: fits within our array
		ArrayType *item;
		if (m_next < m_allocated)
			item = new(item_ptr(m_next)) ArrayType;

		// otherwise, allocate from the chain
		else
		{
			if (!m_chain)
				m_chain = std::make_unique<poly_array<ArrayType, 0>>();
			item = &m_chain->next();
		}

		// set the last item
		m_next++;
		if (TrackingCount > 0)
		{
			assert(tracking_index < TrackingCount);
			m_last[tracking_index] = item;
		}
		return *item;
	}

	// return the last
	ArrayType &last(int tracking_index = 0) const
	{
		assert(tracking_index < TrackingCount);
		assert(m_last[tracking_index] != nullptr);
		return *m_last[tracking_index];
	}

private:
	// internal helper to make size pointers
	ArrayType *item_ptr(u32 index)
	{
		assert(index < m_allocated);
		return reinterpret_cast<ArrayType *>(m_base + index * ITEM_SIZE);
	}

	// reallocate to the given size
	void realloc(u32 count)
	{
		// round the count up to a chunk size
		count = ((count + ITEMS_PER_CHUNK - 1) / ITEMS_PER_CHUNK) * ITEMS_PER_CHUNK;

		// allocate a fresh new array
		std::unique_ptr<u8[]> new_alloc = std::make_unique<u8[]>(ITEM_SIZE * count + CACHE_LINE_SIZE);
		std::fill_n(&new_alloc[0], ITEM_SIZE * count + CACHE_LINE_SIZE, 0);

		// align the base to a cache line
		m_base = reinterpret_cast<u8 *>((uintptr_t(new_alloc.get()) + CACHE_LINE_SIZE - 1) & CACHE_LINE_MASK);

		// repopulate last items into the base of the new array
		repopulate();

		// replace the old allocation with the new one
		m_alloc = std::move(new_alloc);
		m_allocated = count;
	}

	// repopulate items
	void repopulate()
	{
		for (int tracking_index = 0; tracking_index < TrackingCount; tracking_index++)
			if (m_last[tracking_index] != nullptr)
			{
				if (m_last[tracking_index] == item_ptr(m_next))
					m_next++;
				else
					next(tracking_index) = *m_last[tracking_index];
			}
	}

	// internal state
	u8 *m_base;
	u32 m_next;
	u32 m_max;
	u32 m_allocated;
	std::unique_ptr<u8[]> m_alloc;
	std::unique_ptr<poly_array<ArrayType, 0>> m_chain;
	std::array<ArrayType *, TrackingCount> m_last;
};


// poly_manager is a template class
template<typename BaseType, class ObjectType, int MaxParams, u8 Flags = 0>
class poly_manager
{
public:
	// each vertex has an X/Y coordinate and a set of parameters
	struct vertex_t
	{
		vertex_t() { }
		vertex_t(BaseType _x, BaseType _y) { x = _x; y = _y; }

		BaseType x, y;                          // X, Y coordinates
		std::array<BaseType, MaxParams> p;      // iterated parameters
	};

	// a single extent describes a span and a list of parameter extents
	struct extent_t
	{
		struct param_t
		{
			BaseType start;                     // parameter value at start
			BaseType dpdx;                      // dp/dx relative to start
			BaseType dpdy;                      // dp/dy relative to start
		};
		int16_t startx, stopx;                  // starting (inclusive)/ending (exclusive) endpoints
		std::array<param_t, MaxParams> param;   // array of parameter start/delays
		void *userdata;                         // custom per-span data
	};

	// delegate type for scanline callbacks
	using render_delegate = delegate<void (int32_t, extent_t const &, ObjectType const &, int)>;

	// poly_array of object data
	using objectdata_array = poly_array<ObjectType, 1>;

	// construction/destruction
	poly_manager(running_machine &machine);
	virtual ~poly_manager();

	// synchronization
	void wait(char const *debug_reason = "general");

	// return a reference to our ObjectType poly_array
	objectdata_array &object_data() { return m_object; }

	// register a poly_array to be reset after a wait
	void register_poly_array(poly_array_base &array) { m_arrays.push_back(&array); }

	// tiles
	template<int ParamCount>
	uint32_t render_tile(rectangle const &cliprect, render_delegate callback, vertex_t const &v1, vertex_t const &v2);

	// triangles
	template<int ParamCount>
	uint32_t render_triangle(rectangle const &cliprect, render_delegate callback, vertex_t const &v1, vertex_t const &v2, vertex_t const &v3);
	template<int ParamCount>
	uint32_t render_triangle_fan(rectangle const &cliprect, render_delegate callback, int numverts, vertex_t const *v);
	template<int ParamCount>
	uint32_t render_triangle_strip(rectangle const &cliprect, render_delegate callback, int numverts, vertex_t const *v);

	// polygons
	template<int NumVerts, int ParamCount>
	uint32_t render_polygon(rectangle const &cliprect, render_delegate callback, vertex_t const *v);

	// direct custom extents
	template<int ParamCount>
	uint32_t render_extents(rectangle const &cliprect, render_delegate callback, int startscanline, int numscanlines, extent_t const *extents);

	// public helpers
	template<int ParamCount>
	int zclip_if_less(int numverts, vertex_t const *v, vertex_t *outv, BaseType clipval);

private:
	// number of profiling ticks before we consider a wait "long"
	static constexpr osd_ticks_t POLY_LOG_WAIT_THRESHOLD = 1000;

	static constexpr int SCANLINES_PER_BUCKET = 32;
	static constexpr int TOTAL_BUCKETS        = (512 / SCANLINES_PER_BUCKET);

	// primitive_info describes a single primitive
	struct primitive_info
	{
		poly_manager *      m_owner;                // pointer back to the poly manager
		ObjectType *        m_object;               // object data pointer
		render_delegate     m_callback;             // callback to handle a scanline's worth of work
	};

	// internal unit of work
	struct work_unit
	{
		work_unit &operator=(work_unit const &rhs)
		{
			// this is just to satisfy the compiler; we don't actually copy
			fatalerror("Attempt to copy work_unit");
		}

		std::atomic<uint32_t> count_next;             // number of scanlines and index of next item to process
		primitive_info *      primitive;              // pointer to primitive
		int32_t               scanline;               // starting scanline
		uint32_t              previtem;               // index of previous item in the same bucket
		extent_t              extent[SCANLINES_PER_BUCKET]; // array of scanline extents
	};

	// internal array types
	using primitive_array = poly_array<primitive_info, 0>;
	using unit_array = poly_array<work_unit, 0>;

	// round in a cross-platform consistent manner
	inline int32_t round_coordinate(BaseType value)
	{
		int32_t result = int32_t(std::floor(value));
		if (value > 0 && result < 0)
			return INT_MAX - 1;
		return result + (value - BaseType(result) > BaseType(0.5));
	}

	// internal helpers
	primitive_info &primitive_alloc(int minx, int maxx, int miny, int maxy, render_delegate callback)
	{
		// return and initialize the next one
		primitive_info &primitive = m_primitive.next();
		primitive.m_owner = this;
		primitive.m_object = &m_object.last();
		primitive.m_callback = callback;
		return primitive;
	}

	// enqueue work items in contiguous chunks
	void queue_items(u32 start)
	{
		// do nothing if no queue; items will be processed on the next wait
		if (m_queue == nullptr)
			return;

		// enqueue the items in contiguous chunks
		while (start < m_unit.count())
		{
			u32 chunk;
			work_unit *base = m_unit.contiguous(start, m_unit.count() - start, chunk);
			osd_work_item_queue_multiple(m_queue, work_item_callback, chunk, base, m_unit.itemsize(), WORK_ITEM_FLAG_AUTO_RELEASE);
			start += chunk;
		}
	}

	static void *work_item_callback(void *param, int threadid);
	void presave() { wait("pre-save"); }

	// queue management
	osd_work_queue *m_queue;               // work queue

	// arrays
	primitive_array m_primitive;           // array of primitives
	objectdata_array m_object;             // array of object data
	unit_array m_unit;                     // array of work units
	std::vector<poly_array_base *> m_arrays; // list of arrays we are managing

	// buckets
	uint32_t m_unit_bucket[TOTAL_BUCKETS]; // buckets for tracking unit usage

	// statistics
	uint32_t m_tiles;                       // number of tiles queued
	uint32_t m_triangles;                   // number of triangles queued
	uint32_t m_polygons;                    // number of polygons queued
	uint64_t m_pixels;                      // number of pixels rendered
#if KEEP_POLY_STATISTICS
	uint32_t m_conflicts[WORK_MAX_THREADS] = { 0 }; // number of conflicts found, per thread
	uint32_t m_resolved[WORK_MAX_THREADS] = { 0 };  // number of conflicts resolved, per thread
#endif
#if TRACK_POLY_WAITS
	static std::string friendly_number(u64 number);
	struct wait_tracker
	{
		void update(int items, osd_ticks_t time)
		{
			total_waits++;
			if (items > 0)
			{
				total_actual_waits++;
				total_cycles += time;
				if (time < 100)
					bucket_waits[0]++;
				else if (time < 1000)
					bucket_waits[1]++;
				else if (time < 10000)
					bucket_waits[2]++;
				else
					bucket_waits[3]++;
			}
		}

		u32 total_waits = 0;
		u32 total_actual_waits = 0;
		u32 bucket_waits[4] = { 0 };
		u64 total_cycles = 0;
	};
	using waitmap_t = std::unordered_map<std::string, wait_tracker>;
	waitmap_t m_waitmap;
#endif
};


//-------------------------------------------------
//  poly_manager - constructor
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
poly_manager<BaseType, ObjectType, MaxParams, Flags>::poly_manager(running_machine &machine) :
	m_queue(nullptr),
	m_tiles(0),
	m_triangles(0),
	m_polygons(0),
	m_pixels(0)
{
	// create the work queue
	if (!(Flags & POLY_FLAG_NO_WORK_QUEUE))
		m_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI | WORK_QUEUE_FLAG_HIGH_FREQ);

	// initialize the buckets to empty
	std::fill_n(&m_unit_bucket[0], std::size(m_unit_bucket), 0xffffffff);

	// register our arrays for reset
	register_poly_array(m_primitive);
	register_poly_array(m_object);
	register_poly_array(m_unit);

	// request a pre-save callback for synchronization
	machine.save().register_presave(save_prepost_delegate(FUNC(poly_manager::presave), this));
}


//-------------------------------------------------
//  ~poly_manager - destructor
//-------------------------------------------------

#if TRACK_POLY_WAITS
template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
inline std::string poly_manager<BaseType, ObjectType, MaxParams, Flags>::friendly_number(u64 number)
{
	static char const s_suffixes[] = " kmbtqisp";
	double value = double(number);
	int suffixnum = 0;

	if (number < 1000000)
		return string_format("%6d ", int(number));
	while (value >= 1000)
	{
		value /= 1000.0;
		suffixnum++;
	}
	if (value >= 100)
		return string_format("%6.1f%c", value, s_suffixes[suffixnum]);
	if (value >= 10)
		return string_format("%6.2f%c", value, s_suffixes[suffixnum]);
	return string_format("%6.3f%c", value, s_suffixes[suffixnum]);
}
#endif

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
poly_manager<BaseType, ObjectType, MaxParams, Flags>::~poly_manager()
{
#if KEEP_POLY_STATISTICS
{
	// accumulate stats over the entire collection
	int conflicts = 0, resolved = 0;
	for (int i = 0; i < std::size(m_conflicts); i++)
	{
		conflicts += m_conflicts[i];
		resolved += m_resolved[i];
	}

	// output global stats
	osd_printf_info("Total triangles = %d\n", m_triangles);
	osd_printf_info("Total polygons = %d\n", m_polygons);
	if (m_pixels > 1000000000)
		osd_printf_info("Total pixels   = %d%09d\n", uint32_t(m_pixels / 1000000000), uint32_t(m_pixels % 1000000000));
	else
		osd_printf_info("Total pixels   = %d\n", uint32_t(m_pixels));

	osd_printf_info("Conflicts:   %d resolved, %d total\n", resolved, conflicts);
	osd_printf_info("Units:       %5d used, %5d allocated, %4d bytes each, %7d total\n", m_unit.max(), m_unit.allocated(), int(m_unit.itemsize()), int(m_unit.allocated() * m_unit.itemsize()));
	osd_printf_info("Primitives:  %5d used, %5d allocated, %4d bytes each, %7d total\n", m_primitive.max(), m_primitive.allocated(), int(m_primitive.itemsize()), int(m_primitive.allocated() * m_primitive.itemsize()));
	osd_printf_info("Object data: %5d used, %5d allocated, %4d bytes each, %7d total\n", m_object.max(), m_object.allocated(), int(m_object.itemsize()), int(m_object.allocated() * m_object.itemsize()));
}
#endif
#if TRACK_POLY_WAITS
{
	osd_printf_info("Wait summary:\n");
	osd_printf_info("Cause                        Cycles  Waits  Actuals Average   <100   100-1k  1k-10k   10k+\n");
	osd_printf_info("--------------------------  ------- ------- ------- ------- ------- ------- ------- -------\n");
	while (1)
	{
		typename waitmap_t::value_type *biggest = nullptr;
		for (auto &item : m_waitmap)
			if (item.second.total_cycles > 0)
				if (biggest == nullptr || item.second.total_cycles > biggest->second.total_cycles)
					biggest = &item;

		if (biggest == nullptr)
			break;

		osd_printf_info("%-28s%-7s %-7s %-7s %-7s %-7s %-7s %-7s %-7s\n",
			biggest->first.c_str(),
			friendly_number(biggest->second.total_cycles).c_str(),
			friendly_number(biggest->second.total_waits).c_str(),
			friendly_number(biggest->second.total_actual_waits).c_str(),
			(biggest->second.total_actual_waits == 0) ? "n/a" : friendly_number(biggest->second.total_cycles / biggest->second.total_actual_waits).c_str(),
			friendly_number(biggest->second.bucket_waits[0]).c_str(),
			friendly_number(biggest->second.bucket_waits[1]).c_str(),
			friendly_number(biggest->second.bucket_waits[2]).c_str(),
			friendly_number(biggest->second.bucket_waits[3]).c_str());

		biggest->second.total_cycles = 0;
	}
}
#endif

	// free the work queue
	if (m_queue != nullptr)
		osd_work_queue_free(m_queue);
}


//-------------------------------------------------
//  work_item_callback - process a work item
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
void *poly_manager<BaseType, ObjectType, MaxParams, Flags>::work_item_callback(void *param, int threadid)
{
	while (1)
	{
		work_unit &unit = *(work_unit *)param;
		primitive_info &primitive = *unit.primitive;
		int count = unit.count_next & 0xff;
		uint32_t orig_count_next;

		// if our previous item isn't done yet, enqueue this item to the end and proceed
		if (unit.previtem != 0xffffffff)
		{
			work_unit &prevunit = primitive.m_owner->m_unit.byindex(unit.previtem);
			if (prevunit.count_next != 0)
			{
				uint32_t unitnum = primitive.m_owner->m_unit.indexof(unit);
				uint32_t new_count_next;

				// attempt to atomically swap in this new value
				do
				{
					orig_count_next = prevunit.count_next;
					new_count_next = orig_count_next | (unitnum << 8);
				} while (!prevunit.count_next.compare_exchange_weak(orig_count_next, new_count_next, std::memory_order_release, std::memory_order_relaxed));

#if KEEP_POLY_STATISTICS
				// track resolved conflicts
				primitive.m_owner->m_conflicts[threadid]++;
				if (orig_count_next != 0)
					primitive.m_owner->m_resolved[threadid]++;
#endif
				// if we succeeded, skip out early so we can do other work
				if (orig_count_next != 0)
					break;
			}
		}

		// iterate over extents
		for (int curscan = 0; curscan < count; curscan++)
			primitive.m_callback(unit.scanline + curscan, unit.extent[curscan], *primitive.m_object, threadid);

		// set our count to 0 and re-fetch the original count value
		do
		{
			orig_count_next = unit.count_next;
		} while (!unit.count_next.compare_exchange_weak(orig_count_next, 0, std::memory_order_release, std::memory_order_relaxed));

		// if we have no more work to do, do nothing
		orig_count_next >>= 8;
		if (orig_count_next == 0)
			break;
		param = &primitive.m_owner->m_unit.byindex(orig_count_next);
	}
	return nullptr;
}


//-------------------------------------------------
//  wait - stall until all work is complete
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
void poly_manager<BaseType, ObjectType, MaxParams, Flags>::wait(char const *debug_reason)
{
	// early out if no units outstanding
	if (m_unit.count() == 0)
		return;

#if TRACK_POLY_WAITS
	int items = osd_work_queue_items(m_queue);
	osd_ticks_t time = get_profile_ticks();
#endif

	// wait for all pending work items to complete
	if (m_queue != nullptr)
		osd_work_queue_wait(m_queue, osd_ticks_per_second() * 100);

	// if we don't have a queue, just run the whole list now
	else
		for (int unitnum = 0; unitnum < m_unit.count(); unitnum++)
			work_item_callback(&m_unit.byindex(unitnum), 0);

#if TRACK_POLY_WAITS
	m_waitmap[debug_reason].update(items, get_profile_ticks() - time);
#endif

	// clear the buckets
	std::fill_n(&m_unit_bucket[0], std::size(m_unit_bucket), 0xffffffff);

	// reset all the poly arrays
	for (auto array : m_arrays)
		array->reset();
}


//-------------------------------------------------
//  render_tile - render a tile
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
template<int ParamCount>
uint32_t poly_manager<BaseType, ObjectType, MaxParams, Flags>::render_tile(rectangle const &cliprect, render_delegate callback, vertex_t const &_v1, vertex_t const &_v2)
{
	vertex_t const *v1 = &_v1;
	vertex_t const *v2 = &_v2;

	// first sort by Y
	if (v2->y < v1->y)
		std::swap(v1, v2);

	// compute some integral X/Y vertex values
	int32_t v1y = round_coordinate(v1->y);
	int32_t v2y = round_coordinate(v2->y);

	// clip coordinates
	int32_t v1yclip = v1y;
	int32_t v2yclip = v2y;
	if (!(Flags & POLY_FLAG_NO_CLIPPING))
	{
		v1yclip = std::max(v1yclip, cliprect.top());
		v2yclip = std::min(v2yclip, cliprect.bottom() + 1);
		if (v2yclip - v1yclip <= 0)
			return 0;
	}

	// determine total X extents
	BaseType minx = v1->x;
	BaseType maxx = v2->x;
	if (minx > maxx)
		return 0;

	// allocate and populate a new primitive
	primitive_info &primitive = primitive_alloc(round_coordinate(minx), round_coordinate(maxx), v1yclip, v2yclip, callback);

	// compute parameter deltas
	std::array<BaseType, ParamCount> param_dpdx;
	std::array<BaseType, ParamCount> param_dpdy;
	if (ParamCount > 0)
	{
		BaseType oox = BaseType(1.0) / (v2->x - v1->x);
		BaseType ooy = BaseType(1.0) / (v2->y - v1->y);
		for (int paramnum = 0; paramnum < ParamCount; paramnum++)
		{
			param_dpdx[paramnum] = oox * (v2->p[paramnum] - v1->p[paramnum]);
			param_dpdy[paramnum] = ooy * (v2->p[paramnum] - v1->p[paramnum]);
		}
	}

	// clamp to full pixels
	int32_t istartx = round_coordinate(v1->x);
	int32_t istopx = round_coordinate(v2->x);

	// force start < stop
	if (istartx > istopx)
		std::swap(istartx, istopx);

	// apply left/right clipping
	if (!(Flags & POLY_FLAG_NO_CLIPPING))
	{
		istartx = std::max(istartx, cliprect.left());
		istopx = std::min(istopx, cliprect.right() + 1);
		if (istartx >= istopx)
			return 0;
	}

	// compute the X extents for each scanline
	int32_t pixels = 0;
	uint32_t startunit = m_unit.count();
	int32_t scaninc = 1;
	for (int32_t curscan = v1yclip; curscan < v2yclip; curscan += scaninc)
	{
		uint32_t bucketnum = (uint32_t(curscan) / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		uint32_t unit_index = m_unit.count();
		work_unit &unit = m_unit.next();

		// determine how much to advance to hit the next bucket
		scaninc = SCANLINES_PER_BUCKET - uint32_t(curscan) % SCANLINES_PER_BUCKET;

		// fill in the work unit basics
		unit.primitive = &primitive;
		unit.count_next = std::min(v2yclip - curscan, scaninc);
		unit.scanline = curscan;
		unit.previtem = m_unit_bucket[bucketnum];
		m_unit_bucket[bucketnum] = unit_index;

		// iterate over extents
		for (int extnum = 0; extnum < unit.count_next; extnum++)
		{
			// set the extent and update the total pixel count
			extent_t &extent = unit.extent[extnum];
			extent.startx = istartx;
			extent.stopx = istopx;
			pixels += istopx - istartx;

			// fill in the parameters for the extent
			if (ParamCount > 0)
			{
				BaseType fullstartx = BaseType(istartx) + BaseType(0.5);
				BaseType fully = BaseType(curscan + extnum) + BaseType(0.5);
				for (int paramnum = 0; paramnum < ParamCount; paramnum++)
				{
					extent.param[paramnum].start = v1->p[paramnum] + fullstartx * param_dpdx[paramnum] + fully * param_dpdy[paramnum];
					extent.param[paramnum].dpdx = param_dpdx[paramnum];
					extent.param[paramnum].dpdy = param_dpdy[paramnum];
				}
			}
		}
	}

	// enqueue the work items
	queue_items(startunit);

	// return the total number of pixels in the triangle
	m_tiles++;
	m_pixels += pixels;
	return pixels;
}


//-------------------------------------------------
//  render_triangle - render a single triangle
//  given 3 vertexes
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
template<int ParamCount>
uint32_t poly_manager<BaseType, ObjectType, MaxParams, Flags>::render_triangle(const rectangle &cliprect, render_delegate callback, const vertex_t &_v1, const vertex_t &_v2, const vertex_t &_v3)
{
	vertex_t const *v1 = &_v1;
	vertex_t const *v2 = &_v2;
	vertex_t const *v3 = &_v3;

	// first sort by Y
	if (v2->y < v1->y)
		std::swap(v1, v2);
	if (v3->y < v2->y)
	{
		std::swap(v2, v3);
		if (v2->y < v1->y)
			std::swap(v1, v2);
	}

	// compute some integral X/Y vertex values
	int32_t v1y = round_coordinate(v1->y);
	int32_t v3y = round_coordinate(v3->y);

	// clip coordinates
	int32_t v1yclip = v1y;
	int32_t v3yclip = v3y;
	if (!(Flags & POLY_FLAG_NO_CLIPPING))
	{
		v1yclip = std::max(v1yclip, cliprect.top());
		v3yclip = std::min(v3yclip, cliprect.bottom() + 1);
		if (v3yclip - v1yclip <= 0)
			return 0;
	}

	// determine total X extents
	BaseType minx = std::min(std::min(v1->x, v2->x), v3->x);
	BaseType maxx = std::max(std::max(v1->x, v2->x), v3->x);

	// allocate and populate a new primitive
	primitive_info &primitive = primitive_alloc(round_coordinate(minx), round_coordinate(maxx), v1yclip, v3yclip, callback);

	// compute the slopes for each portion of the triangle
	BaseType dxdy_v1v2 = (v2->y == v1->y) ? BaseType(0.0) : (v2->x - v1->x) / (v2->y - v1->y);
	BaseType dxdy_v1v3 = (v3->y == v1->y) ? BaseType(0.0) : (v3->x - v1->x) / (v3->y - v1->y);
	BaseType dxdy_v2v3 = (v3->y == v2->y) ? BaseType(0.0) : (v3->x - v2->x) / (v3->y - v2->y);

	// compute parameter starting points and deltas
	std::array<BaseType, ParamCount> param_start;
	std::array<BaseType, ParamCount> param_dpdx;
	std::array<BaseType, ParamCount> param_dpdy;
	if (ParamCount > 0)
	{
		BaseType a00 = v2->y - v3->y;
		BaseType a01 = v3->x - v2->x;
		BaseType a02 = v2->x*v3->y - v3->x*v2->y;
		BaseType a10 = v3->y - v1->y;
		BaseType a11 = v1->x - v3->x;
		BaseType a12 = v3->x*v1->y - v1->x*v3->y;
		BaseType a20 = v1->y - v2->y;
		BaseType a21 = v2->x - v1->x;
		BaseType a22 = v1->x*v2->y - v2->x*v1->y;
		BaseType det = a02 + a12 + a22;

		if (std::abs(det) < BaseType(0.00001))
		{
			for (int paramnum = 0; paramnum < ParamCount; paramnum++)
			{
				param_dpdx[paramnum] = BaseType(0.0);
				param_dpdy[paramnum] = BaseType(0.0);
				param_start[paramnum] = v1->p[paramnum];
			}
		}
		else
		{
			BaseType idet = BaseType(1.0) / det;
			for (int paramnum = 0; paramnum < ParamCount; paramnum++)
			{
				param_dpdx[paramnum]  = idet * (v1->p[paramnum]*a00 + v2->p[paramnum]*a10 + v3->p[paramnum]*a20);
				param_dpdy[paramnum]  = idet * (v1->p[paramnum]*a01 + v2->p[paramnum]*a11 + v3->p[paramnum]*a21);
				param_start[paramnum] = idet * (v1->p[paramnum]*a02 + v2->p[paramnum]*a12 + v3->p[paramnum]*a22);
			}
		}
	}

	// compute the X extents for each scanline
	int32_t pixels = 0;
	uint32_t startunit = m_unit.count();
	int32_t scaninc = 1;
	for (int32_t curscan = v1yclip; curscan < v3yclip; curscan += scaninc)
	{
		uint32_t bucketnum = (uint32_t(curscan) / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		uint32_t unit_index = m_unit.count();
		work_unit &unit = m_unit.next();

		// determine how much to advance to hit the next bucket
		scaninc = SCANLINES_PER_BUCKET - uint32_t(curscan) % SCANLINES_PER_BUCKET;

		// fill in the work unit basics
		unit.primitive = &primitive;
		unit.count_next = std::min(v3yclip - curscan, scaninc);
		unit.scanline = curscan;
		unit.previtem = m_unit_bucket[bucketnum];
		m_unit_bucket[bucketnum] = unit_index;

		// iterate over extents
		for (int extnum = 0; extnum < unit.count_next; extnum++)
		{
			// compute the ending X based on which part of the triangle we're in
			BaseType fully = BaseType(curscan + extnum) + BaseType(0.5);
			BaseType startx = v1->x + (fully - v1->y) * dxdy_v1v3;
			BaseType stopx;
			if (fully < v2->y)
				stopx = v1->x + (fully - v1->y) * dxdy_v1v2;
			else
				stopx = v2->x + (fully - v2->y) * dxdy_v2v3;

			// clamp to full pixels
			int32_t istartx = round_coordinate(startx);
			int32_t istopx = round_coordinate(stopx);

			// force start < stop
			if (istartx > istopx)
				std::swap(istartx, istopx);

			// apply left/right clipping
			if (!(Flags & POLY_FLAG_NO_CLIPPING))
			{
				istartx = std::max(istartx, cliprect.left());
				istopx = std::min(istopx, cliprect.right() + 1);
			}

			// set the extent and update the total pixel count
			if (istartx >= istopx)
				istartx = istopx = 0;
			extent_t &extent = unit.extent[extnum];
			extent.startx = istartx;
			extent.stopx = istopx;
			pixels += istopx - istartx;

			// fill in the parameters for the extent
			BaseType fullstartx = BaseType(istartx) + BaseType(0.5);
			for (int paramnum = 0; paramnum < ParamCount; paramnum++)
			{
				extent.param[paramnum].start = param_start[paramnum] + fullstartx * param_dpdx[paramnum] + fully * param_dpdy[paramnum];
				extent.param[paramnum].dpdx = param_dpdx[paramnum];
				extent.param[paramnum].dpdy = param_dpdy[paramnum];
			}
		}
	}

	// enqueue the work items
	queue_items(startunit);

	// return the total number of pixels in the triangle
	m_triangles++;
	m_pixels += pixels;
	return pixels;
}


//-------------------------------------------------
//  render_triangle_fan - render a set of
//  triangles in a fan
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
template<int ParamCount>
uint32_t poly_manager<BaseType, ObjectType, MaxParams, Flags>::render_triangle_fan(rectangle const &cliprect, render_delegate callback, int numverts, vertex_t const *v)
{
	// iterate over vertices
	uint32_t pixels = 0;
	for (int vertnum = 2; vertnum < numverts; vertnum++)
		pixels += render_triangle<ParamCount>(cliprect, callback, v[0], v[vertnum - 1], v[vertnum]);
	return pixels;
}


//-------------------------------------------------
//  render_triangle_strip - render a set of
//  triangles in a strip
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
template<int ParamCount>
uint32_t poly_manager<BaseType, ObjectType, MaxParams, Flags>::render_triangle_strip(rectangle const &cliprect, render_delegate callback, int numverts, vertex_t const *v)
{
	// iterate over vertices
	uint32_t pixels = 0;
	for (int vertnum = 2; vertnum < numverts; vertnum++)
		pixels += render_triangle<ParamCount>(cliprect, callback, v[vertnum - 2], v[vertnum - 1], v[vertnum]);
	return pixels;
}


//-------------------------------------------------
//  render_extents - perform a custom render of
//  an object, given specific extents
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
template<int ParamCount>
uint32_t poly_manager<BaseType, ObjectType, MaxParams, Flags>::render_extents(rectangle const &cliprect, render_delegate callback, int startscanline, int numscanlines, extent_t const *extents)
{
	// clip coordinates
	int32_t v1yclip = startscanline;
	int32_t v3yclip = startscanline + numscanlines;
	if (!(Flags & POLY_FLAG_NO_CLIPPING))
	{
		v1yclip = std::max(v1yclip, cliprect.top());
		v3yclip = std::min(v3yclip, cliprect.bottom() + 1);
		if (v3yclip - v1yclip <= 0)
			return 0;
	}

	// allocate and populate a new primitive
	primitive_info &primitive = primitive_alloc(0, 0, v1yclip, v3yclip, callback);

	// compute the X extents for each scanline
	int32_t pixels = 0;
	uint32_t startunit = m_unit.count();
	int32_t scaninc = 1;
	for (int32_t curscan = v1yclip; curscan < v3yclip; curscan += scaninc)
	{
		uint32_t bucketnum = (uint32_t(curscan) / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		uint32_t unit_index = m_unit.count();
		work_unit &unit = m_unit.next();

		// determine how much to advance to hit the next bucket
		scaninc = SCANLINES_PER_BUCKET - uint32_t(curscan) % SCANLINES_PER_BUCKET;

		// fill in the work unit basics
		unit.primitive = &primitive;
		unit.count_next = std::min(v3yclip - curscan, scaninc);
		unit.scanline = curscan;
		unit.previtem = m_unit_bucket[bucketnum];
		m_unit_bucket[bucketnum] = unit_index;

		// iterate over extents
		for (int extnum = 0; extnum < unit.count_next; extnum++)
		{
			extent_t const &srcextent = extents[(curscan + extnum) - startscanline];
			int32_t istartx = srcextent.startx, istopx = srcextent.stopx;

			// apply left/right clipping
			if (!(Flags & POLY_FLAG_NO_CLIPPING))
			{
				istartx = std::max(istartx, cliprect.left());
				istartx = std::min(istartx, cliprect.right() + 1);
				istopx = std::max(istopx, cliprect.left());
				istopx = std::min(istopx, cliprect.right() + 1);
			}

			// set the extent and update the total pixel count
			extent_t &extent = unit.extent[extnum];
			extent.startx = istartx;
			extent.stopx = istopx;

			// fill in the parameters for the extent
			for (int paramnum = 0; paramnum < ParamCount; paramnum++)
			{
				extent.param[paramnum].start = srcextent.param[paramnum].start;
				extent.param[paramnum].dpdx = srcextent.param[paramnum].dpdx;
				extent.param[paramnum].dpdy = srcextent.param[paramnum].dpdy;
			}
			extent.userdata = srcextent.userdata;

			if (istartx < istopx)
				pixels += istopx - istartx;
			else if (istopx < istartx)
				pixels += istartx - istopx;
		}
	}

	// enqueue the work items
	queue_items(startunit);

	// return the total number of pixels in the object
	m_triangles++;
	m_pixels += pixels;
	return pixels;
}


//-------------------------------------------------
//  render_polygon - render a single polygon up
//  to 32 vertices
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
template<int NumVerts, int ParamCount>
uint32_t poly_manager<BaseType, ObjectType, MaxParams, Flags>::render_polygon(rectangle const &cliprect, render_delegate callback, vertex_t const *v)
{
	// determine min/max Y vertices
	BaseType minx = v[0].x;
	BaseType maxx = v[0].x;
	int minv = 0;
	int maxv = 0;
	for (int vertnum = 1; vertnum < NumVerts; vertnum++)
	{
		if (v[vertnum].y < v[minv].y)
			minv = vertnum;
		else if (v[vertnum].y > v[maxv].y)
			maxv = vertnum;
		minx = std::min(minx, v[vertnum].x);
		maxx = std::max(maxx, v[vertnum].x);
	}

	// determine start/end scanlines
	int32_t miny = round_coordinate(v[minv].y);
	int32_t maxy = round_coordinate(v[maxv].y);

	// clip coordinates
	int32_t minyclip = miny;
	int32_t maxyclip = maxy;
	if (!(Flags & POLY_FLAG_NO_CLIPPING))
	{
		minyclip = std::max(minyclip, cliprect.top());
		maxyclip = std::min(maxyclip, cliprect.bottom() + 1);
		if (maxyclip - minyclip <= 0)
			return 0;
	}

	// allocate a new primitive
	primitive_info &primitive = primitive_alloc(round_coordinate(minx), round_coordinate(maxx), minyclip, maxyclip, callback);

	// walk forward to build up the forward edge list
	struct poly_edge
	{
		poly_edge *next;                       // next edge in sequence
		int index;                             // index of this edge
		vertex_t const *v1;                    // pointer to first vertex
		vertex_t const *v2;                    // pointer to second vertex
		BaseType dxdy;                         // dx/dy along the edge
		std::array<BaseType, MaxParams> dpdy;  // per-parameter dp/dy values
	};
	poly_edge fedgelist[NumVerts - 1];
	poly_edge *edgeptr = &fedgelist[0];
	for (int curv = minv; curv != maxv; curv = (curv == NumVerts - 1) ? 0 : (curv + 1))
	{
		// set the two vertices
		edgeptr->v1 = &v[curv];
		edgeptr->v2 = &v[(curv == NumVerts - 1) ? 0 : (curv + 1)];

		// if horizontal, skip altogether
		if (edgeptr->v1->y == edgeptr->v2->y)
			continue;

		// need dx/dy always, and parameter deltas as necessary
		BaseType ooy = BaseType(1.0) / (edgeptr->v2->y - edgeptr->v1->y);
		edgeptr->dxdy = (edgeptr->v2->x - edgeptr->v1->x) * ooy;
		for (int paramnum = 0; paramnum < ParamCount; paramnum++)
			edgeptr->dpdy[paramnum] = (edgeptr->v2->p[paramnum] - edgeptr->v1->p[paramnum]) * ooy;
		++edgeptr;
	}

	// walk backward to build up the backward edge list
	poly_edge bedgelist[NumVerts - 1];
	edgeptr = &bedgelist[0];
	for (int curv = minv; curv != maxv; curv = (curv == 0) ? (NumVerts - 1) : (curv - 1))
	{
		// set the two vertices
		edgeptr->v1 = &v[curv];
		edgeptr->v2 = &v[(curv == 0) ? (NumVerts - 1) : (curv - 1)];

		// if horizontal, skip altogether
		if (edgeptr->v1->y == edgeptr->v2->y)
			continue;

		// need dx/dy always, and parameter deltas as necessary
		BaseType ooy = BaseType(1.0) / (edgeptr->v2->y - edgeptr->v1->y);
		edgeptr->dxdy = (edgeptr->v2->x - edgeptr->v1->x) * ooy;
		for (int paramnum = 0; paramnum < ParamCount; paramnum++)
			edgeptr->dpdy[paramnum] = (edgeptr->v2->p[paramnum] - edgeptr->v1->p[paramnum]) * ooy;
		++edgeptr;
	}

	// determine which list is left/right:
	// if the first vertex is shared, compare the slopes
	// if the first vertex is not shared, compare the X coordinates
	poly_edge const *ledge, *redge;
	if ((fedgelist[0].v1 == bedgelist[0].v1 && fedgelist[0].dxdy < bedgelist[0].dxdy) ||
		(fedgelist[0].v1 != bedgelist[0].v1 && fedgelist[0].v1->x < bedgelist[0].v1->x))
	{
		ledge = fedgelist;
		redge = bedgelist;
	}
	else
	{
		ledge = bedgelist;
		redge = fedgelist;
	}

	// compute the X extents for each scanline
	int32_t pixels = 0;
	uint32_t startunit = m_unit.count();
	int32_t scaninc = 1;
	for (int32_t curscan = minyclip; curscan < maxyclip; curscan += scaninc)
	{
		uint32_t bucketnum = (uint32_t(curscan) / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		uint32_t unit_index = m_unit.count();
		work_unit &unit = m_unit.next();

		// determine how much to advance to hit the next bucket
		scaninc = SCANLINES_PER_BUCKET - uint32_t(curscan) % SCANLINES_PER_BUCKET;

		// fill in the work unit basics
		unit.primitive = &primitive;
		unit.count_next = std::min(maxyclip - curscan, scaninc);
		unit.scanline = curscan;
		unit.previtem = m_unit_bucket[bucketnum];
		m_unit_bucket[bucketnum] = unit_index;

		// iterate over extents
		for (int extnum = 0; extnum < unit.count_next; extnum++)
		{
			// compute the ending X based on which part of the triangle we're in
			BaseType fully = BaseType(curscan + extnum) + BaseType(0.5);
			while (fully > ledge->v2->y && fully < v[maxv].y)
				++ledge;
			while (fully > redge->v2->y && fully < v[maxv].y)
				++redge;
			BaseType startx = ledge->v1->x + (fully - ledge->v1->y) * ledge->dxdy;
			BaseType stopx = redge->v1->x + (fully - redge->v1->y) * redge->dxdy;

			// clamp to full pixels
			int32_t istartx = round_coordinate(startx);
			int32_t istopx = round_coordinate(stopx);

			// compute parameter starting points and deltas
			extent_t &extent = unit.extent[extnum];
			if (ParamCount > 0)
			{
				BaseType ldy = fully - ledge->v1->y;
				BaseType rdy = fully - redge->v1->y;
				BaseType oox = BaseType(1.0) / (stopx - startx);

				// iterate over parameters
				for (int paramnum = 0; paramnum < ParamCount; paramnum++)
				{
					BaseType lparam = ledge->v1->p[paramnum] + ldy * ledge->dpdy[paramnum];
					BaseType rparam = redge->v1->p[paramnum] + rdy * redge->dpdy[paramnum];
					BaseType dpdx = (rparam - lparam) * oox;

					extent.param[paramnum].start = lparam;// - (BaseType(istartx) + 0.5f) * dpdx;
					extent.param[paramnum].dpdx = dpdx;
					extent.param[paramnum].dpdy = ledge->dpdy[paramnum];
				}
			}

			// apply left/right clipping
			if (!(Flags & POLY_FLAG_NO_CLIPPING))
			{
				if (istartx < cliprect.left())
				{
					for (int paramnum = 0; paramnum < ParamCount; paramnum++)
						extent.param[paramnum].start += (cliprect.left() - istartx) * extent.param[paramnum].dpdx;
					istartx = cliprect.left();
				}
				if (istopx > cliprect.right())
					istopx = cliprect.right() + 1;
			}

			// set the extent and update the total pixel count
			if (istartx >= istopx)
				istartx = istopx = 0;
			extent.startx = istartx;
			extent.stopx = istopx;
			pixels += istopx - istartx;
		}
	}

	// enqueue the work items
	queue_items(startunit);

	// return the total number of pixels in the polygon
	m_polygons++;
	m_pixels += pixels;
	return pixels;
}


//-------------------------------------------------
//  zclip_if_less - clip a polygon using p[0] as
//  a z coordinate
//-------------------------------------------------

template<typename BaseType, class ObjectType, int MaxParams, u8 Flags>
template<int ParamCount>
int poly_manager<BaseType, ObjectType, MaxParams, Flags>::zclip_if_less(int numverts, vertex_t const *v, vertex_t *outv, BaseType clipval)
{
	bool prevclipped = (v[numverts - 1].p[0] < clipval);
	vertex_t *nextout = outv;

	// iterate over vertices
	for (int vertnum = 0; vertnum < numverts; vertnum++)
	{
		bool thisclipped = (v[vertnum].p[0] < clipval);

		// if we switched from clipped to non-clipped, interpolate a vertex
		if (thisclipped != prevclipped)
		{
			vertex_t const &v1 = v[(vertnum == 0) ? (numverts - 1) : (vertnum - 1)];
			vertex_t const &v2 = v[vertnum];
			BaseType frac = (clipval - v1.p[0]) / (v2.p[0] - v1.p[0]);
			nextout->x = v1.x + frac * (v2.x - v1.x);
			nextout->y = v1.y + frac * (v2.y - v1.y);
			for (int paramnum = 0; paramnum < ParamCount; paramnum++)
				nextout->p[paramnum] = v1.p[paramnum] + frac * (v2.p[paramnum] - v1.p[paramnum]);
			++nextout;
		}

		// if this vertex is not clipped, copy it in
		if (!thisclipped)
			*nextout++ = v[vertnum];

		// remember the last state
		prevclipped = thisclipped;
	}
	return nextout - outv;
}


template<typename BaseType, int MaxParams>
struct frustum_clip_vertex
{
	BaseType x, y, z, w;                // A 3d coordinate already transformed by a projection matrix
	std::array<BaseType, MaxParams> p;  // Additional parameters to clip
};


template<typename BaseType, int MaxParams>
int frustum_clip_w(frustum_clip_vertex<BaseType, MaxParams> const *v, int num_vertices, frustum_clip_vertex<BaseType, MaxParams> *out)
{
	if (num_vertices <= 0)
		return 0;

	const BaseType W_PLANE = 0.000001f;

	frustum_clip_vertex<BaseType, MaxParams> clipv[10];
	int clip_verts = 0;

	int previ = num_vertices - 1;

	for (int i=0; i < num_vertices; i++)
	{
		int v1_side = (v[i].w < W_PLANE) ? -1 : 1;
		int v2_side = (v[previ].w < W_PLANE) ? -1 : 1;

		if ((v1_side * v2_side) < 0)        // edge goes through W plane
		{
			// insert vertex at intersection point
			BaseType wdiv = v[previ].w - v[i].w;
			if (wdiv == 0.0f)       // 0 edge means degenerate polygon
				return 0;

			BaseType t = fabs((W_PLANE - v[previ].w) / wdiv);

			clipv[clip_verts].x = v[previ].x + ((v[i].x - v[previ].x) * t);
			clipv[clip_verts].y = v[previ].y + ((v[i].y - v[previ].y) * t);
			clipv[clip_verts].z = v[previ].z + ((v[i].z - v[previ].z) * t);
			clipv[clip_verts].w = v[previ].w + ((v[i].w - v[previ].w) * t);

			// Interpolate the rest of the parameters
			for (int pi = 0; pi < MaxParams; pi++)
				clipv[clip_verts].p[pi] = v[previ].p[pi] + ((v[i].p[pi] - v[previ].p[pi]) * t);

			++clip_verts;
		}
		if (v1_side > 0)                // current point is inside
		{
			clipv[clip_verts] = v[i];
			++clip_verts;
		}

		previ = i;
	}

	memcpy(&out[0], &clipv[0], sizeof(out[0]) * clip_verts);
	return clip_verts;
}


template<typename BaseType, int MaxParams>
int frustum_clip(frustum_clip_vertex<BaseType, MaxParams> const *v, int num_vertices, frustum_clip_vertex<BaseType, MaxParams> *out, int axis, int sign)
{
	if (num_vertices <= 0)
		return 0;

	frustum_clip_vertex<BaseType, MaxParams> clipv[10];
	int clip_verts = 0;

	int previ = num_vertices - 1;

	for (int i=0; i < num_vertices; i++)
	{
		int v1_side, v2_side;
		BaseType* v1a = (BaseType*)&v[i];
		BaseType* v2a = (BaseType*)&v[previ];

		BaseType v1_axis, v2_axis;

		if (sign)       // +axis
		{
			v1_axis = v1a[axis];
			v2_axis = v2a[axis];
		}
		else            // -axis
		{
			v1_axis = -v1a[axis];
			v2_axis = -v2a[axis];
		}

		v1_side = (v1_axis <= v[i].w) ? 1 : -1;
		v2_side = (v2_axis <= v[previ].w) ? 1 : -1;

		if ((v1_side * v2_side) < 0)        // edge goes through W plane
		{
			// insert vertex at intersection point
			BaseType wdiv = ((v[previ].w - v2_axis) - (v[i].w - v1_axis));

			if (wdiv == 0.0f)           // 0 edge means degenerate polygon
				return 0;

			BaseType t = fabs((v[previ].w - v2_axis) / wdiv);

			clipv[clip_verts].x = v[previ].x + ((v[i].x - v[previ].x) * t);
			clipv[clip_verts].y = v[previ].y + ((v[i].y - v[previ].y) * t);
			clipv[clip_verts].z = v[previ].z + ((v[i].z - v[previ].z) * t);
			clipv[clip_verts].w = v[previ].w + ((v[i].w - v[previ].w) * t);

			// Interpolate the rest of the parameters
			for (int pi = 0; pi < MaxParams; pi++)
				clipv[clip_verts].p[pi] = v[previ].p[pi] + ((v[i].p[pi] - v[previ].p[pi]) * t);

			++clip_verts;
		}
		if (v1_side > 0)                // current point is inside
		{
			clipv[clip_verts] = v[i];
			++clip_verts;
		}

		previ = i;
	}

	memcpy(&out[0], &clipv[0], sizeof(out[0]) * clip_verts);
	return clip_verts;
}


template<typename BaseType, int MaxParams>
int frustum_clip_all(frustum_clip_vertex<BaseType, MaxParams> *clip_vert, int num_vertices, frustum_clip_vertex<BaseType, MaxParams> *out)
{
	num_vertices = frustum_clip_w<BaseType, MaxParams>(clip_vert, num_vertices, clip_vert);
	num_vertices = frustum_clip<BaseType, MaxParams>(clip_vert, num_vertices, clip_vert, 0, 0);      // W <= -X
	num_vertices = frustum_clip<BaseType, MaxParams>(clip_vert, num_vertices, clip_vert, 0, 1);      // W <= +X
	num_vertices = frustum_clip<BaseType, MaxParams>(clip_vert, num_vertices, clip_vert, 1, 0);      // W <= -Y
	num_vertices = frustum_clip<BaseType, MaxParams>(clip_vert, num_vertices, clip_vert, 1, 1);      // W <= +X
	num_vertices = frustum_clip<BaseType, MaxParams>(clip_vert, num_vertices, clip_vert, 2, 0);      // W <= -Z
	num_vertices = frustum_clip<BaseType, MaxParams>(clip_vert, num_vertices, clip_vert, 2, 1);      // W <= +Z
	out = clip_vert;
	return num_vertices;
}


#endif  // MAME_DEVICES_VIDEO_POLY_H
