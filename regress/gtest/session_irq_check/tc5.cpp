#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

uint8_t vector[100];
unsigned int idx=0;

extern "C"
{
    void stopsim();
}

void Record()
{
    vector[idx++]=PORTC;
}

void FireINT0()
{
    PORTD|= ( 1 << 2 ); // INT0
}

void FireINT1()
{
    PORTD|= ( 1 << 3 ); // INT1 
}

void FireINT2()
{
    PORTB|= ( 1 << 2 ); // INT2 
}

void ResetINT0()
{
    PORTD&= 0xff - ( 1 << 2 ); // INT0
}

void ResetINT1()
{
    PORTD&= 0xff- ( 1 << 3 ); // INT1
}

void ResetINT2()
{
    PORTB&= 0xff-( 1 << 2 ); // INT2
}

void stopsim()
{
    while(1);
}

int main()
{
    // set PORTS to 0
    DDRB = 0xff;
    PORTB = 0x00;

    DDRD = 0xff;
    PORTD = 0x00;

    // use PORT C for LEDS
    DDRC = 0xff;
    // switch them off ( invers )
    PORTC = 0x0f;


    //Setup INT1 + INT0 to rising edge
    MCUCR = (1 << ISC11 ) | ( 1 << ISC10 )| // INT1
        ( 1 << ISC01 ) | ( 1 << ISC00 ) ;   // INT0

    MCUCSR = (1 << ISC2 ); // INT2 auf rising edge

    GICR = ( 1<< INT1 )| ( 1 << INT0 ) | ( 1 << INT2 ); // INT0..3 active


    // fire the interrupts
    FireINT0();
    FireINT1();
    FireINT2();

    _NOP();
    sei();
    cli();  // expect no irq
    Record();   // idx[0] -> 0x0f;

    // now we only want to see the first interrupt
    sei(); 
    _NOP(); // exactly the first irq should fire
    cli(); // and nothing more
    Record(); // idx[1] -> 0x0e;

    // now again lets fire the next pending irq
    sei();
    _NOP();
    cli();
    Record();   // idx[2] -> 0x0c;

    // and now we get the last irq
    sei();
    _NOP();
    cli();
    Record();   // idx[3] -> 0x08;

    cli();

    // now reset the ports
    ResetINT0();
    ResetINT1();
    ResetINT2();

    // CLear LED Port
    PORTC = 0x0f;

    // now checking for correct ordering
    FireINT2(); // lowest order
    sei(); 
    cli(); // nothing should happen
    Record();   // idx[4] -> 0x0f;

    // now fire another irq with higher prio 
    FireINT1(); // mid prio
    sei();
    _NOP();
    cli(); 
    Record(); // idx[5] -> 0x0d;

    FireINT0();
    sei();
    _NOP();
    cli();
    Record();   //idx[6] -> 0x0c;

    // now the pending INT2 should run
    sei();
    _NOP();
    cli();
    Record();   // idx[7] -> 0x08;

    // now reset the ports
    ResetINT0();
    ResetINT1();
    ResetINT2();

    // CLear LED Port
    PORTC = 0x0f;

    // now we do it faster...
    PORTB|= ( 1 << 2 ); // INT2
    sei(); 
    PORTD|= ( 1 << 2 ); // INT0
    cli();

    // we expect that we get INT2 served first, because the irq vector
    // is already fetched and I-flag still allows interrupt handling.
    // as the last command in pipe enables a higher priority irq
    // but the lower one is already fetched, we see the lower one first!
    // this is validated on real m32 hardware
    Record(); // idx[8] -> 0x0b;
    sei();
    _NOP();
    cli();

    Record(); // idx[9] -> 0x0a; and now INT 0

    //stopsim();
}

ISR( INT0_vect )
{
    PORTC &= 0xff-1;
}
ISR( INT1_vect )
{
    PORTC &= 0xff-2;
}
ISR( INT2_vect )
{
    PORTC &= 0xff-4;
}

