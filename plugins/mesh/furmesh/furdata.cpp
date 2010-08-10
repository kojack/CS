/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <cssysdef.h>

#include "furmesh.h"
#include "furdata.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /********************
  *  csTextureRGBA
  ********************/

  csTextureRGBA::csTextureRGBA ()
  {
    handle = 0;
    data = 0;
  }

  csTextureRGBA::csTextureRGBA (int width, int height)
  {
    this->width = width;
    this->height = height;
    handle = 0;
    data = 0;
  }

  uint8 csTextureRGBA::Get(int x, int y, int channel) const 
  {
    int pos = 4 * ( x + y * width ) + channel;

    if (pos >= 4 * width * height || pos < 0)
      return 0;

    return data[ pos ];
  }

  void csTextureRGBA::Set(int x, int y, int channel, uint8 value)
  {
    int pos = 4 * ( x + y * width ) + channel;

    if (pos >= 4 * width * height || pos < 0)
      return;

    data[ pos ] = value;
  }

  bool csTextureRGBA::Read()
  {
    CS::StructuredTextureFormat readbackFmt 
      (CS::TextureFormatStrings::ConvertStructured ("abgr8"));

    csRef<iDataBuffer> db = handle->Readback(readbackFmt);

    if (!db)
      return false;

    handle->GetOriginalDimensions(width, height);
    data = db->GetUint8();

    if (!data)
      return false;

    return true;
  }

  void csTextureRGBA::Write()
  {
    handle->Blit(0, 0, width, height / 2, data);
    handle->Blit(0, height / 2, width, height / 2, data + (width * height * 2));
  }

  bool csTextureRGBA::Create(iGraphics3D* g3d)
  {
    if (!g3d)
      return false;

    handle = g3d->GetTextureManager()->CreateTexture(width,height, csimg2D, "abgr8", 
      CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_NOFILTER);

    if(!handle)
      return false;

    return true;
  }

  void csTextureRGBA::SaveImage(iObjectRegistry* object_reg, const char* texname) const
  {
    csRef<iImageIO> imageio = csQueryRegistry<iImageIO> (object_reg);
    csRef<iVFS> VFS = csQueryRegistry<iVFS> (object_reg);

    if(!data)
    {
      csPrintfErr ("Bad data buffer!\n");
      return;
    }

    csRef<iImage> image;
    image.AttachNew(new csImageMemory (width, height, data,false,
      CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));

    if(!image.IsValid())
    {
      csPrintfErr ("Error creating image\n");
      return;
    }

    csPrintf ("Saving %zu KB of data.\n", 
      csImageTools::ComputeDataSize (image)/1024);

    csRef<iDataBuffer> db = imageio->Save (image, "image/png", "progressive");
    if (db)
    {
      if (!VFS->WriteFile (texname, (const char*)db->GetData (), db->GetSize ()))
      {
        csPrintfErr ("Failed to write file '%s'!", texname);
        return;
      }
    }
    else
    {
      csPrintfErr ("Failed to save png image for basemap!");
      return;
    }	    
  }

  /********************
  *  csHairData
  ********************/

  void csHairData::Clear()
  {
    delete controlPoints;
  }

  /********************
  *  csHairStrand
  ********************/

  csVector2 csHairStrand::GetUV( const csArray<csGuideHair> &guideHairs,
    const csArray<csGuideHairLOD> &guideHairsLOD ) const
  {
    csVector2 strandUV(0);

    for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
      if (guideHairsRef[j].index < guideHairs.GetSize() )
        strandUV += guideHairsRef[j].distance * 
        guideHairs.Get(guideHairsRef[j].index).uv;
      else
        strandUV += guideHairsRef[j].distance * 
        guideHairsLOD.Get(guideHairsRef[j].index - guideHairs.GetSize()).uv;

    return strandUV;
  }

  void csHairStrand::Generate( size_t controlPointsCount,
    const csArray<csGuideHair> &guideHairs, 
    const csArray<csGuideHairLOD> &guideHairsLOD )
  {
    // generate control points
    this -> controlPointsCount = controlPointsCount;

    controlPoints = new csVector3[ controlPointsCount ];

    for ( size_t i = 0 ; i < controlPointsCount ; i ++ )
    {
      controlPoints[i] = csVector3(0);

      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideHairs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance *
          guideHairs.Get(guideHairsRef[j].index).controlPoints[i];
        else
          controlPoints[i] += guideHairsRef[j].distance *
          guideHairsLOD.Get(guideHairsRef[j].index - 
          guideHairs.GetSize()).controlPoints[i];
    }
  }

  void csHairStrand::Update( const csArray<csGuideHair> &guideHairs,
    const csArray<csGuideHairLOD> &guideHairsLOD )
  {
    for ( size_t i = 0 ; i < controlPointsCount; i++ )
    {
      controlPoints[i] = csVector3(0);
      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideHairs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance * 
          (guideHairs.Get(guideHairsRef[j].index).controlPoints[i]);
        else
          controlPoints[i] += guideHairsRef[j].distance * (guideHairsLOD.Get
          (guideHairsRef[j].index - guideHairs.GetSize()).controlPoints[i]);
    }
  }

  /********************
  *  csGuideHair
  ********************/

  void csGuideHair::Generate (size_t controlPointsCount, float distance,
    const csVector3& pos, const csVector3& direction)
  {
    this->controlPointsCount = controlPointsCount;

    controlPoints = new csVector3[ controlPointsCount ];

    for ( size_t j = 0 ; j < controlPointsCount ; j ++ )
      controlPoints[j] = pos + j * distance * direction;
  }

  /********************
  *  csGuideHairLOD
  ********************/

  void csGuideHairLOD::Generate( size_t controlPointsCount,
    const csArray<csGuideHair> &guideHairs, 
    const csArray<csGuideHairLOD> &guideHairsLOD )
  {
    // generate control points
    this -> controlPointsCount = controlPointsCount;

    controlPoints = new csVector3[ controlPointsCount ];

    for ( size_t i = 0 ; i < controlPointsCount ; i ++ )
    {
      controlPoints[i] = csVector3(0);
      
      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideHairs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance *
          guideHairs.Get(guideHairsRef[j].index).controlPoints[i];
        else
          controlPoints[i] += guideHairsRef[j].distance *
          guideHairsLOD.Get(guideHairsRef[j].index - 
          guideHairs.GetSize()).controlPoints[i];
    }
  }

  void csGuideHairLOD::Update( const csArray<csGuideHair> &guideHairs,
    const csArray<csGuideHairLOD> &guideHairsLOD )
  {
    for ( size_t i = 0 ; i < controlPointsCount; i++ )
    {
      controlPoints[i] = csVector3(0);
      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideHairs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance * 
          (guideHairs.Get(guideHairsRef[j].index).controlPoints[i]);
        else
          controlPoints[i] += guideHairsRef[j].distance * (guideHairsLOD.Get
          (guideHairsRef[j].index - guideHairs.GetSize()).controlPoints[i]);
    }
  }
}
CS_PLUGIN_NAMESPACE_END(FurMesh)

