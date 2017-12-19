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

/*
http://www.iue.tuwien.ac.at/phd/minixhofer/node52.html

The last element of a GDS II file is the box. Following the BOX record
are the optional ELFLAGS and PLEX records, a mandatory LAYER record, a
BOXTYPE record with a zero argument, and an XY record. The XY must
contain five points that describe a closed, four-sided box. Unlike the
boundary, this is not a filled figure. Therefore it cannot be used for
IC geometry.
*/

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

GDScell *
GDSreadBox(int gdsfildes, GDSstruct *structptr)
{
  fprintf(stderr, "READING BOX NOT YET IMPLEMENTED\n");
  return NULL;
}
