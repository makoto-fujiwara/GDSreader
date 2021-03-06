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
#ifndef _GDSstruct_h_
#define _GDSstruct_h_

extern bbox GDSgetStructBBox(GDSstruct *structptr);
extern void StructToPS(FILE *psfile, GDSstruct *structptr);
extern void StructToXgraph(FILE *xgraphfile, GDSstruct *structptr);
extern void GDSfreeStruct(GDScell *cell);
extern GDSstruct *GDSdupStruct(GDSstruct *structptr);
extern GDSstruct *GDSsrefToStruct(srefEl *sref);
extern GDSstruct *GDSreadStruct(int gdsfildes, GDSlibrary *libptr);


#endif /* _GDSstruct_h_ */

