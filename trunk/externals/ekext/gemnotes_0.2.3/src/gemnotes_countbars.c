#include "m_pd.h"
#include <math.h>

static t_class *gemnotes_countbars_class;

/* list inlet: [rhythm dur vel start]
 * when the BPM changes, the offset accumulates the start time of the first
 * note, and is subtracted from subsequent inter-onset times
 */

/* The Gemnotes system can only handle one voice at the moment. */

/* this object is probably where key signatures (flatsharp flag) and dynamics 
 * will be calculated eventually, since it has access to the pitch and 
 * velocity information
 */

typedef struct _gemnotes_countbars {
  t_object x_obj;
  t_atom outlist[5];
  t_float bpm, qtime, divtime, numer, denom;
  //  int setoffset, iotdiv;
  t_float allcount, barcount, barlen, bardiv, iot, offset, rhythm, dur, notedur, nbars;
  t_float start, pitch, vel;
  t_float prevstart, previot, prevdurt, prevdur, tdiff, polydiff;
  t_float outstart;
  int ndurs, polycompare, debug;
  t_outlet *note, *bars;
} t_gemnotes_countbars;

/* There are real problems with this algorithm. Firstly it assumes that all notes
 * are not polyrhythmic against the basic pulse, so manual editing of the score
 * will be necessary if there are polyrhythmic note divisions. Secondly, there can
 * only be one voice with chords.
 * Perhaps this should all be folded into polyquant, with polyquant
 * measuring all 136 MIDI timings internally...
 * ...for live input transcription, there would need to be quantization
 * of the inter-onset time.
 * I need to learn about clocks in PD!
 */

void gemnotes_countbars_note (t_gemnotes_countbars *z, t_symbol *s, int argc, t_atom *argv)
{
  float newcount = 0;
  if(argc == 5)
    {
      z->prevstart = z->start;
      z->prevdur = z->dur;

      z->pitch = atom_getfloat(argv);
      z->rhythm = atom_getfloat(argv+1);
      z->dur = atom_getfloat(argv+2);
      z->vel = atom_getfloat(argv+3);
      z->start = atom_getfloat(argv+4);

      z->prevdurt = z->notedur;
      z->notedur = z->divtime * z->dur;


      z->previot = z->iot;
      z->iot = z->start - z->prevstart;

      z->tdiff = z->iot - z->prevdurt;
      z->outstart = 0;
      /* Deal with polyrhythmic notes - a kludge until polyquant is integrated with this object */
      z->polydiff = z->iot / z->divtime;
      z->polycompare = (int)z->polydiff;
      if(z->tdiff < 2 && z->tdiff > -2 && z->allcount > 0)
	{
	  newcount = z->barcount + z->prevdur;
	  if(newcount >= z->barlen)
	    {
	      z->nbars++;
	      outlet_float(z->bars, z->nbars);
	    }
	  z->barcount = (float)((int)newcount % (int)z->barlen);
	  z->allcount++;
	  if(z->debug == 1) post("barcount = %d, barlen = %d, + sign",(int)z->barcount, (int)z->barlen);
	  z->outstart = -1;
	}
      else if((float)z->polycompare - z->polydiff < -2)
	{
	  newcount = z->barcount + z->polydiff;
	  if(newcount >= z->barlen)
	    {
	      z->nbars++;
	      outlet_float(z->bars, z->nbars);
	      newcount -= z->barlen;
	      z->barcount = newcount;
	    }
	  else if(newcount < z->barlen)
	    {
	      z->barcount = newcount;
	    }
	  z->outstart = z->barcount;
	  if(z->debug == 1) post("barcount = %d, barlen = %d, poly",(int)z->barcount, (int)z->barlen);
	}
      else if(z->iot < 2 && z->allcount > 0)
	{
	  z->outstart = -2;
	  post("chord");
	}
      else if(z->allcount == 0)
	{
	  z->outstart = 0;
	  z->allcount++;
	  z->nbars = 1;
	  outlet_float(z->bars, z->nbars);
	  post("start");
	}
      else if(z->tdiff > 1)
	{
	  //assumes that the inter-onset time is always either 1 duration or more
	  //this bit will need to be changed for multi-voice scenarios to be supported
	  //but since it is proportional notation, maybe we can minimize rests
	  z->ndurs = 1;
	  while(z->tdiff > 1)
	    {
	      z->ndurs++;
	      z->tdiff -= z->divtime;
	    }
	  z->allcount += (float)z->ndurs;
	  newcount = (float)z->ndurs;
	  while(newcount >= z->barlen)
	    {
	      newcount -= z->barlen;
	      z->nbars++;
	      outlet_float(z->bars, z->nbars);
	    }
	  newcount += z->barcount;
	  if(newcount >= z->barlen)
	    {
	      z->nbars++;
	      outlet_float(z->bars, z->nbars);
	    }
	  z->barcount = (float)((int)(z->barcount + (float)z->ndurs) % (int)z->barlen);
	  z->outstart = z->barcount;
	  if(z->debug == 1) post("barcount = %d, barlen = %d - floatstart",(int)z->barcount, (int)z->barlen);
	}
      else
	{
	  post("iot:%f,tdiff:%f,prevdur:%f,previot:%f,nomethod",z->iot, z->tdiff, z->prevdurt, z->previot);
	}
      SETFLOAT(&z->outlist[0], z->outstart);
      SETFLOAT(&z->outlist[1], z->pitch);
      SETFLOAT(&z->outlist[2], z->dur);
      SETFLOAT(&z->outlist[4], z->vel);
      outlet_list(z->note, gensym("list"), 5, z->outlist);
    }
}

void gemnotes_countbars_bpm (t_gemnotes_countbars *z, t_floatarg f)
{
  if(f != z->bpm && f > 0)
    {
      post("Tempo = %f Beats Per-Minute",f);
      z->bpm = f;
      //      z->setoffset = 1;
      z->qtime = 60000/f;
      z->divtime = z->qtime / (z->bardiv / 4);
      z->notedur = z->divtime * z->dur;
    }
}

void gemnotes_countbars_bar (t_gemnotes_countbars *z, t_symbol *s, int argc, t_atom *argv)
{
  if(argc > 2)
    {
      z->numer = atom_getfloat(argv);
      z->denom = atom_getfloat(argv+1);
      z->bardiv = atom_getfloat(argv+2);
      z->barlen = z->numer / z->denom * z->bardiv;
      z->divtime = z->qtime / (z->bardiv / 4);
      z->prevdurt = z->divtime;
      z->barcount = 0;
      post("Bar of %d/%d, divisions: %d", (int)z->numer, (int)z->denom, (int)z->bardiv);
    }
  else if(argc > 1)
    {
      z->numer = atom_getfloat(argv);
      z->denom = atom_getfloat(argv+1);
      z->barlen = z->numer / z->denom * z->bardiv;
      z->divtime = z->qtime / (z->bardiv / 4);
      z->prevdurt = z->divtime;
      z->barcount = 0;
      post("Bar of %d/%d, divisions: %d", (int)z->numer, (int)z->denom, (int)z->bardiv);
    }
  else if(argc > 0)
    {
      z->numer = atom_getfloat(argv);
      z->barlen = z->numer / z->denom * z->bardiv;
      z->divtime = z->qtime / (z->bardiv / 4);
      z->prevdurt = z->divtime;
      z->barcount = 0;
      post("Bar of %d/%d, divisions: %d", (int)z->numer, (int)z->denom, (int)z->bardiv);
    }
}

void gemnotes_countbars_debug(t_gemnotes_countbars *z, t_floatarg f)
{
  z->debug = f != 0 ? (int)f : 0;
}

void gemnotes_countbars_reset(t_gemnotes_countbars *z)
{
  z->allcount = z->barcount = 0;
  z->nbars = 1;
}

void *gemnotes_countbars_new(t_symbol *s, int argc, t_atom *argv)
{
  t_gemnotes_countbars *z = (t_gemnotes_countbars *)pd_new(gemnotes_countbars_class);
  int i;
  for(i=0;i<5;i++) SETFLOAT(&z->outlist[i], 0);

  z->bpm = 120;
  z->numer = 4;
  z->denom = 4;
  z->bardiv = 16;
  // args: bpm, numer, denom, div
  if(argc > 0) z->bpm = atom_getfloat(argv);
  if(argc > 1) z->numer = atom_getfloat(argv+1);
  if(argc > 2) z->denom = atom_getfloat(argv+2);
  if(argc > 3) z->bardiv = atom_getfloat(argv+3);

  z->qtime = 60000/z->bpm;
  z->divtime = z->qtime / (z->bardiv / z->denom);
  z->prevdurt = z->divtime;

  z->barlen = z->numer / z->denom * z->bardiv;
  z->barcount = z->allcount = 0;
  z->previot = 0;
  z->prevstart = -125;
  z->nbars = 1;
  z->note = outlet_new(&z->x_obj, gensym("list"));
  z->bars = outlet_new(&z->x_obj, gensym("float"));

  return(void *)z;
}

void gemnotes_countbars_setup(void) 
{
  gemnotes_countbars_class = class_new(gensym("gemnotes_countbars"),
  (t_newmethod)gemnotes_countbars_new,
  0, sizeof(t_gemnotes_countbars),
  0, A_GIMME, 0);
  post("gemnotes_countbars - yu need this for realtime notation...");
  post("version 0.4...by Ed Kelly 2010 - morph_2016@yahoo.co.uk");

  class_addmethod(gemnotes_countbars_class, (t_method)gemnotes_countbars_note, gensym("note"), A_GIMME, 0);
  class_addmethod(gemnotes_countbars_class, (t_method)gemnotes_countbars_bar, gensym("bar"), A_GIMME, 0);
  class_addmethod(gemnotes_countbars_class, (t_method)gemnotes_countbars_bpm, gensym("bpm"), A_DEFFLOAT, 0);
  class_addmethod(gemnotes_countbars_class, (t_method)gemnotes_countbars_reset, gensym("reset"), A_DEFFLOAT, 0);
  class_addmethod(gemnotes_countbars_class, (t_method)gemnotes_countbars_debug, gensym("debug"), A_DEFFLOAT, 0);
}
