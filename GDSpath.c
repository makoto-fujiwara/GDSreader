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
#include <GDSaux.h>
#include <GDStransf.h>
#include <GDSboundary.h>

static double
getAngle(coord x1, coord y1, coord x2, coord y2)
{
  double angle;

  if(x1 == x2)
    angle =  M_PI_2 * ((y2 > y1) ? 1 : -1);
  else
  {
    angle = atan(fabs(y2 - y1)/fabs(x2 - x1));
    if(y2 >= y1)
    {
      if(x2 >= x1)
        angle += 0;
      else
        angle = M_PI - angle;
    }
    else
    {
      if(x2 >= x1)
        angle = 2 * M_PI - angle;
      else
        angle += M_PI;
    }
  }

  return angle;
}

#define EPS 1e-8

static point
getDeltaXY(coord hw, point p1, point p2, point p3)
{
  double alpha, beta, theta, r;
  point pnt;

  alpha = getAngle(p1.x, p1.y, p2.x, p2.y);
  beta = getAngle(p2.x, p2.y, p3.x, p3.y);

  theta = (alpha + beta + M_PI)/2.0;
  
  if(fabs(cos((alpha - beta)/2.0)) < EPS)
  {
    fprintf(stderr, "Internal algorithm error: cos((alpha - beta)/2) = 0\n");
    exit(1);
  }
  r = ((double) hw) / cos((alpha - beta)/2.0);

  pnt.x = (coord) (r * cos(theta));
  pnt.y = (coord) (r * sin(theta));

  return pnt;
}

static point
getEndDeltaXY(coord hw, point p1, point p2)
{
  double alpha, theta, r;
  point pnt;

  alpha = getAngle(p1.x, p1.y, p2.x, p2.y);

  theta = alpha;
  r= hw;
  pnt.x = (coord)(-r * sin(theta));
  pnt.y = (coord)(r * cos(theta));
  
  return pnt;
}


boundaryEl *
PathToBoundary(pathEl *path)
{
  boundaryEl *boundary;
  coord hw = path->width / 2;
  int i, numpoints = path->numpoints;
  point *points, deltaxy;

  if (numpoints < 2)
  {
    fprintf(stderr,
            "PathToBoundary(): don't know to handle wires < 2 pts yet\n");
    return NULL;
  }

  points = (point *)MALLOC(sizeof(point) * (2 * numpoints + 1));

  deltaxy = getEndDeltaXY(hw, path->points[0], path->points[1]);

  if(path->pathtype == BUTT_END)
  {
    points[0].x = path->points[0].x + deltaxy.x;
    points[0].y = path->points[0].y + deltaxy.y;
    points[2 * numpoints].x = path->points[0].x + deltaxy.x;
    points[2 * numpoints].y = path->points[0].y + deltaxy.y;
    points[2 * numpoints - 1].x = path->points[0].x - deltaxy.x;
    points[2 * numpoints - 1].y = path->points[0].y - deltaxy.y;
  }
  else /* Extended end */
  {
    points[0].x = path->points[0].x + deltaxy.x - deltaxy.y;
    points[0].y = path->points[0].y + deltaxy.y - deltaxy.x;
    points[2 * numpoints].x = path->points[0].x + deltaxy.x - deltaxy.y;
    points[2 * numpoints].y = path->points[0].y + deltaxy.y - deltaxy.x;
    points[2 * numpoints - 1].x = path->points[0].x - deltaxy.x - deltaxy.y;
    points[2 * numpoints - 1].y = path->points[0].y - deltaxy.y - deltaxy.x;
  }


  for(i = 1; i < numpoints - 1; i++)
  {
    deltaxy = getDeltaXY(hw, path->points[i - 1],
                         path->points[i], path->points[i + 1]);
    points[i].x = path->points[i].x + deltaxy.x;
    points[i].y = path->points[i].y + deltaxy.y;
    points[2 * numpoints - i - 1].x = path->points[i].x - deltaxy.x;
    points[2 * numpoints - i - 1].y = path->points[i].y - deltaxy.y;
  }

  deltaxy = getEndDeltaXY(hw, path->points[numpoints - 2],
                          path->points[numpoints - 1]);
  if(path->pathtype == BUTT_END)
  {
    points[numpoints - 1].x = path->points[numpoints - 1].x + deltaxy.x;
    points[numpoints - 1].y = path->points[numpoints - 1].y + deltaxy.y;
    points[numpoints].x = path->points[numpoints - 1].x - deltaxy.x;
    points[numpoints].y = path->points[numpoints - 1].y - deltaxy.y;
  }
  else /* Extended end */
  {
    points[numpoints - 1].x = path->points[numpoints - 1].x + deltaxy.x + deltaxy.y;
    points[numpoints - 1].y = path->points[numpoints - 1].y + deltaxy.y + deltaxy.x;
    points[numpoints].x = path->points[numpoints - 1].x - deltaxy.x + deltaxy.y;
    points[numpoints].y = path->points[numpoints - 1].y - deltaxy.y + deltaxy.x;
  }

  boundary = (boundaryEl *)MALLOC(sizeof(boundaryEl));
  boundary->layerptr = path->layerptr;
  boundary->numpoints = numpoints * 2 + 1;
  boundary->points = points;

  return boundary;
}

bbox
GDSgetPathBBox(pathEl *path)
{
  bbox bbx;
  boundaryEl *boundary;

  boundary = PathToBoundary(path);
  if(boundary == NULL)
  {
    bbx.ll.x = BIGVAL;
    bbx.ll.y = BIGVAL;
    bbx.ur.x = -BIGVAL;
    bbx.ur.y = -BIGVAL;
    return bbx;
  }
  bbx = GDSgetBoundaryBBox(boundary);
  FREE(boundary->points);
  FREE(boundary);

  return bbx;
}

void
PathToPS(FILE *psfile, pathEl *path, PSStyle psStyle)
{
  boundaryEl *boundary;

  if(path->layerptr->layerno != psStyle.gdsno)
    return;
  boundary = PathToBoundary(path);
  if(boundary == NULL)
    return;
  BoundaryToPS(psfile, boundary, psStyle);
  FREE(boundary->points);
  FREE(boundary);
}

void
PathToPOV(FILE *povfile, pathEl *path, PSStyle psStyle)
{
  boundaryEl *boundary;
  boundary = PathToBoundary(path);
  if(boundary == NULL)
    return;
  BoundaryToPOV(povfile, boundary, psStyle);
  FREE(boundary->points);
  FREE(boundary);
}

void
PathToHPGL(FILE *hpglfile, pathEl *path, PSStyle psStyle)
{
  boundaryEl *boundary;

  if(path->layerptr->layerno != psStyle.gdsno)
    return;
  boundary = PathToBoundary(path);
  if(boundary == NULL)
    return;
  BoundaryToHPGL(hpglfile, boundary, psStyle);
  FREE(boundary->points);
  FREE(boundary);
}

/*
 * Frees the cell contents and removes the layer list reference.
 * The structure list reference has to be removed at the upper level.
 */
void
GDSfreePath(GDScell *cell)
{
  GDScell *cellptr;

  if(cell == NULL)
    return;
  if(cell->type != PATH)
    return;

  if(cell == cell->detail.path->layerptr->cells)
    cell->detail.path->layerptr->cells = cell->next;
  else
  {
    for(cellptr = cell->detail.path->layerptr->cells;
        cellptr->next != NULL; cellptr = cellptr->next)
    {
      if(cellptr->next == cell)
      {
        cellptr->next = cellptr->next->next;
        break;
      }
    }
    if(cellptr == NULL)
      fprintf(stderr, "Oops! GDSfreePath(): Missing cell in layerptr\n");
  }
  FREE(cell->detail.path->points);
  FREE(cell->detail.path);
  return;
}

GDScell *
GDSdupPath(pathEl *path)
{
  int i;
  GDScell *newcell;
  pathEl *pathptr;
  
  if(path == NULL)
    return NULL;

  pathptr = (pathEl *)MALLOC(sizeof(pathEl));
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = PATH;
  newcell->detail.path = pathptr;

  newcell->next = path->layerptr->cells;
  path->layerptr->cells = newcell;
  
  pathptr->layerptr = path->layerptr;
  pathptr->pathtype = path->pathtype;
  pathptr->width= path->width;
  pathptr->numpoints = path->numpoints;
  pathptr->points = (point *)MALLOC(pathptr->numpoints * sizeof(point));
  for(i = 0; i < pathptr->numpoints; i ++)
  {
    (pathptr->points[i]).x = (path->points[i]).x;
    (pathptr->points[i]).y = (path->points[i]).y;
  }
  
  return newcell;
}

/*
 * Apply the transform transf to all points in path.
 */
void
GDStransfPath(pathEl *path, transform *transf)
{
  int i;
  double mag;
  for(i = 0; i < path->numpoints; i++)
    path->points[i] = GDStransfPoint(&(path->points[i]), transf);
  /* Check to see if width needs to be changed. */
  GDSGetTransfValues(transf,&i,&mag);
  if (mag!=1.0) { path->width= (coord)((double)path->width*mag); }
  return;
}

GDScell *
GDSreadPath(int gdsfildes, GDSstruct *structptr)
{
  unsigned char *record;
  int i, nbytes, layerno,datatype;
  layer *layerptr;
  GDScell *newcell;
  pathEl *pathptr;

  pathptr = (pathEl *)MALLOC(sizeof(pathEl));
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = PATH;
  newcell->detail.path = pathptr;

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != LAYER)
  {
    fprintf(stderr, "Missing LAYER field in PATH element. Abort!\n");
    exit(1);
  }
  layerno = GDSreadInt2(record + 2);
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != DATATYPE)
  {
    fprintf(stderr, "Missing DATATYPE field in PATH element. Abort!\n");
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
  pathptr->layerptr = layerptr;

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != PATHTYPE)
  {
    fprintf(stderr, "Missing PATHTYPE field in PATH element. Abort!\n");
    exit(1);
  }
  pathptr->pathtype = GDSreadInt2(record + 2);
  FREE(record);

  switch(pathptr->pathtype)
  {
    case ROUND_END:
      fprintf(stderr,
              "Round ended path not supported. Used extended instead\n");
    case CUSTOMPLUS_END:
      fprintf(stderr,
              "Custom ended path not supported. Used extended instead\n");
      pathptr->pathtype = EXTENDED_END;
    case EXTENDED_END:
    case BUTT_END:
      break;
    default:
      fprintf(stderr, "Unknown pathtype %d. Used butt end instead\n",
              pathptr->pathtype);
      pathptr->pathtype = BUTT_END;
      break;
  }
  if(GDSreadRecord(gdsfildes, &record, &nbytes) != WIDTH)
  {
    fprintf(stderr, "Missing WIDTH field in PATH element. Abort!\n");
    exit(1);
  }
  pathptr->width = GDSreadInt4(record + 2);
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)
  {
    fprintf(stderr, "Missing XY field in PATH element. Abort!\n");
    exit(1);
  }
  pathptr->numpoints = (nbytes - 2) / 8;
  pathptr->points = (point *)MALLOC(pathptr->numpoints * sizeof(point));

  for(i = 0; i < pathptr->numpoints; i ++)
  {
    (pathptr->points[i]).x = GDSreadInt4(record + 8 * i + 2);
    (pathptr->points[i]).y = GDSreadInt4(record + 8 * i + 6);
  }
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != ENDEL)
  {
    fprintf(stderr, "Missing ENDEL field in PATH element. Abort!\n");
    exit(1);
  }
  FREE(record);

  fprintf(stdout, "Path on layer %d, datatype %d, of type %d with width = %d:\n",
          layerno, layerptr->datatype,pathptr->pathtype, pathptr->width);
  for(i = 0; i < pathptr->numpoints; i++)
    fprintf(stdout, "point[%d] = %9d %9d\n",
            i, (pathptr->points[i]).x, (pathptr->points[i]).y);
  return newcell;
}

