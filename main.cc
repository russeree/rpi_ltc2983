// C++ LIBS
#include <iostream>
#include <bitset> 
// C LIBS
#include <string.h>
#include <errno.h>
#include <stdio.h>

// OBTAINED C LIBS
#include <wiringPi.h>
#include <wiringPiSPI.h>

// CREATED C LIBS 
#include "ltc2983.h"

#define DEBUG

int init_ltc2983 (int spi_channel); 
int setup_thermocouple(unsigned int *channel_asgn, unsigned char tc_type, unsigned char cj_assignment,  bool snl_ended, bool oc_chk, unsigned char oc_curr);
int setup_diode(unsigned int *channel_asgn, bool snl_ended, bool three_readings, bool averaging, unsigned char exc_current, unsigned int ideality_f);
int tx_buffer_stitch (unsigned char *tx_buffer, unsigned char *dat_buffer, int bytes);
int gen_transaction(unsigned int *buff, unsigned char trans_type, unsigned short int address, unsigned char data);
int write_all_channel_assignments(unsigned char *tx_buff, unsigned int *asgn_table); 
unsigned int or_mask_gen(unsigned int value, unsigned int bit_pos);

int main()
{
    unsigned int chnl_asgn_map [20] = {}; // Intermap of LTC2983 ADC channel mapping
    unsigned char spi_tx [100] = {};      // Spi Tansaction buffer, This gets pushed out to the IC and replace with BCM rx spi pin data
    unsigned int trans_buff;              // Stores a word containing the transaction (32 bit) 
    int sts;
    int spi_chnl;
    // Setup Wiring Libs
    wiringPiSetupGpio();
    std::cout << "Raspi Thermocouple Hat Control Application\nEnter SPI channel of LTC2983 ";
    std::cin  >> spi_chnl;
    // Init the LTC2983 on input SPI channel
    init_ltc2983(spi_chnl);
    sts =  wiringPiSPIDataRW (spi_chnl, &spi_tx[0], 4);
    // Sets up Thermocouples and Diodes as CJ
    setup_thermocouple(&chnl_asgn_map[CHANNEL_3],  TYPE_K, CJ_CHNL_1,  SNGL, OC_CHK_ON, TC_10UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_7],  TYPE_K, CJ_CHNL_5,  SNGL, OC_CHK_ON, TC_10UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_11], TYPE_K, CJ_CHNL_9,  SNGL, OC_CHK_ON, TC_10UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_15], TYPE_K, CJ_CHNL_13, SNGL, OC_CHK_ON, TC_10UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_19], TYPE_K, CJ_CHNL_17, SNGL, OC_CHK_ON, TC_10UA);
    setup_diode (&chnl_asgn_map[CHANNEL_1],  SNGL, CONV_2, D_AVG_OFF, D_10UA, 0b0100000000110001001001);
    setup_diode (&chnl_asgn_map[CHANNEL_5],  SNGL, CONV_2, D_AVG_OFF, D_10UA, 0b0100000000110001001001);
    setup_diode (&chnl_asgn_map[CHANNEL_9],  SNGL, CONV_2, D_AVG_OFF, D_10UA, 0b0100000000110001001001);
    setup_diode (&chnl_asgn_map[CHANNEL_13], SNGL, CONV_2, D_AVG_OFF, D_10UA, 0b0100000000110001001001);
    setup_diode (&chnl_asgn_map[CHANNEL_17], SNGL, CONV_2, D_AVG_OFF, D_10UA, 0b0100000000110001001001);
    
    // Print the channel configs
    for (int i = 0; i < 20; i++) 
        std::cout << std::bitset<32>(chnl_asgn_map[i]) << '\n';

    gen_transaction(&trans_buff, READ, 0x0F0F, 0x0A);
    std::cout << trans_buff << '\n';
    return 0;
}

// This initializes the LTC2983
int init_ltc2983 (int spi_channel)
{
    int status;
    // Check to make sure the spi channel exists return 2;
    if ((spi_channel > 1) | (spi_channel < 0))
    {
        std::cout << "SPI channel is invalid, 0 and 1 are Supported\n";
        return 2; 
    }
    // Reset Device and set GPIO pin 26 high [This is the RST signal]
    pinMode(26, OUTPUT);
    digitalWrite(26, LOW);
    delay(100);
    pinMode(26, OUTPUT);
    digitalWrite(26, HIGH);
    delay(300);

    // Setup SPI based on the SS channel R[12,13] is jumped to. [Will print errors]
    if (wiringPiSPISetup (spi_channel, 50000) < 0)
    {
      fprintf (stderr, "SPI Setup failed: %s\n", strerror (errno));
      return 1; 
    }
    return 0;
}

// Setup Thermocouple on a Channel
// PARS
// channel_asgn  = channel in application channel mapping
// tc_type       = value of thermocouple type being used *READ DEFINES*
// cj_assignment = cold junction matched channel *READ DEFINES*
// sgl_ended     = single ended operation flag
// oc_chk        = overcurrent checking, disable to use external exciation source 
// oc_curr       = excitation current output in microamps
int setup_thermocouple(unsigned int *channel_asgn, unsigned char tc_type, unsigned char cj_assignment,  bool snl_ended, bool oc_chk, unsigned char oc_curr)
{
    unsigned int dat_buff = 0x0000; 
    dat_buff |= or_mask_gen(oc_curr, 18);
    dat_buff |= or_mask_gen(oc_chk, 20);
    dat_buff |= or_mask_gen(snl_ended, 21); 
    dat_buff |= or_mask_gen(cj_assignment, 22);
    dat_buff |= or_mask_gen(tc_type, 27); 
    *channel_asgn = dat_buff;
    return 0;
}

// Setup a Thermal Diode on a Channel
// PARS
// channel_asgn   = Channel in application channel mapping
// snl_ended      = Singleended configuration
// three_readings = Three samples or Two 
// averaging      = Averages Samples
// exc_current    = Excitation curretn settings
// ideality_f     = Ideality Factor
int setup_diode(unsigned int *channel_asgn, bool snl_ended, bool three_readings, bool averaging, unsigned char exc_current, unsigned int ideality_f)
{
    unsigned int dat_buff = 0x0000;
    dat_buff |= or_mask_gen(ideality_f, 0);
    dat_buff |= or_mask_gen(exc_current, 22);
    dat_buff |= or_mask_gen(averaging, 24);
    dat_buff |= or_mask_gen(three_readings, 25);
    dat_buff |= or_mask_gen(snl_ended, 26);
    dat_buff |= or_mask_gen(0b11100, 27);
    *channel_asgn = dat_buff;
    return 0;
}

// Copies memory from one location into another, data must be read in as unsigned chars
int tx_buffer_stitch (unsigned char *tx_buffer, unsigned char *dat_buffer, int bytes) 
{
    for (int i = 0; i < bytes; i++)
    {
        tx_buffer[i] = dat_buffer[i];
    }
    return 0;
}
// Generates a mask that is meant to be bitwise OR'd with an unsigned int
unsigned int or_mask_gen(unsigned int value, unsigned int bit_pos)
{
    return (value << bit_pos); 
}

// Generates 32 bits for a SPI transaction to the LTC2983, This is written as a series of 4 byes to the buffer
int gen_transaction(unsigned int *buff, unsigned char trans_type, unsigned short int address, unsigned char data)
{
    unsigned char t_buff [4] = {};
    t_buff[3] = trans_type;
    t_buff[2] = address & 0xff;
    t_buff[1] = (address >> 8) & 0xff;
    t_buff[0] = data;
    for (int i = 0; i < 4; i++)
    {
        *buff |= (t_buff[i] << (i*8));
    }

    return 0;
}

// This will configure all the channels on the LTC2983 with the channel assignment data table
int write_all_channel_assignments(unsigned int *tx_buff, unsigned int *asgn_table) 
{
    unsigned int trans_buff;      // Buffers the full tranaction byte to be written to the LTC2983
    unsigned char byte_buff [4]; // Used to buffer a word into a byte   
    // Generate the first transaction to burst from then write all subsequent bytes 
    for(int i = 0; i < 20; i++)
    {
        // Convert the chaneel DATA into a 4 byte array
        byte_buff[0] =  asgn_table[i] & 0xff;
        byte_buff[1] = (asgn_table[i] >> 8) & 0xff;
        byte_buff[2] = (asgn_table[i] >> 16) & 0xff;
        byte_buff[3] = (asgn_table[i] >> 24) & 0xff;
        // Now that the byte has been created with the channel data, write and inital packet conaining the start address, then write the subsequent bytes
        gen_transaction(&trans_buff, WRITE, (0x0200 + (i * 4)), byte_buff[0]);
        #ifdef DEBUG
            std::cout << "Channel " << i << " config transaction value = " << trans_buff << '\n';
        #endif

    }
    return 0;
}
