/*
 * zeroxpos~ - detect zero-crossings in a block of audio
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

static t_class *zeroxpos_tilde_class;

typedef struct _zeroxpos_control
{
  t_float *c_input;
  t_int final_pol;
} t_zeroxpos_control;

typedef struct _zeroxpos_tilde 
{
  t_object x_obj;
  t_zeroxpos_control x_ctl;
  t_float f_num, f_dummy, f_dir;
  t_int i_bang, i_pol, i_count, i_ndx, i_mode;
  t_outlet *f_pos, *f_pol;
} t_zeroxpos_tilde;

t_int *zeroxpos_tilde_perform(t_int *w) 
{
  t_zeroxpos_tilde     *x =   (t_zeroxpos_tilde *)(w[1]);
  t_zeroxpos_control *ctl = (t_zeroxpos_control *)(w[2]);  
  int                   n =                  (int)(w[3]);
  t_float            *in = ctl->c_input;
  x->f_dir = x->f_num >= 0 ? 1 : -1;
  int number = x->f_num >= 1 || x->f_num <= -1 ? (int)fabs(x->f_num) : 1;
  int count = x->i_count;
  int polarity = 1;
  int i = 0;
  x->i_pol = 0;
  x->i_ndx = -1;
  int prev = ctl->final_pol == 0 ? in[0] : ctl->final_pol;

  if(x->i_mode == 0)
    {
      if(x->f_dir > 0)
	{
	  for(i=0;i<n;i++)
	    {
	      polarity = in[i] >= 0 ? 1 : -1;
	      if((polarity < prev || polarity > prev) && count == number && x->i_bang == 1)
		{
		  x->i_ndx = i;
		  x->i_pol = polarity;
		  count += 1e+06;
		  x->i_bang = 0;
		  outlet_float(x->f_pol, (float)x->i_pol);
		  outlet_float(x->f_pos, (float)x->i_ndx);
		}
	      if((polarity < prev || polarity > prev) && count < number) count++;
	      if(i==n-1&&count<number)
		{
		  ctl->final_pol = polarity;
		  x->i_count = count;
		}
	      prev = polarity;
	    }
	}
      else if(x->f_dir < 0)
	{
	  for(i=n-1;i>=0;i--)
	    {
	      polarity = in[i] >= 0 ? 1 : -1;
	      if((polarity < prev || polarity > prev) && count == number && x->i_bang == 1)
		{
		  x->i_ndx = i;
		  x->i_pol = polarity;
		  count += 1e+06;
		  x->i_bang = 0;
		  outlet_float(x->f_pol, (float)(x->i_pol*-1));
		  outlet_float(x->f_pos, (float)x->i_ndx+1);
		}
	      if((polarity < prev || polarity > prev) && count < number) count++;
	      if(i==0&&count<number)
		{
		  ctl->final_pol = polarity;
		  x->i_count = count;
		}
	      prev = polarity;
	    }
	}
    }
  else if(x->i_mode != 0)
    {
      if(x->f_dir > 0)
	{
	  for(i=0;i<n;i++)
	    {
	      polarity = in[i] >= 0 ? 1 : -1;
	      if((polarity < prev || polarity > prev) && count == number)
		{
		  x->i_ndx = i;
		  x->i_pol = polarity;
		  count += 1e+06;
		  x->i_bang = 0;
		}
	      if((polarity < prev || polarity > prev) && count < number) count++;
	      if(i==n-1&&count<number)
		{
		  ctl->final_pol = polarity;
		  x->i_count = count;
		}
	      prev = polarity;
	    }
	}
      else if(x->f_dir < 0)
	{
	  for(i=n-1;i>=0;i--)
	    {
	      polarity = in[i] >= 0 ? 1 : -1;
	      if((polarity < prev || polarity > prev) && count == number)
		{
		  x->i_ndx = i;
		  x->i_pol = polarity;
		  count += 1e+06;
		  x->i_bang = 0;
		}
	      if((polarity < prev || polarity > prev) && count < number) count++;
	      if(i==0&&count<number)
		{
		  ctl->final_pol = polarity;
		  x->i_count = count;
		}
	      prev = polarity;
	    }
	}
    }
  return(w+4);
}

void zeroxpos_tilde_bang(t_zeroxpos_tilde *x)
{
  if(x->i_mode == 0)
    {
      x->i_bang = 1;
      x->i_count = 0;
    }
  else if(x->i_mode != 0 && x->f_dir > 0)
    {
      outlet_float(x->f_pol, (float)x->i_pol);
      outlet_float(x->f_pos, (float)x->i_ndx);
    }
  else if(x->i_mode != 0 && x->f_dir < 0)
    {
      outlet_float(x->f_pol, (float)(x->i_pol*-1));
      outlet_float(x->f_pos, (float)x->i_ndx+1);
    }
}

void zeroxpos_tilde_mode(t_zeroxpos_tilde *x, t_floatarg fmode)
{
  x->i_mode = (int)fmode;
}

void *zeroxpos_tilde_dsp(t_zeroxpos_tilde *x, t_signal **sp) 
{
  x->x_ctl.c_input = sp[0]->s_vec;
  dsp_add(zeroxpos_tilde_perform, 3, x, &x->x_ctl, sp[0]->s_n);
  return (void *)x;
}

void *zeroxpos_tilde_new(t_floatarg f) 
{
  t_zeroxpos_tilde *x = (t_zeroxpos_tilde *)pd_new(zeroxpos_tilde_class);
  x->f_dir = f >= 0 ? 1 : -1;
  x->f_num = f >= 1 || f <= -1 ? (int)fabs(f) : 1; 
  x->x_ctl.final_pol = 0;
  x->i_count = 0;
  floatinlet_new (&x->x_obj, &x->f_num);
  x->f_pos = outlet_new(&x->x_obj, gensym("float"));
  x->f_pol = outlet_new(&x->x_obj, gensym("float"));
  return (void *)x;
}

void zeroxpos_tilde_setup(void) 
{
  zeroxpos_tilde_class = class_new(gensym("zeroxpos~"), 
  (t_newmethod)zeroxpos_tilde_new, 
  0, sizeof(t_zeroxpos_tilde),
  CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("|¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬zeroxpos~``````````````````````|");
  post("|find 1st, 2nd or 3rd etc zero crossing point in frame|");
  post("|````edward¬¬¬¬¬¬¬¬¬¬¬¬kelly``````````````````2005¬¬¬¬|");


  class_addbang(zeroxpos_tilde_class, zeroxpos_tilde_bang);
  class_addmethod(zeroxpos_tilde_class, (t_method)zeroxpos_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(zeroxpos_tilde_class, (t_method)zeroxpos_tilde_mode, gensym("mode"), A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(zeroxpos_tilde_class, t_zeroxpos_tilde, f_dummy);
}

