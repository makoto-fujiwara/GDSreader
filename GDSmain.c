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
#include <GDSreader.h>
#include <GDStoHPGL.h>
#include <GDStoPS.h>
#include <GDStoPOV.h>

static void
printUsage(char *progname)
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "%s [options]\n", progname);
  fprintf(stderr, "where options are:\n");
  fprintf(stderr, "-h, --help:              prints this message\n");
  fprintf(stderr, "-H, --hpglfile \"file\":   opens \"file\" for writing HPGL\n");
  fprintf(stderr, "                         defaults to %s\n", DEFAULT_HPGL_FILE);
  fprintf(stderr, "-p, --psfile \"file\":     opens \"file\" for writing PostScript\n");
  fprintf(stderr, "                         defaults to %s\n", DEFAULT_PS_FILE);
  fprintf(stderr, "-v, --povfile \"file\":     opens \"file\" for writing POVRAY\n");
  fprintf(stderr, "                         defaults to %s\n", DEFAULT_POV_FILE);
  fprintf(stderr, "-i, --ifile \"file\":      opens \"file\" as the GDSii input file\n");
  fprintf(stderr, "                         if not specified, stdin is used\n");
  fprintf(stderr, "-c, --configfile \"file\": opens \"file\" as the layers definition file\n");
  fprintf(stderr, "                         defaults to %s\n", DEFAULT_CONFIG_FILE);
  fprintf(stderr, "-s, --struct \"sname\":    \"sname\" is the top cell to be printed\n");
  fprintf(stderr, "                         if not specified, the names of all structures\n");
  fprintf(stderr, "                         and layers # are written to stdout.\n");
  fprintf(stderr, "                         No PS/HPGL file are generated\n");
}

int
main(argc, argv)
  int argc;
  char **argv;
{
  GDSlibrary *libptr;
  int i, ifile = -1;
  char *configfile = NULL, *psfile = NULL, *povfile = NULL, *hpglfile = NULL, *structname = NULL;
  
  for(i = 1; i < argc; i++)
  {
    if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--ifile"))
    {
      i++;
      if(i == argc)
      {
        fprintf(stderr, "Missing argument for the option -ifile. Exiting...\n");
        exit(1);
      }
      ifile = open(argv[i], O_RDONLY);
      if(ifile == -1)
      {
        fprintf(stderr, "Couldn't open the file \"%s\". Exiting...\n",
                argv[i]);
        exit(1);
      }
    }
    else if(!strcmp(argv[i], "-s") || !strcmp(argv[i], "--struct"))
    {
      i++;
      if(i == argc)
      {
        fprintf(stderr, "Missing argument for the option %s. Exiting...\n",
                argv[i]);
        exit(1);
      }
      structname = argv[i];
    }
    else if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--configfile"))
    {
      i++;
      if(i == argc)
      {
        fprintf(stderr, "Missing argument for the option %s. Exiting...\n",
                argv[i]);
        exit(1);
      }
      configfile = argv[i];
    }
    else if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--psfile"))
    {
      i++;
      if(i == argc)
      {
        fprintf(stderr, "Missing argument for the option %s. Exiting...\n",
                argv[i]);
        exit(1);
      }
      psfile = argv[i];
    }
    else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--povfile"))
    {
      i++;
      if(i == argc)
      {
        fprintf(stderr, "Missing argument for the option %s. Exiting...\n",
                argv[i]);
        exit(1);
      }
      povfile = argv[i];
    }
    else if(!strcmp(argv[i], "-H") || !strcmp(argv[i], "--hpglfile"))
    {
      i ++;
      if(i == argc)
      {
        fprintf(stderr, "Missing argument for the option %s. Exiting...\n",
                argv[i]);
        exit(1);
      }
      hpglfile = argv[i];
    }
    else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
    {
      printUsage(argv[0]);
      exit(0);
    }
    else
    {
      fprintf(stderr, "Unknown option \"%s\"\n", argv[i]);
      printUsage(argv[0]);
      exit(1);
    }
  }

  if(ifile == -1)
    ifile = 0;
  
  libptr = GDSreadLib(ifile);
  if(libptr == NULL)
  {
    fprintf(stderr, "Couldn't read the Calma library. Exiting...\n");
    exit(1);
  }
  close(ifile);

  if(structname == NULL)
  {
    fprintf(stderr,
            "For the time being, please look at the file result.txt\n"); 
    exit(0);
  }
  else
  {
    if(configfile == NULL)
      configfile = strdup(DEFAULT_CONFIG_FILE);
    if(psfile == NULL)
      psfile = strdup(DEFAULT_PS_FILE);
    if(povfile == NULL)
      povfile = strdup(DEFAULT_POV_FILE);
    if(hpglfile == NULL)
      hpglfile = strdup(DEFAULT_HPGL_FILE);
 
    GDStoPS(libptr, psfile, configfile, structname);
    GDStoPOV(libptr, povfile, structname);

    GDStoHPGL(libptr, hpglfile, structname); 
  }
 return(0);
}
