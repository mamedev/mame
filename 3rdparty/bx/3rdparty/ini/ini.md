ini.h
=====

Library: [ini.h](../ini.h)


Examples
========

Loading an ini file and retrieving values
-----------------------------------------

```cpp
#define INI_IMPLEMENTATION
#include "ini.h"

#include <stdio.h>
#include <stdlib.h>

int main()
	{
	FILE* fp = fopen( "test.ini", "r" );
	fseek( fp, 0, SEEK_END );
	int size = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	char* data = (char*) malloc( size + 1 );
	fread( data, 1, size, fp );
	data[ size ] = '\0';
	fclose( fp );

	ini_t* ini = ini_load( data );
	free( data );
	int second_index = ini_find_property( ini, INI_GLOBAL_SECTION, "SecondSetting" );
	char const* second = ini_property_value( ini, INI_GLOBAL_SECTION, second_index );
	printf( "%s=%s\n", "SecondSetting", second );
	int section = ini_find_section( ini, "MySection" );
	int third_index = ini_find_property( ini, section, "ThirdSetting" );
	char const* third = ini_property_value( ini, section, third_index );
	printf( "%s=%s\n", "ThirdSetting", third );
	ini_destroy( ini );

	return 0;
	}
```

Creating a new ini file
-----------------------

```cpp
#define INI_IMPLEMENTATION
#include "ini.h"

#include <stdio.h>
#include <stdlib.h>

int main()
	{       
	ini_t* ini = ini_create();
	ini_property_add( ini, INI_GLOBAL_SECTION, "FirstSetting", "Test" );
	ini_property_add( ini, INI_GLOBAL_SECTION, "SecondSetting", "2" );
	int section = ini_section_add( ini, "MySection" );
	ini_property_add( ini, section, "ThirdSetting", "Three" );

	int size = ini_save( ini, NULL, 0 ); // Find the size needed
	char* data = (char*) malloc( size );
	size = ini_save( ini, data, size ); // Actually save the file
	ini_destroy( ini );

	FILE* fp = fopen( "test.ini", "w" );
	fwrite( data, 1, size, fp );
	fclose( fp );
	free( data );

	return 0;
	}
```


API Documentation
=================

ini.h is a small library for reading classic .ini files. It is a single-header library, and does not need any .lib files 
or other binaries, or any build scripts. To use it, you just include ini.h to get the API declarations. To get the 
definitions, you must include ini.h from *one* single C or C++ file, and #define the symbol `INI_IMPLEMENTATION` before 
you do. 


Customization
-------------
There are a few different things in ini.h which are configurable by #defines. The customizations only affect the 
implementation, so will only need to be defined in the file where you have the #define INI_IMPLEMENTATION.

Note that if all customizations are utilized, ini.h will include no external files whatsoever, which might be useful
if you need full control over what code is being built.


### Custom memory allocators

To store the internal data structures, ini.h needs to do dynamic allocation by calling `malloc`. Programs might want to 
keep track of allocations done, or use custom defined pools to allocate memory from. ini.h allows for specifying custom 
memory allocation functions for `malloc` and `free`.
This is done with the following code:

    #define INI_IMPLEMENTATION
    #define INI_MALLOC( ctx, size ) ( my_custom_malloc( ctx, size ) )
    #define INI_FREE( ctx, ptr ) ( my_custom_free( ctx, ptr ) )
    #include "ini.h"

where `my_custom_malloc` and `my_custom_free` are your own memory allocation/deallocation functions. The `ctx` parameter
is an optional parameter of type `void*`. When `ini_create` or `ini_load` is called, you can pass in a `memctx` 
parameter, which can be a pointer to anything you like, and which will be passed through as the `ctx` parameter to every 
`INI_MALLOC`/`INI_FREE` call. For example, if you are doing memory tracking, you can pass a pointer to your tracking 
data as `memctx`, and in your custom allocation/deallocation function, you can cast the `ctx` param back to the 
right type, and access the tracking data.

If no custom allocator is defined, ini.h will default to `malloc` and `free` from the C runtime library.


### Custom C runtime function

The library makes use of three additional functions from the C runtime library, and for full flexibility, it allows you 
to substitute them for your own. Here's an example:

    #define INI_IMPLEMENTATION
    #define INI_MEMCPY( dst, src, cnt ) ( my_memcpy_func( dst, src, cnt ) )
    #define INI_STRLEN( s ) ( my_strlen_func( s ) )
    #define INI_STRICMP( s1, s2 ) ( my_stricmp_func( s1, s2 ) )
    #include "ini.h"

If no custom function is defined, ini.h will default to the C runtime library equivalent.


ini_create
----------
    
    ini_t* ini_create( void* memctx )

Instantiates a new, empty ini structure, which can be manipulated with other API calls, to fill it with data. To save it
out to an ini-file string, use `ini_save`. When no longer needed, it can be destroyed by calling `ini_destroy`.
`memctx` is a pointer to user defined data which will be passed through to the custom INI_MALLOC/INI_FREE calls. It can 
be NULL if no user defined data is needed.


ini_load
--------

    ini_t* ini_load( char const* data, void* memctx )

Parse the zero-terminated string `data` containing an ini-file, and create a new ini_t instance containing the data. 
The instance can be manipulated with other API calls to enumerate sections/properties and retrieve values. When no 
longer needed, it can be destroyed by calling `ini_destroy`. `memctx` is a pointer to user defined data which will be 
passed through to the custom INI_MALLOC/INI_FREE calls. It can be NULL if no user defined data is needed.


ini_save
--------
    
    int ini_save( ini_t const* ini, char* data, int size )

Saves an ini structure as a zero-terminated ini-file string, into the specified buffer. Returns the number of bytes 
written, including the zero terminator. If `data` is NULL, nothing is written, but `ini_save` still returns the number
of bytes it would have written. If the size of `data`, as specified in the `size` parameter, is smaller than that 
required, only part of the ini-file string will be written. `ini_save` still returns the number of bytes it would have
written had the buffer been large enough.


ini_destroy
-----------

    void ini_destroy( ini_t* ini )

Destroy an `ini_t` instance created by calling `ini_load` or `ini_create`, releasing the memory allocated by it. No
further API calls are valid on an `ini_t` instance after calling `ini_destroy` on it.


ini_section_count
-----------------

    int ini_section_count( ini_t const* ini )

Returns the number of sections in an ini file. There's at least one section in an ini file (the global section), but 
there can be many more, each specified in the file by the section name wrapped in square brackets [ ].


ini_section_name
----------------

    char const* ini_section_name( ini_t const* ini, int section )

Returns the name of the section with the specified index. `section` must be non-negative and less than the value 
returned by `ini_section_count`, or `ini_section_name` will return NULL. The defined constant `INI_GLOBAL_SECTION` can
be used to indicate the global section.


ini_property_count
------------------

    int ini_property_count( ini_t const* ini, int section )

Returns the number of properties belonging to the section with the specified index. `section` must be non-negative and 
less than the value returned by `ini_section_count`, or `ini_section_name` will return 0. The defined constant 
`INI_GLOBAL_SECTION` can be used to indicate the global section. Properties are declared in the ini-file on he format
`name=value`.


ini_property_name
-----------------

    char const* ini_property_name( ini_t const* ini, int section, int property )

Returns the name of the property with the specified index `property` in the section with the specified index `section`.
`section` must be non-negative and less than the value returned by `ini_section_count`, and `property` must be 
non-negative and less than the value returned by `ini_property_count`, or `ini_property_name` will return NULL. The 
defined constant `INI_GLOBAL_SECTION` can be used to indicate the global section.


ini_property_value
------------------

    char const* ini_property_value( ini_t const* ini, int section, int property )

Returns the value of the property with the specified index `property` in the section with the specified index `section`.
`section` must be non-negative and less than the value returned by `ini_section_count`, and `property` must be 
non-negative and less than the value returned by `ini_property_count`, or `ini_property_value` will return NULL. The 
defined constant `INI_GLOBAL_SECTION` can be used to indicate the global section.


ini_find_section
----------------

    int ini_find_section( ini_t const* ini, char const* name, int name_length )

Finds the section with the specified name, and returns its index. `name_length` specifies the number of characters in
`name`, which does not have to be zero-terminated. If `name_length` is zero, the length is determined automatically, but
in this case `name` has to be zero-terminated. If no section with the specified name could be found, the value
`INI_NOT_FOUND` is returned.


ini_find_property
-----------------

    int ini_find_property( ini_t const* ini, int section, char const* name, int name_length )

Finds the property with the specified name, within the section with the specified index, and returns the index of the 
property. `name_length` specifies the number of characters in `name`, which does not have to be zero-terminated. If 
`name_length` is zero, the length is determined automatically, but in this case `name` has to be zero-terminated. If no 
property with the specified name could be found within the specified section, the value `INI_NOT_FOUND` is  returned.
`section` must be non-negative and less than the value returned by `ini_section_count`, or `ini_find_property` will 
return `INI_NOT_FOUND`. The defined constant `INI_GLOBAL_SECTION` can be used to indicate the global section.


ini_section_add
---------------

    int ini_section_add( ini_t* ini, char const* name, int length )

Adds a section with the specified name, and returns the index it was added at. There is no check done to see if a 
section with the specified name already exists - multiple sections of the same name are allowed. `length` specifies the 
number of characters in `name`, which does not have to be zero-terminated. If `length` is zero, the length is determined 
automatically, but in this case `name` has to be zero-terminated.


ini_property_add
----------------
    
    void ini_property_add( ini_t* ini, int section, char const* name, int name_length, char const* value, int value_length )

Adds a property with the specified name and value to the specified section, and returns the index it was added at. There 
is no check done to see if a property with the specified name already exists - multiple properties of the same name are 
allowed. `name_length` and `value_length` specifies the number of characters in `name` and `value`, which does not have 
to be zero-terminated. If `name_length` or `value_length` is zero, the length is determined automatically, but in this 
case `name`/`value` has to be zero-terminated. `section` must be non-negative and less than the value returned by
`ini_section_count`, or the property will not be added. The defined constant `INI_GLOBAL_SECTION` can be used to 
indicate the global section.


ini_section_remove
------------------

    void ini_section_remove( ini_t* ini, int section )

Removes the section with the specified index, and all properties within it. `section` must be non-negative and less than 
the value returned by `ini_section_count`. The defined constant `INI_GLOBAL_SECTION` can be used to indicate the global 
section. Note that removing a section will shuffle section indices, so that section indices you may have stored will no 
longer indicate the same section as it did before the remove. Use the find functions to update your indices.


ini_property_remove
-------------------

    void ini_property_remove( ini_t* ini, int section, int property )

Removes the property with the specified index from the specified section. `section` must be non-negative and less than 
the value returned by `ini_section_count`, and `property` must be non-negative and less than the value returned by 
`ini_property_count`. The defined constant `INI_GLOBAL_SECTION` can be used to indicate the global section. Note that 
removing a property will shuffle property indices within the specified section, so that property indices you may have 
stored will no longer indicate the same property as it did before the remove. Use the find functions to update your 
indices.


ini_section_name_set
--------------------

    void ini_section_name_set( ini_t* ini, int section, char const* name, int length )

Change the name of the section with the specified index. `section` must be non-negative and less than the value returned 
by `ini_section_count`. The defined constant `INI_GLOBAL_SECTION` can be used to indicate the global section. `length` 
specifies the number of characters in `name`, which does not have to be zero-terminated. If `length` is zero, the length 
is determined automatically, but in this case `name` has to be zero-terminated.


ini_property_name_set
---------------------

    void ini_property_name_set( ini_t* ini, int section, int property, char const* name, int length )

Change the name of the property with the specified index in the specified section. `section` must be non-negative and 
less than the value returned by `ini_section_count`, and `property` must be non-negative and less than the value 
returned by `ini_property_count`. The defined constant `INI_GLOBAL_SECTION` can be used to indicate the global section.
`length` specifies the number of characters in `name`, which does not have to be zero-terminated. If `length` is zero, 
the length is determined automatically, but in this case `name` has to be zero-terminated.


ini_property_value_set
----------------------

    void ini_property_value_set( ini_t* ini, int section, int property, char const* value, int length  )

Change the value of the property with the specified index in the specified section. `section` must be non-negative and 
less than the value returned by `ini_section_count`, and `property` must be non-negative and less than the value 
returned by `ini_property_count`. The defined constant `INI_GLOBAL_SECTION` can be used to indicate the global section.
`length` specifies the number of characters in `value`, which does not have to be zero-terminated. If `length` is zero, 
the length is determined automatically, but in this case `value` has to be zero-terminated.
