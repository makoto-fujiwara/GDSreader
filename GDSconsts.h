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
#define DEFAULT_CONFIG_FILE ".layers.config"
#define DEFAULT_HPGL_FILE "result.plt"
#define DEFAULT_PS_FILE "result.ps"
#define DEFAULT_POV_FILE "result.pov"


#define InternalScaleFactor 100
#define BIGVAL 2147483647
#define CURRENTMAG 1.0
#define CURRENTANGLE 0.0

#define BUTT_END 0
#define ROUND_END 1
#define EXTENDED_END 2
#define CUSTOMPLUS_END 4

#define TOP 0
#define MIDDLE 1
#define BOTTOM 2
#define LEFT 0
#define CENTER 1
#define RIGHT 2

#define NOFILL 0
#define FILL 1
#define HATCH 2
#define CROSSHATCH 3

#define HEADER 0
#define BGNLIB 1
#define LIBNAME 2
#define UNITS 3
#define ENDLIB 4
#define BGNSTR 5
#define STRNAME 6
#define ENDSTR 7
#define BOUNDARY 8
#define PATH 9
#define SREF 10
#define AREF 11
#define TEXT 12
#define LAYER 13
#define DATATYPE 14
#define WIDTH 15
#define XY 16
#define ENDEL 17
#define SNAME 18
#define COLROW 19
#define TEXTNODE 20
#define NODE 21
#define TEXTTYPE 22
#define PRESENTATION 23
#define SPACING 24
#define STRING 25
#define STRANS 26
#define MAG 27
#define ANGLE 28
#define UINTEGER 29
#define USTRING 30
#define REFLIBS 31
#define FONTS 32
#define PATHTYPE 33
#define GENERATIONS 34
#define ATTRTABLE 35
#define STYPTABLE 36
#define STRTYPE 37
#define ELFLAGS 38
#define ELKEY 39
#define LINKTYPE 40
#define LINKKEYS 41
#define NODETYPE 42
#define PROPATTR 43
#define PROPVALUE 44
#define BOX 45
#define BOXTYPE 46
#define PLEX 47
#define BGNEXTN 48
#define ENDTEXTN 49
#define TAPENUM 50
#define TAPECODE 51
#define STRCLASS 52
#define RESERVED 53
#define FORMAT 54
#define MASK 55
#define ENDMASKS 56
#define LIBDIRSIZE 57
#define SRFNAME 58
#define LIBSECUR 59
#define UNKNOWN 60
