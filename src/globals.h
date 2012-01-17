/*
    $Id$


    Copyright (C) 2003 Florian Schmitt, Science and Computing AG
                       f.schmitt@science-computing.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef GLOBALS_H
#define GLOBALS_H

#ifndef BUILDDATESTR
#ifndef WIN32
#define BUILDDATESTR "unknown"
#else
#define BUILDDATESTR __DATE__+" "__TIME__
#endif
#endif
#ifndef VERSIONSTR
#define VERSIONSTR "unknown"
#endif

#include <QString>

/** Enthaelt das Verzeichnis, in dem das sctime-Executable liegt*/
extern QString execDir;

/** Enthaelt das Config-Verzeichnis */
extern QString configDir;

extern QString lockfilePath;

#endif
