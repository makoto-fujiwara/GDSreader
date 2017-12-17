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
#include <string.h>
#include <math.h>

#include <GDSstructs.h>
#include <GDSconsts.h>
#include <GDSglobals.h>
#include <GDSaux.h>
#include <GDSreader.h>
#include <GDSboundary.h>
#include <GDSpath.h>
#include <GDSsref.h>
#include <GDSaref.h>
#include <GDStext.h>
#include <GDStext.h>
#include <GDSnode.h>
#include <GDSbox.h>
#include <GDSstruct.h>
#include <GDStransf.h>

static void
writeHPGLHeader(FILE *hpglfile)
{
  fprintf(hpglfile, "%%-1BBPINNP64MC1QL100WU0PW0.13\n");
}

static void
writeHPGLTail(FILE *hpglfile)
{
  fprintf(hpglfile, "PUSP0PG;\n");
}

static void
writeHPGLHatch(FILE *hpglfile, PSStyle psStyle, double scale)
{
  if(psStyle.fill)
  {
    fprintf(hpglfile, "FT1\n");
    return;
  }
  switch(psStyle.hatch)
  {
    case 1:
      fprintf(hpglfile, "FT3,%d,%.2f\n",
              (int)(psStyle.step * 40.0 / scale), psStyle.angle);
      return;
    case 2:
      fprintf(hpglfile, "FT4,%d,%.2f\n",
              (int)(psStyle.step * 40.0 / scale), psStyle.angle);
      return;
    case 0:
    default:
      return;
  }
}

static void
writeHPGLPen(FILE *hpglfile, PSStyle psStyle)
{
  if(psStyle.hidden)
    return;
  fprintf(hpglfile, "PC%d,%d,%d,%d", psStyle.gdsno,
          (int)(255.0 * psStyle.edgecolor.r + 0.5),
          (int)(255.0 * psStyle.edgecolor.g + 0.5),
          (int)(255.0 * psStyle.edgecolor.b + 0.5));
}

void
GDStoHPGL(GDSlibrary *libptr, char *hpglfile, char *topcellname)
{
  GDSstruct *structptr;
  GDScell *cellptr;
  FILE *hpglfileptr;
  bbox bbx;
  double scale, scalex, scaley;
  int i;
  
  if(topcellname == NULL)
    return;

  hpglfileptr = fopen(hpglfile, "w");
  if(hpglfileptr == NULL)
  {
    fprintf(stderr, "Couldn't open %s for writing. Aborting...\n", hpglfile);
    exit(1);
  }
  writeHPGLHeader(hpglfileptr);
  for(i = 0; i < howmanylayers; i++)
    writeHPGLPen(hpglfileptr, psStyles[i]);

  structptr = GDSgetStructByName(libptr, topcellname);
  if(structptr == NULL)
  {
    fprintf(stderr, "Couldn't find the structure named \"%s\"\n",
            topcellname);
    return;
  }
  bbx = GDSgetStructBBox(structptr);
  scalex = 33.6 * InternalScaleFactor * 1016.0 * 1.0 / (bbx.ur.x - bbx.ll.x);
  scaley = 42.6 * InternalScaleFactor * 1016.0 * 1.0 / (bbx.ur.y - bbx.ll.y);
  scale = scalex;
  fprintf(stderr, "scalex = %f, scaley = %f\n", scalex, scaley);
  fprintf(hpglfileptr, "\nPS%d,%d;\n", (int)(33.6 * 1016),
          (int)(33.6 * 1016 * (bbx.ur.y - bbx.ll.y) / (bbx.ur.x - bbx.ll.x)));

  fprintf(stderr, "scale = %f\n", scale);
  fprintf(hpglfileptr, "IP%d,%d,%d,%dSC%d,%d,%d,%d\n",
          0, 0, (int)(33.6 * 1016),
          (int)(33.6 * 1016 * (bbx.ur.y - bbx.ll.y) / (bbx.ur.x - bbx.ll.x)),
          (int)(bbx.ll.x / InternalScaleFactor - 33.6 * 1016.0 * 0.05 / scale),
          (int)(bbx.ur.x / InternalScaleFactor + 33.6 * 1016.0 * 0.05 / scale),
          (int)(bbx.ll.y / InternalScaleFactor - 33.6 * 1016.0 * 0.05 / scale),
          (int)(bbx.ur.y / InternalScaleFactor + 33.6 * 1016.0 * 0.05 / scale));
  for(i = 0; i < howmanylayers; i++)
  {
    if(psStyles[i].hidden)
      continue;

    fprintf(hpglfileptr, "SP%d\n", psStyles[i].gdsno);
    writeHPGLHatch(hpglfileptr, psStyles[i], scale);

    for(cellptr = structptr->cells; cellptr != NULL;
        cellptr = cellptr->next)
    {
      switch(cellptr->type)
      {
        case BOUNDARY:
          BoundaryToHPGL(hpglfileptr, cellptr->detail.boundary, psStyles[i]);
          break;
        case PATH:
          PathToHPGL(hpglfileptr, cellptr->detail.path, psStyles[i]);
          break;
        case SREF:
          SrefToHPGL(hpglfileptr, cellptr->detail.sref,
                       cellptr->detail.sref->transfptr, psStyles[i]);
          break;
        case AREF:
          ArefToHPGL(hpglfileptr, cellptr->detail.aref, &Ident, psStyles[i]);
          break;
        case TEXT:
          TextToHPGL(hpglfileptr, cellptr->detail.text, &Ident, psStyles[i]);
          break;
        case NODE:
          NodeToHPGL(hpglfileptr, cellptr->detail.node, psStyles[i]);
          break;
        case BOX:
          BoxToHPGL(hpglfileptr, cellptr->detail.box, psStyles[i]);
          break;
        default:
          break;
      } /* switch */
    } /* cell loop */
  }
  writeHPGLTail(hpglfileptr);
  fclose(hpglfileptr);
}
