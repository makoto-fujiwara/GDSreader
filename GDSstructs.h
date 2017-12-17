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
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * GDSstructs.c contains the definition of all data types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef int coord;

typedef struct point
{
  coord x;
  coord y;
} point;

typedef struct bbox
{
  point ll;
  point ur;
} bbox;

typedef struct layer
{
  int layerno;
  int datatype;
  char *name;
  struct GDScell *cells;
  struct layer *next;
} layer;

typedef struct transform
{
  double a, b, c;
  double d, e, f;
} transform;


typedef struct GDSstruct
{
  layer *layers; /* this only holds layers for non-sref and non-arefs */
  char *name;
  struct GDScell *cells;
  struct GDSstruct *next;
} GDSstruct;

typedef struct GDSlibrary
{
  char *name;
  char *lastmod;
  char *lastacc;
  double userunit;
  double meterunit;
  GDSstruct *structs;
} GDSlibrary;

typedef struct GDScell
{
  int type;
  union
  {
     struct boundaryEl *boundary;
     struct pathEl *path;
     struct srefEl *sref;
     struct arefEl *aref;
     struct textEl *text;
     struct nodeEl *node;
     struct boxEl *box;
   } detail;

  struct GDScell *next;
} GDScell;

typedef struct boundaryEl
{
  layer *layerptr;
  int numpoints;
  point *points;
} boundaryEl;

typedef struct pathEl
{
  layer *layerptr;
  int pathtype;
  coord width;
  int numpoints;
  point *points;
} pathEl;

typedef struct srefEl
{
  GDSstruct *strptr;
  char *refname;
  transform *transfptr;
} srefEl;

typedef struct arefEl
{
  GDSstruct *strptr;
  int cols;
  int rows;
  coord colspacing;
  coord rowspacing;
  char *refname;
  transform *transfptr;
} arefEl;

typedef struct textEl
{
  layer *layerptr;
  int hjustif;
  int vjustif;
  int font;
  int pathtype;
  coord width;
  char *string;
  transform *transfptr;
/* dirty hack. I have to make GDSgetTransform to understand mag */
  double mag;
} textEl;

typedef struct nodeEl
{
  layer *layerptr;
  point ref;
} nodeEl;

typedef struct boxEl
{
  layer *layerptr;
  int numpoints;
  point *points;
} boxEl;

/*-------------------------------------*/
/* Other structures not related to GDS */
/*-------------------------------------*/

typedef struct color
{
  double r;
  double b;
  double g;
} color;

typedef struct PSStyle
{
  char *layername;
  int gdsno;
  int datatype;
  int dataspec; /* If 0, then no datatype supplied. */
  int hidden;
  color edgecolor;
  int fill;
  int hatch;
  double angle;
  double step;
  double thickness;
  double depth;
} PSStyle;
