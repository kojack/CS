/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "csutil/set.h"

#include "common.h"
#include "chunkyalloc.h"
#include "swappable.h"
#include "vertexdata.h"

namespace lighter
{
  class Object;
  struct ElementProxy;

  class Primitive;
  class TempElementAreas;
  
  class ElementAreasAlloc : public ChunkyAllocator<2*1024*1024>
  {
  };

  class ElementAreas : public CS::Memory::CustomAllocated
  {
    friend class Primitive;
    
    float fullArea;
    size_t elementCount;

    csBitArray elementsBits;
    enum IndexType { idxUI8, idxUI16, idxUI32, idxSizeT };
    IndexType elementIndexType;
    size_t elementChunkID;
    size_t areasChunkID;
    size_t fractionalLength;
    
    void Finalize (const TempElementAreas&);
  public:
    ElementAreas();
    ElementAreas (const ElementAreas& other);
    ~ElementAreas();

    float GetElementArea (size_t element) const;
    size_t GetSize() const { return elementCount; }
  };

  /**
   * Primitive in the radiosity world.
   * Represents a single primitive (face) in the radiosity world.
   */
  class PrimitiveBase
  {
  protected:
    inline PrimitiveBase (ObjectBaseVertexData* vertexData) 
      : vertexData (vertexData)
    {
    }
    inline PrimitiveBase (const PrimitiveBase& other) 
      : vertexData (other.vertexData), triangle (other.triangle), 
        plane (other.plane)
    {
    }
  public:
    typedef CS::TriangleT<size_t> TriangleType;

    /// Calculate center
    const csVector3 GetCenter () const;

    /// Calculate area, forwarder
    float GetArea () const;

    /// Get min/max extent in given dimension
    void GetExtent (uint dimension, float &min, float &max) const;

    /// Classify with respect to another plane
    int Classify (const csPlane3 &plane) const;

    /// Calculate and save primitive plane
    void ComputePlane ();

    /// Calculate min-max UV-coords
    void ComputeMinMaxUV (const Vector2Array& lightmapUVs,
      csVector2 &min, csVector2 &max) const;
    void ComputeMinMaxUV (const ObjectVertexData::Vector2Array& lightmapUVs,
      csVector2 &min, csVector2 &max) const;

    inline TriangleType& GetTriangle () { return triangle; }
    inline const TriangleType& GetTriangle () const { return triangle; }
    inline void SetTriangle (const TriangleType& t) { triangle = t; }

    inline const csPlane3& GetPlane () const { return plane; }

    /*inline const FactoryPrimitive* GetOriginalPrimitive () const 
    { return originalPrim; }
    inline void SetOriginalPrimitive (FactoryPrimitive*p) 
    { originalPrim = p; }*/

    inline ObjectBaseVertexData& GetVertexData () 
    { return *vertexData; }
    inline const ObjectBaseVertexData& GetVertexData () const 
    { return *vertexData; }

  protected:
    /// Vertex data holder
    ObjectBaseVertexData* vertexData;

    /// Index array for this primitive
    TriangleType triangle;

    /// Computed plane
    /* Plane normal seems to point into opposite direction compared to e.g.
     * auto-computed vertex normals. (FIXME?) */
    csPlane3 plane;
  };

  class FactoryPrimitive : public PrimitiveBase
  {
  public:
    // Constructors
    inline FactoryPrimitive (ObjectFactoryVertexData &dataHolder) 
      : PrimitiveBase (&dataHolder)
    {
    }

    inline ObjectFactoryVertexData & GetVertexData () 
    { return *(static_cast<ObjectFactoryVertexData*> (vertexData)); }
    inline const ObjectFactoryVertexData & GetVertexData () const 
    { return *(static_cast<ObjectFactoryVertexData*> (vertexData)); }

  };

  class Primitive : public PrimitiveBase
  {
  public:
    inline Primitive (ObjectVertexData &dataHolder) 
      : PrimitiveBase (&dataHolder), uFormVector (0), vFormVector (0), 
        /*illuminationColor (0,0,0), reflectanceColor (1.0f,1.0f,1.0f), */
        minCoord (0), minUV (0,0), maxUV (0,0), /*originalPrim (0), */
        radObject (0)
    {
    }
    inline Primitive (const Primitive& other) 
      : PrimitiveBase (other), elementAreas (other.elementAreas), 
        uFormVector (other.uFormVector), vFormVector (other.uFormVector), 
        minCoord (other.minCoord), minUV (other.minUV), maxUV (other.maxUV), 
        radObject (other.radObject), globalLightmapID (other.globalLightmapID),
        lambdaCoeffTV (other.lambdaCoeffTV), myCoeffTV (other.myCoeffTV)
    {
    }
    inline ~Primitive () { }

    /**
     * Prepare the primitive, create elements.
     */
    void Prepare ();

    /// Given a point, compute the element index
    size_t ComputeElementIndex (const csVector3& pt) const;

    /// Given an element index, compute center point
    csVector3 ComputeElementCenter (size_t index) const;

    /// Get interpolated normal at point
    csVector3 ComputeNormal (const csVector3& point) const;

    /// Calculate min-max UV-coords
    void ComputeMinMaxUV (csVector2 &min, csVector2 &max) const;

    /// Calculate min-max UV-coords as integers
    inline void ComputeMinMaxUV (int &minU, int &maxU, int &minV, int &maxV) const
    {
      csVector2 min, max;
      ComputeMinMaxUV (min, max);
      minU = (int)floor (min.x);
      minV = (int)floor (min.y);
      maxU = (int)ceil (max.x);
      maxV = (int)ceil (max.y);
    }

    using PrimitiveBase::ComputeMinMaxUV;

    /// Calculate the u/v form vectors
    void ComputeUVTransform ();

    /// Compute if point is inside primitive or not
    bool PointInside (const csVector3& pt) const;

    /// Compute the two barycentric coordinates given a point
    void ComputeBaryCoords (const csVector3& v, float& lambda, float& my) const;

    /// Get number of elements
    size_t GetElementCount () const
    {
      return elementAreas.GetSize ();
    }

    /// Get u/v offset of element
    inline void GetElementUV (size_t elementIndex, size_t &u, size_t &v) const
    {
      size_t uWidth = size_t (maxUV.x - minUV.x + 1);

      u = (elementIndex % uWidth);
      v = (elementIndex / uWidth);
    }

    /// Get an element proxy given element index
    ElementProxy GetElement (size_t index);

    /// Get an element proxy given point
    ElementProxy GetElement (const csVector3& pt);

    inline ObjectVertexData & GetVertexData () 
    { return *(static_cast<ObjectVertexData*> (vertexData)); }
    inline const ObjectVertexData& GetVertexData () const 
    { return *(static_cast<ObjectVertexData*> (vertexData)); }

    inline const csVector3& GetuFormVector () const { return uFormVector; }
    inline const csVector3& GetvFormVector () const { return vFormVector; }

    /*inline const csColor& GetIlluminationColor () const { return illuminationColor; }
    inline const csColor& GetReflectanceColor () const { return reflectanceColor; }*/

    inline const ElementAreas& GetElementAreas () const { return elementAreas; }

    inline const csVector3& GetMinCoord () const { return minCoord; }
    inline const csVector2& GetMinUV () const { return minUV; }
    inline const csVector2& GetMaxUV () const { return maxUV; }

    inline const Object* GetObject () const { return radObject; }
    inline Object* GetObject () { return radObject; }
    inline void SetObject (Object *obj) { radObject = obj; }

    inline uint GetGlobalLightmapID () const { return globalLightmapID; }
    inline void SetGlobalLightmapID (uint id) { globalLightmapID = id; }

    
protected:
    //@{
    /// Elements
    ElementAreas elementAreas;
    //@}

    /// Mapping vectors
    csVector3 uFormVector;
    csVector3 vFormVector;

    /// Color
    //csColor illuminationColor;
    //csColor reflectanceColor;

    /// Min/max data
    csVector3 minCoord;
    csVector2 minUV;
    csVector2 maxUV;

    /// Original object
    Object* radObject;

    /// GLobal lightmap id
    uint globalLightmapID;

    /* Coefficients to computer barycentric coordinates for a point on the
     * primitive. 
     * \remarks
     * Also see http://en.wikipedia.org/wiki/Barycentric_coordinates_%28mathematics%29
     *
     * Tho, the formulae used in this code are, in comparison to those given,
     * in the above article, quite mangled.
     */
    csVector3 lambdaCoeffTV, myCoeffTV;

    void ComputeBaryCoeffs ();
  };

  typedef csArray<FactoryPrimitive> FactoryPrimitiveArray;
  typedef csArray<Primitive> PrimitiveArray;
  typedef csArray<Primitive*> PrimitivePtrArray;
  typedef csSet<Primitive*> PrimitivePtrSet;


  /// Small helper that acts as a reference to a single element (pixel)
  struct ElementProxy
  {
    ElementProxy (Primitive& p, size_t e)
      : primitive (p), element (e)
    {
    }

    /// Primitive we are a part of
    Primitive& primitive;

    /// Primitive index
    size_t element;
  };
}

template<>
class csHashComputer<lighter::Primitive*> : public csHashComputerIntegral<lighter::Primitive*> {};

#endif
