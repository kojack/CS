/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#include "cssysdef.h"
#include "csutil/strset.h"
#include "csutil/util.h"

csStringSet::csStringSet ()
{
  IDCounter = 0;
}

csStringSet::~csStringSet ()
{
}

csStringID csStringSet::Request (const char *Name)
{
  csStringID id = Registry.Request (Name);
  if (id == csInvalidStringID)
  {
    Registry.Register (Name, IDCounter);
    IDCounter++;
    return IDCounter-1;
  }
  else
  {
    return id;
  }
}

const char* csStringSet::Request (csStringID id)
{
  return Registry.Request (id);
}

void csStringSet::Clear ()
{
  Registry.Clear ();
}

