/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __IVARIA_REPORTER_H__
#define __IVARIA_REPORTER_H__

#include <stdarg.h>
#include "csutil/scf.h"

/**
 * Severity level for iReporter: BUG severity level.
 * This is the worst thing that can happen. It means that some code
 * detected a bug in Crystal Space.
 */
#define CS_REPORTER_SEVERITY_BUG 0

/**
 * Severity level for iReporter: ERROR severity level.
 * There was an error of some kind. Usually this is an error while
 * reading data.
 */
#define CS_REPORTER_SEVERITY_ERROR 1

/**
 * Severity level for iReporter: WARNING severity level.
 * There was some condition which is non fatal but is suspicious.
 */
#define CS_REPORTER_SEVERITY_WARNING 2

/**
 * Severity level for iReporter: NOTIFY severity level.
 * Just a notification message.
 */
#define CS_REPORTER_SEVERITY_NOTIFY 3


SCF_VERSION (iReporter, 0, 0, 1);

/**
 * This is the interface for the error/message reporter plugin.
 */
struct iReporter : public iBase
{
  /**
   * Report something. The given message string should be formed like:
   * 'crystalspace.<source>.<type>.<detail>'. Example:
   * 'crystalspace.sprite2dloader.parse.material'.
   */
  virtual void Report (int severity, const char* msgId,
  	const char* description, ...) = 0;

  /**
   * Report something. va_list version.
   */
  virtual void ReportV (int severity, const char* msgId,
  	const char* description, va_list) = 0;

  /**
   * Clear all messages in the reporter. If severity is -1 then all
   * will be deleted. Otherwise only messages of the specified severity
   * will be deleted.
   */
  virtual void Clear (int severity = -1) = 0;

  /**
   * Clear all messages in the reporter for which the id matches with
   * the given mask. The mask can contain '*' or '?' wildcards. This
   * can be used to clear all messages from some source like:
   * Clear("crystalspace.sprite2dloader.*")
   */
  virtual void Clear (const char* mask) = 0;

  /**
   * Give the number of reported messages currently registered.
   */
  virtual int GetMessageCount () const = 0;

  /**
   * Get message severity. Returns -1 if message doesn't exist.
   */
  virtual int GetMessageSeverity (int idx) const = 0;

  /**
   * Get message id. Returns NULL if message doesn't exist.
   */
  virtual const char* GetMessageId (int idx) const = 0;

  /**
   * Get message description. Returns NULL if message doesn't exist.
   */
  virtual const char* GetMessageDescription (int idx) const = 0;
};

#endif // __IVARIA_REPORTER_H__

