#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "celib/types.h"
#include "celib/memory/types.h"

#include <vector>

/*******************************************************************************
**** Containers
*******************************************************************************/
namespace cetech {

    /***************************************************************************
    **** Generic array
    ***************************************************************************/
    template < typename T > struct Array {

        /***********************************************************************
        **** Constructor
        ***********************************************************************/
        explicit Array(Allocator& a);

        /***********************************************************************
        **** Copy constructor
        ***********************************************************************/
        Array(const Array &other);

        /***********************************************************************
        **** Destructor
        ***********************************************************************/
        ~Array();

        /***********************************************************************
        **** Set operator
        ***********************************************************************/
        Array& operator = (const Array &other);

        /***********************************************************************
        **** Item accessor []
        ***********************************************************************/
        T& operator[] (const uint32_t i);

        /***********************************************************************
        **** Const item accessor []
        ***********************************************************************/
        const T& operator[] (const uint32_t i) const;

        /***********************************************************************
        **** Members variables
        ***********************************************************************/
        Allocator* _allocator; // Pointer to allocator.
        uint32_t _size;        // Size
        uint32_t _capacity;    // Allocate size.
        T* _data;              // Array data;
    };


    /***************************************************************************
    **** Hash
    ***************************************************************************/
    template < typename T > struct Hash {

        /***********************************************************************
        **** Constructor
        ***********************************************************************/
        explicit Hash(Allocator& a);

        /*! Destroy hash map
         */
        //~Hash();

        /***********************************************************************
        **** Hash entry
        ***********************************************************************/
        struct Entry {
            uint64_t key;  // Key.
            uint32_t next; // Next entry index.
            T value;       // Entry value.
        };

        /***********************************************************************
        **** Members variables
        ***********************************************************************/
        Array < uint32_t > _hash; // Key hash -> Data index.
        Array < Entry > _data;    // Data.
    };

    /***************************************************************************
    **** A double-ended queue/ring buffer
    ***************************************************************************/
    template < typename T > struct Queue {

        /***********************************************************************
        **** Constructor
        ***********************************************************************/
        explicit Queue(Allocator& a);

        /***********************************************************************
        **** Item accessor []
        ***********************************************************************/
        T& operator[] (const uint32_t i);

        /***********************************************************************
        **** Const item accessor []
        ***********************************************************************/
        const T& operator[] (const uint32_t i) const;

        /***********************************************************************
        **** Members variables
        ***********************************************************************/
        Array < T > _data; // Data
        uint32_t _size;    // Size
        uint32_t _offset;  // Offset
    };


    /***************************************************************************
    **** Event stream
    ***************************************************************************/
    struct EventStream {

        /***********************************************************************
        **** Constructor
        ***********************************************************************/
        explicit EventStream(Allocator& allocator);

        /***********************************************************************
        **** Members variables
        ***********************************************************************/
        Array < char > stream;
    };
}
