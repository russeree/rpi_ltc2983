// C++ LIBS
#include <iostream>
#include <bitset>
#include <cmath>
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
int get_command_status(int spi_channel);  
int read_data (unsigned short int address, unsigned char *results, unsigned int bytes, unsigned int spi_channel); 
int setup_thermocouple(unsigned int *channel_asgn, unsigned char tc_type, unsigned char cj_assignment,  bool snl_ended, bool oc_chk, unsigned char oc_curr);
int setup_diode(unsigned int *channel_asgn, bool snl_ended, bool three_readings, bool averaging, unsigned char exc_current, unsigned int ideality_f);
int tx_buffer_stitch (unsigned char *tx_buffer, unsigned char *dat_buffer, int bytes);
int gen_transaction(unsigned int *buff, unsigned char trans_type, unsigned short int address, unsigned char data);
int write_all_channel_assignments(unsigned char *tx_buff, unsigned int *asgn_table, int spi_channel);
int all_chnnel_conversion(int spi_channel);
int read_channel_value(int spi_channel, int channel_number, unsigned char *results);
unsigned int or_mask_gen(unsigned int value, unsigned int bit_pos);
int bin_to_temp(unsigned int *results, double output, unsigned int channel);  

int main()
{
    const unsigned int d_ideality_f = 0x00101042; // Diode ideality factor of ~ 1.04
    unsigned int chnl_asgn_map [20] = {};         // Intermap of LTC2983 ADC channel mapping
    unsigned char spi_tx [200] = {};              // SPI Tansaction buffer, This gets pushed out to the IC and replace with BCM rx spi pin data
    unsigned char spi_rx [200] = {};              // SPI rx buffer for read data transactions
    unsigned int trans_buff;                      // Stores a word containing the transaction (32 bit)

    int sts;
    int spi_chnl;
    // Setup Wiring Libs
    wiringPiSetupGpio();
    std::cout << "Raspi Thermocouple Hat Control Application\nEnter SPI channel of LTC2983 ";
    std::cin  >> spi_chnl;
    // Init the LTC2983 on input SPI channel
    init_ltc2983(spi_chnl);
    // Sets up Thermocouples and Diodes as CJ
    setup_thermocouple(&chnl_asgn_map[CHANNEL_3],  TYPE_K, CJ_CHNL_1,  SNGL, OC_CHK_ON, TC_100UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_7],  TYPE_K, CJ_CHNL_5,  SNGL, OC_CHK_ON, TC_100UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_11], TYPE_K, CJ_CHNL_9,  SNGL, OC_CHK_ON, TC_100UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_15], TYPE_K, CJ_CHNL_13, SNGL, OC_CHK_ON, TC_100UA);
    setup_thermocouple(&chnl_asgn_map[CHANNEL_19], TYPE_K, CJ_CHNL_17, SNGL, OC_CHK_ON, TC_100UA);
    setup_diode (&chnl_asgn_map[CHANNEL_1],  SNGL, CONV_3, D_AVG_OFF, D_20UA, d_ideality_f);
    setup_diode (&chnl_asgn_map[CHANNEL_5],  SNGL, CONV_3, D_AVG_OFF, D_20UA, d_ideality_f);
    setup_diode (&chnl_asgn_map[CHANNEL_9],  SNGL, CONV_3, D_AVG_OFF, D_20UA, d_ideality_f);
    setup_diode (&chnl_asgn_map[CHANNEL_13], SNGL, CONV_3, D_AVG_OFF, D_20UA, d_ideality_f);
    setup_diode (&chnl_asgn_map[CHANNEL_17], SNGL, CONV_3, D_AVG_OFF, D_20UA, d_ideality_f);
    
    // Generate the bytes for the TXbuffer channel map
    write_all_channel_assignments(&spi_tx[0], &chnl_asgn_map[0], spi_chnl);
    
    // Read-Back Channel Table
    for(int i = 0; i < 20; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            trans_buff = 0;
            unsigned short int address = (0x0200 + (4*i) + j);
            gen_transaction(&trans_buff, READ, address, 0x00);
            for(int k = 0; k < 4; k++)
            {
                spi_tx[3-k] = (trans_buff >> (8*k)) & 0xff;
            }
            sts =  wiringPiSPIDataRW (spi_chnl, &spi_tx[0], 4);
        }
    }
    
    // Perform a conversion
    all_chnnel_conversion(spi_chnl);
    read_channel_value(spi_chnl, CHANNEL_3, &spi_tx[0]);
    read_data(CNV_RSLTS, &spi_rx[0], 80, spi_chnl);
    get_command_status(spi_chnl);

    return 0;

}

// This initializes the LTC2983
int init_ltc2983 (int spi_channel)
{
    int status;
    unsigned char spi_tx [4] = {}; // Spi Tansaction buffer, This gets pushed out to the IC and replace with BCM rx spi pin data
    unsigned int trans_buff;
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
    if (wiringPiSPISetup (spi_channel, 100000) < 0)
    {
      fprintf (stderr, "SPI Setup failed: %s\n", strerror (errno));
      return 1; 
    }
    // Setup the device in Fahrenheit mode with 50/60HZ Rejection filters both enabled.  
    gen_transaction(&trans_buff, WRITE, 0x00F0, 0b00000100);
    for(int k = 0; k < 4; k++)
        spi_tx[3-k] = (trans_buff >> (8*k)) & 0xff;
    // Write the Global Configuration register
    status = wiringPiSPIDataRW (spi_channel, &spi_tx[0], 4);
    return 0;
}

// Setup Thermocouple on a Channel :VERI:
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

// Setup a Thermal Diode on a Channel :VERI:
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

// Copies memory from one location into another, data must be read in as unsigned chars :UNTESTED:
int tx_buffer_stitch (unsigned char *tx_buffer, unsigned char *dat_buffer, int bytes) 
{
    for (int i = 0; i < bytes; i++)
    {
        tx_buffer[i] = dat_buffer[i];
    }
    return 0;
}
// Generates a mask that is meant to be bitwise OR'd with an unsigned int :VERI:
unsigned int or_mask_gen(unsigned int value, unsigned int bit_pos)
{
    return (value << bit_pos); 
}

// Generates 32 bits for a SPI transaction to the LTC2983, This is written as a series of 4 byes to the buffer *CLEARS BUFFER CONTENTS* :VERI:
int gen_transaction(unsigned int *buff, unsigned char trans_type, unsigned short int address, unsigned char data)
{
    *buff = 0; 
    unsigned char t_buff [4] = {};
    t_buff[3] = trans_type;
    t_buff[1] = address & 0xff;
    t_buff[2] = (address >> 8) & 0xff;
    t_buff[0] = data;
    for (int i = 0; i < 4; i++)
    {
        *buff |= (t_buff[i] << (i*8));
    }
    return 0;
}

// his will configure all the channels on the LTC2983 with the channel assignment data table !!!!TEST ME!!!
// NEEDS wiringPI
int write_all_channel_assignments(unsigned char *tx_buff, unsigned int *asgn_table, int spi_channel) 
{
    unsigned int trans_buff = 0;       // Buffers the full tranaction byte to be written to the LTC2983
    unsigned char byte_buff [4] = {};  // Used to buffer a word into a byte
    unsigned char byte_null [4] = {};  // Null byte buffer used for transactions
    unsigned short int address = 0;    // Address Storage
    // Generate SPI Transactions 
    for(int i = 0; i < 20; i++)
    {
        std::cout << "Channel " << i << " data written." << '\n';
        // Convert the chaneel assignment into a 4 byte array for DEBUG read back
        for (int j = 0; j < 4; j++)
            byte_buff[3-j] = (asgn_table[i] >> j*8) & 0xFF;
        for (int j = 0; j < 4; j++)
        {   

            for(int k = 0; k < 4; k++)
                byte_null[k] = byte_buff[k]; 
            address = (0x0200 + i*4 + j);
            #ifdef DEBUG
            std::cout << "Contentes of byte[" << j << "] tansacton = " << std::bitset<8>(byte_null[j]) << '\n';
            #endif
            gen_transaction(&trans_buff, WRITE, address, byte_null[j]);
            for(int k = 0; k < 4; k++)
                byte_null[3-k] = (trans_buff >> (8*k)) & 0xff;
            wiringPiSPIDataRW (spi_channel, &byte_null[0], 4);
        }
    }
    return 0;
}

// Initiate a complete converstion on all channels (No ARGS)
int all_chnnel_conversion(int spi_channel)
{
    unsigned int temp = 0;
    unsigned char tx_buff [4];
    unsigned int all_mask = 0x000FFFFF;
    unsigned short int address = 0x00F4;
    pinMode(19, INPUT);
    for(int i = 0; i < 4; i++)
    {
        gen_transaction(&temp, WRITE, (address + i), (all_mask >> (32-(8*i)) & 0xff));
        for(int j = 0; j < 4; j++)
        {
            tx_buff[3-j] = (temp >> (8*j)) & 0xff;
        }
        wiringPiSPIDataRW (spi_channel, &tx_buff[0], 4);
    }
    gen_transaction(&temp, WRITE, 0x0000, 0x80);
    for(int i = 0; i < 4; i++)
        tx_buff[3-i] = (temp >> (8*i)) & 0xff;
    wiringPiSPIDataRW (spi_channel, &tx_buff[0], 4);
    while(!digitalRead(19));
    std::cout << "conversion_complete\n";
    return 0;
}

// Channels start at 0
int read_channel_value(int spi_channel, int channel_number, unsigned char *results)
{
    unsigned short int base_addr = 0x0010;
    unsigned int trans_buff;
    unsigned char tx_buff[4];
    for (int i = 0; i < 4; i++)
    {
        gen_transaction(&trans_buff, READ, (base_addr + 4*channel_number + i), 0x00);
        for(int j = 0; j < 4; j++)
            tx_buff[3-j] = (trans_buff >> (8*j)) & 0xff;
        wiringPiSPIDataRW (spi_channel, &tx_buff[0], 4);
        results[i] = tx_buff[3];
    }
    return 0;
}

/**
 * @desc: Convert channel data to a real number
 * @param: [results] Points to the output byte of the the LTC2983
 * @param: [output] Points to the output double that contains the real num temp result
 * @param: [channe] The channel number of results to be converted: this is used to get the insturment type and catch errors
 **/
int bin_to_temp(unsigned int *results, double output, unsigned int channel)
{
    // Read out the channel configuration

    return 0; 
}

/**
 * @desc: Reads a word over SPI
 * @param: [address] The address you want to begin the word read from; 
 **/
int read_data (unsigned short int address, unsigned char *results, unsigned int bytes, unsigned int spi_channel)
{
    unsigned int tx_buff;          // Stores the complete transaction
    unsigned char tx[bytes] = {};  // you could use your results pointer but mem is cheap
    for(int i = 0; i < bytes; i++)
    { 
        tx_buff = 0;
        gen_transaction(&tx_buff, READ, (address + i), 0x00);
        for(int j = 0; j < 4; j++)
        {
            tx[3-j] = (tx_buff >> (8*j)) & 0xff;
        }
        wiringPiSPIDataRW (spi_channel, &tx[0], 4);
        results[i] = tx[3];
    }
    return 0;
}

/** 
 * @desc: Read the Command Regiser and prints a Readable message if DEBUG is defined.
 * @param: [spi_channel] the spi channel that commands will be written to.
 **/
int get_command_status(int spi_channel)
{
    int status;
    unsigned char status_reg;
    read_data(0x0000, &status_reg, 1, spi_channel);
    switch(status_reg)
    {
        case 0x80: 
        {
            #ifdef DEBUG
            std::cout << "LTC2983 is initialized with no channel configuration.\n";
            #endif
            status = 2;
            break;
        }
        case 0x40: 
        {
            #ifdef DEBUG
            std::cout << "LTC2983 is ready for use\n";
            #endif
            status = 1;
            break;
        }
        default: 
        {
            #ifdef DEBUG
            std::cout << "Result " << std::bitset<8>(status_reg) << " was not defined as a valid response.\n";
            #endif
            status = 3;
            break;
        }
    }
    return status;
}
