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

#include <GDSstructs.h>
#include <GDSconsts.h>
#include <GDSaux.h>
#include <GDStransf.h>

#define FONTNAME "Helvetica"
#define FONTSIZE 20

bbox
GDSgetTextBBox(textEl *text)
{
  bbox bbx;

  bbx.ll.x = BIGVAL;
  bbx.ll.y = BIGVAL;
  bbx.ur.x = -BIGVAL;
  bbx.ur.y = -BIGVAL;

  return bbx;
}

void
TextToPS(FILE *psfile, textEl *text, transform *transf, PSStyle psStyle)
{
  point pnt, ref00;
  transform *newtransf;

  if(text->layerptr->layerno != psStyle.gdsno)
    return;

  newtransf = GDStransfTransf(text->transfptr, transf);
  fprintf(psfile, "/%s %.3f f\n", FONTNAME, FONTSIZE * text->mag);

  ref00.x = 0;
  ref00.y = 0;
  pnt = GDStransfPoint(&ref00, newtransf);
  fprintf(psfile, "(%s) %d %d t\n",
          text->string, pnt.x / InternalScaleFactor,
          pnt.y / InternalScaleFactor);
  GDSfreeTransf(newtransf);
  return;
}

void
TextToPOV(FILE *povfile, textEl *text, transform *transf, PSStyle psStyle)
{
/*
  point pnt,ref00;
*/
  transform *newtransf;
  /* fprintf(povfile,"// Rendering text...  Not yet implemented\n"); */
  if(text->layerptr->layerno != psStyle.gdsno)
    return;

  newtransf = GDStransfTransf(text->transfptr, transf);
/*  fprintf(povfile, "//%s %.3f f\n", FONTNAME, FONTSIZE * text->mag);

  ref00.x = 0;
  ref00.y = 0;
  pnt = GDStransfPoint(&ref00, newtransf);
    fprintf(povfile, " // (%s) %d %d t\n",
          text->string, pnt.x / InternalScaleFactor,
          pnt.y / InternalScaleFactor); */
  GDSfreeTransf(newtransf);
  return;
}

void
TextToHPGL(FILE *hpglfile, textEl *text, transform *transf, PSStyle psStyle)
{
  point pnt, ref00;
  transform *newtransf;

  if(text->layerptr->layerno != psStyle.gdsno)
    return;

  newtransf = GDStransfTransf(text->transfptr, transf);
  fprintf(hpglfile, "SR%f,%f", 0.02 * text->mag, 0.02 * text->mag);

  ref00.x = 0;
  ref00.y = 0;
  pnt = GDStransfPoint(&ref00, newtransf);
  fprintf(hpglfile, "PU%d,%dLO6LB%s\n",
          pnt.x / InternalScaleFactor,
          pnt.y / InternalScaleFactor, text->string);
  GDSfreeTransf(newtransf);
  return;
}

/*
 * Frees the cell contents and removes the layer list reference.
 * The structure list reference has to be removed at the upper level.
 */
void
GDSfreeText(GDScell *cell)
{
  GDScell *cellptr;

  if(cell == NULL)
    return;
  if(cell->type != PATH)
    return;

  if(cell == cell->detail.text->layerptr->cells)
    cell->detail.text->layerptr->cells = cell->next;
  else
  {
    for(cellptr = cell->detail.text->layerptr->cells;
        cellptr->next != NULL; cellptr = cellptr->next)
    {
      if(cellptr->next == cell)
      {
        cellptr->next = cellptr->next->next;
        break;
      }
    }
    if(cellptr == NULL)
      fprintf(stderr, "Oops! GDSfreeText(): Missing cell in layerptr\n");
  }
  FREE(cell->detail.text->string);
  FREE(cell->detail.text->transfptr);
  FREE(cell->detail.text);
  return;
}

GDScell *
GDSdupText(textEl *text)
{
  GDScell *newcell;
  textEl *textptr;

  if(text == NULL)
    return NULL;

  textptr = (textEl *)MALLOC(sizeof(textEl));
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = TEXT;
  newcell->detail.text = textptr;

  newcell->next = text->layerptr->cells;
  text->layerptr->cells = newcell;

  textptr->layerptr = text->layerptr;
  textptr->hjustif = text->hjustif;
  textptr->vjustif = text->vjustif;
  textptr->font = text->font;
  textptr->width = text->width;
  textptr->pathtype = text->pathtype;
  textptr->string = strdup(text->string);
  textptr->transfptr = GDSdupTransf(text->transfptr);
  
  return newcell;
}

/*
 * Apply the transform transf to the text element
 */
void
GDStransfText(textEl *text, transform *transf)
{
  transform *transfptr;

  return;
  transfptr = text->transfptr;
  text->transfptr = GDStransfTransf(transfptr, transf);
  FREE(transfptr);

  return;
}

GDScell *
GDSreadText(int gdsfildes, GDSstruct *structptr)
{
  unsigned char *record, chunk;
  int nbytes, layerno, rectype,datatype;
  layer *layerptr;
  point ref;
  double mag, angle;
  int mirror = 0;
  GDScell *newcell;
  textEl *textptr;

  textptr = (textEl *)MALLOC(sizeof(textEl));
  newcell = (GDScell *)MALLOC(sizeof(GDScell));
  newcell->type = TEXT;
  newcell->detail.text = textptr;

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != LAYER)
  {
    fprintf(stderr, "Missing LAYER field in TEXT element. Abort!\n");
    exit(1);
  }
  layerno = GDSreadInt2(record + 2);
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != TEXTTYPE)
  {
    fprintf(stderr, "Missing TEXTTYPE field in TEXT element. Abort!\n");
    /* exit(1); */
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
  textptr->layerptr = layerptr;

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != PRESENTATION)
  {
    fprintf(stderr, "Missing PRESENTATION field in TEXT element. Continuing!\n");
  }
  chunk = record[2];
  chunk &= 0x30;
  chunk >>= 4;
  textptr->font = chunk;

  chunk = record[2];
  chunk &= 0x0c;
  chunk >>= 2;
  switch(chunk)
  {
    case 0x00:
      textptr->vjustif = TOP;
      break;
    case 0x01:
      textptr->vjustif = MIDDLE;
      break;
    case 0x02:
      textptr->vjustif = BOTTOM;
      break;
    default:
      textptr->vjustif = MIDDLE;
      fprintf(stderr, "Illegal vertical justification in TEXT element.\n");
  }

  chunk = record[2];
  chunk &= 0x03;
  switch(chunk)
  {
    case 0x00:
      textptr->hjustif = LEFT;
      break;
    case 0x01:
      textptr->hjustif = CENTER;
      break;
    case 0x02:
      textptr->hjustif = RIGHT;
      break;
    default:
      textptr->vjustif = CENTER;
      fprintf(stderr, "Illegal horizontal justification in TEXT element.\n");
  }
  if(record[2] & 0xffc0)
  {
    fprintf(stderr, "Undefined bits are set in PRESENTATION\n");
    /* exit(1); */
  }
  FREE(record);

  rectype = GDSreadRecord(gdsfildes, &record, &nbytes);
  if(rectype == STRANS)
  {
    chunk = record[2];
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
            if(GDSreadRecord(gdsfildes, &record, &nbytes) != XY)
            {
              fprintf(stderr, "Missing XY field in TEXT element. Abort!\n");
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
            fprintf(stderr, "Missing XY field in TEXT element. Abort!\n");
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
          fprintf(stderr, "Missing XY field in TEXT element. Abort!\n");
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
                "Missing MAG, ANGLE or XY field in TEXT element. Abort!\n");
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
    fprintf(stderr, "Missing STRANS or XY field in TEXT element. Continuing!\n");
  }

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != STRING)
  {
    fprintf(stderr, "Missing STRING field in TEXT element. Continuing!\n");
  }
  if((textptr->string = GDSreadString(record + 2, nbytes - 4)) == NULL) 
  {
    fprintf(stderr, "Bad STRING field in TEXT element. Abort!\n");
    exit(1);
  }
  FREE(record);

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != ENDEL)
  {
    fprintf(stderr, "Missing ENDEL field in TEXT element. Continuing!\n");
  }
  FREE(record);

  textptr->transfptr = GDSgetTransf(ref, angle, mag, mirror);
  textptr->mag = mag;

  fprintf(stdout, "String on layer %d, datatype %d, at %d,%d = \"%s\"\n",
          layerno, layerptr->datatype, ref.x, ref.y, textptr->string);
  fprintf(stdout, "hjust = %d, vjust = %d, mag = %e and angle = %e\n",
          textptr->hjustif, textptr->vjustif, mag, angle);
  return newcell;
}
