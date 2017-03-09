
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**       File: integral.h
**    Summary: standatd data types and macros
**    Version: @(VERSION)
**       Date: @(DATE)
**     Author: Alexander Kuzmich
**
**************************************************************************
**************************************************************************
**
**  Functions:
**
**
**
**   Compiler: @(COMPILER)
**    Remarks:
** ext. Units:
**
@@    History:
@@
**************************************************************************
**    all rights reserved
*************************************************************************/

// prevent multiple includes
#ifndef INTEGRAL_H
#define INTEGRAL_H

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else /* 
       */
#define EXTERN_C extern
#endif /* 
        */
#endif /* 
        */

#include <linux/types.h>

#ifndef BOOLEAN
#define BOOLEAN __u8
#endif

#ifndef BYTE
#define BYTE  __u8
#endif

#ifndef UINT8
#define UINT8 __u8
#endif

#ifndef UINT16
#define UINT16 __u16
#endif

#ifndef WORD
#define WORD  __u16
#endif

#ifndef UINT32
#define UINT32 __u32
#endif

#ifndef DWORD
#define DWORD  __u32
#endif

#ifndef UINT64
#define UINT64 __u64
#endif

#ifndef QWORD
#define QWORD __u64
#endif

#ifndef INT16
#define INT16  __s16
#endif

#ifndef INT32
#define INT32  __s32
#endif

#ifndef BOOL
#define BOOL  UINT8
#endif

#ifndef TRUE
#define TRUE  1
#endif /* 
        */

#ifndef FALSE
#define FALSE 0
#endif /* 
        */

#ifndef NULL
#ifdef __cplusplus
#define NULL   0
#else
#define NULL   ((void*)0)
#endif
#endif

#ifndef PBYTE
typedef BYTE *PBYTE;
#endif

#ifndef PWORD
typedef WORD *PWORD;
#endif

#ifndef DWORD
#define DWORD  UINT32
#endif

#ifndef PDWORD
typedef DWORD *PDWORD;
#endif

#ifndef VOID
#define VOID void
#endif

#ifdef __KERNEL__
#include <linux/delay.h>
#define IODELAY(a) mdelay((a))
#endif

#define NO_ERROR 0

typedef unsigned long BCI_BRD_HDL;

//------------------------------------------------------------------------
// Macro:
//  MAKEWORD
//
// Description:
//  This macro creates a 16-bit value by concatenating the specified
//  8-bit values. 
//
// Arguments:
//  l -> Specifies the low-order byte of the new value. 
//  h -> Specifies the high-order byte of the new value. 
//
// Results:
//  Returns the new 16-bit value.
//------------------------------------------------------------------------
#ifndef MAKEWORD
#define MAKEWORD(l,h) ((WORD)(((BYTE)(l))|(((WORD)((BYTE)(h)))<<8)))
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  MAKELONG
//
// Description:
//  This macro creates a 32-bit value by concatenating the specified
//  16-bit values. 
//
// Arguments:
//  l -> Specifies the low-order word of the new value. 
//  h -> Specifies the high-order word of the new value. 
//
// Results:
//  Returns the new 32-bit value.
//------------------------------------------------------------------------
#ifndef MAKELONG
#define MAKELONG(l,h) ((LONG)(((WORD)(l))|(((DWORD)((WORD)(h)))<<16)))
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  FIELDOFFSET
//
// Description:
//  This macro calculates the offset of a member variable within the
//  specified data structure.
//
// Arguments:
//  type  -> type of the data structure
//  field -> name of the member variable
//
// Results:
//  Returns the offset of the member variable within the specified
//  data structure.
//------------------------------------------------------------------------
#ifndef FIELDOFFSET
#define FIELDOFFSET(type,field) ((DWORD)(&((type*)0)->field))
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  HOSTOBJECT
//
// Description:
//  This macro retrieve a reference to the host object.
//
// Arguments:
//  t -> Type of the host object
//  m -> Member variable which represents the inner object
//
// Results:
//  Reference to the host object.
//------------------------------------------------------------------------
#ifdef __cplusplus
#define HOSTOBJECT(t,m) ((t&) *((PCHAR) this - (DWORD)(&((t*)0)->m)))
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  OUTEROBJECT
//
// Description:
//  This macro retrieve a pointer to the outer object.
//
// Arguments:
//  i -> Pointer to the inner object
//  m -> Member variable which represents the inner object
//  t -> Type of the outer object
//
// Results:
//  Pointer to the outer object.
//------------------------------------------------------------------------
#define OUTEROBJECT(i,m,t) ((t*) ((PCHAR)(i) - (DWORD)(&((t*)0)->m)))

//------------------------------------------------------------------------
// Macro:
//  ARRAYSIZE, NUMELEM
//
// Description:
//  This macro retreives the number of elements within the specified array.
//
// Arguments:
//  array -> array for which to retrieve the number of elements
//
// Results:
//  Returns the number of elements within the specified array.
//------------------------------------------------------------------------
#ifndef ARRAYSIZE
#define ARRAYSIZE(array) (sizeof(array)/sizeof((array)[0]))
#endif /* 
        */

#ifndef NUMELEM
#define NUMELEM(array) (sizeof(array) / sizeof(array[0]))
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  min
//
// Description:
//  This macro retreives the smaller of two values.
//
// Arguments:
//  a -> value 1 to be compared
//  b -> value 2 to be compared
//
// Results:
//  Returns the smaller of the two arguments.
//------------------------------------------------------------------------
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  max
//
// Description:
//  This macro retreives the larger of two values.
//
// Arguments:
//  a -> value 1 to be compared
//  b -> value 2 to be compared
//
// Results:
//  Returns the larger of the two arguments.
//------------------------------------------------------------------------
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  LONIBBLE
//
// Description:
//  This macro retreives the lower 4 bits of the given 8-bit value.
//
// Arguments:
//  b -> 8-bit value
//
// Results:
//  Low-order nibble (bit 0..3) of the specified value.
//------------------------------------------------------------------------
#ifndef LONIBBLE
#define LONIBBLE(b) ( (BYTE) (b) & 0x0F )
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  HINIBBLE
//
// Description:
//  This macro retreives the upper 4 bits of the given 8-bit value.
//
// Arguments:
//  b -> 8-bit value
//
// Results:
//  High-order nibble (bit 4..7) of the specified value.
//------------------------------------------------------------------------
#ifndef HINIBBLE
#define HINIBBLE(b) ( (BYTE) (b) >> 4 )
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  LOBYTE
//
// Description:
//  This macro retreives the lower 8 bits of the given 16-bit value.
//
// Arguments:
//  w -> 16-bit value
//
// Results:
//  Low-order byte (bit 0..7) of the specified value.
//------------------------------------------------------------------------
#ifndef LOBYTE
#define LOBYTE(w) ( (BYTE) (w) )
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  HIBYTE
//
// Description:
//  This macro retreives the upper 8 bits of the given 16-bit value.
//
// Arguments:
//  w -> 16-bit value
//
// Results:
//  High-order byte (bit 8..15) of the specified value.
//------------------------------------------------------------------------
#ifndef HIBYTE
#define HIBYTE(w) ( (BYTE) ((WORD)(w) >> 8) )
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  LOWORD
//
// Description:
//  This macro retreives the lower 16 bits of the given 32-bit value.
//
// Arguments:
//  l -> 32-bit value
//
// Results:
//  Low-order word (bit 0..15) of the specified value.
//------------------------------------------------------------------------
#ifndef LOWORD
#define LOWORD(l) ( (WORD) (l) )
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  HIWORD
//
// Description:
//  This macro retreives the upper 16 bits of the given 32-bit value.
//
// Arguments:
//  l -> 32-bit value
//
// Results:
//  High-order word (bit 8..15) of the specified value.
//------------------------------------------------------------------------
#ifndef HIWORD
#define HIWORD(l) ( (WORD) ((DWORD)(l) >> 16) )
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  SETPVAR
//
// Description:
//  This macro stores a value in the specified variable.
//
// Arguments:
//  p -> points to the variable
//  v -> value to store in the variable
//
// Results:
//  none
//------------------------------------------------------------------------
#ifndef SETPVAR
#define SETPVAR(p,v) ( (p) ? (*(p) = (v)) : 0 )
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  SWAP
//
// Description:
//  This macro exchange two variables.
//
// Arguments:
//  type -> data type of the variables
//  a    -> variable 1
//  b    -> variable 2
//
// Results:
//  none
//------------------------------------------------------------------------
#ifndef SWAP
#define SWAP(type,a,b) {type x=(type)(a);(type)(a)=(type)(b);(type)(b)=x;}
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  SWAPB, SWAPW, SWAPDW
//
// Description:
//  This macros exchange two variables of type BYTE, WORD or DWORD.
//
// Arguments:
//  a -> variable 1
//  b -> variable 2
//
// Results:
//  none
//------------------------------------------------------------------------
#ifndef SWAPB
#define SWAPB(a,b)  {BYTE x=(BYTE)(a);(BYTE)(a)=(BYTE)(b);(BYTE)(b)=x;}
#endif /* 
        */

#ifndef SWAPW
#define SWAPW(a,b)  {WORD x=(WORD)(a);(WORD)(a)=(WORD)(b);(WORD)(b)=x;}
#endif /* 
        */

#ifndef SWAPDW
#define SWAPDW(a,b) {DWORD x=(DWORD)(a);(DWORD)(a)=(DWORD)(b);(DWORD)(b)=x;}
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  RNDUPB, RNDUPW, RNDUPDW
//
// Description:
//  This macro returns a BYTE, WORD, or DWORD number rounded up to
//  the nearest multiple of the specified base value.
//
// Arguments:
//  v -> value to round up
//  b -> unsigned base value
//
// Results:
//  Rounded value.
//------------------------------------------------------------------------
#ifndef RNDUPB
#define RNDUPB(v,b) ( ((BYTE)(v) + (b) - 1) &~ ((b) - 1) )
#endif /* 
        */

#ifndef RNDUPW
#define RNDUPW(v,b) ( ((WORD)(v) + (b) - 1) &~ ((b) - 1) )
#endif /* 
        */

#ifndef RNDUPDW
#define RNDUPDW(v,b) ( ((DWORD)(v) + (b) - 1) &~ ((b) - 1) )
#endif /* 
        */

//------------------------------------------------------------------------
// Macro:
//  RNDDNB, RNDDNW, RNDDNDW
//
// Description:
//  This macro returns a BYTE, WORD or DWORD number rounded down to
//  the nearest multiple of the specified base value.
//
// Arguments:
//  v -> value to round down
//  b -> unsigned base value
//
// Results:
//  Rounded value.
//------------------------------------------------------------------------
#ifndef RNDDNB
#define RNDDNB(v,b) ( (BYTE)(v) &~ ((b) - 1) )
#endif /* 
        */

#ifndef RNDDNW
#define RNDDNW(v,b) ( (WORD)(v) &~ ((b) - 1) )
#endif /* 
        */

#ifndef RNDDNDW
#define RNDDNDW(v,b) ( (DWORD)(v) &~ ((b) - 1) )
#endif /* 
        */

#endif /* INTEGRAL_H */
