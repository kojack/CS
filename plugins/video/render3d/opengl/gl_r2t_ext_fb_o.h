/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#ifndef __CS_GL_R2T_EXT_FB_O_H__
#define __CS_GL_R2T_EXT_FB_O_H__

#include "gl_r2t_framebuf.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

class csGLRender2TextureEXTfbo : public csGLRender2TextureFramebuf
{
  bool enableFBO;
  GLuint framebuffer, depthRB, stencilRB;
  int fb_w, fb_h;
  iTextureHandle* txthandle;
  csString fboMsg;

  void FreeBuffers();
  const char* FBStatusStr (GLenum status);
public:
  csGLRender2TextureEXTfbo (csGLGraphics3D* G3D) 
    : csGLRender2TextureFramebuf (G3D), enableFBO(true), framebuffer (0),
      depthRB(0), stencilRB(0), fb_w(-1), fb_h(-1), txthandle(0) { }
  virtual ~csGLRender2TextureEXTfbo();

  virtual void SetRenderTarget (iTextureHandle* handle, bool persistent,
  	int subtexture);
  virtual void FinishDraw ();
};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_R2T_EXT_FB_O_H__

