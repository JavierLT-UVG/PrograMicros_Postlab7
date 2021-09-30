/*
 * Archivo:         Prelab7.c
 * Dispositivo:     PIC16F887
 * Autor:           Francisco Javier López
 *
 * Programa:        Incremento y decremento de Puerto A usando dos botones
 *                  y contador decimal con displays en el Puerto C
 * Hardware:        2 Push buttons en Puerto B, 8 Leds en Puerto A, Display
 *                  de 7 segmentos de 3 dígitos y 3 transistores en Puerto C
 * 
 * Creado: 28 de septiembre de 2021
 * Última Modificación: 29 de septiembre de 2021
 */


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

//============================================================================
//============================ VARIABLES GLOBALES ============================
//============================================================================
uint8_t num;                    // Variable contadora 
uint8_t estado;                 // Determina el estado de los transistores

uint8_t cen;                    // Variables que almacenan los dígitos
uint8_t dec;
uint8_t uni;

uint8_t cen_d;                  // Variables que almacenan los dígitos después
uint8_t dec_d;                  // de haber sido modificados por la tabla
uint8_t uni_d;

//============================================================================
//========================= DECLARACIÓN DE FUNCIONES =========================
//============================================================================
void config_io(void);
void config_reloj(void);
void config_tmr0(void);
void config_int(void);
void config_pullups(void);
void divisor(uint8_t num, uint8_t *centena, uint8_t *decena, uint8_t *unidad);
uint8_t tabla(uint8_t valor);

//============================================================================
//============================== INTERRUPCIONES ==============================
//============================================================================
void __interrupt() isr (void)
{
    if(INTCONbits.T0IF)         // Si la bandera está encendida, entrar
    {
        INTCONbits.T0IF = 0;    // Limpiar bandera
        PORTD = 0;              // Limpiar último transistor encendido 
        switch(estado)
        {
        case 0:
            estado = 1;
            PORTD = 0b001;      // Encender transistor 1
            PORTC = cen_d;      // Display recibe valor de centenas
            TMR0 = 131;         // Timer0 a 2ms
            break;
        case 1:
            estado = 2;
            PORTD = 0b010;      // Encender transistor 2
            PORTC = dec_d;      // Display recibe valor de decenas
            TMR0 = 131;         // Timer0 a 2ms
            break;
        case 2:
            estado = 0;
            PORTD = 0b100;      // Encender transistor 3
            PORTC = uni_d;      // Display recibe valor de unidades
            TMR0 = 131;         // Timer0 a 2ms
            break;
        default:
            estado = 0;
            TMR0 = 131;         // Timer0 a 2ms
            break;
        }
    }  
}

//============================================================================
//=================================== MAIN ===================================
//============================================================================
void main(void) 
{
    config_io();
    config_reloj();
    config_tmr0();
    config_int();
    config_pullups();
    
    while(1)
    {
        if(!PORTBbits.RB0)      // Si RB0 = 0, entrar
        {
            while(!RB0);        // Mientras que RB0 = 0, decrementar
            num --;
        }
        
        if(!PORTBbits.RB1)      // Si RB1 = 0, entrar
        {
            while(!RB1);        // Mientras que RB1 = 0, decrementar
            num ++;
        }
        
        PORTA = num;            // PortA siempre asume el valor del contador
        
        divisor(num, &cen, &dec, &uni); // División en dígitos del contador
   
        uni_d = tabla(uni);    // Asignar a cada variable el valor traducido
        dec_d = tabla(dec);    // por la función "tabla", que puede ser leído
        cen_d = tabla(cen);    // por el display de 7 segmentos
        
        PORTC = cen_d;
        PORTD = 0b1;
    }
}

//============================================================================
//================================ FUNCIONES =================================
//============================================================================
void config_io(void)            // Configuración de entradas y salidas
{
    ANSEL   =   0;              // Puertos digitales
    ANSELH  =   0;
    TRISA   =   0;              // PuertoA como salida para leds
    TRISB   =   0b11;           // PuertoB en 0 y 1 como entrada para botones
    TRISC   =   0;              // PuertoC como salida para display
    TRISD   =   0b11111000;     // PuertoD como salida para transistores
    PORTA   =   0;              // Limpiar PuertoA
    PORTB   =   0;              // Limpiar PuertoB
    PORTC   =   0;              // Limpiar PuertoC
    PORTD   =   0;              // Limpiar PuertoD
    return;
}

void config_reloj(void)         // Configuración del oscilador
{
    OSCCONbits.IRCF = 0b100;    // 1MHz
    OSCCONbits.SCS = 1;         // Reloj interno
    return;
}

void config_tmr0(void)
{
    OPTION_REGbits.T0CS = 0;    // Reloj interno seleccionado
    OPTION_REGbits.T0SE = 0;    // Flancos positivos
    OPTION_REGbits.PSA = 0;     // Prescaler a Timer0
    OPTION_REGbits.PS2 = 0;     // Prescaler (001 = 1:4)
    OPTION_REGbits.PS1 = 0;     
    OPTION_REGbits.PS0 = 1;     
    TMR0 = 131;                 // Preload para 2 ms
    return;
}

void config_int(void)           // Configuración de interrupciones
{
    INTCONbits.GIE  = 1;        // Activar interrupciones globales
    INTCONbits.T0IE = 1;        // Activar interrupciones de timer0
    INTCONbits.T0IF = 0;        // Apagar bandera de timer0
    return;
}

void config_pullups(void)       // Configuración de pull-ups
{
    OPTION_REGbits.nRBPU = 0;   // Pull-ups activadas
    WPUB = 0b11;                // Pull-ups asignadas a RB0 y RB1
    return;
}

void divisor(uint8_t num, uint8_t *centena, uint8_t *decena, uint8_t *unidad)
{
    uint8_t tempRes;            // Variable aux para decenas y unidades
    *centena = num / 100;       // Dividir por 100 para obtener centenas
    tempRes = num % 100;        // Mod 100 para aislar decenas y unidades
    *decena = tempRes / 10;     // Dividir el aux por 10 para obtener decenas
    *unidad = tempRes % 10;     // Mod 10 al aux para obtener unidades
    return;
}

uint8_t tabla(uint8_t valor)    // Traduce números de 0-9 a valores de display
{
    switch(valor)
    {
        case 0:
            return 0b00111111;
            break;
        case 1:
            return 0b00000110;
            break;
        case 2:
            return 0b01011011;
            break;
        case 3:
            return 0b01001111;
            break;
        case 4:
            return 0b01100110;
            break;
        case 5:
            return 0b01101101;
            break;
        case 6:
            return 0b01111101;
            break;
        case 7:
            return 0b00000111;
            break;
        case 8:
            return 0b01111111;
            break;
        case 9:
            return 0b01101111;
            break;
        default:
            return 0b00111111;  // Default también es un 0
            break;
    }
}