/*
    Copyright (C) 2003 by Rene Jager
    (renej@frog.nl, renej.frog@yucom.be, renej_frog@sourceforge.net)

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

using System;
using System.Runtime.InteropServices;

namespace CrystalSpace
{
  public class CS: cspace
  {
    // A static and global object registry
    public static iObjectRegistry theObjectRegistry = null;

    // Requests the plugins for a given csPluginRequest array, using the
    // static object registry defined in cspace.
    public static bool RequestPlugins(csPluginRequest[] plugRequest)
    {
       return RequestPlugins(theObjectRegistry, plugRequest);
    }

    // Requests the plugins for a given csPluginRequest array.
    public static bool RequestPlugins(iObjectRegistry object_reg, csPluginRequest[] plugRequest)
    {
      if(object_reg == null)
	return false;

      csPluginRequestArray plgRequestArray = new csPluginRequestArray();
      foreach(csPluginRequest plg in plugRequest)
	plgRequestArray.Push(plg);
      return csInitializer._RequestPlugins(object_reg, plgRequestArray);
    }

    public static csPluginRequest CS_REQUEST_PLUGIN(string name, string intf)
    {
      return new csPluginRequest(name, intf);
    }
			
    public static csPluginRequest CS_REQUEST_VFS()
    {
      return CS_REQUEST_PLUGIN("crystalspace.kernel.vfs", "iVFS");
    }
		
    public static csPluginRequest CS_REQUEST_FONTSERVER()
    {
      return CS_REQUEST_PLUGIN("crystalspace.font.server.default",
	    "iFontServer");
    }
	
    public static csPluginRequest CS_REQUEST_IMAGELOADER()
    {
      return CS_REQUEST_PLUGIN("crystalspace.graphic.image.io.multiplexer",
	    "iImageIO");
    }
		
    public static csPluginRequest CS_REQUEST_NULL3D()
    {
      return CS_REQUEST_PLUGIN("crystalspace.graphics3d.null", "iGraphics3D");
    }
		
    public static csPluginRequest CS_REQUEST_SOFTWARE3D()
    {
      return CS_REQUEST_PLUGIN("crystalspace.graphics3d.software",
	    "iGraphics3D");
    }
		
    public static csPluginRequest CS_REQUEST_OPENGL3D()
    {
      return CS_REQUEST_PLUGIN("crystalspace.graphics3d.opengl",
	    "iGraphics3D");
    }

    public static csPluginRequest CS_REQUEST_ENGINE()
    {
      return CS_REQUEST_PLUGIN("crystalspace.engine.3d", "iEngine");
    }
		
    public static csPluginRequest CS_REQUEST_LEVELLOADER()
    {
      return CS_REQUEST_PLUGIN("crystalspace.level.loader", "iLoader");
    }
		
    public static csPluginRequest CS_REQUEST_LEVELSAVER()
    {
      return CS_REQUEST_PLUGIN("crystalspace.level.saver", "iSaver");
    }
	
    public static csPluginRequest CS_REQUEST_REPORTER()
    {
      return CS_REQUEST_PLUGIN("crystalspace.utilities.reporter", "iReporter");
    }
		
    public static csPluginRequest CS_REQUEST_REPORTERLISTENER()
    {
      return CS_REQUEST_PLUGIN("crystalspace.utilities.stdrep",
	    "iStandardReporterListener");
    }
		
    public static csPluginRequest CS_REQUEST_CONSOLEOUT()
    {
      return CS_REQUEST_PLUGIN("crystalspace.console.output.standard",
      "iConsoleOutput");
    }
		
    //Miscellaneous constants
    public const uint VFS_FILE_UNCOMPRESSED = 0x80000000;
    public const uint CS_ZBUF_MESH     = 0x80000000;
    public const uint CS_ZBUF_MESH2    = 0x80000001;
    public const uint CS_MIXMODE_TYPE_MESH = 0x80000000;
    public const uint CS_MIXMODE_TYPE_MASK = 0xc0000000;
    public const uint CS_FX_MESH = 0x80000000;
    public const uint CS_FX_MASK_MIXMODE = 0xf0ff0000;
    public const uint CS_LIGHT_ACTIVE_HALO = 0x80000000;
    public const uint CS_IMGFMT_INVALID = 0x80000000;
    public const double CS_BOUNDINGBOX_MAXVALUE = 1000000000.0;
    public const char CSKEY_BACKSPACE = '\b';

    // CS_VEC_* constants
    public static csVector3 CS_VEC_FORWARD = new csVector3(0,0,1);
    public static csVector3 CS_VEC_BACKWARD = new csVector3(0,0,-1);
    public static csVector3 CS_VEC_RIGHT = new csVector3(1,0,0);
    public static csVector3 CS_VEC_LEFT = new csVector3(-1,0,0);
    public static csVector3 CS_VEC_UP = new csVector3(0,1,0);
    public static csVector3 CS_VEC_DOWN = new csVector3(0,-1,0);
    public static csVector3 CS_VEC_ROT_RIGHT = new csVector3(0,1,0);
    public static csVector3 CS_VEC_ROT_LEFT = new csVector3(0,-1,0);
    public static csVector3 CS_VEC_TILT_RIGHT = new csVector3(0,0,-1);
    public static csVector3 CS_VEC_TILT_LEFT = new csVector3(0,0,1);
    public static csVector3 CS_VEC_TILT_UP = new csVector3(-1,0,0);
    public static csVector3 CS_VEC_TILT_DOWN = new csVector3(1,0,0);
  };
}; //namespace CrystalSpace

