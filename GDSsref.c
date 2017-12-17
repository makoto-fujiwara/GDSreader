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
#include <GDSboundary.h>
#include <GDSpath.h>
#include <GDSaref.h>
#include <GDStext.h>
#include <GDSnode.h>
#include <GDSbox.h>
#include <GDStransf.h>

bbox
GDSgetSrefBBox(srefEl *sref, transform *transf)
{
  bbox bbx, bbx1;
  GDScell *cellptr, *newcell;
  transform *newtransf;

  bbx.ll.x = BIGVAL;
  bbx.ll.y = BIGVAL;
  bbx.ur.x = -BIGVAL;
  bbx.ur.y = -BIGVAL;

  newtransf = GDStransfTransf(sref->transfptr, transf);
  for(cellptr = sref->strptr->cells; cellptr != NULL; cellptr = cellptr->next)
  {
    switch(cellptr->type)
    {
      case BOUNDARY:
        newcell = GDSdupBoundary(cellptr->detail.boundary);
        GDStransfBoundary(newcell->detail.boundary, newtransf);
        bbx1 = GDSgetBoundaryBBox(newcell->detail.boundary);
        GDSfreeBoundary(newcell);
        FREE(newcell);
        break;
      case PATH:
        newcell = GDSdupPath(cellptr->detail.path);
        GDStransfPath(newcell->detail.path, newtransf);
        bbx1 = GDSgetPathBBox(newcell->detail.path);
        GDSfreePath(newcell);
        FREE(newcell);
        break;
      case SREF:
        bbx1 = GDSgetSrefBBox(cellptr->detail.sref, newtransf);
        break;
      case AREF:
        bbx1 = GDSgetArefBBox(cellptr->detail.aref, newtransf);
        break;
      case TEXT:
        bbx1 = GDSgetTextBBox(cellptr->detail.text);
        break;
      case NODE:
      case BOX:
      default:
        bbx1.ll.x = BIGVAL;
        bbx1.ll.y = BIGVAL;
        bbx1.ur.x = -BIGVAL;
        bbx1.ur.y = -BIGVAL;
        break;
    }
    if(bbx.ll.x > bbx1.ll.x)
      bbx.ll.x = bbx1.ll.x;
    if(bbx.ll.y > bbx1.ll.y)
      bbx.ll.y = bbx1.ll.y;
    if(bbx.ur.x < bbx1.ur.x)
      bbx.ur.x = bbx1.ur.x;
    if(bbx.ur.y < bbx1.ur.y)
      bbx.ur.y = bbx1.ur.y;
  }
  GDSfreeTransf(newtransf);
  return bbx;
}


void
SrefToPS(FILE *psfile, srefEl *sref, transform *transf, PSStyle psStyle)
{
  GDScell *cellptr, *newcell;
  transform *newtransf;

  newtransf = GDStransfTransf(sref->transfptr, transf); 
  for(cellptr = sref->strptr->cells; cellptr != NULL; cellptr = cellptr->next)
    switch(cellptr->type)
    {
      case BOUNDARY:  
        newcell = GDSdupBoundary(cellptr->detail.boundary);
        GDStransfBoundary(newcell->detail.boundary, newtransf);
        BoundaryToPS(psfile, newcell->detail.boundary, psStyle);
        GDSfreeBoundary(newcell);
        FREE(newcell);
        break;
      case PATH:
        newcell = GDSdupPath(cellptr->detail.path);
        GDStransfPath(newcell->detail.path, newtransf);
        PathToPS(psfile, newcell->detail.path, psStyle);
        GDSfreePath(newcell);
        FREE(newcell);
        break;
      case SREF:
        SrefToPS(psfile, cellptr->detail.sref, newtransf, psStyle);
        break;
      case AREF:
        ArefToPS(psfile, cellptr->detail.aref, newtransf, psStyle);
        break;
      case TEXT:
        TextToPS(psfile, cellptr->detail.text, newtransf, psStyle);
        break;
      case NODE:
        NodeToPS(psfile, cellptr->detail.node, psStyle);
        break;
      case BOX:
        BoxToPS(psfile, cellptr->detail.box, psStyle);
        break;
      default:
        break;
    }
  GDSfreeTransf(newtransf);
}


void
SrefToPOV(FILE *povfileptr, srefEl *sref, transform *transf, PSStyle psStyle)
{
 char *escsrefcell=NULL;
 transform *newtransf;  
 int mirror;
 double mag,angle;
  
 escsrefcell=GDSEscPovString(sref->refname);
 newtransf = GDStransfTransf(sref->transfptr, transf);
 fprintf(povfileptr,"//Sref\n");
 angle=-1.0*GDSGetTransfValues(newtransf,&mirror,&mag);
 /* left hand rule for POV Y axis */
 fprintf(povfileptr," object { %s \n",escsrefcell);
 fprintf(povfileptr,"  scale < %f,1.0,%f>\n",mag,(mirror?(-1.0*mag):mag)); /* Here Z axis is Y */
 fprintf(povfileptr,"  rotate < 0.0,%f,0.0 >\n"
          "  translate <%f,0.0,%f >\n"
          " }\n",angle,newtransf->c,newtransf->f
       );
 FREE(newtransf);
 FREE(escsrefcell);
}


void
SrefToHPGL(FILE *hpglfile, srefEl *sref, transform *transf, PSStyle psStyle)
{
  GDScell *cellptr, *newcell;
  transform *newtransf;
  
  for(cellptr = sref->strptr->cells; cellptr != NULL; cellptr = cellptr->next)
    switch(cellptr->type)
    {
      case BOUNDARY:
        newcell = GDSdupBoundary(cellptr->detail.boundary);
#ifdef _NOTDEF_
        if(cellptr->detail.boundary->numpoints == 5)
          fprintf(stdout, "before: (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n",
                  cellptr->detail.boundary->points[0].x,
                  cellptr->detail.boundary->points[0].y,
                  cellptr->detail.boundary->points[1].x,
                  cellptr->detail.boundary->points[1].y,
                  cellptr->detail.boundary->points[2].x,
                  cellptr->detail.boundary->points[2].y,
                  cellptr->detail.boundary->points[3].x,
                  cellptr->detail.boundary->points[3].y);
#endif /* _NOTDEF_ */
        GDStransfBoundary(newcell->detail.boundary, transf);
#ifdef _NOTDEF_
        if(cellptr->detail.boundary->numpoints == 5)
          fprintf(stdout, "after: (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n",
                  newcell->detail.boundary->points[0].x,
                  newcell->detail.boundary->points[0].y,
                  newcell->detail.boundary->points[1].x,
                  newcell->detail.boundary->points[1].y,
                  newcell->detail.boundary->points[2].x,
                  newcell->detail.boundary->points[2].y,
                  newcell->detail.boundary->points[3].x,
                  newcell->detail.boundary->points[3].y);
#endif /* _NOTDEF_ */
        BoundaryToHPGL(hpglfile, newcell->detail.boundary, psStyle);
        GDSfreeBoundary(newcell);
        FREE(newcell);
        break;
      case PATH:
        newcell = GDSdupPath(cellptr->detail.path);
        GDStransfPath(newcell->detail.path, transf);
        PathToHPGL(hpglfile, newcell->detail.path, psStyle);
        GDSfreePath(newcell);
        FREE(newcell);
        break;
      case SREF:
        newtransf = GDStransfTransf(cellptr->detail.sref->transfptr, transf);
        SrefToHPGL(hpglfile, cellptr->detail.sref, newtransf, psStyle);
        GDSfreeTransf(newtransf);
        break;
      case AREF:
        ArefToHPGL(hpglfile, cellptr->detail.aref, transf, psStyle);
        break;
      case TEXT:
        TextToHPGL(hpglfile, cellptr->detail.text, transf, psStyle);
        break;
      case NODE:
        NodeToHPGL(hpglfile, cellptr->detail.node, psStyle);
        break;
      case BOX:
        BoxToHPGL(hpglfile, cellptr->detail.box, psStyle);
        break;
      default:
        break;
    }
}



GDScell *
GDSreadSref(int gdsfildes, GDSstruct *structptr)
{
  unsigned char *record, chunk;
  int rectype, nbytes;
  point ref;
  double mag, angle;
  int mirror = 0;
  GDScell *newcell;
  srefEl *srefptr;

  srefptr = (srefEl *)MALLOC(sizeof(srefEl));
  srefptr->strptr = NULL; /* to be inserted after all structs have been read */
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = SREF;
  newcell->detail.sref = srefptr;

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != SNAME)
  {
    fprintf(stderr, "Missing SNAME field in SREF element. Abort!\n");
    /* exit(1); */
  }
  if((srefptr->refname = GDSreadString(record + 2, nbytes - 4)) == NULL)
  {
    fprintf(stderr, "Bad STRING field in SREF element. Abort!\n");
    exit(1);
  }
  FREE(record);

  rectype = GDSreadRecord(gdsfildes, &record, &nbytes);
  if(rectype == STRANS)
  {
    chunk = record[2];
    FREE(record);
    if(chunk & 0x80)
      mirror = 1;
    switch(GDSreadRecord(gdsfildes, &record, &nbytes)) 
    {
      case MAG:
        mag = GDSreadReal8(record + 2);
        if(!(chunk & 0x02))
          mag *= CURRENTMAG;
        FREE(record);
        switch(GDSreadRecord(gdsfildes, &record, &nbytes)) 
        {
          case ANGLE:
            angle = GDSreadReal8(record + 2);
            if(!(chunk & 0x01))
              angle += CURRENTANGLE;
            FREE(record);
            if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)
            {
              fprintf(stderr, "Missing XY field in SREF element. Abort!\n");
              exit(1);
            }
            ref.x = GDSreadInt4(record + 2);
            ref.y = GDSreadInt4(record + 6);
            FREE(record);
            break;
          case XY:
            angle = CURRENTANGLE;
            ref.x = GDSreadInt4(record + 2);
            ref.y = GDSreadInt4(record + 6);
            FREE(record);
            break;
          default:
            fprintf(stderr, "Missing XY field in SREF element. Abort!\n");
            exit(1);
        }
        break;
      case ANGLE:
        mag = CURRENTMAG;
        angle = GDSreadReal8(record + 2);
        if(!(chunk & 0x01))
          angle += CURRENTANGLE;
        FREE(record);
        if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)
        {
          fprintf(stderr, "Missing XY field in SREF element. Abort!\n");
          exit(1);
        }
        ref.x = GDSreadInt4(record + 2);
        ref.y = GDSreadInt4(record + 6);
        FREE(record);
        break;
      case XY:
        mag = CURRENTMAG;
        angle = CURRENTANGLE;
        ref.x = GDSreadInt4(record + 2);
        ref.y = GDSreadInt4(record + 6);
        FREE(record);
        break;
      default:
        fprintf(stderr,
                "Missing MAG, ANGLE or XY field in SREF element. Abort!\n");
        exit(1);
    }
  }
  else if(rectype == XY)
  {
    mag = CURRENTMAG;
    angle = CURRENTANGLE;
    ref.x = GDSreadInt4(record + 2);
    ref.y = GDSreadInt4(record + 6);
    FREE(record);
  }
  else
  {
    fprintf(stderr, "Missing STRANS or XY field in SREF element. Abort!\n");
    exit(1);
  }
  if(GDSreadRecord(gdsfildes, &record, &nbytes) != ENDEL)
  {
    fprintf(stderr, "Missing ENDEL field in SREF element. Abort!\n");
    exit(1);
  }
  FREE(record);

    srefptr->transfptr = GDSgetTransf(ref, angle, mag, mirror);

  fprintf(stdout, "Sref at %d,%d, angle = %.2e, mag = %.2e, mirror = %d of cell named \"%s\"\n",
          ref.x, ref.y, angle, mag, mirror, srefptr->refname);
  return newcell;
}
