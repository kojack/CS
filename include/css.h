/*
    Copyright (C) 1998 by Jorrit Tyberghein
	CSScript module created by Brandon Ehle
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

//The king of all include files 
//Good for being a precompiled header, by utilizing the NO_* macros, while stop a complete build

#ifndef __CS_H__
#define __CS_H__

#ifndef NOTHING

//System
#ifndef NO_SYS
#include "sysdef.h"
#endif

//SCF
#ifndef NO_CSSCF
#include "csutil/scf.h"
#include "itxtmgr.h"
#endif

//CS Sys
#ifndef NO_CSSYS
#include "cssys/sysdriv.h"
#endif

//CS Interfaces
#ifndef NO_CSINTERFACE
#include "ipolygon.h"
#include "ithing.h"
#endif

//CS Geom
#ifndef NO_CSGEOM
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#endif

//CS Engine
#ifndef NO_CSENGINE
#include "csengine/collider.h"
#include "csengine/csview.h"
#include "csengine/light.h"
#endif

//CS Object
#ifndef NO_CSOBJECT
#endif

//CS Util
#ifndef NO_CSUTIL
#include "csutil/inifile.h"
#include "csutil/vfs.h"
#endif

//CS Tools
#ifndef NO_CSTOOLS
#include "cstools/simpcons.h"
#endif

//CS Parser
#ifndef NO_CSPARSER
#include "csparser/csloader.h"
#endif

//CS Support
#ifndef NO_SUPPORT
#include "support/command.h"
#endif

#endif

#endif
