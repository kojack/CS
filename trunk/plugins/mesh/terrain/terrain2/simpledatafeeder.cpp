/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007 by Marten Svanfeldt

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

#include "csgeom/csrect.h"
#include "csgfx/imagemanipulate.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"

#include "iengine/material.h"
#include "igraphic/image.h"
#include "imap/loader.h"
#include "imesh/terrain2.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "simpledatafeeder.h"
#include "feederhelper.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{
SCF_IMPLEMENT_FACTORY (csTerrainSimpleDataFeeder)

csTerrainSimpleDataFeeder::csTerrainSimpleDataFeeder (iBase* parent)
  : scfImplementationType (this, parent)
{
}

csTerrainSimpleDataFeeder::~csTerrainSimpleDataFeeder ()
{
}

csPtr<iTerrainCellFeederProperties> csTerrainSimpleDataFeeder::CreateProperties ()
{
  return csPtr<iTerrainCellFeederProperties> (
      new csTerrainSimpleDataFeederProperties);
}

bool csTerrainSimpleDataFeeder::PreLoad (iTerrainCell* cell)
{
  return false;
}

bool csTerrainSimpleDataFeeder::Load (iTerrainCell* cell)
{
  csTerrainSimpleDataFeederProperties* properties = 
    (csTerrainSimpleDataFeederProperties*)cell->GetFeederProperties ();

  if (!loader || !properties)
    return false;

  if (properties->heightmapSource.IsEmpty ())
    return false;

  int width = cell->GetGridWidth ();
  int height = cell->GetGridHeight ();

  csLockedHeightData data = cell->LockHeightData (csRect(0, 0, width, height));
  
  HeightFeederParser mapReader (properties->heightmapSource, 
    properties->heightmapFormat, loader, objectReg);
  mapReader.Load (data.data, width, height, data.pitch, cell->GetSize ().y, 
    properties->heightOffset);

  cell->UnlockHeightData ();
  
  if (!properties->materialmapSource.IsEmpty ())
  {  
    csRef<iImage> materialMap = loader->LoadImage (properties->materialmapSource.GetDataSafe (),
      CS_IMGFMT_PALETTED8);

    if (!materialMap) 
      return false;
    
    if (materialMap->GetWidth () != cell->GetMaterialMapWidth () ||
        materialMap->GetHeight () != cell->GetMaterialMapHeight ())
    {
      materialMap = csImageManipulate::Rescale (materialMap,
        cell->GetMaterialMapWidth (), cell->GetMaterialMapHeight ());
    }
    
    int mwidth = materialMap->GetWidth ();
    int mheight = materialMap->GetHeight ();

    csLockedMaterialMap mdata = cell->LockMaterialMap (csRect (0, 0, mwidth, mheight));

    const unsigned char* materialmap = (const unsigned char*)materialMap->GetImageData ();
      
    for (int y = 0; y < mheight; ++y)
    {
      memcpy (mdata.data, materialmap, mwidth);
      mdata.data += mdata.pitch;
      materialmap += mwidth;
    } 
    
    cell->UnlockMaterialMap ();
  }

  if (engine)
  {
    for (size_t i = 0; i < properties->alphaMaps.GetSize (); ++i)
    {
      iMaterialWrapper* mat = engine->FindMaterial (
        properties->alphaMaps[i].material.GetDataSafe ());

      csRef<iImage> alphaMap = loader->LoadImage (
        properties->alphaMaps[i].alphaSource.GetDataSafe (),
        CS_IMGFMT_ANY);

      if (mat && alphaMap)
      {
        cell->SetAlphaMask (mat, alphaMap);
      }
    }
  }

  return true;
}

bool csTerrainSimpleDataFeeder::Initialize (iObjectRegistry* object_reg)
{
  this->objectReg = object_reg;

  loader = csQueryRegistry<iLoader> (objectReg);
  engine = csQueryRegistry<iEngine> (objectReg);

  return true;
}

void csTerrainSimpleDataFeeder::SetParameter (const char* param, const char* value)
{
}


csTerrainSimpleDataFeederProperties::csTerrainSimpleDataFeederProperties ()
  : scfImplementationType (this), heightOffset (0.0f)
{
}

csTerrainSimpleDataFeederProperties::csTerrainSimpleDataFeederProperties (
  csTerrainSimpleDataFeederProperties& other)
  : scfImplementationType (this),
    heightmapSource (other.heightmapSource),
    heightmapFormat (other.heightmapFormat),
    materialmapSource (other.materialmapSource),
    alphaMaps (other.alphaMaps),
    heightOffset (other.heightOffset)
{
}


void csTerrainSimpleDataFeederProperties::SetHeightmapSource (const char* source,
                                                              const char* format)
{
  heightmapSource = source;
  heightmapFormat = format;
}

void csTerrainSimpleDataFeederProperties::SetMaterialMapSource (const char* source)
{
  materialmapSource = source;
}

void csTerrainSimpleDataFeederProperties::SetHeightOffset (float offset)
{
  heightOffset = offset;
}

void csTerrainSimpleDataFeederProperties::AddAlphaMap (const char* material, 
                                                       const char* alphaMapSource)
{
  for (size_t i = 0; i < alphaMaps.GetSize (); ++i)
  {
    if (alphaMaps[i].material == material)
    {
      alphaMaps[i].alphaSource = alphaMapSource;
      return;
    }
  }

  AlphaPair p = {material, alphaMapSource};
  alphaMaps.Push (p);
}

void csTerrainSimpleDataFeederProperties::SetParameter (const char* param, const char* value)
{
  if (strcasecmp (param, "heightmap source") == 0)
  {
    heightmapSource = value;
  }
  else if (strcasecmp (param, "heightmap format") == 0)
  {
    heightmapFormat = value;
  }
  else if (strcasecmp (param, "materialmap source") == 0)
  {
    materialmapSource = value;
  }
  else if (strcasecmp (param, "offset") == 0)
  {
    heightOffset = atof (value);
  }
}

csPtr<iTerrainCellFeederProperties> csTerrainSimpleDataFeederProperties::Clone ()
{
  return csPtr<iTerrainCellFeederProperties> (new csTerrainSimpleDataFeederProperties (*this));
}

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
