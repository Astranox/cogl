/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2010 Intel Corporation.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cogl-util.h"
#include "cogl-object-private.h"
#include "cogl-primitive.h"
#include "cogl-primitive-private.h"
#include "cogl-attribute-private.h"

#include <stdarg.h>
#include <string.h>

static void _cogl_primitive_free (CoglPrimitive *primitive);

COGL_OBJECT_DEFINE (Primitive, primitive);

CoglPrimitive *
cogl_primitive_new_with_attributes (CoglVerticesMode mode,
                                    int n_vertices,
                                    CoglAttribute **attributes,
                                    int n_attributes)
{
  CoglPrimitive *primitive;
  int i;

  primitive = g_slice_alloc (sizeof (CoglPrimitive) +
                             sizeof (CoglAttribute *) * (n_attributes - 1));
  primitive->mode = mode;
  primitive->first_vertex = 0;
  primitive->n_vertices = n_vertices;
  primitive->indices = NULL;
  primitive->immutable_ref = 0;

  primitive->n_attributes = n_attributes;
  primitive->n_embedded_attributes = n_attributes;
  primitive->attributes = &primitive->embedded_attribute;
  for (i = 0; i < n_attributes; i++)
    {
      CoglAttribute *attribute = attributes[i];
      cogl_object_ref (attribute);

      _COGL_RETURN_VAL_IF_FAIL (cogl_is_attribute (attribute), NULL);

      primitive->attributes[i] = attribute;
    }

  return _cogl_primitive_object_new (primitive);
}

/* This is just an internal convenience wrapper around
   new_with_attributes that also unrefs the attributes. It is just
   used for the builtin struct constructors */
static CoglPrimitive *
_cogl_primitive_new_with_attributes_unref (CoglVerticesMode mode,
                                           int n_vertices,
                                           CoglAttribute **attributes,
                                           int n_attributes)
{
  CoglPrimitive *primitive;
  int i;

  primitive = cogl_primitive_new_with_attributes (mode,
                                                  n_vertices,
                                                  attributes,
                                                  n_attributes);

  for (i = 0; i < n_attributes; i++)
    cogl_object_unref (attributes[i]);

  return primitive;
}

CoglPrimitive *
cogl_primitive_new (CoglVerticesMode mode,
                    int n_vertices,
                    ...)
{
  va_list ap;
  int n_attributes;
  CoglAttribute **attributes;
  int i;
  CoglAttribute *attribute;

  va_start (ap, n_vertices);
  for (n_attributes = 0; va_arg (ap, CoglAttribute *); n_attributes++)
    ;
  va_end (ap);

  attributes = g_alloca (sizeof (CoglAttribute *) * n_attributes);

  va_start (ap, n_vertices);
  for (i = 0; (attribute = va_arg (ap, CoglAttribute *)); i++)
    attributes[i] = attribute;
  va_end (ap);

  return cogl_primitive_new_with_attributes (mode, n_vertices,
                                             attributes,
                                             i);
}

CoglPrimitive *
cogl_primitive_new_p2 (CoglVerticesMode mode,
                       int n_vertices,
                       const CoglVertexP2 *data)
{
  CoglAttributeBuffer *attribute_buffer =
    cogl_attribute_buffer_new (n_vertices * sizeof (CoglVertexP2), data);
  CoglAttribute *attributes[1];

  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (CoglVertexP2),
                                      offsetof (CoglVertexP2, x),
                                      2,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);

  cogl_object_unref (attribute_buffer);

  return _cogl_primitive_new_with_attributes_unref (mode, n_vertices,
                                                    attributes,
                                                    1);
}

CoglPrimitive *
cogl_primitive_new_p3 (CoglVerticesMode mode,
                       int n_vertices,
                       const CoglVertexP3 *data)
{
  CoglAttributeBuffer *attribute_buffer =
    cogl_attribute_buffer_new (n_vertices * sizeof (CoglVertexP3), data);
  CoglAttribute *attributes[1];

  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (CoglVertexP3),
                                      offsetof (CoglVertexP3, x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);

  cogl_object_unref (attribute_buffer);

  return _cogl_primitive_new_with_attributes_unref (mode, n_vertices,
                                                    attributes,
                                                    1);
}

CoglPrimitive *
cogl_primitive_new_p2c4 (CoglVerticesMode mode,
                         int n_vertices,
                         const CoglVertexP2C4 *data)
{
  CoglAttributeBuffer *attribute_buffer =
    cogl_attribute_buffer_new (n_vertices * sizeof (CoglVertexP2C4), data);
  CoglAttribute *attributes[2];

  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (CoglVertexP2C4),
                                      offsetof (CoglVertexP2C4, x),
                                      2,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_color_in",
                                      sizeof (CoglVertexP2C4),
                                      offsetof (CoglVertexP2C4, r),
                                      4,
                                      COGL_ATTRIBUTE_TYPE_UNSIGNED_BYTE);

  cogl_object_unref (attribute_buffer);

  return _cogl_primitive_new_with_attributes_unref (mode, n_vertices,
                                                    attributes,
                                                    2);
}

CoglPrimitive *
cogl_primitive_new_p3c4 (CoglVerticesMode mode,
                         int n_vertices,
                         const CoglVertexP3C4 *data)
{
  CoglAttributeBuffer *attribute_buffer =
    cogl_attribute_buffer_new (n_vertices * sizeof (CoglVertexP3C4), data);
  CoglAttribute *attributes[2];

  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (CoglVertexP3C4),
                                      offsetof (CoglVertexP3C4, x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_color_in",
                                      sizeof (CoglVertexP3C4),
                                      offsetof (CoglVertexP3C4, r),
                                      4,
                                      COGL_ATTRIBUTE_TYPE_UNSIGNED_BYTE);

  cogl_object_unref (attribute_buffer);

  return _cogl_primitive_new_with_attributes_unref (mode, n_vertices,
                                                    attributes,
                                                    2);
}

CoglPrimitive *
cogl_primitive_new_p2t2 (CoglVerticesMode mode,
                         int n_vertices,
                         const CoglVertexP2T2 *data)
{
  CoglAttributeBuffer *attribute_buffer =
    cogl_attribute_buffer_new (n_vertices * sizeof (CoglVertexP2T2), data);
  CoglAttribute *attributes[2];

  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (CoglVertexP2T2),
                                      offsetof (CoglVertexP2T2, x),
                                      2,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_tex_coord0_in",
                                      sizeof (CoglVertexP2T2),
                                      offsetof (CoglVertexP2T2, s),
                                      2,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);

  cogl_object_unref (attribute_buffer);

  return _cogl_primitive_new_with_attributes_unref (mode, n_vertices,
                                                    attributes,
                                                    2);
}

CoglPrimitive *
cogl_primitive_new_p3t2 (CoglVerticesMode mode,
                         int n_vertices,
                         const CoglVertexP3T2 *data)
{
  CoglAttributeBuffer *attribute_buffer =
    cogl_attribute_buffer_new (n_vertices * sizeof (CoglVertexP3T2), data);
  CoglAttribute *attributes[2];

  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (CoglVertexP3T2),
                                      offsetof (CoglVertexP3T2, x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_tex_coord0_in",
                                      sizeof (CoglVertexP3T2),
                                      offsetof (CoglVertexP3T2, s),
                                      2,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);

  cogl_object_unref (attribute_buffer);

  return _cogl_primitive_new_with_attributes_unref (mode, n_vertices,
                                                    attributes,
                                                    2);
}

CoglPrimitive *
cogl_primitive_new_p2t2c4 (CoglVerticesMode mode,
                           int n_vertices,
                           const CoglVertexP2T2C4 *data)
{
  CoglAttributeBuffer *attribute_buffer =
    cogl_attribute_buffer_new (n_vertices * sizeof (CoglVertexP2T2C4), data);
  CoglAttribute *attributes[3];

  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (CoglVertexP2T2C4),
                                      offsetof (CoglVertexP2T2C4, x),
                                      2,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_tex_coord0_in",
                                      sizeof (CoglVertexP2T2C4),
                                      offsetof (CoglVertexP2T2C4, s),
                                      2,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[2] = cogl_attribute_new (attribute_buffer,
                                      "cogl_color_in",
                                      sizeof (CoglVertexP2T2C4),
                                      offsetof (CoglVertexP2T2C4, r),
                                      4,
                                      COGL_ATTRIBUTE_TYPE_UNSIGNED_BYTE);

  cogl_object_unref (attribute_buffer);

  return _cogl_primitive_new_with_attributes_unref (mode, n_vertices,
                                                    attributes,
                                                    3);
}

CoglPrimitive *
cogl_primitive_new_p3t2c4 (CoglVerticesMode mode,
                           int n_vertices,
                           const CoglVertexP3T2C4 *data)
{
  CoglAttributeBuffer *attribute_buffer =
    cogl_attribute_buffer_new (n_vertices * sizeof (CoglVertexP3T2C4), data);
  CoglAttribute *attributes[3];

  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (CoglVertexP3T2C4),
                                      offsetof (CoglVertexP3T2C4, x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_tex_coord0_in",
                                      sizeof (CoglVertexP3T2C4),
                                      offsetof (CoglVertexP3T2C4, s),
                                      2,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[2] = cogl_attribute_new (attribute_buffer,
                                      "cogl_color_in",
                                      sizeof (CoglVertexP3T2C4),
                                      offsetof (CoglVertexP3T2C4, r),
                                      4,
                                      COGL_ATTRIBUTE_TYPE_UNSIGNED_BYTE);

  cogl_object_unref (attribute_buffer);

  return _cogl_primitive_new_with_attributes_unref (mode, n_vertices,
                                                    attributes,
                                                    3);
}

static void
_cogl_primitive_free (CoglPrimitive *primitive)
{
  int i;

  for (i = 0; i < primitive->n_attributes; i++)
    cogl_object_unref (primitive->attributes[i]);

  if (primitive->attributes != &primitive->embedded_attribute)
    g_slice_free1 (sizeof (CoglAttribute *) * primitive->n_attributes,
                   primitive->attributes);

  g_slice_free1 (sizeof (CoglPrimitive) +
                 sizeof (CoglAttribute *) *
                 (primitive->n_embedded_attributes - 1), primitive);
}

static void
warn_about_midscene_changes (void)
{
  static gboolean seen = FALSE;
  if (!seen)
    {
      g_warning ("Mid-scene modification of primitives has "
                 "undefined results\n");
      seen = TRUE;
    }
}

void
cogl_primitive_set_attributes (CoglPrimitive *primitive,
                               CoglAttribute **attributes,
                               int n_attributes)
{
  int i;

  _COGL_RETURN_IF_FAIL (cogl_is_primitive (primitive));

  if (G_UNLIKELY (primitive->immutable_ref))
    {
      warn_about_midscene_changes ();
      return;
    }

  /* NB: we don't unref the previous attributes before refing the new
   * in case we would end up releasing the last reference for an
   * attribute thats actually in the new list too. */
  for (i = 0; i < n_attributes; i++)
    {
      _COGL_RETURN_IF_FAIL (cogl_is_attribute (attributes[i]));
      cogl_object_ref (attributes[i]);
    }

  for (i = 0; i < primitive->n_attributes; i++)
    cogl_object_unref (primitive->attributes[i]);

  /* First try to use the embedded storage assocated with the
   * primitive, else fallback to slice allocating separate storage for
   * the attribute pointers... */

  if (n_attributes <= primitive->n_embedded_attributes)
    {
      if (primitive->attributes != &primitive->embedded_attribute)
        g_slice_free1 (sizeof (CoglAttribute *) * primitive->n_attributes,
                       primitive->attributes);
      primitive->attributes = &primitive->embedded_attribute;
    }
  else
    {
      if (primitive->attributes != &primitive->embedded_attribute)
        g_slice_free1 (sizeof (CoglAttribute *) * primitive->n_attributes,
                       primitive->attributes);
      primitive->attributes =
        g_slice_alloc (sizeof (CoglAttribute *) * n_attributes);
    }

  memcpy (primitive->attributes, attributes,
          sizeof (CoglAttribute *) * n_attributes);

  primitive->n_attributes = n_attributes;
}

int
cogl_primitive_get_first_vertex (CoglPrimitive *primitive)
{
  _COGL_RETURN_VAL_IF_FAIL (cogl_is_primitive (primitive), 0);

  return primitive->first_vertex;
}

void
cogl_primitive_set_first_vertex (CoglPrimitive *primitive,
                                 int first_vertex)
{
  _COGL_RETURN_IF_FAIL (cogl_is_primitive (primitive));

  if (G_UNLIKELY (primitive->immutable_ref))
    {
      warn_about_midscene_changes ();
      return;
    }

  primitive->first_vertex = first_vertex;
}

int
cogl_primitive_get_n_vertices (CoglPrimitive *primitive)
{
  _COGL_RETURN_VAL_IF_FAIL (cogl_is_primitive (primitive), 0);

  return primitive->n_vertices;
}

void
cogl_primitive_set_n_vertices (CoglPrimitive *primitive,
                               int n_vertices)
{
  _COGL_RETURN_IF_FAIL (cogl_is_primitive (primitive));

  primitive->n_vertices = n_vertices;
}

CoglVerticesMode
cogl_primitive_get_mode (CoglPrimitive *primitive)
{
  _COGL_RETURN_VAL_IF_FAIL (cogl_is_primitive (primitive), 0);

  return primitive->mode;
}

void
cogl_primitive_set_mode (CoglPrimitive *primitive,
                         CoglVerticesMode mode)
{
  _COGL_RETURN_IF_FAIL (cogl_is_primitive (primitive));

  if (G_UNLIKELY (primitive->immutable_ref))
    {
      warn_about_midscene_changes ();
      return;
    }

  primitive->mode = mode;
}

void
cogl_primitive_set_indices (CoglPrimitive *primitive,
                            CoglIndices *indices,
                            int n_indices)
{
  _COGL_RETURN_IF_FAIL (cogl_is_primitive (primitive));

  if (G_UNLIKELY (primitive->immutable_ref))
    {
      warn_about_midscene_changes ();
      return;
    }

  if (indices)
    cogl_object_ref (indices);
  if (primitive->indices)
    cogl_object_unref (primitive->indices);
  primitive->indices = indices;
  primitive->n_vertices = n_indices;
}

CoglPrimitive *
_cogl_primitive_immutable_ref (CoglPrimitive *primitive)
{
  int i;

  _COGL_RETURN_VAL_IF_FAIL (cogl_is_primitive (primitive), NULL);

  primitive->immutable_ref++;

  for (i = 0; i < primitive->n_attributes; i++)
    _cogl_attribute_immutable_ref (primitive->attributes[i]);

  return primitive;
}

void
_cogl_primitive_immutable_unref (CoglPrimitive *primitive)
{
  int i;

  _COGL_RETURN_IF_FAIL (cogl_is_primitive (primitive));
  _COGL_RETURN_IF_FAIL (primitive->immutable_ref > 0);

  primitive->immutable_ref--;

  for (i = 0; i < primitive->n_attributes; i++)
    _cogl_attribute_immutable_unref (primitive->attributes[i]);
}

void
_cogl_primitive_draw (CoglPrimitive *primitive,
                      CoglDrawFlags flags)
{
  if (primitive->indices)
    _cogl_draw_indexed_attributes (primitive->mode,
                                   primitive->first_vertex,
                                   primitive->n_vertices,
                                   primitive->indices,
                                   primitive->attributes,
                                   primitive->n_attributes,
                                   flags);
  else
    _cogl_draw_attributes (primitive->mode,
                           primitive->first_vertex,
                           primitive->n_vertices,
                           primitive->attributes,
                           primitive->n_attributes,
                           flags);
}

/* XXX: cogl_draw_primitive() ? */
void
cogl_primitive_draw (CoglPrimitive *primitive)
{
  _cogl_primitive_draw (primitive, 0 /* no flags */);
}
