/*
    Copyright (C) 2007 by Frank Richter

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

#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/shader/shader.h"
#include "csutil/objreg.h"
#include "csutil/xmltiny.h"

#include "csplugincommon/rendermanager/renderlayers.h"

namespace CS
{
  namespace RenderManager
  {
    void AddDefaultBaseLayers (iObjectRegistry* objectReg,
      MultipleRenderLayer& layers, uint flags, iShader* defaultShader)
    {
      csRef<iShaderManager> shaderManager = 
        csQueryRegistry<iShaderManager> (objectReg);
      csRef<iStringSet> stringSet = csQueryRegistryTagInterface<iStringSet> (
        objectReg, "crystalspace.shared.stringset");

      if (defaultShader == 0)
        defaultShader = shaderManager->GetShader ("lighting_default");
      SingleRenderLayer baseLayer (defaultShader);
      baseLayer.AddShaderType (stringSet->Request("base"));
      baseLayer.AddShaderType (stringSet->Request("ambient"));
      if (!(flags & defaultlayerNoTerrain))
        baseLayer.AddShaderType (stringSet->Request("splatting ambient"));
      baseLayer.AddShaderType (stringSet->Request("standard"));
      layers.AddLayers (baseLayer);
      
      if (!(flags & defaultlayerNoTerrain))
      {
        SingleRenderLayer splatLayer;
        splatLayer.AddShaderType (stringSet->Request("terrain splat"));
        layers.AddLayers (splatLayer);
      }
    }
    
    namespace
    {
      static const char messageID[] = "crystalspace.renderlayers.parser";
        
      class LayersDocParser
      {
        iObjectRegistry* objectReg;
        csRef<iSyntaxService> synldr;
        csRef<iStringSet> stringSet;
        
	csStringHash xmltokens;
      #define CS_TOKEN_ITEM_FILE "libs/csplugincommon/rendermanager/renderlayers.tok"
      #include "cstool/tokenlist.h"
      #undef CS_TOKEN_ITEM_FILE
      
        bool ParseLayer (iDocumentNode* node, SingleRenderLayer& layer)
        {
          size_t maxLightPasses = 1;
          size_t maxLights = ~0;
          csRef<iShader> defaultShader;
          csDirtyAccessArray<csStringID> shaderTypes;
        
	  csRef<iDocumentNodeIterator> it = node->GetNodes();
	  while (it->HasNext())
	  {
	    csRef<iDocumentNode> child = it->Next();
	    if (child->GetType() != CS_NODE_ELEMENT) continue;
	    
	    csStringID id = xmltokens.Request (child->GetValue());
	    switch (id)
	    {
	      case XMLTOKEN_MAXLIGHTPASSES:
	        {
	          int v = child->GetContentsValueAsInt();
	          if (v < 0)
	          {
	            synldr->ReportError (messageID, 
	              child, "Invalid maximum light passes number");
	            return false;
	          }
	          maxLightPasses = v;
	        }
	        break;
	      case XMLTOKEN_MAXLIGHTS:
	        {
	          int v = child->GetContentsValueAsInt();
	          if (v < 0)
	          {
	            synldr->ReportError (messageID, 
	              child, "Invalid maximum lights number");
	            return false;
	          }
	          maxLights = v;
	        }
	        break;
	      case XMLTOKEN_DEFAULTSHADER:
	        {
	          defaultShader = synldr->ParseShaderRef (
	            0/* @@@ FIXME: not that nice */, child);
	          if (!defaultShader.IsValid()) return false;
	        }
	        break;
	      case XMLTOKEN_SHADERTYPE:
	        {
	          shaderTypes.Push (stringSet->Request (child->GetContentsValue()));
	        }
	        break;
	      default:
	        synldr->ReportBadToken (child);
	        return false;
	    }
	  }
	  
	  layer = SingleRenderLayer (shaderTypes.GetArray(),
	    shaderTypes.GetSize(), defaultShader, maxLightPasses, maxLights);
	  
          return true;
        }
      public:
        LayersDocParser (iObjectRegistry* objectReg) : objectReg (objectReg)
        {
          InitTokenTable (xmltokens);
          synldr = csQueryRegistry<iSyntaxService> (objectReg);
          CS_ASSERT(synldr);
	  stringSet = csQueryRegistryTagInterface<iStringSet> (
	    objectReg, "crystalspace.shared.stringset");
          CS_ASSERT(stringSet);
        }
        
        bool Parse (iDocumentNode* node, MultipleRenderLayer& layers)
        {
	  csRef<iDocumentNodeIterator> it = node->GetNodes();
	  while (it->HasNext())
	  {
	    csRef<iDocumentNode> child = it->Next();
	    if (child->GetType() != CS_NODE_ELEMENT) continue;
	    
	    csStringID id = xmltokens.Request (child->GetValue());
	    switch (id)
	    {
	      case XMLTOKEN_LAYER:
	        {
	          SingleRenderLayer layer;
	          if (!ParseLayer (child, layer)) return false;
	          layers.AddLayers (layer);
	        }
	        break;
	      default:
	        if (synldr)
	          synldr->ReportBadToken (child);
	        return false;
	    }
	  }
          return true;
        }
      };
    }
  
    bool AddLayersFromDocument (iObjectRegistry* objectReg, iDocumentNode* node,
      MultipleRenderLayer& layers)
    {
      LayersDocParser parser (objectReg);
      return parser.Parse (node, layers);
    }

    bool AddLayersFromFile (iObjectRegistry* objectReg, const char* fileName,
      MultipleRenderLayer& layers)
    {
      csRef<iDocumentSystem> docsys = csQueryRegistry<iDocumentSystem> (
        objectReg);
      if (!docsys.IsValid())
        docsys.AttachNew (new csTinyDocumentSystem ());
      
      csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
      CS_ASSERT(vfs);
      csRef<iFile> file = vfs->Open (fileName, VFS_FILE_READ);
      if (!file)
      {
        csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, messageID,
          "Error opening '%s'", fileName);
        return false;
      }
      
      csRef<iDocument> doc = docsys->CreateDocument();
      const char* error = doc->Parse (file);
      if (error != 0)
      {
        csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, messageID,
          "Error parsing '%s': %s", fileName, error);
        return false;
      }
      
      csRef<iDocumentNode> docRoot = doc->GetRoot();
      if (!docRoot) return false;
      csRef<iDocumentNode> layerConfigNode = docRoot->GetNode ("layerconfig");
      if (!layerConfigNode)
      {
        csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, messageID,
          "No <layerconfig> in '%s'", fileName);
        return false;
      }
      return AddLayersFromDocument (objectReg, layerConfigNode, layers);
    }
    
  } // namespace RenderManager
} // namespace CS
