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
#include <GDStransf.h>
#include <GDSsref.h>
#include <GDSaux.h>

bbox
GDSgetArefBBox(arefEl *aref, transform *transf)
{
  bbox bbx, bbx1;
  srefEl sref;
  transform *newtransf;
  point refpoint;
  int i, j;

  bbx.ll.x = BIGVAL;
  bbx.ll.y = BIGVAL;
  bbx.ur.x = -BIGVAL;
  bbx.ur.y = -BIGVAL;

  for(i = 0; i < aref->cols; i++)
    for(j = 0; j < aref->rows; j++)
    {
      sref.strptr = aref->strptr;
      sref.refname = aref->refname;

      refpoint.x = i * aref->colspacing;
      refpoint.y = j * aref->rowspacing;
      sref.transfptr = GDStranslateTransf(&Ident, refpoint);
      newtransf = GDStransfTransf(aref->transfptr, transf);
      bbx1 = GDSgetSrefBBox(&sref, newtransf);

      GDSfreeTransf(sref.transfptr);
      GDSfreeTransf(newtransf);

      if(bbx.ll.x > bbx1.ll.x)
        bbx.ll.x = bbx1.ll.x;
      if(bbx.ll.y > bbx1.ll.y)
        bbx.ll.y = bbx1.ll.y;
      if(bbx.ur.x < bbx1.ur.x)
        bbx.ur.x = bbx1.ur.x;
      if(bbx.ur.y < bbx1.ur.y)
        bbx.ur.y = bbx1.ur.y;
    }
  return bbx;
}

void
ArefToPS(FILE *psfile, arefEl *aref, transform *transf, PSStyle psStyle)
{
  srefEl sref;
  transform *newtransf;
  point refpoint;
  int i, j;

  for(i = 0; i < aref->cols; i++)
    for(j = 0; j < aref->rows; j++)
    {
      sref.strptr = aref->strptr;
      sref.refname = aref->refname;

      refpoint.x = i * aref->colspacing;
      refpoint.y = j * aref->rowspacing;
      sref.transfptr = GDStranslateTransf(&Ident, refpoint);
      newtransf = GDStransfTransf(aref->transfptr, transf);
      SrefToPS(psfile, &sref, newtransf, psStyle);
      GDSfreeTransf(sref.transfptr);
      GDSfreeTransf(newtransf);
    }
}

void
ArefToPOV(FILE *povfile, arefEl *aref, transform *transf, PSStyle psStyle)
{
  srefEl sref;
  transform *newtransf;
  point refpoint;
  int i, j;
  fprintf(stderr,"Rendering Aref->POV\n"
    "  Rows: %d\n  Cols: %d\n  RowSpace: %d\n  ColSpace: %d\n",
    aref->rows,aref->cols,aref->rowspacing,aref->colspacing
   ); 
  for(i = 0; i < aref->cols; i++)
    for(j = 0; j < aref->rows; j++)
    {
      sref.strptr = aref->strptr;
      sref.refname = aref->refname;

      refpoint.x = i * aref->colspacing;
      refpoint.y = j * aref->rowspacing;
      sref.transfptr = GDStranslateTransf(&Ident, refpoint);
      newtransf = GDStransfTransf(aref->transfptr, transf);
      /* Here we have a sref being instanced with additional
               translations. */
      SrefToPOV(povfile, &sref, newtransf, psStyle);
      GDSfreeTransf(sref.transfptr);
      GDSfreeTransf(newtransf);
    }
}

void
ArefToHPGL(FILE *hpglfile, arefEl *aref, transform *transf, PSStyle psStyle)
{
  srefEl sref;
  transform *transfptr, *tp1;
  point refpoint;
  int i, j;
  
  for(i = 0; i < aref->cols; i++)
    for(j = 0; j < aref->rows; j++)
    {
      sref.strptr = aref->strptr;
      sref.refname = aref->refname;

      refpoint.x = i * aref->colspacing;
      refpoint.y = j * aref->rowspacing;
      tp1 = GDStranslateTransf(&Ident, refpoint);
      transfptr = GDStransfTransf(tp1, aref->transfptr);
      sref.transfptr = GDStransfTransf(transfptr, transf);
      SrefToHPGL(hpglfile, &sref, sref.transfptr, psStyle);
      GDSfreeTransf(sref.transfptr);
      GDSfreeTransf(transfptr);
      GDSfreeTransf(tp1);
    }
}

GDScell *
GDSreadAref(int gdsfildes, GDSstruct *structptr)
{
  unsigned char *record, chunk;
  int rectype, nbytes;
  point ref, col, row;
  double angle, mag;
  int mirror = 0;
  GDScell *newcell;
  arefEl *arefptr;

  arefptr = (arefEl *)MALLOC(sizeof(arefEl));
  arefptr->strptr = NULL; /* to be inserted after all structs have been read */
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = AREF;
  newcell->detail.aref = arefptr;
  if(GDSreadRecord(gdsfildes, &record, &nbytes) != SNAME)
  {
    fprintf(stderr, "Missing SNAME field in AREF element. Abort!\n");
    /* exit(1); */
  }
  if((arefptr->refname = GDSreadString(record + 2, nbytes - 4)) == NULL)
  {
    fprintf(stderr, "Bad STRING field in AREF element. Abort!\n");
    exit(1);
  }
  FREE(record);

  rectype = GDSreadRecord(gdsfildes, &record, &nbytes);
  if(rectype == STRANS)
  {
    chunk = record[2];
    if(chunk & 0x80)
      mirror = 1;

    FREE(record);
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
            if(GDSreadRecord(gdsfildes, &record, &nbytes) != COLROW)
            {
              fprintf(stderr, "Missing COLROW field in AREF element. Abort!\n");
              exit(1);
            }
            arefptr->cols = GDSreadInt2(record + 2);
            arefptr->rows = GDSreadInt2(record + 4);
            FREE(record);
            break;
          case COLROW:
            angle = CURRENTANGLE;
            arefptr->cols = GDSreadInt2(record + 2);
            arefptr->rows = GDSreadInt2(record + 4);
            FREE(record);
            break;
          default:
            fprintf(stderr, "Missing COLROW field in AREF element. Abort!\n");
            exit(1);
        }
        break;
      case ANGLE:
        mag = CURRENTMAG;
        angle = GDSreadReal8(record + 2);
        if(!(chunk & 0x01))
          angle += CURRENTANGLE;
        FREE(record);
        if(GDSreadRecord(gdsfildes, &record, &nbytes) != COLROW)
        {
          fprintf(stderr, "Missing COLROW field in AREF element. Abort!\n");
          exit(1);
        }
        arefptr->cols = GDSreadInt2(record + 2);
        arefptr->rows = GDSreadInt2(record + 4);
        FREE(record);
        break;
      case COLROW:
        mag = CURRENTMAG;
        angle = CURRENTANGLE;
        arefptr->cols = GDSreadInt2(record + 2);
        arefptr->rows = GDSreadInt2(record + 4);
        FREE(record);
        break;
      default:
        fprintf(stderr,
                "Missing MAG, ANGLE or COLROW field in AREF element. Abort!\n");
        exit(1);
    }
  }
  else if(rectype == COLROW)
  {
    mag = CURRENTMAG;
    angle = CURRENTANGLE;
    arefptr->cols = GDSreadInt2(record + 2);
    arefptr->rows = GDSreadInt2(record + 4);
    FREE(record);
  }
  else
  {
    fprintf(stderr, "Missing STRANS or COLROW field in AREF element. Abort!\n");
    exit(1);
  }

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)
  {
    fprintf(stderr, "Missing XY field in AREF element. Abort!\n");
    exit(1);
  }
  if(arefptr->rows == 0 || arefptr->cols == 0)
  {
    fprintf(stderr, "AREF element with 0 rows/cols.  Abort!\n");
    exit(1);
  }

  ref.x = GDSreadInt4(record + 2);
  ref.y = GDSreadInt4(record + 6);
  col.x = GDSreadInt4(record + 10);
  col.y = GDSreadInt4(record + 14);
  row.x = GDSreadInt4(record + 18);
  row.y = GDSreadInt4(record + 22);
  FREE(record);

  arefptr->transfptr = GDSgetTransf(ref, angle, mag, mirror);

/* Since GDSinvtransfPoint performs also the translation, the unused dimension
   must be equal to 0 (i.e. the original one was identical to the reference
   point one) and the useful one is exactly the spacing multiplied by the number
   of cells in that direction */

  col = GDSinvtransfPoint(&col, arefptr->transfptr);

  if(col.x < 0)
  {
    fprintf(stdout, "Error in AREF! Found a y-axis mirrored array. This is impossible so I'm exiting.\n");
    exit(1);    
  }
  if(col.y != 0)
  {
    fprintf(stdout, "Error in AREF! The second point in XY is broken.\n");
    exit(1);
  }

  row = GDSinvtransfPoint(&row, arefptr->transfptr);

  if(row.x != 0)
  {
    fprintf(stdout, "Error in AREF! The third point in XY is broken.\n");
    exit(1);
  }

  arefptr->colspacing =
    col.x / arefptr->cols;
  arefptr->rowspacing =
    row.y / arefptr->rows;

  if(row.y < 0)
    arefptr->rowspacing *=-1.0;
/*    arefptr->transfptr->c -= arefptr->rowspacing; */
  if(GDSreadRecord(gdsfildes, &record, &nbytes) != ENDEL)
  {
    fprintf(stderr, "Missing ENDEL field in AREF element. Skipping!\n");
    // exit(1);
  }
  FREE(record);

  fprintf(stdout, " %04d %s Aref at %9d,%9d of cell named \"%s\", c/r  = %d/%d, spacing = %d/%d, mirror = %d\n", 
	  __LINE__, __func__,
          ref.x, ref.y, arefptr->refname, arefptr->cols, arefptr->rows,
          arefptr->colspacing, arefptr->rowspacing, mirror);  
  return newcell;
}
