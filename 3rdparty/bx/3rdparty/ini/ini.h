/*
------------------------------------------------------------------------------
          Licensing information can be found at the end of the file.
------------------------------------------------------------------------------

ini.h - v1.1 - Simple ini-file reader for C/C++.

Do this:
    #define INI_IMPLEMENTATION
before you include this file in *one* C/C++ file to create the implementation.
*/

#ifndef ini_h
#define ini_h

#define INI_GLOBAL_SECTION ( 0 )
#define INI_NOT_FOUND ( -1 )

typedef struct ini_t ini_t;

ini_t* ini_create( void* memctx );
ini_t* ini_load( char const* data, void* memctx );

int ini_save( ini_t const* ini, char* data, int size );
void ini_destroy( ini_t* ini );

int ini_section_count( ini_t const* ini );
char const* ini_section_name( ini_t const* ini, int section );

int ini_property_count( ini_t const* ini, int section );
char const* ini_property_name( ini_t const* ini, int section, int property );
char const* ini_property_value( ini_t const* ini, int section, int property );

int ini_find_section( ini_t const* ini, char const* name, int name_length );
int ini_find_property( ini_t const* ini, int section, char const* name, int name_length );

int ini_section_add( ini_t* ini, char const* name, int length );
void ini_property_add( ini_t* ini, int section, char const* name, int name_length, char const* value, int value_length );
void ini_section_remove( ini_t* ini, int section );
void ini_property_remove( ini_t* ini, int section, int property );

void ini_section_name_set( ini_t* ini, int section, char const* name, int length );
void ini_property_name_set( ini_t* ini, int section, int property, char const* name, int length );
void ini_property_value_set( ini_t* ini, int section, int property, char const* value, int length  );

#endif /* ini_h */


/**

Examples
========

Loading an ini file and retrieving values
-----------------------------------------

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


Creating a new ini file
-----------------------

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
    #define INI_STRNICMP( s1, s2, cnt ) ( my_strnicmp_func( s1, s2, cnt ) )
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

**/


/*
----------------------
    IMPLEMENTATION
----------------------
*/

#ifdef INI_IMPLEMENTATION
#undef INI_IMPLEMENTATION

#define INITIAL_CAPACITY ( 256 )

#ifndef _CRT_NONSTDC_NO_DEPRECATE
    #define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <stddef.h>

#ifndef INI_MALLOC
    #include <stdlib.h>
    #define INI_MALLOC( ctx, size ) ( malloc( size ) )
    #define INI_FREE( ctx, ptr ) ( free( ptr ) )
#endif

#ifndef INI_MEMCPY
    #include <string.h>
    #define INI_MEMCPY( dst, src, cnt ) ( memcpy( dst, src, cnt ) )
#endif

#ifndef INI_STRLEN
    #include <string.h>
    #define INI_STRLEN( s ) ( strlen( s ) )
#endif

#ifndef INI_STRNICMP
    #ifdef _WIN32
        #include <string.h>
        #define INI_STRNICMP( s1, s2, cnt ) ( strnicmp( s1, s2, cnt ) )
    #else
        #include <string.h>
        #define INI_STRNICMP( s1, s2, cnt ) ( strncasecmp( s1, s2, cnt ) )
    #endif
#endif


struct ini_internal_section_t
    {
    char name[ 32 ];
    char* name_large;
    };


struct ini_internal_property_t
    {
    int section;
    char name[ 32 ];
    char* name_large;
    char value[ 64 ];
    char* value_large;
    };


struct ini_t
    {
    struct ini_internal_section_t* sections;
    int section_capacity;
    int section_count;

    struct ini_internal_property_t* properties;
    int property_capacity;
    int property_count;

    void* memctx;
    };


static int ini_internal_property_index( ini_t const* ini, int section, int property )
    {
    int i;
    int p;

    if( ini && section >= 0 && section < ini->section_count )
        {
        p = 0;
        for( i = 0; i < ini->property_count; ++i )
            {
            if( ini->properties[ i ].section == section )
                {
                if( p == property ) return i;
                ++p;
                }
            }
        }

    return INI_NOT_FOUND;
    }


ini_t* ini_create( void* memctx )
    {
    ini_t* ini;

    ini = (ini_t*) INI_MALLOC( memctx, sizeof( ini_t ) );
    ini->memctx = memctx;
    ini->sections = (struct ini_internal_section_t*) INI_MALLOC( ini->memctx, INITIAL_CAPACITY * sizeof( ini->sections[ 0 ] ) );
    ini->section_capacity = INITIAL_CAPACITY;
    ini->section_count = 1; /* global section */
    ini->sections[ 0 ].name[ 0 ] = '\0';
    ini->sections[ 0 ].name_large = 0;
    ini->properties = (struct ini_internal_property_t*) INI_MALLOC( ini->memctx, INITIAL_CAPACITY * sizeof( ini->properties[ 0 ] ) );
    ini->property_capacity = INITIAL_CAPACITY;
    ini->property_count = 0;
    return ini;
    }


ini_t* ini_load( char const* data, unsigned int len, void* memctx )
    {
    ini_t* ini;
    char const* ptr;
    int s;
    char const* start;
    char const* start2;
    int l;
    char const* end;

    ini = ini_create( memctx );

    ptr = data;
    end = ptr + len;
    if( ptr )
        {
        s = 0;
        while( ptr < end && *ptr )
            {
            /* trim leading whitespace */
            while( ptr < end && *ptr && *ptr <=' ' )
                ++ptr;

            /* done? */
            if( !*ptr ) break;

            /* comment */
            else if( *ptr == ';' )
                {
                while( *ptr && *ptr !='\n' )
                    ++ptr;
                }
            /* section */
            else if( *ptr == '[' )
                {
                ++ptr;
                start = ptr;
                while( *ptr && *ptr !=']' && *ptr != '\n' )
                    ++ptr;

                if( *ptr == ']' )
                    {
                    s = ini_section_add( ini, start, (int)( ptr - start) );
                    ++ptr;
                    }
                }
            /* property */
            else
                {
                start = ptr;
                while( ptr < end && *ptr && *ptr !='=' && *ptr != '\n' )
                    ++ptr;

                if( *ptr == '=' )
                    {
                    l = (int)( ptr - start);
                    ++ptr;
                    while( ptr < end && *ptr && *ptr <= ' ' && *ptr != '\n' )
                        ptr++;
                    start2 = ptr;
                    while( ptr < end && *ptr && *ptr != '\n' )
                        ++ptr;
                    while( *(--ptr) <= ' ' )
                        (void)ptr;
                    ptr++;
                    ini_property_add( ini, s, start, l, start2, (int)( ptr - start2) );
                    }
                }
            }
        }

    return ini;
    }


int ini_save( ini_t const* ini, char* data, int size )
    {
    int s;
    int p;
    int i;
    int l;
    char* n;
    int pos;

    if( ini )
        {
        pos = 0;
        for( s = 0; s < ini->section_count; ++s )
            {
            n = ini->sections[ s ].name_large ? ini->sections[ s ].name_large : ini->sections[ s ].name;
            l = (int) INI_STRLEN( n );
            if( l > 0 )
                {
                if( data && pos < size ) data[ pos ] = '[';
                ++pos;
                for( i = 0; i < l; ++i )
                    {
                    if( data && pos < size ) data[ pos ] = n[ i ];
                    ++pos;
                    }
                if( data && pos < size ) data[ pos ] = ']';
                ++pos;
                if( data && pos < size ) data[ pos ] = '\n';
                ++pos;
                }

            for( p = 0; p < ini->property_count; ++p )
                {
                if( ini->properties[ p ].section == s )
                    {
                    n = ini->properties[ p ].name_large ? ini->properties[ p ].name_large : ini->properties[ p ].name;
                    l = (int) INI_STRLEN( n );
                    for( i = 0; i < l; ++i )
                        {
                        if( data && pos < size ) data[ pos ] = n[ i ];
                        ++pos;
                        }
                    if( data && pos < size ) data[ pos ] = '=';
                    ++pos;
                    n = ini->properties[ p ].value_large ? ini->properties[ p ].value_large : ini->properties[ p ].value;
                    l = (int) INI_STRLEN( n );
                    for( i = 0; i < l; ++i )
                        {
                        if( data && pos < size ) data[ pos ] = n[ i ];
                        ++pos;
                        }
                    if( data && pos < size ) data[ pos ] = '\n';
                    ++pos;
                    }
                }

            if( pos > 0 )
                {
                if( data && pos < size ) data[ pos ] = '\n';
                ++pos;
                }
            }

        if( data && pos < size ) data[ pos ] = '\0';
        ++pos;

        return pos;
        }

    return 0;
    }


void ini_destroy( ini_t* ini )
    {
    int i;

    if( ini )
        {
        for( i = 0; i < ini->property_count; ++i )
            {
            if( ini->properties[ i ].value_large ) INI_FREE( ini->memctx, ini->properties[ i ].value_large );
            if( ini->properties[ i ].name_large ) INI_FREE( ini->memctx, ini->properties[ i ].name_large );
            }
        for( i = 0; i < ini->section_count; ++i )
            if( ini->sections[ i ].name_large ) INI_FREE( ini->memctx, ini->sections[ i ].name_large );
        INI_FREE( ini->memctx, ini->properties );
        INI_FREE( ini->memctx, ini->sections );
        INI_FREE( ini->memctx, ini );
        }
    }


int ini_section_count( ini_t const* ini )
    {
    if( ini ) return ini->section_count;
    return 0;
    }


char const* ini_section_name( ini_t const* ini, int section )
    {
    if( ini && section >= 0 && section < ini->section_count )
        return ini->sections[ section ].name_large ? ini->sections[ section ].name_large : ini->sections[ section ].name;

    return NULL;
    }


int ini_property_count( ini_t const* ini, int section )
    {
    int i;
    int count;

    if( ini )
        {
        count = 0;
        for( i = 0; i < ini->property_count; ++i )
            {
            if( ini->properties[ i ].section == section ) ++count;
            }
        return count;
        }

    return 0;
    }


char const* ini_property_name( ini_t const* ini, int section, int property )
    {
    int p;

    if( ini && section >= 0 && section < ini->section_count )
        {
        p = ini_internal_property_index( ini, section, property );
        if( p != INI_NOT_FOUND )
            return ini->properties[ p ].name_large ? ini->properties[ p ].name_large : ini->properties[ p ].name;
        }

    return NULL;
    }


char const* ini_property_value( ini_t const* ini, int section, int property )
    {
    int p;

    if( ini && section >= 0 && section < ini->section_count )
        {
        p = ini_internal_property_index( ini, section, property );
        if( p != INI_NOT_FOUND )
            return ini->properties[ p ].value_large ? ini->properties[ p ].value_large : ini->properties[ p ].value;
        }

    return NULL;
    }


int ini_find_section( ini_t const* ini, char const* name, int name_length )
    {
    int i;

    if( ini && name )
        {
        if( name_length <= 0 ) name_length = (int) INI_STRLEN( name );
        for( i = 0; i < ini->section_count; ++i )
            {
            char const* const other =
                ini->sections[ i ].name_large ? ini->sections[ i ].name_large : ini->sections[ i ].name;
            if( (int) INI_STRLEN( other ) == name_length && INI_STRNICMP( name, other, name_length ) == 0 )
                return i;
            }
        }

    return INI_NOT_FOUND;
    }


int ini_find_property( ini_t const* ini, int section, char const* name, int name_length )
    {
    int i;
    int c;

    if( ini && name && section >= 0 && section < ini->section_count)
        {
        if( name_length <= 0 ) name_length = (int) INI_STRLEN( name );
        c = 0;
        for( i = 0; i < ini->property_capacity; ++i )
            {
            if( ini->properties[ i ].section == section )
                {
                char const* const other =
                    ini->properties[ i ].name_large ? ini->properties[ i ].name_large : ini->properties[ i ].name;
                if( (int) INI_STRLEN( other ) == name_length && INI_STRNICMP( name, other, name_length ) == 0 )
                    return c;
                ++c;
                }
            }
        }

    return INI_NOT_FOUND;
    }


int ini_section_add( ini_t* ini, char const* name, int length )
    {
    struct ini_internal_section_t* new_sections;

    if( ini && name )
        {
        if( length <= 0 ) length = (int) INI_STRLEN( name );
        if( ini->section_count >= ini->section_capacity )
            {
            ini->section_capacity *= 2;
            new_sections = (struct ini_internal_section_t*) INI_MALLOC( ini->memctx,
                ini->section_capacity * sizeof( ini->sections[ 0 ] ) );
            INI_MEMCPY( new_sections, ini->sections, ini->section_count * sizeof( ini->sections[ 0 ] ) );
            INI_FREE( ini->memctx, ini->sections );
            ini->sections = new_sections;
            }

        ini->sections[ ini->section_count ].name_large = 0;
        if( length + 1 >= sizeof( ini->sections[ 0 ].name ) )
            {
            ini->sections[ ini->section_count ].name_large = (char*) INI_MALLOC( ini->memctx, (size_t) length + 1 );
            INI_MEMCPY( ini->sections[ ini->section_count ].name_large, name, (size_t) length );
            ini->sections[ ini->section_count ].name_large[ length ] = '\0';
            }
        else
            {
            INI_MEMCPY( ini->sections[ ini->section_count ].name, name, (size_t) length );
            ini->sections[ ini->section_count ].name[ length ] = '\0';
            }

        return ini->section_count++;
        }
    return INI_NOT_FOUND;
    }


void ini_property_add( ini_t* ini, int section, char const* name, int name_length, char const* value, int value_length )
    {
    struct ini_internal_property_t* new_properties;

    if( ini && name && section >= 0 && section < ini->section_count )
        {
        if( name_length <= 0 ) name_length = (int) INI_STRLEN( name );
        if( value_length <= 0 ) value_length = (int) INI_STRLEN( value );

        if( ini->property_count >= ini->property_capacity )
            {

            ini->property_capacity *= 2;
            new_properties = (struct ini_internal_property_t*) INI_MALLOC( ini->memctx,
                ini->property_capacity * sizeof( ini->properties[ 0 ] ) );
            INI_MEMCPY( new_properties, ini->properties, ini->property_count * sizeof( ini->properties[ 0 ] ) );
            INI_FREE( ini->memctx, ini->properties );
            ini->properties = new_properties;
            }

        ini->properties[ ini->property_count ].section = section;
        ini->properties[ ini->property_count ].name_large = 0;
        ini->properties[ ini->property_count ].value_large = 0;

        if( name_length + 1 >= sizeof( ini->properties[ 0 ].name ) )
            {
            ini->properties[ ini->property_count ].name_large = (char*) INI_MALLOC( ini->memctx, (size_t) name_length + 1 );
            INI_MEMCPY( ini->properties[ ini->property_count ].name_large, name, (size_t) name_length );
            ini->properties[ ini->property_count ].name_large[ name_length ] = '\0';
            }
        else
            {
            INI_MEMCPY( ini->properties[ ini->property_count ].name, name, (size_t) name_length );
            ini->properties[ ini->property_count ].name[ name_length ] = '\0';
            }

        if( value_length + 1 >= sizeof( ini->properties[ 0 ].value ) )
            {
            ini->properties[ ini->property_count ].value_large = (char*) INI_MALLOC( ini->memctx, (size_t) value_length + 1 );
            INI_MEMCPY( ini->properties[ ini->property_count ].value_large, value, (size_t) value_length );
            ini->properties[ ini->property_count ].value_large[ value_length ] = '\0';
            }
        else
            {
            INI_MEMCPY( ini->properties[ ini->property_count ].value, value, (size_t) value_length );
            ini->properties[ ini->property_count ].value[ value_length ] = '\0';
            }

        ++ini->property_count;
        }
    }


void ini_section_remove( ini_t* ini, int section )
    {
    int p;

    if( ini && section >= 0 && section < ini->section_count )
        {
        if( ini->sections[ section ].name_large ) INI_FREE( ini->memctx, ini->sections[ section ].name_large );
        for( p = ini->property_count - 1; p >= 0; --p )
            {
            if( ini->properties[ p ].section == section )
                {
                if( ini->properties[ p ].value_large ) INI_FREE( ini->memctx, ini->properties[ p ].value_large );
                if( ini->properties[ p ].name_large ) INI_FREE( ini->memctx, ini->properties[ p ].name_large );
                ini->properties[ p ] = ini->properties[ --ini->property_count ];
                }
            }

        ini->sections[ section ] = ini->sections[ --ini->section_count  ];

        for( p = 0; p < ini->property_count; ++p )
            {
            if( ini->properties[ p ].section == ini->section_count )
                ini->properties[ p ].section = section;
            }
        }
    }


void ini_property_remove( ini_t* ini, int section, int property )
    {
    int p;

    if( ini && section >= 0 && section < ini->section_count )
        {
        p = ini_internal_property_index( ini, section, property );
        if( p != INI_NOT_FOUND )
            {
            if( ini->properties[ p ].value_large ) INI_FREE( ini->memctx, ini->properties[ p ].value_large );
            if( ini->properties[ p ].name_large ) INI_FREE( ini->memctx, ini->properties[ p ].name_large );
            ini->properties[ p ] = ini->properties[ --ini->property_count  ];
            return;
            }
        }
    }


void ini_section_name_set( ini_t* ini, int section, char const* name, int length )
    {
    if( ini && name && section >= 0 && section < ini->section_count )
        {
        if( length <= 0 ) length = (int) INI_STRLEN( name );
        if( ini->sections[ section ].name_large ) INI_FREE( ini->memctx, ini->sections[ section ].name_large );
        ini->sections[ section ].name_large = 0;

        if( length + 1 >= sizeof( ini->sections[ 0 ].name ) )
            {
            ini->sections[ section ].name_large = (char*) INI_MALLOC( ini->memctx, (size_t) length + 1 );
            INI_MEMCPY( ini->sections[ section ].name_large, name, (size_t) length );
            ini->sections[ section ].name_large[ length ] = '\0';
            }
        else
            {
            INI_MEMCPY( ini->sections[ section ].name, name, (size_t) length );
            ini->sections[ section ].name[ length ] = '\0';
            }
        }
    }


void ini_property_name_set( ini_t* ini, int section, int property, char const* name, int length )
    {
    int p;

    if( ini && name && section >= 0 && section < ini->section_count )
        {
        if( length <= 0 ) length = (int) INI_STRLEN( name );
        p = ini_internal_property_index( ini, section, property );
        if( p != INI_NOT_FOUND )
            {
            if( ini->properties[ p ].name_large ) INI_FREE( ini->memctx, ini->properties[ p ].name_large );
            ini->properties[ ini->property_count ].name_large = 0;

            if( length + 1 >= sizeof( ini->properties[ 0 ].name ) )
                {
                ini->properties[ p ].name_large = (char*) INI_MALLOC( ini->memctx, (size_t) length + 1 );
                INI_MEMCPY( ini->properties[ p ].name_large, name, (size_t) length );
                ini->properties[ p ].name_large[ length ] = '\0';
                }
            else
                {
                INI_MEMCPY( ini->properties[ p ].name, name, (size_t) length );
                ini->properties[ p ].name[ length ] = '\0';
                }
            }
        }
    }


void ini_property_value_set( ini_t* ini, int section, int property, char const* value, int length )
    {
    int p;

    if( ini && value && section >= 0 && section < ini->section_count )
        {
        if( length <= 0 ) length = (int) INI_STRLEN( value );
        p = ini_internal_property_index( ini, section, property );
        if( p != INI_NOT_FOUND )
            {
            if( ini->properties[ p ].value_large ) INI_FREE( ini->memctx, ini->properties[ p ].value_large );
            ini->properties[ ini->property_count ].value_large = 0;

            if( length + 1 >= sizeof( ini->properties[ 0 ].value ) )
                {
                ini->properties[ p ].value_large = (char*) INI_MALLOC( ini->memctx, (size_t) length + 1 );
                INI_MEMCPY( ini->properties[ p ].name_large, value, (size_t) length );
                ini->properties[ p ].value_large[ length ] = '\0';
                }
            else
                {
                INI_MEMCPY( ini->properties[ p ].value, value, (size_t) length );
                ini->properties[ p ].name[ length ] = '\0';
                }
            }
        }
    }


#endif /* INI_IMPLEMENTATION */

/*
revision history:
    1.1     customization, added documentation, cleanup
    1.0     first publicly released version
*/

/*
------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2015 Mattias Gustavsson

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

------------------------------------------------------------------------------

ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
*/
