
all : test
test: ../my_vm.h
	gcc test.c -L../ -lmy_vm -m32 -lm -o test
	gcc multi_test.c -L../ -lmy_vm -m32 -lm -o mtest -lpthread 

ours: ../my_vm.h
	gcc ourtest.c -L../ -lmy_vm -m32  -lm -o ourtest

clean:
	rm -rf test mtest ourtest

