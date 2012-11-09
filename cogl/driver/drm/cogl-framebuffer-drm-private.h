/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2012 Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *
 *
 * Authors:
 *   Robert Bragg <robert@linux.intel.com>
 */

#ifndef _COGL_FRAMEBUFFER_NOP_PRIVATE_H_
#define _COGL_FRAMEBUFFER_NOP_PRIVATE_H_

#include "cogl-types.h"
#include "cogl-context-private.h"

CoglBool
_cogl_offscreen_drm_allocate (CoglOffscreen *offscreen,
                             CoglError **error);

void
_cogl_offscreen_drm_free (CoglOffscreen *offscreen);

void
_cogl_framebuffer_drm_flush_state (CoglFramebuffer *draw_buffer,
                                   CoglFramebuffer *read_buffer,
                                   CoglFramebufferState state);

void
_cogl_framebuffer_drm_clear (CoglFramebuffer *framebuffer,
                            unsigned long buffers,
                            float red,
                            float green,
                            float blue,
                            float alpha);

void
_cogl_framebuffer_drm_query_bits (CoglFramebuffer *framebuffer,
                                 int *red,
                                 int *green,
                                 int *blue,
                                 int *alpha);

void
_cogl_framebuffer_drm_finish (CoglFramebuffer *framebuffer);

void
_cogl_framebuffer_drm_discard_buffers (CoglFramebuffer *framebuffer,
                                       unsigned long buffers);

void
_cogl_framebuffer_drm_draw_attributes (CoglFramebuffer *framebuffer,
                                       CoglPipeline *pipeline,
                                       CoglVerticesMode mode,
                                       int first_vertex,
                                       int n_vertices,
                                       CoglAttribute **attributes,
                                       int n_attributes,
                                       CoglDrawFlags flags);

void
_cogl_framebuffer_drm_draw_indexed_attributes (CoglFramebuffer *framebuffer,
                                               CoglPipeline *pipeline,
                                               CoglVerticesMode mode,
                                               int first_vertex,
                                               int n_vertices,
                                               CoglIndices *indices,
                                               CoglAttribute **attributes,
                                               int n_attributes,
                                               CoglDrawFlags flags);

CoglBool
_cogl_framebuffer_drm_read_pixels_into_bitmap (CoglFramebuffer *framebuffer,
                                               int x,
                                               int y,
                                               CoglReadPixelsFlags source,
                                               CoglBitmap *bitmap,
                                               CoglError **error);

#endif /* _COGL_FRAMEBUFFER_NOP_PRIVATE_H_ */
