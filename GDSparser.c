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
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <GDSstructs.h>
#include <GDSconsts.h>
#include <GDSglobals.h>


/* Read in the layers.config file.  If there is no datatype
  specified in a record, then we should use that layer for all
  unspecified datatypes?  I added a dummy entry at position
  zero which is hidden.  
*/

int
GDSinitPSStyles(FILE *cfgfile)
{
  char seps[] = {" \t\n"};
  char line[1024], buf[1024], *token;
  int counter = 0, lineno = 0, firsttime;
  color edgecolor;
  
            fprintf(stdout, "dummy hidden layer body\n");
            edgecolor.r = edgecolor.g = edgecolor.b = 0.0;
            psStyles[counter].edgecolor = edgecolor;
            psStyles[counter].gdsno = -1;
            psStyles[counter].datatype = -3;
            psStyles[counter].dataspec = 0;
            psStyles[counter].layername = strdup("NULL");
            psStyles[counter].hidden = 1;
            psStyles[counter].fill = 0;
            psStyles[counter].hatch = 0;
            psStyles[counter].angle = 0.0;  /* arbitrary numbers since the */
            psStyles[counter].step = 100.0; /* hatch is 0 */
            psStyles[counter].depth = 0.0;  
            psStyles[counter].thickness = 0.0;  
  counter++;
  
  while(!feof(cfgfile))
  {
    if(fgets(line, 1024, cfgfile) == NULL)
      break;
    lineno++;
    firsttime = 1;
    if(line[0] == '#')
    /* comment line */
      continue;
    do
    {
      if(firsttime)
      {
        firsttime = 0;
        token = strtok(line, seps);
      }
      else
        token = strtok(NULL, seps);
      if(token == NULL)
        continue;
      switch(token[0])
      {
	case 'd':
          if(!strcmp(token, "depth"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              psStyles[counter].depth = atof(token);
              if(errno == EINVAL)
              {
                fprintf(stderr,
                        "Syntax error at line %d: bad number syntax %s\n",
                        lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing number\n",
                      lineno);
              exit(1);
            }
          }
          else {
          
           if(!strcmp(token, "datatype"))
        	  {
        	    token = strtok(NULL, seps);
        	    if(token != NULL)
        	    {
        	      psStyles[counter].datatype = atoi(token);
        	      psStyles[counter].dataspec = 1;
        	      if(errno == EINVAL)
        	      {
                	fprintf(stderr,
                        	"Syntax error at line %d: bad number syntax %s\n",
                        	lineno, token);
                	exit(1);
        	      }
        	    }
        	    else
        	    {
        	      fprintf(stderr, "Syntax error at line %d: missing number\n",
                	      lineno);
        	      exit(1);
        	    }
        	  }
          
        	  else
        	  {
        	    fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                	    lineno, token);
        	    exit(1);
        	  }
          }
          break;
       case 't':
          if(!strcmp(token, "thickness"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              psStyles[counter].thickness = atof(token);
              if(errno == EINVAL)
              {
                fprintf(stderr,
                        "Syntax error at line %d: bad number syntax %s\n",
                        lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing number\n",
                      lineno);
              exit(1);
            }
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
       case 'e':
          if(!strcmp(token, "endlayer"))
          {
            psStyles[counter].edgecolor = edgecolor;
            if(psStyles[counter].layername == NULL)
            {
              sprintf(buf, "Layer%d", psStyles[counter].gdsno);
              psStyles[counter].layername = strdup(buf);
            }
            fprintf(stdout, "layername = %s, gdsno = %d, type = %d, hidden = %d, red = %.2f, green = %.2f, blue = %.2f,\n"
                            " fill = %d hatch = %d, angle = %.2f, step = %.2f, depth = %.2f, thickness = %.2f\n",
                    psStyles[counter].layername, 
                    psStyles[counter].gdsno,
                    (psStyles[counter].dataspec?psStyles[counter].datatype:-1), 
                    psStyles[counter].hidden,
                    psStyles[counter].edgecolor.r,
                    psStyles[counter].edgecolor.g,
                    psStyles[counter].edgecolor.b,
                    psStyles[counter].fill, psStyles[counter].hatch,
                    psStyles[counter].angle, psStyles[counter].step,
                    psStyles[counter].depth,psStyles[counter].thickness);
            counter++;
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
        case 'n':
          if(!strcmp(token, "newlayer"))
          {
            fprintf(stdout, "new layer body\n");
            edgecolor.r = edgecolor.g = edgecolor.b = 0.0;
            psStyles[counter].edgecolor = edgecolor;
            psStyles[counter].gdsno = 0;
            psStyles[counter].datatype = 0;
            psStyles[counter].dataspec = 0;
            psStyles[counter].layername = NULL;
            psStyles[counter].hidden = 1;
            psStyles[counter].fill = 0;
            psStyles[counter].hatch = 0;
            psStyles[counter].angle = 0.0;  /* arbitrary numbers since the */
            psStyles[counter].step = 100.0; /* hatch is 0 */
            psStyles[counter].depth = 0.0;  
            psStyles[counter].thickness = 0.0;  
          }
          else if(!strcmp(token, "name"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
              psStyles[counter].layername = strdup(token);
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing layer name\n",
                      lineno);
              exit(1);
            }
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
        case 'h':
          if(!strcmp(token, "hatch"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              if(!strcmp(token, "no") || !strcmp(token, "false"))
                psStyles[counter].hatch = 0;
              else if(!strcmp(token, "single"))
                psStyles[counter].hatch = 1;
              else if(!strcmp(token, "cross"))
                psStyles[counter].hatch = 2;
              else
              {
                fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                        lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing token\n",
                      lineno);
              exit(1);
            }
          }
          else if(!strcmp(token, "hidden"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              if(!strcmp(token, "no") || !strcmp(token, "false"))
                psStyles[counter].hidden = 0;
              else if(!strcmp(token, "yes") || !strcmp(token, "true"))
                psStyles[counter].hidden = 1;
              else
              {
                fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                        lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing token\n",
                      lineno);
              exit(1);
            }
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
        case 'r':
          if(!strcmp(token, "red"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              edgecolor.r = atof(token);
              if(errno == EINVAL)
              {
                fprintf(stderr,
                        "Syntax error at line %d: bad number syntax %s\n",
                        lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing number\n",
                      lineno);
              exit(1);
            }
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
        case 'g':
          if(!strcmp(token, "gdsno"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              psStyles[counter].gdsno = atoi(token);
              if(errno == EINVAL)
              {
                fprintf(stderr,
                        "Syntax error at line %d: bad number syntax %s\n",
                        lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing number\n",
                      lineno);
              exit(1);
            }
          }
          else if(!strcmp(token, "green"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              edgecolor.g = atof(token);
              if(errno == EINVAL)
              {
                fprintf(stderr,
                        "Syntax error at line %d: bad number syntax %s\n",
                        lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing number\n",
                      lineno);
              exit(1);
            }
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
        case 'b':
          if(!strcmp(token, "blue"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              edgecolor.b = atof(token);
              if(errno == EINVAL)
              {
                fprintf(stderr,
                        "Syntax error at line %d: bad number syntax %s\n",
                        lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing number\n",
                      lineno);
              exit(1);
            }
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
        case 'f':
          if(!strcmp(token, "fill"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              if(!strcmp(token, "no") || !strcmp(token, "false"))
                psStyles[counter].fill = 0;
              else if(!strcmp(token, "yes") || !strcmp(token, "true"))
                psStyles[counter].fill = 1;
              else
              {
                fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                      lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing token\n",
                      lineno);
              exit(1);
            }
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
        case 'a':
          if(!strcmp(token, "angle"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              psStyles[counter].angle = atof(token);
              if(errno == EINVAL)
              {
                fprintf(stderr,
                        "Syntax error at line %d: bad number syntax %s\n",
                        lineno, token);
                exit(1);
              }
              while(psStyles[counter].angle > 180.0)
                psStyles[counter].angle -= 180.0;
              while(psStyles[counter].angle < 0.0)
                psStyles[counter].angle += 180.0;
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing number\n",
                      lineno);
              exit(1);
            }
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
        case 's':
          if(!strcmp(token, "step"))
          {
            token = strtok(NULL, seps);
            if(token != NULL)
            {
              psStyles[counter].step = atof(token);
              if(errno == EINVAL)
              {
                fprintf(stderr,
                        "Syntax error at line %d: bad number syntax %s\n",
                        lineno, token);
                exit(1);
              }
            }
            else
            {
              fprintf(stderr, "Syntax error at line %d: missing number\n",
                      lineno);
              exit(1);
            }
          }
          else
          {
            fprintf(stderr, "Syntax error at line %d: unknown token %s\n",
                    lineno, token);
            exit(1);
          }
          break;
        default:
          fprintf(stderr, "token = %s\n", token);
          break;
      }
    }
    while(token != NULL);
  }
  psStyles[counter].gdsno = 0; /* Last entry.  Not really used as a flag. */

  return counter; 
}
