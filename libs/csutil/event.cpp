/*
    Event system related helpers
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>

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
#include "csutil/event.h"

utf32_char csKeyEventHelper::GetRawCode (const iEvent* event)
{
  uint32 code;
  if (event->Retrieve ("keyCodeRaw", code) != csEventErrNone)
    return 0;
  return code;
}

utf32_char csKeyEventHelper::GetCookedCode (const iEvent* event)
{
  uint32 code;
  if (event->Retrieve ("keyCodeCooked", code) != csEventErrNone)
    return 0;
  return code;
}

void csKeyEventHelper::GetModifiers (const iEvent* event, 
				     csKeyModifiers& modifiers)
{
  memset (&modifiers, 0, sizeof (modifiers));

  const void* mod;
  size_t modSize;
  if (event->Retrieve ("keyModifiers", mod, modSize) != csEventErrNone) 
    return;
  memcpy (&modifiers, mod, MIN (sizeof (modifiers), modSize));
}

void csKeyEventHelper::GetModifiers (uint32 mask, csKeyModifiers& modifiers)
{
  for (int type = 0; type < csKeyModifierTypeLast; type++)
    if (mask & (1 << type))
      modifiers.modifiers[type] = (1 << csKeyModifierNumAny);
}

csKeyEventType csKeyEventHelper::GetEventType (const iEvent* event)
{
  uint8 type;
  if (event->Retrieve ("keyEventType", type) != csEventErrNone)
    return (csKeyEventType)-1;
  return (csKeyEventType)type;
}

bool csKeyEventHelper::GetAutoRepeat (const iEvent* event)
{
  bool autoRep;
  if (event->Retrieve ("keyAutoRepeat", autoRep) != csEventErrNone) 
    return false;
  return autoRep;
}
				    
csKeyCharType csKeyEventHelper::GetCharacterType (const iEvent* event)
{
  uint8 type;
  if (event->Retrieve ("keyCharType", type) != csEventErrNone)
    return (csKeyCharType)-1;
  return (csKeyCharType)type;
}

bool csKeyEventHelper::GetEventData (const iEvent* event, 
				      csKeyEventData& data)
{
  if (!CS_IS_KEYBOARD_EVENT (*event)) return false;

  data.autoRepeat = GetAutoRepeat (event);
  data.charType = GetCharacterType (event);
  data.codeCooked = GetCookedCode (event);
  data.codeRaw = GetRawCode (event);
  data.eventType = GetEventType (event);
  GetModifiers (event, data.modifiers);

  return true;
}

uint32 csKeyEventHelper::GetModifiersBits (const iEvent* event)
{
  csKeyModifiers m;
  GetModifiers (event, m);
  return GetModifiersBits (m);
}

uint32 csKeyEventHelper::GetModifiersBits (const csKeyModifiers& m)
{
  uint32 res = 0;

  for (int n = 0; n < csKeyModifierTypeLast; n++)
  {
    if (m.modifiers[n] != 0)
      res |= (1 << n);
  }

  return res;
}

//---------------------------------------------------------------------------

uint csMouseEventHelper::GetNumber (const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("mNumber", res);
  return res;
}

int csMouseEventHelper::GetAxis (const iEvent *event, uint axis)
{
  const void *_xs; size_t _xs_sz;
  uint8 axs;
  if (event->Retrieve("mAxes", _xs, _xs_sz) != csEventErrNone)
    return 0;
  if (event->Retrieve("mNumAxes", axs) != csEventErrNone)
    return 0;
  const int32 *axdata = (int32 *) _xs;
  if ((axis > 0) && (axis <= axs))
    return axdata[axis - 1];
  else
    return 0;
}

uint csMouseEventHelper::GetButton (const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("mButton", res);
  return res;
}

uint csMouseEventHelper::GetNumAxes(const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("mNumAxes", res);
  return res;
}

bool csMouseEventHelper::GetEventData (const iEvent* event, 
					csMouseEventData& data)
{
  if (!CS_IS_MOUSE_EVENT (*event)) return false;

  const void *_ax = 0; size_t _ax_sz = 0;
  uint8 ui8;
  csEventError ok = csEventErrNone;
  ok = event->Retrieve("mAxes", _ax, _ax_sz);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("mNumAxes", ui8);
  CS_ASSERT(ok == csEventErrNone);
  data.numAxes = ui8;
  for (uint iter=0 ; iter<CS_MAX_MOUSE_AXES ; iter++) {
    if (iter<data.numAxes)
      data.axes[iter] = ((int32*)_ax)[iter];
    else
      data.axes[iter] = 0;
  }
  data.x = data.axes[0];
  data.y = data.axes[1];
  ok = event->Retrieve("mButton", ui8);
  CS_ASSERT(ok == csEventErrNone);
  data.Button = ui8;
  ok = event->Retrieve("keyModifiers", data.Modifiers);
  CS_ASSERT(ok == csEventErrNone);
  return true;
}

//---------------------------------------------------------------------------

uint csJoystickEventHelper::GetNumber(const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("jsNumber", res);
  return res;
}

int csJoystickEventHelper::GetAxis(const iEvent *event, uint axis)
{
  const void *_xs; size_t _xs_sz;
  uint8 axs;
  if (event->Retrieve("jsAxes", _xs, _xs_sz) != csEventErrNone)
    return 0;
  if (event->Retrieve("jsNumAxes", axs) != csEventErrNone)
    return 0;
  const int *axdata = (int *) _xs;
  if ((axis > 0) && (axis <= axs))
    return axdata[axis - 1];
  else
    return 0;
}

uint csJoystickEventHelper::GetButton(const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("jsButton", res);
  return res;
}

uint csJoystickEventHelper::GetNumAxes(const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("jsNumAxes", res);
  return res;
}

bool csJoystickEventHelper::GetEventData (const iEvent* event, 
					   csJoystickEventData& data)
{
  if (!CS_IS_JOYSTICK_EVENT (*event)) return false;
  
  const void *_ax = 0; size_t _ax_sz = 0;
  uint8 ui8;
  csEventError ok = csEventErrNone;
  ok = event->Retrieve("jsNumber", ui8);
  data.number = ui8;
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("jsAxes", _ax, _ax_sz);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("jsNumAxes", ui8);
  data.numAxes = ui8;
  CS_ASSERT(ok == csEventErrNone);
  for (uint iter=0 ; iter<CS_MAX_JOYSTICK_AXES ; iter++) {
    if (iter<data.numAxes)
      data.axes[iter] = ((int32 *)_ax)[iter];
    else
      data.axes[iter] = 0;
  }
  ok = event->Retrieve("jsAxesChanged", data.axesChanged);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("jsButton", ui8);
  CS_ASSERT(ok == csEventErrNone);
  data.Button = ui8;
  ok = event->Retrieve("keyModifiers", data.Modifiers);
  CS_ASSERT(ok == csEventErrNone);
  return true;
}

//---------------------------------------------------------------------------

uint csCommandEventHelper::GetCode(const iEvent* event)
{
  uint32 res = 0;
  event->Retrieve("cmdCode", res);
  return res;
}

intptr_t csCommandEventHelper::GetInfo(const iEvent* event)
{
  int64 res = 0;
  event->Retrieve("cmdInfo", res);
  return res;
}

bool csCommandEventHelper::GetEventData(const iEvent* event, 
					 csCommandEventData& data)
{
  if (!CS_IS_COMMAND_EVENT (*event)) return false;

  csEventError ok = csEventErrNone;
  uint32 ui32;
  ok = event->Retrieve("cmdCode", ui32);
  CS_ASSERT(ok == csEventErrNone);
  data.Code = ui32;
  int64 ipt;
  ok = event->Retrieve("cmdInfo", ipt);
  data.Info = ipt;
  CS_ASSERT(ok == csEventErrNone);
  return true;
}
