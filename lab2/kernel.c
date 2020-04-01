#include "uart.h"

#define LOADER_START_ADDR 0x80000

extern char __loader_size[];
unsigned int loader_size;
char *gpu_load_addr ;

unsigned int get_kernel_size(){
    unsigned int size;
    size = uart_getc();
    size |= uart_getc()<<8;
    size |= uart_getc()<<16;
    size |= uart_getc()<<24;
    return size;
}

unsigned long calc_loader_copy_addr(unsigned long load_kernel_addr){
    unsigned long copy_addr;
    loader_size = (unsigned long)__loader_size;
    // uart_send_int(loader_size); uart_puts("\n");
    // unsigned int stack_reserve = 1024;
    copy_addr = load_kernel_addr - 0x10000;
    // uart_send_int(copy_addr); uart_puts("\n");
    return copy_addr;
}

void recieve_kernel(char *load_addr, unsigned int size){
    //uart_puts("enter recieve function...\n");
    char *kernel=(char*)load_addr;
    while(size--) 
        *kernel++ = uart_getc();
}

void copy_loader_and_jump(char *copy_addr, char *load_kernel_addr, unsigned int kernel_size){
    gpu_load_addr = (char*)LOADER_START_ADDR;

    //copy loader
    char *loader_old_base = gpu_load_addr;
    char *loader_new_base = copy_addr;
    unsigned int loader_byte = loader_size ;
    // uart_send_int(loader_byte); uart_puts("\n");
    while(loader_byte--)
        *loader_new_base++ = *loader_old_base++;
    // uart_hex((unsigned long)loader_old_base); uart_puts("\n");
    // uart_hex((unsigned long)loader_new_base); uart_puts("\n");

    //copy stack
    char *old_stack = gpu_load_addr-1;
    char *new_stack = copy_addr-1;
    char* sp;
    asm volatile ("mov %0, sp" :"=r"(sp));
    // uart_send_int(LOADER_START_ADDR-(unsigned long)sp); uart_puts("\n");
    while(old_stack >= sp)
        *new_stack-- = *old_stack--;
    sp = new_stack + 1;
    asm volatile ("mov sp, %0" ::"r"(sp));
    
    // jump to copied loader 
    // when the x30 is determined, below should not enclude any function call!!
    // no problem, but contain magic number... 
    // unsigned int next_instr_in_copy = (unsigned long)copy_addr + 0x84c; //magic number...-> offset to next instrc(objdump). 
    // asm volatile ("mov x30, %0" ::"r"(next_instr_in_copy));


    // when using inline asm, becareful of the register to write!!! 
    // ex: below will cause error, due to x0 save the copy_addr value!!!
    // then the result will be very unstable
    /*uart_puts("loader backup done\n");
      asm volatile ("ldr x0, =0x10000; sub x30, x30, x0");*/
    //I made a stuuuuuupid mistake -> asm volatile ("sub x30, x30, #0x10000"); 
    asm volatile ("sub x30, x30, #0x80000;"
                  "add x30, x30, %0"::"r"(copy_addr));

    // unsigned long lr;
    // asm volatile ("mov %0, x30" :"=r"(lr));
    // uart_hex(lr); uart_puts("\n");
    // uart_hex((unsigned long)copy_addr); uart_puts("\n");
    // uart_hex( lr - (unsigned long)copy_addr); uart_puts("\n");

    //note: stack operation is restricted when using inline asm
}

