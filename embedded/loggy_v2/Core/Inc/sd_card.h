/*
 * sd_card.h
 *
 *  Created on: Apr 01, 2025
 *      Author: Lucy
 */

#ifndef INC_SD_CARD_H_
#define INC_SD_CARD_H_

#define RECORDING 1
#define NOT_RECORDING 0

#define OPEN 1
#define CLOSED 0

#define NUMBER_CHANNELS  8   // Define number of channels
#define DELAY            0.5f // Define delay (using 'f' for float)

int initialise_sd_card();
void update_sd_card();
void write_data(FIL *fil, float channelReading[]);
void initialise_headers(FIL *fil, uint8_t unit[]);

#endif /* INC_SD_CARD_H_ */
