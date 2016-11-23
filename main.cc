// C++ LIBS
#include <iostream>
#include <bitset>
#include <cmath>
#include <fstream>

// C LIBS
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// OBTAINED C LIBS
#include <wiringPi.h>
#include <wiringPiSPI.h>

// CREATED C LIBS
#include "ltc2983.h"

// #define DEBUG // Default Debug verbosity:
// #define DEBUG_L1 // DEBUG Verbosity of level 1: Extended error messages: Byte level transation and buffer readouts

int main(int argc, char *argv[])
{
    std::ofstream results;
    results.open("results.txt");
    const unsigned int d_ideality_f = 0x00101042; // Diode ideality factor of ~ 1.04
    unsigned int chnl_asgn_map [20] = {};         // Intermap of LTC2983 ADC channel mapping
    unsigned char spi_tx [200] = {};              // SPI Tansaction buffer, This gets pushed out to the IC and replace with BCM rx spi pin data
    unsigned char spi_rx [200] = {};              // SPI rx buffer for read data transactions
    unsigned int trans_buff;                      // Stores a word containing the transaction (32 bit)
    float temperature;                            // Final temperature

    int sts;
    int spi_chnl;
    // Setup Wiring Libs
    wiringPiSetupGpio();
    std::cout << "Raspi Thermocouple Hat Control Application\nEnter SPI channel of LTC2983 ";
    std::cin  >> spi_chnl;
    ltc2983 ltc2983_0(spi_chnl);
    // Init the LTC2983 on input SPI channel
    // Sets up Thermocouples and Diodes as CJ
    setup_thermocouple(&chnl_asgn_map[CHANNEL_3],  TYPE_K, CJ_CHNL_1,  SNGL, OC_CHK_ON, TC_100UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_7],  TYPE_K, CJ_CHNL_5,  SNGL, OC_CHK_ON, TC_100UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_11], TYPE_K, CJ_CHNL_9,  SNGL, OC_CHK_ON, TC_100UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_15], TYPE_K, CJ_CHNL_13, SNGL, OC_CHK_ON, TC_100UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_19], TYPE_K, CJ_CHNL_17, SNGL, OC_CHK_ON, TC_100UA);
    setup_diode (&chnl_asgn_map[CHANNEL_1],  SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);
    setup_diode (&chnl_asgn_map[CHANNEL_5],  SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);
    setup_diode (&chnl_asgn_map[CHANNEL_9],  SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);
    setup_diode (&chnl_asgn_map[CHANNEL_13], SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);
    setup_diode (&chnl_asgn_map[CHANNEL_17], SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);

    // Generate the bytes for the TXbuffer channel map
    write_all_channel_assignments(&spi_tx[0], &chnl_asgn_map[0], spi_chnl);

    // Perform a conversion
    all_chnnel_conversion(spi_chnl);
    for(int i = 0; i < 1800; i++)
    {
        all_chnnel_conversion(spi_chnl);
        temperature = read_channel_double(spi_chnl, 2);
        if(temperature == 9000)
            std::cout << "The temperature of channel " << 2 << " = ERROR" << "\n";
        else
            std::cout << "The temperature of channel " << 2 << " = " << temperature << "\n";
        results << temperature << '\n';
    }
    results.close();
    return 0;
}
