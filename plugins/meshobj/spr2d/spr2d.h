/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef _SPR2D_H_
#define _SPR2D_H_

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "imeshobj.h"
#include "imspr2d.h"
#include "igraph3d.h"
#include "itranman.h"
#include "iconfig.h"
#include "iparticl.h"

struct iMaterialWrapper;
class csSprite2DMeshObjectFactory;

/**
 * Sprite 2D version of mesh object.
 */
class csSprite2DMeshObject : public iMeshObject
{
private:
  csSprite2DMeshObjectFactory* factory;

  iMaterialWrapper* material;
  UInt MixMode;

  /**
   * Array of 3D vertices.
   */
  csColoredVertices vertices;

  /**
   * If false then we don't do lighting but instead use
   * the given colors.
   */
  bool lighting;

  /// Temporary camera space vector between DrawTest() and Draw().
  csVector3 cam;
  /// Polygon.
  G3DPolygonDPFX g3dpolyfx;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

public:
  /// Constructor.
  csSprite2DMeshObject (csSprite2DMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csSprite2DMeshObject ();

  /// Get the vertex array.
  csColoredVertices& GetVertices () { return vertices; }
  /** 
   * Set vertices to form a regular n-polygon around (0,0),
   * optionally also set u,v to corresponding coordinates in a texture.
   * Large n approximates a circle with radius 1. n must be > 2. 
   */
  void CreateRegularVertices (int n, bool setuv);

  ///------------------------ iMeshObject implementation ------------------------
  DECLARE_IBASE;

  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable);
  virtual void GetObjectBoundingBox (csBox3& bbox, bool accurate = false);

  //------------------------- iSprite2DState implementation ----------------
  class Sprite2DState : public iSprite2DState
  {
    DECLARE_EMBEDDED_IBASE (csSprite2DMeshObject);
    virtual void SetLighting (bool l) { scfParent->lighting = l; }
    virtual bool HasLighting () { return scfParent->lighting; }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () { return scfParent->MixMode; }
    virtual csColoredVertices& GetVertices ()
    {
      return scfParent->GetVertices ();
    }
    virtual void CreateRegularVertices (int n, bool setuv)
    {
      scfParent->CreateRegularVertices (n, setuv);
    }
  } scfiSprite2DState;
  friend class Sprite2DState;
};

/**
 * Factory for 2D sprites. This factory also implements iSprite2DFactoryState.
 */
class csSprite2DMeshObjectFactory : public iMeshObjectFactory
{
private:
  iMaterialWrapper* material;
  UInt MixMode;
  /**
   * If false then we don't do lighting but instead use
   * the given colors.
   */
  bool lighting;

public:
  /// Constructor.
  csSprite2DMeshObjectFactory ();

  /// Destructor.
  virtual ~csSprite2DMeshObjectFactory ();

  /// Has this sprite lighting?
  bool HasLighting () { return lighting; }
  /// Get the material for this 2D sprite.
  iMaterialWrapper* GetMaterialWrapper () { return material; }
  /// Get mixmode.
  UInt GetMixMode () { return MixMode; }

  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObject* NewInstance ();

  //------------------------- iSprite2DFactoryState implementation ----------------
  class Sprite2DFactoryState : public iSprite2DFactoryState
  {
    DECLARE_EMBEDDED_IBASE (csSprite2DMeshObjectFactory);
    virtual void SetLighting (bool l) { scfParent->lighting = l; }
    virtual bool HasLighting () { return scfParent->HasLighting (); }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () { return scfParent->MixMode; }
  } scfiSprite2DFactoryState;
  friend class Sprite2DFactoryState;
};

/**
 * Sprite 2D type. This is the plugin you have to use to create instances
 * of csSprite2DMeshObjectFactory.
 */
class csSprite2DMeshObjectType : public iMeshObjectType
{
public:
  /// Constructor.
  csSprite2DMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csSprite2DMeshObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  //------------------------ iMeshObjectType implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
};

#endif // _SPR2D_H_

