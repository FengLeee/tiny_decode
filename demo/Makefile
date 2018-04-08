CC = gcc

#TOOLCHAIN = /opt/poky/1.5/sysroots/i686-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi

#CC = $(TOOLCHAIN)/arm-poky-linux-gnueabi-gcc
CC_OPTIONS = -g -msse -msse2 -Wall -fPIC   -Wno-unused-function -rdynamic  -std=c99
#CC_OPTIONS = -I/opt/poky/1.5/sysroots/cortexa9-vfp-neon-poky-linux-gnueabi/usr/include -Wall -O3 -std=c99 -march=armv7-a -mfpu=neon -ftree-vectorize -mfloat-abi=softfp -ffast-math --sysroot=/opt/poky/1.5/sysroots/cortexa9-vfp-neon-poky-linux-gnueabi
LIBS = -L../asr-engine -lyoungtone -lm -lpthread -lstdc++ 
#LIBS = -L/opt/poky/1.5/sysroots/cortexa9-vfp-neon-poky-linux-gnueabi/usr/lib -L../asr-engine -lyoungtone -lm -lpthread -lstdc++ -L/opt/poky/1.5/sysroots/cortexa9-vfp-neon-poky-linux-gnueabi -L/opt/poky/1.5/sysroots/cortexa9-vfp-neon-poky-linux-gnueabi/usr/lib/alsa-lib

main : wave-reader.o main.o
	$(CC) $(CC_OPTIONS)  wave-reader.o main.o -o main $(LIBS)
	rm *.o -rf

main.o : main.c
	$(CC) $(CC_OPTIONS) -o main.o -c main.c

wave-reader.o : wave-reader.c
	$(CC) $(CC_OPTIONS) -o wave-reader.o -c wave-reader.c

clean :
	rm -rf *.o main