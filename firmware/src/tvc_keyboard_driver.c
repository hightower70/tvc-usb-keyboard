#include <plib.h>
#include "tvc_keyboard_driver.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
#define ALTGR_KEY_COUNT 8
#define INVALID_ROW 0xff

///////////////////////////////////////////////////////////////////////////////
// Types

typedef struct
{
	BYTE row;
	BYTE column;
	BOOL shift;
} AltGrKeyInfo;

///////////////////////////////////////////////////////////////////////////////
// Local functions
static void UpdateShiftState(void);
static void UpdateAltGrKeys(BYTE in_row, BYTE in_column, BOOL in_shift, BOOL in_pressed);

///////////////////////////////////////////////////////////////////////////////
// Global variables
static BYTE l_keyboard_matrix[TVC_KEYBOARD_MATRIX_ROW_COUNT];
static AltGrKeyInfo l_altgr_keys[ALTGR_KEY_COUNT];
static BOOL l_shift_state = FALSE;
static BOOL l_altgr_state = FALSE;

///////////////////////////////////////////////////////////////////////////////
// Initializes TVC keyboard driver
void InitKeyboardDriver(void)
{
	DWORD temp;

	// init
	ClearKeyboardMatrix();

	// 8-bit open dran port for keyboard column data
	mPORTASetPinsDigitalOut(BIT_8 | BIT_9);
	mPORTAOpenDrainOpen(BIT_8 | BIT_9);
	mPORTCSetPinsDigitalOut(BIT_4 | BIT_5 | BIT_6 | BIT_7 | BIT_8 | BIT_9);
	mPORTCOpenDrainOpen(BIT_4 | BIT_5 | BIT_6 | BIT_7 | BIT_8 | BIT_9);

	// setup row select input pins
	mPORTBSetPinsDigitalIn(BIT_5 | BIT_7 | BIT_8 | BIT_9 );

	// setup pin change interrupt
	mCNBOpen(CNB_ON | CNB_IDLE_CON, CNB5_ENABLE | CNB7_ENABLE | CNB8_ENABLE | CNB9_ENABLE, CNB5_PULLUP_ENABLE | CNB7_PULLUP_ENABLE | CNB8_PULLUP_ENABLE | CNB9_PULLUP_ENABLE);
	temp = mPORTBRead();	// clear IT flag
	ConfigIntCNB(CHANGE_INT_ON | CHANGE_INT_PRI_7);	// enable interrupt
}

///////////////////////////////////////////////////////////////////////////////
// Clears keyboard matrix
void ClearKeyboardMatrix(void)
{
	BYTE i;

	// init matrix
	for(i=0; i<TVC_KEYBOARD_MATRIX_ROW_COUNT ; i++)
	{
		l_keyboard_matrix[i] = 0xff;
	}

	for(i=0; i<ALTGR_KEY_COUNT; i++)
	{
		l_altgr_keys[i].row = INVALID_ROW;
	}

	l_shift_state = FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Updates keyboard output
void UpdateMatrixOutput(void)
{
	DWORD row;
	BYTE data;
	DWORD data_low;
	DWORD data_high;

	// generate row address
	row = mPORTBRead() & (BIT_5 | BIT_7 | BIT_8 | BIT_9);
	if((row & BIT_5) != 0)
		row |= BIT_6;

	row = (row >> 6);

	// set column data for the selected row data
	if(row >= TVC_KEYBOARD_MATRIX_ROW_COUNT)
		data = 0xff;
	else
		data = l_keyboard_matrix[row];

	// update column lines

	// lowest two bits
	data_low = (data & 0x03) << 8;
	mPORTAClearBits(~data_low);
	mPORTASetBits(data_low);

	// highest six bits
	data_high = (data & 0xfc) << 2;
	mPORTCClearBits(~data_high);
	mPORTCSetBits(data_high);
}

///////////////////////////////////////////////////////////////////////////////
// Interrupt handler
void __ISR(_CHANGE_NOTICE_VECTOR, ipl7auto) ChangeNotice_Handler(void)
{
	// check for keyboard row change
	if(mCNBGetIntFlag())
	{
		UpdateMatrixOutput();

		// clear the interrupt flag
		mCNBClearIntFlag();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Update modifier keys state
void UpdateModifierKeys(BYTE in_state)
{
	BYTE i;

	// Shift
	if((in_state & (MF_LEFT_SHIFT | MF_RIGHT_SHIFT)) != 0)
	{
		l_keyboard_matrix[6] &= ~(1 << 3);
		l_shift_state = TRUE;
		UpdateShiftState();
	}
	else
	{
		l_keyboard_matrix[6] |= (1 << 3);
		l_shift_state = FALSE;
		UpdateShiftState();
	}

	// Ctrl
	if((in_state & (MF_LEFT_CONTROL | MF_RIGHT_CONTROL)) != 0)
		l_keyboard_matrix[7] &= ~(1 << 4);
	else
		l_keyboard_matrix[7] |= (1 << 4);

	// Alt
	if((in_state & (MF_LEFT_ALT)) != 0)
		l_keyboard_matrix[7] &= ~(1 << 0);
	else
		l_keyboard_matrix[7] |= (1 << 0);

	// AltGr state
	if((in_state & (MF_RIGHT_ALT)) != 0)
	{
		l_altgr_state = TRUE;
	}
	else
	{
		if(l_altgr_state)
		{
			// remove all AltGr Key if AltGr is released
			for(i=0; i<ALTGR_KEY_COUNT; i++)
			{
				if( l_altgr_keys[i].row != INVALID_ROW)
				{
					l_keyboard_matrix[l_altgr_keys[i].row] |= (1 << l_altgr_keys[i].column);
				}

				l_altgr_keys[i].row = INVALID_ROW;
			}

			UpdateShiftState();

			l_altgr_state = FALSE;
		}
	}

	INTDisableInterrupts();
	UpdateMatrixOutput();
	INTEnableInterrupts();
}

///////////////////////////////////////////////////////////////////////////////
// Updates TVC keyboard matrix
void UpdateKeyboardMatrix(WORD in_keycode, BYTE in_modifiers, BOOL in_pressed)
{
	BYTE row = 0xff;
	BYTE column = 0xff;


	switch(in_keycode)
	{
		case 80:	row = 8; column = 6; break;  //  8/6 - left
		case 82:	row = 8; column = 1; break;  //  8/1 - up
		case 79:	row = 8; column = 5; break;  //  8/5 - right
		case 81:	row = 8; column = 2; break;  //  8/2 - down

		case 40:	row = 5; column = 4; break;  //  5/4 - RETURN
    case 44:	row = 7; column = 5; break;  //  7/5 - SPACE
    case 41:	row = 7; column = 3; break;  //  7/3 - ESCAPE
		case 42:	row = 5; column = 0; break;  //  5/0 - DEL
		case 73:	row = 8; column = 0; break;  //  8/0 - INSERT
		case 57:	row = 6; column = 5; break;  //  6/5 - LOCK

    case 54:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 2; column = 3;							 //  2/3 - ( ; )
				UpdateAltGrKeys(row, column, FALSE, in_pressed);
			}
			else
			{
				row = 7; column = 1;							 //  7/1 - , (comma)
			}
			break;

		case 55:	row = 7; column = 2; break;  //  7/2 - . (dot)

		case 53:	row = 0; column = 3; break;  //  0/3 - 0

    case 30:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 1; column = 0;							 //  1/0 - ( ~ )
				UpdateAltGrKeys(row, column, TRUE, in_pressed);
			}
			else
			{
				row = 0; column = 6;							 //  0/6 - 1
			}
			break;

    case 31:	row = 0; column = 2; break;  //  0/2 - 2

    case 32:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 1; column = 0;							 //  1/0 - ( ^ )
				UpdateAltGrKeys(row, column, FALSE, in_pressed);
			}
			else
			{
				row = 0; column = 1;							 //  0/1 - 3
			}
			break;

		case 33:	row = 0; column = 7; break;  //  0/7 - 4
    case 34:	row = 0; column = 0; break;  //  0/0 - 5
    case 35:	row = 0; column = 4; break;  //  0/4 - 6
    case 36:	row = 1; column = 7; break;  //  1/7 - 7
    case 37:	row = 1; column = 1; break;  //  1/1 - 8
    case 38:	row = 1; column = 2; break;  //  1/2 - 9

    case 4:		row = 4; column = 6; break;  //  4/6 - A

    case 5:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 3; column = 4; 							 //  4/3 - ( { )
				UpdateAltGrKeys(row, column, TRUE, in_pressed);
			}
			else
			{
				row = 6; column = 0;							 //  6/0 - B
			}
			break;

    case 6:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 0; column = 3;									//  0/3 - ( & )
				UpdateAltGrKeys(row, column, TRUE, in_pressed);
			}
			else
			{
				row = 6; column = 1;									//  6/1 - C
			}
			break;

		case 7:		row = 4; column = 1; break;  //  4/1 - D
    case 8:		row = 2; column = 1; break;  //  2/1 - E

    case 9:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 3; column = 4; 							 //  4/3 - ( [ )
				UpdateAltGrKeys(row, column, FALSE, in_pressed);
			}
			else
			{
				row = 4; column = 7;							 //  4/7 - F
			}
			break;

    case 10:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 3; column = 0; 							 //  3/0 - ( ] )
				UpdateAltGrKeys(row, column, FALSE, in_pressed);
			}
			else
			{
				row = 4; column = 0;							 //  4/0 - G
			}
			break;

    case 11:	row = 4; column = 4; break;  //  4/4 - H
		case 12:	row = 3; column = 1; break;  //  3/1 - I
    case 13:	row = 5; column = 7; break;  //  5/7 - J
    case 14:	row = 5; column = 1; break;  //  5/1 - K
    case 15:	row = 5; column = 2; break;  //  5/2 - L
    case 16:	row = 7; column = 7; break;  //  7/7 - M

    case 17:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 3; column = 0; 							 //  3/0 - ( } )
				UpdateAltGrKeys(row, column, TRUE, in_pressed);
			}
			else
			{
				row = 6; column = 4;							 //  6/4 - N
			}
			break;

    case 18:	row = 3; column = 2; break;  //  3/2 - O
    case 19:	row = 3; column = 6; break;  //  3/6 - P

    case 20:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 4; column = 3;							 //  4/3 - ( \ )
				UpdateAltGrKeys(row, column, FALSE, in_pressed);
			}
			else
			{
				row = 2; column = 6;							 //  2/6 - Q
			}
			break;

    case 21:	row = 2; column = 7; break;  //  2/7 - R
		case 22:	row = 4; column = 2; break;  //  4/2 - S
    case 23:	row = 2; column = 0; break;  //  2/0 - T
    case 24:	row = 3; column = 7; break;  //  3/7 - U

    case 25:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 2; column = 5;							 //  2/5 - @
				UpdateAltGrKeys(row, column, FALSE, in_pressed);
			}
			else
			{
				row = 6; column = 7;							 //  6/7 - V
			}
			break;

    case 26:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 4; column = 3;							 //  4/3 - ( | )
				UpdateAltGrKeys(row, column, TRUE, in_pressed);
			}
			else
			{
				row = 2; column = 2;							 //  2/2 - W
			}
			break;

		case 27:
			if(in_modifiers == MF_RIGHT_ALT)
			{
				row = 1; column = 4;							 //  1/4 - #
				UpdateAltGrKeys(row, column, TRUE, in_pressed);
			}
			else
			{
				row = 6; column = 2;							 //  6/2 - X
			}
			break;

    case 29:
			if(in_modifiers == MF_RIGHT_ALT)     // only right alt (ALT Gr) is allowed
			{
				row = 4; column = 5;							 //  4/5 - ( > )
				UpdateAltGrKeys(row, column, TRUE, in_pressed);
			}
			else
			{
				row = 6; column = 6;							 //  6/6 - Y
			}
			break;


    case 28:	row = 2; column = 4; break;  //  2/4 - Z

    case 56:
			if(in_modifiers == MF_RIGHT_ALT)     // only right alt (ALT Gr) is allowed
			{
				row = 1; column = 4;							 //  1/4 - ( * )
				UpdateAltGrKeys(row, column, FALSE, in_pressed);
			}
			else
			{
				row = 7; column = 6;							 //  7/6 - ( -_ )
			}
			break;

		case 52:	row = 5; column = 3; break;  //  5/3 - A'

		case 51:
			if(in_modifiers == MF_RIGHT_ALT)     // only right alt (ALT Gr) is allowed
			{
				row = 2; column = 3;							 //  2/3 - ( $ )
				UpdateAltGrKeys(row, column, TRUE, in_pressed);
			}
			else
			{
				row = 5; column = 6;							 //  5/6 - E'
			}
			break;

		case 100:
			if(in_modifiers == MF_RIGHT_ALT)     // only right alt (ALT Gr) is allowed
			{
				row = 4; column = 5;							 //  1/4 - ( < )
				UpdateAltGrKeys(row, column, FALSE, in_pressed);
			}
			else
			{
				row = 0; column = 5;							 //  0/5 - I'
			}
			break;

    case 46:	row = 1; column = 5; break;  //  1/5 - O'
		case 39:	row = 1; column = 6; break;  //  1/6 - O:
    case 47:	row = 3; column = 3; break;  //  3/3 - O"
		case 45:	row = 1; column = 3; break;  //  1/3 - U:
    case 50:	row = 5; column = 5; break;  //  5/5 - U"
    case 48:	row = 3; column = 5; break;  //  3/5 - U'
	}

	if (row < 0xff && column < 0xff)
	{
		if(in_pressed)
			l_keyboard_matrix[row] &= ~(1 << column);
		else
			l_keyboard_matrix[row] |= (1 << column);
	}

	UpdateShiftState();

	INTDisableInterrupts();
	UpdateMatrixOutput();
	INTEnableInterrupts();
}

///////////////////////////////////////////////////////////////////////////////
// Adds key to the pressed altgr key list
static void UpdateAltGrKeys(BYTE in_row, BYTE in_column, BOOL in_shift, BOOL in_pressed)
{
	BYTE i;

	if(in_pressed)
	{
		// add to list

		// find free position
		i = 0;
		while(i<ALTGR_KEY_COUNT)
		{
			// store key data
			if(l_altgr_keys[i].row == INVALID_ROW)
			{
				l_altgr_keys[i].row			= in_row;
				l_altgr_keys[i].column	= in_column;
				l_altgr_keys[i].shift		= in_shift;

				break;
			}

			i++;

		}
	}
	else
	{
		// remove from list and from the matrix
		for(i=0; i<ALTGR_KEY_COUNT; i++)
		{
			if(l_altgr_keys[i].row == in_row && l_altgr_keys[i].column == in_column && l_altgr_keys[i].shift == in_shift)
			{
				//l_keyboard_matrix[l_altgr_keys[i].row] |= (1 << l_altgr_keys[i].column);
				l_altgr_keys[i].row = INVALID_ROW;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Updates shift key state
static void UpdateShiftState(void)
{
	BOOL shift_state = FALSE;
	BYTE i;

	for(i=0; i<ALTGR_KEY_COUNT && !shift_state; i++)
	{
		if(l_altgr_keys[i].row != INVALID_ROW && l_altgr_keys[i].shift )
			shift_state = TRUE;
	}

	// Shift
	if(shift_state || l_shift_state)
	{
		l_keyboard_matrix[6] &= ~(1 << 3);
	}
	else
	{
		l_keyboard_matrix[6] |= (1 << 3);
	}
}
