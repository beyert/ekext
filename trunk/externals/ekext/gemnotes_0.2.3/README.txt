Welcome to the first ever public release of Gemnotes.

Gemnotes is a system of realtime music notation rendering using Pure Data and
Gem.

This software is released under the BSD license.

The system includes one external - gemnotes_counter. Source code is included and released under the simple BSD license enclosed with the code. There are binaries for Linux and Mac OS X (10.6 Intel) compiled against Pure Data 0.42-5 in the ext/ folder. They should work with the examples, without any installation procedure. If you want to experiment with the gemnotes counter directly, you can call it by saving a patch in this folder then typing ext/gemnotes_counter in an object box.

There are two other externals: polyquant is a polyrhythmic quantizer, and gemnotes_barcount is used in translate_midi.pd, a patch that attempts to create gemnotes compatible scores from standard MIDI files.

Installation:
Linux:

cd /path/to/gemnotes-0.2/src
make && sudo make install

Or use the binaries in the /ext folder:
cd /path/to/gemnotes-0.2/ext
sudo cp *.pd_linux /usr/lib/pd/extra

Mac OS X:

You can compile the externals yourself using gcc if you have XCode installed following the steps above for Linux.
If you want to try the binaries in Pd-extended, right-click on Pd-extended and "show package contents" then copy the files from the /ext folder into Contents/Resources/extra

Gemnotes uses dynamic patching to generate music notation based on TrueType Fonts (TTF) and 3D primitives.

There are many limitations with the current system, and errors often occur. This project has a simple score language associated with the makevoice.pd (dynamic patching) abstraction and the gemnotes_counter object. Patches can also be made as static elements, and staves can be turned on and off. 

If you have a modification of the code you wish to share with me, post to the pd-list (after joining the list) or write to morph_2016@yahoo.co.uk

Changes for version 0.2

1. Dynamics from ppp to fff are implemented, and hairpins (crescendo/decrescendo marks)
2. Basic articulation (accent) may be added to a note.

Changes between PDCON 2011 and the 0.1 release:

1. The stave/note math is decoupled from the Gemhead
2. Object IDs are implemented so that reclocking of the stave math happens as a new object is placed, rather than all the time.

------------- To Do for version 0.3 -------------

More articulations
More clefs
Live input using gemnotes_barcount and polyquant
