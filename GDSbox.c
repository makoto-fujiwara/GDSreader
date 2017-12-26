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
  int i, nbytes, layerno, datatype;
  layer *layerptr;
  GDScell *newcell;
  boxEl *boxptr;
  int tmp;

  boxptr = (boxEl *)MALLOC(sizeof(pathEl));
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = BOX;
  newcell->detail.box = boxptr;

  if(tmp = GDSreadRecord(gdsfildes, &record, &nbytes) != LAYER) {
    fprintf(stderr, "%04d Missing LAYER field in BOX element %02x. Abort!\n", __LINE__, tmp );  exit(1);  }
  layerno = GDSreadInt2(record + 2);

  FREE(record);

    if(tmp = GDSreadRecord(gdsfildes, &record, &nbytes) != BOXTYPE) {
    fprintf(stderr, "%04d Missing LAYER field in BOX element %02x. Abort!\n", __LINE__, tmp );  exit(1);  }
  layerno = GDSreadInt2(record + 2);

  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)  {
    fprintf(stderr, "Missing XY field in BOX element. Abort!\n");   exit(1);  }
  signed int x, y;
  x = GDSreadInt4(record + 2);
  y = GDSreadInt4(record + 6);
  FREE(record);

  //  fprintf(stderr, " %04d layerno: %02d x, y: %08d, %08d\n",__LINE__, layerno, x, y);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != ENDEL)  {
    fprintf(stderr, " %04d Missing  ENDEL field in BOX element. Abort!\n", __LINE__);    exit(1);  }
  FREE(record);
  return newcell;
}
