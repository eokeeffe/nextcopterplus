//***********************************************************
//* menu_servos.c
//***********************************************************

//***********************************************************
//* Includes
//***********************************************************

#include <avr/pgmspace.h> 
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <util/delay.h>
#include "..\inc\io_cfg.h"
#include "..\inc\init.h"
#include "..\inc\mugui.h"
#include "..\inc\glcd_menu.h"
#include "..\inc\menu_ext.h"
#include "..\inc\glcd_driver.h"
#include "..\inc\main.h"
#include "..\inc\eeprom.h"
#include "..\inc\mixer.h"
#include "..\inc\imu.h"

//************************************************************
// Prototypes
//************************************************************

// Menu items
void menu_servo_setup(uint8_t section);

//************************************************************
// Defines
//************************************************************

#define SERVOSTART 62 	// Start of OUT1-OUT8 Menu text items
#define SERVOOFFSET 80	// LCD horizontal offsets

#define SERVOTEXT1 93 	// Start of value text items
#define SERVOTEXT2 103
#define SERVOTEXT3 38 
#define SERVOTEXT4 38
#define SERVOTEXT5 22

#define SERVOITEMS 8 	// Number of menu items

//************************************************************
// Servo menu items
//************************************************************
	 
const uint8_t ServoMenuText[5][SERVOITEMS] PROGMEM = 
{
	{141,141,141,141,141,141,141,141},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
};

const menu_range_t servo_menu_ranges[5][SERVOITEMS] PROGMEM = 
{
	{
		{OFF, ON,1,1,OFF},				// Reverse
		{OFF, ON,1,1,OFF},				// Reverse
		{OFF, ON,1,1,OFF},				// Reverse
		{OFF, ON,1,1,OFF},				// Reverse
		{OFF, ON,1,1,OFF},				// Reverse
		{OFF, ON,1,1,OFF},				// Reverse
		{OFF, ON,1,1,OFF},				// Reverse
		{OFF, ON,1,1,OFF},				// Reverse
	},
	{
		{-125,125,1,0,0}, 				// Offset
		{-125,125,1,0,0}, 				// Offset
		{-125,125,1,0,0}, 				// Offset
		{-125,125,1,0,0}, 				// Offset
		{-125,125,1,0,0}, 				// Offset
		{-125,125,1,0,0}, 				// Offset
		{-125,125,1,0,0}, 				// Offset
		{-125,125,1,0,0}, 				// Offset
	},
	{
		{-125,125,5,0,-100}, 			// Min travel
		{-125,125,5,0,-100}, 			// Min travel
		{-125,125,5,0,-100}, 			// Min travel
		{-125,125,5,0,-100}, 			// Min travel
		{-125,125,5,0,-100}, 			// Min travel
		{-125,125,5,0,-100}, 			// Min travel
		{-125,125,5,0,-100}, 			// Min travel
		{-125,125,5,0,-100}, 			// Min travel
	},
	{
		{-125,125,5,0,100}, 			// Max travel
		{-125,125,5,0,100}, 			// Max travel
		{-125,125,5,0,100}, 			// Max travel
		{-125,125,5,0,100}, 			// Max travel
		{-125,125,5,0,100}, 			// Max travel
		{-125,125,5,0,100}, 			// Max travel
		{-125,125,5,0,100}, 			// Max travel
		{-125,125,5,0,100}, 			// Max travel
	},
	{
		{-125,125,5,0,0}, 				// Failsafe
		{-125,125,5,0,0}, 				// Failsafe
		{-125,125,5,0,0}, 				// Failsafe
		{-125,125,5,0,0}, 				// Failsafe
		{-125,125,5,0,0}, 				// Failsafe
		{-125,125,5,0,0}, 				// Failsafe
		{-125,125,5,0,0}, 				// Failsafe
		{-125,125,5,0,0}, 				// Failsafe
	}
};

//************************************************************
// Main menu-specific setup
//************************************************************

void menu_servo_setup(uint8_t section)
{
	static	uint8_t servo_top = SERVOSTART;
	static	uint8_t old_section;

	int8_t values[SERVOITEMS];
	menu_range_t range;
	uint8_t text_link;
	uint8_t i = 0;

	// Get menu offsets
	// 1 = Reverse, 2 = Offset, 3 = Min, 4 = Max, 5 = Failsafe

	// If submenu item has changed, reset submenu positions
	if (section != old_section)
	{
		servo_top = SERVOSTART;
		old_section = section;
	}
		
	while(button != BACK)
	{
		// Load values from eeprom
		for (i = 0; i < SERVOITEMS; i++)
		{
			switch(section)
			{
				case 1:
					memcpy(&values[i],&Config.Channel[i].Servo_reverse,sizeof(int8_t));
					break;
				case 2:
					memcpy(&values[i],&Config.Channel[i].Offset,sizeof(int8_t));
					break;
				case 3:
					memcpy(&values[i],&Config.Channel[i].min_travel,sizeof(int8_t));
					break;
				case 4:
					memcpy(&values[i],&Config.Channel[i].max_travel,sizeof(int8_t));
					break;
				case 5:
					memcpy(&values[i],&Config.Channel[i].Failsafe,sizeof(int8_t));
					break;
				default:
					memcpy(&values[i],&Config.Channel[i].Servo_reverse,sizeof(int8_t));
					break;
			}
		}

		// Print menu
		print_menu_items(servo_top, SERVOSTART, &values[0], SERVOITEMS, (prog_uchar*)servo_menu_ranges[section - 1], SERVOOFFSET, (prog_uchar*)ServoMenuText[section - 1], cursor);

		// Handle menu changes
		update_menu(SERVOITEMS, SERVOSTART, 0, button, &cursor, &servo_top, &menu_temp);
		range = get_menu_range ((prog_uchar*)servo_menu_ranges[section - 1], menu_temp - SERVOSTART);

		if (button == ENTER)
		{
			text_link = pgm_read_byte(&ServoMenuText[section - 1][menu_temp - SERVOSTART]);
			values[menu_temp - SERVOSTART] = do_menu_item(menu_temp, values[menu_temp - SERVOSTART], range, 0, text_link);
		}

		// Update value in config structure
		for (i = 0; i < SERVOITEMS; i++)
		{
			switch(section)
			{
				case 1:
					memcpy(&Config.Channel[i].Servo_reverse,&values[i],sizeof(int8_t));
					break;
				case 2:
					memcpy(&Config.Channel[i].Offset,&values[i],sizeof(int8_t));
					break;
				case 3:
					memcpy(&Config.Channel[i].min_travel,&values[i],sizeof(int8_t));
					break;
				case 4:
					memcpy(&Config.Channel[i].max_travel,&values[i],sizeof(int8_t));
					break;
				case 5:
					memcpy(&Config.Channel[i].Failsafe,&values[i],sizeof(int8_t));
					break;
				default:
					memcpy(&Config.Channel[i].Servo_reverse,&values[i],sizeof(int8_t));
					break;
			}
		}

		if (button == ENTER)
		{
			Save_Config_to_EEPROM(); // Save value and return
		}
	}

	_delay_ms(200);
}

