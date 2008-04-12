/***************************************************************************

    machine/pcshare.c

    Functions to emulate general aspects of the machine
    (RAM, ROM, interrupts, I/O ports)

    The information herein is heavily based on
    'Ralph Browns Interrupt List'
    Release 52, Last Change 20oct96

    TODO:
    clean up (maybe split) the different pieces of hardware
    PIC, PIT, DMA... add support for LPT, COM (almost done)
    emulation of a serial mouse on a COM port (almost done)
    support for Game Controller port at 0x0201
    support for XT harddisk (under the way, see machine/pc_hdc.c)
    whatever 'hardware' comes to mind,
    maybe SoundBlaster? EGA? VGA?

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "memconv.h"
#include "machine/8255ppi.h"

#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"

#include "machine/8237dma.h"
#include "machine/pckeybrd.h"

#define VERBOSE_DBG 0       /* general debug messages */
#define DBG_LOG(N,M,A) \
	if(VERBOSE_DBG>=N){ if( M )logerror("%11.6f: %-24s",attotime_to_double(timer_get_time()),(char*)M ); logerror A; }

#define VERBOSE_JOY 0		/* JOY (joystick port) */
#define JOY_LOG(N,M,A) \
	if(VERBOSE_JOY>=N){ if( M )logerror("%11.6f: %-24s",attotime_to_double(timer_get_time()),(char*)M ); logerror A; }


static emu_timer *pc_keyboard_timer;

static TIMER_CALLBACK( pc_keyb_timer );


/* ----------------------------------------------------------------------- */

static void pc_pic_set_int_line(int which, int interrupt)
{
	switch(which)
	{
		case 0:
			/* Master */
			cpunum_set_input_line(Machine, 0, 0, interrupt ? HOLD_LINE : CLEAR_LINE);
			break;

		case 1:
			/* Slave */
			pic8259_set_irq_line(0, 2, interrupt);
			break;
	}
}



void init_pc_common(UINT32 flags)
{
	/* PC-XT keyboard */
	if (flags & PCCOMMON_KEYBOARD_AT)
		at_keyboard_init(AT_KEYBOARD_TYPE_AT);
	else
		at_keyboard_init(AT_KEYBOARD_TYPE_PC);
	at_keyboard_set_scan_code_set(1);

	/* PIC */
	pic8259_init(2, pc_pic_set_int_line);

	pc_keyboard_timer = timer_alloc(pc_keyb_timer, NULL);
}

/*
   keyboard seams to permanently sent data clocked by the mainboard
   clock line low for longer means "resync", keyboard sends 0xaa as answer
   will become automatically 0x00 after a while
*/

static struct {
	UINT8 data;
	int on;
	int self_test;
} pc_keyb= { 0 };

UINT8 pc_keyb_read(void)
{
	return pc_keyb.data;
}



static TIMER_CALLBACK( pc_keyb_timer )
{
	if ( pc_keyb.on ) {
		pc_keyboard();
	} else {
		/* Clock has been low for more than 5 msec, start diagnostic test */
		at_keyboard_reset();
		pc_keyb.self_test = 1;
	}
}



void pc_keyb_set_clock(int on)
{
	on = on ? 1 : 0;

	if (pc_keyb.on != on)
	{
		if (!on)
			timer_adjust_oneshot(pc_keyboard_timer, ATTOTIME_IN_MSEC(5), 0);
		else {
			if ( pc_keyb.self_test ) {
				/* The self test of the keyboard takes some time. 2 msec seems to work. */
				/* This still needs to verified against a real keyboard. */
				timer_adjust_oneshot(pc_keyboard_timer, ATTOTIME_IN_MSEC( 2 ), 0);
			} else {
				timer_reset(pc_keyboard_timer, attotime_never);
				pc_keyb.self_test = 0;
			}
		}

		pc_keyb.on = on;
	}
}

void pc_keyb_clear(void)
{
	pc_keyb.data = 0;
	pic8259_set_irq_line(0, 1, 0);
}

void pc_keyboard(void)
{
	int data;

	at_keyboard_polling();

	if (pc_keyb.on)
	{
		if ( (data=at_keyboard_read())!=-1) {
			pc_keyb.data = data;
			DBG_LOG(1,"KB_scancode",("$%02x\n", pc_keyb.data));
			pic8259_set_irq_line(0, 1, 1);
			pc_keyb.self_test = 0;
		}
	}
}
