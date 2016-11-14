#ifndef LTC2983
#define LTC2983
// Class Wrapper for the LTC2983
class ltc2983
{
// Functions
public:
    ltc2983();
    virtual ~ltc2983();
    int init_ltc2983 (int spi_channel);
    int get_command_status(int spi_channel);
    int read_data (unsigned short int address, unsigned char *results, unsigned int bytes, unsigned int spi_channel);
    int setup_thermocouple(unsigned int *channel_asgn, unsigned char tc_type, unsigned char cj_assignment,  bool snl_ended, bool oc_chk, unsigned char oc_curr);
    int setup_diode(unsigned int *channel_asgn, bool snl_ended, bool three_readings, bool averaging, unsigned char exc_current, unsigned int ideality_f);
    int tx_buffer_stitch (unsigned char *tx_buffer, unsigned char *dat_buffer, int bytes);
    int gen_transaction(unsigned int *buff, unsigned char trans_type, unsigned short int address, unsigned char data);
    int write_all_channel_assignments(unsigned char *tx_buff, unsigned int *asgn_table, int spi_channel);
    int all_chnnel_conversion(int spi_channel);
    int read_channel_raw_value(int spi_channel, int channel_number, unsigned char *results);
    int ltc_2983_channel_err_decode(int spi_channel, int channel_number);
    float read_channel_double(int spi_channel, int channel_number);
    unsigned int or_mask_gen(unsigned int value, unsigned int bit_pos);
// Protected Varibles
protected:
    int status; //Contains the Status of the class object and can be checked for errors
    const unsigned int d_ideality_f = 0x00101042; // Contains a single diode ideailtiy factor. Use this if all diodes have the same ideality. USE WITH THERMOCOUPLE HAT
    unsigned int c_d_ideality_f [20]; // Contains an array ables to store multiple ideality factor valyes
    int spi_channel; // SPI CHANNEL BEING USED 
    unsigned int chnl_asgn_config[20]; // Stores the raw bits to be sent out for a channel assignment
};

/**
 * By: Reese Russell
 * Description: Blow this point are a list of defines neccesay to configure the ltc2983
 * These defines are incomplete as of 11/13/2016 but they are accurate and offer enough
 * functionality to enable the LTC2983 to be configured for the rPI themocouple hat.
 * FEATURES:
 * - Channel Assigments
 * - SPI commands
 * - Thermocouple assigments
 * - Diodes assigments
 * - Thermocuple Varibles
 * - Diode Varibles
 * Notes:
 * Please read the datasheet to understand how the configuration definitions
 * as paramaters are used to configure the ltc2983
 **/

// REGISTER DEFINITIONS
// WRITE AND READ COMMANDS
#define NOP         0x01
#define WRITE       0x02
#define READ        0x03

// BASE ADDRESS MAP
#define CNV_RSLTS   0x0010 // START: 0x010 -> END: 0x05F [Word]
#define CHNL_MAP    0x0200 // START: 0x200 -> END: 0x24F [Word]

// TC SE/DIFF VALS
#define SNGL        true
#define DIFF        false

// Sensor Channel Selection *INCOMPLETE*
// This is the sesnor type selection for the channel
// This list is incomplete and designed to acomidate thermocouples and diodes for CJ compensation
// LTC2983 contains many more sensortpyes, please add these in if you have time or have a design requirement
#define UNASSIGNED  0b00000
#define TYPE_J      0b00001
#define TYPE_K      0b00010
#define TYPE_E      0b00011
#define TYPE_N      0b00100
#define TYPE_R      0b00101
#define TYPE_S      0b00110
#define TYPE_T      0b01000
#define TYPE_CUST   0b01001
#define TYPE_DIODE  0b11100

// Channel Number to Array Bindings
#define CHANNEL_1   0
#define CHANNEL_2   1
#define CHANNEL_3   2
#define CHANNEL_4   3
#define CHANNEL_5   4
#define CHANNEL_6   5
#define CHANNEL_7   6
#define CHANNEL_8   7
#define CHANNEL_9   8
#define CHANNEL_10  9
#define CHANNEL_11  10
#define CHANNEL_12  11
#define CHANNEL_13  12
#define CHANNEL_14  13
#define CHANNEL_15  14
#define CHANNEL_16  15
#define CHANNEL_17  16
#define CHANNEL_18  17
#define CHANNEL_19  18
#define CHANNEL_20  19

// Input Channel Mapping *COMPLETE*
// This is the input channels represented in binary form
// Includes MULTI CHANNEL and SLEEP addesses
#define MULTI_CHNL  0b10000000
#define CHNL_1      0b10000001
#define CHNL_2      0b10000010
#define CHNL_3      0b10000011
#define CHNL_4      0b10000100
#define CHNL_5      0b10000101
#define CHNL_6      0b10000110
#define CHNL_7      0b10000111
#define CHNL_8      0b10001000
#define CHNL_9      0b10001001
#define CHNL_10     0b10001010
#define CHNL_11     0b10001011
#define CHNL_12     0b10001100
#define CHNL_13     0b10001101
#define CHNL_14     0b10001110
#define CHNL_15     0b10001111
#define CHNL_16     0b10010000
#define CHNL_17     0b10010001
#define CHNL_18     0b10010010
#define CHNL_19     0b10010011
#define CHNL_20     0b10010100
#define CHNL_SLP    0b10010111

/* Thermocouple Definitions */

// TC Cold Junction Mapping *COMPLETE*
// This is the cold junction channels represented in binary form
// Cold Junction Pointers
#define NO_CJ       0b00000
#define CJ_CHNL_1   0b00001
#define CJ_CHNL_2   0b00010
#define CJ_CHNL_3   0b00011
#define CJ_CHNL_4   0b00100
#define CJ_CHNL_5   0b00101
#define CJ_CHNL_6   0b00110
#define CJ_CHNL_7   0b00111
#define CJ_CHNL_8   0b01000
#define CJ_CHNL_9   0b01001
#define CJ_CHNL_10  0b01010
#define CJ_CHNL_11  0b01011
#define CJ_CHNL_12  0b01100
#define CJ_CHNL_13  0b01101
#define CJ_CHNL_14  0b01110
#define CJ_CHNL_15  0b01111
#define CJ_CHNL_16  0b10000
#define CJ_CHNL_17  0b10001
#define CJ_CHNL_18  0b10010
#define CJ_CHNL_19  0b10011
#define CJ_CHNL_20  0b10100

// TC Excitation Current ex. EXT, This is the excitation current provided by the LTC2983: Datasheet has more info
// This is measured in microamps of excitation current [UA APPENDED]
// EXT is the external excitation source will be used
#define TC_EXT_C    0b00
#define TC_10UA     0b00
#define TC_100UA    0b01
#define TC_500UA    0b10
#define TC_1000UA   0b11

// TC SE/DIFF VALS
#define SNGL        true
#define DIFF        false

// Over current protection
#define OC_CHK_ON   true
#define OC_CHK_OFF  false

/* DIODE CONFIGURATION */

// 2 or 3 readigs, Diode Readings at 2 or 3 different current levels
#define CONV_3      true
#define CONV_2      false

// Averaging mode enabled *Use this only if diode is stable
#define D_AVG_ON    true
#define D_AVG_OFF   false

// Excitation Current Settins
#define D_10UA      0b00
#define D_20UA      0b01
#define D_40UA      0b10
#define D_80UA      0b11

#endif
