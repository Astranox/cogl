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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cogl.h"
#include "cogl-clip-state-private.h"
#include "cogl-framebuffer-private.h"
#include "cogl-journal-private.h"

void
cogl2_clip_push_from_path (CoglPath *path)
{
  CoglFramebuffer *framebuffer;
  CoglClipState *clip_state;
  CoglMatrix modelview_matrix;

  framebuffer = _cogl_get_draw_buffer ();
  clip_state = _cogl_framebuffer_get_clip_state (framebuffer);

  cogl_get_modelview_matrix (&modelview_matrix);

  clip_state->stacks->data =
    _cogl_clip_stack_push_from_path (clip_state->stacks->data,
                                     path,
                                     &modelview_matrix);
}
