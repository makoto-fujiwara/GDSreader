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
extern char * MALLOC(int size);
extern void FREE(void *ptr);

extern double GDSreadReal8(unsigned char* record);
extern int GDSreadInt2(unsigned char* record);
extern int GDSreadInt4(unsigned char* record);
extern char * GDSreadString(unsigned char* record, int len);
extern char * GDSEscPovString(char *s);
extern int GDSrecordType(unsigned char record_type);
extern void GDSaddDates(GDSlibrary* libptr, unsigned char* record);
extern int GDSreadRecord(int gdsfildes, unsigned char **record, int *nbytes);
extern void GDSunreadRecord(int gdsfildes, int nbytes);
extern char *GDSpsStyleNoToTEXName(int i);
extern char *GDSLayerToTEXName(layer *layerptr);
