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
#include <GDSparser.h>

static char
*PSInit[] = 
{
  "%!PS-Adobe-2.0",
  "%%EndComments",
  "/m { %def",
  "  moveto",
  "} bind def",
  "",
  "/l { %def",
  "  lineto",
  "} bind def",
  "",
  "/t { %def",
  "  moveto show",
  "} bind def",
  "",
  "/selectfont where { %true",
  "  pop",
  "}{ %false",
  "  /selectfont { %def",
  "    exch findfont exch scalefont setfont",
  "  } bind def",
  "}",
  "/f /selectfont load def",
  "[] 0 setdash",
  "",
  "/definepattern { %def",
  "  7 dict begin",
  "    /FontDict 9 dict def",
  "    FontDict begin",
  "      /cache exch def",
  "      /key exch def",
  "      /proc exch cvx def",
  "      /mtx exch matrix invertmatrix def",
  "      /height exch def",
  "      /width exch def",
  "      /ctm matrix currentmatrix def",
  "      /ptm matrix identmatrix def",
  "      /str",
  "      (12345678901234567890123456789012)",
  "      def",
  "    end",
  "    /FontBBox [ %def",
  "      0 0 FontDict /width get",
  "      FontDict /height get",
  "    ] def",
  "    /FontMatrix FontDict /mtx get def",
  "    /Encoding StandardEncoding def",
  "    /FontType 3 def",
  "    /BuildChar { %def",
  "      pop begin",
  "      FontDict begin",
  "        width 0 cache { %ifelse",
  "          0 0 width height setcachedevice",
  "        }{ %else",
  "          setcharwidth",
  "        } ifelse",
  "        0 0 moveto width 0 lineto",
  "        width height lineto 0 height lineto",
  "        closepath clip newpath",
  "        gsave proc grestore",
  "      end end",
  "    } def",
  "    FontDict /key get currentdict definefont",
  "  end",
  "} bind def",
  "",
  "/patternpath { %def",
  "  dup type /dicttype eq { %ifelse",
  "    begin FontDict /ctm get setmatrix",
  "  }{ %else",
  "    exch begin FontDict /ctm get setmatrix",
  "    concat",
  "  } ifelse",
  "  currentdict setfont",
  "  FontDict begin",
  "    FontMatrix concat",
  "    width 0 dtransform",
  "    round width div exch round width div exch",
  "    0 height dtransform",
  "    round height div exch",
  "    round height div exch",
  "    0 0 transform round exch round exch",
  "    ptm astore setmatrix",
  "    ",
  "    pathbbox",
  "    height div ceiling height mul 4 1 roll",
  "    width div ceiling width mul 4 1 roll",
  "    height div floor height mul 4 1 roll",
  "    width div floor width mul 4 1 roll",
  "    ",
  "    2 index sub height div ceiling cvi exch",
  "    3 index sub width div ceiling cvi exch",
  "    4 2 roll moveto",
  "    ",
  "    FontMatrix ptm invertmatrix pop",
  "    { %repeat",
  "      gsave",
  "        ptm concat",
  "        dup str length idiv { %repeat",
  "          str show",
  "        } repeat",
  "        dup str length mod str exch",
  "        0 exch getinterval show",
  "      grestore",
  "      0 height rmoveto",
  "    } repeat",
  "    pop",
  "  end end",
  "} bind def",
  "",
  "/patternfill { %def",
  "  gsave",
  "    clip patternpath",
  "  grestore",
  "  newpath",
  "} bind def",
  "",
  NULL
};

/* This should be made dynamic.  Can have layers *AND* datatypes */
PSStyle psStyles[1024];
int howmanylayers;

static void
writePSHeader(FILE *psfile)
{
  int i;

  for(i = 0; PSInit[i] != NULL; i++)
    fprintf(psfile, "%s\n", PSInit[i]);
}

/*
 * Code stolen from the Blue Book. The next revision will use patterns instead.
 */
static void
writePSHatchFont(FILE *psfile, PSStyle psStyle)
{
  double w, h;

  switch(psStyle.hatch)
  {
    case 0:
      break;
    case 1:
      if(sin(psStyle.angle) == 0.0 || cos(psStyle.angle) == 0.0)
      {
        fprintf(stderr, "Horizontal/vertical hatching not supported yet.\n");
        fprintf(stderr, "Please check layer \"%s\" configuration. Exiting.\n",
                psStyle.layername);
        exit(1);
      }
      w = psStyle.step * 72.0 / (25.4 * sin(psStyle.angle * M_PI / 180.0));
      h = psStyle.step * 72.0 / (25.4 * cos(psStyle.angle * M_PI / 180.0));
      if(h > 0.0)
      {
        fprintf(psfile, "%.2f %.2f [300 72 div 0 0 300 72 div 0 0]\n", w, h);
        fprintf(psfile, "{ %%definepattern\n  2 setlinecap\n");
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                w / 2.0, 0.0, w, h / 2.0);
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                0.0, h / 2.0, w / 2.0, h);
        fprintf(psfile, "  0.1 setlinewidth stroke\n} bind\n");
        fprintf(psfile, "/FONT%s true definepattern pop\n\n",
                psStyle.layername);
      }
      else
      {
        fprintf(psfile, "%.2f %.2f [300 72 div 0 0 300 72 div 0 0]\n", w, - h);
        fprintf(psfile, "{ %%definepattern\n  2 setlinecap\n");
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                w / 2.0, 0.0, 0.0, - h / 2.0);
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                w, - h / 2.0, w / 2.0, - h);
        fprintf(psfile, "  0.1 setlinewidth stroke\n} bind\n");
        fprintf(psfile, "/FONT%s true definepattern pop\n\n",
                psStyle.layername);
      }
      break;
    case 2:
      if(sin(psStyle.angle) == 0.0 || cos(psStyle.angle) == 0.0)
      {
        fprintf(stderr, "Horizontal/vertical hatching not supported yet.\n");
        fprintf(stderr, "Please check layer \"%s\" configuration. Exiting.\n",
                psStyle.layername);
        exit(1);
      }
      w = psStyle.step / sin(psStyle.angle * M_PI / 180.0);
      h = psStyle.step / cos(psStyle.angle * M_PI / 180.0);
      if(h > 0.0)
      {
        fprintf(psfile, "%.2f %.2f [300 72 div 0 0 300 72 div 0 0]\n", w, h);
        fprintf(psfile, "{ %%definepattern\n  2 setlinecap\n");
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                w / 2.0, 0.0, w, h / 2.0);
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                0.0, h / 2.0, w / 2.0, h);
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                w / 2.0, h, w, h / 2.0);
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                0.0, h / 2.0, w / 2.0, 0.0);
        fprintf(psfile, "  0.1 setlinewidth stroke\n} bind\n");
        fprintf(psfile, "/FONT%s true definepattern pop\n\n",
                psStyle.layername);
      }
      else
      {
        fprintf(psfile, "%.2f %.2f [300 72 div 0 0 300 72 div 0 0]\n", w, - h);
        fprintf(psfile, "{ %%definepattern\n  2 setlinecap\n");
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                w / 2.0, 0.0, 0.0, - h / 2.0);
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                w, - h / 2.0, w / 2.0, - h);
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                w / 2.0, -h, 0.0, - h / 2.0);
        fprintf(psfile, "  %.2f %.2f m %.2f %.2f l\n",
                w, - h / 2.0, w / 2.0, 0.0);
        fprintf(psfile, "  0.1 setlinewidth stroke\n} bind\n");
        fprintf(psfile, "/FONT%s true definepattern pop\n\n",
                psStyle.layername);
      }
      break;
    default:
      break;
  }
}

void
GDStoPS(GDSlibrary *libptr, char *psfile, char *configfile, char *topcellname)
{
  GDSstruct *structptr;
  GDScell *cellptr;
  FILE *psfileptr, *cfgfileptr;
  bbox bbx;
  double scalex, scaley, scale;
  int i;
  
  cfgfileptr = fopen(configfile, "r");
  if(cfgfileptr == NULL)
  {
    fprintf(stderr, "Couldn't open %s for reading. Aborting...\n", configfile);
    exit(1);
  }
  howmanylayers = GDSinitPSStyles(cfgfileptr);
  if(howmanylayers < 2)
    return;
  fclose(cfgfileptr);
  psfileptr = fopen(psfile, "w");
  if(psfileptr == NULL)
  {
    fprintf(stderr, "Couldn't open %s for writing. Aborting...\n", psfile);
    exit(1);
  }
  writePSHeader(psfileptr);
   /* Offset by one to get around dummy hidden hack. */
  for(i = 1; i < howmanylayers; i++)
    writePSHatchFont(psfileptr, psStyles[i]);
  fprintf(psfileptr, "%%%%End of prologue\n");

  structptr = GDSgetStructByName(libptr, topcellname);
  if(structptr == NULL)
  {
    fprintf(stderr, "Couldn't find the structure named \"%s\"\n",
            topcellname);
    return;
  }
  bbx = GDSgetStructBBox(structptr);
  scalex = 8.5 * InternalScaleFactor * 72.0 * 0.85 / (bbx.ur.x - bbx.ll.x);
  scaley = 11.0 * InternalScaleFactor * 72.0 * 0.85 / (bbx.ur.y - bbx.ll.y);
  fprintf(stderr, "scalex = %f, scaley = %f\n", scalex, scaley);
  scale = (scalex < scaley) ? scalex : scaley;
  fprintf(psfileptr, "%f %f scale\n", scale, scale);
  fprintf(psfileptr, "%f %f translate\n",
          - (double)bbx.ll.x / InternalScaleFactor +
          8.5 * 72.0 * 0.075 / scale,
          - (double)bbx.ll.y / InternalScaleFactor +
          11.0 * 72.0 * 0.075 / scale);

  fprintf(psfileptr, "0 setlinewidth\n");
  /* Offset by one to get around dummy hidden hack. */
  for(i = 1; i < howmanylayers; i++)
  {
    if(psStyles[i].hidden)
      continue;

    fprintf(psfileptr, "%% Layer %s\n", psStyles[i].layername);
    fprintf(psfileptr, "gsave\n%.3f %.3f %.3f setrgbcolor\nnewpath\n",
            psStyles[i].edgecolor.r, psStyles[i].edgecolor.g,
            psStyles[i].edgecolor.b);

    for(cellptr = structptr->cells; cellptr != NULL;
        cellptr = cellptr->next)
    {
      switch(cellptr->type)
      {
        case BOUNDARY:
          BoundaryToPS(psfileptr, cellptr->detail.boundary,
                       psStyles[i]);
          break;
        case PATH:
          PathToPS(psfileptr, cellptr->detail.path, psStyles[i]);
          break;
        case SREF:
          SrefToPS(psfileptr, cellptr->detail.sref, &Ident, psStyles[i]);
          break;
        case AREF:
          ArefToPS(psfileptr, cellptr->detail.aref, &Ident, psStyles[i]);
          break;
        case TEXT:
          TextToPS(psfileptr, cellptr->detail.text, &Ident, psStyles[i]);
          break;
        case NODE:
          NodeToPS(psfileptr, cellptr->detail.node, psStyles[i]);
          break;
        case BOX:
          BoxToPS(psfileptr, cellptr->detail.box, psStyles[i]);
          break;
        default:
          break;
      } /* switch */
    } /* cell loop */
  }
  fprintf(psfileptr, "showpage\n");

  fclose(psfileptr);
}
