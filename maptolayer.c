
/*

  Reads in a gds2cds.map file and produces a first cut layers.config 
    file.

gds2cds.map file format:

 layername layerfeaturename layernumber layerfeaturenumber

Optional whitespace everywhere

By default, I am now making all layers hidden.


-----------------


*/

#include <stdio.h>
#include <stdlib.h>






int convfile(FILE *infile, FILE *outfile)
{
 char lna[100],lfna[100];
 int layn,layfn,rc,i,red,green,blue;
 int layermap[256];
 for(i=0;i<256;i++) layermap[i]=0;
 while (!feof(infile)) {
  if ((rc=fscanf(infile," %s %s %d %d ",lna,lfna,&layn,&layfn))!=4) {
   fprintf(stderr,"ERROR: incomplete read.");
   exit(1);
  }
  if (layermap[layn]==0) {
    /* Assume a 64 color space, 4^3, so 4 reds 4 blues 4 greens */
    /* Reserve black and white.  That gives 1/5th */
    red = layn / 16 ; /* 0..3 */
    green = (layn - ( red * 16 )) / 4 ; /* 0..3 */
    blue =  layn - red*16 - green*4 ;
    fprintf(outfile,
         "newlayer\n"
         " gdsno %d\n"
         " hidden %s\n"
         " name %s\n"
         " red %1.1f\n"
         " green %2.1f\n"
         " blue %.1f\n",layn,"yes",lna,0.2+0.2*(float)red ,
                0.2+0.2*(float)green,0.2+0.2*(float)blue);
    fprintf(outfile,     
         " fill %s\n"
/* ,"no"); */

         " hatch %s\n"
         " angle %.1f\n"
         " step %.1f\n"
	 " thickness %f\n"
	 " depth %f\n"                                
         ,"no","cross",45.0,3.0,0.1,0.1*(float)layn);

    fprintf(outfile,"endlayer\n\n");

    layermap[layn]=1;
   }
 }



 return(0);
}








int main(int argc,char *argv[])
{

 
 convfile(stdin,stdout);




















 return(0);
}
