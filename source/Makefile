# Generated automatically from Makefile.in by configure.
############################################################################
## ISO MPEG Audio Subgroup Software Simulation Group (1996)
## ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
##
## $Id: Makefile,v 1.1.1.1 2002/07/25 06:04:36 Exp $
##
## Makefile for decoder. Requires GNU make.
##
## $Log: Makefile,v $
##
## Revision 1.2  1996/03/28 02:06:41  
## Added test script
##
## Revision 1.1  1996/02/14 05:19:13  
## Initial revision
##
## 
############################################################################

CC = gcc

c_sources = \
	common.c \
	decode.c \
	huffman.c \
	ieeefloat.c \
	voices.c \
	portableio.c

OBJ = $(c_sources:.c=.o)
DEP = $(c_sources:.c=.d)

NINT_SWITCH = 

CC_SWITCHES = -g -O -DUNIX -DBS_FORMAT=BINARY $(NINT_SWITCH) -DNDEBUG

PGM = voices 

LIBS =  -lm


%.o: %.c 
	$(CC) $(CC_SWITCHES) -c $< -o $@

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CC_SWITCHES) $< | sed '\''s/$*.o/& $@/g'\'' > $@'


$(PGM):	$(OBJ) Makefile
	$(CC) -o $(PGM) $(OBJ) $(LIBS)
	mkdir ../bin
	mv $(PGM) ../bin/$(PGM)

clean:
	-rm $(OBJ) $(DEP)

tags: TAGS

TAGS: ${c_sources}
	etags -T ${c_sources}

-include $(DEP)


test/%_l.pcm test/%_r.pcm : test/%.mpg.dec
	multi_to_mono test/$*.mpg.dec test/$*_l.pcm test/$*_r.pcm
	absdiff test/$*_l.pcm test/pcm/$*_l.pcm
	absdiff test/$*_r.pcm test/pcm/$*_r.pcm

test/%_I.pcm test/%_II.pcm : test/%.mpg.dec
	multi_to_mono test/$*.mpg.dec test/$*_I.pcm test/$*_II.pcm
	absdiff test/$*_I.pcm test/pcm/$*_I.pcm
	absdiff test/$*_II.pcm test/pcm/$*_II.pcm

test/%.pcm : test/%.mpg.dec
	cp test/$*.mpg.dec test/$*.pcm
	absdiff test/$*.pcm test/pcm/$*.pcm

%.mpg.dec : %.mpg decode
	decode $*.mpg

test: \
	test/test24_l.pcm \
	test/test25_l.pcm \
	test/test26_l.pcm \
	test/test27_l.pcm \
	test/test28_l.pcm \
	test/test29_l.pcm \
	test/test30.pcm \
	test/test31_I.pcm \
	test/test32_l.pcm \
	test/test33_l.pcm

