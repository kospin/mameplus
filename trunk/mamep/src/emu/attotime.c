// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    attotime.c

    Support functions for working with attotime data.

***************************************************************************/

#include "emucore.h"
#include "eminline.h"
#include "attotime.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const attotime attotime::zero(0, 0);
const attotime attotime::never(ATTOTIME_MAX_SECONDS, 0);



//**************************************************************************
//  CORE MATH FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  operator*= - multiply an attotime by a
//  constant
//-------------------------------------------------

attotime &attotime::operator*=(UINT32 factor)
{
	// if one of the items is attotime::never, return attotime::never
	if (seconds >= ATTOTIME_MAX_SECONDS)
		return *this = never;

	// 0 times anything is zero
	if (factor == 0)
		return *this = zero;

	// split attoseconds into upper and lower halves which fit into 32 bits
	UINT32 attolo;
	UINT32 attohi = divu_64x32_rem(attoseconds, ATTOSECONDS_PER_SECOND_SQRT, &attolo);

	// scale the lower half, then split into high/low parts
	UINT64 temp = mulu_32x32(attolo, factor);
	UINT32 reslo;
	temp = divu_64x32_rem(temp, ATTOSECONDS_PER_SECOND_SQRT, &reslo);

	// scale the upper half, then split into high/low parts
	temp += mulu_32x32(attohi, factor);
	UINT32 reshi;
	temp = divu_64x32_rem(temp, ATTOSECONDS_PER_SECOND_SQRT, &reshi);

	// scale the seconds
	temp += mulu_32x32(seconds, factor);
	if (temp >= ATTOTIME_MAX_SECONDS)
		return *this = never;

	// build the result
	seconds = temp;
	attoseconds = (attoseconds_t)reslo + mul_32x32(reshi, ATTOSECONDS_PER_SECOND_SQRT);
	return *this;
}


//-------------------------------------------------
//  operator/= - divide an attotime by a constant
//-------------------------------------------------

attotime &attotime::operator/=(UINT32 factor)
{
	// if one of the items is attotime::never, return attotime::never
	if (seconds >= ATTOTIME_MAX_SECONDS)
		return *this = never;

	// ignore divide by zero
	if (factor == 0)
		return *this;

	// split attoseconds into upper and lower halves which fit into 32 bits
	UINT32 attolo;
	UINT32 attohi = divu_64x32_rem(attoseconds, ATTOSECONDS_PER_SECOND_SQRT, &attolo);

	// divide the seconds and get the remainder
	UINT32 remainder;
	seconds = divu_64x32_rem(seconds, factor, &remainder);

	// combine the upper half of attoseconds with the remainder and divide that
	UINT64 temp = (INT64)attohi + mulu_32x32(remainder, ATTOSECONDS_PER_SECOND_SQRT);
	UINT32 reshi = divu_64x32_rem(temp, factor, &remainder);

	// combine the lower half of attoseconds with the remainder and divide that
	temp = attolo + mulu_32x32(remainder, ATTOSECONDS_PER_SECOND_SQRT);
	UINT32 reslo = divu_64x32_rem(temp, factor, &remainder);

	// round based on the remainder
	attoseconds = (attoseconds_t)reslo + mulu_32x32(reshi, ATTOSECONDS_PER_SECOND_SQRT);
	if (remainder >= factor / 2)
		if (++attoseconds >= ATTOSECONDS_PER_SECOND)
		{
			attoseconds = 0;
			seconds++;
		}
	return *this;
}


//-------------------------------------------------
//  as_string - return a temporary printable
//  string describing an attotime
//-------------------------------------------------

const char *attotime::as_string(int precision) const
{
	static char buffers[8][30];
	static int nextbuf;
	char *buffer = &buffers[nextbuf++ % 8][0];

	// special case: never
	if (*this == never)
		sprintf(buffer, "%-*s", precision, "(never)");

	// case 1: we want no precision; seconds only
	else if (precision == 0)
		sprintf(buffer, "%d", seconds);

	// case 2: we want 9 or fewer digits of precision
	else if (precision <= 9)
	{
		UINT32 upper = attoseconds / ATTOSECONDS_PER_SECOND_SQRT;
		int temp = precision;
		while (temp < 9)
		{
			upper /= 10;
			temp++;
		}
		sprintf(buffer, "%d.%0*d", seconds, precision, upper);
	}

	// case 3: more than 9 digits of precision
	else
	{
		UINT32 lower;
		UINT32 upper = divu_64x32_rem(attoseconds, ATTOSECONDS_PER_SECOND_SQRT, &lower);
		int temp = precision;
		while (temp < 18)
		{
			lower /= 10;
			temp++;
		}
		sprintf(buffer, "%d.%09d%0*d", seconds, upper, precision - 9, lower);
	}
	return buffer;
}
