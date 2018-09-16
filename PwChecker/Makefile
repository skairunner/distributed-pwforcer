cc=gcc
opt=-Wall -std=c99

all: lib/libpwchecker.a

obj/sha2_obj.o: SHA2/sha2.c
	mkdir -p obj
	$(cc) -o obj/sha2_obj.o -c SHA2/sha2.c -ISHA2 $(opt)

obj/sha2_wrapper.o: sha2_wrapper.c
	mkdir -p obj
	$(cc) -o obj/sha2_wrapper.o -c sha2_wrapper.c -I. -I./SHA2 $(opt)

obj/pw_checker.o: pw_checker.c
	mkdir -p obj
	$(cc) -o obj/pw_checker.o -c pw_checker.c -I. -I./SHA2 $(opt)

lib/libpwchecker.a: obj/pw_checker.o obj/sha2_obj.o obj/sha2_wrapper.o
	mkdir -p lib
	ar cr lib/libpwchecker.a obj/pw_checker.o obj/sha2_obj.o obj/sha2_wrapper.o

clean:
	rm -rf lib/* obj/*
