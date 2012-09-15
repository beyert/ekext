/*                                                                              
 *  polyquant: polyrhythmic notelength parser
 *  Copyright (C) 2010 edward kelly <morph_2016@yahoo.co.uk>
 *
 * This software is BSD licensed. See LICENSE.txt for more details
 */

#include "m_pd.h"
#include <math.h>

t_class *polyquant_class;

typedef struct _polyquant {
  t_object x_obj;
  float dmatrix[18][9];
  float tmatrix[18][9][64];
  t_float bpm, qtime, wtime, plimit, dlimit, mode, sim_win, sim_sign, simile, bestsim, bestdiv, besttime, bestnum, bestsign, startoff, startnow;
  t_outlet *time, *div, *num, *sign;//, *sta;
} t_polyquant;

static float primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61};

void polyquant_float(t_polyquant *x, t_floatarg f) {
  int i, j, k;
  float sim_min;
  x->bestsim = 0;
  // x->sim_win = x->sim_win > 0 ? x->sim_win : 0.01; /* defaults to 0.01 so that we avoid /0 errors */
  for(i = 0;i < x->plimit;i++) {
    for(j = x->dlimit;j < 9;j++) {
      for(k=0;k<64;k++) {
	float tvalue = x->tmatrix[i][j][k];
	sim_min = (f > tvalue) ? (f - tvalue) : (tvalue - f);
	x->sim_sign = (f >= tvalue) ? 1 : -1;
	x->simile = 1 / ((sim_min / x->sim_win) + 1);
	if(x->mode == 0) {
	  if(x->simile >= x->bestsim){
	    x->bestsim = x->simile;
	    x->bestdiv = x->dmatrix[i][j];
	    x->besttime = tvalue;
	    x->bestnum = (float)k+1;
	    x->bestsign = x->sim_sign;
	  }
	}
	else if(x->mode == 1) {
	  if(x->simile >= x->bestsim && x->sim_sign == -1){
	    x->bestsim = x->simile;
	    x->bestdiv = x->dmatrix[i][j];
	    x->besttime = tvalue;
	    x->bestnum = (float)k+1;
	    x->bestsign = x->sim_sign;
	  }
	}
	else if(x->mode == 2) {
	  if(x->simile >= x->bestsim && x->sim_sign == 1){
	    x->bestsim = x->simile;
	    x->bestdiv = x->dmatrix[i][j];
	    x->besttime = tvalue;
	    x->bestnum = (float)k+1;
	    x->bestsign = x->sim_sign;
	  }
	}
      }
    }
    //so...the start time quantized, and then the quantized start becomes 0
    //so that the duration alone is quantized.
    //build it into the gemnotes_counter, so that you can make a gemnotes_counter_live
  }
  outlet_float(x->sign, x->bestsign);
  outlet_float(x->num, x->bestnum);
  outlet_float(x->div, x->bestdiv);
  outlet_float(x->time, x->besttime);
}

/*
 * The rarity of vintage instruments is a musician's cartel
 */

void polyquant_bang(t_polyquant *x) {
  outlet_float(x->sign, x->bestsign);
  outlet_float(x->num, x->bestnum);
  outlet_float(x->div, x->bestdiv);
  outlet_float(x->time, x->besttime);
}

void polyquant_bpm(t_polyquant *x, t_floatarg f) {
  x->bpm = f > 0 ? f : 120;
  x->qtime = 60000/x->bpm;
  x->wtime = 4 * x->qtime;
}

void polyquant_plimit(t_polyquant *x, t_floatarg f) {
  x->plimit = f < 1 ? 1 : f > 18 ? 18 : f;
}

void polyquant_dlimit(t_polyquant *x, t_floatarg f) {
  x->dlimit = f < 0 ? 0 : f > 8 ? 8 : f;
}

void polyquant_mode(t_polyquant *x, t_floatarg f) {
  x->mode = f < 0 ? 0 : f > 2 ? 2 : f;
}

void polyquant_win(t_polyquant *x, t_floatarg f) {
  x->sim_win = f > 0 ? f : 10;
}

void *polyquant_new(t_floatarg ibpm, t_floatarg flim)
{
  t_polyquant *x = (t_polyquant *)pd_new(polyquant_class);
  x->mode = 0;
  x->bpm = ibpm > 0 ? ibpm : 120;
  x->qtime = 60000 / x->bpm;
  x->wtime = x->qtime * 4;
  x->plimit = flim < 1 ? 1 : flim > 18 ? 18 : flim;
  x->dlimit = 4;
  x->sim_win = 10;
  x->time = outlet_new(&x->x_obj, gensym("float"));
  x->div = outlet_new(&x->x_obj, gensym("float"));
  x->num = outlet_new(&x->x_obj, gensym("float"));
  x->sign = outlet_new(&x->x_obj, gensym("float"));

  int i, j, k;
  float div, time;
  //init dmatrix
  for(i=0;i<18;i++) {
    for(j=0;j<9;j++) {
      x->dmatrix[i][j] = 0;
    }
  }

  j = 0;
  div = 0;
  for(i=0;i<18;i++) {
    div = primes[i];
    while(div<=128) {
      div *= 2;
    }
    x->dmatrix[i][0] = div;
    while(div>=1) {
      j++;
      div *= 0.5;
      x->dmatrix[i][j] = div;
    }
  }
  //init tmatrix
  for(i=0;i<18;i++) {
    for(j=0;j<9;j++) {
      div = x->dmatrix[i][j];
      if(div>0) {
	k = 0;
	time = x->wtime / div;
	for(k=0;k<64;k++) {
	  x->tmatrix[i][j][k] = time * (k + 1);
	}
      }
    }
  }
  return(void *)x;
}


void polyquant_setup(void) {
  polyquant_class = class_new(gensym("polyquant"),
  (t_newmethod)polyquant_new,
  0, sizeof(t_polyquant),
  0, A_DEFFLOAT, A_DEFFLOAT, 0);
  post("|---------->polyquant<----------|");
  post("|---->polyrhythmic quantize<----|");
  post("|-->edward<--->kelly<--->2010<--|");

  class_addbang(polyquant_class, polyquant_bang);
  class_addfloat(polyquant_class, polyquant_float);
  class_addmethod(polyquant_class, (t_method)polyquant_bpm, gensym("bpm"), A_DEFFLOAT, 0);
  class_addmethod(polyquant_class, (t_method)polyquant_plimit, gensym("plimit"), A_DEFFLOAT, 0);
  class_addmethod(polyquant_class, (t_method)polyquant_dlimit, gensym("dlimit"), A_DEFFLOAT, 0);
  class_addmethod(polyquant_class, (t_method)polyquant_mode, gensym("mode"), A_DEFFLOAT, 0);
  class_addmethod(polyquant_class, (t_method)polyquant_win, gensym("win"), A_DEFFLOAT, 0);
}
