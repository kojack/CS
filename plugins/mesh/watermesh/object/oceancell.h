#ifndef __CS_OCEANCELL_H__
#define __CS_OCEANCELL_H__

#include "cssysdef.h"

#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/tri.h"
#include "csgeom/box.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/cscolor.h"
#include "csutil/leakguard.h"
#include "cstool/objmodel.h"
#include "csgfx/renderbuffer.h"

namespace CS
{
namespace Plugins
{
namespace WaterMesh
{
	enum OceanLOD
	{
		LOD_LEVEL_1 = 0,
		LOD_LEVEL_2,
		LOD_LEVEL_3,
		LOD_LEVEL_4,
		LOD_LEVEL_5,
		NUM_LOD_LEVELS
	};
	
	class csOceanCell
	{
		private:
			//X-Z Grid coordinates of bottom right corner
			csVector2 gc;
			float len, wid;
			
			//Ocean height
			float oHeight;
			
			csBox3 bbox;
			OceanLOD type;
			
			bool isSetup;
			bool buffersNeedSetup;
			bool bufferHoldersNeedSetup;
			
			bool mesh_vertices_dirty_flag;
		  	bool mesh_texels_dirty_flag;
		  	bool mesh_normals_dirty_flag;
		  	bool mesh_triangle_dirty_flag;
			
			csRef<iRenderBuffer> vertex_buffer;
		  	csRef<iRenderBuffer> texel_buffer;
		  	csRef<iRenderBuffer> index_buffer;
		  	csRef<iRenderBuffer> normal_buffer;
			csRef<iRenderBuffer> color_buffer;

			csDirtyAccessArray<csVector3> verts;
			csDirtyAccessArray<csVector3> norms;
			csDirtyAccessArray<csVector2> texs;
			csDirtyAccessArray<csColor>	cols;
			csDirtyAccessArray<csTriangle> tris;
			
			void CalculateBBox();
			
		public:
			csRef<csRenderBufferHolder> bufferHolder;
			
			// len and wid must be a multiple of 10 and greater than 20
			// for LOD to work.
			csOceanCell(float len, float wid, OceanLOD level);
			~csOceanCell();
		
			inline OceanLOD GetType() { return type; }
			
			void SetupVertices();
			void SetupBufferHolder();
			void SetupBuffers();
			
			inline void SetOHeight(float h) { oHeight = h; }
			
			inline uint GetNumVerts() { return verts.GetSize(); }
			inline uint GetNumIndexes() { return tris.GetSize() * 3; }
			inline uint GetNumTris() { return tris.GetSize(); }				
	};
	
	class csOceanNode
	{
	public:		
		csVector2 gc;
		csOceanNode(csVector2 pos, float len, float wid);
		~csOceanNode();
		
		csBox3 GetBBox () { return bbox; }
		
		csVector3 GetCenter() const;
		
		inline float GetLen() { return len; }
		inline float GetWid() { return wid; }
		
		csOceanNode GetLeft() const;
		csOceanNode GetRight() const;
		csOceanNode GetUp() const;
		csOceanNode GetDown() const;
		
	private:
		float len, wid;
		float oHeight;
		
		csBox3 bbox;
	};
	
	typedef struct renderCell
	{
		int cell;
		csVector2 pos;
	} csRenderCell;
}
}
}
#endif // __CS_OCEANCELL_H__