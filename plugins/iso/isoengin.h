/*
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
*/

#ifndef __ISOENGIN_H__
#define __ISOENGIN_H__

#include "ivaria/iso.h"

/**
 *
*/
class csIsoEngine : public iIsoEngine {
private:
  /// the system
  iSystem* system;
  /// 2d canvas
  iGraphics2D* g2d;
  /// 3d renderer
  iGraphics3D* g3d;
  /// texturemanager
  iTextureManager* txtmgr;

  /// current world
  iIsoWorld *world;

public:
  DECLARE_IBASE;

  /// Create engine
  csIsoEngine (iBase *iParent);
  /// Destroy engine
  virtual ~csIsoEngine ();

  //----- iPlugin ------------------------------------------------------
  /// For the system to initialize the plugin, and return success status
  virtual bool Initialize (iSystem* p);
  /// Intercept events
  virtual bool HandleEvent (iEvent& e);

  //----- iIsoEngine ---------------------------------------------------
  virtual iSystem* GetSystem() const {return system;}
  virtual iGraphics2D* GetG2D() const {return g2d;}
  virtual iGraphics3D* GetG3D() const {return g3d;}
  virtual iTextureManager* GetTextureManager() const {return txtmgr;}
  virtual iIsoWorld* CreateWorld();
  virtual void SetCurrentWorld(iIsoWorld *world) {csIsoEngine::world=world;}
  virtual iIsoWorld *GetCurrentWorld() const {return world;}
  virtual iIsoView* CreateView(iIsoWorld *world);
  virtual iIsoLight* CreateLight();
  virtual iIsoSprite* CreateSprite();
  virtual int GetBeginDrawFlags () const;
  virtual iIsoSprite* CreateFloorSprite(const csVector3& pos, float w, 
    float h);
  virtual iIsoSprite* CreateFrontSprite(const csVector3& pos, float w, 
    float h);

};

#endif
