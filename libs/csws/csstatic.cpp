/*
    Crystal Space Windowing System: static control class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#define SYSDEF_CASE
#include "sysdef.h"
#include "csws/csstatic.h"
#include "csws/csapp.h"

csStatic::csStatic (csComponent *iParent, csComponent *iLink, const char *iText,
  csStaticStyle iStyle) : csComponent (iParent)
{
  Init (iStyle);
  SetText (iText);
  link = iLink;
  SetSuggestedSize (0, 0);
  CheckUp ();
}

csStatic::csStatic (csComponent *iParent, csStaticStyle iStyle)
  : csComponent (iParent)
{
  Init (iStyle);
  SetSuggestedSize (0, 0);
}

csStatic::csStatic (csComponent *iParent, csSprite2D *iBitmap)
  : csComponent (iParent)
{
  Init (csscsBitmap);
  Bitmap = iBitmap;
  SetSuggestedSize (0, 0);
}

csStatic::~csStatic ()
{
  CHKB (delete Bitmap);
}

void csStatic::Init (csStaticStyle iStyle)
{
  Bitmap = NULL;
  underline_pos = -1;
  link = NULL;
  SetPalette (CSPAL_STATIC);
  style = iStyle;
  if (style != csscsRectangle)
  {
    state |= CSS_TRANSPARENT;
    if (parent)
      SetColor (CSPAL_STATIC_BACKGROUND, parent->GetColor (0));
  }
  TextAlignment = CSSTA_LEFT | CSSTA_VCENTER;
  linkactive = false;
  linkdisabled = false;
}

void csStatic::Draw ()
{
  int disabled = link ? linkdisabled : false;
  int textcolor = disabled ? CSPAL_STATIC_DTEXT :
    linkactive ? CSPAL_STATIC_ATEXT : CSPAL_STATIC_ITEXT;
  switch (style)
  {
    case csscsEmpty:
      break;
    case csscsLabel:
      DrawUnderline (0, 0, text, underline_pos, textcolor);
      break;
    case csscsFrameLabel:
    {
      int fry = (TextHeight () - 1) / 2;
      Rect3D (0, fry, bound.Width (), bound.Height () - fry,
        CSPAL_STATIC_LIGHT3D, CSPAL_STATIC_DARK3D);
      Rect3D (1, fry + 1, bound.Width () - 1, bound.Height () - fry - 1,
        CSPAL_STATIC_DARK3D, CSPAL_STATIC_LIGHT3D);
      int txtx = TextWidth ("///");
      Text (txtx, 0, textcolor, CSPAL_STATIC_BACKGROUND, text);
      DrawUnderline (txtx, 0, text, underline_pos, textcolor);
      break;
    }
    case csscsBitmap:
      if (Bitmap && Bitmap->GetTextureHandle ())
      {
        Sprite2D (Bitmap, 0, 0, bound.Width (), bound.Height ());
        break;
      }
    case csscsRectangle:
      Box (0, 0, bound.Width (), bound.Height (), CSPAL_STATIC_BACKGROUND);
      break;
    case csscsText:
      break;
  } /* endswitch */

  if (text && (style != csscsFrameLabel))
  {
    int x, y;
    switch (TextAlignment & CSSTA_HALIGNMASK)
    {
      case CSSTA_LEFT:    x = 0; break;
      case CSSTA_RIGHT:   x = bound.Width () - TextWidth (text); break;
      case CSSTA_HCENTER: x = (bound.Width () - TextWidth (text)) / 2; break;
      default:            return;
    } /* endswitch */
    switch (TextAlignment & CSSTA_VALIGNMASK)
    {
      case CSSTA_TOP:     y = 0; break;
      case CSSTA_BOTTOM:  y = bound.Height () - TextHeight (); break;
      case CSSTA_VCENTER: y = (bound.Height () - TextHeight ()) / 2; break;
      default:            return;
    } /* endswitch */
    Text (x, y, textcolor, -1, text);
  }

  csComponent::Draw ();
}

bool csStatic::IsHotKey (csEvent &Event)
{
  return ((underline_pos >= 0)
       && ((Event.Key.ShiftKeys & CSMASK_CTRL) == 0)
       && (UPPERCASE (Event.Key.Code) == UPPERCASE (text [underline_pos])));
}

bool csStatic::HandleEvent (csEvent &Event)
{
  CheckUp ();

  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdStaticGetBitmap:
          Event.Command.Info = Bitmap;
          break;
        case cscmdStaticSetBitmap:
          if (style == csscsBitmap)
          {
            Bitmap = (csSprite2D *)Event.Command.Info;
            Event.Command.Info = NULL;
            Invalidate ();
          }
          break;
      } /* endswitch */
      break;
    case csevMouseDown:
    case csevMouseDoubleClick:
      if ((style == csscsLabel)
       || (style == csscsFrameLabel))
      {
        // for frames, check if mouse is within label text
        if (style == csscsFrameLabel)
        {
          int xmin = TextWidth ("///");
          csRect r (xmin, 0, xmin + TextWidth (text), TextHeight ());
          if (!r.Contains (Event.Mouse.x, Event.Mouse.y))
            break;
        }
        if (!app->MouseOwner
         && link)
        {
          app->CaptureMouse (this);
          link->SendCommand (cscmdStaticMouseEvent, (void *)&Event);
          // if link did not captured the mouse, release it
          if (app->MouseOwner == this)
            app->CaptureMouse (NULL);
          CheckUp ();
          return true;
        } /* endif */
      } /* endif */
      break;
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

bool csStatic::PostHandleEvent (csEvent &Event)
{
  CheckUp ();
  if ((style == csscsLabel)
   || (style == csscsFrameLabel))
    switch (Event.Type)
    {
      case csevKeyDown:
        if (!app->KeyboardOwner
         && parent->GetState (CSS_FOCUSED)
         && IsHotKey (Event)
         && link)
        {
          link->Select ();
          oldKO = app->CaptureKeyboard (this);
          link->SendCommand (cscmdStaticHotKeyEvent, (void *)&Event);
          CheckUp ();
          return true;
        }
        break;
      case csevKeyUp:
        if (app->KeyboardOwner
         && IsHotKey (Event)
         && link)
        {
          link->SendCommand (cscmdStaticHotKeyEvent, (void *)&Event);
          app->CaptureKeyboard (oldKO);
          CheckUp ();
          return true;
        }
        break;
    } /* endswitch */
  return csComponent::PostHandleEvent (Event);
}

void csStatic::SuggestSize (int &w, int &h)
{
  w = 0; h = 0;
  switch (style)
  {
    case csscsEmpty:
      break;
    case csscsRectangle:
      break;
    case csscsLabel:
    case csscsFrameLabel:
    case csscsText:
      if (text)
      {
        w = TextWidth (text);
        h = TextHeight ();
      } /* endif */
      break;
    case csscsBitmap:
      if (Bitmap)
      {
        w = Bitmap->Width ();
        h = Bitmap->Height ();
      } /* endif */
      break;
  } /* endswitch */
  if (style == csscsLabel)
    h++;
}

void csStatic::SetText (const char *iText)
{
  if (style == csscsText)
    csComponent::SetText (iText);
  else
  {
    PrepareLabel (iText, text, underline_pos);
    Invalidate ();
  }
}

void csStatic::CheckUp ()
{
  if (!link)
    return;
  bool newlinkactive = !!link->GetState (CSS_FOCUSED);
  bool newlinkdisabled = !!link->GetState (CSS_DISABLED);
  if ((linkactive != newlinkactive) || (linkdisabled != newlinkdisabled))
  {
    linkdisabled = newlinkdisabled;
    linkactive = newlinkactive;
    Invalidate ();
  }
}
