#!/usr/bin/env python
import sys
import xml.dom.minidom
from xml.dom.minidom import Node

C_NS = "http://www.gtk.org/introspection/c/1.0"

# enum types to dump (.gir names)
enum_types = (
    "BufferBit",
    "ColorMask",
    "Driver",
    "PipelineAlphaFunc",
    "PipelineCullFaceMode",
    "PipelineFilter",
    "PipelineWrapMode",
    "PixelFormat",
    "RendererConstraint",
    "TextureFlags",
    "VerticesMode",
    "Winding",
    "WinsysID",
)

# object types to dump (.gir names)
object_types = (
    "Context",
    "Display",
    "Framebuffer",
    "Onscreen",
    "OnscreenTemplate",
    "Pipeline",
    "Renderer",
    "SwapChain",
    "Texture",
)

# The struct types (value types) are written by hand
struct_types = (
    'Color',
    'Matrix',
    'Quaternion',
)

hand_written_types = (
    'TextureType'
)

# maps from .gir names to cogl-sharp types/methods
name_overrides = {
    'Display': {
        'handwritten': (
            'setup'
        ),
    },
    'Framebuffer': {
        'class': 'FrameBuffer',
        'methods': {
            'clear4f': 'clear'
        },
        'handwritten': (
            'draw_textured_rectangles',
        ),
        'blacklist': (
            'vdraw_attributes', 'draw_rectangles'
        )
    },
    'Onscreen': {
        'class': 'OnScreen',
        'blacklist': (
            'swap_region',
        )
    },
    'OnscreenTemplate': {
        'class': 'OnScreenTemplate',
    },
    'Offscreen': {
        'class': 'OffScreen'
    },
    'Pipeline': {
        'methods': {
            'set_color4f': 'set_color'
        },
        'blacklist': (
            'set_uniform_float', 'set_uniform_int', 'set_uniform_matrix'
        )
    },
    'PixelFormat': {
        'uppercase': 1
    },
    'Renderer': {
        'handwritten': (
            'check_onscreen_template', 'connect'
        )
    },
}

basic_types_map = {
    'gfloat': 'float',
    'gint': 'int',
    'guint': 'uint',
    'Bool': 'bool', # CoglBool is 4 bytes, which the the default marshalling
                    # for C#'s bool (which itself is 1 byte).
}

header_enum="""/* This file has been generated by parse-gir.py, do not hand edit */
using System;

namespace Cogl
{
"""
footer_enum="""}
"""

def make_enum_name(enum_name, member_name):
    uppercase = False
    if enum_name in name_overrides and 'uppercase' in name_overrides[enum_name]:
        uppercase = True

    if uppercase and member_name != 'any':
        return member_name.upper()

    words = member_name.split('_')
    return "".join(map(lambda x: x.capitalize(), words))

def do_generate_enum(node, is_flags):
        type_name = node.getAttribute("name")
        if type_name not in enum_types:
            return

        print("Generate %s" % type_name + ".cs")

        fo = open(type_name + ".cs", "w+")
        fo.write(header_enum)
        if is_flags:
            fo.write("    [Flags]\n")
        fo.write("    public enum %s\n" % type_name)
        fo.write("    {\n")
        members = node.getElementsByTagName("member")
        n = 1
        for member in members:
            member_name = member.getAttribute("name")
            value = member.getAttribute("value")
            enum_name = make_enum_name(type_name, member_name)
            if n < len(members):
                fo.write("        %s = %s,\n" % (enum_name, value))
            else:
                fo.write("        %s = %s\n" % (enum_name, value))
            n += 1
        fo.write("    }\n")
        fo.write(footer_enum)
        fo.close()

def generate_enumerations(doc):
    for node in doc.getElementsByTagName("enumeration"):
        do_generate_enum(node, False)

def generate_bitfields(doc):
    for node in doc.getElementsByTagName("bitfield"):
        do_generate_enum(node, True)

header_class="""/* This file has been generated by parse-gir.py, do not hand edit */
using System;
using System.Runtime.InteropServices;

namespace Cogl
{
"""
footer_class="""}
"""

def make_method_name(gir_name):
    words = gir_name.split('_')
    return "".join(map(lambda x: x.capitalize(), words))

def known_type(gir_name):
    if not gir_name:
        return False

    if gir_name == "none":
        return True;

    return (gir_name in enum_types or
            gir_name in object_types or
            gir_name in basic_types_map or
            gir_name in struct_types or
            gir_name in hand_written_types)

def is_pointer_type(c_type):
    return c_type.endswith("*")

def derive_native_type(gir_type, c_type):
    if gir_type == 'none':
        return 'void'

    if gir_type in struct_types:
        return gir_type

    if is_pointer_type(c_type):
        return 'IntPtr'

    if gir_type in enum_types:
        return gir_type

    if gir_type in hand_written_types:
        return gir_type

    if gir_type in basic_types_map:
        return basic_types_map[gir_type]

    print("Error: trying to derivate (%s,%s)" % (gir_type, c_type))
    assert False

def derive_cs_type(gir_type, c_type):
    if gir_type == 'none':
        return 'void'

    if gir_type in basic_types_map:
        return basic_types_map[gir_type]

    return gir_type

def apply_name_override(gir_name, overrides):
    if not overrides or not 'methods' in overrides:
        return gir_name

    if not gir_name in overrides['methods']:
        return gir_name

    return overrides['methods'][gir_name]

def is_blacklisted(gir_name, overrides):
    if not overrides:
        return False

    if not 'blacklist' in overrides:
        return False

    return gir_name in overrides['blacklist']

def is_handwritten(gir_name, overrides):
    if not overrides:
        return False

    if not 'handwritten' in overrides:
        return False

    return gir_name in overrides['handwritten']

def generate_method(node, overrides, fo):
    gir_name = node.getAttribute("name")

    gir_name = apply_name_override(gir_name, overrides)

    native_method_name = node.getAttributeNS(C_NS, "identifier")
    native_return_value = "void"
    native_params = ['IntPtr o']
    cs_method_name = make_method_name(gir_name)
    cs_return_value = "void"
    cs_params = []
    call_params = ['handle']

    if is_handwritten(gir_name, overrides):
        return

    if is_blacklisted(gir_name, overrides):
        print("  Skipping %s, blacklisted" % cs_method_name)
        return

    # Let's figure out if we can generate that method (ie if we know how to
    # handle the types of the return value and of the parameters).
    # At the same time, we compute the nececessary return value and parameter
    # strings for the generation code below.

    # Fist let's start with the return value, ...
    return_value = node.getElementsByTagName("return-value")
    assert len(return_value) == 1
    return_value = return_value.item(0)
    return_type = return_value.getElementsByTagName("type").item(0)
    return_type_name = return_type.getAttribute("name")
    return_c_type = return_type.getAttributeNS(C_NS, "type")

    if not known_type(return_type_name):
        print("  Skipping %s, unknown return type '%s' (%s)" %
              (cs_method_name, return_type_name, return_c_type))
        return

    native_return_value = derive_native_type(return_type_name, return_c_type)
    cs_return_value = derive_cs_type(return_type_name, return_c_type)

    return_object = return_type_name in object_types

    # ... then the parameters
    params_list = node.getElementsByTagName("parameters")
    assert len(params_list) <= 1
    params_list = params_list.item(0)

    generatable = True
    if params_list:
        params = params_list.getElementsByTagName("parameter")
        for param in params:
            direction = param.getAttribute("direction")
            if direction == 'out':
                print("  Skipping %s, out parameters not supported yet" %
                      (cs_method_name))
                break

            param_type = param.getElementsByTagName("type")
            assert len(param_type) == 1
            c_type = param_type.item(0).getAttributeNS(C_NS, "type")
            gir_type = param_type.item(0).getAttribute("name")
            if not known_type(gir_type):
                print("  Skipping %s, unknown parameter type '%s' (%s)" %
                      (cs_method_name, gir_type, c_type))
                generatable = False
                break

            # time to update {native,cs}_params and call_params
            param_name = param.getAttribute("name")
            ref = 'ref ' if gir_type in struct_types else ''
            t = derive_native_type(gir_type, c_type)
            native_params.append(ref + t + ' ' + param_name)

            t = derive_cs_type(gir_type, c_type)
            cs_params.append(ref + t + ' ' + param_name)

            handle = '.Handle' if gir_type in object_types else ''
            call_params.append(ref + param_name + handle)

    if not generatable:
        return

    return_str = 'return ' if (cs_return_value != 'void') else ''

    fo.write("        [DllImport(\"cogl2.dll\")]\n")
    fo.write("        public static extern %s %s(%s);\n\n" %
            (native_return_value, native_method_name, ", ".join(native_params)))

    fo.write("        public %s %s(%s)\n" %
             (cs_return_value, cs_method_name, ", ".join(cs_params)))
    fo.write("        {\n")
    if not return_object:
        fo.write("            %s%s(%s);\n" %
                 (return_str, native_method_name, ", ".join(call_params)))
    else:
        fo.write("            IntPtr p = %s(%s);\n" %
                 (native_method_name, ", ".join(call_params)))
        fo.write("            return new %s(p);\n" % cs_return_value)
    fo.write("        }\n\n")

def generate_classes(doc):
    for node in doc.getElementsByTagName("record"):
        type_name = node.getAttribute("name")
        if type_name not in object_types:
            continue

        overrides = None
        if type_name in name_overrides:
            overrides = name_overrides[type_name]
        if overrides and 'class' in overrides:
            type_name = overrides['class']

        print("Generate _%s" % type_name + ".cs")

        fo = open("_" + type_name + ".cs", "w+")
        fo.write(header_class)
        fo.write("    public partial class %s\n" % type_name)
        fo.write("    {\n")
        for method in node.getElementsByTagName("method"):
            generate_method(method, overrides, fo)
        fo.write("    }\n")
        fo.write(footer_class)
        fo.close()

doc = xml.dom.minidom.parse(sys.argv[1])
generate_enumerations(doc)
generate_bitfields(doc)
generate_classes(doc)
