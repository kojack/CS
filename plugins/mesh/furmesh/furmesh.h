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

#ifndef __FUR_MESH_H__
#define __FUR_MESH_H__

#include <imesh/furmesh.h>

#include "furmeshfactory.h"
#include "furdata.h"

#include "crystalspace.h"
#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  class FurMesh : public scfImplementationExt1<FurMesh, csMeshObject, CS::Mesh::iFurMesh>
  {
  public:
    CS_LEAKGUARD_DECLARE(FurMesh);
    FurMesh (iEngine* engine, iObjectRegistry* object_reg, 
      iMeshObjectFactory* object_factory);
    virtual ~FurMesh ();

    // From iMeshObject
    virtual iMeshObjectFactory* GetFactory () const;

    virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
      csVector3& isect, float* pr, int* polygon_idx,
      iMaterialWrapper** baseMaterial, iMaterialArray* materials);

    virtual void NextFrame (csTicks current_time,const csVector3& pos,
      uint currentFrame);
    
    virtual bool SetMaterialWrapper (iMaterialWrapper* );

    virtual iMaterialWrapper* GetMaterialWrapper () const;

    virtual void UpdateObjectBoundingBox();

    virtual CS::Graphics::RenderMesh** GetRenderMeshes (int& num, iRenderView*, 
      iMovable*, uint32);

    /// some mesh properties
    virtual void SetMixMode (uint mode);
    virtual uint GetMixMode () const;
    virtual void SetPriority (uint priority);
    virtual uint GetPriority () const;
    virtual void SetZBufMode (csZBufMode z_buf_mode);
    virtual csZBufMode GetZBufMode () const;
    virtual void SetIndexRange (uint indexstart, uint indexend);
    
    // From iFurMesh
    virtual void GenerateGeometry (iView* view, iSector *room);
    virtual void SetGuideLOD(float guideLOD);
    virtual void SetStrandLOD(float strandLOD);
    virtual void SetLOD(float lod);

    virtual void SetPhysicsControl (CS::Mesh::iFurPhysicsControl* physicsControl);
    virtual void StartPhysicsControl ( );
    virtual void StopPhysicsControl ( );

    // Temporary - Set Mesh and Submesh
    virtual void SetMeshFactory ( CS::Mesh::iAnimatedMeshFactory* meshFactory);
    virtual void SetMeshFactorySubMesh ( CS::Mesh::iAnimatedMeshSubMeshFactory* 
      meshFactorySubMesh );
    // Set Material
    virtual void SetBaseMaterial ( iMaterial* baseMaterial );
    // Set HairStrandGenerator
    virtual void SetFurStrandGenerator( CS::Mesh::iFurStrandGenerator* hairStrandGenerator);
    // Get HairStrandGenerator
    virtual CS::Mesh::iFurStrandGenerator* GetFurStrandGenerator( ) const;

  private:
    csRef<iMaterialWrapper> materialWrapper;
    csDirtyAccessArray<csRenderMesh*> renderMeshes;
    iObjectRegistry* object_reg;
    iMeshObjectFactory* object_factory;
    csRef<CS::Mesh::iFurMeshFactory> factory;
    /// Model
    iEngine* engine;
    /// Fur geometry
    csRef<iView> view;
    csArray<csHairStrand> hairStrands;
    csArray<csGuideHair> guideHairs;
    csArray<csTriangle> guideHairsTriangles;
    csArray<csGuideHairLOD> guideHairsLOD;
    csRef<CS::Mesh::iFurPhysicsControl> physicsControl;
    csRef<CS::Mesh::iFurStrandGenerator> hairStrandGenerator;
    csVector3* positionShift;
    csRandomGen *rng;
    float guideLOD;
    float strandLOD;
    size_t hairStrandsLODSize;
    bool physicsControlEnabled;
    /// Temp fur geometry
    csRef<CS::Mesh::iAnimatedMeshFactory> meshFactory;
    csRef<CS::Mesh::iAnimatedMeshSubMeshFactory> meshFactorySubMesh;
    csRef<iMaterial> baseMaterial;
    csRef<iShaderVarStringSet> svStrings;
    /// Density & Height maps
    csTextureRGBA densitymap;
    float densityFactorGuideHairs;
    float densityFactorHairStrands;
    csTextureRGBA heightmap;
    float heightFactor;
    float displaceDistance;
    float strandWidth;
    float strandWidthLOD;
    float controlPointsDistance;
    float positionDeviation;
    int growTangents;
    /// render mesh data
    csRef<csRenderBufferHolder> bufferholder;
    csRef<csShaderVariableContext> svContext;
    uint mixmode;
    uint priority;
    csZBufMode z_buf_mode;
    uint indexstart, indexend;
    /// functions
    void SetRigidBody (iRigidBody* rigidBody);
    void GenerateGuideHairs();
    void SynchronizeGuideHairs();
    void GenerateGuideHairsLOD();
    void GenerateHairStrands();
    void TriangleAreaDensity(const csTriangle& triangle, float &area, float &density,
      csGuideHair& A, csGuideHair& B, csGuideHair& C);
    /// debug
    void SaveUVImage();
    /// setters
    void SetBaseMaterialProperties();
    /// update
    void Update();
    void UpdateGuideHairs();
  };

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __FUR_MESH_H__
