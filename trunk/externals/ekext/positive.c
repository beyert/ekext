/*
 * positive - wraps floats back into positive numbers from negative
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

t_class *positive_class;

typedef struct _positive
{
  t_object x_obj;
  t_float value;
  t_outlet *positive;
} t_positive;

void positive_float(t_positive *y, t_floatarg f)
{
	y->value = f;
	if(y->value < 0)
	{
		int i = (int)y->value;
		i = i * -1 + 1;
		y->value += (float)i;
	}
	outlet_float(y->positive, y->value);
}

void positive_bang(t_positive *y)
{
    outlet_float(y->positive, y->value);
}

void *positive_new(t_floatarg f)
{
  t_positive *y = (t_positive *)pd_new(positive_class);
  y->value = f;
  y->positive = outlet_new(&y->x_obj, gensym("float"));
  return(void *)y;
}

void positive_setup(void) 
{
  positive_class = class_new(gensym("positive"),
  (t_newmethod)positive_new,
  0, sizeof(t_positive),
			       0, A_DEFFLOAT, 0);
  post("positive always returns a positive value");
  post("negative values are shifted up");

  class_addbang(positive_class, positive_bang);
  class_addfloat(positive_class, positive_float);
}
