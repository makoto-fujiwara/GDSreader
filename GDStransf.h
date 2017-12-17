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
#ifndef _GDStransf_h_
#define _GDStransf_h_


extern transform *GDSgetTransf(point ref, double angle,
                               double mag, int mirror); 
extern double GDSGetTransfValues(transform *t,int *mirror, double *mag);
extern point GDStransfPoint(point *pointptr, transform *transfptr);
extern transform *GDStransfTransf(transform *transf1, transform *transf2);
extern transform *GDStranslateTransf(transform *transf, point refpoint);
extern transform *GDSdupTransf(transform *transf);
extern void GDSfreeTransf(transform *transf);
extern point GDSinvtransfPoint(point *pointptr, transform *transfptr);
extern transform * GDSinvertTransf(transform *transf);

extern transform Ident;

#endif /* _GDStransf_h_ */

