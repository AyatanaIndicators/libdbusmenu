/*
Types that are used in several objects.

Copyright 2011 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of either or both of the following licenses:

1) the GNU Lesser General Public License version 3, as published by the 
Free Software Foundation; and/or
2) the GNU Lesser General Public License version 2.1, as published by 
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the applicable version of the GNU Lesser General Public 
License for more details.

You should have received a copy of both the GNU Lesser General Public 
License version 3 and version 2.1 along with this program.  If not, see 
<http://www.gnu.org/licenses/>
*/

#ifndef __DBUSMENU_TYPES_H__
#define __DBUSMENU_TYPES_H__

#include <glib.h>

G_BEGIN_DECLS

/**
	DbusmenuTextDirection:
	@DBUSMENU_TEXT_DIRECTION_NONE: Unspecified text direction
	@DBUSMENU_TEXT_DIRECTION_LTR: Left-to-right text direction
	@DBUSMENU_TEXT_DIRECTION_RTL: Right-to-left text direction

	The direction of text that the strings that this server
	will be sending strings as.
*/
typedef enum _DbusmenuTextDirection DbusmenuTextDirection;
enum _DbusmenuTextDirection { /*< prefix=DBUSMENU >*/
	DBUSMENU_TEXT_DIRECTION_NONE, /*< nick=none >*/
	DBUSMENU_TEXT_DIRECTION_LTR,  /*< nick=ltr  >*/
	DBUSMENU_TEXT_DIRECTION_RTL   /*< nick=rtl  >*/
};


G_END_DECLS

#endif /* __DBUSMENU_TYPES_H__ */

