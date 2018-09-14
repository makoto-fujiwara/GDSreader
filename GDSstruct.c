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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <GDSstructs.h>
#include <GDSconsts.h>
#include <GDSaux.h>
#include <GDSboundary.h>
#include <GDSpath.h>
#include <GDSsref.h>
#include <GDSaref.h>
#include <GDStext.h>
#include <GDStext.h>
#include <GDSnode.h>
#include <GDSbox.h>
#include <GDStransf.h>

bbox
GDSgetStructBBox(GDSstruct *structptr)
{
  GDScell *cellptr;
  bbox bbx, bbx1;

  bbx.ll.x = BIGVAL;
  bbx.ll.y = BIGVAL;
  bbx.ur.x = -BIGVAL;
  bbx.ur.y = -BIGVAL;

  for(cellptr = structptr->cells; cellptr != NULL; cellptr = cellptr->next)
  {
    switch(cellptr->type)
    {
      case BOUNDARY:
        bbx1 = GDSgetBoundaryBBox(cellptr->detail.boundary);
        break;
      case PATH:
        bbx1 = GDSgetPathBBox(cellptr->detail.path);
        break;
      case TEXT:
        bbx1 = GDSgetTextBBox(cellptr->detail.text);
        break;
      case SREF:
        bbx1 = GDSgetSrefBBox(cellptr->detail.sref, &Ident);
        break;
      case AREF:
        bbx1 = GDSgetArefBBox(cellptr->detail.aref, &Ident);
        break;
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
  fprintf(stderr, "minx = %d, miny = %d, maxx = %d, maxy = %d\n",
          bbx.ll.x, bbx.ll.y, bbx.ur.x, bbx.ur.y);
  return bbx;
}

GDSstruct *
GDSreadStruct(int gdsfildes, GDSlibrary *libptr)
{
  unsigned char *record;
  int nbytes;
  GDSstruct *structptr;
  GDScell *newcell;

  if(GDSreadRecord(gdsfildes, &record, &nbytes) != BGNSTR)
  {
    FREE(record);
    GDSunreadRecord(gdsfildes, nbytes);
    if(GDSreadRecord(gdsfildes, &record, &nbytes) == ENDLIB)
      return NULL;
    else
    {
      fprintf(stderr, "Missing ENDLIB field. Abort!\n");
      exit(1);
    }
  }
  else
    FREE(record);

  structptr = (GDSstruct *)MALLOC(sizeof(GDSstruct));
  structptr->cells = NULL;
  structptr->layers = NULL;

  while(1)
  {
    int type = GDSreadRecord(gdsfildes, &record, &nbytes);
    switch(type)
    {
      case STRNAME:
        if((structptr->name = GDSreadString(record + 2, nbytes - 4)) == NULL)
        {
          fprintf(stderr, "Bad STRNAME record. Aborting\n");
          exit(1);
        }
        fprintf(stdout, " %04d %s strname = %s\n", __LINE__, __func__, structptr->name);
        break;
      case BOUNDARY:
        if((newcell = GDSreadBoundary(gdsfildes, structptr)) == NULL)
          return NULL;
        newcell->next = structptr->cells;
        structptr->cells = newcell;
        break;
      case PATH:
        if((newcell = GDSreadPath(gdsfildes, structptr)) == NULL)
          return NULL;
        newcell->next = structptr->cells;
        structptr->cells = newcell;
        break;
      case SREF:
        if((newcell = GDSreadSref(gdsfildes, structptr)) == NULL)
          return NULL;
        newcell->next = structptr->cells;
        structptr->cells = newcell;
        break;
      case AREF:
        if((newcell = GDSreadAref(gdsfildes, structptr)) == NULL)
          return NULL;
        newcell->next = structptr->cells;
        structptr->cells = newcell;
        break;
      case TEXT:
        if((newcell = GDSreadText(gdsfildes, structptr)) == NULL)
          return NULL;
        newcell->next = structptr->cells;
        structptr->cells = newcell;
        break;
      case NODE:
        if((newcell = GDSreadNode(gdsfildes, structptr)) == NULL)
          return NULL;
        newcell->next = structptr->cells;
        structptr->cells = newcell;
        break;
      case BOX:
        if((newcell = GDSreadBox(gdsfildes, structptr)) == NULL)
          return NULL;
        newcell->next = structptr->cells;
        structptr->cells = newcell;
        break;
/* These two cases may be reworked.
   Probably the return statement should be at ENDSTR. */
      case ENDSTR:
        FREE(record);
        return structptr;
      case BGNSTR:
      case ENDLIB:
        fprintf(stderr, "Incorrect record type in GDSreadStruct()\n");
        FREE(record);
        return NULL;
      default:
        fprintf(stderr, "%04d Unknown record type %d\n", __LINE__, type);
        break;
    }
    FREE(record);
  }
}

GDSstruct *
GDSsrefToStruct(srefEl *sref)
{
  GDSstruct *structptr;
  GDScell *cellptr, *newcell;
  char string[2048];
  int flag;
  
  structptr = (GDSstruct *)MALLOC(sizeof(GDSstruct));
  structptr->next = NULL;
  structptr->cells = NULL;
  structptr->layers = NULL; /* will be added later */
  sprintf(string, "%s_flatten", sref->refname);
  structptr->name = strdup(string);
  for(cellptr = sref->strptr->cells; cellptr != NULL; cellptr = cellptr->next)
  {
    switch(cellptr->type)
    {
      case BOUNDARY:
        flag = 1;
        newcell = GDSdupBoundary(cellptr->detail.boundary);
        break;
      case PATH:
        flag = 1;
        newcell = GDSdupPath(cellptr->detail.path);
        break;
      case TEXT:
        flag = 1;
        newcell = GDSdupText(cellptr->detail.text);
        break;
      case SREF:
        flag = 0;
        /* still a lot of work to do */
        break;
      case AREF:
        flag = 0;
        /* still a lot of work to do */
        break;
      default:
        flag = 0;
        break;
    }
    if(flag)
    {
      newcell->next = structptr->cells;
      structptr->cells = newcell;
    }
  }
  return structptr;
}
