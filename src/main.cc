// C++ LIBS
#include <iostream>
#include <bitset>
#include <cmath>
#include <fstream>

// C LIBS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// OBTAINED C LIBS
#include <wiringPi.h>
#include <wiringPiSPI.h>

// CREATED C LIBS
#include <ltc2983.hpp>

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
    ltc2983_0.setup_thermocouple(CHANNEL_3,  TYPE_K, CJ_CHNL_1,  SNGL, OC_CHK_ON, TC_100UA);
    ltc2983_0.setup_thermocouple(CHANNEL_7,  TYPE_K, CJ_CHNL_5,  SNGL, OC_CHK_ON, TC_100UA);
    ltc2983_0.setup_thermocouple(CHANNEL_11, TYPE_K, CJ_CHNL_9,  SNGL, OC_CHK_ON, TC_100UA);
    ltc2983_0.setup_thermocouple(CHANNEL_15, TYPE_K, CJ_CHNL_13, SNGL, OC_CHK_ON, TC_100UA);
    ltc2983_0.setup_thermocouple(CHANNEL_19, TYPE_K, CJ_CHNL_17, SNGL, OC_CHK_ON, TC_100UA);
    ltc2983_0.setup_diode (CHANNEL_1,  SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);
    ltc2983_0.setup_diode (CHANNEL_5,  SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);
    ltc2983_0.setup_diode (CHANNEL_9,  SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);
    ltc2983_0.setup_diode (CHANNEL_13, SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);
    ltc2983_0.setup_diode (CHANNEL_17, SNGL, CONV_2, D_AVG_OFF, D_20UA, d_ideality_f);

    // Generate the bytes for the TXbuffer channel map
    ltc2983_0.write_all_channel_assignments(&spi_tx[0]);

    // Perform a conversion
    ltc2983_0.all_chnnel_conversion();
    for(int i = 0; i < 1800; i++)
    {
        ltc2983_0.all_chnnel_conversion();
        temperature = ltc2983_0.read_channel_double(CHANNEL_3);
        if(temperature == 9000)
            std::cout << "The temperature of channel " << CHANNEL_3 << " = ERROR" << "\n";
        else
            std::cout << "The temperature of channel " << CHANNEL_3 << " = " << temperature << "\n";
        results << temperature << '\n';
    }
    results.close();
    return 0;
}
