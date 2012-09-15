/*
 *  hssc~ : Highest Significant Spectral Component, according to amplitude ratio to
 *  Strongest Significant Spectral Component. 
 * Copyright (c) 2005, Dr Edward Kelly <morph_2016@yahoo.co.uk>
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
#include <math.h>

static t_class *hssc_tilde_class;

typedef struct _hssc_tilde 
{
  t_object x_obj;
  t_float f;
  t_float f_maxbin, f_minbin, f_ratio;
  t_outlet *f_hssc, *f_sssc;
} t_hssc_tilde;

t_int *hssc_tilde_perform(t_int *w)
{
  t_hssc_tilde *x = (t_hssc_tilde *)(w[1]);
  t_sample  *real =     (t_sample *)(w[2]);
  t_sample  *imag =     (t_sample *)(w[3]);
  int           n =            (int)(w[4]);
  int incr = 0;
  double vectorr, vectori;
  double max = 0;
  double alpha;
  x->f_maxbin = x->f_minbin = 0;
  x->f_ratio = x->f_ratio > 0 ? x->f_ratio : 100;

  while (n--)
    {
      vectorr = (*real++);
	  vectori = (*imag++);
	  alpha = sqrt((vectorr * vectorr) + (vectori * vectori));
      x->f_maxbin = alpha > max ? incr : x->f_maxbin;
      max = alpha > max ? alpha : max;
      x->f_minbin = alpha > (max / x->f_ratio) ? incr : x->f_minbin;
      incr++;
    }
  outlet_float(x->f_sssc, x->f_maxbin);	
  outlet_float(x->f_hssc, x->f_minbin);
  
  return(w+5);
}

void hssc_tilde_dsp(t_hssc_tilde *x, t_signal **sp)
{
  dsp_add(hssc_tilde_perform, 4, x,
	  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void *hssc_tilde_new(t_floatarg f)
{
  t_hssc_tilde *x = (t_hssc_tilde *)pd_new(hssc_tilde_class);

  x->f_ratio = f;

  inlet_new (&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  floatinlet_new (&x->x_obj, &x->f_ratio);
  x->f_hssc = outlet_new(&x->x_obj, gensym("float"));
  x->f_sssc = outlet_new(&x->x_obj, gensym("float"));

  return (void *)x;
}


void hssc_tilde_setup(void)
{
  hssc_tilde_class = class_new(gensym("hssc~"),
				     (t_newmethod)hssc_tilde_new,
				     0, sizeof(t_hssc_tilde),
				     CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("|=================hssc~==================|");
  post("|=highest significant spectral component=|");
  post("|======edward=======kelly=======2005=====|");

  class_addmethod(hssc_tilde_class, (t_method)hssc_tilde_dsp,
		  gensym("dsp"), 0);

  CLASS_MAINSIGNALIN(hssc_tilde_class, t_hssc_tilde, f);
}
