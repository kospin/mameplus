const char glsl_plain_idx16_lut_fsh_src[] =
"\n"
"#pragma optimize (on)\n"
"#pragma debug (off)\n"
"\n"
"uniform sampler2D color_texture;\n"
"uniform sampler2D colortable_texture;\n"
"uniform vec2      colortable_sz;      // ct size\n"
"uniform vec2      colortable_pow2_sz; // pow2 ct size\n"
"\n"
"void main()\n"
"{\n"
"   vec4 color_tex;\n"
"   vec2 color_map_coord;\n"
"\n"
"   // normalized texture coordinates ..\n"
"   color_tex = texture2D(color_texture, gl_TexCoord[0].st);\n"
"\n"
"   // GL_UNSIGNED_SHORT GL_ALPHA in ALPHA16 conversion:\n"
"   // general: f = c / ((2*N)-1), c color bitfield, N number of bits\n"
"   // ushort:  c = ((2**16)-1)*f;\n"
"   color_map_coord.x = floor( 65535.0 * color_tex.a + 0.5 );\n"
"\n"
"   // map it to the 2D lut table\n"
"   color_map_coord.y = floor(color_map_coord.x/colortable_sz.x);\n"
"   color_map_coord.x =   mod(color_map_coord.x,colortable_sz.x);\n"
"\n"
"   gl_FragColor = texture2D(colortable_texture, (color_map_coord+vec2(0.5)) / colortable_pow2_sz);\n"
"}\n"
"\n"
"\n"
;
