#include <uart.h>

#include <dassert.h>
#include <ts7200.h>

int uart_write(int channel, char c) {
	int *flags, *data, *ctlr;
	switch(channel) {
	case COM1:
		flags = (int *)(UART1_BASE + UART_FLAG_OFFSET);
		data = (int *)(UART1_BASE + UART_DATA_OFFSET);
        ctlr = (int *)(UART1_BASE + UART_CTLR_OFFSET);
		break;
	case COM2:
		flags = (int *)(UART2_BASE + UART_FLAG_OFFSET);
		data = (int *)(UART2_BASE + UART_DATA_OFFSET);
        ctlr = (int *)(UART2_BASE + UART_CTLR_OFFSET);
		break;
	default:
		return -1;
		break;
	}

    dassert(!(*flags & TXFF_MASK), "Not ready to Transmit\n");
    dassert(!(*flags & TXBUSY_MASK), "UART Busy\n");
    if (channel == COM1) dassert(*flags & CTS_MASK, "CTS not set\n");
	*data = c;

    // Re-enable the Transmit Interrupt
    *ctlr |= TIEN_MASK;

	return 0;
}

int uart_read(int channel) {
	int *flags, *data;
	unsigned char c;

	switch(channel) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}

    dassert(*flags & TXFF_MASK, "Not ready to Recieve\n");

	c = *data;
	return c;
}

int uart_enable_interrupt(int channel, enum uart_interrupt interrupt) {
	int *ctlr;
    switch(channel) {
        case COM1:
            ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
            break;
        case COM2:
            ctlr = (int *)( UART2_BASE + UART_CTLR_OFFSET );
            break;
        default:
            return -1;
    }

    int interrupt_mask;
    switch(interrupt) {
        case T_INTERRUPT:
            interrupt_mask = TIEN_MASK;
            break;
        case R_INTERRUPT:
            interrupt_mask = RIEN_MASK;
            break;
        case MS_INTERRUPT:
            interrupt_mask = MSIEN_MASK;
            break;
        case RT_INTERRUPT:
            interrupt_mask = RTIEN_MASK;
            break;
        default:
            return -2;
    }

    *ctlr |= interrupt_mask;

    return 0;
}

int uart_disable_interrupt(int channel, enum uart_interrupt interrupt) {
	int *ctlr;
    switch(channel) {
        case COM1:
            ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
            break;
        case COM2:
            ctlr = (int *)( UART2_BASE + UART_CTLR_OFFSET );
            break;
        default:
            return -1;
    }

    int interrupt_mask;
    switch(interrupt) {
        case T_INTERRUPT:
            interrupt_mask = TIEN_MASK;
            break;
        case R_INTERRUPT:
            interrupt_mask = RIEN_MASK;
            break;
        case MS_INTERRUPT:
            interrupt_mask = MSIEN_MASK;
            break;
        case RT_INTERRUPT:
            interrupt_mask = RTIEN_MASK;
            break;
        default:
            return -2;
    }

    *ctlr &= ~interrupt_mask;

    return 0;
}

/*
 * The UARTs are initialized by RedBoot to the following state
 * 	115,200 bps
 * 	8 bits
 * 	no parity
 * 	fifos enabled
 */
int uart_setfifo( int channel, int state ) {
    int *line, buf;
    switch( channel ) {
        case COM1:
            line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
            break;
        case COM2:
            line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
            break;
        default:
            return -1;
            break;
    }
    buf = *line;
    buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
    *line = buf;
    return 0;
}

int uart_setspeed(int channel, int speed) {
    int *high, *low;
    switch( channel ) {
        case COM1:
            high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
            low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
            break;
        case COM2:
            high = (int *)( UART2_BASE + UART_LCRM_OFFSET );
            low = (int *)( UART2_BASE + UART_LCRL_OFFSET );
            break;
        default:
            return -1;
            break;
    }
    switch( speed ) {
        case 115200:
            *high = 0x0;
            *low = 0x3;
            return 0;
        case 2400:
            *high = 0x0;
            *low = 0xbf;
            return 0;
        default:
            return -1;
    }
}

int uart_setstop(int channel, int num) {
	int *line, buf;
	switch(channel) {
        case COM1:
            line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
            break;
        case COM2:
            line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
            break;
        default:
            return -1;
            break;
    }
	buf = *line;
	buf = (num == 2) ? buf | STP2_MASK : buf & ~STP2_MASK;
	*line = buf;
	return 0;
}

void com1_initialize() {
    /*
    int *high = (int *)( UART1_BASE + UART_LCRH_OFFSET );
    int *mid = (int *)( UART1_BASE + UART_LCRM_OFFSET );
    int *low = (int *)( UART1_BASE + UART_LCRL_OFFSET );

    *low = 0xbf; // Set speed correctly
    *mid = 0;
    *high = (*high & ~FEN_MASK) + STP2_MASK; // Disable FIFO and enable 2 stop

    int i;
    for (i = 0; i < 100; ++i) {}

    int *ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
    *ctlr = TIEN_MASK + MSIEN_MASK;
    */
}

void com2_initialize() {
}

void uart_initialize(int channel) {
    switch(channel) {
        case COM1:
            com1_initialize();
            break;
        case COM2:
            com2_initialize();
            break;
        default:
            log("Initializing Invalid UART\n");
    }
}
