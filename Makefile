all: fix_sized_allocator_test
	./fix_sized_allocator_test

CCFLAGS = -I include -O3 -c -std=c++14 

fix_sized_allocator.o: include/quick/fix_sized_allocator.hpp fix_sized_allocator.cpp
	g++ $(CCFLAGS) fix_sized_allocator.cpp -o fix_sized_allocator.o

fix_sized_allocator_test.o: include/quick/fix_sized_allocator.hpp fix_sized_allocator_test.cpp
	g++ $(CCFLAGS) fix_sized_allocator_test.cpp -o fix_sized_allocator_test.o

fix_sized_allocator_test: fix_sized_allocator_test.o fix_sized_allocator.o
	g++ fix_sized_allocator_test.o fix_sized_allocator.o -o fix_sized_allocator_test

clean:
	rm *.o fix_sized_allocator_test
