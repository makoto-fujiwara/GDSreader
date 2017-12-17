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
#ifndef _GDStext_h_
#define _GDStext_h_

extern bbox GDSgetTextBBox(textEl *text);
extern void TextToPS(FILE *psfile, textEl *text,
                     transform *transf, PSStyle psStyle);
extern void TextToPOV(FILE *povfile, textEl *text,
                     transform *transf, PSStyle psStyle);
extern void TextToHPGL(FILE *hpglfile, textEl *text,
                     transform *transf, PSStyle psStyle);
extern void GDSfreeText(GDScell *cell);
extern GDScell *GDSdupText(textEl *text);
extern void GDStransfText(textEl *text, transform *transf);
extern GDScell *GDSreadText(int gdsfildes, GDSstruct *structptr);

#endif /* _GDStext_h_ */

