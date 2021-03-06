// C++ LIBS
#include <iostream>
#include <bitset>
#include <fstream>
// C LIBS
#include <string.h>
#include <errno.h>
#include <stdio.h>
// OBTAINED C LIBS
#include <wiringPi.h>
#include <wiringPiSPI.h>
// CREATED C LIBS
#include <ltc2983.hpp>

/**
 * @desc: initializes the ltc2983
 * @param: [spi_channel] the spi_channel that will be used
 **/
ltc2983::ltc2983(int spi_channel)
{
  unsigned char spi_tx [4] = {}; // Spi Tansaction buffer, This gets pushed out to the IC and replace with BCM rx spi pin data
  unsigned int trans_buff;
  // Check to make sure the spi channel exists return 2;
  try
  {
    if ((spi_channel > 1) | (spi_channel < 0))
      throw 2;
    else
      this -> spi_channel = spi_channel;
  }
  catch(int err)
  {
    std::cout << "SPI channel is invalid, 0 and 1 are Supported\n";
  }
  // Reset Device and set GPIO pin 26 high [This is the RST signal]
  pinMode(26, OUTPUT);
  digitalWrite(26, LOW);
  delay(100);
  pinMode(26, OUTPUT);
  digitalWrite(26, HIGH);
  delay(300);
  // Setup SPI based on the SS channel R[12,13] is jumped to. [Will print errors]
  try{
    if (wiringPiSPISetup (spi_channel, 100000) < 0)
      throw 1;
  }
  catch(int err)
  {
    fprintf (stderr, "SPI Setup failed: %s\n", strerror (errno));
  }
  // Setup the device in Fahrenheit mode with 50/60HZ Rejection filters both enabled.
  gen_transaction(&trans_buff, WRITE, 0x00F0, 0b00000100);
  for(int k = 0; k < 4; k++)
      spi_tx[3-k] = (trans_buff >> (8*k)) & 0xff;
  // Write the Global Configuration register
  this -> status = wiringPiSPIDataRW (spi_channel, &spi_tx[0], 4);
}
/**
 * @desc: LTC2983 destructor
 **/
ltc2983::~ltc2983()
{
}
/**
 * @desc: Returns the shared porinter of ltc2983
 **/
std::shared_ptr<ltc2983> ltc2983::getptr(void)
{
    return shared_from_this();
}
/**
 * @desc: Read the Command Regiser and prints a Readable message if DEBUG is defined.
 **/
int ltc2983::get_command_status(void)
{
    int status;
    unsigned char status_reg;
    read_data(0x0000, &status_reg, 1);
    #ifdef DEBUG
    std::cout << "Command register read and ";
    #endif
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
            std::cout << "result " << std::bitset<8>(status_reg) << " was not defined as a valid response.\n";
            #endif
            status = 3;
            break;
        }
    }
    return status;
}
/**
 * @desc: Reads a a predetermined number of bytes over SPI
 * @param: [address] The address you want to begin the word read from;
 **/
int ltc2983::read_data (unsigned short int address, unsigned char *results, unsigned int bytes)
{
    unsigned int tx_buff;              // Stores the complete transaction
    unsigned char tx[bytes * 4] = {};  // you could use your results pointer but mem is cheap
    for(int i = 0; i < bytes; i++)
    {
        tx_buff = 0;
        gen_transaction(&tx_buff, READ, (address + i), 0x00);
        for(int j = 0; j < 4; j++)
        {
            tx[3-j] = (tx_buff >> (8*j)) & 0xff;
        }
        wiringPiSPIDataRW (this -> spi_channel, &tx[0], 4);
        results[i] = tx[3];
    }
    return 0;
}
/**
 * @desc: Setup Thermocouple on a Channel :VERI:
 * @param: [channel_asgn] channel in application channel mapping, Channels Start @ 1
 * @param: [tc_type] value of thermocouple type being used *READ DEFINES*
 * @param: [cj_assignment] cold junction matched channel *READ DEFINES*
 * @param: [sgl_ended] single ended operation flag
 * @param: [oc_chk] overcurrent checking, disable to use external exciation source
 * @param: [excitation] current output in microamps
 **/
 int ltc2983::setup_thermocouple(unsigned int channel, unsigned char tc_type, unsigned char cj_assignment,  bool snl_ended, bool oc_chk, unsigned char oc_curr)
 {
     unsigned int dat_buff = 0x0000;
     dat_buff |= or_mask_gen(oc_curr, 18);
     dat_buff |= or_mask_gen(oc_chk, 20);
     dat_buff |= or_mask_gen(snl_ended, 21);
     dat_buff |= or_mask_gen(cj_assignment, 22);
     dat_buff |= or_mask_gen(tc_type, 27);
     this -> chnl_asgn_map[channel - 1] = dat_buff;
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
int ltc2983::setup_diode(unsigned int channel, bool snl_ended, bool three_readings, bool averaging, unsigned char exc_current, unsigned int ideality_f)
{
    unsigned int dat_buff = 0x0000;
    dat_buff |= or_mask_gen(ideality_f, 0);
    dat_buff |= or_mask_gen(exc_current, 22);
    dat_buff |= or_mask_gen(averaging, 24);
    dat_buff |= or_mask_gen(three_readings, 25);
    dat_buff |= or_mask_gen(snl_ended, 26);
    dat_buff |= or_mask_gen(0b11100, 27);
    this -> chnl_asgn_map[channel - 1] = dat_buff;
    return 0;
}
/**
 * @desc: This will configure all the channels on the LTC2983 with the channel assignment data table.
 * @param: [*tx_buff] pointer to the tx buffer for transaction data
 * @depends: wiringPI
 **/
int ltc2983::write_all_channel_assignments(unsigned char *tx_buff)
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
            byte_buff[3-j] = (this -> chnl_asgn_map[i] >> j*8) & 0xFF;
        for (int j = 0; j < 4; j++)
        {

            for(int k = 0; k < 4; k++)
                byte_null[k] = byte_buff[k];
            address = (0x0200 + i*4 + j);
            #ifdef DEBUG_L1
            std::cout << "Contentes of byte[" << j << "] tansacton = " << std::bitset<8>(byte_null[j]) << '\n';
            #endif
            gen_transaction(&trans_buff, WRITE, address, byte_null[j]);
            for(int k = 0; k < 4; k++)
                byte_null[3-k] = (trans_buff >> (8*k)) & 0xff;
            wiringPiSPIDataRW (this -> spi_channel, &byte_null[0], 4);
        }
    }
    return 0;
}
/**
 * @desc: Initiates a long conversion on all channels based on the mask value
 * @depends: wiringPI
 **/
int ltc2983::all_chnnel_conversion(void)
{
    unsigned int temp = 0;
    unsigned char tx_buff [4];
    unsigned char dat_buff;
    unsigned int all_mask = 0x000FFFFF;
    unsigned short int address = 0x00F4;
    pinMode(19, INPUT);
    for(int i = 0; i < 4; i++)
    {
        dat_buff = ((all_mask >> (24-8*i)) & 0xff);
        gen_transaction(&temp, WRITE, (address + i), dat_buff);
        #ifdef DEBUG_L1
        std::cout << "Writing multi channel conversion mask byte " << i << ": " << std::bitset<8>(dat_buff) << '\n';
        #endif
        for(int j = 0; j < 4; j++)
        {
            tx_buff[3-j] = (temp >> (8*j)) & 0xff;
        }
        wiringPiSPIDataRW (this -> spi_channel, &tx_buff[0], 4);
    }
    gen_transaction(&temp, WRITE, 0x0000, 0x80);
    for(int i = 0; i < 4; i++)
        tx_buff[3-i] = (temp >> (8*i)) & 0xff;
    wiringPiSPIDataRW (this -> spi_channel, &tx_buff[0], 4);
    while(!digitalRead(19));
    #ifdef DEBUG
    std::cout << "conversion_complete\n";
    #endif
    return 0;
}
/**
 * @desc: Reads the channels data configuration from the IC and Conversion status byte. Channel index = 1
 * @param: [spi_channel] spi channel to retrive channel_config from
 * @note: Errors could and should be enumerated at some point but for now they will just be a series of values; LSB = 0 MSB = 7
 **/
 int ltc2983::channel_err_decode(int channel_number)
 {
   int status;
   unsigned char sensor_type;
   unsigned char conversion_status;
   unsigned short int conversion_result_address = (0x0010 + (4 * (channel_number - 1)));
   unsigned int error_bit_pos;
   std::string error_string[8] = {"VALID", "ADC OUT OF RANGE", "SENSOR UNDER RANGE", "SENSOR OVER RANGE", "CJ SOFT FAULT", "CJ HARD FAULT", "HARD ADC OUT OF RANGE", "SENSOR HARD FAULT"};
   // Readback the channel configuration
   status = read_data(conversion_result_address, &conversion_status, 1);
   switch (conversion_status)
   {
       case 0xFF:
       {
           #ifdef DEBUG
           std::cout << "Sensor conversion on channel " << channel_number << " status byte returned 0xFF: ALL ERROR BITS AND VLIAD FLAGGED\n";
           #endif
           return 1;
       }
       case 0x01:
       {
           #ifdef DEBUG
           std::cout << "Conversion on channel " << channel_number << " is valid.\n";
           #endif
           return 0;
       }
       case 0x00:
       {
           #ifdef DEBUG
           std::cout << "No conversion on channel " << channel_number << " occoured.\n";
           #endif
           return 3;
       }
       default:
       {
           #ifdef DEBUG
           std::cout << "Conversion on channel " << channel_number << " contained the following errors.\n";
           for(int i = 0; i < 7; i ++)
           {
               if((conversion_status >> i) & 0x01)
               {
                   std::cout << error_string[i] << '\n';
               }
           }
           #endif
           return 2;
       }
   }
   // If you get here you have problems.
   return 4;
}
/**
 * @desc: Reads the channel value out and will return a double with the results, prints errors channel index = 1
 * @param: [spi_channel] SPI channel used for transactions
 * @param: [channel_number] Channel number of conversion to be read from
 **/
 float ltc2983::read_channel_double(int channel_number)
 {
     int status;
     unsigned int result;
     float temperature;
     bool sign;
     unsigned char chnl_dat_buff[4];
     unsigned short int conversion_result_address = (0x0010 + (4 * (channel_number - 1)));
     // Read out the channel information from the SPI bus.
     status = read_data(conversion_result_address, &chnl_dat_buff[0], 4);
     // Now that the channel data buffer is filled check for errors
     if (channel_err_decode(channel_number)  != 0)
     {
         #ifdef DEBUG
         std::cout << "Result on channel " << channel_number << " is invalid.\n";
         #endif
         return 9000;
     }
     else
     {
         #ifdef DEBUG_L1
         for(int i = 0; i < 4; i++)
         {
             std::cout << "Raw read value of channel " << channel_number << " byte " << i << " = " << std::bitset<8>(chnl_dat_buff[i]) << '\n';
         }
         #endif
         result = 0;
         result = result | ((unsigned int) chnl_dat_buff[1]<<16)
                         | ((unsigned int) chnl_dat_buff[2]<<8)
                         | ((unsigned int) chnl_dat_buff[3]);
         #ifdef DEBUG_L1
         std::cout << "Raw bin of result = " << std::bitset<32>(result) << "\n";
         #endif
         // Convert a 24bit 2s compliment into a 32bit 2s compliment number
         if ((chnl_dat_buff[1]&0b10000000)==128)
             sign=true;
         else sign=false;
         // append 1s to the MSB 8 bits
         if (sign)
             result = result | 0xFF000000;
         // Compensate for precision
         temperature = float(result) / 1024;

         return temperature;
     }
     return 9001; //Never should see this return
 }
/**
 * @desc: Generates a mask that is meant to be bitwise OR'd with an unsigned int :VERI:
 * @param [value] the value needing to be shifted into a bit position for masking
 * @param [bit] bit position to shift value by. LSB = LSB
 **/
extern unsigned int or_mask_gen(unsigned int value, unsigned int bit_pos)
{
    return (value << bit_pos);
}
// Generates 32 bits for a SPI transaction to the LTC2983, This is written as a series of 4 byes to the buffer *CLEARS BUFFER CONTENTS* :VERI:
extern int gen_transaction(unsigned int *buff, unsigned char trans_type, unsigned short int address, unsigned char data)
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

/**
 * @desc: Takes in a string: Idealy from the socket, and if the string = "LTC2983 -v" it will print out the version
 * @param: [input] the input string to be tested
 **/
void ltc2983::get_ver(void)
{
#ifndef VER_STRING
    std::cout << "Compiled without LTC2983 version.";
#else
    std::cout << "LTC2983 module version " << VER_STRING;
#endif
}
