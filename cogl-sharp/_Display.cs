/* This file has been generated by parse-gir.py, do not hand edit */
using System;
using System.Runtime.InteropServices;

namespace Cogl
{
    public partial class Display
    {
        [DllImport("cogl2.dll")]
        public static extern IntPtr cogl_display_get_renderer(IntPtr o);

        public Renderer GetRenderer()
        {
            IntPtr p = cogl_display_get_renderer(handle);
            return new Renderer(p);
        }

    }
}
