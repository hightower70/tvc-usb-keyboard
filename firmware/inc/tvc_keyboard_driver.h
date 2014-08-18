/* 
 * File:   tvc_mapping.h
 * Author: arvai.laszlo
 *
 * Created on 2014. április 1., 13:14
 */

#ifndef TVC_MAPPING_H
#define	TVC_MAPPING_H

///////////////////////////////////////////////////////////////////////////////
// Includes
#include "GenericTypeDefs.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
#define TVC_KEYBOARD_MATRIX_ROW_COUNT 10

// modifier flags
#define MF_LEFT_SHIFT			(1<<0)
#define MF_RIGHT_SHIFT		(1<<1)
#define MF_LEFT_CONTROL		(1<<2)
#define MF_RIGHT_CONTROL	(1<<3)
#define MF_LEFT_ALT				(1<<4)
#define MF_RIGHT_ALT			(1<<5)
#define MF_LEFT_GUI				(1<<6)
#define MF_RIGHT_GUI			(1<<7)


///////////////////////////////////////////////////////////////////////////////
// Global variables


///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void InitKeyboardDriver(void);
void UpdateKeyboardMatrix(WORD in_keycode, BYTE in_modifiers, BOOL in_pressed);
void UpdateModifierKeys(BYTE in_state);
void ClearKeyboardMatrix(void);


#endif	/* TVC_MAPPING_H */

