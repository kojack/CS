/*
  Copyright (C) 2003-2005 by Marten Svanfeldt
			     Anders Stenberg

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

#ifndef __CS_CSGFX_RENDERBUFFER_H__
#define __CS_CSGFX_RENDERBUFFER_H__

/**\file
 * Render buffer.
 */

#include "csextern.h"
#include "csutil/leakguard.h"
#include "csutil/scf_implementation.h"
#include "ivideo/rndbuf.h"

/**\addtogroup gfx
 * @{ 
 */

/**
 * Structure describing the properties of the individual buffers to be 
 * interleaved.
 */
struct csInterleavedSubBufferOptions
{
  /// Components Types; usually CS_BUFCOMP_UNSIGNED_INT
  csRenderBufferComponentType componentType;
  /// Number of components per element (e.g. 4 for RGBA)
  uint componentCount;
};

#ifdef CS_DEBUG
class csCallStack;
#endif

/**
 * Render buffer - basic container for mesh geometry data.
 */
class CS_CRYSTALSPACE_EXPORT csRenderBuffer :
  public scfImplementation1<csRenderBuffer, iRenderBuffer>
{
protected:
  /**
   * Constructor.
   */
  csRenderBuffer (size_t size, csRenderBufferType type, 
    csRenderBufferComponentType componentType, uint componentCount, 
    size_t rangeStart, size_t rangeEnd, bool copy);
public:
  CS_LEAKGUARD_DECLARE (csRenderBuffer);

  /**
   * Destructor
   */
  virtual ~csRenderBuffer ();

  /**
  * Lock the buffer to allow writing and give us a pointer to the data.
  * The pointer will be (void*)-1 if there was some error.
  * \param lockType The type of lock desired.
  */
  virtual void* Lock (csRenderBufferLockType lockType);

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release();

  /**
   * Copy data to the render buffer.
   */
  virtual void CopyInto (const void *data, size_t elementCount,
    size_t elemOffset = 0);

  /// Gets the number of components per element
  virtual int GetComponentCount () const
  {
    return props.compCount;
  }

  /// Gets the component type (float, int, etc)
  virtual csRenderBufferComponentType GetComponentType () const 
  {
    return props.comptype;
  }

  /// Get type of buffer (static/dynamic)
  virtual csRenderBufferType GetBufferType() const
  {
    return props.bufferType;
  }

  /// Get the size of the buffer (in bytes)
  virtual size_t GetSize() const
  {
    return bufferSize;
  }

  /// Get the stride of the buffer (in bytes)
  virtual size_t GetStride() const 
  {
    return props.stride;
  }

  virtual size_t GetElementDistance() const
  {
    return props.stride ? props.stride :
      props.compCount * csRenderBufferComponentSizes[props.comptype];
  }

  virtual size_t GetOffset() const
  { return props.offset; }

  /// Get version
  virtual uint GetVersion ()
  {
    return version;
  }

  virtual bool IsMasterBuffer ()
  {
    return !masterBuffer.IsValid();
  }

  virtual iRenderBuffer* GetMasterBuffer () const
  {
    return masterBuffer;
  }

  virtual bool IsIndexBuffer() const
  { return props.isIndex; }

  virtual size_t GetRangeStart() const
  { return rangeStart; }
  virtual size_t GetRangeEnd() const
  { return rangeEnd; }

  virtual size_t GetElementCount() const;

  /**
   * Create a render buffer.
   * \param elementCount Number of elements in the buffer.
   * \param type Type of buffer; CS_BUF_DYNAMIC, CS_BUF_STATIC or 
   *  CS_BUF_STREAM.
   * \param componentType Components Types; CS_BUFCOMP_FLOAT, CS_BUFCOMP_INT,
   *        etc
   * \param componentCount Number of components per element (e.g. 4 for RGBA)
   * \param copy if true (default) then this buffer will make a copy of the
   *        data, else just svae the buffer pointers provided by the caller.
   *        This has some implications: CopyInto() does not copy, merely update
   *	    the internal buffer pointer. Lock() just returns that pointer.
   *        The pointer passed to CopyInto() must be valid over the lifetime of
   *        the render buffer.
   */
  static csRef<iRenderBuffer> CreateRenderBuffer (size_t elementCount, 
    csRenderBufferType type, csRenderBufferComponentType componentType, 
    uint componentCount, bool copy = true);
  /**
   * Create an index buffer.
   * \param elementCount Number of elements in the buffer.
   * \param type Type of buffer; CS_BUF_DYNAMIC, CS_BUF_STATIC or 
   *  CS_BUF_STREAM.
   * \param componentType Components Types; usually CS_BUFCOMP_UNSIGNED_INT
   * \param rangeStart Minimum index value that is expected to be written to 
   *  the created buffer.
   * \param rangeEnd Maximum index value that is expected to be written to 
   *  the created buffer.
   * \param copy if true (default) then this buffer will make a copy of the
   *        data, else just svae the buffer pointers provided by the caller.
   *        This has some implications: CopyInto() does not copy, merely update
   *	    the internal buffer pointer. Lock() just returns that pointer.
   *        The pointer passed to CopyInto() must be valid over the lifetime of
   *        the render buffer.
   */
  static csRef<iRenderBuffer> CreateIndexRenderBuffer (size_t elementCount, 
    csRenderBufferType type, csRenderBufferComponentType componentType,
    size_t rangeStart, size_t rangeEnd, bool copy = true);
  /**
   * Create an interleaved renderbuffer (You would use this then set stride to
   * determine offset and stride of the interleaved buffer
   * \param elementCount Number of elements in the buffer.
   * \param type Type of buffer; CS_BUF_DYNAMIC, CS_BUF_STATIC or 
   *  CS_BUF_STREAM.
   * \param count number of render buffers you want
   * \param elements Array of csInterleavedSubBufferOptions describing the 
   *  properties of the individual buffers to be interleaved.
   * \param buffers an array of render buffer references that can hold
   *  at least 'count' render buffers.
   *
   * \code
   *  static const csInterleavedSubBufferOptions interleavedElements[2] =
   *    {{CS_BUFCOMP_FLOAT, 3}, {CS_BUFCOMP_FLOAT, 2}};
   *  csRef<iRenderBuffer> buffers[2];
   *  csRenderBuffer::CreateInterleavedRenderBuffers (num_verts, CS_BUF_STATIC,
   *    2, interleavedElements, buffers);
   *  csRef<iRenderBuffer> vertex_buffer = buffers[0];
   *  csRef<iRenderBuffer> texel_buffer = buffers[1];
   * \endcode
   */
  static csRef<iRenderBuffer> CreateInterleavedRenderBuffers (
    size_t elementCount, 
    csRenderBufferType type, uint count, 
    const csInterleavedSubBufferOptions* elements, 
    csRef<iRenderBuffer>* buffers);

  /**
   * Utility to retrieve the "friendly" string name of a buffer description,
   * e.g. "position" for CS_BUFFER_POSITION.
   */
  static const char* GetDescrFromBufferName (csRenderBufferName bufferName);
  /**
   * Retrieve the buffer name for a "friendly" buffer description.
   * Can be used to parse e.g. shader files.
   */
  static csRenderBufferName GetBufferNameFromDescr (const char* name);
protected:
  /// Total size of the buffer
  size_t bufferSize;

  /**
   * To scrape off a few bytes use bitfields; assumes values are in sane 
   * limits.
   */
  struct Props
  {
    /// hint about main usage
    csRenderBufferType bufferType : 2;
    /// datatype for each component
    csRenderBufferComponentType comptype : 4; 
  
    /// number of components per element
    uint compCount : 8;
    /// buffer stride
    size_t stride : 8;
    /// offset from buffer start to data
    size_t offset : 8;

    /// should we copy data, or just use supplied buffer
    bool doCopy : 1; 
    /// if buffer should be deleted on deallocation
    bool doDelete : 1;
    /// currently locked? (to prevent recursive locking)
    bool isLocked : 1;
    /// if this is index-buffer  
    bool isIndex : 1;

    /// last type of lock used
    csRenderBufferLockType lastLock : 2;

    Props (csRenderBufferType type, csRenderBufferComponentType componentType,
      uint componentCount, bool copy) : bufferType (type), 
      comptype (componentType), compCount (componentCount), stride(0), 
      offset (0), doCopy (copy), doDelete (false), isLocked (false), 
      isIndex (false) 
    {
      CS_ASSERT(componentCount <= 255); // Just to be sure...
    }
  } props;

  /// range start for index-buffer
  size_t rangeStart; 
  /// range start for index-buffer
  size_t rangeEnd; 
  
  /// modification number
  unsigned int version; 

  /// buffer holding the data
  unsigned char *buffer; 
  
  csRef<iRenderBuffer> masterBuffer;
#ifdef CS_DEBUG
  csCallStack* lockStack;
#endif
};

/** @} */

#endif // __CS_CSGFX_RENDERBUFFER_H__
