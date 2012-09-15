/*  list_sum - total sum of list values, with setting of each element independently
 * Copyright (c) 2007, Dr Edward Kelly <morph_2016@yahoo.co.uk>
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

static t_class *list_sum_class;

typedef struct _contents
{
  t_atom list[1024];
} t_contents;

typedef struct _list_sum
{
  t_object x_obj;
  t_contents contents;
  t_float total, highest, maxlen, wrap, reset, accum, remainder, firsttime;
  t_outlet *sum, *length; /*, *remains, *diff;*/
} t_list_sum;

void list_sum_all(t_list_sum *x, t_symbol *s, int argc, t_atom *argv)
{
  argc = argc < 1024 ? argc : 1024;
  int i, j, maxindex;
  float current;
  x->total = 0;
  maxindex = argc > x->maxlen ? x->maxlen-1 : argc-1;
  for(i=0;i<argc;i++)
    {
      current = atom_getfloat(argv+i);
      SETFLOAT(&x->contents.list[i], current);
      if(i<=maxindex)
	{
	  x->total += current;
	}
      else if (i > maxindex && x->wrap > x->maxlen && i < x->wrap)
	{
	  j = i % (int)x->maxlen;
	  current = atom_getfloat(argv+j);
	  x->total += current;
	}
    }
  x->highest = (float)argc-1;
  outlet_float(x->length, (float)argc);
  outlet_float(x->sum, x->total);
}

void list_sum_set(t_list_sum *x, t_floatarg element, t_floatarg value)
{
  int i, j, indx, top;
  float current;
  if(element < 1024)
    {
      x->total = 0;
      float f_indx = element-1;
      indx = (int)f_indx;
      x->highest = f_indx > x->highest ? f_indx : x->highest;
      if(x->wrap <= x->maxlen)
	{
	  top = x->highest >= x->maxlen ? x->maxlen : x->highest + 1;
	}
      else
	{
	  top = x->highest >= x->wrap ? x->wrap : x->highest + 1;
	}
      SETFLOAT(&x->contents.list[indx], value);
      for(i=0;i<top;i++)
	{
	  if(i<x->maxlen)
	    {
	      current = atom_getfloatarg(i, 1024, x->contents.list);
	      x->total += current;
	    }
	  else
	    {
	      j = i % (int)x->maxlen;
	      current = atom_getfloatarg(j, 1024, x->contents.list);
	      x->total += current;
	    }
	}
      outlet_float(x->length, x->highest+1);
      outlet_float(x->sum, x->total);
    }
}

//next: list_sum_follow - clock based?
//void list_sum_follow(t_list_sum *x, t_symbol *s, t_floatarg index, t_floatarg value)
//{
//  int i = (int) index;
//  if(index == (int)x->reset || x->firsttime == 1)
//    {
//      x->accum = 0;
//      x->remainder = x->total;
//      x->firsttime = 0;
//    }
//  float current = atom_getfloatarg(i, 1024, x->contents.list);
//  float diff = value - current;
//  x->accum += diff;
//  outlet_float(x->diff, x->accum);
//  x->remainder -=current;
//  outlet_float(x->remains, x->remainder);
//}


void list_sum_clear(t_list_sum *x)
{
  int i;
  for(i=0;i<1024;i++)
    {
    SETFLOAT(&x->contents.list[i], 0);
    }
  x->highest=0;
  x->total=0;
}

void list_sum_print(t_list_sum *x)
{
  int i;
  float element;
  for(i=0;i<=x->highest;i++)
    {
      element = atom_getfloatarg(i, 1024, x->contents.list);
      post("%d ", element);
    }
}

void *list_sum_new(t_symbol *s, int argc, t_atom *argv)
{
  int i;
  t_list_sum *x = (t_list_sum *)pd_new(list_sum_class);
  x->total = 0;
  x->highest = 0;
  x->maxlen = 1024;
  x->wrap = 1024;
  x->reset = 16;
  x->firsttime = 1;
  for(i=0;i<1024;i++)
    {
      SETFLOAT(&x->contents.list[i], 0);
    }
  floatinlet_new(&x->x_obj, &x->maxlen);
  floatinlet_new(&x->x_obj, &x->wrap);
  //  floatinlet_new(&x->x_obj, &x->reset);
  x->sum = outlet_new(&x->x_obj, &s_float);
  x->length = outlet_new(&x->x_obj, &s_float);
  //  x->remains = outlet_new(&x->x_obj, &s_float);
  //  x->diff = outlet_new(&x->x_obj, &s_float);
  return (void *)x;
}

void list_sum_setup(void) {
  list_sum_class = class_new(gensym("list_sum"),
  (t_newmethod)list_sum_new,
  0, sizeof(t_list_sum),
  0, A_DEFFLOAT, 0);
  post("|<<<<<<<<<<<<<<<<<<<<list_sum>>>>>>>>>>>>>>>>>>>>|");
  post("|<<<calculate the sum of a list, with wrapping>>>|");
  post("|<<<<<<<<<edward-------kelly-------2007>>>>>>>>>>|");

  //  class_addmethod(list_sum_class, (t_method)list_sum_follow, gensym("follow"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(list_sum_class, (t_method)list_sum_all, gensym("all"), A_GIMME, 0);
  class_addmethod(list_sum_class, (t_method)list_sum_clear, gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(list_sum_class, (t_method)list_sum_print, gensym("print"), A_DEFFLOAT, 0);
  class_addmethod(list_sum_class, (t_method)list_sum_set, gensym("set"), A_DEFFLOAT, A_DEFFLOAT, 0);
}
