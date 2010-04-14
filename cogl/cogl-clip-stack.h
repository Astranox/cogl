/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2007,2008,2009,2010 Intel Corporation.
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef __COGL_CLIP_STACK_H
#define __COGL_CLIP_STACK_H

typedef struct _CoglClipStack CoglClipStack;

CoglClipStack *
_cogl_clip_stack_new (void);

void
_cogl_clip_stack_free (CoglClipStack *stack);

void
_cogl_clip_stack_push_window_rectangle (CoglClipStack *stack,
                                        int x_offset,
                                        int y_offset,
                                        int width,
                                        int height);

void
_cogl_clip_stack_push_rectangle (CoglClipStack *stack,
                                 float x_1,
                                 float y_1,
                                 float x_2,
                                 float y_2,
                                 const CoglMatrix *modelview_matrix);

void
_cogl_clip_stack_push_from_path (CoglClipStack *stack,
                                 CoglHandle path,
                                 const CoglMatrix *modelview_matrix);
void
_cogl_clip_stack_pop (CoglClipStack *stack);

void
_cogl_clip_stack_flush (CoglClipStack *stack,
                        gboolean *stencil_used_p);

#endif /* __COGL_CLIP_STACK_H */
