/*
 * File:   FT8XX.h
 * Author: Intellitrol
 *
 * Created on 28 juin 2016, 09:13
 */

#ifndef __FT8XX_H_
#define __FT8XX_H_

#include "FT_Gpu.h"
#include "FT8XX_user_definition.h"

//Structure definition for FT8XX primitives. Define a number for each of them in
//FT8XX_user_define.h. 

//Module initializations functions

class FT8XX_EVE 
{
   public :
      void init (void); 
      void set_touch_tag (unsigned char prim_type, unsigned char prim_num, unsigned char tag_num);
      unsigned char read_touch_tag (void);
      unsigned char get_touch_tag (void);
      void clear_touch_tag (void);
      void touchpanel_init (void);
      STTouch touchpanel_read (STTouch touch_read);
      void start_new_dl (void);
      void update_screen_dl (void);  
      void write_dl_char (unsigned char data);
      void write_dl_int (unsigned int data);
      void write_dl_long (unsigned long data);
      void host_command (unsigned char cmd);
      void wr8 (unsigned long adr, unsigned char data);
      void wr16(unsigned long adr, unsigned int data);
      void wr32(unsigned long adr, unsigned long data);
      unsigned char rd8 (unsigned long adr);
      unsigned int rd16 (unsigned long adr);
      unsigned long rd32 (unsigned long adr);
      unsigned int inc_cmd_offset (unsigned int cur_off, unsigned char cmd_size);
      unsigned int get_cmd_offset_value (void);

      void clear_screen (unsigned long color);
      void set_context_color (unsigned long color);
      void set_context_bcolor (unsigned long color);
      void set_context_fcolor (unsigned long color);
      
      // 
      void draw_line_h (unsigned int x1, unsigned int x2, unsigned int y, unsigned int w);
      void draw_line_v (unsigned int y1, unsigned int y2, unsigned int x, unsigned int w);
      void draw_point (unsigned int x, unsigned int y, unsigned int r);
      void modify_element_string (unsigned char number, unsigned char type, char *str);  
      void write_bitmap (const unsigned char *img_ptr, const unsigned char *lut_ptr, unsigned long img_length, unsigned long base_adr);    

      void CMD_tracker(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char tag);

      #if MAX_GRADIENT_NB > 0
         void CMD_gradient(unsigned char number, unsigned int x0, unsigned int y0, unsigned long rgb0, unsigned int x1, unsigned int y1, unsigned long rgb1);
         void draw_gradient (STGradient *st_Gradient);
         void modify_gradient (STGradient *st_Gradient, unsigned char type, unsigned long value);
         unsigned char get_gradient_nb (void);
      #endif

      #if MAX_KEYS_NB > 0
         void CMD_keys(unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int f, unsigned int opt, char *str);
         void draw_keys(STKeys *st_Keys);
         unsigned char get_keys_nb (void);
      #endif

      #if MAX_WINDOW_NB > 0
         unsigned char check_window (STWindow *st_Window, STTouch touch_data);
         unsigned char check_touch (STWindow *st_Window, STTouch touch_data);
         void CMD_window (unsigned char number, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
         unsigned char check_screen_press (STTouch touch_data);
      #endif

      #if MAX_SLIDER_NB > 0
         void CMD_slider (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int v, unsigned int r);
         //unsigned int slider_update (STTouch touch_read, STSlider *st_Slider, STWindow *st_Window);
         void draw_slider (STSlider *st_Slider);
         void modify_slider (STSlider *st_Slider, unsigned char type, unsigned int value);
         unsigned char get_slider_nb (void);
      #endif

      #if MAX_TEXT_NB > 0
         void CMD_text (unsigned char number, unsigned int x, unsigned int y, unsigned int f, unsigned int o, const char *str);
         void draw_text (STText *st_Text);
         unsigned char get_text_nb (void);
      #endif

      #if MAX_NUMBER_NB > 0
         void CMD_number (unsigned char number, unsigned int x, unsigned int y, unsigned int f, unsigned int o, unsigned long n);
         void draw_number (STNumber *st_Number);
         void modify_number (STNumber *st_Number, unsigned char type, unsigned long value);
         unsigned char get_number_nb (void);
      #endif

      #if MAX_BUTTON_NB > 0
         void CMD_button (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int f, unsigned int o, const char *str);
         void draw_button (STButton *st_Button);
         void modify_button (STButton *st_Button, unsigned char type, unsigned int value);
         void modify_button_string (STButton *st_Button, char *str, unsigned char length);
         unsigned char get_button_nb (void);
      #endif

      #if MAX_TOGGLE_NB > 0
         void CMD_toggle (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int f, unsigned int o, unsigned char state, const char *str);
         void draw_toggle (STToggle *st_Toggle);
         void change_toggle_state (STToggle *st_Toggle, unsigned char state);
         unsigned char get_toggle_nb (void);
      #endif

      #if MAX_CLOCK_NB > 0
         void CMD_clock (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned char h, unsigned char m, unsigned char s, unsigned char ms);
         void modify_clock_hms (STClock *st_Clock, unsigned char h, unsigned char m, unsigned char s);
         void draw_clock (STClock *st_Clock);
         unsigned char get_clock_nb (void);
      #endif

      #if MAX_SCROLLBAR_NB > 0
         void CMD_scrollbar (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int val, unsigned int size, unsigned int range);
         void draw_scrollbar  (STScrollbar *st_Scrollbar);
         void modify_scrollbar (STScrollbar *st_Scrollbar, unsigned char type, unsigned int value);
         unsigned char get_scrollbar_nb (void);
      #endif

      #if MAX_PROGRESS_NB > 0
         void CMD_progress (unsigned char number, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int opt, unsigned int val, unsigned int range);
         void draw_progress (STProgress *st_Progress);
         void modify_progress (STProgress *st_Progress, unsigned char val);
         unsigned char get_progress_nb (void);
      #endif

      #if MAX_GAUGE_NB > 0
         void CMD_gauge (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned int maj, unsigned int min, unsigned int val, unsigned int range);
         void draw_gauge (STGauge *st_Gauge);
         void modify_gauge (STGauge *st_Gauge, unsigned char type, unsigned int value);
         unsigned char get_gauge_nb (void);
      #endif

      #if MAX_DIAL_NB > 0
         void CMD_dial (unsigned char number, unsigned int x, unsigned int y, unsigned int r, unsigned int opt, unsigned int val);
         void draw_dial (STDial *st_Dial);
         unsigned char get_dial_nb (void);
      #endif

      #if MAX_RECT_NB > 0
         void CMD_rectangle (unsigned char number, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int w);
         void draw_rectangle (STRectangle *st_Rectangle);
         unsigned char get_rectangle_nb (void);
      #endif  

   private :
         unsigned char rectangle_nb;
         unsigned char dial_nb;
         unsigned char gauge_nb;
         unsigned char progress_nb;
         unsigned char scrollbar_nb;
         unsigned char clock_nb;
         unsigned char toggle_nb;
         unsigned char button_nb;
         unsigned char number_nb;
         unsigned char text_nb;
         unsigned char slider_nb;
         unsigned char keys_nb;
         unsigned char gradient_nb;
         unsigned char touch_tag;
         unsigned char tag_num;
   protected :

};

#endif
