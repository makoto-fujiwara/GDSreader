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
#include <GDSstruct.h>

void
GDSreadHeader(int gdsfile)
{
  unsigned char *record;
  int nbytes;

  if(GDSreadRecord(gdsfile, &record, &nbytes) != HEADER)
  {
    fprintf(stderr, "HEADER is missing. Aborting\n");
    exit(1);
  }
  FREE(record);
}

void
GDSreadUpToFirstStruct(int gdsfildes, GDSlibrary *libptr)
{
  unsigned char *record;
  int nbytes;

  while(1)
  {
    switch(GDSreadRecord(gdsfildes, &record, &nbytes))
    {
      case BGNLIB:
        GDSaddDates(libptr, record);
        fprintf(stdout, "lastmod = %s, lastacc = %s\n",
                libptr->lastmod, libptr->lastacc);
        break;
      case LIBDIRSIZE:
        fprintf(stderr, "Not yet handling LIBDIRSIZE!\n");
        break;

      case SRFNAME:
        fprintf(stderr, "Not yet handling SRFNAME!\n");
        break;
      case LIBSECUR:
        fprintf(stderr, "Not yet handling LIBSECUR!\n");
        break;
      case REFLIBS:
        fprintf(stderr, "Not yet handling REFLIBS!\n");
        break;
      case FONTS:
        fprintf(stderr, "Not yet handling FONTS!\n");
        break;
      case LIBNAME:
        if((libptr->name = GDSreadString(record + 2, nbytes - 4)) == NULL)
        {
          fprintf(stderr, "Bad LIBNAME record. Aborting\n");
          exit(1);
        }
        fprintf(stdout, "libname = %s\n", libptr->name);
        break;
      case UNITS:
        libptr->userunit = GDSreadReal8(record + 2);
        libptr->meterunit = GDSreadReal8(record + 10);
        fprintf(stdout, "userunit = %e, meterunit = %e\n",
                libptr->userunit, libptr->meterunit);
        break;
      case BGNSTR:
        /* should read the last modified/read dates */
        GDSunreadRecord(gdsfildes, nbytes);
        return;
      default:
        break;
    }
    FREE(record);
  }
}

GDSlibrary *
GDSreadLib(int gdsfildes)
{
  FILE *outfile;
  GDSlibrary *libptr;
  GDSstruct *structptr, *structptr1;
  GDScell *cellptr;
  layer *layerptr;

  if((outfile = fopen("result.txt", "w")) == NULL)
  {
    fprintf(stderr, "Couldn't open result.txt for writing\n");
    exit(1);
  }
  libptr = (GDSlibrary *)MALLOC(sizeof(GDSlibrary));
  libptr->structs = NULL;

  GDSreadHeader(gdsfildes);
  GDSreadUpToFirstStruct(gdsfildes, libptr);
  while((structptr = GDSreadStruct(gdsfildes, libptr)) != NULL)
  {
    fprintf(outfile, "Read a struct named %s which contains the layers:\n",
            structptr->name);
    for(layerptr = structptr->layers;
        layerptr != NULL; layerptr = layerptr->next)
      fprintf(outfile, "no. %d, type %d,", layerptr->layerno,layerptr->datatype);
    fprintf(outfile, "\nand the cells:\n:");
    for(cellptr = structptr->cells; cellptr != NULL; cellptr = cellptr->next)
    {
      switch(cellptr->type)
      {
        case BOUNDARY:
          fprintf(outfile, "BOUNDARY, ");
          break;
        case PATH:
          fprintf(outfile, "PATH, ");
          break;
        case SREF:
          fprintf(outfile, "SREF(%s), ", cellptr->detail.sref->refname);
          break;
        case AREF:
          fprintf(outfile, "AREF(%s), ", cellptr->detail.aref->refname);
          break;
        case TEXT:
          fprintf(outfile, "TEXT, ");
          break;
        case NODE:
          fprintf(outfile, "NODE, ");
          break;
        case BOX:
          fprintf(outfile, "BOX, ");
          break;
      }
    }

    fprintf(outfile, "\n\n");


    structptr->next = libptr->structs;
    libptr->structs = structptr;
  }

  fclose(outfile);
  /* update the strptr pointers for SREF and AREF elements */
  for(structptr = libptr->structs; structptr != NULL;
      structptr = structptr->next)
    for(cellptr = structptr->cells; cellptr != NULL;
        cellptr = cellptr->next)
    {
      if(cellptr->type == SREF)
      {
        for(structptr1 = libptr->structs; structptr1 != NULL;
            structptr1 = structptr1->next)
          if(!strcmp(structptr1->name, cellptr->detail.sref->refname))
          {
            cellptr->detail.sref->strptr = structptr1;
            fprintf(stdout, " %04d %s Fixed reference for SREF \"%s\"\n", __LINE__, __func__,
                    cellptr->detail.sref->refname);
            break;
          }
      }
      else if(cellptr->type == AREF)
      {
        for(structptr1 = libptr->structs; structptr1 != NULL;
            structptr1 = structptr1->next)
          if(!strcmp(structptr1->name, cellptr->detail.aref->refname))
          {
            cellptr->detail.aref->strptr = structptr1;
            fprintf(stdout, " %04d %s Fixed reference for AREF \"%s\"\n", __LINE__, __func__,
                    cellptr->detail.aref->refname);
            break;
          }
      }
    }
  return libptr;
}

GDSstruct *
GDSgetStructByName(GDSlibrary *library, char *structname)
{
  GDSstruct *structptr;
  
  for(structptr = library->structs;
      structptr != NULL; structptr = structptr->next)
  {
    fprintf(stdout, "\"%s\"\n", structptr->name);
    if(!strcmp(structptr->name, structname))
      break;
  }
  
  return structptr;
}
