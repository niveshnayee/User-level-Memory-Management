#include "my_vm.h"
#include "math.h"
#include <string.h>

//----------------  Global Variables --------
void* physicalMem;
char* physicalBitmap;
char* virtualBitmap;

int offset;
int inner_bits;
int outter_bits;
int lenPhysicalBitmap;
int lenVirtualBitmap;
int numPhysicalPages;

int tlb_lookup = 0;
int tlb_miss = 0;

int firstCall = 0;


pthread_mutex_t mutex;
// pthread_mutex_t Pbitmap;


// struct tlb tlb_store;

struct tlb tlb_entries[TLB_ENTRIES];



// ---------------- Helper Functions ----------------

static void set_bit_at_index(char *bitmap, int index)
{
    //in our 4 KB page size case, lenPhysicalBitmap is 262144. There are 262144 PPNs
    int bitmapIDX = lenPhysicalBitmap - (index/8) - 1;
    int bitPos = index % 8;

    bitmap[bitmapIDX] |= (1 << bitPos);

    return;
}

static void unset_bit_at_index(char *bitmap, int index)
{
    //in our 4 KB page size case, lenPhysicalBitmap is 262144. There are 262144 PPNs
    int bitmapIDX = lenPhysicalBitmap - (index/8) - 1;
    int bitPos = index % 8;
    int bit_mask = 0b11111111;
    bit_mask -= (1 << bitPos);
    
    bitmap[bitmapIDX] &= (bit_mask);

    return;
}

/* 
 * Function 3: GETTING A BIT AT AN INDEX 
 * Function to get a bit at "index"
 */
static int get_bit_at_index(char *bitmap, int index)
{
    //Get to the location in the character bitmap array
    //Implement your code here
    int bitmapIDX = lenPhysicalBitmap - (index/8) - 1;
    int bitPos = index % 8;


    return (bitmap[bitmapIDX] & (1 << bitPos)) >> bitPos;
}

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating

    // need to determine the offset, highOrderBits
    offset = log2(PGSIZE);
    int virtual_page_number = 32 - offset; // both included outter and inner bits


    //no. of PTE per page = Page Size / PTE size
    // inner bits  = log2(PTE per page)
    inner_bits = log2(PGSIZE/sizeof(pte_t));
    outter_bits = virtual_page_number - inner_bits; // rest are the outtter bits

    //int num_entries;
    int num_entries = 1 << outter_bits;  // num of entries = num of virtual pages
    int page_table_size = num_entries * sizeof(pte_t); // 4KB 
    
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
    physicalMem = (void*)malloc(MEMSIZE);
    
    //physical bitmap is used to determine if a physical page is allocated or not
    //physical bitMap calculation: 
        //number of physical pages = size of physical memory / page size
        //need a bit for each physical page. 1 character is 8 bits, so we need to allocate (numPhysicalPages/8) characters
    numPhysicalPages = MEMSIZE/PGSIZE;
    lenPhysicalBitmap = numPhysicalPages/8;
    physicalBitmap = (char*)malloc(lenPhysicalBitmap); //fix this line to compute based on the constants in the .h file

     // Initializing physical bitmap
    
    for(int i = 0; i < sizeof(physicalBitmap); i++){
        physicalBitmap[i] = 0;
    }

    // int numVirtualPages = MAX_MEMSIZE/PGSIZE;
    // //There are 8 bits in 1 byte
    // virtualBitmap = (char *)malloc(numVirtualPages/8);

    // // Initializing virtual bitmap
    // for(int i = 0; i < sizeof(virtualBitmap)/sizeof(virtualBitmap[0]); i++){
    //     virtualBitmap[i] = 0;
    // }

    // // need to allocate one page Directory in phy mem
    // for(int i=0; i <MEMSIZE; i += sizeof(pde_t))  //Changed incrementation to sizeof(pde_t) cuz we storing the value as that
    // {
    //     // unsigned long val = 0;
    //     ((pde_t*)physicalMem)[i / sizeof(pde_t)]= 0; //if page directory entry value is 0, means there is no corresponding page table
    //     // physicalMem+i = &val;
    // }
    memset(physicalMem, 0, MEMSIZE);

    //set the last bit of physical bitmap because we have allocated page directory to PPN 1 in our physical memory
    set_bit_at_index(physicalBitmap,0);


    for (int i = 0; i < TLB_ENTRIES; ++i) {
        tlb_entries[i].tag = (unsigned long)0; // Set an appropriate value for the VPN (Virtual Page Number)
        tlb_entries[i].pa = (unsigned long)0;  // Set an appropriate value for the corresponding physical address
    }
    
    
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(unsigned long vpn, unsigned long *pa)
{

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    int idx = vpn % TLB_ENTRIES;

    tlb_entries[idx].tag = vpn;
    tlb_entries[idx].pa = pa;

    return 0;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(unsigned long vpn) {

    /* Part 2: TLB lookup code here */
    if(tlb_entries[vpn % TLB_ENTRIES].tag == vpn)
    {
        pte_t* pa = tlb_entries[vpn % TLB_ENTRIES].pa;
        return pa;
    }
    else
    {
        return NULL;
    }
        
    


   /*This function should return a pte_t pointer*/
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/
    miss_rate = (tlb_miss * 1.0)/(tlb_lookup * 1.0);

    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    */

    //should come back to this and confirm that the casting all works
    unsigned long vpn = (unsigned long)va >> offset;

    //Checking if the tlb entry has the pa.
    tlb_lookup++;
    unsigned long* pa = check_TLB(vpn);

    if(pa != NULL)
    {
        return pa;
    }

    tlb_miss++;

    unsigned long PD_index = (unsigned long)va >> (offset + inner_bits);
    unsigned long bit_mask = (unsigned long)va << outter_bits;
    unsigned long PT_index = bit_mask >> (offset + outter_bits);

    bit_mask = (unsigned long)va << (outter_bits + inner_bits);
    unsigned long offset_value = bit_mask >> (outter_bits + inner_bits);

    if(((unsigned long*)physicalMem)[PD_index] == 0)
    {
        return NULL;
    }
        
    else
    {

        // assuming that value inside the PD will have the exact index location of PM where the PT starts
        // PT_start_index is the exact byte in physical memory where the page table starts
        pte_t PT_start_index = ((unsigned long*)physicalMem)[PD_index]; 

        //  with PT_start, get the index of targeted page
        pte_t PTE_index = PT_start_index + (PT_index * sizeof(pte_t));// changed sizeof(pte_t) from 4, to be precise

         // the value at that index will be the address of the target page in unsigned long
        if( ((unsigned long*)physicalMem)[PTE_index] == 0)
        {
            return NULL;
        }

        unsigned long target_page_address = ((unsigned long*)physicalMem)[PTE_index];
        //add the offset to get to the exact location
        unsigned long* targetAddress = (unsigned long*)((void*)target_page_address + offset_value);

        add_TLB(vpn, targetAddress);

        return targetAddress;

    } 
    /* Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */

    //If translation not successful, then return NULL
    return NULL; 
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pa, unsigned long *arr)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    unsigned long vpn = (unsigned long)va >> offset;

   

    unsigned long PD_index = (unsigned long)va >> (offset + inner_bits);
    unsigned long bit_mask = (unsigned long)va << outter_bits;
    unsigned long PT_index = bit_mask >> (offset + outter_bits);

    if(((unsigned long*)physicalMem)[PD_index] == 0) //page table is not allocated in the PDE
    {
        int allocated  = -1;
        // create new page table in Physical memory and then add the starting index to this page directory
        for(int i = 1; i < numPhysicalPages; i++)
        {
            if(get_bit_at_index(physicalBitmap, i) == 0)
            {
                //this will give byte index in physical memory of starting index of the page table we want
                pte_t PT_start_index = (pte_t)(i * PGSIZE);

                for(int i = 0; i< sizeof(arr) / sizeof(arr[0]); i++)
                {
                    pte_t* page_address = (pte_t*)&(((unsigned long*)physicalMem)[PT_start_index/4]);
                    if(page_address == (pte_t*)(arr[i])) //confirm checking right comparison.
                        continue;
                    
                }
                // assign the PT starting index to the page directory
                ((unsigned long*)physicalMem)[PD_index] = PT_start_index;
                
                //  with PT_start, get the index of targeted page
                unsigned long target_index = PT_start_index + (PT_index * sizeof(pte_t));// changed sizeof(pte_t) from 4, to be precise

                ((unsigned long*)physicalMem)[target_index] = (unsigned long)pa; // assigning pa is the address of the targeted page

                set_bit_at_index(physicalBitmap, i);

                void* start = physicalMem;
                int index =  (pa-start)/PGSIZE;
                set_bit_at_index(physicalBitmap ,index);


                allocated = 0;

                add_TLB(vpn, pa);
                
                break;
            }
        }
        return allocated;
    }
    else
    { //page table is already allocated in the PDE

        // assuming that value inside the PD will have the exact index location of PM where the PT start
        pte_t PT_start = ((unsigned long*)physicalMem)[PD_index];// changed sizeof(pde_t) from 4, to be precise

        //  with PT_start, get the index of targeted page
        pte_t target_index = PT_start + (PT_index * sizeof(pte_t));// changed sizeof(pte_t) from 4, to be precise
        ((unsigned long*)physicalMem)[target_index] = (pte_t)pa; // assigning pa is the address of the targeted page

        void* start = physicalMem;
        int index =  (pa-start)/PGSIZE;

        set_bit_at_index(physicalBitmap ,index);

        add_TLB(vpn, pa);
        return 0;
    } 

    return -1;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) 
{
    unsigned long** arr = malloc(sizeof(unsigned long**) * num_pages);
    int idx = 0;
    
    
    for(int i = 1; i < numPhysicalPages; i++)
    {
        if(num_pages == 0)
            break;
        if(get_bit_at_index(physicalBitmap, i) == 0)
        {
            pte_t* page_address = (pte_t*)&(((unsigned long*)physicalMem)[i * PGSIZE/4]); // i is the PPN, need to find the actual index in PM array

            arr[idx] = page_address;
            idx++;
            //Might need to set_bit_at_index here for synchronization purpose
            num_pages--;
        }
    }

    if(num_pages != 0)
    {

        return NULL;
    }
    
    return arr;
    
    //Use virtual address bitmap to find the next free page
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *t_malloc(unsigned int num_bytes) {
    // first we finding the avail free physical pages for no. of bytes user requested (will get the array of PA of free pages)
    // second, find consecutive PTE for the VA to return 
    // third, call page map to map the PA's to the consecutive VA's(consecutive PTE's)
    // return first VA 

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */
    pthread_mutex_lock(&mutex);

    if(firstCall == 0)
    {
        set_physical_mem();
        firstCall = 1;
    }

    int num_pages = ceil((double)num_bytes/PGSIZE); //if 2.5 pages,we should allocate 3 as 2 will be less.

    unsigned long* arr = get_next_avail(num_pages);
    if(arr == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return "ERROR";
    }



    // need to change it to int and set to -1 to find if it allocated correctly or not 
    int consecutiveCount = 0;
    unsigned long PD_index_va = 0;
    unsigned long PT_index_va = 0;
    unsigned long free_PDE = 0;

    for(int i = 0; i < PGSIZE/pde_t; i++) // outer loop for walking the PD
    {
        // if the PD maps to PT then will go to this if stat
        if( ((unsigned long*)physicalMem)[i] != 0 ) 
        {
            pte_t PT_start_index = (pte_t)(((unsigned long*)physicalMem)[i]);

            // inner loop for walking the PT
            for(int j = (int)(PT_start_index); j < (PT_start_index + (PGSIZE/sizeof(pte_t))); j++)
            {
                pte_t pageTableEntry = ((pte_t*)physicalMem)[j];

                if (pageTableEntry == 0) {
                    // printf("  Page Table Entry %d: %lu\n", j / sizeof(pte_t), pageTableEntry);
                    if(PT_index_va == 0)
                        PT_index_va = (unsigned long)j; // gets the index or exact bytes location of PT index for va
                    
                    // Increment the consecutive counter
                    consecutiveCount++;

                    // Check if enough consecutive entries are found
                    if (consecutiveCount == num_pages) {
                        break; 
                    }
                } else {
                    // Reset the consecutive counter if the entry is zero
                    consecutiveCount = 0;
                    PT_index_va = 0;
                }
            }

            if (consecutiveCount == num_pages) {
                PD_index_va = (unsigned long)i; // gets the index or exact bytes location of PD index for va
                break; 
            }
        }
        else{
            free_PDE = (unsigned long)i;
        }

    }

    if(PT_index_va == 0)
    {
        // if there no avail contiguos PTE then it will get the free PD index and PT index will be 0
        PD_index_va = free_PDE;
    }

    PD_index_va /= sizeof(pde_t);
    PT_index_va /= sizeof(pte_t);

    unsigned long va = (PD_index_va << (inner_bits + offset)) | (PT_index_va << offset);

    unsigned long va_PTE = 0;
    for(int i = 0; i< num_pages; i++)
    {
        va_PTE = (PD_index_va << (inner_bits + offset)) | ( (PT_index_va + i) << offset);
        int success = page_map(NULL, (void*)va_PTE, (void*)arr[i], arr);
        if(success == -1)
        {
            pthread_mutex_unlock(&mutex);
            return "ERROR";
        }
    }
   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */
    pthread_mutex_unlock(&mutex);
    return (void*)va;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void t_free(void *va, int size) {
    // set the page table entry to 0 and then unset the physicalBitmap to 1

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */
    pthread_mutex_lock(&mutex);

    unsigned long vpn = (unsigned long)va >> offset;

    unsigned long PD_index = (unsigned long)va >> (offset + inner_bits);
    unsigned long bit_mask = (unsigned long)va << outter_bits;
    unsigned long PT_index = bit_mask >> (offset + outter_bits);

    pte_t PT_start_index = ((unsigned long*)physicalMem)[PD_index]; 

    pte_t PTE_index = PT_start_index + (PT_index * sizeof(pte_t));

    int num_pages = ceil((double)size/PGSIZE);

    if(num_pages*sizeof(pte_t)  > (PGSIZE - (PTE_index % PGSIZE)))
    {
        pthread_mutex_unlock(&mutex);
        return; // this mean that the size is going out of the page table boundry 
    }
    
    unsigned long va_new = (PD_index << (inner_bits+offset)) | ( (PT_index + num_pages-1) << offset);


    // pte_t* pa = translate(NULL,(void*)va_new);
    // // if(pa == NULL)
    // //     return;

    pte_t* pa = translate(NULL,va);
    if(pa != NULL)
    {
        unset_bit_at_index(physicalBitmap, ((void*)pa - physicalMem)/PGSIZE ); // set the Physical page in bitmap to free '0'
    }

    ((unsigned long*)physicalMem)[PTE_index] = 0; // set the PTE to free
    ((unsigned long*)physicalMem)[PD_index] = 0;

    tlb_entries[vpn%TLB_ENTRIES].tag = (unsigned long)0;
    tlb_entries[vpn%TLB_ENTRIES].pa = (unsigned long)0;
    
    
    if(num_pages > 1)
    {
        for(int i = 1; i<num_pages; i++)
        {
           va_new = (PD_index << (inner_bits+offset)) | ( (PT_index + i) << offset); 
           unsigned long vpn_new = va_new >> offset;

           pte_t PTE_index = PT_start_index + ((PT_index+i) * sizeof(pte_t));

           pa = translate(NULL,(void*)va_new);
           if(pa != NULL)
            {
                unset_bit_at_index(physicalBitmap, ((void*)pa - physicalMem)/PGSIZE );// set the Physical page in bitmap to free '0'
            }

            ((unsigned long*)physicalMem)[PTE_index] = 0;// set the PTE to free
            ((unsigned long*)physicalMem)[PD_index] = 0;

            tlb_entries[vpn_new %TLB_ENTRIES].tag = (unsigned long)0;
            tlb_entries[vpn_new %TLB_ENTRIES].pa = (unsigned long)0;

        }
    }

    pthread_mutex_unlock(&mutex);
    return;
    // if(pa == NULL) return -1;
    
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
 * The function returns 0 if the put is successfull and -1 otherwise.
*/
int put_value(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */
    pthread_mutex_lock(&mutex);
    
    int num_pages = ceil((double)size/PGSIZE);
     pte_t* pa = translate(NULL,va);
    if(pa == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return -1;
    } 
    // int size_left = size;
    
    if(num_pages == 1)
    {
        memcpy(pa,val,size);
    }
    else
    {
        unsigned long PD_index = (unsigned long)va >> (offset + inner_bits);
        unsigned long bit_mask = (unsigned long)va << outter_bits;
        unsigned long PT_index = bit_mask >> (offset + outter_bits);

        unsigned long va_new = (PD_index << (inner_bits+offset)) | ( (PT_index + num_pages-1) << offset);

        pa = translate(NULL,(void*)va_new);
        if(pa == NULL)
        {
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        bit_mask = (unsigned long)va << (outter_bits + inner_bits);
        unsigned long offset_value = bit_mask >> (outter_bits + inner_bits);

        // pa = translate(NULL,va);
        memcpy(pa,val,(PGSIZE-offset_value) );
        size -= (PGSIZE - offset_value);
        val += (PGSIZE - offset_value);

        for(int i=1; i<num_pages; i++)
        {
            va_new = (PD_index << (inner_bits+offset)) | ( (PT_index + i) << offset);

            pa = translate(NULL,(void*)va_new);
            if(pa == NULL)
            {
                pthread_mutex_unlock(&mutex);
                return -1;
            }

            if(size >= PGSIZE)
            {
                memcpy(pa,val, PGSIZE);
                size -= PGSIZE;
                val += PGSIZE;
            }
            else
            {
                memcpy(pa,val, size);
                size -= PGSIZE;
                val += PGSIZE;
            }
            if(size == 0)
                break;

        }

    }

    pthread_mutex_unlock(&mutex);
    return 0;


    /*return -1 if put_value failed and 0 if put is successfull*/

}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */
    pthread_mutex_lock(&mutex);

    int num_pages = ceil((double)size/PGSIZE);
    pte_t* pa = translate(NULL,va);
    if(pa == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return;
    } 
    // int size_left = size;
    
    if(num_pages == 1)
    {
        memcpy(val,pa,size);
    }
    else
    {
        unsigned long PD_index = (unsigned long)va >> (offset + inner_bits);
        unsigned long bit_mask = (unsigned long)va << outter_bits;
        unsigned long PT_index = bit_mask >> (offset + outter_bits);

        // calculating VA of the last PTE based on the size has been passed in and need to check if the last page is valid
        unsigned long va_new = (PD_index << (inner_bits+offset)) | ( (PT_index + num_pages-1) << offset);
        pa = translate(NULL,(void*)va_new);
        if(pa == NULL)
        {
            pthread_mutex_unlock(&mutex);
            return;
        }

        bit_mask = (unsigned long)va << (outter_bits + inner_bits);
        unsigned long offset_value = bit_mask >> (outter_bits + inner_bits);

        // pa = translate(NULL,va);
        memcpy(val,pa,(PGSIZE-offset_value) );
        size -= (PGSIZE - offset_value);
        val += (PGSIZE - offset_value);

        for(int i=1; i<num_pages; i++)
        {
            va_new = (PD_index << (inner_bits+offset)) | ( (PT_index + i) << offset);

            pa = translate(NULL,(void*)va_new);
            if(pa == NULL)
            {
                pthread_mutex_unlock(&mutex);
                return ;
            }

            if(size >= PGSIZE)
            {
                memcpy(val,pa, PGSIZE);
                size -= PGSIZE;
                val += PGSIZE;
            }
            else
            {
                memcpy(val, pa, size);
                size -= PGSIZE;
                val += PGSIZE;
            }
            if(size == 0)
                break;

        }

    }
    pthread_mutex_unlock(&mutex);
    return;

}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */

    int x, y, val_size = sizeof(int);
    int i, j, k;
    for (i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            unsigned int a, b, c = 0;
            
            for (k = 0; k < size; k++) {
                
                int address_a = (unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int));
                int address_b = (unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int));
                get_value( (void *)address_a, &a, sizeof(int));
                get_value( (void *)address_b, &b, sizeof(int));
                // printf("Values at the index: %d, %d, %d, %d, %d\n", 
                //     a, b, size, (i * size + k), (k * size + j));
                c += (a * b);
                
            }
            
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            // printf("This is the c: %d, address: %x!\n", c, address_c);
            put_value((void *)address_c, (void *)&c, sizeof(int));
        }
    }
}



