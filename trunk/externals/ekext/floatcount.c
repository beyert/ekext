/*
 * count and wrap floating-point numbers
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
#include <math.h>

t_class *floatcount_class;

typedef struct _floatcount
{
  t_object x_obj;
  t_float floatcount, prev, recount, remainder, step, sign, low, high, range;
  t_int f_int, init, signchange, remfloat, newfloat;
  t_outlet *count, *icount, *remain, *diff, *isign, *wrapped;
} t_floatcount;

void floatcount_float(t_floatcount *y, t_floatarg f)
{
  outlet_float(y->diff, f - y->floatcount);
  y->init = 0;
  y->floatcount = f;
}

void floatcount_reset(t_floatcount *y)
{
  y->floatcount = y->low;
  y->prev = y->low;
  y->init = 1;
  y->signchange = 0;
}

void floatcount_bang(t_floatcount *y)
{
  if(y->init)
    {
      y->floatcount = y->low;
      y->init = 0;
    }
  if (y->signchange == 0)
    {
      if (y->floatcount < y->low)
	{
	  y->floatcount = y->floatcount + y->high - y->low;
	  outlet_bang(y->wrapped);
	}
      else if (y->floatcount >= y->high)
	{
	  y->floatcount = y->floatcount - y->high + y->low;
	  outlet_bang(y->wrapped);
	}
      y->f_int = (int)y->floatcount;
      y->remainder = y->high - y->floatcount;
      outlet_float(y->isign, y->sign);
      outlet_float(y->diff, 0);
      outlet_float(y->remain, y->remainder);
      outlet_float(y->icount, (float)y->f_int);
      outlet_float(y->count, y->floatcount);
      y->prev = y->floatcount;
      y->floatcount += y->step;
    }
  else if (y->signchange == 1)
    {
      y->floatcount = y->prev + y->step;
      if (y->floatcount < y->low)
	{
	  y->floatcount = y->floatcount + y->high - y->low;
	  outlet_bang(y->wrapped);
	}
      else if (y->floatcount >= y->high)
	{
	  y->floatcount = y->floatcount - y->high + y->low;
	  outlet_bang(y->wrapped);
	}
      y->f_int = (int)y->floatcount;
      y->remainder = y->high - y->floatcount;
      outlet_float(y->isign, y->sign);
      outlet_float(y->diff, 0);
      outlet_float(y->remain, y->remainder);
      outlet_float(y->icount, (float)y->f_int);
      outlet_float(y->count, y->floatcount);
      y->signchange = 0;
      y->prev = y->floatcount;
      y->floatcount += y->step;
    }
}

void floatcount_step(t_floatcount *y, t_floatarg f)
{
  y->step = f != 0 ? f : 1;
  float oldsign = y->sign;
  y->sign = y->step > 0 ? 1 : -1;
  if(y->sign != oldsign)
    {
      y->signchange = 1;
    }
  while(y->range != 0 && fabs(y->step) > y->range)
    {
      y->step = (fabs(y->step) - y->range)* y->sign;
    }
  //y->floatcount = y->prev + y->step;
}

void floatcount_high(t_floatcount *y, t_floatarg f)
{
  float flow = y->low;
  float fhigh = f;
  if (flow > fhigh) {
    y->high = flow;
    y->low  = fhigh;
  }
  else {
    y->high = fhigh;
    y->low = flow;
  }
  y->range = y->high - y->low;
  while(y->range != 0 && fabs(y->step) > y->range)
    {
      y->step = (fabs(y->step) - y->range)* y->sign;
    }
}

void floatcount_low(t_floatcount *y, t_floatarg f)
{
  float flow = f;
  float fhigh = y->high;
  if (flow > fhigh) {
    y->high = flow;
    y->low  = fhigh;
  }
  else {
    y->high = fhigh;
    y->low = flow;
  }
  y->range = y->high - y->low;
  while(y->range != 0 && fabs(y->step) > y->range)
    {
      y->step = (fabs(y->step) - y->range)* y->sign;
    }
}

void floatcount_floatmode(t_floatcount *y, t_floatarg f)
{
  y->remfloat = f != 0 ? 1 : 0;
}

// creation arguments: stepsize, highlimit, lowlimit, startvalue
void *floatcount_new(t_symbol *s, int argc, t_atom *argv)
{
  float flow, fhigh, fstep;
  t_floatcount *y = (t_floatcount *)pd_new(floatcount_class);
  y->init = 1;
  if (argc>4)
    {
      fstep = atom_getfloat(argv);
      fhigh = atom_getfloat(argv+1);
      flow  = atom_getfloat(argv+2);
      y->remfloat = (argv+3) != 0 ? 1 : 0;
      y->step = fstep != 0 ? fstep : 1;
      y->sign = y->step > 0 ? 1 : -1;
      if (flow > fhigh) {
	y->high = flow;
	y->low  = fhigh;
      }
      else {
	y->high = fhigh;
	y->low = flow;
      }
      y->range = y->high - y->low;
      y->floatcount = atom_getfloat(argv+3);
      y->init = 0;
      y->remfloat = 0;
    }
  else if (argc>3)
    {
      fstep = atom_getfloat(argv);
      fhigh = atom_getfloat(argv+1);
      flow  = atom_getfloat(argv+2);
      y->step = fstep != 0 ? fstep : 1;
      y->sign = y->step > 0 ? 1 : -1;
      if (flow > fhigh) {
	y->high = flow;
	y->low  = fhigh;
      }
      else {
	y->high = fhigh;
	y->low = flow;
      }
      y->range = y->high - y->low;
      y->floatcount = atom_getfloat(argv+3);
      y->init = 0;
      y->remfloat = 0;
    }
  else if (argc>2)
    {
      fstep = atom_getfloat(argv);
      fhigh = atom_getfloat(argv+1);
      flow  = atom_getfloat(argv+2);
      y->step = fstep != 0 ? fstep : 1;
      y->sign = y->step > 0 ? 1 : -1;
      if (flow > fhigh) {
	y->high = flow;
	y->low  = fhigh;
      }
      else {
	y->high = fhigh;
	y->low = flow;
      }
      y->range = y->high - y->low;
      y->floatcount = y->low;
      y->init = 0;
      y->remfloat = 0;
    }
  else if (argc>1)
    {
      fstep = atom_getfloat(argv);
      fhigh = atom_getfloat(argv+1);
      y->step = fstep != 0 ? fstep : 1;
      y->sign = y->step > 0 ? 1 : -1;
      if (fhigh < 0) {
	y->high = 0;
	y->low  = fhigh;
      }
      else {
	y->high = fhigh;
	y->low = 0;
      }
      y->floatcount = 0;
      y->remfloat = 0;
    }
  else if (argc)
    {
      fstep = atom_getfloat(argv);
      y->step = fstep != 0 ? fstep : 1;
      y->sign = y->step > 0 ? 1 : -1;
      y->high = 100;
      y->low  = 0;
      y->range = 100;
      y->floatcount = 0;
    }
  else
    {
      y->step = 1;
      y->sign = 1;
      y->high = 100;
      y->low  = 0;
      y->range = 100;
      y->floatcount = 0;
      y->remfloat = 0;
    }
  y->count = outlet_new(&y->x_obj, gensym("float"));
  y->icount = outlet_new(&y->x_obj, gensym("float"));
  y->remain = outlet_new(&y->x_obj, gensym("float"));
  y->diff = outlet_new(&y->x_obj, gensym("float"));
  y->isign = outlet_new(&y->x_obj, gensym("float"));
  y->wrapped = outlet_new(&y->x_obj, gensym("bang"));
  return(void *)y;
}

void floatcount_setup(void) 
{
  floatcount_class = class_new(gensym("floatcount"),
  (t_newmethod)floatcount_new,
  0, sizeof(t_floatcount),
  0, A_GIMME, 0);
  post("floatcount counts floats _.");
  class_addbang(floatcount_class, floatcount_bang);
  class_addfloat(floatcount_class, floatcount_float);
  class_addmethod(floatcount_class, (t_method)floatcount_step, gensym("step"), A_DEFFLOAT, 0);
  class_addmethod(floatcount_class, (t_method)floatcount_high, gensym("high"), A_DEFFLOAT, 0);
  class_addmethod(floatcount_class, (t_method)floatcount_low, gensym("low"), A_DEFFLOAT, 0);
  class_addmethod(floatcount_class, (t_method)floatcount_reset, gensym("reset"), A_DEFFLOAT, 0);
  class_addmethod(floatcount_class, (t_method)floatcount_floatmode, gensym("mode"), A_DEFFLOAT, 0);
}
