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

#ifndef __CS_SOFTSHADER_SCANLINE_H__
#define __CS_SOFTSHADER_SCANLINE_H__

#include "ivideo/rendermesh.h"

#include "csplugincommon/softshader/types.h"

#include "scan_z.h"
#include "scanline_base.h"

CS_PLUGIN_NAMESPACE_BEGIN(SoftShader)
{
  using namespace CS::PluginCommon::SoftShader;

  struct Color_None
  {
    static const size_t compCount = 0;

    Color_None (ScanlineRendererBase* /*This*/) {}

    CS_FORCEINLINE
    void Apply (const ScanlineComp* /*color*/, Pixel& /*col*/) {}
  };

  struct ColorSource_Vertex
  {
    static const size_t compCount = 4;

    ColorSource_Vertex (ScanlineRendererBase* /*This*/) {}

    CS_FORCEINLINE 
    int32 GetFixedR (const ScanlineComp* color) const
    { return color[0].c.GetFixed(); }
    CS_FORCEINLINE 
    int32 GetFixedG (const ScanlineComp* color) const
    { return color[1].c.GetFixed(); }
    CS_FORCEINLINE 
    int32 GetFixedB (const ScanlineComp* color) const
    { return color[2].c.GetFixed(); }
    CS_FORCEINLINE 
    int32 GetFixedA (const ScanlineComp* color) const
    { return color[3].c.GetFixed(); }
  };

  struct ColorSource_Constant
  {
    static const size_t compCount = 0;

    csFixed16 r, g, b, a;

    ColorSource_Constant (ScanlineRendererBase* This) :
      r (This->constColor[0]), g (This->constColor[1]), 
      b (This->constColor[2]), a (This->constColor[3]) {}

    CS_FORCEINLINE 
    int32 GetFixedR (const ScanlineComp* color) const { return r.GetFixed(); }
    CS_FORCEINLINE 
    int32 GetFixedG (const ScanlineComp* color) const { return g.GetFixed(); }
    CS_FORCEINLINE
    int32 GetFixedB (const ScanlineComp* color) const { return b.GetFixed(); }
    CS_FORCEINLINE 
    int32 GetFixedA (const ScanlineComp* color) const { return a.GetFixed(); }
  };

  template<typename ColorSource>
  struct Color_Multiply : protected ColorSource
  {
    using ColorSource::compCount;

    const int cshift;
    const int ashift;

    Color_Multiply (ScanlineRendererBase* This) : 
      ColorSource (This), cshift (This->colorShift), 
        ashift (This->alphaShift) {}

    CS_FORCEINLINE
    static uint8 ClampAndShift (int32 x, const int shift)
    {
      return (x & 0x80000000) ? 0 : 
	(((x >> shift) & 0x7fffff00) ? 0xff : (x >> shift));
    }

    CS_FORCEINLINE
    void Apply (const ScanlineComp* color, Pixel& col) 
    {
      col.c.r = ClampAndShift (col.c.r * this->GetFixedR (color), cshift);
      col.c.g = ClampAndShift (col.c.g * this->GetFixedG (color), cshift);
      col.c.b = ClampAndShift (col.c.b * this->GetFixedB (color), cshift);
      col.c.a = ClampAndShift (col.c.a * this->GetFixedA (color), ashift);
    }
  };

  struct Color2_None
  {
    static const size_t compCount = 0;

    Color2_None (ScanlineRendererBase* /*This*/) {}

    CS_FORCEINLINE
    void Apply (const ScanlineComp* /*color*/, Pixel& /*col*/) {}
  };

  struct Color2_Sum
  {
    static const size_t compCount = 3;

    Color2_Sum (ScanlineRendererBase* /*This*/) {}

    CS_FORCEINLINE
    static uint8 ClampAndShift (int32 x)
    {
      return (x & 0x80000000) ? 0 : 
	(((x >> 8) & 0x7fffff00) ? 0xff : (x >> 8));
    }

    CS_FORCEINLINE
    void Apply (const ScanlineComp* color2, 
      Pixel& col) 
    {
      col.c.r = ClampAndShift ((col.c.r << 8) + color2[0].c.GetFixed());
      col.c.g = ClampAndShift ((col.c.g << 8) + color2[1].c.GetFixed());
      col.c.b = ClampAndShift ((col.c.b << 8) + color2[2].c.GetFixed());
    }
  };

  struct Source_Texture
  {
    static const size_t compCount = 2;

    const uint32* bitmap;
    const int v_shift_r;
    const int and_w;
    const int and_h;

    Source_Texture (ScanlineRendererBase* This) :
      bitmap(This->bitmap),
      v_shift_r(This->v_shift_r),
      and_w(This->and_w),
      and_h(This->and_h)
    {
    }

    CS_FORCEINLINE
    void GetColor (const ScanlineComp* tc, Pixel& col)
    {
      int u = (int)(tc[0].c);
      int32 v = tc[1].c.GetFixed();
      uint32 texel = bitmap [((v >> v_shift_r) & and_h) + (u & and_w)];
      col.ui32 = texel;
    }
  };

  struct Source_Flat
  {
    static const size_t compCount = 0;

    const Pixel flat_col;

    Source_Flat (ScanlineRendererBase* This) :
      flat_col(This->flat_col) { }

    CS_FORCEINLINE
    void GetColor (const ScanlineComp* /*tc*/, Pixel& col)
    {
      col = flat_col;
    }
  };

  class ScanlineRenderer : public ScanlineRendererBase
  {
  public:
    csVector4 dnTC;

  private:
    template <typename Source, typename Color, 
      typename Zmode, bool needColors, bool doAlphaTest, typename Color2>
    struct ScanlineImpl
    {
      static void Scan (iScanlineRenderer* _This,
	InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
	int ipolStep, int ipolShift,
	uint32* dest, uint len, uint32 *zbuff)
      {
	const size_t offsetColor = 0;
	const size_t offsetColor2 = offsetColor + Color::compCount;
	const size_t offsetTC = offsetColor2 + Color2::compCount;
	const size_t myIpolFloatNum = offsetTC + Source::compCount;

	InterpolateScanlinePersp<myIpolFloatNum> ipol;
	ipol.Setup (L, R, len, ipolStep, ipolShift);
	ScanlineRenderer* This = (ScanlineRenderer*)_This;

	Source colSrc (This);
	Color col (This);
	Color2 col2 (This);

	uint32* destend = dest + len;
	Zmode Z (ipol, zbuff);

	while (dest < destend)
	{
	  if (Z.Test())
	  {
	    if (needColors || doAlphaTest)
	    {
	      Pixel px;
	      colSrc.GetColor (ipol.GetFloat (offsetTC), px);
	      if (needColors)
	      {
	        col.Apply (ipol.GetFloat (offsetColor), px);
	        col2.Apply (ipol.GetFloat (offsetColor2), px);
	      }

	      if (doAlphaTest)
	      {
		const uint flag = (px.c.a & 0x80);
		px.c.a = (px.c.a >> 1) | flag;
		if (flag) Z.Update();
	      }
	      else
	      {
		px.c.a = (px.c.a >> 1) | 0x80;
		Z.Update();
	      }
	      *dest = px.ui32;
	    }
	    else
	    {
	      const Pixel px (0, 0, 0, 0x80);
	      *dest = px.ui32;
	      Z.Update();
	    }
	  }
	  else
	    *dest = 0;
	  dest++;
	  ipol.Advance();
	  Z.Advance();
	} /* endwhile */
      }
    };
    template<typename Source, typename Color, typename Zmode, 
      bool needColors, bool doAlphaTest>
    iScanlineRenderer::ScanlineProc GetScanlineProcSCCnCA ()
    {
      if (colorSum)
	return ScanlineImpl<Source, Color, Zmode, needColors, doAlphaTest, 
	  Color2_Sum>::Scan;
      else
	return ScanlineImpl<Source, Color, Zmode, needColors, doAlphaTest,
	  Color2_None>::Scan;
    }
    template<typename Source, typename Color, typename Zmode, 
      bool needColors>
    iScanlineRenderer::ScanlineProc GetScanlineProcSCCnC (bool doAlphaTest)
    {
      if (doAlphaTest)
	return GetScanlineProcSCCnCA<Source, Color, Zmode, needColors, true> ();
      else
	return GetScanlineProcSCCnCA<Source, Color, Zmode, needColors, false> ();
    }
    template<typename Source, typename Color, typename Zmode>
    iScanlineRenderer::ScanlineProc GetScanlineProcSCC (bool needColors,
						        bool doAlphaTest)
    {
      if (needColors)
	return GetScanlineProcSCCnC<Source, Color, Zmode, true> (doAlphaTest);
      else
	return GetScanlineProcSCCnC<Source, Color, Zmode, false> (doAlphaTest);
    }
    template<typename Source, typename Color>
    iScanlineRenderer::ScanlineProc GetScanlineProcSC (csZBufMode zmode,
                                                       bool needColors,
						       bool doAlphaTest)
    {
      switch (zmode)
      {
	case CS_ZBUF_NONE:
  	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZNone> (needColors, doAlphaTest);
	case CS_ZBUF_FILL:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZFill> (needColors, doAlphaTest);
	case CS_ZBUF_TEST:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZTest> (needColors, doAlphaTest);
	case CS_ZBUF_USE:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZUse> (needColors, doAlphaTest);
	case CS_ZBUF_EQUAL:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZEqual> (needColors, doAlphaTest);
	case CS_ZBUF_INVERT:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZInvert> (needColors, doAlphaTest);
	default:
	  return 0;
      }
    }
    template<typename Source>
    iScanlineRenderer::ScanlineProc GetScanlineProcS (csZBufMode zmode,
						      bool needColors,
						      bool doAlphaTest)
    {
      if (doConstColor)
        return GetScanlineProcSC<Source, 
          Color_Multiply<ColorSource_Constant> > (zmode, needColors, doAlphaTest);
      else if (doColor)
        return GetScanlineProcSC<Source, 
          Color_Multiply<ColorSource_Vertex> > (zmode, needColors, doAlphaTest);
      else
	return GetScanlineProcSC<Source, Color_None> (zmode, needColors, doAlphaTest);
    }
    iScanlineRenderer::ScanlineProc GetScanlineProc (csZBufMode zmode,
						     bool needColors,
						     bool doAlphaTest)
    {
      if (doTexture)
	return GetScanlineProcS<Source_Texture> (zmode, needColors, doAlphaTest);
      else
	return GetScanlineProcS<Source_Flat> (zmode, needColors, doAlphaTest);
    }
  public:
    bool SetupMesh (TexturesMask availableTextures, BuffersMask availableBuffers, 
      const csRenderMeshModes& modes, bool needColors, 
      RenderInfoMesh& renderInfoMesh)
    {
      renderInfoMesh.desiredBuffers = 0;
      doTexture = availableTextures & 1;
      if (doTexture)
	renderInfoMesh.desiredBuffers |= CS_SOFT3D_BUFFERFLAG(TEXCOORD);

      doColor = ((availableBuffers & CS_SOFT3D_BUFFERFLAG(COLOR)) != 0)
        && !doConstColor;
      if (colorSum)
      {
	static const size_t myBufferComps[] = {4, 3, 2};
	if (doColor)
	{
	  renderInfoMesh.bufferComps = myBufferComps;
	  renderInfoMesh.desiredBuffers |= CS_SOFT3D_BUFFERFLAG(COLOR);
	}
	else
	{
	  renderInfoMesh.bufferComps = &myBufferComps[1];
	}
	renderInfoMesh.desiredBuffers |= CS_SOFT3D_BUFFERFLAG(SECONDARY_COLOR);
      }
      else
      {
	static const size_t myBufferComps[] = {4, 2};
	if (doColor)
	{
	  renderInfoMesh.bufferComps = myBufferComps;
	  renderInfoMesh.desiredBuffers |= CS_SOFT3D_BUFFERFLAG(COLOR);
	}
	else
	  renderInfoMesh.bufferComps = &myBufferComps[1];
      }

      renderInfoMesh.renderer = this;

      bool doAlphaTest;
      switch (modes.mixmode & CS_MIXMODE_ALPHATEST_MASK)
      {
	case CS_MIXMODE_ALPHATEST_ENABLE:
	  doAlphaTest = true;
	  break;
	case CS_MIXMODE_ALPHATEST_DISABLE:
	  doAlphaTest = false;
	  break;
	default:
	case CS_MIXMODE_ALPHATEST_AUTO:
	  doAlphaTest = (modes.alphaType == csAlphaMode::alphaBinary);
	  break;
      }
      proc = GetScanlineProc (modes.z_buf_mode,
	needColors, doAlphaTest);

      return proc != 0;
    }
    bool SetupTriangle (SoftwareTexture** textures, 
      const RenderInfoMesh& /*renderInfoMesh*/, 
      RenderInfoTriangle& renderInfoTri)
    {
      renderInfoTri.denormBuffers = 0;
      
      SoftwareTexture* tex = textures[0];
      if (tex != 0)
      {
	bitmap = tex->bitmap;
        v_shift_r = tex->shf_w;
        and_w = tex->and_w;
        and_h = tex->and_h << v_shift_r;
        v_shift_r = 16 - v_shift_r;

	renderInfoTri.denormFactors = &dnTC;
	renderInfoTri.denormBuffers |= CS_SOFT3D_BUFFERFLAG(TEXCOORD);

	dnTC.Set (tex->w, tex->h, 0.0f, 0.0f);
      }
      else if (doTexture)
	return false;

      renderInfoTri.proc = proc;

      return true;
    }
  private:
    // Settings determined per-mesh
    bool doTexture;
    bool doColor;
    ScanlineProc proc;
  };

}
CS_PLUGIN_NAMESPACE_END(SoftShader)

#endif // __CS_SOFTSHADER_SCANLINE_H__
