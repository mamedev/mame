/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "precompiled.h"
#include <Rocket/Core/Dictionary.h>

/* NOTE: This is dictionary implementation is copied from PYTHON 
   which is well known for its dictionary SPEED.

   It uses an optimised hash table implementation. */

/*
 There are three kinds of slots in the table:
 
 1. Unused.  me_key == me_value == NULL
 Does not hold an active (key, value) pair now and never did.  Unused can
 transition to Active upon key insertion.  This is the only case in which
 me_key is NULL, and is each slot's initial state.
 
 2. Active.  me_key != NULL and me_key != dummy and me_value != NULL
 Holds an active (key, value) pair.  Active can transition to Dummy upon
 key deletion.  This is the only case in which me_value != NULL.
 
 3. Dummy.  me_key == dummy and me_value == NULL
 Previously held an active (key, value) pair, but that was deleted and an
 active pair has not yet overwritten the slot.  Dummy can transition to
 Active upon key insertion.  Dummy slots cannot be made Unused again
 (cannot have me_key set to NULL), else the probe sequence in case of
 collision would have no way to know they were once active.
 
 Note: .popitem() abuses the me_hash field of an Unused or Dummy slot to
 hold a search finger.  The me_hash field of Unused or Dummy slots has no
 meaning otherwise.
 
 
 To ensure the lookup algorithm terminates, there must be at least one Unused
 slot (NULL key) in the table.
 The value ma_fill is the number of non-NULL keys (sum of Active and Dummy);
 ma_used is the number of non-NULL, non-dummy keys (== the number of non-NULL
													 values == the number of Active items).
 To avoid slowing down lookups on a near-full table, we resize the table when
 it's two-thirds full.
*/

namespace Rocket {
namespace Core {


/* See large comment block below.  This must be >= 1. */
#define PERTURB_SHIFT 5

/* switch these defines if you want dumps all dictionary access */
#define DICTIONARY_DEBUG_CODE(x)
//#define DICTIONARY_DEBUG_CODE(x) x

/*
 Major subtleties ahead:  Most hash schemes depend on having a "good" hash
 function, in the sense of simulating randomness.  Python doesn't:  its most
 important hash functions (for strings and ints) are very regular in common
 cases:
 
 >>> map(hash, (0, 1, 2, 3))
 [0, 1, 2, 3]
 >>> map(hash, ("namea", "nameb", "namec", "named"))
 [-1658398457, -1658398460, -1658398459, -1658398462]
 >>>
 
 This isn't necessarily bad!  To the contrary, in a table of size 2**i, taking
 the low-order i bits as the initial table index is extremely fast, and there
 are no collisions at all for dicts indexed by a contiguous range of ints.
 The same is approximately true when keys are "consecutive" strings.  So this
 gives better-than-random behavior in common cases, and that's very desirable.
 
 OTOH, when collisions occur, the tendency to fill contiguous slices of the
 hash table makes a good collision resolution strategy crucial.  Taking only
 the last i bits of the hash code is also vulnerable:  for example, consider
 [i << 16 for i in range(20000)] as a set of keys.  Since ints are their own
 hash codes, and this fits in a dict of size 2**15, the last 15 bits of every
 hash code are all 0:  they *all* map to the same table index.
 
 But catering to unusual cases should not slow the usual ones, so we just take
 the last i bits anyway.  It's up to collision resolution to do the rest.  If
 we *usually* find the key we're looking for on the first try (and, it turns
 out, we usually do -- the table load factor is kept under 2/3, so the odds
 are solidly in our favor), then it makes best sense to keep the initial index
 computation dirt cheap.
 
 The first half of collision resolution is to visit table indices via this
 recurrence:
 
 j = ((5*j) + 1) mod 2**i
 
 For any initial j in range(2**i), repeating that 2**i times generates each
 int in range(2**i) exactly once (see any text on random-number generation for
 proof).  By itself, this doesn't help much:  like linear probing (setting
 j += 1, or j -= 1, on each loop trip), it scans the table entries in a fixed
 order.  This would be bad, except that's not the only thing we do, and it's
 actually *good* in the common cases where hash keys are consecutive.  In an
 example that's really too small to make this entirely clear, for a table of
 size 2**3 the order of indices is:
 
 0 -> 1 -> 6 -> 7 -> 4 -> 5 -> 2 -> 3 -> 0 [and here it's repeating]
 
 If two things come in at index 5, the first place we look after is index 2,
 not 6, so if another comes in at index 6 the collision at 5 didn't hurt it.
 Linear probing is deadly in this case because there the fixed probe order
 is the *same* as the order consecutive keys are likely to arrive.  But it's
 extremely unlikely hash codes will follow a 5*j+1 recurrence by accident,
 and certain that consecutive hash codes do not.
 
 The other half of the strategy is to get the other bits of the hash code
 into play.  This is done by initializing a (unsigned) vrbl "perturb" to the
 full hash code, and changing the recurrence to:
 
 j = (5*j) + 1 + perturb;
 perturb >>= PERTURB_SHIFT;
 use j % 2**i as the next table index;
 
 Now the probe sequence depends (eventually) on every bit in the hash code,
 and the pseudo-scrambling property of recurring on 5*j+1 is more valuable,
 because it quickly magnifies small differences in the bits that didn't affect
 the initial index.  Note that because perturb is unsigned, if the recurrence
 is executed often enough perturb eventually becomes and remains 0.  At that
 point (very rarely reached) the recurrence is on (just) 5*j+1 again, and
 that's certain to find an empty slot eventually (since it generates every int
 in range(2**i), and we make sure there's always at least one empty slot).
 
 Selecting a good value for PERTURB_SHIFT is a balancing act.  You want it
 small so that the high bits of the hash code continue to affect the probe
 sequence across iterations; but you want it large so that in really bad cases
 the high-order hash bits have an effect on early iterations.  5 was "the
 best" in minimizing total collisions across experiments Tim Peters ran (on
 both normal and pathological cases), but 4 and 6 weren't significantly worse.
 
 Historical:  Reimer Behrends contributed the idea of using a polynomial-based
 approach, using repeated multiplication by x in GF(2**n) where an irreducible
 polynomial for each table size was chosen such that x was a primitive root.
 Christian Tismer later extended that to use division by x instead, as an
 efficient way to get the high bits of the hash code into play.  This scheme
 also gave excellent collision statistics, but was more expensive:  two
 if-tests were required inside the loop; computing "the next" index took about
 the same number of operations but without as much potential parallelism
 (e.g., computing 5*j can go on at the same time as computing 1+perturb in the
  above, and then shifting perturb can be done while the table index is being
  masked); and the dictobject struct required a member to hold the table's
 polynomial.  In Tim's experiments the current scheme ran faster, produced
 equally good collision statistics, needed less code & used less memory.
 */

static String dummy_key = String(128, "###DUMMYROCKETDICTKEY%d###", &dummy_key);

Dictionary::Dictionary() 
{ 
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS,"Dictionary::New Dict"); )
	ResetToMinimumSize();  
}

Dictionary::Dictionary(const Dictionary &dict) 
{
	ResetToMinimumSize();
	Copy(dict);
}

Dictionary::~Dictionary() 
{
	Clear();
}

/*
 The basic lookup function used by all operations.
 This is based on Algorithm D from Knuth Vol. 3, Sec. 6.4.
 Open addressing is preferred over chaining since the link overhead for
 chaining would be substantial (100% with typical malloc overhead).
 
 The initial probe index is computed as hash mod the table size. Subsequent
 probe indices are computed as explained earlier.
 
 All arithmetic on hash should ignore overflow.
 
 (The details in this version are due to Tim Peters, building on many past
  contributions by Reimer Behrends, Jyrki Alakuijala, Vladimir Marangozov and
  Christian Tismer).
 
 This function must never return NULL; failures are indicated by returning
 a dictentry* for which the me_value field is NULL.  Exceptions are never
 reported by this function, and outstanding exceptions are maintained.
 */

/*
 * Hacked up version of lookdict which can assume keys are always strings;
 * this assumption allows testing for errors during PyObject_Compare() to
 * be dropped; string-string comparisons never raise exceptions.  This also
 * means we don't need to go through PyObject_Compare(); we can always use
 * _PyString_Eq directly.
 *
 * This is valuable because the general-case error handling in lookdict() is
 * expensive, and dicts with pure-string keys are very common.
 */
Dictionary::DictionaryEntry* Dictionary::Retrieve(const String& key, Hash hash) const
{
	register Hash i = 0;
	register unsigned int perturb;
	register DictionaryEntry *freeslot;
	register unsigned int mask = this->mask;
	DictionaryEntry *ep0 = table;
	register DictionaryEntry *ep;
	
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::Retreive %s", key); )
		/* Make sure this function doesn't have to handle non-string keys,
		including subclasses of str; e.g., one reason to subclass
		strings is to override __eq__, and for speed we don't cater to
		that here. */
		
	i = hash & mask;
	ep = &ep0[i];
	if (ep->key.Empty() || ep->key == key)
		return ep;
	if (ep->key == dummy_key)
		freeslot = ep;
	else {
		if (ep->hash == hash && ep->key == key) {
			return ep;
		}
		freeslot = NULL;
	}
	
	/* In the loop, me_key == dummy_key is by far (factor of 100s) the
		least likely outcome, so test for that last. */
	for (perturb = hash; ; perturb >>= PERTURB_SHIFT) {
		i = (i << 2) + i + perturb + 1;
		ep = &ep0[i & mask];
		if (ep->key.Empty())    
			return freeslot == NULL ? ep : freeslot;
		/*if (ep->me_key == key
		    || (ep->me_hash == hash
		        && ep->me_key != dummy_key
				&& _PyString_Eq(ep->me_key, key)))*/
		if (ep->key == key)
			return ep;
		if (ep->key == dummy_key && freeslot == NULL)
			freeslot = ep;
	}
}

/*
 Internal routine to insert a new item into the table.
 Used both by the internal resize routine and by the public insert routine.
 */
void Dictionary::Insert(const String& key, Hash hash, const Variant& value)
{	
	register DictionaryEntry *ep;	
	
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::Insert %s", key); );
	ep = Retrieve(key, hash);
	if (ep->value.GetType() !=Variant::NONE) 
	{
		ep->value = value;
	} 
	else 
	{
		if (ep->key.Empty())
		{
			num_full++;
		}
		else if ( ep->key != dummy_key )
		{
			//delete ep->key;
		}
		
		ep->key = key;		
		ep->hash = hash;
		ep->value = value;
		num_used++;
	}  
}

/*
 Restructure the table by allocating a new table and reinserting all
 items again.  When entries have been deleted, the new table may
 actually be smaller than the old one.
 */
bool Dictionary::Reserve(int minused)
{
	int newsize;
	DictionaryEntry *oldtable, *newtable, *ep;
	int i = 0;
	bool is_oldtable_malloced;
	DictionaryEntry small_copy[DICTIONARY_MINSIZE];
	
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::Reserve %d", minused); )
	ROCKET_ASSERT(minused >= 0);
	
	/* Find the smallest table size > minused. */
	for (newsize = DICTIONARY_MINSIZE;
	     newsize <= minused && newsize > 0;
	     newsize <<= 1);
	
	ROCKET_ASSERT(newsize > 0);
	
	if (newsize <= 0) {		
		return false;
	}
	
	// If its the same size, ignore the request
	if ((unsigned)newsize == mask + 1)
		return true;
	
	/* Get space for a new table. */
	oldtable = table;
	ROCKET_ASSERT(oldtable != NULL);
	is_oldtable_malloced = oldtable != small_table;
	
	if (newsize == DICTIONARY_MINSIZE) {
		/* A large table is shrinking, or we can't get any smaller. */
		newtable = small_table;
		if (newtable == oldtable) {
			if (num_full == num_used) {
				/* No dummies, so no point doing anything. */
				return true;
			}
			/* We're not going to resize it, but rebuild the
			table anyway to purge old dummy_key entries.
			Subtle:  This is *necessary* if fill==size,
			as lookdict needs at least one virgin slot to
			terminate failing searches.  If fill < size, it's
			merely desirable, as dummies slow searches. */
			ROCKET_ASSERT(num_full > num_used);
			memcpy(small_copy, oldtable, sizeof(small_copy));
			oldtable = small_copy;
		}
	}	else {
		newtable = new DictionaryEntry[newsize];
		
		ROCKET_ASSERT(newtable);
		
		if (newtable == NULL) {
			return false;
		}
	}
	
	/* Make the dict empty, using the new table. */
	ROCKET_ASSERT(newtable != oldtable);
	table = newtable;
	mask = newsize - 1;
	//memset(newtable, 0, sizeof(DictionaryEntry) * newsize);
	num_used = 0;
	i = num_full;
	num_full = 0;
	
	/* Copy the data over; this is refcount-neutral for active entries;
	   dummy_key entries aren't copied over, of course */
	for (ep = oldtable; i > 0; ep++) {
		if (ep->value.GetType() != Variant::NONE) {	/* active entry */
			--i;
			Insert(ep->key, ep->hash, ep->value);
			
			//delete[] ep->key;
		}
		else if (!ep->key.Empty()) {	/* dummy_key entry */
			--i;
			ROCKET_ASSERT(ep->key == dummy_key);
		}
/* else key == value == NULL:  nothing to do */
	}

	if (is_oldtable_malloced)
		delete[] oldtable;
	return true;
}

Variant* Dictionary::Get(const String& key) const
{
	Hash hash;
	
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::Get %s", key); );
	hash = StringUtilities::FNVHash( key.CString() );
	
	DictionaryEntry* result = Retrieve(key, hash);
	if (!result || result->key.Empty() || result->key == dummy_key)
	{
		return NULL;
	}

	return &result->value;
}

Variant* Dictionary::operator[](const String& key) const
{
	return Get(key);
}

/* CAUTION: PyDict_SetItem() must guarantee that it won't resize the
* dictionary if it is merely replacing the value for an existing key.
* This is means that it's safe to loop over a dictionary with
* PyDict_Next() and occasionally replace a value -- but you can't
* insert new keys or remove them.
*/
void Dictionary::Set(const String& key, const Variant &value)
{
	if (key.Empty())
	{
		Log::Message(Log::LT_WARNING, "Unable to set value on dictionary, empty key specified.");
		return;
	}

	register Hash hash;
	register unsigned int n_used;  
	
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::Set %s", key); );
	hash = StringUtilities::FNVHash( key.CString() );
	
	ROCKET_ASSERT(num_full <= mask);  /* at least one empty slot */
	n_used = num_used;  
	
	Insert( key, hash, value );  
	
	/* If we added a key, we can safely resize.  Otherwise skip this!
		* If fill >= 2/3 size, adjust size.  Normally, this doubles the
		* size, but it's also possible for the dict to shrink (if ma_fill is
															   * much larger than ma_used, meaning a lot of dict keys have been
															   * deleted).
		*/
	if ((num_used > n_used) && (num_full * 3) >= (mask + 1) * 2) 
	{
		if (!Reserve(num_used * 2)) 
		{
			Log::Message(Log::LT_ALWAYS, "Dictionary::Error resizing dictionary after insert");
		}
	}
}

bool Dictionary::Remove(const String& key)
{
	register Hash hash;
	register DictionaryEntry *ep;  
	
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::Remove %s", key) );
	hash = StringUtilities::FNVHash( key.CString() );
	
	ep = Retrieve(key, hash);
	
	if (ep->value.GetType() == Variant::NONE) 
	{
		return false;
	}
		
	ep->key = dummy_key;
	ep->value.Clear();
	num_used--;	
	
	return true;
}

void Dictionary::Clear()
{	
	DictionaryEntry *ep;
	bool table_is_malloced;
	int n_full;
	int i, n;
	
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::Clear") );
	n = mask + 1;
	i = 0;
	
	table_is_malloced = table != small_table;
	n_full = num_full;
	
	// Clear things up
	for (ep = table; n_full > 0; ++ep) {
		
		ROCKET_ASSERT(i < n);
		++i;

		if (!ep->key.Empty()) {
			--n_full;
			ep->key.Clear();		
			ep->value.Clear();
		} else {
			ROCKET_ASSERT(ep->value.GetType() == Variant::NONE);
		}
	}
	(void)n;
	if (table_is_malloced)
		delete [] table;

	ResetToMinimumSize();
}

bool Dictionary::IsEmpty() const
{ 
	return num_used == 0; 
}

int Dictionary::Size() const
{ 
	return num_used; 
}

// Merges another dictionary into this one. Any existing values stored against similar keys will be updated.
void Dictionary::Merge(const Dictionary& dict)
{
	int index = 0;
	String key;
	Variant* value;

	while (dict.Iterate(index, key, value))
	{
		Set(key, *value);
	}
}


/* CAUTION:  In general, it isn't safe to use PyDict_Next in a loop that
* mutates the dict.  One exception:  it is safe if the loop merely changes
* the values associated with the keys (but doesn't insert new keys or
									   * delete keys), via PyDict_SetItem().
*/
bool Dictionary::Iterate(int &pos, String& key, Variant* &value) const
{
	register unsigned int i;

	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::Next %d", pos); )
		i = pos;
	while (i <= mask && table[i].value.GetType() == Variant::NONE)
		i++;
	pos = i+1;
	if (i > mask)
		return false;

	key = table[i].key;

	value = &table[i].value;

	return true;
}

void Dictionary::operator=(const Dictionary &dict) {
	Copy(dict);
}

void Dictionary::Copy(const Dictionary &dict) {
	register unsigned int i;
	
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::Copy Dict (Size: %d)", dict.num_used); )

	// Clear our current state
	Clear();

	// Resize our table
	Reserve(dict.mask);  

	// Copy elements across
	for ( i = 0; i < dict.mask + 1; i++ )
	{
		table[i].hash = dict.table[i].hash;		
		table[i].key = dict.table[i].key;			
		table[i].value = dict.table[i].value;		
	}

	// Set properties
	num_used = dict.num_used;
	num_full = dict.num_full;
	mask = dict.mask;
}  

void Dictionary::ResetToMinimumSize() 
{
	DICTIONARY_DEBUG_CODE( Log::Message(LC_CORE, Log::LT_ALWAYS, "Dictionary::ResetToMinimumSize"); )
	// Reset to small table
	for ( size_t i = 0; i < DICTIONARY_MINSIZE; i++ )
	{		
		small_table[i].hash = 0;
		small_table[i].key.Clear();
		small_table[i].value.Clear();
	}
	num_used = 0;
	num_full = 0;
	table = small_table;
	mask = DICTIONARY_MINSIZE - 1; 
}

}
}
