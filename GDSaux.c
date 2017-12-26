/*
 * gdsreader - simple Calma parser/printer tool
 * Copyright (C) 1999 Serban-Mihai Popescu, serbanp@ix.netcom.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <GDSstructs.h>
#include <GDSconsts.h>
#include <GDSglobals.h>
#include <GDStoPOV.h>
char *
MALLOC(int size)
{
  return (char *)malloc(size);
}

void
FREE(void *ptr)
{
  free(ptr);
}

int
GDSreadInt2(unsigned char* record)
{
  int result;

  result = record[0];
  result <<= 8;
  result += record[1];
  if(result & 0x8000)
  {
    result &= 0x7fff;
    result ^= 0x7fff;
    result += 1;
    result = -result;
  }

  return result;
}

int
GDSreadInt4(unsigned char* record)
{
  int i;
  unsigned int result;

  for(i = 0, result = 0; i < 4; i++)
  {
    result <<= 8;
    result += record[i];
  }
  if(result & 0x80000000)
  {
    result &= 0x7fffffff;
    result ^= 0x7fffffff;
    result += 1;
    result = -result;
  }

  return result;
}

double
GDSreadReal8(unsigned char* record)
{
  int i, sign, exponent;
  unsigned long long mantissa_int;
  double mantissa_float, result;

  sign = record[0] & 0x80;
  exponent = (record[0] & 0x7f) - 64; 
  mantissa_int = 0;

  for(i = 1; i < 8; i++)
  {
    mantissa_int <<= 8;
    mantissa_int += record[i];
  }
  mantissa_float = (double)mantissa_int / pow(2, 56);
  result = mantissa_float * pow(16, (float)exponent);
  if(sign)
    result = -result;

  return result;
}

char *
GDSreadString(unsigned char* record, int len)
{
  char *result, string[1024];
  int i;

  if(len > 1024)
    len = 1024;
  for(i = 0; i < len; i++)
  {
    string[i] = record[i];
    if(record[i] == '\0')
      break;
  }
  string[len] = '\0';

  result = strdup(string);
  return result;
}



const char *POVKeyWords[]={
"aa_level", "aa_threshold", "abs", "absorption", "accuracy", "acos",
"acosh", "adaptive", "adc_bailout", "agate", "agate_turb", "all",
"all_intersections", "alpha", "altitude", "always_sample", "ambient",
"ambient_light", "angle", "aperture", "append", "arc_angle", "area_light",
"array", "asc", "ascii", "asin", "asinh", "assumed_gamma", "atan",
"atan2", "atanh", "autostop", "average", "b_spline", "background",
"bezier_spline", "bicubic_patch", "black_hole", "blob", "blue",
"blur_samples", "bounded_by", "box", "boxed", "bozo", "break", "brick",
"brick_size", "brightness", "brilliance", "bump_map", "bump_size",
"bumps", "camera", "case", "caustics", "ceil", "cells", "charset",
"checker", "chr", "circular", "clipped_by", "clock", "clock_delta",
"clock_on", "collect", "color", "color_map", "colour", "colour_map",
"component", "composite", "concat", "cone", "confidence", "conic_sweep",
"conserve_energy", "contained_by", "control0", "control1", "coords",
"cos", "cosh", "count", "crackle", "crand", "cube", "cubic",
"cubic_spline", "cubic_wave", "cutaway_textures", "cylinder",
"cylindrical", "debug", "declare", "default", "defined", "degrees",
"density", "density_file", "density_map", "dents", "df3", "difference",
"diffuse", "dimension_size", "dimensions", "direction", "disc",
"dispersion", "dispersion_samples", "dist_exp", "distance", "div",
"double_illuminate", "eccentricity", "else", "emission", "end", "error",
"error_bound", "evaluate", "exp", "expand_thresholds", "exponent",
"exterior", "extinction", "face_indices", "facets", "fade_color",
"fade_colour", "fade_distance", "fade_power", "falloff", "falloff_angle",
"false", "fclose", "file_exists", "filter", "final_clock", "final_frame",
"finish", "fisheye", "flatness", "flip", "floor", "focal_point", "fog",
"fog_alt", "fog_offset", "fog_type", "fopen", "form", "frame_number",
"frequency", "fresnel", "function", "gather", "gif", "global_lights",
"global_settings", "gradient", "granite", "gray", "gray_threshold",
"green", "height_field", "hexagon", "hf_gray_16", "hierarchy",
"hypercomplex", "hollow", "if", "ifdef", "iff", "ifndef", "image_height",
"image_map", "image_pattern", "image_width", "include", "initial_clock",
"initial_frame", "inside", "inside_vector", "int", "interior",
"interior_texture", "internal", "interpolate", "intersection",
"intervals", "inverse", "ior", "irid", "irid_wavelength", "isosurface",
"jitter", "jpeg", "julia", "julia_fractal", "lambda", "lathe", "leopard",
"light_group", "light_source", "linear_spline", "linear_sweep", "ln",
"load_file", "local", "location", "log", "look_at", "looks_like",
"low_error_factor", "macro", "magnet", "major_radius", "mandel",
"map_type", "marble", "material", "material_map", "matrix", "max",
"max_extent", "max_gradient", "max_intersections", "max_iteration",
"max_sample", "max_trace", "max_trace_level", "media",
"media_attenuation", "media_interaction", "merge", "mesh", "mesh2",
"metallic", "method", "metric", "min", "min_extent", "minimum_reuse",
"mod", "mortar", "natural_spline", "nearest_count", "no", "no_bump_scale",
"no_image", "no_reflection", "no_shadow", "noise_generator", "normal",
"normal_indices", "normal_map", "normal_vectors", "number_of_waves",
"object", "octaves", "off", "offset", "omega", "omnimax", "on", "once",
"onion", "open", "orient", "orientation", "orthographic", "panoramic",
"parallel", "parametric", "pass_through", "pattern", "perspective", "pgm",
"phase", "phong", "phong_size", "photons", "pi", "pigment", "pigment_map",
"pigment_pattern", "planar", "plane", "png", "point_at", "poly",
"poly_wave", "polygon", "pot", "pow", "ppm", "precision", "precompute",
"pretrace_end", "pretrace_start", "prism", "prod", "projected_through",
"pwr", "quadratic_spline", "quadric", "quartic", "quaternion",
"quick_color", "quick_colour", "quilted", "radial", "radians",
"radiosity", "radius", "rainbow", "ramp_wave", "rand", "range", "ratio",
"read", "reciprocal", "recursion_limit", "red", "reflection",
"reflection_exponent", "refraction", "render", "repeat", "rgb", "rgbf",
"rgbft", "rgbt", "right", "ripples", "rotate", "roughness", "samples",
"save_file", "scale", "scallop_wave", "scattering", "seed", "select",
"shadowless", "sin", "sine_wave", "sinh", "size", "sky", "sky_sphere",
"slice", "slope", "slope_map", "smooth", "smooth_triangle", "solid",
"sor", "spacing", "specular", "sphere", "sphere_sweep", "spherical",
"spiral1", "spiral2", "spline", "split_union", "spotlight", "spotted",
"sqr", "sqrt", "statistics", "str", "strcmp", "strength", "strlen",
"strlwr", "strupr", "sturm", "substr", "sum", "superellipsoid", "switch",
"sys", "t", "tan", "tanh", "target", "text", "texture", "texture_list",
"texture_map", "tga", "thickness", "threshold", "tiff", "tightness",
"tile2", "tiles", "tolerance", "toroidal", "torus", "trace", "transform",
"translate", "transmit", "triangle", "triangle_wave", "true", "ttf",
"turb_depth", "turbulence", "type", "u", "u_steps", "ultra_wide_angle",
"undef", "union", "up", "use_alpha", "use_color", "use_colour",
"use_index", "utf8", "uv_indices", "uv_mapping", "uv_vectors", "v",
"v_steps", "val", "variance", "vaxis_rotate", "vcross", "vdot", "version",
"vertex_vectors", "vlength", "vnormalize", "vrotate", "vstr",
"vturbulence", "warning", "warp", "water_level", "waves", "while",
"width", "wood", "wrinkles", "write", "x", "y", "yes", "z",
NULL
};





/* Returns hashed string.  Uses hex val of chars from right to left */
char *
GetPOVhashedStr(char *s)
{
 int i,l;
 char *retv,cb[2]=" ";
 char c;
 l=strlen(s);
 retv=(char *)MALLOC(sizeof(char)*41);
 retv[0]='X';
 retv[1]='\0';
 i=l;
 while (i>=0 && strlen(retv)<=38) {
  c=(s[i]&0xf0)>>4;
  if (c>9) cb[0]='A'+c-9; else cb[0]='0'+c;
  strcat(retv,cb);
  c=s[i]&0x0f;
  if (c>9) cb[0]='A'+c-9; else cb[0]='0'+c;
  strcat(retv,cb);
  i--;
 }
 return(retv);
}


/* 
 Escapes a string used in GDS format so it cannot conflict with
 POVRAY.  Returns a dynamically allocated string which should be 
 freed using FREE.

 POVRAY identifiers are [A-Za-z0-9_] up to 40 characters long, and 
 the first character must be alphabetic.

 All POVRAY reserved words are lower case.  If there is a upper case
 char anywhere, there is no conflict.

 Any word which is either too long, or would become too due to 
 escaping, will be hashed to a pseudo-random identifier.
 
 I use layer texture identifiers with GDS_TEX_%d, so if that matches,
 we need to fix too.

*/

char *
GDSEscPovString(char *s)
{
 int i,l;
 char *retv=NULL;
 if (s==NULL) return(s);
 if ((l=strlen(s))>40) return(GetPOVhashedStr(s));
 if (!strncmp(s,"GDS_TEX_",8)) { /* close enough to a 
       layer ID to require an escape. */
   return(GetPOVhashedStr(s));
  }
 retv=(char *)MALLOC(sizeof(char)*41);
 strcpy(retv,s);
 if (l==0) return(retv);
 /* Keyword Test */
 i=0;
 while (POVKeyWords[i]!=NULL) {
  if (!strcmp(retv,POVKeyWords[i])) {
    /* We have a keyword and must escape. Can add a 'K' at the
       end to escape .*/
    strcat(retv,"K");
    return(retv);
  }
  i++;
 }
 i=0;
 while (i<l) {
  /* Fix ith character */
  if (!isalpha(retv[i]) && !(i>0 && (isdigit(retv[i]) || retv[i]=='_'))) {
   /* Must escape bad character.  Can use E_ for now.  Later might
       use E__HEXVAL_ to make more reversable. */
   if (l+1<=40) { /* We can still fit it. */
    memmove(i+retv+1,i+retv,l-i+1);
    retv[i]='E';
    retv[i+1]='_';
    l++;
   } else { /* Give up */
    FREE(retv);
    return(GetPOVhashedStr(s));
   }
  }
  i++;
 }
 return(retv);
}







void
GDSaddDates(GDSlibrary* libptr, unsigned char* record)
{
  int my, mmo, md, mh, mmi, ms;
  int ay, amo, ad, ah, ami, as;
  char string[1024];

  my = GDSreadInt2(record + 2);
  mmo = GDSreadInt2(record + 4);
  md = GDSreadInt2(record + 6);
  mh = GDSreadInt2(record + 8);
  mmi = GDSreadInt2(record + 10);
  ms = GDSreadInt2(record + 12);

  ay = GDSreadInt2(record + 14);
  amo = GDSreadInt2(record + 16);
  ad = GDSreadInt2(record + 18);
  ah = GDSreadInt2(record + 20);
  ami = GDSreadInt2(record + 22);
  as = GDSreadInt2(record + 24);

  if (my < 1900) { my  += 1900;}
  if (ay < 1900) { ay  += 1900;}  

  sprintf(string, "%02d/%02d/%02d at %02d:%02d:%02d", mmo, md, my, mh, mmi, ms);
  libptr->lastmod = strdup(string);
  sprintf(string, "%02d/%02d/%02d at %02d:%02d:%02d", amo, ad, ay, ah, ami, as);
  libptr->lastacc = strdup(string);
}

int
GDSrecordType(unsigned char record_type)
{
  int rtype;

  switch(record_type)
  {
    case 0x00:
      rtype = HEADER;
      break;
    case 0x01:
      rtype = BGNLIB;
      break;
    case 0x02:
      rtype = LIBNAME;
      break;
    case 0x03:
      rtype = UNITS;
      break;
    case 0x04:
      rtype = ENDLIB;
      break;
    case 0x05:
      rtype = BGNSTR;
      break;
    case 0x06:
      rtype = STRNAME;
      break;
    case 0x07:
      rtype = ENDSTR;
      break;
    case 0x08:
      rtype = BOUNDARY;
      break;
    case 0x09:
      rtype = PATH;
      break;
    case 0x0a:
      rtype = SREF;
      break;
    case 0x0b:
      rtype = AREF;
      break;
    case 0x0c:
      rtype = TEXT;
      break;
    case 0x0d:
      rtype = LAYER;
      break;
    case 0x0e:
      rtype = DATATYPE;
      break;
    case 0x0f:
      rtype = WIDTH;
      break;
    case 0x10:
      rtype = XY;
      break;
    case 0x11:
      rtype = ENDEL;
      break;
    case 0x12:
      rtype = SNAME;
      break;
    case 0x13:
      rtype = COLROW;
      break;
    case 0x14:
      rtype = TEXTNODE;
      break;
    case 0x15:
      rtype = NODE;
      break;
    case 0x16:
      rtype = TEXTTYPE;
      break;
    case 0x17:
      rtype = PRESENTATION;
      break;
    case 0x18:
      rtype = SPACING;
      break;
    case 0x19:
      rtype = STRING;
      break;
    case 0x1a:
      rtype = STRANS;
      break;
    case 0x1b:
      rtype = MAG;
      break;
    case 0x1c:
      rtype = ANGLE;
      break;
    case 0x1d:
      rtype = UINTEGER;
      break;
    case 0x1e:
      rtype = USTRING;
      break;
    case 0x1f:
      rtype = REFLIBS;
      break;
    case 0x20:
      rtype = FONTS;
      break;
    case 0x21:
      rtype = PATHTYPE;
      break;
    case 0x22:
      rtype = GENERATIONS;
      break;
    case 0x23:
      rtype = ATTRTABLE;
      break;
    case 0x24:
      rtype = STYPTABLE;
      break;
    case 0x25:
      rtype = STRTYPE;
      break;
    case 0x26:
      rtype = ELFLAGS;
      break;
    case 0x27:
      rtype = ELKEY;
      break;
    case 0x28:
      rtype = LINKTYPE;
      break;
    case 0x29:
      rtype = LINKKEYS;
      break;
    case 0x2a:
      rtype = NODETYPE;
      break;
    case 0x2b:
      rtype = PROPATTR;
      break;
    case 0x2c:
      rtype = PROPVALUE;
      break;
    case 0x2d:
      rtype = BOX;
      break;
    case 0x2e:
      rtype = BOXTYPE;
      break;
    case 0x2f:
      rtype = PLEX;
      break;
    case 0x30:
      rtype = BGNEXTN;
      break;
    case 0x31:
      rtype = ENDTEXTN;
      break;
    case 0x32:
      rtype = TAPENUM;
      break;
    case 0x33:
      rtype = TAPECODE;
      break;
    case 0x34:
      rtype = STRCLASS;
      break;
    case 0x35:
      rtype = RESERVED;
      break;
    case 0x36:
      rtype = FORMAT;
      break;
    case 0x37:
      rtype = MASK;
      break;
    case 0x38:
      rtype = ENDMASKS;
      break;
    case 0x39:
      rtype = LIBDIRSIZE;
      break;
    case 0x3a:
      rtype = SRFNAME;
      break;
    case 0x3b:
      rtype = LIBSECUR;
      break;
    default:
      rtype = UNKNOWN;
      break;
    }
  return rtype;
}

void
GDSunreadRecord(int gdsfildes, int nbytes)
{
  lseek(gdsfildes, -nbytes, SEEK_CUR);
  return;
}

int
GDSreadRecord(int gdsfildes, unsigned char **record, int *nbytes)
{
  int nread;
  unsigned char nbyte_array[2];

  if((nread = read(gdsfildes, nbyte_array, 2)) != 2)
  {
    if(nread == 1)
    {
      fprintf(stderr, "Error in reading the GDS file. Aborting\n");
      exit(1);
    }
    else
    {
      *record = NULL;
      return -1;
    }
  }
  *nbytes = nbyte_array[0] * 256 + nbyte_array[1];
  if(*nbytes != 0)
  {
    *record = (unsigned char *)MALLOC(*nbytes - 2);
    if((nread = read(gdsfildes, *record, *nbytes - 2)) != *nbytes - 2)
    {
      fprintf(stderr, "Error in reading the GDS file. Aborting\n");
      exit(1);
    }
  }
  return GDSrecordType((*record)[0]);
}

 
/* Returns dynamically allocated string. */
char *GDSpsStyleNoToTEXName(int i)
{

 const char fstr1[]="GDS_TEX_%d";
 const char fstr2[]="GDS_TEX_%d_%d";
 char *retstr;
 if (psStyles[i].dataspec) {
  retstr=(char *)MALLOC(sizeof(char)*(strlen(fstr2)+30));
  sprintf(retstr,fstr2,psStyles[i].gdsno,psStyles[i].datatype);

 } else {
  retstr=(char *)MALLOC(sizeof(char)*(strlen(fstr2)+15));
  sprintf(retstr,fstr1,psStyles[i].gdsno);
 }
 return(retstr);
}



char *GDSLayerToTEXName(layer *layerptr)
{
 int i;
 i=layer_to_style_no(layerptr);
 return(GDSpsStyleNoToTEXName(i));
}














