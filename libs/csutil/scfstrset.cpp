/*
    Copyright (C) 2003 by Anders Stenberg

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

#define CS_CSUTIL_LIB
#include "cssysdef.h"
#include "csutil/scfstrset.h"

SCF_IMPLEMENT_IBASE (csScfStringSet)
  SCF_IMPLEMENTS_INTERFACE (iStringSet)
SCF_IMPLEMENT_IBASE_END

csStringID csScfStringSet::Request (const char *s)
{
  return set.Request (s);
}

const char* csScfStringSet::Request (csStringID id)
{
  return set.Request (id);
}

void csScfStringSet::Clear ()
{
  set.Clear ();
}
