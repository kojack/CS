/*
    Copyright (C) 2003 by Keith Fulton

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

#ifndef __CS_SPRCAL3D_H__
#define __CS_SPRCAL3D_H__

#include "cssys/sysfunc.h"
#include "csutil/cscolor.h"
#include "csutil/parray.h"
#include "csutil/randomgen.h"
#include "csutil/refarr.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "csgeom/objmodel.h"
#include "imesh/spritecal3d.h"
#include "imesh/object.h"
#include "iengine/material.h"
#include "iengine/lod.h"
#include "iutil/config.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "ivideo/graph3d.h"
#ifndef CS_USE_NEW_RENDERER
#include "ivideo/vbufmgr.h"
#else
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"
#include "cstool/anonrndbuf.h"
#endif // CS_USE_NEW_RENDERER
#include "ivideo/material.h"
#include "qint.h"

#include <cal3d/cal3d.h>

struct iObjectRegistry;
struct iEngine;
struct iMaterialWrapper;

#define ALL_LOD_FEATURES (CS_LOD_TRIANGLE_REDUCTION|CS_LOD_DISTANCE_REDUCTION)

struct csCal3DAnimation
{
    int      index;
    csString name;
    int	     type;
    float    base_velocity;
    float    min_velocity;
    float    max_velocity;
};

struct csCal3DMesh
{
    int		      index;
    csString	      name;
    bool	      attach_by_default;
    iMaterialWrapper *default_material;
};

class csSpriteCal3DMeshObject;

/**
 * A Cal3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames.
 * This class represents a template from which a csSpriteCal3D
 * class can be made.
 */
class csSpriteCal3DMeshObjectFactory : public iMeshObjectFactory
{
private:
  friend class csSpriteCal3DMeshObject;

  /// Material handle as returned by iTextureManager.
  iBase* logparent;

  /// If true then this factory has been initialized.
  bool initialized;

  /// This is the factory equivalent class in cal3d.
  CalCoreModel calCoreModel;
  csArray<csCal3DAnimation*> anims;
  csArray<csCal3DMesh*>      submeshes;

  csString     basePath;
  float	       renderScale;

public:
  iObjectRegistry* object_reg;
  iVirtualClock* vc;

  csRef<iGraphics3D> g3d;

  /**
   * Reference to the engine (optional because sprites can also be
   * used for the isometric engine).
   */
  iEngine* engine;

  /// Create the sprite template.
  csSpriteCal3DMeshObjectFactory (iBase *pParent);
  /// Destroy the template.
  virtual ~csSpriteCal3DMeshObjectFactory ();

  void Report (int severity, const char* msg, ...);

  //------------------------iSpriteCal3dFactoryState implementation --------
  /// Create a new core object.
  bool Create(const char *name);
  void ReportLastError ();
  void SetBasePath(const char *path);
  void SetRenderScale(float scale);
  float GetRenderScale() { return renderScale; }
  bool LoadCoreSkeleton(const char *filename);
  int  LoadCoreAnimation(const char *filename,const char *name,int type,float base_vel,float min_vel,float max_vel);
  bool LoadCoreMesh(const char *filename,const char *name,bool attach,iMaterialWrapper *defmat);
  bool AddCoreMaterial(iMaterialWrapper *mat);
  void BindMaterials();

  int  GetMeshCount() { return submeshes.Length(); }
  const char *GetMeshName(int idx);
  int  FindMeshName(const char *meshName);
  bool IsMeshDefault(int idx);

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual csPtr<iMeshObject> NewInstance ();
  virtual void HardTransform (const csReversibleTransform& t) { };
  virtual bool SupportsHardTransform () const { return true; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }


  //------------------ iPolygonMesh interface implementation ----------------//

  int GetVertexCount() { return 0; }
  csVector3* GetVertices () { return 0; }
  int GetTriangleCount () { return 0; }
  csMeshedPolygon* GetPolygons () { return 0; }

  struct PolyMesh : public iPolygonMesh
  {
  private:
    csSpriteCal3DMeshObjectFactory* factory;
  public:
    SCF_DECLARE_IBASE;

    void SetFactory (csSpriteCal3DMeshObjectFactory* Factory)
    {
      factory = Factory;
    }

    /// Get the number of vertices for this mesh.
    virtual int GetVertexCount ()
    {
      return factory->GetVertexCount ();
    }
    /// Get the pointer to the array of vertices.
    virtual csVector3* GetVertices ()
    {
      return factory->GetVertices ();
    }
    /// Get the number of polygons for this mesh.
    virtual int GetPolygonCount ()
    {
      return factory->GetTriangleCount ();
    }

    /// Get the pointer to the array of polygons.
    virtual csMeshedPolygon* GetPolygons ()
    {
      return factory->GetPolygons();
    }

    /// Cleanup.
    virtual void Cleanup () { } //  delete[] polygons; polygons = 0; }
    
    virtual bool IsDeformable () const { return false;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh () : polygons (0)
    {
      SCF_CONSTRUCT_IBASE (0);
    }
    virtual ~PolyMesh () { Cleanup (); }

    csMeshedPolygon* polygons;
  } scfiPolygonMesh;
  friend struct PolyMesh;

  //------------------------- iObjectModel implementation ----------------
  void GetObjectBoundingBox (csBox3& bbox, int type, csVector3 *verts,int vertCount);
  void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  void GetRadius (csVector3& rad, csVector3& cent);

  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory);
    virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL)
    {
      scfParent->GetObjectBoundingBox (bbox, type);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }

  //--------------------- iSpriteCal3DFactoryState implementation -------------//
  struct SpriteCal3DFactoryState : public iSpriteCal3DFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory);

    virtual bool Create(const char *name)
    { return scfParent->Create(name); }

    virtual void ReportLastError ()
    { scfParent->ReportLastError(); }

    virtual void SetBasePath(const char *path)
    { scfParent->SetBasePath(path); }

    virtual void SetRenderScale(float scale)
    { scfParent->SetRenderScale(scale); }

    virtual bool LoadCoreSkeleton(const char *filename)
    { return scfParent->LoadCoreSkeleton(filename); }

    virtual int LoadCoreAnimation(const char *filename,const char *name,int type,float base_vel,float min_vel,float max_vel)
    { return scfParent->LoadCoreAnimation(filename,name,type,base_vel,min_vel,max_vel); }

    virtual bool LoadCoreMesh(const char *filename,const char *name,bool attach,iMaterialWrapper *defmat)
    { return scfParent->LoadCoreMesh(filename,name,attach,defmat); }

    virtual bool AddCoreMaterial(iMaterialWrapper *mat)
    { return scfParent->AddCoreMaterial(mat); }

    virtual void BindMaterials()
    { scfParent->BindMaterials(); }

    virtual int  GetMeshCount()
    { return scfParent->GetMeshCount(); }

    virtual const char *GetMeshName(int idx)
    { return scfParent->GetMeshName(idx); }

    virtual int  FindMeshName(const char *meshName)
    { return scfParent->FindMeshName(meshName); }

    virtual bool IsMeshDefault(int idx)
    { return scfParent->IsMeshDefault(idx); }

  } scfiSpriteCal3DFactoryState;
  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory);
    virtual void SetLOD (float, float)
    {
    }
    virtual void GetLOD (float& m, float& a) const
    {
      m = 0;
      a = 1;
    }
    virtual void SetLOD (iSharedVariable*, iSharedVariable*)
    {
    }
    virtual void GetLOD (iSharedVariable*& varm, iSharedVariable* &vara)
    {
      varm = 0;
      vara = 0;
    }
    virtual int GetLODPolygonCount (float /*lod*/) const
    {
      return 0;
    }
    virtual void GetLOD(iSharedVariable *& first,iSharedVariable *& second) const
    {
      first=0; second=0;
    }
  } scfiLODControl;
  friend struct LODControl;

};

/**
 * A Cal3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames (a frame may be controlled by
 * a skeleton).
 */
class csSpriteCal3DMeshObject : public iMeshObject
{
private:
  iMeshObjectDrawCallback* vis_cb;
  uint32 current_features;  // LOD Control thing
  iBase* logparent;
  CalModel calModel;
  float last_update_time;
  csArray<csCal3DAnimation*> active_anims;

#ifndef CS_USE_NEW_RENDERER
  iVertexBufferManager* vbufmgr;
  csRef<iVertexBuffer> vbuf;
//  csArray<G3DTriangleMesh>  mesh;
//  csArray<bool>             initialized;
//  csArray<csColor*>         mesh_colors;
  bool arrays_initialized;
  csArray<int>		     attached_ids;
  csArray<G3DTriangleMesh>  *meshes;
  csArray<bool>             *is_initialized;
  csArray<csColor*>         *meshes_colors;
  csBox3 object_bbox;

#endif

  void SetupObject ();
  void SetupObjectSubmesh(int index);
  void SetupVertexBuffer (int mesh,int submesh,int num_vertices,int num_triangles,csTriangle *triangles);
  bool DrawSubmesh (iGraphics3D* g3d,iRenderView* rview,CalRenderer *pCalRenderer,int mesh,int submesh,iMaterialWrapper *material);
  void UpdateLightingSubmesh (iLight** lights, int num_lights,iMovable* movable,CalRenderer *pCalRenderer,int mesh, int submesh);

public:
  SCF_DECLARE_IBASE;

  /// The parent.
  csSpriteCal3DMeshObjectFactory* factory;

  /// Constructor.
  csSpriteCal3DMeshObject (iBase *pParent, CalCoreModel& calCoreModel);
  /// Destructor.
  virtual ~csSpriteCal3DMeshObject ();

  /// Set the factory.
  void SetFactory (csSpriteCal3DMeshObjectFactory* factory);

  /// Get the factory.
  csSpriteCal3DMeshObjectFactory* GetFactory3D () const { return factory; }


  ///------------------------ iMeshObject implementation ----------------------
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
      csVector3& intersect, float* pr) { return false; }
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
      csVector3& intersect, float* pr) { return false; }

  virtual bool SetColor (const csColor& col)
  {
    //SetBaseColor (col);
    return true;
  }
  virtual bool GetColor (csColor& col) const
  {
    //GetBaseColor (col);
    return true;
  }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat)
  {
    //SetMaterial (mat);
    return true;
  }
  virtual iMaterialWrapper* GetMaterialWrapper () const
  {
    return 0; //GetMaterial ();
  }
  virtual int GetPortalCount () const { return 0; }
  virtual iPortal* GetPortal (int) const { return 0; }


  virtual iMeshObjectFactory* GetFactory () const
  {
    csRef<iMeshObjectFactory> ifact (SCF_QUERY_INTERFACE (factory,
    	iMeshObjectFactory));
    return ifact;	// DecRef is ok here.
  }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual csRenderMesh **GetRenderMeshes (int &) { return 0; }
  virtual void UpdateLighting (iLight** lights, int num_lights,
      iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable, csZBufMode mode);
  virtual csRenderMesh **GetRenderMeshes (int &n);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    if (cb) cb->IncRef ();
    if (vis_cb) vis_cb->DecRef ();
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }
  virtual bool Advance (csTicks current_time);

  virtual void NextFrame (csTicks current_time,const csVector3& new_pos)
  {   
    Advance (current_time);
  }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);

    /// Get the number of vertices for this mesh.
    virtual int GetVertexCount ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertexCount ();
    }
    /// Get the pointer to the array of vertices.
    virtual csVector3* GetVertices ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertices ();
    }
    /// Get the number of polygons for this mesh.
    virtual int GetPolygonCount ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetTriangleCount ();
    }

    /// Get the pointer to the array of polygons.
    virtual csMeshedPolygon* GetPolygons ();

    /// Cleanup.
    virtual void Cleanup () { delete[] polygons; polygons = 0; }
    
    virtual bool IsDeformable () const { return false;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh () : polygons (0) { }
    virtual ~PolyMesh () { Cleanup (); }

    csMeshedPolygon* polygons;
  } scfiPolygonMesh;
  friend struct PolyMesh;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }

  //--------------------- iSpriteCal3DState implementation -------------//
  int GetAnimCount();
  const char *GetAnimName(int idx);
  int  GetAnimType(int idx);
  int  FindAnim(const char *name);
  void ClearAllAnims();
  bool SetAnimCycle(const char *name, float weight);
  bool AddAnimCycle(const char *name, float weight, float delay);
  void ClearAnimCycle(int idx, float delay);
  bool ClearAnimCycle(const char *name, float delay);
  bool SetAnimAction(const char *name, float delayIn, float delayOut);
  bool SetVelocity(float vel);
  void SetLOD(float lod);
  
  bool AttachCoreMesh(const char *meshname);
  bool AttachCoreMesh(int mesh_id,int iMatWrapID);
  bool DetachCoreMesh(const char *meshname);
  bool DetachCoreMesh(int mesh_id);

  struct SpriteCal3DState : public iSpriteCal3DState
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSpriteCal3DMeshObject);

    virtual int GetAnimCount()
    {
	return scfParent->GetAnimCount();
    }
    virtual const char *GetAnimName(int idx)
    {
	return scfParent->GetAnimName(idx);
    }
    virtual int GetAnimType(int idx)
    {
	return scfParent->GetAnimType(idx);
    }

    virtual void ClearAllAnims()
    {
	scfParent->ClearAllAnims();
    }

    virtual bool SetAnimCycle(const char *name, float weight)
    {
	return scfParent->SetAnimCycle(name,weight);
    }
    virtual bool AddAnimCycle(const char *name, float weight, float delay)
    {
	return scfParent->AddAnimCycle(name,weight,delay);
    }
    virtual bool ClearAnimCycle(const char *name, float delay)
    {
	return scfParent->ClearAnimCycle(name,delay);
    }

    virtual bool SetAnimAction(const char *name, float delayIn, float delayOut)
    {
	return scfParent->SetAnimAction(name,delayIn,delayOut);
    }

    virtual bool SetVelocity(float vel)
    {
	return scfParent->SetVelocity(vel);
    }

    virtual void SetLOD(float lod)
    {
	scfParent->SetLOD(lod);
    }
    virtual bool AttachCoreMesh(const char *meshname)
    {  return scfParent->AttachCoreMesh(meshname); }

    virtual bool AttachCoreMesh(int mesh_id,int iMatWrapID)
    {  return scfParent->AttachCoreMesh(mesh_id,iMatWrapID); }

    virtual bool DetachCoreMesh(const char *meshname)
    {  return scfParent->DetachCoreMesh(meshname); }

    virtual bool DetachCoreMesh(int mesh_id)
    {  return scfParent->DetachCoreMesh(mesh_id); }

  } scfiSpriteCal3DState;
  friend struct SpriteCal3DState;

  //--------------------- iLODControl implementation -------------//
  int GetLODPolygonCount (float lod)
  { return 0; }

  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
    virtual void SetLOD (float, float) {  }
    virtual void GetLOD (float& m, float& a) const { m = 0; a = 1; }
    virtual void SetLOD (iSharedVariable*, iSharedVariable*) {  }
    virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara)
    {
      varm = 0;
      vara = 0;
    }
    virtual int GetLODPolygonCount (float lod) const
    {
      return scfParent->GetLODPolygonCount (lod);
    }
    virtual void GetLOD(iSharedVariable *& first,iSharedVariable *& second) const
    {
      first=0; second=0;
    }
  } scfiLODControl;
  friend struct LODControl;

  /// interface to receive state of vertexbuffermanager
  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
    virtual void ManagerClosing ();
  }scfiVertexBufferManagerClient;
  friend struct eiVertexBufferManagerClient;

  void GetObjectBoundingBox (csBox3& bbox, int type, csVector3 *verts,int vertCount);
  void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  void GetRadius (csVector3& rad, csVector3& cent);

  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
    virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL)
    {
      scfParent->GetObjectBoundingBox (bbox, type);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
  } scfiObjectModel;
  friend class ObjectModel;
};

/**
 * Sprite Cal3D type. This is the plugin you have to use to create instances
 * of csSpriteCal3DMeshObjectFactory.
 */
class csSpriteCal3DMeshObjectType : public iMeshObjectType
{
private:
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  iEngine* engine;

public:
  /// Constructor.
  csSpriteCal3DMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csSpriteCal3DMeshObjectType ();

  bool Initialize (iObjectRegistry* p);

  //------------------------ iMeshObjectType implementation --------------
  SCF_DECLARE_IBASE;

  /// New Factory.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  //------------------- iConfig interface implementation -------------------
  struct csSpriteCal3DConfig : public iConfig
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectType);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option) { return false; }
    virtual bool SetOption (int id, csVariant* value) { return false; }
    virtual bool GetOption (int id, csVariant* value) { return false; }
  } scfiConfig;
  friend struct csSpriteCal3DConfig;

  //--------------------- iComponent interface implementation
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSpriteCal3DMeshObjectType);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;

  //--------------------- iLODControl implementation -------------//
  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectType);
    virtual void SetLOD (float, float)
    {
    }
    virtual void GetLOD (float& m, float& a) const
    {
      m = 0;
      a = 1;
    }
    virtual void SetLOD (iSharedVariable*, iSharedVariable*)
    {
    }
    virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara)
    {
      varm = 0;
      vara = 0;
    }
    virtual int GetLODPolygonCount (float /*lod*/) const
    {
      return 0;
    }
    virtual void GetLOD(iSharedVariable *& first,iSharedVariable *& second) const
    {
      first=0; second=0;
    }
  } scfiLODControl;
  friend struct LODControl;
};

#endif // __CS_SPRCAL3D_H__
