/*
 * Histogram for integers
 * Copyright (c) 2011, Dr Edward Kelly <morph_2016@yahoo.co.uk>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * 
 * provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "m_pd.h"

#define MAXENTRIES 13823
#define LASTENTRY 13822

static t_class *ihisto_class;

/* mode = 0: most recent when equal, 1: highest, 2: lowest */

typedef struct _hist
{
  t_atom histogram[MAXENTRIES];
} t_hist;

typedef struct _ihisto
{
  t_object x_obj;
  t_hist histog;
  int intinput, size;
  t_float max, maxvalue, count, exclude, mode;
  t_outlet *peak, *counted, *current, *counting, *histo;
} t_ihisto;

void ihisto_float(t_ihisto *x, t_floatarg fin)
{
  x->intinput = fin > 0 ? (int)fin < x->size ? (int)fin : x->size - 1 : 0;
  x->count = atom_getfloatarg(x->intinput, MAXENTRIES, x->histog.histogram);
  x->count++;
  SETFLOAT(&x->histog.histogram[x->intinput],x->count);

  if(x->mode == 0)
    {
      if(x->count >= x->maxvalue && x->intinput != (int)x->exclude)
	{
	  x->max = (float)x->intinput;
	  x->maxvalue = x->count;
	}
    }
  else if (x->mode == 1)
    {
      if(x->count >= x->maxvalue && x->intinput >= (int)x->max && x->intinput != (int)x->exclude || x->count > x->maxvalue && x->intinput != (int)x->exclude)
	{
	  x->max = (float)x->intinput;
	  x->maxvalue = x->count;
	}
    }
  else if (x->mode == 2)
    {
      if(x->count >= x->maxvalue && x->intinput <= (int)x->max && x->intinput != (int)x->exclude || x->count > x->maxvalue && x->intinput != (int)x->exclude)
	{
	  x->max = (float)x->intinput;
	  x->maxvalue = x->count;
	}
    }

  outlet_float(x->current, x->max);
  outlet_float(x->counting, x->maxvalue);
}

void ihisto_bang(t_ihisto *x)
{
  if(x->max)
    {
      outlet_float(x->counted, x->maxvalue);
      outlet_float(x->peak, x->max);
    }

  int i;
  for(i=0;i<x->size;i++)
    {
      SETFLOAT(x->histog.histogram+i, 0);
    }

  x->max = -1;
  x->maxvalue = 0;
  x->count = 0;
  x->intinput = -1;
  x->exclude = -1;
}

void ihisto_poll(t_ihisto *x)
{
  if(x->max)
    {
      outlet_float(x->counted, x->maxvalue);
      outlet_float(x->peak, x->max);
    }
}

void ihisto_mode(t_ihisto *x, t_floatarg f)
{
  x->mode = f < 0 ? 0 : f > 2 ? 2 : (int)f;
}

void ihisto_get(t_ihisto *x)
{
  outlet_list(x->histo, gensym("list"), x->size, x->histog.histogram);
}

void ihisto_exclude(t_ihisto *x, t_floatarg f)
{
  x->exclude = f;
}

void *ihisto_new(t_floatarg f)
{
  t_ihisto *x = (t_ihisto *)pd_new(ihisto_class);
  int init = (int)f;
  x->size = init > MAXENTRIES ? MAXENTRIES : init <= 0 ? 128 : init;

  x->max = -1;
  x->maxvalue = 0;
  x->count = 0;
  x->intinput = 0;
  x->exclude = -1;

  int i;
  for(i=0;i<MAXENTRIES;i++) 
    {
      SETFLOAT(x->histog.histogram+i, 0);
    }
  x->peak = outlet_new(&x->x_obj, &s_float);
  x->counted = outlet_new(&x->x_obj, &s_float);
  x->current = outlet_new(&x->x_obj, &s_float);
  x->counting = outlet_new(&x->x_obj, &s_float);
  x->histo = outlet_new(&x->x_obj, &s_list);

  return (void *)x;
}

void ihisto_setup(void) 
{
  ihisto_class = class_new(gensym("ihisto"),
  (t_newmethod)ihisto_new,
  0, sizeof(t_ihisto),
  0, A_DEFFLOAT, 0);
  post("Integer Histogram by Ed Kelly 2011");

  class_addfloat(ihisto_class, ihisto_float);
  class_addbang(ihisto_class, ihisto_bang);
  class_addmethod(ihisto_class, (t_method)ihisto_poll, gensym("poll"), A_DEFFLOAT, 0);
  class_addmethod(ihisto_class, (t_method)ihisto_mode, gensym("mode"), A_DEFFLOAT, 0);
  class_addmethod(ihisto_class, (t_method)ihisto_get, gensym("get"), A_GIMME, 0);
  class_addmethod(ihisto_class, (t_method)ihisto_exclude, gensym("exclude"), A_DEFFLOAT, 0);
}
