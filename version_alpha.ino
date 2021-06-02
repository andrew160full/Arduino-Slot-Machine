#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Fonts/FreeSansBold9pt7b.h>

#define TFT_CS                10
#define TFT_RST                8
#define TFT_DC                 9
#define SW_pin                12
#define X_pin                 A0 // joystick x axis pin
#define Y_pin                 A1 // joystick y axis pin
#define RED_PIN                4
#define GREEN_PIN              5
#define BLUE_PIN               6
#define buttonRight_pin        2
#define MAX_CREDIT_INITIAL 99995
#define MIN_CREDIT_INITIAL     5
#define MAX_CREDIT      99999995
#define MIN_CREDIT             5
#define box_size              24
#define ORANGE_TRIANGLE        0
#define BLUE_TRIANGLE          1
#define GREEN_SQUARE           2
#define MAGENTA_CIRCLE         3
#define LIGHTBLUE_DIAMOND      4
#define SEVEN                  5
#define OPTION_START           1
#define OPTION_BET             2
#define OPTION_RESTART         3
#define initial_x_axis       533
#define initial_y_axis       533

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

struct box {
   byte symbol;
   byte w0, h0;
};

long credit, new_credit; // in-game current credit
bool inMenu = false, ledOn = false;
struct box boxes[3][5]; // boxes that contains graphic symbols
struct box new_boxes[3][5]; // newly-generated boxes
long current_bet, new_bet, current_win;
int button_value = false; // changes when button is pressed
bool winner_boxes[3][5] = {false}; // check which boxes are winners
bool won = false;
unsigned long ledTimer, wonTimer = 1200;

void setup() {

   pinMode(SW_pin, INPUT);
   digitalWrite(SW_pin, HIGH);
   Serial.begin(9600);
   randomSeed(analogRead(2));
   
   tft.initR(INITR_144GREENTAB);
}

int getMultiplyFactor(int countSame, byte symbol) {

   int multiplyFactor = 0;

   if (countSame == 1) {
       return multiplyFactor;
   }
   if (countSame == 2 && symbol == MAGENTA_CIRCLE) {
     multiplyFactor = 2;
   } else if (countSame == 3) {
     multiplyFactor = 4;
   } else if (countSame == 4) {
     multiplyFactor = 10;
   } else if (countSame == 5) {
     multiplyFactor = 25;
   }
   if (symbol == SEVEN) {
     multiplyFactor *= 8;
   }
   return multiplyFactor;
}

void colorWinnerBoxes() {

   int x0, y0, w, h;


   if (won) {
     tft.getTextBounds(String(credit), 64, 82, &x0, &y0, &w, &h);
     tft.drawRect(x0, y0, w, h, 0XA000);
     tft.fillRect(x0, y0, w, h, 0XA000);
  
     tft.getTextBounds(String("Credit:"), 18, 82, &x0, &y0, &w, &h);
     tft.drawRect(x0, y0, w, h, 0XA000);
     tft.fillRect(x0, y0, w, h, 0XA000);
     
     tft.setCursor(18, 82);
     tft.setFont(NULL);
     tft.setTextColor(ST77XX_YELLOW);
     tft.setTextSize(1);
     tft.print("Won:");
  
     tft.setCursor(64, 82);
     tft.print(current_win);
  
     wonTimer = millis();
   }
   

   for (int i = 0; i < 3; i ++) {
     for (int j = 0; j < 5; j ++) {
        if (winner_boxes[i][j]) {
           // color current box in red
           tft.drawRect(boxes[i][j].w0, boxes[i][j].h0, box_size, box_size, ST77XX_RED);
           tft.fillRect(boxes[i][j].w0, boxes[i][j].h0, box_size, box_size, ST77XX_RED);
    
           drawSymbol(boxes[i][j].w0, boxes[i][j].h0, boxes[i][j].symbol);
        }
     }
   }


   // reset winner boxes
   for (int i = 0; i < 3; i++) {
     for (int j = 0; j < 5; j++) {
       winner_boxes[i][j] = false;
     }
   }
}

void checkWinnings() {

   int multiplyFactor = 0, countSame;
   byte symbol;

   current_win = 0;
   
   // check horizontal lines
   for (int i = 0; i < 3; i++) {
     symbol = boxes[i][0].symbol;
     countSame = 1;
     for (int j = 1; j < 5; j++) {
       if (boxes[i][j].symbol == symbol) {
         countSame ++;
       } else {
         break;
       }
     }
     
     multiplyFactor = getMultiplyFactor(countSame, symbol);
     if (multiplyFactor > 0) {
       won = true;
     }
     current_win += current_bet * multiplyFactor;

     if ((countSame == 2 && symbol == MAGENTA_CIRCLE) || countSame > 2) {
       for (int j = 0; j < countSame; j++) {
         winner_boxes[i][j] = true;
       }
     }
   }

   countSame = 0;
   // check for upper diagonal winnings
   for (int i = 2; i >= 0; i--) {
     if (i == 2) {
       symbol = boxes[i][0].symbol;
     } else if (symbol != boxes[i][i].symbol) {
       break;
     }
     countSame ++;
   }
   if ((countSame == 2 && symbol == MAGENTA_CIRCLE) || countSame > 2) {
     for (int i = countSame - 1; i >= 0; i --) {
       winner_boxes[i][i] = true;
     }
   }

   if (countSame == 3) {
     if (boxes[1][3].symbol == symbol) {
       countSame ++;
       winner_boxes[1][3] = true;

       if (boxes[2][4].symbol == symbol) {
         countSame ++;
         winner_boxes[2][4] = true;
       }
     }
   }

   multiplyFactor = getMultiplyFactor(countSame, symbol);
   if (multiplyFactor > 0) {
       won = true;
   }
   current_win += current_bet * multiplyFactor;

   countSame = 0;
   // check for lower diagonal winnings
   for (int i = 0; i < 3; i++) {
     if (i == 0) {
       symbol = boxes[i][0].symbol;
     } else if (symbol != boxes[i][i].symbol) {
       break;
     }
     countSame ++;
   }

   if ((countSame == 2 && symbol == MAGENTA_CIRCLE) || countSame > 2) {
     for (int i = 0; i < countSame; i++) {
       winner_boxes[i][i] = true;
     }
   }

   if (countSame == 3) {
     if (boxes[1][3].symbol == symbol) {
       countSame ++;
       winner_boxes[1][3] = true;

       if (boxes[0][4].symbol == symbol) {
         countSame ++;
         winner_boxes[0][4] = true;
       }
     }
   }

   multiplyFactor = getMultiplyFactor(countSame, symbol);
   if (multiplyFactor > 0) {
       won = true;
   }
   current_win += current_bet * multiplyFactor;

   // update current credit
   new_credit = credit + current_win;
   drawCreditText();
   credit = new_credit;

   if (won) {
     analogWrite(RED_PIN, 10);
     analogWrite(GREEN_PIN, 255);
     analogWrite(BLUE_PIN, 10);
     ledTimer = millis();
     ledOn = true;
   }
   
}

void drawStartOption(bool selected) {

   tft.setTextColor(ST77XX_YELLOW);
   if (selected) {
     tft.drawRoundRect(4, 92, 120, 11, 10, 0x4000);
     tft.fillRoundRect(4, 92, 120, 11, 10, 0x4000);
   } else {
     tft.drawRoundRect(4, 92, 120, 11, 10, 0xA000);
     tft.fillRoundRect(4, 92, 120, 11, 10, 0xA000);
   }
   tft.setCursor(50, 94);
   tft.setTextSize(1);
   tft.print("Start");
}

void drawBetOption(bool selected) {

   // bet option text
   if (selected) {
     tft.drawRoundRect(4, 104, 120, 11, 10, 0x4000);
     tft.fillRoundRect(4, 104, 120, 11, 10, 0x4000);
   } else {
      tft.drawRoundRect(4, 104, 120, 11, 10, 0xA000);
     tft.fillRoundRect(4, 104, 120, 11, 10, 0xA000);
   }
   tft.setCursor(19, 106);
   tft.setTextSize(1);
   tft.setTextColor(ST77XX_YELLOW);
   tft.print("Bet:");

   // bet option left arrow
   tft.drawTriangle(48, 106, 44, 109, 48, 112, ST77XX_YELLOW);
   tft.fillTriangle(48, 106, 44, 109, 48, 112, ST77XX_YELLOW);

   // bet option right arrow
   tft.drawTriangle(106, 106, 110, 109, 106, 112, ST77XX_YELLOW);
   tft.fillTriangle(106, 106, 110, 109, 106, 112, ST77XX_YELLOW);
  
   // bet option credit text
   tft.drawRect(52, 104, 52, 11, 0x0000);
   tft.fillRect(52, 104, 52, 11, 0x0000);
   tft.setTextColor(ST77XX_WHITE);
   tft.setCursor(54, 106);
   tft.print(current_bet);
}

void drawRestartOption(bool selected) {

   // restart game option text
   tft.setTextColor(ST77XX_YELLOW);
   if (selected) {
     tft.drawRoundRect(4, 116, 120, 11, 10, 0x4000);
     tft.fillRoundRect(4, 116, 120, 11, 10, 0x4000);
   } else {
     tft.drawRoundRect(4, 116, 120, 11, 10, 0xA000);
     tft.fillRoundRect(4, 116, 120, 11, 10, 0xA000);
   }
   tft.setCursor(29, 118);
   tft.setTextSize(1);
   tft.print("Restart game");
}

void printCreditText() {
   // credit text
   tft.setCursor(18, 82);
   tft.setFont(NULL);
   tft.setTextColor(ST77XX_YELLOW);
   tft.setTextSize(1);
   tft.print("Credit:");
}

void drawCreditText() {

   int x0, y0, w, h;

   tft.getTextBounds(String(credit), 64, 82, &x0, &y0, &w, &h);
   tft.drawRect(x0, y0, w, h, 0XA000);
   tft.fillRect(x0, y0, w, h, 0XA000);
   tft.setTextColor(ST77XX_YELLOW);
   tft.setCursor(64, 82);
   tft.print(new_credit);
}

void drawOrangeTriangle(byte w0, byte h0) {

   tft.drawTriangle(w0 + 12, h0 + 4, w0 + 4, h0 + 20, w0 + 20, h0 + 20, ST77XX_ORANGE);
   tft.fillTriangle(w0 + 12, h0 + 4, w0 + 4, h0 + 20, w0 + 20, h0 + 20, ST77XX_ORANGE);
}

void drawBlueTriangle(byte w0, byte h0) {

   tft.drawTriangle(w0 + 4, h0 + 4, w0 + 20, h0 + 4, w0 + 12, h0 + 20, ST77XX_BLUE);
   tft.fillTriangle(w0 + 4, h0 + 4, w0 + 20, h0 + 4, w0 + 12, h0 + 20, ST77XX_BLUE);
}

void drawGreenSquare(byte w0, byte h0) {

   tft.drawRect(w0 + 4, h0 + 4, 16, 16, ST77XX_GREEN);
   tft.fillRect(w0 + 4, h0 + 4, 16, 16, ST77XX_GREEN);

}

void drawMagentaCircle(byte w0, byte h0) {

   tft.drawCircle(w0 + 12, h0 + 12, 8, ST77XX_MAGENTA);
   tft.fillCircle(w0 + 12, h0 + 12, 8, ST77XX_MAGENTA);

}

void drawLightBlueDiamond(byte w0, byte h0) {

   tft.drawTriangle(w0 + 4, h0 + 12, w0 + 12, h0 + 4, w0 + 20, h0 + 12, ST77XX_CYAN);
   tft.fillTriangle(w0 + 4, h0 + 12, w0 + 12, h0 + 4, w0 + 20, h0 + 12, ST77XX_CYAN);

   tft.drawTriangle(w0 + 5, h0 + 13, w0 + 12, h0 + 20, w0 + 19, h0 + 13, ST77XX_CYAN);
   tft.fillTriangle(w0 + 5, h0 + 13, w0 + 12, h0 + 20, w0 + 19, h0 + 13, ST77XX_CYAN);
}

void drawSeven(byte w0, byte h0) {

   tft.drawRect(w0 + 6, h0 + 6, 12, 2, ST77XX_YELLOW);
   tft.fillRect(w0 + 6, h0 + 6, 12, 2, ST77XX_YELLOW);

   tft.drawTriangle(w0 + 14, h0 + 8, w0 + 10, h0 + 18, w0 + 12, h0 + 18, ST77XX_YELLOW);
   tft.fillTriangle(w0 + 14, h0 + 8, w0 + 10, h0 + 18, w0 + 12, h0 + 18, ST77XX_YELLOW);

   tft.drawTriangle(w0 + 16, h0 + 8, w0 + 14, h0 + 8, w0 + 12, h0 + 18, ST77XX_YELLOW);
   tft.fillTriangle(w0 + 16, h0 + 8, w0 + 14, h0 + 8, w0 + 12, h0 + 18, ST77XX_YELLOW); 
}

void drawSymbol(byte w0, byte h0, byte symbol) {

   switch(symbol) {

     case ORANGE_TRIANGLE:
       drawOrangeTriangle(w0, h0);
       break;

     case BLUE_TRIANGLE:
       drawBlueTriangle(w0, h0);
       break;

     case GREEN_SQUARE:
       drawGreenSquare(w0, h0);
       break;

     case MAGENTA_CIRCLE:
       drawMagentaCircle(w0, h0);
       break;

     case LIGHTBLUE_DIAMOND:
       drawLightBlueDiamond(w0, h0);
       break;

     case SEVEN:
       drawSeven(w0, h0);
       break;
   }
}

void drawMainMenu() {

   int x1, y1, w, h, speed, x_axis, y_axis;
   long newCredit;

   credit = 5; // initial credit
  
   tft.fillScreen(ST77XX_BLACK);
   
   tft.drawRoundRect(2, 2, 124, 124, 10, 0XF6E7);
   tft.fillRoundRect(2, 2, 124, 124, 10, 0XF6E7);

   tft.setTextColor(ST77XX_BLACK);
   tft.setTextSize(1);
   tft.setTextWrap(false);
   
   // select credit text
   tft.setCursor(7, 32);
   tft.setFont(&FreeSansBold9pt7b);
   tft.print("Select credit:");


   // credit number text box
   tft.drawRect(29, 54, 67, 26, 0x0000);
   tft.fillRect(30, 54 + 1, 65, 24, 0xFFFF);

   // draw two triangles next to credit number text box
   tft.drawTriangle(26, 54, 14, 66, 26, 78, 0x0000);
   tft.fillTriangle(26, 54, 14, 66, 26, 78, 0x0000);

   tft.drawTriangle(98, 54, 110, 66, 98, 78, 0x0000);
   tft.fillTriangle(98, 54, 110, 66, 98, 78, 0x0000);

   // draw red container for 'play game'
   tft.drawRoundRect(10, 86, 108, 30, 10, ST77XX_RED);
   tft.fillRoundRect(10, 86, 108, 30, 10, ST77XX_RED);

   // play game text
   tft.setCursor(19, tft.height() / 2 + 42);
   tft.print("Play game");

   tft.setCursor(tft.width() / 2 - 30, 74);
   tft.print(credit);


   while (true) {

     button_value = digitalRead(buttonRight_pin);
     if (button_value != 0) {
       drawGame();
     }
     y_axis = analogRead(Y_pin);

     if (abs(y_axis - initial_y_axis) >= 10) {

       if (abs(y_axis - initial_y_axis) / 100 >= 1) {
         speed = ((y_axis - initial_y_axis) / 100);

         if (abs(speed) == 4) {
           speed *= 2;
         }
         if (speed > 1) {
           speed *= speed;
         } else if (speed < -1) {
           speed *= -speed;
         }
         
         if (credit + 5 * speed < MIN_CREDIT_INITIAL) {
            newCredit = MIN_CREDIT_INITIAL;
         } else if (credit + 5 * speed > MAX_CREDIT_INITIAL) {
            newCredit = MAX_CREDIT_INITIAL;
         } else {
            newCredit = credit + 5 * speed;
         }

         tft.getTextBounds(String(credit), tft.width() / 2 - 30, 74, &x1, &y1, &w, &h);
         tft.drawRect(x1, y1, w, h, 0xFFFF);
         tft.fillRect(x1, y1, w, h, 0xFFFF);
         credit = newCredit;
         tft.setCursor(tft.width() / 2 - 30, 74);
         tft.print(credit);
         delay(150);
       }
     }
   }  
}

void drawGame() {

   byte current_option = OPTION_START;
   bool start = false;
   int x0, y0, w, h, speed, x_axis, y_axis;
   unsigned long optionTimer = 200, timerBetOption = 200;

   current_bet = 5;

   tft.fillScreen(ST77XX_BLACK);
   tft.setFont();

   // upper gray container
   tft.drawRect(0, 0, 128, 78, 0x6B4D);
   tft.fillRect(0, 0, 128, 78, 0x6B4D);

   // generate first boxes
   for (int i = 0; i < 3; i++) {
     for (int j = 0; j < 5; j++) {
        boxes[i][j].w0 = 2 + j * (box_size + 1);
        boxes[i][j].h0 = 2 + i * (box_size + 1);
        boxes[i][j].symbol = random(6);

        // decrease chance to get 'seven' symbol
        if (boxes[i][j].symbol == SEVEN) {
          boxes[i][j].symbol = random(6);
        }
    
        tft.drawRect(boxes[i][j].w0, boxes[i][j].h0, box_size, box_size, 0x5008);
        tft.fillRect(boxes[i][j].w0, boxes[i][j].h0, box_size, box_size, 0x5008);
        drawSymbol(boxes[i][j].w0, boxes[i][j].h0, boxes[i][j].symbol);
     }
   }

   // red lower container
   tft.drawRect(0, 79, 128, 50, 0xA000);
   tft.fillRect(0, 79, 128, 50, 0xA000);

   // horizontal line between containers
   tft.drawFastHLine(0, 78, 128, 0x0000);

   printCreditText();

   new_credit = credit;
   drawCreditText();

   // draw start option container and text
   drawStartOption(true);

   // draw bet option container and text
   drawBetOption(false);

   // draw restart option container and text
   drawRestartOption(false);

   while (true) {
     x_axis = analogRead(X_pin);

     if (won && millis() - wonTimer >= 1200) {

       // clear current winning text
       tft.getTextBounds(String(current_win), 64, 82, &x0, &y0, &w, &h);
       tft.drawRect(x0, y0, w, h, 0XA000);
       tft.fillRect(x0, y0, w, h, 0XA000);

       tft.getTextBounds(String("Won:"), 18, 82, &x0, &y0, &w, &h);
       tft.drawRect(x0, y0, w, h, 0XA000);
       tft.fillRect(x0, y0, w, h, 0XA000);

       printCreditText();
    
       new_credit = credit;
       drawCreditText();
     
       won = false;
     }

     if (ledOn && millis() - ledTimer > 800) {
       ledOn = false;
       analogWrite(RED_PIN, 0);
       analogWrite(GREEN_PIN, 0);
       analogWrite(BLUE_PIN, 0);
     }

     // if joystick was moved vertically
     if (millis() - optionTimer >= 200 && abs(x_axis - initial_x_axis) >= 400) {

       // joystick was moved upwards
       if (x_axis - initial_x_axis >= 400) {
         optionTimer = millis();

         if (current_option == OPTION_BET) {

           drawBetOption(false);
           drawStartOption(true);
           current_option = OPTION_START;
         } else if (current_option == OPTION_RESTART) {

           drawRestartOption(false);
           drawBetOption(true);
           current_option = OPTION_BET;
         }
       } else if (millis() - optionTimer >= 200) {
         // joystick was moved downwards
         optionTimer = millis();

         if (current_option == OPTION_START) {

           drawStartOption(false);
           drawBetOption(true);
           current_option = OPTION_BET;

         } else if (current_option == OPTION_BET) {

           drawBetOption(false);
           drawRestartOption(true);
           current_option = OPTION_RESTART;
         }
       }
     }
     button_value = digitalRead(buttonRight_pin);

     if (current_option == OPTION_START && button_value != 0 && credit >= 5
        && current_bet <= credit) {
       start = true;
     } else if (current_option == OPTION_RESTART && button_value != 0) {
       drawMainMenu();
     } else if (current_option == OPTION_BET && millis() - timerBetOption >= 250) {
        timerBetOption = millis();

        y_axis = analogRead(Y_pin);

        // if joystick moved horizontally
        if (abs(y_axis - initial_y_axis) >= 20 && credit >= 5) {
  
         if (abs(y_axis - initial_y_axis) / 100 >= 1) {
           speed = ((y_axis - initial_y_axis) / 100);

           if (speed == 4) {
             speed = 10;
           } else if (speed == -4) {
             speed = -10;
           }
           if (speed > 2) {
             speed *= speed;
           } else if (speed < -2) {
             speed *= -speed;
           }
           
           if (current_bet + 5 * speed < MIN_CREDIT) {
              new_bet = MIN_CREDIT_INITIAL;
           } else if (current_bet + 5 * speed > credit) {
              new_bet = credit;
           } else {
              new_bet = current_bet + 5 * speed;
           }
  
           tft.getTextBounds(String(current_bet), 54, 106, &x0, &y0, &w, &h);
           tft.drawRect(x0, y0, w, h, 0X0000);
           tft.fillRect(x0, y0, w, h, 0X0000);
           current_bet = new_bet;
           tft.setTextColor(ST77XX_WHITE);
           tft.setCursor(54, 106);
           tft.print(current_bet);
           tft.setTextColor(ST77XX_YELLOW);
         }
       }
     }
    
     // if start option was selected
     if (start) {

       // clear current winning value text
       tft.getTextBounds(String("Won:"), 18, 82, &x0, &y0, &w, &h);
       tft.drawRect(x0, y0, w, h, 0XA000);
       tft.fillRect(x0, y0, w, h, 0XA000);

       printCreditText();

       new_credit = credit - current_bet;
       drawCreditText();
       credit = new_credit;

       start = false ;
       for (int i = 0; i < 3; i++) {
         for (int j = 0; j < 5; j++) {
            // lower the old boxes so they stay correct distance from each other
            boxes[i][j].h0 ++;
            // generate new boxes
            new_boxes[i][j].w0 = 2 + j * (box_size + 1);
            new_boxes[i][j].h0 = 2 + i * (box_size + 1) - 75;
            new_boxes[i][j].symbol = random(6);
    
            tft.drawRect(boxes[i][j].w0, boxes[i][j].h0, box_size, box_size, 0x5008);
            tft.fillRect(boxes[i][j].w0, boxes[i][j].h0, box_size, box_size, 0x5008);
    
            tft.drawRect(new_boxes[i][j].w0, new_boxes[i][j].h0, box_size, box_size, 0x5008);
            tft.fillRect(new_boxes[i][j].w0, new_boxes[i][j].h0, box_size, box_size, 0x5008);
    
            drawSymbol(boxes[i][j].w0, boxes[i][j].h0, boxes[i][j].symbol);
            drawSymbol(new_boxes[i][j].w0, new_boxes[i][j].h0, new_boxes[i][j].symbol);
         }
       }
    
       register bool draw_new_boxes;
    
       while (new_boxes[0][0].h0 != 2) {
         // upper gray container
         tft.drawRect(0, 0, 128, 78, 0x6B4D);
         tft.fillRect(0, 0, 128, 78, 0x6B4D);
    
         for (int i = 0; i < 3; i++) {
    
           draw_new_boxes = true;
           // draw old boxes
           if (boxes[i][0].h0 + 5 < 78) {
    
              register byte height = boxes[i][0].h0 + 4;
              register byte diff = 0;
    
              draw_new_boxes = false;
    
              if (height + box_size >= 78) {
                diff = height + box_size - 78;
              }
                
              for (int j = 0; j < 5; j++) {
                 boxes[i][j].h0 += 5;
                 new_boxes[i][j].h0 += 5;
      
                 tft.drawRect(boxes[i][j].w0, height, box_size, box_size - diff, 0x5008);
                 tft.fillRect(boxes[i][j].w0, height, box_size, box_size - diff, 0x5008);
                 tft.drawRect(new_boxes[i][j].w0, new_boxes[i][j].h0, box_size, box_size, 0x5008);
                 tft.fillRect(new_boxes[i][j].w0, new_boxes[i][j].h0, box_size, box_size, 0x5008);
    
                 if (height + box_size < 80) {
                    drawSymbol(boxes[i][j].w0, height, boxes[i][j].symbol);
                 }
                 drawSymbol(new_boxes[i][j].w0, new_boxes[i][j].h0, new_boxes[i][j].symbol);
              }
    
           }
           
           // draw new boxes
           if (draw_new_boxes == true) {
             for (int j = 0; j < 5; j++) {
                  new_boxes[i][j].h0 += 5;
    
                  tft.drawRect(new_boxes[i][j].w0, new_boxes[i][j].h0, box_size, box_size, 0x5008);
                  tft.fillRect(new_boxes[i][j].w0, new_boxes[i][j].h0, box_size, box_size, 0x5008);
                  drawSymbol(new_boxes[i][j].w0, new_boxes[i][j].h0, new_boxes[i][j].symbol);
             }
           }
           
           delay(10);
         }
       }
    
       for (int i = 0; i < 3; i++) {
         for (int j = 0; j < 5; j++) {
           boxes[i][j] = new_boxes[i][j];
         }
       }
       checkWinnings();
       colorWinnerBoxes();
     }

   }
   
}

void loop() {

   if (inMenu == false) {
     drawMainMenu();
   }
   inMenu = true;
}
