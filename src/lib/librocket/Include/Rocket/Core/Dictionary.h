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

#ifndef ROCKETCOREDICTIONARY_H
#define ROCKETCOREDICTIONARY_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Variant.h>

namespace Rocket {
namespace Core {

/**
	A dictionay is a container of variants.
	It uses a hash table to maintain a string key to variant mapping.

	@author Lloyd Weehuizen
 */

class ROCKETCORE_API Dictionary
{
public:
	Dictionary();
	Dictionary(const Dictionary &dict);
	~Dictionary();

	/// Store an item in the dictionary
	void Set(const String& key, const Variant &value);
  
	/// Templated set eases setting of values
	template <typename T>
	inline void Set(const String& key, const T& value);
	
	/// Get an item from the dictionary
	Variant* Get(const String& key) const;
	Variant* operator[](const String& key) const;
	
	/// Get a value from the dictionary, if it doesn't exist
	/// use the supplied default value
	template <typename T>
	inline T Get(const String& key, const T& default_val) const;
	
	/// Get a value from the dictionary, returns if the
	/// value was found or not.
	template <typename T>
	inline bool GetInto(const String& key, T& value) const;

	/// Remove an item from the dictionary
	bool Remove(const String& key);

	/// Iterate through a dictionary
	bool Iterate(int &pos, String& key, Variant* &value) const;
	template <typename T>
	bool Iterate(int &pos, String& key, T& value) const;

	/// Reserve the specified number of entries in the dictionary
	bool Reserve(int size);

	/// Empty the dictionary
	void Clear();

	/// Is the dictionary empty?
	bool IsEmpty() const;

	/// Items in the dict
	int Size() const;

	/// Merges another dictionary into this one. Any existing values stored against similar keys will be updated.
	void Merge(const Dictionary& dict);

	// Copy
	void operator=(const Dictionary &dict);

private:
	unsigned int num_full;  // Active + # Dummy
	unsigned int num_used;  // Active

	/* DICTIONARY_MINSIZE is the minimum size of a dictionary.  This many slots are
	 * allocated directly in the dict object (in the small_table member).
	 * It must be a power of 2, and at least 4.  8 allows dicts with no more
	 * than 5 active entries to live in small_table (and so avoid an
	 * additional malloc); instrumentation suggested this suffices for the
	 * majority of dicts (consisting mostly of usually-small instance dicts and
	 * usually-small dicts created to pass keyword arguments).
	 */
	static const int DICTIONARY_MINSIZE = 8;

	// Individual entry in a dictionary
	struct DictionaryEntry
	{
		DictionaryEntry() : hash(0) {}
		Hash hash;		// Cached hash of key
		String key;		// key in plain text
		Variant value;	// Value for this entry
	};
  
  /* The table contains mask + 1 slots, and that's a power of 2.
	 * We store the mask instead of the size because the mask is more
	 * frequently needed.
	 */
	unsigned int mask;

	// Small dictionaries just use this, saves mallocs for small tables
	DictionaryEntry small_table[DICTIONARY_MINSIZE];

	/// Pointer to table in use, may be malloc'd or may point to smallTable
	DictionaryEntry* table;

	/// Insert an item
	void Insert(const String& key, Hash hash, const Variant& value);

	/// Retrieve an item
	DictionaryEntry* Retrieve(const String& key, Hash hash) const;

	/// Reset to small dictionary
	void ResetToMinimumSize();  

	// Copy another dict
	void Copy(const Dictionary &dict);
};

#include <Rocket/Core/Dictionary.inl>

}
}

#endif
