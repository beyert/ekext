/*
 *
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

static t_class *maskxor_class;

typedef struct _maskxor
{
  t_object x_obj;
  t_atom *maskl, *maskr, *maskxor;
  int lengthl, lengthr, lengthm;
  t_float suml, sumr;
  t_float f_in, yes;
  t_float mode;
  t_outlet *thru, *bool, *maskout;
} t_maskxor;

void maskxor_float(t_maskxor *x, t_floatarg fin)
{
  int input = (int)fin;
  if (input >= 0 && input < x->lengthm) {
    if(x->mode == 0)
      {
	x->f_in = fin;
	x->yes = atom_getfloatarg(input, x->lengthm, x->maskxor);
	outlet_float(x->bool, x->yes);
	if(x->yes != 0)
	  {
	    outlet_float(x->thru, x->f_in);
	  }
      }
    else if(x->sumr>x->suml)
      {
	x->f_in = fin;
	x->yes = atom_getfloatarg(input, x->lengthm, x->maskxor);
	outlet_float(x->bool, x->yes);
	if(x->yes != 0)
	  {
	    outlet_float(x->thru, x->f_in);
	  }
      }
  }
}

void maskxor_bang(t_maskxor *x, t_symbol *s)
{
  outlet_list(x->maskout, &s_list, x->lengthm, x->maskxor);
  outlet_float(x->bool, x->yes);
  if(x->yes != 0)
    {
      outlet_float(x->thru, x->f_in);
    }
}

void maskxor_listl(t_maskxor *x, t_symbol *s, int argc, t_atom *argv)
{
  int i;
  x->suml = 0;
  float listr_element, listl_element, xor_element;
  if (x->maskl) {
    freebytes(x->maskl, x->lengthl * sizeof(t_atom));
    x->maskl = 0;
    x->lengthl = 0;
  }

  x->maskl = copybytes(argv, argc * sizeof(t_atom));
  x->lengthl = argc;

  if (x->maskxor) {
    freebytes(x->maskxor, x->lengthm * sizeof(t_atom));
    x->maskxor = 0;
    x->lengthm = 0;
  }
  x->lengthm = x->lengthr > x->lengthl ? x->lengthr : x->lengthl;
  x->maskxor = getbytes(x->lengthm * sizeof(t_atom));

  for(i=0;i<x->lengthm;i++)
    {
      if(i>=x->lengthm)
	{
	  listl_element = 0;
	}
      else
	{
	  listl_element = atom_getfloat(argv+i);
	}
      if(listl_element != 0) 
	{
	  x->suml++;
	}
      if(i>=x->lengthr) 
	{
	  listr_element = 0;
	}
      else 
	{
	  listr_element = atom_getfloatarg(i,x->lengthr,x->maskr);
	}
      xor_element = (float)((int)listl_element ^ (int)listr_element);
      SETFLOAT(&x->maskxor[i], xor_element);
    }
  outlet_list(x->maskout, &s_list, x->lengthm, x->maskxor);
}

void maskxor_listr(t_maskxor *x, t_symbol *s, int argc, t_atom *argv)
{
  int i;
  x->sumr = 0;

  if (x->maskr) {
    freebytes(x->maskr, x->lengthr * sizeof(t_atom));
    x->maskr = 0;
    x->lengthr = 0;
  }

  x->maskr = copybytes(argv, argc * sizeof(t_atom));
  x->lengthr = argc;


  float listr_element;
  for(i=0;i<argc;i++) {
    listr_element = atom_getfloat(argv+i);
    if(listr_element != 0)
      {
	x->sumr++;
      }
  }
}

void maskxor_mode(t_maskxor *x, t_floatarg fmode)
{
  x->mode = fmode;
}

void maskxor_clear(t_maskxor *x)
{
  int i;
  if (x->maskl) {
    freebytes(x->maskl, x->lengthl * sizeof(t_atom));
    x->maskl = 0;
    x->lengthl = 0;
  }
  if (x->maskr) {
    freebytes(x->maskr, x->lengthr * sizeof(t_atom));
    x->maskr = 0;
    x->lengthr = 0;
  }
  if (x->maskxor) {
    freebytes(x->maskxor, x->lengthm * sizeof(t_atom));
    x->maskxor = 0;
    x->lengthm = 0;
  }

  x->yes = x->f_in = 0;
}

void maskxor_print(t_maskxor *x)
{
  int i;
  float element;
  if (x->maskl) {
    for(i=0;i<x->lengthl;i++) {
      element = atom_getfloatarg(i, x->lengthl, x->maskl);
      post("maskl element %d = %f",i,element);
    }
  }
  if (x->maskr) {
    for(i=0;i<x->lengthr;i++) {
      element = atom_getfloatarg(i, x->lengthr, x->maskr);
      post("maskr element %d = %f",i,element);
    }
  }
  if (x->maskxor) {
    for(i=0;i<x->lengthm;i++) {
      element = atom_getfloatarg(i, x->lengthm, x->maskxor);
      post("maskxor element %d = %f",i,element);
    }
  }
  post("mode = %f, lengthl = %d, lengthr = %d, lengthm = %d",x->mode,x->lengthl,x->lengthr,x->lengthm);
}

void *maskxor_new(t_symbol *s, t_floatarg fmode)
{
  int i;
  t_maskxor *x = (t_maskxor *)pd_new(maskxor_class);
  x->mode = fmode != 0 ? 1 : 0;

  x->maskl = 0;
  x->lengthl = 0;
  x->maskr = 0;
  x->lengthr = 0;
  x->maskxor = 0;
  x->lengthm = 0;

  x->thru = outlet_new(&x->x_obj, &s_float);
  x->bool = outlet_new(&x->x_obj, &s_float);
  x->maskout = outlet_new(&x->x_obj, &s_list);
  return (void *)x;
}

void maskxor_setup(void) 
{
  maskxor_class = class_new(gensym("maskxor"),
  (t_newmethod)maskxor_new,
  0, sizeof(t_maskxor),
  CLASS_DEFAULT, A_DEFFLOAT, 0);
  post("|..-.--.-..-maskxor.-...--.-..|");
  post("|    exclusive-or mask-map    |");
  post("|.--.- Edward Kelly 2006 ---.-|");

  class_addfloat(maskxor_class, maskxor_float);
  class_addmethod(maskxor_class, (t_method)maskxor_listl, gensym("listl"), A_GIMME, 0, 0);
  class_addmethod(maskxor_class, (t_method)maskxor_listr, gensym("listr"), A_GIMME, 0, 0);
  class_addbang(maskxor_class, (t_method)maskxor_bang);
  class_addmethod(maskxor_class, (t_method)maskxor_mode, gensym("mode"), A_DEFFLOAT, 0);
  class_addmethod(maskxor_class, (t_method)maskxor_clear, gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(maskxor_class, (t_method)maskxor_print, gensym("print"), A_DEFFLOAT, 0);
}
