/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "sysdef.h"
#include "csengine/dumper.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/bsp.h"
#include "csgeom/polyclip.h"
#include "csgeom/frustrum.h"
#include "csengine/sysitf.h"
#include "csengine/camera.h"
#include "csengine/polyplan.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/polyset.h"
#include "csengine/triangle.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/cssprite.h"
#include "csengine/light.h"
#include "csengine/lghtmap.h"
#include "csobject/nameobj.h"

void Dumper::dump (csMatrix3* m, char* name)
{
  CsPrintf (MSG_DEBUG_0, "Matrix '%s':\n", name);
  CsPrintf (MSG_DEBUG_0, "/\n");
  CsPrintf (MSG_DEBUG_0, "| %3.2f %3.2f %3.2f\n", m->m11, m->m12, m->m13);
  CsPrintf (MSG_DEBUG_0, "| %3.2f %3.2f %3.2f\n", m->m21, m->m22, m->m23);
  CsPrintf (MSG_DEBUG_0, "| %3.2f %3.2f %3.2f\n", m->m31, m->m32, m->m33);
  CsPrintf (MSG_DEBUG_0, "\\\n");
}

void Dumper::dump (csVector3* v, char* name)
{
  CsPrintf (MSG_DEBUG_0, "Vector '%s': (%f,%f,%f)\n", name, v->x, v->y, v->z);
}

void Dumper::dump (csVector2* v, char* name)
{
  CsPrintf (MSG_DEBUG_0, "Vector '%s': (%f,%f)\n", name, v->x, v->y);
}

void Dumper::dump (csPlane* p)
{
  CsPrintf (MSG_DEBUG_0, "A=%2.2f B=%2.2f C=%2.2f D=%2.2f\n",
            p->norm.x, p->norm.y, p->norm.z, p->DD);
}

void Dumper::dump (csBox* b)
{
  CsPrintf (MSG_DEBUG_0, "(%2.2f,%2.2f)-(%2.2f,%2.2f)",
  	b->MinX (), b->MinY (), b->MaxX (), b->MaxY ());
}

void Dumper::dump (csCamera* c)
{
  CsPrintf (MSG_DEBUG_0, "csSector: %s\n", 
            c->sector ? csNameObject::GetName(*(c->sector)) : "(?)");
  dump (&c->v_o2t, "Camera vector");
  dump (&c->m_o2t, "Camera matrix");
}

void Dumper::dump (csPolyPlane* p)
{
  const char* pl_name = csNameObject::GetName (*p);
  CsPrintf (MSG_DEBUG_0, "PolyPlane '%s' id=%ld:\n", pl_name ? pl_name : "(no name)",
            p->GetID ());
  dump (&p->m_obj2tex, "Mot");
  dump (&p->v_obj2tex, "Vot");
  dump (&p->m_world2tex, "Mwt");
  dump (&p->v_world2tex, "Vwt");
  dump (&p->m_cam2tex, "Mct");
  dump (&p->v_cam2tex, "Vct");
  CsPrintf (MSG_DEBUG_0, "    Plane normal (object): "); dump (&p->plane_obj);
  CsPrintf (MSG_DEBUG_0, "    Plane normal (world):  "); dump (&p->plane_wor);
  CsPrintf (MSG_DEBUG_0, "    Plane normal (camera): "); dump (&p->plane_cam);
}

void Dumper::dump (csPolygon3D* p)
{
  csPolygonSet* ps = (csPolygonSet*)p->GetParent ();
  CsPrintf (MSG_DEBUG_0, "Dump polygon '%s/%s' id=%ld:\n", csNameObject::GetName(*ps),
  	csNameObject::GetName(*p), p->GetID ());
  CsPrintf (MSG_DEBUG_0, "    num_vertices=%d  max_vertices=%d\n", p->num_vertices, p->max_vertices);
  if (p->portal)
  {
    csPortal* portal = p->portal;
    CsPrintf (MSG_DEBUG_0, "    Polygon is a CS portal to sector '%s'.\n", 
                csNameObject::GetName(*(portal->GetSector())) );
    CsPrintf (MSG_DEBUG_0, "    Alpha=%d\n", p->GetAlpha ());
  }
  int i;
  for (i = 0 ; i < p->num_vertices ; i++)
    CsPrintf (MSG_DEBUG_0, "        v[%d]: (%f,%f,%f)  camera:(%f,%f,%f)\n",
    	i,
    	p->Vwor (i).x, p->Vwor (i).y, p->Vwor (i).z,
    	p->Vcam (i).x, p->Vcam (i).y, p->Vcam (i).z);
  dump (p->plane);
  if (p->tex) dump (p->tex, "PolyTexture 1");
  if (p->tex1) dump (p->tex1, "PolyTexture 2");
  if (p->tex2) dump (p->tex2, "PolyTexture 3");
  if (p->tex3) dump (p->tex3, "PolyTexture 4");

  csLightPatch* lp = p->lightpatches;
  while (lp)
  {
    CsPrintf (MSG_DEBUG_0, "  LightPatch (num_vertices=%d, light=%08lx)\n", lp->num_vertices,
    	lp->GetLight ());
    lp = lp->GetNextPoly ();
  }
}

void Dumper::dump (csPolygonSet* p)
{
  CsPrintf (MSG_DEBUG_0, "========================================================\n");
  CsPrintf (MSG_DEBUG_0, "Dump sector '%s' id=%ld:\n", 
            csNameObject::GetName(*p), p->GetID ());
  CsPrintf (MSG_DEBUG_0, "    num_vertices=%d max_vertices=%d num_polygon=%d max_polygon=%d\n",
	p->num_vertices, p->max_vertices, p->num_polygon, p->max_polygon);
  CsPrintf (MSG_DEBUG_0, "This sector %s a BSP.\n", p->bsp ? "uses" : "doesn't use");
  int i;

  CsPrintf (MSG_DEBUG_0, "  Vertices:\n");
  for (i = 0 ; i < p->num_vertices ; i++)
  {
    CsPrintf (MSG_DEBUG_0, "    Vertex[%d]=\n", i);
    dump (&p->wor_verts[i], "world");
    dump (&p->obj_verts[i], "object");
    //dump (&p->cam_verts[i], "camera");
    CsPrintf (MSG_DEBUG_0, "\n");
  }
  CsPrintf (MSG_DEBUG_0, "  Polygons:\n");
  for (i = 0 ; i < p->num_polygon ; i++)
  {
    CsPrintf (MSG_DEBUG_0, "------------------------------------------\n");
    csPolygon3D* pp = (csPolygon3D*)p->polygons[i];
    dump (pp);
  }
}

void Dumper::dump (csSector* s)
{
  dump ((csPolygonSet*)s);
  int i;
  for (i = 0 ; i < s->sprites.Length () ; i++)
  {
    csSprite3D* sp3d = (csSprite3D*)s->sprites[i];
    dump (sp3d);
  }
}

void Dumper::dump (csWorld* w)
{
  int sn = w->sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* s = (csSector*)w->sectors[sn];
    dump (s);
  }
}

void Dumper::dump (csSpriteTemplate* s)
{
  CsPrintf (MSG_DEBUG_0, "Dump sprite template '%s' id=%ld:\n", 
            csNameObject::GetName(*s), s->GetID ());
  CsPrintf (MSG_DEBUG_0, "%d vertices.\n", s->num_vertices);
  CsPrintf (MSG_DEBUG_0, "%d triangles.\n", s->GetBaseMesh ()->GetNumTriangles ());
  CsPrintf (MSG_DEBUG_0, "%d frames.\n", s->GetNumFrames ());
  CsPrintf (MSG_DEBUG_0, "%d actions.\n", s->GetNumActions ());
}
 
void Dumper::dump (csSprite3D* s)
{
  CsPrintf (MSG_DEBUG_0, "Dump sprite '%s' id=%ld:\n", 
            csNameObject::GetName(*s), s->GetID ());
  dump (s->tpl);
  dump (&s->m_obj2world, "Object->world");
  dump (&s->v_obj2world, "Object->world");
#if 0
  int i;
  CsPrintf (MSG_DEBUG_0, "Last transformed frame:\n");
  for (i = 0 ; i < s->tpl->num_vertices ; i++)
    CsPrintf (MSG_DEBUG_0, "  V%d: tex:(%f,%f) cam:(%f,%f,%f) scr:(%f,%f) vis:%d\n",
    	i, s->tr_frame->GetTexel (i).x, s->tr_frame->GetTexel (i).y,
    	s->tr_frame->GetVertex (i).x, s->tr_frame->GetVertex (i).y, s->tr_frame->GetVertex (i).z,
	s->persp[i].x, s->persp[i].y, s->visible[i]);
#endif
}

void Dumper::dump (csPolyTexture* p, char* name)
{
  CsPrintf (MSG_DEBUG_0, "  PolyTexture '%s'\n", name);
  //@@@CsPrintf (MSG_DEBUG_0, "    in_cache=%d dyn_dirty=%d\n", in_cache, dyn_dirty);
  CsPrintf (MSG_DEBUG_0, "    dirty_size=%d=%dx%d dirty_cnt=%d\n", p->dirty_size, p->dirty_w, p->dirty_h, p->dirty_cnt);
  if (p->dirty_matrix)
  {
    CsPrintf (MSG_DEBUG_0, "    dirty_matrix=[\n");
    int i, j;
    for (i = 0 ; i < p->dirty_h ; i++)
    {
      CsPrintf (MSG_DEBUG_0, "        ");
      for (j = 0 ; j < p->dirty_w ; j++)
        CsPrintf (MSG_DEBUG_0, "%d ", p->dirty_matrix[p->dirty_w*i + j]);
      CsPrintf (MSG_DEBUG_0, "\n");
    }
    CsPrintf (MSG_DEBUG_0, "    ]\n");
  }
  else CsPrintf (MSG_DEBUG_0, "    dirty_matrix=NULL\n");
  CsPrintf (MSG_DEBUG_0, "    Imin_u=%d Imin_v=%d Imax_u=%d Imax_v=%d\n", p->Imin_u, p->Imin_v, p->Imax_u, p->Imax_v);
  CsPrintf (MSG_DEBUG_0, "    Fmin_u=%f Fmin_v=%f Fmax_u=%f Fmax_v=%f\n", p->Fmin_u, p->Fmin_v, p->Fmax_u, p->Fmax_v);
  if (p->lm)
    CsPrintf (MSG_DEBUG_0, "    lm: lwidth=%d lheight=%d rwidth=%d rheight=%d lm_size=%d\n", p->lm->lwidth,
  	p->lm->lheight, p->lm->rwidth, p->lm->rheight, p->lm->lm_size);
  else
    CsPrintf (MSG_DEBUG_0, "    lm: no lightmap\n");
  CsPrintf (MSG_DEBUG_0, "    shf_u=%d and_u=%d\n", p->shf_u, p->and_u);
  CsPrintf (MSG_DEBUG_0, "    du=%d dv=%d fdu=%f fdv=%f\n", p->du, p->dv, p->fdu, p->fdv);
  CsPrintf (MSG_DEBUG_0, "    w=%d h=%d w_orig=%d size=%d\n", p->w, p->h, p->w_orig, p->size);
  CsPrintf (MSG_DEBUG_0, "    mipmap_size=%d mipmap_shift=%d\n", p->mipmap_size, p->mipmap_shift);
}

void Dumper::dump (csPolygon2D* p, char* name)
{
  CsPrintf (MSG_DEBUG_0, "Dump polygon 2D '%s':\n", name);
  CsPrintf (MSG_DEBUG_0, "    num_vertices=%d  max_vertices=%d\n", p->num_vertices, p->max_vertices);
  int i;
  for (i = 0 ; i < p->num_vertices ; i++)
    CsPrintf (MSG_DEBUG_0, "        v[%d]: (%f,%f)\n", i, p->vertices[i].x, p->vertices[i].y);
}

void Dumper::dump (csBspTree* tree, csBspNode* node, int indent)
{
  if (!node) return;
  int i;
  char spaces[256];
  for (i = 0 ; i < indent ; i++) spaces[i] = ' ';
  spaces[indent] = 0;
  CsPrintf (MSG_DEBUG_0, "%sThere are %d polygons in this node.\n", spaces, node->num);
  CsPrintf (MSG_DEBUG_0, "%s Front:\n", spaces);
  dump (tree, node->front, indent+2);
  CsPrintf (MSG_DEBUG_0, "%s Back:\n", spaces);
  dump (tree, node->back, indent+2);
}

void Dumper::dump (csBspTree* tree)
{
  dump (tree, tree->root, 0);
}

void Dumper::dump (csPolygonClipper* clipper, char* name)
{
  CsPrintf (MSG_DEBUG_0, "PolygonClipper '%s'\n", name);
  int i;
  for (i = 0 ; i < clipper->ClipPolyVertices ; i++)
    CsPrintf (MSG_DEBUG_0, "  %d: (%f,%f)\n", i, clipper->ClipPoly[i].x, clipper->ClipPoly[i].y);
}

void Dumper::dump (csFrustrum* frustrum, char* name)
{
  CsPrintf (MSG_DEBUG_0, "csFrustrum '%s'\n", name);
  if (!frustrum)
  {
    CsPrintf (MSG_DEBUG_0, "  NULL\n");
    return;
  }
  if (frustrum->IsEmpty ())
  {
    CsPrintf (MSG_DEBUG_0, "  EMPTY\n");
    return;
  }
  if (frustrum->IsInfinite ())
  {
    CsPrintf (MSG_DEBUG_0, "  INFINITE\n");
    return;
  }
  int i;
  CsPrintf (MSG_DEBUG_0, "  "); dump (&frustrum->GetOrigin (), "origin");
  for (i = 0 ; i < frustrum->GetNumVertices () ; i++)
  {
    CsPrintf (MSG_DEBUG_0, "  ");
    char buf[20];
    sprintf (buf, "[%d]", i);
    dump (&frustrum->GetVertex (i), buf);
  }
  if (frustrum->GetBackPlane ())
  {
    CsPrintf (MSG_DEBUG_0, "  ");
    dump (frustrum->GetBackPlane ());
  }
}

