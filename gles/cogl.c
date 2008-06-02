/*
 * Clutter COGL
 *
 * A basic GL/GLES Abstraction/Utility Layer
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *
 * Copyright (C) 2007 OpenedHand
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cogl.h"

#include <string.h>

#include "cogl-internal.h"
#include "cogl-util.h"
#include "cogl-context.h"

#include "cogl-gles2-wrapper.h"

/* GL error to string conversion */
#if COGL_DEBUG
struct token_string
{
  GLuint Token;
  const char *String;
};

static const struct token_string Errors[] = {
  { GL_NO_ERROR, "no error" },
  { GL_INVALID_ENUM, "invalid enumerant" },
  { GL_INVALID_VALUE, "invalid value" },
  { GL_INVALID_OPERATION, "invalid operation" },
  { GL_STACK_OVERFLOW, "stack overflow" },
  { GL_STACK_UNDERFLOW, "stack underflow" },
  { GL_OUT_OF_MEMORY, "out of memory" },
#ifdef GL_INVALID_FRAMEBUFFER_OPERATION_EXT
  { GL_INVALID_FRAMEBUFFER_OPERATION_EXT, "invalid framebuffer operation" },
#endif
  { ~0, NULL }
};

const char*
_cogl_error_string(GLenum errorCode)
{
  int i;
  for (i = 0; Errors[i].String; i++) {
    if (Errors[i].Token == errorCode)
      return Errors[i].String;
  }
  return "unknown";
}
#endif


CoglFuncPtr
cogl_get_proc_address (const gchar* name)
{
  return NULL;
}

gboolean 
cogl_check_extension (const gchar *name, const gchar *ext)
{
  return FALSE;
}

void
cogl_paint_init (const ClutterColor *color)
{
#if COGL_DEBUG
  fprintf(stderr, "\n ============== Paint Start ================ \n");
#endif

  cogl_wrap_glClearColorx ((color->red << 16) / 0xff, 
			   (color->green << 16) / 0xff,
			   (color->blue << 16) / 0xff, 
			   0xff);

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  cogl_wrap_glDisable (GL_LIGHTING);
  cogl_wrap_glDisable (GL_FOG);
}

/* FIXME: inline most of these  */
void
cogl_push_matrix (void)
{
  GE( cogl_wrap_glPushMatrix() );
}

void
cogl_pop_matrix (void)
{
  GE( cogl_wrap_glPopMatrix() );
}

void
cogl_scale (ClutterFixed x, ClutterFixed y)
{
  GE( cogl_wrap_glScalex (x, y, CFX_ONE) );
}

void
cogl_translatex (ClutterFixed x, ClutterFixed y, ClutterFixed z)
{
  GE( cogl_wrap_glTranslatex (x, y, z) );
}

void
cogl_translate (gint x, gint y, gint z)
{
  GE( cogl_wrap_glTranslatex (CLUTTER_INT_TO_FIXED(x), 
			      CLUTTER_INT_TO_FIXED(y), 
			      CLUTTER_INT_TO_FIXED(z)) );
}

void
cogl_rotatex (ClutterFixed angle, 
	      ClutterFixed x, 
	      ClutterFixed y, 
	      ClutterFixed z)
{
  GE( cogl_wrap_glRotatex (angle,x,y,z) );
}

void
cogl_rotate (gint angle, gint x, gint y, gint z)
{
  GE( cogl_wrap_glRotatex (CLUTTER_INT_TO_FIXED(angle),
		 CLUTTER_INT_TO_FIXED(x), 
		 CLUTTER_INT_TO_FIXED(y), 
		 CLUTTER_INT_TO_FIXED(z)) );
}

static inline gboolean
cogl_toggle_flag (CoglContext *ctx,
		  gulong new_flags,
		  gulong flag,
		  GLenum gl_flag)
{
  /* Toggles and caches a single enable flag on or off
   * by comparing to current state
   */
  if (new_flags & flag)
    {
      if (!(ctx->enable_flags & flag))
	{
	  GE( cogl_wrap_glEnable (gl_flag) );
	  ctx->enable_flags |= flag;
	  return TRUE;
	}
    }
  else if (ctx->enable_flags & flag)
    {
      GE( cogl_wrap_glDisable (gl_flag) );
      ctx->enable_flags &= ~flag;
    }
  
  return FALSE;
}

static inline gboolean
cogl_toggle_client_flag (CoglContext *ctx,
			 gulong new_flags,
			 gulong flag,
			 GLenum gl_flag)
{
  /* Toggles and caches a single client-side enable flag
   * on or off by comparing to current state
   */
  if (new_flags & flag)
    {
      if (!(ctx->enable_flags & flag))
	{
	  GE( cogl_wrap_glEnableClientState (gl_flag) );
	  ctx->enable_flags |= flag;
	  return TRUE;
	}
    }
  else if (ctx->enable_flags & flag)
    {
      GE( cogl_wrap_glDisableClientState (gl_flag) );
      ctx->enable_flags &= ~flag;
    }
  
  return FALSE;
}

void
cogl_enable (gulong flags)
{
  /* This function essentially caches glEnable state() in the
   * hope of lessening number GL traffic.
  */
  _COGL_GET_CONTEXT (ctx, NO_RETVAL);
  
  cogl_toggle_flag (ctx, flags,
                    COGL_ENABLE_BLEND,
                    GL_BLEND);
  
  cogl_toggle_flag (ctx, flags,
		    COGL_ENABLE_TEXTURE_2D,
		    GL_TEXTURE_2D);
  
  cogl_toggle_client_flag (ctx, flags,
			   COGL_ENABLE_VERTEX_ARRAY,
			   GL_VERTEX_ARRAY);
  
  cogl_toggle_client_flag (ctx, flags,
			   COGL_ENABLE_TEXCOORD_ARRAY,
			   GL_TEXTURE_COORD_ARRAY);

  cogl_toggle_client_flag (ctx, flags,
			   COGL_ENABLE_COLOR_ARRAY,
			   GL_COLOR_ARRAY);
}

gulong
cogl_get_enable ()
{
  _COGL_GET_CONTEXT (ctx, 0);
  
  return ctx->enable_flags;
}

void
cogl_blend_func (COGLenum src_factor, COGLenum dst_factor)
{
  /* This function caches the blending setup in the
   * hope of lessening GL traffic.
   */
  _COGL_GET_CONTEXT (ctx, NO_RETVAL);
  
  if (ctx->blend_src_factor != src_factor ||
      ctx->blend_dst_factor != dst_factor)
    {
      glBlendFunc (src_factor, dst_factor);
      ctx->blend_src_factor = src_factor;
      ctx->blend_dst_factor = dst_factor;
    }
}

void
cogl_enable_depth_test (gboolean setting)
{
  if (setting)
    {
      cogl_wrap_glEnable (GL_DEPTH_TEST);
      cogl_wrap_glEnable (GL_ALPHA_TEST);
      glDepthFunc (GL_LEQUAL);
      cogl_wrap_glAlphaFunc (GL_GREATER, 0.1);
    }
  else
    {
      cogl_wrap_glDisable (GL_DEPTH_TEST);
      cogl_wrap_glDisable (GL_ALPHA_TEST);
    }
}

void
cogl_color (const ClutterColor *color)
{
  _COGL_GET_CONTEXT (ctx, NO_RETVAL);
  
#if 0 /*HAVE_GLES_COLOR4UB*/

  /* NOTE: seems SDK_OGLES-1.1_LINUX_PCEMULATION_2.02.22.0756 has this call 
   * but is broken - see #857. Therefor disabling. 
  */

  /* 
   * GLES 1.1 does actually have this function, it's in the header file but
   * missing in the reference manual (and SDK):
   *
   * http://www.khronos.org/egl/headers/1_1/gl.h
   */
  GE( glColor4ub (color->red,
		  color->green,
		  color->blue,
		  color->alpha) );


#else
  /* conversion can cause issues with picking on some gles implementations */
  GE( cogl_wrap_glColor4x ((color->red << 16) / 0xff,  
			   (color->green << 16) / 0xff,  
			   (color->blue << 16) / 0xff,   
			   (color->alpha << 16) / 0xff));
#endif
  
  /* Store alpha for proper blending enables */
  ctx->color_alpha = color->alpha;
}

void
cogl_clip_set (ClutterFixed x_offset,
               ClutterFixed y_offset,
               ClutterFixed width,
               ClutterFixed height)
{
  if (cogl_features_available (COGL_FEATURE_FOUR_CLIP_PLANES))
    {
      GLfixed eqn_left[4] = { CFX_ONE, 0, 0, -x_offset };
      GLfixed eqn_right[4] = { -CFX_ONE, 0, 0,  x_offset + width };
      GLfixed eqn_top[4] = { 0, CFX_ONE, 0, -y_offset };
      GLfixed eqn_bottom[4] = { 0, -CFX_ONE, 0,  y_offset + height };

      GE( cogl_wrap_glClipPlanex (GL_CLIP_PLANE0, eqn_left) );
      GE( cogl_wrap_glClipPlanex (GL_CLIP_PLANE1, eqn_right) );
      GE( cogl_wrap_glClipPlanex (GL_CLIP_PLANE2, eqn_top) );
      GE( cogl_wrap_glClipPlanex (GL_CLIP_PLANE3, eqn_bottom) );
      GE( cogl_wrap_glEnable (GL_CLIP_PLANE0) );
      GE( cogl_wrap_glEnable (GL_CLIP_PLANE1) );
      GE( cogl_wrap_glEnable (GL_CLIP_PLANE2) );
      GE( cogl_wrap_glEnable (GL_CLIP_PLANE3) );
    }
  else if (cogl_features_available (COGL_FEATURE_STENCIL_BUFFER))
    {
      GE( cogl_wrap_glEnable (GL_STENCIL_TEST) );

      GE( glClearStencil (0) );
      GE( glClear (GL_STENCIL_BUFFER_BIT) );

      GE( glStencilFunc (GL_NEVER, 0x1, 0x1) );
      GE( glStencilOp (GL_INCR, GL_INCR, GL_INCR) );

      GE( cogl_wrap_glColor4x (CFX_ONE, CFX_ONE, CFX_ONE, CFX_ONE ) );

      cogl_rectanglex (x_offset, y_offset, width, height);

      GE( glStencilFunc (GL_EQUAL, 0x1, 0x1) );
      GE( glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP) );
    }
}

void
cogl_clip_unset (void)
{
  if (cogl_features_available (COGL_FEATURE_FOUR_CLIP_PLANES))
    {
      GE( cogl_wrap_glDisable (GL_CLIP_PLANE3) );
      GE( cogl_wrap_glDisable (GL_CLIP_PLANE2) );
      GE( cogl_wrap_glDisable (GL_CLIP_PLANE1) );
      GE( cogl_wrap_glDisable (GL_CLIP_PLANE0) );
    }
  else if (cogl_features_available (COGL_FEATURE_STENCIL_BUFFER))
    {
      GE( cogl_wrap_glDisable (GL_STENCIL_TEST) );
    }
}

void
cogl_alpha_func (COGLenum     func, 
		 ClutterFixed ref)
{
  GE( cogl_wrap_glAlphaFunc (func, CLUTTER_FIXED_TO_FLOAT(ref)) );
}

/*
 * Fixed point implementation of the perspective function
 */
void
cogl_perspective (ClutterFixed fovy,
		  ClutterFixed aspect,
		  ClutterFixed zNear,
		  ClutterFixed zFar)
{
  ClutterFixed xmax, ymax;
  ClutterFixed x, y, c, d;
  ClutterFixed fovy_rad_half = CFX_MUL (fovy, CFX_PI) / 360;

  GLfixed m[16];
  
  memset (&m[0], 0, sizeof (m));

  /*
   * Based on the original algorithm in perspective():
   * 
   * 1) xmin = -xmax => xmax + xmin == 0 && xmax - xmin == 2 * xmax
   * same true for y, hence: a == 0 && b == 0;
   *
   * 2) When working with small numbers, we can are loosing significant
   * precision, hence we use clutter_qmulx() here, not the fast macro.
   */
  ymax = clutter_qmulx (zNear, CFX_DIV (clutter_sinx (fovy_rad_half),
					clutter_cosx (fovy_rad_half)));
  xmax = clutter_qmulx (ymax, aspect);

  x = CFX_DIV (zNear, xmax);
  y = CFX_DIV (zNear, ymax);
  c = CFX_DIV (-(zFar + zNear), ( zFar - zNear));
  d = CFX_DIV (-(clutter_qmulx (2*zFar, zNear)), (zFar - zNear));

#define M(row,col)  m[col*4+row]
  M(0,0) = x;
  M(1,1) = y;
  M(2,2) = c;
  M(2,3) = d;
  M(3,2) = 1 + ~CFX_ONE;

  GE( cogl_wrap_glMultMatrixx (m) );
#undef M
}

void
cogl_setup_viewport (guint        w,
		     guint        h,
		     ClutterFixed fovy,
		     ClutterFixed aspect,
		     ClutterFixed z_near,
		     ClutterFixed z_far)
{
  gint width = (gint) w;
  gint height = (gint) h;
  ClutterFixed z_camera;
  
  GE( glViewport (0, 0, width, height) );
  GE( cogl_wrap_glMatrixMode (GL_PROJECTION) );
  GE( cogl_wrap_glLoadIdentity () );

  /* For Ortho projection.
   * cogl_wrap_glOrthox (0, width << 16, 0,  height << 16,  -1 << 16, 1 << 16);
  */

  cogl_perspective (fovy, aspect, z_near, z_far);
  
  GE( cogl_wrap_glMatrixMode (GL_MODELVIEW) );
  GE( cogl_wrap_glLoadIdentity () );

  /*
   * camera distance from screen, 0.5 * tan (FOV)
   *
   * See comments in ../gl/cogl.c
   */
#define DEFAULT_Z_CAMERA 0.869f
  z_camera = CLUTTER_FLOAT_TO_FIXED (DEFAULT_Z_CAMERA);

  if (fovy != CFX_60)
  {
    ClutterFixed fovy_rad = CFX_MUL (fovy, CFX_PI) / 180;
  
    z_camera = CFX_DIV (clutter_sinx (fovy_rad),
			clutter_cosx (fovy_rad)) >> 1;
  }
  

  GE( cogl_wrap_glTranslatex (-1 << 15, -1 << 15, -z_camera));

  GE( cogl_wrap_glScalex ( CFX_ONE / width, 
		 -CFX_ONE / height,
		 CFX_ONE / width));

  GE( cogl_wrap_glTranslatex (0, -CFX_ONE * height, 0) );
}

static void
_cogl_features_init ()
{
  ClutterFeatureFlags flags = 0;
  int                 stencil_bits = 0;
  int                 max_clip_planes = 0;

  _COGL_GET_CONTEXT (ctx, NO_RETVAL);

  GE( cogl_wrap_glGetIntegerv (GL_STENCIL_BITS, &stencil_bits) );
  if (stencil_bits > 0)
    flags |= COGL_FEATURE_STENCIL_BUFFER;

  GE( cogl_wrap_glGetIntegerv (GL_MAX_CLIP_PLANES, &max_clip_planes) );
  if (max_clip_planes >= 4)
    flags |= COGL_FEATURE_FOUR_CLIP_PLANES;

#ifdef HAVE_COGL_GLES2
  flags |= COGL_FEATURE_SHADERS_GLSL;
#endif

  ctx->feature_flags = flags;
  ctx->features_cached = TRUE;
}

ClutterFeatureFlags
cogl_get_features ()
{
  _COGL_GET_CONTEXT (ctx, 0);
  
  if (!ctx->features_cached)
    _cogl_features_init ();
  
  return ctx->feature_flags;
}

gboolean
cogl_features_available (CoglFeatureFlags features)
{
  _COGL_GET_CONTEXT (ctx, 0);
  
  if (!ctx->features_cached)
    _cogl_features_init ();
  
  return (ctx->feature_flags & features) == features;
}

void
cogl_get_modelview_matrix (ClutterFixed m[16])
{
  cogl_wrap_glGetFixedv(GL_MODELVIEW_MATRIX, &m[0]);
}

void
cogl_get_projection_matrix (ClutterFixed m[16])
{
  cogl_wrap_glGetFixedv(GL_PROJECTION_MATRIX, &m[0]);
}

void
cogl_get_viewport (ClutterFixed v[4])
{
  cogl_wrap_glGetFixedv(GL_VIEWPORT, &v[0]);
}

void
cogl_get_bitmasks (gint *red, gint *green, gint *blue, gint *alpha)
{
  if (red)
    GE( cogl_wrap_glGetIntegerv(GL_RED_BITS, red) );
  if (green)
    GE( cogl_wrap_glGetIntegerv(GL_GREEN_BITS, green) );
  if (blue)
    GE( cogl_wrap_glGetIntegerv(GL_BLUE_BITS, blue) );
  if (alpha)
    GE( cogl_wrap_glGetIntegerv(GL_ALPHA_BITS, alpha ) );
}

void
cogl_fog_set (const ClutterColor *fog_color,
              ClutterFixed        density,
              ClutterFixed        z_near,
              ClutterFixed        z_far)
{
  GLfixed fogColor[4];

  fogColor[0] = (fog_color->red   << 16) / 0xff;
  fogColor[1] = (fog_color->green << 16) / 0xff;
  fogColor[2] = (fog_color->blue  << 16) / 0xff;
  fogColor[3] = (fog_color->alpha << 16) / 0xff;

  cogl_wrap_glEnable (GL_FOG);

  cogl_wrap_glFogxv (GL_FOG_COLOR, fogColor);

  cogl_wrap_glFogx (GL_FOG_MODE, GL_LINEAR);
  glHint (GL_FOG_HINT, GL_NICEST);

  cogl_wrap_glFogx (GL_FOG_DENSITY, (GLfixed) density);
  cogl_wrap_glFogx (GL_FOG_START, (GLfixed) z_near);
  cogl_wrap_glFogx (GL_FOG_END, (GLfixed) z_far);
}
