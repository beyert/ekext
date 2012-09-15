/* peakit~ - find frequencies and magnitudes of FFT peaks
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
#include <string.h>

static t_class *peakit_tilde_class;

typedef struct _bin_store
{
  t_atom peaks[2048];
  t_atom indices[2048];
  t_atom theta[4096];
  t_atom t_prev[4096];
  t_atom t_delta[4096];
  t_atom peakfreqs[2048];
  t_float *f_real, *f_imag;
} t_bin_store;

typedef struct _peakit_tilde
{
  t_object x_obj;
  t_bin_store x_ctl;
  t_int minmeth;
  t_outlet *peaks_list, *mags_list;
  t_float f_npeaks, f_minmag, f_dummy;
} t_peakit_tilde;

t_int *peakit_tilde_perform(t_int *w)
{
  t_peakit_tilde  *x =  (t_peakit_tilde *)(w[1]);
  t_bin_store *store =     (t_bin_store *)(w[2]);
  t_sample  *profile =        (t_sample *)(w[3]);
  int              n =               (int)(w[4]);
  t_float      *real = store->f_real;
  t_float      *imag = store->f_imag;
  int vector = n;
  float s_rate = sys_getsr();
  float bin = s_rate/n;
  float boff = bin/2;
  float inexp;
  float interp;
  float minus_two = 0;
  float minus_one = 0;
  float alpha = 0;
  float peakfreq;
  float theta = 0;
  float t_prev = 0;
  float t_delta;
  float pi=M_PI;
  float twopi=2*pi;
  t_int npeaks = 0;
  int i, ndx;

  /* find peaks in fourier series */
  for (i = 0; i < n; i++)
    {
      alpha = sqrt(real[i] * real[i] + imag[i] * imag[i]);
      theta = atan2(real[i], imag[i]);
      t_prev = atom_getfloatarg(i, 16384, store->theta);
      SETFLOAT (&store->t_prev[i], t_prev);
      SETFLOAT (&store->theta[i], theta);
      t_delta = (atom_getfloatarg(i, 16384, store->theta))-(atom_getfloatarg(i, 16384, store->t_prev));
      SETFLOAT (&store->t_delta[i], t_delta);
      inexp = x->f_minmag * 1-(0.1*(log10(boff+i*bin)));
      if (minus_two<minus_one && alpha<minus_one && minus_one>x->f_minmag && x->minmeth == 0)
	{
	  SETFLOAT (&store->peaks[npeaks],minus_one);
	  SETFLOAT (&store->indices[npeaks],i-1);
	  npeaks++;
	}
      else if (minus_two<minus_one && alpha<minus_one && minus_one>x->f_minmag && x->minmeth == 0)
      minus_two = minus_one;
      minus_one = alpha;
    }
  for (i = 0; i < npeaks; i++)
    {
      ndx = atom_getfloatarg(i, 8192, store->indices);
      t_delta = atom_getfloatarg(ndx, 16384, store->t_delta);
      theta = atom_getfloatarg(ndx, 8192, store->theta);
      t_prev = atom_getfloatarg(ndx, 8192, store->t_prev);
      t_delta = (t_delta < -pi) ? (theta+twopi)-t_prev : (t_delta > pi) ? theta-(t_prev+twopi) : t_delta;
      peakfreq = (ndx * bin + (boff * -(t_delta/pi)));
      SETFLOAT (&store->peakfreqs[i], peakfreq);
    }
  outlet_list(x->mags_list, gensym("list"), (npeaks - 1), store->peaks);
  outlet_list(x->peaks_list, gensym("list"), (npeaks - 1), store->peakfreqs);
  return(w+5);
}

void peakit_tilde_iexp(t_peakit_tilde *x, t_floatarg f)
{
  x->minmeth = f;
}

void *peakit_tilde_dsp(t_peakit_tilde *x, t_signal **sp)
{
  x->x_ctl.f_real = sp[0]->s_vec;
  x->x_ctl.f_imag = sp[1]->s_vec;
  dsp_add(peakit_tilde_perform, 4, x, &x->x_ctl, sp[0]->s_vec, sp[0]->s_n);
  return (void *)x;
}

void *peakit_tilde_new(t_floatarg f)
{
  t_peakit_tilde *x = (t_peakit_tilde *)pd_new(peakit_tilde_class);
  x->f_minmag = f;
  x->minmeth = 0;
  memset(x->x_ctl.theta, 0, 8192);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  floatinlet_new(&x->x_obj, &x->f_minmag);
  x->peaks_list = outlet_new(&x->x_obj, &s_list);
  x->mags_list = outlet_new(&x->x_obj, &s_list);
  return (void *)x;
}

void peakit_tilde_setup(void)
{
  peakit_tilde_class = class_new(gensym("peakit~"), 
  (t_newmethod)peakit_tilde_new, 
  0, sizeof(t_peakit_tilde),
  CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("|--<>---<>--<peakit~>-<>--<>---<>--|");
  post("|--<FFT peaks list>-<>--<>---<>----|");
  post("|-<>-<edward>-<kelly>---<>-<2005>--|");


  class_addmethod(peakit_tilde_class, (t_method)peakit_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(peakit_tilde_class, (t_method)peakit_tilde_iexp, &s_float, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(peakit_tilde_class, t_peakit_tilde, f_dummy);
}
