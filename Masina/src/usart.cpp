#include <avr/io.h>
#include <stdio.h>

#include "usart.h"

/* Stream's putchar() implementation to send a byte using USART0 */
static int _usart0_putchar(char c, FILE *stream)
{
    if (c == '\n')  /* convert '\n' to CRLF */
        _usart0_putchar('\r', stream);
    USART0_transmit(c);
    return 0;
}

/* Initialize the USART peripheral. */
void USART0_init(uint16_t ubrr)
{
    /* baud rate registers */
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    /* enable TX and RX */
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    /* frame format: 8 bits, 1 stop, no parity */
    UCSR0C = (3 << UCSZ00); // 8N1
}

/*
 * Receives a byte from USART.
 *
 * @return the data byte received;
 */
char USART0_receive(void)
{
    /* busy wait until reception is complete */
    while (!(UCSR0A & (1 << RXC0)));

    /* the received byte is read from this register */
    return UDR0;
}

/*
 * Transmit a byte through the USART.
 *
 * @param `data`: the character to send
 */
void USART0_transmit(char data)
{
    /* wait until buffer is empty */
    while (!(UCSR0A & (1 << UDRE0)));

    /* by writing to this register, transmission hardware is triggered */
    UDR0 = data;
}

/*
 * Trasmits a null-terminated string through the USART.
 *
 * @param str: the string to send
 */
void USART0_print(const char *str)
{
    while (*str != '\0')
        USART0_transmit(*str++);
}

/*
 * Trasmits a uint32_t number through the USART.
 *
 * @param var: the number to send
 */
void USART0_print_num(const int32_t var) {
    char text[20];
    sprintf(text, "%ld\n", var);
    USART0_print(text);
}
