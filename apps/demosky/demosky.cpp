/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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


    demosky - This application shows a (slow) way to produce a sky using
    procedural textures and some fractal algorithms
*/

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "apps/demosky/demosky.h"
#include "cstool/proctex.h"
#include "cstool/prsky.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivaria/conout.h"
#include "imesh/sprite2d.h"
#include "imesh/object.h"
#include "imap/parser.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "ivaria/reporter.h"
#include "igraphic/imageio.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "iutil/vfs.h"
#include "csutil/event.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// the global system driver variable
DemoSky *System;

void DemoSky::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.application.demosky", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

DemoSky::DemoSky ()
{
  sky = 0;
  sky_f = 0;
  sky_b = 0;
  sky_l = 0;
  sky_r = 0;
  sky_u = 0;
  sky_d = 0;
  flock = 0;
}

DemoSky::~DemoSky ()
{
  delete flock;
  delete sky;
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  iObjectRegistry* object_reg = System->object_reg;
  delete System; System = 0;
  csInitializer::DestroyApplication (object_reg);
}

void DemoSky::SetTexSpace(csProcSkyTexture *skytex,
  iThingFactoryState *walls_state,
  int size, const csVector3& orig, const csVector3& upt, float ulen,
  const csVector3& vpt, float vlen)
{
  csVector3 texorig = orig;
  csVector3 texu = upt;
  float texulen = ulen;
  csVector3 texv = vpt;
  float texvlen = vlen;
  /// copied, now adjust
  csVector3 uvector = upt - orig;
  csVector3 vvector = vpt - orig;
  /// to have 1 pixel going over the edges.
  texorig -= uvector / float(size);
  texorig -= vvector / float(size);
  texu += uvector / float(size);
  texv += vvector / float(size);
  texulen += ulen * 2.0f / float(size);
  texvlen += vlen * 2.0f / float(size);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
  	texorig, texu, texulen, texv, -texvlen);
  skytex->SetTextureSpace(texorig, texu-texorig, texv-texorig);
}

static bool DemoSkyEventHandler (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    System->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    System->FinishFrame ();
    return true;
  }
  else
  {
    return System ? System->HandleEvent (ev) : false;
  }
}

bool DemoSky::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize app!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_CONSOLEOUT,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, DemoSkyEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    exit (-1);
  }

  LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!LevelLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    exit (-1);
  }

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    exit (-1);
  }

  myG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (!myG2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics2D plugin!");
    exit (-1);
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    exit (-1);
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Procedural Sky Demo");
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  // Setup the texture manager
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  font = myG2D->GetFontServer()->LoadFont(CSFONT_LARGE);

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Crystal Space Procedural Sky Demo.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  sky = new csProcSky();
  sky->SetAnimated(object_reg, false);
  sky_f.AttachNew (new csProcSkyTexture(sky));
  iMaterialWrapper* imatf = sky_f->Initialize(object_reg, engine, txtmgr, "sky_f");
  sky_b.AttachNew (new csProcSkyTexture(sky));
  iMaterialWrapper* imatb = sky_b->Initialize(object_reg, engine, txtmgr, "sky_b");
  sky_l.AttachNew (new csProcSkyTexture(sky));
  iMaterialWrapper* imatl = sky_l->Initialize(object_reg, engine, txtmgr, "sky_l");
  sky_r.AttachNew (new csProcSkyTexture(sky));
  iMaterialWrapper* imatr = sky_r->Initialize(object_reg, engine, txtmgr, "sky_r");
  sky_u.AttachNew (new csProcSkyTexture(sky));
  iMaterialWrapper* imatu = sky_u->Initialize(object_reg, engine, txtmgr, "sky_u");
  sky_d.AttachNew (new csProcSkyTexture(sky));
  iMaterialWrapper* imatd = sky_d->Initialize(object_reg, engine, txtmgr, "sky_d");

  room = engine->CreateSector ("room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws =
  	SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();

  int p;
  float size = 500.0; /// size of the skybox -- around 0,0,0 for now.
  float simi = size; //*255./256.; /// sizeminor
  p = walls_state->AddQuad (
  	csVector3 (-size, -simi, size),
  	csVector3 (size, -simi, size),
  	csVector3 (size, -simi, -size),
  	csVector3 (-size, -simi, -size));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, imatd);
  SetTexSpace (sky_d, walls_state, 256, walls_state->GetPolygonVertex  (p, 0),
  	walls_state->GetPolygonVertex  (p, 1), 2.0f * size,
	walls_state->GetPolygonVertex (p, 3), 2.0f * size);
  walls_state->GetPolygonFlags (p).Set(CS_POLY_LIGHTING, 0);

  p = walls_state->AddQuad (
  	csVector3 (-size, simi, -size),
  	csVector3 (size, simi, -size),
	csVector3 (size, simi, size),
	csVector3 (-size, simi, size));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, imatu);
  SetTexSpace (sky_u, walls_state, 256, walls_state->GetPolygonVertex  (p, 0),
  	walls_state->GetPolygonVertex  (p, 1), 2.0f * size,
	walls_state->GetPolygonVertex (p, 3), 2.0f * size);
  walls_state->GetPolygonFlags (p).Set(CS_POLY_LIGHTING, 0);

  p = walls_state->AddQuad (
  	csVector3 (-size, size, simi),
  	csVector3 (size, size, simi),
	csVector3 (size, -size, simi),
	csVector3 (-size, -size, simi));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, imatf);
  SetTexSpace (sky_f, walls_state, 256, walls_state->GetPolygonVertex  (p, 0),
  	walls_state->GetPolygonVertex  (p, 1), 2.0f * size,
	walls_state->GetPolygonVertex (p, 3), 2.0f * size);
  walls_state->GetPolygonFlags (p).Set(CS_POLY_LIGHTING, 0);

  p = walls_state->AddQuad (
  	csVector3 (simi, size, size),
  	csVector3 (simi, size, -size),
	csVector3 (simi, -size, -size),
	csVector3 (simi, -size, size));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, imatr);
  SetTexSpace (sky_r, walls_state, 256, walls_state->GetPolygonVertex  (p, 0),
  	walls_state->GetPolygonVertex  (p, 1), 2.0f * size,
	walls_state->GetPolygonVertex (p, 3), 2.0f * size);
  walls_state->GetPolygonFlags (p).Set(CS_POLY_LIGHTING, 0);

  p = walls_state->AddQuad (
  	csVector3 (-simi, size, -size),
  	csVector3 (-simi, size, size),
	csVector3 (-simi, -size, size),
	csVector3 (-simi, -size, -size));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, imatl);
  SetTexSpace (sky_l, walls_state, 256, walls_state->GetPolygonVertex  (p, 0),
  	walls_state->GetPolygonVertex  (p, 1), 2.0f * size,
	walls_state->GetPolygonVertex (p, 3), 2.0f * size);
  walls_state->GetPolygonFlags (p).Set(CS_POLY_LIGHTING, 0);

  p = walls_state->AddQuad (
  	csVector3 (size, size, -simi),
  	csVector3 (-size, size, -simi),
	csVector3 (-size, -size, -simi),
	csVector3 (size, -size, -simi));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, imatb);
  SetTexSpace (sky_b, walls_state, 256, walls_state->GetPolygonVertex  (p, 0),
  	walls_state->GetPolygonVertex  (p, 1), 2.0f * size,
	walls_state->GetPolygonVertex (p, 3), 2.0f * size);
  walls_state->GetPolygonFlags (p).Set(CS_POLY_LIGHTING, 0);

  LevelLoader->LoadTexture ("seagull", "/lib/std/seagull.gif");
  iMaterialWrapper *sg = engine->GetMaterialList ()->FindByName("seagull");
  flock = new Flock(engine, 10, sg, room);

  engine->Prepare ();

  Report (CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = csPtr<iView> (new csView (engine, myG3D));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
  view->SetRectangle (0, 0, myG2D->GetWidth (), myG2D->GetHeight ());

  return true;
}

void DemoSky::SetupFrame ()
{
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  flock->Update(elapsed_time);

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0f) * (0.03f * 20.0f);

  if (kbd->GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (CS_VEC_FORWARD * 4.0f * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (CS_VEC_BACKWARD * 4.0f * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  view->Draw ();

  // Start drawing 2D graphics.
  if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  const char *text = "Press 't' to toggle animation. Escape quits."
    " Arrow keys/pgup/pgdown to move.";
  int txtx = 10;
  int txty = myG2D->GetHeight() - 20;
  myG2D->Write(font, txtx+1, txty+1, myG2D->FindRGB(80,80,80),
    -1, text);
  myG2D->Write(font, txtx, txty, myG2D->FindRGB(255,255,255),
    -1, text);
}

void DemoSky::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (0);
}

bool DemoSky::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&Event) == 't'))  
  {
    /// toggle animation
    sky->SetAnimated(object_reg, !sky->GetAnimated(), csGetTicks ());
    return true;
  }

  if ((Event.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_ESC))  
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }

  return false;
}

//--- Flock -----------------------
Flock::Flock(iEngine *engine, int num, iMaterialWrapper *mat, iSector *sector)
{
  printf("Creating flock of %d birds\n", num);
  //  mat->IncRef ();
  nr = num;
  spr = new csRef<iMeshWrapper> [nr];
  speed = new csVector3[nr];
  accel = new csVector3[nr];
  int i;
  csRef<iMeshFactoryWrapper> fact (engine->CreateMeshFactory(
    "crystalspace.mesh.object.sprite.2d", "BirdFactory"));
  csRef<iSprite2DFactoryState> state (
  	SCF_QUERY_INTERFACE(fact->GetMeshObjectFactory(),
	iSprite2DFactoryState));
  state->SetMaterialWrapper(mat);
  state->SetLighting(false);

  csVector3 startpos(20,10,20);
  csVector3 pos;

  focus = startpos;
  foc_speed.Set(-5.0f, 0.0f, +5.0f);
  foc_accel.Set(0.0f, 0.0f, 0.0f);

  for(i=0; i<nr; i++)
  {
    pos = startpos;
    speed[i].Set(0.0f, 0.0f, 0.0f);
    speed[i].x = (float(rand() + 1.0f) / float(RAND_MAX)) * 3.0f - 1.5f;
    speed[i].y = (float(rand() + 1.0f) / float(RAND_MAX)) * 1.0f - 0.5f;
    speed[i].z = (float(rand() + 1.0f) / float(RAND_MAX)) * 3.0f - 1.5f;
    speed[i] += foc_speed * 1.0f;
    accel[i].Set(0.0f, 0.0f, 0.0f);
    pos.x += (float(rand() + 1.0f) / float(RAND_MAX)) * 20.0f;
    pos.z -= (float(rand() + 1.0f) / float(RAND_MAX)) * 20.0f;
    spr[i] = engine->CreateMeshWrapper(fact, "Bird", sector, pos);

    csRef<iSprite2DState> sprstate (SCF_QUERY_INTERFACE(spr[i]->GetMeshObject(),
      iSprite2DState));
    sprstate->GetVertices().SetLength(4);
    sprstate->GetVertices()[0].color_init.Set(1.0f, 1.0f, 1.0f);
    sprstate->GetVertices()[1].color_init.Set(1.0f, 1.0f, 1.0f);
    sprstate->GetVertices()[2].color_init.Set(1.0f, 1.0f, 1.0f);
    sprstate->GetVertices()[3].color_init.Set(1.0f, 1.0f, 1.0f);

    float sz = 1.0;
    sprstate->GetVertices()[0].pos.Set(-sz, sz);
    sprstate->GetVertices()[0].u = 0.2f;
    sprstate->GetVertices()[0].v = 0.0f;
    sprstate->GetVertices()[1].pos.Set(+sz, sz);
    sprstate->GetVertices()[1].u = 0.8f;
    sprstate->GetVertices()[1].v = 0.0f;
    sprstate->GetVertices()[2].pos.Set(+sz, -sz);
    sprstate->GetVertices()[2].u = 0.8f;
    sprstate->GetVertices()[2].v = 1.0f;
    sprstate->GetVertices()[3].pos.Set(-sz, -sz);
    sprstate->GetVertices()[3].u = 0.2f;
    sprstate->GetVertices()[3].v = 1.0f;

  }
}


Flock::~Flock()
{
  int i;
  for(i=0; i<nr; i++)
    spr[i] = 0;
  delete[] spr;
  delete[] speed;
  delete[] accel;
}


static void Clamp( float &val, float max)
{
  if(val>max) val=max;
  else if(val<-max) val=-max;
}

void Flock::Update(csTicks elapsed)
{
  float dt = float(elapsed) * 0.001f; /// delta t in seconds
  /// move focus
  /// physics
  int i;
  csVector3 avg(0,0,0);
  for(i=0; i<nr; i++)
    avg += spr[i]->GetMovable()->GetPosition();
  avg /= float(nr);

  foc_accel = (-avg)*0.1 - focus*0.1;
  foc_accel.y = 0;
  foc_speed += foc_accel * dt;
  focus += foc_speed * dt;

  /// move each bird -- going towards focus
  for(i=0; i<nr; i++)
  {
    /// aim to focus
    csVector3 want = focus - spr[i]->GetMovable()->GetPosition();
    accel[i] += want * 0.1;
    float maxaccel = 2.5;
    if(accel[i].SquaredNorm() > maxaccel)
    {
      Clamp(accel[i].x, maxaccel);
      Clamp(accel[i].y, maxaccel / 2.0f);
      Clamp(accel[i].z, maxaccel);
    }
    /// physics
    speed[i] += accel[i] * dt;
    float maxspeed = 10.0f;
    if(accel[i].SquaredNorm() > maxspeed)
    {
      Clamp(speed[i].x, maxspeed);
      Clamp(speed[i].y, maxspeed / 2.0f);
      Clamp(speed[i].z, maxspeed);
    }
    float perturb = 0.1f;
    speed[i].x += (float(rand() + 1.0f) / float(RAND_MAX)) * perturb - perturb * 0.5f;
    speed[i].y += (float(rand() + 1.0f) / float(RAND_MAX)) * perturb - perturb * 0.5f;
    speed[i].z += (float(rand() + 1.0f) / float(RAND_MAX)) * perturb - perturb * 0.5f;
    speed[i].z *= 1.0f + (float(rand() + 1.0f) / float(RAND_MAX)) * 0.2f - 0.1f;
    csVector3 move = speed[i] * dt;
    spr[i]->GetMovable()->MovePosition(move);
    spr[i]->GetMovable()->UpdateMove();
  }
}


/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (0));

  // Create our main class.
  System = new DemoSky ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, 0))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup ();
    exit (1);
  }

  // Main loop.
  csDefaultRunLoop(System->object_reg);

  Cleanup ();

  return 0;
}


