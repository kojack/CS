/*
Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles

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

// g3d_d3d.h
// csGraphics3DDirect3DDx6 Class Declaration
// Written by dan
// Some modifications by Nathaniel

// Ported to COM by Dan Ogles on 8.26.98

#ifndef G3D_D3D_H
#define G3D_D3D_H

#include <windows.h>
#include "ddraw.h"
#include "d3d.h"
#include "d3dcaps.h"

#include "cs3d/direct3d6/d3dcache.h"
#include "csutil/scf.h"
#include "cssys/win32/IDDetect.h"
#include "igraph3d.h"
#include "ihalo.h"
#include "iplugin.h"
#include "ipolygon.h"

//DIRECT3D DRIVER HRESULTS/////////////////////////////

#define CSD3DERR_ERRORSTARTINGDIRECTDRAW    MAKE_CSHRESULT(0x0000)
#define CSD3DERR_WRONGVERSION               MAKE_CSHRESULT(0x5001)
#define CSD3DERR_NOPRIMARYSURFACE           MAKE_CSHRESULT(0x5002)
#define CSD3DERR_NOZBUFFER                  MAKE_CSHRESULT(0x5003)
#define CSD3DERR_CANNOTATTACH               MAKE_CSHRESULT(0x5004)
#define CSD3DERR_NOCOMPATIBLESURFACEDESC    MAKE_CSHRESULT(0x5005)
#define CSD3DERR_ERRORCREATINGMATERIAL      MAKE_CSHRESULT(0x5006)
#define CSD3DERR_ERRORCREATINGVIEWPORT      MAKE_CSHRESULT(0x5007)
#define CSD3DERR_ERRORCLEARINGBUFFER        MAKE_CSHRESULT(0x5008)
#define CSD3DERR_ERRORBEGINSCENE            MAKE_CSHRESULT(0x5009)
#define CSD3DERR_ERRORENDSCENE              MAKE_CSHRESULT(0x500A)
#define CSD3DERR_INTERNALERROR              MAKE_CSHRESULT(0x000B)

//DIRECT3D SPECIFIC STRUCTS///////////////////////////

class D3DTextureCache;
class D3DLightMapCache;

/// a fan of triangles.
struct D3D_TriangleFan
{
  int *vert;
  int num_tri;
  struct D3DTextureCache_Data *tex;
  struct D3DLightCache_Data *lit;
};

/// the Direct3D implementation of the Graphics3D class.
class csGraphics3DDirect3DDx6 : public iGraphics3D, public iHaloRasterizer
{
  /// Pointer to DirectDraw class
  IDirectDraw4* m_lpDD;
  /// Primary Surface
  IDirectDrawSurface4* m_lpddPrimary;
  /// Offscreen Surface
  IDirectDrawSurface4* m_lpddDevice;
  
  /// Direct3D class
  IDirect3D3* m_lpD3D;
  /// ZBuffer surface
  IDirectDrawSurface4* m_lpddZBuffer;
  /// D3D Device
  IDirect3DDevice3* m_lpd3dDevice;
  
  /// D3d Viewport
  IDirect3DViewport3* m_lpd3dViewport;
  /// D3d Background material.
  IDirect3DMaterial3* m_lpd3dBackMat;
  /// D3D handle to the background material.
  D3DMATERIALHANDLE m_hd3dBackMat;
  
  /// whether this is a hardware device.
  bool m_bIsHardware;
  /// the bit-depth of this device.
  DWORD m_dwDeviceBitDepth;
  /// the globally unique identifier for this device.
  GUID m_Guid;
  
  /// the texture cache.
  D3DTextureCache* m_pTextureCache;
  /// the lightmap cache.
  D3DLightMapCache* m_pLightmapCache;
  
  /// supported lightmap type
  int m_iTypeLightmap;
  
  /// Dimensions of viewport.
  int m_nWidth,  m_nHeight;
  
  /// Half-dimensions of viewport.
  int m_nHalfWidth,  m_nHalfHeight;
  
  /// The current read/write settings for the Z-buffer.
  G3DZBufMode m_ZBufMode;
  
  /// The current drawing mode (2D/3D)
  int m_nDrawMode;
  
  /**
   * DAN: render-states
   * these override any other variable settings.
   */
  bool rstate_dither;
  bool rstate_specular;
  bool rstate_bilinearmap;
  bool rstate_trilinearmap;
  bool rstate_gouraud;
  bool rstate_flat;
  bool rstate_alphablend;
  int rstate_mipmap;

  /// Capabilities of the renderer.
  G3D_CAPS m_Caps;
  
  /// The camera object.
  iCamera* m_pCamera;
  
  /// the 2d graphics driver.
  iGraphics2D* m_piG2D;
  
  /// The directdraw device description
  IDirectDetectionInternal* m_pDirectDevice;
  
  /// The system driver
  iSystem* m_piSystem;

  /// use 16 bit texture else 8 bit
  bool use16BitTexture;

  /// The private configuration file
  csIniFile *config;
  
public:
  DECLARE_IBASE;

  static DDPIXELFORMAT m_ddpfLightmapPixelFormat;
  static DDPIXELFORMAT m_ddpfTexturePixelFormat;
 
  /// The constructor. It is passed an interface to the system using it.
  csGraphics3DDirect3DDx6 (iBase *iParent);
  /// the destructor.
  virtual ~csGraphics3DDirect3DDx6 ();
  
  /// opens Direct3D.
  virtual bool Open(const char* Title);
  /// closes Direct3D.
  virtual void Close();

  ///
  virtual bool Initialize (iSystem *iSys);
  
  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height);
  
  /// Start a new frame.
  virtual bool BeginDraw (int DrawFlags);
  
  /// End the frame and do a page swap.
  virtual void FinishDraw ();
  
  /// Set the mode for the Z buffer (functionality also exists in SetRenderState).
  virtual void SetZBufMode (G3DZBufMode mode);
  
  /// Draw the projected polygon with light and texture.
  virtual void DrawPolygon (G3DPolygonDP& poly);
  /// Draw debug poly
  virtual void DrawPolygonDebug(G3DPolygonDP& poly) { }
  
  /// Draw a Line.
  virtual void DrawLine (csVector3& v1, csVector3& v2, int color);
  
  /// Draw a projected (non-perspective correct) polygon.
  virtual void DrawPolygonQuick (G3DPolygonDPQ& poly);
  
  /// Give a texture to Graphics3D to cache it.
  virtual void CacheTexture (iPolygonTexture* texture);
  
  /// Release a texture from the cache.
  virtual void UncacheTexture (iPolygonTexture* texture);
  
  /// Get the capabilities of this driver: NOT IMPLEMENTED.
  virtual void GetCaps (G3D_CAPS *caps);
  
  /// Dump the texture cache.
  virtual void DumpCache ();
  
  /// Clear the texture cache.
  virtual void ClearCache ();
  ///
  virtual G3D_COLORMAPFORMAT GetColormapFormat ()
  { return G3DCOLORFORMAT_PRIVATE; }
  
  /// Set the camera object.
  virtual void SetCamera (iCamera *pCamera)
  { m_pCamera = pCamera; }
  
  /// Print the screen.
  virtual void Print(csRect* rect)
  { return m_piG2D->Print(rect); }
  
  /// Set a render state
  virtual bool SetRenderState(G3D_RENDERSTATEOPTION, long value);
  /// Get a render state
  virtual long GetRenderState(G3D_RENDERSTATEOPTION);
  
  /// Get a z-buffer point
  virtual long *GetZBufPoint(int, int)
  { return NULL; }
  
  /// Get the width
  virtual int GetWidth () { return m_nWidth; }
  ///
  virtual int GetHeight () { return m_nHeight; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y);
  
  ///
  virtual bool NeedsPO2Maps ()
  { return true; }
  ///
  virtual int GetMaximumAspectRatio ()
  { return 32768; }
  
  /// 
  virtual iGraphics2D *GetDriver2D () 
  { return m_piG2D; }
  
  virtual void OpenFogObject (CS_ID id, csFog* fog);
  virtual void AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype);
  virtual void CloseFogObject (CS_ID id);
  
  virtual csHaloHandle CreateHalo (float r, float g, float b) { return NULL; }
  virtual void DestroyHalo (csHaloHandle haloInfo) { }
  virtual void DrawHalo (csVector3* pCenter, float fIntensity, csHaloHandle haloInfo) { }
  virtual bool TestHalo (csVector3* pCenter) { return false; }

private:
  
  // print to the system's device
  void SysPrintf(int mode, char* str, ...);
  
  // texture format enumeration callback function :: static member function.
  static HRESULT CALLBACK EnumTextFormatsCallback(LPDDPIXELFORMAT lpddpf, LPVOID lpUserArg);
  static HRESULT CALLBACK EnumZBufferCallback( DDPIXELFORMAT* pddpf, VOID* pddpfDesired );
  
  /// used to get the appropriate device.
  bool EnumDevices(void);
  /// whether the device is locked or not.
  bool m_bIsLocked;
  
  /// used to set up polygon geometry before rasterization.
  inline void SetupPolygon( G3DPolygonDP& poly, float& J1, float& J2, float& J3, 
    float& K1, float& K2, float& K3,
    float& M,  float& N,  float& O  );
};

#endif // G3D_D3D_H
