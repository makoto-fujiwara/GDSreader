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
NodeToHPGL(FILE *hpglfile, nodeEl *node, PSStyle psStyle)
{
  return;
}

void
NodeToPS(FILE *psfile, nodeEl *node, PSStyle psStyle)
{
  return;
}

void
NodeToPOV(FILE *povfile, nodeEl *node, PSStyle psStyle)
{
  fprintf(povfile,"// Node pov-printing not yet implemented.\n");
  return;
}

void
GDSfreeNode(GDScell *cell)
{
  fprintf(stderr, "NOT YET IMPLEMENTED\n");
}

GDScell *
GDSdupNode(nodeEl *node)
{
  fprintf(stderr, "NOT YET IMPLEMENTED\n");
  return NULL;
}

GDScell *
GDSreadNode(int gdsfildes, GDSstruct *structptr)
{
  fprintf(stderr, "READING NODE NOT YET IMPLEMENTED\n");
  return NULL;
}
