/*
    Copyright (C) 2002 by Anders Stenberg
    Written by Anders Stenberg

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
#include "cstypes.h"
#include "csutil/csvector.h"
#include "csutil/scf.h"
#include "csutil/hashmap.h"
#include "csutil/strset.h"

#include "ivideo/effects/efpass.h"
#include "efpass.h"
#include "ivideo/effects/eflayer.h"
#include "eflayer.h"

iEffectLayer* csEffectPass::CreateLayer()
{
  csEffectLayer* layerobj = new csEffectLayer();
  csRef<iEffectLayer> layer (SCF_QUERY_INTERFACE( layerobj, iEffectLayer ));
  layers.Push( layer );
  layer->IncRef ();	// To avoid smart pointer release.
  return layer;
}

int csEffectPass::GetLayerCount()
{
  return layers.Length();
}

iEffectLayer* csEffectPass::GetLayer( int layer )
{
  return (iEffectLayer*)(layers.Get( layer ));
}
