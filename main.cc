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
    unsigned char spi_tx [200] = {};      // Spi Tansaction buffer, This gets pushed out to the IC and replace with BCM rx spi pin data
    unsigned int trans_buff;              // Stores a word containing the transaction (32 bit) 
    int sts;
    int spi_chnl;
    // Setup Wiring Libs
    wiringPiSetupGpio();
    std::cout << "Raspi Thermocouple Hat Control Application\nEnter SPI channel of LTC2983 ";
    std::cin  >> spi_chnl;
    // Init the LTC2983 on input SPI channel
    init_ltc2983(spi_chnl);
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
    
    // Generate the bytes for the TXbuffer channel map
    write_all_channel_assignments(&spi_tx[0], &chnl_asgn_map[0]);
    sts =  wiringPiSPIDataRW (spi_chnl, &spi_tx[0], 140);
    
    spi_tx[0] = 0x03;
    spi_tx[1] = 0x02;
    spi_tx[2] = 0x00;
    spi_tx[3] = 0x00;

    sts =  wiringPiSPIDataRW (spi_chnl, &spi_tx[0], 4);

    for(int i = 0; i < 20; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            trans_buff = 0;
            unsigned short int address = (0x0200 + (4*i) + j);
            gen_transaction(&trans_buff, READ, 0xff, 0x00);
            std::cout << std::hex << trans_buff << "\n";
            std::cout << std::hex << address << "\n";
            for(int k = 0; k < 4; k++)
                spi_tx[k-3] = (trans_buff >> (8*k)) & 0xff;
            sts =  wiringPiSPIDataRW (spi_chnl, &spi_tx[0], 4);
            for(int k = 0; k < 4; k++)
                std::cout << spi_tx[k];
            std::cout << '\n';
        }
    }

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

// Generates 32 bits for a SPI transaction to the LTC2983, This is written as a series of 4 byes to the buffer *CLEARS BUFFER CONTENTS*
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

// This will configure all the channels on the LTC2983 with the channel assignment data table !!!!TEST ME!!!
int write_all_channel_assignments(unsigned char *tx_buff, unsigned int *asgn_table) 
{
    unsigned int trans_buff = 0;       // Buffers the full tranaction byte to be written to the LTC2983
    unsigned char byte_buff [4] = {};  // Used to buffer a word into a byte
    unsigned short int address = 0;    // Address Storage
    // Generate the first transaction to burst from then write all subsequent bytes 
    for(int i = 0; i < 20; i++)
    {
        std::cout << "\nChannel " << i << '\n';
        address = 0x0200 + (i * 0x04); 
        // Convert the chaneel assignment into a 4 byte array for DEBUG read back
        for (int j = 0; j < 4; j++)
            byte_buff[3-j] = (asgn_table[i] >> j*8) & 0xFF;
        for (int j = 0; j < 4; j++)
            std::cout << "Byte " << j << " Channel " << i << " = " << std::bitset<8>(byte_buff[j]) << '\n';
        // Now that the byte has been created with the channel data, write and inital packet conaining the start address, then write the subsequent bytes
        std::cout << "Byte buffer value 0 = "  << std::bitset<8>(byte_buff[0]) << '\n';
        std::cout << "Address value = " << std::hex << address << '\n';
        gen_transaction(&trans_buff, WRITE, address, byte_buff[0]);
        #ifdef DEBUG
            std::cout << "Assignment Table Enrty for Channel " << i << " = " << std::bitset<32>(asgn_table[i]) << '\n';
            std::cout << "Channel " << std::dec << i << " config transaction value   = " << std::bitset<32>(trans_buff) << '\n';
        #endif
        // Place the Burst frame header that was generated into the first 4 byes of the tx buffer. (The document state that aditional data can be written this could be flawed)
        for(int j = 0; j < 4; j++)
            tx_buff[(i * 7) + (3 -j)] = ((trans_buff >> j*8) & 0xff);
        // Next Write the Aditional 3 Data Bytes to the buffer.
        for(int j = 0; j < 3; j++) 
            tx_buff[i * 7 + j + 4] = byte_buff[j+1]; 
    }
    std::cout << "\ncomplete bitstring to be sent:\n";
    for(int i = 0; i < 140; i++)
        std::cout << std::bitset<8>(tx_buff[i]); 
    std::cout << '\n';
    return 0;
}
