/*
Copyright (C) 2002 by M�rten Svanfeldt
                      Anders Stenberg

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __GLSHADER_AVP_H__
#define __GLSHADER_AVP_H__

#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"

class csShaderGLAVP : public iShaderProgram
{
private:
  enum
  {
    XMLTOKEN_ARBVP = 1,
    XMLTOKEN_DECLARE,
    XMLTOKEN_MAP,
    XMLTOKEN_PROGRAM
  };

  struct variablemapentry
  {
    variablemapentry() { name = 0; }
    ~variablemapentry() { if(name) delete name; }
    char* name;
    int registernum;
  };

  csGLExtensionManager* ext;
  csRef<iObjectRegistry> object_reg;

  unsigned int program_num;

  csHashMap variables;
  csBasicVector variablemap;
  csStringHash xmltokens;

  void BuildTokenHash();

public:
  SCF_DECLARE_IBASE;

  csShaderGLAVP(iObjectRegistry* objreg, csGLExtensionManager* ext)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    this->object_reg = objreg;
    this->ext = ext;
  }
  virtual ~csShaderGLAVP () {}

  bool LoadProgram( const char* programstring );
  bool LoadProgramStringToGL( const char* programstring );

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate(iShaderPass* current);

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate(iShaderPass* current);

  /* Propertybag - get property, return false if no such property found
   * Which properties there is is implementation specific
   */
  virtual bool GetProperty(const char* name, iString* string) {return false;};
  virtual bool GetProperty(const char* name, int* string) {return false;};
  virtual bool GetProperty(const char* name, csVector3* string) {return false;};
//  virtual bool GetProperty(const char* name, csVector4* string) {};

  /* Propertybag - set property.
   * Which properties there is is implementation specific
   */
  virtual bool SetProperty(const char* name, iString* string) {return false;};
  virtual bool SetProperty(const char* name, int* string) {return false;};
  virtual bool SetProperty(const char* name, csVector3* string) {return false;};
//  virtual bool SetProperty(const char* name, csVector4* string) {return false;};

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable) 
    { /*do not allow externals to add variables*/ return false; };
  /// Get variable
  virtual iShaderVariable* GetVariable(const char* string);
  /// Get all variable stringnames added to this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames(); 
};


#endif //__GLSHADER_AVP_H__

