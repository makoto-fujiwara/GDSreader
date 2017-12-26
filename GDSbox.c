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
#include <strings.h>

#include <GDSstructs.h>
#include <GDSconsts.h>
#include <GDSaux.h>


void
BoxToHPGL(FILE *hpglfile, boxEl *box, PSStyle psStyle)
{
  return;
}

void
BoxToPS(FILE *psfile, boxEl *box, PSStyle psStyle)
{
  return;
}

void
BoxToPOV(FILE *povfile, boxEl *box, PSStyle psStyle)
{
 fprintf(povfile,"// box pov-printing not yet implemented.\n");
 return;
}

void
GDSfreeBox(GDScell *cell)
{
  fprintf(stderr, "NOT YET IMPLEMENTED\n");
}

GDScell *
GDSdupBox(boxEl *box)
{
  fprintf(stderr, "NOT YET IMPLEMENTED\n");
  return NULL;
}
/* Boundary Layer DataType XY */

GDScell *
GDSreadBoxBoundary(int gdsfildes, GDSstruct *structptr)
{
  unsigned char *record;
  int i, nbytes, layerno,datatype;
  layer *layerptr;
  GDScell *newcell;
  boxEl *boxptr;
  int tmp, x, y;

  printf(" %04d  ****  %s %s\n", __LINE__, __func__, __FILE__);
  if(GDSreadRecord(gdsfildes, &record, &nbytes) != BOUNDARY)  {
    fprintf(stderr, "Missing  ENDEL field in BOX element. Abort!\n");    exit(1);  }
  FREE(record);

    if(GDSreadRecord(gdsfildes, &record, &nbytes) != LAYER) {
    fprintf(stderr, "%04d Missing LAYER field in BOX element. Abort!\n", __LINE__ );  exit(1);  }
  layerno = GDSreadInt2(record + 2);
  //  printf (" %04d layerno %02d\n", __LINE__, layerno);
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != DATATYPE) {
    fprintf(stderr, "%04d Missing LAYER field in BOX element. Abort!\n", __LINE__ );  exit(1);  }
  datatype = GDSreadInt2(record + 2);
  //  printf (" %04d datatype %02d\n", __LINE__, datatype);
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)  {
    fprintf(stderr, "Missing XY field in BOX element. Abort!\n");   exit(1);  }

  x = GDSreadInt4(record + 2);
  y = GDSreadInt4(record + 6);
  //  fprintf(stderr, " %04d x, y: %08d, %08d\n",__LINE__, x, y);
  FREE(record);
 fprintf(stderr, " %04d layno: %02d, datatype: %02d x, y: %08d, %08d\n",
	 layerno, datatype, x, y);

if(GDSreadRecord(gdsfildes, &record, &nbytes) != ENDEL)  {
  fprintf(stderr, " %04d Missing  ENDEL field in BOX element. Abort!\n", __LINE__);    exit(1);  }
  FREE(record);
  return NULL;
}

/* Box Layer BoxType XY */

GDScell *
GDSreadBoxBox(int gdsfildes, GDSstruct *structptr)
{
  unsigned char *record;
  int i, nbytes, layerno,datatype;
  layer *layerptr;
  GDScell *newcell;
  boxEl *boxptr;
  int tmp;
  int x,y;

  printf(" %04d  ****  %s %s\n", __LINE__, __func__, __FILE__);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != BOX)  {
    fprintf(stderr, " %04d Missing  ENDEL field in BOX element. Abort!\n", __LINE__ );    exit(1);  }
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != LAYER) {
    fprintf(stderr, "%04d Missing LAYER field in BOX element. Abort!\n", __LINE__ );  exit(1);  }
  layerno = GDSreadInt2(record + 2);
  printf (" %04d layerno %02d\n", __LINE__, layerno);
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != BOXTYPE)  {
    fprintf(stderr, "Missing BOXTYPE field in BOX element. Abort!\n");  exit(1);  }

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)  {
    fprintf(stderr, "Missing XY field in BOX element. Abort!\n");   exit(1);  }
  //  signed int x, y;

  x = GDSreadInt4(record + 2);
  y = GDSreadInt4(record + 6);
  fprintf(stderr, " %04d x, y: %08d, %08d\n",__LINE__, x, y);
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != ENDEL)  {
    fprintf(stderr, " %04d Missing  ENDEL field in BOX element. Abort!\n", __LINE__);    exit(1);  }
  FREE(record);
  return NULL;
}
/*
http://www.iue.tuwien.ac.at/phd/minixhofer/node52.html

The last element of a GDS II file is the box. Following the BOX record
are the optional ELFLAGS and PLEX records, a mandatory LAYER record, a
BOXTYPE record with a zero argument, and an XY record. The XY must
contain five points that describe a closed, four-sided box. Unlike the
boundary, this is not a filled figure. Therefore it cannot be used for
IC geometry.
*/

/*
  BOX
  ELFLAGS
  PLEX
  LAYER
  XY
*/
GDScell *
GDSreadBox(int gdsfildes, GDSstruct *structptr)
{

  unsigned char *record;
  int i, nbytes, layerno,datatype;
  layer *layerptr;
  GDScell *newcell;
  boxEl *boxptr;
  int tmp;

  boxptr = (boxEl *)MALLOC(sizeof(pathEl));
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = BOX;
  newcell->detail.box = boxptr;

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != ELFLAGS)  {
    fprintf(stderr, " %04d %s ** ELFAGS field in BOX element not found, OK.\n", __LINE__, __func__);  }
    FREE(record);
  if(GDSreadRecord(gdsfildes, &record, &nbytes) != PLEX)  {
    fprintf(stderr, " %04d %s ** PLEX   field in BOX element not found, OK.\n", __LINE__, __func__);  }
    FREE(record);


  if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)  {
    fprintf(stderr, "Missing XY field in BOX element. Abort!\n");   exit(1);  }
  signed int x, y;

  x = GDSreadInt4(record + 2);
  y = GDSreadInt4(record + 6);
  fprintf(stderr, " %04d x, y: %08d, %08d\n",__LINE__, x, y);
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != ENDEL)  {
    fprintf(stderr, " %04d Missing  ENDEL field in BOX element. Abort!\n", __LINE__);    exit(1);  }
  FREE(record);

  /* 1 */   GDSreadBoxBox     (gdsfildes, structptr);
  /* 2 */   GDSreadBoxBox     (gdsfildes, structptr);
  /* 3 */   GDSreadBoxBoundary(gdsfildes, structptr);
  /* 4 */   GDSreadBoxBoundary(gdsfildes, structptr);
  /* 5 */   GDSreadBoxBox     (gdsfildes, structptr);
  /* 6 */   GDSreadBoxBoundary(gdsfildes, structptr);  
  /* 7 */   GDSreadBoxBox     (gdsfildes, structptr);
  /* 8 */   GDSreadBoxBox     (gdsfildes, structptr);
  /* 9 */   GDSreadBoxBox     (gdsfildes, structptr);
  /*10 */   GDSreadBoxBox     (gdsfildes, structptr);
  /*11 */   GDSreadBoxBox     (gdsfildes, structptr);
  /*12 */   GDSreadBoxBox     (gdsfildes, structptr);      
  /*
 0214 002d 4 -> BOX 
 0217 000d 6 -> LAYER
   */
  int n;
  for (n = 0; n<10;n++) {
     GDSreadBoxBox     (gdsfildes, structptr);
  }

  tmp = GDSreadRecord(gdsfildes, &record, &nbytes);
  printf (" %04d %04x %0d\n", __LINE__, tmp, nbytes);

  tmp = GDSreadRecord(gdsfildes, &record, &nbytes);
  printf (" %04d %04x %0d\n", __LINE__, tmp, nbytes);

  return NULL;
}
