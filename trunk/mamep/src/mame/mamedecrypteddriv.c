/******************************************************************************

    mamedriv.c

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    The list of all available drivers. Drivers have to be included here to be
    recognized by the executable.

    To save some typing, we use a hack here. This file is recursively #included
    twice, with different definitions of the DRIVER() macro. The first one
    declares external references to the drivers; the second one builds an array
    storing all the drivers.

******************************************************************************/

#include "driver.h"

#ifndef DRIVER_RECURSIVE

#define DRIVER_RECURSIVE

/* step 1: declare all external references */
#define DRIVER(NAME) extern game_driver driver_##NAME;
#include "mamedecrypteddriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &driver_##NAME,
const game_driver * const decrypteddrivers[] =
{
#include "mamedecrypteddriv.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */
	/* CPS2 Phoenix Edition */
	DRIVER( ddtodd )	/* 12/04/1994 (c) 1993 (Euro) Phoenix Edition */
	DRIVER( avspd )		/* 20/05/1994 (c) 1994 (Euro) Phoenix Edition */
	DRIVER( ringdstd )	/* 02/09/1994 (c) 1994 (Euro) Phoenix Edition */
	DRIVER( xmcotad )	/* 05/01/1995 (c) 1994 (Euro) Phoenix Edition */
	DRIVER( nwarrud )	/* 06/04/1995 (c) 1995 (US) Phoenix Edition */
	DRIVER( sfad )		/* 27/07/1995 (c) 1995 (Euro) Phoenix Edition */
	DRIVER( 19xxd )		/* 07/12/1995 (c) 1996 (US) Phoenix Edition */
	DRIVER( ddsomud )	/* 19/06/1996 (c) 1996 (US) Phoenix Edition */
	DRIVER( spf2xjd )	/* 31/05/1996 (c) 1996 (Japan) Phoenix Edition */
	DRIVER( megamn2d )	/* 08/07/1996 (c) 1996 (US) Phoenix Edition */
	DRIVER( sfz2aad )	/* 26/08/1996 (c) 1996 (Asia) Phoenix Edition */
	DRIVER( xmvsfu1d )	/* 04/10/1996 (c) 1996 (US) Phoenix Edition */
	DRIVER( batcird )	/* 19/03/1997 (c) 1997 (Euro) Phoenix Edition */
	DRIVER( vsavd )		/* 19/05/1997 (c) 1997 (Euro) Phoenix Edition */
	DRIVER( mvscud )	/* 23/01/1998 (c) 1998 (US) Phoenix Edition */
	DRIVER( sfa3ud )	/* 04/09/1998 (c) 1998 (US) Phoenix Edition */
	DRIVER( gwingjd )	/* 23/02/1999 (c) 1999 Takumi (Japan) Phoenix Edition */
	DRIVER( 1944d )		/* 20/06/2000 (c) 2000 Eighting/Raizing (US) Phoenix Edition */
	DRIVER( hsf2d )		/* 02/02/2004 (c) 2004 (Asia) Phoenix Edition */
	DRIVER( dstlku1d )	/* 05/07/1994 (c) 1994 (Phoenix Edition, US 940705) */
	DRIVER( progerjd )	/* 17/01/2001 (c) 2001 Cave (Phoenix Edition, Japan) */
	DRIVER( ssf2ud )	/* 11/09/1993 (c) 1993 (Phoenix Edition, US) */
	DRIVER( mshud )		/* 24/10/1995 (c) 1995 (Phoenix Edition, US) */
	DRIVER( sfz2ad )	/* 27/02/1996 (c) 1996 (Phoenix Edition, ASIA) */
	DRIVER( ssf2tbd )	/* 11/19/1993 (c) 1993 (Phoenix Edition, World) */
	DRIVER( ssf2xjd )	/* 23/02/1994 (c) 1994 (Phoenix Edition, Japan) */

	/* NeoGeo decrypted junk */
	DRIVER( zupapad )	/* 0070 Zupapa - released in 2001, 1994 prototype probably exists */
	DRIVER( kof99d )	/* 0251 (c) 1999 SNK */
	DRIVER( ganryud )	/* 0252 (c) 1999 Visco */
	DRIVER( garoud )	/* 0253 (c) 1999 SNK */
	DRIVER( s1945pd )	/* 0254 (c) 1999 Psikyo */
	DRIVER( preisl2d )	/* 0255 (c) 1999 Yumekobo */
	DRIVER( mslug3d )	/* 0256 (c) 2000 SNK */
	DRIVER( kof2000d )	/* 0257 (c) 2000 SNK */
	DRIVER( nitdd )		/* 0260 (c) 2000 Eleven / Gavaking */
	DRIVER( sengok3d )	/* 0261 (c) 2001 SNK */
	DRIVER( kof2001d )	/* 0262 (c) 2001 Eolith / SNK */
	DRIVER( mslug4d )	/* 0263 (c) 2002 Mega Enterprise */
	DRIVER( rotdd )		/* 0264 (c) 2002 Evoga */
	DRIVER( kof2002d )	/* 0265 (c) 2002 Eolith / Playmore */
	DRIVER( matrimd )	/* 0266 (c) 2002 Atlus */
	DRIVER( ct2k3ad )	/* Bootleg */
	DRIVER( cthd2k3d )	/* Bootleg */
	DRIVER( kof10thd )	/* Bootleg */
	DRIVER( kof2003d )	/* 0271 (c) 2003 Playmore */
	DRIVER( mslug5d )	/* 0268 (c) 2003 Playmore */
	DRIVER( samsho5d )	/* 0270 (c) 2003 Playmore */
	DRIVER( samsh5sd )	/* 0272 (c) 2003 Playmore */
	DRIVER( lans2k4d )	/* Bootleg */
	DRIVER( svcd )	/* 0269 (c) 2003 Playmore / Capcom */
	DRIVER( jckeygpd )
	DRIVER( kf2k3pcd )	/* 0271 (c) 2003 Playmore - JAMMA PCB */
	DRIVER( kogd )	/* Bootleg */
	DRIVER( pnyaad )	/* 0267 (c) 2003 Aiky / Taito */
	DRIVER( bangbedp )	/* 0259 (c) 2000 Visco */

#endif	/* DRIVER_RECURSIVE */
