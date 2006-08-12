/*
    Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein

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
#include "csqsqrt.h"
#include "iengine/portal.h"
#include "csutil/debug.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include <csgfx/renderbuffer.h>

#include "material.h"
#include "impmesh.h"
#include "sector.h"
#include "meshobj.h"
#include "light.h"
#include "engine.h"

csImposterMesh::csImposterMesh (csEngine* engine, csMeshWrapper *parent)
{
  parent_mesh = parent;
  tex = new csImposterProcTex (engine, this);
  ready	= false;
  incidence_dist = 0;
  csImposterMesh::engine = engine;

  //Add four to initialise array because of bug in csPoly3D
  cutout.AddVertex(csVector3());
  cutout.AddVertex(csVector3());
  cutout.AddVertex(csVector3());
  cutout.AddVertex(csVector3());
}

float csImposterMesh::CalcIncidenceAngleDist (iRenderView *rview)
{
  // Jorrit, not sure about this correctness but it compiles at least. -Keith

  // Calculate angle of incidence vs. the camera
  iCamera* camera = rview->GetCamera ();
  csReversibleTransform obj = (parent_mesh->GetCsMovable()).csMovable::GetTransform ();
  csReversibleTransform cam = camera->GetTransform ();
  csReversibleTransform seg = obj / cam;  // Matrix Math Magic!
  csVector3 straight(0,0,1);
  csVector3 pt = seg * straight;
  return csSquaredDist::PointPoint (straight, pt);
}

bool csImposterMesh::CheckIncidenceAngle (iRenderView *rview, float tolerance)
{
  float const dist2 = CalcIncidenceAngleDist(rview);
  float diff = dist2 - incidence_dist;
  if (diff < 0) diff = -diff;

  // If not ok, mark for redraw of imposter
  if (diff > tolerance)
  {
    SetImposterReady (false);
    return false;
  }
  return true;
}

void csImposterMesh::FindImposterRectangle (const iCamera* c)
{
  // Called from csImposterProcTex during Anim.
  //  (Recalc of texture causes recalc of imposter poly also.)

  // Rotate camera to look at object directly.
  // Get screen bounding box, modified to also return depth of
  //  point of max width or height in the box.
  // Rotate camera back to original lookat
  // Project screen bounding box, at the returned depth to
  //  the camera transform to rotate it around where we need it
  // Save as csPoly3d for later rendering

  res = parent_mesh->GetScreenBoundingBox (c);

  csVector3 v1 = c->InvPerspective (res.sbox.GetCorner(0), res.distance);
  csVector3 v2 = c->InvPerspective (res.sbox.GetCorner(1), res.distance);
  csVector3 v3 = c->InvPerspective (res.sbox.GetCorner(3), res.distance);
  csVector3 v4 = c->InvPerspective (res.sbox.GetCorner(2), res.distance);

v1 = c->GetTransform ().This2Other (v1);
v2 = c->GetTransform ().This2Other (v2);
v3 = c->GetTransform ().This2Other (v3);
v4 = c->GetTransform ().This2Other (v4);

/*
  csVector3 v1 = c->InvPerspective (csVector2(0,0), 3);
  csVector3 v2 = c->InvPerspective (csVector2(0,200), 3);
  csVector3 v3 = c->InvPerspective (csVector2(200,200), 3);
  csVector3 v4 = c->InvPerspective (csVector2(200,0), 3);

v1 = c->GetTransform ().This2Other (v1);
v2 = c->GetTransform ().This2Other (v2);
v3 = c->GetTransform ().This2Other (v3);
v4 = c->GetTransform ().This2Other (v4);

printf("Min: %f %f %f\n",v1[0],v1[1],v1[2]);
printf("Max: %f %f %f\n",v2[0],v2[1],v2[2]);
printf("Max: %f %f %f\n",v3[0],v3[1],v3[2]);
printf("Max: %f %f %f\n",v4[0],v4[1],v4[2]);
*/

  cutout[0] = v1;
  cutout[1] = v2;
  cutout[2] = v3;
  cutout[3] = v4;
}


static bool mesh_init = false;

CS_IMPLEMENT_STATIC_VAR (GetMeshIndices, csDirtyAccessArray<uint>, ());
static size_t mesh_indices_count = 0;
CS_IMPLEMENT_STATIC_VAR (GetMeshVertices, 
  csDirtyAccessArray<csVector3>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshTexels, 
  csDirtyAccessArray<csVector2>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshColors, 
  csDirtyAccessArray<csVector4>, ());

csRenderMesh** csImposterMesh::GetRenderMesh(iRenderView *rview)
{
  bool rmCreated;
  csRenderMesh*& mesh = rmHolder.GetUnusedMesh (rmCreated,
    rview->GetCurrentFrameNumber ());

  if (rmCreated)
  {
    printf("impostermesh init\n");
    mesh_init = true;
    mesh->meshtype = CS_MESHTYPE_TRIANGLES;
    mesh->mixmode = CS_FX_ALPHA;
    mesh->z_buf_mode = CS_ZBUF_TEST;
  }
  mesh_indices_count = 0;
  GetMeshVertices ()->Empty ();
  GetMeshTexels ()->Empty ();
  GetMeshColors ()->Empty ();

//  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
  iMaterialWrapper* tm = engine->CreateMaterial ("test", tex->GetTexture ());
  
if (tm == 0) printf ("Uuups\n");
  tm->Visit();
  mesh->material = tm;


  csDirtyAccessArray<uint>& mesh_indices = *GetMeshIndices ();
  csDirtyAccessArray<csVector2>& mesh_texels = *GetMeshTexels ();
  csDirtyAccessArray<csVector4>& mesh_colors = *GetMeshColors ();
  
  mesh_indices.Put (0, 0);
  mesh_indices.Put (1, 1);
  mesh_indices.Put (2, 2);
  mesh_indices.Put (3, 2);
  mesh_indices.Put (4, 3);
  mesh_indices.Put (5, 0);
  mesh_indices_count += 6;

  mesh->indexstart = 0;
  mesh->indexend = mesh_indices_count;

  mesh->object2world = csReversibleTransform ();

  mesh_texels.Push (csVector2 (0,0));
  mesh_texels.Push (csVector2 (1,0));
  mesh_texels.Push (csVector2 (1,1));
  mesh_texels.Push (csVector2 (0,1));

  csVector4 c (1, 1, 1, 1.0);
  mesh_colors.Push (c);
  mesh_colors.Push (c);
  mesh_colors.Push (c);
  mesh_colors.Push (c);

  csRef<csRenderBuffer> indexBuffer = 
    csRenderBuffer::CreateIndexRenderBuffer(
    mesh_indices_count, CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, 3);
  indexBuffer->CopyInto(mesh_indices.GetArray(), mesh_indices_count);

  csRef<csRenderBuffer> vertBuffer = csRenderBuffer::CreateRenderBuffer(
    4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
  vertBuffer->CopyInto(cutout.GetVertices (), 4);

  csRef<csRenderBuffer> texBuffer = csRenderBuffer::CreateRenderBuffer(
    4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
  texBuffer->CopyInto(mesh_texels.GetArray(), 4);

  csRef<csRenderBuffer> colBuffer = csRenderBuffer::CreateRenderBuffer(
    4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
  colBuffer->CopyInto(mesh_colors.GetArray(), 4);

  csRef<csRenderBufferHolder> buffer = new csRenderBufferHolder();
  buffer->SetRenderBuffer (CS_BUFFER_INDEX, indexBuffer);
  buffer->SetRenderBuffer (CS_BUFFER_POSITION, vertBuffer);
  buffer->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texBuffer);
  buffer->SetRenderBuffer (CS_BUFFER_COLOR, colBuffer);
  mesh->buffers = buffer;
  mesh->variablecontext = new csShaderVariableContext();

  return &mesh;
}
