/*
 * qemu with -kernel file.bin ==> start to execute from 0x10000 ==>
 * file.bin must be loaded to 0x10000 ==> . = 0x10000
 */
ENTRY(VectorsStart)
SECTIONS
{
  . = 0x10000;
  .text : {
      ts.o
      *(.text .rodata)
  }
  .data : { *(.data) }
  .bss : { *(.bss) }

  . = ALIGN(8);

 . = . + 0x1000; 
  svc_stack_top = .;
  . = . + 0x1000; 
  irq_stack_top = .;
}

