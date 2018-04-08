 CC = gcc
# CC = arm-linux-gnueabi-gcc

#TOOLCHAIN = /opt/poky/1.5/sysroots/i686-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi

#CC = $(TOOLCHAIN)/arm-poky-linux-gnueabi-gcc

#AR = $(TOOLCHAIN)/arm-poky-linux-gnueabi-ar

#RANLIB = $(TOOLCHAIN)/arm-poky-linux-gnueabi-ranlib

# CC_OPTIONS = -I../ -O2 -g3 -Wall -c -fmessage-length=0 -march=armv7-a -mfpu=neon -ftree-vectorize -mfloat-abi=softfp
CC_OPTIONS = -I../ -O2 -g3 -Wall -c -fmessage-length=0 -march=corei7-avx

#CC_OPTIONS = -I../ -I/opt/poky/1.5/sysroots/cortexa9-vfp-neon-poky-linux-gnueabi/usr/include -O3 -Wall -c -fmessage-length=0 -march=armv7-a -mfpu=neon -ftree-vectorize -mfloat-abi=softfp -ffast-math --sysroot=/opt/poky/1.5/sysroots/cortexa9-vfp-neon-poky-linux-gnueabi

 LIBS = -lstdc++ -lpthread -lm

OBJ = common.o mult-thread.o data-interface.o hash-list.o unordered-set.o vector.o wfst.o word-symbol.o  cstring.o hmm.o fft.o cmvn.o compute-filter-bank.o compute-mel-bank.o extract-feature.o wave-reader.o component.o net-base.o net.o net-input.o net-interface.o prior.o dnn-decodable-interface.o faster-decoder.o feature-input.o online-faster-decoder.o asr-container.o asr.o engine.o yt_asr_option.o

# main-engine : $(OBJ)
#	$(CC) -o main-engine $(OBJ) $(LIBS)

libyoungtone.a :  $(OBJ)
	ar cru libyoungtone.a $(OBJ)
	ranlib libyoungtone.a
	chmod 775 libyoungtone.a
	mv libyoungtone.a ..
	rm *.o

# base
common.o : ../base/common.cc
	$(CC) $(CC_OPTIONS) ../base/common.cc

# util
mult-thread.o : ../util/mult-thread.cc
	$(CC) $(CC_OPTIONS) ../util/mult-thread.cc

data-interface.o : ../util/data-interface.cc
	$(CC) $(CC_OPTIONS) ../util/data-interface.cc

hash-list.o : ../util/hash-list.cc
	$(CC) $(CC_OPTIONS) ../util/hash-list.cc

unordered-set.o : ../util/unordered-set.cc
	$(CC) $(CC_OPTIONS) ../util/unordered-set.cc

vector.o : ../util/vector.cc
	$(CC) $(CC_OPTIONS) ../util/vector.cc

wfst.o : ../util/wfst.cc
	$(CC) $(CC_OPTIONS) ../util/wfst.cc

word-symbol.o : ../util/word-symbol.cc
	$(CC) $(CC_OPTIONS) ../util/word-symbol.cc

cstring.o : ../util/cstring.cc
	$(CC) $(CC_OPTIONS) ../util/cstring.cc

# hmm
hmm.o : ../hmm/hmm.cc
	$(CC) $(CC_OPTIONS) ../hmm/hmm.cc

# math
fft.o : ../math/fft.cc
	$(CC) $(CC_OPTIONS) ../math/fft.cc

# feature
cmvn.o : ../feature/cmvn.cc
	$(CC) $(CC_OPTIONS) ../feature/cmvn.cc

compute-filter-bank.o : ../feature/compute-filter-bank.cc
	$(CC) $(CC_OPTIONS) ../feature/compute-filter-bank.cc

compute-mel-bank.o : ../feature/compute-mel-bank.cc
	$(CC) $(CC_OPTIONS) ../feature/compute-mel-bank.cc

extract-feature.o : ../feature/extract-feature.cc
	$(CC) $(CC_OPTIONS) ../feature/extract-feature.cc

wave-reader.o : ../feature/wave-reader.cc
	$(CC) $(CC_OPTIONS) ../feature/wave-reader.cc

# net
component.o : ../net/component.cc
	$(CC) $(CC_OPTIONS) ../net/component.cc

net-base.o : ../net/net-base.cc
	$(CC) $(CC_OPTIONS) ../net/net-base.cc

net.o : ../net/net.cc
	$(CC) $(CC_OPTIONS) ../net/net.cc

net-input.o : ../net/net-input.cc
	$(CC) $(CC_OPTIONS) ../net/net-input.cc

net-interface.o : ../net/net-interface.cc
	$(CC) $(CC_OPTIONS) ../net/net-interface.cc

prior.o : ../net/prior.cc
	$(CC) $(CC_OPTIONS) ../net/prior.cc

# decode
dnn-decodable-interface.o : ../decode/dnn-decodable-interface.cc
	$(CC) $(CC_OPTIONS) ../decode/dnn-decodable-interface.cc

faster-decoder.o : ../decode/faster-decoder.cc
	$(CC) $(CC_OPTIONS) ../decode/faster-decoder.cc

feature-input.o : ../decode/feature-input.cc
	$(CC) $(CC_OPTIONS) ../decode/feature-input.cc

online-faster-decoder.o : ../decode/online-faster-decoder.cc
	$(CC) $(CC_OPTIONS) ../decode/online-faster-decoder.cc

# engine
asr-container.o : ../engine/asr-container.cc
	$(CC) $(CC_OPTIONS) ../engine/asr-container.cc

engine.o : ../engine/engine.cc
	$(CC) $(CC_OPTIONS) ../engine/engine.cc

asr.o : ../engine/asr.cc
	$(CC) $(CC_OPTIONS) ../engine/asr.cc

yt_asr_option.o : ../engine/yt_asr_option.cc
	$(CC) $(CC_OPTIONS) ../engine/yt_asr_option.cc

clean :
	rm -rf *.o ../*.a
