OBJS = utils.o ctr_crypto.o main.o extdata.o aesmac.o vsxe.o titledb.o
POLAR_OBJS = polarssl/aes.o polarssl/bignum.o polarssl/rsa.o polarssl/sha2.o
LIBS = -static-libgcc -static-libstdc++
CXXFLAGS = -I.
CFLAGS = --std=c99 -Wall -I.
OUTPUT = bin/extdata_tool
CC = gcc

main: $(OBJS) $(POLAR_OBJS)
	g++ -o $(OUTPUT) $(LIBS) $(OBJS) $(POLAR_OBJS)


clean:
	rm -rf $(OUTPUT) $(OBJS) $(POLAR_OBJS)