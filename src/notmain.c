void PUT32 ( unsigned int, unsigned int );
unsigned int GET32 ( unsigned int );
void DOWFI ( void );

#define RCCBASE 0x40021000
#define RCC_CR          (RCCBASE+0x00)
#define RCC_CFGR        (RCCBASE+0x04)
#define RCC_APB1RSTR    (RCCBASE+0x10)
#define RCC_APB2ENR     (RCCBASE+0x18)
#define RCC_APB1ENR     (RCCBASE+0x1C)
#define RCC_BDCR        (RCCBASE+0x70)

#define GPIOABASE 0x40010800
#define GPIOA_CRL     (GPIOABASE+0x00)
#define GPIOA_CRH     (GPIOABASE+0x04)
#define GPIOA_IDR     (GPIOABASE+0x08)
#define GPIOA_ODR     (GPIOABASE+0x0C)
#define GPIOA_BSRR    (GPIOABASE+0x10)
#define GPIOA_BRR     (GPIOABASE+0x14)
#define GPIOA_LCKR    (GPIOABASE+0x18)

#define USART2BASE 0x40004400
#define USART2_SR       (USART2BASE+0x00)
#define USART2_DR       (USART2BASE+0x04)
#define USART2_BRR      (USART2BASE+0x08)
#define USART2_CR1      (USART2BASE+0x0C)

#define TIM2BASE  0x40000000
#define TIM2_CR1        (TIM2BASE+0x00)
#define TIM2_DIER       (TIM2BASE+0x0C)
#define TIM2_SR         (TIM2BASE+0x10)
#define TIM2_CNT        (TIM2BASE+0x24)
#define TIM2_PSC        (TIM2BASE+0x28)
#define TIM2_ARR        (TIM2BASE+0x2C)

#define FLASH_ACR	((volatile unsigned long *) 0x40022000)

#define FLASH_PREFETCH	0x0010	/* enable prefetch buffer */

#define FLASH_WAIT0	0x0000	/* for sysclk <= 24 Mhz */
#define FLASH_WAIT1	0x0001	/* for 24 < sysclk <= 48 Mhz */
#define FLASH_WAIT2	0x0002	/* for 48 < sysclk <= 72 Mhz */

#define NVIC_ISER0  0xE000E100
#define NVIC_ISER1  0xE000E104
#define NVIC_ICPR0  0xE000E280
#define NVIC_ICPR1  0xE000E284

//PA2 is USART2_TX alternate function 1
//PA3 is USART2_RX alternate function 1

struct nvic {
    volatile unsigned long iser[3];	/* 00 */
    volatile unsigned long icer[3];	/* 0c */
    /* ... */
};

#define NVIC_BASE  ((void *)0xE000E100)
#define NUM_IRQ	68

void
nvic_enable ( int irq )
{
    struct nvic *np = NVIC_BASE;

    if ( irq >= NUM_IRQ )
        return;

    np->iser[irq/32] = 1 << (irq%32);
}

static int clock_init ( void )
{
    unsigned int ra;

    ra=GET32(RCC_CFGR);
    ra |= 1;
    PUT32(RCC_CFGR,ra);

    ra=GET32(RCC_CR);
    ra &= ~(1<<16);
    ra |= 1;
    PUT32(RCC_CR,ra);
    //while (!(GET32(RCC_CR) & (1 << 25)));

    return(0);
}

static int uart2_init ( void )
{
    unsigned int ra;

    ra=GET32(RCC_APB2ENR);
    ra|=1<<2; //enable port A
    PUT32(RCC_APB2ENR,ra);

    ra=GET32(RCC_APB1ENR);
    ra|=1<<17; //enable USART2
    PUT32(RCC_APB1ENR,ra);

    ra=GET32(GPIOA_CRL);
    ra &= ~(3<<10); //PA2 CNF clear
    ra &= ~(3<<14); //PA3 CNF clear
    ra &= ~(3<<8);  //PA2 MODE clear
    ra &= ~(3<<12); //PA3 MODE clear
    ra |= 2<<10; //PA2 CNF set AF PP
    ra |= 1<<14; //PA3 CNF set FI
    ra |= 3<<8;  //PA2 MODE set Out 50MHz
    ra |= 0<<12; //PA3 MODE set In
    PUT32(GPIOA_CRL, ra);

    ra=GET32(RCC_APB1RSTR);
    ra |= 1<<17; //reset USART2
    PUT32(RCC_APB1RSTR,ra);
    ra &= ~(1<<17);
    PUT32(RCC_APB1RSTR,ra);

    //PUT32(USART2_CR1,(1<<13));

    //8000000/(16*115200) = 4.34  4+5/16
    PUT32(USART2_BRR,0x45);
    PUT32(USART2_CR1,(1<<3)|(1<<2)|(1<<13));

    return(0);
}

static void uart2_send ( unsigned int x )
{
    while (1)
        if (GET32(USART2_SR) & (1<<7))
            break;
    PUT32(USART2_DR,x);
}


static void hexstrings ( unsigned int d )
{
    //unsigned int ra;
    unsigned int rb;
    unsigned int rc;

    rb=32;
    while(1)
    {
        rb-=4;
        rc=(d>>rb)&0xF;
        if(rc>9) rc+=0x37; else rc+=0x30;
        uart2_send(rc);
        if(rb==0) break;
    }
    uart2_send(0x20);
}

static void hexstring ( unsigned int d )
{
    hexstrings(d);
    uart2_send(0x0D);
    uart2_send(0x0A);
}

void tim2_handler ( void )
{
    uart2_send(0x55);
    PUT32(TIM2_SR,0);
    PUT32(NVIC_ICPR0, (1<<28));
}

int notmain ( void )
{
    unsigned int ra;

    asm volatile ("cpsie i");
    clock_init();
    uart2_init();
    hexstring(0x12345678);

    ra=GET32(RCC_APB1ENR);
    ra |= 1<<0; //enable TIM2
    PUT32(RCC_APB1ENR,ra);

    if (1) {
        PUT32(TIM2_CR1,0x0000);
        PUT32(TIM2_DIER,0x0001);
        PUT32(TIM2_PSC,800);
        PUT32(TIM2_ARR,10000);
        PUT32(TIM2_CNT,0x0000);
        PUT32(TIM2_CR1,0x0201);
        PUT32(TIM2_SR,0);
        //PUT32(NVIC_ICPR1,0x00040000);
        nvic_enable(28);
        PUT32(NVIC_ISER0, (1<<28));
        while (1) {
            DOWFI();
            uart2_send(0x56);
            uart2_send(0x0D);
            uart2_send(0x0A);
        }
    }

    return(0);
}
