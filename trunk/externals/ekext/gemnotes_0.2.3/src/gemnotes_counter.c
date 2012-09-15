/* Gemnotes_counter is a beat, beamgroup and bar counter for the gemnotes system.
 * It is used to count the elements of music notation, and to send commands to
 * dynamic object makers to render notes, rests, time signatures and barlines.
 * It keeps track of the number of obects created, and whether objects are linked
 * together (such as beamed groups of notes).
 * BSD license
 */

#include "m_pd.h"
#include <math.h>

static int gemnotes_counter_splits[6][33] = {
  /* 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 */
  {0,1,2,3,4,3,6,4,8,6,6, 8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
  {0,0,0,0,0,2,0,3,0,3,4, 3, 4, 3, 6, 4, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
  {0,0,0,0,0,0,0,0,0,0,0, 0, 0, 2, 0, 3, 0, 3, 6, 3, 4, 3, 6, 4, 8, 6, 8, 8, 8, 8, 8, 8, 8},
  {0,0,0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 3, 0, 3, 2, 3, 4, 3, 6, 4, 8},
  {0,0,0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 3, 0},
  {0,0,0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

t_class *gemnotes_counter_class;

typedef struct gemnotes_counter
{
  t_object x_obj;
  t_atom groups[128];
  t_atom keysig[7];
  t_atom temposign[3];
    t_atom dynamics[5];
    t_atom terrace[2];
    t_float articulation;
//    t_float artnote;
  int numgroups, nowgroup, beforerest;//, lastgroup;
  t_float allcount, allstep;
  t_float barcount, barlen, barrem, numerator, denominator, nextnumer, nextdenom;
  t_float groupcount, grouplen, groupnum, groupdiv, grouprem, gnoterem, bnotecount;
  t_float stepdiv, rhythm, duration, beam, tied, step, rrest, tailon;
  t_float objects, initobjects, groupobject, barlast;
  t_float barnotes, barrests, barpos, restlen, lastdur;
  t_float tempo, stavelen, fstavenum, group_id;
  int stavenum;
  t_float fractional, offset, initoffset, keyset, rested, prevrested, polyrhythm;
  t_outlet *off, *rhy, *dur, *grp, *gcount, *tie, *bm, *rest, *rhyrest, *bar, *slave, *numer, *denom, *g_obj, *tail, *tuple, *bpm, *n_obj, *debugger, *groupid, *artic, *dyn;//, *stave;
} t_gemnotes_counter;

void gemnotes_counter_bang(t_gemnotes_counter *z)
/* This function inserts a note at the current position, then makes any adjustments
 * needed for the next beamgroups and bars etc. It also generates tie signals,
 * and indicates whether tuples are being used */
{
//    outlet_float(z->artic, z->articulation);
//    z->articulation = 0;
    z->gnoterem = z->grouprem / z->stepdiv;
  float zpoly, polydiv;
  int ipoly, gdiv, prevstave;
  float chunk = z->step;
  float thischunk, thatchunk, maxchunk, tail;
  float prevnumer, prevdenom;
  int i, j, x, y;
  while(chunk > 0)
    {
//      z->rested = 0;
      thischunk = z->gnoterem <= chunk ? z->gnoterem : chunk;
      thischunk = thischunk > 32 ? 32 : thischunk;
      chunk -= thischunk;
      z->beam = z->groupcount > 0 && z->groupdiv >= 8 ? 1 : 0;
      i = (int)thischunk;
      for(j=0;j<6;j++)
	{
	  x = gemnotes_counter_splits[j][i];
	  if(x > 0)
	    {
	      /*	      prevstave = z->stavenum;
	      z->stavenum = (int)((z->allcount*z->allstep+z->offset) / z->stavelen);
	      z->fstavenum = (float)z->stavenum;
	      outlet_float(z->stave,z->fstavenum);
	      if(z->stavenum > prevstave)
		{
		  z->objects = z->initobjects;
		  z->groupobject = z->objects;
		  z->groupnum -= z->groupcount;
		  z->groupcount = 0;
		  z->beam = 0;
		  z->allcount = 0;
		} */
	      z->tailon = z->grouprem <= z->step && z->rested == 1 || z->groupcount == z->groupnum - 1 && z->rested == 1 ? 1 : 0;
	      thischunk -= (float)x;
	      z->tied = chunk > 0 || thischunk > 0 ? 1 : 0;

	      z->barlast = 0;
	      outlet_float(z->groupid,z->group_id);
	      outlet_float(z->n_obj, z->objects);
	      outlet_float(z->tuple, z->polyrhythm);
	      if(z->beam == 0)
		{
		  z->groupobject = z->objects;
		  outlet_float(z->g_obj, z->objects);
		}
            outlet_float(z->artic, z->articulation);
            z->articulation = 0;
            outlet_float(z->tail, z->tailon);
	      z->objects++;
	      outlet_float(z->bm, z->beam);
	      outlet_float(z->tie, z->tied);
	      outlet_float(z->gcount, z->groupcount);
	      outlet_float(z->grp, z->groupnum);
	      outlet_float(z->dur, (float)x);
	      outlet_float(z->rhy, z->rhythm);
	      outlet_float(z->off, z->allcount*z->allstep+z->offset);
	      thatchunk = (float)x * z->stepdiv;
	      maxchunk = thatchunk > 16 ? 16 : thatchunk;
	      z->allcount += maxchunk;
	      z->barcount += thatchunk;
	      z->barrem -= thatchunk;
	      //	      z->barnotes += thatchunk;
	      z->groupcount += (float)x;
	      z->grouprem -= thatchunk;
	      z->lastdur = thatchunk;
	      z->prevrested = z->rested;
	      z->rested = 0;
	    }
	}
      if(z->barrem < 1 && z->allcount > 0 && z->barlast == 0)
	{
	  z->group_id++;
	  z->barlast = 1;
	  outlet_float(z->groupid, z->group_id);
	  outlet_float(z->n_obj, z->objects);
	  //	  z->barpos += (z->barnotes + z->barrests) * z->allstep;
	  maxchunk = z->lastdur > 16 ? 16 : z->lastdur;
	  z->barpos = (z->allcount - (maxchunk / 2)) * z->allstep + z->offset;
	  // This is the alternative barpos method, if the other one can't be made to work.
	  z->objects++;
	  outlet_float(z->bar, z->barpos);
	  z->barnotes = z->barrests = 0;
	  if(z->nextnumer)
	    {
	      prevnumer = z->numerator;
	      prevdenom = z->denominator;
	      z->numerator = z->nextnumer;
	      if(z->nextdenom)
		{
		  z->denominator = z->nextdenom;
		}
	      if(z->numerator != prevnumer || z->denominator != prevdenom)
		{
		  outlet_float(z->n_obj, z->objects);
		  z->objects++;
		  outlet_float(z->denom, z->denominator);
		  outlet_float(z->numer, z->numerator);
		}
	      z->nextnumer = z->nextdenom = 0;
	      z->barlen = (64 / z->denominator) * z->numerator;
	    }
	  z->nowgroup = 0;
	  z->groupcount = 0;
	  z->barcount = 0;
	  z->groupnum = atom_getfloatarg(z->nowgroup, 128, z->groups);
	  z->groupdiv = atom_getfloatarg(z->nowgroup + 1, 128, z->groups);
	  z->polyrhythm = 0;
	  gdiv = (int)z->groupdiv;
	  if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
	    {
	      //	      polydiv = 1;
	      zpoly = z->groupdiv;
	      ipoly = (int)zpoly;

	      for(i=0;i<7;i++)
		{
		  //		  polydiv *= 0.5;
		  zpoly *= 0.5;
		  ipoly = (int)zpoly;
		  post("ipoly = %d, zpoly = %f",ipoly,zpoly);
		  if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
		}
	    }
	  z->stepdiv = 64 / z->groupdiv;
	  z->grouplen = z->groupnum * z->stepdiv;
	  z->grouprem = z->grouplen;
	  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
	  z->barrem = z->barlen;
	}
      else if(z->grouprem < 1)
	{
	  z->group_id++;
	  outlet_float(z->groupid, z->group_id);
	  z->nowgroup = (z->nowgroup + 1) % z->numgroups;
	  z->groupcount = 0;
	  z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
	  z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
	  z->polyrhythm = 0;
	  gdiv = (int)z->groupdiv;
	  if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
	    {
	      //	      polydiv = 1;
	      zpoly = z->groupdiv;
	      ipoly = (int)zpoly;

	      for(i=0;i<7;i++)
		{
		  //		  polydiv *= 0.5;
		  zpoly *= 0.5;
		  ipoly = (int)zpoly;
		  post("ipoly = %d, zpoly = %f",ipoly,zpoly);
		  if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
		}
	    }
	  z->stepdiv = 64 / z->groupdiv;
	  z->grouplen = z->groupnum * z->stepdiv;
	  z->grouprem = z->grouplen;
	  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
	}
    }
    outlet_float(z->debugger, z->allcount*z->allstep+z->offset);
}

void gemnotes_counter_float(t_gemnotes_counter *z, t_floatarg f)
/* this function inserts rests if the float is different from the current position,
 * also a barline if the float is less than the current position within the bar. 
 * Otherwise it inserts a note at the current position if the float is the same (sic)*/
{
  z->gnoterem = z->grouprem / z->stepdiv;
  int i, j, x;
  float fpos = f * z->stepdiv;
  int ipoly, gdiv, prevstave;
  float zpoly, polydiv;
  float chunk, thischunk, thatchunk, maxchunk, prevnumer, prevdenom;
  if(fpos != z->barcount)
    {
      if(fpos < z->barcount)
	{
	  //	  z->beam = 0;
	  j = 0;
	  chunk = (z->barrem + fpos) / z->stepdiv;
	  z->barpos += ((z->barrem / z->stepdiv) * z->restlen + z->barnotes) * z->allstep; 
	  while(chunk > 0)
	    {
	      z->barlast = 0;
	      thischunk = z->gnoterem <= chunk ? z->gnoterem : chunk;
	      thischunk = thischunk > 32 ? 32 : thischunk;
	      chunk -= thischunk;
	      i = (int)thischunk;
	      for(j=0;j<6;j++)
		{
		  x = gemnotes_counter_splits[j][i];
		  if(x > 0)
		    {
		      /*		      prevstave = z->stavenum;
		      z->stavenum = (int)((z->allcount*z->allstep+z->offset) / z->stavelen);
		      z->fstavenum = (float)z->stavenum;
		      outlet_float(z->stave,z->fstavenum);
		      if(z->stavenum > prevstave)
			{
			  z->objects = z->initobjects;
			  z->groupobject = z->objects;
			  z->groupnum -= z->groupcount;
			  z->groupcount = 0;
			  z->allcount = 0;
			}
		      if(z->rested == 0 && z->prevrested == 1)
			{
			  z->tailon = z->groupobject;
			  outlet_float(z->tail, z->tailon);
			  } */
		      z->prevrested = z->rested;
		      z->rested = 1;
		      outlet_float(z->n_obj, z->objects);
		      z->objects++;
		      outlet_float(z->tuple, z->polyrhythm);
		      outlet_float(z->rhyrest, z->rhythm);
		      outlet_float(z->dur, (float)x);
		      outlet_float(z->rest, z->allcount*z->allstep + z->offset);
		      thatchunk = (float)x * z->stepdiv;
		      z->allcount += thatchunk * z->restlen;
		      //		      z->barrests += thatchunk * z->restlen;
		      z->barcount += thatchunk;
		      z->barrem -= thatchunk;
		      z->groupcount += (float)x;
		      thischunk -= (float)x;
		      z->grouprem -= thatchunk;
		      z->lastdur = thatchunk * z->restlen;
		      z->rested = 1;
		    }
		}
	      if(z->barrem < 1 && z->barlast == 0) //&& z->allcount > 0)
		{
		  z->group_id++;
		  outlet_float(z->groupid, z->group_id);
		  outlet_float(z->n_obj, z->objects);
		  z->barlast = 1;
		  z->objects++;
		  maxchunk = z->lastdur > 16 ? 16 : z->lastdur;
		  z->barpos = (z->allcount - (maxchunk / 2)) * z->allstep + z->offset;
		  //		  z->barpos += (z->barnotes + z->barrests) * z->allstep;
		  outlet_float(z->bar, z->barpos);
		  z->barnotes = z->barrests = 0;
		  if(z->nextnumer)
		    {
		      prevnumer = z->numerator;
		      prevdenom = z->denominator;
		      z->numerator = z->nextnumer;
		      if(z->nextdenom)
			{
			  z->denominator = z->nextdenom;
			}
		      if(z->numerator != prevnumer || z->denominator != prevdenom)
			{
			  outlet_float(z->n_obj, z->objects);
			  z->objects++;
			  outlet_float(z->denom, z->denominator);
			  outlet_float(z->numer, z->numerator);
			}
		      z->nextnumer = z->nextdenom = 0;
		      z->barlen = (64 / z->denominator) * z->numerator;
		    }

		  z->nowgroup = 0;
		  z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
		  z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
		  z->stepdiv = 64 / z->groupdiv;
		  z->barcount = fpos * z->stepdiv;
		  z->barrem = z->barlen - z->barcount;
		  z->grouplen = z->groupnum * z->stepdiv;
		  z->grouprem = z->groupnum - fpos;
		  if(z->grouprem <= 0) // a horrible kludge!!!recursion needed
		    {
		      z->nowgroup = (z->nowgroup + 1) % z->numgroups;
		      z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
		      z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
		      z->stepdiv = 64 / z->groupdiv;
		      z->grouplen = z->groupnum * z->stepdiv;
		      z->grouprem = z->groupnum - (0 - fpos);
		      if(z->grouprem <= 0)
			{
			  z->nowgroup = (z->nowgroup + 1) % z->numgroups;
			  z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
			  z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
			  z->stepdiv = 64 / z->groupdiv;
			  z->grouplen = z->groupnum * z->stepdiv;
			  z->grouprem = z->groupnum - (0 - z->grouprem);
			}
		    }
		  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
		  z->groupcount = 0;
		  z->polyrhythm = 0;
		  gdiv = (int)z->groupdiv;
		  if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
		    {
		      //		      polydiv = 1;
		      zpoly = z->groupdiv;
		      ipoly = (int)zpoly;

		      for(i=0;i<7;i++)
			{
			  //			  polydiv *= 0.5;
			  zpoly *= 0.5;
			  ipoly = (int)zpoly;
			  post("ipoly = %d, zpoly = %f",ipoly,zpoly);
			  if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
			}
		    }
		}
	      else if(z->grouprem < 1)
		{
		  z->group_id++;
		  outlet_float(z->groupid, z->group_id);
		  z->nowgroup = (z->nowgroup + 1) % z->numgroups;
		  z->groupcount = 0;
		  z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
		  z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
		  z->stepdiv = 64 / z->groupdiv;
		  z->grouplen = z->groupnum * z->stepdiv;
		  z->grouprem = z->grouplen;
		  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
		  z->polyrhythm = 0;
		  gdiv = (int)z->groupdiv;
		  if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
		    {
		      polydiv = 1;
		      zpoly = z->groupdiv;
		      ipoly = (int)zpoly;

		      for(i=0;i<7;i++)
			{
			  polydiv *= 0.5;
			  zpoly *= 0.5;
			  ipoly = (int)zpoly;
			  post("ipoly = %d, zpoly = %f",ipoly,zpoly);
			  if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
			}
		    }
		}
	    }
	}
      else if(fpos > z->barcount)
	{
	  z->beam = 0;
	  chunk = (fpos - z->barcount) / z->stepdiv;
	  while(chunk > 0)
	    {
	      thischunk = z->gnoterem <= chunk ? z->gnoterem : chunk;
	      thischunk = thischunk > 32 ? 32 : thischunk;
	      chunk -= thischunk;
	      i = (int)thischunk;
	      for(j=0;j<6;j++)
		{
		  x = gemnotes_counter_splits[j][i];
		  if(x > 0)
		    {
		      z->barlast = 0;
		      /*		      prevstave = z->stavenum;
		      z->stavenum = (int)((z->allcount*z->allstep+z->offset) / z->stavelen);
		      z->fstavenum = (float)z->stavenum;
		      outlet_float(z->stave,z->fstavenum);
		      if(z->stavenum > prevstave)
			{
			  z->objects = z->initobjects;
			  z->groupobject = z->objects;
			  z->groupnum -= z->groupcount;
			  z->groupcount = 0;
			  z->allcount = 0;
			  } */
		      if(z->rested == 0 && z->prevrested == 1)
			{
			  z->tailon = z->groupobject;
			  outlet_float(z->tail, z->tailon);
			}
		      z->prevrested = z->rested;
		      z->rested = 1;
		      outlet_float(z->groupid, z->group_id);
		      outlet_float(z->n_obj, z->objects);
		      z->objects++;
		      outlet_float(z->tuple, z->polyrhythm);
		      outlet_float(z->rhyrest, z->rhythm);
		      outlet_float(z->dur, (float)x);
		      outlet_float(z->rest, z->allcount*z->allstep + z->offset);
		      thatchunk = (float)x * z->stepdiv;
		      z->allcount += thatchunk * z->restlen;
		      //		      z->barrests += thatchunk * z->restlen;
		      z->barcount += thatchunk;
		      z->barrem -= thatchunk;
		      z->groupcount += (float)x;
		      thischunk -= (float)x;
		      z->grouprem -= thatchunk;
		      z->lastdur = thatchunk * z->restlen;
		    }
		}
	      if(z->grouprem < 1)
		{
		  z->group_id++;
		  outlet_float(z->groupid, z->group_id);
		  z->nowgroup = (z->nowgroup + 1) % z->numgroups;
		  z->groupcount = 0;
		  z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
		  z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
		  z->polyrhythm = 0;
		  gdiv = (int)z->groupdiv;
		  if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
		    {
		      //		      polydiv = 1;
		      zpoly = z->groupdiv;
		      ipoly = (int)zpoly;

		      for(i=0;i<7;i++)
			{
			  //			  polydiv *= 0.5;
			  zpoly *= 0.5;
			  ipoly = (int)zpoly;
			  post("ipoly = %d, zpoly = %f",ipoly,zpoly);
			  if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
			}
		    }
		  z->stepdiv = 64 / z->groupdiv;
		  z->grouplen = z->groupnum * z->stepdiv;
		  z->grouprem = z->grouplen;
		  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
		}
	    }
	}
    }
/* the entire "bang" routine */
  chunk = z->step;
  while(chunk > 0)
    {
      thischunk = z->gnoterem <= chunk ? z->gnoterem : chunk;
      thischunk = thischunk > 32 ? 32 : thischunk;
      chunk -= thischunk;
      /*      if(z->rested == 1)
	{
	  z->groupnum -= z->groupcount;
	  z->groupcount = 0;
	} */
      z->beam = z->grouprem && z->groupcount > 0 && z->groupdiv >= 8 ? 1 : 0;
      i = (int)thischunk;
      for(j=0;j<6;j++)
	{
	  x = gemnotes_counter_splits[j][i];
	  if(x > 0)
	    {
	      /*	      prevstave = z->stavenum;
	      z->stavenum = (int)((z->allcount*z->allstep+z->offset) / z->stavelen);
	      z->fstavenum = (float)z->stavenum;
	      outlet_float(z->stave,z->fstavenum);
	      if(z->stavenum > prevstave)
		{
		  z->objects = z->initobjects;
		  z->groupobject = z->objects;
		  z->groupnum -= z->groupcount;
		  z->groupcount = 0;
		  z->beam = 0;
		  z->allcount = 0;
		  } */
	      z->tailon = z->grouprem <= z->step && z->rested == 1 || z->groupcount == z->groupnum - 1 && z->rested == 1 ? 1 : 0;
	      thischunk -= (float)x;
	      z->tied = chunk > 0 || thischunk > 0 ? 1 : 0;

	      z->barlast = 0;
	      outlet_float(z->n_obj, z->objects);
	      outlet_float(z->tuple, z->polyrhythm);
	      if(z->beam == 0)
		{
		  z->groupobject = z->objects;
		  outlet_float(z->g_obj, z->objects);
		}
            outlet_float(z->artic, z->articulation);
            z->articulation = 0;
	      outlet_float(z->tail, z->tailon);
	      z->objects++;
	      outlet_float(z->bm, z->beam);
	      outlet_float(z->tie, z->tied);
	      outlet_float(z->gcount, z->groupcount);
	      outlet_float(z->grp, z->groupnum);
	      outlet_float(z->dur, (float)x);
	      outlet_float(z->rhy, z->rhythm);
	      outlet_float(z->off, z->allcount*z->allstep+z->offset);
	      thatchunk = (float)x * z->stepdiv;
	      maxchunk = thatchunk > 16 ? 16 : thatchunk;
	      z->allcount += maxchunk;
	      z->barcount += thatchunk;
	      z->barrem -= thatchunk;
	      //	      z->barnotes += thatchunk;
	      z->groupcount += (float)x;
	      z->grouprem -= thatchunk;
	      z->prevrested = z->rested;
	      z->rested = 0;
	      z->lastdur = thatchunk * z->restlen;
	    }
	}
      if(z->barrem < 1 && z->allcount > 0 && z->barlast == 0)
	{
	  z->barlast = 1;
	  z->group_id++;
	  outlet_float(z->groupid, z->group_id);
	  outlet_float(z->n_obj, z->objects);
	  maxchunk = z->lastdur > 16 ? 16 : z->lastdur;
	  z->barpos = (z->allcount - (maxchunk / 2)) * z->allstep + z->offset;
	  //	  z->barpos += (z->barnotes + z->barrests) * z->allstep;
	  z->objects++;
	  outlet_float(z->bar, z->barpos);
	  z->barnotes = z->barrests = 0;
	  if(z->nextnumer)
	    {
	      prevnumer = z->numerator;
	      prevdenom = z->denominator;
	      z->numerator = z->nextnumer;
	      if(z->nextdenom)
		{
		  z->denominator = z->nextdenom;
		}
	      if(z->numerator != prevnumer || z->denominator != prevdenom)
		{
		  outlet_float(z->n_obj, z->objects);
		  z->objects++;
		  outlet_float(z->denom, z->denominator);
		  outlet_float(z->numer, z->numerator);
		}
	      z->nextnumer = z->nextdenom = 0;
	      z->barlen = (64 / z->denominator) * z->numerator;
	    }
	  z->nowgroup = 0;
	  z->groupcount = 0;
	  z->barcount = 0;
	  z->groupnum = atom_getfloatarg(z->nowgroup, 128, z->groups);
	  z->groupdiv = atom_getfloatarg(z->nowgroup + 1, 128, z->groups);
	  z->polyrhythm = 0;
	  gdiv = (int)z->groupdiv;
	  if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
	    {
	      //	      polydiv = 1;
	      zpoly = z->groupdiv;
	      ipoly = (int)zpoly;

	      for(i=0;i<7;i++)
		{
		  //		  polydiv *= 0.5;
		  zpoly *= 0.5;
		  ipoly = (int)zpoly;
		  post("ipoly = %d, zpoly = %f",ipoly,zpoly);
		  if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
		}
	    }
	  z->stepdiv = 64 / z->groupdiv;
	  z->grouplen = z->groupnum * z->stepdiv;
	  z->grouprem = z->grouplen;
	  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
	  z->barrem = z->barlen;
	}
      else if(z->grouprem < 1)
	{
	  z->group_id++;
	  outlet_float(z->groupid, z->group_id);

	  z->nowgroup = (z->nowgroup + 1) % z->numgroups;
	  z->groupcount = 0;
	  z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
	  z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
	  z->polyrhythm = 0;
	  gdiv = (int)z->groupdiv;
	  if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
	    {
	      //	      polydiv = 1;
	      zpoly = z->groupdiv;
	      ipoly = (int)zpoly;

	      for(i=0;i<7;i++)
		{
		  //		  polydiv *= 0.5;
		  zpoly *= 0.5;
		  ipoly = (int)zpoly;
		  post("ipoly = %d, zpoly = %f",ipoly,zpoly);
		  if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
		}
	    }
	  z->stepdiv = 64 / z->groupdiv;
	  z->grouplen = z->groupnum * z->stepdiv;
	  z->grouprem = z->grouplen;
	  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
	}
    }
    outlet_float(z->debugger, z->allcount*z->allstep+z->offset);
}

void gemnotes_counter_bar(t_gemnotes_counter *z, t_symbol *s, int argc, t_atom *argv)
{
  z->gnoterem = z->grouprem / z->stepdiv;
  int i, j, x, y, ipoly;
  int scope = (int)((float)argc * 0.5);
  //  post("scope = %d", scope);
  float chunk, thischunk, thatchunk, maxchunk, zpoly;
  float num, div, prevnumer, prevdenom;
  j = 0;
  // make a rests function for a whole bar, or the rest of the bar, and a barline;
  chunk = z->barrem / z->stepdiv;
  //  post("chunk = %f",chunk);
  z->barpos += (z->barnotes + ((z->barrem / z->stepdiv) * z->restlen) * z->allstep);
  if(z->allcount > 0)
    {
      /*      while(chunk > 0)
	{
	  thischunk = z->gnoterem <= chunk ? z->gnoterem : chunk;
	  thischunk = thischunk > 32 ? 32 : thischunk;
	  chunk -= thischunk;
	  i = (int)thischunk;
	  //	  post("i = %d",i);
	
	  for(j=0;j<6;j++)
	    {
	      x = gemnotes_counter_splits[j][i];
	      //	      post("x = %d",x);
	      if(x > 0)
		{
		  if(z->rested == 0 && z->objects == z->groupobject)
		    {
		      z->tailon = z->groupobject;
		      outlet_float(z->tail, z->tailon);
		    }
		  z->rested = 1;
		  outlet_float(z->n_obj, z->objects);
		  z->objects++;
		  outlet_float(z->tuple, z->polyrhythm);
		  outlet_float(z->rhyrest, z->rhythm);
		  outlet_float(z->dur, (float)x);
		  outlet_float(z->rest, z->allcount*z->allstep + z->offset);
	      
		  thatchunk = (float)x * z->stepdiv;
		  z->allcount += thatchunk * z->restlen;
		  z->barcount += thatchunk;
		  z->barrem -= thatchunk;
		  z->groupcount += (float)x;
		  thischunk -= (float)x;
		  z->grouprem -= thatchunk;
		}
	    }
	    } */
      if(z->barlast == 0)
	{
	  z->barlast = 1;
	  outlet_float(z->n_obj, z->objects);
	  z->objects++;
	  maxchunk = z->lastdur > 16 ? 16 : z->lastdur;
	  z->barpos = (z->allcount - (maxchunk / 2)) * z->allstep + z->offset;
	  outlet_float(z->bar, z->barpos);
	  z->barnotes = z->barrests = 0;
	}
    }
  if(argc)
    {
      prevnumer = z->numerator;
      prevdenom = z->denominator;
      z->numerator = atom_getfloat(argv);
      if(argc > 1)
	{
	  z->denominator = atom_getfloat(argv+1);
	  z->numgroups = 0;
	  if(scope > 1)
	    {
	      for(i = 0;i < scope - 1;i++)
		{
		  j = i + 1;
		  num = atom_getfloat(argv+(j*2));
		  div = atom_getfloat(argv+(j*2+1));
		  SETFLOAT(&z->groups[i*2],num);
		  SETFLOAT(&z->groups[i*2+1],div);
		  z->numgroups++;
		}
	    }
	}
      if(z->numerator != prevnumer || z->denominator != prevdenom || z->allcount == 0)
	{
	  outlet_float(z->n_obj, z->objects);
	  z->objects++;
	  outlet_float(z->denom, z->denominator);
	  outlet_float(z->numer, z->numerator);
	}
      z->barlen = (64 / z->denominator) * z->numerator;
    }
  z->nowgroup = 0;
  z->groupcount = 0;
  z->barcount = 0;
  z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
  z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
  z->stepdiv = 64 / z->groupdiv;
  z->grouplen = z->groupnum * z->stepdiv;
  z->grouprem = z->grouplen;
  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
  z->barrem = z->barlen;
  outlet_float(z->debugger, z->allcount*z->allstep+z->offset);
}

void gemnotes_counter_articulation(t_gemnotes_counter *z, t_floatarg art)
{
    z->articulation = art;
}

void gemnotes_counter_dyn(t_gemnotes_counter *z, t_symbol *s, int argc, t_atom *argv)
{
    float terraced = 0;
    float dynfirst = 0;
    float dynlen = 0;
    float dynlast = 0;
    float hairpin = 0;
    switch(argc)
    //      outlet_list(x->mapout, gensym("list"), x->max+1, x->x_map.nomap);

    {
        case 1:
            terraced = atom_getfloat(argv);
            SETFLOAT(&z->terrace[0], z->allcount*z->allstep+z->offset);
            SETFLOAT(&z->terrace[1], terraced);
            outlet_float(z->n_obj, z->objects);
            z->objects++;
            outlet_list(z->dyn, gensym("list"), 2, z->terrace);
            break;
        case 3:
            dynfirst = atom_getfloat(argv);
            dynlen = atom_getfloat(argv+1) * z->stepdiv * z->allstep;
            dynlast = atom_getfloat(argv+2);
            hairpin = dynfirst > dynlast ? 0 : 1;   
            SETFLOAT(&z->dynamics[0], z->allcount*z->allstep+z->offset);
            SETFLOAT(&z->dynamics[1], dynfirst);
            SETFLOAT(&z->dynamics[2], dynlen);
            SETFLOAT(&z->dynamics[3], dynlast);
            SETFLOAT(&z->dynamics[4], hairpin); //bt
            outlet_float(z->n_obj, z->objects);
            z->objects++;
            outlet_list(z->dyn, gensym("list"), 5, z->dynamics);
//            outlet_float(z->off, z->allcount*z->allstep+z->offset);
            break;
        default:
            break;            
    }
}

// new method 6-1-11
void gemnotes_counter_groups(t_gemnotes_counter *z, t_symbol *s, int argc, t_atom *argv)
{
  float argc2 = (float)argc * 0.5;
  int iargc2 = (int)argc2;
  int i,j,k;
  z->numgroups = iargc2;
  j = 0;
  k = 0;
  float num, div;
  num = 4;
  div = 16;
  if(iargc2)
    {
      for(i=0;i<iargc2;i++)
	{
	  j = i*2;
	  k = j+1;
	  num = atom_getfloat(argv+j);
	  div = atom_getfloat(argv+k);
	  SETFLOAT(&z->groups[j],num);
	  SETFLOAT(&z->groups[k],div);
	}
      if((float)iargc2 < argc2)
	{
	  z->numgroups++;
	  num = atom_getfloat(argv+(argc-1));
	  SETFLOAT(&z->groups[j],num);
	  SETFLOAT(&z->groups[k],div);
	}
    }
  else if(argc2)
    {
      num = atom_getfloat(argv);
      div = 16;
      SETFLOAT(&z->groups[j],num);
      SETFLOAT(&z->groups[k],div);
    }
  z->nowgroup = 0;
  z->groupcount = 0;
  z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
  z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
  z->stepdiv = 64 / z->groupdiv;
  z->grouplen = z->groupnum * z->stepdiv;
  z->grouprem = z->grouplen;
  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
}

void gemnotes_counter_group(t_gemnotes_counter *z, t_floatarg f1, t_floatarg f2)
{
  int ipoly, gdiv, i;
  float zpoly, polydiv;
  if(f1 && f2)
    {
      z->groupcount = 0;
      z->groupnum = f1;
      z->groupdiv = f2;
      z->stepdiv = 64 / z->groupdiv;
      z->grouplen = z->groupnum * z->stepdiv;
      z->grouprem = z->grouplen;
      z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
      z->polyrhythm = 0;
      gdiv = z->groupdiv;
      z->group_id++;
      outlet_float(z->groupid, z->group_id);
      if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
	{
	  //	  polydiv = 1;
	  zpoly = z->groupdiv;
	  ipoly = (int)zpoly;

	  for(i=0;i<7;i++)
	    {
	      //	      polydiv *= 0.5;
	      zpoly *= 0.5;
	      ipoly = (int)zpoly;
	      post("ipoly = %d, zpoly = %f",ipoly,zpoly);
	      if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
	    }
	}
    }
}

void gemnotes_counter_rest(t_gemnotes_counter *z, t_symbol *s, int argc, t_atom *argv)
{
  z->gnoterem = z->grouprem / z->stepdiv;

  float chunk, thischunk, thatchunk, prevnumer, prevdenom;
  int i, j, x;
  float zpoly, polydiv;
  int ipoly, gdiv, prevstave;
  float rdur = argc > 0 ? atom_getfloat(argv) : z->step;
  z->restlen = argc > 1 ? atom_getfloat(argv+1) : z->restlen;
  if(rdur > 0)
    {
      if(z->rested == 0 && z->prevrested == 1)
	{
	  z->tailon = z->groupobject;
	  outlet_float(z->tail, z->tailon);
	}
      z->beam = 0;
      j = 0;
      chunk = rdur;
      while(chunk > 0)
	{
	  thischunk = z->gnoterem <= chunk ? z->gnoterem : chunk;
	  thischunk = thischunk > 32 ? 32 : thischunk;
	  chunk -= thischunk;
	  i = (int)thischunk;
	  x = gemnotes_counter_splits[j][i];
	  if(x > 0)
	    {
	      /*	      prevstave = z->stavenum;
	      z->stavenum = (int)((z->allcount*z->allstep+z->offset) / z->stavelen);
	      z->fstavenum = (float)z->stavenum;
	      outlet_float(z->stave,z->fstavenum);
	      if(z->stavenum > prevstave)
		{
		  z->objects = z->initobjects;
		  z->groupobject = z->objects;
		  outlet_float(z->g_obj, z->objects);
		  z->allcount = 0;
		  } */
	      z->barlast = 0;
	      outlet_float(z->n_obj, z->objects);
	      z->objects++;
	      outlet_float(z->tuple, z->polyrhythm);
	      outlet_float(z->rhyrest, z->rhythm);
	      outlet_float(z->dur, (float)x);
	      outlet_float(z->rest, z->allcount*z->allstep+z->offset);
	      thatchunk = (float)x * z->stepdiv;
	      z->allcount += thatchunk * z->restlen;
	      z->lastdur = thatchunk * z->restlen;
	      //	      z->barrests += thatchunk * z->restlen;
	      z->barcount += thatchunk;
	      z->barrem -= thatchunk;
	      z->groupcount += (float)x;
	      thischunk -= (float)x;
	      z->grouprem -= thatchunk;
	      z->prevrested = z->rested;
	      z->rested = 1;
	    }
	  if(z->barrem < 1 && z->barlast == 0)
	    {
	      z->barlast = 1;
	      outlet_float(z->n_obj, z->objects);
	      //	      z->barpos += (z->barnotes + z->barrests) * z->allstep;
	      z->barpos = (z->allcount - (z->lastdur / 2)) * z->allstep + z->offset;
	      z->objects++;
	      outlet_float(z->bar, z->barpos);
	      z->barnotes = z->barrests = 0;
	      if(z->nextnumer)
		{
		  prevnumer = z->numerator;
		  prevdenom = z->denominator;
		  z->numerator = z->nextnumer;
		  if(z->nextdenom)
		    {
		      z->denominator = z->nextdenom;
		    }
		  if(z->numerator != prevnumer || z->denominator != prevdenom)
		    {
		      outlet_float(z->n_obj, z->objects);
		      z->objects++;
		      outlet_float(z->denom, z->denominator);
		      outlet_float(z->numer, z->numerator);
		    }
		  z->nextnumer = z->nextdenom = 0;
		  z->barlen = (64 / z->denominator) * z->numerator;
		}

	      z->nowgroup = 0;
	      z->groupcount = 0;
	      z->barcount = 0;
	      z->groupnum = atom_getfloatarg(z->nowgroup, 128, z->groups);
	      z->groupdiv = atom_getfloatarg(z->nowgroup + 1, 128, z->groups);
	      z->polyrhythm = 0;
	      gdiv = z->groupdiv;
	      if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
		{
		  //		  polydiv = 1;
		  zpoly = z->groupdiv;
		  ipoly = (int)zpoly;

		  for(i=0;i<7;i++)
		    {
		      //		      polydiv *= 0.5;
		      zpoly *= 0.5;
		      ipoly = (int)zpoly;
		      post("ipoly = %d, zpoly = %f",ipoly,zpoly);
		      if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
		    }
		}
	      z->stepdiv = 64 / z->groupdiv;
	      z->grouplen = z->groupnum * z->stepdiv;
	      z->grouprem = z->grouplen;
	      z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
	      z->barrem = z->barlen;
	    }
	  else if(z->grouprem < 1)
	    {
	      z->nowgroup = (z->nowgroup + 1) % z->numgroups;
	      z->groupcount = 0;
	      z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
	      z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
	      z->polyrhythm = 0;
	      gdiv = z->groupdiv;
	      if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
		{
		  //		  polydiv = 1;
		  zpoly = z->groupdiv;
		  ipoly = (int)zpoly;

		  for(i=0;i<7;i++)
		    {
		      //		      polydiv *= 0.5;
		      zpoly *= 0.5;
		      ipoly = (int)zpoly;
		      post("ipoly = %d, zpoly = %f",ipoly,zpoly);
		      if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
		    }
		}
	      z->stepdiv = 64 / z->groupdiv;
	      z->grouplen = z->groupnum * z->stepdiv;
	      z->grouprem = z->grouplen;
	      z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
	    }
	}
    }
    outlet_float(z->debugger, z->allcount*z->allstep+z->offset);
}

void gemnotes_counter_nextbar(t_gemnotes_counter *z, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      z->nextnumer = atom_getfloat(argv);
      if(argc > 1) z->nextdenom = atom_getfloat(argv+1);
      else z->nextdenom = z->denominator;
    }
}

void gemnotes_counter_tempo(t_gemnotes_counter *z, t_symbol *s, int argc, t_atom *argv)
{
  float thisoffset, thisbpm, thisrhythm, thatbpm, thatrhythm;
  thisrhythm = 4;
  thisoffset = z->allcount*z->allstep;
  if(argc)
    {
      thisbpm = atom_getfloat(argv);
      if(argc > 1)
	{
	  thisrhythm = atom_getfloat(argv+1);
	}
      //      else thisrhythm = atom_getfloatarg(1,2,z->temposign);
      //      thatbpm = atom_getfloatarg(0,2,z->temposign);
      //      thatrhythm = atom_getfloatarg(1,2,z->temposign);
      //      if(thisbpm != thatbpm || thisrhythm != thatrhythm)
      //	{
      outlet_float(z->n_obj, z->objects);
      z->objects++;
      //      if(z->allcount == 0)
      SETFLOAT(&z->temposign[0],thisoffset);
      //      else if (z->allcount > 0) SETFLOAT(&z->temposign[0],z->allcount*z->allstep+z->offset);
      SETFLOAT(&z->temposign[1],thisbpm);
      SETFLOAT(&z->temposign[2],thisrhythm);
      outlet_list(z->bpm, gensym("list"), 3, z->temposign);
      //	}
    }
  post("thisoffset = %f", thisoffset);
}

void gemnotes_counter_debug(t_gemnotes_counter *z, t_floatarg f)
{
  post("numer = %d,denom = %d,groupnum = %d,groupdiv = %d",(int)z->numerator,(int)z->denominator,(int)z->groupnum,(int)z->groupdiv);
  post("allcount = %d, barcount = %d, groupcount = %d",(int)z->allcount,(int)z->barcount,(int)z->groupcount);
  post("barlen = %d,barrem = %d,grouplen = %d,grouprem = %d",(int)z->barlen,(int)z->barrem,(int)z->grouplen,(int)z->grouprem);
  int i = f;
  int j, x;
  for(j=0;j<6;j++)
    {
      x = gemnotes_counter_splits[j][i];
      post("j = %d, i = %d, x = %d",j,i,x);
    }
}

void gemnotes_counter_step(t_gemnotes_counter *z, t_floatarg f)
{
  z->step = f;
  z->fractional = ((int)(z->step * 100) % 100) != 0 ? 1 : 0;
}

void gemnotes_counter_offset(t_gemnotes_counter *z, t_floatarg f)
{
  if(f)
    {
      z->offset += f;
    }
  else if(!f)
    {
      z->offset = z->initoffset;
    }
}

void gemnotes_counter_chord(t_gemnotes_counter *z)
{
    outlet_float(z->artic, z->articulation);
    z->articulation = 0;
  outlet_float(z->n_obj, z->objects);
  z->objects++;
  outlet_bang(z->slave);
}

void gemnotes_counter_hdsq(t_gemnotes_counter *z, t_floatarg f)
{
  if(f)
    {
        z->offset += z->allcount * z->allstep;
        z->allstep = f;
        z->allcount = 0;
    }
  else if(!f)
    {
        z->offset += z->allcount * z->allstep;
      z->allstep = 0.15;
        z->allcount = 0;
    }
}

void gemnotes_counter_reset(t_gemnotes_counter *z)
{
  int gdiv, ipoly, i;
  float zpoly, polydiv;
  z->group_id = 0;
  z->allcount = 0;
  z->nowgroup = 0;
  z->groupcount = 0;
  z->barcount = 0;
  z->barnotes = z->barrests = z->barpos = 0;
  z->offset = z->initoffset;
  z->objects = z->initobjects;
  z->groupnum = atom_getfloatarg(z->nowgroup * 2, 128, z->groups);
  z->groupdiv = atom_getfloatarg(z->nowgroup * 2 + 1, 128, z->groups);
  z->polyrhythm = 0;
  gdiv = z->groupdiv;
  if(gdiv != 64 && gdiv != 32 && gdiv != 16 && gdiv != 8 && gdiv != 4 && gdiv != 2 && gdiv != 1)
    {
      polydiv = 1;
      zpoly = z->groupdiv;
      ipoly = (int)zpoly;

      for(i=0;i<7;i++)
	{
	  polydiv *= 0.5;
	  zpoly *= 0.5;
	  ipoly = (int)zpoly;
	  if(fabs(zpoly - (float)ipoly) < 0.1) z->polyrhythm = zpoly;
	}
    }
  z->stepdiv = 64 / z->groupdiv;
  z->grouplen = z->groupnum * z->stepdiv;
  z->grouprem = z->grouplen;
  z->rhythm = z->groupdiv >= 64 ? 64 : z->groupdiv >= 32 ? 32 : z->groupdiv >= 16 ? 16 : z->groupdiv >= 8 ? 8 : z->groupdiv >= 4 ? 4 : z->groupdiv >= 2 ? 2 : 1;
  z->barrem = z->barlen;
  //  z->stavenum = 0;
  //  outlet_float(z->stave,0);
  outlet_float(z->debugger, z->allcount);
}

void *gemnotes_counter_new(t_symbol *s, int argc, t_atom *argv)
{
  t_gemnotes_counter *z = (t_gemnotes_counter *)pd_new(gemnotes_counter_class);
  int i;
  for(i=0;i<128;i++)
    {
      SETFLOAT(&z->groups[i],0);
    }
  SETFLOAT(&z->groups[0],4);
  SETFLOAT(&z->groups[1],16);

  SETFLOAT(&z->temposign[0],0);
  SETFLOAT(&z->temposign[1],120);
  SETFLOAT(&z->temposign[2],4);

  z->group_id = 0;
  z->numgroups = 1;
  z->nowgroup = 0;
  //  z->lastgroup = 0;
  z->stepdiv = 4;
  z->allstep = 0.15;
  z->allcount = z->barcount = z->groupcount = 0;
  z->step = 1;
  z->barrem = z->barlen = 48;

  z->grouprem = z->grouplen = 16;
  z->groupnum = 4;
  z->groupdiv = 16;

  z->barnotes = 0;
  z->barrests = 0;
  z->barpos = 0;
  z->barlast = 0;
  z->restlen = 0.5;

  z->beam = z->tied = 0;
  z->rhythm = 16;

  z->numerator = argc > 0 ? atom_getfloat(argv) : 3;
  z->denominator = argc > 1 ? atom_getfloat(argv+1) : 4;
  z->nextnumer = z->nextdenom = 0;

  z->initobjects = argc > 2 ? atom_getfloat(argv+2) : 0;
  z->objects = z->initobjects;
  z->initoffset = argc > 3 ? atom_getfloat(argv+3) : 0;
  z->offset = z->initoffset;
  z->groupobject = -1;
  z->stavelen = argc > 4 ? atom_getfloat(argv+4) : 18;
  z->stavenum = 0;
  z->fstavenum = 0;
    z->articulation = 0;

  z->off = outlet_new(&z->x_obj, gensym("float"));
  z->rhy = outlet_new(&z->x_obj, gensym("float"));
  z->dur = outlet_new(&z->x_obj, gensym("float"));
  z->grp = outlet_new(&z->x_obj, gensym("float"));
  z->gcount = outlet_new(&z->x_obj, gensym("float"));
  z->tie = outlet_new(&z->x_obj, gensym("float"));
  z->bm = outlet_new(&z->x_obj, gensym("float"));
  z->rest = outlet_new(&z->x_obj, gensym("float"));
  z->rhyrest = outlet_new(&z->x_obj, gensym("float"));
  z->bar = outlet_new(&z->x_obj, gensym("float"));
  z->slave = outlet_new(&z->x_obj, gensym("bang"));
  z->numer = outlet_new(&z->x_obj, gensym("float"));
  z->denom = outlet_new(&z->x_obj, gensym("float"));
  z->tail = outlet_new(&z->x_obj, gensym("float"));
  z->tuple = outlet_new(&z->x_obj, gensym("float"));
  z->bpm = outlet_new(&z->x_obj, gensym("list"));
  z->g_obj = outlet_new(&z->x_obj, gensym("float"));
  z->n_obj = outlet_new(&z->x_obj, gensym("float"));
  z->debugger = outlet_new(&z->x_obj, gensym("float"));
  z->groupid = outlet_new(&z->x_obj, gensym("float"));
    z->artic = outlet_new(&z->x_obj, gensym("float"));
    z->dyn = outlet_new(&z->x_obj, gensym("list"));
  //  z->stave = outlet_new(&z->x_obj, gensym("float"));
  return(void *)z;
}

void gemnotes_counter_setup(void) 
{
  gemnotes_counter_class = class_new(gensym("gemnotes_counter"),
  (t_newmethod)gemnotes_counter_new,
  0, sizeof(t_gemnotes_counter),
  0, A_GIMME, 0);
  post("gemnotes_counter - segmented all / bar / beamgroup counter...");
  post("...for dynamic object creation of musical scores.");
  post("version 0.6...by Ed Kelly 2012 - morph_2016@yahoo.co.uk");

  class_addbang(gemnotes_counter_class, gemnotes_counter_bang);
  class_addfloat(gemnotes_counter_class, gemnotes_counter_float);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_step, gensym("step"), A_DEFFLOAT, 0);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_reset, gensym("reset"), A_DEFFLOAT, 0);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_offset, gensym("offset"), A_DEFFLOAT, 0);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_hdsq, gensym("hdsq"), A_DEFFLOAT, 0);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_chord, gensym("chord"), A_DEFFLOAT, 0);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_rest, gensym("rest"), A_GIMME, 0);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_bar, gensym("bar"), A_GIMME, 0);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_tempo, gensym("tempo"), A_GIMME, 0);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_group, gensym("group"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_debug, gensym("debug"), A_DEFFLOAT, 0);
    class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_nextbar, gensym("nextbar"), A_GIMME, 0);
    class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_dyn, gensym("dyn"), A_GIMME, 0);
    class_addmethod(gemnotes_counter_class, (t_method)gemnotes_counter_articulation, gensym("articulation"), A_DEFFLOAT, 0);
}
