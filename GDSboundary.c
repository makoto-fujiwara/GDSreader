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
#include <math.h>

#include <GDSstructs.h>
#include <GDSconsts.h>
#include <GDSglobals.h>
#include <GDSaux.h>
#include <GDStransf.h>

/*
 * Both PostScript and HPGL interpreters are faster when dealing with rectangles
 */
static int
BoundaryIsRectangle(boundaryEl *boundary)
{
  if(boundary->numpoints != 5)
    return 0;

  if(boundary->points[0].x != boundary->points[1].x)
  {
    if(boundary->points[0].y != boundary->points[1].y)
      return 0;
    if(boundary->points[0].x != boundary->points[3].x)
      return 0;
    if(boundary->points[2].x != boundary->points[1].x)
      return 0;
    if(boundary->points[2].y != boundary->points[3].y)
      return 0;
  }
  else
  {
    if(boundary->points[0].y != boundary->points[3].y)
      return 0;
    if(boundary->points[2].y != boundary->points[1].y)
      return 0;
    if(boundary->points[2].x != boundary->points[3].x)
      return 0;
  }
  return 1;
}

bbox
GDSgetBoundaryBBox(boundaryEl *boundary)
{
  int i;
  bbox bbx;

  bbx.ll.x = BIGVAL;
  bbx.ll.y = BIGVAL;
  bbx.ur.x = -BIGVAL;
  bbx.ur.y = -BIGVAL;

  for(i = 0; i < boundary->numpoints; i++)
  {
    if(boundary->points[i].x < bbx.ll.x)
      bbx.ll.x = boundary->points[i].x;
    if(boundary->points[i].x > bbx.ur.x)
      bbx.ur.x = boundary->points[i].x;

    if(boundary->points[i].y < bbx.ll.y)
      bbx.ll.y = boundary->points[i].y;
    if(boundary->points[i].y > bbx.ur.y)
      bbx.ur.y = boundary->points[i].y;
  }

  return bbx;
}

void
BoundaryToHPGL(FILE *hpglfile, boundaryEl *boundary, PSStyle psStyle)
{
  int i;
  
  if(boundary->layerptr->layerno != psStyle.gdsno)
    return;

  if(BoundaryIsRectangle(boundary))
  {
    fprintf(hpglfile, "PU%d,%dER%d,%d",
            boundary->points[0].x / InternalScaleFactor,
            boundary->points[0].y / InternalScaleFactor,
            (boundary->points[2].x - boundary->points[0].x) /
             InternalScaleFactor,
            (boundary->points[2].y - boundary->points[0].y) /
             InternalScaleFactor);
    if(psStyle.fill || psStyle.hatch)
      fprintf(hpglfile, "PU%d,%dRR%d,%d",
              boundary->points[0].x / InternalScaleFactor,
              boundary->points[0].y / InternalScaleFactor,
              (boundary->points[2].x - boundary->points[0].x) /
               InternalScaleFactor,
              (boundary->points[2].y - boundary->points[0].y) /
               InternalScaleFactor);
  }
  else
  {
    fprintf(hpglfile, "PU%d,%dPM0",
            boundary->points[0].x / InternalScaleFactor,
            boundary->points[0].y / InternalScaleFactor);
    for(i = 1; i < boundary->numpoints; i++)
      fprintf(hpglfile, "PD%d,%d",
              boundary->points[i].x / InternalScaleFactor,
              boundary->points[i].y / InternalScaleFactor);
    fprintf(hpglfile, "PM2EP");
    if(psStyle.fill || psStyle.hatch)
      fprintf(hpglfile, "FP");
  }
  fprintf(hpglfile, "\n");
}

int
GDSgetBoundaryWinding(boundaryEl *boundary)
{
  bbox bbx;
  coord xmin, ymin, ycut, x1, x2, y1, y2;
  int i, imin;

  ymin = BIGVAL;

  bbx = GDSgetBoundaryBBox(boundary);

  xmin = bbx.ll.x + (bbx.ur.x - bbx.ll.x) / 2;
  for(i = 0; i < boundary->numpoints - 1; i++)
  {
    x1 = boundary->points[i].x;
    y1 = boundary->points[i].y;
    x2 = boundary->points[i + 1].x;
    y2 = boundary->points[i + 1].y;

    if(x1 == x2)
      continue;
    else if(boundary->points[i].x > boundary->points[i + 1].x)
    {
      x1 = boundary->points[i + 1].x;
      y1 = boundary->points[i + 1].y;
      x2 = boundary->points[i].x;
      y2 = boundary->points[i].y;
    }
    else
    {
      x1 = boundary->points[i].x;
      y1 = boundary->points[i].y;
      x2 = boundary->points[i + 1].x;
      y2 = boundary->points[i + 1].y;
    }
    if(x1 <= xmin && x2 >= xmin)
      ycut = y1 + (coord)(((double)(y2 - y1)) * (xmin - x1) / (x2 - x1));
    else
      continue;
    if(ycut < ymin)
    {
      ymin = ycut;
      imin = i;
    }
  }
  if(boundary->points[imin].x > boundary->points[imin + 1].x)
    return 1;
  else
    return -1;
}


void
BoundaryToPS(FILE *psfile, boundaryEl *boundary, PSStyle psStyle)
{
  int i;

  if(boundary->layerptr->layerno != psStyle.gdsno)
    return;

  if(BoundaryIsRectangle(boundary))
  {
    fprintf(psfile, "%d %d %d %d rectstroke\n",
            boundary->points[0].x / InternalScaleFactor,
            boundary->points[0].y / InternalScaleFactor,
            (boundary->points[2].x - boundary->points[0].x) / InternalScaleFactor,
            (boundary->points[2].y - boundary->points[0].y) / InternalScaleFactor);
  }
  else
  {
    fprintf(psfile, "%d %d m\n",
            boundary->points[boundary->numpoints - 1].x / InternalScaleFactor,
            boundary->points[boundary->numpoints - 1].y / InternalScaleFactor);
    for(i = boundary->numpoints - 2; i >= 0; i--)
      fprintf(psfile, "%d %d l\n",
              boundary->points[i].x / InternalScaleFactor,
              boundary->points[i].y / InternalScaleFactor);
    fprintf(psfile, "closepath stroke\n");
  }
  fprintf(psfile, "%d %d m\n",
          boundary->points[boundary->numpoints - 1].x / InternalScaleFactor,
          boundary->points[boundary->numpoints - 1].y / InternalScaleFactor);
  for(i = boundary->numpoints - 2; i >= 0; i--)
    fprintf(psfile, "%d %d l\n",
            boundary->points[i].x / InternalScaleFactor,
            boundary->points[i].y / InternalScaleFactor);
  fprintf(psfile, "closepath\n");

  if(psStyle.fill)
    fprintf(psfile, "fill\n");
  if(psStyle.hatch)
    fprintf(psfile, "/FONT%s findfont patternfill\n", psStyle.layername);

  return;
}

void
BoundaryToPOV(FILE *povfile, boundaryEl *boundary, PSStyle psStyle)
{
  int i;
  char *ts;
  if(BoundaryIsRectangle(boundary)) /* Basically a box */
  {
    fprintf(povfile, " box { <%f,%f,%f> <%f,%f,%f>\n",
     (double)(boundary->points[2].x), 
     psStyle.depth, 
     (double)(boundary->points[2].y),
     (double)(boundary->points[0].x),
     psStyle.depth-psStyle.thickness,
     (double)(boundary->points[0].y));
     fprintf(povfile,"  texture { %s }\n",
           ts=GDSLayerToTEXName(boundary->layerptr));
     fprintf(povfile, " }\n");
     FREE(ts);
  }
  else
  {

     fprintf(povfile, " prism {\n  linear_spline %f,%f,%d\n",psStyle.depth,
                            psStyle.depth-psStyle.thickness,boundary->numpoints);
     for(i=0 ;i < (boundary->numpoints)-1 ; i++)
       fprintf(povfile, " <%f,%f>,\n",
              (double)(boundary->points[i].x),
              (double)(boundary->points[i].y));
     fprintf(povfile, " <%f,%f>\n",
              (double)(boundary->points[(boundary->numpoints)-1].x),
              (double)(boundary->points[(boundary->numpoints)-1].y));
     fprintf(povfile,"  texture { %s }\n",
                 ts=GDSLayerToTEXName(boundary->layerptr));
     fprintf(povfile,"}\n");
     FREE(ts);
  }
  return;
}


/*
 * Frees the cell contents and removes the layer list reference.
 * The structure list reference has to be removed at the upper level.
 */
void
GDSfreeBoundary(GDScell *cell)
{
  GDScell *cellptr;

  if(cell == NULL)
    return;
  if(cell->type != BOUNDARY)
    return;

  if(cell == cell->detail.boundary->layerptr->cells)
    cell->detail.boundary->layerptr->cells = cell->next;
  else
  {
    for(cellptr = cell->detail.boundary->layerptr->cells;
        cellptr->next != NULL; cellptr = cellptr->next)
    {
      if(cellptr->next == cell)
      {
        cellptr->next = cellptr->next->next;
        break;
      }
    }
    if(cellptr == NULL)
      fprintf(stderr, "Oops! GDSfreeBoundary(): Missing cell in layerptr\n");
  }
  FREE(cell->detail.boundary->points);
  FREE(cell->detail.boundary);
  return;
}

GDScell *
GDSdupBoundary(boundaryEl *boundary)
{
  int i;
  GDScell *newcell;
  boundaryEl *boundaryptr;
  
  if(boundary == NULL)
    return NULL;

  boundaryptr = (boundaryEl *)MALLOC(sizeof(boundaryEl));
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = BOUNDARY;
  newcell->detail.boundary = boundaryptr;

  newcell->next = boundary->layerptr->cells;
  boundary->layerptr->cells = newcell;
  
  boundaryptr->layerptr = boundary->layerptr;
  boundaryptr->numpoints = boundary->numpoints;
  boundaryptr->points = (point *)MALLOC(boundaryptr->numpoints * sizeof(point));
  for(i = 0; i < boundaryptr->numpoints; i ++)
  {
    (boundaryptr->points[i]).x = (boundary->points[i]).x;
    (boundaryptr->points[i]).y = (boundary->points[i]).y;
  }
  
  return newcell;
}

/*
 * Apply the transform transf to all points in boundary.
 */
void
GDStransfBoundary(boundaryEl *boundary, transform *transf)
{
  int i;
  
  for(i = 0; i < boundary->numpoints; i++)
    boundary->points[i] = GDStransfPoint(&(boundary->points[i]), transf);
  
  return;
}

GDScell *
GDSreadBoundary(int gdsfildes, GDSstruct *structptr)
{
  unsigned char *record;
  int i, nbytes, layerno, datatype;
  layer *layerptr;
  GDScell *newcell;
  boundaryEl *boundaryptr;

  boundaryptr = (boundaryEl *)MALLOC(sizeof(boundaryEl));
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = BOUNDARY;
  newcell->detail.boundary = boundaryptr;

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != LAYER)
  {
    fprintf(stderr, "Missing LAYER field in BOUNDARY element. Abort!\n");
    exit(1);
  }
  layerno = GDSreadInt2(record + 2);
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != DATATYPE)
  {
    fprintf(stderr, "Missing DATATYPE field in BOUNDARY element. Abort!\n");
    exit(1);
  }
  datatype = GDSreadInt2(record + 2);
  FREE(record);

  for(layerptr = structptr->layers;
      layerptr != NULL; layerptr = layerptr->next)
    if(layerptr->layerno == layerno && layerptr->datatype==datatype)
      break;
  if(layerptr == NULL)
  {
    layerptr = (layer *)MALLOC(sizeof(layer));
    layerptr->layerno = layerno;
    layerptr->datatype = datatype;
    layerptr->name = NULL; /* to be added later */
    layerptr->cells = newcell;
    layerptr->next = structptr->layers;
    structptr->layers = layerptr;
  }
  else
  {
    newcell->next = layerptr->cells;
    layerptr->cells = newcell;
  }
  boundaryptr->layerptr = layerptr;


  if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)
  {
    fprintf(stderr, "Missing XY field in BOUNDARY element. Abort!\n");
    exit(1);
  }
  boundaryptr->numpoints = (nbytes - 2) / 8;
  boundaryptr->points = (point *)MALLOC(boundaryptr->numpoints * sizeof(point));

  for(i = 0; i < boundaryptr->numpoints; i ++)
  {
    (boundaryptr->points[i]).x = GDSreadInt4(record + 8 * i + 2);
    (boundaryptr->points[i]).y = GDSreadInt4(record + 8 * i + 6);
  }
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != ENDEL)
  {
    fprintf(stderr, "Missing ENDEL field in BOUNDARY element. Abort!\n");
    exit(1);
  }
  FREE(record);

  fprintf(stdout, " %04d %s Boundary on layer %d, datatype %d:\n", __LINE__, __func__, layerno,
        layerptr->datatype);
  for(i = 0; i < boundaryptr->numpoints; i++)
    fprintf(stdout, " %04d %s point[%03d] = %9d %9d\n", __LINE__, __func__,
            i, (boundaryptr->points[i]).x, (boundaryptr->points[i]).y);
  return newcell;
}

