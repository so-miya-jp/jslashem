/* $Id: riputil.c,v 1.4 2005/05/12 14:45:12 so-miya Exp $ */
/* Copyright (c) Slash'EM Development Team 2001-2002 */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, 2004-2005
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

extern const char *killed_by_prefix[];

char *
get_killer_string(how)
int how;
{
    static char buf[BUFSZ];
    /* Put together death description */
    switch (killer_format) {
	default: impossible("bad killer format?");
	case KILLED_BY_AN:
#if 0 /*JP*/
	    if (Instant_Death) {
		Strcpy(buf, "instantly ");
		strcat(buf, killed_by_prefix[how]);
	    }
	    else if (Quick_Death) {
		Strcpy(buf, "quickly ");
		strcat(buf, killed_by_prefix[how]);
	    }
	    else
		Strcpy(buf, killed_by_prefix[how]);
	    Strcat(buf, an(killer));
#else
	    if (Instant_Death) {
		Strcpy(buf, "‚ ‚Á‚¯‚È‚­");
	    } else if (Quick_Death) {
		Strcpy(buf, "‚ ‚Á‚Æ‚¢‚¤‚Ü‚É");
	    } else Strcpy(buf, "") ;
	    Strcat(buf, an(killer));
	    Strcat(buf, killed_by_prefix[how]);
#endif
	    break;
	case KILLED_BY:
#if 0 /*JP*/
	    if (Instant_Death) {
		Strcpy(buf, "instantly ");
		strcat(buf, killed_by_prefix[how]);
	    }
	    else if (Quick_Death) {
		Strcpy(buf, "quickly ");
		strcat(buf, killed_by_prefix[how]);
	    }
	    else
		Strcpy(buf, killed_by_prefix[how]);
	    Strcat(buf, killer);
#else
	    if (Instant_Death) {
		Strcpy(buf, "‚ ‚Á‚¯‚È‚­");
	    } else if (Quick_Death) {
		Strcpy(buf, "‚ ‚Á‚Æ‚¢‚¤‚Ü‚É");
	    } else Strcpy(buf, "") ;
	    Strcat(buf, killer);
	    Strcat(buf, killed_by_prefix[how]);
#endif
	    break;
	case NO_KILLER_PREFIX:
	    Strcpy(buf, killer);
	    break;
#if 1 /*JP*/
	case KILLED_SUFFIX:
	    Strcpy(buf, killer);
	    Strcat(buf, "ŽE‚³‚ê‚½");
#endif
    }
    return buf;
}
