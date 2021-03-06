/*
 * bmt~ : bassmidtreble, according to threshold 
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

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

static t_class *bmt_tilde_class;

typedef struct _bmt_tilde 
{
  t_object x_obj;
  t_atom levels[128]; // a ridiculous number of bands!!!
  t_atom binmost[128];
  //  t_atom xovers[127]; // and their crossover band points.
  // Make this so the bands can be log! or pow x!

  t_float f;
  t_float mode;
  t_float length; // how many do we have?
  t_outlet *bmt, *bins;
} t_bmt_tilde;

t_int *bmt_tilde_perform(t_int *w)
{
  t_bmt_tilde  *x =  (t_bmt_tilde *)(w[1]);
  t_sample  *real =     (t_sample *)(w[2]);
  t_sample  *imag =     (t_sample *)(w[3]);
  int           n =            (int)(w[4]);
  float incr = 0;
  float max = 0;
  float vectorr, vectori;
  float alpha;
  int n_real = n / 2;
  float bsize = n_real / x->length;
  float block = 0;
  float reblock = 0;
  int ilength = (int)(x->length);
  int iblock = 0;

  while (n--)
    {
      vectorr = (*real++);
      vectori = (*imag++);
      if (n > n_real)
	{
	  if (x->mode == 0)
	    {
	      reblock = block;
	      alpha = sqrt((vectorr * vectorr) + (vectori * vectori));
	      block = (int)(incr / bsize);
	      iblock = (int)(block);
	      if (block != reblock)
		{
		  max = alpha;
		}
	      else if (alpha > max)
		{
		  max = alpha;
		  SETFLOAT(&x->levels[iblock], max);
		  SETFLOAT(&x->binmost[iblock], incr-((int)(block*bsize)));
		}
	    }
	  else if (x->mode == 1)
	    {
	      reblock = block;
	      alpha = sqrt((vectorr * vectorr) + (vectori * vectori));
	      block = (int)(incr / bsize);
	      iblock = (int)(block);
	      if (block != reblock)
		{
		  max = alpha;
		}
	      else if (block == reblock)
		{
		  max += alpha;
		  SETFLOAT(&x->levels[iblock], max);
		  SETFLOAT(&x->binmost[iblock], incr-((int)(block*bsize)));
		}
	    }
      } 
      incr++;
    }
  outlet_list(x->bins, gensym("list"), ilength, x->binmost);
  outlet_list(x->bmt, gensym("list"), ilength, x->levels);
  //outlet_list(x->f_levs, (float)max);
  //outlet_list(x->f_bmt, x->f_topbin);

  return(w+5);
}

void bmt_tilde_mode(t_bmt_tilde *x, t_floatarg n)
{
  x->mode = n;
}

void bmt_tilde_dsp(t_bmt_tilde *x, t_signal **sp)
{
  dsp_add(bmt_tilde_perform, 4, x,
	  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void *bmt_tilde_new(t_floatarg nbands)
{
  t_bmt_tilde *x = (t_bmt_tilde *)pd_new(bmt_tilde_class);

  x->length = nbands > 1 ? nbands > 128 ? 128 : nbands : 3;
  // set min max nbands
  inlet_new (&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  //  floatinlet_new (&x->x_obj, &x->f_thresh);
  int i;
  x->mode = 0; //make this a creation arg...Ed
  for(i=0;i<128;i++)
    {
	  SETFLOAT(&x->levels[i], 0);
	  SETFLOAT(&x->binmost[i], 0);
    }
  x->bmt = outlet_new(&x->x_obj, gensym("list"));
  x->bins = outlet_new(&x->x_obj, gensym("list"));
  return (void *)x;
}

void bmt_tilde_setup(void)
{
  bmt_tilde_class = class_new(gensym("bmt~"),
				     (t_newmethod)bmt_tilde_new,
				     0, sizeof(t_bmt_tilde),
				     CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("|=======bmt~========|");
  post("|=bass==mid==treble=|");
  post("|=ed==kelly===2010==|");

  class_addmethod(bmt_tilde_class, (t_method)bmt_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(bmt_tilde_class, (t_method)bmt_tilde_mode, gensym("mode"), A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(bmt_tilde_class, t_bmt_tilde, f);
}
