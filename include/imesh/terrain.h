/*
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __CS_IMESH_TERRAIN_H__
#define __CS_IMESH_TERRAIN_H__

#include "csgeom/vector3.h"

#include "csutil/array.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"

#include "iengine/material.h"

struct iImage;
struct iMovable;
class csTransform;

SCF_VERSION (iTerrainObjectState, 0, 0, 1);

/**
 * This will override the settings for material in the parent
 */
struct iTerrainObjectState : public iBase
{
  /**
   * Set/Get the material palette, this is used to specify materials on a 
   * bytemap representing the material makeup of the terrain
   */
  virtual bool SetMaterialPalette (const csArray<iMaterialWrapper*>& pal) = 0;
  virtual csArray<iMaterialWrapper*> GetMaterialPalette () = 0;

  /**
   * In short, the materialmap paints the palette onto the terrain like
   * an indexed image format paints a color palette onto the screen
   * It sets the materials per pixel in the material map.  If x and y are not
   * equal to the heightmap the materialmap will scale accordingly.  If they
   * are equal than the material map will map a single material to a single
   * high level quad in the final terrain.
   * Note x and y must be 2^n and usually they must be equal
   */
  virtual bool SetMaterialMap (csArray<char> data, int x, int y) = 0;
  virtual bool SetMaterialMap (iImage* map) = 0;
  virtual csArray<char> GetMaterialMap () = 0;

  /**
   * The distance at which splatting is no longer in effect and the
   * base texture is the only layer.
   */
  virtual void SetLODDistance (float distance) = 0;
  virtual float GetLODDistance () = 0;

  /**
   * Sets the screenspace error tolerance in numbers of pixels. 
   * Error will be less than the given tolerance so setting the tolerance
   * to values less than 1 is worthless.
   */
  virtual void SetErrorTolerance (float error) = 0;
  virtual float GetErrorTolerance () = 0;

  /**
   * Save/Restore preprocessing information, the algorithm will 
   * do some preprocessing based on the material and height information
   * this allows the process to be saved out to a file and cached 
   * for later reuse (maybe this should be the caching system)
   * In some cases it may actually memorymap this file
   */
  virtual bool SaveState (const char *filename) = 0;
  virtual bool RestoreState (const char *filename) = 0;

  /// Detects collision with a specific transform
  virtual int CollisionDetect (iMovable *m, csTransform *p) = 0;
};

SCF_VERSION (iTerrainFactoryState, 0, 0, 1);

/**
 * Allows the setting of a set of generic terrain parameters outside
 * any specific algorithm.  It is up to the algorithm to determine the
 * best use of the information provided in the interface.
 */
struct iTerrainFactoryState : public iBase
{
  /// Set/Get the 3 axis scale of the terrain
  virtual void SetScale (const csVector3& scale) = 0;
  virtual csVector3 GetScale () = 0;

  /**
   * Sets the heightmap for heightvalues
   * Note x and y must be 2^n + 1 and usually they must be equal
   */
  virtual bool SetHeightMap (const csArray<float>& data, int x, int y) = 0;
  virtual bool SetHeightMap (iImage* map) = 0;
  virtual csArray<float> GetHeightMap () = 0;

  /**
   * Save/Restore preprocessing information, the algorithm will 
   * do some preprocessing based on the material and height information
   * this allows the process to be saved out to a file and cached 
   * for later reuse (maybe this should be the caching system)
   * In some cases it may actually memorymap this file
   */
  virtual bool SaveState (const char *filename) = 0;
  virtual bool RestoreState (const char *filename) = 0;

};

#endif // __CS_IMESH_TERRAIN_H__
