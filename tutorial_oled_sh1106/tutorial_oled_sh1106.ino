//https://github.com/olikraus/u8g2/wiki/u8g2reference#drawxbm
//https://github.com/olikraus/u8g2/wiki/fntlist8#8-pixel-height
//https://ezgif.com/gif-to-jpg/ezgif-1-6895ff7004.gif
//https://sourceforge.net/projects/frhed/files/1.%20Stable%20Releases/1.6.0/Frhed-1.6.0-Setup.exe/download
//https://github.com/L33t-dot-UK/U8g2_Tutorials
#include <U8g2lib.h>
#include "frames.h"
/********* ISTANZA CLASSE DISPLAY OLED I2C SH1106 128X64 ************/
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

/********* DEFINIZIONE CLASSE HELPER TIMER ************/
class TimerC {

  private:
        unsigned long _start_time;
        unsigned long _elapsed_time;
  
  public:
      TimerC()
      {
        _start_time = 0;
        _elapsed_time = 0;
      }

      void start()
      {
        if(_start_time != 0)
        {
          return;
        }
        _start_time = millis();
      }
      
      unsigned long getET()
      {
        //in millisecondi
        _elapsed_time = millis() - _start_time;
        return _elapsed_time;
      }
      double getETSec()
      {
        unsigned long et_milli =  getET();
        return et_milli/1000.0;
      }
      void reset()
      {
        _start_time = millis();
        _elapsed_time = 0;
      }
      void stop_()
      {
        if(_start_time == 0)
        {
          return;
        }
        _start_time = 0;
        _elapsed_time = 0;
        
      }
};

class SnowGenerator
{
  public:
  static const uint8_t nfiocchi=3;
  uint8_t vRandX[64][nfiocchi]  = {{}};
  //uint8_t vRandY[64]  = {};
  uint8_t currentY;
  bool start_over;

  private:
  U8G2_SH1106_128X64_NONAME_F_HW_I2C* _u8g2;

  public:
    SnowGenerator(U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2_)
    {
      _u8g2 = u8g2_;
      randomSeed(analogRead(0));
      for(int i=0; i<nfiocchi; ++i)
      {
        vRandX[0][i] = (uint8_t)random(0, 128);
      }
      currentY=0;
      start_over=false;
      
    }
    void Animate(bool draw)
    {     
      //shifto ogni riga di +1 : e.g, la riga y alla riga y+1
      for(int y=62; y>=0; --y)
      {
        for(int i=0; i<nfiocchi; ++i)
        {
          vRandX[y+1][i] = vRandX[y][i];
        }
      }
            
      //creo la neve virtuale per la riga 0     
      for(int i=0; i<nfiocchi; ++i)
      {
        vRandX[0][i] = (uint8_t)random(0, 128);
      }

      if (draw)
      {
        for(int y=0; y<64; ++y)
        {
          if(!start_over)
          {
            if(y>currentY)
            {
              break;
            }
          }
          for(int x=0; x<nfiocchi; ++x)
          {
            
            this->_u8g2->drawPixel(vRandX[y][x],y);
          }
          
        }
      }
    
      ++currentY;
      if (currentY>=63)
      {
        currentY=0;
        start_over=true;
      }
    }
};

enum TipoAnimazione
{
  oscillazione,
  movimento
};

class BitmapAnimation
{
  public:
    int nframe;
    int display_height;
    int display_width;
    int current_frame;
    int X;
    int Y;
    int msChangeFrame;
    int frame_height;
    int frame_width;
    int Xincrement;
    double Yincrement;

  private:
    int StartY;
    double current_time;
    bool first_scan;
    TimerC* timerFrames;
    TimerC* timerMove;
    double phaseY;
    TipoAnimazione taY;


  public:
    BitmapAnimation(int dh_, int dw_, int fh_, int fw_, int nframe_, int startX_, int startY_, int msChangeFrame_, int Xincrement_, double Yincrement_, TipoAnimazione taY_)
    {
        frame_height=fh_;
        frame_width=fw_;
        nframe=nframe_;
        display_height = dh_;
        display_width = dw_;
        current_frame=0;
        X=startX_;
        taY = taY_;
        Yincrement = Yincrement_;
        phaseY = 0;
        if (taY==TipoAnimazione::movimento)
        {
          Y=startY_; 
        }
        else
        {
          Y=0;
          StartY = startY_;         
        }             
        Xincrement=Xincrement_;
        current_time = 0;
        first_scan = true;
        timerFrames = new TimerC();
        timerMove = new TimerC();
        timerFrames->start();
        timerMove->start();
    }

    void Animate(bool anim)
    {
        if (anim)
        {
            if(timerFrames->getET()>=msChangeFrame)
            {
              timerFrames->reset();
              //cambia frame
              ++current_frame;
              if(current_frame>=nframe)
                current_frame=0;
            }
            

            //Per la Y eseguo un movimento di "fluttuazione" nell'aria della slitta
            //lo simulo tramite una sinusoide
            long et_move = timerMove->getET();
            if(et_move>0)
            {
              timerMove->reset();

              X+=Xincrement;
              //se il punto X=0 (corner in alto a SX) è fuori dallo schermo a destra:
              if (X>display_width)
              {
                //allora porto tutta l'immagine sul lato esterno sinistro dello schermo
                //in questo modo il bordo destro dell'immagine si trova alla cordinata X=0.
                //L'immagine cosi esce da destra e rientra da sinistra
                X = -frame_width;
                phaseY=0;
              }
              

              //current_time += et_move*0.001; //in sec
              //Y = StartY + 3*sin(TWO_PI*100*current_time);
              if(taY==TipoAnimazione::oscillazione)
              {
                  Y = StartY + 5*sin(phaseY);
                  phaseY+=Yincrement; //M_PI/8;
              }
              else
              {
                Y+=static_cast<int>(Yincrement);
              }
              

              //Serial.println(Y);
            }
            

        }
        else
        {
          //statica
        }
    }



};

                                                  //int dh_, int dw_, int fh_, int fw_, int nframe_, int startX_, int startY_, int msChangeFrame_, int Xincrement_, double Yincrement_, TipoAnimazione taY_
BitmapAnimation santa_claus_anim = BitmapAnimation(u8g2.getDisplayHeight(),
                                                  u8g2.getDisplayWidth(),
                                                  frame_height_, //altezza dei frame in pixels
                                                  frame_width_, //larghezza dei frame in pixels
                                                  animation_frames, //numero frames totali
                                                  0, //StartX pixels
                                                  15,//StartY pixels
                                                  100,//tempo di campionamento nuovo frame [millisec]
                                                  5, //Incremento in X [pixels]
                                                  M_PI/8, //Incremento in Y [pixels o Angolo in radianti]
                                                  TipoAnimazione::oscillazione); //tipo di aniamzione lungo Y
SnowGenerator snow = SnowGenerator(&u8g2);

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setBitmapMode(1); //set the background to transparant
}

void loop() 
{

  santa_claus_anim.Animate(true);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(4,16,"Happy Xmas");
  snow.Animate(true);
  


  u8g2.drawXBMP(santa_claus_anim.X,
                santa_claus_anim.Y, 
                santa_claus_anim.frame_width, 
                santa_claus_anim.frame_height, 
                list_of_frames[santa_claus_anim.current_frame]);
  u8g2.sendBuffer();

  //u8g2.writeBufferXBM(Serial);

  delay(10);

}
