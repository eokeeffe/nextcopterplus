//***********************************************************
//* menu_mixer.c
//***********************************************************

//***********************************************************
//* Includes
//***********************************************************

#include "compiledefs.h"
#include <avr/pgmspace.h> 
#include <avr/io.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include "io_cfg.h"
#include "init.h"
#include "mugui.h"
#include "glcd_menu.h"
#include "menu_ext.h"
#include "glcd_driver.h"
#include "main.h"
#include "eeprom.h"
#include "mixer.h"

//************************************************************
// Prototypes
//************************************************************

// Menu items
void menu_mixer(uint8_t i);

//************************************************************
// Defines
//************************************************************

#define MIXERITEMS 38	// Number of mixer menu items
#define MIXERSTART 192 	// Start of Menu text items
#define MIXOFFSET  89	// Value offsets

//************************************************************
// RC menu items
//************************************************************
	 
const uint8_t MixerMenuText[MIXERITEMS] PROGMEM = 
{
	46,0,0,0,0,0,0,56,
	143,143,143,143,143,143,143,143,143,143,143,143,
	0,0,0,0,0,0,230,0,230,0,230,0,230,0,230,0,230,0
};

const menu_range_t mixer_menu_ranges[MIXERITEMS] PROGMEM = 
{
		// Motor control
		{SERVO,MOTOR,1,1,SERVO},		// Motor marker (0)
		{1,99,1,0,50},					// P1.n Position (%)
		{-125,125,1,0,0},				// P1 Offset (%)
		{-125,125,1,0,0},				// P1.n Offset (%)
		{-125,125,1,0,0},				// P2 Offset (%)
		{0,125,1,0,100},				// P1 throttle volume (5)
		{0,125,1,0,100},				// P2 throttle volume
#ifdef KK21
		{LINEAR,SQRTSINE,1,1,LINEAR},	// Throttle curves for KK2.1
#else
		{LINEAR,SINE,1,1,LINEAR},		// Throttle curves for KK2.0
#endif

		// Mixer ranges
		{OFF, SCALE,1,1,OFF},			// P1 roll_gyro (8)
		{OFF, SCALE,1,1,OFF},			// P2 roll_gyro
		{OFF, SCALE,1,1,OFF},			// P1 pitch_gyro
		{OFF, SCALE,1,1,OFF},			// P2 pitch_gyro
		{OFF, SCALE,1,1,OFF},			// P1 yaw_gyro
		{OFF, SCALE,1,1,OFF},			// P2 yaw_gyro
		{OFF, SCALE,1,1,OFF},			// P1 roll_acc
		{OFF, SCALE,1,1,OFF},			// P2 roll_acc
		{OFF, SCALE,1,1,OFF},			// P1 pitch_acc
		{OFF, SCALE,1,1,OFF},			// P2 pitch_acc
		{OFF, SCALE,1,1,OFF},			// P1 Z_delta_acc
		{OFF, SCALE,1,1,OFF},			// P2 Z_delta_acc

		// Sources
		{-125,125,1,0,0},				// P1 Aileron volume (20)
		{-125,125,1,0,0},				// P2 Aileron volume
		{-125,125,1,0,0},				// P1 Elevator volume
		{-125,125,1,0,0},				// P2 Elevator volume
		{-125,125,1,0,0},				// P1 Rudder volume
		{-125,125,1,0,0},				// P2 Rudder volume
		{SRC1,NOMIX,1,1,NOMIX},			// P1 Source A
		{-125,125,1,0,0},				// P1 Source A volume
		{SRC1,NOMIX,1,1,NOMIX},			// P2 Source A
		{-125,125,1,0,0},				// P2 Source A volume
		{SRC1,NOMIX,1,1,NOMIX},			// P1 Source B
		{-125,125,1,0,0},				// P1 Source B volume
		{SRC1,NOMIX,1,1,NOMIX},			// P2 Source B
		{-125,125,1,0,0},				// P2 Source B volume
		{SRC1,NOMIX,1,1,NOMIX},			// P1 Source C
		{-125,125,1,0,0},				// P1 Source C volume
		{SRC1,NOMIX,1,1,NOMIX},			// P2 Source C
		{-125,125,1,0,0},				// P2 Source C volume
};

//************************************************************
// Main menu-specific setup
//************************************************************

void menu_mixer(uint8_t i)
{
	static uint8_t mix_top = MIXERSTART;
	int8_t *value_ptr;
	int8_t values[MIXERITEMS];
	menu_range_t range;
	uint8_t text_link = 0;

	// If submenu item has changed, reset submenu positions
	if (menu_flag)
	{
		mix_top = MIXERSTART;
		menu_flag = 0;
	}

	while(button != BACK)
	{
		value_ptr = &values[0];

		// Expand sensor bit values into values array
		if ((Config.Channel[i].P1_sensors & (1 << MotorMarker)) != 0)
		{
			values[0] = ON;
		}
		else
		{
			values[0] = OFF;
		}

		// P1 roll gyro
		if ((Config.Channel[i].P1_sensors & (1 << RollGyro)) != 0)
		{
			if ((Config.Channel[i].P1_scale & (1 << RollScale)) != 0)
			{	
				values[8] = SCALE;
			}
			else
			{
				values[8] = ON;
			}
		}
		else
		{
			values[8] = OFF;
		}

		// P2 roll gyro
		if ((Config.Channel[i].P2_sensors & (1 << RollGyro)) != 0)
		{
			if ((Config.Channel[i].P2_scale & (1 << RollScale)) != 0)
			{	
				values[9] = SCALE;
			}
			else
			{
				values[9] = ON;
			}
		}
		else
		{
			values[9] = OFF;
		}

		// P1 pitch gyro
		if ((Config.Channel[i].P1_sensors & (1 << PitchGyro)) != 0)
		{
			if ((Config.Channel[i].P1_scale & (1 << PitchScale)) != 0)
			{	
				values[10] = SCALE;
			}
			else
			{
				values[10] = ON;
			}
		}
		else
		{
			values[10] = OFF;
		}

		// P2 pitch gyro
		if ((Config.Channel[i].P2_sensors & (1 << PitchGyro)) != 0)
		{
			if ((Config.Channel[i].P2_scale & (1 << PitchScale)) != 0)
			{	
				values[11] = SCALE;
			}
			else
			{
				values[11] = ON;
			}
		}
		else
		{
			values[11] = OFF;
		}

		// P1 yaw_gyro
		if ((Config.Channel[i].P1_sensors & (1 << YawGyro)) != 0)
		{
			if ((Config.Channel[i].P1_scale & (1 << YawScale)) != 0)
			{	
				values[12] = SCALE;
			}
			else
			{
				values[12] = ON;
			}
		}
		else
		{
			values[12] = OFF;
		}

		// P2 yaw gyro
		if ((Config.Channel[i].P2_sensors & (1 << YawGyro)) != 0)
		{
			if ((Config.Channel[i].P2_scale & (1 << YawScale)) != 0)
			{	
				values[13] = SCALE;
			}
			else
			{
				values[13] = ON;
			}
		}
		else
		{
			values[13] = OFF;
		}

		// P1 roll acc
		if ((Config.Channel[i].P1_sensors & (1 << RollAcc)) != 0)
		{
			if ((Config.Channel[i].P1_scale & (1 << AccRollScale)) != 0)
			{	
				values[14] = SCALE;
			}
			else
			{
				values[14] = ON;
			}
		}
		else
		{
			values[14] = OFF;
		}

		// P2 roll acc
		if ((Config.Channel[i].P2_sensors & (1 << RollAcc)) != 0)
		{
			if ((Config.Channel[i].P2_scale & (1 << AccRollScale)) != 0)
			{	
				values[15] = SCALE;
			}
			else
			{
				values[15] = ON;
			}
		}
		else
		{
			values[15] = OFF;
		}

		// P1 pitch acc
		if ((Config.Channel[i].P1_sensors & (1 << PitchAcc)) != 0)
		{
			if ((Config.Channel[i].P1_scale & (1 << AccPitchScale)) != 0)
			{	
				values[16] = SCALE;
			}
			else
			{
				values[16] = ON;
			}
		}
		else
		{
			values[16] = OFF;
		}

		// P2 pitch acc
		if ((Config.Channel[i].P2_sensors & (1 << PitchAcc)) != 0)
		{
			if ((Config.Channel[i].P2_scale & (1 << AccPitchScale)) != 0)
			{	
				values[17] = SCALE;
			}
			else
			{
				values[17] = ON;
			}
		}
		else
		{
			values[17] = OFF;
		}

		// P1 Z delta acc
		if ((Config.Channel[i].P1_sensors & (1 << ZDeltaAcc)) != 0)
		{
			if ((Config.Channel[i].P1_scale & (1 << AccZScale)) != 0)
			{	
				values[18] = SCALE;
			}
			else
			{
				values[18] = ON;
			}
		}
		else
		{
			values[18] = OFF;
		}

		// P2 Z delta acc
		if ((Config.Channel[i].P2_sensors & (1 << ZDeltaAcc)) != 0)
		{
			if ((Config.Channel[i].P2_scale & (1 << AccZScale)) != 0)
			{	
				values[19] = SCALE;
			}
			else
			{
				values[19] = ON;
			}
		}
		else
		{
			values[19] = OFF;
		}

		// Assemble remaining byte data for P1 offset to throttle curve into array
		memcpy(&values[1],&Config.Channel[i].P1n_position, 7);
		// Assemble remaining byte data for P1_aileron_volume to P2_source_c_volume into array
		memcpy(&values[20],&Config.Channel[i].P1_aileron_volume, 18);

		// Print menu
		print_menu_items(mix_top, MIXERSTART, value_ptr, 1, (prog_uchar*)mixer_menu_ranges, 0, MIXOFFSET, (prog_uchar*)MixerMenuText, cursor);

		// Handle menu changes
		update_menu(MIXERITEMS, MIXERSTART, 0, button, &cursor, &mix_top, &menu_temp);
		range = get_menu_range ((prog_uchar*)mixer_menu_ranges, menu_temp - MIXERSTART);

		if (button == ENTER)
		{
			text_link = pgm_read_byte(&MixerMenuText[menu_temp - MIXERSTART]);
			do_menu_item(menu_temp, value_ptr + (menu_temp - MIXERSTART), 1, range, 0, text_link, false, 0);
		}

		// Save modified byte data for P1 offset to throttle curve back to Config
		memcpy(&Config.Channel[i].P1n_position,&values[1], 7);
		// Save modified byte data for P1_source_a to P2_source_c_volume back to Config
		memcpy(&Config.Channel[i].P1_aileron_volume,&values[20], 18);

		// Clear flags before reconstruction
		Config.Channel[i].P1_sensors = 0;
		Config.Channel[i].P2_sensors = 0;
		Config.Channel[i].P1_scale = 0;
		Config.Channel[i].P2_scale = 0;

		// Recompress byte data for servos back into bit values for the sensors byte
		if (values[0] == ON)
		{
			Config.Channel[i].P1_sensors |= (1 << MotorMarker);
		}
			else
		{
			Config.Channel[i].P1_sensors &= ~(1 << MotorMarker);	
		}

		// P1 roll gyro
		switch (values[8])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P1_scale |= (1 << RollScale);
			case ON:
				Config.Channel[i].P1_sensors |= (1 << RollGyro);
				break;
		}
		// P2 roll gyro
		switch (values[9])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P2_scale |= (1 << RollScale);
			case ON:
				Config.Channel[i].P2_sensors |= (1 << RollGyro);
				break;
		}
		// P1 pitch gyro
		switch (values[10])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P1_scale |= (1 << PitchScale);
			case ON:
				Config.Channel[i].P1_sensors |= (1 << PitchGyro);
				break;
		}
		// P2 pitch gyro
		switch (values[11])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P2_scale |= (1 << PitchScale);
			case ON:
				Config.Channel[i].P2_sensors |= (1 << PitchGyro);
				break;
		}
		// P1 yaw gyro
		switch (values[12])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P1_scale |= (1 << YawScale);
			case ON:
				Config.Channel[i].P1_sensors |= (1 << YawGyro);
				break;
		}
		// P2 yaw gyro
		switch (values[13])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P2_scale |= (1 << YawScale);
			case ON:
				Config.Channel[i].P2_sensors |= (1 << YawGyro);
				break;
		}
		// P1 roll acc
		switch (values[14])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P1_scale |= (1 << AccRollScale);
			case ON:
				Config.Channel[i].P1_sensors |= (1 << RollAcc);
				break;
		}
		// P2 roll acc
		switch (values[15])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P2_scale |= (1 << AccRollScale);
			case ON:
				Config.Channel[i].P2_sensors |= (1 << RollAcc);
				break;
		}
		// P1 pitch acc
		switch (values[16])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P1_scale |= (1 << AccPitchScale);
			case ON:
				Config.Channel[i].P1_sensors |= (1 << PitchAcc);
				break;
		}

		// P2 pitch acc
		switch (values[17])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P2_scale |= (1 << AccPitchScale);
			case ON:
				Config.Channel[i].P2_sensors |= (1 << PitchAcc);
				break;
		}
		// P1 Z delta acc
		switch (values[18])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P1_scale |= (1 << AccZScale);
			case ON:
				Config.Channel[i].P1_sensors |= (1 << ZDeltaAcc);
				break;
		}
		// P2 Z delta acc
		switch (values[19])
		{
			case OFF:
				break;
			case SCALE:
				Config.Channel[i].P2_scale |= (1 << AccZScale);
			case ON:
				Config.Channel[i].P2_sensors |= (1 << ZDeltaAcc);
				break;
		}

		// Save and exit
		if (button == ENTER)
		{
			UpdateLimits();			 // Update travel limits based on percentages
			Save_Config_to_EEPROM(); // Save value and return
		}

	} // while(button != BACK)

	_delay_ms(200);
}



