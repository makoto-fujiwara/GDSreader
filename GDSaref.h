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
#ifndef _GDSaref_h_
#define _GDSaref_h_

extern bbox GDSgetArefBBox(arefEl *aref, transform *transf);
extern void ArefToPS(FILE *psfile, arefEl *aref,
                     transform *transf, PSStyle psStyle);
extern void ArefToPOV(FILE *povfile, arefEl *aref,
                     transform *transf, PSStyle psStyle);
extern void ArefToHPGL(FILE *hpglfile, arefEl *aref,
                     transform *transf, PSStyle psStyle);
extern GDScell *GDSreadAref(int gdsfildes, GDSstruct *structptr);

#endif /* _GDSaref_h_ */

