/*
    Copyright (C) 2000 by Samuel Humphreys

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

#ifndef __CS_PROTEX3D_H__
#define __CS_PROTEX3D_H__

#include "sft3dcom.h"
#include "isprotex.h"

class csTextureMMSoftware;


class csSoftProcTexture3D : public csGraphics3DSoftwareCommon, 
			    public iSoftProcTexture
{
  /// True when it is necessary to reprepare a texture each update.
  bool reprepare;

  /**
   * The first instance of a procedural texture utilising a dedicated 
   * 8bit texture manager.
   */
  static csSoftProcTexture3D *head_texG3D;

  csGraphics3DSoftwareCommon* partner;

public:
  csTextureMMSoftware *soft_tex_mm;
  csTextureMMSoftware *parent_tex_mm;
  DECLARE_IBASE;

  csSoftProcTexture3D (iBase *iParent);
  virtual ~csSoftProcTexture3D ();

  bool Prepare (csTextureMMSoftware *tex_mm,
		csGraphics3DSoftwareCommon *parent_g3d,
		csSoftProcTexture3D *partner_g3d,
		void *buffer, uint8 *bitmap,
		csPixelFormat *pfmt, RGBPixel *palette, bool alone_hint);

  virtual bool Initialize (iSystem *iSys);

  virtual void Print (csRect *area);

  // The entry interface for other than software drivers..
  // implementation of iSoftProcTexture
  virtual iTextureHandle *CreateOffScreenRenderer 
    (iGraphics3D *parent_g3d, iGraphics3D *partner_g3d, int width, int height, 
     void *buffer, csOffScreenBuffer hint, csPixelFormat *ipfmt);

  virtual void ConvertMode ();
};

#endif // __CS_PROTEX3D_H__
