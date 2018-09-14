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
/*
static char
*PSInit[] = 
{
 
};
*/


extern PSStyle psStyles[];
extern int howmanylayers; /* set by GDSinitPSStyles(cfgfileptr); in GDStoPS.c */


/* 
   Layer to style...  Map layer to style number using best find. 
   all layers have gdsno and datatype, not all styles have datatype
   Ugh...  What about layers that have no entry in config file?  Damn.
   We need to return a number for a style that is hidden.  I will make
   a dummy [0] style with a neg gds number which will always be present.
   
   Ugh.  That also means I have to modify the GDStoPS and GDStoHPGL 
   converters to ignore entry zero.  Damn, this code sucks.
   
*/
int layer_to_style_no(layer *layerptr)
{
 int i,generic=-1;
 for(i=0;i<howmanylayers;i++) {
   if (psStyles[i].gdsno==layerptr->layerno && psStyles[i].dataspec 
           && psStyles[i].datatype==layerptr->datatype)
              {
                return(i); /* Perfect Match */
              }
   if (psStyles[i].gdsno==layerptr->layerno && !(psStyles[i].dataspec)) generic=i;
  }
 if (generic!=-1) {
   return(generic);
  }
 fprintf(stderr,"Warning: Layer %d, datatype %d found with no corresponding\n"
                " layer entry in configuration file.\n",
          layerptr->layerno,layerptr->datatype);
 /* exit(1); */
 return(0);  /* Dummy hidden layer */
}




/* Returns true if any part of structure is visible, actually 
 returns number of items visible.  Subitems count as one. */
int is_visible(GDSstruct *structptr)
{
 layer *lp;
 GDScell *cellptr;
 int vis=0;
  lp=structptr->layers;
  while (lp!=NULL) { 
   if (!psStyles[layer_to_style_no(lp)].hidden) {
     vis++;
    }
   lp=lp->next;
  }
 /* Now we must check srefs and arefs if they are present. */
 cellptr=structptr->cells;
 while (cellptr!=NULL) {
  switch(cellptr->type)
      {
        case SREF:
           if (is_visible(cellptr->detail.aref->strptr)) vis++;  
          break;
        case AREF:
           if (is_visible(cellptr->detail.sref->strptr)) vis++;  
          break;
        default:
          break;
      } 
   cellptr=cellptr->next;  
  }
 return(vis); 
} 

/* Dump a particular cell declaration to the file.  
   We need to see if any of the layers are visible first, also 
   check if only a single subcell VISIBLE, no union or braces.... 
   note this is not topcellname here really...  */
void
GDStoPOVCell(GDSlibrary *libptr, FILE *povfileptr, char *topcellname)
{
  GDSstruct *structptr;
  GDScell *cellptr;
  bbox bbx;
  char *esctopcell=NULL;
  
  double scalex, scaley ; /*, scale; */
  int i=0,subcells=0;


  structptr = GDSgetStructByName(libptr, topcellname);
  if(structptr == NULL)
  {
    fprintf(stderr, "Couldn't find the structure named \"%s\"\n",
            topcellname);
    return;
  }
  /* We should check to see if any of the layers in this cell are visible.  If 
     not, then we should draw no structure(s)? */
  /* Visibility check */

  if (!(subcells=is_visible(structptr))) return;
 
  bbx = GDSgetStructBBox(structptr);

  scalex = 8.5 * InternalScaleFactor * 72.0 * 0.85 / (bbx.ur.x - bbx.ll.x);
  scaley = 11.0 * InternalScaleFactor * 72.0 * 0.85 / (bbx.ur.y - bbx.ll.y);
  fprintf(stderr, "scalex = %f, scaley = %f\n", scalex, scaley);
/*
  scale = (scalex < scaley) ? scalex : scaley;
*/

  /* Loop through any layers not invisible. */
  esctopcell=GDSEscPovString(topcellname);
  if (subcells>1) {
    fprintf(povfileptr,"#declare %s = union {\n",esctopcell);
  } else {
    fprintf(povfileptr,"#declare %s = \n",esctopcell);
  }

    /* Loop through cells */
    for(cellptr = structptr->cells; cellptr != NULL;
        cellptr = cellptr->next)
    {
      switch(cellptr->type)
      {
        case BOUNDARY: /* All drawing is actually done here. */
          if (!(psStyles[i=layer_to_style_no(cellptr->detail.boundary->layerptr)].hidden))       
          BoundaryToPOV(povfileptr, cellptr->detail.boundary,
                       psStyles[i]);
          break;
        case PATH:
          if (!(psStyles[i=layer_to_style_no(cellptr->detail.path->layerptr)].hidden))       
          PathToPOV(povfileptr, cellptr->detail.path, psStyles[i]);
          break;
        case SREF:
           if (is_visible(cellptr->detail.sref->strptr))
              SrefToPOV(povfileptr, cellptr->detail.sref, &Ident, psStyles[i]);
          break;
        case AREF: /* calls sref for each element in the array */
          fprintf(povfileptr,"//Aref:\n");
           if (is_visible(cellptr->detail.aref->strptr))
              ArefToPOV(povfileptr, cellptr->detail.aref, &Ident, psStyles[i]);
          break;
        case TEXT: /* does nothing in POV yet */
          if (!psStyles[i=layer_to_style_no(cellptr->detail.text->layerptr)].hidden)       
            TextToPOV(povfileptr, cellptr->detail.text, &Ident, psStyles[i]);
          break;
        case NODE: /* does nothing? */
          if (!psStyles[i=layer_to_style_no(cellptr->detail.node->layerptr)].hidden)       
            NodeToPOV(povfileptr, cellptr->detail.node, psStyles[i]);
          break;
        case BOX: /* does nothing? */
          if (!psStyles[i=layer_to_style_no(cellptr->detail.box->layerptr)].hidden)                
            BoxToPOV(povfileptr, cellptr->detail.box, psStyles[i]);
          break;
        default:
          break;
      } /* switch */
    } /* cell loop */
  if (subcells>1) {
    fprintf(povfileptr,"}\n");
  } else {
    fprintf(povfileptr,"\n");
  }
  
  FREE(esctopcell);
}



struct callorder {
 char *name;
 struct callorder *next;
} *callorderfirst=NULL;



/* Call with topcell, will descend heirarchy and create a call order */

void POVStructsInOrder(GDSlibrary *libptr,char *cellname)
{
 GDSstruct *structptr;
 GDScell *fcell;
 struct callorder *cur,*pre;
 structptr = GDSgetStructByName(libptr, cellname);
 fcell=structptr->cells;
 while (fcell!=NULL) {
  if (fcell->type==SREF) {
   POVStructsInOrder(libptr,fcell->detail.sref->refname);
  } 
  if (fcell->type==AREF) {
   POVStructsInOrder(libptr,fcell->detail.aref->refname);
  } 
  fcell=fcell->next;
 } 
 cur=callorderfirst;
 pre=NULL;
 while (cur!=NULL && strcmp(cellname,cur->name)!=0) {
  pre=cur;
  cur=cur->next; 
 }
 if (cur==NULL) { /* Must add */
  cur=(struct callorder *)MALLOC(sizeof(struct callorder));
  cur->next=NULL;
  cur->name=(char *)MALLOC(sizeof(char)*(strlen(cellname)+1));
  strcpy(cur->name,cellname);
  if (pre==NULL) callorderfirst=cur; else pre->next=cur;
 }
}


void
GDStoPOV(GDSlibrary *libptr, char *povfile, char *topcellname)
{
 FILE *povfileptr;
 GDSstruct *structptr;
 struct callorder *cur;
 bbox bbx;
 char *esctopcell,*ts;
 int i;

 povfileptr = fopen(povfile, "w");

 if(povfileptr == NULL)
  {
    fprintf(stderr, "Couldn't open %s for writing. Aborting...\n", povfile);
    exit(1);
  }  

  structptr = GDSgetStructByName(libptr, topcellname);
  if(structptr == NULL)
  {
    fprintf(stderr, "Couldn't find the structure named \"%s\"\n",
            topcellname);
    fclose(povfileptr);
    return;
  }  
  bbx = GDSgetStructBBox(structptr); 
  esctopcell=GDSEscPovString(topcellname);
  fprintf(povfileptr,
    "// POVRAY file generated from gdsreader-0.3.1\n"
    "// Top level cell is %s (or %s if escaping is needed.)\n"
    "// Bounding box is (%d,%d)-(%d,%d)\n"
    "// UserUnit: %e MeterUnit: %e \n"
    "// Since thickness in the config is specified in microns,\n"
    "//   the instance of the final object will be vertically scaled\n"
    "//   by %e at the end of the file.\n\n"
 //   "#include \"colors.inc\"\n"
 //   "#include \"shapes.inc\"\n"
 //   "#include \"finish.inc\"\n"
 //   "#include \"glass.inc\"\n"
    "camera {\n"
    " location <%f,%f,%f>\n"
    " look_at <%f,0,%f>\n"
    "}\n"
    "global_settings { ambient_light rgb <4.5,4.5,4.5> }\n"
    "light_source { <%f,%f,%f> color rgb <1.0, 1.0, 1.0> }\n\n",
     topcellname,esctopcell,
     /* bbox of whole thing. */ bbx.ur.x,bbx.ur.y,bbx.ll.x,bbx.ll.y,
     /* This is the units specified in the file: */ libptr->userunit,libptr->meterunit,
     /* Scale vertically by: ----> */ 1.0e-6/libptr->meterunit,
     /* Camera at a corner, height is x+y */
     (double)bbx.ll.x, (double)abs(bbx.ur.x-bbx.ll.x)+abs(bbx.ur.y-bbx.ll.y)   ,(double)bbx.ll.y,
     /* Look at center of bounding box */
     (bbx.ur.x+bbx.ll.x)/2.0,(bbx.ur.y+bbx.ll.y)/2.0,
     /* Light source diagonally from camera, height same as camera */
     (double)bbx.ur.x, (double) abs(bbx.ur.x-bbx.ll.x)+abs(bbx.ur.y-bbx.ll.y)   ,(double)bbx.ll.y
    );
/*  fprintf(stderr,"Wrote POV Header.\n");
  fflush(stderr); */
  /* Time to go through the "how many layers" PSStyles and dump them */
  for(i=0;i<howmanylayers;i++) if (!(psStyles[i].hidden)) {
     /* Print out a layer description. */
     fprintf(povfileptr,
       "#declare %s = texture { // Layer name: %s \n"
       "     pigment {\n"
       "        color rgb <%f,%f,%f>\n"
       "     }\n"
       "  }\n",
       ts=GDSpsStyleNoToTEXName(i),
       psStyles[i].layername,
       psStyles[i].edgecolor.r,
       psStyles[i].edgecolor.g,
       psStyles[i].edgecolor.b );
      FREE(ts);
     }
  /* I need to find the cells, in the right order, and then dump them. */
  POVStructsInOrder(libptr,topcellname);
  cur=callorderfirst;
  while (cur!=NULL) {
   GDStoPOVCell(libptr, povfileptr, cur->name);
   cur=cur->next;
  }
  fprintf(povfileptr, "object { %s scale <1.0,%f,1.0> }\n// end of povray file\n",esctopcell,1.0e-6/libptr->meterunit);
  FREE(esctopcell);

  /* x,z x,z */



 fclose(povfileptr);
}
