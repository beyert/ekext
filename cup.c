/* cup counts up ^_^
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

t_class *cup_class;

typedef struct _cup
{
  t_object x_obj;
  t_int f_count, fa;
  t_outlet *count;
} t_cup;

void cup_float(t_cup *y, t_floatarg f)
{
  y->f_count = f;
}

void cup_bang(t_cup *y)
{
  outlet_float(y->count, y->f_count);
  y->f_count += 1;
}

void cup_setbang(t_cup *y, t_floatarg f)
{
  y->f_count = f;
  outlet_float(y->count, y->f_count);
  y->f_count += 1;
}

void *cup_new(t_floatarg f)
{
  t_cup *y = (t_cup *)pd_new(cup_class);
  y->fa = f;
  y->f_count = 0;
  y->count = outlet_new(&y->x_obj, gensym("float"));
  return(void *)y;
}

void cup_setup(void) 
{
  cup_class = class_new(gensym("cup"),
  (t_newmethod)cup_new,
  0, sizeof(t_cup),
  0, A_DEFFLOAT, 0);
  post("cup counts up ^_^");

  class_addbang(cup_class, cup_bang);
  class_addfloat(cup_class, cup_float);
  class_addmethod(cup_class, (t_method)cup_setbang, gensym("setbang"), A_DEFFLOAT, 0);}
