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

#ifndef __PATH_H__
#define __PATH_H__

#include "csgeom/spline.h"

/**
 * A path in 3D.
 * An object or camera can use this object to trace a path in 3D.
 * This is particularly useful in combination with
 * csReversibleTransform::LookAt().
 */
class csPath : public csCatmullRomSpline
{
private:
  void SetVectorAsDimensionValues (int dim, csVector3* v)
  {
    int i;
    float* x, * y, * z;
    x = new float [GetNumPoints ()];
    y = new float [GetNumPoints ()];
    z = new float [GetNumPoints ()];
    for (i = 0 ; i < GetNumPoints () ; i++)
    {
      x[i] = v[i].x;
      y[i] = v[i].y;
      z[i] = v[i].z;
    }
    SetDimensionValues (dim+0, x);
    SetDimensionValues (dim+1, y);
    SetDimensionValues (dim+2, z);
    delete[] x;
    delete[] y;
    delete[] z;
  }

public:
  /// Create a path with p points.
  csPath (int p) : csCatmullRomSpline (9, p) { }

  /// Destroy the path.
  virtual ~csPath () { }

  /// Set the position vectors (first three dimensions of the cubic spline).
  void SetPositionVectors (csVector3* v)
  {
    SetVectorAsDimensionValues (0, v);
  }
  /// Set the up vectors (dimensions 3 to 5).
  void SetUpVectors (csVector3* v)
  {
    SetVectorAsDimensionValues (3, v);
  }
  /// Set the forward vectors (dimensions 6 to 8).
  void SetForwardVectors (csVector3* v)
  {
    SetVectorAsDimensionValues (6, v);
  }

  /// Get the interpolated position.
  void GetInterpolatedPosition (csVector3& pos)
  {
    pos.x = GetInterpolatedDimension (0);
    pos.y = GetInterpolatedDimension (1);
    pos.z = GetInterpolatedDimension (2);
  }
  /// Get the interpolated up vector.
  void GetInterpolatedUp (csVector3& pos)
  {
    pos.x = GetInterpolatedDimension (3);
    pos.y = GetInterpolatedDimension (4);
    pos.z = GetInterpolatedDimension (5);
  }
  /// Get the interpolated forward vector.
  void GetInterpolatedForward (csVector3& pos)
  {
    pos.x = GetInterpolatedDimension (6);
    pos.y = GetInterpolatedDimension (7);
    pos.z = GetInterpolatedDimension (8);
  }
};

#endif // __PATH_H__
