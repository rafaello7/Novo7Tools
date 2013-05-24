
/* Example program which may be executed using allwindisk.
 * To execute on arm, invoke:
 *      allwindisk mx 0 hello.bin
 */

void (*printf)(const char *fmt, ...) = (void(*)(const char*, ...))0x4A000004;

void Main(void)
{
    printf("Hello, World !!!\n");
}


void __attribute__((section(".init"))) _start(void)
{
    extern unsigned *__bss_start, *__bss_end;
    unsigned *p;

    /* clear BSS area */
    for(p = __bss_start; p != __bss_end; ++p)
        *p = 0;
    Main();
}

