#include <stdlib.h>
#include <stdio.h>
#include "../my_vm.h"

int main() {
    // set_physical_mem();

    // int inner_bits = 10;
    // int outter_bits = 10;

    // unsigned long PD_index = 0b0000000010;  // 10 in binary
    // unsigned long PT_index = 0b0000000010;  // 10 in binary
    // unsigned long offset = 0b000000000000000000000000;  // 12 in binary

    // // Construct the 32-bit virtual address
    // unsigned long virtualAddress = (PD_index << (offset + inner_bits)) |
    //                                (PT_index << offset) |
    //                                offset;

    // // Convert the virtual address to a void* pointer
    // void* virtualAddressPtr = (void*)virtualAddress;

    // translate(0, virtualAddressPtr);
    // void *pointers[15];
    // int alloc_size = 10000;

    // for(int i = 0; i<10; i++)
    // {
    //     pointers[i] = t_malloc(alloc_size);
    // }
    // int cop = 911;
    // for(int i=0; i< 10; i++)
    // {
    //     cop += 1;
    //     int val = put_value(pointers[i], &cop, sizeof(cop));
    //      if(val == -1)
    //         printf("Failure!!!!");

    //     int cop_no = 0;
    //     get_value(pointers[i], &cop_no, sizeof(cop_no));
    //     printf("getting value %d \n", cop_no);
    // }

    // for(int i = 0; i<10; i++)
    // {
    //     t_free(pointers[i], alloc_size);
    // }

    void *a = t_malloc(400);
    void *b = t_malloc(500);
    void *c = t_malloc(600);

    int old_a = (int)a;

    t_free(a, 400);
    t_free(b, 500);
    t_free(c, 600);

    a = t_malloc(400);
    if ((int)a == old_a)
        printf("free function works\n");

    
    

    printf("hey this is TA\n");
}