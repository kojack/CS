/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#include "cssys/csuctransform.h"

#include "graph2d.h"
#include "softfontcache.h"

//---------------------------------------------------------------------------

csSoftFontCache::csSoftFontCache (csGraphics2D* G2D)
{
  csSoftFontCache::G2D = G2D;
}

csSoftFontCache::GlyphCacheData* csSoftFontCache::InternalCacheGlyph (
  KnownFont* font, utf32_char glyph)
{
  SoftGlyphCacheData* newData = new SoftGlyphCacheData;
  SetupCacheData (newData, font, glyph);
  int w, h;
  newData->glyphData = font->font->GetGlyphBitmap (glyph, w, h);
  newData->glyphDataAlpha = font->font->GetGlyphAlphaBitmap (glyph, w, h);
  return newData;

}

void csSoftFontCache::InternalUncacheGlyph (GlyphCacheData* cacheData)
{
  SoftGlyphCacheData* softCacheData = (SoftGlyphCacheData*)cacheData;
  delete softCacheData;
}


//---------------------------------------------------------------------------

csSoftFontCache8::csSoftFontCache8 (csGraphics2D* G2D) : csSoftFontCache (G2D)
{
}

#define WS_NAME csSoftFontCache8::WriteString
#define WS_NAME2 csSoftFontCache8::WriteStringBaseline
#define WS_PIXTYPE uint8
#include "writechr.inc"

//---------------------------------------------------------------------------

csSoftFontCache16_555::csSoftFontCache16_555 (csGraphics2D* G2D) : csSoftFontCache (G2D)
{
}

#define WS_NAME csSoftFontCache16_555::WriteString
#define WS_NAME2 csSoftFontCache16_555::WriteStringBaseline
#define WS_PIXTYPE uint16
#define WS_ALPHA_AVAILABLE
#define WS_A_R5G5B5
#include "writechr.inc"

//---------------------------------------------------------------------------

csSoftFontCache16_565::csSoftFontCache16_565 (csGraphics2D* G2D) : csSoftFontCache (G2D)
{
}

#define WS_NAME csSoftFontCache16_565::WriteString
#define WS_NAME2 csSoftFontCache16_565::WriteStringBaseline
#define WS_PIXTYPE uint16
#define WS_ALPHA_AVAILABLE
#define WS_A_R5G6B5
#include "writechr.inc"

//---------------------------------------------------------------------------

csSoftFontCache32::csSoftFontCache32 (csGraphics2D* G2D) : csSoftFontCache (G2D)
{
}

#define WS_NAME csSoftFontCache32::WriteString
#define WS_NAME2 csSoftFontCache32::WriteStringBaseline
#define WS_PIXTYPE uint32
#define WS_ALPHA_AVAILABLE
#define WS_A_R8G8B8
#include "writechr.inc"

