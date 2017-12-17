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
#include <math.h>
#include <strings.h>
#include <unistd.h>
#include <GDSstructs.h>
#include <GDSconsts.h>

#include <GDSaux.h>

/*
 * Most of the code has been adapted from the beautiful piece of sw. named Magic
 * Thank you, guys!
 */

transform Ident = { 1.0, 0.0, 0, 0.0, 1.0, 0 };


double
GDSgetDeterminant(transform *transf)
{
  return (transf->a * transf->e - transf->b * transf->d);
}

coord
myRound(double value)
{
  if(value < 0.0)
    return (coord)(value - 0.5);
  else
    return (coord)(value + 0.5);
}

transform *
GDSinvertTransf(transform *transf)
{
  transform *invtransf;
  double det;

  det = GDSgetDeterminant(transf);
  if(det == 0.0)
  {
    fprintf(stderr, "Error in GDSinvertTransf(): matrix cannot be inverted\n");
    exit(1);
  }
  
  invtransf = (transform *)MALLOC(sizeof(transform));
  invtransf->a = transf->e / det;
  invtransf->b = - transf->b / det;
  invtransf->c = (transf->b * transf->f - transf->c * transf->e) / det;
  invtransf->d = - transf->d / det;
  invtransf->e = transf->a / det;
  invtransf->f = (transf->d * transf->c - transf->a * transf->f) / det;

  return invtransf;
}


/* We put a point in, and do the backwards transform on it. */
point
GDSinvtransfPoint(point *pointptr, transform *transfptr)
{
  point newpoint;
  double det;
  double a, b;
  
  det = GDSgetDeterminant(transfptr);
  if(det == 0.0)
  {
    fprintf(stderr,
            "Error in GDSinvtransfPoint(): matrix cannot be inverted\n");
    exit(1);
  }
  
  a = (pointptr->x - transfptr->c) / det;
  b = (pointptr->y - transfptr->f) / det;
  newpoint.x = myRound(a * transfptr->e - b * transfptr->b);
  newpoint.y = myRound(b * transfptr->a - a * transfptr->d);
  
  return newpoint;
}

/*
 * Creates a transform based on the GDSII transformation hints.
 * For now, mag is not used.  (Actually, I added this.  (Pete)
 */
transform *
GDSgetTransf(point ref, double angle, double mag, int mirror)
{
  transform *newtransf;

  newtransf = (transform *)MALLOC(sizeof(transform));
  newtransf->a = mag*cos(angle * M_PI / 180.0);
  newtransf->b = -mag*sin(angle * M_PI / 180.0);
  newtransf->c = ref.x;
  newtransf->d = mag*sin(angle * M_PI / 180.0);
  newtransf->e = mag*cos(angle * M_PI / 180.0);
  newtransf->f = ref.y;

/* GDSII understands only the Y mirroring */

/* Reflecting about X means changing *Y* */
  if (mirror) {
    newtransf->b = -newtransf->b;
    newtransf->e = -newtransf->e;
  }
  return newtransf;
}



/* Returns angle in degrees implied by the
     transform matrix. */
double GDSGetTransfValues(transform *t, int *mirror, double *mag)
{
 double retval;

/* 
|nx|    | mx*cos(R) -my*sin(R)||x|      |ox|
|ny| =  | mx*sin(R)  my*cos(R)||y|   +  |oy|

Generally, mx=my=1.  However, if my=-mx, we are flipped.


Not flipped:

ab
de

 --|+-
 +-|++
-------
 -+|++
 --|-+

flipped:

 -+|++
 ++|+-
-------
 --|+-
 -+|--

if b*d is POSITIVE, or a*e is NEGATIVE we are flipped.

*/
 
 *mag=(double)sqrt((t->d) * (t->d) + (t->e)*(t->e));
 /* If b and d have different signs *or* a and e have different signs,
     then we are mirrored.  one might be zero, but not both. */
 if ((t->b)*(t->d)>0.0 || (t->a)*(t->e)<0.0) *mirror=1; else *mirror=0;
 
 /* If mirrored, d and e are wrong in sign. */
 /* We can get angle from a and -b     d*/
 
 /* if a==0, do not take atan. */
 if (fabs(t->a)<1.0e-14) {
  if (t->d<=0) return(-90.0); /* Remember b=-sin(R) */
  return(90.0);
 }
 if (t->a<0.0) 
   retval=atan((t->d)/t->a)+asin(1.0)*2.0;
     else 
   retval=atan((t->d)/t->a);
 return(retval*90.0/asin(1.0));
}



point
GDStransfPoint(point *pointptr, transform *transfptr)
{
  point newpoint;
  
  newpoint.x = myRound(pointptr->x * transfptr->a + pointptr->y * transfptr->b +
                       transfptr->c);
  newpoint.y = myRound(pointptr->x * transfptr->d + pointptr->y * transfptr->e +
                       transfptr->f);
  
  return newpoint;
}

transform *
GDSdupTransf(transform *transf)
{
  transform *newtransf;
  
  newtransf = (transform *)MALLOC(sizeof(transform));
  
  newtransf->a = transf->a;
  newtransf->b = transf->b;
  newtransf->c = transf->c;
  newtransf->d = transf->d;
  newtransf->e = transf->e;
  newtransf->f = transf->f;
  
  return newtransf;
}

void
GDSfreeTransf(transform *transf)
{
  FREE(transf);
}

/*
 * newtransf, if applied, is equivalent to the application of transf1,
 * followed by the application of transf2.
 */
transform *
GDStransfTransf(transform *transf1, transform *transf2)
{
  transform *newtransf;
  
  newtransf = (transform *)MALLOC(sizeof(transform));
  newtransf->a = transf1->a * transf2->a + transf1->d * transf2->b;
  newtransf->b = transf1->b * transf2->a + transf1->e * transf2->b;
  newtransf->c = transf1->c * transf2->a + transf1->f * transf2->b + transf2->c;
  newtransf->d = transf1->a * transf2->d + transf1->d * transf2->e;
  newtransf->e = transf1->b * transf2->d + transf1->e * transf2->e;
  newtransf->f = transf1->c * transf2->d + transf1->f * transf2->e + transf2->f;

  return newtransf;
}


transform *
GDStranslateTransf(transform *transf, point refpoint)
{
  transform *newtransf;
  
  newtransf = (transform *)MALLOC(sizeof(transform));
  newtransf->a = transf->a;
  newtransf->b = transf->b;
  newtransf->d = transf->d;
  newtransf->e = transf->e;
  newtransf->c = transf->c + refpoint.x;
  newtransf->f = transf->f + refpoint.y;
  
  return newtransf;
}
