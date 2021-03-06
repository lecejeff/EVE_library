#include <Arduino.h>
#include "FT8XX.h"
#include "io_expander.h"
#include "rtc.h"
#include <SPI.h>

#if MAX_KEYS_NB > 0
STKeys st_Keys[MAX_KEYS_NB];
#endif

#if MAX_GRADIENT_NB > 0
STGradient st_Gradient[MAX_GRADIENT_NB];
#endif

#if MAX_SCROLLBAR_NB > 0
STScrollbar st_Scrollbar[MAX_SCROLLBAR_NB];
#endif

#if MAX_DIAL_NB > 0
STDial st_Dial[MAX_DIAL_NB];
#endif

#if MAX_TEXT_NB > 0
STText st_Text[MAX_TEXT_NB];
#endif

#if MAX_GAUGE_NB > 0
STGauge st_Gauge[MAX_GAUGE_NB];
#endif

#if MAX_PROGRESS_NB > 0
STProgress st_Progress[MAX_PROGRESS_NB];
#endif

#if MAX_CLOCK_NB > 0
STClock st_Clock[MAX_CLOCK_NB];
#endif

#if MAX_TOGGLE_NB > 0
STToggle st_Toggle[MAX_TOGGLE_NB];
#endif

#if MAX_RECT_NB > 0
STRectangle st_Rectangle[MAX_RECT_NB];
#endif

#if MAX_BUTTON_NB > 0
STButton st_Button[MAX_BUTTON_NB];
#endif

#if MAX_NUMBER_NB > 0
STNumber st_Number[MAX_NUMBER_NB];
#endif

#if MAX_SLIDER_NB > 0
STSlider st_Slider[MAX_SLIDER_NB];
#endif

#if MAX_WINDOW_NB > 0
STWindow st_Window[MAX_WINDOW_NB];
#endif

#ifdef RIVERDI_800x480_CAPACITIVE_FT813
unsigned int ft8xx_lcd_hcycle = 928;
unsigned int ft8xx_lcd_hsize = 800;
unsigned int ft8xx_lcd_hoffset = 88;
unsigned int ft8xx_lcd_hsync0 = 40;
unsigned int ft8xx_lcd_hsync1 = 88;
unsigned int ft8xx_lcd_cspread = 1;

unsigned int ft8xx_lcd_vcycle = 525;
unsigned int ft8xx_lcd_vsize = 480;
unsigned int ft8xx_lcd_voffset = 32;
unsigned int ft8xx_lcd_vsync0 = 13;
unsigned int ft8xx_lcd_vsync1 = 16;
unsigned int ft8xx_lcd_swizzle = 0;
unsigned int ft8xx_lcd_pclk_pol = 0;
unsigned int ft8xx_lcd_pclk = 2;
#endif

#ifdef RIVERDI_480x272_RESISTIVE_FT800
unsigned int ft8xx_lcd_hcycle = 548;
unsigned int ft8xx_lcd_hsize = 480;
unsigned int ft8xx_lcd_hoffset = 43;
unsigned int ft8xx_lcd_hsync0 = 0;
unsigned int ft8xx_lcd_hsync1 = 41;
unsigned int ft8xx_lcd_cspread = 1;

unsigned int ft8xx_lcd_vcycle = 292;
unsigned int ft8xx_lcd_vsize = 272;
unsigned int ft8xx_lcd_voffset = 12;
unsigned int ft8xx_lcd_vsync0 = 0;
unsigned int ft8xx_lcd_vsync1 = 10;
unsigned int ft8xx_lcd_swizzle = 0;
unsigned int ft8xx_lcd_pclk_pol = 1;
unsigned int ft8xx_lcd_pclk = 5;
#endif

extern RTC_class rtc;
STTouch touch_data;
extern IO_expander expander_i2c;

// Do not touch these variables
unsigned int cmdBufferRd;            // Used to navigate command ring buffer
unsigned int cmdBufferWr = 0x0000;   // Used to navigate command ring buffer
unsigned int cmdOffset = 0x0000;    // Used to navigate command rung buffer

//***************************unsigned char FT_init (void))*******************************//
//Description : Function initializes FT8XXX display to given parameters
//
//Function prototype : unsigned char FT_init (void)
//
//Enter params       : None
//
//Exit params        : 0 : init was a success
//                     1 : init failed due to bad processor clk
//
//Function call      : unsigned char = FT_init();
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::init (void)
{
    unsigned char duty = 0, gpio = 0, reg_id_value = 0;
    tag_num = 0;
    // Initialize primitive counter
    rectangle_nb = 0;
    dial_nb = 0;
    gauge_nb = 0;
    progress_nb = 0;
    scrollbar_nb = 0;
    clock_nb = 0;
    toggle_nb = 0;
    button_nb = 0;
    number_nb = 0;
    text_nb = 0;
    slider_nb = 0;
    keys_nb = 0;
    gradient_nb = 0;    

    // Do not remove
    pinMode(FT8XX_nCS_PIN, OUTPUT);
    pinMode(FT8XX_nCS_PIN, HIGH);

    pinMode(FT8XX_nINT_PIN, INPUT);

    // Replace delay calls with the one for arduino
    delay(100);

    expander_i2c.write_bit(EXPANDER_nFT_PD_BIT, 1);     // Set #PD pin HIGH
    delay(50);
    expander_i2c.write_bit(EXPANDER_nFT_PD_BIT, 0);     // Set #PD pin LOW
    delay(50);
    expander_i2c.write_bit(EXPANDER_nFT_PD_BIT, 1);     // Set #PD pin HIGH   
    delay(50);  

    host_command(FT8XX_ACTIVE);             //FT8XX wake_up command
    delay(50);
    host_command(FT8XX_ACTIVE);             //FT8XX wake_up command
    delay(50);
    host_command(FT8XX_ACTIVE);             //FT8XX wake_up command
    delay(50);  

    #ifdef FT_80X_ENABLE
    host_command(FT8XX_CLKEXT);             //Set clock to external oscillator
    #endif

    #ifdef FT_81X_ENABLE
    host_command(FT8XX_CLKINT);             //Set clock to internal oscillator
    #endif
    delay(50);
    host_command(FT8XX_CLK48M);             //FT8XX clock set to 48MHz
    delay(50);
    host_command(FT8XX_CORERST);            //reset FT8XX core CPU
    delay(50);
    host_command(FT8XX_GPUACTIVE);          //activate GPU
    delay(100);
    reg_id_value = rd8(REG_ID);        //FT_read_8bit(0x102400);
    while (reg_id_value != 0x7C)                //Check if clock switch was performed
    {
        yield();
        reg_id_value = rd8(REG_ID);
        delay(10);
    }
    //Clock switch was a success, initialize FT8XX display parameters
    wr8(REG_PCLK, 0);                 // no PCLK on init, wait for init done
    wr8(REG_PWM_DUTY, 0);             // no backlight until init done

    wr16(REG_HCYCLE,  ft8xx_lcd_hcycle);    // total number of clocks per line, incl front/back porch
    wr16(REG_HSIZE,   ft8xx_lcd_hsize);     // active display width
    wr16(REG_HOFFSET, ft8xx_lcd_hoffset);   // start of active line
    wr16(REG_HSYNC0,  ft8xx_lcd_hsync0);    // start of horizontal sync pulse
    wr16(REG_HSYNC1,  ft8xx_lcd_hsync1);    // end of horizontal sync pulse
    wr16(REG_CSPREAD, ft8xx_lcd_cspread);

    wr16(REG_VCYCLE,  ft8xx_lcd_vcycle);    // total number of lines per screen, incl pre/post
    wr16(REG_VSIZE,   ft8xx_lcd_vsize);     // active display height       
    wr16(REG_VOFFSET, ft8xx_lcd_voffset);   // start of active screen
    wr16(REG_VSYNC0,  ft8xx_lcd_vsync0);    // start of vertical sync pulse
    wr16(REG_VSYNC1,  ft8xx_lcd_vsync1);    // end of vertical sync pulse
    wr16(REG_SWIZZLE,  ft8xx_lcd_swizzle);   // FT8XX output to LCD - pin order
    wr16(REG_PCLK_POL, ft8xx_lcd_pclk_pol);  // LCD data is clocked in on this PCLK edge

    wr8(REG_VOL_PB, ZERO);            // turn recorded audio volume down
    wr8(REG_VOL_SOUND, ZERO);         // turn synthesizer volume down
    wr16(REG_SOUND, 0x6000);          // set synthesizer to mute

    //***************************************
    // Write Initial Display List & Enable Display (clear screen, set ptr to 0)
    start_new_dl();
    clear_screen(BLACK);
    update_screen_dl();
    gpio = rd8(REG_GPIO);  // Read the FT800 GPIO register for a read/modify/write operation
    gpio = gpio | 0x80;   // set bit 7 of FT800 GPIO register (DISP) - others are inputs
    wr8(REG_GPIO, gpio);  // Enable the DISP signal to the LCD panel
    wr8(REG_PCLK, ft8xx_lcd_pclk);     // Now start clocking data to the LCD panel
    for (duty = 0; duty < 127; duty++) //127 is full
    {
        yield();
        wr8(REG_PWM_DUTY, duty); // Turn on backlight - ramp up slowly to full brighness
        delay(1);
    }
    // If you want to calibrate the touchpanel, uncomment the following lines
    #ifdef FT_80X_ENABLE
    wr8(REG_TOUCH_MODE, FT8XX_TOUCH_MODE_CONTINUOUS);    //Touch enabled
    #endif

    #ifdef FT_81X_ENABLE
        wr8(REG_CTOUCH_MODE, FT8XX_TOUCH_MODE_CONTINUOUS);      // Touch enabled
        wr8(REG_CTOUCH_EXTENDED, 1);                            // Compatibility mode
    #endif
    touchpanel_init();
}

//**********************void FT_touchpanel_init (void)************************//
//Description : Function initializes FT801 touch panel, calibration too
//
//Function prototype : void FT_touchpanel_init (void)
//
//Enter params       : None
//
//Exit params        : None
//
//Function call      : FT_touchpanel_init();
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::touchpanel_init (void)
{
    start_new_dl();                    //Start new dlist
    write_dl_long(CLEAR(1, 1, 1));
    write_dl_long(CMD_CALIBRATE);           //Run self-calibration routine
    update_screen_dl();                //update dlist
    //This while loop will end the touchpanel initialisation
    do
    {
        yield();
        cmdBufferRd = rd16(REG_CMD_READ);
        cmdBufferWr = rd16(REG_CMD_WRITE);
    }   while ((cmdBufferWr != 0) && (cmdBufferWr != cmdBufferRd));

    #ifdef TOUCH_PANEL_CAPACITIVE
        wr8(REG_CTOUCH_EXTENDED, 1);   //Compatibility touch mode
    #endif
}

void FT8XX_EVE::clear_touch_tag (void)
{
    touch_tag = 0;
}

unsigned char FT8XX_EVE::read_touch_tag (void)
{
    static unsigned char touch_counter = 1;
    static unsigned char tag_flag = 0;
    // Check if the screen was touched
    if (((rd32(REG_TOUCH_DIRECT_XY)) & 0x8000) == 0x0000)
    {
        if (++touch_counter > 5)
        {
            touch_counter = 5;     
            if (tag_flag == 0)
            {
                tag_flag = 1;   
                touch_tag = rd32(REG_TOUCH_TAG);                            
            }
        }
    }
    else
    {
        if (--touch_counter < 1)
        {
            touch_counter = 1;
            tag_flag = 0; 
        }                     
    }

    return touch_tag;
}

unsigned char FT8XX_EVE::get_touch_tag (void)
{
    return touch_tag;
}

void FT8XX_EVE::set_touch_tag (unsigned char prim_type, unsigned char prim_num, unsigned char tag_num)
{  
    switch (prim_type)
    {
        #if MAX_TOGGLE_NB > 0
        case FT_PRIM_TOGGLE:
            st_Toggle[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_BUTTON_NB > 0
        case FT_PRIM_BUTTON:
            st_Button[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_TEXT_NB > 0
        case FT_PRIM_TEXT:
            st_Text[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_GRADIENT_NB > 0
        case FT_PRIM_GRADIENT:
            st_Gradient[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_KEYS_NB > 0
        case FT_PRIM_KEYS:
            st_Keys[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_PROGRESS_NB > 0
        case FT_PRIM_PROGRESS:
            st_Progress[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_SCROLLBAR_NB > 0
        case FT_PRIM_SCROLLBAR:
            st_Scrollbar[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_GAUGE_NB > 0
        case FT_PRIM_GAUGE:
            st_Gauge[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_CLOCK_NB > 0
        case FT_PRIM_CLOCK:
            st_Clock[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_DIAL_NB > 0
        case FT_PRIM_DIAL:
            st_Dial[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_NUMBER_NB > 0
        case FT_PRIM_NUMBER:
            st_Number[prim_num].touch_tag = tag_num;
        break;
        #endif

        #if MAX_SLIDER_NB > 0
        case FT_PRIM_SLIDER:
            st_Slider[prim_num].touch_tag = tag_num;
        break;
        #endif
    }
}

//**********************void FT_write_command (unsigned char command)********************//
//Description : Function write command to FT801
//
//Function prototype : void FT_write_command (unsigned char command)
//
//Enter params       : unsigned char command : command to send to FT801
//
//Exit params        : None
//
//Function call      : FT_write_command (FT800_CLK48M);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::host_command (unsigned char command)
{
    SPI.begin();                        // Command frame :
    digitalWrite(FT8XX_nCS_PIN, LOW);   // Set #CS low
    SPI.transfer(command);              // Byte 0 = command, see FT8XX_command_list
    SPI.transfer(0);                    // Byte 1 = dummy, send 0
    SPI.transfer(0);                    // Byte 2 = dummy, send 0
    SPI.end();                          
    digitalWrite(FT8XX_nCS_PIN, HIGH);  // Set #CS high
}

//**********************void FT_write_8bit (unsigned long adr, unsigned char data)******************//
//Description : Function writes 8 bit data to specified adr
//
//Function prototype : void FT_write_8bit (unsigned long adr, unsigned char data)
//
//Enter params       : unsigned long adr : FT801 register address
//                     unsigned char data: Data to write to register
//
//Exit params        : None
//
//Function call      : FT_write_8bit (CMD_BUTTON, OPT_NONE);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::wr8 (unsigned long adr, unsigned char data)
{
    // byte 0 = (unsigned char)((adr >> 16) | MEM_WRITE);   // Write 24 bit ADR
    // byte 1 = (unsigned char)(adr>>8);                    // 
    // byte 2 = adr                                         //
    // byte 3 = data                                        // Write 8 bit data
                                                            // Little endian
    SPI.begin();
    digitalWrite(FT8XX_nCS_PIN, LOW);
    SPI.transfer((unsigned char)((adr >> 16) | MEM_WRITE));
    SPI.transfer((unsigned char)(adr >> 8));
    SPI.transfer(adr);
    SPI.transfer(data);
    SPI.end();
    digitalWrite(FT8XX_nCS_PIN, HIGH);
}

//*******************void FT_write_16bit (unsigned long adr, unsigned int data)********************//
//Description : Function writes 16 bit data + adr to FT8XX
//
//Function prototype : void FT_write_16bit (unsigned long adr, unsigned int data)
//
//Enter params       : unsigned long adr : register adress
//                     unsigned int data: data to write
//
//Exit params        : None
//
//Function call      : FT_write_16bit(REG, 0x0A1E);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::wr16 (unsigned long adr, unsigned int data)
{
    // byte 0 = (unsigned char)((adr >> 16) | MEM_WRITE);     // Write 24 bit ADR
    // byte 1 = (unsigned char)(adr>>8);                      // 
    // byte 2 = adr;                                          //
    // byte 3 = (unsigned char)(data);                        // Write 16 bit data
    // byte 4 = (unsigned char)(data >> 8);                   // Little endian

    SPI.begin();
    digitalWrite(FT8XX_nCS_PIN, LOW);
    SPI.transfer((unsigned char)((adr >> 16) | MEM_WRITE));
    SPI.transfer((unsigned char)(adr >> 8));
    SPI.transfer(adr);
    SPI.transfer(data);
    SPI.transfer((unsigned char)(data >> 8));
    SPI.end();
    digitalWrite(FT8XX_nCS_PIN, HIGH);
}

//*******************void FT_write_32bit (unsigned long adr, unsigned long data)********************//
//Description : Function writes 32 bit data + adr to FT8XX
//
//Function prototype : void FT_write_32bit (unsigned long adr, unsigned long data)
//
//Enter params       : unsigned long adr : register adress
//                     unsigned long data: data to write
//
//Exit params        : None
//
//Function call      : FT_write_32bit(REG, 0x0A1E0A12);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::wr32 (unsigned long adr, unsigned long data)
{
    // byte 0 = (unsigned char)((adr >> 16) | MEM_WRITE);     // Write 24 bit ADR
    // byte 1 = (unsigned char)(adr>>8);                      //
    // byte 2 = adr;                                          //
    // byte 3 = (unsigned char)(data);                        // Writing 32 bit data
    // byte 4 = (unsigned char)(data >> 8);                   // Little endian
    // byte 5 = (unsigned char)(data >> 16);                  //
    // byte 6 = (unsigned char)(data >> 24);                  //

    SPI.begin();
    digitalWrite(FT8XX_nCS_PIN, LOW);
    SPI.transfer((unsigned char)((adr >> 16) | MEM_WRITE));
    SPI.transfer((unsigned char)(adr >> 8));
    SPI.transfer(adr);
    SPI.transfer(data);
    SPI.transfer((unsigned char)(data >> 8));
    SPI.transfer((unsigned char)(data >> 16));
    SPI.transfer((unsigned char)(data >> 24));
    SPI.end();
    digitalWrite(FT8XX_nCS_PIN, HIGH);
}

//*************************unsigned char FT_read_8bit (unsigned long adr)***************************//
//Description : Function reads 8 bit data from FT8XX
//
//Function prototype : unsigned char FT_read_8bit (unsigned long adr)
//
//Enter params       : unsigned long adr : register adress
//
//Exit params        : unsigned char : data from register
//
//Function call      : unsigned char = FT_read_8bit(0x0A1E0A12);
//
//Intellitrol  08/07/2016
//******************************************************************************
unsigned char FT8XX_EVE::rd8 (unsigned long adr)
{
    // byte 0 = (unsigned char)((adr >> 16) | MEM_WRITE);   // Write 24 bit ADR
    // byte 1 = (unsigned char)(adr>>8);                    // 
    // byte 2 = adr                                         //
    // byte 3 = dummy                                       // send 0
    // byte 4 = rd8                                         // send 0, read 8-bit value from FT8XX
    unsigned char rd8 = 0;
    SPI.begin();
    digitalWrite(FT8XX_nCS_PIN, LOW);
    SPI.transfer((unsigned char)((adr >> 16) | MEM_READ));
    SPI.transfer((unsigned char)(adr >> 8));
    SPI.transfer(adr);
    SPI.transfer(0);
    rd8 = SPI.transfer(0);
    SPI.end();
    digitalWrite(FT8XX_nCS_PIN, HIGH);

    return rd8;
}

//*************************unsigned int FT_read_16bit (unsigned long adr)**************************//
//Description : Function reads 16 bit data from FT8XX
//
//Function prototype : unsigned char FT_read_16bit (unsigned long adr)
//
//Enter params       : unsigned long adr : register adress
//
//Exit params        : unsigned int : data from register
//
//Function call      : unsigned int = FT_read_16bit(0x0A1E0A12);
//
//Intellitrol  08/07/2016
//******************************************************************************
unsigned int FT8XX_EVE::rd16 (unsigned long adr)
{
    // byte 0 = (unsigned char)((adr >> 16) | MEM_WRITE);   // Write 24 bit ADR
    // byte 1 = (unsigned char)(adr>>8);                    // 
    // byte 2 = adr                                         //
    // byte 3 = dummy                                       // send 0
    // byte 4 = rd16, LSbyte                                // send 0, read less significant byte of the 16-bit data
    // byte 5 = rd16, MSbyte                                // send 0, read most significant byte of the 16-bit data

    unsigned char data_read1, data_read2;
    unsigned int rd16 = 0x0000;
    SPI.begin();
    digitalWrite(FT8XX_nCS_PIN, LOW);
    SPI.transfer((unsigned char)((adr >> 16) | MEM_READ));
    SPI.transfer((unsigned char)(adr >> 8));
    SPI.transfer(adr);
    SPI.transfer(0);
    data_read1 = SPI.transfer(0);
    data_read2 = SPI.transfer(0);
    SPI.end();
    digitalWrite(FT8XX_nCS_PIN, HIGH);

    rd16 = ((data_read2 << 8) | data_read1);
    return (rd16);
}

//*************************unsigned long FT_read_32bit (unsigned long adr)**************************//
//Description : Function reads 32 bit data from FT8XX
//
//Function prototype : unsigned char FT_read_32bit (unsigned long adr)
//
//Enter params       : unsigned long adr : register adress
//
//Exit params        : unsigned long : data from register
//
//Function call      : unsigned long = FT_read_16bit(0x0A1E0A12);
//
//Intellitrol  08/07/2016
//******************************************************************************
unsigned long FT8XX_EVE::rd32 (unsigned long adr)
{
    // byte 0 = (unsigned char)((adr >> 16) | MEM_WRITE);   // Write 24 bit ADR
    // byte 1 = (unsigned char)(adr>>8);                    // 
    // byte 2 = adr                                         //
    // byte 3 = dummy                                       // send 0
    // byte 4 = rd16, 0..7                                  // send 0, read bits 0..7 of the 32-bit data
    // byte 5 = rd16, 8..15                                 // send 0, read bits 8..15 of the 32-bit data
    // byte 6 = rd32, 16..23                                // send 0, read bits 16..23 of the 32-bit data
    // byte 7 = rd32, 24..31                                // send 0, read bits 24..31 of the 32-bit data
    unsigned long data_read1, data_read2, data_read3, data_read4;
    unsigned long rd32 = 0x00000000;

    SPI.begin();
    digitalWrite(FT8XX_nCS_PIN, LOW);
    SPI.transfer((unsigned char)((adr >> 16) | MEM_READ));
    SPI.transfer((unsigned char)(adr >> 8));
    SPI.transfer(adr);
    SPI.transfer(0);
    data_read1 = SPI.transfer(0);
    data_read2 = SPI.transfer(0);
    data_read3 = SPI.transfer(0);
    data_read4 = SPI.transfer(0);
    SPI.end();
    digitalWrite(FT8XX_nCS_PIN, HIGH);

    rd32 = (unsigned long)(data_read4 << 24);
    rd32 = (unsigned long)(rd32 | data_read3 << 16);
    rd32 = (unsigned long)(rd32 | data_read2 << 8);
    rd32 = (unsigned long)(rd32 | data_read1);
    return (rd32);
}

//*************************void FT_start_new_dl (void)************************//
//Description : Function starts a new display list
//
//Function prototype : void FT_start_new_dl (void)
//
//Enter params       : none
//
//Exit params        : none
//
//Function call      : FT_start_new_dl();
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::start_new_dl (void)
{
    // Read processor read/write pointers, make them equals to start new DL
    cmdOffset = 0;
    do
    {
        cmdBufferRd = rd16(REG_CMD_READ);
        cmdBufferWr = rd16(REG_CMD_WRITE);
    }
    while ((cmdBufferWr != 0) && (cmdBufferWr != cmdBufferRd));
    // Ready to print a new display list
    cmdOffset = cmdBufferWr;  // Offset set to begin of display buffer
    write_dl_long(CMD_DLSTART); // Start of new display list
    write_dl_long(CLEAR(1, 1, 1));
}

//*********************void FT_update_screen_dl (void)************************//
//Description : Function updates the actual displa with new display list
//
//Function prototype : void FT_update_screen_dl (void)
//
//Enter params       : none
//
//Exit params        : none
//
//Function call      : FT_update_screen_dl();
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::update_screen_dl (void)
{
    write_dl_long(FT8XX_DISPLAY());           // Request display swap
    write_dl_long(CMD_SWAP);            // swap internal display list
    wr16(REG_CMD_WRITE, cmdOffset);     // Write list to display, now active
}

//************************void write_dl_char (unsigned char byte)************************//
//Description : Function writes char to display list
//
//Function prototype : void write_dl_char (unsigned char byte)
//
//Enter params       : unsinged char : data to write to display list
//
//Exit params        : none
//
//Function call      : write_dl_char(0xAE);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::write_dl_char (unsigned char data)
{
    wr16(RAM_CMD + cmdOffset, data);          // Write data to display list
    cmdOffset = inc_cmd_offset(cmdOffset, 1); // get new cmdOffset value
}

//************************void write_dl_int (unsigned int d1)************************//
//Description : Function writes int to display list
//
//Function prototype : void write_dl_int (unsigned int d1)
//
//Enter params       : unsinged int : data to write to display list
//
//Exit params        : none
//
//Function call      : FT_write_dlint(0xAE0C);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::write_dl_int (unsigned int data)
{
    wr16(RAM_CMD + cmdOffset, data);            // write data to display list
    cmdOffset = inc_cmd_offset(cmdOffset, 2);   // get new cmdOffset value
}

//**************************void write_dl_long (unsigned long cmd)*************************//
//Description : Function writes long to display list
//
//Function prototype : void write_dl_long (unsigned long cmd)
//
//Enter params       : unsinged long : data to write to display list
//
//Exit params        : none
//
//Function call      : FT_write_dlint(0xAE0CDDE3);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::write_dl_long (unsigned long data)
{
    wr32(RAM_CMD + cmdOffset, data);          // write data to display list
    cmdOffset = inc_cmd_offset(cmdOffset, 4); // get new cmdOffset value
}

//****************unsigned int FT_inc_cmd_offset (unsigned int cur_off, unsigned char cmd_size)**************//
//Description : Function increments write ring buffer inside FT801, and returns
//              the new offset value to stay between a range of 0 through 4096
//
//Function prototype : unsigned int FT_inc_cmd_offset (unsigned int cur_off, unsigned char cmd_size)
//
//Enter params       : unsigned int cur_off : current pointer offset
//                     unsigned char cmd_size: cmd size to add to pointer offset
//
//Exit params        : unsigned int : new offset value
//
//Function call      : unsigned int = FT_inc_cmd_offset(CURR_OFF, 4);
//
//Intellitrol  08/07/2016
//******************************************************************************
unsigned int FT8XX_EVE::inc_cmd_offset (unsigned int cur_off, unsigned char cmd_size)
{
    unsigned int new_offset;
    new_offset = cur_off + cmd_size;     // increm new offset
    if (new_offset > 4095)               // Ring buffer size not exceeded ?
    {
        new_offset = (new_offset - 4096);
    }
    return new_offset;                   //return new offset value
}


//**********************unsigned int FT_get_cmd_offset_value (void)*********************//
//Description : Function gets actual cmdOffset value
//
//Function prototype : unsigned int FT_get_cmd_offset_value (void)
//
//Enter params       : none
//
//Exit params        : unsigned int : cmdOffset value
//
//Function call      :unsigned int = FT_get_cmd_offset_value();
//
//Intellitrol  08/07/2016
//******************************************************************************
unsigned int FT8XX_EVE::get_cmd_offset_value (void)
{
    return (cmdOffset); // return cmdOffset value
}

//*************************void FT_set_bcolor (unsigned long color)**********************//
//Description : Function set's display background color
//
//Function prototype : void FT_set_bcolor (unsigned long color)
//
//Enter params       : unsigned long : color wanted, as R/G/B value
//
//Exit params        : none
//
//Function call      : FT_set_bcolor (RED);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::set_context_bcolor (unsigned long color)
{
    write_dl_long(CMD_BGCOLOR); // Write Bcolor command to display list
    write_dl_long(color);       // Write color
}

//*************************void FT_set_fcolor (unsigned long color)**********************//
//Description : Function set's display foreground color
//
//Function prototype : void FT_set_fcolor (unsigned long color)
//
//Enter params       : unsigned long : color wanted, as R/G/B value
//
//Exit params        : none
//
//Function call      : FT_set_fcolor (RED);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::set_context_fcolor (unsigned long color)
{
    write_dl_long(CMD_FGCOLOR); // Write Bcolor command to display lis
    write_dl_long(color);       // Write color
}

//**************************void FT_set_color (unsigned long color)**********************//
//Description : FT directive to set new color to primitives
//
//Function prototype : void FT_set_color (unsigned long color)
//
//Enter params       : unsigned long : color wanted, as R/G/B value
//
//Exit params        : none
//
//Function call      : FT_set_color (RED);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::set_context_color (unsigned long color)
{
    unsigned char R, G, B;
    B = (unsigned char)color;
    G = (unsigned char)(color >> 8);
    R = (unsigned char)(color >> 16);
    write_dl_long(COLOR_RGB(R, G, B));
}

void FT8XX_EVE::write_bitmap (const unsigned char *img_ptr, const unsigned char *lut_ptr, unsigned long img_length, unsigned long base_adr)
{
    unsigned long counter = 0;
    unsigned int lut_counter = 0;

    #ifdef FT_80X_ENABLE
    while (lut_counter < FT_RAM_PAL_SIZE)
    {
        wr8(RAM_PAL + lut_counter, *lut_ptr++);
        lut_counter++;
    }

    while (img_length> 0)
    {
        wr8((RAM_G + counter++ + base_adr), *img_ptr++);
        img_length--;
    }
    #endif
}   

void FT8XX_EVE::draw_point (unsigned int x, unsigned int y, unsigned int r)
{
    write_dl_long(BEGIN(FTPOINTS));             //Begin primitive
    write_dl_long(POINT_SIZE(r * 16));          //write line width
    write_dl_long(VERTEX2F(x * 16, y * 16));    //draw line
}
//**void init_slider (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int v, unsigned int r)**//
//Description : Function will init a st_Slider[number] object with input values
//
//Function prototype : void init_slider (unsigned char number, unsigned int x, y, w, h, opt, v, r))
//
//Enter params       : unsigned char : number : number of stslider object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          w      : width of object
//                          h      : height of object
//                          opt    : object options (refer to FT datasheet for valid options)
//                          v      : value
//                          r      : range
//
//Exit params        : none
//
//Function call      : init_slider (0, 50, 50, 20, 100, OPT_NONE, 50, 100);
//                     //Init slider 0 at position 50,50 with a w/h of 20/100,
//                     with no special options, value of 50 over a range of 100
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_SLIDER_NB > 0
void FT8XX_EVE::CMD_slider (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int v, unsigned int r)
{    
    st_Slider[number].x = x;    //initialise variable with input values
    st_Slider[number].y = y;
    st_Slider[number].w = w;
    st_Slider[number].h = h;
    st_Slider[number].opt = opt;
    st_Slider[number].val = v;
    st_Slider[number].range = r;
    slider_nb++;
}

//*****************void FT_draw_slider (STSlider *st_Slider)*******************//
//Description : FT function to draw a touch slider, specified through STSlider
//
//Function prototype : void FT_draw_slider (STSlider *st_Slider)
//
//Enter params       : *st_Slider : slider struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_slider (st_Slider[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_slider (STSlider *st_Slider)
{
    write_dl_long(CMD_SLIDER);
    write_dl_int(st_Slider->x);     // x
    write_dl_int(st_Slider->y);     // y
    write_dl_int(st_Slider->w);  // width
    write_dl_int(st_Slider->h); // height
    write_dl_int(st_Slider->opt);    // option
    write_dl_int(st_Slider->val);  // 16 bit value
    write_dl_long(st_Slider->range); // 32 bit range (stay in 4 bytes multiples)
}

void FT8XX_EVE::modify_slider (STSlider *st_Slider, unsigned char type, unsigned int value)
{
    switch (type)
    {
        case SLIDER_X:
            st_Slider->x = value;
        break;

        case SLIDER_Y:
            st_Slider->y = value;
        break;

        case SLIDER_W:
            st_Slider->w = value;
        break;

        case SLIDER_H:
            st_Slider->h = value;
        break;

        case SLIDER_OPT:
            st_Slider->opt = value;
        break;

        case SLIDER_VAL:
            if (value > st_Slider->range)
            {
                st_Slider->val = st_Slider->range;
            }
            else
            st_Slider->val = value;
        break;

        case SLIDER_RANGE:
            st_Slider->range = value;
        break;

        default:
        break;
    }
}

unsigned char FT8XX_EVE::get_slider_nb (void)
{
    return slider_nb;
}
#endif //#if MAX_SLIDER_NB > 0

//void init_button (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int f, unsigned int o, const char *str)//
//Description : Function will init a st_Button[number] object with input values
//
//Function prototype : void init_button (unsigned char number, unsigned int x, y, w, h, o, const char *str))
//
//Enter params       : unsigned char : number : number of st_Button object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          w      : width of object
//                          h      : height of object
//                          o      : object options (refer to FT datasheet for valid options)
//                          str    : string inside the box ("Hello world")
//
//Exit params        : none
//
//Function call      : init_button (0, 50, 50, 20, 100, OPT_NONE, "Hello world");
//                     //Init button 0 at position 50,50 with a w/h of 20/100,
//                     with no special options and Hello world text
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_BUTTON_NB > 0
void FT8XX_EVE::CMD_button (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int f, unsigned int o, const char *str)
{
    unsigned char cnt = 0, temp = 0; //initialise variable with input values
    st_Button[number].x = x;
    st_Button[number].y = y;
    st_Button[number].w = w;
    st_Button[number].h = h;
    st_Button[number].font = f;
    st_Button[number].opt = o;
    st_Button[number].state = 0;    //State is, by default, 0
    //This while loop fills st_Button struct with input string
    while (*str != 0x00)
    {
        st_Button[number].str[cnt] = *str;
        str++;
        cnt++;
    }
    
    //this part of the code ensures that transactions are 4 bytes wide. if
    //string byte # is not a multiple of 4, data is appended with null bytes
    st_Button[number].str[cnt] = 0x00;
    cnt++;
    temp = cnt & 0x0F;
    if ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
    {
        while ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
        {
        st_Button[number].str[cnt] = 0x00;
        cnt++;
        temp = cnt & 0x0F;
        }
    }
    st_Button[number].len = cnt; //length is written to struct
    button_nb++;
}

//*****************void FT_draw_button (STButton *st_Button)*******************//
//Description : FT function to draw a button, specified through STButton struct
//
//Function prototype : void FT_draw_button (STButton *st_Button)
//
//Enter params       : *st_Button : button struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_button (st_Button[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_button (STButton *st_Button)
{
    unsigned char cnt = 0;
    write_dl_long(CMD_BUTTON);
    write_dl_int(st_Button->x);    // x position on screen
    write_dl_int(st_Button->y);    // y position on screen
    write_dl_int(st_Button->w);    // width
    write_dl_int(st_Button->h);    // height
    write_dl_int(st_Button->font); // primitive font
    write_dl_int(st_Button->opt);  // primitive options
    while (cnt < st_Button->len)        // write button text until eos
    {
        write_dl_char(st_Button->str[cnt]);
        cnt++;
    }
}

void FT8XX_EVE::modify_button (STButton *st_Button, unsigned char type, unsigned int value)
{
    switch (type)
    {
        case BUTTON_X:
            st_Button->x = value;
        break;

        case BUTTON_Y:  
            st_Button->y = value;
        break;

        case BUTTON_W:
            st_Button->w = value;
        break;

        case BUTTON_H:
            st_Button->h = value;
        break;

        case BUTTON_FONT:
            st_Button->font = value;
        break;

        case BUTTON_OPT:
            st_Button->opt = value;
        break;

        default:
        break; 
    }
}

unsigned char FT8XX_EVE::get_button_nb (void)
{
    return button_nb;
}
#endif //#if MAX_BUTTON_NB > 0


//*****void init_text (unsigned char number, unsigned int x, unsigned int y, unsigned int f, unsigned int o, const char *str)******//
//Description : Function will init a st_Text[number] object with input values
//
//Function prototype : void init_text (unsigned char number, unsigned int x, y, f, o, const char *str))
//
//Enter params       : unsigned char : number : number of st_Text object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          f      : font of object
//                          o      : object options (refer to FT datasheet for valid options)
//                          str    : string value("Hello world")
//
//Exit params        : none
//
//Function call      : init_text (0, 50, 50, 23, OPT_NONE, "Hello world");
//                     //Init text 0 at position 50,50 with font 23, no option
//                     and Hello world text
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_TEXT_NB > 0
void FT8XX_EVE::CMD_text (unsigned char number, unsigned int x, unsigned int y, unsigned int f, unsigned int o, const char *str)
{
    unsigned char cnt = 0, temp = 0; //initialise variable with input values
    st_Text[number].x = x;
    st_Text[number].y = y;
    st_Text[number].font = f;
    st_Text[number].opt = o;
    //This while loop fills st_Button struct with input string
    while (*str != 0x00)
    {
        st_Text[number].str[cnt] = *str;
        str++;
        cnt++;
    }

    //this part of the code ensures that transactions are 4 bytes wide. if
    //string byte # is not a multiple of 4, data is appended with null bytes
    st_Text[number].str[cnt] = 0x00;
    cnt++;
    temp = cnt & 0x0F;
    if ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
    {
        while ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
        {
        st_Text[number].str[cnt] = 0x00;
        cnt++;
        temp = cnt & 0x0F;
        }
    }
    st_Text[number].len = cnt; //length is written to struct
    text_nb++;
}

//*******************void FT_draw_text (STText *st_Text)***********************//
//Description : Function draws text, specified by STText properties
//
//Function prototype : void FT_draw_text (STText *st_Text)
//
//Enter params       : *st_Text : text struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_text (st_Text[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_text (STText *st_Text)
{
    unsigned char cnt = 0;
    write_dl_long(CMD_TEXT);        // FT text command
    write_dl_int(st_Text->x);       // x position on screen
    write_dl_int(st_Text->y);       // y position on screen
    write_dl_int(st_Text->font);    // font parameter
    write_dl_int(st_Text->opt);     // FT text primitives options
    while (cnt < st_Text->len)      // write text until eos
    {
        write_dl_char(st_Text->str[cnt]);
        cnt++;
    }
}

unsigned char FT8XX_EVE::get_text_nb (void)
{
    return text_nb;
}
#endif //#if MAX_TEXT_NB > 0

#if MAX_GRADIENT_NB > 0
void FT8XX_EVE::CMD_gradient(unsigned char number, unsigned int x0, unsigned int y0, unsigned long rgb0, unsigned int x1, unsigned int y1, unsigned long rgb1)
{
    st_Gradient[number].x0 = x0;
    st_Gradient[number].y0 = y0;
    st_Gradient[number].rgb0 = rgb0;
    st_Gradient[number].x1 = x1;
    st_Gradient[number].y1 = y1;
    st_Gradient[number].rgb1 = rgb1;
    gradient_nb++;
}

void FT8XX_EVE::draw_gradient (STGradient *st_Gradient)
{
    write_dl_long(CMD_GRADIENT);        // 
    write_dl_int(st_Gradient->x0);       //
    write_dl_int(st_Gradient->y0);       // 
    write_dl_long(st_Gradient->rgb0);    // 
    write_dl_int(st_Gradient->x1);     //
    write_dl_int(st_Gradient->y1);     //
    write_dl_long(st_Gradient->rgb1);    //         
}

void FT8XX_EVE::modify_gradient (STGradient *st_Gradient, unsigned char type, unsigned long value)
{
    switch(type)
    {
        case GRADIENT_X0:
            st_Gradient->x0 = value;
        break;

        case GRADIENT_Y0:
            st_Gradient->y0 = value;
        break;

        case GRADIENT_RGB0:
            st_Gradient->rgb0 = value;
        break;

        case GRADIENT_X1:
            st_Gradient->x1 = value;
        break;

        case GRADIENT_Y1:
            st_Gradient->y1 = value;
        break;

        case GRADIENT_RGB1:
            st_Gradient->rgb1 = value;
        break;

        default:
        break;
    }
}

unsigned char FT8XX_EVE::get_gradient_nb (void)
{
    return gradient_nb;
}
#endif

//*********void init_number (unsigned char number, unsigned int x, unsigned int y, unsigned int f, unsigned int o, unsigned long n*********//
//Description : Function will init a st_Number[number] object with input values
//
//Function prototype : void init_number (unsigned char number, unsigned int x, y, f, o, unsigned long n))
//
//Enter params       : unsigned char : number : number of st_Number object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          f      : font of object
//                          o      : object options (refer to FT datasheet for valid options)
//                     unsigned long : n      : value of number (0 to 2^32)
//
//Exit params        : none
//
//Function call      : init_number (0, 50, 50, 23, OPT_NONE, 0x0A0A);
//                     //Init number 0 at position 50,50 with font 23, no option
//                     and with a value of 0x0A0A
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_NUMBER_NB > 0
void FT8XX_EVE::CMD_number (unsigned char number, unsigned int x, unsigned int y, unsigned int f, unsigned int o, unsigned long n)
{
    st_Number[number].x = x;   //initialise struct
    st_Number[number].y = y;
    st_Number[number].font = f;
    st_Number[number].opt = o;
    st_Number[number].num = n;
    number_nb++;
}

//*******************void FT_draw_number (STNumber *st_Number)*****************//
//Description : Function draws number, specified by STNumber properties
//
//Function prototype : void FT_draw_number (STNumber *st_Number)
//
//Enter params       : *st_Number : number struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_number (st_Number[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_number (STNumber *st_Number)
{
    write_dl_long(CMD_NUMBER);
    write_dl_int(st_Number->x);    // x
    write_dl_int(st_Number->y);    // y
    write_dl_int(st_Number->font); // font
    write_dl_int(st_Number->opt);  // option
    write_dl_long(st_Number->num); // 32 bit number
}

void FT8XX_EVE::modify_number (STNumber *st_Number, unsigned char type, unsigned long value)
{
    switch (type)
    {
        case NUMBER_X:
            st_Number->x = value;
        break;

        case NUMBER_Y: 
            st_Number->y = value;
        break;

        case NUMBER_FONT:
            st_Number->font = value;
        break;

        case NUMBER_OPT:
            st_Number->opt = value;
        break;

        case NUMBER_VAL:
            st_Number->num = value;
        break;

        default:
        break;
    }
}

unsigned char FT8XX_EVE::get_number_nb (void)
{
    return number_nb;
}
#endif //#if MAX_NUMBER_NB > 0

//********void init_window (unsigned char number, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)***********//
//Description : Function will init a st_Window[number] touch window
//              A window is NOT a viewable rectangle : it's transparent and
//              used to track&detect touch tag on the display to process touch
//              input.
//
//Function prototype : void init_window (unsigned char number, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
//
//Enter params       : unsigned char : number : number of st_Window object(0 to MAX_PRIM_XX)
//                     unsigned int : x1     : x start position
//                          y1     : y start position
//                          x2     : x end position
//                          y2     : y end position
//
//Exit params        : none
//
//Function call      : init_window (100, 50, 200, 100);
//                     Init a touch window that start from (100,50) and ends
//                     at (200,100) (x,y)
//
//                  (100,50)
//                     ____________________________________
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |____________________________________|
//                                                      (200,100)
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_WINDOW_NB > 0
void FT8XX_EVE::CMD_window (unsigned char number, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    st_Window[number].x1 = x1;        //set struct default values
    st_Window[number].x2 = x2;
    st_Window[number].y1 = y1;
    st_Window[number].y2 = y2;
    st_Window[number].ucCntr = 1;
    st_Window[number].ucNewState = 0;
    st_Window[number].ucOldState = 0;
    st_Window[number].ucTouchGood = 0;
    st_Window[number].ucReadOK = 0;
    st_Window[number].one_touch = 0;
}
#endif //#if MAX_WINDOW_NB > 0


//****void init_rectangle (unsigned char number, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int w)******//
//Description : Function will init a st_Rectangle[number] rectangle
//              A rectangle is viewable on the display : it writes 4 lines
//              of width w
//
//Function prototype : void init_rectangle (unsigned char number, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int w))
//
//Enter params       : unsigned char : number : number of st_Rectangle object(0 to MAX_PRIM_XX)
//                     unsigned int : x1     : x start position
//                          y1     : y start position
//                          x2     : x end position
//                          y2     : y end position
//                           w     : width of line (refer to FT801 datasheet)
//
//Exit params        : none
//
//Function call      : init_rectangle (100, 50, 200, 100, 10);
//                     Init a rectangle that start from (100,50) and ends
//                     at (200,100) (x,y) with line width of 10
//
//                  (100,50)
//                     ____________________________________
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |                                    |
//                    |____________________________________|
//                                                      (200,100)
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_RECT_NB > 0
void FT8XX_EVE::CMD_rectangle (unsigned char number, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int w)
{
    st_Rectangle[number].x1 = x1;//set struct default values
    st_Rectangle[number].y1 = y1;
    st_Rectangle[number].x2 = x2;
    st_Rectangle[number].y2 = y2;
    st_Rectangle[number].w = w;
}

//**************void FT_draw_rectangle (STRectangle *st_Rectangle)************//
//Description : FT function to draw a rectangle from st_Rectangle parameters
//
//Function prototype : void FT_draw_rectangle (STRectangle *st_Rectangle)
//
//Enter params       : *st_Rectangle : struct which contains rectangle values
//
//Exit params        : none
//
//Function call      : FT_draw_rectangle (&strectangle[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_rectangle (STRectangle *st_Rectangle)
{
    //function calls FT_draw_line_h and FT_draw_line_v which handles
    //line drawing for x and y positions
    draw_line_h(st_Rectangle->x1, st_Rectangle->x2, st_Rectangle->y1, st_Rectangle->w);
    draw_line_h(st_Rectangle->x1, st_Rectangle->x2, st_Rectangle->y2, st_Rectangle->w);
    draw_line_v(st_Rectangle->y1, st_Rectangle->y2, st_Rectangle->x1, st_Rectangle->w);
    draw_line_v(st_Rectangle->y1, st_Rectangle->y2, st_Rectangle->x2, st_Rectangle->w);
}
#endif //#if MAX_RECT_NB > 0


//void init_togglesw (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int f, unsigned int o, const char *str)//
//Description : Function will init a st_Togglesw[number] object with input values
//
//Function prototype : void init_togglesw(unsigned char number, unsigned int x, y, w, f, o, const char *str)
//
//Enter params       : unsigned char : number : number of st_Togglesw object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          w      : width of center circle
//                          f      : object font (refer to FT datasheet for valid font)
//                          o      : object options (refer to FT datasheet for valid options)
//                     const char *   : on/off toggle text, seperated by \xff
//
//Exit params        : none
//
//Function call      : init_togglesw (0, 50, 50, 20, 23, OPT_NONE, "OFF\xffON");
//                     //init togglesw at pos 50,50, with a center circle width
//                     of 20, text at font 23, NO options and both toggle state
//                     text are OFF and ON
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_TOGGLE_NB > 0
void FT8XX_EVE::CMD_toggle (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int f, unsigned int o, unsigned char state, const char *str)
{
    unsigned char cnt = 0, temp = 0;//set struct default values
    st_Toggle[number].x1 = x;
    st_Toggle[number].y1 = y;
    st_Toggle[number].w = w;
    st_Toggle[number].f = f;
    st_Toggle[number].opt = o;
    st_Toggle[number].state = state; //0 by default, FFFF = 1
    //This while loop fills st_Button struct with input string
    while (*str != 0x00)
    {
        st_Toggle[number].str[cnt] = *str;
        str++;
        cnt++;
    }

    //this part of the code ensures that transactions are 4 bytes wide. if
    //string byte # is not a multiple of 4, data is appended with null bytes
    st_Toggle[number].str[cnt] = 0x00;
    cnt++;
    temp = cnt & 0x0F;
    if ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
    {
        while ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
        {
        st_Toggle[number].str[cnt] = 0x00;
        cnt++;
        temp = cnt & 0x0F;
        }
    }
    st_Toggle[number].len = cnt;//write length to struct

    toggle_nb++;
}


//***************void FT_draw_togglesw (STTogglesw *st_Togglesw)**************//
//Description : Function draws a toggle switch, specified by STTogglesw properties
//
//Function prototype : void FT_draw_togglesw (STTogglesw *st_Togglesw)
//
//Enter params       : *st_Togglesw : togglesw struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_togglesw (&st_Togglesw[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_toggle (STToggle *st_Toggle)
{
    unsigned char cnt = 0;
    write_dl_long(CMD_TOGGLE);
    write_dl_int(st_Toggle->x1);    // x
    write_dl_int(st_Toggle->y1);    // y
    write_dl_int(st_Toggle->w);     //
    write_dl_int(st_Toggle->f);     // font
    write_dl_int(st_Toggle->opt);   // option
    write_dl_int(st_Toggle->state); // state
    while (cnt < st_Toggle->len)         // write text until eos
    {
        write_dl_char(st_Toggle->str[cnt]);
        cnt++;
    }
}

void FT8XX_EVE::change_toggle_state (STToggle *st_Toggle, unsigned char state)
{
    st_Toggle->state = state;
}

unsigned char FT8XX_EVE::get_toggle_nb (void)
{
    return toggle_nb;
}
#endif //#if MAX_TOGGLE_NB > 0


//*******void init_dial (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned int val)********//
//Description : Function will init a st_Dial[number] object with input values
//
//Function prototype : void init_dial (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned int val)
//
//Enter params       : unsigned char : number : number of st_Dial object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          r      : circle radius
//                          opt    : object font (refer to FT datasheet for valid font)
//                          val    : dial initial value (refer to FT datasheet for val/position relationship)
//
//Exit params        : none
//
//Function call      : init_dial (0, 50, 50, 20, OPT_NONE, 0x8000);
//                     //init dial at pos 50,50, with a center circle radius
//                     of 20, NO options and dial is at 0 oClock
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_DIAL_NB > 0
void FT8XX_EVE::CMD_dial (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned int val)
{
    st_Dial[number].x = x;    //set struct default values
    st_Dial[number].y = y;
    st_Dial[number].r = r;
    st_Dial[number].opt = opt;
    st_Dial[number].val = val;
    dial_nb++;
}

//********************void FT_draw_dial (STDial *st_Dial)***********************//
//Description : FT function to draw a dial, specified through st_Dial
//
//Function prototype : void FT_draw_dial (STDial *st_Dial)
//
//Enter params       : *st_Dial : st_Dial struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_dial (&st_Dial[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_dial (STDial *st_Dial)
{
    write_dl_long(CMD_DIAL);     //write FT command to draw a dial
    write_dl_int(st_Dial->x);//write values to command
    write_dl_int(st_Dial->y);
    write_dl_int(st_Dial->r);
    write_dl_int(st_Dial->opt);
    write_dl_int(st_Dial->val);
    write_dl_int(0);
}

unsigned char FT8XX_EVE::get_dial_nb (void)
{
    return dial_nb;
}
#endif //#if MAX_DIAL_NB > 0


//void init_progress (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int val, unsigned int range)//
//Description : Function will init a st_Progress[number] object with input values
//
//Function prototype : void init_progress (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int val, unsigned int range)
//
//Enter params       : unsigned char : number : number of st_Progress object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          w      : width of bar
//                          h      : height of bar
//                          opt    : options for object
//                          val    : bar value
//                          range  : bar min-max range
//
//Exit params        : none
//
//Function call      : init_progress (0, 50, 50, 20, 100, OPT_NONE, 50, 100);
//                   //init a progress bar from pos (50, 50), width of 20 and
//                   height of 20, with no special options and initialize it
//                   at mid position (value of 50 over a range of 100)
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_PROGRESS_NB > 0
void FT8XX_EVE::CMD_progress (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int val, unsigned int range)
{
  st_Progress[number].x = x;        //set struct default values
  st_Progress[number].y = y;
  st_Progress[number].w = w;
  st_Progress[number].h = h;
  st_Progress[number].opt = opt;
  st_Progress[number].val = val;
  st_Progress[number].range = range;
  progress_nb++;
}

//**************void FT_draw_progress (STProgress *st_Progress)***************//
//Description : FT function to draw a progress bar, specified through st_Progress
//
//Function prototype : void FT_draw_progress (STProgress *st_Progress)
//
//Enter params       : *st_Progress : st_Progress struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_progress (&st_Progress[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_progress (STProgress *st_Progress)
{
    write_dl_long(CMD_PROGRESS);        //write FT command to draw a progress bar
    write_dl_int(st_Progress->x);       //write values to command
    write_dl_int(st_Progress->y);
    write_dl_int(st_Progress->w);
    write_dl_int(st_Progress->h);
    write_dl_int(st_Progress->opt);
    write_dl_int(st_Progress->val);
    write_dl_int(st_Progress->range);
    write_dl_int(0);
}

void FT8XX_EVE::modify_progress (STProgress *st_Progress, unsigned char val)
{
    st_Progress->val = val;
}

unsigned char FT8XX_EVE::get_progress_nb (void)
{
    return progress_nb;
}

#endif //#if MAX_PROGRESS_NB > 0



//void init_scroller (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int val, unsigned int size, unsigned int range)//
//Description : Function will init a st_Scroller[number] object with input values
//
//Function prototype : void init_scroller (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int val, unsigned int size, unsigned int range)
//
//Enter params       : unsigned char : number : number of st_Scroller object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          w      : width of bar
//                          h      : height of bar
//                          opt    : options for object
//                          val    : bar value
//                          size   : size value (scroller width)
//                          range  : bar min-max range
//
//Exit params        : none
//
//Function call      : init_scroller (0, 50, 50, 20, 100, OPT_NONE, 50, 10 100);
//                   //init a scroller from pos (50, 50), width of 20 and
//                   height of 20, with no special options and initialize it
//                   at mid position (value of 50 over a range of 100) with a
//                   scroller width of 10
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_SCROLLBAR_NB > 0
void FT8XX_EVE::CMD_scrollbar (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int val, unsigned int size, unsigned int range)
{
    st_Scrollbar[number].x = x;        //set struct default values
    st_Scrollbar[number].y = y;
    st_Scrollbar[number].w = w;
    st_Scrollbar[number].h = h;
    st_Scrollbar[number].opt = opt;
    st_Scrollbar[number].val = val;
    st_Scrollbar[number].size = size;
    st_Scrollbar[number].range = range;
    scrollbar_nb++;
}

//**************void FT_draw_scroller (STScroller *st_Scroller)***************//
//Description : FT function to draw a scroller, specified through st_Scroller
//
//Function prototype : void FT_draw_scroller (STScroller *st_Scroller)
//
//Enter params       : *st_Scroller : scroller struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_scroller (&st_Scroller[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_scrollbar (STScrollbar *st_Scrollbar)
{
    write_dl_long(CMD_SCROLLBAR);    //write FT command to draw scrollbar
    write_dl_int(st_Scrollbar->x);//write values to command
    write_dl_int(st_Scrollbar->y);
    write_dl_int(st_Scrollbar->w);
    write_dl_int(st_Scrollbar->h);
    write_dl_int(st_Scrollbar->opt);
    write_dl_int(st_Scrollbar->val);
    write_dl_int(st_Scrollbar->size);
    write_dl_int(st_Scrollbar->range);
}

void FT8XX_EVE::modify_scrollbar (STScrollbar *st_Scrollbar, unsigned char type, unsigned int value)
{
    switch (type)
    {
        case SCROLLBAR_X:
            st_Scrollbar->x = value;
        break;

        case SCROLLBAR_Y:
            st_Scrollbar->y = value;
        break;

        case SCROLLBAR_WIDTH:
            st_Scrollbar->w = value;
        break;

        case SCROLLBAR_HEIGHT:
            st_Scrollbar->h = value;
        break;

        case SCROLLBAR_OPT:
            st_Scrollbar->opt = value;
        break;

        case SCROLLBAR_VAL:
            st_Scrollbar->val = value;
        break;
        
        case SCROLLBAR_SIZE:
            st_Scrollbar->size = value;
        break;

        case SCROLLBAR_RANGE:
            if (value > st_Scrollbar->range)
            {
                st_Scrollbar->val = st_Scrollbar->range;
            }
            else
            st_Scrollbar->range = value;
        break;

        default:
        break;
    }
}

unsigned char FT8XX_EVE::get_scrollbar_nb (void)
{
    return scrollbar_nb;
}
#endif //#if MAX_SCROLLBAR_NB > 0


//void init_gauge (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned int maj, unsigned int min, unsigned int val, unsigned int range)//
//Description : Function will init a st_Gauge[number] object with input values
//
//Function prototype : void init_gauge (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned int maj, unsigned int min, unsigned int val, unsigned int range)
//
//Enter params       : unsigned char : number : number of st_Gauge object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          r      : radius of gauge
//                          opt    : options for object
//                          maj    : major divisions value
//                          min    : minor divisions value
//                          val    : initial value
//                          range  : gauge range
//
//Exit params        : none
//
//Function call      : init_gauge (0, 50, 50, 20, OPT_NONE, 10, 5, 50, 100);
//                   //init a gauge at pos (50,50) with a radius of 20, no opt,
//                   10 major divisions, each major division is divided in 5
//                   minor division, with a value of 50 over a range of 100
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_GAUGE_NB > 0
void FT8XX_EVE::CMD_gauge (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned int maj, unsigned int min, unsigned int val, unsigned int range)
{
    st_Gauge[number].x = x;        //set struct default values
    st_Gauge[number].y = y;
    st_Gauge[number].r = r;
    st_Gauge[number].opt = opt;
    st_Gauge[number].maj = maj;
    st_Gauge[number].min = min;
    st_Gauge[number].val = val;
    st_Gauge[number].range = range;
    gauge_nb++;
}

//******************void FT_draw_gauge (STGauge *st_Gauge)********************//
//Description : FT function to draw a gauge, specified through st_Gauge
//
//Function prototype : void FT_draw_gauge (STGauge *st_Gauge)
//
//Enter params       : *st_Gauge : st_Gauge struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_gauge (&st_Gauge[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_gauge (STGauge *st_Gauge)
{
    write_dl_long(CMD_GAUGE);     //write FT command to draw a gauge
    write_dl_int(st_Gauge->x);//write values to command
    write_dl_int(st_Gauge->y);
    write_dl_int(st_Gauge->r);
    write_dl_int(st_Gauge->opt);
    write_dl_int(st_Gauge->maj);
    write_dl_int(st_Gauge->min);
    write_dl_int(st_Gauge->val);
    write_dl_int(st_Gauge->range);
}

void FT8XX_EVE::modify_gauge (STGauge *st_Gauge, unsigned char type, unsigned int value)
{
    switch(type)
    {
        case GAUGE_X:
            st_Gauge->x = value;
        break;

        case GAUGE_Y:
            st_Gauge->y = value;
        break;

        case GAUGE_RADIUS:
            st_Gauge->r = value;
        break;

        case GAUGE_OPT:
            st_Gauge->opt = value;
        break;

        case GAUGE_MAJ:
            st_Gauge->maj = value;
        break;

        case GAUGE_MIN:
            st_Gauge->min = value;
        break;

        case GAUGE_VAL:
            st_Gauge->val = value;
        break;

        case GAUGE_RANGE:
            st_Gauge->range = value;
        break;

        default:
        break;
    }
}

unsigned char FT8XX_EVE::get_gauge_nb (void)
{
    return gauge_nb;
}
#endif //#if MAX_GAUGE_NB > 0


//void init_clock (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned char h, unsigned char m, unsigned char s, unsigned char ms)//
//Description : Function will init a st_Clock[number] object with input values
//
//Function prototype : void init_clock (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned char h, unsigned char m, unsigned char s, unsigned char ms)
//
//Enter params       : unsigned char : number : number of st_Clock object(0 to MAX_PRIM_XX)
//                     unsigned int : x      : x position on screen
//                          y      : y position on screen
//                          r      : radius of clock
//                          opt    : options for object
//                          h      : hours value
//                          m      : minutes value
//                          s      : seconds value
//                          ms     : milliseconds value
//
//Exit params        : none
//
//Function call      : init_clock(0, 50, 50, 50, OPT_NONE, 8, 15, 0, 0);
//                     init a clock at position (50,50) with a radius of 50,
//                     no options and time set to 8:15:00:00
//
//Intellitrol  08/07/2016
//******************************************************************************
#if MAX_CLOCK_NB > 0
void FT8XX_EVE::CMD_clock (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned char h, unsigned char m, unsigned char s, unsigned char ms)
{
    st_Clock[number].x = x;        //set struct default values
    st_Clock[number].y = y;
    st_Clock[number].r = r;
    st_Clock[number].opt = opt;
    st_Clock[number].h = h;
    st_Clock[number].m = m;
    st_Clock[number].s = s;
    st_Clock[number].ms = ms;
    clock_nb++;
}

//*******************void FT_draw_clock (STClock *st_Clock)*********************//
//Description : FT function to draw a clock, specified through STClock struct
//
//Function prototype : void FT_draw_clock (STClock *st_Clock)
//
//Enter params       : *st_Clock : clock struct including gfx parameters
//
//Exit params        : none
//
//Function call      : FT_draw_clock (&st_Clock[0]);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_clock (STClock *st_Clock)
{
    write_dl_long(CMD_CLOCK);       //write FT command to draw a clock
    write_dl_int(st_Clock->x);  //write values to command
    write_dl_int(st_Clock->y);
    write_dl_int(st_Clock->r);
    write_dl_int(st_Clock->opt);
    write_dl_int(st_Clock->h);
    write_dl_int(st_Clock->m);
    write_dl_int(st_Clock->s);
    write_dl_int(st_Clock->ms);
}

//*******void FT_modify_clock_hms (STClock *st_Clock, unsigned char h, unsigned char m, unsigned char s)*******//
//Description : FT function updates h,m,s value of input st_Clock
//
//Function prototype : void FT_modify_clock_hms (STClock *st_Clock, unsigned char h, unsigned char m, unsigned char s)
//
//Enter params       : *st_Clock : clock struct including gfx parameters
//                   : unsigned char h     : hour value
//                        m     : minute value
//                        s     : second value
//
//Exit params        : none
//
//Function call      : FT_modify_clock_hms(&st_Clock[0], 8, 15, 0);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::modify_clock_hms (STClock *st_Clock, unsigned char h, unsigned char m, unsigned char s)
{
    st_Clock->h = h;
    st_Clock->m = m;
    st_Clock->s = s;
}

unsigned char FT8XX_EVE::get_clock_nb (void)
{
    return clock_nb;
}
#endif //#if MAX_CLOCK_NB > 0   

#if MAX_KEYS_NB > 0
void FT8XX_EVE::CMD_keys (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int f, unsigned int opt, char *str)
{
    unsigned char cnt = 0, temp = 0;//set struct default values
    st_Keys[number].x = x;
    st_Keys[number].y = y;
    st_Keys[number].w = w;
    st_Keys[number].h = h;
    st_Keys[number].f = f;
    st_Keys[number].opt = opt;
    //This while loop fills st_Button struct with input string
    while (*str != 0x00)
    {
        st_Keys[number].str[cnt] = *str;
        str++;
        cnt++;
    }

    //this part of the code ensures that transactions are 4 bytes wide. if
    //string byte # is not a multiple of 4, data is appended with null bytes
    st_Keys[number].str[cnt] = 0x00;
    cnt++;
    temp = cnt & 0x0F;
    if ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
    {
        while ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
        {
        st_Keys[number].str[cnt] = 0x00;
        cnt++;
        temp = cnt & 0x0F;
        }
    }
    st_Keys[number].len = cnt;//write length to struct
    keys_nb++;
}

void FT8XX_EVE::draw_keys(STKeys *st_Keys)
{
    unsigned char cnt = 0;
    write_dl_long(CMD_KEYS);
    write_dl_int(st_Keys->x);
    write_dl_int(st_Keys->y);
    write_dl_int(st_Keys->w);
    write_dl_int(st_Keys->h);
    write_dl_int(st_Keys->f);
    write_dl_int(st_Keys->opt);
    while (cnt < st_Keys->len)         // write text until eos
    {
        write_dl_char(st_Keys->str[cnt]);
        cnt++;
    }
}

unsigned char FT8XX_EVE::get_keys_nb (void)
{
    return keys_nb;
}
#endif

void FT8XX_EVE::CMD_tracker(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char tag)
{
    write_dl_long(CMD_TRACK);       //write FT command to draw a clock
    write_dl_int(x);  //write values to command
    write_dl_int(y);
    write_dl_int(w);
    write_dl_int(h);
    write_dl_int(tag); 
}

//********************void FT_clear_screen (unsigned long color)************************//
//Description : FT function to clear primitives on screen and update backgrnd
//              color with specified input color
//
//Function prototype : void FT_clear_screen (unsigned long color)
//
//Enter params       : unsigned long color : background color (0,R,G,B)(r,g,b = 1 byte each)
//
//Exit params        : none
//
//Function call      : FT_clear_screen (RED);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::clear_screen (unsigned long color)
{
    write_dl_long(CLEAR_COLOR_RGB(color >> 16, color >> 8, color)); //Write color
    write_dl_long(CLEAR(1, 1, 1));       //clear primitives, cache and backgrnd
}

//***************void FT_draw_line_h (unsigned int x1, unsigned int x2, unsigned int y, unsigned int w)**************//
//Description : FT function to draw a horizontal line from x1 to x2, at y pos,
//              with a width of w
//
//Function prototype : void FT_draw_line_h (unsigned int x1, unsigned int x2, unsigned int y, unsigned int w)
//
//Enter params       : unsigned int x1 : x start value
//                        x2 : x end value
//                         y : y pos value
//                         w : line width value
//
//Exit params        : none
//
//Function call      : FT_draw_line_h (20, 70, 120, 10);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_line_h (unsigned int x1, unsigned int x2, unsigned int y, unsigned int w)
{
    write_dl_long(BEGIN(LINES));         //Begin primitive
    write_dl_long(LINE_WIDTH(w));        //write line width
    write_dl_long(VERTEX2F(x1 * 16, y * 16)); //draw line
    write_dl_long(VERTEX2F(x2 * 16, y * 16)); //draw line
}

//***************void FT_draw_line_v (unsigned int y1, unsigned int y2, unsigned int x, unsigned int w)**************//
//Description : FT function to draw a vertical line from y1 to y2, at x pos,
//              with a width of w
//
//Function prototype : void FT_draw_line_v (unsigned int y1, unsigned int y2, unsigned int x, unsigned int w)
//
//Enter params       : unsigned int y1 : y start value
//                        y2 : y end value
//                         x : x pos value
//                         w : line width value
//
//Exit params        : none
//
//Function call      : FT_draw_line_v (20, 70, 120, 10);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::draw_line_v (unsigned int y1, unsigned int y2, unsigned int x, unsigned int w)
{
    write_dl_long(BEGIN(LINES));         //Begin primitive
    write_dl_long(LINE_WIDTH(w));        //write line width
    write_dl_long(VERTEX2F(x * 16, y1 * 16)); //draw line
    write_dl_long(VERTEX2F(x * 16, y2 * 16)); //draw line
}

//*****************STTouch FT_touchpanel_read (STTouch touch_read)***********//
//Description : Function read touch panel (touch 1 to 5)
//
//Function prototype : STTouch FT_touchpanel_read (STTouch touch_read)
//
//Enter params       : STTouch : struct which contains touch values
//
//Exit params        : STTouch : struct which contains new touch values
//
//Function call      : touch_data = FT_touchpanel_read (touch_data);
//
//Intellitrol  08/07/2016
//******************************************************************************
STTouch FT8XX_EVE::touchpanel_read (STTouch touch_read)
{
    //Function scroll through each touch register, read them and convert them
    unsigned long temp_data;
    #ifdef TOUCH_PANEL_CAPACITIVE
        
        temp_data = rd32(REG_CTOUCH_TOUCH0_XY);
        touch_read.X0 = (temp_data >> 16);
        touch_read.Y0 = temp_data;


        temp_data = rd32(REG_CTOUCH_TOUCH1_XY);
        touch_read.X1 = (temp_data >> 16);
        touch_read.Y1 = temp_data;


        temp_data = rd32(REG_CTOUCH_TOUCH2_XY);
        touch_read.X2 = (temp_data >> 16);
        touch_read.Y2 = temp_data;


        temp_data = rd32(REG_CTOUCH_TOUCH3_XY);
        touch_read.X3 = (temp_data >> 16);
        touch_read.Y3 = temp_data;


        touch_read.X4 = rd16(REG_CTOUCH_TOUCH4_X);
        touch_read.Y4 = rd16(REG_CTOUCH_TOUCH4_Y);
        return touch_read;   //return new struct with new values
    #endif

    #ifdef TOUCH_PANEL_RESISTIVE
        temp_data = rd32(REG_TOUCH_SCREEN_XY);
        touch_read.X1 = (temp_data >> 16);
        touch_read.Y1 = temp_data;
        return touch_read;   //return new struct with new values
    #endif
}

//******unsigned char ucCheckTouchWindow (STWindow *st_Window, STTouch touch_data)*******//
//Description : Function read touch input and analyze if touch is inside
//              specified st_Window. If so, debounce the window and make it
//              active,
//
//Function prototype : unsigned char ucCheckTouchWindow (STWindow *st_Window, STTouch touch_data)
//
//Enter params       : STTouch   : struct which contains touch values
//                     *st_Window : pointer to struct which contains window value
//
//Exit params        : unsigned char : window active (1) or inactive(0)
//
//Function call      : unsigned char = ucCheckTouchWindow (&st_Window[0], touch_data);
//
//Intellitrol  08/07/2016
//*****************************************************************************
#if MAX_WINDOW_NB > 0
unsigned char FT8XX_EVE::check_window (STWindow *st_Window, STTouch touch_data)
{
    unsigned char ucRet = 0;
    //look via ucCheckTouch() if x and input parameter from touch data are
    //inside specified touch window
    if (check_screen_press(touch_data) == 1)
    {
        if (check_touch(st_Window, touch_data) == 1)
        {
            if (++st_Window->ucCntr >= 5) //Treshold counter, debouncer
            {
                st_Window->ucCntr = 5;      //Value standstill
                st_Window->ucTouchGood = 1; //TouchGood value = true, window active
            }
        }

        //read value is not inside specified st_Window
        else
        {
            if (--st_Window->ucCntr <= 1)  //Decrem treshold counter
            {
                st_Window->ucCntr = 1;      //Treshold min value reload
                st_Window->ucTouchGood = 0; //TouchGood = false, window inactive
            }
        }
        //save new and old button detection
        st_Window->ucOldState = st_Window->ucNewState;
        st_Window->ucNewState = 1;
    }
    //no tactile press detected
    else
    {
        if (--st_Window->ucCntr <= 1)  //Decrem treshold counter
        {
            st_Window->ucCntr = 1;      //Treshold min value reload
            st_Window->ucTouchGood = 0; //TouchGood = false
        }
        //save new and old button detection
        st_Window->ucOldState = st_Window->ucNewState;
        st_Window->ucNewState = 0;
    }
    //if old state was released and new state is pressed, and touchgood is true
    //we have a successful active window
    if ((st_Window->ucOldState == 1) && (st_Window->ucNewState == 1))
    {
        if (st_Window->ucTouchGood == 1) 
        {
            ucRet = 1; //active window
        }

        else 
        {
            ucRet = 0;
        }
    }
    return (ucRet);
}

//**********unsigned char ucCheckTouch (STWindow *st_Window, STTouch touch_data)*********//
//Description : Function verify that provided touch_data is inside specified
//              st_Window.
//
//Function prototype : unsigned char ucCheckTouch (STWindow *st_Window, STTouch touch_data)
//
//input param        : *st_Window : struct which contains window values
//                     STTouch   : struct which contains touch values
//
//output param       : unsigned char : 1 = true, 0 = false
//
//function call  : ucCheckTouch (50,50,200,150,ucTouchX, ucTouchY);
//
//Intellitrol  08/07/2016
//***************************************************************************//
unsigned char FT8XX_EVE::check_touch (STWindow *st_Window, STTouch touch_data)
{
  unsigned char X1G = 0, Y1G = 0, X2G = 0, Y2G = 0, X3G = 0, Y3G = 0, X4G = 0, Y4G = 0,
                X5G = 0, Y5G = 0, CH1 = 0, CH2 = 0, CH3 = 0, CH4 = 0, CH5 = 0, TOT = 0;
  if ((touch_data.X0 >= st_Window->x1) && (touch_data.X0 <= st_Window->x2)) {
    X1G = 1;
  }
  else {
    X1G = 0;
  }
  if ((touch_data.Y0 >= st_Window->y1) && (touch_data.Y0 <= st_Window->y2)) {
    Y1G = 1;
  }
  else {
    Y1G = 0;
  }
  if ((X1G == 1) && (Y1G == 1)) {
    CH1 = 1;
    st_Window->touch_tag = 1;
  }
  else {
    CH1 = 0;
    st_Window->touch_tag = 0;
  }

  if (CH1 == 0) //window was not inside channel 0 value, scroll through channel 1
  {
    if ((touch_data.X1 >= st_Window->x1) && (touch_data.X1 <= st_Window->x2)) {
      X2G = 1;
    }
    else {
      X2G = 0;
    }
    if ((touch_data.Y1 >= st_Window->y1) && (touch_data.Y1 <= st_Window->y2)) {
      Y2G = 1;
    }
    else {
      Y2G = 0;
    }
    if ((X2G == 1) && (Y2G == 1)) {
      CH2 = 1;
      st_Window->touch_tag = 2;
    }
    else {
      CH2 = 0;
      st_Window->touch_tag = 0;
    }
  }

  if ((CH1 == 0) && (CH2 == 0)) //not yet pressed
  {
    if ((touch_data.X2 >= st_Window->x1) && (touch_data.X2 <= st_Window->x2)) {
      X3G = 1;
    }
    else {
      X3G = 0;
    }
    if ((touch_data.Y2 >= st_Window->y1) && (touch_data.Y2 <= st_Window->y2)) {
      Y3G = 1;
    }
    else {
      Y3G = 0;
    }
    if ((X3G == 1) && (Y3G == 1)) {
      CH3 = 1;
      st_Window->touch_tag = 3;
    }
    else {
      CH3 = 0;
      st_Window->touch_tag = 0;
    }
  }

  if ((CH1 == 0) && (CH2 == 0) && (CH3 == 0)) //not yet pressed
  {
    if ((touch_data.X3 >= st_Window->x1) && (touch_data.X3 <= st_Window->x2)) {
      X4G = 1;
    }
    else {
      X4G = 0;
    }
    if ((touch_data.Y3 >= st_Window->y1) && (touch_data.Y3 <= st_Window->y2)) {
      Y4G = 1;
    }
    else {
      Y4G = 0;
    }
    if ((X4G == 1) && (Y4G == 1)) {
      CH4 = 1;
      st_Window->touch_tag = 4;
    }
    else {
      CH4 = 0;
      st_Window->touch_tag = 0;
    }
  }

  if ((CH1 == 0) && (CH2 == 0) && (CH3 == 0) && (CH4 == 0)) //not yet pressed
  {
    if ((touch_data.X4 >= st_Window->x1) && (touch_data.X4 <= st_Window->x2)) {
      X5G = 1;
    }
    else {
      X5G = 0;
    }
    if ((touch_data.Y4 >= st_Window->y1) && (touch_data.Y4 <= st_Window->y2)) {
      Y5G = 1;
    }
    else {
      Y5G = 0;
    }
    if ((X5G == 1) && (Y5G == 1)) {
      CH5 = 1;
      st_Window->touch_tag = 5;
    }
    else {
      CH5 = 0;
      st_Window->touch_tag = 0;
    }
  }

  if ((CH1 == 1) || (CH2 == 1) || (CH3 == 1) || (CH4 == 1) || (CH5 == 1)) {
    TOT = 1;
  }
  else {
    TOT = 0;
  }
  return (TOT);
}

unsigned char FT8XX_EVE::check_screen_press (STTouch touch_data)
{
  if ((touch_data.X0 != 0x8000) || (touch_data.X1 != 0x8000) ||
      (touch_data.X2 != 0x8000) || (touch_data.X3 != 0x8000) || (touch_data.X4 != 0x8000))
  {
        return 1;
  }
  else 
        return 0;
}
#endif //#if MAX_WINDOW_NB > 0



//******void FT_modify_element_string (unsigned char number, unsigned char type, const char *str)*****//
//Description : Function modify input primitives string, calculate its new
//              length and append the required bytes if necessary
//
//Function prototype : void FT_modify_element_string (unsigned char number, unsigned char type, const char *str)
//
//Enter params       : number : number of the element (stBoite[number])
//                     type   : type of primitive (defined in FT8XX.h)
//                     *str   : string to modify
//
//Exit params        : None
//
//Function call      : FT_modify_element_string(0, FT_PRIM_TOGGLESW, "on\xffoff);
//
//Intellitrol  08/07/2016
//******************************************************************************
void FT8XX_EVE::modify_element_string (unsigned char number, unsigned char type, char * str)
{
  unsigned char cnt = 0, temp = 0;

  //Based on primitive type, same code is executed to append text / calculate
  //offset for display list to be a multiple of 5
  switch (type)
  {
    #if MAX_TEXT_NB > 0
    case FT_PRIM_TEXT:
      while (*str != 0)
      {
        st_Text[number].str[cnt] = *str; //write string to struct
        str++;
        cnt++;
      }
      st_Text[number].str[cnt] = 0x00;     //last byte must be a 0
      cnt++;
      temp = cnt & 0x0F;
      //if string byte # is not a multiple of 4 bytes, append null bytes
      if ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
      {
        while ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
        {
          st_Text[number].str[cnt] = 0x00;
          cnt++;
          temp = cnt & 0x0F;
        }
      }
      //get string length into struct
      st_Text[number].len = cnt;
      break;
#endif //#if MAX_TEXT_NB > 0

#if MAX_BUTTON_NB > 0
    case FT_PRIM_BUTTON:
      while (*str != 0)
      {
        st_Button[number].str[cnt] = *str;
        str++;
        cnt++;
      }
      st_Button[number].str[cnt] = 0x00;
      cnt++;
      temp = cnt & 0x0F;
      if ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
      {
        while ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
        {
          st_Button[number].str[cnt] = 0x00;
          cnt++;
          temp = cnt & 0x0F;
        }
      }
      st_Button[number].len = cnt;
      break;
#endif //#if MAX_BUTTON_NB > 0

#if MAX_TOGGLESW_NB > 0
    case FT_PRIM_TOGGLESW:
      while (*str != 0)
      {
        st_Togglesw[number].str[cnt] = *str;
        str++;
        cnt++;
      }
      st_Togglesw[number].str[cnt] = 0x00;
      cnt++;
      temp = cnt & 0x0F;
      if ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
      {
        while ((temp != 0) && (temp != 4) && (temp != 8) && (temp != 12))
        {
          st_Togglesw[number].str[cnt] = 0x00;
          cnt++;
          temp = cnt & 0x0F;
        }
      }
      st_Togglesw[number].len = cnt;
      break;
#endif //#if MAX_TOGGLESW_NB > 0
  }
}