/*
Copyright (C) 2007 by Michael Gist

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
#include "csutil/processorspecdetection.h"

int main()
{
    CS::Platform::ProcessorSpecDetection detect;

    if(detect.HasMMX())
    {
        printf("MMX is supported! :-)\n");
    }
    else
    {
        printf("MMX is not supported! :-( \n");
    }
    if(detect.HasSSE())
    {
        printf("SSE is supported! :-)\n");
    }
    else
    {
        printf("SSE is not supported! :-(\n");
    }
    if(detect.HasSSE2())
    {
        printf("SSE2 is supported! :-)\n");
    }
    else
    {
        printf("SSE2 is not supported! :-(\n");
    }
    if(detect.HasSSE3())
    {
        printf("SSE3 is supported! :-)\n");
    }
    else
    {
        printf("SSE3 is not supported! :-(\n");
    }
    if(detect.HasAltiVec())
    {
        printf("AltiVec is supported! :-)\n");
    }
    else
    {
        printf("AltiVec is not supported! :-(\n");
    }
    return 0;
}
