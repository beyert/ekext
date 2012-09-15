/*
 * delta and delta-of-delta values.
 * Copyright (c) 2012, Dr Edward Kelly <morph_2016@yahoo.co.uk>
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

t_class *doubledelta_class;

typedef struct _doubledelta
{
  t_object x_obj;
  t_float f_now, f_prev, f_delta, f_delta_prev, f_doubledelta, fa;
  t_outlet *delta, *doubledelta;
} t_doubledelta;

void doubledelta_float(t_doubledelta *y, t_floatarg f)
{
  y->f_delta_prev = y->f_delta;
  y->f_prev = y->f_now;
  y->f_now = f;
  y->f_delta = y->f_now - y->f_prev;
  y->f_doubledelta = y->f_delta - y->f_delta_prev;
  outlet_float(y->doubledelta, y->f_doubledelta);
  outlet_float(y->delta, y->f_delta);
}

void doubledelta_bang(t_doubledelta *y)
{
  outlet_float(y->doubledelta, y->f_doubledelta);
  outlet_float(y->delta, y->f_delta);
}

void *doubledelta_new(t_floatarg f)
{
  t_doubledelta *y = (t_doubledelta *)pd_new(doubledelta_class);
  y->fa = f;
  y->delta = outlet_new(&y->x_obj, gensym("float"));
  y->doubledelta = outlet_new(&y->x_obj, gensym("float"));
  return(void *)y;
}

void doubledelta_setup(void) 
{
  doubledelta_class = class_new(gensym("doubledelta"),
  (t_newmethod)doubledelta_new,
  0, sizeof(t_doubledelta),
  0, A_DEFFLOAT, 0);
  post("delta & delta-delta values, <morph_2016@yahoo.co.uk>");

  class_addbang(doubledelta_class, doubledelta_bang);
  class_addfloat(doubledelta_class, doubledelta_float);
}
